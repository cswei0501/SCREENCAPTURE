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

#include "capture_audio.h"

using namespace android;

#if defined(NDK_AUDIO_RECORD)
static SLAndroidSimpleBufferQueueItf recorderBufferQueue;
static short recorderBuffer[RECORDER_FRAMES];
static unsigned int recorderSize;
static SLmilliHertz recorderSR;

static LIVEDATABUFFER m_AudioBuff;
#endif

/*****************************************************
* Func: CWiMoAudio
* Function: construction function
* Parameters: void
*****************************************************/
CWiMoAudio::CWiMoAudio(CWiMoControl* cWiMo)
{
	m_cWiMo = cWiMo;
	bAudioInit = false;
	g_sendAudio = NULL;
	g_AudioBuff = NULL;
	Init();
}

/*****************************************************
* Func: ~CWiMoAudio
* Function: deconstruction function
* Parameters: void
*****************************************************/
CWiMoAudio::~CWiMoAudio()
{
	UnInit();
}

/*****************************************************
* Func: Init
* Function: init audio
* Parameters: void
*****************************************************/
SCRESULT CWiMoAudio::Init()
{
	if(bAudioInit) return S_OK;

	tAudioCapture_hnd = 0;
	tAudioRenderThread_hnd = 0;
	socketFD = 0;
	audioDataSize = 0;
	audioDataBuff = NULL;
	pdl = NULL;
	SetAudioOutputEnable_I = NULL;
	readPcmData_R = NULL;

	if(ConnectServer() != S_OK)
	{
		LOGE("failed to ConnectServer()\n");
		return E_CONNECTFAILED;
	}

	g_AudioBuff = new LIVEDATABUFFER(m_cWiMo);
	
	if((tAudioRenderThread_hnd = CreateThreadHelper(AudioRenderThread, (void*)this)) == E_THREAD_CREATE)
	{
		LOGE("failed to tAudioRenderThread_hnd\n");
		return E_THREAD_CREATE;
	}

	pdl = m_cWiMo->GetLibHandle();
	if(m_cWiMo->GetAudioMethod() == METHOD_LNV_AUD_MIXER)
	{
		LOGI("LNV audio capture!\n");
	}
	else if(m_cWiMo->GetAudioMethod() == METHOD_HS_AUD_MIXER && pdl != NULL)
	{
		LOGI("HS audio capture!\n");
		SetAudioOutputEnable_I = (pF_INT)dlsym(pdl, "setAudioOutputEnable");
		readPcmData_R = (pF_pUCHAR_pINT_pINT64)dlsym(pdl, "readPcmData");
		audioDataBuff = (unsigned char*)malloc(4096);
		if(!audioDataBuff)
		{
			LOGE("HS failed malloc audio audioDataBuff\n");
			return E_MALLOCFAILED;
		}
		if((tAudioCapture_hnd = CreateThreadHelper(HSCapAudio, (void*)this)) == E_THREAD_CREATE)
		{
			LOGE("failed to tHSCapAudio_hnd\n");
			return E_THREAD_CREATE;
		}
	}
	else if(m_cWiMo->GetAudioMethod() == METHOD_ZTE_AUD_MIXER)
	{
		audioHndFD = 0;
		audioDataSize = ZTEDATASIZEAUDIO;
		audioDataBuff = (unsigned char*)malloc(audioDataSize+1);
		if(!audioDataBuff)
		{
			LOGE("failed malloc audio audioDataBuff\n");
			return E_MALLOCFAILED;
		}
		if(ZTEAudioBufferInit() != S_OK)
		{
			LOGE("failed to AudioBufferInit()\n");
			return E_CONNECTFAILED;
		}
		if((tAudioCapture_hnd = CreateThreadHelper(ZTESaveAudioData, (void*)this)) == E_THREAD_CREATE)
		{
			LOGE("failed to fillSendData_hnd\n");
			return E_THREAD_CREATE;
		}
	}
	else 
	{
#ifdef AudioSWCapture
	#if defined(NDK_AUDIO_RECORD)
		// engine interfaces
		engineObject = NULL;
		engineEngine = NULL;

		// recorder interfaces
		recorderObject = NULL;
		recorderRecord = NULL;
		
		NDKCreateEngine();
		ndkAudio_hnd = 0;
		if((ndkAudio_hnd = CreateThreadHelper(NDKInitAudioRecord, (void*)this)) == E_THREAD_CREATE)
		{
			LOGE("failed to ndkAudio_hnd\n");
			return E_THREAD_CREATE;
		}
	#else
		pAudio_Recorder = NULL;
		if((tAudioCapture_hnd = CreateThreadHelper(ProcessAudio, (void*)this)) == E_THREAD_CREATE)
		{
			LOGE("failed to tAudioCapture_hnd\n");
			return E_THREAD_CREATE;
		}
	#endif
#endif
	}

	bAudioInit = true;
	return S_OK;
}

/*****************************************************
* Func: UnInit
* Function: UnInit audio
* Parameters: void
*****************************************************/
SCRESULT CWiMoAudio::UnInit()
{
	if(!bAudioInit)	return E_INVALIDARG;

	if(m_cWiMo->GetAudioMethod() == METHOD_LNV_AUD_MIXER)
	{
		LOGI("CWiMoAudio::UnInit LNV \n");	
	}
	else if(m_cWiMo->GetAudioMethod() == METHOD_HS_AUD_MIXER)
	{
		if(audioDataBuff)
		{
			free(audioDataBuff);
			audioDataBuff = NULL;
		}
	}
	else if(m_cWiMo->GetAudioMethod() == METHOD_ZTE_AUD_MIXER)
	{
		ZTEAudioBufferUnInit();
		if(audioDataBuff)
		{
			free(audioDataBuff);
			audioDataBuff = NULL;
		}
	}
	else
	{
#ifdef AudioSWCapture
	#if defined(NDK_AUDIO_RECORD)
		if(NDKStopRecord() != S_OK)
			LOGE("NDKStopRecord() error\n");
		NDKUnInitAudio();
		NDKDestoryEngine();
		/*
		if(ndkAudio_hnd)
		{
			pthread_join(ndkAudio_hnd , NULL);
			ndkAudio_hnd  = 0;
		}*/
	#else
		if(pAudio_Recorder)
		{
			pAudio_Recorder->stop();
			delete pAudio_Recorder;
			pAudio_Recorder = 0;
		}
	#endif
#endif
	}
	
	if(tAudioCapture_hnd)
	{
		pthread_join(tAudioCapture_hnd, NULL);
		tAudioCapture_hnd = 0;
	}
	if(tAudioRenderThread_hnd)
	{
		pthread_join(tAudioRenderThread_hnd, NULL);
		tAudioRenderThread_hnd = 0;
	}
	if(g_AudioBuff)
	{
		delete g_AudioBuff;
		g_AudioBuff = NULL;
	}
	if(socketFD != S_OK)
	{
		close(socketFD);
		socketFD = 0;
	}
	bAudioInit = false;
	return S_OK;
}

/*****************************************************
* Func: FILEDATABUFFER
* Function: start audio record
* Parameters: param: thread input parameter
*****************************************************/
FILEDATABUFFER::FILEDATABUFFER()
{
	file = fopen("/data/data/audiorecord.pcm", "r");
	validDataSize = 0;
}

FILEDATABUFFER::~FILEDATABUFFER()
{
	fclose(file);
}


int FILEDATABUFFER::SendData(int size,unsigned char* ppBuff)
{
	validDataSize  += size;
	return size;
}

int FILEDATABUFFER::GetData(int size,unsigned char* ppBuff)
{
	long dataInBuffer = 0;

	if(validDataSize < size )
		return S_OK;

	char dummyBuff[size];
	if(ppBuff)
		fread(ppBuff, 1, size, file);
	else
		fread(dummyBuff, 1, size, file);
	
	validDataSize  -= size;
//	usleep(3*1000);
//	LOGE("Decrease data[%d] mReadPointer:%p mBuff :%p idx:%d\n",size,mReadPointer ,mBuff , mReadPointer - mBuff );

	return size;
}

int FILEDATABUFFER::SkipData(int size)
{
	LOGE("SkipData :%d ",size);
	if(validDataSize <size)
		size = validDataSize;

	GetData(size,0);
	return S_OK;
}


LIVEDATABUFFER::LIVEDATABUFFER(CWiMoControl *pWimo)
{
	m_cWiMo = pWimo;
	mBuff = NULL;
	bufferSize = AUDIOBUFFERSIZE;
	mBuff = (unsigned  char*)malloc(bufferSize);
	memset(mBuff, 0 ,sizeof(bufferSize));
	mWritePointer = mBuff;
	mReadPointer = mBuff;
	validDataSize = 0;

//	m_dumpFile = fopen("/data/data/audiodump.pcm", "wb");
//	LOGE("m_dumpFile :%p",m_dumpFile );
}

LIVEDATABUFFER::~LIVEDATABUFFER()
{
//	fclose(m_dumpFile);
	free(mBuff);
}

int LIVEDATABUFFER::SendData(int size,unsigned char* ppBuff)
{
	if(mBuff == NULL)
		return 0;

	long endBufferSize = mBuff + bufferSize - mWritePointer; 
	while(m_cWiMo->GetStatusWiMo() != WIMO_STATUS_FINISH && validDataSize + size > bufferSize)
	{
		//LOGE("saving audio data!!!\n");
		usleep(2*NumOneThousand);
	}
//    fwrite(ppBuff, size, 1, m_dumpFile);

	if(endBufferSize >= size)
	{
		memcpy(mWritePointer, ppBuff, size);
		mWritePointer += size;
	}
	else
	{
		long startBufferSize = size - endBufferSize;
		memcpy(mWritePointer, ppBuff, endBufferSize);

		memcpy(mBuff , ppBuff+endBufferSize , startBufferSize);
		mWritePointer = mBuff + startBufferSize;
	}
	validDataSize  += size;
//	LOGE("Add data[%d] mWritePointer:%p mBuff :%p idx:%d\n",size,mWritePointer,mBuff , mWritePointer - mBuff );
	return size;
}

int LIVEDATABUFFER::GetData(int size,unsigned char* ppBuff)
{
	if((validDataSize < size ))
		return S_OK;

//copy out data
	long endBufferSize = mBuff + bufferSize - mReadPointer; 
	if(endBufferSize > size)
	{
		if(ppBuff)
			memcpy(ppBuff, mReadPointer, size);
		mReadPointer += size;
	}
	else
	{
		if(ppBuff)
		{
			memcpy(ppBuff, mReadPointer, endBufferSize);
			memcpy(ppBuff+endBufferSize, mBuff,size-endBufferSize);
		}
		mReadPointer  = mBuff + size-endBufferSize;
	}

	validDataSize  -= size;
	//usleep(3*NumOneThousand);
//	LOGE("Decrease data[%d] mReadPointer:%p mBuff :%p idx:%d\n",size,mReadPointer ,mBuff , mReadPointer - mBuff );

	return size;
}

int LIVEDATABUFFER::SkipData(int size)
{
	LOGE("SkipData :%d ",size);
	if(validDataSize  <size)
		size = validDataSize;

	GetData(size,0);
	return S_OK;
}

void LIVEDATABUFFER::ResetValidDataSize()
{
	validDataSize = 0;
}

CSENDAUDIO::CSENDAUDIO(CWiMoAudio *pis)
{
	CWiMoAudio *pthis = (CWiMoAudio *)pis;
	m_socketFD = pthis->socketFD;
	mBuff = (unsigned  char*)malloc(1*1024*1024);

	memset(&serveraudio, 0,sizeof(serveraudio));
	serveraudio.sin_family=AF_INET;
	serveraudio.sin_addr.s_addr=INADDR_ANY;
	serveraudio.sin_port=htons(AUDIOPORT); 

	len = sizeof(sockaddr_in);
}

CSENDAUDIO::~CSENDAUDIO()
{
	free(mBuff);
}

void CSENDAUDIO::SetServerAddr(long server_audio)
{
	serveraudio.sin_addr.s_addr = server_audio;
}
	
int CSENDAUDIO::SendData(unsigned char * data,int dataSize)
{
	if(serveraudio.sin_addr.s_addr != 0)
	{
		int i=0;
		for(i=0;i<dataSize/1024;i++)
		{
			memset(mBuff,0,4);
			int j=0;
			for(j=0;j<1024/2;j++)
			{
				mBuff[4+j*2] = data[i*1024 + j*2 +1];
				mBuff[4+j*2+1] = data[i*1024 + j*2 ];
			}
			//        memcpy(Buffer + 4,data + i*1024,1024);
			sendto(m_socketFD,mBuff,4+1024,0,(struct sockaddr*)&serveraudio,len);
		}

		if(dataSize %1024)
		{
			memset(mBuff,0,4);
			int j=0;
			for(j=0;j<(dataSize%1024)/2;j++)
			{
				mBuff[4+j*2] = data[i*1024 + j*2 +1];
				mBuff[4+j*2+1] = data[i*1024 + j*2 ];
			}

			//        memcpy(Buffer + 4,data + i*1024,dataSize%1024);
			sendto(m_socketFD,mBuff,4+dataSize%1024,0,(struct sockaddr*)&serveraudio,len); 
		}
	}
	return S_OK;
}

void* CWiMoAudio::AudioRenderThread(void* param)
{
	CWiMoAudio *pthis = (CWiMoAudio*)param;
	
	long dataPerSec = 44100*2*2;
	unsigned char buffer[READPERTIME] ;
	bool bFirstly = true;
	long byteRead = 0;
	memset(buffer, 0 ,sizeof(buffer));

#if ANDROID_RESOURCE_BUILDER
	pid_t tid  = gettid();
	LOGI("AudioRenderThread tid: %d\n", tid);
	set_sched_policy(tid, SP_FOREGROUND);
	setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_AUDIO);
#endif	
	pthis->g_sendAudio  = new CSENDAUDIO(pthis);
	DDword startTime = GetTickCount();
	while(pthis->m_cWiMo->GetStatusWiMo() != WIMO_STATUS_FINISH)
	{
		if(pthis->m_cWiMo->GetStatusWiMo() != WIMO_STATUS_CONNECTING)
		{
			//LOGI("handshake is not successful, continue\n");
			usleep(QUICKWAITTIMES);
			continue;
		}
		else if(bFirstly)
		{
			 pthis->g_sendAudio->SetServerAddr(pthis->m_cWiMo->pCNetwork->GetServerIpAddr());
			 bFirstly = false;
		}
//GetData

	#if defined(NDK_AUDIO_RECORD)
		byteRead  = m_AudioBuff.GetData(READPERTIME, buffer);
	#else
		byteRead  = pthis->g_AudioBuff->GetData(READPERTIME, buffer);
	#endif
		
		if(byteRead  != READPERTIME)
		{
			usleep(1*NumOneThousand);
			continue;
		}
		
//SendData
		pthis->g_sendAudio->SendData(buffer, byteRead);

		usleep(3*NumOneThousand);
	#if 0
//Sleep			
		float costTime= GetTickCount() - startTime;
		float idealTime =  READPERTIME*1000/dataPerSec;
//		LOGE("costTime:%d = idealTime :%d \n",costTime,idealTime);

		if(costTime < idealTime)
			usleep(1000*(idealTime - costTime));
		else
		{
			float wasteTime = (costTime -idealTime)/1000;
			float wasteData = dataPerSec* wasteTime;
//	LOGE("costTime:%f = idealTime :%f \n",costTime,idealTime);
			LOGE("wasteTime :%f = wasteData:%f \n",wasteTime ,wasteData);
			//SkipData
//	g_AudioBuff.SkipData(wasteData);
		}
		startTime = GetTickCount();
	#endif
	}

	pthis->g_AudioBuff->ResetValidDataSize();
	
	LOGI("Quit AudioRenderThread");
	delete pthis->g_sendAudio;
	pthis->g_sendAudio = NULL;

	LOGI("Quit AudioRenderThread success ");
	return 0;
}

/*****************************************************
* Func: ConnectServer
* Function: creat socket dategram to check port
* Parameters: void
*****************************************************/
int CWiMoAudio::ConnectServer(void)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(AUDIOPORT);
	
	socketFD=socket(AF_INET,SOCK_DGRAM,0);
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, (const struct sockaddr *)&addr, sizeof(&addr));
	if(bind(socketFD, (const struct sockaddr *)&addr, sizeof(addr)) !=0)
	{
		close(socketFD);
		socketFD = 0;
		LOGE("Failed to audio bind\n");
		return E_FAILED;
	}
	return S_OK;
}

/*****************************************************
* Func: HSCapAudio
* Function: HS capture audio
* Parameters: param: thread input parameter
*****************************************************/
void* CWiMoAudio::HSCapAudio(void* param)
{
	CWiMoAudio *pthis = (CWiMoAudio*)param;
	int ret = -1;
	int bufferData = 0;
	bool bOnlyOneRun = true;

#if ANDROID_RESOURCE_BUILDER	
	pid_t tid  = gettid();
	LOGE("HSCapAudio tid: %d\n", tid);
	set_sched_policy(tid, SP_FOREGROUND);
	setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_AUDIO);
#endif	
	while(pthis->m_cWiMo->GetStatusWiMo() != WIMO_STATUS_FINISH)
	{
		if(pthis->m_cWiMo->GetStatusWiMo() != WIMO_STATUS_CONNECTING)
		{
			usleep(QUICKWAITTIMES);
			continue;
		}
		if(bOnlyOneRun && pthis->SetAudioOutputEnable_I)
		{
			(*pthis->SetAudioOutputEnable_I)(HS_AUDIO_OFF);
			bOnlyOneRun = false;
		}
		
		//ret = readPcmData(pthis->audioDataBuff, &pthis->audioDataSize, &pthis->ddTimeStampAud);	
		ret = (*pthis->readPcmData_R)(pthis->audioDataBuff, &pthis->audioDataSize, &pthis->ddTimeStampAud);	
		//if(readPcmData(pthis->audioDataBuff, &pthis->audioDataSize, &pthis->ddTimeStampAud) != 0)
		//	continue;
		if(pthis->audioDataSize == 0)
		{
			usleep(1*NumOneThousand);
			continue;
		}
		//LOGI("ret: %d, audioDataSize:%d, ddTimeStampAud:%lld\n", ret, pthis->audioDataSize, pthis->ddTimeStampAud);

		pthis->g_AudioBuff->SendData(pthis->audioDataSize, pthis->audioDataBuff);
#if 0
		bufferData += pthis->audioDataSize;
		if(bufferData > DATA_PER_SECOND_441K)
		{
			bufferData = 0;
			usleep(NumOneThousand*NumOneThousand);
		}
#else		
		usleep(5*NumOneThousand);
#endif
	}

	if(pthis->SetAudioOutputEnable_I)
		(*pthis->SetAudioOutputEnable_I)(HS_AUDIO_ON);

	LOGI("quit audio\n");

	return 0;
}

/*****************************************************
* Func: AudioBufferInit
* Function: init aduio buffer
* Parameters: void
*****************************************************/
int CWiMoAudio::ZTEAudioBufferInit(void)
{
	char cDevNode[128];
	sprintf(cDevNode,"/dev/pipe-pcm");
	LOGE("test %s \n",cDevNode);
	audioHndFD = open(cDevNode, O_RDONLY);
	if(audioHndFD < 0)
	{
		LOGE("/dev/pcm-pipe open error\n");
		return E_FAILED;
	}

	return S_OK;
}

/*****************************************************
* Func: AudioBufferUnInit
* Function: uninit audio buffer
* Parameters: void
*****************************************************/
void CWiMoAudio::ZTEAudioBufferUnInit(void)
{
	if(audioHndFD != 0)
	{
		close(audioHndFD);
		audioHndFD =0;
	}
}

/*****************************************************
* Func: ZTESaveAudioData
* Function: save audio data
* Parameters: param: thread input parameter
*****************************************************/
void* CWiMoAudio::ZTESaveAudioData(void* param)
{
	CWiMoAudio* pthis = (CWiMoAudio*)param;
	int rSize = 0;
	int bufferData = 0;

#if ANDROID_RESOURCE_BUILDER	
	pid_t tid  = gettid();
	LOGE("ZTESaveAudioData tid: %d\n", tid);
	set_sched_policy(tid, SP_FOREGROUND);
	setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_AUDIO);
#endif

	while(pthis->m_cWiMo->GetStatusWiMo() != WIMO_STATUS_FINISH)
	{
		if((rSize = read(pthis->audioHndFD, pthis->audioDataBuff, pthis->audioDataSize)) == -1)
		{
			LOGE("read audioHndFD error!\n");
			return (void*)E_FAILED;
		}

		if(rSize > 0)
		{
			//LOGI("rSize: %d\n", rSize);
			pthis->g_AudioBuff->SendData(rSize, pthis->audioDataBuff);
			bufferData += rSize;
		}
		else
			continue;

		if(bufferData >= DATA_PER_SECOND_441K)
		{
			bufferData = 0;
			usleep(NumOneThousand*NumOneThousand);
		}	
	}
	
	LOGI("ZTESaveAudioData audio quit success!");
	return 0;
}

#ifdef AudioSWCapture
#if defined(NDK_AUDIO_RECORD)
void CWiMoAudio::NDKCreateEngine()
{
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
}

void CWiMoAudio::NDKDestoryEngine()
{
	// destroy engine object, and invalidate all associated interfaces
	if (engineObject != NULL) 
	{
		(*engineObject)->Destroy(engineObject);
		engineObject = NULL;
		engineEngine = NULL;
	}
}

void* CWiMoAudio::NDKInitAudioRecord(void* ptr)
{
	CWiMoAudio* pthis = (CWiMoAudio*)ptr;

	SLresult result;

	// configure audio source
	SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
	        SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
	SLDataSource audioSrc = {&loc_dev, NULL};

	// configure audio sink
	SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
	    SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
	    SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
	SLDataSink audioSnk = {&loc_bq, &format_pcm};

	// create audio recorder
	// (requires the RECORD_AUDIO permission)
	const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
	const SLboolean req[1] = {SL_BOOLEAN_TRUE};
	result = (*pthis->engineEngine)->CreateAudioRecorder(pthis->engineEngine, &pthis->recorderObject, &audioSrc,
	        &audioSnk, 1, id, req);
	if (SL_RESULT_SUCCESS != result) {
	    return JNI_FALSE;
	}

	// realize the audio recorder
	result = (*pthis->recorderObject)->Realize(pthis->recorderObject, SL_BOOLEAN_FALSE);
	if (SL_RESULT_SUCCESS != result) {
	    return JNI_FALSE;
	}

	// get the record interface
	result = (*pthis->recorderObject)->GetInterface(pthis->recorderObject, SL_IID_RECORD, &pthis->recorderRecord);
	assert(SL_RESULT_SUCCESS == result);

	// get the buffer queue interface
	result = (*pthis->recorderObject)->GetInterface(pthis->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
	        &recorderBufferQueue);
	assert(SL_RESULT_SUCCESS == result);

	// register callback on the buffer queue
	result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, pthis->NDKbqRecorderCallback,
	        recorderBuffer);
	assert(SL_RESULT_SUCCESS == result);

	pthis->NDKStartRecording();

	return 0;
}

void CWiMoAudio::NDKUnInitAudio()
{
	// destroy audio recorder object, and invalidate all associated interfaces
	if (recorderObject != NULL) 
	{
		(*recorderObject)->Destroy(recorderObject);
		recorderObject = NULL;
		recorderRecord = NULL;
		recorderBufferQueue = NULL;
	}
}

void CWiMoAudio::NDKStartRecording()
{
	SLresult result;

	// in case already recording, stop recording and clear buffer queue
	result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
	assert(SL_RESULT_SUCCESS == result);
	result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
	assert(SL_RESULT_SUCCESS == result);

	// the buffer is not valid for playback yet
	recorderSize = 0;

	// enqueue an empty buffer to be filled by the recorder
	// (for streaming recording, we would enqueue at least 2 empty buffers to start things off)
	result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBuffer,
	        RECORDER_FRAMES * sizeof(short));
	// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
	// which for this code example would indicate a programming error
	assert(SL_RESULT_SUCCESS == result);

	// start recording
	result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
	assert(0 == result);
}

int CWiMoAudio::NDKStopRecord()
{
	SLresult result;
	result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
	if (SL_RESULT_SUCCESS == result) 
	{
		recorderSize = RECORDER_FRAMES * sizeof(short);
		recorderSR = SL_SAMPLINGRATE_44_1;
		return S_OK;
	}
	return E_FAILED;
}

void CWiMoAudio::NDKbqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	assert(bq == bqRecorderBufferQueue);
	assert(NULL == context);
	// for streaming recording, here we would call Enqueue to give recorder the next buffer to fill
	// but instead, this is a one-time buffer so we stop recording
	recorderSize = RECORDER_FRAMES * sizeof(short);
	recorderSR = SL_SAMPLINGRATE_44_1;
#if 0
	static const char* g_pcmFileName = "/sdcard/testrecord.pcm";
	FILE * file = fopen(g_pcmFileName, "ab+");
	if (file == NULL)
	{
		LOGE("Failed to open record file %s\n", g_pcmFileName);
		return;
	}
	LOGI("recorderSize: %d,L:%d\n",recorderSize, __LINE__);
	fwrite(context, 1, recorderSize, file);
	fclose(file);
#endif

	m_AudioBuff.SendData(recorderSize, (unsigned char*)context);

	SLresult result;
	result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBuffer,
	RECORDER_FRAMES * sizeof(short));
	// the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
	// which for this code example would indicate a programming error
	assert(0 == result);

}

#else

/*****************************************************
* Func: AudioRecordCb
* Function: audio record callback and send audio data
* Parameters:
*	event: get audio record event
*	user: no use
*	info: audio infomation about source data and size
*****************************************************/
void CWiMoAudio::AudioRecordCb(int event, void* user, void *info)
{
	CWiMoAudio* pthis =  (CWiMoAudio*)user ;

	if ((pthis->m_cWiMo->GetStatusWiMo() == WIMO_STATUS_CONNECTING) && event == AudioRecord::EVENT_MORE_DATA)
	{
		AudioRecord::Buffer * audioBuf = (AudioRecord::Buffer *)info;
		pthis->g_AudioBuff->SendData(audioBuf->size, (unsigned char*)audioBuf->raw);
	}
}

/*****************************************************
* Func: ProcessAudio
* Function: start audio record
* Parameters: param: thread input parameter
*****************************************************/
void* CWiMoAudio::ProcessAudio(void* param)
{
	CWiMoAudio *pthis = (CWiMoAudio*)param;

#ifdef NEXUSONE_AUDIO_MIXER
	pthis->pAudio_Recorder = new AudioRecord(AUDIO_SOURCE_MIXER, 44100, AudioSystem::PCM_16_BIT, 
		AudioSystem::CHANNEL_IN_STEREO, 0, 0, pthis->AudioRecordCb, pthis);
#else
	pthis->pAudio_Recorder = new AudioRecord(AUDIO_SOURCE_DEFAULT, 44100, AudioSystem::PCM_16_BIT, 
		AudioSystem::CHANNEL_IN_STEREO, 0, 0, pthis->AudioRecordCb, pthis);
#endif
	if (OK != pthis->pAudio_Recorder->initCheck())
	{
		pthis->m_cWiMo->SetStatusWiMo(WIMO_STATUS_FINISH);
		LOGE("Failed to init AudioRecord!\n");
		return (void*)E_FAILED;
	}
	if (OK != pthis->pAudio_Recorder->start())
	{
		LOGE("Failed to start audio recorder!\n");
		pthis->m_cWiMo->SetStatusWiMo(WIMO_STATUS_FINISH);
		return (void*)E_FAILED;
	}
	LOGI("audio recoder started !!! \n");

	return 0;
}
#endif
#endif /*defined(AudioSWCapture)*/

