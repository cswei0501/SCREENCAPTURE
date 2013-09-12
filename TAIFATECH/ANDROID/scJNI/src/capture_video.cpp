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

#include <dlfcn.h> 
#include <cpu-features.h>
#include "capture_video.h"

using namespace android;

/*****************************************************
* Func: CWiMoVideo
* Function: construction function
* Parameters: void
*****************************************************/
CWiMoVideo::CWiMoVideo(CWiMoControl* pis, char* libPath)
{
	m_cControl = pis;
	m_pAppLibPath = libPath;
	//LOGI("CWiMoVideo lib path : %s\n", m_pAppLibPath);
	
	Init();
}

/*****************************************************
* Func: ~CWiMoVideo
* Function: deconstruction function
* Parameters: void
*****************************************************/
CWiMoVideo::~CWiMoVideo()
{
	UnInit();
}

/*****************************************************
* Func: Init
* Function: init parameters
* Parameters: void
*****************************************************/
void CWiMoVideo::Init()
{
	m_iAlignedWidth = 0;
	m_iAlignedHeight = 0;
	m_gapValueW = 0;
	m_gapValueH = 0;
	m_offsetSize = 0;
	m_mJpegEnc = NULL;
	m_iHndFpBuff = NULL;
	m_screenshotBuff = NULL;
	m_lnvDev = NULL;
	m_JPEG_FileBuff1 = NULL;
	m_JPEG_FileBuff2 = NULL;
	m_iHndFD  = 0;
	m_iFirstImageSize = 0;
	m_iMmapSize = 0;
	m_sStride = 0;
	m_averageCostTimeEveryFrame = 0;
	m_sumOverTimeOfVideoProcess = 0;
	m_currUpdate = 0;
	m_prevUpdate = m_currUpdate - 1;
	m_pZTEmap = NULL;
	m_pMapSize = 0;
	m_bWriteRead = false;
	m_bSyncCapEncZTE = false;
	m_bSendBuffFlag1 = false;
	m_bSendBuffFlag2 = false;
	m_ddTimeStampVid = 0;
	m_bNeon = false;
	m_bArmv7orUP = false;
	m_bRevertUVPlanes = true;
	m_bOffset = false;
	m_shotFD = 0;

	m_pdl = NULL;
	m_imagePdl = NULL;
	m_encodePdl = NULL;
	m_rotate_I = NULL;
	m_getScreenDimemsion_pIpI = NULL;
	m_readJpegData_R = NULL;

	m_CI_IMGPROC_Create = NULL;
	m_CI_IMGPROC_ProcessFrame = NULL;
	m_CI_IMGPROC_Destroy = NULL;

	m_CI_JpgEnc_Open = NULL;
	m_CI_JpgEnc_Frame = NULL;
	m_CI_JpgEnc_Close = NULL;
	
	memset(&m_fbfinfo, 0, sizeof(fb_fix_screeninfo));
	memset(&m_fbvinfo, 0, sizeof(fb_var_screeninfo));
	memset(&m_sshotHeader, 0, sizeof(CI_SCRSHOT_HEADER));
	
	m_JPEG_Filesize1 = 0;
	m_JPEG_Filesize2 = 0;

	if(m_cControl->GetVideoMethod() != METHOD_LNV_HW_INTERFACE)
	{
		m_JPEG_FileBuff1 = (char *)malloc(JPEGBUFFERSIZE);
		if(!m_JPEG_FileBuff1)
		{
			LOGE("failed malloc m_JPEG_FileBuff1 \n");
			goto ERROR_QUIT;
		}
	}
	m_JPEG_FileBuff2 = (char *)malloc(JPEGBUFFERSIZE);
	if(!m_JPEG_FileBuff2)
	{
		LOGE("failed malloc m_JPEG_FileBuff2 \n");
		goto ERROR_QUIT;
	}

	m_pdl = m_cControl->GetLibHandle();
	if(m_cControl->GetVideoMethod() == METHOD_HS_HW_INTERFACE && m_pdl != NULL)
	{
		LOGI("entry HS HW video capture!\n");
		if(HSInit() != S_OK)
		{
			LOGE("failed to HSInit()!\n");
			goto ERROR_QUIT;
		}
	}
	else if(m_cControl->GetVideoMethod() == METHOD_LNV_HW_INTERFACE)
	{
		LOGI("entry Lenovo HW video capture!\n");
		CreateThreadHelper(LNVInitInThread, this);
	}
	else if(m_cControl->GetVideoMethod() == METHOD_ZTE_HW_INTERFACE)
	{
		LOGI("entry ZTE HW video capture!\n");
		if(ZTEInit() != S_OK)
		{
			LOGE("failed to ZTEInit()!\n");
			goto ERROR_QUIT;
		}
	}
	else if(m_cControl->GetVideoMethod() == METHOD_GAME_INTERFACE)
	{
		LOGI("entry GAME video capture!\n");
		if(QiYiGameInit() != S_OK)
		{
			LOGE("failed to GameInit()!\n");
			goto ERROR_QUIT;
		}
	}
	else if(m_cControl->GetVideoMethod() == METHOD_FRAMEBUFFER_INTERFACE)
	{
		LOGI("entry FRAMEBUFFER video capture!\n");
		if(ReadFB0Init() != S_OK)
		{
			LOGE("failed to ReadFB0Init()!\n");
			goto ERROR_QUIT;
		}
	}
	else 	if(m_cControl->GetVideoMethod() == METHOD_SCREENSHOT_INTERFACE)
	{
		LOGI("entry SCREENSHOT video capture!\n");
		CreateThreadHelper(InitScreenshotThread, this);
	}
	
	return;
	
ERROR_QUIT:
	m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);
}

/*****************************************************
* Func: UnInit
* Function: UnInit parameters
* Parameters: void
*****************************************************/
void CWiMoVideo::UnInit()
{
	if(m_cControl->GetVideoMethod() == METHOD_HS_HW_INTERFACE)
	{
		LOGI("quit HS HW video capture!\n");
	}
	else if(m_cControl->GetVideoMethod() == METHOD_LNV_HW_INTERFACE)
	{
		LOGI("quit LNV HW video capture!\n");
		LNVUnInit();
	}
	else if(m_cControl->GetVideoMethod() == METHOD_ZTE_HW_INTERFACE)
	{
		LOGI("quit ZTE HW video capture!\n");
		ZTEUnInit();
	}
	else if(m_cControl->GetVideoMethod() == METHOD_GAME_INTERFACE)
	{
		LOGI("quit Game video capture!\n");
		QiYiGameUnInit();
	}
	else	if(m_cControl->GetVideoMethod() == METHOD_FRAMEBUFFER_INTERFACE)
	{
		LOGI("quit framebuffer video capture!\n");
		ReadFB0UnInit();	
	}
	else 	if(m_cControl->GetVideoMethod() == METHOD_SCREENSHOT_INTERFACE)
	{
		LOGI("quit framebuffer video capture!\n");	
		ReadScreenshotUnInit();
	}

	if(m_JPEG_FileBuff2)
	{
		free(m_JPEG_FileBuff2);
		m_JPEG_FileBuff2 = NULL;
	}	
	if(m_JPEG_FileBuff1 && m_cControl->GetVideoMethod() != METHOD_LNV_HW_INTERFACE)
	{
		free(m_JPEG_FileBuff1);
		m_JPEG_FileBuff1 = NULL;
	}
}

/*****************************************************
* Func: GetJPEGData
* Function: get jpeg data
* Parameters: 
*	jpegSize: (out)jpeg size
*	return: jpeg data address
*****************************************************/
char* CWiMoVideo::GetJPEGData(int *jpegSize)
{
	char* tmpBuf = NULL;

	if(m_cControl->GetVideoMethod() == METHOD_LNV_HW_INTERFACE
		//||m_cControl->GetVideoMethod() == METHOD_ZTE_HW_INTERFACE
		)//||m_cControl->GetVideoMethod() == METHOD_HS_HW_INTERFACE)
	{
		if(m_cControl->GetCallingBuffIndex() == CALLINGBUFFER1)
		{
			*jpegSize  = m_JPEG_Filesize1;
			tmpBuf = m_JPEG_FileBuff1;
			m_JPEG_Filesize1 = 0;
		}
		else
			return NULL;
	}
	else
	{	
		if(!m_bSendBuffFlag1 && !m_bSendBuffFlag2)
			return NULL;

		m_prevUpdate = m_currUpdate;
		if(m_bSendBuffFlag1)
		{
			m_cControl->SetCallingBuffIndex(CALLINGBUFFER1);
			*jpegSize  = m_JPEG_Filesize1;
			m_JPEG_Filesize1 = 0;
			m_currUpdate = 1;
			tmpBuf = m_JPEG_FileBuff1;
		}
		else if(m_bSendBuffFlag2)
		{
			m_cControl->SetCallingBuffIndex(CALLINGBUFFER2);
			*jpegSize  = m_JPEG_Filesize2;
			m_JPEG_Filesize2 = 0;
			m_currUpdate = 2;
			tmpBuf = m_JPEG_FileBuff2;
		}

		if(m_prevUpdate == m_currUpdate)
		{
			//LOGE("m_currUpdate and m_prevUpdate is equality\n");
			return NULL;
		}
	}

	return tmpBuf;
}

/*****************************************************
* Func: LoopVideoThread
* Function: the thread receives information from the device
* Parameters: param: thread input parameter
*****************************************************/
void *CWiMoVideo::LoopThread(void * param)
{
	CWiMoVideo *pthis = (CWiMoVideo*)param;
	
	if(pthis->m_cControl->GetVideoMethod() == METHOD_VIDEO_ERROR)
	{
		LOGI("capture is METHOD_VIDEO_ERROR!\n");
		goto LoopVideoError;
	}
	else if(pthis->m_cControl->GetVideoMethod() == METHOD_HW_VIDEO_INTERFACE)
	{
		LOGI("capture is METHOD_HW_VIDEO_INTERFACE!\n");
	}
	else if(pthis->m_cControl->GetVideoMethod() == METHOD_HS_HW_INTERFACE)
	{
		if(pthis->LoopHSVideo() != S_OK)
			goto LoopVideoError;
	}
	else if(pthis->m_cControl->GetVideoMethod() == METHOD_LNV_HW_INTERFACE)
	{
		if(pthis->LoopLNVVideo() != S_OK)
			goto LoopVideoError;
	}
	else if(pthis->m_cControl->GetVideoMethod() == METHOD_ZTE_HW_INTERFACE)
	{
		if(pthis->LoopZTEVideo() != S_OK)
			goto LoopVideoError;
	}	
	else if(pthis->m_cControl->GetVideoMethod() == METHOD_FRAMEBUFFER_INTERFACE
		|| pthis->m_cControl->GetVideoMethod() == METHOD_SCREENSHOT_INTERFACE
		|| pthis->m_cControl->GetVideoMethod() == METHOD_GAME_INTERFACE)
	{
		LOGI("capture is SW interface!\n");
		if(pthis->LoopSWVideo() != S_OK)
			goto LoopVideoError;
	}

	LOGI("free buffer successfully!\n");	
	return 0;
LoopVideoError:
	pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);
	pthis->UnInit();
	LOGI("LoopThread end\n");
	return (void*)E_FAILED;
}

/*****************************************************
* Func: HSInit
* Function: hisense init
* Parameters: void
*****************************************************/
int CWiMoVideo::HSInit()
{
	m_getScreenDimemsion_pIpI = (pF_pINT_pINT)dlsym(m_pdl, "getScreenDimension");
	m_rotate_I = (pF_INT)dlsym(m_pdl, "rotate");
	m_readJpegData_R = (pF_pUCHAR_pINT_pINT64_pINT)dlsym(m_pdl, "readJpegData");

	if(m_getScreenDimemsion_pIpI)
		(*m_getScreenDimemsion_pIpI)(&m_iAlignedWidth, &m_iAlignedHeight);

	m_cControl->SetDeviceWidthHeight(m_iAlignedWidth, m_iAlignedHeight);
	m_iAlignedWidth = m_iAlignedWidth &~15;
	m_iAlignedHeight = m_iAlignedHeight &~15;
	LOGI("m_iAlignedWidth:%d, m_iAlignedHeight:%d\n",m_iAlignedWidth, m_iAlignedHeight);
	m_cControl->SetAlignedWidthHeight(m_iAlignedWidth, m_iAlignedHeight);
	
	m_iHSRotate = HS_ROTATE_0;

	CountOffsetSize();

	return S_OK;
}

/*****************************************************
* Func: ZTEInit
* Function: zte init
* Parameters: void
*****************************************************/
int CWiMoVideo::ZTEInit()
{
	WIMOPREFIX prefix;
	FILE* fd = NULL;
	memset(&prefix, 0, sizeof(WIMOPREFIX));
		
	fd = fopen(ZTEVIDEODATAYUV, "rb");
	if(fd == NULL)
		return E_FAILED;
	fread(&prefix, sizeof(prefix), 1, fd);
	fclose(fd);

	while(m_cControl->GetStatusWiMo() == WIMO_STATUS_INITING && (prefix.header.width == 0 && prefix.header.height == 0))
	{
		fd = fopen(ZTEVIDEODATAYUV, "rb");
		if(fd == NULL)
			return E_FAILED;
		fread(&prefix, sizeof(prefix), 1, fd);
		fclose(fd);
	}
	LOGI("width: %ld, height: %ld, fmt:%d!\n", prefix.header.width, prefix.header.height, prefix.header.fmt);
	m_cControl->SetDeviceWidthHeight(prefix.header.width, prefix.header.height);
	
	m_iAlignedWidth = m_cControl->GetDeviceWidth() &~15;
	m_iAlignedHeight = m_cControl->GetDeviceHeight() &~15;
	m_cControl->SetAlignedWidthHeight(m_iAlignedWidth, m_iAlignedHeight);

	m_pMapSize = m_iAlignedWidth*m_iAlignedHeight*3/2 + sizeof(prefix);
	int dp = open(ZTEVIDEODATAYUV, O_RDONLY);
//	lseek(dp, sizeof(format), SEEK_SET);
	m_pZTEmap = (unsigned char*)mmap(NULL, m_pMapSize, PROT_READ, MAP_SHARED, dp, 0);
	if((void*)-1 == m_pZTEmap && m_pZTEmap != 0)
	{
		LOGE("mmap m_pZTEmap error\n");
		close(dp);
		return E_FAILED;
	}
	close(dp);

	CountOffsetSize();

	return S_OK;
}

/*****************************************************
* Func: ZTEUnInit
* Function: zte uninit
* Parameters: void
*****************************************************/
void CWiMoVideo::ZTEUnInit()
{
	if(m_pZTEmap)
	{
		munmap(m_pZTEmap, m_pMapSize);
	}
}

/*****************************************************
* Func: loadLNVLib
* Function: lenovo load library
* Parameters: id: handle id [in]
*		      path: lib path [in]
*		      pHmi: hwjepgapi_module_t param [out]
*****************************************************/
int CWiMoVideo::loadLNVLib(const char *id, const char *path, const struct hwjepgapi_module_t **pHmi)
{
	int status;
	void *handle;
	struct hwjepgapi_module_t *hmi;
	const char *sym = HAL_INFO_SYM_STR;
	
	/*
	* load the symbols resolving undefined symbols before
	* dlopen returns. Since RTLD_GLOBAL is not or'd in with
	* RTLD_NOW the external symbols will not be global
	*/
	handle = dlopen(path, RTLD_NOW);
	if (handle == NULL) 
	{
		char const *err_str = dlerror();
		LOGE("load: module=%s\n%s", path, err_str?err_str:"unknown");
		status = -EINVAL;
		goto DONE;
	}

	/* Get the address of the struct hal_module_info. */
	hmi = (struct hwjepgapi_module_t *)dlsym(handle, sym);
	if (hmi == NULL) 
	{
		LOGE("load: couldn't find symbol %s", sym);
		status = -EINVAL;
		goto DONE;
	}

	/* Check that the id matches */
	if (strcmp(HWJPEG_MODULE_ID, hmi->id) != 0) 
	{
		LOGE("load: id=%s != hmi->id=%s", id, hmi->id);
		status = -EINVAL;
		goto DONE;
	}

	hmi->dso = handle;

	/* success */
	status = 0;

DONE:
	if (status != 0) 
	{
		hmi = NULL;
		if (handle != NULL) 
		{
			dlclose(handle);
			handle = NULL;
		}
	}
	else
	{
		LOGV("loaded HAL id=%s path=%s hmi=%p handle=%p",
		id, path, *pHmi, handle);
	}

	*pHmi = hmi;

	return status;
}

/*****************************************************
* Func: LNVInitInThread
* Function: the thread init 
* Parameters: param: thread input parameter
*****************************************************/
void *CWiMoVideo::LNVInitInThread(void * param)
{
	CWiMoVideo* pthis = (CWiMoVideo*)param;
	if(pthis->LNVInit() != S_OK)
	{
		LOGE("failed to lnvInit!\n");
		pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);
	}
	pthread_detach(pthread_self());
	
	return 0;
}

/*****************************************************
* Func: LNVInit
* Function: lenovo uninit
* Parameters: void
*****************************************************/
int CWiMoVideo::LNVInit()
{
	const hwjepgapi_module_t *mod;
	int jpegw = 0, jpegh = 0;

	m_JPEG_FileBuff1 = NULL;
	if(loadLNVLib(HWJPEG_MODULE_ID, LIB_NAME_LENOVO, &mod) != 0)
		return E_FAILED;
	
	jpeg_hw_device_open(mod, &m_lnvDev,JPEG_QUALITY);

	m_lnvDev->getHWResolution(m_lnvDev,&jpegw,&jpegh);
	LOGI("jpegw: %d, jpegh: %d\n", jpegw, jpegh);
	
	m_cControl->SetDeviceWidthHeight(jpegw, jpegh);
	m_cControl->SetAlignedWidthHeight(jpegw, jpegh);
	
	return S_OK;
}

/*****************************************************
* Func: LNVUnInit
* Function: lenovo uninit
* Parameters: void
*****************************************************/
void CWiMoVideo::LNVUnInit()
{
	if(m_lnvDev)
	{
		jpeg_hw_device_close(m_lnvDev);
		m_lnvDev = NULL;
	}
}

int CWiMoVideo::QiYiGameInit()
{
	m_iAlignedWidth = m_cControl->GetDeviceWidth() &~15;
	m_iAlignedHeight = m_cControl->GetDeviceHeight() &~15;
	m_sStride = m_cControl->GetDeviceWidth();

	m_bRevertUVPlanes = false;

	CountOffsetSize();
	
	return S_OK;
}

void CWiMoVideo::QiYiGameUnInit()
{

}

/*****************************************************
* Func: ReadFB0Init
* Function: init frame buffer to read resource data
* Parameters: void
*****************************************************/
int CWiMoVideo::ReadFB0Init()
{
	char cDevNode[128];
	sprintf(cDevNode, FRAMEBUFERFB);
	LOGE("test %s \n",cDevNode);
	m_iHndFD = open(cDevNode, O_RDONLY);

	if(m_iHndFD < 0)
	{
		LOGE("/dev/graphics/fb0 open error\n");
		return E_FAILED;
	}

	int ret = -1;
	ret = ioctl(m_iHndFD, FBIOGET_VSCREENINFO, &m_fbvinfo);
	if(ret)
	{
		LOGE("ioctl(..., FBIOGET_VSCREENINFO, ...) on framebuffer error %x\n", ret);
		return E_FAILED;
	}
	ret = ioctl(m_iHndFD, FBIOGET_FSCREENINFO, &m_fbfinfo);
	if(ret)
	{
		LOGE("ioctl(..., FBIOGET_FSCREENINFO, ...) on framebuffer error %x\n", ret);
		return E_FAILED;
	}

	m_iFirstImageSize = m_fbvinfo.xres * m_fbvinfo.bits_per_pixel * m_fbvinfo.yres / 8;
	m_sStride = m_fbfinfo.line_length / (m_fbvinfo.bits_per_pixel >> 3);
	m_iMmapSize = m_fbfinfo.smem_len;
	
	LOGI("m_fbvinfo.xres:%d ,y:%d bye: %d, m_sStride:%d\n",m_fbvinfo.xres,m_fbvinfo.yres,m_fbvinfo.bits_per_pixel,m_sStride);
	if(m_fbvinfo.xres == 0 || m_fbvinfo.yres ==0 )
		return E_FAILED;
	
	m_cControl->SetDeviceWidthHeight(m_fbvinfo.xres, m_fbvinfo.yres);
	if(m_sStride != (int)m_fbvinfo.xres)
	{//the device has already aligned up to 16 bytes.
		if(m_cControl->GetDeviceModel() == DEVICE_XIAOMI2_41)
		{
			m_iAlignedWidth = m_cControl->GetDeviceWidth();
			m_iAlignedHeight = m_cControl->GetDeviceHeight();
		}
		else
		{
			m_iAlignedWidth = m_sStride;
			m_iAlignedHeight = m_cControl->GetDeviceWidth()*m_cControl->GetDeviceHeight()/m_iAlignedWidth;
		}
	}
	else
	{//aligned down to 16 bytes
		m_iAlignedWidth = m_cControl->GetDeviceWidth() &~15;
		m_iAlignedHeight = m_cControl->GetDeviceHeight() &~15;
	}
	LOGI("m_iAlignedWidth:%d, m_iAlignedHeight:%d\n",m_iAlignedWidth, m_iAlignedHeight);
	m_cControl->SetAlignedWidthHeight(m_iAlignedWidth, m_iAlignedHeight);

	CountOffsetSize();

#ifdef MTK_CAPTURE_FRAME_BUFFER
	m_iHndFpBuff = (Byte*)malloc(m_iFirstImageSize);
	if(!m_iHndFpBuff)
	{
		LOGE("failed malloc m_iHndFpBuff \n");
		return E_FAILED;
	}
	LOGI("mtk capture frame buffer success!\n");
#else
	m_iHndFpBuff = (unsigned char *)mmap(0, m_iMmapSize, PROT_READ, MAP_SHARED, m_iHndFD, 0);
	if((void*)-1 == m_iHndFpBuff)
	{
		LOGE("mmap framebuffer error\n");
		return E_FAILED;
	}
#endif

	return S_OK;
}

/*****************************************************
* Func: ReadFB0UnInit
* Function: uninit frame buffer resource
* Parameters: void
*****************************************************/
void CWiMoVideo::ReadFB0UnInit()
{
	if(m_iHndFD != 0)
	{
		close(m_iHndFD);
		m_iHndFD =0;
	}
#ifdef MTK_CAPTURE_FRAME_BUFFER
	if(m_iHndFpBuff)
	{
		free(m_iHndFpBuff);
		m_iHndFpBuff = NULL;
	}
#else
	if(m_iHndFpBuff)
	{
		munmap(m_iHndFpBuff, m_iMmapSize);
		m_iHndFpBuff = NULL;
		m_iMmapSize = 0;
	}
#endif
}

void *CWiMoVideo::InitScreenshotThread(void * param)
{
	CWiMoVideo *pthis = (CWiMoVideo*)param;
	
	if(pthis->ReadScreenshotInit() != S_OK)
	{
		LOGE("failed to ReadScreenshotInit()!\n");
		pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);
	}

	return 0;
}

/*****************************************************
* Func: ReadScreenshotInit
* Function:  init system method to read resource data
* Parameters: void
*****************************************************/
int CWiMoVideo::ReadScreenshotInit()
{
	m_shotFD = open(CIDATAFILEPATH, O_RDONLY);
	if(!m_shotFD){
		LOGE("ReadScreenshotInit open failed!\n");
		return E_FAILED;
	}
	read(m_shotFD, &m_sshotHeader, sizeof(CI_SCRSHOT_HEADER));
	m_screenshotBuff = (Byte*)malloc(m_sshotHeader.width*m_sshotHeader.height*4);
	if(m_screenshotBuff == NULL){
		LOGE("malloc m_iHndFpBuff failed!\n");
	}	
	read(m_shotFD, m_screenshotBuff, m_sshotHeader.size);

	LOGI("screenshot: w: %d, h: %d, size: %d\n", m_sshotHeader.width, m_sshotHeader.height, m_sshotHeader.size);
	m_cControl->SetDeviceWidthHeight(m_sshotHeader.width, m_sshotHeader.height);
	m_sStride = m_cControl->GetDeviceWidth();
	m_iAlignedWidth = m_cControl->GetDeviceWidth() &~15;
	m_iAlignedHeight = m_cControl->GetDeviceHeight() &~15;
	m_cControl->SetAlignedWidthHeight(m_iAlignedWidth, m_iAlignedHeight);

	CountOffsetSize();

	return S_OK;
}

/*****************************************************
* Func: ReadScreenshotUnInit
* Function: uninit frame buffer resource
* Parameters: void
*****************************************************/
void CWiMoVideo::ReadScreenshotUnInit()
{
	if(m_screenshotBuff)
	{
		free(m_screenshotBuff);
		m_screenshotBuff = NULL;
	}
}

void CWiMoVideo::CountOffsetSize()
{
	if(m_iAlignedWidth != m_cControl->GetDeviceWidth())
	{
		m_gapValueW = (m_cControl->GetDeviceWidth() - m_iAlignedWidth)/2;
	}
	if(m_iAlignedHeight != m_cControl->GetDeviceHeight())
	{
		m_gapValueH = (m_cControl->GetDeviceHeight() - m_iAlignedHeight)/2;
	}
	if(m_gapValueH != 0)
		m_offsetSize = m_gapValueH*m_cControl->GetDeviceWidth();
	
	//LOGI("m_iAligned w,h: %d,%d, m_gapValue w,h: %d, %d,m_offsetSize:%d\n",m_iAlignedWidth,m_iAlignedHeight,m_gapValueW,m_gapValueH,m_offsetSize);	
}

/*****************************************************
* Func: InitImgInfoHelper
* Function: Init video source image information
* Parameters: 
*	pImgInfo: set image information
*	pDst: will save yuv data
*****************************************************/
int CWiMoVideo::InitImgInfoHelper(IMGINFO *pImgInfo, unsigned char **pDst)
{	
	pImgInfo->pDst[0] = pDst[0];
	pImgInfo->pDst[1] = pDst[1];
	pImgInfo->pDst[2] = pDst[2];

	pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	pImgInfo->ImgFmtOut = CI_IMAGEPROC_FORMAT_YCBCR420;
	pImgInfo->interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(m_cControl->GetVideoMethod() == METHOD_SCREENSHOT_INTERFACE)
	{
		LOGE("m_sshotHeader.fmt: %d\n", m_sshotHeader.fmt);
		if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_RGB_565)
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB565;
		else if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_RGBA_4444)
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB888;
		else if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_BGRA_8888)
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ABGR32;
		else if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_RGBA_8888)
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ARGB32;
	}
	else 	if(m_cControl->GetVideoMethod() == METHOD_FRAMEBUFFER_INTERFACE)
	{
		LOGE("red:%d gre:%d blue:%d \n",m_fbvinfo.red.offset ,m_fbvinfo.green.offset , m_fbvinfo.blue.offset);
		if((m_fbvinfo.red.offset == 0 && m_fbvinfo.green.offset == 8&& m_fbvinfo.blue.offset == 16)
			|| (m_fbvinfo.red.offset == 24 && m_fbvinfo.green.offset == 16 && m_fbvinfo.blue.offset == 8))
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ARGB32;
		else if(m_fbvinfo.red.offset == 11 && m_fbvinfo.green.offset == 5 && m_fbvinfo.blue.offset == 0)
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB565;
		else
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ABGR32;
	}
	else 	if(m_cControl->GetVideoMethod() == METHOD_GAME_INTERFACE)
	{
		if(m_cControl->GetVideoType() == VIDEO_TYPE_ARGB8888)
			pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ABGR32;
	}

	pImgInfo->dstWidth = pImgInfo->srcWidth = m_cControl->GetDeviceWidth();
	pImgInfo->dstHeight = pImgInfo->srcHeight = m_cControl->GetDeviceHeight();
	
	if(pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_RGB565)
		pImgInfo->srcStride[0] = m_sStride*2;
	else if(pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_ABGR32 || pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_ARGB32)
		pImgInfo->srcStride[0] = m_sStride*4;
	else
	{
		LOGE("invalid format!!!");
		return E_INVALIDARG;
	}

	m_offsetSize = pImgInfo->srcStride[0]/m_sStride*m_offsetSize;
	pImgInfo->srcStride[1] = pImgInfo->srcStride[2] = 0;
	return S_OK;
}

int CWiMoVideo::CheckSupportNeon()
{
	AndroidCpuFamily cpuFamily = android_getCpuFamily();
//	int cpuCoreCnt = android_getCpuCount();

	//LOGI("CPU family (hex): %u", cpuFamily);
#if 0
	if(cpuFamily == ANDROID_CPU_FAMILY_ARM) {
                LOGI("-ARM\n");
        } else if(cpuFamily == ANDROID_CPU_FAMILY_X86) {
                LOGI("-X86\n");
        } else {
                LOGI("-unknown\n");
        }
#endif
//	LOGI("Number of CPU Cores (hex): %u\n", cpuCoreCnt);

	if(cpuFamily == ANDROID_CPU_FAMILY_ARM) 
	{
		uint64_t cpuFeatures = android_getCpuFeatures();

		//LOGI("CPU features (hex): %llu\n", cpuFeatures);

		if(cpuFeatures & ANDROID_CPU_ARM_FEATURE_NEON) {
			//LOGI("NEON: Arm NEON is supported.\n");
			m_bNeon = true;
		} else {
			//LOGI("NEON: Arm NEON is not supported.\n");
			m_bNeon = false;
		}

		if(cpuFeatures & ANDROID_CPU_ARM_FEATURE_ARMv7) {
			//LOGI("ARCH: ARMv7.\n");
			m_bArmv7orUP = true;
		} else {
			//LOGI("ARCH: not are ARMv7.\n");
			m_bArmv7orUP = false;
		}
	/*
		if(cpuFeatures & ANDROID_CPU_ARM_FEATURE_VFPv3) {
			LOGI("VFPv3 is supported.\n");
		} else {
			LOGI("VFPv3 is not supported.\n");
		}
	*/	
	}

	return 0;
}

int CWiMoVideo::LoadLibFunc()
{
	if(m_imagePdl != NULL)
	{
		m_CI_IMGPROC_Create = (pF_ppVOID_pVOID_pCI_IMAGEPROC)dlsym(m_imagePdl, "CI_IMAGEPROC_Create");
		m_CI_IMGPROC_ProcessFrame = (pF_pVOID_ppU8_pU32_ppU8_pU32_pCI_IMAGEPROC)dlsym(m_imagePdl, "CI_IMAGEPROC_ProcessFrame");
		m_CI_IMGPROC_Destroy = (pF_pVOID)dlsym(m_imagePdl, "CI_IMAGEPROC_Destroy");
	}
	else
	{
		const char* ch_errno = dlerror();
		LOGE("dlopen image proc libs failed, ch_errno:%s !\n", ch_errno);
		return E_FAILED;
	}
	
	if(m_encodePdl != NULL)
	{
		m_CI_JpgEnc_Open = (pF_ppVOID_pJPGENC)dlsym(m_encodePdl, "CI_JPGENC_Open");
		m_CI_JpgEnc_Frame = (pF_pVOID_ppU8_pU32_ciSize_pU8_pU32)dlsym(m_encodePdl, "CI_JPGENC_Frame");
		m_CI_JpgEnc_Close = (pF_pVOID)dlsym(m_encodePdl, "CI_JPGENC_Close");
	}
	else
	{
		const char* ch_errno = dlerror();
		LOGE("dlopen encoder jpeg libs failed, ch_errno:%s !\n",ch_errno);
		return E_FAILED;
	}
		
	return S_OK;
}

int CWiMoVideo::OpenImgProcessLib()
{
	CheckSupportNeon();
	char imgPath[STRINGDATESIZE] = {0};
	char encPath[STRINGDATESIZE] = {0};
	memcpy(imgPath, m_pAppLibPath, strlen(m_pAppLibPath));
	memcpy(encPath, m_pAppLibPath, strlen(m_pAppLibPath));
		
	if(m_bNeon)
	{
		LOGI("it is support neon processor!\n");
		strcat(imgPath, "libciviproc_neon.so");
		strcat(encPath, "libcijpegenc_neon.so");
	}
	else if(m_bArmv7orUP) 
	{
		LOGI("it is support armv5 processor!\n");
		strcat(imgPath, "libciviproc_arm.so");
		strcat(encPath, "libcijpegenc_armv5.so");
	}
	else
	{
		LOGI("it is support armv4 processor!\n");
		m_bRevertUVPlanes = false;
		strcat(imgPath, "libciviproc_arm.so");
		strcat(encPath, "libcijpegenc_armv4.so");		
	}
	m_imagePdl = dlopen(imgPath, RTLD_NOW);
	m_encodePdl = dlopen(encPath, RTLD_NOW);
	if(LoadLibFunc() != S_OK)
		return E_FAILED;
	
	return S_OK;
}

void CWiMoVideo::CloseLib()
{
	if(m_imagePdl != NULL)
		dlclose(m_imagePdl);
	if(m_encodePdl != NULL)
		dlclose(m_encodePdl);
}

/*****************************************************
* Func: RGB2YUV
* Function: process rgb and output yuv data
* Parameters: 
*	pImgInfo: set input and output information
*	srcSize: set input width and height
*	dstSize: set output width and height
*****************************************************/
int CWiMoVideo::RGB2YUV(IMGINFO *pImgInfo, CI_IMAGEPROC_SIZE srcSize,CI_IMAGEPROC_SIZE dstSize)
{
	if(!pImgInfo)
	{
		LOGE("pImgInfo is empty in RGB2YUV()!\n");
		return E_FAILED;
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
	if(m_CI_IMGPROC_Create)
		retimg  = (*m_CI_IMGPROC_Create)(&hImgPrc, NULL, &CreateOpt);
	if(retimg)
	{
		LOGE("imageproc create failed!!!!retimg  :%d \n",retimg);
		return E_FAILED;
	}
	if(m_CI_IMGPROC_ProcessFrame)
		retimg = (*m_CI_IMGPROC_ProcessFrame)(hImgPrc, pImgInfo->pSrc, pImgInfo->srcStride, pImgInfo->pDst, pImgInfo->dstStride, &ProcessOpt);
	if(retimg)
	{
		LOGE("process frame failed!!!!retimg  :%d \n",retimg);
		return E_FAILED;
	}
	if(m_CI_IMGPROC_Destroy)
		(*m_CI_IMGPROC_Destroy)(hImgPrc);

	return S_OK;
}

/*****************************************************
* Func: YUV2JPG
* Function: convert yuv to jpeg
* Parameters: 
*	pImgInfo: set input image information
*	srcSize: the width and height of the YUV image
*****************************************************/
int CWiMoVideo::YUV2JPG(IMGINFO *pImgInfo, CI_SIZE srcSize)
{
	if(!pImgInfo)
	{
		LOGE("pImgInfo is empty in YUV2JPG()!\n");
		return E_FAILED;
	}
	
	CI_JPGENCOPENOPTIONS OpenOptions;
	CI_VOID *pEncoder;
	CI_RESULT ret = -1;
	int format = 3;

	OpenOptions.u8OutType = CI_OUT_JPEG;
	if((srcSize.width==320 && srcSize.height==480)
		|| (srcSize.width==240 && srcSize.height==320))
	{
		OpenOptions.u8Quality = CI_QUALITY_LEVEL4;
	}else{
		OpenOptions.u8Quality = CI_QUALITY_LEVEL3;
	} 
	
	if(m_cControl->GetVideoMethod() == METHOD_ZTE_HW_INTERFACE && pImgInfo->format == YV12)
		OpenOptions.u8YUVType = CI_YUV420_PLANAR;
	else	
		OpenOptions.u8YUVType = format == 3? CI_YUV420_PLANAR : format;

	if(m_CI_JpgEnc_Open)
		ret  = (*m_CI_JpgEnc_Open)(&pEncoder, &OpenOptions);	
	if(ret != CI_SOK)
	{
		LOGE("enc initial failed, %x!\n",ret);
		return E_FAILED;
	}
	if((!m_bSendBuffFlag1 && m_cControl->GetCallingBuffIndex() == CALLINGBUFFER2)
		|| m_cControl->GetCallingBuffIndex() == CALLINGENTRANCEFIRSTLY)
	{
		if(m_CI_JpgEnc_Frame)
			ret  = (*m_CI_JpgEnc_Frame)(pEncoder, pImgInfo->pDst, pImgInfo->dstStride, srcSize, (CI_U8*)m_JPEG_FileBuff1, (CI_U32*)&m_JPEG_Filesize1);
		if(ret != CI_SOK)
		{
			LOGE("enc enc failed!ret 1: %x\n", ret);
			return E_FAILED;
		}
		m_bSendBuffFlag1 = true;
		m_bSendBuffFlag2 = false;
	}
	else if(!m_bSendBuffFlag2 && m_cControl->GetCallingBuffIndex() == CALLINGBUFFER1)
	{
		if(m_CI_JpgEnc_Frame)
			ret  = (*m_CI_JpgEnc_Frame)(pEncoder, pImgInfo->pDst, pImgInfo->dstStride, srcSize, (CI_U8*)m_JPEG_FileBuff2, (CI_U32*)&m_JPEG_Filesize2);
		if(ret != CI_SOK)
		{
			LOGE("enc enc failed!ret 2: %x\n", ret);
			return E_FAILED;
		}
		m_bSendBuffFlag1 = false;
		m_bSendBuffFlag2 = true;
	}
	else
	{
		//LOGE("it is no enc process!\n");
		return S_CONTINUE;
	}
	if(m_CI_JpgEnc_Close)
		(*m_CI_JpgEnc_Close)(pEncoder);	

	return S_OK;	
}

/*****************************************************
* Func: ChangeAttribute
* Function: according rotation to change the param attributes
* Parameters: 
*			pImgInfo: image info
*			yuvSrcSize: yuv source size for changing yuv to jpeg
*			srcSize: source size for changing rgb to yuv
*			dstSize: destination size for changing rgb to yuv
*****************************************************/
void CWiMoVideo::ChangeAttribute(IMGINFO *pImgInfo, CI_SIZE &yuvSrcSize, CI_IMAGEPROC_SIZE &srcSize, CI_IMAGEPROC_SIZE &dstSize)
{
	int w = 0, h = 0;
	if(m_cControl->GetDeviceWidth()/16 == 0)
		w = m_cControl->GetDeviceWidth();
	else
		w = (m_cControl->GetDeviceWidth() + 15)&~15;

	if(m_cControl->GetDeviceHeight()/16 == 0)
		h = m_cControl->GetDeviceHeight();
	else
		h = (m_cControl->GetDeviceHeight() + 15)&~15;

	if(m_cControl->GetTVRotateOrientation() == CURRENT_TV_ROTATE_90R || m_cControl->GetTVRotateOrientation() == CURRENT_TV_ROTATE_90L)
	{
		if(m_cControl->GetRotateOrientation() == ROTATION_90 || m_cControl->GetRotateOrientation() == ROTATION_270)
		{		
			pImgInfo->dstStride[0] = w;
			yuvSrcSize.width = pImgInfo->dstWidth = pImgInfo->srcWidth;
			yuvSrcSize.height = pImgInfo->dstHeight = pImgInfo->srcHeight;
			if(m_cControl->GetTVRotateOrientation() == CURRENT_TV_ROTATE_90R)
			{
				if(m_cControl->GetRotateOrientation() == ROTATION_90)
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_180;
				else if(m_cControl->GetRotateOrientation() == ROTATION_270)
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
			}
			else if(m_cControl->GetTVRotateOrientation() == CURRENT_TV_ROTATE_90L)
			{
				if(m_cControl->GetRotateOrientation() == ROTATION_90)
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
				else if(m_cControl->GetRotateOrientation() == ROTATION_270)
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_180;
			}
		}	
		else
		{
			pImgInfo->dstStride[0] = h;
			yuvSrcSize.width = pImgInfo->dstWidth = pImgInfo->srcHeight &~15;
			yuvSrcSize.height = pImgInfo->dstHeight = pImgInfo->srcWidth &~15;				
			if(m_cControl->GetTVRotateOrientation() == CURRENT_TV_ROTATE_90R)
			{
				if(m_cControl->GetRotateOrientation() == ROTATION_0)
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90L;
				else
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90R;
			}
			else
			{
				if(m_cControl->GetRotateOrientation() == ROTATION_0)
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90R;
				else
					pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90L;
			}
		}
	}
	else if(m_cControl->GetTVRotateOrientation() == CURRENT_TV_ROTATE_0)
	{
		if(m_cControl->GetRotateOrientation() == ROTATION_90 || m_cControl->GetRotateOrientation() == ROTATION_270)
		{		
			pImgInfo->dstStride[0] = h;
			yuvSrcSize.width = pImgInfo->dstHeight = pImgInfo->srcHeight &~15;
			yuvSrcSize.height = pImgInfo->dstWidth = pImgInfo->srcWidth &~15;
			if(m_cControl->GetRotateOrientation() == ROTATION_90)
				pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90L;
			else
				pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90R;
		}	
		else
		{
			pImgInfo->dstStride[0] = w;
			yuvSrcSize.width = pImgInfo->dstWidth = pImgInfo->srcWidth;
			yuvSrcSize.height = pImgInfo->dstHeight = pImgInfo->srcHeight;
			if(m_cControl->GetRotateOrientation() == ROTATION_0)
				pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
			else
				pImgInfo->rotation = CI_IMAGEPROC_ROTATION_180;
		}
	}

	if(pImgInfo->rotation == CI_IMAGEPROC_ROTATION_90L ||pImgInfo->rotation == CI_IMAGEPROC_ROTATION_90R
		|| pImgInfo->rotation == CI_IMAGEPROC_ROTATION_180)
	{
		srcSize.s32Width = dstSize.s32Width = pImgInfo->srcWidth &~15;
		srcSize.s32Height = dstSize.s32Height = pImgInfo->srcHeight &~15;
		m_bOffset = true;
	}
	else
	{
		srcSize.s32Width = dstSize.s32Width = pImgInfo->srcWidth &~7;
		srcSize.s32Height = dstSize.s32Height = pImgInfo->srcHeight &~7;
		m_bOffset = false;
	}	
	pImgInfo->dstStride[1] = pImgInfo->dstStride[0]>>1;
	pImgInfo->dstStride[2] = pImgInfo->dstStride[0]>>1;
}

int CWiMoVideo::ScreenshotUpdataInfo(IMGINFO *pImgInfo)
{
	if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_RGB_565)
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB565;
	else if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_RGBA_4444)
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB888;
	else if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_BGRA_8888)
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ABGR32;
	else if(m_sshotHeader.fmt == CI_PIXEL_FORMAT_RGBA_8888)
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ARGB32;

	int size = read(m_shotFD, &m_sshotHeader, sizeof(m_sshotHeader));
	if(size == -1)
		return E_FAILED;
	size = read(m_shotFD, m_screenshotBuff, m_sshotHeader.size);
	if(size == -1)
		return E_FAILED;

	return S_OK;
}

int CWiMoVideo::FramBufferUpdataInfo(IMGINFO *pImgInfo)
{
#ifdef MTK_CAPTURE_FRAME_BUFFER
		if(ioctl(m_iHndFD, MTKFB_CAPTURE_FRAMEBUFFER, &m_iHndFpBuff) < 0) {      // read data back from fb
	        	LOGE("ioctl of MTKFB_CAPTURE_FRAMEBUFFER fail\n");
			return E_FAILED;
		}
#endif
	if((m_fbvinfo.red.offset == 0 && m_fbvinfo.green.offset == 8&& m_fbvinfo.blue.offset == 16)
		|| (m_fbvinfo.red.offset == 24 && m_fbvinfo.green.offset == 16 && m_fbvinfo.blue.offset == 8))
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ARGB32;
	else if(m_fbvinfo.red.offset == 11 && m_fbvinfo.green.offset == 5 && m_fbvinfo.blue.offset == 0)
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB565;
	else
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ABGR32;

	return S_OK;
}

/*****************************************************
* Func: ProcessVideo
* Function: process video data
* Parameters: pImgInfo: image parameters
*****************************************************/
int CWiMoVideo::ProcessVideo(IMGINFO *pImgInfo)
{
	if(pImgInfo == NULL)
		return E_FAILED;
	
	CI_IMAGEPROC_SIZE srcSize = {0, 0};
	CI_IMAGEPROC_SIZE dstSize = {0, 0};
	CI_SIZE yuvSrcSize = {0,0};
	int srcDataSize = 0;

	//alocation buffer and src buffer
	if(m_shotFD!=0 && (m_cControl->GetVideoMethod() == METHOD_SCREENSHOT_INTERFACE))
	{
		if(ScreenshotUpdataInfo(pImgInfo) != S_OK)
			return E_FAILED;
		pImgInfo->pSrc[0] = (unsigned char*)m_screenshotBuff;
	}
	else if(m_cControl->GetVideoMethod() == METHOD_FRAMEBUFFER_INTERFACE)
	{
		if(FramBufferUpdataInfo(pImgInfo) != S_OK)
			return E_FAILED;
		pImgInfo->pSrc[0] = (unsigned char*)m_iHndFpBuff;
	}
	else if(m_cControl->GetVideoMethod() == METHOD_GAME_INTERFACE)
	{
		pImgInfo->pSrc[0] = (unsigned char*)m_cControl->GetVideoDataBuff(&srcDataSize);
		if(srcDataSize == 0)
			return S_CONTINUE;
	}

	if(pImgInfo->pSrc[0] == NULL)
		return S_CONTINUE;

	ChangeAttribute(pImgInfo, yuvSrcSize, srcSize, dstSize);

	if(m_bOffset && m_gapValueH > 0)
	{
		pImgInfo->pSrc[0] = pImgInfo->pSrc[0] + m_offsetSize;
		//LOGI("pSrc[0]: %p, m_screenshotBuff: %p, m_offsetSize: %d\n", pImgInfo->pSrc[0],m_screenshotBuff,m_offsetSize);		
	}
		
	if(RGB2YUV(pImgInfo,srcSize,dstSize) != S_OK)
	{
		LOGE("RGB2YUV enc frame failure!\n");
		return E_FAILED;
	}
	if(m_bRevertUVPlanes)
	{
		unsigned char* yuvDst = NULL;
		yuvDst = pImgInfo->pDst[1];
		pImgInfo->pDst[1] = pImgInfo->pDst[2];
		pImgInfo->pDst[2] = yuvDst;
	}
	if(YUV2JPG(pImgInfo, yuvSrcSize) == E_FAILED)
	{
		LOGE("Jpeg enc frame failure!\n");
		return E_FAILED;
	}

	return S_OK;
}

/*****************************************************
* Func: CountAndPrintFPS
* Function: count and print fps
* Parameters: 
*	frames: current frames
*	startTick: note start time every seconds
*****************************************************/
void CWiMoVideo::CountAndPrintFPS(int &frames, DDword &startTick)
{
	if(GetTickCount() - startTick > NumOneThousand)
	{
		if(m_cControl->pCPUControl != NULL && frames > m_cControl->pCPUControl->GetMaxFPS())
		{
			if(frames > m_mTargetFPS)
				frames = m_mTargetFPS;
			m_cControl->pCPUControl->SetMaxFPS(frames);
			LOGI("maxFrames:%d\n",m_cControl->pCPUControl->GetMaxFPS());
		}
		LOGI("fps :%d\n",frames);

		startTick = GetTickCount();
		frames = 0;
	}
}

/*****************************************************
* Func: SetAverageSleep
* Function: set average sleep every frame
* Parameters: 
*	processVideoTime: cost time of the processing video
*	avgSleep: set average sleep time
*****************************************************/
int CWiMoVideo::SetAverageSleep(DDword videoProcessTime, int &avgSleep)
{
	if(m_cControl->pCPUControl == NULL)
	{
		avgSleep = MINTIMESLEEP;
		return 0;
	}
		
	int m_averageCostTimeEveryFrame = NumOneThousand / m_cControl->pCPUControl->GetTargetFPS();
	int requireSleepTime = m_averageCostTimeEveryFrame - videoProcessTime;

	if(requireSleepTime < 0)
		avgSleep = MINTIMESLEEP;
	else if(requireSleepTime < MINTIMESLEEP && requireSleepTime >= 0)
	{
		m_sumOverTimeOfVideoProcess += MINTIMESLEEP + videoProcessTime - m_averageCostTimeEveryFrame;
		avgSleep = MINTIMESLEEP;

		if(m_sumOverTimeOfVideoProcess > NumOneThousand)
			m_sumOverTimeOfVideoProcess = 0;
	}
	else	
	{
		if(0 == m_sumOverTimeOfVideoProcess)
		{
			avgSleep = requireSleepTime;
			return S_OK;
		}
		
		if(m_sumOverTimeOfVideoProcess > requireSleepTime)
		{
			m_sumOverTimeOfVideoProcess = MINTIMESLEEP + m_sumOverTimeOfVideoProcess - requireSleepTime;
			avgSleep = MINTIMESLEEP;
		}
		else
		{
			avgSleep = requireSleepTime - m_sumOverTimeOfVideoProcess;
			if(avgSleep > MINTIMESLEEP)
				m_sumOverTimeOfVideoProcess = 0;
			else
			{
				m_sumOverTimeOfVideoProcess = MINTIMESLEEP -avgSleep;
				avgSleep = MINTIMESLEEP;
			}			
		}
	}

	//LOGI("avgSleep: %d, videoCostTime:%ld,m_averageCostTimeEveryFrame:%d,m_sumOverTimeOfVideoProcess:%d\n",
	//	avgSleep,videoProcessTime,m_averageCostTimeEveryFrame,m_sumOverTimeOfVideoProcess);

	return S_OK;
}

/*****************************************************
* Func: LoopSWVideo
* Function: loop video capture software
* Parameters: void
*****************************************************/
int CWiMoVideo::LoopSWVideo()
{
	unsigned char *pDst[3] = {0,0,0};
	IMGINFO ImgInfo;
	int frames = 0;
	DDword startTick = 0;
	DDword processVideoTime = 0;
	int avgSleep = 0;
	int ret = -1;

	int width = (m_cControl->GetDeviceWidth() + 15)&~15;
	int height = (m_cControl->GetDeviceHeight() + 15)&~15;

	pDst[0] = (unsigned char *)malloc(width*height*4);	
	if(!pDst[0])
	{
		LOGE("failed malloc pDst[0] \n");
		goto QUIT;
	}
	pDst[1] = pDst[0] + width*height;
	pDst[2] = pDst[1] + width*height/4;

	memset(&ImgInfo ,0 ,sizeof(IMGINFO));

	if(m_cControl->GetLimitedFPS())
		m_mTargetFPS = MAX_TARGET_FPS_5;
	else
		m_mTargetFPS = MAX_TARGET_FPS_30;
	
	if(InitImgInfoHelper(&ImgInfo, pDst) != 0)
	{
		LOGE("failed init ImgInfo\n");
		goto QUIT;
	}

	OpenImgProcessLib();
	
	startTick = GetTickCount();
	while(m_cControl->GetStatusWiMo() == WIMO_STATUS_CONNECTING)
	{
		processVideoTime = GetTickCount();
		ret = ProcessVideo(&ImgInfo);
		if(ret == E_FAILED)
			goto QUIT;
		if(ret == S_CONTINUE)
		{
			usleep(1*NumOneThousand);
			continue;
		}

		frames++;
		CountAndPrintFPS(frames, startTick);
		SetAverageSleep(GetTickCount() - processVideoTime, avgSleep);
		usleep(avgSleep*NumOneThousand);
	}

	CloseLib();
	
	if(pDst[0])
	{
		free(pDst[0]);
		pDst[0] = 0;
	}
	return S_OK;
	
QUIT:
	if(pDst[0])
	{
		free(pDst[0]);
		pDst[0] = 0;
	}
	
	return E_FAILED;
}

/*****************************************************
* Func: LoopLNVVideo
* Function: loop video capture software for Lenovo
* Parameters: void
*****************************************************/
int CWiMoVideo::LoopLNVVideo()
{
	int frames = 0;
	DDword startTick = 0;
	DDword lastickTime = 0;
	int avgSleep = 0;
	int firstFrame = 0;
	int rotation = ROTATION_0;
	int TVRotate = CURRENT_TV_ROTATE_0;
	
#if ANDROID_RESOURCE_BUILDER		
	pid_t tid  = gettid();
	LOGE("LoopLNVVideo tid: %d\n", tid);
	set_sched_policy(tid, SP_FOREGROUND);
	setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_DISPLAY);
#endif

	lastickTime = startTick = GetTickCount();
	while(m_cControl->GetStatusWiMo() == WIMO_STATUS_CONNECTING)
	{
		rotation = m_cControl->GetRotateOrientation();
		TVRotate = m_cControl->GetTVRotateOrientation();
		
		CountAndPrintFPS(frames, startTick);
		if(m_cControl->GetCallingBuffIndex() == CALLINGBUFFER2
			||m_cControl->GetCallingBuffIndex() == CALLINGENTRANCEFIRSTLY)
		{
			if((TVRotate == CURRENT_TV_ROTATE_0 && rotation == ROTATION_0)
				|| (TVRotate == CURRENT_TV_ROTATE_90R && rotation == ROTATION_270)
				|| (TVRotate == CURRENT_TV_ROTATE_90L && rotation == ROTATION_90))
			{
				m_lnvDev->encode(m_lnvDev,&m_JPEG_FileBuff1,&m_JPEG_Filesize1,LENOVO_ROTATE_0, (int64_t*)&m_ddTimeStampVid);
			}
			else if((TVRotate == CURRENT_TV_ROTATE_0 && rotation == ROTATION_270)
				|| (TVRotate == CURRENT_TV_ROTATE_90L && rotation == ROTATION_0))
			{
				m_lnvDev->encode(m_lnvDev,&m_JPEG_FileBuff1,&m_JPEG_Filesize1,LENOVO_ROTATE_90, (int64_t*)&m_ddTimeStampVid);
			}
			else if((TVRotate == CURRENT_TV_ROTATE_90R && rotation == ROTATION_90)
				||(TVRotate == CURRENT_TV_ROTATE_90L && rotation == ROTATION_270))
			{
				m_lnvDev->encode(m_lnvDev,&m_JPEG_FileBuff1,&m_JPEG_Filesize1,LENOVO_ROTATE_180, (int64_t*)&m_ddTimeStampVid);
			}
			else if((TVRotate == CURRENT_TV_ROTATE_0 && rotation == ROTATION_90)
				||(TVRotate == CURRENT_TV_ROTATE_90R && rotation == ROTATION_0))
			{
				m_lnvDev->encode(m_lnvDev,&m_JPEG_FileBuff1,&m_JPEG_Filesize1,LENOVO_ROTATE_270, (int64_t*)&m_ddTimeStampVid)	;
			}
			frames++;		
			m_cControl->SetCallingBuffIndex(CALLINGBUFFER1);
		}
		else
		{
			//LOGE("it is not copy completely!\n");
			usleep(2*NumOneThousand);
			continue;
		}
	}

	LOGI("quit LoopLNVVideo!\n");
	return S_OK;
}

/*****************************************************
* Func: LoopHSVideo
* Function: loop video capture for Hisense
* Parameters: void
*****************************************************/
int CWiMoVideo::LoopHSVideo()
{
	int frames = 0;
	DDword startTick = 0;
	DDword setRotateTime = 0;
	int firstFrame = 0;
	int ret = -1;
	int rotation = ROTATION_0;
	int TVRotate = CURRENT_TV_ROTATE_0;
	int settedRotation = -1;

#if ANDROID_RESOURCE_BUILDER
	pid_t tid  = gettid();
	LOGE("LoopHSVideo tid: %d\n", tid);
	set_sched_policy(tid, SP_FOREGROUND);
	setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_DISPLAY);
#endif
	setRotateTime = startTick = GetTickCount();
	while(m_cControl->GetStatusWiMo() == WIMO_STATUS_CONNECTING)
	{
		CountAndPrintFPS(frames, startTick);
		if(m_rotate_I && ((GetTickCount() - setRotateTime > NumFiveHundred) || firstFrame == 0))
		{
			rotation = m_cControl->GetRotateOrientation();
			TVRotate = m_cControl->GetTVRotateOrientation();
			if((TVRotate == CURRENT_TV_ROTATE_0 && rotation == ROTATION_0)
				|| (TVRotate == CURRENT_TV_ROTATE_90R && rotation == ROTATION_270)
				|| (TVRotate == CURRENT_TV_ROTATE_90L && rotation == ROTATION_90))
			{	
				(*m_rotate_I)(HS_ROTATE_0);
				settedRotation = HS_ROTATE_0;
			}
			else if((TVRotate == CURRENT_TV_ROTATE_0 && rotation == ROTATION_90)
				|| (TVRotate == CURRENT_TV_ROTATE_90R && rotation == ROTATION_0))
			{
				(*m_rotate_I)(HS_ROTATE_90);
				settedRotation = HS_ROTATE_90;
			}
			else if((TVRotate == CURRENT_TV_ROTATE_90R && rotation == ROTATION_90)
				||(TVRotate == CURRENT_TV_ROTATE_90L && rotation == ROTATION_270))
			{
				(*m_rotate_I)(HS_ROTATE_180);
				settedRotation = HS_ROTATE_180;
			}
			else if((TVRotate == CURRENT_TV_ROTATE_0 && rotation == ROTATION_270)
				||(TVRotate == CURRENT_TV_ROTATE_90L && rotation == ROTATION_0))
			{
				(*m_rotate_I)(HS_ROTATE_270);
				settedRotation = HS_ROTATE_270;
			}
			
			firstFrame++;
			setRotateTime = GetTickCount();
		}

		if(m_readJpegData_R && ((!m_bSendBuffFlag1 && m_cControl->GetCallingBuffIndex() == CALLINGBUFFER2)
			|| m_cControl->GetCallingBuffIndex() == CALLINGENTRANCEFIRSTLY))
		{
			if((ret = (*m_readJpegData_R)((Byte*)m_JPEG_FileBuff1, &m_JPEG_Filesize1, &m_ddTimeStampVid, &m_iHSRotate)) != 0)
				goto TO_SLEEP;
			
			if(m_iHSRotate != settedRotation)
			{
				LOGE("jpeg buffer1 rotate error,m_iHSRotate:%d, rotation:%d!\n", m_iHSRotate, rotation);
				continue;
			}
			frames++;
			m_bSendBuffFlag1 = true;
			m_bSendBuffFlag2 = false;
		}
		else if(m_readJpegData_R && !m_bSendBuffFlag2 && m_cControl->GetCallingBuffIndex() == CALLINGBUFFER1)
		{
			if((ret = (*m_readJpegData_R)((Byte*)m_JPEG_FileBuff2, &m_JPEG_Filesize2, &m_ddTimeStampVid, &m_iHSRotate)) != 0)
				goto TO_SLEEP;

			if(m_iHSRotate != settedRotation)
			{
				LOGE("jpeg buffer2 rotate error,m_iHSRotate: %d, rotation: %d!\n",m_iHSRotate, rotation);
				continue;
			}
			frames++;
			m_bSendBuffFlag1 = false;
			m_bSendBuffFlag2 = true;
		}

TO_SLEEP:		
		usleep(8*NumOneThousand);
	}

	LOGI("quit LoopHSVideo!\n");
	return S_OK;
}

/*****************************************************
* Func: LoopZTEVideo
* Function: loop video capture for ZTE
* Parameters: void
*****************************************************/
int CWiMoVideo::LoopZTEVideo()
{
	int ret = -1;
	int frames = 0;
	int currIndex = 0;
	int prevIndex = 0;
	DDword startTick = 0;
	DDword processVideoTime = 0;
	WIMOPREFIX prefix;
	CI_IMAGEPROC_SIZE srcSize = {0, 0};
	CI_SIZE yuvSrcSize = {0,0};
	IMGINFO ImgInfo;

#if ANDROID_RESOURCE_BUILDER
	pid_t tid  = gettid();
	LOGE("LoopZTEVideo tid: %d\n", tid);
	set_sched_policy(tid, SP_FOREGROUND);
	setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_DISPLAY);
#endif

	memset(&ImgInfo ,0 ,sizeof(IMGINFO));
	memset(&prefix, 0, sizeof(WIMOPREFIX));

	int width = (m_cControl->GetDeviceWidth() + 15)&~15;
	int height = (m_cControl->GetDeviceHeight() + 15)&~15;
	
	ImgInfo.pDst[0] = (unsigned char *)malloc(width*height*4);	
	if(!ImgInfo.pDst[0])
	{
		LOGE("failed malloc pDst[0] \n");
	}
	ImgInfo.pDst[1] = ImgInfo.pDst[0] + width*height;
	ImgInfo.pDst[2] = ImgInfo.pDst[1] + width*height/4;

	OpenImgProcessLib();
	
	processVideoTime = startTick = GetTickCount();
	while(m_cControl->GetStatusWiMo() == WIMO_STATUS_CONNECTING)
	{
		CountAndPrintFPS(frames, startTick);

		memcpy(&prefix, m_pZTEmap, sizeof(prefix));
		yuvSrcSize.width = m_iAlignedWidth = prefix.header.width;
		yuvSrcSize.height = m_iAlignedHeight = prefix.header.height;
		currIndex = prefix.index;
		ImgInfo.format =  prefix.header.fmt;
		ImgInfo.dstStride[0] = prefix.stride;
		ImgInfo.dstStride[1] = ImgInfo.dstStride[0]>>1;
		ImgInfo.dstStride[2] = ImgInfo.dstStride[0]>>1;

		if(currIndex > prevIndex || (currIndex == 0 && prevIndex == MAXINTNUM))
		{
			//LOGI("prevcount: %d, curr count: %d\n", prevIndex, currIndex);
			prevIndex = currIndex;
			memcpy(ImgInfo.pDst[0], m_pZTEmap + sizeof(prefix), m_pMapSize - sizeof(prefix));
			ImgInfo.pDst[1] = ImgInfo.pDst[0] + m_iAlignedWidth*m_iAlignedHeight;
			ImgInfo.pDst[2] = ImgInfo.pDst[1] + m_iAlignedWidth*m_iAlignedHeight/4;
	
			ret = YUV2JPG(&ImgInfo, yuvSrcSize);
			if(ret == S_CONTINUE)
				continue;
			else if(ret == E_FAILED)
			{
				LOGE("Jpeg enc frame failure!\n");
				break;
			}
			frames++;
		}
	}

	CloseLib();
	
	if(ImgInfo.pDst[0])
	{
		free(ImgInfo.pDst[0]);
		ImgInfo.pDst[0] = NULL;
	}

	m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);

	LOGI("quit LoopZTEVideo!\n");	
	return S_OK;
}

