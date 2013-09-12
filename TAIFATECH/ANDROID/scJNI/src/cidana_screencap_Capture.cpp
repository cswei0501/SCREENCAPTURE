/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "capture_control.h"
#include "cidana_screencap_Capture.h"

using namespace android;

/*****************************************************
* Func: CWiMoControl
* Function: construction function
* Parameters: void
*****************************************************/
CWiMoControl::CWiMoControl(JNIEnv * env, jobject thiz, jint videoMode, jint audioMode, char* libPath)
{
	pCAudio = NULL;
	pCPUControl = NULL;
	pCNetwork = NULL;
	pCVideo = NULL;

	pdl = NULL;
	Start_II = NULL;
	Stop_V = NULL;
	pVideoBuff = NULL;
	iVideoSize = 0;
	pAudioBuff = NULL;
	pVidSrcBuff1 = NULL;
	pVidSrcBuff2 = NULL;
	iVidSrcBuffSize1 = 0;
	iVidSrcBuffSize2 = 0;
	bSrcBuffFlag1 = BUFFERWRITE;
	bSrcBuffFlag2 = BUFFERWRITE;
	currUpdate = 0;
	prevUpdate = currUpdate - 1;
	bLock = false;
	bLimitedFPS = false;

	m_calledScreenshot = CALLEDSHOTOFF;
	iUIrotate = ROTATION_0;
	rotateFlag = iUIrotate - 1;
	checkCPUUtilization_hnd = 0;
	checkTimoutThread_hnd = 0;
	LoopVideoThread_hnd = 0;	
	bQuitJNIFlag = false;
	bConvertWH = false;
	bJNIInit = false;
	iDeviceModel = -1;
	m_env = env;
	m_thiz = thiz;
	iAudioCapMethod = METHOD_AUDIO_ERROR;
	iCaptureMethod = METHOD_VIDEO_ERROR;
	iTvRotate = CURRENT_TV_ROTATE_0;
	statusWiMo = WIMO_STATUS_FINISH;
	iCallingBuffIndex = CALLINGENTRANCEFIRSTLY;
	iCallingSrcIndex = CALLINGENTRANCEFIRSTLY;
	iVidDataType = -1;

	memset(pAppLibPath, 0, sizeof(pAppLibPath));
	memcpy(pAppLibPath, libPath, strlen(libPath));
	
	memset(&sTimeBomb, 0, sizeof(sTimeBomb));
	sTimeBomb.tm_year = TIME_BOMB_YEAR - INTERNATIONAL_STD_YEAR;
	sTimeBomb.tm_mon = TIME_BOMB_MONTH;
	sTimeBomb.tm_mday = TIME_BOMB_DAY;
	sTimeBomb.tm_hour = TIME_BOMB_HOUR;
	sTimeBomb.tm_min = TIME_BOMB_MINUTE;
	sTimeBomb.tm_sec = TIME_BOMB_SECOND;
	sTimeBomb.tm_isdst = 0;
	timeBomb = (DDword)mktime(&sTimeBomb);
	
	if(videoMode == JNI_TS_STREAM_INTERFACE) // for wimo2.0
	{
		iCaptureMethod = METHOD_TS_STREAM_INTERFACE;
		iAudioCapMethod = METHOD_TS_AUD_STREAM;
	}
	else
	{
		//check audio mode
	#ifdef AudioSWCapture
		iAudioCapMethod = METHOD_SW_ANDROID_AUD_SPEAKER;
	#else
		if(audioMode == JNI_AUD_DEFAULT)
			iAudioCapMethod = METHOD_AUDIO_DEFAULT;
		else if(audioMode == JNI_AUDIO_SPEAKER)
			iAudioCapMethod = METHOD_SW_ANDROID_AUD_SPEAKER;
 		else if(audioMode == JNI_AUDIO_METHOD_LNV)
			iAudioCapMethod = METHOD_LNV_AUD_MIXER;
 		else if(audioMode == JNI_AUDIO_METHOD_HS)
			iAudioCapMethod = METHOD_HS_AUD_MIXER;
		else if(audioMode == JNI_AUDIO_METHOD_ZTE)
			iAudioCapMethod = METHOD_ZTE_AUD_MIXER;
		else if(audioMode == JNI_AUDIO_METHOD_GAME)
			iAudioCapMethod = METHOD_GAME_AUDIO;
		else if(audioMode == JNI_AUDIO_METHOD_HAOLIAN)
			iAudioCapMethod = METHOD_SW_ANDROID_AUD_SPEAKER;
	#endif
		//check video mode
		if(videoMode == JNI_HW_VID_INTERFACE)
			iCaptureMethod = METHOD_HW_VIDEO_INTERFACE;
 		else if(videoMode == JNI_SW_VID_FRAMEBUFFER)
			iCaptureMethod = METHOD_FRAMEBUFFER_INTERFACE;
 		else if(videoMode == JNI_SW_VID_SCREENSHOT)
			iCaptureMethod = METHOD_SCREENSHOT_INTERFACE;
 		else if(videoMode == JNI_LNV_VIDEO_INTERFACE)
			iCaptureMethod = METHOD_LNV_HW_INTERFACE;
		else if(videoMode == JNI_HS_VIDEO_INTERFACE)
			iCaptureMethod = METHOD_HS_HW_INTERFACE;
		else if(videoMode == JNI_ZTE_VIDEO_INTERFACE)
			iCaptureMethod = METHOD_ZTE_HW_INTERFACE;
		else if(videoMode == JNI_GAME_VIDEO_INTERFACE)
			iCaptureMethod = METHOD_GAME_INTERFACE;
		else if(videoMode == JNI_HAOLIAN_VIDEO_INTERFACE)
		{
			iCaptureMethod = METHOD_FRAMEBUFFER_INTERFACE;
			bLimitedFPS = true;
		}
		else if(videoMode == JNI_XIAOMI2_VIDEO_INTERFACE)
		{
			iDeviceModel = DEVICE_XIAOMI2_41;
			iCaptureMethod = METHOD_FRAMEBUFFER_INTERFACE;
			if(audioMode == JNI_AUDIO_METHOD_HAOLIAN)
				bLimitedFPS = true;
		}
 	}	

	if(iCaptureMethod == METHOD_HW_VIDEO_INTERFACE || iCaptureMethod == METHOD_GAME_INTERFACE)
	{
		pVidSrcBuff1 = (Byte*)malloc(VIDEO_SRC_BUFFER_SIZE);
		if(pVidSrcBuff1 == NULL)
		{
			LOGE("malloc pVidSrcBuff1 failed!\n");
		}
		pVidSrcBuff2 = (Byte*)malloc(VIDEO_SRC_BUFFER_SIZE);
		if(pVidSrcBuff2 == NULL)
		{
			LOGE("malloc pVidSrcBuff2 failed!\n");
		}
	}
	
	if(iCaptureMethod == METHOD_HS_HW_INTERFACE && iAudioCapMethod == METHOD_HS_AUD_MIXER)
	{
		pdl = dlopen("/system/lib/libavcap.so", RTLD_NOW);
		if(pdl != NULL)	
		{
			Start_II = (pF_INT_INT)dlsym(pdl, "start");
			Stop_V = (pF_VOID)dlsym(pdl, "stop");
		}
		else
		{
			LOGE("dlopen HS libs failed!\n");
			iAudioCapMethod = METHOD_AUDIO_ERROR;
			iCaptureMethod = METHOD_VIDEO_ERROR;
		}
	}
	
	LOGI("iCaptureMethod:%d, iAudioCapMethod:%d\n", iCaptureMethod,iAudioCapMethod);
	
	JniInit(m_env, m_thiz);
}

/*****************************************************
* Func: ~CWiMoControl
* Function: deconstruction function
* Parameters: void
*****************************************************/
CWiMoControl::~CWiMoControl()
{
	JniUnInit(m_env, m_thiz);
	if(pdl)
		dlclose(pdl);
	if(pVidSrcBuff1)
	{
		free(pVidSrcBuff1);
		pVidSrcBuff1 = NULL;
	}
	if(pVidSrcBuff2)
	{
		free(pVidSrcBuff2);
		pVidSrcBuff2 = NULL;
	}
}

/*****************************************************
* Func: JniInit
* Function:  jni init parameters
* Parameters:
*	env:	environment variable
*	thiz: object
*****************************************************/
int CWiMoControl::JniInit(JNIEnv * env, jobject thiz)
{
	if(bJNIInit) return S_OK;

	if(!env || !thiz)
	{
		LOGE("FAILED to get env && thiz\n");
		return E_INVALIDARG;
	}
	//get java method
	env->GetJavaVM(&jCallbackFun.jvm); 
	if(!jCallbackFun.jvm)
	{
		LOGE("FAILED to get JVM \n");
		return E_FAILED;
	}
	jCallbackFun.env = env;
	jCallbackFun.obj = env->NewGlobalRef(thiz);
	jCallbackFun.cls = env->GetObjectClass(thiz);
	jCallbackFun.cls = (jclass)env->NewGlobalRef(jCallbackFun.cls);
	jCallbackFun.callbackFuns = env->GetMethodID(jCallbackFun.cls, "javaCallback", "(I)I");

	bJNIInit = true;
	return S_OK;
}

/*****************************************************
* Func: JniUnInit
* Function: Release Video SC
* Parameters:
*	env:	environment variable
*	thiz: object
*****************************************************/
int CWiMoControl::JniUnInit(JNIEnv * env, jobject thiz)
{
	if(!bJNIInit) return S_OK;

	jCallbackFun.env = env;
	jCallbackFun.env->DeleteGlobalRef(jCallbackFun.obj);
	jCallbackFun.env->DeleteGlobalRef(jCallbackFun.cls);
	memset(&jCallbackFun,0,sizeof(jCallbackFun));	
	
	bJNIInit = false;
	return S_OK;
}

/*****************************************************
* Function: StartWiMo
* Parameters: void
*****************************************************/
int CWiMoControl::StartWiMo()
{
#if 0
	if(iCaptureMethod != METHOD_LNV_HW_INTERFACE && iCaptureMethod != METHOD_HS_HW_INTERFACE)
	{
		if(timeBomb < GetTickCount()/NumOneThousand)
		{
			LOGE("time bomb, please update version to newest!\n");
			goto INIT_FAILED_JUMP;
		}
	}
#endif
	if(statusWiMo == WIMO_STATUS_INITING 
		|| statusWiMo == WIMO_STATUS_CONNECTING) 
		return S_OK;
	
	#if OUTLOG
		LOGI("connecting, please wait for a moment...\n");
	#endif
	statusWiMo = WIMO_STATUS_INITING;
	if(Init() != S_OK)
	{
		LOGE("failed to Init()!\n");
		goto INIT_FAILED_JUMP;
	}

	checkTimoutThread_hnd =  CreateThreadHelper(CheckConnectTimeoutThread, (void*)this);
	if(checkTimoutThread_hnd == 0)
	{
		LOGE("creat thread CheckConnectTimeoutThread failed!\n");
		goto INIT_FAILED_JUMP;
	}

	return S_OK;
INIT_FAILED_JUMP:
	statusWiMo = WIMO_STATUS_FINISH;
	NotifyUIStop();
	UnInit();
	
	return E_FAILED;
}

/*****************************************************
* Function: StopWiMo
* Parameters: void
*****************************************************/
int CWiMoControl::StopWiMo()
{
	if(statusWiMo == WIMO_STATUS_FINISH)
		return S_OK;
	
	statusWiMo = WIMO_STATUS_FINISH;
	if(iCaptureMethod != METHOD_TS_STREAM_INTERFACE)
	{
		if(checkCPUUtilization_hnd)
		{
			pthread_join(checkCPUUtilization_hnd, NULL);
			checkCPUUtilization_hnd = 0;
		}
		if(LoopVideoThread_hnd)
		{
			pthread_join(LoopVideoThread_hnd, NULL);
			LoopVideoThread_hnd = 0;
		}
	}
	if(checkTimoutThread_hnd)
	{
		pthread_join(checkTimoutThread_hnd, NULL);
		checkTimoutThread_hnd = 0;
	}
	UnInit();

	#if OUTLOG
		LOGI("Disconnection!\n");
	#endif
	return S_OK;
}

/*****************************************************
* Function: SendVideoData
* Parameters: 
*		data: jpeg data
*		vSizeHW: the size of jpeg data
*		width: jpeg width
*		height: jpeg height
*****************************************************/
int CWiMoControl::SendVideoData(int dataType, char* data, int vSize, int width, int height, int rotate)
{
	//LOGI("video dataType:%d,data:%d, vSize:%d,width:%d,height:%d,rotate:%d\n",dataType,*data, vSize,width,height,rotate);
	if(vSize == 0)
	{
		if(width !=0 && height !=0)
		{
			ALIGNED_WIDTH = SRC_WIDTH = width;// &~ 7;
			ALIGNED_HEIGTH = SRC_HEIGHT = height;// &~ 7;
			iVidDataType = dataType;
			if(JNI_VIDEO_TYPE_ARGB8888 == dataType)
				iVidDataType = VIDEO_TYPE_ARGB8888;
			else if(JNI_VIDEO_TYPE_JPEG == dataType)
				iVidDataType = VIDEO_TYPE_JPEG;
		}
		else
		{
			LOGE("data is null\n");
			return E_FAILED;	
		}
	}
	bLock = true;
//	iUIrotate  = rotate;
	if(JNI_VIDEO_TYPE_ARGB8888 == dataType)
		iVidDataType = VIDEO_TYPE_ARGB8888;
	else if(JNI_VIDEO_TYPE_JPEG == dataType)
		iVidDataType = VIDEO_TYPE_JPEG;
	
	if(VIDEO_TYPE_JPEG == iVidDataType &&
		(iCallingBuffIndex == CALLINGBUFFER2 || iCallingBuffIndex == CALLINGENTRANCEFIRSTLY))
	{
		pVideoBuff = data;
		iVideoSize = vSize;
		iCallingBuffIndex = CALLINGBUFFER1;
	}
	else if(VIDEO_TYPE_ARGB8888 == iVidDataType)
	{
		if(iCallingSrcIndex == CALLINGENTRANCEFIRSTLY
			||(bSrcBuffFlag1 == BUFFERWRITE && iCallingSrcIndex == CALLINGBUFFER1))
		{
			memcpy(pVidSrcBuff1, (Byte*)data, vSize);
			iVidSrcBuffSize1 = vSize;
		}
		else if(bSrcBuffFlag2 == BUFFERWRITE && iCallingSrcIndex == CALLINGBUFFER2)
		{
			memcpy(pVidSrcBuff2, (Byte*)data, vSize);
			iVidSrcBuffSize2 = vSize;
		}
	}
	bLock = false;
	
	return S_OK;
}

/*****************************************************
* Function: SendAudioData
* Parameters: 
*		data: audio data of pcm
*		size: the size of audio data
*****************************************************/
int CWiMoControl::SendAudioData(int dataType, char* data, int size, int sampleRate, int bitRate, int channels)
{
	//LOGI("audio dataType:%d,data:%d, size:%d,sampleRate:%d,bitRate:%d,channels:%d\n",dataType,*data, size,sampleRate,bitRate,channels);
	if(data == NULL)
	{
		LOGE("data is null\n");
		return E_FAILED;	
	}

	if(pCAudio && JNI_AUDIO_TYPE_PCM == dataType)
	{
		pAudioBuff = (LIVEDATABUFFER*)pCAudio->GetSendAudioBuffer();
		pAudioBuff->SendData(size, (unsigned char*)data);
	}
	return S_OK;
}

/*****************************************************
* Function: setSelectDevice
* Parameters: 
*		nTVRotate: set tv rotation
*****************************************************/
int CWiMoControl::SetTVRotate(int nTVRotate)
{
	if(iTvRotate == nTVRotate)
		return S_OK;

	if(nTVRotate == JNI_CURRENT_TV_ROTATE_90R)
		iTvRotate = CURRENT_TV_ROTATE_90R;
	else if(nTVRotate == JNI_CURRENT_TV_ROTATE_90L)
		iTvRotate = CURRENT_TV_ROTATE_90L;
	else //default tv is not rotate
		iTvRotate = CURRENT_TV_ROTATE_0;
			
	LOGI("SetTVRotate: %d\n", iTvRotate);
	return S_OK;
}

/*****************************************************
* Function: SetVideoRotate
* Parameters: 
*		nRotate: 0: no rotate;
*				1: rotate left 90
*				2: rotate 180
*				3: rotate right 90
*****************************************************/
int CWiMoControl::SetVideoRotate(int nRotate)
{
	if(iUIrotate == nRotate)
		return S_OK;
	
	iUIrotate = nRotate;
//	LOGI("rotate: %d\n", nRotate);
	return S_OK;
}

/*****************************************************
* Function: CheckFB0
* Parameters: void
*****************************************************/
int CWiMoControl::CheckFB0()
{
	if(iCaptureMethod == METHOD_TS_STREAM_INTERFACE)
		return S_OK;
	else if(iCaptureMethod == METHOD_FRAMEBUFFER_INTERFACE
		|| iCaptureMethod == METHOD_SCREENSHOT_INTERFACE)
	{
		FILE* fb = fopen(FRAMEBUFERFB, "r");
		if(fb == NULL)
			return E_FAILED;
		fclose(fb);
	}

	return S_OK;
}

/*****************************************************
* Function: NotifyScreenshotCalled
* Parameters: called, 0: don't call screen shot
*				1: call screen shot method
*****************************************************/
int CWiMoControl::NotifyScreenshotCalled(int called)
{
	m_calledScreenshot = called;
	return 0;
}

/*****************************************************
* Func: SetDeviceWidthHeight
* Function: set device width and height
* Parameters: width: device widht
*			height: device height
*****************************************************/
void CWiMoControl::SetDeviceWidthHeight(int width, int height)
{
	SRC_WIDTH = width;
	SRC_HEIGHT = height;
}

/*****************************************************
* Func: SetAlignedWidthHeight
* Function: set widht and height aligned to 16
* Parameters: width: aligned widht
*			height: aligned height
*****************************************************/
void CWiMoControl::SetAlignedWidthHeight(int width, int height)
{
	ALIGNED_WIDTH = width;
	ALIGNED_HEIGTH = height;
}

/*****************************************************
* Func: GetAudioMethod
* Function: get audio capture method
* Parameters: void
*****************************************************/
int CWiMoControl::GetAudioMethod()
{
	return iAudioCapMethod;
}

/*****************************************************
* Func: GetVideoMethod
* Function: get video capture method
* Parameters: void
*****************************************************/
int CWiMoControl::GetVideoMethod()
{
	return iCaptureMethod;
}

/*****************************************************
* Func: GetVideoMethod
* Function: get video capture method
* Parameters: void
*****************************************************/
void CWiMoControl::SetVideoMethod(int method)
{
	iCaptureMethod = method;
}

/*****************************************************
* Func: GetVideoType
* Function: get video data type
* Parameters: void
*****************************************************/
int CWiMoControl::GetVideoType()
{
	return iVidDataType;
}

/*****************************************************
* Func: GetVideoDataBuff
* Function: get video data buffer
* Parameters: size: valide data size
* return buffer address
*****************************************************/
char* CWiMoControl::GetVideoDataBuff(int *size)
{
	char* buff = NULL;
	
	if(VIDEO_TYPE_JPEG == iVidDataType)
	{
		if(iCallingBuffIndex == CALLINGBUFFER1)
		{
			*size = iVideoSize;
			iVideoSize = 0;
			buff = pVideoBuff;
		}
	}
	else if(VIDEO_TYPE_ARGB8888 == iVidDataType && bLock == false)
	{
		prevUpdate = currUpdate;

		if((iCallingSrcIndex == CALLINGBUFFER1 || iCallingSrcIndex == CALLINGENTRANCEFIRSTLY)
			&& iVidSrcBuffSize1 != 0)
		{
			iCallingSrcIndex = CALLINGBUFFER2;
			bSrcBuffFlag1 = BUFFERREAD;
			bSrcBuffFlag2 = BUFFERWRITE;
			buff = (char*)pVidSrcBuff1;
			*size = iVidSrcBuffSize1;
			iVidSrcBuffSize1 = 0;
			currUpdate = 1;
		}
		else if(iCallingSrcIndex == CALLINGBUFFER2 && iVidSrcBuffSize2 != 0)
		{
			iCallingSrcIndex = CALLINGBUFFER1;
			bSrcBuffFlag2 = BUFFERREAD;
			bSrcBuffFlag1 = BUFFERWRITE;
			buff = (char*)pVidSrcBuff2;
			*size = iVidSrcBuffSize2;
			iVidSrcBuffSize2 = 0;
			currUpdate = 2;
		}

		if(prevUpdate == currUpdate)
			return NULL;
	}

	return buff;
}

/*****************************************************
* Func: GetLimitedFPS
* Function: return limited video FPS whether or not
* Parameters: void
*****************************************************/
bool CWiMoControl::GetLimitedFPS(void)
{
	return bLimitedFPS;
}

/*****************************************************
* Func: GetDeviceModel
* Function: return device mode
* Parameters: void
*****************************************************/
int CWiMoControl::GetDeviceModel(void)
{
	return iDeviceModel;
}

/*****************************************************
* Func: Init
* Function: init serveral global parameters
* Parameters: void
*****************************************************/
int CWiMoControl::Init()
{
	if(iCaptureMethod != METHOD_GAME_INTERFACE)
	{
		SRC_WIDTH = 0;
		SRC_HEIGHT = 0;
		ALIGNED_WIDTH = 0;
		ALIGNED_HEIGTH = 0;
	}
	iCallingBuffIndex = CALLINGENTRANCEFIRSTLY;
	iCallingSrcIndex = CALLINGENTRANCEFIRSTLY;
	currUpdate = 0;
	prevUpdate = currUpdate - 1;
	
	if(iCaptureMethod == METHOD_VIDEO_ERROR)
		return E_FAILED;

	if(iCaptureMethod == METHOD_HS_HW_INTERFACE)
	{
		iHSHandle = -1;
		if(Start_II && (iHSHandle = (*Start_II)(HS_ROTATE_0, HS_QFACOTR_10)) != 0)
			return E_FAILED;
	}
	
	if((iAudioCapMethod == METHOD_AUDIO_DEFAULT 
		|| iAudioCapMethod == METHOD_SW_ANDROID_AUD_SPEAKER
		|| iAudioCapMethod == METHOD_HS_AUD_MIXER
		|| iAudioCapMethod == METHOD_LNV_AUD_MIXER
		|| iAudioCapMethod == METHOD_ZTE_AUD_MIXER) && iCaptureMethod != METHOD_TS_STREAM_INTERFACE)
		pCAudio = new CWiMoAudio(this);

	if(iCaptureMethod != METHOD_TS_STREAM_INTERFACE && iCaptureMethod != METHOD_HW_VIDEO_INTERFACE)
	{
		if(iCaptureMethod == METHOD_FRAMEBUFFER_INTERFACE)
		{
			pCPUControl = new CCPUControl(this);
		}
		pCVideo = new CWiMoVideo(this, pAppLibPath);
	}
	pCNetwork = new CWiMoNetwork(this);

	return S_OK;
}

/*****************************************************
* Func: AndroidUnInit
* Function: UnInit all global parameters
* Parameters: void
*****************************************************/
void CWiMoControl::UnInit()
{
	if(pCNetwork)
	{
		delete pCNetwork;
		pCNetwork = NULL;
	}
	if(iCaptureMethod != METHOD_TS_STREAM_INTERFACE && iCaptureMethod != METHOD_HW_VIDEO_INTERFACE)
	{
		if(pCVideo)
		{
			delete pCVideo;
			pCVideo = NULL;
		}
		if(pCPUControl && (iCaptureMethod != METHOD_HS_HW_INTERFACE && iCaptureMethod != METHOD_LNV_HW_INTERFACE))
		{
			delete pCPUControl;
			pCPUControl = NULL;
		}	
	}
	if((iAudioCapMethod == METHOD_AUDIO_DEFAULT 
		|| iAudioCapMethod == METHOD_SW_ANDROID_AUD_SPEAKER
		|| iAudioCapMethod == METHOD_HS_AUD_MIXER
		|| iAudioCapMethod == METHOD_LNV_AUD_MIXER) && iCaptureMethod != METHOD_TS_STREAM_INTERFACE)
	{
		if(pCAudio)
		{
			delete pCAudio;
			pCAudio = NULL;
		}
	}
	if(iCaptureMethod == METHOD_HS_HW_INTERFACE && iHSHandle == 0 && Stop_V)
	{
		iHSHandle = -1;
		(*Stop_V)();
	}
}

/*****************************************************
* Func: NotifyUIStop
* Function: notify UI to stop it
* Parameters: void
*****************************************************/
void CWiMoControl::NotifyUIStop()
{
	if(pCNetwork !=NULL && pCNetwork->GetDisconnectReason() == DISCONNECT_KICKED)
	{
		LOGI("quit sdk, be kicked\n");
		JniStatusChangedCallBack(CONNECTIONWIMOKICKED);
	}
	else if(pCNetwork !=NULL && pCNetwork->GetDisconnectReason() == DISCONNECT_TIMEOUT)
	{
		LOGI("quit sdk, the network is timeout\n");
		JniStatusChangedCallBack(CONNECTIONWIMOTIMEOUT);		
	}
	else
		JniStatusChangedCallBack(CONNECTIONWIMOFAIL);
	
	if(iCaptureMethod == METHOD_HS_HW_INTERFACE && iHSHandle == 0 && Stop_V)
	{
		iHSHandle = -1;
		(*Stop_V)();
	}
}

/*****************************************************
* Func: MainThread
* Function: start WiMo NONBLOCK loop
* Parameters: param: thread input parameter
*****************************************************/
void* CWiMoControl::CheckConnectTimeoutThread(void* param)
{
	CWiMoControl* pthis = (CWiMoControl*)param;
	DDword startTime = GetTickCount();

	while(pthis->statusWiMo == WIMO_STATUS_INITING)
	{
		if(GetTickCount() - startTime < NumTimeOut)
		{
			usleep(LOOPWAITTIMES);
			continue;
		}
		else
		{
			pthis->pCNetwork->SetDisconnectReason(DISCONNECT_TIMEOUT);
			break;
		}
	}
	//timeout or status changed. 

	 if(pthis->statusWiMo == WIMO_STATUS_CONNECTING)
	 {
		pthis->JniStatusChangedCallBack(CONNECTIONWIMOSUCCESS);
		if(pthis->iCaptureMethod != METHOD_TS_STREAM_INTERFACE && pthis->iCaptureMethod != METHOD_HW_VIDEO_INTERFACE)
		{
			if(pthis->pCVideo != NULL 
				&& ((pthis->LoopVideoThread_hnd = CreateThreadHelper(pthis->pCVideo->LoopThread, (void*)pthis->pCVideo)) == 0)
			)
			{
				LOGE("creat thread LoopVideoThread_hnd failed!\n");
				return (void*)-1;
			}

			if(pthis->iCaptureMethod == METHOD_FRAMEBUFFER_INTERFACE && pthis->pCPUControl != NULL)
			{
				pthis->checkCPUUtilization_hnd = CreateThreadHelper(pthis->pCPUControl->LoopCheckCPUUtilization,(void*)pthis->pCPUControl);
				if(pthis->checkCPUUtilization_hnd == 0)
				{
					LOGE("pCPUControl->CreateThreadHelper() failed!\n");
					return (void*)-1;
				}
			}
		}
	 }
	 else
	 {
 		pthis->statusWiMo = WIMO_STATUS_FINISH;
	 	//pthis->NotifyUIStop();
		pthis->UnInit();
	 }

	return 0;
}


/*****************************************************
* Func: JniStatusChangedCallBack
* Function: Call back flag of the success or failure
* Parameters: 
*		connectionFlag: 0: failure
*					   1: success
*****************************************************/
void CWiMoControl::JniStatusChangedCallBack(int connectionFlag)
{
	bool isAttached = false;
	int status = 0;

	status = jCallbackFun.jvm->GetEnv((void **) &jCallbackFun.env, JNI_VERSION_1_4);
	if(status< 0)
	{
		//LOGE("callback_handler: failed to get JNI environment, " "assuming native thread");
		status = jCallbackFun.jvm->AttachCurrentThread(&jCallbackFun.env, NULL);
		if(status< 0)
		{
			LOGE("callback_handler: failed to attach "  "current thread");
			return;
		}
		isAttached = true;
	}
	if(connectionFlag == CONNECTIONWIMOSUCCESS)
	{
		LOGI("Connect to WiMo SUCCESSFULLY!\n");
		if(iCaptureMethod == METHOD_TS_STREAM_INTERFACE)
		{
			jclass objClass = jCallbackFun.env->GetObjectClass(jCallbackFun.obj); 
			//get ipAddress(long)
			jfieldID ipAddress = jCallbackFun.env->GetFieldID(objClass, "ipAddr", "J"); 
			long longFieldVal = jCallbackFun.env->GetLongField(jCallbackFun.obj, ipAddress); 
			
			//LOGI("ipAddress field value:%ld, valid ip:%ld",longFieldVal,pCNetwork->GetServerIpAddr());
			jCallbackFun.env->SetLongField(jCallbackFun.obj,ipAddress, pCNetwork->GetServerIpAddr());  

			//get udpPort(int)
			jfieldID port = jCallbackFun.env->GetFieldID(objClass, "udpPort", "I"); 
			int  intFieldVal = jCallbackFun.env->GetIntField(jCallbackFun.obj, port);
			//LOGI("port field value:%d",intFieldVal);
			jCallbackFun.env->SetIntField(jCallbackFun.obj,port, TSUDPPORT);  
		}
	}
	else
	{
		if(statusWiMo == WIMO_STATUS_FINISH)
		{
			bQuitJNIFlag = true;
			LOGI("Connect WiMo FINSHED\n");
		}
	}
#if OUTLOG
	LOGE("env: %p, obj: %p, callback: %p\n", jCallbackFun.env, jCallbackFun.obj, jCallbackFun.callbackFuns);
#endif
	jCallbackFun.env->CallIntMethod(jCallbackFun.obj, jCallbackFun.callbackFuns,connectionFlag);
	if (isAttached)
		jCallbackFun.jvm->DetachCurrentThread();
}

// java中的jstring, 转化为c的一个字符数组 
int Jstring2CStr(JNIEnv* env, jstring jstr, char* buff)
{
	char* rtn = buff;
	if(rtn == NULL)
		return -1;
	jclass clsstring = env->FindClass("java/lang/String");  
	jstring strencode = env->NewStringUTF("GB2312");  
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");  
	jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr,mid,strencode); // String .getByte("GB2312");  
	jsize alen = env->GetArrayLength(barr);  
	jbyte* ba = env->GetByteArrayElements(barr,JNI_FALSE);  
	//LOGI("alen: %d\n", alen);
	if(alen > 0)  
	{  
		memcpy(rtn,ba,alen);  
		rtn[alen]=0;  
	}  
	env->ReleaseByteArrayElements(barr,ba,0);  //释放内存  

	return S_OK; 
}

int jbyteArrayToChar(JNIEnv *env, jbyteArray bytes, jint size, char* buff)
{
	char *rtn = buff;
	if(rtn == NULL)
		return -1;
	jsize len = (jsize)size;  

	jbyte *arrayBody = env->GetByteArrayElements(bytes,0);  
	if(len > 0)
	{  
		memcpy(rtn, arrayBody, len);  
		rtn[len]=0;  
	}  
	env->ReleaseByteArrayElements(bytes, arrayBody, 0);  
	
	return 0;  
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    jniInit
 * Signature: (II)I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_jniInit
  (JNIEnv *env, jobject thiz, jint videoMode, jint audioMode)
{
	LOGI("%s \n",__FUNCTION__);
	jclass objClass = env->GetObjectClass(thiz); 
	//取String libPath
	jfieldID path = env->GetFieldID(objClass, "libPath", "Ljava/lang/String;"); 

	//判断是否为NULL
       if(!path) {
           LOGE("libPath: failed to get field ID");
           return NULL;
       }
	//获取此参数的值,基本类型的，赋值的话直接等于即可
	jstring str = (jstring) env->GetObjectField(thiz, path);

	char* libPath = new char[STRINGDATESIZE];
	Jstring2CStr(env, str, libPath);
	//LOGI("libPath: %s\n", libPath);
	
	CWiMoControl * pcWiMo = new CWiMoControl(env,thiz,videoMode, audioMode, libPath);
	delete libPath;
	
	return (int)pcWiMo;
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    jniUnInit
 * Signature: ()I
 *****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_jniUnInit
(JNIEnv * env, jobject thiz)
{
	LOGI("%s \n",__FUNCTION__);
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;

	delete cWiMo;
	cWiMo = NULL;
	return S_OK;
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    startWiMo
 * Signature: ()I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_startWiMo
  (JNIEnv *env, jobject thiz)
{
	LOGI("%s \n",__FUNCTION__);
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;
	return cWiMo->StartWiMo();
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    stopWiMo
 * Signature: ()I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_stopWiMo
  (JNIEnv *env, jobject thiz)
{
	LOGI("%s \n",__FUNCTION__);
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer 
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;
	return cWiMo->StopWiMo();
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    sendVideoData
 * Signature: (I[BIIII)I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_sendVideoData
(JNIEnv * env, jobject thiz, jint dataType, jbyteArray data, jint size, jint width, jint height, jint rotate)
{
	//LOGI("%s \n",__FUNCTION__);
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer 
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;

	char *dataBuf = (char*)malloc(size+1);
	if(jbyteArrayToChar(env, data, size, dataBuf) != 0)
	{
		LOGE("sendVideoData: malloc buffer failed!\n");
		return E_FAILED;
	}

/*
	//byte[] abyte 
	jfieldID arrFieldId = env->GetFieldID(objClass, "abyte", "[B"); 
	if (arrFieldId == 0)
	{
	LOGI("Faild to get the abyte field");
	return 0;
	}
	jbyteArray jarr = (jbyteArray)env->GetObjectField(test_obj, arrFieldId); 
	jbyte *arr = env->GetByteArrayElements(jarr, 0);
	LOGI("Reach here arr=%x",arr);

	int len = (env)->GetArrayLength(jarr);
	LOGI("Reach here len=%d",len );

	for(int i=0;i<len;i++)
	{ 
	LOGI("TestClass's abyte field[%d] value:%d",i,arr[i]);
	arr[i] += 1;
	}
	env->SetByteArrayRegion(jarr,0, len,arr);
	(env)->SetObjectField(test_obj,arrFieldId,jarr);  

	arr = env->GetByteArrayElements(jarr, 0); 
	for(int i=0;i<len;i++)
	{ 
	LOGI("TestClass's abyte field[%d] value:%d",i,arr[i]);
	}

	(env)->ReleaseByteArrayElements(jarr, arr, 0 );
*/	
	cWiMo->SendVideoData(dataType, dataBuf, size, width, height, rotate);
	if(dataBuf)
		free(dataBuf);
	return S_OK;
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    sendAudioData
 * Signature: (I[BIIII)I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_sendAudioData
(JNIEnv * env, jobject thiz, jint dataType, jbyteArray data, jint size, jint sampleRate, jint bitRate, jint channels)
{
	//LOGI("%s \n",__FUNCTION__);
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;

	char *dataBuf = (char*)malloc(size+1);
	
	if(jbyteArrayToChar(env, data, size, dataBuf) != 0)
	{
		LOGE("sendAudioData: malloc buffer failed!\n");
		return E_FAILED;
	}

	cWiMo->SendAudioData(dataType, dataBuf, size, sampleRate, bitRate, channels);
	if(dataBuf)
		free(dataBuf);
	return S_OK;
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    setTVRotate
 * Signature: (I)I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_setTVRotate
(JNIEnv * env, jobject thiz, jint nTVRotate)
{
	LOGI("%s \n",__FUNCTION__);
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;
	return cWiMo->SetTVRotate(nTVRotate);
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    setVideoRotate
 * Signature: (I)I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_setVideoRotate
(JNIEnv * env, jobject thiz, jint rotate)
{
	//LOGI("%s , rotate: %d\n",__FUNCTION__, rotate);
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;
	return cWiMo->SetVideoRotate(rotate);
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    checkFb0
 * Signature: ()I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_checkFb0
(JNIEnv * env, jobject thiz)
{
	LOGI("%s \n",__FUNCTION__);
 
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;
	return cWiMo->CheckFB0();
}

/*****************************************************
 * Class:     com_cmcc_wimo_WiMoCapture
 * Method:    notifyScreenshotCalled
 * Signature: ()I
*****************************************************/
JNIEXPORT jint JNICALL Java_com_cmcc_wimo_WiMoCapture_notifyScreenshotCalled
(JNIEnv * env, jobject thiz, jint shotCalled)
{
	//LOGI("%s \n",__FUNCTION__);
 
	jclass objClass = env->GetObjectClass(thiz); 
	//取int cValPointer
	jfieldID j = env->GetFieldID(objClass, "cValPointer", "I"); 
	if (j == 0) 
	{
		LOGI("Faild to get the j field");
		return E_FAILED; 
	}
	int intFieldVal = env->GetIntField(thiz, j); 
	CWiMoControl* cWiMo = (CWiMoControl*)intFieldVal;
	if(!cWiMo)
		return E_FAILED;
	return cWiMo->NotifyScreenshotCalled(shotCalled);
}
