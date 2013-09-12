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

#include "capture_type.h"

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************
* Func: CreateThreadHelper
* Function: creat a thread
* Parameters:
*		PFunc: function pointer
*		SockProc: function name
*****************************************************/
pthread_t CreateThreadHelper(PFunc SockProc, void* pthis)
{
	pthread_t pid = 0;
	if(SockProc == NULL)
		return S_OK;
	int err = 0;
	struct sched_param schedRecv;
	pthread_attr_t attrRecv;
	pthread_attr_init(&attrRecv);
	err = pthread_create(&pid, &attrRecv, SockProc, pthis);
	if(err != 0)
	{
		LOGE("create thread failed!\n");
		return E_THREAD_CREATE;
	}
	schedRecv.sched_priority = 90;
	pthread_setschedparam(pid, SCHED_RR, &schedRecv);

	return pid;
}
#ifdef __cplusplus
}
#endif
