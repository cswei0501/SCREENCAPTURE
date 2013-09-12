#define LOG_NDEBUG 0
#define LOG_TAG "Screen_capture"

#include <utils/Log.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <SkImageEncoder.h>
#include <SkBitmap.h>
#include <SkStream.h>
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

#include "android_runtime/AndroidRuntime.h"
#include <linux/android_pmem.h>

#include "ci_codec_type.h"
#include "ci_imageproc.h"
#include "ci_jpegenc.h"
#include "capture_encoder.h"

#define NumOneThousand 1000

using namespace android;

unsigned char* iHndFpBuff = NULL;
unsigned char* JPEG_FileBuff = NULL;
int JPEG_Filesize = 0;
int iHndFB = 0;
int iHndFBSize = 0;
long SRC_WIDTH = 0;
long SRC_HEIGHT = 0;

struct fb_fix_screeninfo fbfinfo;
struct fb_var_screeninfo fbvinfo;

inline long GetTickCount(void)
{
	struct timeval my_timeval;
	if (gettimeofday(&my_timeval, NULL))
		return 0;
	return (long)(my_timeval.tv_sec * NumOneThousand) + (my_timeval.tv_usec / NumOneThousand);
}

int InitFrameBuff()
{
	char cDevNode[128];

	sprintf(cDevNode,"/dev/graphics/fb0");
	printf("test %s \n",cDevNode);
	iHndFB = open(cDevNode, O_RDONLY);

	if(iHndFB < 0)
	{
		printf("/dev/graphics/fb0 open error\n");
		return -1;
	}
	printf("/dev/graphics/fb0 open success\n");

	int ret = -1;
	ret = ioctl(iHndFB, FBIOGET_VSCREENINFO, &fbvinfo);
	if(ret)
	{
		printf("ioctl(..., FBIOGET_VSCREENINFO, ...) on framebuffer error %x\n", ret);
		return -1;
	}
	ret = ioctl(iHndFB, FBIOGET_FSCREENINFO, &fbfinfo);
	if(ret)
	{
		printf("ioctl(..., FBIOGET_FSCREENINFO, ...) on framebuffer error %x\n", ret);
		return -1;
	}

	iHndFBSize = fbvinfo.xres * fbvinfo.bits_per_pixel * fbvinfo.yres / 8;

	SRC_WIDTH = fbvinfo.xres;
	SRC_HEIGHT = fbvinfo.yres;

	printf("fbvinfo.xres:%d ,y:%d bye: %d\n",fbvinfo.xres,fbvinfo.yres, fbvinfo.bits_per_pixel);
	printf("red:%d gre:%d blue:%d \n",fbvinfo.red.offset ,fbvinfo.green.offset , fbvinfo.blue.offset);

	if(SRC_WIDTH == 0 || SRC_HEIGHT ==0 )
		return -1;
	
	iHndFpBuff = (unsigned char *)mmap(0, iHndFBSize, PROT_READ, MAP_SHARED, iHndFB, 0);
	if((void*)-1 == iHndFpBuff)
	{
		printf("mmap framebuffer error\n");
		return -1;
	}

	return 0;
}

void UnInitFrameBuff()
{
	if(iHndFB != 0)
	{
		close(iHndFB);
		iHndFB =0;
	}
	if(iHndFpBuff)
	{
		munmap(iHndFpBuff, iHndFBSize);
		iHndFpBuff = 0;
		iHndFBSize = 0;
	}
}

int InitImgInfoHelper(IMGINFO *pImgInfo, unsigned char **pDst)
{
	memset(pImgInfo ,0 ,sizeof(IMGINFO));
	
	pImgInfo->dstFormat = 0;
	pImgInfo->dstWidth = pImgInfo->srcWidth = SRC_WIDTH;
	pImgInfo->dstHeight = pImgInfo->srcHeight = SRC_HEIGHT;

	//IMAGE PROCESSING needs to do 16pixel alignment on dest buffer.
	if(pImgInfo->dstWidth % 16 != 0)
		pImgInfo->dstWidth = pImgInfo->dstWidth &~15;
	if(pImgInfo->dstHeight %16 != 0)
		pImgInfo->dstHeight = pImgInfo->dstHeight &~15;

	pImgInfo->pDst[0] = pDst[0];
	pImgInfo->pDst[1] = pDst[1];
	pImgInfo->pDst[2] = pDst[2];

	pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	pImgInfo->ImgFmtOut = CI_IMAGEPROC_FORMAT_YCBCR420;
	pImgInfo->interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;
	
	if((fbvinfo.red.offset == 0 && fbvinfo.green.offset == 8&& fbvinfo.blue.offset == 16)
		|| (fbvinfo.red.offset == 24 && fbvinfo.green.offset == 16 && fbvinfo.blue.offset == 8))
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ARGB32;
	else if(fbvinfo.red.offset == 11 && fbvinfo.green.offset == 5 && fbvinfo.blue.offset == 0)
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB565;
	else
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ABGR32;

	if(pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_RGB565)
		pImgInfo->srcStride[0] = pImgInfo->srcWidth*2;
	else if(pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_ABGR32 || pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_ARGB32)
		pImgInfo->srcStride[0] = pImgInfo->srcWidth*4;
	else
	{
		printf("invalid format!!!");
		return -1;
	}

	return 0;
}

/*****************************************************
* Func: RGB2YUV
* Function: process rgb and output yuv data
* Parameters: 
*	pImgInfo: set input and output information
*	srcSize: set input width and height
*	dstSize: set output width and height
*****************************************************/
int RGB2YUV(IMGINFO *pImgInfo, CI_IMAGEPROC_SIZE srcSize,CI_IMAGEPROC_SIZE dstSize)
{
	if(!pImgInfo)
	{
		printf("pImgInfo is empty in RGB2YUV()!\n");
		return -1;
	}

	CI_VOID* hImgPrc=NULL;
	CI_IMAGEPROC_CREATEOPTION  CreateOpt;
	CI_IMAGEPROC_PROCESSOPTION ProcessOpt;
	memset(&CreateOpt, 0,sizeof(CreateOpt));
	memset(&ProcessOpt, 0,sizeof(ProcessOpt));

	CreateOpt.u32ColorSpaceConversion	= pImgInfo->ImgFmtOut | pImgInfo->ImgFmtIn << 8;
	CreateOpt.u32Rotation				= pImgInfo->rotation;
	CreateOpt.u32Interpolation			= pImgInfo->interpolation;
	CreateOpt.u32SrcWidth				= srcSize.s32Width;
	CreateOpt.u32SrcHeight			= srcSize.s32Height;
	
	CreateOpt.u32DstWidth				= dstSize.s32Width;
	CreateOpt.u32DstHeight			= dstSize.s32Height;
	CreateOpt.u32Alpha				= 0xff;//0x80;

	ProcessOpt.u32Size				= sizeof(CI_IMAGEPROC_PROCESSOPTION);
	ProcessOpt.u32Interpolation			= pImgInfo->interpolation;
	int retimg = 0;
	retimg  = CI_IMAGEPROC_Create(&hImgPrc, NULL, &CreateOpt);
	if(retimg)
	{
		printf("imageproc create failed!!!!retimg  :%d \n",retimg);
		return -1;
	}

	retimg = CI_IMAGEPROC_ProcessFrame(hImgPrc, pImgInfo->pSrc, pImgInfo->srcStride, pImgInfo->pDst, pImgInfo->dstStride, &ProcessOpt);
	if(retimg)
	{
		printf("process frame failed!!!!retimg  :%x\n",retimg);
		return -1;
	}
	CI_IMAGEPROC_Destroy(hImgPrc);

	return 0;
}

/*****************************************************
* Func: YUV2JPG
* Function: convert yuv to jpeg
* Parameters: 
*	pImgInfo: set input image information
*	srcSize: the width and height of the YUV image
*****************************************************/
int YUV2JPG(IMGINFO *pImgInfo, CI_SIZE srcSize)
{
	if(!pImgInfo)
	{
		printf("pImgInfo is empty in YUV2JPG()!\n");
		return -1;
	}
	
	CI_JPGENCOPENOPTIONS OpenOptions;
	CI_VOID *pEncoder;
	CI_RESULT ret;
	int format = 3;

	OpenOptions.u8OutType = CI_OUT_JPEG;
	OpenOptions.u8YUVType = format == 3? CI_YUV420_PLANAR : format;
	OpenOptions.u8Quality = CI_QUALITY_LEVEL4;
	ret = CI_JPGENC_Open(&pEncoder, &OpenOptions);
	if(ret != CI_SOK)
	{
		printf("enc initial failed!\n");
		return -1;
	}
	ret = CI_JPGENC_Frame(pEncoder, pImgInfo->pDst, pImgInfo->dstStride, srcSize, (CI_U8*)JPEG_FileBuff, (CI_U32*)&JPEG_Filesize);
	if(ret != CI_SOK)
	{
		printf("enc enc failed!ret: %d\n", ret);
		return -1;
	}
	CI_JPGENC_Close(pEncoder);
	
	return 0;	
}

/*****************************************************
* Func: ProcessVideo
* Function: process video data
* Parameters: pImgInfo: image parameters
*****************************************************/
int ProcessVideo(IMGINFO *pImgInfo)
{
	if(pImgInfo == NULL)
		return -1;
	
	CI_IMAGEPROC_SIZE srcSize = {0, 0};
	CI_IMAGEPROC_SIZE dstSize = {0, 0};

	//alocation buffer and src buffer
	pImgInfo->pSrc[0] = (unsigned char*)iHndFpBuff;
	
	srcSize.s32Width = dstSize.s32Width = pImgInfo->srcWidth;
	srcSize.s32Height = dstSize.s32Height = pImgInfo->srcHeight;

	pImgInfo->srcStride[1] = pImgInfo->srcStride[2] = 0;

	pImgInfo->dstStride[0] = (dstSize.s32Width + 15)&~15;
	pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	pImgInfo->dstStride[1] = pImgInfo->dstStride[0]>>1;
	pImgInfo->dstStride[2] = pImgInfo->dstStride[0]>>1;
	
	if(RGB2YUV(pImgInfo,srcSize,dstSize) != 0)
	{
		printf("RGB2YUV enc frame failure!\n");
		return -1;
	}

	unsigned char* yuvDst = NULL;
	yuvDst = pImgInfo->pDst[1];
	pImgInfo->pDst[1] = pImgInfo->pDst[2];
	pImgInfo->pDst[2] = yuvDst;

	CI_SIZE yuvSrcSize = {0,0};

	pImgInfo->dstStride[0] = (dstSize.s32Width + 15)&~15;
	yuvSrcSize.width = pImgInfo->dstWidth;
	yuvSrcSize.height = pImgInfo->dstHeight;
	
	pImgInfo->dstStride[1] = pImgInfo->dstStride[0]>>1;
	pImgInfo->dstStride[2] = pImgInfo->dstStride[0]>>1;
	
	if(YUV2JPG(pImgInfo, yuvSrcSize) != 0)
	{
		printf("Jpeg enc frame failure!\n");
		return -1;
	}

	return 0;
}

void WriteToFile()
{
	FILE *fp = NULL;
	fp = fopen("/data/data/test.jpg","wb");
	fwrite(JPEG_FileBuff, 1, JPEG_Filesize, fp);
	fclose(fp);
}

void LoopVideo(void)
{
	unsigned char *pDst[3] = {0,0,0};
	IMGINFO ImgInfo;
	int frames = 0;
	long startTick = 0;
	long stopTick = 0;
	long processVideoTime = 0;
	
	int width = (SRC_WIDTH + 15)&~15;
	int height = (SRC_HEIGHT + 15)&~15;

	pDst[0] = (unsigned char *)malloc(width*height*4);	
	if(!pDst[0])
	{
		printf("failed malloc pDst[0] \n");
	}
	pDst[1] = pDst[0] + width*height;
	pDst[2] = pDst[1] + width*height/4;

	if(InitImgInfoHelper(&ImgInfo, pDst) != 0)
	{
		printf("failed init ImgInfo\n");
	}

	stopTick = startTick = GetTickCount();
	while(GetTickCount() - stopTick < 3*60*NumOneThousand)
	{
		if(ProcessVideo(&ImgInfo) != 0)
			break;

		//WriteToFile();
		
		frames++;
		if(GetTickCount() - startTick > NumOneThousand)
		{
			int currFps = 0;
			currFps = frames*NumOneThousand/(GetTickCount() - startTick);

			printf("fps :%d\n",currFps);

			startTick = GetTickCount();
			frames = 0;
		}

		//usleep(NumOneThousand);
	}
}

int main(int argc, char* argv[])
{
	JPEG_Filesize = 0;
	JPEG_FileBuff = (unsigned char *)malloc(1024*1024*4);
	if(!JPEG_FileBuff)
	{
		printf("failed malloc JPEG_FileBuff \n");
		return -1;
	}
	
	if(InitFrameBuff() != 0)
		printf("init frame buffer failed!\n");

	LoopVideo();

	UnInitFrameBuff();

	if(JPEG_FileBuff)
	{
		free(JPEG_FileBuff);
		JPEG_FileBuff = NULL;
	}
	printf("test capture and encoder successfully!\n");
	
	return 0;
}
