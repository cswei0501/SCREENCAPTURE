#ifndef __CAPTURE_CHECK_CPU_H__
#define __CAPTURE_CHECK_CPU_H__

#include "capture_type.h"
#include "capture_control.h"

namespace android{

#define CPU_UPDATE_FREQ	2

typedef struct CPUInfo
{
	long unsigned 						utime;
	long unsigned 						ntime;
	long unsigned 						stime;
	long unsigned 						itime;
	long unsigned 						iowtime;
	long unsigned 						irqtime;
	long unsigned 						sirqtime;
}TF_CPUInfo;

class CWiMoControl;
class CCPUControl
{
public:
	CCPUControl(CWiMoControl* m_status);
	~CCPUControl();
	inline void SetTargetFPS(int fps){targetFrames = fps;}
	inline int GetTargetFPS(void){return targetFrames;}
	inline void SetMaxFPS(int fps){maxFrames = fps;}
	inline int GetMaxFPS(void){return maxFrames;}
	void UpdateFramesAccordingToCPU();
	static void* LoopCheckCPUUtilization(void* param);
private:
	TF_CPUInfo oldCPU, newCPU;
	Byte UtilizationOfCPU[2];
	int frames;
	int maxFrames;
	int targetFrames;
	unsigned int currCPUUtilization;
	int saveLastestFrames;
	int saveTimesOfOverloadCPU;
	int mVal;

	CWiMoControl* m_pWimo;
};

};
#endif
