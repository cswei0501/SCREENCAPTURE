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

#include <stdio.h>
#include <stdlib.h>
#include "wimo.h"

#include <fcntl.h> 
#include <unistd.h> 
#include <sys/mman.h> 
#include <math.h>
#include <time.h>

#ifdef __linux__
#include <stdarg.h>
#endif

#define ANDROID_NDK
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

typedef long long						DDword;			///< unsigned long long

#define ALIGNED_UP_16 15
#define NumOneThousand 1000
#define SHAREFILENAME "/data/data/wimo.yuv"

unsigned char* iHandBuff = NULL;
static int reMapFile = 1;
static int currIndex = 0;
static long sizeData = 0;
int frames = 0;
int mapLen = 0;
long alignedWidth = 0;
long alignedHeight = 0;
WIMOPREFIX prefix;
DDword sTime = 0;

FILE* fd = NULL;

inline DDword GetTickCount(void)
{
	struct timeval my_timeval;
	if (gettimeofday(&my_timeval, NULL))
		return 0;

	return ((DDword)my_timeval.tv_sec*NumOneThousand) + ((DDword)my_timeval.tv_usec / NumOneThousand);
}

int IsFileExist()
{
	FILE* fp = fopen(SHAREFILENAME, "r");
	if(fp == NULL)
	{
		reMapFile = 1;
		if((void*)-1 == iHandBuff && iHandBuff != NULL && mapLen != 0)
		{
			munmap(iHandBuff, mapLen);
			mapLen = 0;
			iHandBuff = NULL;
		}
	}
	else
	{
		fclose(fp);
		return -1;
	}

	return 0;
}

int MmapFile(long width, long height, VIDEOFORAMT fmt)
{
	alignedWidth = (width+ALIGNED_UP_16) &~ALIGNED_UP_16;
	alignedHeight = (height+ALIGNED_UP_16) &~ALIGNED_UP_16;
	long maxSize = alignedWidth*alignedHeight*6+sizeof(WIMOPREFIX);
	if(fmt == 0)
		sizeData = alignedWidth*alignedHeight*3/2;
	else
		sizeData = alignedWidth*alignedHeight*3;

	long size = maxSize > sizeData?maxSize:sizeData;
	
	int fd = open(SHAREFILENAME,O_CREAT|O_RDWR|O_TRUNC, 0777);
	
	memset(&prefix, 0 ,sizeof(WIMOPREFIX));
	prefix.header.fmt = fmt;
	prefix.header.width = width;
	prefix.header.height = height;
	prefix.stride = alignedWidth;
	prefix.index = 0;
	write(fd, &prefix, sizeof(prefix));
	mapLen = sizeData+sizeof(prefix);
	lseek(fd, size+1, SEEK_SET); // reserved memory space
	write(fd, " ", 1);

	iHandBuff = (unsigned char*)mmap(NULL, mapLen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if((void*)-1 == iHandBuff && iHandBuff != NULL)
	{
		LOGE("mmap framebuffer error\n");
		close(fd);
		return -1;
	}
	close(fd);	
	LOGI("mmap successfully!\n");
	
	return 0;
}

void CopyToFile(unsigned char* pBuff, long width, long height, VIDEOFORAMT format)
{
	if(iHandBuff != (void*)-1 && iHandBuff != 0)
	{
		if(currIndex >= MAXINTNUM)
			currIndex = 0;
		currIndex++;
		frames++;
		int i = 0;
		alignedWidth = (width+ALIGNED_UP_16) &~ALIGNED_UP_16;
		alignedHeight = height;
		prefix.header.fmt = format;
		prefix.header.width = alignedWidth;
		prefix.header.height = alignedHeight;
		prefix.index = currIndex;
		prefix.stride = alignedWidth;
		if(alignedWidth != width)
		{
			for(i=0; i<height*3/2; i++)
			{
				memcpy(iHandBuff+sizeof(prefix)+i*alignedWidth, pBuff+i*width, width);	
				if(i<=height)
					memset(iHandBuff+sizeof(prefix)+i*alignedWidth+width, 0x0, alignedWidth-width);
				else if(i>height && i<height*3/2)
					memset(iHandBuff+sizeof(prefix)+i*alignedWidth+width, 0x80, alignedWidth-width);
			}
		}
		else
			memcpy(iHandBuff+sizeof(prefix), pBuff, sizeData);	
		
		memcpy(iHandBuff, &prefix, sizeof(prefix));
	}
}

int  CaptureVideo(unsigned char* pBuffer, long width, long height,  VIDEOFORAMT format)
{
	if(pBuffer == 0 || width == 0|| height ==0 || format>=VIDEOFORAMT_MAX)
		return -1;

	if(IsFileExist() == 0 || reMapFile == 1)
	{
		reMapFile = 0;
		if(MmapFile(width, height, format) != 0)
		{
			LOGE("CaptureVideo mmap failed!\n");
			return -1;
		}
	}

//	LOGE("size:%ld,%ld,pBuffer: %p, iHandBuff:%p, currIndex:%d\n",width, height,pBuffer, iHandBuff,currIndex);
	if(GetTickCount() - sTime > 1000)
	{
		LOGI("obtain fps: %d\n", frames);
		sTime = GetTickCount();
		frames = 0;
	}
	CopyToFile(pBuffer, width, height, format);
	
	return 0;
}


