
#pragma once

//#ifdef _DEBUG
//#define TRACE_PACKET_DEBUG
//#define TRACE2_PACKET_DEBUG
//#endif

#include "BaseSystem.h"
typedef unsigned short	UINT16;

class BaseSocket;

#define PACKET_SIZE_PICE		256
#define PACKET_SIZE_PICE_HEADER	6
#define PACKET_SIZE_PICE_DATA	(PACKET_SIZE_PICE-PACKET_SIZE_PICE_HEADER)
#define PACKET_SIZE_SINGLE		1024
#define SIZE_SHORT				sizeof(UINT16)

class BaseNetConnector// : public PTAllocT<BaseNetConnector>
{
public:
	static BaseNetConnector	sm_sample;
	BaseNetConnector(UINT16 _nQueueSize=1024);
	~BaseNetConnector(void);

	void init(UINT16 _nQueueSize=1024);
	void release();
	void releaseQueue();

	enum{
		UNKNOW_TYPE,
		TYPE_INDEX_REQ,
		TYPE_INDEX_RES,
		TYPE_ODERED,// as send this data, record to send and then check to received or not by remote.
		TYPE_REALTIME,// if fail to send, don't send data again, also don't record to send.
		TYPE_PING_REQ,
		TYPE_PING_RES,
		TYPE_SUCCESS,
		TYPE_MISSED,
		TYPE_DISCONNECT,
		TYPE_TIMEOUT,
		TYPE_OVERFLOW,
		TYPE_BANDWIDTH,
		TYPE_RESET,
		TYPE_FAILTOCREATE,
		TYPE_MAX
	};

	enum {
		CT_NONE,
		CT_TRING,
		CT_CONNECTED,
		CT_DISCONNECTED,
		CT_RELEASE
	};

	typedef struct __ST_NET_PACKET_QUEUE__
	{
		UINT16	nType;
		UINT16	nSize;
		char *pData;
	}ST_NetPacketQueue;

	typedef struct __ST_NET_PACKET_PICE__
	{
		UINT16	nType;// 2 byte
		UINT16	nSerialNumber;
		UINT16	nSize;
		char Data;
	}ST_NetPacketPice;
	typedef struct __ST_NET_PACKET_SINGLE__
	{
		UINT16	nSize;
		UINT32	nTimeSend;
		char Data;// # of ST_NetPacketPice
	}ST_NetPacketSingle;

	typedef struct __ST_NET_PACKET_LIST__
	{
		UINT32		nTimeSendFirst;
		UINT32		nTimeSendOthers;
		bbyte				nSendCount;
		ST_NetPacketPice	*pstPacketPice;
	}ST_NetPacketList;
#ifdef _WIN32
	typedef std::vector<ST_NetPacketList, PT_allocator<ST_NetPacketList> >	STLVPacketList;
#else
	typedef std::vector<ST_NetPacketList>	STLVPacketList;
#endif

	UINT16 get_size_sending();
	bool send(UINT16 _nSize, const char *_pData, UINT16 _nType=TYPE_ODERED);
	UINT16 recv(char *_pData, UINT16 _nBufferSize, UINT16 *_pnType=NULL);
	void disconnect();
	UINT16 update(char *_pPacket, UINT16 _nSize);
	UINT32 time_remote();
	void timeout_set(UINT32 _nTimeout);
	int connect_type();
	UINT32 time_respons_avg()
	{
		return m_nTimeResponsAvg;
	}

	friend class BaseNetConnector;
	//==========================================================================
	// Start Update by OJ : 2011-02-10
public:
	UINT32 index_get()
	{
		return m_nIndex;
	}
	void set_server(bool _bServer){ 
		m_bServer = _bServer;
	}
	bool is_sending();

	void set_ip(const char *_anIP){
		memcpy(m_anIP, _anIP, 4);
	}
protected:
	UINT32	m_nIndex;
	char			m_anIP[4];
	//================================= End Update by OJ : 2011-02-10
protected:
	//==========================================================================
	// Start Update by OJ : 2011-01-17
	UINT16 packet_make_success_(char *_pBuf, UINT16 _nLeaveBuf);// success
	UINT16 packet_make_miss_(char *_pBuf, UINT16 _nLeaveBuf);// miss
	UINT16 packet_make_resend_miss_(char *_pBuf, UINT16 _nLeaveBuf);// miss
	UINT16 packet_make_data_(char *_pBuf, UINT16 _nLeaveBuf);
	UINT16 packet_make_resend_over_(char *_pBuf, UINT16 _nLeaveBuf);
	UINT16 packet_make_ping_req_(char *_pBuf, UINT16 _nLeaveBuf);
	UINT16 packet_make_ping_res_(char *_pBuf, UINT16 _nLeaveBuf);
	char			*m_pPacketSending;
	char			*m_pPacketSendingPos;
	UINT16	m_nSizeSending;
	UINT16	m_nSerialSend;
	//================================= End Update by OJ : 2011-01-17
	//==========================================================================
	// Start Update by OJ : 2011-01-17
	UINT16 packet_process_success_(char *_pBuf, int *_pnSizeSuccess);// success
	UINT16 packet_process_miss_(char *_pBuf);// 
	UINT16 packet_process_data_(char *_pBuf);
	UINT16 packet_process_ping_req_(char *_pBuf);
	UINT16 packet_process_ping_res_(char *_pBuf);
	bool		packet_process_check_miss_(UINT16 _nSerialLast);
	void		packet_process_check_success_(UINT16 _nSerialLast);
	void		packet_process_check_responstime_(UINT32 _nTimeRespons);
	void		packet_process_update_();
	char			*m_pPacketReceiving;
	char			*m_pPacketReceivingPos;
	UINT16	m_nSizeReceiving;
	bool			m_bPingRequested;
	STLVUShort		*m_pstlVMissedReq;
	STLVUShort		*m_pstlVMissed;
	STLVUShort		*m_pstlVSuccessed;
	//================================= End Update by OJ : 2011-01-17
	
	STLVPacketList	*m_pstlVPacketListSended;
	STLVPacketList	*m_pstlVPacketListReceived;

	BaseCircleQueue	m_queueConnectorInput;
	BaseCircleQueue	m_queueConnectorOutput;
	bool	m_bHoldReceive;
	bool	m_bHoldSend;
	bool	m_bServer;
public:
	bool is_hold_receive(){ return m_bHoldReceive; }
	bool is_hold_send() { return m_bHoldSend; }
	bool is_useto_receivepacket() {	return m_bReceiveFirstPacket;	}
	bool is_serial_packet(UINT16 _nType);
	UINT32 time_from_create();
protected:
	int	m_nConnectType;
	bool	m_bReceiveFirstPacket;	// Used to receive packet
	UINT32 m_nTimeCreated;
	//==========================================================================
	// Start Update by OJ : 2011-01-25
public:
	UINT32 get_idle();
	void respons_get_times(UINT32	*_pnTimeMin, UINT32	*_pnTimeMax, UINT32	*_pnTimeAvg);
	void statistics(UINT32 *_pnCntMissed, UINT32 *_pnCntResended, UINT32 *_pnCntSuccessed, UINT32 *_pnSendingPacket, UINT32 *_pnReceivingPacket);
protected:
	UINT32	m_nCountMissed;
	UINT32	m_nCountResended;
	UINT32	m_nCountSuccessed;

	UINT32	m_nTimeResponsMin;
	UINT32	m_nTimeResponsMax;
	UINT32	m_nTimeResponsAvg;
	UINT32	m_nTimeLastSend;
	UINT32	m_nTimeOver;//----
	UINT32	m_nTimeLastReceive;
	UINT32	m_nTimeout;// disconnect if no packet for awhile m_nTimeout 
	UINT32	m_nIdelCount;
	//================================= End Update by OJ : 2011-01-25
	UINT16	m_nSerialComplete;
	UINT16	m_nSerialFinish;
	UINT16	m_nSerialReceiveMax;
#ifdef _DEBUG
	//STLVUShort		m_stlVSendedSuccess;
#endif
	UINT16	m_nSerialLast;

	
	INT32	m_nDelayPerPacket; // If missed packet is, post pond for each packet by this value
	INT32	m_nBandwidthStart;
	INT32	m_nBandwidthEnd;
	INT32	m_nBandwidthCount;
	INT32	m_nBandwidthLimit;
	INT32	m_nBandwidthSentByte;
	INT32	m_nBandwidthTimeNext;
	float	m_fBandwidthSpeed;

	//==========================================================================
	// Start Update by OJ : 2011-01-27
public:
	void time_set_remote(UINT32 _nTime);
	UINT32 timediffer_get_remote();
	UINT32 time_get_local();
	void bandwidth_set(INT32 _nByteLimit);

	bool m_bDebugDisable;

protected:
	UINT16	m_nQueueSize;
	UINT32	m_nTimeRemote;
	UINT32	m_nTimeDiffer;
	UINT32	m_nTimeLocal;
	//================================= End Update by OJ : 2011-01-27

	PT_OPTHeader;
public:
	static bool is_up_value(UINT16 _nUp, UINT16 _nDn); //
};


typedef void (*FuncOnReceive)(UINT32 _nIndex);

class BaseNetManager
{
public:
	PtObjectHeader2(BaseNetManager);

	BaseNetManager(bool _bServer, const char *_strIP=NULL, UINT16 _nPort=5333, int _index_start = 919000);
	BaseNetManager();
	virtual ~BaseNetManager();
	void init(bool _bServer, const char *_strIP=NULL, UINT16 _nPort=5333, int _index_start=919000);
	void reinit(); // for debug, remake socket and keep to continue.
	void releaseQueue();
	//void release();

	bool send(UINT32 _nIndex, char *_pData, UINT16 _nSize, UINT16 _nType=BaseNetConnector::TYPE_ODERED);
	void weakup();
	bool broadcast(char *_pData, UINT16 _nSize, UINT16 _nType=BaseNetConnector::TYPE_ODERED);
	UINT16 recv(UINT32 *_pnIndex, char *_pData, UINT16 _nSizeBuf, UINT16 *_pnType=NULL);
	void end();
	
	int managerid_get() { return (m_nServerPort * 1000 + m_nManagerID); }
	int server_port_get() { return m_nServerPort; }
	UINT32 get_time_response(UINT32 _nIndex);
	bool get_connector_address(UINT32 _nIndex, bbyte *_strIP, UINT16 *_pnPort);
	bool get_connector_address(UINT32 _nIndex, char *_strIP, UINT32 _nSize);
	void timeout_set(UINT32 _nTimeout);
	void bandwidth_set(UINT32 _nLimit);
	bool is_connect(UINT32 _nIndex);
	static char *ip_get(char anIp[4], char *_strIpOut);
	void print_all_connection(); // print all connection socket.
	UINT32 time_get_remote();
	size_t	size();// return number of client index;

	// for havvy trafic detecator
	static bool sm_bTracePacket;
	// end of havvy 
protected:
    static DEF_ThreadCallBack(update);
	UINT16 send_packet(BaseSocket &_Socket, char *_pAddress, UINT16 _nSize, UINT16 _nSizeHeader, char *_pBuffer, UINT32 _index);
	void connector_all_release();
	//==========================================================================
	// Start Update by OJ : 2011-01-19
public:
	void release_thread();
	bool is_release_thread() { return m_bReleaseThread; }
	void process_byself();
	bool is_process_byself() { return m_bProcessBySelf; }
protected:
	bool	m_bReleaseThread;
	bool	m_bProcessBySelf;
	//================================= End Update by OJ : 2011-01-19

	//==========================================================================
	// Start Update by OJ : 2011-01-19
	void push_input_();
	void pop_output_();
	BaseCircleQueue	m_queueInput;
	BaseCircleQueue	m_queueOutput;
	BaseCircleQueue m_queueOutDelay; // deliver respons time
	char			*m_pReceiveBuffer;
	//================================= End Update by OJ : 2011-01-19

	BaseSocket	*m_pSocket;
	bool	m_bBeginThread;
	bool	m_bEndThread;
	UINT32 m_nIndex;
	bool	m_bServer;
	UINT32 m_nSleepCnt;
	UINT32 m_nTimeOut;
	UINT32 m_nBandwidth;
	int		m_nServerPort;
	int		m_nManagerID;
	
	PT_OPTHeader;
	friend class BaseNetManager;
	//==========================================================================
	// Start Update by OJ : 2011-01-18
public:
	UINT32 index_get(){ return m_nIndex; }
	void	loop_sleep_set(UINT32 _nSleep) { m_nSleepCnt = _nSleep; }
	UINT32 index_get_new_(char *_pData, UINT16 _nSize, char *_psocketaddr, char *_anIP, short _nPort, int _index_start = -1);
	bool client_index_set(int _index_new);
	void release_index(UINT32 _nIndex);
	bool type_index_packet_check(UINT16 _nType){
		if(_nType == BaseNetConnector::TYPE_INDEX_RES || _nType == BaseNetConnector::TYPE_INDEX_REQ
			|| _nType == BaseNetConnector::TYPE_DISCONNECT || _nType == BaseNetConnector::TYPE_TIMEOUT
			|| _nType == BaseNetConnector::TYPE_OVERFLOW)
			return true;
		return false;
	}

	//================================= End Update by OJ : 2011-01-18

	//==========================================================================
	// Start Update by OJ : 2011-01-18
public:
	typedef struct __ST_CONNECTOR__{
		UINT32		nIndex;
		char				anIP[4];
		short				nPort;
		BaseNetConnector	*pConnector;
		char				sockaddr[16];
		//struct sockaddr_in {
		//	short   sin_family;
		//	u_short sin_port;
		//	struct  in_addr sin_addr;
		//	char    sin_zero[8];
		//;}
	}ST_BaseNetConnector;
	typedef std::map<UINT32, BaseNetManager::ST_BaseNetConnector, std::less<UINT32>, PT_allocator<std::pair<const UINT32, BaseNetManager::ST_BaseNetConnector> > >		STLMnstConnector;
	typedef std::vector<BaseNetManager::ST_BaseNetConnector, PT_allocator<BaseNetManager::ST_BaseNetConnector> >						STLVstConnector;
	
	bool m_bDebugDisable;
protected:
	STLMnstConnector	*m_pstlMnstConnector;	// accessable by thread
	STLVstConnector		*m_pstlVstConnector;	// accessable by thread
	//================================= End Update by OJ : 2011-01-18

	
	//==========================================================================
	// Start Update by OJ : 2015-03-11
protected:
	UINT32 m_nIndexCounter;
	FuncOnReceive m_funcOnReceive;
public:
	void set_onreceive(FuncOnReceive _func);
	void onreceive(UINT32 _nIndex);
	
	void local_message(UINT32 _nIndex, UINT16 _nType);
	void local_event_process();
	void send_disconnect(char *_sockaddr, UINT32 _index);
public:
	typedef struct __ST_CONNECTOR_EXT_{
		UINT32		nIndex;
		char				anIP[4];
		short				nPort;
		UINT32		nTimeResponse;
	}ST_BaseNetConnectorExt;
	typedef std::map<UINT32, BaseNetManager::ST_BaseNetConnectorExt, std::less<UINT32>, PT_allocator<std::pair<const UINT32, BaseNetManager::ST_BaseNetConnectorExt> > >		STLMnstConnectorExt;
	typedef std::vector<BaseNetManager::ST_BaseNetConnectorExt, PT_allocator<BaseNetManager::ST_BaseNetConnectorExt> >							STLVstConnectorExt;
	
	typedef struct __ST_EVENT_EXT_{
		UINT32		nIndex;
		UINT32		nEvent;
	}ST_EventExt;
	enum{
		EXT_NONE,
		EXT_DISCONNECT,
		EXT_EVENT_MAX
	};
	
protected:
	STLMnstConnectorExt		*m_pstlMnstConnectorExt;	// accessable by external main
	STLVstConnectorExt		*m_pstlVstConnectorExt;		// accessable by external main
	BaseCircleQueue			m_queueEvent;
	UINT32			m_nTimeDiffer;

	static char s_strIP[255];
	//================================= End Update by OJ : 2015-03-11


	//==========================================================================
	// Start Update by OJ : 2023-11-10 support mass packet
public:
	typedef struct __ST_MassBuffer_ {
		char* buffer;
		int received;
		int size;
	}ST_MassBuffer;
	char *MassReceive(int _idx, char* _packet, int _size); // when finished to get, return true;
	ST_MassBuffer* MassBuffer(int _idx);
	void MassRelease(int _idx);
	void MassReleaseAll();
protected:

	typedef std::map<int, ST_MassBuffer, std::less<int>, PT_allocator<std::pair<const int, ST_MassBuffer> > >	STLMnMassBuffer;
	STLMnMassBuffer	m_massBuffers;
	//================================= End Update by OJ : 2023-11-10

	//============================================ 2024-04-23
	// 
public:
	static STLVpVoid	ms_netManagers;
	static bool check(BaseNetManager* _manager);
	bool isok();
	//================================= End
};
