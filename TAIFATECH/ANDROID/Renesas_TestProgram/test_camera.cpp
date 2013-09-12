
#define LOG_NDEBUG 0
#define LOG_TAG "Screen_capture"

#include <utils/Log.h>
#include <surfaceflinger/ISurfaceComposer.h>

#include <arpa/inet.h>
#include <net/if_arp.h> 
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/sockios.h>
#include <linux/if.h> 
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <binder/Permission.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include <linux/android_pmem.h>

extern "C"{
#include <emxx_jpeglib.h>
#include <utility.h>
}

#define ENCODE_FILE_NAMES        			"/data/camenc420.yuv"
#define CAMERA_DEVICES					"/dev/v4l/video1"
#define CAMERA_TEMP_JPG_FILE_NAMES	"/data/camtemp.jpg"

#include <time.h> //-lrt
unsigned int timeGetTime()
{
        unsigned int uptime = 0;
        struct timespec on;
        if(clock_gettime(CLOCK_MONOTONIC, &on) == 0)
                 uptime = on.tv_sec*1000 + on.tv_nsec/1000000;
        return uptime;
}

int main(int ac, char* av[])
{
	unsigned char *buff = NULL;
	int file_size = 0, ret = 0;
	FILE *pp;
	FILE *outp;

	pp = fopen(ENCODE_FILE_NAMES, "rb");
	if(pp == NULL)
	{
		LOGE("ENCODE_FILE_NAMES open fail\n");
		return -1;
	}
	fseek(pp, 0, SEEK_END);
	file_size = ftell(pp);
	LOGI("file_size: %d\n", file_size);
	
	buff = (unsigned char*)malloc(sizeof(unsigned char)*(file_size+1));
	if(buff == NULL){
		LOGE("malloc fail!\n");
		return -1;
	}
	memset(buff, 0, sizeof(unsigned char)*(file_size+1));
	
	fseek(pp, 0, SEEK_SET);
	int cout = fread(buff, file_size, 1, pp);
	LOGI("cout: %d\n", cout);
	if(cout != 1){
		LOGE("fread fail\n");
		return -1;
	}
	fclose(pp);
	
	outp = fopen(CAMERA_TEMP_JPG_FILE_NAMES, "wb");
	if(outp == NULL)
	{
		LOGE("CAMERA_TEMP_JPG_FILE_NAMES open fail\n");
		return -1;
	}
	
	long tick = timeGetTime();
	for(int i=0;i<100;i++)
	{
		ret = emxx_yuv_to_jpeg(buff, 720, 480, outp, 85, OMF_MC_JPEGD_SAMPLING22);
		if (ret != 0) {
			LOGE("yuv file convert to jpeg file failed!!!!");
		}
	}
	LOGE("time :%ld \n",timeGetTime() - tick);

	fclose(outp);
	free(buff);
	
	return 0;
}
