#ifndef ANDROID_HEADER_H
#define ANDROID_HEADER_H

typedef struct _IMGINFO
{
	 //0 :yuv420, 1:rgb565, 2:rgb, 3:argb
	int dstFormat;
	int srcWidth;
	int srcHeight;
	int dstWidth;
	int dstHeight;
	unsigned char *pSrc[3];
	unsigned char *pDst[3];
	CI_U32 srcStride[3];
	CI_U32 dstStride[3];
	CI_IMAGEPROC_ROTATION rotation;
	CI_IMAGEPROC_FORMAT ImgFmtIn;
	CI_IMAGEPROC_FORMAT ImgFmtOut;
	CI_IMAGEPROC_INTERPOLATION interpolation;
}IMGINFO;

int SetVideoRotate(int nRotate);
void* SockStreamProc(void* ptr);

#endif
