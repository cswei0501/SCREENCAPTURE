#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "dp.h"
#include "ci_jpegenc.h"

long long GetCurrTime()
{
    struct timeval my_timeval;
    if (gettimeofday(&my_timeval, NULL))
        return 0;
    return (long long)(my_timeval.tv_sec * 1000) + (my_timeval.tv_usec / 1000);
}
int main(int argc, char **argv)
{

	CI_SIZE srcSize;
	CI_JPGENCOPENOPTIONS OpenOptions;
	CI_VOID *pEncoder = 0;
	CI_RESULT ret = 0;

	FILE* pFile = 0;
	pFile = fopen("/sdcard/544x960.yuv", "rb");
	if(!pFile)
	{
		LOGE("failed to open file\n");
		return -1;
	}
	LOGE("start test %d \n",__LINE__);
	unsigned char *m_JPEG_FileBuff = NULL;
	unsigned char *srcBufferArray[3];
	CI_U32 srcStride[3];
	unsigned char* pDataBuffer = 0;
	memset(&srcStride, 0 ,sizeof(srcStride));
	
	memset(&srcSize, 0 ,sizeof(srcSize));
	srcSize.width = 544;
	srcSize.height = 960;

	srcStride[0] = srcSize.width;
	srcStride[1] = srcSize.width/2;
	srcStride[2] = srcSize.width/2;


	LOGE("start test %d \n",__LINE__);
	pDataBuffer = malloc(srcSize.width*srcSize.height*3/2);
	if(!pDataBuffer)
		return -1;

	LOGE("start test %d \n",__LINE__);
	m_JPEG_FileBuff  = malloc(srcSize.width*srcSize.height*3/2);
	if(!m_JPEG_FileBuff )
		return -1;

	fread(pDataBuffer, 1, srcSize.width*srcSize.height*3/2, pFile);
	fclose(pFile);
	srcBufferArray[0] = pDataBuffer ;
	srcBufferArray[1] = pDataBuffer +srcSize.width*srcSize.height;
	srcBufferArray[2] = pDataBuffer +srcSize.width*srcSize.height*5/4;
	
	
	memset(&OpenOptions, 0 ,sizeof(OpenOptions));

	OpenOptions.u8OutType = CI_OUT_JPEG;
	OpenOptions.u8YUVType = CI_YUV420_PLANAR ;
	OpenOptions.u8Quality = CI_QUALITY_LEVEL4;
	ret = CI_JPGENC_Open(&pEncoder, &OpenOptions);
	if(ret != CI_SOK)
	{
		LOGE("enc initial failed!\n");
		return -1;
	}
	CI_U32	m_JPEG_Filesize = 0;

	long frameCnt = 0;
	long long startTick = GetCurrTime();

	long evaluateFrameCnt = 1000;
	while(1)
	{
		ret = CI_JPGENC_Frame(pEncoder, srcBufferArray, srcStride, srcSize, 
			(CI_U8*)m_JPEG_FileBuff, (CI_U32*)&m_JPEG_Filesize);
		if(ret != CI_SOK)
		{
			LOGE("enc enc failed!ret: %d\n", ret);
			return -1;
		}

		frameCnt++;
		if(frameCnt >evaluateFrameCnt)
			break;
	}


	FILE* pFile2 = 0;
//	pFile2 = fopen("/sdcard/544x960.jpg", "wb");

//	fwrite(m_JPEG_FileBuff, 1,m_JPEG_Filesize, pFile2 );
	
	long long endTick = GetCurrTime();

	LOGE("end time %lld \n",endTick );
	LOGE("fps:%lld \n", evaluateFrameCnt*1000/(endTick-startTick) );
	CI_JPGENC_Close(pEncoder);

	if(pFile)
		fclose(pFile);
	if(pFile2)
		fclose(pFile2);

	if(m_JPEG_FileBuff)
		free(m_JPEG_FileBuff);
	if(pDataBuffer)
		free(pDataBuffer);


	return 0;
}
