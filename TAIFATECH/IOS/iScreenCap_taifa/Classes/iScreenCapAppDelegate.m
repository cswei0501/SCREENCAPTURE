//
//  iScreenCapAppDelegate.m
//  iScreenCap
//
//  Created by weiqiang xu on 11-6-24.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#import <UIKit/UIKit.h>
#import <AssetsLibrary/AssetsLibrary.h>
int count = 0;
CGImageRef UIGetScreenImage(void);

#include <sys/socket.h> 
#include <sys/sockio.h> 
#include <net/if.h> 
#include <netinet/in.h>


#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // Per msqr
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>

#include "ci_jpegenc.h"
typedef char									Char;													///< char
typedef char*									PChar;												///< char *
typedef const char*						PCChar;												///< const char *
typedef unsigned char					Byte;													///< unsigned char
typedef unsigned char*				PByte;												///< unsigned char *
typedef const unsigned char*	PCByte;												///< const unsigned char *
typedef short									Short;												///< short
typedef short*								PShort;												///< short *
typedef unsigned short				Word;													///< unsigned short
typedef unsigned short*				PWord;												///< unsigned short *
typedef int										Int;													///< int
typedef int*									PInt;													///< int *
typedef unsigned int					Dword;												///< unsigned int
typedef unsigned int*					PDword;												///< unsigned int *
typedef float									Float;												///< float
typedef float*								PFloat;												///< float *
typedef bool									BL;	

#define UDP_PORT							48689
#define JPEG_PORT							2068
#define	SND_PORT							2066
#define COM_PORT							2067
#define Product								0x6301 
#define STEP_CHK_MAX_COUNTER	1000		// Each counter is 10ms

#define SERVERADDR  inet_addr("192.168.168.56")

struct sockaddr_in serverVideo;
char* pTXbuffer = NULL;
typedef struct HeaderPacket
{
	Byte								Signature[4];
	Word 								ProductID;
	Word 								RandCode;
	Word 								SeqID;
}TF_HeaderPacket;

typedef struct CmdPacket														// 6Bytes
{
	Word								CmdType;
	Word								OPCode;
	Word								DataLen;
}CmdTF; 


struct InitConnectPacket
{
	struct HeaderPacket				Header;
	struct CmdPacket						Command;
};


struct CommnadPacket																// 0x0103
{
	struct HeaderPacket				TF_HeaderPacket;							// 002A
	struct CmdPacket						CmdTF;												// 0034
	Byte 								IOCtrlHeader[6];							// 003A
	Byte 								Reserve[12];
	Byte 								MachineName[12];
	Word								SeqNum;
	Byte 								DummySpace[124];
};

struct SessionPacket																// 0x0201
{
	struct HeaderPacket				TF_HeaderPacket;							// 002A
	struct CmdPacket						CmdTF;												// 0034
	Byte 								PCIp[4];											// 4
	Word								CmdPort;											// 2
	Byte 								ProductType[4];								// 4
	Word								ProtocolVersion;							// 2
	Byte 								MyGroupName[12];							// 12
	Byte 								MachineName[12];							// 12
	Dword					 			MyTicks;											// 006C
	Dword 							ReplyTicks;										// 4
	Word								ReplySeqID;										// 2
};

struct BroadcastPacket															// 0x0101
{
	struct HeaderPacket				TF_HeaderPacket;							// 002A
	struct CmdPacket						CmdTF;												// 0034
	Byte 								ProductType[4];								// 003A
	Word								ProtocolVersion;
	Byte								GroupName[12];
	Byte 								MachineName[12];
	Dword								AskType;
};

struct Recv_Command																	// 0x0201
{
	struct HeaderPacket				TF_HeaderPacket;							// 002A
	struct CmdPacket						CmdTF;												// 0034
	Byte 								MyIP[4];											// 0040
	Word								CmdPort;
	Byte								Reserve[4];
	Byte								ProductType[4];
	Word								ProtocolVersion;
	Byte 								MyGroupName[12];							// 12
	Byte 								MachineName[12];				
	Byte 								UserName[8];									// 0062
	Dword					 			MyTicks;											// 006C
};

struct ConnectingPassword
{	
	struct HeaderPacket				TF_HeaderPacket;
	struct CmdPacket						CmdTF;
	Byte 								UserName[8];
	Byte 								Password[8];
	Byte 								Config[18];		
	Byte  							MyTicks[4];
	Dword  							ReplyTicks;
	Word 								ReplySeqID;
	Byte  							SupportList[12];
};

struct Protocol_Connect
{
	struct HeaderPacket				TF_HeaderPacket;
	struct CmdPacket						CmdTF;
	Byte 								IOCtrlDstMac[6];
	Byte 								DestIP[4];
	Byte 								Config[18];			
	Dword								MyTicks;
	Byte								ReplyTicks[4];
	Word								ReplySeqID;
	Byte								SupportList[12];
};

struct SourceStatus
{
	struct HeaderPacket				TF_HeaderPacket;
	struct CmdPacket						CmdTF;
	Byte 								IOCtrlDstMac[6];
	Byte 								DestIP[4];
	Word 								InputSource;
	Word 								SourceWidth;
	Word 								SourceHeight;
	Word 								SourceFrame;
	Word 								XmtWidth;
	Word 								XmtHeight;
	Word 								XmtAudio;	
	Dword  							MyTicks;
	Byte  							RemoteOff;
	Byte  							CompressHightlow;
	Byte  							HDCP_ONOFF;
	Byte  							Macrovision;
	Byte  							AviInfoDB;		// Joeho...2010/07/14
	Byte  							ScaleLevel;		// Joeho...2010/07/14
};

long GetTickCount(void)
{
	struct timeval my_timeval;
	if (gettimeofday(&my_timeval, NULL))
		return 0;
	return (long)(my_timeval.tv_sec * 1000) + (my_timeval.tv_usec / 1000);
    
}

#include <sys/socket.h>

static	Int						m_IOCtrlStep = 1;
static Byte 											IOCtrlDstIP[4]					;
static Byte 											IOCtrlDstMac[6]					;
static Byte												IOCtrlSupportList[]			= 
{0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static struct BroadcastPacket						IOCtrlBrucastPacketCmd;
static struct CommnadPacket							IOCtrlSendPacketCmd;
static struct SessionPacket							IOCtrlSendSessionCmd;
static struct Recv_Command								IOCtrlRecvPacketCmd;
static struct ConnectingPassword					IOCtrlRecvConnectCmd;
static struct Protocol_Connect						IOCtrlSendConnectCmd;
static struct SourceStatus								IOCtrlSendSourceCmd;
int 		m_nResolutionTicks					= 0x00001234;
long SRC_WIDTH = 0;
long SRC_HEIGHT = 0;
long SCREEN_WIDTH = 0;
long SCREEN_HEIGHT = 0;

int socketServerClient = 0;
struct sockaddr_in server;
int len =sizeof(struct sockaddr);


int Tick()
{
	memcpy(IOCtrlSendSourceCmd.TF_HeaderPacket.Signature, "TF6z", sizeof(IOCtrlSendSourceCmd.TF_HeaderPacket.Signature));
	memcpy(IOCtrlSendSourceCmd.IOCtrlDstMac, IOCtrlDstMac, sizeof(IOCtrlSendSourceCmd.IOCtrlDstMac));
	memcpy(IOCtrlSendSourceCmd.DestIP, IOCtrlDstIP, sizeof(IOCtrlSendSourceCmd.DestIP));
	
    SCREEN_WIDTH = SRC_WIDTH; 
    SCREEN_HEIGHT =SRC_HEIGHT; 
	
	
	IOCtrlSendSourceCmd.SourceWidth									= htons((u_short)SRC_WIDTH);
	IOCtrlSendSourceCmd.SourceHeight								= htons((u_short)SRC_HEIGHT);
	IOCtrlSendSourceCmd.TF_HeaderPacket.ProductID		= htons(Product);
	IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID				= htons(0x0003);
	IOCtrlSendSourceCmd.TF_HeaderPacket.RandCode		= IOCtrlRecvConnectCmd.TF_HeaderPacket.RandCode;
	IOCtrlSendSourceCmd.CmdTF.CmdType								= htons(0x0003);
	IOCtrlSendSourceCmd.CmdTF.OPCode								= htons(0x0303);
	IOCtrlSendSourceCmd.CmdTF.DataLen								= htons(0x0022);
	IOCtrlSendSourceCmd.InputSource									= htons(0x0001);
	IOCtrlSendSourceCmd.SourceFrame									= htons(0x0258);
	IOCtrlSendSourceCmd.XmtWidth										= htons((u_short)SCREEN_WIDTH);
	IOCtrlSendSourceCmd.XmtHeight										= htons((u_short)SCREEN_HEIGHT);
	IOCtrlSendSourceCmd.XmtAudio										= htons(0x00);
	IOCtrlSendSourceCmd.MyTicks				 							= htonl(++m_nResolutionTicks);
	IOCtrlSendSourceCmd.RemoteOff										= 0x03;
	IOCtrlSendSourceCmd.CompressHightlow						= 0x01;
	IOCtrlSendSourceCmd.HDCP_ONOFF									= 0x00;
	IOCtrlSendSourceCmd.Macrovision									= 0x00;
	IOCtrlSendSourceCmd.AviInfoDB										= 0x00;
	IOCtrlSendSourceCmd.ScaleLevel									= 0;
	
	ssize_t n = sendto(socketServerClient,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
	if ((n==0))
	{
        printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
        return -1;
	}
	
	if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
		IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
	else
		IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);
	
	return 0;
}



void* thread_Recv(void* ptr)
{
	
	unsigned char buffer[1024];
	struct sockaddr recvserver;
	
	memset(&recvserver, 0,sizeof(recvserver));
	while(1)
	{
		recvfrom(socketServerClient,      buffer,      sizeof(buffer),      0,      (struct sockaddr*)&recvserver,&len);
		
		struct HeaderPacket* header	= (struct HeaderPacket*)buffer;
		struct CmdPacket*		cmd			= (struct CmdPacket*)(buffer+ sizeof(struct HeaderPacket));
		
		//     printf("Recv cmd :0x%x\n",ntohs(cmd->OPCode));
		switch(ntohs(cmd->OPCode)) 
		{
			case 0x0101:
				if(memcmp(header->Signature, "TF6z", 4) != 0 || m_IOCtrlStep != 2) 
				{
					break;
				}
				else 
				{
					memcpy((Char*)&IOCtrlBrucastPacketCmd, buffer, ntohs(cmd->DataLen) + 16);
					++m_IOCtrlStep;
					printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
				}
				break;
				
			case 0x0201:
				if(memcmp(header->Signature, "TF6z", 4) != 0 || m_IOCtrlStep != 5) 
				{
					break;
				}
				else 
				{
					memcpy((Char*)&IOCtrlRecvPacketCmd, buffer, ntohs(cmd->DataLen) + 16);
					++m_IOCtrlStep;
					printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
				}
				break;
				
			case 0x0204:
				if(memcmp(header->Signature, "TF6z", 4) != 0 || m_IOCtrlStep != 7) 
				{
					break;
				}
				else 
				{
					memcpy((Char*)&IOCtrlRecvConnectCmd, buffer, ntohs(cmd->DataLen) + 16);
					++m_IOCtrlStep;
					printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
				}
				break;
				
			case 0x0301:
				//        printf("alive \n");
				break;
			default:
				break;
		}
		
	}
	
	return 0;
	
}

void* thread_Recorder(void* ptr)
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    long start,end,total_time,fps;
	
	short frameNumber = 0;
	unsigned char *pInBuf = NULL;
	unsigned char *pOutBuf = NULL;
	unsigned char *pYUVBUf = NULL;
	start = GetTickCount();

	while (1) {
        //count ++;
        //if (count == 10)
        //    break;
        CGImageRef screen = UIGetScreenImage();
#ifdef USUIIMAGE
        UIImage* image = [UIImage imageWithCGImage:screen];
        NSData* imageData = UIImageJPEGRepresentation(image, 0.5);
        printf("ImageData=%p data=%p length=%d\n", imageData, imageData.bytes, imageData.length);
        NSString * FileName = [[NSString alloc] initWithFormat:@"/private/var/root/build/test_dump%03d.jpg", count];
        [imageData writeToFile:FileName atomically:NO];
        [imageData release];
        [image release];
#else
        CFDataRef dataref = CGDataProviderCopyData(CGImageGetDataProvider(screen));
        long datalength = CFDataGetLength(dataref);
        const UInt8 * data = CFDataGetBytePtr(dataref);
        //printf("ImageData=%p length=%d\n", data, datalength);
#endif
#if 1
		int width, height, format;
		int filesize;
		CI_U8 *pSrc[3];
		CI_U32 srcStride[3];
		CI_SIZE srcSize;
		CI_JPGENCOPENOPTIONS OpenOptions;
		CI_RESULT ret;
		CI_VOID *pEncoder;		
		width = CGImageGetWidth(screen);
		height = CGImageGetHeight(screen);
		printf("width:%d height:%d \n",width,height);
		format = 3;
		filesize = width * height * 4;
		pInBuf = (unsigned char *) data;
		
		SRC_WIDTH = SCREEN_WIDTH = width;
		SRC_HEIGHT = SCREEN_HEIGHT = height;
		if(pOutBuf == NULL)
		{
			pOutBuf = (unsigned char *)malloc(filesize);
			if(NULL == pOutBuf)
			{
				printf("No memory for decoding!\n");
				printf("Exiting!\n");
				goto JPGEncExit;
			}
		}
		
		if(pYUVBUf == NULL)
		{
			pYUVBUf = (unsigned char *)malloc(width * height * 3 / 2);
			if(NULL == pYUVBUf)
			{
				printf("No memory for color convsion!\n");
				printf("Exiting!\n");
				goto JPGEncExit;
			}		
		}
		
        pSrc[0] = pYUVBUf;
        pSrc[1] = pYUVBUf + width * height;
        pSrc[2] = pSrc[1] + width * height / 4;
        srcStride[0] = width;        
		srcStride[1] = width>>1;
		srcStride[2] = width>>1;
        CIARGB32ToYCbCr420((int *)pInBuf, width * 4, pSrc, srcStride, width, height);
   		
		
		
		srcSize.width = width;
		srcSize.height = height;
		
		OpenOptions.u8OutType = CI_OUT_JPEG;
		OpenOptions.u8YUVType = format == 3? CI_YUV420_PLANAR : format;
		OpenOptions.u8Quality = CI_QUALITY_LEVEL4;
		
		ret = CI_JPGENC_Open(&pEncoder, &OpenOptions);
		if(ret != CI_SOK)
			goto JPGEncExit;
		
		ret = CI_JPGENC_Frame(pEncoder, pSrc, srcStride, srcSize, pOutBuf, (CI_U32*)&filesize);
		CI_JPGENC_Close(pEncoder);
		
		/*
		 FILE *fp_jpg = NULL;
		 fp_jpg = fopen("test.jpg", "wb");
		 if(NULL == fp_jpg)
		 {
		 printf("Can not open output jpg!\n");
		 printf("Exiting!\n");
		 goto JPGEncExit;
		 }
		 
		 fwrite(pOutBuf, 1, filesize, fp_jpg);
		 if(fp_jpg)
		 fclose(fp_jpg);
		 */
		char* pJPEGBuffer = pOutBuf;
		int FileLength = filesize;
		int FileReadPos = 0;
		
        pTXbuffer[0] = pTXbuffer[1] = pTXbuffer[2] = pTXbuffer[3] = 0;
        pTXbuffer[4] = (frameNumber>>8) &0xff;
        pTXbuffer[5] = frameNumber &0xff;
        memset(&pTXbuffer[6],0,12);
		
        serverVideo.sin_port=htons(0x0813); 
        sendto(socketServerClient,pTXbuffer,18,0,(struct sockaddr*)&serverVideo,len); 
        
        int lastpacketLength = (int)(FileLength%((int)1456));
		
		short i;
        for(i=0;i< FileLength/(1456);i++)
        {
			pTXbuffer[0] = (frameNumber>>8) &0xff;
			pTXbuffer[1] = frameNumber &0xff;
			pTXbuffer[2] = (i>>8) &0xff;
			pTXbuffer[3] = i &0xff;
			
			memcpy(&pTXbuffer[4], &pJPEGBuffer[FileReadPos],1456);
			FileReadPos += 1456;
			
            serverVideo.sin_port=htons(0x0814); 
            ssize_t n = sendto(socketServerClient,pTXbuffer,1456+4,0,(struct sockaddr*)&serverVideo,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
        }
		
		
        if( lastpacketLength< 64)
        {
            pTXbuffer[0] = (frameNumber>>8) &0xff;
            pTXbuffer[1] = frameNumber &0xff;
            pTXbuffer[2] = (((FileLength/1456)>>8) &0xff) | 0x80;
            pTXbuffer[3] = (FileLength/1456) &0xff;
			
            memcpy(&pTXbuffer[4], &pJPEGBuffer[FileReadPos],lastpacketLength);
            FileReadPos += lastpacketLength;
			
            memset(&pTXbuffer[4 + lastpacketLength], 0, 64-lastpacketLength);
            
            ssize_t n = sendto(socketServerClient,pTXbuffer,4+64,0,(struct sockaddr*)&serverVideo,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
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
			
            memcpy(&pTXbuffer[4], &pJPEGBuffer[FileReadPos],lastpacketLength);
            FileReadPos += lastpacketLength;
			
			int i;
            for(i=0;i<lastpacketLength  - 1;i++)
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
			ssize_t n = sendto(socketServerClient,pTXbuffer,4+ invalidLength,0,(struct sockaddr*)&serverVideo,len); 
			if ((n==0))
			{
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
			}
			
			pTXbuffer[0] = (frameNumber>>8) &0xff;
			pTXbuffer[1] = frameNumber &0xff;
			pTXbuffer[2] = (((FileLength/1456+1)>>8) &0xff) |0x80;
			pTXbuffer[3] = (FileLength/1456+1) &0xff;
			
			pTXbuffer[4] = 0xff;
			pTXbuffer[5] = 0xd9;
			memset(&pTXbuffer[6],0,62);
			
			n = sendto(socketServerClient,pTXbuffer,4+ 64,0,(struct sockaddr*)&serverVideo,len); 
			if ((n==0))
			{
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
			}
			
		}
		
#endif
	JPGEncExit:
		
		[dataref release];
		
        CGImageRelease(screen);
		
		if(GetTickCount() - start > 1000)
		{
			Tick();
			start = GetTickCount();
		}
        end = GetTickCount();
		
        total_time = end - start;
        fps = ((int64_t)100000) / total_time;
        //printf("FPS=%ld.%ld \n", fps / 100, fps%100);
        usleep(5*1000);
    }
	[pool release];		
	return NULL;
}

#define MAXINTERFACES 10

long long GetIPAddress(unsigned char* pMac)
{ 
	register int fd, intrface, retn = 0; 
	struct ifreq buf[MAXINTERFACES]; 
	struct ifconf ifc; 
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) 
	{ 
		ifc.ifc_len = sizeof buf; 
		ifc.ifc_buf = (caddr_t) buf; 
		if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) 
		{ 
			//ªÒ»°Ω”ø⁄–≈œ¢
			intrface = ifc.ifc_len / sizeof (struct ifreq); 
			printf("interface num is intrface=%d\n\n\n",intrface); 
			//∏˘æ›ΩËø⁄–≈œ¢—≠ª∑ªÒ»°…Ë±∏IP∫ÕMACµÿ÷∑
			while (intrface-- > 0 ) 
			{ 
				if(strcmp(buf[intrface].ifr_name,"en0" ))
					continue;
				
				//ªÒ»°…Ë±∏√˚≥∆
				printf ("net device %s\n", buf[intrface].ifr_name); 
				
				//≈–∂œÕ¯ø®¿‡–Õ 
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
				//≈–∂œÕ¯ø®◊¥Ã¨ 
				if (buf[intrface].ifr_flags & IFF_UP) 
				{ 
					puts("the interface status is UP" ); 
				} 
				else 
				{ 
					puts("the interface status is DOWN" ); 
				} 
				//ªÒ»°µ±«∞Õ¯ø®µƒIPµÿ÷∑ 
				if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface]))) 
				{ 
					struct in_addr addr = ((((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
					//	if(((addr.s_addr&0xff )== 192) && 
					//	   (((addr.s_addr>>8)&0xff)  == 168)&&
					//	   (((addr.s_addr>>16)&0xff )== 168))
					{
						puts ("IP address is:" ); 
						printf("%d.%d.%d.%d\n", 
							   addr.s_addr&0xff,
							   addr.s_addr>>8&0xff,
							   addr.s_addr>>16&0xff,
							   addr.s_addr>>24&0xff);
						
						
						
						int                  mib[6];
						size_t               len;
						char                *buf;
						unsigned char       *ptr;
						struct if_msghdr    *ifm;
						struct sockaddr_dl  *sdl;
						
						mib[0] = CTL_NET;
						mib[1] = AF_ROUTE;
						mib[2] = 0;
						mib[3] = AF_LINK;
						mib[4] = NET_RT_IFLIST;
						
						if ((mib[5] = if_nametoindex("en0")) == 0) {
							printf("Error: if_nametoindex error\n");
							return -1;
						}
						
						if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
							printf("Error: sysctl, take 1\n");
							return -1;
						}
						
						if ((buf = malloc(len)) == NULL) {
							printf("Could not allocate memory. error!\n");
							return -1;
						}
						
						if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
							printf("Error: sysctl, take 2");
							return -1;
						}
						
						ifm = (struct if_msghdr *)buf;
						sdl = (struct sockaddr_dl *)(ifm + 1);
						ptr = (unsigned char *)LLADDR(sdl);    
						free(buf);
						memcpy(pMac, ptr, 6);
						
						printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5]);
						/*
						 if (!(ioctl (fd, SIOCGIFHWADDR, (char *) &buf[intrface]))) 
						 { 
						 puts ("HW address is:" ); 
						 printf("%02x:%02x:%02x:%02x:%02x:%02x\n", 
						 (unsigned char)buf[intrface].ifr_hwaddr.sa_data[0], 
						 (unsigned char)buf[intrface].ifr_hwaddr.sa_data[1], 
						 (unsigned char)buf[intrface].ifr_hwaddr.sa_data[2], 
						 (unsigned char)buf[intrface].ifr_hwaddr.sa_data[3], 
						 (unsigned char)buf[intrface].ifr_hwaddr.sa_data[4], 
						 (unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]); 
						 if(pMac)
						 {
						 int i;
						 for(i=0;i<6;i++)
						 pMac[i] = (unsigned char)buf[intrface].ifr_hwaddr.sa_data[i];
						 
						 }
						 puts("" ); 
						 puts("" ); 
						 } 
						 else
						 puts("failed to get mac address ");
						 */
						close (fd); 
						return addr.s_addr; 
					}
					
				} 
			}    
		}
	} //while
	return 0; 
} 


int init()
{
	pTXbuffer = (char*)malloc(1*1024*1024);
    if(!pTXbuffer)  printf("failed to malloc pTXbuffer \n");
	
	
    memset(&serverVideo, 0,sizeof(serverVideo));
    serverVideo.sin_family=AF_INET;
    serverVideo.sin_addr.s_addr=SERVERADDR;
	
    struct sockaddr_in addr;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(UDP_PORT);
	
    socketServerClient=socket(AF_INET,SOCK_DGRAM,0);
    if(bind(socketServerClient, (const struct sockaddr *)&addr, sizeof(addr)) !=0)
		printf("failed to bind\n");
	
    long long ipaddr = GetIPAddress(IOCtrlDstMac);
    if(!(ipaddr))
    {
		printf("failed to GetIPAddress() \n");
		while(1);
    }
    IOCtrlDstIP[0] = ipaddr &0xff;//192;
    IOCtrlDstIP[1] = ipaddr>>8 &0xff;
    IOCtrlDstIP[2] = ipaddr>>16 &0xff;
    IOCtrlDstIP[3] = ipaddr>>24 &0xff;
	
    memset(&server, 0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(UDP_PORT); ///serverµƒº‡Ã˝∂Àø⁄
    server.sin_addr.s_addr=SERVERADDR; 
	
	return 0;
}


void* test_thread(void* ptr)
{
	init();
	
	
	pthread_t pid;
	struct sched_param schedRecv;
	pthread_attr_t attrRecv;
	pthread_attr_init(&attrRecv);
	pthread_create(&pid, &attrRecv, thread_Recv, NULL);
    schedRecv.sched_priority = 90;
    pthread_setschedparam(pid, SCHED_RR, &schedRecv);
	long startTick, currtick;
	startTick = GetTickCount();
    while (1) {
		if(m_IOCtrlStep == 1) //OPCODE_TF6x0_DISCOVER_AND_RESET , wait for OPCODE_TF6x0_DISCOVER
        {
            struct InitConnectPacket initconnect;
            initconnect.Header.ProductID 		= htons(Product);
            initconnect.Header.RandCode			= htons(0x00);
            initconnect.Header.SeqID				= htons(0x00);
            initconnect.Command.CmdType			= htons(0x0001);
            initconnect.Command.OPCode			= htons(0x0103);
            initconnect.Command.DataLen			= htons(0x0020);
            memcpy(initconnect.Header.Signature, "TF6z", sizeof(initconnect.Header.Signature));
			
            ssize_t	n = sendto(socketServerClient,&initconnect,sizeof(initconnect),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
 			m_IOCtrlStep++;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);

			
        }
        else if(m_IOCtrlStep == 3)  //OPCODE_TF6x0_DISCOVER_AND_RESET
        {
            static Byte 											IOCtrlHeader[6]					= {0xc0, 0x00, 0xc0, 0x03, 0x00, 0x92};
            static Byte 											IOCtrlMachineNameID[12] = {0x50, 0x43, 0x32, 0x54, 0x56, 0x2d, 0x50, 0x43, 0x41, 0x00, 0x00, 0x00};
			
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
			
            ssize_t n = sendto(socketServerClient,&IOCtrlSendPacketCmd,sizeof(IOCtrlSendPacketCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
            IOCtrlSendPacketCmd.TF_HeaderPacket.SeqID			+= htons(0x01); 
            IOCtrlSendPacketCmd.TF_HeaderPacket.RandCode	 = IOCtrlSendPacketCmd.TF_HeaderPacket.SeqID;
            n = sendto(socketServerClient,&IOCtrlSendPacketCmd,sizeof(IOCtrlSendPacketCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
            m_IOCtrlStep++;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
        }
        else if(m_IOCtrlStep == 4)  //OPCODE_TF6x0_CONNECTING, wait for  OPCODE_TF6x0_CONNECTING
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
			
			int i;
			for(i=0;i<4;i++)
				IOCtrlSendSessionCmd.PCIp[i] = IOCtrlDstIP[i];
			
            static Byte												IOCtrlProductType[4]		= {0x01, 0x00, 0x40, 0x03};
            static Byte 											IOCtrlMachineNameID[12] = {0x50, 0x43, 0x32, 0x54, 0x56, 0x2d, 0x50, 0x43, 0x41, 0x00, 0x00, 0x00};
			
            memcpy(IOCtrlSendSessionCmd.ProductType, IOCtrlProductType, sizeof(IOCtrlSendSessionCmd.ProductType));
            memcpy(IOCtrlSendSessionCmd.MachineName, IOCtrlMachineNameID, sizeof(IOCtrlSendSessionCmd.MachineName));
			
            IOCtrlSendSessionCmd.MyTicks = htonl(0x12345678);
			
            ssize_t n = sendto(socketServerClient,&IOCtrlSendSessionCmd,sizeof(IOCtrlSendSessionCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
            m_IOCtrlStep++;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
		}
        else if(m_IOCtrlStep == 6)  //OPCODE_TF6x0_CONNECTING, wait for OPCODE_TF6x0_CONNECTING_PASSWORD
        {
			IOCtrlSendSessionCmd.TF_HeaderPacket.RandCode =	IOCtrlSendPacketCmd.TF_HeaderPacket.RandCode + htons(0x01);
			IOCtrlSendSessionCmd.TF_HeaderPacket.SeqID		= IOCtrlSendPacketCmd.TF_HeaderPacket.SeqID + htons(0x01);
			IOCtrlSendSessionCmd.CmdTF.OPCode							= htons(0x0201);
			IOCtrlSendSessionCmd.ReplyTicks								= IOCtrlRecvPacketCmd.MyTicks;
			IOCtrlSendSessionCmd.ReplySeqID								= IOCtrlRecvPacketCmd.TF_HeaderPacket.SeqID;
			IOCtrlSendSessionCmd.CmdTF.CmdType						= htons(0x8002);
            ssize_t n = sendto(socketServerClient,&IOCtrlSendSessionCmd,sizeof(IOCtrlSendSessionCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			//                m_IOCtrlStep++;
            m_IOCtrlStep+=2 ;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
        }
        else if(m_IOCtrlStep == 8)  //OPCODE_TF6x0_CONNECTING_PASSWORD
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
			
            ssize_t n = sendto(socketServerClient,&IOCtrlSendConnectCmd,sizeof(IOCtrlSendConnectCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
            m_IOCtrlStep++;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
        }
        else if(m_IOCtrlStep == 9)  //OPCODE_TF6x0_SOURCE_STATUS_ANNOUNCE
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
            IOCtrlSendSourceCmd.SourceWidth								= htons((u_short)SRC_WIDTH);
            IOCtrlSendSourceCmd.SourceHeight							= htons((u_short)SRC_HEIGHT);
            IOCtrlSendSourceCmd.SourceFrame								= htons(0x0258);
			IOCtrlSendSourceCmd.XmtWidth										= htons((u_short)SCREEN_WIDTH);
			IOCtrlSendSourceCmd.XmtHeight										= htons((u_short)SCREEN_HEIGHT);
            IOCtrlSendSourceCmd.XmtAudio									= htons(0x00);
            IOCtrlSendSourceCmd.MyTicks										= htonl(m_nResolutionTicks);
            IOCtrlSendSourceCmd.RemoteOff									= 0x03;
            IOCtrlSendSourceCmd.CompressHightlow					= 0x01;
            IOCtrlSendSourceCmd.HDCP_ONOFF								= 0x00;
            IOCtrlSendSourceCmd.Macrovision								= 0x00;
            IOCtrlSendSourceCmd.AviInfoDB									= 0x00;
            IOCtrlSendSourceCmd.ScaleLevel								= 0;//m_SldScreenSizeLevelPos;
			
            ssize_t  n = sendto(socketServerClient,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
  			if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
  				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
  			else
  				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);
			
			
			IOCtrlSendSourceCmd.MyTicks = htonl(0x00001235);
            n = sendto(socketServerClient,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
  			if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
  				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
  			else
  				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);
			
			
    		IOCtrlSendSourceCmd.MyTicks = htonl(0x00001236);
            n = sendto(socketServerClient,&IOCtrlSendSourceCmd,sizeof(IOCtrlSendSourceCmd),0,(struct sockaddr*)&server,len); 
            if ((n==0))
            {
				printf("send to server failed n:%d errno:%d \n",(int)n,(int)errno);
				return -1;
            }
			
  			if(IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID == htons(0xffff))
  				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID  = htons(0x00);
  			else
  				IOCtrlSendSourceCmd.TF_HeaderPacket.SeqID += htons(0x01);
			
            m_IOCtrlStep++;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
		}
        else if(m_IOCtrlStep == 10) //OPCODE_TF6x0_SOURCE_STATUS_ANNOUNCE
        {
            m_IOCtrlStep++;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
        }
        else if(m_IOCtrlStep == 11)
        {
			struct sched_param sched;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&pid, &attr, thread_Recorder, NULL);
			sched.sched_priority = 90;
			pthread_setschedparam(pid, SCHED_RR, &sched);
			
			m_IOCtrlStep++;
            printf("Move from %d to %d \n",m_IOCtrlStep-1, m_IOCtrlStep);
			return 0;
        }
		else {
			
			if(m_IOCtrlStep < 11)
			{
					if(GetTickCount() - startTick > 5000)
					{
						if(m_IOCtrlStep == 2 || m_IOCtrlStep == 5)
							m_IOCtrlStep = 1;
						startTick = GetTickCount();
					}
			}
			
			usleep(10*1000);
		}
		
		
    }
	
    return 0;
}

#import "iScreenCapAppDelegate.h"
#import "iScreenCapViewController.h"

@implementation iScreenCapAppDelegate

@synthesize window;
@synthesize viewController;


#pragma mark -
#pragma mark Application lifecycle





- (BOOL) startRun
{
		
	NSLog(@"star	 run");
	pthread_t pid;
	struct sched_param sched;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&pid, &attr, test_thread, NULL);
	sched.sched_priority = 90;
	pthread_setschedparam(pid, SCHED_RR, &sched);
	
	NSLog(@"start run success ");
	return true;
}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    
    // Override point for customization after application launch.

	// Set the view controller as the window's root view controller and display.
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
	NSLog(@"1");
	self.startRun;
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}


@end
