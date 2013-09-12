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
// 	Copyright (c) 2010  Cidana, Inc.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
//
// This file is furnished as part of the Cidana Playback SDK. 
// Usage of this file, code, concepts, and/or algorithms is prohibited
// except under the terms of the Software Licensing Agreement with Cidana.
//

#ifndef _CI_IMAGEPROC_H_
#define _CI_IMAGEPROC_H_

#ifdef __cplusplus
extern "C" {
#endif

// {17A12BC8-DF53-4ac7-BACF-E854742427AE}
CI_DEFINE_GUID(IID_CI_IMAGEPROC_C, 
0x17a12bc8, 0xdf53, 0x4ac7, 0xba, 0xcf, 0xe8, 0x54, 0x74, 0x24, 0x27, 0xae);

// u32PropId for CI_IMAGEPROC_Set & CI_IMAGEPROC_Get
enum
{
	CI_IMAGEPROC_PROPID_COLOROPTION = 1,	// set/get color option(brightness, contrast and saturation), and the capability
	CI_IMAGEPROC_PROPID_COLORKEY,			// set/get alpha for argb/agbr, etc
	CI_IMAGEPROC_PROPID_COLORMATRIX,		// set/get color matrix
	CI_IMAGEPROC_PROPID_COLORRANGE,			// set/get YUV range
};

enum
{
	CI_IMAGEPROC_FORMAT_YCBCR420 = 0,
	CI_IMAGEPROC_FORMAT_YCBCR422 = 1,
	CI_IMAGEPROC_FORMAT_YCBCR444 = 2,
	CI_IMAGEPROC_FORMAT_RGB565 = 3,
	CI_IMAGEPROC_FORMAT_RGB888 = 4,
	CI_IMAGEPROC_FORMAT_ARGB32 = 5,
	CI_IMAGEPROC_FORMAT_YUY2 = 6,
	CI_IMAGEPROC_FORMAT_UYVY = 7,
	CI_IMAGEPROC_FORMAT_NV12 = 8,
	CI_IMAGEPROC_FORMAT_ABGR32 = 9,
	CI_IMAGEPROC_FORMAT_RGB555 = 10,
};

enum
{
	CI_IMAGEPROC_CSC_YCBCR420_TO_RGB565 = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_RGB565),
	CI_IMAGEPROC_CSC_YCBCR420_TO_ARGB32 = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_ARGB32),
	CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420 = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_YCBCR420),
	CI_IMAGEPROC_CSC_RGB565_TO_ARGB32 = (CI_IMAGEPROC_FORMAT_RGB565<<8 | CI_IMAGEPROC_FORMAT_ARGB32),
	CI_IMAGEPROC_CSC_YCBCR444_TO_RGB565 = (CI_IMAGEPROC_FORMAT_YCBCR444<<8 | CI_IMAGEPROC_FORMAT_RGB565),
	CI_IMAGEPROC_CSC_YCBCR444_TO_RGB888 = (CI_IMAGEPROC_FORMAT_YCBCR444<<8 | CI_IMAGEPROC_FORMAT_RGB888),
	CI_IMAGEPROC_CSC_YCBCR444_TO_ARGB32 = (CI_IMAGEPROC_FORMAT_YCBCR444<<8 | CI_IMAGEPROC_FORMAT_ARGB32),
	CI_IMAGEPROC_CSC_YCBCR420_TO_YUY2 = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_YUY2),
	CI_IMAGEPROC_CSC_YCBCR420_TO_UYVY = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_UYVY),
	CI_IMAGEPROC_CSC_RGB565_TO_RGB565 = (CI_IMAGEPROC_FORMAT_RGB565<<8 | CI_IMAGEPROC_FORMAT_RGB565),
	CI_IMAGEPROC_CSC_RGB888_TO_RGB565 = (CI_IMAGEPROC_FORMAT_RGB888<<8 | CI_IMAGEPROC_FORMAT_RGB565),
	CI_IMAGEPROC_CSC_YCBCR420_TO_RGB888 = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_RGB888),
	CI_IMAGEPROC_CSC_RGB888_TO_RGB888 = (CI_IMAGEPROC_FORMAT_RGB888<<8 | CI_IMAGEPROC_FORMAT_RGB888),
	CI_IMAGEPROC_CSC_YCBCR422_TO_RGB565 = (CI_IMAGEPROC_FORMAT_YCBCR422<<8 | CI_IMAGEPROC_FORMAT_RGB565),
	CI_IMAGEPROC_CSC_YCBCR420_TO_NV12 = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_NV12),
	CI_IMAGEPROC_CSC_ARGB32_TO_YCBCR420 = (CI_IMAGEPROC_FORMAT_ARGB32<<8 | CI_IMAGEPROC_FORMAT_YCBCR420),
	CI_IMAGEPROC_CSC_ABGR32_TO_YCBCR420 = (CI_IMAGEPROC_FORMAT_ABGR32<<8 | CI_IMAGEPROC_FORMAT_YCBCR420),
	CI_IMAGEPROC_CSC_RGB565_TO_YCBCR420 = (CI_IMAGEPROC_FORMAT_RGB565<<8 | CI_IMAGEPROC_FORMAT_YCBCR420),
	CI_IMAGEPROC_CSC_YCBCR420_TO_RGB555 = (CI_IMAGEPROC_FORMAT_YCBCR420<<8 | CI_IMAGEPROC_FORMAT_RGB555),
	CI_IMAGEPROC_CSC_NV12_TO_YCBCR420 = (CI_IMAGEPROC_FORMAT_NV12<<8 | CI_IMAGEPROC_FORMAT_YCBCR420),
};

enum
{
	CI_IMAGEPROC_ROTATION_DISABLE = 0,		// no rotation
	CI_IMAGEPROC_ROTATION_90L = 1,			// rotating by 90 degree anticlockwise
	CI_IMAGEPROC_ROTATION_90R = 2,			// rotating by 90 degree clockwise
	CI_IMAGEPROC_ROTATION_180 = 3,			// rotating by 180
	CI_IMAGEPROC_ROTATION_FLIP_H = 4,		// reflection horizontal
	CI_IMAGEPROC_ROTATION_FLIP_V = 5,		// reflection vertical
};

enum
{
	CI_IMAGEPROC_INTERPOLATION_NEAREST = 0,
	CI_IMAGEPROC_INTERPOLATION_LINEAR = 1,
	CI_IMAGEPROC_INTERPOLATION_MEDIAN = 2,
	CI_IMAGEPROC_INTERPOLATION_NEARLINEAR  = 3,
};

enum 
{
	CI_IMAGEPROC_DeInterlace_None = 0,
	CI_IMAGEPROC_DeInterlace_Fast,
	CI_IMAGEPROC_DeInterlace_Discard,
	CI_IMAGEPROC_DeInterlace_Bob,
	CI_IMAGEPROC_DeInterlace_Blend,
	CI_IMAGEPROC_DeInterlace_Mean,
	CI_IMAGEPROC_DeInterlace_Linear,
	CI_IMAGEPROC_DeInterlace_X,
};
enum
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// BT470BG / ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
	// SMPTE170M / ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
	//
	// H264 Matrix coefficients = 5 or 6
	//
	// KR = 0.299; KB = 0.114 
	CI_IMAGEPROC_CSC_MATRIX_0 = 0,
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// BT709 / ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
	//
	// H264 Matrix coefficients = 1
	//
	// KR = 0.2126; KB = 0.0722
	CI_IMAGEPROC_CSC_MATRIX_1,
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// SMPTE420M
	//
	// H264 Matrix coefficients = 7
	//
	// KR = KR = 0.212; KB = 0.087 
	CI_IMAGEPROC_CSC_MATRIX_2,
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// United States Federal Communications Commission Title 47 Code of Federal Regulations (2003) 73.682 (a) (20)
	//
	// H264 Matrix coefficients = 4
	//
	// KR = 0.30; KB = 0.11
	CI_IMAGEPROC_CSC_MATRIX_3,
};

enum
{
	CI_IMAGEPROC_CSC_FULL_RANGE_YCbCr = 0,
	CI_IMAGEPROC_CSC_FULL_RANGE_YUV,
};

typedef struct
{
	CI_S32 s32Width;
	CI_S32 s32Height;
} CI_IMAGEPROC_SIZE;

typedef struct
{
	CI_S32 s32Brightness;
	CI_S32 s32Contrast;
	CI_S32 s32Saturation;
} CI_IMAGEPROC_COLOROPTION;

typedef struct
{
	CI_U32 u32Size;					// size of this structure
	CI_U32 u32ColorSpaceConversion;
	CI_U32 u32Rotation;
	CI_U32 u32Interpolation;
	CI_U32 u32SrcWidth;
	CI_U32 u32SrcHeight;
	CI_U32 u32DstWidth;
	CI_U32 u32DstHeight;
	CI_U32 u32Alpha;				// 0 to 255 for alpha
	CI_U32 u32ColorKey;				// [(y<<16)|(u<<8)|(v<<0)]
	CI_U32 u32ColorMatrix;			// default = CI_IMAGEPROC_CSC_MATRIX_0;
	CI_U32 u32YUVRange;				// default = CI_IMAGEPROC_CSC_FULL_RANGE_YCbCr;
} CI_IMAGEPROC_CREATEOPTION;

typedef struct
{
	CI_U32 u32Size;					// size of this structure
	CI_U32 u32Interpolation;
	CI_U32 u32DeInterlace;
} CI_IMAGEPROC_PROCESSOPTION;

CI_RESULT CI_IMAGEPROC_Create(
	CI_VOID **pProcessor,
	CI_VOID *pLicense,
	CI_IMAGEPROC_CREATEOPTION *pOption);

typedef CI_RESULT (* LPFN_CI_IMAGEPROC_Create)(
	CI_VOID **pProcessor,
	CI_VOID *pLicense,
	CI_IMAGEPROC_CREATEOPTION *pOption);

CI_RESULT CI_IMAGEPROC_Destroy(
	CI_VOID *pProcessor);

typedef CI_RESULT (* LPFN_CI_IMAGEPROC_Destroy)(
	CI_VOID *pProcessor);

CI_RESULT CI_IMAGEPROC_ProcessFrame(
	CI_VOID *pProcessor,
	CI_U8 *pSrc[3],
	CI_U32 srcStride[3],
	CI_U8 *pDst[3],
	CI_U32 dstStride[3],
	CI_IMAGEPROC_PROCESSOPTION *pOption);

typedef CI_RESULT (* LPFN_CI_IMAGEPROC_ProcessFrame)(
	CI_VOID *pProcessor,
	CI_U8 *pSrc[3],
	CI_U32 srcStride[3],
	CI_U8 *pDst[3],
	CI_U32 dstStride[3],
	CI_IMAGEPROC_PROCESSOPTION *pOption);

CI_RESULT CI_IMAGEPROC_Reset(
	CI_VOID *pProcessor,
	IN CI_U32 u32Flags);

typedef CI_RESULT (* LPFN_CI_IMAGEPROC_Reset)(
	CI_VOID *pProcessor,
	IN CI_U32 u32Flags);

CI_RESULT CI_IMAGEPROC_Set(
	IN OUT CI_VOID *pProcessor,
	IN CI_U32 u32PropId,
	IN CONST CI_VOID *pData,
	IN CI_U32 u32DataLen);

typedef CI_RESULT (* LPFN_CI_IMAGEPROC_Set)(
	IN OUT CI_VOID *pProcessor,
	IN CI_U32 u32PropId,
	IN CONST CI_VOID *pData,
	IN CI_U32 u32DataLen);

CI_RESULT CI_IMAGEPROC_Get(
	IN OUT CI_VOID *pProcessor,
	IN CI_U32 u32PropId,
	OUT CI_VOID *pData,
	IN CI_U32 u32DataLen,
	OUT CI_U32 *pu32DataUsed);

typedef CI_RESULT (* LPFN_CI_IMAGEPROC_Get)(
	IN OUT CI_VOID *pProcessor,
	IN CI_U32 u32PropId,
	OUT CI_VOID *pData,
	IN CI_U32 u32DataLen,
	OUT CI_U32 *pu32DataUsed);

typedef struct
{
	LPFN_CI_IMAGEPROC_Create Create;
	LPFN_CI_IMAGEPROC_Destroy Destroy;
	LPFN_CI_IMAGEPROC_ProcessFrame ProcessFrame;
	LPFN_CI_IMAGEPROC_Reset Reset;
	LPFN_CI_IMAGEPROC_Get Get;
	LPFN_CI_IMAGEPROC_Set Set;
} CI_IMAGEPROC_FUNCTIONS;

#ifdef __cplusplus
}
#endif
#endif // _CI_IMAGEPROC_H_


