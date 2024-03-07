#include "stdafx.h"
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include "BaseSocket.h"
#include <errno.h>

int BaseSocket::sm_nWinsockCount	= 0;

PtObjectCpp(BaseSocket);
PT_OPTCPP(BaseSocket)

BaseSocket::BaseSocket()
{
}

BaseSocket::BaseSocket(bool _bServer, const char *_strServer, unsigned short _nPort)
{
	init(_bServer, _strServer, _nPort);
}

sockaddr_in *BaseSocket::address_convert(sockaddr_in *_addr, const char *_server, unsigned short _port)
{
	/* Clear out m_sRemote struct */
	memset((void *)_addr, '\0', sizeof(struct sockaddr_in));
	
	/* Set family and port */
	_addr->sin_family = AF_INET;
	_addr->sin_port = htons(_port);

	struct hostent *hp;					/* Information about this computer */
	/* Set address automatically if desired */
	/* Get host name of this computer */
	//gethostname(host_name, sizeof(host_name));
	hp = gethostbyname(_server);

	/* Check for NULL pointer */
	if (hp == NULL)
	{
	   fprintf(stderr, "Could not get host name.\n");
	   return NULL;
	}
#ifndef WIN32
	/* Assign the address */
	char strBuffer[255];
	unsigned add[4];
	for(int i=0; i<4; i++)
	   add[i] = (unsigned char)hp->h_addr_list[0][i];
	sprintf_s(strBuffer, 255, "%u.%u.%u.%u"
			 , add[0], add[1], add[2], add[3]);
	inet_aton(strBuffer, &_addr->sin_addr);
	printf("%s\n", strBuffer);
#else
		   /* Assign the address */
	_addr->sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
	_addr->sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
	_addr->sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
	_addr->sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
#endif
	return _addr;
}

void BaseSocket::init(bool _bServer, const char *_strServer, unsigned short _nPort)
{
	m_bConnected	= false;
	m_bServer		= _bServer;

    //printf("IP:%s Port:%d\n", _strServer, _nPort);
	if(sm_nWinsockCount == 0)
	{
		/* Open windows connection */
		BaseSystem::WSAStartup();
	}

	/* Open a datagram socket */
	m_socket = (UINT32)socket(AF_INET, SOCK_DGRAM, 0);
#ifdef WIN32
	if (m_socket == INVALID_SOCKET)
#else
    if(m_socket < 0)
#endif
	{
		fprintf(stderr, "Could not create socket.\n");
		m_bSocketInit	= false;
	}
	else {
		m_bSocketInit = true;

		port_number = _nPort;
		if (_strServer && strncmp(_strServer, "localhost", 9) != 0) {
			strcpy_s(host_name, 255, _strServer);
			if (address_convert((sockaddr_in*)m_sRemote, host_name, port_number)
				== NULL)
				return;
		}else {
			strcpy_s(host_name, 255, "localhost");

			if (!m_bServer) {
				if (address_convert((sockaddr_in*)m_sRemote, host_name, port_number)
					== NULL)
					return;
			}
			else {
				((struct sockaddr_in*)m_sRemote)->sin_family = AF_INET;
				((struct sockaddr_in*)m_sRemote)->sin_addr.s_addr = INADDR_ANY;
				((struct sockaddr_in*)m_sRemote)->sin_port = htons(port_number);
			}
		}

		/* Clear out m_sLocal struct */
		memset((void *)m_sLocal, '\0', sizeof(struct sockaddr_in));

		memcpy(m_sLocal, m_sRemote, sizeof(sockaddr_in));

		if(!_bServer)
			((sockaddr_in*)m_sLocal)->sin_port	= 0;
	}

	sm_nWinsockCount++;
}

void BaseSocket::release()
{
}

BaseSocket::~BaseSocket(void)
{
	close_();

	sm_nWinsockCount--;
	if(sm_nWinsockCount == 0)
	{
        BaseSystem::WSACleanup();
	}
}

void BaseSocket::close_()
{
	if(m_bSocketInit)
	{
#ifdef WIN32
		closesocket(m_socket);
#else
        close(m_socket);
        printf("close socket %d\n", m_socket);
#endif
		m_bSocketInit = false;
		m_bConnected	= false;
	}
}

void BaseSocket::set_port(unsigned short _nPort)
{
	port_number	= _nPort;
	((sockaddr_in*)m_sRemote)->sin_port	= htons(_nPort);
}

void BaseSocket::weakup() {
	if (!m_bSocketInit)
		return;

#ifdef WIN32
	closesocket(m_socket);
#else
	close(m_socket);
#endif
	
	/* Open a datagram socket */
	m_socket = (UINT32)socket(AF_INET, SOCK_DGRAM, 0);
#ifdef WIN32
	if (m_socket == INVALID_SOCKET)
#else
	if (m_socket < 0)
#endif
	{
		fprintf(stderr, "Could not create socket in weakup.\n");
		m_bSocketInit = false;
		m_bConnected = false;
		return;
	}

#ifdef WIN32
	u_long iMode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &iMode))
#else
	if (fcntl(m_socket, F_SETFL, O_NONBLOCK) == -1)
#endif
	{
		fprintf(stderr, "ioctlsoket failed in weakup");
		m_bConnected = false;
	}

	if (m_bServer)
	{
		struct sockaddr *sAddr;
		sAddr = (struct sockaddr*)m_sLocal;
		/* Bind address to socket */
		if (::bind(m_socket, sAddr, sizeof(struct sockaddr_in)) == -1)
		{
			fprintf(stderr, "Could not bind name to socket in weakup.\n");
            
            //printf("Error code: %d\n", errno);
			m_bConnected = false;
            
		}
	}
}

int BaseSocket::bind()
{    
#ifdef WIN32
	u_long iMode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &iMode))
#else
    if(fcntl(m_socket, F_SETFL, O_NONBLOCK) == -1)
#endif
	{
		fprintf(stderr, "ioctlsoket failed");
		return 0;
	}

	if(m_bServer)
	{
		struct sockaddr *sAddr;
		sAddr	= (struct sockaddr*)m_sLocal;
		/* Bind address to socket */

		int result = 0, cnt = 0;
		do {
			result = ::bind(m_socket, sAddr, sizeof(struct sockaddr_in));
			if (result != -1)
				break;
			BaseSystem::Sleep(100);
			cnt++;
		} while (result == -1 && errno == 48 && cnt <= 10); // if 48(already using), some of other thread is using this port and can closing so try while a second.

		if (result == -1)
		{
			fprintf(stderr, "Could not bind name to socket.\n");
            
            //printf("Error2 code: %d\n", errno);
		}else{
            fprintf(stderr, "Server socket created.\n");
			m_bConnected	= true;
		}
    }else{
        fprintf(stderr, "Client socket created.\n");
		m_bConnected	= true;
	}
	return 0;
}

//#ifdef _DEBUG
//#define ERROR_TEST_PACKET
//#endif

void code_encode(char *_pData, int _nSize)
{
	static char kyes[14] = { 2, 3, 4, 5, (char)-100, (char)-113, 2, (char)-110, (char)-115, (char)-116, 2, 3, 4, (char)-122 };
	for (int i = 0; i < _nSize; i++)
	{
		*(_pData + i) = *(_pData + i) ^ kyes[i % 14];
	}
}

int BaseSocket::send(char *_pData, int _nSize, char *_pRemote)
{
	/* Tranmsit data to get time */
	INT32 server_length = sizeof(struct sockaddr_in);
	int nRet = -1;

	if(_pRemote && _pRemote != (char*)1)
			memcpy(m_sRemote, _pRemote, sizeof(sockaddr));
	code_encode(_pData, _nSize);

	bool bNormal = true;
#ifdef ERROR_TEST_PACKET
	if(_pRemote != (char*)1)
	{
		static int s_nMissRate	= 0;
		s_nMissRate++;
		if((s_nMissRate % 15) == 1)
		{
			nRet	= 0;
			//printf(stderr, "s");
			printf("x");
			bNormal = false;
		}
	}
#endif
	if(bNormal)
	{
#ifdef _WIN32
		nRet = (int)sendto(m_socket, _pData, (INT32)_nSize, 0, (struct sockaddr *)m_sRemote, server_length);

#ifdef _DEBUG
		sockaddr_in dSocket;
		memcpy(&dSocket, m_sRemote, sizeof(sockaddr_in));

		//printf("\nsend: size%d, ip:%d, %d, %d, %d port:%d", nRet, dSocket.sin_addr.S_un.S_un_b.s_b1
		//	, dSocket.sin_addr.S_un.S_un_b.s_b2
		//	, dSocket.sin_addr.S_un.S_un_b.s_b3
		//	, dSocket.sin_addr.S_un.S_un_b.s_b4
		//	, dSocket.sin_port);
#endif

#else
		nRet = (int)sendto(m_socket, _pData, (INT32)_nSize, 0, (struct sockaddr *)m_sRemote, (socklen_t)server_length);
#endif
		//printf(".");
#ifdef ERROR_TEST_PACKET
		if(_pRemote != (char*)1)
			printf(".");
#endif
	}
	
	if (nRet == -1)
	{
		//fprintf(stderr, "Error transmitting data.\n");
		//fprintf(stderr, "E");
		return -1;
	}
	return nRet;
}

int	BaseSocket::receive(char *_pData, int _nSize, char *_pRemote, char *_anIP, short *_pnPort)
{
	int nRet;

	/* Receive bytes from m_sLocal */
#ifdef WIN32
	int client_length = (int)sizeof(struct sockaddr_in);
	nRet = recvfrom(m_socket, _pData, _nSize, 0, (struct sockaddr *)m_sRemote, &client_length);
#else
	INT32 client_length = (INT32)sizeof(struct sockaddr_in);
	nRet = (int)recvfrom(m_socket, _pData, (INT32)_nSize, 0, (struct sockaddr *)m_sRemote, (socklen_t*)&client_length);
#endif

#ifdef ERROR_TEST_PACKET
	static int s_nMissRate2	= 0;

	if(nRet > 0 && _anIP != (char*)1)
	{
		s_nMissRate2++;
		if((s_nMissRate2 % 10) == 1)
		{
			nRet	= 0;
			printf("|");
		}else{
			printf("-");
		}
	}
#endif
	code_encode(_pData, nRet);
	//for(int i=0; i<nRet; i++)
	//	*(_pData+i)	= *(_pData+i) ^ 0x53;
	
	if(_pRemote)
		memcpy(_pRemote, m_sRemote, sizeof(sockaddr));
	sockaddr_in *pstSock = (sockaddr_in*)_pRemote;
	if(_anIP && _anIP != (char*)1)
	{
#ifdef _WIN32
     	_anIP[0] = pstSock->sin_addr.S_un.S_un_b.s_b1;
		_anIP[1] = pstSock->sin_addr.S_un.S_un_b.s_b2;
		_anIP[2] = pstSock->sin_addr.S_un.S_un_b.s_b3;
		_anIP[3] = pstSock->sin_addr.S_un.S_un_b.s_b4;
#else
        _anIP[0] = (pstSock->sin_addr.s_addr >> 24) & 0xFF;
        _anIP[1] = (pstSock->sin_addr.s_addr >> 16) & 0xFF;
        _anIP[2] = (pstSock->sin_addr.s_addr >> 8) & 0xFF;
        _anIP[3] = pstSock->sin_addr.s_addr & 0xFF;
#endif
//        ((sockaddr_in*)pstSock)->sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
	}
	if(_pnPort)
		*_pnPort = pstSock->sin_port;
	// UDP doesn't conform the data correctness.
	// So you have to check data when you receive.
	if (nRet < 0)
	{
		//fprintf(stderr, "Could not receive datagram.\n");
		return -1;
	}

	return nRet;
}

bool BaseSocket::is_connected()
{
	return m_bConnected;
}

bool BaseSocket::get_remote(char *_pRemote, char *_pnIP, short *_pnPort)
{
	memcpy(_pRemote, m_sRemote, 16);
	sockaddr_in *pstSock = (sockaddr_in*)_pRemote;
	if(_pnIP)
	{
#ifdef _WIN32
        _pnIP[0] = pstSock->sin_addr.S_un.S_un_b.s_b1;
        _pnIP[1] = pstSock->sin_addr.S_un.S_un_b.s_b2;
        _pnIP[2] = pstSock->sin_addr.S_un.S_un_b.s_b3;
        _pnIP[3] = pstSock->sin_addr.S_un.S_un_b.s_b4;
#else
        _pnIP[0] = (pstSock->sin_addr.s_addr >> 24) & 0xFF;
        _pnIP[1] = (pstSock->sin_addr.s_addr >> 16) & 0xFF;
        _pnIP[2] = (pstSock->sin_addr.s_addr >> 8) & 0xFF;
        _pnIP[3] = pstSock->sin_addr.s_addr & 0xFF;
#endif
	}
	if(pstSock)
		*_pnPort = pstSock->sin_port;
	return true;
}
