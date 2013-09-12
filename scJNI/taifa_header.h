#ifndef _Included_TAIFA_HEADER
#define _Included_TAIFA_HEADER


#define UDP_PORT							48689
#define Product								0x6301 

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
	HeaderPacket				Header;
	CmdPacket						Command;
};


struct CommnadPacket																// 0x0103
{
	HeaderPacket				TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;												// 0034
	Byte 								IOCtrlHeader[6];							// 003A
	Byte 								Reserve[12];
	Byte 								MachineName[12];
	Word								SeqNum;
	Byte 								DummySpace[124];
};

struct SessionPacket																// 0x0201
{
	HeaderPacket				TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;												// 0034
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
	HeaderPacket				TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;												// 0034
	Byte 								ProductType[4];								// 003A
	Word								ProtocolVersion;
	Byte								GroupName[12];
	Byte 								MachineName[12];
	Dword								AskType;
};

struct Recv_Command																	// 0x0201
{
	HeaderPacket				TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;												// 0034
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
	HeaderPacket				TF_HeaderPacket;
	CmdPacket						CmdTF;
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
	HeaderPacket				TF_HeaderPacket;
	CmdPacket						CmdTF;
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
	HeaderPacket				TF_HeaderPacket;
	CmdPacket						CmdTF;
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
	Byte  							audioSampleRate;		// 0=48k. 1=44.1k
	Byte  							Vertical;		// [Golan] 2011/08/24 1:vertical 0:horisental
};

typedef struct 
{
	JNIEnv    *env;
	JavaVM    *jvm;
	jobject   obj;
	jclass    cls;
	jmethodID callbackFuns;
}JAVA_CALLBACK_FUN;

#endif

