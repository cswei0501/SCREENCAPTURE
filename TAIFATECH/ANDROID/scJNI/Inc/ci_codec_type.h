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
//
// This file is furnished as part of the Cidana Codec SDK. 
// Usage of this file, code, concepts, and/or algorithms is prohibited
// except under the terms of the Software Licensing Agreement with Cidana.
//

#ifndef _CIDANA_CODEC_TYPE_H_
#define _CIDANA_CODEC_TYPE_H_

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef CI_BASIC_TYPES_DEFINED
#define CI_BASIC_TYPES_DEFINED
typedef unsigned char		CI_U8;
typedef unsigned short		CI_U16;
typedef unsigned int		CI_U32;
typedef unsigned long long	CI_U64;
typedef signed char			CI_S8;
typedef signed short		CI_S16;
typedef signed int			CI_S32;
typedef signed long long	CI_S64;
typedef float				CI_F32;
typedef double				CI_F64;
typedef int					CI_BOOL;
typedef void				CI_VOID;
typedef int					CI_RESULT;
#endif	//CI_BASIC_TYPES_DEFINED

#ifndef CI_DEFINE_GUID
	#ifndef GUID_DEFINED
		#define GUID_DEFINED
		#if defined(__midl)
		typedef struct {
			unsigned long  Data1;
			unsigned short Data2;
			unsigned short Data3;
			byte           Data4[ 8 ];
		} GUID;
		#else
		typedef struct _GUID {
			unsigned long  Data1;
			unsigned short Data2;
			unsigned short Data3;
			unsigned char  Data4[ 8 ];
		} GUID;
		#endif
	#endif
	#ifndef FAR
		#ifdef _WIN32
		#define FAR
		#else
		#define FAR
		#endif
	#endif
	#ifndef EXTERN_C
		#ifdef __cplusplus
		#define EXTERN_C    extern "C"
		#else
		#define EXTERN_C    extern
		#endif
	#endif
	#ifdef __GNUC__  /* gcc/g++ compiler (Linux, Symbian ARMI) does the equivalent of _declspec(selectany) */
		#define ATTRIBUTE_WEAK     __attribute__ ((weak))
		#define DECLSPEC_SELECTANY
	#elif defined(__ADS__)
		#define ATTRIBUTE_WEAK  
		#define DECLSPEC_SELECTANY 
	#else
		#define ATTRIBUTE_WEAK
		#if !defined(DECLSPEC_SELECTANY)
		#define DECLSPEC_SELECTANY _declspec(selectany)
		#endif
	#endif

	#define CI_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		 EXTERN_C DECLSPEC_SELECTANY const GUID FAR name ATTRIBUTE_WEAK \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	#ifndef _REFGUID_DEFINED
	#define _REFGUID_DEFINED
	#ifdef __cplusplus
	#define REFGUID const GUID &
	#else
	#define REFGUID const GUID * __MIDL_CONST
	#endif
	#endif
#endif //CI_DEFINE_GUID

#ifdef __GNUC__
CI_DEFINE_GUID(IID_IUnknown, 
	0x2cb59f1f, 0x85a5, 0x43ed, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif	//__GNUC__

#ifndef CI_BASIC_RESULTS_DEFINED
#define CI_BASIC_RESULTS_DEFINED
// These result follow HRESULT model, where success result>=0, error result<0
enum
{	// CI_RESULT return types
	CI_SOK					= 0x00000000,
	CI_SFALSE				= 0x00000001,
	CI_EPENDING				= 0x8000000A,
	CI_ENOTIMPL				= 0x80004001,
	CI_ENOINTERFACE			= 0x80004002,
	CI_EPOINTER				= 0x80004003,
	CI_EABORT				= 0x80004004,
	CI_EFAIL				= 0x80004005,
	CI_EHANDLE				= 0x80004006,
	CI_EUNEXPECTED			= 0x8000ffff,	// returned if decoder not initialized yet
	CI_EACCESSDENIED		= 0x80070005,
	CI_EOUTOFMEMORY			= 0x8007000E,
	CI_EINVALIDARG			= 0x80070057,
	CI_EPROPSETUNSUPPORTED	= 0x80070492,
	CI_EPROPIDUNSUPPORTED	= 0x80070490,
	CI_ELICENSE				= 0x80040FFF,
	CI_EINPBUFTOOSMALL		= 0x80041200,
	CI_EOUTBUFTOOSMALL		= 0x80041201,
	CI_EMODENOTSUPPORTED	= 0x80041202,
	CI_EBITSTREAM			= 0x80041203,
// module specific codes ¨C should start from 0x80040200 and will overlap between modules. 
// CI_MODULENAME_E(type) ==>  [0x80040200, 0x8004ffff]  (HRESULT  facility  ITF,  code range 0x0200-0xffff) 
// CI_MODULENAME_S(type) ==>  [0x00000200, 0x0000ffff]
};
#endif	//CI_BASIC_RESULTS_DEFINED

#ifndef CI_RESET_DEFINED
#define CI_RESET_DEFINED
enum
{	// Reset flags
	CI_RESET_CLEARSTATE			= (1<<0),
	CI_RESET_CLEARINPUTBUFFER	= (1<<1),
	CI_RESET_CLEAROUTPUTBUFFER	= (1<<2),
};
#endif //CI_RESET_DEFINED

enum
{	// Decode status
	CI_DECODE_FORMATCHANGE	= (1<<0),
	CI_DECODE_ERRORCONCEAL	= (1<<1),
	CI_DECODE_ERRORBLEND	= (1<<2),
};

enum
{
	CI_AUDEC_PROPID_BUFFERINFO = 1,
	CI_AUDEC_PROPID_CONFIGINFO = 2,
	CI_AUDEC_PROPID_STREAMINFO = 3,
	CI_AUDEC_PROPID_FILESUFFIXES = 4,	// common file suffixes
	CI_AUDEC_PROPID_STATISTICS = 5,		// decoding statistics
};

enum
{
	CI_AUDEC_BUFFERFLAG_VARIABLEOUTPUT = (1<<0),
};

// minimum Audio BufferInfo content
typedef struct
{
	CI_U32 u32Size;
	CI_U32 u32MinInputBuffer;
	CI_U32 u32MinOutputBuffer;
	CI_U32 u32MinInputDataForOpen;
	CI_U32 u32BufferFlag;
} CI_AUDEC_BUFFERINFO;

// minimum Audio StreamInfo content
#define CI_AUDEC_STREAMINFO_COMMON \
	CI_U32 u32Size;	\
	CI_U32 u32OutputSampleRate;	\
	CI_U32 u32OutputChannels; \
	CI_U32 u32OutputBitsPerSample; \
	CI_U32 u32MinInputSize;	\
	CI_U32 u32MaxInputSize;	\
	CI_U32 u32MinOutputSize; \
	CI_U32 u32MaxOutputSize; \
	CI_U32 u32SourceBitRate; \
	CI_U32 u32SourceChannels; \
	CI_U32 u32SourceBitPerSample; \
	CI_U32 u32SourceBitsPerSecond; \
	CI_U8 szCodecName[32];\
	CI_U8 szCodecFormat[32];\
	CI_U8 szCodecAdditional[192];

typedef struct
{
	CI_AUDEC_STREAMINFO_COMMON
} CI_AUDEC_STREAMINFO;

// Audio Statistics.  This is produced by the internal translation layer.
typedef struct
{
	CI_U32 u32Size;						// ** size of this structure
	CI_U32 u32EstimatedBitrate;			// ** if nonzero, the estimated bitrate in bits per second
	CI_U32 u32ErrorsSinceCreate;		// ** number of frame errors since creating decoder
	CI_U32 u32ErrorsSinceReset;			// ** number of frame errors since last Reset() called.
	CI_U64 u64InputBytesSinceCreate;	// ** number of input bytes consumed by Decode() since Create().  (discarded bytes by Reset() not included).
	CI_U64 u64InputBytesSinceReset;		// ** number of input bytes consumed by Decode() since last Reset() called.
	CI_U64 u64OutputBytesSinceCreate;	// ** number of output bytes produced by Decode() since Create().
	CI_U64 u64OutputBytesSinceReset;	// ** number of output bytes produced by Decode() since last Reset() called.
} CI_AUDEC_STATISTICS;

enum
{
	CI_AUDEC_DECODEFLAG_ENDOFSTREAM = (1<<0),
};

#define CI_AUDEC_DECODESIDEINFO_COMMON \
      CI_U32 u32Size; \
      CI_U32 u32DecodeFlag; \
      CI_U32 u32Reserved;

typedef struct
{
	CI_U8 u8Precision;
	CI_U8 u8Valid;          // this is essentially one bit
	CI_U8 u8Signed;         // this is essentially one bit
	CI_U8 u8Flags;          // TBD
	CI_U32 u32TimeScale;
	CI_U64 u64Timestamp;
} CI_DECODE_PTS;

/*
LPFN_CI_AUDEC_SetPts

This function is used to associate a PTS structure to a point
u32InPtsOffset (always equal or forward) ahead in the input
decoding buffer.  This PTS structure is queued by the Audio Decoder 
PTS manager and retrieved by LPFN_CI_AUDEC_GetPts when
the decoding crosses the input buffer position specified.

Parameters:
	u32InpPtsOffset: Number of bytes ahead in the input decode buffer
		with which to associate the PTS.
	pInPts: A CI_DECODE_PTS structure which is associated with the
		input buffer position.  
	
Return code:
	CI_SOK if successful.
	CI_EFAIL if unsuccessful.
	CI_EPOINTER if pInPts is 0.
*/
typedef CI_RESULT (* LPFN_CI_AUDEC_SetPts)(IN OUT CI_VOID *pDecoder,
	IN CI_U32 u32InPtsOffset,
	IN CI_DECODE_PTS *pInPts);

/*
LPFN_CI_AUDEC_GetPts

This function is used to retrieve the last PTS structure
which was associated to a buffer position crossed by Audio Decoding.
The PTS structures are associated with input buffer positions
through the LPFN_CI_AUDEC_SetPts function.

Parameters:
	pOutPts: A CI_DECODE_PTS structure which is associated with the
		input buffer position that was last crossed by the decoder.  
	bNext: If TRUE look for the next upcoming PTS, if FALSE look at the
		current decoded PTS.  Typically if bNext is TRUE then bFree is FALSE.
	bFree: If TRUE free the PTS from consideration again
		(generally set to TRUE when the PTS is recognized,
		FALSE if you just want to "peek" at the PTS). 
		If FALSE, then the same PTS can be read over and over again,
		until an new PTS is read in by the Audio Decoder or a GetPts()
		is called with TRUE.

Return code:
	CI_SOK if successful.
	CI_EFAIL if unsuccessful or no Pts found.
	CI_EPOINTER if pOutPts is 0.
*/
typedef CI_RESULT (* LPFN_CI_AUDEC_GetPts)(IN OUT CI_VOID *pDecoder,
	OUT CI_DECODE_PTS *pOutPts, IN CI_BOOL bNext, IN CI_BOOL bFree);

enum
{
	CI_VIDEC_LRD_FF = 0,
	CI_VIDEC_LRD_FH = 1,
	CI_VIDEC_LRD_HF = 2,
	CI_VIDEC_LRD_HH = 3,
};

enum
{
	CI_VIDEC_PROPID_BUFFERINFO = 1,
	CI_VIDEC_PROPID_CONFIGINFO = 2,			// not implented
	CI_VIDEC_PROPID_STREAMINFO = 3,			// 
	CI_VIDEC_PROPID_FILESUFFIXES = 4,		// common file suffixes
	CI_VIDEC_PROPID_STATISTICS = 5,			// decoding statistics
	CI_VIDEC_PROPID_CSCMATRIXCOEF = 6,	// get the matrix coefficient of conversion RGB domain
	CI_VIDEC_PROPID_SKIPTOI = 0xD0000001,	//
	CI_VIDEC_PROPID_TEMPORALSCALING,		// IFRAMEONLY IPFRAMEONLY ALLFRAMES
	CI_VIDEC_PROPID_SPATIALSCALING,			// FF, FH, HF, HH
};

typedef CI_VOID (* LPFN_CI_CreateLock)(CI_VOID **ppLock);
typedef CI_VOID (* LPFN_CI_DestroyLock)(CI_VOID **ppLock);
typedef CI_VOID (* LPFN_CI_Lock)(CI_VOID *pLock);
typedef CI_VOID (* LPFN_CI_Unlock)(CI_VOID *pLock);

typedef struct 
{	//call back functions for sharing output frame with caller
	LPFN_CI_CreateLock pfnCreateLock;		//create critical section object
	LPFN_CI_DestroyLock pfnDestroyLock;		//destroy critical section object
	LPFN_CI_Lock pfnLock;					//lock critical section object
	LPFN_CI_Unlock pfnUnlock;				//unlock critical section object
} CI_DECODE_LOCKFUNCTIONS;

enum
{
      CI_VIDEC_SKIP_FRAMETYPE_SKIPPABLE = 1,
      CI_VIDEC_SKIP_FRAMETYPE_PREDICTION = 2,
      CI_VIDEC_SKIP_FRAMETYPE_REFERENCE = 3,
};

typedef CI_RESULT (* LPFN_CI_CheckSkippedFrame)(
	IN CI_VOID *pContext,
	IN CI_U32 u32FrameType,
	IN CI_DECODE_PTS *pPts,
	OUT CI_U32 *pu32Skipped,
	OUT CI_U32 *pu32PictureQuality);
/*
LPFN_CI_CheckSkippedFrame

Application supplied callback function provided as a mechanism 
to skip decoding of frames. The decoder will call this function
after parsing the frame type in the bitstream but before the
actual decoding of the frame.  

The u32FrameType variable is set with a value of:

CI_VIDEC_SKIP_FRAMETYPE_SKIPPABLE:  can skip the frame without consequences
	to other frames. This is typically a "B" frame.
CI_VIDEC_SKIP_FRAMETYPE_PREDICTION: can skip the frame, but subsequent frames
	which use it for prediction will also be skipped. This is typically a
	"P" frame.
CI_VIDEC_SKIP_FRAMETYPE_REFERENCE:  a key (intra) reference frame - skip this
	and the decoder will seek to the next reference frame.  This is typically
	an "I" frame.

	if 0, the frame type is unknown.

The pPts variable, if nonzero, points to a PTS structure with
the information regarding the PTS of that frame.  This may be
a zero pointer if the PTS is unknown.

The application can set pu32Skipped to 1 in the callback to 
skip the frame.  There may be side-effects of skipping a frame that is not an 
unreferenced B frame (subsequent frames may be dropped until the next I frame). 
This function pointer is an optional argument to OpenOptions
and may be 0 if all frames are to be decoded.

Parameters:
	pContext: the context passed in by OpenOptions.
	u32FrameType: CI_VIDEC_SKIP_FRAMETYPE.
	pPts: if nonzero, the PTS of the frame.
	pu32Skipped: Set to 1 to skip frame.  Ignore or leave at 0 to process frame.

Return code: 
	Return CI_SOK on success.
	Return CI_EFAIL to ignore callback (not preferred)
*/

typedef CI_RESULT (* LPFN_CI_GetData)(
	IN CI_VOID *pContext,
	OUT CI_U8 *pOutBuffer,
	IN CI_S32 s32BufLen,
	OUT CI_S32 *ps32ReadBytes,
	OUT CI_DECODE_PTS *pPts);
/*
LPFN_CI_GetData

Application supplied callback function to supply data to the decoder in push mode DecodeFrame(). 
The application should fills the pOutBuffer with data (up to s32BufLen) when it is called.

Parameters:
	pContext: the context passed in by DecodeOptions.
	pOutputBuffer: Callback should fill data into this buffer.
	s32BufLen: maximum length to be stored in buffer.
	ps32ReadBytes: Callback should set this CI_S32 value to the number of bytes read. 0 is allowed.
	pPts: Fill with a valid PTS for the first access unit contained in pOutBuffer.  Ignore if no PTS in buffer.

Return code: 
	Return CI_SOK if stream is still alive (even if ps32ReadBytes are 0).
	Return CI_EFAIL if the stream has ended.
*/


//--------------------------------------------------------
//	CI_U32 u32SourceBitsPerSecond;	// * if nonzero, the number of bits per second
//	CI_U8 szCodecName[32];			// * Example "MPEG2" "MPEG4" "H264" ...
//	CI_U8 szCodecFormat[32];		// * Example "1080p24"  "480i60"  
//	CI_U8 szCodecAdditional[192];	// * Example "ASP", "HP", "MP", "SVC"
#define CI_VIDEC_STREAMINFO_COMMON \
	CI_U32 u32Size; \
	CI_U32 u32Width; \
	CI_U32 u32Height; \
	CI_U32 u32FrameType; \
	CI_U32 u32FrameRateNumerator; \
	CI_U32 u32FrameRateDenominator; \
	CI_U32 u32SourceBitsPerSecond;\
	CI_U8 szCodecName[32];\
	CI_U8 szCodecFormat[32];\
	CI_U8 szCodecAdditional[192];

typedef struct
{
	CI_VIDEC_STREAMINFO_COMMON
}	CI_VIDEC_STREAMINFO;

//----------------------------------------------------
// u32MinFrames, Minimum frame number needed for decoding
// u32MinInputBuffer, Minimum input buffer size for decoding, (K bytes)
// u32DisplayFramesAvailable, Ready to display frame numbers (0 no frame can be displayed)
// u32FreeFrameAvailable, Available free frame numbers (0 can not decode - wait for frame)
// u32BytesInInputBuffer, Byte numbers of internal decode buffer.
#define CI_VIDEC_BUFFERINFO_COMMON \
	CI_U32 u32Size; \
	CI_U32 u32MinFrames; \
	CI_U32 u32MinInputBuffer; \
	CI_U32 u32DisplayFramesAvailable; \
	CI_U32 u32FreeFramesAvailable; \
	CI_U32 u32BytesInInputBuffer;

typedef struct
{
	CI_VIDEC_BUFFERINFO_COMMON
}	CI_VIDEC_BUFFERINFO;

//---------------------------------------------------------------
// CI_DECODE_QUALITY_0, standard compliant mode
// CI_DECODE_QUALITY_1 _2 _3, is not compliant to standard, but the difference is not visible
// CI_DECODE_QUALITY_4 _5 _6, is not compliant to standard, the quality is worse than 1, 2, 3 level.
// CI_DECODE_QUALITY_7 _8 _9, is not compliant to standard, the worst case.
enum
{
	CI_DECODE_QUALITY_0 = 0,
	CI_DECODE_QUALITY_1 = 1,
	CI_DECODE_QUALITY_2 = 2,
	CI_DECODE_QUALITY_3 = 3,
	CI_DECODE_QUALITY_4 = 4,
	CI_DECODE_QUALITY_5 = 5,
	CI_DECODE_QUALITY_6 = 6,
	CI_DECODE_QUALITY_7 = 7,
	CI_DECODE_QUALITY_8 = 8,
	CI_DECODE_QUALITY_9 = 9,
};

enum 
{
	CI_VIDEC_DECODEFLAG_ENDOFSTREAM = (1<<0),	// for H.264 indicates last buffer. Can be delivered with empty data.
};

#define CI_VIDEC_DECODEOPTIONS_COMMON \
	CI_U32 u32Size; \
	CI_DECODE_PTS Pts; \
	CI_VOID *pGetDataContext; \
	LPFN_CI_GetData pfnGetData; \
	CI_U32 u32DecodeFlag; \
	CI_U32 u32LRDFlag;
//------------------------------------------------------------
// u32Size, structure size
// u32DisplayQueueSize, output frame queue size.
// DispQueue, control the critical section object (display frame) access 
// pSkipContext, context for checking skip B frame
// pfnCheckSkippedFrame, check whether current frame should be skipped
// u32LRDFlag, the partical decode mode

#define CI_VIDEC_OPENOPTIONS_COMMON \
	CI_U32 u32Size; \
	CI_U32 u32DisplayQueueSize; \
	CI_DECODE_LOCKFUNCTIONS DispQueue;	\
	CI_VOID *pSkipContext;	\
	LPFN_CI_CheckSkippedFrame pfnCheckSkippedFrame;	\
	CI_U32 u32LRDFlag;

// Video Statistics.  This is produced by the internal translation layer.
typedef struct
{
	CI_U32 u32Size;						// ** size of this structure
	CI_U32 u32EstimatedBitrate;			// ** if nonzero, the estimated bitrate in bits per second
	CI_U32 u32ErrorsSinceOpen;			// ** number of frame errors since creating decoder
	CI_U32 u32ErrorsSinceReset;			// ** number of frame errors since last Reset() called.
	CI_U64 u64InputBytesSinceOpen;		// ** number of input bytes consumed by Decode() since Open().  (discarded bytes by Reset() not included).
	CI_U64 u64InputBytesSinceReset;		// ** number of input bytes consumed by Decode() since last Reset() called.
	CI_U32 u32OutputFramesSinceOpen;	// ** number of output frames produced by Decode() since Open().
	CI_U32 u32OutputFramesSinceReset;	// ** number of output frames produced by Decode() since last Reset() called.
	CI_U32 u32SkippedFramesSinceOpen;	// ** number of output frames produced by Decode() since Open().
	CI_U32 u32SkippedFramesSinceReset;	// ** number of output frames produced by Decode() since last Reset() called.
} CI_VIDEC_STATISTICS;

#endif //_CIDANA_CODEC_TYPE_H_
