#ifndef __CAPTURE_SCREEN_H__
#define __CAPTURE_SCREEN_H__

#include <dlfcn.h> 

#include "ci_codec_type.h"
#include "ci_imageproc.h"
#include "ci_jpegenc.h"
#include "capture_type.h"
#include "capture_network.h"
#include "checkCPU.h"
#include "capture_audio.h"
#include "capture_video.h"
#include "HSCapInterface.h"
#include "ZteInterface.h"

//time bomb
#define INTERNATIONAL_STD_YEAR 1900
#define TIME_BOMB_YEAR 2013
#define TIME_BOMB_MONTH 2	//deadline month 3, 2 means target month - 1 
#define TIME_BOMB_DAY 31
#define TIME_BOMB_HOUR 23
#define TIME_BOMB_MINUTE 59
#define TIME_BOMB_SECOND 59

#define VIDEO_SRC_BUFFER_SIZE 1024*1024*4

typedef enum{
	CALLINGENTRANCEFIRSTLY = 0,
	CALLINGBUFFER1 = 1,
	CALLINGBUFFER2 = 2,
}CALLING_BUFFER_INDEX;

typedef enum{
	BUFFERWRITE = 0,
	BUFFERREAD = 1,
}READ_WRITE_BUFFER;

namespace android {

class CCPUControl;
class CWiMoVideo;
class CWiMoAudio;
class CWiMoNetwork;
class LIVEDATABUFFER;

class CWiMoControl
{
public:
	CWiMoControl(JNIEnv * env, jobject thiz, jint videoMode, jint audioMode, char* libPath);
	~CWiMoControl();
	int  JniInit(JNIEnv *env, jobject thiz);
	int JniUnInit(JNIEnv *env, jobject thiz);
	int StartWiMo();
	int StopWiMo();
	int SendVideoData(int dataType, char* data, int vSizeHW, int width, int height, int rotate);
	int SendAudioData(int dataType, char* data, int size, int sampleRate, int bitRate, int channels);
	int SetTVRotate(int nTVRotate);
	int SetVideoRotate(int nRotate);
	int CheckFB0();
	int NotifyScreenshotCalled(int called);

	inline void SetStatusWiMo(int status){statusWiMo = status;}
	inline int GetStatusWiMo(void){return statusWiMo;}
	inline void SetRotateOrientation(int rotate){rotateFlag = rotate;}
	inline int GetRotateOrientation(void){return rotateFlag;}
	inline int GetUIRotateOrientation(void){return iUIrotate;}
	inline int GetTVRotateOrientation(void){return iTvRotate;}
	inline int GetDeviceWidth(void){return SRC_WIDTH;}
	inline int GetDeviceHeight(void){return SRC_HEIGHT;}
	inline int GetAlignedWidth(void){return ALIGNED_WIDTH;}
	inline int GetAlignedHeight(void){return ALIGNED_HEIGTH;}
	inline int GetCallingBuffIndex(){return iCallingBuffIndex;}
	inline void SetCallingBuffIndex(int index){iCallingBuffIndex = index;}
	inline int GetCallingSrcIndex(){return iCallingSrcIndex;}
	inline void SetCallingSrcIndex(int index){iCallingSrcIndex = index;}
	inline int GetScreenshotSwitch(void){return m_calledScreenshot;}
	
	char* GetVideoDataBuff(int *size);
	void SetDeviceWidthHeight(int width, int height);
	void SetAlignedWidthHeight(int width, int height);
	void* GetLibHandle(void){return pdl;}
	
	int GetAudioMethod(void);
	int GetVideoMethod(void);
	void SetVideoMethod(int method);
	int GetDeviceModel(void);
	int GetVideoType(void);
	bool GetLimitedFPS(void);
	
	void NotifyUIStop(void);	
	
	CCPUControl* pCPUControl;
	CWiMoVideo* pCVideo;
	CWiMoAudio* pCAudio;
	CWiMoNetwork* pCNetwork;
private:
	int Init(void);
	void UnInit(void);
		
	static void* CheckConnectTimeoutThread(void* param);

	void JniStatusChangedCallBack(int connectionFlag);
private:
	JAVA_CALLBACK_FUN jCallbackFun;
	struct tm sTimeBomb;
	DDword timeBomb;

	JNIEnv * m_env;
	jobject m_thiz;

	void* pdl;
	pF_VOID Stop_V;
	pF_INT_INT Start_II;

	char pAppLibPath[STRINGDATESIZE];
	char* pVideoBuff;
	Byte* pVidSrcBuff1;
	Byte* pVidSrcBuff2;
	
	bool bJNIInit;
	bool bQuitJNIFlag;
	bool bConvertWH;
	bool bSrcBuffFlag1;
	bool bSrcBuffFlag2;
	bool bLock;
	bool bLimitedFPS;
	
	int iCallingBuffIndex;
	int iCallingSrcIndex;
	int statusWiMo;
	int rotateFlag;
	int iUIrotate;
	int iCaptureMethod;
	int iDeviceModel;
	int iTvRotate;
	int iAudioCapMethod;
	int iHSHandle;
	int iVidDataType;
	int currUpdate;
	int prevUpdate;
	int m_calledScreenshot;
	
	int SRC_WIDTH;
	int SRC_HEIGHT;
	int ALIGNED_WIDTH;
	int ALIGNED_HEIGTH;
	
	int iVideoSize;
	int iVidSrcBuffSize1;
	int iVidSrcBuffSize2;
	LIVEDATABUFFER* pAudioBuff;

	pthread_t checkCPUUtilization_hnd;	
	pthread_t checkTimoutThread_hnd;
	pthread_t LoopVideoThread_hnd;	
};

};
#endif
