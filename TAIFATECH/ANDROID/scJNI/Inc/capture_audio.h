#ifndef __CAPTURE_AUDIO_H__
#define __CAPTURE_AUDIO_H__

#include "capture_type.h"
#include "capture_control.h"

namespace android{

#define AUDIOPORT				0x0812
#define DATA_PER_SECOND_441K 	44100*2*2
#define DATA_PER_SECOND_48K		48000*2*2
#define ZTEDATASIZEAUDIO 			44100
#define READPERTIME 				1024
#define AUDIOBUFFERSIZE 			1*1024*1024

class CWiMoControl;
class CSENDAUDIO;

class FILEDATABUFFER
{
public:
	FILEDATABUFFER();
	virtual ~FILEDATABUFFER();

	int SendData(int size,unsigned char* ppBuff);
	int GetData(int Req_size,unsigned char* ppBuff);
	int SkipData(int size);
	FILE* file ;

	long validDataSize;
};

class LIVEDATABUFFER
{
public:
	LIVEDATABUFFER(CWiMoControl *pWimo);
	virtual ~LIVEDATABUFFER();

	int SendData(int size,unsigned char* ppBuff);
	int GetData(int Req_size,unsigned char* ppBuff);
	int SkipData(int size);
	void ResetValidDataSize();
	unsigned char* mBuff;
	long bufferSize;
	
	long validDataSize;
	unsigned char* mWritePointer;
	unsigned char* mReadPointer;
	FILE* m_dumpFile;
	CWiMoControl *m_cWiMo;
};

class CWiMoAudio
{
public:
	CWiMoAudio(CWiMoControl* pWimo);
	~CWiMoAudio();
	SCRESULT Init();
	SCRESULT UnInit();
	inline void* GetSendAudioBuffer(){return (void*)g_AudioBuff;};
	int socketFD;
	CWiMoControl *m_cWiMo;
	CSENDAUDIO* g_sendAudio;
private:
	int ConnectServer();
	static void* AudioRenderThread(void* param);
	static void* HSCapAudio(void* param);

	int ZTEAudioBufferInit();
	void ZTEAudioBufferUnInit();
	static void* ZTESaveAudioData(void* param);
#ifdef AudioSWCapture	
	#if defined(NDK_AUDIO_RECORD)
	void NDKCreateEngine();
	void NDKDestoryEngine();
	static void* NDKInitAudioRecord(void* ptr);
	void NDKUnInitAudio();
	void NDKStartRecording();
	int NDKStopRecord();
	static void NDKbqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
	#else
	static void* ProcessAudio(void* param);
	static void AudioRecordCb(int event, void* user, void *info);
	#endif
#endif
private:
	bool bAudioInit;
	Byte* audioDataBuff;
	int audioDataSize;
	int audioHndFD;

#ifdef AudioSWCapture	
	#if defined(NDK_AUDIO_RECORD)
	// engine interfaces
	SLObjectItf engineObject;
	SLEngineItf engineEngine;

	// output mix interfaces
	SLObjectItf outputMixObject;
	SLEnvironmentalReverbItf outputMixEnvironmentalReverb;

	// recorder interfaces
	SLObjectItf recorderObject;
	SLRecordItf recorderRecord;

	pthread_t ndkAudio_hnd;
	#else
	AudioRecord* pAudio_Recorder;
	#endif
#endif
	pthread_t tAudioCapture_hnd;
	pthread_t tAudioRenderThread_hnd;

	void* pdl;
	pF_INT SetAudioOutputEnable_I;
	pF_pUCHAR_pINT_pINT64 readPcmData_R;
	
	DDword ddTimeStampAud;
	//FILEDATABUFFER g_AudioBuff;
	LIVEDATABUFFER *g_AudioBuff;
};

class CSENDAUDIO
{
public:
	CSENDAUDIO(CWiMoAudio* pis);
	virtual ~CSENDAUDIO();
	int SendData(unsigned char * data,int dataSize);
	void SetServerAddr(long server_audio);
	int m_socketFD;
	unsigned char* mBuff;
	struct sockaddr_in serveraudio;
	int len;
};

};
#endif
