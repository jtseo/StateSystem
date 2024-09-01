#include "stdafx.h"
#include "BaseNetConnector.h"
#include "BaseSocket.h"
#ifdef _WIN32
#include <process.h>
#else
#endif
#include <algorithm>
#include "BaseResResource.h"
#include "BaseResManager.h"
#include "BaseResFile.h"

#define	XDIRECTX
#include "DebugContext.h"

#include <chrono>
#include <thread>

#define SOCKET_INDEX		91900
#define PACKET_TIME_OVER	60000
#define PACKET_SENDING_LIMIT	100
#define PACKET_RESEND_TIME_LIMIT	400
#define QUEUE_SIZE_HEADER	(sizeof(INT32)+SIZE_SHORT*2)

#define STATE_MAX_PACKET	(65000) // it can't be over the unsigned short size (64k) and some of headers are included on the size. so resonable size of pure data is 65000.

//#define TRACE_PACKET_DEBUG
const unsigned short s_nSizeHeader = sizeof(unsigned int) * 2 + sizeof(unsigned short);

bool BaseNetManager::sm_bTracePacket = false;
BaseNetConnector	BaseNetConnector::sm_sample(false);

PtObjectCpp(BaseNetManager);
PT_OPTCPP(BaseNetManager)
PT_OPTCPP(BaseNetConnector)

STLVpVoid BaseNetManager::ms_netManagers;

BaseNetConnector::BaseNetConnector(UINT16 _nQueueSize) : 
m_queueConnectorInput("BaseNetConnector::Input"),
m_queueConnectorOutput("BaseNetConnector::Output")
{
	//init(_nQueueSize);
}

void BaseNetConnector::init(UINT16 _nQueueSize)
{
	m_bDebugDisable			= false;
	m_nConnectType			= CT_NONE;
	m_bReceiveFirstPacket	= false;

	m_nQueueSize	= _nQueueSize;
	m_queueConnectorInput.init(m_nQueueSize);
	m_queueConnectorOutput.init(m_nQueueSize);
	m_nSerialSend		= 65534;
	m_nSerialFinish		= 65533;
	m_nSerialComplete	= 65534;
	m_nSerialReceiveMax	= 65533;
	m_bPingRequested	= false;

	static STLVPacketList stlVPacketList;
	PT_OAllocS(m_pstlVPacketListSended, STLVPacketList, stlVPacketList);
	PT_OAllocS(m_pstlVPacketListReceived, STLVPacketList, stlVPacketList);
	m_pstlVPacketListSended->reserve(1024);
	m_pstlVPacketListReceived->reserve(1024);

	m_nTimeLocal	= BaseSystem::timeGetTime();
	m_nTimeLastReceive	= m_nTimeLocal;
	m_nTimeRemote	= 0;
	m_nIdelCount = 0;

	m_nDelayPerPacket = 0;
	m_nTimeOver		= 800;
	m_nBandwidthLimit		= 1024 * 30;
	m_fBandwidthSpeed = (float)m_nBandwidthLimit;
	m_nBandwidthTimeNext	= 0;
	
	m_nTimeResponsMin	= -1;
	m_nTimeResponsMax	= -1;
	m_nTimeResponsAvg	= -1;

	m_nCountMissed		= 0;
	m_nCountResended	= 0;
	m_nCountSuccessed	= 0;
	m_nTimeout			= PACKET_TIME_OVER;

	m_bHoldReceive	= false;
	m_bHoldSend		= false;
	m_bServer		= false;
	m_nTimeCreated = BaseSystem::timeGetTime();

	m_nIndex			= 0;// 0인경우 인덱스를 요청도 받지도 않았다, 0보다 큰경우 할당을 했거나 인덱스를 받았다

	static STLVUShort stlVUShort;
	PT_OAllocS(m_pstlVSuccessed, STLVUShort, stlVUShort);
	PT_OAllocS(m_pstlVMissedReq, STLVUShort, stlVUShort);
	PT_OAllocS(m_pstlVMissed, STLVUShort, stlVUShort);
}

BaseNetConnector::~BaseNetConnector(void)
{
	release();
}

UINT32 BaseNetConnector::time_get_local()
{
	return m_nTimeLocal;
}

void BaseNetConnector::releaseQueue()
{
	m_queueConnectorInput.release();
	m_queueConnectorOutput.release();
}

void BaseNetConnector::release()
{
	if(m_pstlVSuccessed == NULL)
		return;

	PT_OFreeS(m_pstlVSuccessed);
	PT_OFreeS(m_pstlVMissedReq);
	PT_OFreeS(m_pstlVMissed);
	m_pstlVSuccessed = NULL;
	
	m_nConnectType	= CT_RELEASE;
	ST_NetPacketQueue *pPacketQ;

	while((pPacketQ = (ST_NetPacketQueue*)m_queueConnectorInput.pop()) != NULL)
	{
		PT_Free(pPacketQ->pData);
		PT_Free(pPacketQ);
	}

	while((pPacketQ = (ST_NetPacketQueue*)m_queueConnectorOutput.pop()) != NULL)
	{
		PT_Free(pPacketQ->pData);
		PT_Free(pPacketQ);
	}

	if(m_pPacketSending)
	{	
		PT_Free(m_pPacketSending);
		m_pPacketSending = NULL;
	}

	for(UINT32 i=0; i<m_pstlVPacketListReceived->size(); i++)
	{
		if(m_pstlVPacketListReceived->at(i).pstPacketPice)
			PT_Free(m_pstlVPacketListReceived->at(i).pstPacketPice);
	}
	if(m_pstlVPacketListReceived)
		PT_OFreeS(m_pstlVPacketListReceived);
	m_pstlVPacketListReceived = NULL;

	for(UINT32 i=0; i<m_pstlVPacketListSended->size(); i++)
	{
		if(m_pstlVPacketListSended->at(i).pstPacketPice)
			PT_Free(m_pstlVPacketListSended->at(i).pstPacketPice);
	}
	if(m_pstlVPacketListSended)
		PT_OFreeS(m_pstlVPacketListSended);
	m_pstlVPacketListSended = NULL;
	releaseQueue();
}

UINT16 BaseNetConnector::get_size_sending()
{
	return (UINT16)m_queueConnectorInput.size_data();
}

bool BaseNetConnector::send(UINT16 _nSize, const char *_pData, UINT16 _nType)
{
	if (m_nConnectType == CT_NONE)
		m_nConnectType = CT_TRING;

	if(_nType == TYPE_REALTIME
		&& _nSize > PACKET_SIZE_PICE_DATA)
	{// Realtime data는 256(PACKET_SIZE_PICE_DATA)바이트 크기 이상을 보낼 수 없다
		B_ASSERT(_nSize <= PACKET_SIZE_PICE_DATA);
#ifdef TRACE_PACKET_DEBUG
	if(!m_bDebugDisable)
		g_SendMessage(LOG_MSG_CONSOLE, "Block to send big realtime date %d-----------------------------------\n", _nSize);
#endif
		return false;
	}

	if(m_bHoldSend)
	{	
#ifdef TRACE_PACKET_DEBUG
	if(!m_bDebugDisable)
		g_SendMessage(LOG_MSG_CONSOLE, "Holded to send %d-----------------------------------\n", m_queueConnectorInput.size_data());
#endif
		return false;
	}
	//if(!m_bBeginThread)
	//{
	//	_beginthread(update_, 0, this);
	//	m_bBeginThread	= true;
	//}

	UINT16	nSizeQueue	= _nSize + SIZE_SHORT;

	char *pData	= PT_Alloc(char, _nSize + SIZE_SHORT);
	memcpy(pData, &_nSize, SIZE_SHORT);
	memcpy(pData+SIZE_SHORT, _pData, _nSize);
	ST_NetPacketQueue *pPacketQ	= PT_MAlloc(ST_NetPacketQueue);

	pPacketQ->nSize	= nSizeQueue;
	pPacketQ->nType	= _nType;
	pPacketQ->pData	= pData;
	//g_SendMessage(LOG_MSG, "push message --------------- %x %x", this, &m_queueConnectorInput);
	if(!m_queueConnectorInput.push(pPacketQ))
	{
		m_bHoldSend = true;
		PT_Free(pData);
		PT_Free(pPacketQ);
		g_SendMessage(LOG_MSG_FILELOG, "Fail to push data %d-----------------------------------\n", m_queueConnectorInput.size_data());
		return false;
	}

#ifdef TRACE_PACKET_DEBUG
	if(!m_bDebugDisable)
		g_SendMessage(LOG_MSG_CONSOLE, "Send data %d\n", m_queueConnectorInput.size_data());
#endif
	return true;
}

UINT16 BaseNetConnector::recv(char *_pData, UINT16 _nBufferSize, UINT16 *_pnType)
{
	//if(!m_bBeginThread)
	//{
	//	_beginthread(update_, 0, this);
	//	m_bBeginThread	= true;
	//}
	if(_pnType)
		*_pnType = 0;

	ST_NetPacketQueue *pPacketQ;
	if((pPacketQ = (ST_NetPacketQueue*)m_queueConnectorOutput.pop()) != NULL)
	{
		m_bHoldReceive = false;
		UINT16 nSize = 0;
		
		if(pPacketQ->nType != TYPE_DISCONNECT
			&& pPacketQ->nType != TYPE_TIMEOUT)
		{
			nSize	= *((UINT16*)pPacketQ->pData);
			B_ASSERT(nSize <= _nBufferSize);
			if(nSize <= _nBufferSize)
				memcpy(_pData, pPacketQ->pData+SIZE_SHORT, nSize);
		}

		if(_pnType)
			*_pnType	= pPacketQ->nType;
#ifdef TRACE_PACKET_DEBUG
		if(!m_bDebugDisable)
			g_SendMessage(LOG_MSG_CONSOLE, "Recv Data\n");
#endif		
		PT_Free(pPacketQ->pData);
		PT_Free(pPacketQ);
		if(nSize <= _nBufferSize)
			return nSize;
	}

	return 0;
}

int BaseNetConnector::connect_type()
{
	return m_nConnectType;
}

void BaseNetConnector::disconnect()
{
	g_SendMessage(LOG_MSG_FILELOG, ":::::::::::::::::disconnect index%d\n", m_nIndex);
	m_nConnectType	= CT_DISCONNECTED;
}

void BaseNetConnector::time_set_remote(UINT32 _nTime)
{
	m_nTimeRemote	= _nTime;
}

UINT32 BaseNetConnector::timediffer_get_remote()
{
	return m_nTimeDiffer;
}

UINT16 BaseNetConnector::packet_make_success_(char *_pBuf, UINT16 _nLeaveBuf)
{
	//CORUM_PROFILE();

	INT32 nRetSize	= 0;

	if(m_pstlVSuccessed->size() == 0)
		return 0;
	UINT16 nSize	= (UINT16)m_pstlVSuccessed->size();

	if((nSize+2)*SIZE_SHORT > _nLeaveBuf)
		return 0;

	*((short*)_pBuf)	= TYPE_SUCCESS;
	_pBuf	+= SIZE_SHORT;
	memcpy(_pBuf, &nSize, SIZE_SHORT);
	_pBuf	+= SIZE_SHORT;
	memcpy(_pBuf, &m_pstlVSuccessed->at(0), SIZE_SHORT*nSize);
	nRetSize	= (nSize+2)*SIZE_SHORT;

#ifdef TRACE_PACKET_DEBUG
	if(!m_bDebugDisable)
		for(UINT32 i=0; i<m_pstlVSuccessed->size(); i++)
			g_SendMessage(LOG_MSG_CONSOLE, "Make success serial:%d\n", m_pstlVSuccessed->at(i));
#endif

	m_pstlVSuccessed->clear();
	return nRetSize;
}

// 받지 못한 패킷을 리모트에 알려준다.
UINT16 BaseNetConnector::packet_make_miss_(char *_pBuf, UINT16 _nLeaveBuf)
{
	//CORUM_PROFILE();
	INT32 nRetSize	= 0;

	if(m_pstlVMissedReq->size() == 0)
		return 0;

	m_pstlVMissedReq->clear();
	for(UINT16 nMiss = m_nSerialFinish+1; nMiss != m_nSerialReceiveMax; nMiss++)
	{
		bool bFind	= false;
		for(UINT32 i=0; i<m_pstlVPacketListReceived->size(); i++)
		{
			if(m_pstlVPacketListReceived->at(i).pstPacketPice
				&& m_pstlVPacketListReceived->at(i).pstPacketPice->nSerialNumber == nMiss)
			{
				bFind	= true;
				break;
			}
		}
		if(!bFind)
			m_pstlVMissedReq->push_back(nMiss);// Missed Packet을 보낼거라면 없는 패킷을 다 보낸다
	}

	if(m_pstlVMissedReq->size() == 0)
		return 0;

	UINT16 nSize	= (UINT16)m_pstlVMissedReq->size();
	if((nSize+2)*SIZE_SHORT > _nLeaveBuf)
		return 0;

	*((short*)_pBuf)	= TYPE_MISSED;
	_pBuf	+= SIZE_SHORT;
	memcpy(_pBuf, &nSize, SIZE_SHORT);
	_pBuf	+= SIZE_SHORT;
	memcpy(_pBuf, &m_pstlVMissedReq->at(0), SIZE_SHORT*nSize);
	nRetSize	= (nSize+2)*SIZE_SHORT;

	m_pstlVMissedReq->clear();
	return nRetSize;
}

// 리모트에서 받지 못했다고 알려준 패킷을 다시 보낸다.
UINT16 BaseNetConnector::packet_make_resend_miss_(char *_pBuf, UINT16 _nLeaveBuf)
{
	//CORUM_PROFILE();
	INT32 nRetSize	= 0;

	if(m_pstlVMissed->size() == 0)
		return 0;

	//UINT16 nSize	= m_pstlVMissed->size();
	for(UINT32 i=0; i<m_pstlVMissed->size(); i++)
	{
		bool bFind	= false;
		for(UINT32 j=0; j<m_pstlVPacketListSended->size(); j++)
		{
			if(m_pstlVPacketListSended->at(j).pstPacketPice &&
				m_pstlVPacketListSended->at(j).pstPacketPice->nSerialNumber == m_pstlVMissed->at(i))
			{
				if(m_nBandwidthStart == 0 || m_nBandwidthStart > (INT32)m_pstlVPacketListSended->at(j).nTimeSendOthers)
					m_nBandwidthStart = m_pstlVPacketListSended->at(j).nTimeSendOthers;

				if(m_nBandwidthEnd == 0 || m_nBandwidthEnd < (INT32)m_pstlVPacketListSended->at(j).nTimeSendOthers)
					m_nBandwidthEnd = m_pstlVPacketListSended->at(j).nTimeSendOthers;

				UINT16 nSizeOfPice	= m_pstlVPacketListSended->at(j).pstPacketPice->nSize+PACKET_SIZE_PICE_HEADER;
				if(_nLeaveBuf < nSizeOfPice+nRetSize)
				{
					return nRetSize;
				}
				memcpy(_pBuf+nRetSize, m_pstlVPacketListSended->at(j).pstPacketPice, nSizeOfPice);
				nRetSize	+= nSizeOfPice;

				if(!m_bDebugDisable)
					g_SendMessage(LOG_MSG_FILELOG, "Make resend miss serial:%d index%d\n", m_pstlVMissed->at(i), m_nIndex);

				m_pstlVMissed->erase(m_pstlVMissed->begin()+i);
				bFind	= true;
				i--;

				break;
			}
		}

		if(!bFind)
		{
			g_SendMessage(LOG_MSG, "Unknow Packet Requested serial : %d\n", m_pstlVMissed->at(i));
		}
	}
	m_pstlVMissed->clear();
	
	return nRetSize;
}

bool compare_packet(BaseNetConnector::ST_NetPacketList _Packet1, BaseNetConnector::ST_NetPacketList _Packet2)
{
	if(_Packet1.pstPacketPice != NULL
		&& _Packet1.pstPacketPice == NULL)
		return true;

	if(_Packet1.pstPacketPice == NULL
		|| _Packet2.pstPacketPice == NULL)
		return false;
	
	return BaseNetConnector::is_up_value(_Packet2.pstPacketPice->nSerialNumber, _Packet1.pstPacketPice->nSerialNumber);
}

// 리모트에서 응답이 없는 패킷 중 일정 시간 이상 된 패킷을 다시 보낸다.
UINT16 BaseNetConnector::packet_make_resend_over_(char *_pBuf, UINT16 _nLeaveBuf)
{
	//CORUM_PROFILE();
	INT32 nRetSize	= 0;

	//ST_NetPacketList	*pPacketList	= NULL;
	if(m_pstlVPacketListSended->empty())
	{
		return 0;
	}

	std::sort(m_pstlVPacketListSended->begin(), m_pstlVPacketListSended->end(), compare_packet);

	INT32 nCountResending = 0;
	for(UINT32 j=0; j<m_pstlVPacketListSended->size(); j++)
	{
		if(m_pstlVPacketListSended->at(j).pstPacketPice 
			&& m_pstlVPacketListSended->at(j).nTimeSendOthers + m_nTimeOver < m_nTimeLocal)
		{
			if(m_pstlVPacketListSended->at(j).nTimeSendFirst + m_nTimeout < m_nTimeLocal)
			{
				ST_NetPacketQueue *pPacketQ	= PT_MAlloc(ST_NetPacketQueue);
				pPacketQ->nSize	= 0;
				pPacketQ->nType	= TYPE_TIMEOUT;
				pPacketQ->pData	= NULL;
				if(!m_queueConnectorOutput.push(pPacketQ))
				{
					m_bHoldReceive = true;
					PT_Free(pPacketQ);
                    disconnect();
				}
				return 0;
			}

			nCountResending++;
			if (nCountResending > 3) // When Number of packet what have no respons over than 3, don't send anymore.
			{
				if (m_nTimeOver > 10) // To make time out of too long response time, increase time over. 
					m_nTimeOver += m_nTimeOver / 10;
				else
					m_nTimeOver += 10;

				int anIp[4];
				for (int k = 0; k < 4; k++)
					if (m_anIP[k] >= 0)	anIp[k] = m_anIP[k];
					else anIp[k] = 256 + m_anIP[k]; 
				g_SendMessage(LOG_MSG_CONSOLE,"Over3: Index: %d(%d.%d.%d.%d), Resend cnt: %d\n", m_nIndex, anIp[0], anIp[1], anIp[2], anIp[3], nCountResending);
				return nRetSize;
			}

			UINT16 nSizeOfPice	= m_pstlVPacketListSended->at(j).pstPacketPice->nSize+PACKET_SIZE_PICE_HEADER;
			if(_nLeaveBuf < nSizeOfPice+nRetSize)
				return nRetSize;
			m_pstlVPacketListSended->at(j).nTimeSendOthers	= m_nTimeLocal + (m_nTimeOver * m_pstlVPacketListSended->at(j).nSendCount);
			m_pstlVPacketListSended->at(j).nSendCount++;
			memcpy(_pBuf+nRetSize, m_pstlVPacketListSended->at(j).pstPacketPice, nSizeOfPice);
			nRetSize	+= nSizeOfPice;

			m_nCountResended	+= 1;
			//if(!m_bDebugDisable)
			//	g_SendMessage(LOG_MSG_FILELOG, "packet make resend over serial:%d timeover:%d index%d\n", m_pstlVPacketListSended->at(j).pstPacketPice->nSerialNumber, m_nTimeOver, m_nIndex);
		}
	}
	return nRetSize;
}

bool BaseNetConnector::is_sending()
{
	if(m_pPacketSending == NULL 
		&& m_queueConnectorInput.size_data() == 0)
	{
		for(UINT32 i=0; i<m_pstlVPacketListSended->size(); i++)
		{
			if(m_pstlVPacketListSended->at(i).pstPacketPice != NULL)
				return true;
		}
		return false;
	}
	return true;
}

UINT16 BaseNetConnector::packet_make_data_(char *_pBuf, UINT16 _nLeaveBuf)
{
	//CORUM_PROFILE();
	UINT16	nType	= TYPE_ODERED;

	if(m_pstlVPacketListSended->size() > PACKET_SENDING_LIMIT)
	{
		bool bEmptySlot	= false;
		for(UINT32 i=0; i<m_pstlVPacketListSended->size(); i++)
		{
			if(m_pstlVPacketListSended->at(i).pstPacketPice == NULL)
			{
				bEmptySlot	= true;
				m_pstlVPacketListSended->erase(m_pstlVPacketListSended->begin()+i);
				break;
			}
		}
		if(bEmptySlot == false)
		{
			//printf("-");
			return 0;
		}
	}

	if(m_pPacketSending == NULL)
	{// 보내고 있는 데이터가 없는 경우 m_queueInput에서 데이터를 꺼낸다.
		ST_NetPacketQueue *pQueue	= (ST_NetPacketQueue*)m_queueConnectorInput.top();
		if(pQueue == NULL)
			return 0;
		if(m_nIndex == 0 && 
			(pQueue->nType != TYPE_INDEX_REQ && pQueue->nType != TYPE_INDEX_RES))
			return 0;

		if(pQueue->nType == TYPE_INDEX_RES)
			m_nIndex	= *((UINT32*)(pQueue->pData+SIZE_SHORT));

		m_queueConnectorInput.pop();
		m_bHoldSend = false;

		m_nSizeSending		= pQueue->nSize;
		m_pPacketSending	= pQueue->pData;
		m_pPacketSendingPos	= pQueue->pData;
		nType				= pQueue->nType;
		PT_Free(pQueue);
	}
	
	INT32 nRetSize	= m_nSizeSending;
	if(m_nSizeSending > PACKET_SIZE_PICE_DATA)
		nRetSize	= PACKET_SIZE_PICE_DATA;

	if(nRetSize+PACKET_SIZE_PICE_HEADER > _nLeaveBuf)
		return 0;

	ST_NetPacketPice *pPice	= (ST_NetPacketPice*)PT_Alloc(bbyte, nRetSize+PACKET_SIZE_PICE_HEADER);
	pPice->nType			= nType;
	if(is_serial_packet(nType))
	{
		pPice->nSerialNumber	= m_nSerialSend;
		m_nSerialSend++;
	}else{
		pPice->nSerialNumber	= 0;
	}

#ifdef TRACE_PACKET_DEBUG
	if(!m_bDebugDisable)
		g_SendMessage(LOG_MSG_CONSOLE, "Make data serial:%d\n", pPice->nSerialNumber);
#endif
	pPice->nSize			= nRetSize;
	memcpy(&pPice->Data, m_pPacketSendingPos, nRetSize);

	{// 분할 된 패킷이 모두 보내지면 m_queueInput에서 꺼낸 패킷을 지운다
		m_nSizeSending	-= nRetSize;
		m_pPacketSendingPos	+= nRetSize;
		if(m_nSizeSending == 0)
		{	
			PT_Free(m_pPacketSending);
			m_pPacketSending = NULL;
		}
	}

	nRetSize	+= PACKET_SIZE_PICE_HEADER;
	memcpy(_pBuf, pPice, nRetSize);

	if(is_serial_packet(nType))
	{// 추후 보내는데 실패할 경우를 대비해 PacketListSended에 추가한다
		ST_NetPacketList	stList;
		stList.nTimeSendFirst	= m_nTimeLocal;
		stList.nTimeSendOthers	= m_nTimeLocal;
		stList.nSendCount		= 1;
		stList.pstPacketPice	= pPice;

		for(UINT32 i=0; i<m_pstlVPacketListSended->size(); i++)
		{
			if(m_pstlVPacketListSended->at(i).pstPacketPice == NULL)
			{
				m_pstlVPacketListSended->at(i)	= stList;
#ifdef TRACE_PACKET_DEBUG
				for(UINT32 l=0; l<m_pstlVPacketListSended->size();l++)
				{
					if(m_pstlVPacketListSended->at(l).pstPacketPice==NULL)
						printf("*,");
					else
						printf("%d", m_pstlVPacketListSended->at(l).pstPacketPice->nSerialNumber % 10);
				}
				printf("\n");
#endif
				return nRetSize;
			}
		}

		m_pstlVPacketListSended->push_back(stList);
#ifdef TRACE_PACKET_DEBUG
		for(UINT32 l=0; l<m_pstlVPacketListSended->size();l++)
		{
			if(m_pstlVPacketListSended->at(l).pstPacketPice==NULL)
				printf("*,");
			else
				printf("%d", m_pstlVPacketListSended->at(l).pstPacketPice->nSerialNumber % 10);
		}
		printf("\n");
#endif
	}else{
		PT_Free(pPice);
	}
	return nRetSize;
}

UINT16 BaseNetConnector::packet_make_ping_req_(char *_pBuf, UINT16 _nLeaveBuf)
{
	if(SIZE_SHORT > _nLeaveBuf)
		return 0;
	*((short*)_pBuf)	= TYPE_PING_REQ;
	return SIZE_SHORT;
}

UINT16 BaseNetConnector::packet_make_ping_res_(char *_pBuf, UINT16 _nLeaveBuf)
{
	if(SIZE_SHORT > _nLeaveBuf)
		return 0;
	*((short*)_pBuf)	= TYPE_PING_RES;
	return SIZE_SHORT;
}

void BaseNetConnector::respons_get_times(UINT32	*_pnTimeMin, UINT32	*_pnTimeMax, UINT32	*_pnTimeAvg)
{
	*_pnTimeMin	= m_nTimeResponsMin;
	*_pnTimeMax	= m_nTimeResponsMax;
	*_pnTimeAvg	= m_nTimeResponsAvg;

	m_nTimeResponsMin	= -1;
	m_nTimeResponsMax	= -1;
	//m_nTimeResponsAvg	= -1;
}

void BaseNetConnector::timeout_set(UINT32 _nTimeout)
{
	m_nTimeout = _nTimeout;
}

UINT32 BaseNetConnector::get_idle()
{
	return BaseSystem::timeGetTime() - m_nTimeLastReceive;
}

void BaseNetConnector::packet_process_check_responstime_(UINT32 _nTimeRespons)
{
	if(_nTimeRespons == -1)
		return;

	if(m_nTimeResponsMin == -1 || _nTimeRespons < m_nTimeResponsMin)
		m_nTimeResponsMin	= _nTimeRespons;
	
	if(m_nTimeResponsMax == -1 || _nTimeRespons > m_nTimeResponsMax)
		m_nTimeResponsMax	= _nTimeRespons;

	if(m_nTimeResponsAvg == -1)
		m_nTimeResponsAvg	= _nTimeRespons;
	else
		m_nTimeResponsAvg	= (m_nTimeResponsAvg * 9 + _nTimeRespons)/10;

	m_nTimeOver = m_nTimeResponsAvg+200;

	if(_nTimeRespons <= m_nTimeResponsMin+5)
	{
		UINT32 nResposTime;
		nResposTime		= _nTimeRespons/2;
		m_nTimeDiffer	= (m_nTimeRemote + nResposTime) - m_nTimeLocal;
#ifdef TRACE_PACKET_DEBUG
		//if(!m_bDebugDisable)
		//{
		//	g_SendMessage(LOG_MSG, "Local %d, Remote %d\n", m_nTimeLocal, m_nTimeRemote);
		//	g_SendMessage(LOG_MSG, "Differ %d, Respos %d\n", m_nTimeDiffer, nResposTime);
		//}
#endif
	}
}

void BaseNetConnector::statistics(UINT32 *_pnCntMissed, UINT32 *_pnCntResended, UINT32 *_pnCntSuccessed, UINT32 *_pnSendingPacket, UINT32 *_pnReceivingPacket)
{
	*_pnCntMissed	= m_nCountMissed;
	*_pnCntResended	= m_nCountResended;
	*_pnCntSuccessed= m_nCountSuccessed;

	*_pnSendingPacket	= (UINT32)m_pstlVPacketListSended->size();
	*_pnReceivingPacket	= (UINT32)m_pstlVPacketListReceived->size();

	m_nCountMissed		= 0;
	m_nCountResended	= 0;
	m_nCountSuccessed	= 0;
}

UINT16 BaseNetConnector::packet_process_success_(char *_pBuf, int *_pnSizeSuccess)
{
	//CORUM_PROFILE();

	B_ASSERT(*((short*)_pBuf) == TYPE_SUCCESS);
	_pBuf	+= SIZE_SHORT;
	UINT16	nCount;
	nCount	= *((UINT16*)_pBuf);
	_pBuf	+= SIZE_SHORT;

	m_nCountSuccessed	+= nCount;
	m_nBandwidthCount	+= nCount;

	for(UINT32 i=0; i<nCount; i++)
	{
		bool find = false;
		for(UINT32 j=0; j<m_pstlVPacketListSended->size(); j++)
		{
			if(m_pstlVPacketListSended->at(j).pstPacketPice &&
				m_pstlVPacketListSended->at(j).pstPacketPice->nSerialNumber == *((UINT16*)_pBuf))
			{
				find = true;
				packet_process_check_responstime_(m_nTimeLocal - m_pstlVPacketListSended->at(j).nTimeSendFirst);
	
				if(m_nBandwidthStart == 0 || m_nBandwidthStart > (INT32)m_pstlVPacketListSended->at(j).nTimeSendOthers)
					m_nBandwidthStart = m_pstlVPacketListSended->at(j).nTimeSendOthers;

				if(m_nBandwidthEnd == 0 || m_nBandwidthEnd < (INT32)m_pstlVPacketListSended->at(j).nTimeSendOthers)
					m_nBandwidthEnd = m_pstlVPacketListSended->at(j).nTimeSendOthers;

				*_pnSizeSuccess += m_pstlVPacketListSended->at(j).pstPacketPice->nSize;

				PT_Free(m_pstlVPacketListSended->at(j).pstPacketPice);
				m_pstlVPacketListSended->at(j).nTimeSendFirst	= 0;
				m_pstlVPacketListSended->at(j).nTimeSendOthers	= 0;
				m_pstlVPacketListSended->at(j).pstPacketPice		= NULL;

#ifdef TRACE_PACKET_DEBUG
				if(!m_bDebugDisable)
					g_SendMessage(LOG_MSG_CONSOLE, "Process success serial:%d\n", *((UINT16*)_pBuf));
#endif	
#ifdef TRACE_PACKET_DEBUG
				for(UINT32 l=0; l<m_pstlVPacketListSended->size();l++)
				{
					if(m_pstlVPacketListSended->at(l).pstPacketPice==NULL)
						printf("*,");
					else
						printf("%d", m_pstlVPacketListSended->at(l).pstPacketPice->nSerialNumber % 10);
				}
				printf("\n");
#endif
				break;
			}
		}
		_pBuf	+= SIZE_SHORT;
	}

	return (nCount+2)*SIZE_SHORT;
}

UINT16 BaseNetConnector::packet_process_miss_(char *_pBuf)
{
	//CORUM_PROFILE();
	B_ASSERT(*((short*)_pBuf) == TYPE_MISSED);
	_pBuf	+= SIZE_SHORT;
	UINT16	nCount;
	nCount	= *((UINT16*)_pBuf);
	_pBuf	+= SIZE_SHORT;

	m_nCountMissed	+= nCount;
	m_pstlVMissed->clear();
	m_pstlVMissed->resize(nCount);
	memcpy(&m_pstlVMissed->at(0), _pBuf, SIZE_SHORT*nCount);
	for(UINT32 i=0; i<m_pstlVMissed->size(); i++)
		g_SendMessage(LOG_MSG_FILELOG, "Process process miss serial:%d index%d\n", m_pstlVMissed->at(i), m_nIndex);

	//for(UINT32 i=0; i<m_pstlVMissed->size(); i++)
	//{
	//	if(!is_up_value(m_pstlVMissed->at(i), m_nSerialComplete))
	//	{
	//		m_pstlVMissed->erase(m_pstlVMissed->begin()+i);
	//		i--;
	//	}
	//}

	return (nCount+2)*SIZE_SHORT;
}

void BaseNetConnector::packet_process_check_success_(UINT16 _nSerialLast)
{
	for(UINT32 i=0; i<m_pstlVSuccessed->size(); i++)
	{
		if(m_pstlVSuccessed->at(i) == _nSerialLast)
		{
			return;
		}
	}
#ifdef _DEBUG
	//m_stlVSendedSuccess.push_back(_nSerialLast);
#endif
	m_pstlVSuccessed->push_back(_nSerialLast);
}

bool BaseNetConnector::is_up_value(UINT16 _nUp, UINT16 _nDn)
{
	UINT16 _nTop	= -1;
	UINT16 _n4, _n1;
	_n4	= _nTop / 5 * 4;
	_n1 = _nTop / 5;

	if(_nUp > _nDn)
	{
		if(_nUp > _n4 && _nDn < _n1)
			return false;
		return true;
	}else if(_nUp < _nDn){
		if(_nDn > _n4 && _nUp < _n1)
			return true;
	}
	return false;
}

bool BaseNetConnector::packet_process_check_miss_(UINT16 _nSerialLast)
{
	//CORUM_PROFILE();

	if(_nSerialLast == m_nSerialFinish)
	{
#ifdef TRACE_PACKET_DEBUG
		if(!m_bDebugDisable)
			g_SendMessage(LOG_MSG_CONSOLE, "packet_process_data receive dual packet %d\n", _nSerialLast);	
#endif
		return false;
	}

	if(!is_up_value(_nSerialLast, m_nSerialFinish))
	{
#ifdef TRACE_PACKET_DEBUG
		if(!m_bDebugDisable)
			g_SendMessage(LOG_MSG_CONSOLE, "packet_process_data receive old packet %d\n", _nSerialLast);	
#endif
		//B_ASSERT(_nSerialLast > m_nSerialFinish);
		return false;
	}

	if(is_up_value(_nSerialLast, m_nSerialReceiveMax))
	{
		m_nSerialReceiveMax	= _nSerialLast;
#ifdef TRACE_PACKET_DEBUG
		if(!m_bDebugDisable)
			g_SendMessage(LOG_MSG_CONSOLE, "max serial %d\n", m_nSerialReceiveMax);	
#endif
	}

	if(_nSerialLast 
		== (UINT16)(m_nSerialFinish+1)) // cast연산자를 사용하지 않으면 INT32타입으로 연산되어 65536이 0으로 취급되지 않는다
	{
#ifdef TRACE_PACKET_DEBUG
		if(!m_bDebugDisable)
			g_SendMessage(LOG_MSG_CONSOLE, "finish serial %d = %d\n", m_nSerialFinish, _nSerialLast);	
#endif
		m_nSerialFinish	= _nSerialLast;
		if(m_nSerialReceiveMax == m_nSerialFinish)
			return true;
	}

	bool	bSeperated	= false;
	bool	bNoMoreUpValue	= true;
	UINT16	nSerial;
	UINT16	_nFinish	= m_nSerialFinish+1;
	do{
		bNoMoreUpValue	= true;
		UINT32 i;
		bool bFind	= false;
		for(i=0; i<m_pstlVPacketListReceived->size(); i++)
		{
			if(m_pstlVPacketListReceived->at(i).pstPacketPice == NULL)
				continue;
			nSerial	= m_pstlVPacketListReceived->at(i).pstPacketPice->nSerialNumber;
			if(bNoMoreUpValue && is_up_value(nSerial, _nFinish))
				bNoMoreUpValue	= false;
			if(nSerial == _nFinish)
			{
				if(!bSeperated)
				{
#ifdef TRACE_PACKET_DEBUG
					if(!m_bDebugDisable)
						g_SendMessage(LOG_MSG_CONSOLE, "finish serial: %d = %d\n", m_nSerialFinish, _nFinish);	
#endif
					m_nSerialFinish	= _nFinish;
				}
				_nFinish++;
				bFind	= true;
				break;
			}
		}

		if(bFind)
			continue;

		if(bNoMoreUpValue)
			break;

		bSeperated	= true;
		bool bPush	= true;
		for(i=0; i<m_pstlVMissedReq->size(); i++)
		{
			if(m_pstlVMissedReq->at(i) == _nFinish)
			{
				bPush	= false;
				break;
			}
		}
		if(bPush)
			m_pstlVMissedReq->push_back(_nFinish);
		_nFinish++;
	}while(1);
	
	return true;
}

void BaseNetConnector::packet_process_update_()
{
	//CORUM_PROFILE();
	UINT16 nStart	= m_nSerialComplete;
	bool	bFindStart	= false;

	do{
		if(m_pPacketReceiving == NULL)
		{
			ST_NetPacketPice	*pPice;
			nStart++;
			for(UINT32 i=0; i<m_pstlVPacketListReceived->size(); i++)
			{
				pPice	= m_pstlVPacketListReceived->at(i).pstPacketPice;
				if(!pPice)
					continue;
				if(pPice->nSerialNumber == nStart)
				{// Ordered Data의 헤더
					m_nSerialComplete	= nStart;
					m_nSizeReceiving	= *((UINT16*)(&pPice->Data));
					m_pPacketReceiving	= PT_Alloc(char, m_nSizeReceiving+SIZE_SHORT);
					memcpy(m_pPacketReceiving, &pPice->Data, pPice->nSize);
					m_pPacketReceivingPos	= m_pPacketReceiving + pPice->nSize;
					m_nSizeReceiving	-= (pPice->nSize - SIZE_SHORT);
					B_ASSERT(m_nSizeReceiving >= 0);
					bFindStart	= true;

					PT_Free(pPice);
					m_pstlVPacketListReceived->at(i).pstPacketPice	= NULL;
				}
			}
		}

		if(m_pPacketReceiving == NULL)
			break;
		if(m_pPacketReceiving && m_nSizeReceiving > 0)
		{
			do{
				bFindStart	= false;

				nStart++;
				ST_NetPacketPice	*pPice;
				for(UINT32 i=0; i<m_pstlVPacketListReceived->size(); i++)
				{
					pPice	= m_pstlVPacketListReceived->at(i).pstPacketPice;
					if(!pPice)
						continue;
					if(pPice->nSerialNumber == nStart)
					{// Ordered Data의 나머지 
						m_nSerialComplete	= nStart;
						memcpy(m_pPacketReceivingPos, &pPice->Data, pPice->nSize);
						m_pPacketReceivingPos	+= pPice->nSize;
						m_nSizeReceiving		-= pPice->nSize;
						B_ASSERT(m_nSizeReceiving >= 0);
						bFindStart	= true;

						PT_Free(pPice);
						m_pstlVPacketListReceived->at(i).pstPacketPice	= NULL;
					}
				}

				if(m_pPacketReceiving && m_nSizeReceiving == 0)
					break;

				if(!bFindStart)
					return;
			}while(1);	
		}

		if(m_pPacketReceiving && m_nSizeReceiving == 0)
		{
			ST_NetPacketQueue *pPacketQ	= PT_MAlloc(ST_NetPacketQueue);

			pPacketQ->nSize	= *((UINT16*)m_pPacketReceiving) + SIZE_SHORT;
			pPacketQ->nType	= TYPE_ODERED;
			pPacketQ->pData	= m_pPacketReceiving;
			if(m_queueConnectorOutput.push(pPacketQ))
			{
				m_pPacketReceiving	= NULL;
			}else{
				g_SendMessage(LOG_MSG, "Queue data out nSize %d %d\n", pPacketQ->nSize, m_nIndex);
				m_bHoldReceive = true;
				PT_Free(pPacketQ);
			}
		}

		if(!bFindStart)
			return;
		bFindStart	= false;
	}while(m_pPacketReceiving == NULL);
}

UINT16 BaseNetConnector::packet_process_data_(char *_pBuf)
{
	//CORUM_PROFILE();

	ST_NetPacketPice	*pPice;
	pPice	= (ST_NetPacketPice*)_pBuf;
	B_ASSERT(pPice->nType == TYPE_ODERED || pPice->nType == TYPE_REALTIME
		|| pPice->nType == TYPE_INDEX_REQ || pPice->nType == TYPE_INDEX_RES
		|| pPice->nType == TYPE_DISCONNECT);
	UINT16 nRetSize	 = 0;

	if(pPice->nType != TYPE_ODERED)
	{
		ST_NetPacketQueue *pPacketQ	= PT_MAlloc(ST_NetPacketQueue);

		pPacketQ->nSize	= pPice->nSize;
		pPacketQ->nType	= pPice->nType;
		
		if(pPice->nType == TYPE_INDEX_RES)
			m_nIndex	= *((UINT32*)((&pPice->Data)+SIZE_SHORT));

		if(pPice->nType == TYPE_DISCONNECT)
		{
			pPacketQ->pData	= NULL;
			pPacketQ->nSize = 0;
		}else{
			char	*pDataOut	= PT_Alloc(char, pPice->nSize);
			memcpy(pDataOut, &pPice->Data, pPice->nSize);
			pPacketQ->pData	= pDataOut;
		}

		bool bPush = true;
		if(is_serial_packet(pPice->nType))
		{
			packet_process_check_success_(pPice->nSerialNumber);// 성공한 패킷을 기록해 리모트에 알려준다.
			if(!packet_process_check_miss_(pPice->nSerialNumber))
				bPush = false;
		}

		if(bPush)
		{
			if(!m_queueConnectorOutput.push(pPacketQ))
			{
				if(pPacketQ->pData)
					PT_Free(pPacketQ->pData);
				PT_Free(pPacketQ);
				m_bHoldReceive = true;
			}
		}
		else {
			if(pPacketQ->pData)
				PT_Free(pPacketQ->pData);
			PT_Free(pPacketQ);
		}
		return pPice->nSize+PACKET_SIZE_PICE_HEADER;
	}else{
#ifdef	TRACE_PACKET_DEBUG
		if(!m_bDebugDisable)
			g_SendMessage(LOG_MSG_CONSOLE, "packet_process_data start serial %d\n", pPice->nSerialNumber);
#endif
		nRetSize	= pPice->nSize+PACKET_SIZE_PICE_HEADER;

		if(m_pstlVPacketListReceived->size() >= PACKET_SENDING_LIMIT
			&& (UINT16)(m_nSerialFinish+1) != pPice->nSerialNumber)
		{
			bool	bEmptySlot	= false;
			for(UINT32 i=0; i<m_pstlVPacketListReceived->size(); i++)
			{
				if(m_pstlVPacketListReceived->at(i).pstPacketPice == NULL)
				{
					bEmptySlot	= true;
					m_pstlVPacketListReceived->erase(m_pstlVPacketListReceived->begin()+i);
					break;
				}
				if(m_pstlVPacketListReceived->at(i).pstPacketPice->nSerialNumber 
					== pPice->nSerialNumber)
				{
					bEmptySlot	= true;
					break;
				}
			}
			if(bEmptySlot == false)
			{
				g_SendMessage(LOG_MSG, "packet_process_data over flow\n");
				bool bPush	= true;
				for(UINT32 i=0; i<m_pstlVMissedReq->size(); i++)
				{
					if(m_pstlVMissedReq->at(i) == (UINT16)(m_nSerialFinish+1))
					{
						bPush	= false;
						break;
					}
				}
				if(bPush)
					m_pstlVMissedReq->push_back(m_nSerialFinish+1);
				packet_process_update_();
				return nRetSize;
			}
		}
		packet_process_check_success_(pPice->nSerialNumber);// 성공한 패킷을 기록해 리모트에 알려준다.
		if(packet_process_check_miss_(pPice->nSerialNumber))// 중간에 빈 패킷이 있었는지 확인한다.
		{// false일 경우 이미 받아서 처리된 패킷이다.
			ST_NetPacketPice *pPiceReceive	= (ST_NetPacketPice*)PT_Alloc(char, pPice->nSize+PACKET_SIZE_PICE_HEADER);
			memcpy(pPiceReceive, pPice, pPice->nSize+PACKET_SIZE_PICE_HEADER);
			ST_NetPacketList stList;
			stList.nTimeSendFirst	= 0;
			stList.pstPacketPice	= pPiceReceive;

			UINT32 nPointNull	= -1;
			for(UINT32 i=0; i<m_pstlVPacketListReceived->size(); i++)
			{
				if(m_pstlVPacketListReceived->at(i).pstPacketPice == NULL)
				{
					if(nPointNull == -1)
						nPointNull	= i;
				}else if(m_pstlVPacketListReceived->at(i).pstPacketPice->nSerialNumber
							== stList.pstPacketPice->nSerialNumber)
				{
					nPointNull	= i;
					break;
				}
			}

			if(nPointNull == -1){
				m_pstlVPacketListReceived->push_back(stList);
			}else{
				if(m_pstlVPacketListReceived->at(nPointNull).pstPacketPice == NULL){
					m_pstlVPacketListReceived->at(nPointNull)	= stList;
				}else{
					PT_Free(pPiceReceive);
				}
			}
#ifdef	TRACE_PACKET_DEBUG
			if(!m_bDebugDisable)
				g_SendMessage(LOG_MSG_CONSOLE, "packet_process_data end serial %d\n", pPice->nSerialNumber);
#endif
		}
	}

	packet_process_update_();

	return nRetSize;
}

UINT16 BaseNetConnector::packet_process_ping_req_(char *_pBuf)
{
	B_ASSERT(*((short*)_pBuf) == TYPE_PING_REQ);
	m_bPingRequested	= true;
	return SIZE_SHORT;
}

UINT16 BaseNetConnector::packet_process_ping_res_(char *_pBuf)
{
	B_ASSERT(*((short*)_pBuf) == TYPE_PING_RES);
	return SIZE_SHORT;
}

UINT16 BaseNetConnector::update(char *_pPacket, UINT16 _nSize)
{
	//CORUM_PROFILE();

	m_nTimeLocal	= BaseSystem::timeGetTime();
	if(_nSize > 1)
	{
		m_nTimeRemote	= *((UINT32*)(_pPacket-4));
		m_nTimeLastReceive = m_nTimeLocal;
		m_nIdelCount = 0;
	}
	
	m_nBandwidthStart	= 0;
	m_nBandwidthEnd = 0;
	m_nBandwidthCount = 0;
	int nSuccessSum = 0, nSuccessSendableSum = 0;

	//bool	bSendIndex	= false;
	UINT16	nParsed	= 0;
	if(_nSize > 1)// 1인 경우 이차 업데이트로 실 패킷이 올 시간이 아니다.
	{// Receive
		m_nConnectType	= CT_CONNECTED;// Add or Update by OJ 2011-11-14 오후 1:50:38
		m_bReceiveFirstPacket	= true;// Add or Update by OJ 2011-11-14 오후 2:01:15
		short *pnType;
		while(_nSize > nParsed)
		{
			pnType	= (short*)(_pPacket+nParsed);
			switch(*pnType)
			{
			case TYPE_RESET:
				break;
			case TYPE_INDEX_RES:
			case TYPE_INDEX_REQ:
			case TYPE_ODERED:
			case TYPE_REALTIME:
			case TYPE_DISCONNECT:
				nParsed	+= packet_process_data_((char*)pnType);
				break;
			case TYPE_PING_REQ:
				nParsed	+= packet_process_ping_req_((char*)pnType);
				break;
			case TYPE_PING_RES:
				nParsed	+= packet_process_ping_res_((char*)pnType);
				break;
			case TYPE_MISSED:
				nParsed	+= packet_process_miss_((char*)pnType);
				break;
			case TYPE_SUCCESS:
				nParsed += packet_process_success_((char*)pnType, &nSuccessSum);
				m_nBandwidthSentByte -= nSuccessSum;
				break;
			default:
				g_SendMessage(LOG_MSG_FILELOG, "disconnect by Unknow packet. index%d\n", m_nIndex);
				return 0;
			}
		}
	}

	int nTimeLeng = m_nBandwidthEnd - m_nBandwidthStart;
	if (nSuccessSum > 0 && nTimeLeng > 0)
	{
		if (m_pstlVMissed->size() > 0){
			m_fBandwidthSpeed = (float)nSuccessSum / (float)nTimeLeng;
			g_SendMessage(LOG_MSG_FILELOG, "Bandwidth control Success:%d, TimeLen:%d, speed:%f, index%d\n", nSuccessSum, nTimeLeng, m_fBandwidthSpeed, m_nIndex);
			//18:53:19::Bandwidth control Success:196, TimeLen:144, speed:1, index26
		}
		else{
			m_fBandwidthSpeed = (float)m_nBandwidthLimit;
		}
	}
	nSuccessSendableSum = (int)(m_fBandwidthSpeed * (float)(m_nTimeLocal - m_nTimeLastSend));// make sandable bandwidth value in this term
	
	if(_nSize > 1)
	{
		if(m_pstlVMissed->size() == 0)
		{
			m_nDelayPerPacket -= 10;
			if(m_nDelayPerPacket > 0)
				g_SendMessage(LOG_MSG_FILELOG, "Delay updatge %d, index%d\n", m_nDelayPerPacket, m_nIndex);
		}

		if(m_nDelayPerPacket < 0)
			m_nDelayPerPacket = 0;
	}

	if (m_nTimeLastSend + m_nDelayPerPacket > m_nTimeLocal) {
		return 0;// Sending packet is decrased by defend on bandwidth(missed packet). 
	}
	
	unsigned short	nSended	= 0, nSize;
	
	//bool bBandover = false;
	//if (m_nBandwidthSentByte >= m_nBandwidthLimit
	//	&& m_pstlVPacketListSended->size() > 0){
	//	int anIp[4];
	//	for (int k = 0; k < 4; k++)
	//		if (m_anIP[k] >= 0)	anIp[k] = m_anIP[k];
	//		else anIp[k] = 256 + m_anIP[k]; 
	//	g_SendMessage(LOG_MSG_CONSOLE, "BandOver: Index %d(%d.%d.%d.%d) SendingByte %d\n", m_nIndex, anIp[0], anIp[1], anIp[2], anIp[3], m_nBandwidthSentByte);
	//	//printf("BandOver: Index %d(%d.%d.%d.%d) SendingByte %d\n", m_nIndex, anIp[0], anIp[1], anIp[2], anIp[3], m_nBandwidthSentByte);
	//	bBandover = true;
	//}else if(m_nBandwidthTimeNext == 0 || m_nBandwidthTimeNext < (int)m_nTimeLocal)
	//{
	//	m_nBandwidthSentByte = 0;
	//	m_nBandwidthTimeNext = m_nTimeLocal+1000;
	//}
	//
	{// Send
		bool bMakeDelay = false;
		if(m_pstlVMissed->size() > 2) // 미스패킷이 있어야 전송속도를 조절하기 시작한다.
			bMakeDelay = true;
		if(_nSize > 1)// 받은 패킷이 없는경우 성공 실패가 없다.
		{
			nSended	+= packet_make_success_(_pPacket+nSended, PACKET_SIZE_SINGLE-nSended);
			if(nSuccessSendableSum > nSended)
				nSended	+= packet_make_miss_(_pPacket+nSended, PACKET_SIZE_SINGLE-nSended);
			if (nSuccessSendableSum > nSended){
				nSize = packet_make_resend_miss_(_pPacket+nSended, PACKET_SIZE_SINGLE-nSended);
				nSended += nSize;
			}

			if(bMakeDelay)
			{
				INT32 nLength;
				nLength = m_nBandwidthEnd - m_nBandwidthStart;
				if(m_nBandwidthCount == 0)
					m_nDelayPerPacket = nLength;
				else
					m_nDelayPerPacket = nLength / m_nBandwidthCount;

				if(m_nDelayPerPacket > 100)
					m_nDelayPerPacket = 100;
				if(m_nDelayPerPacket < 0)
					m_nDelayPerPacket = 0;
				g_SendMessage(LOG_MSG_CONSOLE, "Delay %d, Cnt %d, Avg %d, over %d, index%d\n", m_nDelayPerPacket, m_nBandwidthCount, m_nTimeResponsAvg, m_nTimeOver,
					m_nIndex);
			}
		}
		if (m_pstlVMissed->size() == 0)
			nSuccessSendableSum = PACKET_SIZE_SINGLE * 2;

		static int last = 0;
		do{
			nSize = packet_make_data_(_pPacket + nSended, PACKET_SIZE_SINGLE - nSended);
			nSended += nSize;
		} while (nSize && nSuccessSendableSum > nSended);
		last = nSended;

		if (nSuccessSendableSum > nSended){
			nSize = packet_make_resend_over_(_pPacket + nSended, PACKET_SIZE_SINGLE - nSended);
			nSended += nSize;
		}

		if(m_bPingRequested)
		{
			if (nSuccessSendableSum > nSended){
				nSize = packet_make_ping_res_(_pPacket + nSended, PACKET_SIZE_SINGLE - nSended);
				m_bPingRequested = false;
				nSended += nSize;
			}
		}		
	}

/*
	if(m_nIdelCount > 3)
	{
		ST_NetPacketQueue *pPacketQ	= PT_MAlloc(ST_NetPacketQueue);
		pPacketQ->nSize	= 0;
		pPacketQ->nType	= TYPE_TIMEOUT;
		pPacketQ->pData	= NULL;
		if(!m_queueConnectorOutput.push(pPacketQ))
		{
			m_bHoldReceive = true;
			PT_Free(pPacketQ);
		}
		
		g_SendMessage(LOG_MSG_FILELOG, "Time Idle over %d, index%d\n", get_idle(), m_nIndex);
		return 0;
	}
	//*/
	/* // erase session ping for mobile environment.
	if(get_idle() > m_nTimeout)
	{
		m_nIdelCount++;
		m_nTimeLastReceive += m_nTimeOver;

		if(nSended == 0)
		{
			if (nSuccessSendableSum > nSended){
				nSize = packet_make_ping_req_(_pPacket + nSended, PACKET_SIZE_SINGLE - nSended);
				nSended += nSize;
			}
		
#ifdef	TRACE_PACKET_DEBUG
			if(!m_bDebugDisable)
				g_SendMessage(LOG_MSG_CONSOLE, "send ping to check session count:%d\n", m_nIdelCount);
#endif
		}
	}
	//*/
	if(nSended > 0)
		m_nTimeLastSend = m_nTimeLocal;
	
	m_nBandwidthSentByte += nSended;
	return nSended;
}

void BaseNetConnector::bandwidth_set(INT32 _nByteLimit)
{
	if(_nByteLimit < 10000)
		_nByteLimit = 1024*10;
	if(_nByteLimit > 1024 * 1024 * 10)
		_nByteLimit = 1024 * 1024 * 10;
	m_nBandwidthLimit = _nByteLimit;
}

BaseNetManager::BaseNetManager():
m_queueInput("BaseNetManager:Input"),
m_queueOutput("BaseNetManager:Output"),
m_queueOutDelay("BaseNetManager::OutDelay"),
m_queueEvent("BaseNetManager::Event")
{
	m_option = 0;
	init();
}

void BaseNetManager::init()
{
	m_bDebugDisable = false;
	m_pSocket	= NULL;

}

BaseNetManager::BaseNetManager(bool _bServer, const char *_strIP, UINT16 _nPort, int _index_start):
m_queueInput("BaseNetManager:Input"),
m_queueOutput("BaseNetManager:Output"),
m_queueOutDelay("BaseNetManager::OutDelay"),
m_queueEvent("BaseNetManager::Event")
{
	//g_SendMessage(LOG_MSG, "trace 002\n");
	init(_bServer, _strIP, _nPort, _index_start);
}

char BaseNetManager::s_strIP[255];

BaseNetManager::STLMnstConnector s_stlMnst;
BaseNetManager::STLVstConnector s_stlVst;
BaseNetManager::STLMnstConnectorExt s_stlMstE;
BaseNetManager::STLVstConnectorExt s_stlVstE;

void BaseNetManager::init(bool _bServer, const char *_strIP, UINT16 _nPort, int _index_start)
{
	ms_netManagers.push_back(this);

	static int s_nManagerSerial = 100;
	m_nManagerID = s_nManagerSerial++;

	m_bDebugDisable = false;
	m_funcOnReceive = NULL;
	m_pSocket	= NULL;
	m_nServerPort = _nPort;
	strcpy_s(s_strIP, 255, _strIP);
	
	UINT32 nQueueSize	= 10240;
	//if(_bServer)
	//	nQueueSize	= 10240;

	m_queueInput.init(nQueueSize);
	m_queueOutput.init(nQueueSize);
	m_queueEvent.init(nQueueSize);
	m_queueOutDelay.init(nQueueSize);

	m_nIndex	= 0;
	m_bBeginThread	= false;
	if(m_pSocket==NULL)
		PT_OAlloc3(m_pSocket, BaseSocket, _bServer, _strIP, _nPort);
	
	m_pSocket->bind();

	m_nSleepCnt		= 10;
	m_bEndThread	= false;
	m_bReleaseThread	= false;
	m_bProcessBySelf	= false;
	m_nIndexCounter = _index_start;
	m_bServer	= _bServer;
	m_nTimeOut = 60000;
	m_nBandwidth = 1024*30;

	printf("init manager socket port : %d\n", m_nServerPort);
	if (!m_bBeginThread)
	{
		PT_OAllocS(m_pstlMnstConnector, STLMnstConnector, s_stlMnst);
		PT_OAllocS(m_pstlVstConnector, STLVstConnector, s_stlVst);
	}

	PT_OAllocS(m_pstlMnstConnectorExt, STLMnstConnectorExt, s_stlMstE);
	PT_OAllocS(m_pstlVstConnectorExt, STLVstConnectorExt, s_stlVstE);

	if(m_bServer)
		m_pstlVstConnector->reserve(1024);

	if(!m_bServer)
	{
		m_pstlVstConnector->reserve(4);

		char anIP[4];
		short nPort;
		char stSocketaddr[16];
		m_pSocket->get_remote(stSocketaddr, anIP, &nPort);
		INT32 nIndex = (INT32)index_get_new_(NULL, 0, stSocketaddr, anIP, nPort, _index_start);

		//if(GetDebugContext().GetState() == 100)
		g_SendMessage(LOG_MSG_FILELOG, ":::::::::::::::::::::::Connect Server REQ Index %d %s: %d\n", nIndex, _strIP, _nPort);
		(*m_pstlMnstConnector)[nIndex].pConnector->send(4, (char*)&nIndex, BaseNetConnector::TYPE_INDEX_REQ);
	}
	else
		m_nIndex = _index_start;

	if(!m_bBeginThread)
	{
		m_bBeginThread	= true;
		//marker
        if(BaseSystem::createthread(update_, 0, this) == -1)
			m_bBeginThread = false;
    }
}

void BaseNetManager::releaseQueue()
{
	m_queueOutput.release();
	m_queueOutDelay.release();
	m_queueInput.release();
	m_queueEvent.release();
}

void BaseNetManager::reinit()
{
	BaseSocket *pSocket = NULL, *pSocketTemp;

	PT_OAlloc3(pSocket, BaseSocket, m_bServer, s_strIP, m_nServerPort);
	pSocket->bind();

	pSocketTemp = m_pSocket;
	m_pSocket = pSocket;

	PT_OFree(pSocketTemp);
}

void BaseNetManager::release()
{	
	if(m_pstlMnstConnectorExt == NULL)
		return;

	MassReleaseAll();
	m_bEndThread = true;

	INT32 nCnt  = 0;
	while(m_bBeginThread && nCnt < 500)
	{
		nCnt++;
		BaseSystem::Sleep(10);
	}

	if(m_pSocket)
		PT_OFree(m_pSocket);
	m_pSocket	= NULL;

	void *pQueue;
	while((pQueue = m_queueOutput.pop()) != NULL)
	{
		PT_Free(pQueue);
	}

	while ((pQueue = m_queueOutDelay.pop()) != NULL) 
	{
		PT_Free(pQueue);
	}

	while((pQueue = m_queueInput.pop()) != NULL)
	{
		PT_Free(pQueue);
	}

	PT_OFreeS(m_pstlMnstConnectorExt);
	PT_OFreeS(m_pstlVstConnectorExt);

	m_pstlMnstConnectorExt = NULL;
	m_pstlVstConnectorExt = NULL;
	releaseQueue();

	for (int i = 0; i < ms_netManagers.size(); i++)
	{
		if (ms_netManagers[i] == this) {
			ms_netManagers.erase(ms_netManagers.begin() + i);
			break;
		}
	}
}

BaseNetManager::~BaseNetManager()
{
	release();
}

bool BaseNetManager::client_index_set(int _index_new)
{
	if (!m_bServer && m_pstlVstConnector->size() == 1)
	{
		(*m_pstlVstConnector)[0].nIndex = _index_new;
		(*m_pstlMnstConnector).clear();
		(*m_pstlMnstConnector)[_index_new] = (*m_pstlVstConnector)[0];
		return true;
	}
	return false;
}

UINT32 BaseNetManager::index_get_new_(char *_pData, UINT16 _nSize, char *_psocketaddr, char *_anIP, short _nPort, int _index_start)
{
	INT32 nIndex = 0;
	
	BaseNetConnector	*pConnector	= NULL;
	for(UINT32 i=0; i<m_pstlVstConnector->size(); i++)
	{// 이미 패킷이 한번 왔으나 네트웍 상황에 따라 확인되지 않아서 다시 보냈을 수 도 있다.
		if(memcmp((*m_pstlVstConnector)[i].sockaddr, _psocketaddr, 16) == 0
			&& (*m_pstlVstConnector)[i].nPort == _nPort
			&& (*m_pstlVstConnector)[i].pConnector != NULL)
		{
			nIndex			= (INT32)(*m_pstlVstConnector)[i].nIndex;
			//pConnector		= (*m_pstlVstConnector)[i].pConnector;
			//pConnector->send(sizeof(UINT32), (char*)&nIndex, BaseNetConnector::TYPE_INDEX_RES);
			//pConnector->timeout_set(m_nTimeOut);
			return nIndex;
		}
	}

	if(!m_bServer)
		nIndex = _index_start;
	else
		nIndex = (INT32)++m_nIndexCounter;

	if (nIndex == -1)
		nIndex = SOCKET_INDEX;

	for(UINT32 i=0; i<m_pstlVstConnector->size(); i++)
	{
		if((*m_pstlVstConnector)[i].pConnector == NULL)
		{
			(*m_pstlVstConnector)[i].nIndex	= nIndex;
			memcpy((*m_pstlVstConnector)[i].sockaddr, _psocketaddr, 16);
			memcpy((*m_pstlVstConnector)[i].anIP, _anIP, 4);
			(*m_pstlVstConnector)[i].nPort = _nPort;
			PT_OAlloc1((*m_pstlVstConnector)[i].pConnector, BaseNetConnector, 10240);

#ifdef TRACE_PACKET_DEBUG
			(*m_pstlVstConnector)[i].pConnector->m_bDebugDisable = m_bDebugDisable;
#endif
			(*m_pstlMnstConnector)[nIndex]	= (*m_pstlVstConnector)[i];
			pConnector	= (*m_pstlVstConnector)[i].pConnector;
			break;
		}
	}

	if(pConnector == NULL)
	{
		ST_BaseNetConnector stConnector;
		stConnector.nIndex	= nIndex;
		memcpy(&stConnector.sockaddr[0], _psocketaddr, 16);
		memcpy(stConnector.anIP, _anIP, 4);
		stConnector.nPort = _nPort;
		PT_OAlloc1(stConnector.pConnector, BaseNetConnector, 10240);
		
#ifdef TRACE_PACKET_DEBUG
		stConnector.pConnector->m_bDebugDisable = m_bDebugDisable;
#endif
		m_pstlVstConnector->push_back(stConnector);
		(*m_pstlMnstConnector)[nIndex]	= stConnector;
		pConnector	= stConnector.pConnector;
	}

	if(m_bServer)
	{
		bbyte anIP[4];
		memcpy(anIP, _anIP, 4);
		// 서버일 경우 이 패킷을 무시하고 클라이언트 일 경우 첫번째 패킷은 인덱스를 알려주는 패킷으로 인식한다.
		//if(GetDebugContext().GetState() >= 50)
		//	g_SendMessage(LOG_MSG_FILELOG, "::::::::::::::::::::::::::: Create Index and send index%d (IP:%u.%u.%u.%u:%u) Connector# %d\n",nIndex, anIP[0], anIP[1], anIP[2], anIP[3], _nPort, m_pstlVstConnector->size());
		pConnector->send(4, (char*)&nIndex, BaseNetConnector::TYPE_INDEX_RES);
	}
	pConnector->set_server(m_bServer);
	pConnector->timeout_set(m_nTimeOut);
	pConnector->bandwidth_set(m_nBandwidth);
	pConnector->set_ip(_anIP);
	return nIndex;
}

void BaseNetManager::local_message(UINT32 _nIndex, UINT16 _nType)
{
	char *pQueue	= PT_Alloc(char, QUEUE_SIZE_HEADER);
	*((UINT32*)pQueue)	= _nIndex;
	*((UINT16*)(pQueue+sizeof(INT32)))	= 0;
	*((UINT16*)(pQueue+sizeof(INT32)+SIZE_SHORT))	= _nType;
	m_queueOutput.push(pQueue);
	if(m_funcOnReceive)
		m_funcOnReceive(_nIndex);
}

void BaseNetManager::push_input_()
{
	//CORUM_PROFILE();
	char *pQueue;

	UINT32	nIndex	= 0;
	UINT16	nSize	= 0;
	UINT16	nType	= 0;
	while((pQueue = (char*)m_queueInput.top()) != NULL)
	{
		nIndex	= *((UINT32*)pQueue);
		nSize	= *((UINT16*)(pQueue+sizeof(INT32)));
		nType	= *((UINT16*)(pQueue+sizeof(INT32)+SIZE_SHORT));

		STLMnstConnector::iterator	it;
		it	= m_pstlMnstConnector->find((INT32)nIndex);
		
		if(it != m_pstlMnstConnector->end())
		{
#ifdef TRACE_PACKET_DEBUG
			it->second.pConnector->m_bDebugDisable = m_bDebugDisable;
#endif // _DEBUG
			if(!it->second.pConnector->send(nSize, pQueue+QUEUE_SIZE_HEADER, nType))
			{
				g_SendMessage(LOG_MSG_FILELOG, "Index index%u by Queue Over\n", it->second.nIndex);
				if(!m_bServer)
					break;
				it->second.pConnector->disconnect();
				local_message(it->second.nIndex, BaseNetConnector::TYPE_OVERFLOW);
			}
		}
		m_queueInput.pop();
		PT_Free(pQueue);
	}
}

void BaseNetManager::set_onreceive(FuncOnReceive _func) 
{ 
	m_funcOnReceive = _func; 
}
void BaseNetManager::onreceive(UINT32 _nIndex) 
{ 
	m_funcOnReceive(_nIndex); 
}

void BaseNetManager::pop_output_()
{
	//CORUM_PROFILE();
	if(m_pstlVstConnector->size() == 0)
		return;

	UINT16	nRetSize;
	UINT16	nType;
	UINT32	nIndex;

	m_pReceiveBuffer = PT_Alloc(char, STATE_MAX_PACKET);
	for(UINT32 i=0; i<m_pstlVstConnector->size(); i++)
	{	
		if((*m_pstlVstConnector)[i].pConnector != NULL &&
			(*m_pstlVstConnector)[i].pConnector->connect_type() == BaseNetConnector::CT_CONNECTED)
		{
			do
			{
				nRetSize	= (*m_pstlVstConnector)[i].pConnector->recv(m_pReceiveBuffer, STATE_MAX_PACKET, &nType);
				if(nRetSize == 0 && nType == 0)
					break;
				nIndex		= (*m_pstlVstConnector)[i].pConnector->index_get();
				
#ifdef TRACE_PACKET_DEBUG
				(*m_pstlVstConnector)[i].pConnector->m_bDebugDisable = m_bDebugDisable;
#endif // _DEBUG
				bool	bIndexPacket = false;
				if(type_index_packet_check(nType))
				{// 첫번 째 패킷은 인덱스를 알려주는 패킷이다.(계정 번호)
#ifdef TRACE_PACKET_DEBUG
					if(!(*m_pstlVstConnector)[i].pConnector->m_bDebugDisable)
						g_SendMessage(LOG_MSG_CONSOLE, "Receive Index %d\n", m_nIndex);
#endif
					bIndexPacket	= true;
					//nRetSize	= 0;
				}

				if(nType == BaseNetConnector::TYPE_DISCONNECT
					|| nType == BaseNetConnector::TYPE_TIMEOUT)
				{
					(*m_pstlVstConnector)[i].pConnector->disconnect();

					switch(nType)
					{
					case BaseNetConnector::TYPE_DISCONNECT:
						g_SendMessage(LOG_MSG_FILELOG, "disconnect by session close packet. index%d\n", nIndex);
						break;
					case BaseNetConnector::TYPE_TIMEOUT:
						g_SendMessage(LOG_MSG_FILELOG, "disconnect by time out packet. index%d\n", nIndex);
						break;
					}
				}

				if(nType == BaseNetConnector::TYPE_INDEX_REQ
					|| nType == BaseNetConnector::TYPE_INDEX_RES)
				{
					ST_BaseNetConnectorExt stExt;
					stExt.nIndex = nIndex;
					stExt.nPort = (*m_pstlVstConnector)[i].nPort;
					stExt.nTimeResponse = 0;
					memcpy(stExt.anIP, (*m_pstlVstConnector)[i].anIP, 4);
					nRetSize = sizeof(ST_BaseNetConnectorExt);
					memcpy(m_pReceiveBuffer, &stExt, nRetSize);
				}

				if(nRetSize > 0 || bIndexPacket)
				{
					char *pQueue	= PT_Alloc(char, nRetSize + QUEUE_SIZE_HEADER);
					*((UINT32*)pQueue)	= nIndex;
					*((UINT16*)(pQueue+4))	= nRetSize;
					*((UINT16*)(pQueue+4+SIZE_SHORT))	= nType;//(*m_pstlVstConnector)[i].pConnector->get_size_sending();
					memcpy(pQueue + QUEUE_SIZE_HEADER, m_pReceiveBuffer, nRetSize);

					m_queueOutput.push(pQueue);

					pQueue = PT_Alloc(char, 4 * 2);
					*((UINT32*)pQueue) = nIndex;
					*((UINT32*)(pQueue + 4)) = (*m_pstlVstConnector)[i].pConnector->time_respons_avg();	// 2015.07.24
					m_queueOutDelay.push(pQueue);

					if(m_funcOnReceive)
						m_funcOnReceive((*m_pstlVstConnector)[i].pConnector->index_get());

				}
			}while(nType);
		}
	}

	PT_Free(m_pReceiveBuffer);
}

void BaseNetManager::connector_all_release()
{
	char bufPacket[PACKET_SIZE_SINGLE + sizeof(UINT32) * 2 + sizeof(UINT16) + PACKET_SIZE_SINGLE];
	*((UINT16*)(&bufPacket[s_nSizeHeader])) = BaseNetConnector::TYPE_DISCONNECT;

	for (UINT32 i = 0; i < m_pstlVstConnector->size(); i++)
	{
		send_packet(*m_pSocket, (*m_pstlVstConnector)[i].sockaddr, 2, s_nSizeHeader, bufPacket, (*m_pstlVstConnector)[i].nIndex);
	}

	for (UINT32 i = 0; i < m_pstlVstConnector->size(); i++)
	{
		if ((*m_pstlVstConnector)[i].pConnector)
		{
			PT_OFree((*m_pstlVstConnector)[i].pConnector);
		}
	}
	m_pstlVstConnector->clear();
	m_pstlMnstConnector->clear();
}

UINT16 BaseNetManager::send_packet(BaseSocket &_Socket, char *_pAddress, UINT16 _nSize, UINT16 _nSizeHeader, char *_pBuffer, UINT32 _index)
{
	*((UINT16*)&_pBuffer[0])	= _nSize;
	*((UINT32*)&_pBuffer[2])		= _index;
	*((UINT32*)&_pBuffer[6])		= BaseSystem::timeGetTime();

	return _Socket.send(_pBuffer, _nSize+_nSizeHeader, _pAddress);
}

typedef struct {
	char	ip[4];
	int		nCnt;
	int		nLastTime;
}ST_IP;
typedef std::vector<ST_IP, PT_allocator<ST_IP> >							STLVstIPs;

bool block_in(char _ip[4])
{
	bool bFind = false;
	static STLVstIPs	s_stlVBlockList;
	int nCurTime = BaseSystem::timeGetTime();
	for (unsigned int i = 0; i < s_stlVBlockList.size(); i++)
	{
		if (s_stlVBlockList[i].nLastTime + 10000 < nCurTime){
			g_SendMessage(LOG_MSG_CONSOLE, "block ip release %d.%d.%d.%d Cnt %d\n", _ip[0], _ip[1], _ip[2], _ip[3], s_stlVBlockList[i].nCnt);
			s_stlVBlockList.erase(s_stlVBlockList.begin() + i);
			i--;
			continue;
		}

		if (!memcmp(_ip, s_stlVBlockList[i].ip, 4))
		{
			if (s_stlVBlockList[i].nLastTime + 500 > nCurTime){
				s_stlVBlockList[i].nCnt++;
				s_stlVBlockList[i].nLastTime = nCurTime;
				g_SendMessage(LOG_MSG_CONSOLE, "block ip %d.%d.%d.%d Cnt %d\n", _ip[0], _ip[1], _ip[2], _ip[3], s_stlVBlockList[i].nCnt);
				if (s_stlVBlockList[i].nCnt > 5){
					g_SendMessage(LOG_MSG_CONSOLE, "block ip %d.%d.%d.%d\n", _ip[0], _ip[1], _ip[2], _ip[3]);
					return true;
				}
			}
			else{
				s_stlVBlockList[i].nCnt = 1;
				s_stlVBlockList[i].nLastTime = nCurTime;
				g_SendMessage(LOG_MSG_CONSOLE, "block ip reset %d.%d.%d.%d Cnt %d\n", _ip[0], _ip[1], _ip[2], _ip[3], s_stlVBlockList[i].nCnt);
			}
			bFind = true;
			break;
		}
	}

	if (!bFind){
		ST_IP stIP;
		memcpy(stIP.ip, _ip, 4);
		stIP.nCnt = 1;
		stIP.nLastTime = nCurTime;
		s_stlVBlockList.push_back(stIP);
		g_SendMessage(LOG_MSG_CONSOLE, "block ip create %d.%d.%d.%d\n", _ip[0], _ip[1], _ip[2], _ip[3]);
	}
	return false;
}

char *BaseNetManager::MassReceive(int _idx, char* _packet, int _size) // when finished to get, return true;
{
	STLMnMassBuffer::iterator it;
	it = m_massBuffers.find(_idx);

	if (it == m_massBuffers.end())
	{
		ST_MassBuffer buf;
		memcpy(&buf.size, _packet, sizeof(int));
		buf.buffer = PT_Alloc(char, buf.size);
		memcpy(buf.buffer, _packet + sizeof(int), _size - sizeof(int));
		buf.received = _size;
		m_massBuffers[_idx] = buf;

		if (buf.size > 0 && buf.received - sizeof(int) == buf.size)
			return buf.buffer;
	} else {
		ST_MassBuffer* buf = &it->second;
		memcpy(buf->buffer + buf->received - sizeof(int), _packet, _size);
		buf->received += _size;

		printf("MassReceived : %d\n", buf->received);
		if (buf->size > 0 && buf->received - sizeof(int) == buf->size)
			return buf->buffer;
	}

	return NULL;
}

BaseNetManager::ST_MassBuffer* BaseNetManager::MassBuffer(int _idx)
{
	return &m_massBuffers[_idx];
}

void BaseNetManager::MassReleaseAll()
{
	STLMnMassBuffer::iterator it = m_massBuffers.begin();
	for (; it != m_massBuffers.end(); it++)
	{
		PT_Free(it->second.buffer);
	}
	m_massBuffers.clear();
}

void BaseNetManager::MassRelease(int _idx)
{
	PT_Free(m_massBuffers[_idx].buffer);
	m_massBuffers.erase(_idx);
}

DEF_ThreadCallBack(BaseNetManager::update)
//void __cdecl BaseNetManager::update(void *_pManager)
{
	PT_ThreadStart(THTYPE_BASE_NETMANAGER);
	BaseNetManager *pManager	= (BaseNetManager*)_pParam;

	printf("thread in %d\n", pManager->server_port_get());

	char bufPacket[PACKET_SIZE_SINGLE+sizeof(UINT32)*2+sizeof(UINT16)+PACKET_SIZE_SINGLE];
	INT32	nSize;
	UINT32	nIndex;
	char sockaddr[16];
	STLMnstConnector::iterator	it;

	//pManager->m_pSocket->bind();

	UINT32  nSizePacketSend;
#ifdef TRACE2_PACKET_DEBUG
	UINT32  nTimedelay, nTimebefor,	nTimecur;
	UINT32 nResponsMin=0, nResponsMax=0, nResponsAvg=0;
	UINT32 nCountSendSuccess=0, nCountSendError=0, nCountRecv=0, nCountSending=0, nCountReceiving=0,
					nContAlive;
	UINT32 nCountPacketMissed=0, nCountPacketSuccess=0, nCountPacketResended=0;
	nTimebefor	= BaseSystem::timeGetTime();
#endif

	mpool_get().hold_shutdown_inc();

	bool dirty = false;
	int sleepCnt = 0;
	do{
		//CORUM_PROFILE_SUB(UPDATE);

		dirty = false;
		pManager->push_input_();
#ifdef TRACE2_PACKET_DEBUG
		UINT32	nTimeSync;
#endif
		char anIP[4];
		short nPort;
		UINT32	nTimeSender;
		INT32 nCountLimit;
		nCountLimit	= (INT32)pManager->m_pstlVstConnector->size()*3;
		while(pManager->m_pSocket != NULL
			&& (nSize = pManager->m_pSocket->receive(&bufPacket[0], PACKET_SIZE_SINGLE+s_nSizeHeader, sockaddr, anIP, &nPort)) > 0)
		{
			if (nSize > 0) {
				if (sleepCnt > 0) {
					//printf("%d+", sleepCnt);
					sleepCnt = 0;
				}
				dirty = true;
			}
			//CORUM_PROFILE_SUB(ReceiveUpdate);
			B_ASSERT(nSize-s_nSizeHeader == *((UINT16*)bufPacket));
			if(nSize-s_nSizeHeader != *((UINT16*)bufPacket))
			{
				g_SendMessage(LOG_MSG_FILELOG, "Packetsize error!!! Packet:%d-%d=%d sent:%d\n", nSize, s_nSizeHeader, nSize-s_nSizeHeader, *((UINT16*)bufPacket));
				continue;
			}
#ifdef TRACE2_PACKET_DEBUG
			nCountRecv++;
#endif
#ifdef TRACE_PACKET_DEBUG
			if(!pManager->m_bDebugDisable)
				g_SendMessage(LOG_MSG_CONSOLE, "UDP Receive data size %d\n", nSize);
#endif
			nIndex		= *((UINT32*)&bufPacket[2]);
			nTimeSender	= *((UINT32*)&bufPacket[6]);
			int nType = ((BaseNetConnector::ST_NetPacketPice*)&bufPacket[s_nSizeHeader])->nType;

			if (sm_bTracePacket){
				int anIp[4];
				for (int k = 0; k < 4; k++)
					if (anIP[k]>=0)	anIp[k] = anIP[k];
					else anIp[k] = 256 + anIP[k];

				char strBuf[255];
				strcpy_s(strBuf, 255, "UNKNOW");
				switch (nType){
				case BaseNetConnector::TYPE_INDEX_REQ:strcpy_s(strBuf, 255, "TYPE_INDEX_REQ");	break;
				case BaseNetConnector::TYPE_INDEX_RES:strcpy_s(strBuf, 255, "TYPE_INDEX_RES");	break;
				case BaseNetConnector::TYPE_ODERED:strcpy_s(strBuf, 255, "TYPE_ODERED");	break;// as send this data, record to send and then check to received or not by remote.
				case BaseNetConnector::TYPE_REALTIME:strcpy_s(strBuf, 255, "TYPE_REALTIME");	break;// if fail to send, don't send data again, also don't record to send.
				case BaseNetConnector::TYPE_PING_REQ:strcpy_s(strBuf, 255, "TYPE_PING_REQ");	break;
				case BaseNetConnector::TYPE_PING_RES:strcpy_s(strBuf, 255, "TYPE_PING_RES");	break;
				case BaseNetConnector::TYPE_SUCCESS:strcpy_s(strBuf, 255, "TYPE_SUCCESS");	break;
				case BaseNetConnector::TYPE_MISSED:strcpy_s(strBuf, 255, "TYPE_MISSED");	break;
				case BaseNetConnector::TYPE_DISCONNECT:strcpy_s(strBuf, 255, "TYPE_DISCONNECT");	break;
				case BaseNetConnector::TYPE_TIMEOUT:strcpy_s(strBuf, 255, "TYPE_TIMEOUT");	break;
				case BaseNetConnector::TYPE_OVERFLOW:strcpy_s(strBuf, 255, "TYPE_OVERFLOW");	break;
				case BaseNetConnector::TYPE_BANDWIDTH:strcpy_s(strBuf, 255, "TYPE_BANDWIDTH");	break;
				case BaseNetConnector::TYPE_RESET:strcpy_s(strBuf, 255, "TYPE_RESET");	break;
				}
				g_SendMessage(LOG_MSG_CONSOLE, "havy recv log: %d.%d.%d.%d index:%d type:%s\n", anIp[0], anIp[1], anIp[2], anIp[3], nIndex, strBuf);
			}
			it	= pManager->m_pstlMnstConnector->find(nIndex);
			if(it == pManager->m_pstlMnstConnector->end()
				&& nType == BaseNetConnector::TYPE_INDEX_REQ)
			{// 새로운 리모트가 들어왔다.
				UINT32 nIndexNew = pManager->index_get_new_(&bufPacket[s_nSizeHeader], nSize-s_nSizeHeader, sockaddr, anIP, nPort);

				it	= pManager->m_pstlMnstConnector->find((INT32)nIndexNew);
				//g_SendMessage(LOG_MSG_CONSOLE, "Req: Port:%d, index: %d", nPort, nIndexNew);
				printf("Req: Port:%d, index: %d\n", nPort, nIndexNew);
			}

			if(it == pManager->m_pstlMnstConnector->end())
			{
				if (pManager->client_index_set(nIndex))
					it = pManager->m_pstlMnstConnector->find(nIndex);
				else {
					if (((BaseNetConnector::ST_NetPacketPice*) & bufPacket[s_nSizeHeader])->nType != BaseNetConnector::TYPE_DISCONNECT
						&& !block_in(anIP))
						pManager->send_disconnect(sockaddr, 0);
					continue;
				}
			}

			if(it->second.pConnector == NULL)
			{
				pManager->m_pstlMnstConnector->erase(it);
				continue;
			}
			else {
				if (memcmp(&(it->second.sockaddr[0]), &(sockaddr[0]), 16) != 0) // if client socket is changed(in case of mobile, ip address can changable.), update client information.
					// It also can applied to a server case. Usually ip for a server is not changed.
				{
					// printf("+_+_+_+_+_+_+_ socket is changed to ip %d, %d, %d, %d\n", (int)anIP[0], (int)anIP[1], (int)anIP[2], (int)anIP[3]);
					memcpy(&it->second.sockaddr[0], sockaddr, 16);
					memcpy(it->second.anIP, anIP, 4);
					it->second.nPort = nPort;
					it->second.pConnector->set_ip(anIP);

					for (UINT32 i = 0; i < pManager->m_pstlVstConnector->size(); i++)
					{
						if ((*pManager->m_pstlVstConnector)[i].pConnector == it->second.pConnector) // Map also should be updated.
						{
							memcpy(&(*pManager->m_pstlVstConnector)[i].sockaddr[0], sockaddr, 16);
							memcpy((*pManager->m_pstlVstConnector)[i].anIP, anIP, 4);
							(*pManager->m_pstlVstConnector)[i].nPort = nPort;
							break;
						}
					}
				}
			}

			// 일상 적인 패킷 업데이트를 진행 한다.
#ifdef TRACE_PACKET_DEBUG
			if(!pManager->m_bDebugDisable)
				g_SendMessage(LOG_MSG_CONSOLE, "%d.", it->second.nIndex);
#endif
			if(it->second.pConnector->connect_type() == BaseNetConnector::CT_DISCONNECTED
				&& it->second.pConnector->is_useto_receivepacket())
			{
				//pManager->send_disconnect(it->second.sockaddr);
				continue;
			}

			if(it->second.pConnector->is_hold_receive())
				continue;

			nSize = it->second.pConnector->update(&bufPacket[s_nSizeHeader], nSize-s_nSizeHeader);
			if(!pManager->m_bServer)
			{
				it->second.nIndex	= it->second.pConnector->index_get();
				pManager->m_nIndex	= it->second.pConnector->index_get();
			}
			if (nSize > 0)
			{
				if (sleepCnt > 0) {
					//printf("%d_", sleepCnt);
					sleepCnt = 0;
				}
				dirty = true;
#ifdef TRACE_PACKET_DEBUG
				if(!pManager->m_bDebugDisable)
					g_SendMessage(LOG_MSG_CONSOLE, "UDP Send 1 data size %d, index %d\n", nSize+s_nSizeHeader, it->second.nIndex);
#endif
				nSizePacketSend	= pManager->send_packet(*pManager->m_pSocket, it->second.sockaddr, nSize, s_nSizeHeader, bufPacket, it->second.nIndex);
				if (sm_bTracePacket){
					int anIp[4];
					for (int k = 0; k < 4; k++)
						if (anIP[k] >= 0)	anIp[k] = anIP[k];
						else anIp[k] = 256 + anIP[k];

					g_SendMessage(LOG_MSG_CONSOLE, "havy send log: %d.%d.%d.%d\n", anIp[0], anIp[1], anIp[2], anIp[3]);
				}
#ifdef TRACE2_PACKET_DEBUG
				if((INT32)nSizePacketSend > 0)
					nCountSendSuccess++;
				else
					nCountSendError++;
#endif
			}else{
				continue;
			}
			
			pManager->m_nTimeDiffer = it->second.pConnector->timediffer_get_remote();
#ifdef TRACE_PACKET_DEBUG
		/*
			if(!pManager->m_bDebugDisable)
			{
				if(pManager->m_bServer)
					g_SendMessage(LOG_MSG, "Time= %d\n", BaseSystem::timeGetTime());
				else
					g_SendMessage(LOG_MSG, "Time= %d\n", pManager->time_get_remote());

			}*/
#endif

			nCountLimit--;
			if(nCountLimit <= 0)
				break;
		}

#ifdef TRACE2_PACKET_DEBUG
		nResponsAvg	= 0;
		nContAlive	= 0;

		nCountSending		= 0;
		nCountReceiving		= 0;
#endif
		bool bSendingPacketExist = false;
		for (UINT32 i = 0; i < pManager->m_pstlVstConnector->size(); i++)
		{
			//CORUM_PROFILE_SUB(ForLoop);
			if ((*pManager->m_pstlVstConnector)[i].pConnector == NULL
				|| (*pManager->m_pstlVstConnector)[i].pConnector->connect_type() >= BaseNetConnector::CT_DISCONNECTED
				)
			{
				continue;
			}
			// without receive packet, process sending packet
			nSize = (*pManager->m_pstlVstConnector)[i].pConnector->update(&bufPacket[s_nSizeHeader], 1);
			if (nSize > 0)
			{
				//printf("%d^", sleepCnt);
				sleepCnt = 0;

				dirty = true;
#ifdef TRACE_PACKET_DEBUG
				if(!pManager->m_bDebugDisable)
				{
					g_SendMessage(LOG_MSG_CONSOLE, "%d", (*pManager->m_pstlVstConnector)[i].nIndex);
					g_SendMessage(LOG_MSG_CONSOLE, "UDP Send 2 data size %d, index %d\n", nSize+s_nSizeHeader, (*pManager->m_pstlVstConnector)[i].nIndex);
				}
#endif
				nSizePacketSend	= pManager->send_packet(*pManager->m_pSocket, (*pManager->m_pstlVstConnector)[i].sockaddr,
					nSize, s_nSizeHeader, bufPacket, (*pManager->m_pstlVstConnector)[i].nIndex);
				if (sm_bTracePacket){
					int anIp[4];
					for (int k = 0; k < 4; k++)
						if ((*pManager->m_pstlVstConnector)[i].anIP[k] >= 0)	anIp[k] = (*pManager->m_pstlVstConnector)[i].anIP[k];
						else anIp[k] = 256 + (*pManager->m_pstlVstConnector)[i].anIP[k];

					g_SendMessage(LOG_MSG_CONSOLE, "havy snd log: %d.%d.%d.%d index:%d\n", anIp[0], anIp[1], anIp[2], anIp[3], (*pManager->m_pstlVstConnector)[i].nIndex);
				}
#ifdef TRACE2_PACKET_DEBUG
				if((INT32)nSizePacketSend > 0)
					nCountSendSuccess++;
				else
					nCountSendError++;
#endif
			}

			if((*pManager->m_pstlVstConnector)[i].pConnector->is_sending())
				bSendingPacketExist = true;

#ifdef TRACE2_PACKET_DEBUG
			{
				nContAlive++;
				UINT32 nMin, nMax, nAvg;
				pManager->(*m_pstlVstConnector)[i].pConnector->respons_get_times(&nMin, &nMax, &nAvg);
				if(nResponsMin == 0 || nResponsMin > nMin)
					nResponsMin	= nMin;
				if(nResponsMax == 0 || nResponsMax == -1 ||
					(nResponsMax < nMax && nMax < 10000))
					nResponsMax	= nMax;
				nResponsAvg	+= nAvg;

				UINT32 nMiss, nSuccess, nResend;
				UINT32 nSending, nReceiving;
				pManager->(*m_pstlVstConnector)[i].pConnector->statistics(&nMiss, &nResend, &nSuccess, &nSending, &nReceiving);
				nCountPacketMissed		+= nMiss;
				nCountPacketResended	+= nResend;
				nCountPacketSuccess		+= nSuccess;

				nCountSending		+= nSending;
				nCountReceiving		+= nReceiving;

				nTimeSync	= pManager->(*m_pstlVstConnector)[i].pConnector->time_get_remote();
			}
#endif
		}

		if(bSendingPacketExist == false
			&& pManager->is_process_byself())
		{
			pManager->release_thread();
		}

		pManager->pop_output_();
		pManager->local_event_process();
		
#ifdef TRACE2_PACKET_DEBUG
		nTimecur	= BaseSystem::timeGetTime();
		nTimedelay	= nTimecur - nTimebefor;
		if(nTimedelay >= 5000 && nCountRecv > 0 && nCountSendSuccess > 0)
		{
			if(nContAlive > 0)
				nResponsAvg	/= nContAlive;
			nTimebefor	= nTimecur;

			if(pManager->m_bServer)
				nTimeSync	= BaseSystem::timeGetTime();

			g_SendMessage(LOG_MSG, "L:%4d E:%1d %4d(%4d) M%2d R%4d/%4d (S%3d/%3d) Time %u\n"
				, nContAlive
				, nCountSendError, nResponsAvg, nResponsMax
				, nCountPacketMissed, nCountPacketResended, nCountPacketSuccess
				, nCountSending, nCountReceiving, nTimeSync
				);

			nCountRecv			= 0;
			nCountSendSuccess	= 0;
			nCountSendError		= 0;

			nResponsMax	= 0;
			nResponsMin	= 0;

			nCountPacketSuccess		= 0;
			nCountPacketMissed		= 0;
			nCountPacketResended	= 0;

			nCountSending	= 0;
			nCountReceiving	= 0;

			//PT_MemDisplay();
			//DisplayLeak();
		}
#endif
		static UINT32 lastDirty = 0;
		if (dirty)
			lastDirty = BaseSystem::timeGetTime();

		UINT32 cur = BaseSystem::timeGetTime();
		if (cur - lastDirty > 50) // when serializing packets arrived, they has some delay. so if you try to receive the packets without delay, it will not return while some microseconds.
		{
			BaseSystem::Sleep(5);
			sleepCnt++;
		}
		else {
			std::chrono::microseconds sleep_duration(10);
			std::this_thread::sleep_for(sleep_duration);
		}
		
	}while(!pManager->m_bEndThread && !mpool_get().is_terminated());

	//CORUM_PROFILE_REPORT(NULL);
	// 로컬이 종료 되면서 리모트에 접속 종료를 알린다.
	pManager->connector_all_release();
	
	PT_OFreeS(pManager->m_pstlMnstConnector);
	PT_OFreeS(pManager->m_pstlVstConnector);
	pManager->m_pstlMnstConnector = NULL;
	pManager->m_pstlVstConnector = NULL;

	pManager->m_bBeginThread	= false;

	PT_ThreadEnd(THTYPE_BASE_NETMANAGER);
	if(pManager->is_release_thread())// refer #1610 end by release_thread() function.
		PT_OFree(pManager);

	mpool_get().hold_shutdown_dec();
    BaseSystem::endthread();
	//_endthread();
    DEF_ThreadReturn;
}

void BaseNetManager::process_byself()
{
	m_bProcessBySelf = true;
}

void BaseNetManager::release_thread() 
{ 
	m_bReleaseThread = true; // when the update function(thread) finished, this class will destoried by self. refer #1602
	end();
}

void BaseNetManager::timeout_set(UINT32 _nTimeout)
{	
	ST_EventExt *pEvent = PT_Alloc(ST_EventExt,1);
	pEvent->nEvent = BaseNetConnector::TYPE_TIMEOUT;
	pEvent->nIndex = _nTimeout;
	m_nTimeOut = _nTimeout;
	m_queueEvent.push(pEvent);
}

void BaseNetManager::bandwidth_set(UINT32 _nBandwidth)
{
	m_nBandwidth = _nBandwidth;

	ST_EventExt *pEvent = PT_Alloc(ST_EventExt,1);
	pEvent->nEvent = BaseNetConnector::TYPE_BANDWIDTH;
	pEvent->nIndex = _nBandwidth;
	m_queueEvent.push(pEvent);
}

bool BaseNetManager::is_connect(UINT32 _nIndex)
{
	if(m_pstlMnstConnectorExt == NULL)
		return false;

	STLMnstConnectorExt::iterator	it;

	it	= m_pstlMnstConnectorExt->find(_nIndex);

	if(it == m_pstlMnstConnectorExt->end())
		return false;
	
	return true;
}

char * BaseNetManager::ip_get(char _anIp[4], char * _strIpOut)
{
	int anIp[4];
	for (int k = 0; k < 4; k++)
	{
		if (_anIp[k] >= 0)	anIp[k] = _anIp[k];
		else anIp[k] = 256 + _anIp[k];
	}

	sprintf_s(_strIpOut, 255, "%d.%d.%d.%d", anIp[0], anIp[1], anIp[2], anIp[3]);
	return _strIpOut;
}

void BaseNetManager::print_all_connection()
{
	STLVstConnectorExt::iterator	it;

	it = m_pstlVstConnectorExt->begin();
	for (; it != m_pstlVstConnectorExt->end(); it++)
	{
		char strBuf[255];
		
		g_SendMessage(LOG_MSG_CONSOLE, "live socket index: %d, response: %d(%s)\n", it->nIndex, it->nTimeResponse, ip_get(it->anIP, strBuf));
	}

	STLVstConnector::const_iterator it2;
	it2 = m_pstlVstConnector->begin();
	for (; it2 != m_pstlVstConnector->end(); it2++)
	{
		if (it2->pConnector)
		{
			g_SendMessage(LOG_MSG_CONSOLE, "Connector %d, type: %d\n", it2->nIndex, it2->pConnector->connect_type());

			if (it2->pConnector->get_idle() > 1000 * 60 * 30)// if over than 30 minute
			{
				g_SendMessage(LOG_MSG_CONSOLE, "Remove index %d\n", it2->nIndex);
				release_index(it2->nIndex);
			}
		}
	}
}

void BaseNetManager::release_index(UINT32 _nIndex)
{
	if(m_pstlMnstConnectorExt == NULL)
		return;

	MassRelease(_nIndex);

	STLMnstConnectorExt::iterator	it;

	it	= m_pstlMnstConnectorExt->find(_nIndex);

	if(it == m_pstlMnstConnectorExt->end())
		return;

	ST_EventExt *pEvent = PT_Alloc(ST_EventExt,1);
	pEvent->nEvent = BaseNetConnector::TYPE_DISCONNECT;
	pEvent->nIndex = _nIndex;
	m_queueEvent.push(pEvent);

	for (UINT32 i = 0; i<m_pstlVstConnectorExt->size(); i++)
	{
		if ((*m_pstlVstConnectorExt)[i].nIndex == _nIndex)
		{
			m_pstlVstConnectorExt->erase(m_pstlVstConnectorExt->begin() + i);
			break;
		}
	}
	m_pstlMnstConnectorExt->erase(it);
}

UINT32 BaseNetManager::get_time_response(UINT32 _nIndex)
{
	if (m_pstlMnstConnectorExt == NULL)
		return 0;

	STLMnstConnectorExt::iterator	it;

	it = m_pstlMnstConnectorExt->find(_nIndex);

	if (it == m_pstlMnstConnectorExt->end())
		return 0;

	return it->second.nTimeResponse;
}

bool BaseNetManager::get_connector_address(UINT32 _nIndex, char *_strIP, UINT32 _nSize)
{
	bbyte anIp[4];
	UINT16 nPort;

	if (get_connector_address(_nIndex, anIp, &nPort))
	{
		unsigned int nIp[4];
		for (int i = 0; i < 4; i++)
			nIp[i] = (unsigned int)anIp[i];
		sprintf_s(_strIP, _nSize, "%u.%u.%u.%u", nIp[0], nIp[1], nIp[2], nIp[3]);
		return true;
	}
	return false;
}

bool BaseNetManager::get_connector_address(UINT32 _nIndex, bbyte *_anIP4, UINT16 *_pnPort)
{	
	if(m_pstlMnstConnectorExt == NULL)
		return false;

	STLMnstConnectorExt::iterator	it;

	it	= m_pstlMnstConnectorExt->find(_nIndex);

	if(it == m_pstlMnstConnectorExt->end())
		return false;

	memcpy(_anIP4, it->second.anIP, 4);
	*_pnPort = it->second.nPort;
	return true;
}

UINT32 BaseNetManager::time_get_remote()
{
	if(m_pstlMnstConnector == NULL)
		return 0;

	STLMnstConnector::iterator	it;

	it	= m_pstlMnstConnector->find(0);

	if(it == m_pstlMnstConnector->end())
		return 0;
#ifdef TRACE_PACKET_DEBUG
	if(!m_bDebugDisable)
	{
		g_SendMessage(LOG_MSG_CONSOLE, "Differ %d, %d\n", m_nTimeDiffer, BaseSystem::timeGetTime());
	}
#endif
	return m_nTimeDiffer + BaseSystem::timeGetTime();
}

size_t	BaseNetManager::size()// return number of client index;
{
	if(m_pstlMnstConnectorExt == NULL)
		return 0;

	return m_pstlVstConnectorExt->size();
}

UINT16 BaseNetManager::recv(UINT32 *_pnIndex, char *_pData, UINT16 _nSizeBuf, UINT16 *_pnType)
{
	char *pQueue	= (char*)m_queueOutput.pop();
	if(pQueue == NULL)
	{
		if(_pnType)
			*_pnType = 0;

		if (m_pSocket != NULL 
			&& !m_pSocket->is_connected() 
			&& m_pSocket->is_server()
			&& m_pSocket->socket_ready())
		{
			if (m_pSocket)
				PT_OFree(m_pSocket);
			m_pSocket = NULL;

			if (_pnType)
				*_pnType = BaseNetConnector::TYPE_FAILTOCREATE;
		}
		return 0;
	}
	if (m_pstlMnstConnectorExt == NULL) {
		//printf("BaseNetManager::recv NULL m_pstlMnstConnnectorExt.\n");
		if (_pnType)
			*_pnType = 0;

		PT_Free(pQueue);

		return 0;
	}
	
	*_pnIndex	= *((UINT32*)pQueue);
	UINT16 nRetSize;
	nRetSize	= *((UINT16*)(pQueue+sizeof(INT32)));

	B_ASSERT(nRetSize <= _nSizeBuf);
	memcpy(_pData, pQueue+QUEUE_SIZE_HEADER, nRetSize);

	UINT16 nType = *((UINT16*)(pQueue+sizeof(INT32)+SIZE_SHORT)); 
	if(_pnType)
		*_pnType	= nType;

	if(type_index_packet_check(nType))
		nRetSize = 0;

	switch(nType)
	{
	case BaseNetConnector::TYPE_DISCONNECT:
	case BaseNetConnector::TYPE_OVERFLOW:
	case BaseNetConnector::TYPE_TIMEOUT:
		{
			STLMnstConnectorExt::iterator it;
			it = m_pstlMnstConnectorExt->find(*_pnIndex);
			if(it != m_pstlMnstConnectorExt->end())
			{
				for(UINT32 i=0; i<m_pstlVstConnectorExt->size(); i++)
				{
					if((*m_pstlVstConnectorExt)[i].nIndex == *_pnIndex)
					{
						m_pstlVstConnectorExt->erase(m_pstlVstConnectorExt->begin()+i);
						break;
					}
				}
				m_pstlMnstConnectorExt->erase(it);
			}else{
				if(_pnType)
					*_pnType = 0;
			}
		}
		break;
	case BaseNetConnector::TYPE_INDEX_REQ:
	case BaseNetConnector::TYPE_INDEX_RES:
		{
			ST_BaseNetConnectorExt stExt;
			memcpy(&stExt, _pData, sizeof(ST_BaseNetConnectorExt));
			(*m_pstlMnstConnectorExt)[*_pnIndex] = stExt;
			m_pstlVstConnectorExt->push_back(stExt);
		}
		break;
	}
	
	PT_Free(pQueue);

	pQueue = (char*)m_queueOutDelay.pop();
	if (pQueue != NULL)
	{
		UINT32 nIndex;
		nIndex = *((UINT32*)pQueue);

		STLMnstConnectorExt::iterator it;
		it = m_pstlMnstConnectorExt->find(nIndex);
		if (it != m_pstlMnstConnectorExt->end())
			it->second.nTimeResponse = *((UINT32*)(pQueue + sizeof(INT32)));
		
		PT_Free(pQueue);
	}
	return nRetSize;
}

void BaseNetManager::end()
{
	m_bEndThread	= true;
}

bool BaseNetManager::send(UINT32 _nIndex, char *_pData, UINT16 _nSize, UINT16 _nType)
{
#ifdef _DEBUG
	//printf("~");
#endif
	char *pQueue	= PT_Alloc(char, _nSize + QUEUE_SIZE_HEADER);
	*((UINT32*)pQueue)	= _nIndex;
	*((UINT16*)(pQueue+sizeof(INT32)))	= _nSize;
	*((UINT16*)(pQueue+sizeof(INT32)+SIZE_SHORT))	= _nType;
	memcpy(pQueue + QUEUE_SIZE_HEADER, _pData, _nSize);

	if(!m_queueInput.push(pQueue))
	{
		PT_Free(pQueue);
		return false;
	}

	return true;
}

void BaseNetManager::weakup() {
	if (m_pSocket == NULL)
		return;

	m_pSocket->weakup();
}

bool BaseNetManager::broadcast(char *_pData, UINT16 _nSize, UINT16 _nType)
{
	if(m_pstlMnstConnectorExt == NULL)
		return false;

	for(UINT32 i=0; i<m_pstlVstConnectorExt->size(); i++)
	{
		char *pQueue	= PT_Alloc(char, _nSize + QUEUE_SIZE_HEADER);
		*((UINT32*)pQueue)	= (*m_pstlVstConnectorExt)[i].nIndex;
		*((UINT16*)(pQueue+sizeof(INT32)))	= _nSize;
		*((UINT16*)(pQueue+sizeof(INT32)+SIZE_SHORT))	= _nType;
		memcpy(pQueue + QUEUE_SIZE_HEADER, _pData, _nSize);

		if(!m_queueInput.push(pQueue))
		{
			PT_Free(pQueue);
			return false;
		}
	}

	return true;
}

void BaseNetManager::send_disconnect(char *_sockaddr, UINT32 _index)
{
	char bufPacket[s_nSizeHeader+2];
	*((unsigned short*)(&bufPacket[s_nSizeHeader])) = BaseNetConnector::TYPE_DISCONNECT;
	send_packet(*m_pSocket, _sockaddr, 2, s_nSizeHeader, bufPacket, _index);
}

void BaseNetManager::local_event_process()
{
	ST_EventExt	*pEvent;
	do{
		pEvent = (ST_EventExt*)m_queueEvent.top();
		if(pEvent == NULL)
			break;

		switch(pEvent->nEvent)
		{
		case BaseNetConnector::TYPE_TIMEOUT:
			for(UINT32 i=0; i<m_pstlVstConnector->size(); i++)
			{
				if((*m_pstlVstConnector)[i].pConnector)
					(*m_pstlVstConnector)[i].pConnector->timeout_set(pEvent->nIndex); // index is timeout when receive TYPE_TIMEOUT
			}	
			break;
		case BaseNetConnector::TYPE_BANDWIDTH:
			for(UINT32 i=0; i<m_pstlVstConnector->size(); i++)
			{
				if((*m_pstlVstConnector)[i].pConnector)
					(*m_pstlVstConnector)[i].pConnector->bandwidth_set(m_nBandwidth); // index is timeout when receive TYPE_TIMEOUT
			}	
			break;
		case BaseNetConnector::TYPE_DISCONNECT:
			{
				STLMnstConnector::iterator it;
				it = m_pstlMnstConnector->find(pEvent->nIndex);

				if(it != m_pstlMnstConnector->end())
				{
					send_disconnect(it->second.sockaddr, it->second.nIndex);
					it->second.pConnector->disconnect();
				}
			}
			break;
		}
		m_queueEvent.pop();
		PT_Free(pEvent);
	}while(pEvent);

	for(UINT32 i=0; i<m_pstlVstConnector->size(); i++)
	{
		if((*m_pstlVstConnector)[i].pConnector && 
			(
				(*m_pstlVstConnector)[i].pConnector->connect_type() >= BaseNetConnector::CT_DISCONNECTED
				|| ((*m_pstlVstConnector)[i].pConnector->connect_type() != BaseNetConnector::CT_CONNECTED &&
					(*m_pstlVstConnector)[i].pConnector->time_from_create() > 1000 * 60)
			)
			)
		{
			PT_OFree((*m_pstlVstConnector)[i].pConnector);
			(*m_pstlVstConnector)[i].pConnector = NULL;
					
			STLMnstConnector::iterator it;
			it = m_pstlMnstConnector->find((*m_pstlVstConnector)[i].nIndex);
			m_pstlMnstConnector->erase(it);
		}
	}
}

UINT32 BaseNetConnector::time_from_create()
{
	return BaseSystem::timeGetTime() - m_nTimeCreated;
}

bool BaseNetConnector::is_serial_packet(UINT16 _nType)
{
	if(_nType == TYPE_ODERED || _nType == TYPE_INDEX_REQ || _nType == TYPE_INDEX_RES)
	//if(_nType == TYPE_ODERED)
		return true;
	return false;
} 

#include "BaseState.h"
#include "BaseStateSpace.h"
#include "BaseStateManager.h"
#include "BaseFile.h"

#undef STDEF_MANAGERNAME
#define STDEF_MANAGERNAME	BaseStateManager

int fnUpdateMessageReceiver(BaseDStructureValue *_pdsvContext, BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvEvent, void*_pVoidData, UINT32 _nTimeDelta)
{
	BaseNetManager *pNetManager = NULL;
	pNetManager = (BaseNetManager*)_pVoidData;
	
	STDEF_Manager(pManager);
	STDEF_BaseState(pState);
	
	UINT32	nIndex;
	char	*pData = PT_Alloc(char, STATE_MAX_PACKET);

	unsigned short	nSize;
	unsigned short nType;
	do
	{
		int nKey = 0;
		nType = 0;
		nSize = pNetManager->recv(&nIndex, pData, STATE_MAX_PACKET, &nType);

		STLMnInt	stlMnGroupId;
		stlMnGroupId[HASH_STATE(Server)] = pNetManager->managerid_get();
		stlMnGroupId[HASH_STATE(Socket)] = nIndex;
		if (nSize > 4)
		{
			if (pNetManager->MassReceive(nIndex, pData, nSize) != NULL)
			{
				BaseNetManager::ST_MassBuffer* buf = pNetManager->MassBuffer(nIndex);
				BaseDStructureValue* pdsvEvent = NULL;
				PT_OAlloc2(pdsvEvent, BaseDStructureValue, pManager->EnumGet(HASH_STATE(HASH_EnumEvent)), 1024);

				if (pdsvEvent->set_dumppacket(pManager, buf->buffer, buf->size, true)) {
#ifdef _DEBUG
					int nKeyState = HASH_STATE(BaseStateEventGlobal);
					const int *event = NULL;
					if(pdsvEvent->get(nKeyState, (const void**)&event))
					{
						int x = *event;
						x++;
						g_SendMessage(LOG_MSG, "receive event: %d\n", *event);
					}
#endif
					BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
					pManager->post_systemevent(pdsvEvent);
				}
				else {
					PT_OFree(pdsvEvent);
					printf("-------------------------------------Packet event doesn't matching value type\n");
				}
				pNetManager->MassRelease(nIndex);
			}
#ifdef _DEBUG
			else {
				g_SendMessage(LOG_MSG, "+");
			}
#endif
		}
		else if (nType == BaseNetConnector::TYPE_INDEX_REQ
			|| nType == BaseNetConnector::TYPE_INDEX_RES)
		{
			BaseDStructureValue	*pdsvEvent = pManager->make_event_state(STRTOHASH("NetSocketOpen"));
			BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionNextIdentifier), stlMnGroupId);// The state what will be added is going to got this identifier group.

			int nTarget = pState->obj_serial_get();
			pdsvEvent->set_alloc(HASH_STATE(BaseEventTargetStateSerial), &nTarget);

			//printf("Opensocket server:%d id:%d\n", pNetManager->managerid_get(), (int)nIndex);
			char strIPAddress[256];
			if (pNetManager->get_connector_address(nIndex, strIPAddress, 256)) {
				pManager->variable_define(STRTOHASH("BaseNetClientIP"), TYPE_STRING, true);
				pdsvEvent->set_alloc(STRTOHASH("BaseNetClientIP"), strIPAddress);
			}
			pManager->post_systemevent(pdsvEvent);
		}
		else if (nType == BaseNetConnector::TYPE_FAILTOCREATE
			)
		{
			BaseDStructureValue	*pdsvEvent = pManager->make_event_state(STRTOHASH("NetSocketFail"));
			int nTarget = pState->obj_serial_get();
			pdsvEvent->set_alloc(HASH_STATE(BaseEventTargetStateSerial), &nTarget);

			//printf("fail to create server:%d id:%d\n", pNetManager->managerid_get(), (int)nIndex);
			pManager->post_systemevent(pdsvEvent);
		}
		else if (nType == BaseNetConnector::TYPE_OVERFLOW
			|| nType == BaseNetConnector::TYPE_TIMEOUT
			)
		{	
			BaseDStructureValue	*pdsvEvent = pManager->make_event_state(STRTOHASH("NetSocketClose"));
			BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
			
			//printf("close server:%d id:%d\n", pNetManager->managerid_get(), (int)nIndex);
			pManager->post_systemevent(pdsvEvent);
		}
		else if (nType == BaseNetConnector::TYPE_DISCONNECT)
		{
			BaseDStructureValue	*pdsvEvent = pManager->make_event_state(STRTOHASH("NetSocketDisconnected"));
			BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);

			//printf("disconnect server:%d id:%d\n", pNetManager->managerid_get(), (int)nIndex);
			pManager->post_systemevent(pdsvEvent);
		}
	} while (nSize || nType);
	PT_Free(pData);
	return 1;
}

STDEF_FUNC(BaseNetListenerRelease_strF)
{
    //printf("Release called\n");
	const char* strServerInfo;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetListenerRelease_strF, strServerInfo);
	char strBuf[4096];
	strcpy_s(strBuf, 4096, strServerInfo);
	BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strBuf, 4096);

	BaseNetManager* pNetManager;

	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvVariable = pState->variable_get();// _pdsvContext;
	int nHashDB = STRTOHASH("BaseNetManager");

	STDEF_Manager(pManager);
    pNetManager = (BaseNetManager*)pdsvVariable->get_point(nHashDB);
	if (pNetManager == NULL || !pNetManager->isok())
	{
		g_SendMessage(LOG_MSG, "fail to find BaseNetManager");
		return 0;
	}

	pdsvVariable->set_alloc(nHashDB, NULL);
	pManager->net_manager_delete(pNetManager);
    pNetManager->release_thread();// It will delete pNetManager in thread

	g_SendMessage(LOG_MSG, "BaseNetManager_End = %x", pNetManager);
	// release socket
	return 1;
}


STDEF_FUNC(BaseNetListenerStart_strF)
{
	const char *strServerInfo;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetListenerStart_strF, strServerInfo);
	char strBuf[4096];
	strcpy_s(strBuf, 4096, strServerInfo);
	BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strBuf, 4096);

	BaseNetManager	*pNetManager;

	STDEF_BaseState(pState);
	BaseDStructureValue *pdsvVariable = pState->variable_get();// _pdsvContext;
	int nHashDB = STRTOHASH("BaseNetManager");

	STDEF_Manager(pManager);
	BaseFile paser;
	paser.OpenFile((void*)strBuf, (int)strlen(strBuf));
	paser.set_asc_seperator(" ,");
	paser.set_asc_deletor(" ,");

	int index_start = 919100;
	int nPort = 19101;
	char strServer[255];
	paser.read_asc_line();
	if (!paser.read_asc_string(strServer, 255))
		return 0;
	if (!paser.read_asc_integer(&nPort))
		return 0;
	if (!paser.read_asc_integer(&index_start))
		index_start = 919200;

	PT_OAlloc4(pNetManager, BaseNetManager, true, strServer, nPort, index_start);
	pManager->net_manager_add(pNetManager);
	pdsvVariable->get_base()->add_column(0, "BaseNetManager", TYPE_INT64);
    pdsvVariable->set_point(nHashDB, pNetManager);
    _pdsvContext->set_point(nHashDB, pNetManager);
    //pdsvVariable->set_alloc(nHashDB, (const void*)&pNetManager);
	//_pdsvContext->set_alloc(nHashDB, (const void*)&pNetManager);
	pState->UpdateFuncRegist(_pdsvEvent, pNetManager, fnUpdateMessageReceiver);

	g_SendMessage(LOG_MSG, "BaseNetManager_ListenStart = %x", pNetManager);
	
	return 1;
}

STDEF_FUNC(BaseNetSocketClose_nF)
{
	BaseNetManager	*pNetManager = NULL;
	int nHashNetManager = STRTOHASH("BaseNetManager");
	BaseDStructureValue *pdsvVariable = _pdsvContext;

	STDEF_BaseState(pState);
	int nIndex = pState->identifier_get(HASH_STATE(Socket));

	if (nIndex > 0)
	{
        pNetManager = (BaseNetManager*)pdsvVariable->get_point(nHashNetManager);
		if (!pNetManager || !pNetManager->isok()) {
			g_SendMessage(LOG_MSG, "BaseNetManager_SocketCloseFail = %d", nIndex);
			return 0;
		}

		pNetManager->release_index(nIndex);
		g_SendMessage(LOG_MSG, "BaseNetManager_SocketClose = %x,%d", pNetManager, nIndex);
		return 1;
	}
	return 0;
}

STDEF_FUNC(BaseNetSocketChange_nF)
{
	const int *pnHashVariable;

	STDEF_GETLOCAL_R(_pdsvBase, BaseNetSocketChange_nF, pnHashVariable);
	
	BaseNetManager	*pNetManager = NULL;
	int nHashNetManager = STRTOHASH("BaseNetManager");
	BaseDStructureValue *pdsvVariable = _pdsvContext;

	STDEF_BaseState(pState);
	int nIndex = pState->identifier_get(HASH_STATE(Socket));

	g_SendMessage(LOG_MSG, "BaseNetManager_SocketChange = %d", nIndex);
	const int *pnId;	
	if (_pdsvEvent->get(*pnHashVariable, (const void **)&pnId))
	{
        pNetManager = (BaseNetManager*)pdsvVariable->get_point(nHashNetManager);
		if (nIndex > 0 && pNetManager)
		{
			if (pNetManager->isok())
				return 0;
			pState->group_id_add(HASH_STATE(Socket), *pnId);
			pNetManager->release_index(nIndex);
			return 1;
		}
	}
	return 0;
}

STDEF_FUNC(BaseNetConnectRelease_strF)
{
	const char* strServerInfo;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetConnectRelease_strF, strServerInfo);
	char strBuf[4096];
	strcpy_s(strBuf, 4096, strServerInfo);
	BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strBuf, 4096);

	BaseNetManager* pNetManager;

	STDEF_Manager(pManager);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvVariable = pState->variable_get();  //_pdsvContext;
	int nHashNetManager = STRTOHASH("BaseNetManager");

    pNetManager = (BaseNetManager*)pdsvVariable->get_point(nHashNetManager);
    
    if (pNetManager == NULL || !pNetManager->isok()) {
        g_SendMessage(LOG_MSG, "BaseNetManager_ConnectEndFail");
        return 0;
    }

    pdsvVariable->set_alloc(nHashNetManager, NULL);
    pManager->net_manager_delete(pNetManager);
    pNetManager->release_thread();// It will delete pNetManager in thread

    g_SendMessage(LOG_MSG, "BaseNetManager_ConnectEnd = %x", pNetManager);
	// release socket
	return 1;
}

STDEF_FUNC(BaseNetConnect_strF)
{
	const char *strServerInfo;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetConnect_strF, strServerInfo);
	char strBuf[4096];
	strcpy_s(strBuf, 4096, strServerInfo);
	BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strBuf, 4096);

	BaseNetManager	*pNetManager;

	STDEF_Manager(pManager);
	STDEF_BaseState(pState);
	BaseDStructureValue *pdsvVariable = pState->variable_get();  //_pdsvContext;
	int nHashNetManager = STRTOHASH("BaseNetManager");
	
	BaseFile paser;
	paser.OpenFile((void*)strBuf, (int)strlen(strBuf));
	paser.set_asc_seperator(" ,");
	paser.set_asc_deletor(" ,");

	int index_start = 0;
	int nPort = 19101;
	char strServer[255];
	paser.read_asc_line();
	if (!paser.read_asc_string(strServer, 255))
		return 0;
	if (!paser.read_asc_integer(&nPort))
		return 0;
	//if (!paser.read_asc_integer(&index_start))
	//	index_start = 0;

	PT_OAlloc4(pNetManager, BaseNetManager, false, strServer, nPort, index_start);
	//mpool_get().observe_push(pNetManager);
	pManager->net_manager_add(pNetManager);
	pdsvVariable->get_base()->add_column(0, "BaseNetManager", TYPE_INT64);
    pdsvVariable->set_point(nHashNetManager, pNetManager);
    //pdsvVariable->set_alloc(nHashNetManager, (const void*)&pNetManager);
	pState->UpdateFuncRegist(_pdsvEvent, pNetManager, fnUpdateMessageReceiver);
	pState->group_id_add(HASH_STATE(Server), pNetManager->managerid_get());
	//pState->identifier_set(pNetManager->managerid_get(), HASH_STATE(Server));

	g_SendMessage(LOG_MSG, "BaseNetManager_Connect = %x", pNetManager);
	
	return 1;
}

bool base_net_event_send(BaseDStructureValue* _event_send, int _index, BaseDStructureValue* _variable_state)
{
	int	nSize = _event_send->get_dumppacket_size();
	if (nSize > 1024 * 1024 * 5) // 1024 *1024 * 5 check 5Mbyte
	{
		g_SendMessage(LOG_MSG_POPUP, "BaseStateManagerLogic::socket_packet_send:: Over size of event 5m dump(%d byte).", nSize);
		return false;
	}

	char* pData = PT_Alloc(char, nSize + sizeof(int));
	nSize = _event_send->get_dumppacket(pData+sizeof(int), nSize);
	memcpy(pData, &nSize, sizeof(int));

	if (nSize == 0) {
		PT_Free(pData);
		return false;
	}

	BaseNetManager* pNetManager = NULL;
	int nHashNetManager = STRTOHASH("BaseNetManager");

	pNetManager = (BaseNetManager*)_variable_state->get_point(nHashNetManager);
	if (_index > 0)
	{
		if (pNetManager == NULL || !pNetManager->isok()) {
			g_SendMessage(LOG_MSG, "BaseNetManager_SendFail");
			PT_Free(pData);
			return false;
		}

		//g_SendMessage(LOG_MSG, "BaseNetManager_Send = %x, %d", pNetManager, _index);

		int sended = 0;
		nSize += sizeof(int);
		for (int sended = 0; sended < nSize; sended += STATE_MAX_PACKET)
		{
			int send = STATE_MAX_PACKET;
			if (send > nSize-sended)
				send = nSize-sended;
			pNetManager->send(_index, pData+sended, send);
		}
		PT_Free(pData);
		return true;
	}
	PT_Free(pData);
	return false;
}
/*
bool back_up_base_net_event_send(BaseDStructureValue *_event_send, int _index, BaseDStructureValue *_variable_state)
{
	int	nSize = _event_send->get_dumppacket_size();
	if (nSize > STATE_MAX_PACKET) // 1024 *1024 * 5 check 5Mbyte
	{
		g_SendMessage(LOG_MSG_POPUP, "BaseStateManagerLogic::socket_packet_send:: Over size of event dump(%d byte).", nSize);
		return false;
	}

	char* pData = PT_Alloc(char, nSize);
	nSize = _event_send->get_dumppacket(pData, nSize);

	if (nSize == 0) {
		PT_Free(pData);
		return false;
	}

	BaseNetManager	*pNetManager = NULL;
	int nHashNetManager = STRTOHASH("BaseNetManager");
	
    pNetManager = (BaseNetManager*)_variable_state->get_point(nHashNetManager);
	if (_index > 0)
	{
		if (pNetManager == NULL) {
			g_SendMessage(LOG_MSG, "BaseNetManager_SendFail");
			PT_Free(pData);
			return false;
		}

		g_SendMessage(LOG_MSG, "BaseNetManager_Send = %x, %d", pNetManager, _index);
		pNetManager->send(_index, pData, nSize);
		PT_Free(pData);
		return true;
	}
	PT_Free(pData);
	return false;
}//*/

STDEF_FUNC(BaseNetEventReturn_nF)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetEventReturn_nF, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);
	BaseDStructureValue* variable_state = pState->variable_get();
	int nBroadCast = 0;

	const int* pnTarget;
	if (_pdsvEvent == NULL || !STDEF_GET(_pdsvEvent, RevNetStateEventCaster, pnTarget))
		STDEF_GET_R(variable_state, RevNetStateEventCaster, pnTarget);
#ifdef _DEBUG
	if(pnTarget)
		g_SendMessage(LOG_MSG, "BaseNetEventReturn serialnetmatch:%d state:%d", *pnTarget, pState->get_key());
#endif
	pdsvEvent->set_alloc(HASH_STATE(BaseEventTargetStateSerial), pnTarget);
	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nBroadCast);

	int index = pState->identifier_get(HASH_STATE(Socket));

	if (!base_net_event_send(pdsvEvent, index, pState->variable_get()))
		return 0;

	pState->EventSendReset(); // remove sent event so next casting will be sent new event.

	return 1;
}

STDEF_FUNC(BaseNetEventPass_nF)
{
	const int* pnType;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetEventPass_nF, pnType);
	STDEF_BaseState(pState);
	STDEF_Manager(manager);

	BaseDStructureValue* pdsvV1 = NULL;
	pdsvV1 = manager->variable_type(*pnType, _pdsvBase, _pdsvContext, _pdsvEvent);
	
	BaseDStructure* pdstEvent = manager->EnumGet(HASH_STATE(HASH_EnumEvent));
	if (pdstEvent == NULL)// StateManager destoried.
		return 0;

	BaseDStructureValue* pdsvEvent;
	PT_OAlloc2(pdsvEvent, BaseDStructureValue, pdstEvent, 1024);

	pdsvV1->variable_transit(pdsvEvent);
	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	pdsvEvent->set_key(nKeyState);
	pdsvEvent->set_alloc(HASH_STATE(TIMETOPROCESS), NULL);

	int nSerial = pState->obj_serial_get();
	pdsvEvent->set_alloc(HASH_STATE(RevNetStateEventCaster), &nSerial);
	int index = pState->identifier_get(HASH_STATE(Socket));

	bool result = base_net_event_send(pdsvEvent, index, pState->variable_get());

	PT_OFree(pdsvEvent);

	if (result)
		return 1;

	return 0;
}

STDEF_FUNC(BaseNetReturnPass_nF)
{
	const int* pnType;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetReturnPass_nF, pnType);
	STDEF_BaseState(pState);
	STDEF_Manager(manager);

	BaseDStructureValue* pdsvV1 = NULL;
	pdsvV1 = manager->variable_type(*pnType, _pdsvBase, _pdsvContext, _pdsvEvent);

	BaseDStructure* pdstEvent = manager->EnumGet(HASH_STATE(HASH_EnumEvent));
	if (pdstEvent == NULL)// StateManager destoried.
		return 0;

	BaseDStructureValue* pdsvEvent;
	PT_OAlloc2(pdsvEvent, BaseDStructureValue, pdstEvent, 1024);

	pdsvV1->variable_transit(pdsvEvent);
	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	pdsvEvent->set_key(nKeyState);
	pdsvEvent->set_alloc(HASH_STATE(TIMETOPROCESS), NULL);

	STDEF_Space(pSpace);
	BaseDStructureValue* variable_state = pState->variable_get();
	int nBroadCast = 0;

	const int* pnTarget;
	if (_pdsvEvent == NULL || !STDEF_GET(_pdsvEvent, RevNetStateEventCaster, pnTarget))
		STDEF_GET_R(variable_state, RevNetStateEventCaster, pnTarget);

	pdsvEvent->set_alloc(HASH_STATE(BaseEventTargetStateSerial), pnTarget);
	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nBroadCast);

	int index = pState->identifier_get(HASH_STATE(Socket));

	bool result = base_net_event_send(pdsvEvent, index, pState->variable_get());

	PT_OFree(pdsvEvent);

	if (!result)
		return 0;
	return 1;
}

STDEF_FUNC(BaseNetEventSend_nF)
{
	const int *pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetEventSend_nF, pnEvent);
	
	STDEF_BaseState(pState);

	BaseDStructureValue *pEvent = pState->EventSendGet(*pnEvent);

	int nSerial = pState->obj_serial_get();
	pEvent->set_alloc(HASH_STATE(RevNetStateEventCaster), &nSerial);
#ifdef _DEBUG
	int d_key = pState->get_key();
	if (d_key != 539992502)
	{
		g_SendMessage(LOG_MSG, "BaseNetEventSend serialnetmatch state:%d serial:%d", pState->get_key(), nSerial);
	}
#endif
	int index = pState->identifier_get(HASH_STATE(Socket));

	if (base_net_event_send(pEvent, index, pState->variable_get()))
		return 1;

	return 0;
}

STDEF_FUNC(BaseNetEventBroadCast_nF)
{
	const int *pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetEventBroadCast_nF, pnEvent);
	char* pData = PT_Alloc(char, STATE_MAX_PACKET);

	STDEF_BaseState(pState);

	BaseDStructureValue *pEvent = pState->EventSendGet(*pnEvent);
	int	nSize = pEvent->get_dumppacket_size();
	if (nSize > STATE_MAX_PACKET)
	{
		g_SendMessage(LOG_MSG_POPUP, "BaseStateManagerLogic::socket_packet_send:: Over size of event dump(%d byte).", nSize);
		PT_Free(pData);
		return 0;
	}

	nSize = pEvent->get_dumppacket(pData, STATE_MAX_PACKET);

	BaseNetManager	*pNetManager;
	int nHashNetManager = STRTOHASH("BaseNetManager");
	BaseDStructureValue* pdsvVariable = pState->variable_get(); //_pdsvContext;

	int nIndex = pState->identifier_get(HASH_STATE(Socket));

    pNetManager = (BaseNetManager*)pdsvVariable->get_point(nHashNetManager);
	if (nIndex > 0)
	{
		if (pNetManager == NULL || !pNetManager->isok()) {
			PT_Free(pData);
			return 0;
		}

		pNetManager->broadcast(pData, nSize);
		PT_Free(pData);
		return 1;
	}
	PT_Free(pData);
	return 0;
}

STDEF_FUNC(BaseNetFileOpen_nIf)
{
	const int *pnParam;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetFileOpen_nIf, pnParam);
	
	const char *strFilename;
	strFilename = (const char*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvContext);
	if (strFilename == NULL)
		return 0;
	//STDEF_GET_R(_pdsvContext, NetFileFilename, strFilename);

	char strBuf[4096];
	BaseSystem::tomulti(strFilename, strBuf, 4096);
	//strcpy_s(strBuf, 4096, strFilename);

	int nRet = 1;
	int nType = BaseFile::OPEN_WRITE;

	void *pfile = NULL;

	STDEF_BaseState(pState);
	STDEF_Manager(pManager);
	if (*pnParam == STRTOHASH("FILE_READ")) {
		nType = BaseFile::OPEN_READ;
		pfile = ((BaseResManager*)pManager->GetResManager())->get_resource(pManager->path_full_make(strBuf, 4096), BaseResFile::GetObjectId(), NULL, NULL, 0);
		
		if (pfile == NULL)
			nRet = 1;
		else {
			((BaseResFile*)pfile)->inc_ref(pState);
			nRet = 0;
		}
	}
	else {
		BaseFile *pBaseFile = NULL;
		PT_OAlloc(pBaseFile, BaseFile);
		nRet = pBaseFile->OpenFile(pManager->path_full_make(strBuf, 4096), nType);

		pfile = (void*)pBaseFile;
	}
	
	if (nRet != 0) {
		g_SendMessage(LOG_MSG, "Fail to open netfile(type:%d)%s::::%s", nType, strFilename, strBuf);
		return 0;
	}
	else {
		g_SendMessage(LOG_MSG, "OpenFile: %s", strBuf);
	}

	INT64 nPoint = (INT64)pfile;

	pManager->variable_define(STRTOHASH("NetFilePoint"), TYPE_INT64);
	pManager->variable_define(STRTOHASH("NetFileSizeStream"), TYPE_INT32);
	_pdsvContext->set_alloc("NetFilePoint", &nPoint);
	int nStream = 0;
	_pdsvContext->set_alloc("NetFileSizeStream", &nStream);

	return 1;
}

STDEF_FUNC(BaseNetFileClose_nF)
{
	const int *pnClose;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetFileClose_nF, pnClose);

	const INT64 *pnPoint;
	STDEF_GET_R(_pdsvContext, NetFilePoint, pnPoint);
	void *pPoint = (void*)*pnPoint;
	STDEF_Manager(pManager);

	if (((BaseResManager*)pManager->GetResManager())->is_resource((void*)*pnPoint, BaseResFile::GetObjectId()))
	{
		BaseResFile *pRes = dynamic_cast<BaseResFile*>((BaseResFile*)pPoint);

		if (pRes)
		{
			STDEF_BaseState(pState);
			
			if (*pnClose == 1 && pRes->GetRefCount() == 2)
			{
				((BaseResManager*)pManager->GetResManager())->ClearResource(pRes);
			}
			pRes->dec_ref(pState);
			_pdsvContext->set_alloc("NetFilePoint", NULL);
			return 1;
		}
	}
	else {
		BaseFile *pfile = dynamic_cast<BaseFile*>((BaseFile*)pPoint);

		if (pfile)
		{
			pfile->CloseFile();
			PT_OFree(pfile);
			_pdsvContext->set_alloc("NetFilePoint", NULL);
			return 1;
		}
	}

	return 0;
}

STDEF_FUNC(BaseNetFileSend_nF)
{
	const int *pnSizePacket;
	STDEF_GETLOCAL_R(_pdsvBase, BaseNetFileSend_nF, pnSizePacket);

	int nSizePacket = 6000;
	if (*pnSizePacket > nSizePacket)
		nSizePacket = *pnSizePacket;

	const INT64 *pnPoint;
	STDEF_GET_R(_pdsvContext, NetFilePoint, pnPoint);

	BaseResFile *pRes = dynamic_cast<BaseResFile*>((BaseResFile*)*pnPoint);

	if (!pRes)
		return 0;

	const int *pnStream, *pnSize;

	STDEF_GET_R(_pdsvContext, NetFileSize, pnSize);
	STDEF_GET_R(_pdsvContext, NetFileSizeStream, pnStream);
	int nStreamCur = *pnStream;

	char *pBuf = PT_Alloc(char, nSizePacket);

	int nRead = pRes->read(nStreamCur, (bbyte*)pBuf, nSizePacket);
	nStreamCur += nRead;

	//printf("stream:%d/%d\n", nStreamCur, *pnSize);
	
	if (nRead <= 0) {
		PT_Free(pBuf);
		return 0;
	}

	STDEF_BaseState(pState);
	BaseDStructureValue *pEvent = pState->EventSendGet();
	short sRead = (short)nRead;
	pEvent->set_alloc("BaseBinary_abV", pBuf, sRead);

	PT_Free(pBuf);
	_pdsvContext->set_alloc("NetFileSizeStream", &nStreamCur);
	return 1;
}

STDEF_FUNC(BaseNetFileReceive_nF)
{
	const INT64 *pnPoint;
	STDEF_GET_R(_pdsvContext, NetFilePoint, pnPoint);

	BaseFile *pfile = (BaseFile*)*pnPoint;

	const int *pnStream, *pnSize;

	STDEF_GET_R(_pdsvContext, NetFileSize, pnSize);
	STDEF_GET_R(_pdsvContext, NetFileSizeStream, pnStream);
	int nStreamCur = *pnStream;

	const char *pBuf = NULL;
	short nCnt;
	STDEF_AGET_R(_pdsvEvent, BaseBinary_abV, pBuf, nCnt);
	
	int nSizePacket = (int)nCnt;
	pfile->Write(pBuf, nSizePacket);
	nStreamCur += nSizePacket;
	//printf("stream:%d\n", nStreamCur);
	
	if (nSizePacket <= 0)
		return 0;

	_pdsvContext->set_alloc("NetFileSizeStream", &nStreamCur);

	return 1;
}


STDEF_FUNC(BaseNetSocketRemake_nF)
{
	BaseNetManager	*pNetManager;
	int nHashNetManager = STRTOHASH("BaseNetManager");
	STDEF_BaseState(state);
	BaseDStructureValue* pdsvVariable = state->variable_get();//_pdsvContext;

    pNetManager = (BaseNetManager*)pdsvVariable->get_point(nHashNetManager);
    if (pNetManager == NULL)
        return 0;

	if (!pNetManager->isok())
		return 0;

    pNetManager->reinit();
    return 1;
}

bool BaseNetManager::check(BaseNetManager* _manager)
{
	for (int i = 0; i < ms_netManagers.size(); i++)
	{
		if (_manager == ms_netManagers[i])
			return true;
	}
	return false;
}

bool BaseNetManager::isok()
{
	return check(this);
}
