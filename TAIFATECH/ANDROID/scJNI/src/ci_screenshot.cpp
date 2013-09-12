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

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <binder/IServiceManager.h>
#include <ISurfaceComposer.h>
#include <string.h>

#include "capture_type.h"

#undef LOG_TAG
#define LOG_TAG "ci_screenshot"

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	ERROR_LIB = 0,
	ANDROID_BUILD_23 = 9,
	ANDROID_BUILD_40 = 14,
	ANDROID_BUILD_40_3 = 15,
	ANDROID_BUILD_41 = 16,
	ANDROID_BUILD_42 = 17,
}ANDROID_PLATFORM;

int WritePidToFile(char* pidFilePath)
{
	if(pidFilePath == NULL){
		fprintf(stderr, "pidFilePath is null!\n");
		return -1;
	}
	
	pid_t pid = -1;
	int fwd = -1;
	char buf[32] = {0};
	int res = 0;

	pid = getpid();
	//printf("writepidtofile pid: %d\n", pid);
	sprintf(buf, "%d\n", pid);
	fwd = open(pidFilePath,  O_RDWR|O_CREAT|O_APPEND, 0664);
	if(fwd == -1){
		fprintf(stderr, "open pidFilePath file failed!\n");
		return -1;
	}
	if((res = chmod(pidFilePath, 0664)) != 0)
	{	
		fprintf(stderr, "chmod pidFilePath res:%d errno:%d\n",res,errno);
		return -1;
	}
	write(fwd, buf, strlen(buf));
	close(fwd);

	return 0;
}

int WipePidFile(char pidFilePath[])
{
	if(pidFilePath == NULL){
		fprintf(stderr, "pidFilePath is null!\n");
		return -1;
	}

	int fwd = -1;
	fwd = open(pidFilePath,  O_RDWR|O_TRUNC, 0664);
	close(fwd);

	return 0;
}

int main(int argc, char** argv)
{
	char* fifoFile = argv[1];
	int versionCode = 0;//atoi(argv[2]);
	//char* pidFilePath = argv[2];
	
	sp<ISurfaceComposer> composer;
	sp<IMemoryHeap> heap;
	PixelFormat f;
	
	pthread_t pid = 0;
	struct sched_param schedRecv;
	pthread_attr_t attrRecv;

	uint32_t w,h;
	status_t err = -1;
	int res;
	int fifofd = 0;
	int quit = 0;
	CI_SCRSHOT_HEADER ci_header;	
	
	if(argc != 3){
		fprintf(stderr, "usage: %s path error\n", argv[0]);
		exit(0);
	}

	memset(&ci_header, 0, sizeof(ci_header));
	const String16 name("SurfaceFlinger");
	getService(name, &composer);
	sp<IBinder> display(composer->getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));

	//WritePidToFile(pidFilePath);

	fprintf(stderr, "version_codes: %d\n",versionCode);

	unlink(fifoFile);
	res = mkfifo(fifoFile, 0664);
	if(res !=0)
	{
		fprintf(stderr, "mkfifo res:%d errno:%d\n",res,errno);
		goto ERROR_QUIT;
	}
	if((res = chmod(fifoFile, 0664)) != 0)
	{	
		fprintf(stderr, "chmod res:%d errno:%d\n",res,errno);
		goto ERROR_QUIT;
	}
	
	fprintf(stderr, "open fifo...\n");
	fifofd = open(fifoFile, O_WRONLY|O_TRUNC);
	if (fifofd == -1) {
		fprintf(stderr, "open ciscreencap file failed!\n");
		goto ERROR_QUIT;
	}
	
	fprintf(stderr, "start capture screen...\n");
	while(true)
	{	
		err = composer->captureScreen(display, &heap, &w, &h, &f, 0, 0, 0, -1UL);
		if (err != NO_ERROR) {
			fprintf(stderr, "screen capture failed: %s\n", strerror(-err));
			exit(0);
		} 
		
		ci_header.width = w;
		ci_header.height = h;
		ci_header.fmt = (CI_PIXEL_FORMAT)f;
		if(f==CI_PIXEL_FORMAT_RGBA_8888 || f==CI_PIXEL_FORMAT_RGBX_8888 || f==CI_PIXEL_FORMAT_BGRA_8888)
			ci_header.size = w*h*4;
		else if(f==CI_PIXEL_FORMAT_RGB_565 || f==CI_PIXEL_FORMAT_RGBA_4444 || f==CI_PIXEL_FORMAT_RGBA_5551)
			ci_header.size = w*h*2;
		else if(f==CI_PIXEL_FORMAT_RGB_888)
			ci_header.size = w*h*3;
		
		if(heap != NULL && fifofd > 0){
			write(fifofd, &ci_header, sizeof(ci_header));
			write(fifofd, (void*)heap->getBase(), ci_header.size);
		}
	}

	fprintf(stderr, "wimo capture screen end\n");
	close(fifofd);

	return 0;

ERROR_QUIT:
	//WipePidFile(pidFilePath);
	if(fifofd != -1)
	{
		close(fifofd);
		fifofd = 0;
	}
	
	return -1;
}

#ifdef __cplusplus
}
#endif

