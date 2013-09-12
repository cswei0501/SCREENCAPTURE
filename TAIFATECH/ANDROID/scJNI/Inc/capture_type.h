#ifndef __CAPTURE_TYPE_H__
#define __CAPTURE_TYPE_H__

#define OUTLOG 0
#define CHECK_CRC 1
#undef LOG_TAG
#define LOG_TAG "Screen_capture"
#define DEBUG 0

#include <arpa/inet.h>
#include <net/if_arp.h> 
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <jni.h>
#include <stdarg.h>
#include <time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <binder/IMemory.h>

#ifdef MTK_CAPTURE_FRAME_BUFFER
#include "mtkfb.h"
#endif

#if defined(ANDROID_RESOURCE_BUILDER) || defined(CYANOGEN_RESOURCE_BUILDER)
#include <media/AudioRecord.h>
#include <media/mediarecorder.h>
using namespace android;
#endif

#ifdef NDK_RESOURCE_BUILDER
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Screen_capture", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "Screen_capture", __VA_ARGS__))
#if defined(NDK_AUDIO_RECORD)
#include <assert.h>
// for native audio
#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// 40 mili seconds of recorded audio at 44.1 kHz stereo, 16-bit signed little endian
#define RECORDER_FRAMES ((44100*2*16/8)/25)
#endif
#endif

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Screen_capture", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "Screen_capture", __VA_ARGS__))
#define LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "Screen_capture", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "Screen_capture", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "Screen_capture", __VA_ARGS__))

#ifndef NDK_RESOURCE_BUILDER
//system method to capture
#include <SkImageEncoder.h>
#include <SkBitmap.h>
#include <SkStream.h>
#endif

#include "ci_codec_type.h"
#include "ci_imageproc.h"
#include "ci_jpegenc.h"

namespace android{

typedef char							Char;				///< char
typedef unsigned char					Byte;				///< unsigned char
typedef void* 							LPVOID;
typedef unsigned int						Dword;			///< unsigned int
typedef long long						DDword;			///< unsigned long long
typedef unsigned short					Word;				///< unsigned short

typedef void* (*PFunc)(void*);
typedef int (*pF_VOID)();
typedef int (*pF_pVOID)(void*);
typedef int (*pF_INT)(int);
typedef int (*pF_INT_INT)(int, int);
typedef int (*pF_pINT_pINT)(int*, int*);
typedef int (*pF_pCHAR_pINT_INT)(char* &, int*, int);
typedef int (*pF_pUCHAR_pINT_pINT64)(Byte*, int*, DDword*);
typedef int (*pF_pUCHAR_pINT_pINT64_pINT)(Byte*, int*, DDword*,int*);

typedef int (*pF_ppVOID_pVOID_pCI_IMAGEPROC)(void**,void*,CI_IMAGEPROC_CREATEOPTION*);
typedef int (*pF_pVOID_ppU8_pU32_ppU8_pU32_pCI_IMAGEPROC)(void*,CI_U8**, \
	CI_U32*,CI_U8**,CI_U32*,CI_IMAGEPROC_PROCESSOPTION*);
typedef int (*pF_ppVOID_pJPGENC)(void**, CI_JPGENCOPENOPTIONS*);
typedef int (*pF_pVOID_ppU8_pU32_ciSize_pU8_pU32)(void*, CI_U8**, CI_U32*, CI_SIZE, CI_U8*, CI_U32*);


#define SERVERADDR  					inet_addr("192.168.168.56")
#define MULTICASTPACKET 				inet_addr("192.168.8.255")
#define BROADCASTPACKET 				inet_addr("255.255.255.255")
#define FRAMEBUFERFB 				"/dev/graphics/fb0"
#define STRINGDATESIZE				256
#define CIDATAFILEPATH				"/data/data/cidana.wimo/lib/sl_fifo"

typedef enum
{
	NumFiftyTimes = 50,
	NumOneHundred = 100,
	NumFiveHundred = 500,
	NumNineHundred = 900,
	NumOneThousand = 1000,
	NumOneAndHalfThousand = 1500,
	NumAliveDummyTime = 2000,
	NumTimeOut = 30000
}NUMDEFINED;

#define LOOPWAITTIMES 50*NumOneThousand
#define QUICKWAITTIMES 5*NumOneThousand

typedef enum
{
	S_OK = 0,
	S_CONTINUE,
	E_FAILED,
	E_INVALIDARG,
	E_MALLOCFAILED,
	E_CONNECTFAILED,
	E_THREAD_CREATE
}SCRESULT;

typedef enum tagWIMO_STATUS
{
 	WIMO_STATUS_INITING = 0,
	WIMO_STATUS_CONNECTING,
	WIMO_STATUS_FINISH
 }WIMOSTATUS;

typedef struct _JAVA_CALLBACK_FUN
{
	JNIEnv    						*env;
	JavaVM    						*jvm;
	jobject   						obj;
	jclass    						cls;
	jmethodID 					callbackFuns;
}JAVA_CALLBACK_FUN;

typedef enum tagROTATION
{
	ROTATION_0 = 0,
	ROTATION_90 = 90,
	ROTATION_180 = 180,
	ROTATION_270 = 270
}ROTATION;

typedef enum tagCONNECTION_WIMO
{
	CONNECTIONWIMOFAIL = 0,
	CONNECTIONWIMOSUCCESS = 1,
	CONNECTIONWIMOKICKED = 2,
	CONNECTIONWIMOTIMEOUT = 3,
}CONNECTION_WIMO;

typedef enum tagCAP_VIDEO_METHOD
{
	METHOD_VIDEO_ERROR = 0,
	METHOD_HW_VIDEO_INTERFACE = 1,
	METHOD_FRAMEBUFFER_INTERFACE = 2,
	METHOD_SCREENSHOT_INTERFACE = 3,
	METHOD_TS_STREAM_INTERFACE = 4,
	METHOD_HS_HW_INTERFACE = 5,
	METHOD_LNV_HW_INTERFACE = 6,
	METHOD_ZTE_HW_INTERFACE = 7,
	METHOD_GAME_INTERFACE = 8,
}CAP_VIDEO_METHOD;

typedef enum tagVIDEOTYPE
{
	VIDEO_TYPE_RGB565 = 0,
	VIDEO_TYPE_ARGB4444 = 1,
	VIDEO_TYPE_ARGB8888 = 2,
	VIDEO_TYPE_YUV420 = 3,
	VIDEO_TYPE_YUV422 = 4,
	VIDEO_TYPE_JPEG = 5,
	VIDEO_TYPE_TS = 6,
}VIDEOTYPE;

typedef enum tagCAP_AUDIO_METHOD
{
	METHOD_AUDIO_ERROR = 0,
	METHOD_AUDIO_DEFAULT = 1,
	METHOD_SW_ANDROID_AUD_SPEAKER = 2,
	METHOD_SW_NDK_AUD_SPEAKER = 3,
	METHOD_TS_AUD_STREAM = 4,
	METHOD_HS_AUD_MIXER = 5,
	METHOD_LNV_AUD_MIXER = 6,
	METHOD_ZTE_AUD_MIXER = 7,
	METHOD_GAME_AUDIO = 8,	
}CAP_AUDIO_METHOD;

typedef enum tagCURRENT_TV_ROTATE
{
	CURRENT_TV_ROTATE_0 = 0,
	CURRENT_TV_ROTATE_90R = 1,
	CURRENT_TV_ROTATE_90L = 2
}CURRENT_TV_ROTATE;

typedef enum tagDISCONNECT_REASON
{
	DISCONNECT_ERROR = 0xff,
	DISCONNECT_TIMEOUT = 0x0,
	DISCONNECT_KICKED = 0x01,
 	DISCONNECT_DATALOST = 0x02,
 }DISCONNECT_REASON;

typedef enum tagDEVICEMODEL
{
	DEVICE_MODEL_DEFAULT = 0,
	DEVICE_LENOVO_S899T_40 = 1, //Lenovo s899t android4.0.3
	DEVICE_MOTO_MT887_40 = 2, //Moto MT887 android4.0.3 
	DEVICE_HS_T96_40 = 3, //HS T96 android4.0.3
	DEVICE_ZTE_U970_40 = 4, //ZTE U970 android4.0.3
	DEVICE_XIAOMI2_41 = 5, //XIAOMI 2 android4.1.1
}DEVICEMODEL;

typedef enum{
	CI_PIXEL_FORMAT_UNKNOWN = 0,
	CI_PIXEL_FORMAT_RGBA_8888 = 1,
	CI_PIXEL_FORMAT_RGBX_8888 = 2,
	CI_PIXEL_FORMAT_RGB_888 = 3,
	CI_PIXEL_FORMAT_RGB_565 = 4,
	CI_PIXEL_FORMAT_BGRA_8888 = 5,
	CI_PIXEL_FORMAT_RGBA_5551 = 6,
	CI_PIXEL_FORMAT_RGBA_4444 = 7,
}CI_PIXEL_FORMAT;

typedef struct
{
	int width;
	int height;
	CI_PIXEL_FORMAT fmt;
	size_t size;
}CI_SCRSHOT_HEADER;

typedef enum
{
	CALLEDSHOTOFF = 0,
	CALLEDSHOTON = 1,
}CI_CALLED_SCRSHOT;

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************
* Func: GetTickCount
* Function: get time right now
* Parameters: void
*****************************************************/
inline DDword GetTickCount(void)
{
	struct timeval my_timeval;
	if (gettimeofday(&my_timeval, NULL))
		return S_OK;

	return ((DDword)my_timeval.tv_sec*NumOneThousand) + ((DDword)my_timeval.tv_usec / NumOneThousand);
}

/*****************************************************
* Func: CreateThreadHelper
* Function: creat a thread
* Parameters:
*		PFunc: function pointer
*		SockProc: function name
*****************************************************/
pthread_t CreateThreadHelper(PFunc SockProc, void* pthis);
#ifdef __cplusplus
}
#endif

};
#endif
