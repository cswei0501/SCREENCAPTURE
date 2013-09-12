// TestSocket_win.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int g_quit = 0;
sockaddr_in g_SenderAddr;

const int DeviceType_IATV       = (0x01 << 1) | 1;
const int DeviceType_IATVCtrl   = (0x03 << 1);
const int DeviceStatusReportCmd           = 0x0002;
const int MagicNumber = 0x5678;
const int StartCaptureScreenCmd           = 0x0016;
const int EndCaptureScreenCmd             = 0x0017;
const int UpdateCaptureScreenCmd          = 0x0018; 

struct StartCaptureScreen
{
	int  MagicNumber;
	int  packet_size;
	int  type;
	int  mode;
};
struct UpdateCaptureScreen
{
	int  MagicNumber;
	int  packet_size;
	int  type;
	int  left;
	int  right;
	int  top;
	int  bottom;
	int  mediatype;                               //1 video    ; 2 audio
	char content[1];
};
struct DeviceStatusReport
{
	int  MagicNumber;
	int  packet_size;
	int  type;
	int  DeviceType;
	int  DeviceStatus;
	int  port; 
	char   devicename[128];
};
DWORD TcpProc(LPVOID p)
{
	int handle_;
	int ret = 0;
	struct sockaddr_in local;

	local.sin_family=AF_INET;
	local.sin_port=0;
	local.sin_addr.s_addr = INADDR_ANY;

	handle_=(int)socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	ret = bind(handle_,(struct sockaddr*)&local,sizeof(local));
	if(ret == -1)
	{
		closesocket(handle_);
		return ret;
	}

	int n = 64*1024;
	setsockopt(handle_, SOL_SOCKET, SO_SNDBUF, (const char*)&n, sizeof(int));
	//setsockopt(g_sock, SOL_SOCKET, SO_RCVBUF, (const char*)&n, sizeof(int));

	//int iMode = 1;
	//ioctlsocket(handle_, FIONBIO, (u_long FAR*) &iMode);

	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.S_un.S_addr = g_SenderAddr.sin_addr.S_un.S_addr;
	saddr.sin_port = htons(g_SenderAddr.sin_port);

	char* x = inet_ntoa(saddr.sin_addr);
	ret = connect(handle_, (sockaddr*)&saddr, sizeof(saddr) );
	
	DWORD err = GetLastError();

	int active = 1;
	char buffer[256] = {0};
	DeviceStatusReport *ptr = (DeviceStatusReport *)buffer;

	ptr->type = htonl(DeviceStatusReportCmd);
	ptr->MagicNumber = htonl(MagicNumber);
	ptr->packet_size = htonl(sizeof(DeviceStatusReport));
	ptr->DeviceType = htonl(DeviceType_IATVCtrl);
	ptr->DeviceStatus = htonl(active);
	gethostname(ptr->devicename, 128);
	send(handle_, buffer, sizeof(DeviceStatusReport), 0 );
	
	memset(buffer, 0, 256);
	StartCaptureScreen *ptr2 = (StartCaptureScreen *) buffer;
	ptr2->type = htonl(StartCaptureScreenCmd);
	ptr2->MagicNumber = htonl(MagicNumber);
	ptr2->packet_size = htonl(sizeof(StartCaptureScreen));
	ptr2->mode = 0; //Text mode
	send(handle_, buffer, sizeof(StartCaptureScreen), 0 );

	int len = 0;
	char* buf = new char[500*1024 + 32];
	FILE* fjpg = fopen("d://test_800x480.jpg", "rb");
	if(fjpg)
	{
		len = fread(buf + 32, 1, 500*1024, fjpg);
		fclose(fjpg);
	}
	len += 32;
	while(!g_quit)
	{
		UpdateCaptureScreen* ptr3 = (UpdateCaptureScreen*)buf;
		ptr3->MagicNumber = htonl(MagicNumber);
		ptr3->packet_size = htonl(len);
		ptr3->type = htonl(UpdateCaptureScreenCmd);
		ptr3->left = ptr3->top = 0;
//		ptr3->right = htonl(1920);
//		ptr3->bottom = htonl(1080);
		ptr3->right = htonl(800);
		ptr3->bottom = htonl(480);
		ptr3->mediatype = htonl(1); //JPEG
		
		send(handle_, buf, len, 0 );
		Sleep(50);
	}
	delete buf;

	ptr2->type = htonl(EndCaptureScreenCmd);
	ptr2->MagicNumber = htonl(MagicNumber);
	ptr2->packet_size = htonl(sizeof(StartCaptureScreen));
	send(handle_, buf, 12, 0 );
	return 0;
}

HANDLE m_hThread1 = 0;

const int BroadcastBeginPort    = 6180;
const int BroadcastEndPort      = 6190;
const int EchoCmd                         = 0x001a;
const int EchoRespCmd                     = 0x001b;

struct Echo{
	int  MagicNumber;
	int  packet_size;
	int  type;
	int  TimeLow32;
	int  TimeHigh32;
	int  Data;
	char   Data1[16];
};

DWORD UdpProc(LPVOID pVoid)
{
	sockaddr_in sin;
	int fbroadcast = 1;
	int handle_ =(int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt ( handle_,SOL_SOCKET,SO_BROADCAST,(char *)&fbroadcast,sizeof ( int ));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(BroadcastBeginPort);
	sin.sin_addr.s_addr = INADDR_ANY;

	int optval = 1;
	setsockopt(handle_,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(int));

	if(bind( handle_, (sockaddr *)&sin, sizeof(sin))!=0)
	{
		closesocket(handle_);
		handle_ = INVALID_SOCKET;
		return 0;
	}

	int active = 1;
	char buffer[256] = {0};
	DeviceStatusReport *ptr = (DeviceStatusReport *)buffer;

	ptr->type = htonl(DeviceStatusReportCmd);
	ptr->MagicNumber = htonl(MagicNumber);
	ptr->packet_size = htonl(sizeof(DeviceStatusReport));
	ptr->DeviceType = htonl(DeviceType_IATVCtrl);
	ptr->DeviceStatus = htonl(active);
	gethostname(ptr->devicename, 128);

	sockaddr_in saudpserv;
	saudpserv.sin_family = AF_INET;
	saudpserv.sin_addr.s_addr = htonl ( INADDR_BROADCAST );
	saudpserv.sin_port = htons (BroadcastBeginPort);
	sendto(handle_, buffer, sizeof(DeviceStatusReport), 0, (const sockaddr*)&saudpserv, sizeof(sockaddr_in) );

	while(!g_quit)
	{
		if(!m_hThread1)
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

			sockaddr_in saudpserv;
			saudpserv.sin_family = AF_INET;
			saudpserv.sin_addr.s_addr = htonl ( INADDR_BROADCAST );
			saudpserv.sin_port = htons (BroadcastBeginPort);
			sendto(handle_, buffer, sizeof(DeviceStatusReport), 0, (const sockaddr*)&saudpserv, sizeof(sockaddr_in) );
		}

		fd_set rdfds_;
		fd_set exceptfds_;
		int maxfd_ = 0;

		FD_ZERO(&rdfds_);
		FD_ZERO(&exceptfds_);

		FD_SET(handle_,&rdfds_);
		FD_SET(handle_,&exceptfds_);
		maxfd_ = handle_;

		//struct timeval t;
		//t.tv_sec = 0;
		//t.tv_usec =1000;
		//int ret = select(maxfd_,&rdfds_,0,&exceptfds_,&t);
		int ret = select(maxfd_+1,&rdfds_,0,&exceptfds_,NULL);
		if(ret < 0)
		{
			return 0;
		}
		if(ret == 0)
		{
			Sleep(10);
			continue;
		}

		if(FD_ISSET(maxfd_,&exceptfds_))
		{
			FD_CLR(maxfd_,&exceptfds_);
			break;
		}
	
		if (FD_ISSET(maxfd_, &rdfds_)) 
		{
			FD_CLR(maxfd_, &rdfds_);
			
			char recvbuff[1024];
			memset(recvbuff,0,1024);
			int SenderAddrSize = sizeof(g_SenderAddr);
			int len = recvfrom( handle_,recvbuff, 256, 0, (sockaddr *) &g_SenderAddr,&SenderAddrSize);
			if(len == SOCKET_ERROR)
			{
				break;
			}

			DeviceStatusReport *ptr = (DeviceStatusReport *)recvbuff;
			if( ntohl(ptr->MagicNumber) == MagicNumber && ntohl(ptr->packet_size) <= len )
			{
				int type = ntohl(ptr->type);
				int devtype = ntohl(ptr->DeviceType);
				if( ntohl(ptr->type) == DeviceStatusReportCmd 
					&& (ntohl(ptr->DeviceType) & DeviceType_IATV ) == DeviceType_IATV )
				{
					if(m_hThread1 == NULL)
					{
						DWORD id;
						g_SenderAddr.sin_port = htonl(ptr->port);
						m_hThread1 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)TcpProc, &g_SenderAddr, 0, &id);
						
					}
					break;
				}
			}
			Echo* ptr2 = (Echo*)recvbuff;
			if( ntohl(ptr2->MagicNumber) == MagicNumber && ntohl(ptr2->packet_size) <= len )
			{
				if( ntohl(ptr2->type) == EchoCmd )
				{
					saudpserv.sin_family = AF_INET;
					saudpserv.sin_addr.s_addr = htonl(g_SenderAddr.sin_addr.S_un.S_addr);
					saudpserv.sin_port = htons(g_SenderAddr.sin_port);
					ptr2->type = htonl(EchoRespCmd);
					sendto(handle_, recvbuff, sizeof(Echo), 0, (const sockaddr*)&saudpserv, sizeof(sockaddr_in) );
					break;
				}
			}
		}
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	g_quit = 0;
	m_hThread1 = NULL;
	DWORD id;
	HANDLE hThread2 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)UdpProc, 0, 0, &id);

	getchar();
	WSACleanup();
	return 0;
}


