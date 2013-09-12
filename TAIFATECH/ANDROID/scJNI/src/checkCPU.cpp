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

#include "checkCPU.h"

using namespace android;

/*****************************************************
* Func: CCPUControl
* Function: consturction function
* Parameters: void
*****************************************************/
CCPUControl::CCPUControl(CWiMoControl* pWimo)
{
	m_pWimo = pWimo;
	currCPUUtilization = 0;
	maxFrames = 0;
	saveTimesOfOverloadCPU = 0;
	memset(&oldCPU, 0, sizeof(CPUInfo));
	memset(&newCPU, 0, sizeof(CPUInfo));
	memset(&UtilizationOfCPU, 0, sizeof(UtilizationOfCPU));

	if(m_pWimo->GetLimitedFPS())
		mVal = MAX_TARGET_FPS_5;
	else
		mVal = MAX_TARGET_FPS_30;
	
	targetFrames = mVal;
	saveLastestFrames = mVal/2;
}

/*****************************************************
* Func: ~CCPUControl
* Function: deconsturction function
* Parameters: void
*****************************************************/
CCPUControl::~CCPUControl()
{

}

/*****************************************************
* Func: UpdateFramesAccordingToCPU
* Function: update frames according to CPU utilization
* Parameters: void
*****************************************************/
void CCPUControl::UpdateFramesAccordingToCPU(void)
{
	if(currCPUUtilization <= 90 && currCPUUtilization >= 85)
	{
		saveTimesOfOverloadCPU = 0;
		saveLastestFrames = targetFrames;
	}
	else if(currCPUUtilization > 90)
	{
		saveTimesOfOverloadCPU++;
		if(saveTimesOfOverloadCPU < 2)
			targetFrames = saveLastestFrames;
		else
			targetFrames = targetFrames/2;
	}
	else if(currCPUUtilization < 85 && currCPUUtilization >= 65)
	{
		saveTimesOfOverloadCPU = 0;
		saveLastestFrames = targetFrames;
		targetFrames = targetFrames+1;
	}
	else
	{
		saveTimesOfOverloadCPU = 0;
		saveLastestFrames = targetFrames;
		targetFrames = targetFrames+2;
	}

	if(targetFrames > mVal || maxFrames > mVal)
		targetFrames = mVal;
#if OUTLOG
	LOGI("saveLastestFrames:%d,targetFrames :%d\n",saveLastestFrames,targetFrames);
#endif
}
	
/*****************************************************
* Func: LoopCheckCPUUtilization
* Function: check CPU utilization right now
* Parameters: 
*	param: thread pointer
*****************************************************/
void* CCPUControl::LoopCheckCPUUtilization(void* param)
{
	CCPUControl* pthis = (CCPUControl*)param;

	long usrTime, sysTime, idleTime,irqTime;
	long unsigned total_delta_time;

	DDword tick = 0;
	DDword sleepTick = 0;
	while(pthis->m_pWimo != NULL && pthis->m_pWimo->GetStatusWiMo() != WIMO_STATUS_FINISH)
	{
		tick = GetTickCount();
		pthis->UtilizationOfCPU[0] = pthis->UtilizationOfCPU[1];
		
		FILE* file = fopen("/proc/stat", "r");
		if (!file)
		{
			LOGE("Could not open /proc/stat.\n");
			if(pthis->m_pWimo != NULL)
				pthis->m_pWimo->SetStatusWiMo(WIMO_STATUS_FINISH);
		}
		int count = fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu", &pthis->newCPU.utime,
			&pthis->newCPU.ntime, &pthis->newCPU.stime,&pthis->newCPU.itime, 
			&pthis->newCPU.iowtime, &pthis->newCPU.irqtime, &pthis->newCPU.sirqtime);
		fclose(file);

		usrTime = (pthis->newCPU.utime + pthis->newCPU.ntime)
				- (pthis->oldCPU.utime + pthis->oldCPU.ntime);
		sysTime = pthis->newCPU.stime - pthis->oldCPU.stime;
		idleTime = pthis->newCPU.itime - pthis->oldCPU.itime;
		irqTime = (pthis->newCPU.iowtime + pthis->newCPU.irqtime + pthis->newCPU.sirqtime)
			-(pthis->oldCPU.iowtime + pthis->oldCPU.irqtime + pthis->oldCPU.sirqtime);
		
		total_delta_time = usrTime + sysTime + idleTime + irqTime;
		
		pthis->currCPUUtilization = (unsigned int)(100 * (usrTime + sysTime + irqTime) / total_delta_time);

		if(pthis->currCPUUtilization == 0)
			pthis->currCPUUtilization = pthis->UtilizationOfCPU[0];

#if OUTLOG
		LOGI("cpu utilization: %d%%\n", pthis->currCPUUtilization);
#endif
		pthis->UtilizationOfCPU[1] = pthis->currCPUUtilization;
		memcpy(&pthis->oldCPU, &pthis->newCPU, sizeof(pthis->oldCPU));

		pthis->UpdateFramesAccordingToCPU();

		sleepTick = GetTickCount()- tick;
		if(sleepTick < NumOneThousand/CPU_UPDATE_FREQ)
		{
			usleep((NumOneThousand/CPU_UPDATE_FREQ - sleepTick)*NumOneThousand);
		}		
		else
		{
			LOGE("check CPU error !!!");
		}
	}

	return 0;
}

