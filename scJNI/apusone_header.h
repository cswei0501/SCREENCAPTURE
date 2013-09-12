#ifndef _Included_Apus_HEADER
#define _Included_Apus_HEADER

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

int APUSRecvPacket(char* pBuff, int length);
int APUS_SockStreamInit();
bool APUS_CheckAttributeOfEcho(char revbuff[], int length);
bool APUS_CheckAttributeOfDeviceStatus(char revbuff[], int length);
void APUS_SendStartCapScreenFunc();
#endif