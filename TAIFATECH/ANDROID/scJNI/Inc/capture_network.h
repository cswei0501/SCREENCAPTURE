#ifndef __CAPTURE_NETWORK_H__
#define __CAPTURE_NETWORK_H__

#include "crc.h"
#include "capture_type.h"
#include "capture_control.h"

namespace android{

#define UDP_PORT					48689
#define SIGNATURE				"TF6z"
#define Product					0x6301 
#define VERSION2					0x20
#define CODEC2					0x4040
#define VERSION1					0x10
#define CODEC1					0x8080
#define TX_PASSWORD_CMD_TYPE	0x0002
#define RX_PASSWORD_CMD_TYPE	0x8002
#define VIDEOPORT1				0x0813
#define VIDEOPORT2				0x0814
#define DATALOSTMAXTIMES		5
#define NETWORK_RECV_FPS		20
#define NET_PROTOCOL_GROUPNAME_SIZE 12
#define USERNAME				"user"

#define TSUDPPORT				65344

typedef struct HeaderPacket
{
	Byte								Signature[4];
	Word 							ProductID;
	Byte								Version;
	Word							Codec;
	Word 							RandCode;
	Word 							SeqID;
}__attribute__ ((packed))TF_HeaderPacket;

typedef struct CmdPacket								// 6Bytes
{
	Word							CmdType;
	Word							OPCode;
	Word							DataLen;
}__attribute__ ((packed))CmdTF; 

typedef struct InitConnectPacket
{
	HeaderPacket						Header;
	CmdPacket						Command;
	Dword							CrcCode;
}__attribute__ ((packed))TF_InitConnectPacket;

typedef struct CommnadPacket								// 0x0103
{
	HeaderPacket						TF_HeaderPacket;	// 002A
	CmdPacket						CmdTF;				// 0034
	Byte 							IOCtrlHeader[6];		// 003A
	Byte 							Reserve[12];
	Byte 							MachineName[NET_PROTOCOL_GROUPNAME_SIZE];
	Word							SeqNum;
	Byte 							DummySpace[124];
	Dword							CrcCode;
}__attribute__ ((packed))TF_CommnadPacket;

typedef struct RecvCommand									// 0x0201
{
	HeaderPacket						TF_HeaderPacket;	// 002A
	CmdPacket						CmdTF;				// 0034
	Byte 							MyIP[4];				// 0040
	Word							CmdPort;
	Byte								Reserve[4];
	Byte								ProductType[4];
	Word							ProtocolVersion;
	Byte 							MyGroupName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte 							MachineName[NET_PROTOCOL_GROUPNAME_SIZE];				
	Byte 							UserName[NET_PROTOCOL_GROUPNAME_SIZE];		// 0062
	Dword					 		MyTicks;				// 006C
	Dword							CrcCode;
}__attribute__ ((packed))TF_RecvCommand;

/*Robin Jan 12, 2012, add Discovery command for sw TX*/
typedef struct DiscoveryPacket																// 0x0101
{
	HeaderPacket						TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;										// 0034
//	Word							CmdPort;									// 2
	Byte 							ProductType[4];								// 4
	Word							ProtocolVersion;							// 2
	Byte 							MyGroupName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte 							MachineName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte								ask_type[4];
	Dword							CrcCode;
}__attribute__ ((packed))TF_DiscoveryPacket; 

typedef struct DiscoveryReplyPacket											// 0x0101
{
	HeaderPacket						TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;										// 0034
//	Byte 							PCIp[4];									// 4
//	Word							CmdPort;									// 2
	Byte 							ProductType[4];								// 4
	Word							ProtocolVersion;							// 2
	Byte 							MyGroupName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte 							MachineName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte								reply_seq_id[2];							// 2
	Dword							CrcCode;
}__attribute__ ((packed))TF_DiscoveryReplyPacket;

typedef struct ConnectPacket																// 0x0201
{
	HeaderPacket						TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;										// 0034
	Char 							DestIP[4];									// 4
	Word							CmdPort;									// 2
	Byte								my_reserved[4];
	Byte 							ProductType[4];								// 4
	Word							ProtocolVersion;							// 2
	Byte 							MyGroupName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte 							MachineName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte								username[NET_PROTOCOL_GROUPNAME_SIZE];								// 8
	Dword					 		MyTicks;									// 006C
	Dword							CrcCode;
}__attribute__ ((packed))TF_ConnectPacket;

typedef struct ConnectReplyPacket															// 0x0201
{
	HeaderPacket						TF_HeaderPacket;							// 002A
	CmdPacket						CmdTF;										// 0034
	Byte 							PCIp[4];									// 4
	Word							CmdPort;									// 2
	Byte 							ProductType[4];								// 4
	Word							ProtocolVersion;							// 2
	Byte 							MyGroupName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte 							MachineName[NET_PROTOCOL_GROUPNAME_SIZE];	// 12
	Byte								username[NET_PROTOCOL_GROUPNAME_SIZE];								// 8
	Dword					 		MyTicks;									// 006C
	Dword					 		reply_ticks;
	Word							reply_seq_id;
	Dword							CrcCode;
}__attribute__ ((packed))TF_ConnectReplyPacket;

typedef struct ConnectPasswordPacket
{	
	HeaderPacket						TF_HeaderPacket;
	CmdPacket						CmdTF;
	//Byte 							UserName[NET_PROTOCOL_GROUPNAME_SIZE];
	Byte 							Password[NET_PROTOCOL_GROUPNAME_SIZE];
	//Byte 							Config[18];		
	Dword  							MyTicks;
	//Dword  							ReplyTicks;
	//Word 							ReplySeqID;
	//Byte  							SupportList[12];
	Dword							CrcCode;
}__attribute__ ((packed))TF_ConnectPasswordPacket;

typedef struct ConnectPasswordReplyPacket
{
	HeaderPacket						TF_HeaderPacket;
	CmdPacket						CmdTF;
	Byte 								Password[NET_PROTOCOL_GROUPNAME_SIZE];
	//Byte 								IOCtrlDstMac[6];
	//Char 							DestIP[4];
	//Byte 								Config[18];
	Dword							MyTicks;
	Dword							ReplyTicks;
	Word							ReplySeqID;
	//Byte								SupportList[12];
	Dword							CrcCode;
}__attribute__ ((packed))TF_ConnectPasswordReplyPacket;

typedef struct DisconnectingPacket
{
	HeaderPacket						Header;
	CmdPacket						Command;
	Byte 								UserName[NET_PROTOCOL_GROUPNAME_SIZE];
	Dword							MyTicks;
	Byte								Disconnect_reason; // 0 is timeout, 1 is user disconnect, 2 is tx datalost
	Dword							CrcCode;
}__attribute__ ((packed))TF_DisconnectingPacket;

typedef struct AliveReportPacket
{
	HeaderPacket						Header;
	CmdPacket						Command;
	Byte 								frame_counter;
	Byte 								success_frames;
	Byte 								fail_frames;
	Dword							MyTicks;
	Word 							display_width;
	Word 							display_height;
	Byte	 							kb_push_flag;
	Byte								kb_push_no;
	Byte								kb_scan_code[20];
	Byte								audio_status;// b7: audio abnormal, [b6:b0]: audio buffer count if b7 is set.
	Byte								next_flag;
	Dword							CrcCode;
}__attribute__ ((packed))TF_AliveReportPacket;

typedef struct DataLostReportPacket
{
	HeaderPacket						Header;
	CmdPacket						Command;
	Dword							MyTicks;
	Dword							CrcCode;
}__attribute__ ((packed))TF_DataLostReportPacket;

// OPCODE_TF6x0_SOURCE_STATUS_ANNOUNCE = 0x0303,
typedef struct SourceStatus
{
	HeaderPacket						TF_HeaderPacket;
	CmdPacket						CmdTF;
	Byte 								IOCtrlDstMac[6];
	Char 							DestIP[4];
	Word 							InputSource;
	Word 							SourceWidth;
	Word 							SourceHeight;
	Word 							SourceFrame;
	Word 							XmtWidth;
	Word 							XmtHeight;
	Word 							XmtAudio;	
	Dword  							MyTicks;
	Byte  							RemoteOff;		// 0- remote off, others- remote on 1: IR remote on, 2: PS2 remote on, 4: audio remote on
	Byte  							CompressHightlow;	// 0:default, 1:low, 2:mid, 3:high
	Byte  							HDCP_ONOFF;		// 0- hdcp disable, 1- hdcp enable
	Byte  							Macrovision;			// 0- macrovision off,  others- macrovision mode
	Byte  							AviInfoDB;			// AVI Infoframe Data Byte2  (M1 M0 is refered currently.)
	Byte  							ScaleLevel;		//用来拉伸屏幕，因为在不同的电视机上，显示效果有微小的区别，有的超出屏幕，有的有黑边，使
													//用这个参数可以实现屏幕的整体缩放
	Byte  							audioSampleRate;	//一个字节， 表示声音的频率, 0=48k, 1=44.1k
	Byte								Position;			//[robin] 2012/02/24 表示手机目前的的方向1:vertical 0:horisental
	Word							TvRotate;		//0x0000: 横屏，0x0001:顺时针90度，0x0002:逆时针90度
	Dword							CrcCode;
}__attribute__ ((packed))TF_SourceStatus;

typedef enum tagNET_PROTOCOL_OPCODE
{
	OPCODE_TF6x0_DISCOVER					= 0x0101,
	OPCODE_TF6x0_CONNECTING				= 0x0201,
	OPCODE_TF6x0_DISCONNECTING              		= 0x0202,
	OPCODE_TF6x0_CONNECTING_PASSWORD	= 0x0204,
	OPCODE_TF6x0_ALIVE_REPORT				= 0x0301,
	OPCODE_TF6x0_DATALOST_REPORT			= 0x0302
}NET_PROTOCOL_OPCODE;

class CWiMoControl;
class CWiMoNetwork
{
public:
	CWiMoNetwork(CWiMoControl* pis);
	~CWiMoNetwork();
	int SendJPGVideo(void);	
	void SendTick(void);
	void SetDisconnectReason(int iReasion);
	int GetDisconnectReason();
	long GetServerIpAddr(void);
private:
	static void * HandshakeThread(void * param);
	static void * NetworkRecvThread(void * param);
	static void* CheckAliveAndResponseReportThread(void * param);

	int InitNet();
	int UnInitNet();
	int NetworkInit(void);
	long long GetIPAndMACAddress(unsigned char* pMac);
	void InitSendDiscoveryCmd();
	void InitSendConnectCmd();
	void InitConnectPassword();
	void InitSendSourceStatus();
	void InitAliveInformation();
	void SendDataLostReport();
	void SendDisconnectCmd();
	bool CheckCRC32(void* buffer, int dataLen);
	
	CWiMoControl* m_cControl;
private:
	struct sockaddr_in server;
	
	TF_CommnadPacket					IOCtrlSendPacketCmd;
	TF_RecvCommand 					IOCtrlRecvPacketCmd;
	TF_DiscoveryPacket 					IOCtrlSendDiscoveryCmd;
	TF_DiscoveryReplyPacket 				IOCtrlRecvDiscoveryCmd;
	TF_ConnectPacket					IOCtrlSendConnectCmd;
	TF_ConnectReplyPacket 				IOCtrlRecvConnectCmd;
	TF_ConnectPasswordPacket			IOCtrlSendConnectPasswordCmd;
	TF_ConnectPasswordReplyPacket		IOCtrlRecvConnectPasswordCmd;
	TF_DisconnectingPacket				IOCtrlSendDisconnectReason;
	TF_AliveReportPacket				IOCtrlSenAliveReportCmd;
	TF_DataLostReportPacket				IOCtrlSendDataLostReportCmd;
	TF_SourceStatus 					IOCtrlSendSourceStatusCmd;

	Byte IOCtrlSupportList[12];
	in_addr TXipaddr;
	Char IOCtrlDstIP[4];
	Char RXDEVIP[128];
	Char multicastIP[128];	
	Byte IOCtrlDstMac[6];

	char* sendSrcBuff;
	char* sendVideoTXbuffer;
	
	short frameNumber;	
	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;

	int IOCtrlStep;
	int nResolutionTicks;
	int mVersion;
	int mCodec;
	
	int socketHandle;
	int send1SocketHandle;
	int send2SocketHandle;
	int iDisconnectReason;//0x0:is timeout, 0x01: user disconnect, 0x02: tx datalost
	DDword overTime;
	
	pthread_t networkRecvThread_hnd;
	pthread_t CheckAliveResponseReportThread_hnd;
	pthread_t handshakeResponseThread_hnd;
};

};
#endif
