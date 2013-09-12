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
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>


CGImageRef UIGetScreenImage(void);

#include <sys/socket.h> 
#include <sys/sockio.h> 
#include <sys/sysctl.h>
#include <net/if_dl.h>
#include <net/if.h> 
#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>
#include <UIKit/UIDevice.h>
#include "ci_jpegenc.h"

//#define OLD
typedef void* 				LPVOID;
typedef unsigned int			DWORD;				///< unsigned int
//typedef bool 					BOOL;
typedef void* (*PFunc)(void*);

long GetTickCount(void)
{
	struct timeval my_timeval;
	if (gettimeofday(&my_timeval, NULL))
		return 0;
	return (long)(my_timeval.tv_sec * 1000) + (my_timeval.tv_usec / 1000);
    
}
BOOL backgroundSupported = NO;
AVAudioSession *audioSession = nil;

long SRC_WIDTH = 0;
long SRC_HEIGHT = 0;
long SCREEN_WIDTH = 0;
long SCREEN_HEIGHT = 0;

static int g_rotate = 0;
static int g_quit = 0;
int streamHandle = 0;
int dgramHandle = 0;
int recv_len = 0;
struct sockaddr_in g_SenderAddr;




void Create_Pthread(PFunc SockProc);

const int DeviceType_IATV				= (0x01 << 1) | 1;
const int DeviceType_IATVCtrl			= (0x03 << 1);
const int DeviceStatusReportCmd		= 0x0002;
const int MagicNumber				= 0x5678;
const int StartCaptureScreenCmd		= 0x0016;
const int EndCaptureScreenCmd		= 0x0017;
const int UpdateCaptureScreenCmd	= 0x0018; 

typedef struct 
{
	int  MagicNumber;
	int  packet_size;
	int  type;
	int  mode;
}StartCaptureScreen;

typedef struct 
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
}UpdateCaptureScreen;

typedef struct 
{
	int  MagicNumber;
	int  packet_size;
	int  type;
	int  DeviceType;
	int  DeviceStatus;
	int  port; 
	char   devicename[128];
}DeviceStatusReport;

const int BroadcastBeginPort    = 6180;
const int BroadcastEndPort      = 6190;
const int EchoCmd                         = 0x001a;
const int EchoRespCmd                     = 0x001b;
const int StartCaptureScreenExCmd = 0x0090;
const int UpdateCaptureScreenExCmd = 0x0091;

typedef struct {
	int  MagicNumber;
	int  packet_size;
	int  type;
	int  TimeLow32;
	int  TimeHigh32;
	int  Data;
	char   Data1[16];
}Echo;

typedef struct 
{
	int  MagicNumber;
	int  packet_size;
	int  type;                      //StartCaptureScreenExCmd 
	int  left;                //reserved
	int  right;                //reserved
	int  top;                //reserved
	int  bottom;            //reserved
	int  mediatype;        //0 h264, 1 jpeg, ......
	int  rotate;            // 0 no rotate,1 rotate 90,2 rotate 180,3 rotate 270 
	int sendTime;
} UpdateCaptureScreenEx;

int SockDatagramInit()
{
	struct sockaddr_in sin;
	int fbroadcast = 1;
	dgramHandle =(int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt ( dgramHandle,SOL_SOCKET,SO_BROADCAST,(char *)&fbroadcast,sizeof ( int ));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(BroadcastBeginPort);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	int optval = 1;
	setsockopt(dgramHandle,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(int));
	
	if(bind( dgramHandle, (struct sockaddr *)&sin, sizeof(sin))!=0)
	{
		close(dgramHandle);
		dgramHandle = 0;
		return 1;
	}

	return 0;
}

void SockDatagramReport()
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
		(const struct sockaddr*)&saudpserv, sizeof(struct sockaddr_in) );
}

int SockStreamInit()
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
		close(streamHandle);
		return -1;
	}
	
	int n = 64*1024;
	setsockopt(streamHandle, SOL_SOCKET, SO_SNDBUF, (const char*)&n, sizeof(int));
	
	//int iMode = 1;
	//ioctlsocket(streamHandle, FIONBIO, (u_long FAR*) &iMode);
	
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	//saddr.sin_addr.S_un.S_addr = g_SenderAddr.sin_addr.S_un.S_addr;
	saddr.sin_addr.s_addr= g_SenderAddr.sin_addr.s_addr;
	saddr.sin_port = htons(g_SenderAddr.sin_port);

    NSLog(@"sin_port:%d,L:%d \n",saddr.sin_port,__LINE__);
	//char* x = inet_ntoa(saddr.sin_addr);
	ret = connect(streamHandle, (struct sockaddr*)&saddr, sizeof(saddr) );
	if(ret != 0)
	{
		return -1;
	}

	return 0;
}

void SockStreamReport()
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

	send(streamHandle, buffer, sizeof(DeviceStatusReport), 0 );
}

int ARGB32ToYCB420(unsigned char **pYUVBuf, unsigned char *pInBuf,
	CI_U8 *pSrc[],CI_U32 srcStride[], int width, int height)
{
    if(*pYUVBuf == NULL)
    {
        *pYUVBuf = (unsigned char *)malloc(width * height * 3 / 2);
        if(NULL == *pYUVBuf)
        {
            NSLog(@"No memory for color convsion!\n");
            return -1;
        }
    }
    
	pSrc[0] = *pYUVBuf;
	pSrc[1] = *pYUVBuf + width * height;
	pSrc[2] = pSrc[1] + width * height / 4;
	srcStride[0] = width;  
	srcStride[1] = width>>1;
	srcStride[2] = width>>1;

   	CIARGB32ToYCbCr420((int *)pInBuf, width * 4, (char**)pSrc, (int*)srcStride, width, height);
    return 0;
}

int CIJpegEncFrame(CI_U8 *pSrc[], unsigned char *pOutBuf, CI_U32 srcStride[], int width, int height, int *filesize)
{
	CI_SIZE srcSize;
	CI_JPGENCOPENOPTIONS OpenOptions;
	CI_VOID *pEncoder;
	CI_RESULT ret;
	int format = 3;
    
	srcSize.width = width;
	srcSize.height = height;
	
	OpenOptions.u8OutType = CI_OUT_JPEG;
	OpenOptions.u8YUVType = format == 3? CI_YUV420_PLANAR : format;
	OpenOptions.u8Quality = CI_QUALITY_LEVEL4;
	
	ret = CI_JPGENC_Open(&pEncoder, &OpenOptions);
	if(ret != CI_SOK)
	{
		printf("enc initial failed!\n");
		return -1;
	}
	
	ret = CI_JPGENC_Frame(pEncoder, pSrc, srcStride, srcSize, pOutBuf, (CI_U32*)filesize);
	if(ret != CI_SOK)
	{
		printf("enc enc failed!\n");
		return -1;
	}
	CI_JPGENC_Close(pEncoder);

	return 0;	
}
	
void UpdateCapScreen(char *buf,unsigned char* pOutBuf,int filesize, int width, int height)
{
	int len = 0;
#ifdef OLD
	UpdateCaptureScreen* ptr = (UpdateCaptureScreen*)buf;
	memcpy(buf + sizeof(UpdateCaptureScreen),pOutBuf,filesize);
	len = filesize+ sizeof(UpdateCaptureScreen);
#else
	memcpy(buf + sizeof(UpdateCaptureScreenEx),pOutBuf,filesize);
	len = filesize+ sizeof(UpdateCaptureScreenEx);
	UpdateCaptureScreenEx* ptr = (UpdateCaptureScreenEx*)buf;
#endif
	ptr->MagicNumber = htonl(MagicNumber);
	ptr->packet_size = htonl(len);
#ifdef OLD
	ptr->type = htonl(UpdateCaptureScreenCmd);
#else
	ptr->type = htonl(UpdateCaptureScreenExCmd );
	ptr->rotate = htonl(g_rotate); //0 no rotate,1 rotate 90,2 rotate 180,3 rotate 270
#endif
	ptr->left = ptr->top = 0;
	//ptr->right = htonl(800);
	ptr->right = htonl(width);
	ptr->bottom = htonl(height);
	ptr->mediatype = htonl(1); //JPEG
	send(streamHandle, buf, len, 0 );

	return;
}

void SendStartCapScreenFunc(char *buf)
{
	StartCaptureScreen *pStart = (StartCaptureScreen *) buf;
	pStart->type = htonl(StartCaptureScreenCmd);
	pStart->MagicNumber = htonl(MagicNumber);
	
#ifdef OLD
	pStart->packet_size = htonl(sizeof(StartCaptureScreen));
#else
	pStart->packet_size = htonl(sizeof(StartCaptureScreenExCmd));
#endif
	pStart->mode = 0; //Text mode
	send(streamHandle, buf, sizeof(StartCaptureScreen), 0 );
}

void SendEndCapScreenFunc(char *buf)
{
	StartCaptureScreen *pEnd = (StartCaptureScreen *) buf;
	pEnd->type = htonl(EndCaptureScreenCmd);
	pEnd->MagicNumber = htonl(MagicNumber);
	
#ifdef OLD
	pEnd->packet_size = htonl(sizeof(StartCaptureScreen));
#else
	pEnd->packet_size = htonl(sizeof(StartCaptureScreenExCmd));
#endif
	pEnd->mode = 0; //Text mode
	send(streamHandle, buf, sizeof(StartCaptureScreen), 0 );
}

/**************************************************************
	Func: SockStreamProc();
	Function: capture screen and jpegenc.
	parameters:ptr

 **************************************************************/
void* SockStreamProc(void* ptr)
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

	if(SockStreamInit() != 0)
		return NULL;

	SockStreamReport();

	char buffer[256] = {0};
	SendStartCapScreenFunc(buffer);

	long frames = 0;
	unsigned char *pInBuf = NULL;
	unsigned char *pOutBuf = NULL;
	unsigned char *pYUVBuf = NULL;
	int filesize, width, height;
    CI_U8 *pSrc[3] = {0, 0, 0};
    CI_U32 srcStride[3] = {0, 0, 0};
    
	char* buf = malloc(sizeof(char)*500*1024 + 32); 
	long startTick = GetTickCount();
	while(!g_quit)
	{
		CGImageRef screen = UIGetScreenImage();
		CFDataRef dataref = CGDataProviderCopyData(CGImageGetDataProvider(screen));
		//long datalength = CFDataGetLength(dataref);
		const UInt8 * data = CFDataGetBytePtr(dataref);
		
		width = CGImageGetWidth(screen);
		height = CGImageGetHeight(screen);
		//printf("ImageData=%p length=%d\n", data, datalength);
		
		pInBuf = (unsigned char*)data;
		filesize = width * height * 4;
        
		SRC_WIDTH = SCREEN_WIDTH = width;
		SRC_HEIGHT = SCREEN_HEIGHT = height;
        
        if(pOutBuf == NULL)
        {
            pOutBuf = (unsigned char *)malloc(filesize);
            if(NULL == pOutBuf)
            {
                NSLog(@"No memory for decoding!\n");
                break;
            }
        }
        
		if(ARGB32ToYCB420(&pYUVBuf, pInBuf, pSrc, srcStride, width, height))
            break;

		if(CIJpegEncFrame(pSrc, pOutBuf, srcStride, width, height, &filesize) != 0)
			break;

		UpdateCapScreen(buf, pOutBuf, filesize, width, height);

		[(id)dataref release];
		CGImageRelease(screen);
		frames++;
        if(GetTickCount()- startTick >1000)
        {
            float fps = (float)(frames*1000)/(float)(GetTickCount()- startTick);
            NSLog(@"fps :%f \n",fps);
            
            startTick = GetTickCount();
            frames = 0;
        }
	}
    SendEndCapScreenFunc(buffer);
    NSLog(@"L:%d\n",__LINE__);
    if(NULL != buf)
        free(buf);
    if(NULL != pYUVBuf)
        free(pYUVBuf);
    if(NULL != pOutBuf)
        free(pOutBuf);
    if(streamHandle != 0)
        close(streamHandle);
	return (void*)1;
}

BOOL CheckAttributeOfDeviceStatus(char revbuff[], int length)
{
	DeviceStatusReport *pp = (DeviceStatusReport *)revbuff;
	if(ntohl(pp->MagicNumber) == MagicNumber  \
		&& ntohl(pp->packet_size) <= length \
		&& ntohl(pp->type) == DeviceStatusReportCmd  \
		&& (ntohl(pp->DeviceType) & DeviceType_IATV ) == DeviceType_IATV )
	   return true;

	return false;
}

BOOL CheckAttributeOfEcho(char revbuff[], int length)
{
	Echo *pm = (Echo*)revbuff;
	if(ntohl(pm->MagicNumber) == MagicNumber  \
		&& ntohl(pm->packet_size) <= length \
		&& ntohl(pm->type) == EchoCmd)
		return true;
	
	return false;
}
	
int RecvPacket(char* pBuff, int length)
{
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
		//Sleep(10);
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
		
		unsigned int SenderAddrSize = sizeof(g_SenderAddr);
		recv_len = recvfrom( dgramHandle,pBuff, length, 0, (struct sockaddr *) &g_SenderAddr,&SenderAddrSize);
		//if(recv_len == SOCKET_ERROR)
		//{
		//	break;
		//}
		return recv_len;
	}
	return 0;
}


void* SockDatagramProc(void* ptr)
{
	char recvbuff[256];
	int g_btcpproc = false;
    
	if(SockDatagramInit())
		return NULL;

	while(!g_quit)
	{
		if(!g_btcpproc)
		{
			SockDatagramReport();
			usleep(500*1000);
		}

		memset(recvbuff,0,sizeof(recvbuff));
		int len = RecvPacket(recvbuff,sizeof(recvbuff));
		if(len == -1)
			return NULL;
		if(len == 0)	//not received ,try again
			continue;

		if(CheckAttributeOfDeviceStatus(recvbuff, len) == true)
		{
			if(g_btcpproc == false)
			{
				DeviceStatusReport *ptr = (DeviceStatusReport *)recvbuff;
				g_SenderAddr.sin_port = htonl(ptr->port);

				Create_Pthread(SockStreamProc);
			 	g_btcpproc = true;
                if(dgramHandle != 0)
                    close(dgramHandle);
			}
			break;
		}
		
		if(CheckAttributeOfEcho(recvbuff, len) == true)
		{
			//SendEchoType(recvbuff);
			Echo *pm = (Echo*)recvbuff;
			struct sockaddr_in saudpserv;
			saudpserv.sin_family = AF_INET;
			saudpserv.sin_addr.s_addr = htonl(g_SenderAddr.sin_addr.s_addr);
			saudpserv.sin_port = htons(g_SenderAddr.sin_port);
			pm->type = htonl(EchoRespCmd);
			sendto(dgramHandle, recvbuff, sizeof(Echo), 0, (const struct sockaddr*)&saudpserv, sizeof(struct sockaddr_in) );
			break;
		}		
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


#import "iScreenCapAppDelegate.h"
#import "iScreenCapViewController.h"

@implementation iScreenCapAppDelegate

@synthesize window;
@synthesize viewController;


#pragma mark -
#pragma mark Application lifecycle

BOOL g_checkStartRun = YES;

-(void) IPhoneInit
{
	UIDevice* mydevice = [UIDevice currentDevice];
	[mydevice beginGeneratingDeviceOrientationNotifications];
	NSLog(@"oriantation :%d \n",[[UIDevice currentDevice] orientation]);
	NSLog(@"multitaskingSupported :%@ \n",mydevice.multitaskingSupported?@"yes":@"no");
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(didRotate:)
												 name:@"UIDeviceOrientationDidChangeNotification" object:nil];

}

- (void) didRotate:(NSNotification *)notification
{	
	NSLog(@"changed to oriantation :%d \n",[[UIDevice currentDevice] orientation]);
	if([[UIDevice currentDevice] orientation] == UIDeviceOrientationPortrait)
		g_rotate = 0;
	else if([[UIDevice currentDevice] orientation] ==  UIDeviceOrientationPortraitUpsideDown)
		g_rotate = 2;
	else if([[UIDevice currentDevice] orientation] ==  UIDeviceOrientationLandscapeLeft)
		g_rotate = 3;
	else if([[UIDevice currentDevice] orientation] ==  UIDeviceOrientationLandscapeRight)
		g_rotate = 1;
		
}

- (BOOL) startRun
{
    if(g_checkStartRun)
    {
        g_checkStartRun = NO;
        NSLog(@"star	 run");	
        [self IPhoneInit];
        Create_Pthread(SockDatagramProc);   
        NSLog(@"start run success ");
        return true;
    }
    return false;
}

- (void)timeBombFunc:(id)sender
{
    NSLog(@"Timebomb timer reached !!! \n");

    exit(0);
    return;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    
    
    [self performSelector:@selector(timeBombFunc:) withObject:nil afterDelay:600];

    // Override point for customization after application launch.

	// Set the view controller as the window's root view controller and display.
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];

    NSLog(@"Start timebomb timer !!! \n");
#if 0    
    UIDevice* device = [UIDevice currentDevice];
    if ([device respondsToSelector:@selector(isMultitaskingSupported)])
        backgroundSupported = device.multitaskingSupported;
    if(backgroundSupported == YES)
       	NSLog(@"multitasking is Supported"); 
    else
       	NSLog(@"multitasking is unSupported"); 
        
    audioSession = [AVAudioSession sharedInstance];
    
    NSError *setCategoryError = nil;
    [audioSession setCategory:AVAudioSessionCategoryPlayback error:&setCategoryError];
    if (setCategoryError) { /* handle the error condition */ }
    
    NSError *activationError = nil;
    [audioSession setActive:YES error:&activationError];
    if (activationError) { /* handle the error condition */ }

#endif
    [self startRun];
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
    NSLog(@"applicationWillResignActive");
    g_quit = 1;
    g_checkStartRun = YES;
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
    NSLog(@"applicationDidEnterBackground");
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
    NSLog(@"applicationWillEnterForeground");
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
    NSLog(@"applicationDidBecomeActive");
    g_quit = 0;
    [self application:nil didFinishLaunchingWithOptions:nil];
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
    NSLog(@"applicationWillTerminate");    
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
