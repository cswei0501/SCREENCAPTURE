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


typedef enum
{
	CI_IMAGEPROC_PROPID_COLOROPTION = 1,
} CI_IMAGEPROC_PROPID;

typedef enum
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
} CI_IMAGEPROC_FORMAT;

typedef enum
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
} CI_IMAGEPROC_CSC;

typedef enum
{
	CI_IMAGEPROC_ROTATION_DISABLE = 0,		// no rotation
	CI_IMAGEPROC_ROTATION_90L = 1,			// rotating by 90 degree anticlockwise
	CI_IMAGEPROC_ROTATION_90R = 2,			// rotating by 90 degree clockwise
	CI_IMAGEPROC_ROTATION_180 = 3,			// rotating by 180
	CI_IMAGEPROC_ROTATION_FLIP_H = 4,		// reflection horizontal
	CI_IMAGEPROC_ROTATION_FLIP_V = 5,		// reflection vertical
} CI_IMAGEPROC_ROTATION;

typedef enum
{
	CI_IMAGEPROC_INTERPOLATION_NEAREST = 0,
	CI_IMAGEPROC_INTERPOLATION_LINEAR = 1,
	CI_IMAGEPROC_INTERPOLATION_MEDIAN = 2,
	CI_IMAGEPROC_INTERPOLATION_NEARLINEAR  = 3,
} CI_IMAGEPROC_INTERPOLATION;

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
	CI_U32 u32Alpha;				// 0 to 255
} CI_IMAGEPROC_CREATEOPTION;

typedef struct
{
	CI_U32 u32Size;					// size of this structure
	CI_IMAGEPROC_INTERPOLATION u32Interpolation;
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


