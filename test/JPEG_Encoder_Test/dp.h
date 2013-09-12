#ifndef _VRPRESENT_DP_H_
#define _VRPRESENT_DP_H_

//#define WRITE_FILE

#define ANDROID
#ifdef ANDROID
#ifndef ANDROID_NDK
#include "utils/Log.h"
#else
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif
#undef LOG_TAG
#define LOG_TAG "VRP"
#endif

#ifndef __linux__
#pragma warning (disable:4995)
#pragma warning (disable:4996)
#endif

#endif
