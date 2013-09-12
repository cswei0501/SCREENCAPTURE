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
#define LOG_TAG "wfd"
#include <utils/Log.h>

#include "sink/WifiDisplaySink.h"

#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <media/IMediaPlayerService.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/foundation/ADebug.h>

#include "wfd.h"

using namespace android;

class CWfdHelper
{
public:
	static CWfdHelper * GetInstance() {
		static CWfdHelper wfd;
		return &wfd;
	}

	int Start(const char * url, Surface * surface, WfdPlayerNotify * playerNotify) {
		ALOGI("%s", __func__);

		if (mSession != NULL)
		{
			ALOGW("WFD session already running.");
			return 1;
		}

		DataSource::RegisterDefaultSniffers();

		mSurfaceTex = surface->getSurfaceTexture();

		mSession = new ANetworkSession;
		if (OK != mSession->start())
		{
			Stop();
			return -1;
		}

		mLooper = new ALooper;

		mSink = new WifiDisplaySink(mSession, mSurfaceTex, playerNotify);
		mLooper->registerHandler(mSink);

		int32_t port = GetPort(url);
		char *colonPos = strrchr(url, ':');
		if (colonPos != NULL)
		{
			*colonPos = '\0';
		}
		ALOGI(">>> Connect to %s, port %d", url, port);
		mSink->start(url, port);

		if (OK != mLooper->start(false, false, PRIORITY_DEFAULT))
		{
			Stop();
			return -1;
		}
		return 0;
	}

	void Stop() {
		ALOGI("%s", __func__);

		if (mSink != NULL)
		{
			mSink->stop();
			mSink.clear();
		}

		if (mSession != NULL)
		{
			mSession->stop();
			mSession.clear();
		}
		ALOGI("%s - session cleared", __func__);

		mLooper.clear();

		mSurfaceTex.clear();

		ALOGI("%s Done.", __func__);
	}

private:
	CWfdHelper() {
	}

	~CWfdHelper() {
	}

	int GetPort(const char * url)
	{
		const char *colonPos = strrchr(url, ':');
		if (colonPos == NULL)
		{
			return kWifiDisplayDefaultPort;
		}
		else
		{
			char *end;
			int port = strtol(colonPos + 1, &end, 10);
			if (*end != '\0' || end == colonPos + 1
					|| port < 1 || port > 65535)
			{
				ALOGW("Illegal port specified, use default");
				return kWifiDisplayDefaultPort;
			}
			else
			{
				return port;
			}
		}
	}
	DISALLOW_EVIL_CONSTRUCTORS(CWfdHelper);

private:
	sp<ISurfaceTexture> mSurfaceTex;
	sp<ANetworkSession> mSession;
	sp<ALooper> mLooper;
	sp<WifiDisplaySink> mSink;

	static const int kWifiDisplayDefaultPort = 7236;
};

class PlayerNotifyImpl : public WfdPlayerNotify
{
public:
	PlayerNotifyImpl() : mCallback(NULL), mUserData(NULL) {}
	~PlayerNotifyImpl() {}

	void SetEventCallback(EventCallback_t callback, void * userData)
	{
		mCallback = callback;
		mUserData = userData;
	}

protected:
	virtual void notify(int msg, int ext1, int ext2)
	{
		if (mCallback != NULL)
		{
			mCallback(msg, ext1, ext2, mUserData);
		}
	}

private:
	EventCallback_t mCallback;
	void * mUserData;
};

int wfd_start(const char * url, void * surface, EventCallback_t callback, void * userData) {
	PlayerNotifyImpl * notify = new PlayerNotifyImpl;
	notify->SetEventCallback(callback, userData);
	return CWfdHelper::GetInstance()->Start(url, (Surface *)surface, notify);
}

void wfd_stop(void) {
	CWfdHelper::GetInstance()->Stop();
}
