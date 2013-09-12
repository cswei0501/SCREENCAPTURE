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

#define LOG_NDEBUG 0
#define LOG_TAG "Screen_capture"

#include <arpa/inet.h>
#include <net/if_arp.h> 
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/sockios.h>
#include <linux/if.h> 
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ANDROID_RESOURCE_BUILDER
#include <utils/Log.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IMemory.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <binder/Permission.h>
#include <SkImageEncoder.h>
#include <SkBitmap.h>
#include <SkStream.h>
#include <JNIHelp.h>
#include <media/AudioRecord.h>
#include <media/mediarecorder.h>
using namespace android;
#endif

#include "cidana_screencap_Capture.h"
#include "ci_codec_type.h"
#include "ci_imageproc.h"
#include "ci_jpegenc.h"

#ifdef NDK_BUILDER
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "Screen_capture", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "Screen_capture", __VA_ARGS__))
#endif

typedef char							Char;			///< char
typedef unsigned char					Byte;			///< unsigned char
typedef void* 						LPVOID;
typedef unsigned int					Dword;		///< unsigned int
typedef unsigned short					Word;			///< unsigned short

#include "android_header.h"
#include "apusone_header.h"
#include "taifa_header.h"

/******************   GLOBLE VARIABLE START ****************/

static JAVA_CALLBACK_FUN jCallbackFun;
typedef void* (*PFunc)(void*);
void Create_Pthread(PFunc SockProc);

//SkBitmap b;
bool g_connectedNetwork = false;
int m_iHndFB  = 0;
int m_iHndFBSize = 0;
unsigned char* m_iHndFpBuff = 0;
int m_JPEG_Filesize = 0;
unsigned char *m_JPEG_FileBuff = NULL;
char* pAudioBuffer = 0;
struct fb_fix_screeninfo m_fbfinfo;
static bool g_Support_FB = 0;
static bool  g_Support_SDK = 0;
struct fb_var_screeninfo m_fbvinfo;
struct sockaddr_in g_SenderAddr;

// wish to frame size
 long DEST_WIDTH = 0;
 long DEST_HEIGHT = 0;

long SRC_WIDTH = 0;
long SRC_HEIGHT = 0;
static const long SCREEN_WIDTH = 480;
static const long SCREEN_HEIGHT = 800;

static long t_outtime = 0;
static int g_rotate = eROTATION_0;
#ifdef TAIFA_TECH
static const int g_self_rotate = true;
#else
static const int g_self_rotate = false;
#endif
static const int connectionWimoSuccess = 1;
static const int connectionWimoFail = 0;

bool g_bUpdated = false;
bool g_dispEnable = false;
bool g_bCheckCPUUpdate = false;
unsigned int g_CPUUtilization = 0;
static int g_saveLastestFrames = 0;
static int g_saveTimesOfOverloadCPU = 0;
int streamHandle = 0;
int dgramHandle = 0;
char* pTXbuffer =0 ;
static Byte 						IOCtrlDstIP[4];
static Byte 						IOCtrlDstMac[6];
static int							m_TAIFA_IOCtrlStep = 1;

//#ifdef TAIFA_TECH

static Byte						IOCtrlSupportList[] = 
{0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static BroadcastPacket				IOCtrlBrucastPacketCmd;
static CommnadPacket				IOCtrlSendPacketCmd;
static SessionPacket				IOCtrlSendSessionCmd;
static Recv_Command				IOCtrlRecvPacketCmd;
static ConnectingPassword			IOCtrlRecvConnectCmd;
static Protocol_Connect			IOCtrlSendConnectCmd;
static SourceStatus				IOCtrlSendSourceCmd;
int 	m_nResolutionTicks			= 0x00001234;

#define SERVERADDR  inet_addr("192.168.168.56")
struct sockaddr_in server;
int len = sizeof(sockaddr_in);
//#endif

//#ifdef APUSONE

const int DeviceType_IATV				= (0x01 << 1) | 1;
const int DeviceType_IATVCtrl			= (0x03 << 1);
const int DeviceStatusReportCmd		= 0x0002;
const int MagicNumber				= 0x5678;
const int StartCaptureScreenCmd		= 0x0016;
const int EndCaptureScreenCmd		= 0x0017;
const int UpdateCaptureScreenCmd	= 0x0018; 

const int BroadcastBeginPort    = 6180;
const int EchoCmd                         = 0x001a;
const int EchoRespCmd                     = 0x001b;
const int StartCaptureScreenExCmd = 0x0090;
const int UpdateCaptureScreenExCmd = 0x0091;

//#endif

struct cpu_info {
    long unsigned utime, ntime, stime, itime;
    long unsigned iowtime, irqtime, sirqtime;
};
static struct cpu_info old_cpu, new_cpu;

/******************   GLOBLE VARIABLE END ****************/

long GetTickCount(void)
{
	struct timeval my_timeval;
	if (gettimeofday(&my_timeval, NULL))
		return 0;
	return (long)(my_timeval.tv_sec * 1000) + (my_timeval.tv_usec / 1000);
    
}

void CaptureCallBack(int connectionFlag)
{
	bool isAttached = false;	
	int status = 0;
	
	status =jCallbackFun.jvm->GetEnv((void **) &jCallbackFun.env, JNI_VERSION_1_4);
	if(status< 0)
		{
		LOGE("callback_handler: failed to get JNI environment, "
			"assuming native thread");
		status =jCallbackFun.jvm->AttachCurrentThread(&jCallbackFun.env, NULL);
		if(status< 0)
			{
			LOGE("callback_handler: failed to attach "
				 "current thread");
			return;
		}
		isAttached = true;
	}
	if(connectionFlag)
		LOGI("Connect to WiMo SUCCESSFULLY!\n");
	else
		LOGE("Connect WiMo FAILED!\n");
	//LOGE("env: %p, obj: %p, callback: %p\n", jCallbackFun.env, jCallbackFun.obj, jCallbackFun.callbackFuns);
	jCallbackFun.env->CallIntMethod(jCallbackFun.obj, jCallbackFun.callbackFuns,connectionFlag);
	if (isAttached)
		jCallbackFun.jvm->DetachCurrentThread();
	
}
	
int SockDatagramInit()
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef TAIFA_TECH	
	addr.sin_port = htons(UDP_PORT);
	dgramHandle=socket(AF_INET,SOCK_DGRAM,0);
	setsockopt(dgramHandle, SOL_SOCKET, SO_REUSEADDR, (const struct sockaddr *)&addr, sizeof(&addr));

	memset(&server, 0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(UDP_PORT); ///server的监听端口
	server.sin_addr.s_addr=SERVERADDR; 
#elif defined(APUSONE)
	addr.sin_port = htons(BroadcastBeginPort);
	int fbroadcast = 1;
	dgramHandle =(int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt ( dgramHandle,SOL_SOCKET,SO_BROADCAST,(char *)&fbroadcast,sizeof ( int ));

	int optval = 1;
	setsockopt(dgramHandle,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(int));
#endif

	if(bind(dgramHandle, (const struct sockaddr *)&addr, sizeof(addr)) !=0)
	{
		close(dgramHandle);
		dgramHandle = 0;
		return -1;
	}
	
	return 0;
}

void APUS_SockDatagramReport()
{
	int active = 1;
	char buffer[256] = {0};
	DeviceStatusReport *ptr = (DeviceStatusReport *)buffer;
	
	ptr->type = htonl(DeviceStatusReportCmd);
	ptr->MagicNumber = htonl(MagicNumber);
	ptr->packet_size = htonl(sizeof(DeviceStatusReport));
	ptr->DeviceType = htonl(DeviceType_IATVCtrl);
	ptr->DeviceStatus = htonl(active);
	gethostname(ptr->devicename, 128);

	struct sockaddr_in saudpserv;
	saudpserv.sin_family = AF_INET;
	saudpserv.sin_addr.s_addr = htonl ( INADDR_BROADCAST );
	saudpserv.sin_port = htons (BroadcastBeginPort);
	sendto(dgramHandle, buffer, sizeof(DeviceStatusReport), 0,
		(const struct sockaddr*)&saudpserv, sizeof(struct sockaddr_in));
}

long long GetIPAddress(unsigned char* pMac)
{ 
	#define MAXINTERFACES 10
	register int fd, intrface, retn = 0; 
	struct ifreq buf[MAXINTERFACES]; 
	struct arpreq arp; 
	struct ifconf ifc; 
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) 
	{ 
		ifc.ifc_len = sizeof buf; 
		ifc.ifc_buf = (caddr_t) buf; 
		if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) 
		{ 
			//获取接口信息
			intrface = ifc.ifc_len / sizeof (struct ifreq); 
			LOGE("interface num is intrface=%d\n\n\n",intrface); 
			//根据借口信息循环获取设备IP和MAC地址
			while (intrface-- > 0) 
			{ 
				LOGE("buf[%d].ifr_name:%s \n",intrface, buf[intrface].ifr_name);
				//  if(strcmp(buf[intrface].ifr_name,"eth0"))
				//    continue;
				//获取设备名称
				LOGE ("net device %s\n", buf[intrface].ifr_name); 

				//判断网卡类型 
				if (!(ioctl (fd, SIOCGIFFLAGS, (char *) &buf[intrface]))) 
				{ 
					if (buf[intrface].ifr_flags & IFF_PROMISC) 
					{ 
						puts ("the interface is PROMISC" ); 
						retn++; 
					} 
				} 
				else 
				{ 
					char str[256]; 
					sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name); 
					perror (str); 
				} 
				//判断网卡状态 
				if (buf[intrface].ifr_flags & IFF_UP) 
				{ 
					puts("the interface status is UP" ); 
				} 
				else 
				{ 
					puts("the interface status is DOWN" ); 
				} 
				//获取当前网卡的IP地址 
				if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface]))) 
				{ 
					in_addr addr = ((((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
					puts ("IP address is:" ); 
					LOGE("%d.%d.%d.%d\n", 
						addr.s_addr&0xff,
						addr.s_addr>>8&0xff,
						addr.s_addr>>16&0xff,
						addr.s_addr>>24&0xff);

					if (!(ioctl (fd, SIOCGIFHWADDR, (char *) &buf[intrface]))) 
					{ 
						puts ("HW address is:" ); 
						LOGE("%02x:%02x:%02x:%02x:%02x:%02x\n", 
							(unsigned char)buf[intrface].ifr_hwaddr.sa_data[0], 
							(unsigned char)buf[intrface].ifr_hwaddr.sa_data[1], 
							(unsigned char)buf[intrface].ifr_hwaddr.sa_data[2], 
							(unsigned char)buf[intrface].ifr_hwaddr.sa_data[3], 
							(unsigned char)buf[intrface].ifr_hwaddr.sa_data[4], 
							(unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]); 
						if(pMac)
						{
							for(int i=0;i<6;i++)
								pMac[i] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[i];

						}
						puts("" ); 
						puts("" ); 
					} 
					else
						puts("failed to get mac address ");
					close (fd); 
					return addr.s_addr; 
				} 
			}    
		}
	} //while
	return 0; 
}

int AndroidInit()
{
	long long ipaddr = 0;
	ipaddr  = GetIPAddress(IOCtrlDstMac);
	if(!(ipaddr))
	{
		LOGE("failed to GetIPAddress() \n");
		return -1;
	}
	LOGE("GetIPAddress() success \n");

	IOCtrlDstIP[0] = ipaddr &0xff;//192;
	IOCtrlDstIP[1] = ipaddr>>8 &0xff;
	IOCtrlDstIP[2] = ipaddr>>16 &0xff;
	IOCtrlDstIP[3] = ipaddr>>24 &0xff;
	
	char cDevNode[128];
	sprintf(cDevNode,"/dev/graphics/fb0");
	LOGE("test %s \n",cDevNode);
	m_iHndFB = open(cDevNode, O_RDONLY);

	if(m_iHndFB < 0)
	{
		LOGE("/dev/graphics/fb0 open error\n");
		return -1;
	}

	int ret = -1;
	ret = ioctl(m_iHndFB, FBIOGET_VSCREENINFO, &m_fbvinfo);
	if(ret)
	{
		LOGE("ioctl(..., FBIOGET_VSCREENINFO, ...) on framebuffer error %x\n", ret);
		return -1;
	}

	ret = ioctl(m_iHndFB, FBIOGET_FSCREENINFO, &m_fbfinfo);
	if(ret)
	{
		LOGE("ioctl(..., FBIOGET_FSCREENINFO, ...) on framebuffer error %x\n", ret);
		return -1;
	}

	m_iHndFBSize = m_fbvinfo.xres * m_fbvinfo.bits_per_pixel * m_fbvinfo.yres / 8;

	DEST_WIDTH = SRC_WIDTH = m_fbvinfo.xres;
	DEST_HEIGHT = SRC_HEIGHT = m_fbvinfo.yres;

	LOGE("m_fbvinfo.xres:%d ,y:%d bye: %d\n",m_fbvinfo.xres,m_fbvinfo.yres, m_fbvinfo.bits_per_pixel);
	m_iHndFpBuff = (unsigned char *)mmap(0, m_iHndFBSize, PROT_READ, MAP_SHARED, m_iHndFB, 0);
	if((void*)-1 == m_iHndFpBuff)
	{
		LOGE("mmap framebuffer error\n");
		return -1;
	}

	SetVideoRotate(g_rotate);
	return 0;
}

void* APUSONEThread(void * param)
{
	char recvbuff[256];

	while(g_dispEnable)
	{
		APUS_SockDatagramReport();
		usleep(500*1000);

		memset(recvbuff,0,sizeof(recvbuff));
		int len = APUSRecvPacket(recvbuff,sizeof(recvbuff));
		if(len == -1)
			return NULL;
		if(len == 0)	//not received ,try again
			continue;

		if(APUS_CheckAttributeOfDeviceStatus(recvbuff, len) == true)
		{
			//Dword id;
			DeviceStatusReport *ptr = (DeviceStatusReport *)recvbuff;
			g_SenderAddr.sin_port = htonl(ptr->port);
			LOGI("g_SenderAddr.sin_port: %d \n", g_SenderAddr.sin_port);

			int ret;
			if((ret = APUS_SockStreamInit()) != 0)
				return NULL;

			APUS_SendStartCapScreenFunc();

			Create_Pthread(SockStreamProc);

			break;
		}
		
		if(APUS_CheckAttributeOfEcho(recvbuff, len) == true)
		{
			LOGE("error!!!! %d \n", __LINE__);
/*
			//SendEchoType(recvbuff);
			Echo *pm = (Echo*)recvbuff;
			struct sockaddr_in saudpserv;
			saudpserv.sin_family = AF_INET;
			saudpserv.sin_addr.s_addr = htonl(g_SenderAddr.sin_addr.s_addr);
			saudpserv.sin_port = htons(g_SenderAddr.sin_port);
			pm->type = htonl(EchoRespCmd);
			sendto(dgramHandle, recvbuff, sizeof(Echo), 0, (const struct sockaddr*)&saudpserv, sizeof(struct sockaddr_in) );
*/
			break;
		}
	}
	return 0;
}

void * TAIFAResponseThread(void * param)
{
	struct timeval cmdStartTick;
	size_t n;

	while(g_dispEnable)
	{
		if(m_TAIFA_IOCtrlStep == 1) //OPCODE_TF6x0_DISCOVER_AND_RESET , wait for OPCODE_TF6x0_DISCOVER
		{
			gettimeofday(&cmdStartTick, 0);

			InitConnectPacket initconnect;
			initconnect.Header.ProductID 		= htons(Product);
			initconnect.Header.RandCode			= htons(0x00);
			initconnect.Header.SeqID				= htons(0x00);
			initconnect.Command.CmdType			= htons(0x0001);
			initconnect.Command.OPCode			= htons(0x0103);
			initconnect.Command.DataLen			= htons(0x0020);
			memcpy(initconnect.Header.Signature, "TF6z", sizeof(initconnect.Header.Signature));

			n = sendto(dgramHandle,&initconnect,sizeof(initconnect),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}

			m_TAIFA_IOCtrlStep++;
			LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);

		}
		else if(m_TAIFA_IOCtrlStep == 3)  //OPCODE_TF6x0_DISCOVER_AND_RESET
		{
			static Byte 	IOCtrlHeader[6]	= {0xc0, 0x00, 0xc0, 0x03, 0x00, 0x92};
			static Byte 	IOCtrlMachineNameID[12] = {0x50, 0x43, 0x32, 0x54, 0x56, 0x2d, 0x50, 0x43, 0x41, 0x00, 0x00, 0x00};

			memset(&IOCtrlSendPacketCmd, 0 ,sizeof(IOCtrlSendPacketCmd));
			IOCtrlSendPacketCmd.SeqNum	= IOCtrlBrucastPacketCmd.TF_HeaderPacket.SeqID;
			IOCtrlSendPacketCmd.TF_HeaderPacket.ProductID 	= htons(Product);
			IOCtrlSendPacketCmd.TF_HeaderPacket.RandCode		= htons(0x00);
			IOCtrlSendPacketCmd.TF_HeaderPacket.SeqID				= htons(0x00);
			IOCtrlSendPacketCmd.CmdTF.DataLen								= htons(0x0020);
			IOCtrlSendPacketCmd.CmdTF.CmdType								= htons(0x0001);
			IOCtrlSendPacketCmd.CmdTF.OPCode								= htons(0x0103);
			memcpy(IOCtrlSendPacketCmd.TF_HeaderPacket.Signature, "TF6z", sizeof(IOCtrlSendPacketCmd.TF_HeaderPacket.Signature));
			memcpy(IOCtrlSendPacketCmd.IOCtrlHeader, IOCtrlHeader, sizeof(IOCtrlSendPacketCmd.IOCtrlHeader));
			memcpy(IOCtrlSendPacketCmd.MachineName, IOCtrlMachineNameID, sizeof(IOCtrlSendPacketCmd.MachineName));

			n = sendto(dgramHandle,&IOCtrlSendPacketCmd,sizeof(IOCtrlSendPacketCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}

			IOCtrlSendPacketCmd.TF_HeaderPacket.SeqID			+= htons(0x01); 
			IOCtrlSendPacketCmd.TF_HeaderPacket.RandCode	 = IOCtrlSendPacketCmd.TF_HeaderPacket.SeqID;
			n = sendto(dgramHandle,&IOCtrlSendPacketCmd,sizeof(IOCtrlSendPacketCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}

			gettimeofday(&cmdStartTick, 0);
			m_TAIFA_IOCtrlStep++;
			LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
		}
		else if(m_TAIFA_IOCtrlStep == 4)  //OPCODE_TF6x0_CONNECTING, wait for  OPCODE_TF6x0_CONNECTING
		{
			memset(&IOCtrlSendSessionCmd, 0, sizeof(IOCtrlSendSessionCmd));
			IOCtrlSendSessionCmd.ReplySeqID = IOCtrlBrucastPacketCmd.TF_HeaderPacket.SeqID;
			IOCtrlSendSessionCmd.TF_HeaderPacket.ProductID	= htons(Product);
			IOCtrlSendSessionCmd.CmdTF.CmdType							= htons(0x0002);
			IOCtrlSendSessionCmd.CmdTF.DataLen							= htons(0x0032);
			IOCtrlSendSessionCmd.CmdTF.OPCode								= htons(0x0201);
			IOCtrlSendSessionCmd.CmdPort										=	htons(UDP_PORT);
			IOCtrlSendSessionCmd.ProtocolVersion						= htons(0x0092);
			memcpy(IOCtrlSendSessionCmd.TF_HeaderPacket.Signature, "TF6z", sizeof(IOCtrlSendSessionCmd.TF_HeaderPacket.Signature));

			for(int i=0;i<4;i++)
				IOCtrlSendSessionCmd.PCIp[i] = IOCtrlDstIP[i];

			static Byte		IOCtrlProductType[4] = {0x01, 0x00, 0x40, 0x03};
			static Byte 		IOCtrlMachineNameID[12] = {0x50, 0x43, 0x32, 0x54, 0x56, 0x2d, 0x50, 0x43, 0x41, 0x00, 0x00, 0x00};

			memcpy(IOCtrlSendSessionCmd.ProductType, IOCtrlProductType, sizeof(IOCtrlSendSessionCmd.ProductType));
			memcpy(IOCtrlSendSessionCmd.MachineName, IOCtrlMachineNameID, sizeof(IOCtrlSendSessionCmd.MachineName));

			IOCtrlSendSessionCmd.MyTicks = htonl(0x12345678);

			n = sendto(dgramHandle,&IOCtrlSendSessionCmd,sizeof(IOCtrlSendSessionCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}

			gettimeofday(&cmdStartTick, 0);
			m_TAIFA_IOCtrlStep++;
			LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
		}
		else if(m_TAIFA_IOCtrlStep == 6)  //OPCODE_TF6x0_CONNECTING, wait for OPCODE_TF6x0_CONNECTING_PASSWORD
		{
			IOCtrlSendSessionCmd.TF_HeaderPacket.RandCode =	IOCtrlSendPacketCmd.TF_HeaderPacket.RandCode + htons(0x01);
			IOCtrlSendSessionCmd.TF_HeaderPacket.SeqID		= IOCtrlSendPacketCmd.TF_HeaderPacket.SeqID + htons(0x01);
			IOCtrlSendSessionCmd.CmdTF.OPCode							= htons(0x0201);
			IOCtrlSendSessionCmd.ReplyTicks								= IOCtrlRecvPacketCmd.MyTicks;
			IOCtrlSendSessionCmd.ReplySeqID								= IOCtrlRecvPacketCmd.TF_HeaderPacket.SeqID;
			IOCtrlSendSessionCmd.CmdTF.CmdType						= htons(0x8002);
			n = sendto(dgramHandle,&IOCtrlSendSessionCmd,sizeof(IOCtrlSendSessionCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}
			//                m_TAIFA_IOCtrlStep++;
			gettimeofday(&cmdStartTick, 0);
			m_TAIFA_IOCtrlStep+=2 ;
			LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
		}
		else if(m_TAIFA_IOCtrlStep == 8)  //OPCODE_TF6x0_CONNECTING_PASSWORD
		{
			memset(&IOCtrlSendConnectCmd, 0 ,sizeof(IOCtrlSendConnectCmd));
			memcpy(IOCtrlSendConnectCmd.TF_HeaderPacket.Signature, "TF6z", sizeof(IOCtrlSendConnectCmd.TF_HeaderPacket.Signature));
			memcpy(IOCtrlSendConnectCmd.IOCtrlDstMac, IOCtrlDstMac, sizeof(IOCtrlSendConnectCmd.IOCtrlDstMac));
			memcpy(IOCtrlSendConnectCmd.DestIP, IOCtrlDstIP, sizeof(IOCtrlSendConnectCmd.DestIP));
			memcpy(IOCtrlSendConnectCmd.Config, IOCtrlRecvConnectCmd.Config, sizeof(IOCtrlSendConnectCmd.Config));
			memcpy(IOCtrlSendConnectCmd.ReplyTicks, IOCtrlRecvConnectCmd.MyTicks, sizeof(IOCtrlSendConnectCmd.ReplyTicks));
			memcpy(IOCtrlSendConnectCmd.SupportList, IOCtrlSupportList, sizeof(IOCtrlSendConnectCmd.SupportList));

			IOCtrlSendConnectCmd.TF_HeaderPacket.ProductID	= htons(Product);
			IOCtrlSendConnectCmd.TF_HeaderPacket.RandCode		= IOCtrlRecvConnectCmd.TF_HeaderPacket.RandCode;
			IOCtrlSendConnectCmd.TF_HeaderPacket.SeqID			= htons(0x0002);
			IOCtrlSendConnectCmd.CmdTF.CmdType							= htons(0x8002);
			IOCtrlSendConnectCmd.CmdTF.OPCode								= htons(0x0204);
			IOCtrlSendConnectCmd.CmdTF.DataLen							= htons(0x0032);
			IOCtrlSendConnectCmd.MyTicks										= htonl(0x4567890a);
			IOCtrlSendConnectCmd.ReplySeqID									= IOCtrlRecvConnectCmd.TF_HeaderPacket.SeqID;

			n = sendto(dgramHandle,&IOCtrlSendConnectCmd,sizeof(IOCtrlSendConnectCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}
			gettimeofday(&cmdStartTick, 0);
			m_TAIFA_IOCtrlStep++;
			LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
		}
		else if(m_TAIFA_IOCtrlStep == 9)  //OPCODE_TF6x0_SOURCE_STATUS_ANNOUNCE
		{
			memset(&IOCtrlSendSourceCmd, 0,sizeof(IOCtrlSendSourceCmd));
			memcpy(IOCtrlSendSourceCmd.TF_HeaderPacket.Signature, "TF6z", sizeof(IOCtrlSendSourceCmd.TF_HeaderPacket.Signature));
			memcpy(IOCtrlSendSourceCmd.IOCtrlDstMac, IOCtrlDstMac, sizeof(IOCtrlSendSourceCmd.IOCtrlDstMac));
			memcpy(IOCtrlSendSourceCmd.DestIP, IOCtrlDstIP, sizeof(IOCtrlSendSourceCmd.DestIP));

			IOCtrlSendSourceCmd.TF_HeaderPacket.ProductID	= htons(Product);
			IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID			= htons(0x0003);
			IOCtrlSendSourceCmd.TF_HeaderPacket.RandCode	= IOCtrlRecvConnectCmd.TF_HeaderPacket.RandCode;
			IOCtrlSendSourceCmd.CmdTF.CmdType							= htons(0x0003);
			IOCtrlSendSourceCmd.CmdTF.OPCode							= htons(0x0303);
			IOCtrlSendSourceCmd.CmdTF.DataLen							= htons(0x0022);
			IOCtrlSendSourceCmd.InputSource								= htons(0x0001);

			IOCtrlSendSourceCmd.SourceFrame								= htons(0x0258);
			IOCtrlSendSourceCmd.XmtWidth										= htons((u_short)0);
			IOCtrlSendSourceCmd.XmtHeight										= htons((u_short)0);
			IOCtrlSendSourceCmd.XmtAudio									= htons(0x00);
			IOCtrlSendSourceCmd.MyTicks										= htonl(m_nResolutionTicks);
			IOCtrlSendSourceCmd.RemoteOff									= 0x03;
			IOCtrlSendSourceCmd.CompressHightlow					= 0x01;
			IOCtrlSendSourceCmd.HDCP_ONOFF								= 0x00;
			IOCtrlSendSourceCmd.Macrovision								= 0x00;
			IOCtrlSendSourceCmd.AviInfoDB									= 0x00;
			IOCtrlSendSourceCmd.ScaleLevel								= 0;//m_SldScreenSizeLevelPos;
			IOCtrlSendSourceCmd.audioSampleRate = 1;

			n = sendto(dgramHandle,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}

			if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
			else
				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);


			IOCtrlSendSourceCmd.MyTicks = htonl(0x00001235);
			n = sendto(dgramHandle,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}

			if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
			else
				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);


			IOCtrlSendSourceCmd.MyTicks = htonl(0x00001236);
			n = sendto(dgramHandle,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return (void*)-1;
			}

			if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
			else
				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);

			gettimeofday(&cmdStartTick, 0);
			m_TAIFA_IOCtrlStep++;
			LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
		}
		else if(m_TAIFA_IOCtrlStep == 10) //OPCODE_TF6x0_SOURCE_STATUS_ANNOUNCE
		{
			gettimeofday(&cmdStartTick, 0);
			m_TAIFA_IOCtrlStep++;
			LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
		}
		else if(m_TAIFA_IOCtrlStep == 11)
		{
			SetVideoRotate(g_rotate);
			Create_Pthread(SockStreamProc);
			break;
		}
		else
		{
			if(m_TAIFA_IOCtrlStep <11)
			{
				struct timeval currTime;
				gettimeofday(&currTime, 0);
				if((currTime.tv_sec*1000 - 
					cmdStartTick.tv_sec*1000 + currTime.tv_usec/1000 - cmdStartTick.tv_usec/1000) > 500)
				{
					if(m_TAIFA_IOCtrlStep  == 2)
					{
						m_TAIFA_IOCtrlStep  = 1;
					}
					else if(m_TAIFA_IOCtrlStep  == 5)
					{
						m_TAIFA_IOCtrlStep  = 1;
					}
					
				}
				usleep(5*1000);
			}
		}
	}
	return 0;
}

void* ProcessDisconnectThread(void * param)
{
	bool isAttached = false;
	int status = 0;

	while(g_connectedNetwork)
	{
		if(GetTickCount() - t_outtime > 6000)
		{
			g_connectedNetwork = false;
			g_dispEnable = false;
			LOGI("miss server and disconnection!\n");
			status =jCallbackFun.jvm->GetEnv((void **) &jCallbackFun.env, JNI_VERSION_1_4);
			if(status< 0) 
			{
				LOGE("callback_handler: failed to get JNI environment, "
					"assuming native thread");
				status =jCallbackFun.jvm->AttachCurrentThread(&jCallbackFun.env, NULL);
				if(status< 0) 
				{
					LOGE("callback_handler: failed to attach "
						 "current thread");
					return (void*)-1;
				}
				isAttached = true;
			}
			//LOGE("env: %p, obj: %p, callback: %p\n", jCallbackFun.env, jCallbackFun.obj, jCallbackFun.callbackFuns);
			jCallbackFun.env->CallIntMethod(jCallbackFun.obj, jCallbackFun.callbackFuns,0);
			break;
		}
		usleep(1000*1000);
	}
	
	if (isAttached)
		jCallbackFun.jvm->DetachCurrentThread();

	return (void*)0;
}

void * TAIFARecvThread(void * param)
{
	unsigned char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	struct sockaddr_in recvserver;
	memset(&recvserver, 0,sizeof(recvserver));
	bool connectedNetwork = false;
	bool isAttached = false;
	int status = 0;

	while(g_dispEnable)
	{
		recvfrom(dgramHandle,      buffer,      sizeof(buffer),      0,      (struct sockaddr*)&recvserver,&len);

		HeaderPacket* header	= (HeaderPacket*)buffer;
		CmdPacket*		cmd			= (CmdPacket*)(buffer+ sizeof(HeaderPacket));

		//LOGE("Recv cmd :0x%x\n",ntohs(cmd->OPCode));
		switch(ntohs(cmd->OPCode)) 
		{
		case 0x0101:
			if(memcmp(header->Signature, "TF6z", 4) != 0 || m_TAIFA_IOCtrlStep != 2) 
			{
				break;
			}
			else 
			{
				memcpy((Char*)&IOCtrlBrucastPacketCmd, buffer, ntohs(cmd->DataLen) + 16);
				++m_TAIFA_IOCtrlStep;
				LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
			}
			break;

		case 0x0201:
			if(memcmp(header->Signature, "TF6z", 4) != 0 || m_TAIFA_IOCtrlStep != 5) 
			{
				break;
			}
			else 
			{
				memcpy((Char*)&IOCtrlRecvPacketCmd, buffer, ntohs(cmd->DataLen) + 16);
				++m_TAIFA_IOCtrlStep;
				LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
			}
			break;

		case 0x0204:
			if(memcmp(header->Signature, "TF6z", 4) != 0 || m_TAIFA_IOCtrlStep != 7) 
			{
				break;
			}
			else 
			{
				memcpy((Char*)&IOCtrlRecvConnectCmd, buffer, ntohs(cmd->DataLen) + 16);
				++m_TAIFA_IOCtrlStep;
				LOGE("Move from %d to %d \n",m_TAIFA_IOCtrlStep-1, m_TAIFA_IOCtrlStep);
			}
			break;

		case 0x0301:
				//LOGE("alive \n");
				if(m_JPEG_FileBuff != 0)
				{
					if(!g_connectedNetwork)
					{
						Create_Pthread(ProcessDisconnectThread);
						g_connectedNetwork = true;
					}
					t_outtime = GetTickCount();
				}
			break;
		default:
			break;
		}
	}

	return 0;
}

#ifdef AudioRecordFlag
long timeSpace = 0;
long totalTime = 0;
int ii = 0;
void TAIFA_AudioRecordCb(unsigned char * data,size_t dataSize)
{
	struct sockaddr_in serveraudio;
	memset(&serveraudio, 0,sizeof(serveraudio));
	serveraudio.sin_family=AF_INET;
	serveraudio.sin_addr.s_addr=SERVERADDR;
	serveraudio.sin_port=htons(0x0812); 

	static bool bFirstTime = true;
	if(bFirstTime )
	{
		LOGE("dataSize:%d \n",dataSize);
		bFirstTime  = false;
	}

#if 0
	ii++;
	long tmp_time = GetTickCount()-timeSpace;
	totalTime = totalTime + tmp_time;
	LOGI("ii: %d, totalTime:%ld,tmp_time:%ld\n",ii, totalTime,tmp_time);
	if(totalTime > 1000)
	{
		totalTime = 0;
		ii = 0;
	}
	timeSpace = GetTickCount();
#endif

#if 0
	FILE* fp = fopen("/sdcard/audiorecord_test.pcm","ab+");
	fwrite(data , 1, dataSize, fp);
	fclose(fp);
#endif

	int i=0;
	for(i=0;i<dataSize/1024;i++)
	{
		memset(pAudioBuffer,0,4);

		int j=0;
		for(j=0;j<1024/2;j++)
		{
			pAudioBuffer[4+j*2] = data[i*1024 + j*2 +1];
			pAudioBuffer[4+j*2+1] = data[i*1024 + j*2 ];
		}
		//        memcpy(pAudioBuffer + 4,data + i*1024,1024);
		sendto(dgramHandle,pAudioBuffer,4+1024,0,(struct sockaddr*)&serveraudio,len);
		if(g_dispEnable)
			usleep(3*1000);
	}

	if(dataSize %1024)
	{
		memset(pAudioBuffer,0,4);
		int j=0;
		for(j=0;j<(dataSize %1024)/2;j++)
		{
			pAudioBuffer[4+j*2] = data[i*1024 + j*2 +1];
			pAudioBuffer[4+j*2+1] = data[i*1024 + j*2 ];
		}

		//        memcpy(pAudioBuffer + 4,data + i*1024,dataSize%1024);
		sendto(dgramHandle,pAudioBuffer,4+dataSize%1024,0,(struct sockaddr*)&serveraudio,len); 
		if(g_dispEnable)
			usleep(4*1000);
	}
}

void AudioRecordCb(int event, void* user, void *info)
{
	struct sockaddr_in server;
	int len =sizeof(sockaddr_in);

	if(pAudioBuffer == 0)
	{
		pAudioBuffer  = (char*)malloc(1*1024*1024);
		if(!pAudioBuffer )  LOGE("failed to malloc audio buffer \n");
		LOGE("malloc audio buffer success \n");
	}
	if (event == AudioRecord::EVENT_MORE_DATA)
	{
		AudioRecord::Buffer * audioBuf = (AudioRecord::Buffer *)info;
		TAIFA_AudioRecordCb((unsigned char * )audioBuf->raw, audioBuf->size);
	}
}
#endif

int APUSRecvPacket(char* pBuff, int length)
{
	int recv_len = 0;
	
	fd_set rdfds_;
	fd_set exceptfds_;
	int maxfd_ = 0;

	FD_ZERO(&rdfds_);
	FD_ZERO(&exceptfds_);

	FD_SET(dgramHandle,&rdfds_);
	FD_SET(dgramHandle,&exceptfds_);
	maxfd_ = dgramHandle;

	//struct timeval t;
	//t.tv_sec = 0;
	//t.tv_usec =1000;
	//int ret = select(maxfd_,&rdfds_,0,&exceptfds_,&t);
	int ret = select(maxfd_+1,&rdfds_,0,&exceptfds_,NULL);
	if(ret < 0)
	{
		return -1;
	}
	if(ret == 0)
	{
		usleep(10*1000);
		return 0;
	}

	if(FD_ISSET(maxfd_,&exceptfds_))
	{
		FD_CLR(maxfd_,&exceptfds_);
		return -1;
	}

	if (FD_ISSET(maxfd_, &rdfds_)) 
	{
		FD_CLR(maxfd_, &rdfds_);
		
		int SenderAddrSize = sizeof(g_SenderAddr);
		recv_len = recvfrom( dgramHandle, pBuff, length, 0, (struct sockaddr *) &g_SenderAddr,&SenderAddrSize);
		//if(recv_len == SOCKET_ERROR)
		//{
		//	break;
		//}
		return recv_len;
	}
	return 0;
}

int APUS_SockStreamInit()
{
	int ret = 0;
	struct sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_port=0;
	local.sin_addr.s_addr = INADDR_ANY;
	
	streamHandle=(int)socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	ret = bind(streamHandle,(struct sockaddr*)&local,sizeof(local));
	if(ret != 0)
	{
		//closesocket(streamHandle);
		return -1;
	}
	
	int n = 64*1024;
	setsockopt(streamHandle, SOL_SOCKET, SO_SNDBUF, (const char*)&n, sizeof(int));
	
	//int iMode = 1;
	//ioctlsocket(streamHandle, FIONBIO, (u_long FAR*) &iMode);
	
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr= g_SenderAddr.sin_addr.s_addr;
	saddr.sin_port = htons(g_SenderAddr.sin_port);
//	saddr.sin_port = htons(BroadcastBeginPort);
	LOGI("g_SenderAddr.sin_addr: %p, L:%d\n", htonl(g_SenderAddr.sin_addr.s_addr),__LINE__);

	struct timeval timeout={3,0};//3s
	ret=setsockopt(streamHandle,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	if(ret != 0)
		LOGE("ret error, L:%d\n", __LINE__);
	
	ret = connect(streamHandle, (struct sockaddr*)&saddr, sizeof(saddr));
	LOGE("errno: %d,L:%d\n", errno, __LINE__);
	if(ret != 0)
	{
		return -1;
	}
	return 0;
}


void APUS_SendStartCapScreenFunc()
{
	int active = 1;
	char buffer[256] = {0};
	memset(buffer,0,sizeof(buffer));
	DeviceStatusReport *ptr = (DeviceStatusReport *)buffer;
	
	ptr->type = htonl(DeviceStatusReportCmd);
	ptr->MagicNumber = htonl(MagicNumber);
	ptr->packet_size = htonl(sizeof(DeviceStatusReport));
	ptr->DeviceType = htonl(DeviceType_IATVCtrl);
	ptr->DeviceStatus = htonl(active);
	gethostname(ptr->devicename, 128);

	send(streamHandle, buffer, sizeof(DeviceStatusReport), 0 );

	memset(buffer,0,sizeof(buffer));
	StartCaptureScreen *pStart = (StartCaptureScreen *) buffer;
	pStart->type = htonl(StartCaptureScreenCmd);
	pStart->MagicNumber = htonl(MagicNumber);
	
	pStart->packet_size = htonl(sizeof(StartCaptureScreenExCmd));
	pStart->mode = 0; //Text mode
	send(streamHandle, buffer, sizeof(StartCaptureScreen), 0 );
}

void Android_UNIT()
{
	g_connectedNetwork = false;
	
	if(pAudioBuffer)
	{
		free(pAudioBuffer);
		pAudioBuffer = NULL;
	}

	if(pTXbuffer)
	{
		free(pTXbuffer);
		pTXbuffer = 0;
	}

	if(dgramHandle != 0)
	{
		close(dgramHandle);
		dgramHandle = 0;
		LOGE("socket datagram proc close dgramHandle!\n");
	}

	if(streamHandle != 0)
	{
		close(streamHandle);
		streamHandle = 0;
		LOGE("socket stream proc close streamHandle!\n");
	}
	if(m_iHndFB != 0)
	{
		close(m_iHndFB);
		m_iHndFB =0;
	}
	if(m_iHndFpBuff)
	{
		munmap(m_iHndFpBuff, m_iHndFBSize);
		m_iHndFpBuff = 0;
		m_iHndFBSize = 0;
	}
	
	if(m_JPEG_FileBuff)
	{
		free(m_JPEG_FileBuff);
		m_JPEG_FileBuff = NULL;
	}

}

void APUS_UNINIT()
{
#if defined(APUSONE)

	char buffer[256] = {0};

	StartCaptureScreen *pEnd = (StartCaptureScreen *) buffer;
	pEnd->type = htonl(EndCaptureScreenCmd);
	pEnd->MagicNumber = htonl(MagicNumber);
	
	pEnd->packet_size = htonl(sizeof(StartCaptureScreenExCmd));
	pEnd->mode = 0; //Text mode
	send(streamHandle, buffer, sizeof(StartCaptureScreen), 0 );
#endif
}

int InitImgInfo(IMGINFO *pImgInfo, unsigned char **pDst)
{
	pImgInfo->dstFormat = 0;
	pImgInfo->srcWidth = SRC_WIDTH;
	pImgInfo->srcHeight = SRC_HEIGHT;
	pImgInfo->dstWidth = DEST_WIDTH;
	pImgInfo->dstHeight = DEST_HEIGHT;

	if(pImgInfo->dstWidth % 16 != 0)
		pImgInfo->dstWidth = pImgInfo->dstWidth &~15;
	if(pImgInfo->dstHeight %16 != 0)
		pImgInfo->dstHeight = pImgInfo->dstHeight &~15;

	pImgInfo->pDst[0] = pDst[0];
	pImgInfo->pDst[1] = pDst[1];
	pImgInfo->pDst[2] = pDst[2];

	pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ARGB32;
	pImgInfo->ImgFmtOut = CI_IMAGEPROC_FORMAT_YCBCR420;
	pImgInfo->interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	static bool printt = true;
	if(printt)
	{	
		printt = false;
		LOGE("red:%d gre:%d blue:%d \n",m_fbvinfo.red.offset ,m_fbvinfo.green.offset , m_fbvinfo.blue.offset);
	}

	if((m_fbvinfo.red.offset == 0 && m_fbvinfo.green.offset == 8&& m_fbvinfo.blue.offset == 16)
		|| (m_fbvinfo.red.offset == 24 && m_fbvinfo.green.offset == 16 && m_fbvinfo.blue.offset == 8))
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ARGB32;
	else if(m_fbvinfo.red.offset == 11 && m_fbvinfo.green.offset == 5 && m_fbvinfo.blue.offset == 0)
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_RGB565;
	else
		pImgInfo->ImgFmtIn = CI_IMAGEPROC_FORMAT_ABGR32;

	if(pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_RGB565)
		pImgInfo->srcStride[0] = pImgInfo->srcWidth*2;
	else if(pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_ABGR32 || pImgInfo->ImgFmtIn == CI_IMAGEPROC_FORMAT_ARGB32)
		pImgInfo->srcStride[0] = pImgInfo->srcWidth*4;
	else
	{
		LOGE("invalid format!!!");
		return -1;
	}

	return 0;
}
	
int GetScreenImageAndProc(IMGINFO *pImgInfo)
{
	//b.setConfig(SkBitmap::kARGB_8888_Config, SRC_WIDTH, SRC_HEIGHT);
	//b.setPixels(m_iHndFpBuff);

	CI_VOID* hImgPrc=NULL;
	CI_IMAGEPROC_SIZE srcSize = {0, 0};
	CI_IMAGEPROC_SIZE dstSize = {0, 0};
	CI_IMAGEPROC_CREATEOPTION  CreateOpt;
	CI_IMAGEPROC_PROCESSOPTION ProcessOpt;
	memset(&CreateOpt, 0,sizeof(CreateOpt));
	memset(&ProcessOpt, 0,sizeof(ProcessOpt));

	srcSize.s32Width = pImgInfo->srcWidth;
	srcSize.s32Height = pImgInfo->srcHeight;

	dstSize.s32Width = DEST_WIDTH;
	dstSize.s32Height = DEST_HEIGHT;

	pImgInfo->srcStride[0] = pImgInfo->srcStride[0];
	pImgInfo->srcStride[1] = pImgInfo->srcStride[2] = 0;
	//alocation buffer and src buffer
	pImgInfo->pSrc[0] = (unsigned char*)m_iHndFpBuff;

	if(g_self_rotate && (g_rotate == eROTATION_90 || g_rotate == eROTATION_270))
	{		
		pImgInfo->dstStride[0] = (dstSize.s32Height + 15)&~15;

		if(g_rotate == eROTATION_90)
			pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90L;
		else if(g_rotate == eROTATION_270)
			pImgInfo->rotation = CI_IMAGEPROC_ROTATION_90R;
	}	
	else
	{
		pImgInfo->dstStride[0] = (dstSize.s32Width + 15)&~15;
		pImgInfo->rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	}
	pImgInfo->dstStride[1] = pImgInfo->dstStride[0]>>1;
	pImgInfo->dstStride[2] = pImgInfo->dstStride[0]>>1;

	CreateOpt.u32ColorSpaceConversion	= pImgInfo->ImgFmtOut | pImgInfo->ImgFmtIn << 8;
	CreateOpt.u32Rotation				= pImgInfo->rotation;
	CreateOpt.u32Interpolation			= pImgInfo->interpolation;
	CreateOpt.u32SrcWidth				= srcSize.s32Width;
	CreateOpt.u32SrcHeight			= srcSize.s32Height;
	
	CreateOpt.u32DstWidth				= dstSize.s32Width;
	CreateOpt.u32DstHeight			= dstSize.s32Height;
	CreateOpt.u32Alpha				= 0x80;

	ProcessOpt.u32Size				= sizeof(CI_IMAGEPROC_PROCESSOPTION);
	ProcessOpt.u32Interpolation			= pImgInfo->interpolation;
	int retimg = 0;
	retimg  = CI_IMAGEPROC_Create(&hImgPrc, NULL, &CreateOpt);
	if(retimg)
	{
		LOGE("imageproc create failed!!!!retimg  :%d \n",retimg);
		return -1;
	}
	
#if 0
	unsigned int* pInput =(unsigned int*)pImgInfo->pSrc[0];
	for(int i=0;i<srcSize.s32Width*srcSize.s32Height;)
	{
		unsigned short r,g,b,a;
		r = 0xff;
		g = 0;
		b = 0;
		a = 0xff;
		
		pInput[i] = ((r<<24)|(g<<16) |(b<<8)|(a<<0));
		pInput[i+1] = ((r<<24)|(g<<16) |(b<<8)|(a<<0));
		pInput[i+2] = ((r<<24)|(g<<16) |(b<<8)|(a<<0));
		pInput[i+3] = ((r<<24)|(g<<16) |(b<<8)|(a<<0));
		i += 120;//srcSize.s32Width;
	}
#endif

	retimg = CI_IMAGEPROC_ProcessFrame(hImgPrc, pImgInfo->pSrc, pImgInfo->srcStride, pImgInfo->pDst, pImgInfo->dstStride, &ProcessOpt);
	if(retimg)
	{
		LOGE("process frame failed!!!!retimg  :%p \n",retimg);
		return -1;
	}
	CI_IMAGEPROC_Destroy(hImgPrc);

	return 0;
}

int CIJpegEncFrame(IMGINFO *pImgInfo)
{
	CI_SIZE srcSize;
	CI_JPGENCOPENOPTIONS OpenOptions;
	CI_VOID *pEncoder;
	CI_RESULT ret;
	int format = 3;
	
#ifndef OPHONE_OR_ASUS
	unsigned char* yuvDst = NULL;
	yuvDst = pImgInfo->pDst[1];
	pImgInfo->pDst[1] = pImgInfo->pDst[2];
	pImgInfo->pDst[2] = yuvDst;
#endif

	if(g_self_rotate && (g_rotate == eROTATION_90 || g_rotate == eROTATION_270))
	{	
		pImgInfo->dstStride[0] = (DEST_HEIGHT + 15)&~15;
		srcSize.width = pImgInfo->dstHeight;
		srcSize.height = pImgInfo->dstWidth;
	}
	else
	{
		pImgInfo->dstStride[0] = (DEST_WIDTH + 15)&~15;
		srcSize.width = pImgInfo->dstWidth;
		srcSize.height = pImgInfo->dstHeight;
	}
	pImgInfo->dstStride[1] = pImgInfo->dstStride[0]>>1;
	pImgInfo->dstStride[2] = pImgInfo->dstStride[0]>>1;
	
	OpenOptions.u8OutType = CI_OUT_JPEG;
	OpenOptions.u8YUVType = format == 3? CI_YUV420_PLANAR : format;
	OpenOptions.u8Quality = CI_QUALITY_LEVEL4;
	ret = CI_JPGENC_Open(&pEncoder, &OpenOptions);
	if(ret != CI_SOK)
	{
		LOGE("enc initial failed!\n");
		return -1;
	}
	ret = CI_JPGENC_Frame(pEncoder, pImgInfo->pDst, pImgInfo->dstStride, srcSize, (CI_U8*)m_JPEG_FileBuff, (CI_U32*)&m_JPEG_Filesize);
	if(ret != CI_SOK)
	{
		LOGE("enc enc failed!ret: %d\n", ret);
		return -1;
	}
	CI_JPGENC_Close(pEncoder);
	
	return 0;	
}

void SendTick()
{
	memcpy(IOCtrlSendSourceCmd.TF_HeaderPacket.Signature, "TF6z", sizeof(IOCtrlSendSourceCmd.TF_HeaderPacket.Signature));
	memcpy(IOCtrlSendSourceCmd.IOCtrlDstMac, IOCtrlDstMac, sizeof(IOCtrlSendSourceCmd.IOCtrlDstMac));
	memcpy(IOCtrlSendSourceCmd.DestIP, IOCtrlDstIP, sizeof(IOCtrlSendSourceCmd.DestIP));

	IOCtrlSendSourceCmd.SourceWidth = htons((u_short)SCREEN_WIDTH>SCREEN_HEIGHT ? SCREEN_WIDTH:SCREEN_HEIGHT);
	IOCtrlSendSourceCmd.SourceHeight	= htons((u_short)SCREEN_WIDTH<SCREEN_HEIGHT ? SCREEN_WIDTH:SCREEN_HEIGHT);

	IOCtrlSendSourceCmd.TF_HeaderPacket.ProductID		= htons(Product);
	IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID				= htons(0x0003);
	IOCtrlSendSourceCmd.TF_HeaderPacket.RandCode		= IOCtrlRecvConnectCmd.TF_HeaderPacket.RandCode;
	IOCtrlSendSourceCmd.CmdTF.CmdType								= htons(0x0003);
	IOCtrlSendSourceCmd.CmdTF.OPCode								= htons(0x0303);
	IOCtrlSendSourceCmd.CmdTF.DataLen								= htons(0x0022);
	IOCtrlSendSourceCmd.InputSource									= htons(0x0001);
	IOCtrlSendSourceCmd.SourceFrame									= htons(0x0258);

	IOCtrlSendSourceCmd.XmtWidth = htons((u_short)SCREEN_WIDTH>SCREEN_HEIGHT ? SCREEN_WIDTH:SCREEN_HEIGHT);
	IOCtrlSendSourceCmd.XmtHeight = htons((u_short)SCREEN_WIDTH<SCREEN_HEIGHT ? SCREEN_WIDTH:SCREEN_HEIGHT);

	IOCtrlSendSourceCmd.XmtAudio										= htons(0x00);
	IOCtrlSendSourceCmd.MyTicks				 							= htonl(++m_nResolutionTicks);
	IOCtrlSendSourceCmd.RemoteOff										= 0x03;
	IOCtrlSendSourceCmd.CompressHightlow						= 0x01;
	IOCtrlSendSourceCmd.HDCP_ONOFF									= 0x00;
	IOCtrlSendSourceCmd.Macrovision									= 0x00;
	IOCtrlSendSourceCmd.AviInfoDB										= 0x00;
	IOCtrlSendSourceCmd.ScaleLevel									= 0;
	IOCtrlSendSourceCmd.audioSampleRate = 1;
	ssize_t n = sendto(dgramHandle,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
	if ((n==0))
	{
		LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
		return;
	}

	if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
		IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
	else
		IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);
}

#ifdef AudioRecordFlag
AudioRecord* m_pAudio_recorder = NULL;
int UninitAudio()
{
	if(m_pAudio_recorder)
	{
		m_pAudio_recorder->stop();
		delete m_pAudio_recorder;
		m_pAudio_recorder = 0;
	}

	return 0;
}

void* InitAudio(void* ptr)
{
	m_pAudio_recorder = new AudioRecord(AUDIO_SOURCE_MIXER, 44100, AudioSystem::PCM_16_BIT, 
		AudioSystem::CHANNEL_IN_STEREO, 0, 0, AudioRecordCb, NULL);
	
	if (OK != m_pAudio_recorder->initCheck())
	{
		LOGE("Failed to init audio recorder!\n");
		return (void*)-1;
	}

	if (OK != m_pAudio_recorder->start())
	{
		LOGE("Failed to start audio recorder!\n");
		return (void*)-1;
	}

	LOGE("audio recoder started !!! \n");

	return 0;
}
#endif

int SendVideo()
{
#if TAIFA_TECH
	static short frameNumber = 0;

	struct sockaddr_in serverVideo;
	memset(&serverVideo, 0,sizeof(serverVideo));
	serverVideo.sin_family=AF_INET;
	serverVideo.sin_addr.s_addr=SERVERADDR;
	
	int FileLength  = m_JPEG_Filesize;
	int FileReadPos = 0;

	pTXbuffer[0] = pTXbuffer[1] = pTXbuffer[2] = pTXbuffer[3] = 0;
	pTXbuffer[4] = (frameNumber>>8) &0xff;
	pTXbuffer[5] = frameNumber &0xff;
	memset(&pTXbuffer[6],0,12);

	serverVideo.sin_port=htons(0x0813); 
	sendto(dgramHandle,pTXbuffer,18,0,(struct sockaddr*)&serverVideo,len); 

	int lastpacketLength = (int)(FileLength%((int)1456));

	for(short i=0;i< FileLength/(1456);i++)
	{
		pTXbuffer[0] = (frameNumber>>8) &0xff;
		pTXbuffer[1] = frameNumber &0xff;
		pTXbuffer[2] = (i>>8) &0xff;
		pTXbuffer[3] = i &0xff;

		memcpy(&pTXbuffer[4], &m_JPEG_FileBuff[FileReadPos],1456);
		FileReadPos += 1456;

		serverVideo.sin_port=htons(0x0814); 
		ssize_t n = sendto(dgramHandle,pTXbuffer,1456+4,0,(struct sockaddr*)&serverVideo,len); 
		if ((n==0))
		{
			LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
			return -1;
		}

	}

	if( lastpacketLength< 64)
	{
		pTXbuffer[0] = (frameNumber>>8) &0xff;
		pTXbuffer[1] = frameNumber &0xff;
		pTXbuffer[2] = (((FileLength/1456)>>8) &0xff) | 0x80;
		pTXbuffer[3] = (FileLength/1456) &0xff;

		memcpy(&pTXbuffer[4], &m_JPEG_FileBuff[FileReadPos],lastpacketLength);
		FileReadPos += lastpacketLength;

		memset(&pTXbuffer[4 + lastpacketLength], 0, 64-lastpacketLength);

		ssize_t n = sendto(dgramHandle,pTXbuffer,4+64,0,(struct sockaddr*)&serverVideo,len); 
		if ((n==0))
		{
			LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
			return -1;
		}

	}
	else
	{
		int invalidLength = -1;
		pTXbuffer[0] = (frameNumber>>8) &0xff;
		pTXbuffer[1] = frameNumber &0xff;
		pTXbuffer[2] = (((FileLength/1456)>>8) &0xff) ;
		pTXbuffer[3] = (FileLength/1456) &0xff;

		memcpy(&pTXbuffer[4], &m_JPEG_FileBuff[FileReadPos],lastpacketLength);
		FileReadPos += lastpacketLength;

		for(int i=0;i<lastpacketLength  - 1;i++)
		{
			if(pTXbuffer[4+i] == 0xff && pTXbuffer[4+i+1] == 0xd9)
			{
				invalidLength = i;
				break;
			}
		}
		if(invalidLength == -1)
		{
			invalidLength = lastpacketLength; 
		}
		ssize_t n = sendto(dgramHandle,pTXbuffer,4+ invalidLength,0,(struct sockaddr*)&serverVideo,len); 
		if ((n==0))
		{
			LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
			return -1;
		}

		pTXbuffer[0] = (frameNumber>>8) &0xff;
		pTXbuffer[1] = frameNumber &0xff;
		pTXbuffer[2] = (((FileLength/1456+1)>>8) &0xff) |0x80;
		pTXbuffer[3] = (FileLength/1456+1) &0xff;

		pTXbuffer[4] = 0xff;
		pTXbuffer[5] = 0xd9;
		memset(&pTXbuffer[6],0,62);

		n = sendto(dgramHandle,pTXbuffer,4+ 64,0,(struct sockaddr*)&serverVideo,len); 
		if ((n==0))
		{
			LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
			return -1;
		}
	}
	frameNumber++;

#elif defined(APUSONE)

	int len = 0;
	memcpy(pTXbuffer + sizeof(UpdateCaptureScreenEx),m_JPEG_FileBuff,m_JPEG_Filesize);
	len = m_JPEG_Filesize+ sizeof(UpdateCaptureScreenEx);
	UpdateCaptureScreenEx* ptr = (UpdateCaptureScreenEx*)pTXbuffer;
	ptr->MagicNumber = htonl(MagicNumber);
	ptr->packet_size = htonl(len);
	ptr->type = htonl(UpdateCaptureScreenExCmd );
	if(!g_self_rotate)
	{
		switch(g_rotate)
		{
			case eROTATION_0:
				ptr->rotate = htonl(0);
 				break;
			case eROTATION_90:
				ptr->rotate = htonl(3);
 				break;
			case eROTATION_180:
				ptr->rotate = htonl(2);
 				break;
			case eROTATION_270:
				ptr->rotate = htonl(1);
 				break;
			default:
				break;
		}
	}
	else
		ptr->rotate = htonl(0); //0 no rotate,3 rotate 90,2 rotate 180,1 rotate 270
	ptr->left = ptr->top = 0;
	//ptr->right = htonl(800);
	ptr->right = htonl(DEST_WIDTH);
	ptr->bottom = htonl(DEST_HEIGHT);
	ptr->mediatype = htonl(1); //JPEG
	int err = -1;
	struct timeval timeout={3,0};//3s
	int ret=setsockopt(streamHandle,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	if(ret != 0)
		LOGE("ret error, L:%d\n", __LINE__);
//	LOGI("len: %d\n", len);
//	LOGE("EBADF:%d, EFAULT:%d, EINTR: %d, \nEAGAIN:%d, ENOBUFS:%d,EINVAL:%d\n", \
//		EBADF,EFAULT,EINTR,EAGAIN,ENOBUFS,EINVAL);

	if(err = send(streamHandle, pTXbuffer, len, 0) != len)
	{
		LOGE("send error: %d, L: %d\n", errno, __LINE__);
		if(errno == 32)
		{
			g_dispEnable = false;
			CaptureCallBack(connectionWimoFail);
		}
	}
#endif
	
	return 0;
}

void* CheckCPUUtilization(void* ptr)
{
	long usrTime, sysTime, idleTime,irqTime;
	long unsigned total_delta_time;

	while(g_dispEnable)
	{
		FILE* file = fopen("/proc/stat", "r");
		if (!file)
		{
			LOGE("Could not open /proc/stat.\n");
			return (void*)-1;
		}
		
		fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu", &new_cpu.utime,
			&new_cpu.ntime, &new_cpu.stime,&new_cpu.itime, 
			&new_cpu.iowtime, &new_cpu.irqtime, &new_cpu.sirqtime);
		fclose(file);

		usrTime = new_cpu.utime + new_cpu.ntime - (old_cpu.utime + old_cpu.ntime);
		sysTime = new_cpu.stime - old_cpu.stime;
		idleTime = new_cpu.itime - old_cpu.itime;
		irqTime = new_cpu.iowtime + new_cpu.irqtime + new_cpu.sirqtime
			-(old_cpu.iowtime + old_cpu.irqtime + old_cpu.sirqtime);
		
		total_delta_time = usrTime + sysTime + idleTime + irqTime;

		//LOGI("usrTime:%ld%%, sysTime:%ld%%, idleTime:%ld%%,irqTime:%ld%% \ncpu utilization:%ld%%\n", 
		//	100*usrTime/total_delta_time,100*sysTime/total_delta_time,100*idleTime/total_delta_time,
		//	100*irqTime/total_delta_time,100*(usrTime+sysTime)/total_delta_time);
		g_CPUUtilization = 100 * (usrTime + sysTime + irqTime) / total_delta_time;
		//LOGI("cpu utilization: %ld%%\n", g_CPUUtilization);
		g_bCheckCPUUpdate = true;
		memcpy(&old_cpu, &new_cpu, sizeof(old_cpu));

		usleep(1000*1000);
	}
	
	return 0;
}

void UpdateFramesAccordingToCPU(int &constantFrames,int &targetFrames)
{
	if(g_bCheckCPUUpdate)
	{
		if(g_CPUUtilization <= 90 && g_CPUUtilization >= 85)
		{
			g_saveTimesOfOverloadCPU = 0;
			g_saveLastestFrames = constantFrames;
		}
		else if(g_CPUUtilization > 90)
		{
			g_saveTimesOfOverloadCPU++;
			if(g_saveTimesOfOverloadCPU < 2)
				constantFrames = g_saveLastestFrames;
			else
				constantFrames = constantFrames / 2;
		}
		else if(g_CPUUtilization < 85 && g_CPUUtilization >= 65)
		{
			g_saveTimesOfOverloadCPU = 0;
			g_saveLastestFrames = constantFrames;
			constantFrames += 1;
		}
		else
		{
			g_saveTimesOfOverloadCPU = 0;
			constantFrames += 2;
		}
		
		if(constantFrames > targetFrames)
			constantFrames = targetFrames;
		
		//LOGI("g_saveLastestFrames:%d,constantFrames :%d\n",g_saveLastestFrames,constantFrames);

		g_bCheckCPUUpdate = false;
	}
}
	
/**************************************************************
	Func: SockStreamProc();
	Function: capture screen and jpegenc.
	parameters:ptr

 **************************************************************/

void* SockStreamProc(void* ptr)
{
	unsigned char *pDst[3] = {0,0,0};
	int frames = 0;
	long startTick = 0;
	long sTick = 0, noteTime = 0;
	int width, height;
	int format = 0;
	float fps = 0;
	IMGINFO ImgInfo, *pImgInfo = &ImgInfo;
	int targetFrames = 30;
	int constantFrames = targetFrames;
	int NoteFrames = 0;
	bool bUpdateTargetFrames = true;

	g_saveLastestFrames = constantFrames/2;
			
	width = DEST_WIDTH;
	height = DEST_HEIGHT;
	//aligned to 16
	//width = width &~15;
	width = (width + 15)&~15;
	height = (height + 15)&~15;

	m_JPEG_FileBuff = (unsigned char *)malloc((width)*(height)*4);
	if(NULL == m_JPEG_FileBuff)
	{
		LOGE("No memory for decoding!\n");
		LOGE("Exiting!\n");
		goto QUIT;
	}

	pDst[0] = (unsigned char *)malloc(width*height*4);	
	if(!pDst[0])
	{
		LOGE("failed malloc pDst[0] \n");
		goto QUIT;
	}

	switch(format)
	{
		case 0:
			pDst[1] = pDst[0] + width*height;
			pDst[2] = pDst[1] + width*height/4;
			break;
		case 1:
		case 2:
		case 3:
			pDst[1] = pDst[2] = 0;
			break;
	}
	
	if(InitImgInfo(pImgInfo, pDst) != 0)
	{
		LOGE("failed init ImgInfo\n");
		goto QUIT;
	}

	pTXbuffer = (char*)malloc(1*1024*1024);
	if(!pTXbuffer)  
	{
		LOGE("failed to malloc pTXbuffer \n");
		goto QUIT;
	}

#ifdef AudioRecordFlag
	Create_Pthread(InitAudio);
#endif

	CaptureCallBack(connectionWimoSuccess);
	Create_Pthread(CheckCPUUtilization);
	
	noteTime = startTick = GetTickCount();
	while(g_dispEnable)
	{
		if(GetScreenImageAndProc(pImgInfo))
		{
			LOGE("GetScreenImageAndProc enc frame failure!\n");
			break;
		}
#if 0
		if(g_self_rotate && (g_rotate == eROTATION_90 || g_rotate == eROTATION_270))
		{
			static bool bFirst = true;
			if(bFirst)
			{
				bFirst  = false;
				FILE* fp = fopen("/data/tst/st.yuv","wb");
				fwrite(pImgInfo->pDst[0], 1, 480*856, fp);
				fwrite(pImgInfo->pDst[1], 1, 480*856/4, fp);
				fwrite(pImgInfo->pDst[2], 1, 480*856/4, fp);
				fclose(fp);
			}
		}
#endif

		if(CIJpegEncFrame(pImgInfo) != 0)
		{
			LOGE("Jpeg enc frame failure!\n");
			break;
		}

#if 0
		static bool bFirstTime = true;
		if(bFirstTime )
		{
			LOGE("m_JPEG_Filesize:%d \n",m_JPEG_Filesize);
			bFirstTime  = false;
			FILE* fp = fopen("/data/tst/start.jpg","wb");
			fwrite(m_JPEG_FileBuff , 1, m_JPEG_Filesize, fp);
			fclose(fp);
		}
#endif

		static long framescc = 0;
		if(g_bUpdated)
		{
			g_bUpdated = false;
#ifdef TAIFA_TECH
			framescc = 20;
			sTick = GetTickCount();
#endif
		}
	
		if(framescc)
		{
			framescc--;
			if(GetTickCount()- sTick > 1000)
			{
				if(SendVideo() != 0)
					LOGE("send video error!\n");
				SendTick();
				sTick = GetTickCount();
			}
			continue;
		}

		if(SendVideo() != 0)
			LOGE("send video error!\n");

		frames++;

		UpdateFramesAccordingToCPU(constantFrames, targetFrames);
		
		if(frames >= targetFrames || frames >= constantFrames)
		{
			long costTime = GetTickCount() - startTick;
		
			if(costTime <1000)
			{
				//LOGE("(GetTickCount() - startTick):%ld\n",(GetTickCount() - startTick));
				usleep((1000 - costTime)*1000);
				fps = (float)(frames*1000)/(float)(GetTickCount() - startTick);
				LOGI("fps :%f \n",fps);
#ifdef TAIFA_TECH
				SendTick();
#endif
				startTick = GetTickCount();
				frames = 0;
			}
		}
				
		if(GetTickCount() - startTick > 1000)
		{
			fps = (float)(frames*1000)/(float)(GetTickCount() - startTick);
			LOGI("frames:%ld, fps :%f \n",frames,fps);
#ifdef TAIFA_TECH
			SendTick();
#endif
			startTick = GetTickCount();
			frames = 0;
		}

		if(GetTickCount() - noteTime < 20*1000)
		{
			if(fps > NoteFrames)
				NoteFrames = fps;
		}
		else
		{	
			if(bUpdateTargetFrames)
			{	
				LOGI("L :%d,NoteFrames max:%d\n",__LINE__, NoteFrames);
				targetFrames = NoteFrames;
				bUpdateTargetFrames = false;
			}
		}
	}

QUIT:
	APUS_UNINIT();
	Android_UNIT();

	if(pDst[0])
	{
		free(pDst[0]);
		pDst[0] = 0;
	}

#ifdef AudioRecordFlag
	UninitAudio();
#endif

	LOGE("free buffer successfully!\n");	
	CaptureCallBack(connectionWimoFail);
	
	return 0;
}


bool APUS_CheckAttributeOfDeviceStatus(char revbuff[], int length)
{
	DeviceStatusReport *pp = (DeviceStatusReport *)revbuff;
	if(ntohl(pp->MagicNumber) == (unsigned int)MagicNumber  \
		&& ntohl(pp->packet_size) <= (unsigned int)length \
		&& ntohl(pp->type) ==(unsigned int) DeviceStatusReportCmd  \
		&& (ntohl(pp->DeviceType) & (unsigned int)DeviceType_IATV ) == (unsigned int)DeviceType_IATV )
		
	   return true;
	return false;
}

bool APUS_CheckAttributeOfEcho(char revbuff[], int length)
{
	Echo *pm = (Echo*)revbuff;
	if(ntohl(pm->MagicNumber) == (unsigned int)MagicNumber  \
		&& ntohl(pm->packet_size) <= (unsigned int)length \
		&& ntohl(pm->type) == (unsigned int)EchoCmd)
		return true;
	
	return false;
}

void* InitThread(void* param)
{
	if(AndroidInit() != 0)
		return (void*)-1;

	if(SockDatagramInit() != 0)
		return (void*)-1;

	return (void*)0;
}

void* SockDatagramProc(void* param)
{
	unsigned int startTime = GetTickCount();

	Create_Pthread(InitThread);
#ifdef TAIFA_TECH
	Create_Pthread(TAIFARecvThread);	
	Create_Pthread(TAIFAResponseThread);
#elif defined(APUSONE)
	Create_Pthread(APUSONEThread);
#endif

	while(!m_JPEG_FileBuff && (GetTickCount() - startTime < 5000))
		usleep(50*1000);
	if(!m_JPEG_FileBuff)	//FAILED. SO DESTRY ALL.
	{
		g_dispEnable = false;
		Android_UNIT();
		CaptureCallBack(connectionWimoFail);
		return (void*)-1;
	}
	
	return 0;
}

void Create_Pthread(PFunc SockProc)
{
	pthread_t pid;
	struct sched_param schedRecv;
	pthread_attr_t attrRecv;
	pthread_attr_init(&attrRecv);
	pthread_create(&pid, &attrRecv, SockProc, NULL);
	schedRecv.sched_priority = 90;
	pthread_setschedparam(pid, SCHED_RR, &schedRecv);
}

int SetVideoEnable(bool enable)
{
	if(g_dispEnable == enable)	return 0;
	
	bool currState = g_dispEnable;
	g_dispEnable = enable;
	LOGI("g_dispEnable: %d, L: %d\n", g_dispEnable, __LINE__);

	if(currState == false && enable == true)
	{
		m_TAIFA_IOCtrlStep = 1;
		Create_Pthread(SockDatagramProc);
	}
	else
	{
		while(m_JPEG_FileBuff)
		{
			usleep(50*1000);
		}
	}
	return 0;
}

int SetVideoRotate(int nRotate)  //true:vertical screen. false: 
{
	if (g_rotate != nRotate)
	{
		g_bUpdated = true;
		if(g_connectedNetwork)
			SendTick();
		g_rotate = nRotate;
	}

#ifdef TAIFA_TECH
	if(g_self_rotate)
	{
		if(g_rotate == eROTATION_90 || g_rotate == eROTATION_270)
	    		IOCtrlSendSourceCmd.Vertical = 0;
		else
	    		IOCtrlSendSourceCmd.Vertical = 1;
	}
	else
    		IOCtrlSendSourceCmd.Vertical = 1;
#endif

	return 0;
}

int cancelConnect()
{
	g_dispEnable = false;
	Android_UNIT();
	CaptureCallBack(connectionWimoFail);
	
	return 0;
}

int initSC()
{
	return 0;
}

JNIEXPORT jint JNICALL Java_cidana_screencap_Capture_initSC
(JNIEnv * env, jobject thiz)
{
	LOGI("Java_cidana_screencap_Capture_initSC");
	//get java method
	env->GetJavaVM(&jCallbackFun.jvm); 
	jCallbackFun.env = env;
	jCallbackFun.obj = env->NewGlobalRef(thiz);
	jCallbackFun.cls = env->GetObjectClass(thiz);
	jCallbackFun.cls = (jclass)env->NewGlobalRef(jCallbackFun.cls);
	jCallbackFun.callbackFuns = env->GetMethodID(jCallbackFun.cls, "javaCallback", "(I)I");

	//LOGE("env: %p, obj: %p, callback: %p\n", jCallbackFun.env, jCallbackFun.obj, jCallbackFun.callbackFuns);
	return initSC();
}

JNIEXPORT jint JNICALL Java_cidana_screencap_Capture_setVideoEnable
(JNIEnv * env, jobject thiz, jboolean enable)
{
	LOGI("Java_cidana_screencap_Capture_setVideoEnable");
	return SetVideoEnable(enable);
}

JNIEXPORT jint JNICALL Java_cidana_screencap_Capture_setDisplayMode
(JNIEnv * env, jobject thiz, jint displaymode)
{
	LOGI("Java_cidana_screencap_Capture_setDisplayMode");
	return 0;
}

JNIEXPORT jint JNICALL Java_cidana_screencap_Capture_setVideoRenderMode
(JNIEnv * env, jobject thiz, jint vrmode)
{
	LOGI("Java_cidana_screencap_Capture_setVideoRenderMode");
	return 0;
}

JNIEXPORT jint JNICALL Java_cidana_screencap_Capture_setVideoRotate
(JNIEnv * env, jobject thiz, jint rotate)
{
	LOGI("setVideoRotate %d ",rotate);
	return SetVideoRotate(rotate);
}

JNIEXPORT jint JNICALL Java_cidana_screencap_Capture_cancelConnect
(JNIEnv * env, jobject thiz)
{
	LOGI("cancelConnect\n");
	return cancelConnect();
}
