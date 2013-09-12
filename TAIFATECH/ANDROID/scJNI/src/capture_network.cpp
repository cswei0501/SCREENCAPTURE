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

#include "capture_network.h"

using namespace android;

/*****************************************************
* Func: CWiMoNetwork
* Function: construction function
* Parameters: void
*****************************************************/
CWiMoNetwork::CWiMoNetwork(CWiMoControl* pis)
{
	m_cControl = pis;
	sendSrcBuff = NULL;
	sendVideoTXbuffer = NULL;

	sendSrcBuff = (char*)malloc(4*1024*1024);
	if(!sendSrcBuff)
	{
		LOGE("failed malloc sendSrcBuff \n");
	}
	sendVideoTXbuffer = (char*)malloc(1*1024*1024);
	if(!sendVideoTXbuffer)
	{
		LOGE("failed malloc sendVideoTXbuffer \n");
	}
	InitNet();
}

/*****************************************************
* Func: ~CWiMoNetwork
* Function: deconstruction function
* Parameters: void
*****************************************************/
CWiMoNetwork::~CWiMoNetwork()
{
	UnInitNet();
	if(sendVideoTXbuffer)
	{
		free(sendVideoTXbuffer);
		sendVideoTXbuffer = NULL;
	}
	if(sendSrcBuff)
	{
		free(sendSrcBuff);
		sendSrcBuff = NULL;
	}
}

/*****************************************************
* Func: InitNet
* Function: init param
* Parameters: void
*****************************************************/
int CWiMoNetwork::InitNet()
{
	SCREEN_WIDTH = 0;
	SCREEN_HEIGHT = 0;
	socketHandle = 0;
	send1SocketHandle = 0;
	send2SocketHandle = 0;
	frameNumber = 0;
	IOCtrlStep = 1;
	nResolutionTicks = 0x00001234;
	iDisconnectReason = DISCONNECT_ERROR;
	networkRecvThread_hnd = 0;
	CheckAliveResponseReportThread_hnd = 0;
	handshakeResponseThread_hnd = 0;
	
	memset(&IOCtrlSupportList, 0, sizeof(IOCtrlSupportList));
	IOCtrlSupportList[1] = 0x03;
	IOCtrlSupportList[3] = 0x03;
	memset(&IOCtrlDstIP, 0, sizeof(IOCtrlDstIP));
	memset(&IOCtrlDstMac, 0, sizeof(IOCtrlDstMac));
	memset(&IOCtrlSendPacketCmd, 0, sizeof(IOCtrlSendPacketCmd));
	memset(&IOCtrlRecvPacketCmd, 0, sizeof(IOCtrlRecvPacketCmd));
	memset(&IOCtrlSendDiscoveryCmd, 0, sizeof(IOCtrlSendDiscoveryCmd));
	memset(&IOCtrlRecvDiscoveryCmd, 0, sizeof(IOCtrlRecvDiscoveryCmd));
	memset(&IOCtrlSendConnectCmd, 0, sizeof(IOCtrlSendConnectCmd));
	memset(&IOCtrlRecvConnectCmd, 0, sizeof(IOCtrlRecvConnectCmd));
	memset(&IOCtrlSendConnectPasswordCmd, 0, sizeof(IOCtrlSendConnectPasswordCmd));
	memset(&IOCtrlRecvConnectPasswordCmd, 0, sizeof(IOCtrlRecvConnectPasswordCmd));
	memset(&IOCtrlSendDisconnectReason, 0, sizeof(IOCtrlSendDisconnectReason));
	memset(&IOCtrlSenAliveReportCmd, 0, sizeof(IOCtrlSenAliveReportCmd));
	memset(&IOCtrlSendDataLostReportCmd, 0, sizeof(IOCtrlSendDataLostReportCmd));
	memset(&IOCtrlSendSourceStatusCmd, 0, sizeof(IOCtrlSendSourceStatusCmd));

	if(m_cControl->GetVideoMethod() == METHOD_TS_STREAM_INTERFACE)
	{
		mVersion = VERSION2;
		mCodec = CODEC2;
	}
	else
	{
		mVersion = VERSION1;
		mCodec = CODEC1;
	}
		
	if(GetIPAndMACAddress(IOCtrlDstMac) != S_OK)
	{
		LOGE("failed to GetIPAndMACAddress()!\n");
		return E_FAILED;
	}
	
	if(NetworkInit() != S_OK)
	{
		LOGE("failed to NetworkInit!\n");
		return E_FAILED;
	}
	
	networkRecvThread_hnd = CreateThreadHelper(NetworkRecvThread, (void*)this) ;
	if(networkRecvThread_hnd == 0)
	{
		LOGE("creat thread NetworkRecvThread failed!\n");
		return E_THREAD_CREATE;
	}
	handshakeResponseThread_hnd = CreateThreadHelper(HandshakeThread, (void*)this) ;
	if(handshakeResponseThread_hnd == 0)
	{
		LOGE("creat thread HandshakeThread failed!\n");
		return E_THREAD_CREATE;
	}

	return S_OK;
}

/*****************************************************
* Func: UnInitNet
* Function: uninit param
* Parameters: 
*****************************************************/
int CWiMoNetwork::UnInitNet()
{
	if(CheckAliveResponseReportThread_hnd)
	{
		pthread_join(CheckAliveResponseReportThread_hnd, NULL);
		CheckAliveResponseReportThread_hnd = 0;
	}
	if(handshakeResponseThread_hnd)
	{
		pthread_join(handshakeResponseThread_hnd, NULL);
		handshakeResponseThread_hnd = 0;
	}
	if(networkRecvThread_hnd)
	{
		pthread_join(networkRecvThread_hnd, NULL);
		networkRecvThread_hnd = 0;
	}

	if(send1SocketHandle != 0)
	{
		close(send1SocketHandle);
		send1SocketHandle = 0;
	}
	if(send2SocketHandle != 0)
	{
		close(send2SocketHandle);
		send2SocketHandle = 0;
	}
	if(socketHandle != 0)
	{
		close(socketHandle);
		socketHandle = 0;
	}
	return 0;
}

/*****************************************************
* Func: SetDisconnectReason
* Function: set disconnection reason
* Parameters: iReason: detail reason
*****************************************************/
void CWiMoNetwork::SetDisconnectReason(int iReason)
{
	iDisconnectReason = iReason;
}

/*****************************************************
* Func: GetDisconnectReason
* Function: get disconnect reason
* Parameters: void
*****************************************************/
int CWiMoNetwork::GetDisconnectReason()
{
	return iDisconnectReason;
}

long CWiMoNetwork::GetServerIpAddr()
{
	return (long)server.sin_addr.s_addr;
}

/*****************************************************
* Func: GetIPAndMACAddress
* Function: return IP and MAC address
* Parameters: pMac: output MAC address
*****************************************************/
long long CWiMoNetwork::GetIPAndMACAddress(unsigned char* pMac)
{ 
	#define MAXINTERFACES 10
	register int fd, intrface, retn = 0; 
	long long ipaddr = 0;
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
			//LOGE("interface num is intrface=%d\n\n\n",intrface); 
			//根据借口信息循环获取设备IP和MAC地址
			while (intrface-- > 0) 
			{ 
				//LOGE("buf[%d].ifr_name:%s \n",intrface, buf[intrface].ifr_name);
				//  if(strcmp(buf[intrface].ifr_name,"eth0"))
				//    continue;
				//获取设备名称
				//LOGE ("net device %s\n", buf[intrface].ifr_name); 

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
					TXipaddr = ((((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
					LOGI("IP address is: %d.%d.%d.%d\n", 
						TXipaddr.s_addr&0xff,
						TXipaddr.s_addr>>8&0xff,
						TXipaddr.s_addr>>16&0xff,
						TXipaddr.s_addr>>24&0xff);
					if (!(ioctl (fd, SIOCGIFHWADDR, (char *) &buf[intrface]))) 
					{ 
						LOGI("HW address is: %02x:%02x:%02x:%02x:%02x:%02x\n", 
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
						LOGE("failed to get mac address ");
					close (fd); 
					ipaddr = TXipaddr.s_addr;
					if(!(ipaddr))
					{
						LOGE("failed to get IP Address\n");
						return E_FAILED;
					}
					LOGI("get IP Address success\n");
					IOCtrlDstIP[0] = ipaddr &0xff;//192;
					IOCtrlDstIP[1] = ipaddr>>8 &0xff;
					IOCtrlDstIP[2] = ipaddr>>16 &0xff;
					IOCtrlDstIP[3] = ipaddr>>24 &0xff;

					return S_OK; 
				} 
			}    
		}
	} //while
	return E_FAILED; 
}

/*****************************************************
* Func: NetworkInit
* Function: Init socket datagram
* Parameters: void
*****************************************************/	
int CWiMoNetwork::NetworkInit(void)
{
	int fbroadcast = 1;
	int optval = 1;

	//handshake src and dest port:48689
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(UDP_PORT);
	
	socketHandle=socket(AF_INET,SOCK_DGRAM,0);

	memset(&server, 0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(UDP_PORT); //server的监听端口
	server.sin_addr.s_addr=INADDR_ANY;

	setsockopt(socketHandle, SOL_SOCKET, SO_BROADCAST, (const struct sockaddr *)&fbroadcast, sizeof(int));
	setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(int));
	
	if(bind(socketHandle, (const struct sockaddr *)&addr, sizeof(addr)) !=0)
	{
		close(socketHandle);
		socketHandle = 0;
		LOGE("NetworkInit bind failed!\n");
		return E_FAILED;
	}
	struct timeval timeout={0,500*1000};//receive time out is 500ms
	setsockopt(socketHandle, SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout,sizeof(timeout));
	setsockopt(socketHandle, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout,sizeof(timeout));
	
	if(m_cControl->GetVideoMethod() != METHOD_TS_STREAM_INTERFACE)
	{
		//send video src port:0x0813
		send1SocketHandle=socket(AF_INET,SOCK_DGRAM,0);
		memset(&addr, 0, sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(VIDEOPORT1);

		setsockopt(send1SocketHandle, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(int));
		if(bind(send1SocketHandle, (const struct sockaddr *)&addr, sizeof(addr)) !=0)
		{
			close(send1SocketHandle);
			send1SocketHandle = 0;
			LOGE("NetworkInit send1SocketHandle bind failed!\n");
			return E_FAILED;
		}
		struct timeval sendTimeOut={0,100*1000};//send time out is 100ms
		setsockopt(send1SocketHandle, SOL_SOCKET,SO_SNDTIMEO, (char *)&sendTimeOut,sizeof(timeout));

		//send video src port:0x0814
		send2SocketHandle=socket(AF_INET,SOCK_DGRAM,0);
		memset(&addr, 0, sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(VIDEOPORT2);

		setsockopt(send2SocketHandle, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(int));
		if(bind(send2SocketHandle, (const struct sockaddr *)&addr, sizeof(addr)) !=0)
		{
			close(send2SocketHandle);
			send2SocketHandle = 0;
			LOGE("NetworkInit send2SocketHandle bind failed!\n");
			return E_FAILED;
		}
		setsockopt(send2SocketHandle, SOL_SOCKET,SO_SNDTIMEO, (char *)&sendTimeOut,sizeof(timeout));
	}
	
	return S_OK;
}

/*****************************************************
* Func: InitSendConnectCmd
* Function: init send to connect command
* Parameters: void
*****************************************************/
void CWiMoNetwork::InitSendConnectCmd()
{
	Byte	IOCtrlHeader[6]					= {0xc0, 0x00, 0xc0, 0x03, 0x00, 0x92};
	Byte	IOCtrlProductType[4]				= {0x01, 0x00, 0x40, 0x03};
	Byte	IOCtrlMachineNameID[12]			= {0x50, 0x43, 0x32, 0x54, 0x56, 0x2d, 0x50, 0x43, 0x41, 0x00, 0x00, 0x00};

	IOCtrlSendConnectCmd.TF_HeaderPacket.ProductID		= htons(Product);
	IOCtrlSendConnectCmd.TF_HeaderPacket.Version    		= mVersion;
	IOCtrlSendConnectCmd.TF_HeaderPacket.Codec      		= htons(mCodec);
	IOCtrlSendConnectCmd.CmdTF.CmdType				= htons(0x0002);
	IOCtrlSendConnectCmd.CmdTF.DataLen				= htons(0x0038);
	IOCtrlSendConnectCmd.CmdTF.OPCode				= htons(0x0201);
	IOCtrlSendConnectCmd.CmdPort					= htons(UDP_PORT);
	IOCtrlSendConnectCmd.ProtocolVersion				= htons(0x0093);
	
	memcpy(IOCtrlSendPacketCmd.IOCtrlHeader, IOCtrlHeader, sizeof(IOCtrlSendPacketCmd.IOCtrlHeader));
	memcpy(IOCtrlSendPacketCmd.MachineName, IOCtrlMachineNameID, sizeof(IOCtrlSendPacketCmd.MachineName));
	memcpy(IOCtrlSendConnectCmd.TF_HeaderPacket.Signature, SIGNATURE, sizeof(IOCtrlSendConnectCmd.TF_HeaderPacket.Signature));
	memcpy(IOCtrlSendConnectCmd.DestIP, IOCtrlDstIP, sizeof(IOCtrlSendConnectCmd.DestIP));
	memcpy(IOCtrlSendConnectCmd.ProductType, IOCtrlProductType, sizeof(IOCtrlSendConnectCmd.ProductType));
	memcpy(IOCtrlSendConnectCmd.MachineName, IOCtrlMachineNameID, sizeof(IOCtrlSendConnectCmd.MachineName));
	memcpy(IOCtrlSendConnectCmd.username, USERNAME, sizeof(USERNAME));
	
	IOCtrlSendConnectCmd.MyTicks 						= htonl(0x12345678);
}

/*****************************************************
* Func: InitSendDiscoveryCmd
* Function: init send discovery cmd
* Parameters: void
*****************************************************/
void CWiMoNetwork::InitSendDiscoveryCmd()
{
	IOCtrlSendDiscoveryCmd.TF_HeaderPacket.ProductID	= htons(Product);
	IOCtrlSendDiscoveryCmd.TF_HeaderPacket.Version    	= mVersion;
	IOCtrlSendDiscoveryCmd.TF_HeaderPacket.Codec	 	= htons(mCodec);
	IOCtrlSendDiscoveryCmd.TF_HeaderPacket.RandCode 	= htons(0x00);
	IOCtrlSendDiscoveryCmd.TF_HeaderPacket.SeqID      	= htons(0x00);

	IOCtrlSendDiscoveryCmd.CmdTF.DataLen			 	= htons(0x0022);
	IOCtrlSendDiscoveryCmd.CmdTF.CmdType			 	= htons(0x0001);
	IOCtrlSendDiscoveryCmd.CmdTF.OPCode   			 	= htons(0x0101);

	IOCtrlSendDiscoveryCmd.ProductType[0]				= 0x8c;
	IOCtrlSendDiscoveryCmd.ProductType[1]				= 0x00;
	IOCtrlSendDiscoveryCmd.ProductType[2]				= 0x40;
	IOCtrlSendDiscoveryCmd.ProductType[3]				= 0x03;
	IOCtrlSendDiscoveryCmd.ProtocolVersion				= htons(0x0093);
	IOCtrlSendDiscoveryCmd.ask_type[0]					= 0xf0;
	IOCtrlSendDiscoveryCmd.ask_type[1]					= 0x00;
	IOCtrlSendDiscoveryCmd.ask_type[2]					= 0x00;
	IOCtrlSendDiscoveryCmd.ask_type[3]					= 0x00;
	
	memcpy(IOCtrlSendDiscoveryCmd.TF_HeaderPacket.Signature, SIGNATURE, sizeof(IOCtrlSendDiscoveryCmd.TF_HeaderPacket.Signature));
}

/*****************************************************
* Func: InitConnectPassword
* Function: init connect password
* Parameters: void
*****************************************************/
void CWiMoNetwork::InitConnectPassword()
{	
	//Byte	IOCtrlUserName[8]	= {'u', 's', 'e', 'r', 0x00, 0x00, 0x00, 0x00};
	Byte	IOCtrlPassword[8]	= {'0', '0', '0', '0', 0x00, 0x00, 0x00, 0x00};
	//Byte	IOCtrlConfig[18]		= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	memcpy(IOCtrlSendConnectPasswordCmd.TF_HeaderPacket.Signature, SIGNATURE, sizeof(IOCtrlSendConnectPasswordCmd.TF_HeaderPacket.Signature));
	//memcpy(IOCtrlSendConnectPasswordCmd.UserName, IOCtrlUserName, sizeof(IOCtrlSendConnectPasswordCmd.UserName));
	memcpy(IOCtrlSendConnectPasswordCmd.Password, IOCtrlPassword, sizeof(IOCtrlPassword));
	//memcpy(IOCtrlSendConnectPasswordCmd.Config, IOCtrlConfig, sizeof(IOCtrlSendConnectPasswordCmd.Config));
	//memcpy(IOCtrlSendConnectPasswordCmd.SupportList, IOCtrlSupportList, sizeof(IOCtrlSendConnectPasswordCmd.SupportList));

	IOCtrlSendConnectPasswordCmd.TF_HeaderPacket.ProductID		= htons(Product);
	IOCtrlSendConnectPasswordCmd.TF_HeaderPacket.Version		= mVersion;
	IOCtrlSendConnectPasswordCmd.TF_HeaderPacket.Codec			= htons(mCodec);
	IOCtrlSendConnectPasswordCmd.TF_HeaderPacket.RandCode		= IOCtrlRecvConnectCmd.TF_HeaderPacket.RandCode + htons(0x01);
	IOCtrlSendConnectPasswordCmd.TF_HeaderPacket.SeqID			= htons(0x0002);
	IOCtrlSendConnectPasswordCmd.CmdTF.CmdType				= htons(TX_PASSWORD_CMD_TYPE);
	IOCtrlSendConnectPasswordCmd.CmdTF.OPCode				= htons(0x0204);
	IOCtrlSendConnectPasswordCmd.CmdTF.DataLen				= htons(0x0010);
	//IOCtrlSendConnectPasswordCmd.ReplyTicks					= htonl(0x4567890b);
	IOCtrlSendConnectPasswordCmd.MyTicks						= htonl(0x4567890a);
	//IOCtrlSendConnectPasswordCmd.ReplySeqID					= IOCtrlRecvConnectCmd.reply_seq_id;
}

/*****************************************************
* Func: InitSendSourceStatus
* Function: init send source status
* Parameters: void
*****************************************************/
void CWiMoNetwork::InitSendSourceStatus()
{
	memcpy(IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Signature, SIGNATURE, sizeof(IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Signature));
	memcpy(IOCtrlSendSourceStatusCmd.IOCtrlDstMac, IOCtrlDstMac, sizeof(IOCtrlSendSourceStatusCmd.IOCtrlDstMac));
	memcpy(IOCtrlSendSourceStatusCmd.DestIP, IOCtrlDstIP, sizeof(IOCtrlSendSourceStatusCmd.DestIP));
	
	IOCtrlSendSourceStatusCmd.TF_HeaderPacket.ProductID		= htons(Product);
	IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Version		= mVersion;
	IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Codec		= htons(mCodec);
	IOCtrlSendSourceStatusCmd.TF_HeaderPacket.SeqID		= htons(0x0003);
	IOCtrlSendSourceStatusCmd.TF_HeaderPacket.RandCode		= IOCtrlRecvConnectPasswordCmd.TF_HeaderPacket.RandCode;
	IOCtrlSendSourceStatusCmd.CmdTF.CmdType				= htons(0x0003);
	IOCtrlSendSourceStatusCmd.CmdTF.OPCode				= htons(0x0303);
	IOCtrlSendSourceStatusCmd.CmdTF.DataLen				= htons(0x0026);
	IOCtrlSendSourceStatusCmd.InputSource					= htons(0x0001);
	IOCtrlSendSourceStatusCmd.SourceWidth					= htons((u_short)((SCREEN_WIDTH)>(SCREEN_HEIGHT) ? (SCREEN_WIDTH):(SCREEN_HEIGHT)));
	IOCtrlSendSourceStatusCmd.SourceHeight					= htons((u_short)((SCREEN_WIDTH)<(SCREEN_HEIGHT) ? (SCREEN_WIDTH):(SCREEN_HEIGHT)));
	IOCtrlSendSourceStatusCmd.SourceFrame				= htons(0x0258);
	IOCtrlSendSourceStatusCmd.XmtWidth					= IOCtrlSendSourceStatusCmd.SourceWidth;
	IOCtrlSendSourceStatusCmd.XmtHeight					= IOCtrlSendSourceStatusCmd.SourceHeight;
	IOCtrlSendSourceStatusCmd.XmtAudio					= htons(0x00);
	IOCtrlSendSourceStatusCmd.MyTicks						= htonl(nResolutionTicks);
	IOCtrlSendSourceStatusCmd.RemoteOff					= 0x03;
	IOCtrlSendSourceStatusCmd.CompressHightlow			= 0x01;
	IOCtrlSendSourceStatusCmd.HDCP_ONOFF				= 0x00;
	IOCtrlSendSourceStatusCmd.Macrovision					= 0x00;
	IOCtrlSendSourceStatusCmd.AviInfoDB					= 0x00;
	IOCtrlSendSourceStatusCmd.ScaleLevel					= 0; //m_SldScreenSizeLevelPos;
	if(SCREEN_WIDTH > SCREEN_HEIGHT)
		IOCtrlSendSourceStatusCmd.Position					= 1;
	else
		IOCtrlSendSourceStatusCmd.Position					= 0;

	if(m_cControl->GetAudioMethod() == METHOD_LNV_AUD_MIXER
		||m_cControl->GetAudioMethod() == METHOD_ZTE_HW_INTERFACE)
		IOCtrlSendSourceStatusCmd.audioSampleRate 			= 0;
	else
		IOCtrlSendSourceStatusCmd.audioSampleRate 			= 1;
	IOCtrlSendSourceStatusCmd.TvRotate					= m_cControl->GetTVRotateOrientation();
}

/*****************************************************
* Func: InitAliveInformation
* Function: init alive report info
* Parameters: void
*****************************************************/
void CWiMoNetwork::InitAliveInformation()
{
	IOCtrlSenAliveReportCmd.Header.ProductID 			= htons(Product);
	IOCtrlSenAliveReportCmd.Header.Version				= mVersion;
	IOCtrlSenAliveReportCmd.Header.Codec				= htons(mCodec);	
	IOCtrlSenAliveReportCmd.Header.RandCode			= htons(0x00);
	IOCtrlSenAliveReportCmd.Header.SeqID				= htons(0x00);
	IOCtrlSenAliveReportCmd.Command.DataLen			= htons(0x0023);
	IOCtrlSenAliveReportCmd.Command.CmdType			= htons(0x0003);
	IOCtrlSenAliveReportCmd.Command.OPCode			= htons(0x0301);
	memcpy(IOCtrlSenAliveReportCmd.Header.Signature, SIGNATURE, sizeof(IOCtrlSenAliveReportCmd.Header.Signature));

	IOCtrlSenAliveReportCmd.MyTicks 					= htonl(0x12345678);
}

/*****************************************************
* Func: SendDataLostReport
* Function: send data lost report
* Parameters: void
*****************************************************/	
void CWiMoNetwork::SendDataLostReport()
{
	size_t n = 0;
	int len = sizeof(sockaddr_in);
	
	IOCtrlSendDataLostReportCmd.Header.ProductID 			= htons(Product);
	IOCtrlSendDataLostReportCmd.Header.Version				= mVersion;
	IOCtrlSendDataLostReportCmd.Header.Codec				= htons(mCodec);	
	IOCtrlSendDataLostReportCmd.Header.RandCode			= htons(0x00);
	IOCtrlSendDataLostReportCmd.Header.SeqID				= htons(0x00);
	IOCtrlSendDataLostReportCmd.Command.DataLen			= htons(0x0004);
	IOCtrlSendDataLostReportCmd.Command.CmdType			= htons(0x0003);
	IOCtrlSendDataLostReportCmd.Command.OPCode			= htons(0x0302);
	IOCtrlSendDataLostReportCmd.MyTicks 					= htonl(0x12345678);

	memcpy(IOCtrlSendDataLostReportCmd.Header.Signature, SIGNATURE, sizeof(IOCtrlSendDataLostReportCmd.Header.Signature));
	if(socketHandle != 0)
	{
	#if CHECK_CRC	
		append_crc_32((Byte *)&(IOCtrlSendDataLostReportCmd), sizeof(IOCtrlSendDataLostReportCmd));
	#endif
		n = sendto(socketHandle,&(IOCtrlSendDataLostReportCmd),sizeof(IOCtrlSendDataLostReportCmd),0,(struct sockaddr*)&server,len); 
		if ((n==0))
		{
			LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
		}
	}
}

/*****************************************************
* Func: SendDisconnectCmd
* Function: send disconnect command
* Parameters: void
*****************************************************/	
void CWiMoNetwork::SendDisconnectCmd()
{
	size_t n;
	int len = sizeof(sockaddr_in);
	
	IOCtrlSendDisconnectReason.Header.ProductID 				= htons(Product);
	IOCtrlSendDisconnectReason.Header.Version				= mVersion;
	IOCtrlSendDisconnectReason.Header.Codec				= htons(mCodec);	
	IOCtrlSendDisconnectReason.Header.RandCode				= htons(0x00);
	IOCtrlSendDisconnectReason.Header.SeqID				= htons(0x00);
	IOCtrlSendDisconnectReason.Command.DataLen			= htons(0x0011);
	IOCtrlSendDisconnectReason.Command.CmdType			= htons(0x0002);
	IOCtrlSendDisconnectReason.Command.OPCode			= htons(0x0202);
	IOCtrlSendDisconnectReason.MyTicks 					= htonl(0x12345678);

	IOCtrlSendDisconnectReason.Disconnect_reason			= htons(iDisconnectReason);

	memcpy(IOCtrlSendDisconnectReason.Header.Signature, SIGNATURE, sizeof(IOCtrlSendDisconnectReason.Header.Signature));
	memcpy(IOCtrlSendConnectCmd.username, USERNAME, sizeof(USERNAME));

	append_crc_32((Byte*)&IOCtrlSendDisconnectReason, sizeof(IOCtrlSendDisconnectReason));

	if(socketHandle == 0)
		return;
	n = sendto(socketHandle,&IOCtrlSendDisconnectReason,sizeof(IOCtrlSendDisconnectReason),0,(struct sockaddr*)&server,len); 
	if ((n==0))
	{
		LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
	}
}

/*****************************************************
* Func: CheckCRC32
* Function: check crc 32
* Parameters: 	buffer: be checked data
*			dataLen: data length
*****************************************************/
bool CWiMoNetwork::CheckCRC32(void* buffer, int dataLen)
{
	if(buffer == NULL)
		return false;

	Byte* feedbackPacket = (Byte*)buffer;
	if(-1 == check_crc_32(feedbackPacket, dataLen))
	{
#if OUTLOG
		LOGE("check crc error\n");
#endif
		return false;
	}
#if OUTLOG
	LOGI("check crc success\n");
#endif
	return true;
}


/*****************************************************
* Func: HandshakeThread
* Function: handshake response with board
* Parameters: param: thread input parameter
*****************************************************/
void * CWiMoNetwork::HandshakeThread(void * param)
{
	CWiMoNetwork* pthis = (CWiMoNetwork*)param;
	struct timeval cmdStartTick;
	size_t n;
	int len = sizeof(sockaddr_in);
	bool runFirstTime = true;
	DDword waitTime = 0;

	sprintf(pthis->multicastIP, "%d.%d.%d.%d", pthis->TXipaddr.s_addr&0xff,pthis->TXipaddr.s_addr>>8&0xff, \
		pthis->TXipaddr.s_addr>>16&0xff,255);

	while(pthis->m_cControl->GetStatusWiMo() == WIMO_STATUS_INITING)
	{
		// Get local ethernet card ip and initialize hand shaking data between TX & RX, and send 0x0103 to RX for broadcast
		if(pthis->IOCtrlStep == 1)
		{
		#if 0
			gettimeofday(&cmdStartTick, 0);

			InitConnectPacket initconnect;
			initconnect.Header.ProductID 			= htons(Product);
			initconnect.Header.Version				= mVersion;
			initconnect.Header.Codec				= htons(mCodec);
			initconnect.Header.RandCode			= htons(0x00);
			initconnect.Header.SeqID				= htons(0x00);
			initconnect.Command.CmdType			= htons(0x0001);
			initconnect.Command.OPCode			= htons(0x0103);
			initconnect.Command.DataLen			= htons(0x0022);		
			memcpy(initconnect.Header.Signature, SIGNATURE, sizeof(initconnect.Header.Signature));
		
			pthis->server.sin_addr.s_addr = BROADCASTPACKET;
		#if CHECK_CRC
			append_crc_32((Byte*)&initconnect, sizeof(initconnect));
		#endif
			n = sendto(pthis->socketHandle,&initconnect,sizeof(initconnect),0,(struct sockaddr*)&pthis->server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				goto handshake_quit_loop;
			}
			//pthis->server.sin_addr.s_addr = inet_addr(pthis->RXDEVIP);
		#endif
			pthis->IOCtrlStep++;
			LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
		}
		// Send to RX...0x0101
		else if(pthis->IOCtrlStep == 2)
		{
			pthis->InitSendDiscoveryCmd();
			pthis->server.sin_addr.s_addr = BROADCASTPACKET;
		#if CHECK_CRC		
			append_crc_32((Byte*)&pthis->IOCtrlSendDiscoveryCmd, sizeof(pthis->IOCtrlSendDiscoveryCmd));
		#endif
			n = sendto(pthis->socketHandle,&pthis->IOCtrlSendDiscoveryCmd,sizeof(pthis->IOCtrlSendDiscoveryCmd),0,(struct sockaddr*)&pthis->server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				goto handshake_quit_loop;
			}
			
			pthis->server.sin_addr.s_addr = inet_addr(pthis->multicastIP);
			n = sendto(pthis->socketHandle,&pthis->IOCtrlSendDiscoveryCmd,sizeof(pthis->IOCtrlSendDiscoveryCmd),0,(struct sockaddr*)&pthis->server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				goto handshake_quit_loop;
			}			

			//pthis->server.sin_addr.s_addr = inet_addr(pthis->RXDEVIP);
			gettimeofday(&cmdStartTick, 0);
			pthis->IOCtrlStep++;
			LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);			
		}
		else if(pthis->IOCtrlStep == 4)
		{
			if(runFirstTime)
			{
				runFirstTime = false;
				pthis->iDisconnectReason = DISCONNECT_ERROR;
				pthis->SendDisconnectCmd();
				pthis->IOCtrlStep = 1;
				continue;
			}

			if(pthis->m_cControl->GetVideoMethod() != METHOD_TS_STREAM_INTERFACE)
			{
				waitTime = GetTickCount();
				pthis->SCREEN_WIDTH = pthis->m_cControl->GetAlignedWidth();
				pthis->SCREEN_HEIGHT = pthis->m_cControl->GetAlignedHeight();
				while(pthis->SCREEN_WIDTH == 0 && pthis->SCREEN_HEIGHT == 0 && GetTickCount() - waitTime < NumOneAndHalfThousand)
				{
					pthis->SCREEN_WIDTH = pthis->m_cControl->GetAlignedWidth();
					pthis->SCREEN_HEIGHT = pthis->m_cControl->GetAlignedHeight();
					continue;
				}
				//LOGI("InitSendSourceStatus: obtain width: %d and height: %d!\n",pthis->SCREEN_WIDTH, pthis->SCREEN_HEIGHT);
				if(pthis->SCREEN_WIDTH == 0 && pthis->SCREEN_HEIGHT == 0)
				{
					LOGE("SCREEN_WIDTH or SCREEN_HEIGHT is 0!\n");
					goto handshake_quit_loop;
				}
			}
					
			pthis->InitSendConnectCmd();
			
			pthis->IOCtrlSendConnectCmd.TF_HeaderPacket.RandCode = pthis->IOCtrlRecvDiscoveryCmd.TF_HeaderPacket.RandCode + htons(0x01);
			pthis->IOCtrlSendConnectCmd.TF_HeaderPacket.SeqID = pthis->IOCtrlRecvDiscoveryCmd.TF_HeaderPacket.SeqID + htons(0x01);
		#if CHECK_CRC	
			append_crc_32((Byte*)&pthis->IOCtrlSendConnectCmd, sizeof(pthis->IOCtrlSendConnectCmd));
		#endif
			LOGI("machine name:%s\n", pthis->IOCtrlRecvDiscoveryCmd.MachineName);
			n = sendto(pthis->socketHandle,&pthis->IOCtrlSendConnectCmd,sizeof(pthis->IOCtrlSendConnectCmd),0,(struct sockaddr*)&pthis->server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				goto handshake_quit_loop;
			}

			gettimeofday(&cmdStartTick, 0);
			pthis->IOCtrlStep++;
			LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
		}
		else if(pthis->IOCtrlStep == 6)
		{
			pthis->InitConnectPassword();
		#if CHECK_CRC		
			append_crc_32((Byte*)&pthis->IOCtrlSendConnectPasswordCmd, sizeof(pthis->IOCtrlSendConnectPasswordCmd));
		#endif
			n = sendto(pthis->socketHandle,&pthis->IOCtrlSendConnectPasswordCmd,sizeof(pthis->IOCtrlSendConnectPasswordCmd),0,(struct sockaddr*)&pthis->server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				goto handshake_quit_loop;
			}
			gettimeofday(&cmdStartTick, 0);
			pthis->IOCtrlStep++;
			LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
		}
		else if(pthis->IOCtrlStep == 8)
		{			
			pthis->InitSendSourceStatus();
		#if CHECK_CRC		
			append_crc_32((Byte*)&pthis->IOCtrlSendSourceStatusCmd, sizeof(pthis->IOCtrlSendSourceStatusCmd));
		#endif
			// OpenCode: 0x0303
			n = sendto(pthis->socketHandle,&pthis->IOCtrlSendSourceStatusCmd,sizeof(pthis->IOCtrlSendSourceStatusCmd),0,(struct sockaddr*)&pthis->server,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				goto handshake_quit_loop;
			}
			gettimeofday(&cmdStartTick, 0);
			pthis->IOCtrlStep++;
			LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
		}
		else if(pthis->IOCtrlStep == 9)  //OPCODE_TF6x0_SOURCE_STATUS_ANNOUNCE
		{
			pthis->IOCtrlStep++;
			LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
		}
		else if(pthis->IOCtrlStep == 10) //OPCODE_TF6x0_SOURCE_STATUS_ANNOUNCE
		{
			gettimeofday(&cmdStartTick, 0);
			pthis->IOCtrlStep++;
			LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
		}
		else if(pthis->IOCtrlStep == 11)
		{
			pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_CONNECTING);
			pthis->overTime = GetTickCount();
			pthis->CheckAliveResponseReportThread_hnd = CreateThreadHelper(CheckAliveAndResponseReportThread, (void*)pthis);
			if(pthis->CheckAliveResponseReportThread_hnd == 0)
			{
				LOGE("creat thread CheckAliveResponseReportThread_hnd failed!\n");
				goto handshake_quit_loop;
			}
			pthis->SendJPGVideo();
			break;
		}
		else
		{
			if(pthis->IOCtrlStep <11)
			{
				struct timeval currTime;
				gettimeofday(&currTime, 0);
				if((currTime.tv_sec*NumOneThousand - 
					cmdStartTick.tv_sec*NumOneThousand + currTime.tv_usec/NumOneThousand - cmdStartTick.tv_usec/NumOneThousand) > 500)
				{
					if(pthis->IOCtrlStep  == 3)
						pthis->IOCtrlStep  = 1;
					else if(pthis->IOCtrlStep  == 5)
						pthis->IOCtrlStep  = 1;
					else if(pthis->IOCtrlStep  == 7)
						pthis->IOCtrlStep  = 1;
				}
				usleep(QUICKWAITTIMES);
			}
		}
	}
	LOGI("HandshakeThread normal end\n");
	return 0;
	
handshake_quit_loop:
	LOGI("HandshakeThread exception end\n");
	pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH); 
	return (void*)E_FAILED;
}

/*****************************************************
* Func: CheckAliveAndResponseReportThread
* Function: the thread check wether alive or not to response
* Parameters: param: thread input parameter
*****************************************************/
void* CWiMoNetwork::CheckAliveAndResponseReportThread(void * param)
{
	CWiMoNetwork* pthis = (CWiMoNetwork*)param;

	size_t n;
	int len = sizeof(sockaddr_in);
	DDword sendDiscoverAndAliveTime, lastickTime = 0;
	struct sockaddr_in tServer;
	int tvOrientation = 0;
	
	memset(&tServer, 0,sizeof(tServer));
	tServer.sin_family=AF_INET;
	tServer.sin_port=htons(UDP_PORT);
	tServer.sin_addr.s_addr=htonl(INADDR_ANY);
	
	pthis->InitSendDiscoveryCmd();
	pthis->InitAliveInformation();
#if CHECK_CRC		
	append_crc_32((Byte*)&pthis->IOCtrlSendDiscoveryCmd, sizeof(pthis->IOCtrlSendDiscoveryCmd));
	append_crc_32((Byte*)&(pthis->IOCtrlSenAliveReportCmd), sizeof(pthis->IOCtrlSenAliveReportCmd));
#endif

	tvOrientation = pthis->m_cControl->GetTVRotateOrientation();
	sendDiscoverAndAliveTime = lastickTime = GetTickCount();
	while(pthis->m_cControl->GetStatusWiMo() == WIMO_STATUS_CONNECTING)
	{
		if((GetTickCount() - pthis->overTime) > NumTimeOut)
		{
			pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);
			pthis->iDisconnectReason = DISCONNECT_TIMEOUT;
			LOGI("miss server and disconnection!\n");
			break;
		}

		if(pthis->m_cControl->GetRotateOrientation() != pthis->m_cControl->GetUIRotateOrientation()
			|| tvOrientation != pthis->m_cControl->GetTVRotateOrientation()
			|| (GetTickCount() - lastickTime > NumOneThousand))
		{
			if(pthis->m_cControl->GetRotateOrientation() != pthis->m_cControl->GetUIRotateOrientation())
				pthis->m_cControl->SetRotateOrientation(pthis->m_cControl->GetUIRotateOrientation());
			if(tvOrientation != pthis->m_cControl->GetTVRotateOrientation())
				tvOrientation = pthis->m_cControl->GetTVRotateOrientation();
			pthis->SendTick();
			//LOGI("send tick every second\n");
			lastickTime = GetTickCount();
		}
			
		if((GetTickCount() - sendDiscoverAndAliveTime > NumAliveDummyTime) && (pthis->socketHandle != 0))
		{
		#if OUTLOG
			LOGI("TX send discovery and alive report\n");
		#endif
			tServer.sin_addr.s_addr = BROADCASTPACKET;
			n = sendto(pthis->socketHandle,&(pthis->IOCtrlSendDiscoveryCmd),sizeof(pthis->IOCtrlSendDiscoveryCmd),0,(struct sockaddr*)&tServer,len); 
			if ((n==0))
			{
				LOGE("BROADCASTPACKET,send IOCtrlSendDiscoveryCmd to server failed n:%d errno:%d \n",(int)n,(int)errno);
			}
			tServer.sin_addr.s_addr = inet_addr(pthis->multicastIP);
			n = sendto(pthis->socketHandle,&pthis->IOCtrlSendDiscoveryCmd,sizeof(pthis->IOCtrlSendDiscoveryCmd),0,(struct sockaddr*)&tServer,len); 
			if ((n==0))
			{
				LOGE("multicastIP, send IOCtrlSendDiscoveryCmd to server failed n:%d errno:%d \n",(int)n,(int)errno);
			}	
			n = sendto(pthis->socketHandle,&(pthis->IOCtrlSenAliveReportCmd),sizeof(pthis->IOCtrlSenAliveReportCmd),0,(struct sockaddr*)&pthis->server,len); 
			if ((n==0))
			{
				LOGE("send IOCtrlSenAliveReportCmd to server failed n:%d errno:%d \n",(int)n,(int)errno);
			}
			sendDiscoverAndAliveTime = GetTickCount();
		}
		usleep(QUICKWAITTIMES);
	}

	LOGI("CheckAliveAndResponseReportThread end!\n");
	return 0;
}

/*****************************************************
* Func: NetworkRecvThread
* Function: the thread receives information from the device
* Parameters: param: thread input parameter
*****************************************************/
void * CWiMoNetwork::NetworkRecvThread(void * param)
{
	CWiMoNetwork *pthis = (CWiMoNetwork*)param;
	char strIP[128];
	unsigned char buffer[1024];
	int len = sizeof(sockaddr_in);
	long dataLostTimes = 0;
	int recvLen = 0;

	memset(strIP, 0, sizeof(strIP));
	memset(buffer, 0, sizeof(buffer));
	struct sockaddr_in recvserver;
	memset(&recvserver, 0,sizeof(recvserver));
	
	while(pthis->m_cControl->GetStatusWiMo() != WIMO_STATUS_FINISH)
	{
		if((recvLen = recvfrom(pthis->socketHandle, buffer, sizeof(buffer), 0, (struct sockaddr*)&recvserver,&len)) == 0)
		{
			usleep(NumOneThousand/NETWORK_RECV_FPS*NumOneThousand);
			continue;
		}
		else
		{
//			usleep(1*NumOneThousand);
		}
		
		HeaderPacket* header	= (HeaderPacket*)buffer;
		CmdPacket* cmd = (CmdPacket*)(buffer+ sizeof(HeaderPacket));

#if OUTLOG
		LOGE("buffer:%x,Recv cmd :0x%x\n",*header->Signature,ntohs(cmd->OPCode));
		LOGI("recvserver.sin_addr.s_addr:%d.%d.%d.%d\n",
			recvserver.sin_addr.s_addr&0xff,
			recvserver.sin_addr.s_addr>>8&0xff,
			recvserver.sin_addr.s_addr>>16&0xff,
			recvserver.sin_addr.s_addr>>24&0xff);
#endif	

		if(pthis->TXipaddr.s_addr == recvserver.sin_addr.s_addr || memcmp(header->Signature, SIGNATURE, 4) != 0)
		{
		#if OUTLOG
			LOGE("It is not belong to server response\n");
		#endif
			usleep(NumOneThousand);
			continue;
		}
	#if CHECK_CRC
		if(recvLen < ntohs(cmd->DataLen) + 19+4 || !pthis->CheckCRC32(buffer, ntohs(cmd->DataLen) + 19+4))
		{
			//LOGE("recv: check CRC error, datalen: %x\n", ntohs(cmd->DataLen));
			usleep(NumOneThousand);
			continue;
		}
	#endif
		sprintf(strIP, "%d.%d.%d.%d", recvserver.sin_addr.s_addr&0xff,recvserver.sin_addr.s_addr>>8&0xff,\
								recvserver.sin_addr.s_addr>>16&0xff,recvserver.sin_addr.s_addr>>24&0xff);
		
		switch(ntohs(cmd->OPCode)) 
		{
		case OPCODE_TF6x0_DISCOVER:
			if(pthis->IOCtrlStep != 3) 
			{
			#if OUTLOG
				LOGI("L:%d\n",__LINE__);
			#endif
				break;
			}
			else 
			{
				strcpy(pthis->RXDEVIP, strIP);
				*((Dword*)&pthis->IOCtrlDstIP[0]) = inet_addr(pthis->RXDEVIP);
				pthis->server.sin_addr.s_addr = inet_addr(strIP);
				memcpy((Char*)&pthis->IOCtrlRecvDiscoveryCmd, buffer, ntohs(cmd->DataLen) + 19);
				pthis->IOCtrlStep += 1;
				LOGE("Move from %d to %d\n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
			}
			break;

		case OPCODE_TF6x0_CONNECTING:
			if(pthis->IOCtrlStep != 5) 
			{
				LOGI("L:%d\n",__LINE__);
				break;
			}
			else 
			{
				strcpy(pthis->RXDEVIP, strIP);
				*((Dword*)&pthis->IOCtrlDstIP[0]) = inet_addr(pthis->RXDEVIP);
				memcpy((Char*)&pthis->IOCtrlRecvConnectCmd, buffer, ntohs(cmd->DataLen) + 19);
				++pthis->IOCtrlStep;
				LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
			}
			break;

		case OPCODE_TF6x0_CONNECTING_PASSWORD:
			if(pthis->IOCtrlStep != 7 || ntohs(cmd->CmdType) != RX_PASSWORD_CMD_TYPE) 
			{
				LOGI("L:%d\n",__LINE__);
				break;
			}
			else 
			{
				strcpy(pthis->RXDEVIP, strIP);
				*((Dword*)&pthis->IOCtrlDstIP[0]) = inet_addr(pthis->RXDEVIP);
				memcpy((Char*)&pthis->IOCtrlRecvConnectPasswordCmd, buffer, ntohs(cmd->DataLen) + 19);
				++pthis->IOCtrlStep;
				LOGE("Move from %d to %d \n",pthis->IOCtrlStep-1, pthis->IOCtrlStep);
			}
			break;
			
		case OPCODE_TF6x0_DISCONNECTING:
			LOGI("disconnecting\n");
			if(pthis->m_cControl->GetStatusWiMo() == WIMO_STATUS_CONNECTING)
			{
				pthis->iDisconnectReason = DISCONNECT_KICKED;
				pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);
			}
			break;
			
		case OPCODE_TF6x0_ALIVE_REPORT:
			//LOGI("alive\n");
			pthis->overTime = GetTickCount();
			dataLostTimes = 0;
			break;
			
		case OPCODE_TF6x0_DATALOST_REPORT:
			dataLostTimes++;
			if(dataLostTimes >= DATALOSTMAXTIMES)
			{
				LOGE("datalost\n");
				pthis->iDisconnectReason = DISCONNECT_DATALOST;
				pthis->SendDataLostReport();
				pthis->m_cControl->SetStatusWiMo(WIMO_STATUS_FINISH);
			}
			break;
			
		default:
			break;
		}

		memset(buffer, 0, sizeof(buffer));
	}

	pthis->SendDisconnectCmd();
	pthis->m_cControl->NotifyUIStop();
	LOGI("NetworkRecvThread normal end\n");

	return 0;
}

/*****************************************************
* Func: SendJPGVideo
* Function: send video data
* Parameters: void
*****************************************************/
int CWiMoNetwork::SendJPGVideo()
{
	char* JpegData = NULL;
	char* tmpBuff = NULL;
	int FileLength = 0;
	int firstFrame = 0;
	int frames = 0;
	DDword sTimes = 0;
	DDword testTime = 0;

#if ANDROID_RESOURCE_BUILDER
	pid_t tid  = gettid();
	LOGI("SendJPGVideo tid: %d\n", tid);
	set_sched_policy(tid, SP_FOREGROUND);
	setpriority(PRIO_PROCESS, tid, ANDROID_PRIORITY_URGENT_DISPLAY);
#endif	
	sTimes = GetTickCount();
	while(m_cControl != NULL && m_cControl->GetStatusWiMo() != WIMO_STATUS_FINISH 
		&& m_cControl->GetVideoMethod() != METHOD_TS_STREAM_INTERFACE)
	{	
		int len = sizeof(sockaddr_in);
		int FileReadPos = 0;
		struct sockaddr_in serverVideo;
		memset(&serverVideo, 0,sizeof(serverVideo));
		serverVideo.sin_family=AF_INET;
		serverVideo.sin_addr.s_addr=server.sin_addr.s_addr;

		if(m_cControl->GetVideoMethod() == METHOD_HW_VIDEO_INTERFACE)
		{
			tmpBuff = m_cControl->GetVideoDataBuff(&FileLength);
			if(tmpBuff == NULL || FileLength == 0)
			{
				usleep(2*NumOneThousand);
				continue;
			}
			memcpy(sendSrcBuff, tmpBuff, FileLength);
			JpegData = sendSrcBuff;
			m_cControl->SetCallingBuffIndex(CALLINGBUFFER2);
		}
		else if(m_cControl->pCVideo != NULL)
		{
			tmpBuff = m_cControl->pCVideo->GetJPEGData(&FileLength);
			if(tmpBuff == NULL || FileLength == 0)
			{
				usleep(5*NumOneThousand);
				continue;
			}
			memcpy(sendSrcBuff, tmpBuff, FileLength);
			JpegData = sendSrcBuff;
		}
		if(m_cControl->GetVideoMethod() == METHOD_LNV_HW_INTERFACE)
			m_cControl->SetCallingBuffIndex(CALLINGBUFFER2);
		if(FileLength == 0)
			continue;
		//LOGI("FileLength:%d\n", FileLength);

		//testTime = GetTickCount();

		sendVideoTXbuffer[0] = sendVideoTXbuffer[1] = sendVideoTXbuffer[2] = sendVideoTXbuffer[3] = 0;
		sendVideoTXbuffer[4] = (frameNumber>>8) &0xff;
		sendVideoTXbuffer[5] = frameNumber &0xff;
		memset(&sendVideoTXbuffer[6],0,12);

		serverVideo.sin_port=htons(VIDEOPORT1); 
		sendto(send1SocketHandle,sendVideoTXbuffer,18,0,(struct sockaddr*)&serverVideo,len); 

		int lastpacketLength = (int)(FileLength%((int)1456));
		for(short i=0;i< FileLength/(1456);i++)
		{
			sendVideoTXbuffer[0] = (frameNumber>>8) &0xff;
			sendVideoTXbuffer[1] = frameNumber &0xff;
			sendVideoTXbuffer[2] = (i>>8) &0xff;
			sendVideoTXbuffer[3] = i &0xff;

			memcpy(&sendVideoTXbuffer[4], &JpegData[FileReadPos],1456);
			FileReadPos += 1456;

			serverVideo.sin_port=htons(VIDEOPORT2); 
			ssize_t n = sendto(send2SocketHandle,sendVideoTXbuffer,1456+4,0,(struct sockaddr*)&serverVideo,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return E_FAILED;
			}
			//针对Height=1280的手机做的特殊处理，i = 68时是100K的数据量，sleep 15ms.
			if((i != 0 && i % 68 == 0) && (m_cControl->GetDeviceHeight() == 1280)) 
				usleep(15*NumOneThousand);
		}

		if( lastpacketLength< 64)
		{
			sendVideoTXbuffer[0] = (frameNumber>>8) &0xff;
			sendVideoTXbuffer[1] = frameNumber &0xff;
			sendVideoTXbuffer[2] = (((FileLength/1456)>>8) &0xff) | 0x80;
			sendVideoTXbuffer[3] = (FileLength/1456) &0xff;

			memcpy(&sendVideoTXbuffer[4], &JpegData[FileReadPos],lastpacketLength);
			FileReadPos += lastpacketLength;

			memset(&sendVideoTXbuffer[4 + lastpacketLength], 0, 64-lastpacketLength);
			ssize_t n = sendto(send2SocketHandle,sendVideoTXbuffer,4+64,0,(struct sockaddr*)&serverVideo,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return E_FAILED;
			}
		}
		else
		{
			int invalidLength = -1;
			sendVideoTXbuffer[0] = (frameNumber>>8) &0xff;
			sendVideoTXbuffer[1] = frameNumber &0xff;
			sendVideoTXbuffer[2] = (((FileLength/1456)>>8) &0xff) ;
			sendVideoTXbuffer[3] = (FileLength/1456) &0xff;

			memcpy(&sendVideoTXbuffer[4], &JpegData[FileReadPos],lastpacketLength);
			FileReadPos += lastpacketLength;

			for(int i=0;i<lastpacketLength  - 1;i++)
			{
				if(sendVideoTXbuffer[4+i] == 0xff && sendVideoTXbuffer[4+i+1] == 0xd9)
				{
					invalidLength = i;
					break;
				}
			}
			if(invalidLength == -1)
			{
				invalidLength = lastpacketLength; 
			}
			ssize_t n = sendto(send2SocketHandle,sendVideoTXbuffer,4+ invalidLength,0,(struct sockaddr*)&serverVideo,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return E_FAILED;
			}

			sendVideoTXbuffer[0] = (frameNumber>>8) &0xff;
			sendVideoTXbuffer[1] = frameNumber &0xff;
			sendVideoTXbuffer[2] = (((FileLength/1456+1)>>8) &0xff) |0x80;
			sendVideoTXbuffer[3] = (FileLength/1456+1) &0xff;

			sendVideoTXbuffer[4] = 0xff;
			sendVideoTXbuffer[5] = 0xd9;
			memset(&sendVideoTXbuffer[6],0,62);

			n = sendto(send2SocketHandle,sendVideoTXbuffer,4+ 64,0,(struct sockaddr*)&serverVideo,len); 
			if ((n==0))
			{
				LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return E_FAILED;
			}
		}
		if(firstFrame == 0)
		{
			m_cControl->pCNetwork->SendTick();
			firstFrame++;
		}			
		frameNumber++;
		//LOGI("testTime :%lld\n", GetTickCount() - testTime);
		frames++;
	/*	
		if(GetTickCount() - sTimes > NumOneThousand)
		{
			LOGI("send fps: %d", frames);
			frames = 0;
			sTimes = GetTickCount();
		}
	*/
		if(m_cControl->GetVideoMethod() != METHOD_LNV_HW_INTERFACE)
		{
			if(m_cControl->GetCallingBuffIndex() == CALLINGBUFFER1)
				m_cControl->SetCallingBuffIndex(CALLINGBUFFER2);
			else if(m_cControl->GetCallingBuffIndex() == CALLINGBUFFER2)
				m_cControl->SetCallingBuffIndex(CALLINGBUFFER1);
		}
	}

	return S_OK;
}

/*****************************************************
* Func: SendTick
* Function: send serveral protocol parameters
* Parameters: void
*****************************************************/
void CWiMoNetwork::SendTick(void)
{
	if(m_cControl->GetStatusWiMo() == WIMO_STATUS_CONNECTING)
	{
		SCREEN_WIDTH = m_cControl->GetAlignedWidth();
		SCREEN_HEIGHT = m_cControl->GetAlignedHeight();
		if(m_cControl->GetVideoMethod() != METHOD_TS_STREAM_INTERFACE
			&& (SCREEN_WIDTH == 0 || SCREEN_WIDTH == 0))
		{
			LOGE("SCREEN_WIDTH: %d,SCREEN_HEIGHT:%d, is 0\n",SCREEN_WIDTH, SCREEN_HEIGHT);
			return;
		}
			
		memcpy(IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Signature, SIGNATURE, sizeof(IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Signature));
		memcpy(IOCtrlSendSourceStatusCmd.IOCtrlDstMac, IOCtrlDstMac, sizeof(IOCtrlSendSourceStatusCmd.IOCtrlDstMac));
		memcpy(IOCtrlSendSourceStatusCmd.DestIP, IOCtrlDstIP, sizeof(IOCtrlSendSourceStatusCmd.DestIP));
		
		IOCtrlSendSourceStatusCmd.TF_HeaderPacket.ProductID		= htons(Product);
		IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Version		= mVersion;
		IOCtrlSendSourceStatusCmd.TF_HeaderPacket.Codec		= htons(mCodec);
		IOCtrlSendSourceStatusCmd.TF_HeaderPacket.SeqID		= htons(0x0003);
		IOCtrlSendSourceStatusCmd.TF_HeaderPacket.RandCode		= IOCtrlRecvConnectCmd.TF_HeaderPacket.RandCode;
		IOCtrlSendSourceStatusCmd.CmdTF.CmdType				= htons(0x0003);
		IOCtrlSendSourceStatusCmd.CmdTF.OPCode				= htons(0x0303);
		IOCtrlSendSourceStatusCmd.CmdTF.DataLen				= htons(0x0026);
		IOCtrlSendSourceStatusCmd.InputSource					= htons(0x0001);
		IOCtrlSendSourceStatusCmd.SourceFrame				= htons(0x0258);
		IOCtrlSendSourceStatusCmd.XmtAudio					= htons(0x00);
		IOCtrlSendSourceStatusCmd.MyTicks				 		= htonl(++nResolutionTicks);
		IOCtrlSendSourceStatusCmd.RemoteOff					= 0x03;
		IOCtrlSendSourceStatusCmd.CompressHightlow			= 0x01;
		IOCtrlSendSourceStatusCmd.HDCP_ONOFF				= 0x00;
		IOCtrlSendSourceStatusCmd.Macrovision					= 0x00;
		IOCtrlSendSourceStatusCmd.AviInfoDB					= 0x00;
		IOCtrlSendSourceStatusCmd.ScaleLevel					= 0;
		
		if(m_cControl->GetAudioMethod() == METHOD_LNV_AUD_MIXER
			||m_cControl->GetAudioMethod() == METHOD_ZTE_HW_INTERFACE)
			IOCtrlSendSourceStatusCmd.audioSampleRate 			= 0;
		else
			IOCtrlSendSourceStatusCmd.audioSampleRate 			= 1;

		if(SCREEN_WIDTH > SCREEN_HEIGHT)
		{
			if(m_cControl->GetRotateOrientation() == ROTATION_90 || m_cControl->GetRotateOrientation() == ROTATION_270)
				IOCtrlSendSourceStatusCmd.Position 			= 1;
			else
				IOCtrlSendSourceStatusCmd.Position 			= 0;
		}
		else
		{
			if(m_cControl->GetRotateOrientation() == ROTATION_90 || m_cControl->GetRotateOrientation() == ROTATION_270)
				IOCtrlSendSourceStatusCmd.Position 			= 0;
			else
				IOCtrlSendSourceStatusCmd.Position 			= 1;
		}

		IOCtrlSendSourceStatusCmd.SourceWidth					= htons((u_short)((SCREEN_WIDTH)>(SCREEN_HEIGHT) ? (SCREEN_WIDTH):(SCREEN_HEIGHT)));
		IOCtrlSendSourceStatusCmd.SourceHeight					= htons((u_short)((SCREEN_WIDTH)<(SCREEN_HEIGHT) ? (SCREEN_WIDTH):(SCREEN_HEIGHT)));
		
		IOCtrlSendSourceStatusCmd.XmtWidth 					= IOCtrlSendSourceStatusCmd.SourceWidth;
		IOCtrlSendSourceStatusCmd.XmtHeight 					= IOCtrlSendSourceStatusCmd.SourceHeight;
		IOCtrlSendSourceStatusCmd.TvRotate					= m_cControl->GetTVRotateOrientation();
	#if CHECK_CRC	
		append_crc_32((Byte*)&IOCtrlSendSourceStatusCmd, sizeof(IOCtrlSendSourceStatusCmd));
	#endif
		int len = sizeof(sockaddr_in);
		ssize_t n = sendto(socketHandle,&IOCtrlSendSourceStatusCmd,sizeof(IOCtrlSendSourceStatusCmd),0,(struct sockaddr*)&server,len); 
		if ((n==0))
		{
			LOGE("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
		}
	}
}


