//=============================================================================
//  CIDANA CONFIDENTIAL INFORMATION
//
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO CIDANA, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2009  Cidana, Inc.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef __CI_JPEGENC_H__
#define __CI_JPEGENC_H__

#include "ci_codec_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	CI_U32 width;
	CI_U32 height;
}CI_SIZE;

// Input format of encoder
#define CI_YUV420_PLANAR	0x00000000
#define CI_YUV422_PLANAR	0x00000001

// Output format of encoder
#define CI_OUT_JPEG		0x00000000
#define CI_OUT_MJPEG	0x00000001

// Encode quality level 0 is lowest, so the size is smaller.
// Quality level 1 is better than level 0, and the size is bigger than level 0
// Quality evel 2 is better than level 1, the size is bigger than level 1 ,and so on.
#define CI_QUALITY_LEVEL0   0x00000000
#define CI_QUALITY_LEVEL1   0x00000001
#define CI_QUALITY_LEVEL2   0x00000002
#define CI_QUALITY_LEVEL3   0x00000003
#define CI_QUALITY_LEVEL4   0x00000004

typedef struct{
	CI_U8 u8YUVType;		//one of CI_YUV
	CI_U8 u8OutType;		//one of CI_OUT
	CI_U8 u8Quality;		//one of CI_QUALITY
}CI_JPGENCOPENOPTIONS;


/*===============================================================================
Function:
	CI_JPGENC_Open()

Description:
	Open and allocates resource for jpeg encoder handler according to the open options

Parameters:
	ppHandle: a pointer to receive the encoder handle.
	pOpenOptions: the options information. 

Return code:
	CI_SOK on success.
	CI_EINVALIDARG if destination YUV type is not YUV420 or YUV422.
	CI_EOUTOFMEMORY if no memory is available.
===============================================================================*/
CI_RESULT CI_JPGENC_Open(CI_VOID **ppHandle, CI_JPGENCOPENOPTIONS *pOpenOptions);

/* ===============================================================================
Function:
	CI_JPGENC_Frame()

Description:
	Encode the input YUV data into jpeg image

Parameters:
	pHandle:	the encoder handle.
	pSrc[]:		the source data address of input YUV image. Y, U and V plane in pSrc[0], pSrc[1] and pSrc[2] respectively. 
	srcStride[]:	the data stride of Y, U and V plane.
	srcSize:	the input YUV image size.
	pDst:		the destination data address of output JPEG image.
	pDstSize:	the file length in bytes of output JPEG image file.

Return code:
	CI_SOK on success.
	CI_EINVALIDARG if there's incorrect encoder handler, data address or size information.
===============================================================================*/
CI_RESULT CI_JPGENC_Frame(CI_VOID *pHandle, CI_U8 *pSrc[3], CI_U32 srcStride[3], CI_SIZE srcSize, CI_U8 *pDst, CI_U32 *pDstSize);

/* ===============================================================================
Function:
	CI_JPGENC_Close()

Description:
	Close and deallocates resource for jpeg encoder handler

Parameters:
	pHandle:	the encoder handle.

Return code:
	CI_SOK on success.
===============================================================================*/
CI_RESULT CI_JPGENC_Close(CI_VOID *pHandle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif//_IVI_JPEGENC_H_