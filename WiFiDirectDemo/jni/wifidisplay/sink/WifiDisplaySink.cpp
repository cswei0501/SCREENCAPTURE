/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "WifiDisplaySink"
#include <utils/Log.h>

#include "WifiDisplaySink.h"
#include "ParsedMessage.h"
#include "RTPSink.h"
#include "Parameters.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <media/mediaplayer.h>

#include "iniparser.h"
#define LOGI ALOGI
#define LOGE ALOGE
namespace android {
dictionary* pini = 0;

WifiDisplaySink::WifiDisplaySink(
        const sp<ANetworkSession> &netSession,
        const sp<ISurfaceTexture> &surfaceTex,
        const sp<WfdPlayerNotify> &playerNotify)
    : mState(UNDEFINED),
      mNetSession(netSession),
      mSurfaceTex(surfaceTex),
      mSessionID(0),
      mNextCSeq(1),
      mPlayerNotify(playerNotify) {
	pini = iniparser_load("/sdcard/wfd.ini");
	if(pini ==0)
		ALOGW("can't find wfd.ini\n");
}

WifiDisplaySink::~WifiDisplaySink() {
	if(pini)
		iniparser_freedict(pini);
}

void WifiDisplaySink::start(const char *sourceHost, int32_t sourcePort) {
    sp<AMessage> msg = new AMessage(kWhatStart, id());
    msg->setString("sourceHost", sourceHost);
    msg->setInt32("sourcePort", sourcePort);
    msg->post();
}

void WifiDisplaySink::start(const char *uri) {
    sp<AMessage> msg = new AMessage(kWhatStart, id());
    msg->setString("setupURI", uri);
    msg->post();
}

void WifiDisplaySink::stop(void) {
	sp<AMessage> msg = new AMessage(kWhatStop, id());
	sp<AMessage> response = new AMessage();
	msg->postAndAwaitResponse(&response);
	looper()->stop();
}

// static
bool WifiDisplaySink::ParseURL(
        const char *url, AString *host, int32_t *port, AString *path,
        AString *user, AString *pass) {
    host->clear();
    *port = 0;
    path->clear();
    user->clear();
    pass->clear();

    if (strncasecmp("rtsp://", url, 7)) {
        return false;
    }

    const char *slashPos = strchr(&url[7], '/');

    if (slashPos == NULL) {
        host->setTo(&url[7]);
        path->setTo("/");
    } else {
        host->setTo(&url[7], slashPos - &url[7]);
        path->setTo(slashPos);
    }

    ssize_t atPos = host->find("@");

    if (atPos >= 0) {
        // Split of user:pass@ from hostname.

        AString userPass(*host, 0, atPos);
        host->erase(0, atPos + 1);

        ssize_t colonPos = userPass.find(":");

        if (colonPos < 0) {
            *user = userPass;
        } else {
            user->setTo(userPass, 0, colonPos);
            pass->setTo(userPass, colonPos + 1, userPass.size() - colonPos - 1);
        }
    }

    const char *colonPos = strchr(host->c_str(), ':');

    if (colonPos != NULL) {
        char *end;
        unsigned long x = strtoul(colonPos + 1, &end, 10);

        if (end == colonPos + 1 || *end != '\0' || x >= 65536) {
            return false;
        }

        *port = x;

        size_t colonOffset = colonPos - host->c_str();
        size_t trailing = host->size() - colonOffset;
        host->erase(colonOffset, trailing);
    } else {
        *port = 554;
    }

    return true;
}

void WifiDisplaySink::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatStart:
        {
            int32_t sourcePort;

            if (msg->findString("setupURI", &mSetupURI)) {
                AString path, user, pass;
                CHECK(ParseURL(
                            mSetupURI.c_str(),
                            &mRTSPHost, &sourcePort, &path, &user, &pass)
                        && user.empty() && pass.empty());
            } else {
                CHECK(msg->findString("sourceHost", &mRTSPHost));
                CHECK(msg->findInt32("sourcePort", &sourcePort));
            }

            sp<AMessage> notify = new AMessage(kWhatRTSPNotify, id());

            status_t err = mNetSession->createRTSPClient(
                    mRTSPHost.c_str(), sourcePort, notify, &mSessionID);
            CHECK_EQ(err, (status_t)OK);

            mState = CONNECTING;
            break;
        }

        case kWhatRTSPNotify:
        {
            int32_t reason;
            CHECK(msg->findInt32("reason", &reason));

            switch (reason) {
                case ANetworkSession::kWhatError:
                {
                    int32_t sessionID;
                    CHECK(msg->findInt32("sessionID", &sessionID));

                    int32_t err;
                    CHECK(msg->findInt32("err", &err));

                    AString detail;
                    CHECK(msg->findString("detail", &detail));

                    ALOGE("An error occurred in session %d (%d, '%s/%s').",
                          sessionID,
                          err,
                          detail.c_str(),
                          strerror(-err));

                    if (sessionID == mSessionID) {
                        ALOGI("Lost control connection.");

                        // The control connection is dead now.
                        mNetSession->destroySession(mSessionID);
                        mSessionID = 0;

                        if (mPlayerNotify != NULL) {
                        	mPlayerNotify->notify(MEDIA_PLAYBACK_COMPLETE, 1, err);
                        }
                    }
                    break;
                }

                case ANetworkSession::kWhatConnected:
                {
                    ALOGI("We're now connected.");
                    mState = CONNECTED;

                    if (!mSetupURI.empty()) {
                        status_t err =
                            sendDescribe(mSessionID, mSetupURI.c_str());

                        CHECK_EQ(err, (status_t)OK);
                    }
                    break;
                }

                case ANetworkSession::kWhatData:
                {
                    onReceiveClientData(msg);
                    break;
                }

                case ANetworkSession::kWhatBinaryData:
                {
                    CHECK(sUseTCPInterleaving);

                    int32_t channel;
                    CHECK(msg->findInt32("channel", &channel));

                    sp<ABuffer> data;
                    CHECK(msg->findBuffer("data", &data));

                    mRTPSink->injectPacket(channel == 0 /* isRTP */, data);
                    break;
                }

                default:
                    TRESPASS();
            }
            break;
        }

        case kWhatStop:
        {
        	unsigned int replyId;
        	msg->senderAwaitsResponse(&replyId);

        	if (mState >= CONNECTED) {
        		status_t err = sendTearDown(mSessionID);

        		if (err == OK) {
        			sp<AMessage> timeOutMsg = new AMessage(kWhatTeardownTriggerTimedOut, id());
        			timeOutMsg->setInt32("ReplyID", replyId);
        			timeOutMsg->post(1 * 1000000ll);
        		}
        		else {
        			sp<AMessage> response = new AMessage;
        			response->setInt32("err", err);
        			response->postReply(replyId);
        		}
        	}
        	else {
        		sp<AMessage> response = new AMessage;
        		response->setInt32("err", 0);
        		response->postReply(replyId);
        	}

            break;
        }

        case kWhatTeardownTriggerTimedOut:
        {
        	ALOGI("kWhatTeardownTriggerTimedOut received");
        	sp<AMessage> response = new AMessage;
        	int replyId;
        	msg->findInt32("ReplyID", &replyId);
        	ALOGI("ReplyID %u", replyId);
        	response->setInt32("err", 0);
        	response->postReply(replyId);

        	break;
        }

        default:
            TRESPASS();
    }
}

void WifiDisplaySink::registerResponseHandler(
        int32_t sessionID, int32_t cseq, HandleRTSPResponseFunc func) {
    ResponseID id;
    id.mSessionID = sessionID;
    id.mCSeq = cseq;
    mResponseHandlers.add(id, func);
}

status_t WifiDisplaySink::sendM2(int32_t sessionID) {
    AString request = "OPTIONS * RTSP/1.0\r\n";
    AppendCommonResponse(&request, mNextCSeq);


	char* s = 0;
	s = GetConfiguration("M2:request");
	if(s)
	{
		request.append(s);
		request.append("\r\n\r\n");
	}
	else
	{
	    request.append(
	            "Require: org.wfa.wfd1.0\r\n"
	            "\r\n");
	}

	ALOGV("-----------------SEND-------------------- \n");
  	ALOGV("%s",request.c_str());
  	ALOGV("----------------------------------------- ");
  status_t err =
        mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID, mNextCSeq, &WifiDisplaySink::onReceiveM2Response);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::onReceiveM2Response(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

	ALOGV("[%s] Success  \n ",__func__);
    return OK;
}

status_t WifiDisplaySink::onReceiveDescribeResponse(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

    ALOGV("[%s] Success  \n ",__func__);
    return sendSetup(sessionID, mSetupURI.c_str());
}

status_t WifiDisplaySink::onReceiveSetupResponse(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

    if (!msg->findString("session", &mPlaybackSessionID)) {
        return ERROR_MALFORMED;
    }

    ALOGV("[%s] Success  \n ",__func__);

    if (!ParsedMessage::GetInt32Attribute(
                mPlaybackSessionID.c_str(),
                "timeout",
                &mPlaybackSessionTimeoutSecs)) {
        mPlaybackSessionTimeoutSecs = -1;
    }

    ssize_t colonPos = mPlaybackSessionID.find(";");
    if (colonPos >= 0) {
        // Strip any options from the returned session id.
        mPlaybackSessionID.erase(
                colonPos, mPlaybackSessionID.size() - colonPos);
    }

    status_t err = configureTransport(msg);

    if (err != OK) {
        return err;
    }

    mState = PAUSED;

    return sendPlay(
            sessionID,
            !mSetupURI.empty()
                ? mSetupURI.c_str() : mWfd_presentation_url.c_str());
}

status_t WifiDisplaySink::configureTransport(const sp<ParsedMessage> &msg) {
    if (sUseTCPInterleaving) {
        return OK;
    }

    AString transport;
    if (!msg->findString("transport", &transport)) {
        ALOGE("Missing 'transport' field in SETUP response.");
        return ERROR_MALFORMED;
    }

    AString sourceHost;
    if (!ParsedMessage::GetAttribute(
                transport.c_str(), "source", &sourceHost)) {
        sourceHost = mRTSPHost;
    }

    AString serverPortStr;
    if (!ParsedMessage::GetAttribute(
                transport.c_str(), "server_port", &serverPortStr)) {
        ALOGE("Missing 'server_port' in Transport field.");
        return ERROR_MALFORMED;
    }

    int rtpPort, rtcpPort;
    if (sscanf(serverPortStr.c_str(), "%d-%d", &rtpPort, &rtcpPort) != 2
            || rtpPort <= 0 || rtpPort > 65535
            || rtcpPort <=0 || rtcpPort > 65535
            || rtcpPort != rtpPort + 1) {
        ALOGE("Invalid server_port description '%s'.",
                serverPortStr.c_str());

        return ERROR_MALFORMED;
    }

    if (rtpPort & 1) {
        ALOGW("Server picked an odd numbered RTP port.");
    }

	ALOGI("Start to connect RTPSink.");
    return mRTPSink->connect(sourceHost.c_str(), rtpPort, rtcpPort);
}

status_t WifiDisplaySink::onReceivePlayResponse(
        int32_t sessionID, const sp<ParsedMessage> &msg) {
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

    mState = PLAYING;
    ALOGV("[%s] Success  \n ",__func__);
    return OK;
}

void WifiDisplaySink::onReceiveClientData(const sp<AMessage> &msg) {
    int32_t sessionID;
    CHECK(msg->findInt32("sessionID", &sessionID));

    sp<RefBase> obj;
    CHECK(msg->findObject("data", &obj));

    sp<ParsedMessage> data =
        static_cast<ParsedMessage *>(obj.get());

    ALOGV("session %d received -----------------\n", sessionID);
    ALOGV("'%s'",   data->debugString().c_str());
    ALOGV("----------------------------------'");

    AString method;
    AString uri;
    data->getRequestField(0, &method);

    int32_t cseq;
    if (!data->findInt32("cseq", &cseq)) {
        sendErrorResponse(sessionID, "400 Bad Request", -1 /* cseq */);
        return;
    }

    if (method.startsWith("RTSP/")) {
        // This is a response.

        ResponseID id;
        id.mSessionID = sessionID;
        id.mCSeq = cseq;

        ssize_t index = mResponseHandlers.indexOfKey(id);

        if (index < 0) {
            ALOGW("Received unsolicited server response, cseq %d", cseq);
            return;
        }

        HandleRTSPResponseFunc func = mResponseHandlers.valueAt(index);
        mResponseHandlers.removeItemsAt(index);

        status_t err = (this->*func)(sessionID, data);
        CHECK_EQ(err, (status_t)OK);
    } else {
        AString version;
        data->getRequestField(2, &version);
        if (!(version == AString("RTSP/1.0"))) {
            sendErrorResponse(sessionID, "505 RTSP Version not supported", cseq);
            return;
        }

        if (method == "OPTIONS") {
            onOptionsRequest(sessionID, cseq, data);
        } else if (method == "GET_PARAMETER") {
            onGetParameterRequest(sessionID, cseq, data);
        } else if (method == "SET_PARAMETER") {
            onSetParameterRequest(sessionID, cseq, data);
        } else {
            sendErrorResponse(sessionID, "405 Method Not Allowed", cseq);
        }
    }
}

void WifiDisplaySink::onOptionsRequest(
        int32_t sessionID,
        int32_t cseq,
        const sp<ParsedMessage> &data) {
    AString response = "RTSP/1.0 200 OK\r\n";
    AppendCommonResponse(&response, cseq);



	char* s = 0;
	s = GetConfiguration("M1:response");
	if(s)
	{
		response.append(s);
		response.append("\r\n\r\n");
	}
	else
	{
	    response.append("Public: org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER\r\n");
	    response.append("\r\n");
	}


	ALOGV("-----------------SEND-------------------- \n");
	ALOGV("%s",response.c_str());
	ALOGV("----------------------------------------- ");

    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);

    err = sendM2(sessionID);
    CHECK_EQ(err, (status_t)OK);
}

void WifiDisplaySink::onGetParameterRequest(
        int32_t sessionID,
        int32_t cseq,
        const sp<ParsedMessage> &data) {

	AString body = "";
 	int iPos;
 	char seg[64];
 	char seg_search[64];
 	sprintf(seg, "wfd_audio_codecs");
 	sprintf(seg_search, "M3:%s",seg);
 	
	iPos = data->debugString().find(seg, 0);
	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("LPCM 00000002 00" );		//pcm 48k 2ch
			body.append(", ");
			body.append("AAC 00000001 00\r\n" );	//aac 48k 2ch
		}
 	}
	
	// wfd_video_formats:
    // 1 byte "native"
    // 1 byte "preferred-display-mode-supported" 0 or 1
    // one or more avc codec structures
    //   1 byte profile
    //   1 byte level
    //   4 byte CEA mask
    //   4 byte VESA mask
    //   4 byte HH mask
    //   1 byte latency
    //   2 byte min-slice-slice
    //   2 byte slice-enc-params
    //   1 byte framerate-control-support
    //   max-hres (none or 2 byte)
    //   max-vres (none or 2 byte)
 	sprintf(seg, "wfd_video_formats");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
 // For 1080p30+720p60+720p30:
		body.append("30 00 02 02 0001ffff 00000000 00000000 00 0000 0000 00 none none\r\n" );
 // For 720p60:
 //   use "30 00 02 02 00000040 00000000 00000000 00 0000 0000 00 none none\r\n"
 // For 720p30:
 //   use "28 00 02 02 00000020 00000000 00000000 00 0000 0000 00 none none\r\n"
 // For 720p24:
 //   use "78 00 02 02 00008000 00000000 00000000 00 0000 0000 00 none none\r\n"
 // For 1080p30:
 //   use "38 00 02 02 00000080 00000000 00000000 00 0000 0000 00 none none\r\n"
 

		}
	}

 	sprintf(seg, "wfd_client_rtp_ports");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("RTP/AVP/UDP;unicast 8888 0 mode=play\r\n" );
		}
	}

 	sprintf(seg, "wfd_uibc_capability");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("none\r\n" );
		}
	}

 	sprintf(seg, "wfd_coupled_sink");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("none\r\n" );
		}

	}

 	sprintf(seg, "wfd_standby_resume_capability");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("none\r\n" );
		}
	}

 	sprintf(seg, "wfd_3d_video_formats");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
 	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("none\r\n" );
		}
	}

 	sprintf(seg, "wfd_content_protection");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
 	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("none\r\n" );
 		}
	}

 	sprintf(seg, "wfd_display_edid");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
 	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );

		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("none\r\n" );
 		}
	}	
	
 	sprintf(seg, "wfd_connector_type");
 	sprintf(seg_search, "M3:%s",seg);
	iPos = data->debugString().find(seg, 0);
 	if(iPos>=0)
	{
		body.append(seg);			body.append(": " );
	
		char* s = 0;
		s = GetConfiguration(seg_search);
		if(s)
		{
			body.append(s);
			body.append("\r\n");
		}
		else
		{
			body.append("none\r\n" );
 		}
	}

    AString response = "RTSP/1.0 200 OK\r\n";
    AppendCommonResponse(&response, cseq);
    response.append("Content-Type: text/parameters\r\n");
    response.append(StringPrintf("Content-Length: %d\r\n", body.size()));
    response.append("\r\n");
    response.append(body);


    ALOGV("-----------------SEND-------------------- \n");
    ALOGV("%s",response.c_str());
    ALOGV("----------------------------------------- ");

    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);
}

status_t WifiDisplaySink::sendDescribe(int32_t sessionID, const char *uri) {
    uri = "rtsp://xwgntvx.is.livestream-api.com/livestreamiphone/wgntv";
    uri = "rtsp://v2.cache6.c.youtube.com/video.3gp?cid=e101d4bf280055f9&fmt=18";

    AString request = StringPrintf("DESCRIBE %s RTSP/1.0\r\n", uri);
    AppendCommonResponse(&request, mNextCSeq);

    request.append("Accept: application/sdp\r\n");
    request.append("\r\n");

 

    ALOGV("-----------------SEND-------------------- \n");
    ALOGV("%s",request.c_str());
    ALOGV("----------------------------------------- ");

    status_t err = mNetSession->sendRequest(
            sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID, mNextCSeq, &WifiDisplaySink::onReceiveDescribeResponse);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::sendSetup(int32_t sessionID, const char *uri) {
    mRTPSink = new RTPSink(mNetSession, mSurfaceTex, mPlayerNotify);
    looper()->registerHandler(mRTPSink);

    status_t err = mRTPSink->init(sUseTCPInterleaving);

    if (err != OK) {
        looper()->unregisterHandler(mRTPSink->id());
        mRTPSink.clear();
        return err;
    }

    AString request = StringPrintf("SETUP %s RTSP/1.0\r\n", uri);

    AppendCommonResponse(&request, mNextCSeq);

    if (sUseTCPInterleaving) {
        request.append("Transport: RTP/AVP/TCP;interleaved=0-1\r\n");
    } else {
        int32_t rtpPort = mRTPSink->getRTPPort();

        request.append(
                StringPrintf(
                    "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d\r\n",
                    rtpPort, rtpPort + 1));
    }

    request.append("\r\n");


    ALOGV("-----------------SEND-------------------- \n");
  	ALOGV("%s",request.c_str());
  	ALOGV("----------------------------------------- ");

    err = mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID, mNextCSeq, &WifiDisplaySink::onReceiveSetupResponse);

    ++mNextCSeq;

    return OK;
}

status_t WifiDisplaySink::sendPlay(int32_t sessionID, const char *uri) {
    AString request = StringPrintf("PLAY %s RTSP/1.0\r\n", uri);

    AppendCommonResponse(&request, mNextCSeq);

    request.append(StringPrintf("Session: %s\r\n", mPlaybackSessionID.c_str()));
    request.append("\r\n");

    ALOGV("-----------------SEND-------------------- \n");
    ALOGV("%s",request.c_str());
    ALOGV("----------------------------------------- ");

    status_t err =
        mNetSession->sendRequest(sessionID, request.c_str(), request.size());

    if (err != OK) {
        return err;
    }

    registerResponseHandler(
            sessionID, mNextCSeq, &WifiDisplaySink::onReceivePlayResponse);

    ++mNextCSeq;

    return OK;
}


status_t WifiDisplaySink::sendTearDown(int32_t sessionID) {
	AString body = "wfd_trigger_method: ";
	ALOGI("Sending TEARDOWN trigger.");
	body.append("TEARDOWN");

	body.append("\r\n");

	AString request = "SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0\r\n";
	AppendCommonResponse(&request, mNextCSeq);

	request.append("Content-Type: text/parameters\r\n");
	request.append(StringPrintf("Content-Length: %d\r\n", body.size()));
	request.append(body);

	ALOGV("-----------------SEND-------------------- \n");
	ALOGV("%s",request.c_str());
	ALOGV("----------------------------------------- ");

	status_t err = mNetSession->sendRequest(sessionID, request.c_str(), request.size());

	if (err != OK) {
		return err;
	}

	registerResponseHandler(
			sessionID, mNextCSeq, &WifiDisplaySink::onReceiveM8Response);

	++mNextCSeq;
	return OK;
}

status_t WifiDisplaySink::onReceiveM8Response(int32_t sessionID, const sp<ParsedMessage> &msg) {
	ALOGI("onReceiveM8Response");
    int32_t statusCode;
    if (!msg->getStatusCode(&statusCode)) {
        return ERROR_MALFORMED;
    }

    if (statusCode != 200) {
        return ERROR_UNSUPPORTED;
    }

    return OK;
}

void WifiDisplaySink::onSetParameterRequest(
        int32_t sessionID,
        int32_t cseq,
        const sp<ParsedMessage> &data) {
    const char *content = data->getContent();

    sp<Parameters> params =
        Parameters::Parse(data->getContent(), strlen(data->getContent()));

	//Find wfd_presentation_URL
	AString value;
	if (params  != NULL) {

	    if (params->findParameter("wfd_presentation_URL", &value))
		{
			mWfd_presentation_url = value;	
			ssize_t pos =0;
			pos = mWfd_presentation_url.find(" ", 0);
			if(pos >= 0)
			{
				mWfd_presentation_url.erase(pos,mWfd_presentation_url.size() - pos);
			}
		}
	    			
	}

    AString response = "RTSP/1.0 200 OK\r\n";
    AppendCommonResponse(&response, cseq);
    response.append("\r\n");


    ALOGV("-----------------SEND-------------------- \n");
  	ALOGV("%s",response.c_str());
  	ALOGV("----------------------------------------- ");

    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);

    if (strstr(content, "wfd_trigger_method: SETUP\r\n") != NULL) {
    	status_t err =
    			sendSetup(
    					sessionID,
    					mWfd_presentation_url.c_str());

    	CHECK_EQ(err, (status_t)OK);
    }
    else if (strstr(content, "wfd_trigger_method: TEARDOWN\r\n") != NULL) {
    	if (mPlayerNotify != NULL) {
    		mPlayerNotify->notify(MEDIA_PLAYBACK_COMPLETE, 1, 0);
    	}
    }

}

void WifiDisplaySink::sendErrorResponse(
        int32_t sessionID,
        const char *errorDetail,
        int32_t cseq) {
    AString response;
    response.append("RTSP/1.0 ");
    response.append(errorDetail);
    response.append("\r\n");

    AppendCommonResponse(&response, cseq);

    response.append("\r\n");


    ALOGV("-----------------SEND-------------------- \n");
    ALOGV("%s",response.c_str());
    ALOGV("----------------------------------------- ");

    status_t err = mNetSession->sendRequest(sessionID, response.c_str());
    CHECK_EQ(err, (status_t)OK);
}

// static
void WifiDisplaySink::AppendCommonResponse(AString *response, int32_t cseq) {
    time_t now = time(NULL);
    struct tm *now2 = gmtime(&now);
    char buf[128];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %z", now2);

    response->append("Date: ");
    response->append(buf);
    response->append("\r\n");

    response->append("User-Agent: stagefright/1.1 (Linux;Android 4.1)\r\n");

    if (cseq >= 0) {
        response->append(StringPrintf("CSeq: %d\r\n", cseq));
    }
}

char* WifiDisplaySink::GetConfiguration(const char* input)
{
	if(pini == 0)
	{
		return 0;
	}

	return  iniparser_getstring(pini, input,0);
}

}  // namespace android
