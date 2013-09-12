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

#ifndef _CIDANA_CODEC_TYPE_H_
#define _CIDANA_CODEC_TYPE_H_

#define IN
#define OUT
#define CONST const

//----------------------------------------------
//basic types
typedef unsigned char		CI_U8;
typedef unsigned short		CI_U16;
typedef unsigned int		CI_U32;
typedef unsigned long long	CI_U64;
typedef char				CI_S8;
typedef short				CI_S16;
typedef int					CI_S32;
typedef long long			CI_S64;
typedef float				CI_F32;
typedef double				CI_F64;
typedef int					CI_BOOL;
typedef void				CI_VOID;
typedef int					CI_RESULT;

//---------------------------------------------
//return codes

//successful type>=0, error type<0
typedef enum
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
} CI_RETURN;

// module specific codes ¨C should start from 0x80040200 and will overlap between modules. 
// CI_MODULENAME_E(type) ==>  [0x80040200, 0x8004ffff]  (HRESULT  facility  ITF,  code range 0x0200-0xffff) 
// CI_MODULENAME_S(type) ==>  [0x00000200, 0x0000ffff]

typedef enum
{	// Reset flags
	CI_RESET_CLEARSTATE			= (1<<0),
	CI_RESET_CLEARINPUTBUFFER	= (1<<1),
	CI_RESET_CLEAROUTPUTBUFFER	= (1<<2),
} CI_RESET_FLAGS;

typedef enum
{	// Decode status
	CI_DECODE_FORMATCHANGE	= (1<<0),
	CI_DECODE_ERRORCONCEAL	= (1<<1),
	CI_DECODE_ERRORBLEND	= (1<<2),
}CI_DECODE_STATUS;

typedef struct 
{
	CI_U8 u8Precision;
	CI_U8 u8Valid;          // this is essentially one bit
	CI_U8 u8Signed;         // this is essentially one bit
	CI_U8 u8Flags;          // TBD
	CI_U32 u32TimeScale;
	CI_U64 u64Timestamp;
} CI_VDec_PTS;

/************************************************************************
 * Inline function declaration
 ************************************************************************/
#if defined(__cplusplus)
#	define CI_INLINE inline
#elif defined(_MSC_VER)
#	define CI_INLINE static __forceinline
#elif defined(__GNUC__)
#	define CI_INLINE static __inline__
#elif defined(__arm) || defined(__ICC) || defined(__ICL) || defined(__ECL)
#	define CI_INLINE static __inline
#else
#	define CI_INLINE static
#endif


/************************************************************************
 * Function call declaration
 ************************************************************************/
#if defined(_WIN32) || defined(_WIN64)
#	define __STDCALL  __stdcall
#	define __CDECL    __cdecl
#else
#	define __STDCALL
#	define __CDECL
#endif


/************************************************************************
* Get current time in milliseconds
* CI_GETTIME	-- Enable/Disable this function
************************************************************************/
#ifndef CI_GETTIME
#define CI_GETTIME
#endif

#ifndef CI_GETTIME

	/* non-defined branch, return ZERO */
	CI_INLINE CI32u __CDECL CIGetTime() { return 0; }

#else	/* CI_GETTIME */

#	if defined(_WIN32_WCE) || defined(_WIN32) || defined(_WIN64)
#		include <Windows.h>
#	elif defined(__GNUC__)
#		include <sys/time.h>
#	endif

	CI_INLINE CI_U32 __CDECL CIGetTime()
	{
#	if defined(_WIN32_WCE) || defined(_WIN32) || defined(_WIN64)

		return GetTickCount();

#	elif defined(__GNUC__)

		struct timeval tv;
		gettimeofday(&tv, 0);	/* Get system time */
		return (tv.tv_sec * 1000 + tv.tv_usec / 1000);

#	else	/* non-defined branch, return ZERO */

		return 0;

#	endif
	}

#endif	/* CI_GETTIME */

/************************************************************************
 * Alignment declaration
 ************************************************************************/
#if defined(_WIN32_WCE)||defined(WIN32)
#define CI_ALIGN32 _declspec(align(32))
#define CI_ALIGN16 _declspec(align(16))
#define CI_ALIGN8 _declspec(align(8))
#elif defined(__linux__)||defined(__GNUC__)
#define CI_ALIGN32 __attribute__((aligned(32)))
#define CI_ALIGN16 __attribute__((aligned(16)))
#define CI_ALIGN8 __attribute__((aligned(8)))
#else
#define CI_ALIGN32 
#define CI_ALIGN16
#define CI_ALIGN8
#endif


#endif

