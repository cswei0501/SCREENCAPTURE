/*
 * Copyright (C) 2009 The Android Open Source Project
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
 *
 */
#ifndef __ZTE_INTERFACE_H__
#define __ZTE_INTERFACE_H__

#include <math.h>

#define ZTEVIDEODATAYUV "/data/data/wimo.yuv"
#define MAXINTNUM (int)(pow(2, 31) -1)

typedef enum
{
	YV12 = 0,
	NV12,
	RGB565,
	RGB888,
	ARGB8888,
	VIDEOFORAMT_MAX	
}VIDEOFORAMT;


typedef struct
{
	VIDEOFORAMT fmt;
	long width;
	long height;
}WIMOHEADER_FMT;

typedef struct
{
	WIMOHEADER_FMT header;
	int stride;
	int index;
}WIMOPREFIX;

//Return Valude:
//0 : means successed.
extern int  CaptureVideo(unsigned char* pBuffer, long width, long height,  VIDEOFORAMT format);

#endif /* __ZTE_INTERFACE_H__ */
