/**
 * Capture interface
 *
 * @Auth: wangxuguang@hisensecom.com
 */

#ifndef CAPTURE_INTERFACE__
#define CAPTURE_INTERFACE__

#include <sys/resource.h>
#include <cutils/sched_policy.h>

#ifdef __cplusplus 
extern "C" {
#endif

#define  NO_ERROR  0
#define  TIMEOUT_ERROR  0xffffff92
#define  UNKNOWN_ERROR  0x80000000

typedef enum tagHS_QFACOTR
{
	HS_QFACOTR_10 = 10,
	HS_QFACOTR_15 = 15,
	HS_QFACOTR_20 = 20,
	HS_QFACOTR_30 = 30,
	HS_QFACOTR_40 = 40,
	HS_QFACOTR_50 = 50,
	HS_QFACOTR_60 = 60,
	HS_QFACOTR_70 = 70,
	HS_QFACOTR_80 = 80,
}HS_QFACOTR;

typedef enum tagHS_ROTATION
{
	HS_ROTATE_0 = 0,
	HS_ROTATE_90 = 90,
	HS_ROTATE_180 = 180,
	HS_ROTATE_270 = 270
}HS_ROTATION;

typedef enum tagHS_AUDIO_SWICTH
{
	HS_AUDIO_OFF = 0,
	HS_AUDIO_ON = 1,
}HS_AUDIO_SWICTH;

typedef long long int64;

/**
 * 0 - success
 */
int getScreenDimension(int* width, int *height);

/**
 * 0 - success
 * @param rotateDegree [in] rotate degree value in the range of [0, 90 ,180, 270]
 * @param qFactor [in] JPEG Q factor value in the range of 1-100
 */
int start(int rotateDegree, int qFacotr);

/**
 * 0 - success
 * @param buffer [in/out]
 * @param len [out]
 * @param timeStamp [out] unit: ns
 * @param degree [out]
 */
int readJpegData(unsigned char* buffer, int* len, int64* timeStamp, int* degree);

/**
 * 0 - success
 * @param buffer [in/out]
 * @param len [out]
 * @param timeStamp [out] unit: ns
 * @param degree [out]
 */
int readPcmData(unsigned char* buffer, int* len, int64* timeStamp);

/**
 * 0 - success
 */
int stop();

/**
 * @param degree [0/90/180/270]
 */
void rotate(int degree);

/**
 * @param enable [in] 0:disable, 1:enable auido output
 */
void setAudioOutputEnable(int enable);

#ifdef __cplusplus 
}
#endif

#endif // CAPTURE_INTERFACE__
