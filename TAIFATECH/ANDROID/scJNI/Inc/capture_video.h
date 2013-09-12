#ifndef __CAPTURE_VIDEO_H__
#define __CAPTURE_VIDEO_H__

#include "ci_codec_type.h"
#include "ci_imageproc.h"
#include "ci_jpegenc.h"
#include "capture_type.h"
#include "capture_control.h"
#include "LenovoInterface.h"

#define MAX_TARGET_FPS_30	30
#define MAX_TARGET_FPS_5	5
#define MINTIMESLEEP 2	// 2miniSec
#define JPEGBUFFERSIZE 1024*1024*1

namespace android{

typedef struct _IMGINFO
{
	int 								format; 			//0 :yuv420, 1:rgb565, 2:rgb, 3:argb
	int 								srcWidth;
	int 								srcHeight;
	int 								dstWidth;
	int 								dstHeight;
	unsigned char 						*pSrc[3];
	unsigned char 						*pDst[3];
	CI_U32 							srcStride[3];
	CI_U32 							dstStride[3];
	int 								rotation;
	int 								ImgFmtIn;
	int 								ImgFmtOut;
	int								interpolation;
}IMGINFO;

class CWiMoControl;
class HWJpegEnc;
class CWiMoVideo
{
public:
	CWiMoVideo(CWiMoControl* pis, char* libPath);
	~CWiMoVideo();

	char* GetJPEGData(int *jpegSize);
	static void * LoopThread(void * param);
private:
	void Init();
	void UnInit();

	int HSInit();
	void HSUnInit();
	int ZTEInit();
	void ZTEUnInit();
	int LNVInit();
	void LNVUnInit();
	int QiYiGameInit(void);
	void QiYiGameUnInit(void);
	int ReadFB0Init();
	void ReadFB0UnInit();
	int ReadScreenshotInit();
	void ReadScreenshotUnInit();

	static void *LNVInitInThread(void * param);
	static void *InitScreenshotThread(void * param);

	void CountOffsetSize();
	int InitImgInfoHelper(IMGINFO *pImgInfo, unsigned char **pDst);
	int RGB2YUV(IMGINFO *pImgInfo, CI_IMAGEPROC_SIZE srcSize,CI_IMAGEPROC_SIZE dstSize);
	int YUV2JPG(IMGINFO *pImgInfo, CI_SIZE srcSize);
	void ChangeAttribute(IMGINFO *pImgInfo, CI_SIZE &yuvSrcSize, CI_IMAGEPROC_SIZE &srcSize, CI_IMAGEPROC_SIZE &dstSize);
	int ScreenshotUpdataInfo(IMGINFO *pImgInfo);
	int FramBufferUpdataInfo(IMGINFO *pImgInfo);
	int ProcessVideo(IMGINFO *pImgInfo);
	void CountAndPrintFPS(int &frames, DDword &startTick);
	int SetAverageSleep(DDword processVideoTime, int &avgSleep);
	int VideoCapCheckInterface(void);
	int LoopSWVideo(void);
	int loadLNVLib(const char *id, const char *path, const struct hwjepgapi_module_t **pHmi);
	int LoopLNVVideo(void);
	int LoopHSVideo(void);
	int LoopZTEVideo(void);

	int CheckSupportNeon();
	int LoadLibFunc();
	int OpenImgProcessLib();
	void CloseLib();
		
	CWiMoControl* m_cControl;
	HWJpegEnc *m_mJpegEnc;	
private:
	struct fb_fix_screeninfo m_fbfinfo;
	struct fb_var_screeninfo m_fbvinfo;
	
	pF_ppVOID_pVOID_pCI_IMAGEPROC m_CI_IMGPROC_Create;
	pF_pVOID_ppU8_pU32_ppU8_pU32_pCI_IMAGEPROC m_CI_IMGPROC_ProcessFrame;
	pF_pVOID m_CI_IMGPROC_Destroy;

	pF_ppVOID_pJPGENC m_CI_JpgEnc_Open;
	pF_pVOID_ppU8_pU32_ciSize_pU8_pU32 m_CI_JpgEnc_Frame;
	pF_pVOID m_CI_JpgEnc_Close;
	
	jpeg_hw_device_t  *m_lnvDev;
	void* m_pdl;
	void* m_imagePdl;
	void* m_encodePdl;
	pF_INT m_rotate_I;
	pF_pINT_pINT m_getScreenDimemsion_pIpI;
	pF_pUCHAR_pINT_pINT64_pINT m_readJpegData_R;

	char* m_pAppLibPath;
	Byte* m_iHndFpBuff;
	Byte* m_screenshotBuff;
	char* m_JPEG_FileBuff1;
	char* m_JPEG_FileBuff2;
	int m_iFirstImageSize;
	int m_iMmapSize;
	int m_JPEG_Filesize1;
	int m_JPEG_Filesize2;

	Byte* m_pZTEmap;
	long m_pMapSize;
	bool m_bWriteRead; // true: write finished, false: read finished
	bool m_bSyncCapEncZTE;
	bool m_bSendBuffFlag1;
	bool m_bSendBuffFlag2;
	bool m_bNeon;
	bool m_bArmv7orUP;
	bool m_bRevertUVPlanes;
	bool m_bOffset;

	int m_iHndFD;
	int m_shotFD;	
	int m_averageCostTimeEveryFrame;
	int m_sumOverTimeOfVideoProcess;

	int m_iAlignedWidth;
	int m_iAlignedHeight;
	int m_gapValueW;
	int m_gapValueH;
	int m_offsetSize;
	int m_sStride;
	int m_iHSRotate;
	int m_currUpdate;
	int m_prevUpdate;
	int m_mTargetFPS;
	
	DDword m_ddTimeStampVid;
	CI_SCRSHOT_HEADER m_sshotHeader;
};

};
#endif

