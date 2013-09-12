#ifndef __CAPTURE_ENCODER_H__
#define __CAPTURE_ENCODER_H__

typedef struct _IMGINFO
{
	int 								dstFormat; 			//0 :yuv420, 1:rgb565, 2:rgb, 3:argb
	int 								srcWidth;
	int 								srcHeight;
	int 								dstWidth;
	int 								dstHeight;
	unsigned char 					*pSrc[3];
	unsigned char 					*pDst[3];
	CI_U32 							srcStride[3];
	CI_U32 							dstStride[3];
	CI_IMAGEPROC_ROTATION 			rotation;
	CI_IMAGEPROC_FORMAT 			ImgFmtIn;
	CI_IMAGEPROC_FORMAT 			ImgFmtOut;
	CI_IMAGEPROC_INTERPOLATION		interpolation;
}IMGINFO;

#endif
