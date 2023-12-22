#include "stdafx.h"
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

#ifndef HAVE__STRNICMP
#define HAVE__STRNICMP
#define _strnicmp strncasecmp
#endif

typedef unsigned short WORD;
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct hostent* LPHOSTENT;
typedef struct servent* LPSERVENT;
typedef struct in_addr* LPIN_ADDR;
typedef struct in_addr  IN_ADDR;
typedef struct sockaddr* LPSOCKADDR;
#define closesocket close

#define LINUX

#else
#include <winsock.h>
#include <windows.h>
//#include <ws2def.h>
#endif
#include <vector>
#include <string>
#include <deque>
#include <map>
#include "BaseHttp.h"
#include <string.h>

BaseHttp::BaseHttp(void)
{
}

BaseHttp::~BaseHttp(void)
{
	clear();
}

void BaseHttp::clear()
{
	for(unsigned i=0; i<stlVpTables_.size(); i++)
	{
		delete stlVpTables_[i];
	}
	stlVpTables_.clear();

	stlVpTablesLoading_.clear();
}

bool BaseHttp::parse(char *pDocBuffer, UINT32 _nSize)
{
	set_asc_seperator("\t ;");
	if(!OpenFile(pDocBuffer, OPEN_READ | OPEN_MEMORY, 0, _nSize))
	{
		while(read_asc_line())
		{
			while(m_strBufferAscLine[m_nCurCharAscLine] != 0 && m_nCurCharAscLine < ASC_MAX_LENGTH)
			{
				if(m_strBufferAscLine[m_nCurCharAscLine] == '<')
				{
					m_nCurCharAscLine++;
					parse_brace();
				}else{
					m_nCurCharAscLine++;
				}
			}
		}
		CloseFile();
		return true;
	}

	return false;
}

bool BaseHttp::parse(char *strFilename)
{
	set_asc_seperator("\t ;");
	if(!OpenFile(strFilename, OPEN_READ))
	{
		while(read_asc_line())
		{
			while(m_strBufferAscLine[m_nCurCharAscLine] != 0 && m_nCurCharAscLine < ASC_MAX_LENGTH)
			{
				if(m_strBufferAscLine[m_nCurCharAscLine] == '<')
				{
					m_nCurCharAscLine++;
					parse_brace();
				}else{
					m_nCurCharAscLine++;
				}
			}
		}
		CloseFile();
		return true;
	}

	return false;
}

void BaseHttp::parse_comment()
{
	while(m_strBufferAscLine[m_nCurCharAscLine] != 0 && m_nCurCharAscLine < ASC_MAX_LENGTH)
	{
		if(m_strBufferAscLine[m_nCurCharAscLine] == '>')
		{
			m_nCurCharAscLine++;
			break;
		}else if(m_strBufferAscLine[m_nCurCharAscLine] == '<'){
			m_nCurCharAscLine++;
			parse_comment();
		}else{
			m_nCurCharAscLine++;
		}
	}
}

int BaseHttp::parse_brace()
{
	if(!parse_token_())
	{
		if(read_asc_line())
			if(!parse_token_())
				return DEF_UNKNOW;
	}

	int nRet = get_def(m_strToken->c_str());
	
	size_t nLen = m_strToken->size();

	if(nRet == DEF_IN_CMT){
		parse_comment();
	}else if((*m_strToken)[nLen-1] != '>')
	{
		do{
			bool bEnd = false;
			while(m_strBufferAscLine[m_nCurCharAscLine] != 0 && m_nCurCharAscLine < ASC_MAX_LENGTH)
			{
				if(m_strBufferAscLine[m_nCurCharAscLine] == '>')
				{
					m_nCurCharAscLine++;
					bEnd = true;
					break;
				}else if(m_strBufferAscLine[m_nCurCharAscLine] == '<'){
					m_nCurCharAscLine++;
					parse_brace();

					if(m_nSizeAscBlock == 0)
						break;
				}else{
					m_nCurCharAscLine++;
				}
			}
			if(bEnd)
				break;
		}while(read_asc_line());
	}

	switch(nRet)
	{
	case DEF_IN_TABLE:
		parse_table();
		break;
	case DEF_IN_TR:
		parse_tr();
		break;
	case DEF_IN_TD:
		parse_td();
		break;
	}
	return nRet;
}

void BaseHttp::parse_table()
{
	STHttpTable *pTable = new STHttpTable;
	stlVpTables_.push_back(pTable);
	stlVpTablesLoading_.push_back(pTable);
	size_t nIndex = stlVpTablesLoading_.size()-1;
	do{
		bool bEnd = false;
		while(m_strBufferAscLine[m_nCurCharAscLine] != 0 && m_nCurCharAscLine < ASC_MAX_LENGTH)
		{
			if(m_strBufferAscLine[m_nCurCharAscLine] == '<'){
				m_nCurCharAscLine++;
				if(DEF_OUT_TABLE == parse_brace())
				{
					stlVpTablesLoading_.erase(stlVpTablesLoading_.begin()+nIndex);
					bEnd = true;
					break;
				}
			}else{
				m_nCurCharAscLine++;
			}
		}
		if(bEnd)
			break;
	}while(read_asc_line());
}

void BaseHttp::parse_tr()
{
	STHttpTable::STLVpText *pstlVpText = new STHttpTable::STLVpText;
	stlVpTablesLoading_.back()->stlVpstlVpArray_.push_back(pstlVpText);

	do{
		bool bEnd = false;
		while(m_strBufferAscLine[m_nCurCharAscLine] != 0 && m_nCurCharAscLine < ASC_MAX_LENGTH)
		{
			if(m_strBufferAscLine[m_nCurCharAscLine] == '<'){
				m_nCurCharAscLine++;
				if(DEF_OUT_TR == parse_brace())
				{
					bEnd = true;
					break;
				}
			}else{
				m_nCurCharAscLine++;
			}
		}
		if(bEnd)
			break;
	}while(read_asc_line());
}

void BaseHttp::parse_td()
{
	std::string *pstring = new std::string;
	stlVpTablesLoading_.back()->stlVpstlVpArray_.back()->push_back(pstring);
	do{
		bool bEnd = false;
		while(m_strBufferAscLine[m_nCurCharAscLine] != 0 && m_nCurCharAscLine < ASC_MAX_LENGTH)
		{
			if(m_strBufferAscLine[m_nCurCharAscLine] == '<'){
				m_nCurCharAscLine++;
				int nRet = parse_brace();
				if(DEF_OUT_TD == nRet || DEF_IN_TD == nRet)
				{
					bEnd = true;
					break;
				}
			}else{
				if(parse_token_())
				{
					if((*m_strToken)[0] != '&' && (*m_strToken)[0] != 0)
						*pstring += (*m_strToken).c_str();
				}

				//m_nCurCharAscLine++;
			}
		}
		if(bEnd)
			break;
	}while(read_asc_line());
}

int BaseHttp::get_def(const char *strToken)
{
	if(_strnicmp(strToken, "!--", 3) == 0)
		return DEF_IN_CMT;
	if(_strnicmp(strToken, "table", 5) == 0)
		return DEF_IN_TABLE;
	if(_strnicmp(strToken, "/table", 6) == 0)
		return DEF_OUT_TABLE;
	if(_strnicmp(strToken, "tr", 2) == 0)
		return DEF_IN_TR;
	if(_strnicmp(strToken, "/tr", 3) == 0)
		return DEF_OUT_TR;
	if(_strnicmp(strToken, "td", 2) == 0)
		return DEF_IN_TD;
	if(_strnicmp(strToken, "/td", 3) == 0)
		return DEF_OUT_TD;

	if(_strnicmp(strToken, "thead", 5) == 0)
		return DEF_UNKNOW;

	if(_strnicmp(strToken, "th", 2) == 0)
		return DEF_IN_TD;
	if(_strnicmp(strToken, "/th", 3) == 0)
		return DEF_OUT_TD;

	return DEF_UNKNOW;
}


bool BaseHttp::parse_token_(bool _quot)
{
	if(m_nCurCharAscLine >= ASC_MAX_LENGTH)
		return false;

	//while(check_seperator_(m_strBufferAscLine[m_nCurCharAscLine])
	//	&& m_strBufferAscLine[m_nCurCharAscLine] != 0
	//	&& m_nCurCharAscLine < ASC_MAX_LENGTH)
	//	m_nCurCharAscLine++;

	if(m_strBufferAscLine[m_nCurCharAscLine] == 0)
		return false;

	int nCnt = 0;

	if(m_strBufferAscLine[m_nCurCharAscLine] == '&')
	{
		m_strToken[nCnt] = m_strBufferAscLine[m_nCurCharAscLine];
		nCnt++;
		m_nCurCharAscLine++;
	}

	int seq = 0;
	while(!check_seperator_(m_strBufferAscLine[m_nCurCharAscLine], seq)
		&& m_strBufferAscLine[m_nCurCharAscLine] != 0
		&& m_strBufferAscLine[m_nCurCharAscLine] != '<'
		&& m_strBufferAscLine[m_nCurCharAscLine] != '&'
		&& m_nCurCharAscLine < ASC_MAX_LENGTH)
	{
		m_strToken[nCnt] = m_strBufferAscLine[m_nCurCharAscLine];
		nCnt++;
		m_nCurCharAscLine++;
		seq++;
	}

	if(m_strBufferAscLine[m_nCurCharAscLine] != 0
		&& m_strBufferAscLine[m_nCurCharAscLine] != '<'
		&& m_strBufferAscLine[m_nCurCharAscLine] != '&')
		m_nCurCharAscLine++;
	(*m_strToken)[nCnt] = 0;
	return true;
}

STHttpTable::~STHttpTable(void)
{
	for(unsigned i=0; i<stlVpstlVpArray_.size(); i++)
	{
		STLVpText *pstlVpText;
		pstlVpText = stlVpstlVpArray_[i];
		for(unsigned j=0; j<pstlVpText->size(); j++)
		{
			delete pstlVpText->at(j);
		}
		delete pstlVpText;
	}
	stlVpstlVpArray_.clear();
}

const char *STHttpTable::get_string(int _nLine, int _nRow)
{
	if((unsigned)_nLine >= stlVpstlVpArray_.size())
		return NULL;

	STLVpText *pstlVpText;
	pstlVpText = stlVpstlVpArray_[_nLine];

	if((unsigned)_nRow >= pstlVpText->size())
		return NULL;

	return pstlVpText->at(_nRow)->c_str();
}

unsigned STHttpTable::get_line()
{
	return (unsigned)stlVpstlVpArray_.size();
}

unsigned STHttpTable::get_row()
{
	if(stlVpstlVpArray_.size() == 0)
		return 0;

	STLVpText *pstlVpText;
	pstlVpText = stlVpstlVpArray_[0];

	return (unsigned)pstlVpText->size();
}

void del_non_num(char *_strNum)
{
	char *strTemp;
	while(*_strNum){
		if(*_strNum < '0' || *_strNum > '9' )
		{
			if(*_strNum == '(')
			{
				*_strNum	= NULL;
				break;
			}

			if(*((short*)_strNum) == 0xe5a1) // '-' sign
				*_strNum = '-';

			if(*_strNum != '-' && *_strNum != '+' && *_strNum != '.')
			{
				strTemp = _strNum;
				while(*strTemp){
					*strTemp = *(strTemp+1);
					strTemp++;
				}
				_strNum--;
			}
		}
		_strNum++;
	}
}

bool STHttpTable::get_int(int _nLine, int _nRow, int *_pnOut)
{
	const char *strString;
	strString = get_string(_nLine, _nRow);
	if(strString == NULL)
		return false;

	char strBuffer[255];
	strcpy_s(strBuffer, 255, strString);
	del_non_num(strBuffer);

	sscanf_s(strBuffer, "%d", _pnOut);
	return true;
}

bool STHttpTable::get_float(int _nLine, int _nRow, float *_pfOut)
{
	const char *strString;
	strString = get_string(_nLine, _nRow);
	if(strString == NULL)
		return false;

	char strBuffer[255];
	strcpy_s(strBuffer, 255, strString);
	del_non_num(strBuffer);

	sscanf_s(strBuffer, "%f", _pfOut);
	return true;
}

int	BaseHttp::get_size_table()
{
	return (int)stlVpTables_.size();
}

STHttpTable *BaseHttp::last()
{
	return stlVpTables_[stlVpTables_.size()-1];
}

STHttpTable *BaseHttp::find(char *_strFirst)
{
	for(unsigned i=0; i<stlVpTables_.size(); i++)
	{
		if(stlVpTables_[i]->stlVpstlVpArray_.size() > 0
			&& *(stlVpTables_[i]->stlVpstlVpArray_[0]->front()) == _strFirst)
		{
			return stlVpTables_[i];
		}
	}
	return NULL;
}


////////////////////////////////////////////////////////////
int BaseHttp::GetHTTP(const char* lpServerName, const char* lpFileName, char **ppDocBuffer)
{
	int	nRet = 0;
	//
	// Use inet_addr() to determine if we're dealing with a name
	// or an address
	//
    IN_ADDR		iaHost;
	LPHOSTENT	lpHostEntry;

	iaHost.s_addr = inet_addr(lpServerName);
	if (iaHost.s_addr == INADDR_NONE)
	{
		// Wasn't an IP address string, assume it is a name
		lpHostEntry = gethostbyname(lpServerName);
	}
	else
	{
		// It was a valid IP address string
		lpHostEntry = gethostbyaddr((const char *)&iaHost, 
			sizeof(struct in_addr), AF_INET);
	}
	if (lpHostEntry == NULL)
	{
		//_pFile->write_bin_string("gethostbyname()");
		puts("fail to get hostname");
		return 0;
	}


	//	
	// Create a TCP/IP stream socket
	//
	SOCKET	Socket;	

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		//_pFile->write_bin_string("socket()"); 
		puts("fail to make socket()");
		return 0;
	}

	//
	// Find the port number for the HTTP service on TCP
	//
	LPSERVENT lpServEnt;
	SOCKADDR_IN saServer;

	lpServEnt = getservbyname("http", "tcp");
	if (lpServEnt == NULL)
		saServer.sin_port = htons(80);
	else
		saServer.sin_port = lpServEnt->s_port;


	//
	// Fill in the rest of the server address structure
	//
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);


	//
	// Connect the socket
	//
	int nSize;

	nSize = connect(Socket, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));
	if (nSize == SOCKET_ERROR)
	{
		//_pFile->write_bin_string("connect()");
		puts("fail to make connection");
		closesocket(Socket);
		return 0;
	}


	//
	// Format the HTTP request
	//
	char szBuffer[1024];

	sprintf_s(szBuffer, 1024, "GET %s\n", lpFileName);
	nSize = (int)send(Socket, szBuffer, (int)strlen(szBuffer), 0);
	if (nSize == SOCKET_ERROR)
	{
		//_pFile->write_bin_string("send()");
		puts("fail to send()");
		closesocket(Socket);	
		return 0;
	}

	struct timeval tv; /* timeval and timeout stuff added by davekw7x */
	//int timeouts = 0;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv))
	{
		puts("setsockopt");
		return 0;
	}
    BaseSystem::Sleep(500);

	//
	// Receive the file contents and print to stdout
	//
	while(1)
	{
		// Wait to receive, nSize = NumberOfBytesReceived
		nSize = (int)recv(Socket, szBuffer, sizeof(szBuffer), 0);
		if (nSize == SOCKET_ERROR)
		{
			puts("fail to recv()");
			break;
		}

		// Did the server close the connection?
		if (nSize == 0)
			break;
		// Write to stdout
		memcpy(*ppDocBuffer, szBuffer, nSize);
		(*ppDocBuffer) += nSize;
		nRet	+= nSize;
	}

	if (nSize == SOCKET_ERROR)
	{
		return 0;
	}
	closesocket(Socket);	

	return nRet;
}

bool BaseHttp::InitHTTP()
{
//	WORD wVersionRequested = MAKEWORD(1,1);
//	WSADATA wsaData;
//	int nRet;

	//
	// Initialize WinSock.dll
	//
    
    BaseSystem::WSAStartup();
//	nRet = WSAStartup(wVersionRequested, &wsaData);
//	if (nRet)
//	{
//		fprintf(stderr,"\nWSAStartup(): %d\n", nRet);
//		WSACleanup();
//		return false;
//	}

	//
	// Check WinSock version
	//
//	if (wsaData.wVersion != wVersionRequested)
//	{
//		fprintf(stderr,"\nWinSock version not supported\n");
//		WSACleanup();
//		return false;
//	}

	return true;
}

void BaseHttp::CloseHTTP()
{
    BaseSystem::WSACleanup();
}

bool BaseHttp::GetData(const char* _serverName, int _port, const char* _request, STLVString *_data_a)
{
	//
	// Use inet_addr() to determine if we're dealing with a name
	// or an address
	//
	//IN_ADDR		iaHost;

	struct hostent* hp;					/* Information about this computer */
	/* Set address automatically if desired */
	/* Get host name of this computer */
	//gethostname(host_name, sizeof(host_name));
	hp = gethostbyname(_serverName);

	/* Check for NULL pointer */
	if (hp == NULL)
	{
		fprintf(stderr, "Could not get host name.\n");
		return false;
	}
	//
	// Create a TCP/IP stream socket
	//
	SOCKET	Socket;

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		//_pFile->write_bin_string("socket()");
		puts("fail to make socket()");
		return false;
	}

	SOCKADDR_IN saServer;

	saServer.sin_port = htons(_port);
	//
	// Fill in the rest of the server address structure
	//
	saServer.sin_family = AF_INET;
	//saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);


#ifndef WIN32
	/* Assign the address */
	char strBuffer[255];
	unsigned add[4];
	for (int i = 0; i < 4; i++)
		add[i] = (unsigned char)hp->h_addr_list[0][i];
	sprintf_s(strBuffer, 255, "%u.%u.%u.%u"
		, add[0], add[1], add[2], add[3]);
	inet_aton(strBuffer, &saServer.sin_addr);
	printf("%s\n", strBuffer);
#else
	/* Assign the address */
	saServer.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
	saServer.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
	saServer.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
	saServer.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
#endif
	//
	// Connect the socket
	//
	int nRet;

	nRet = connect(Socket, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		//_pFile->write_bin_string("connect()");
		puts("fail to make connection");
		closesocket(Socket);
		return false;
	}

	nRet = (int)send(Socket, _request, (int)strlen(_request), 0);
	if (nRet == SOCKET_ERROR)
	{
		//_pFile->write_bin_string("send()");
		puts("fail to send()");
		closesocket(Socket);
		return false;
	}

	struct timeval tv; /* timeval and timeout stuff added by davekw7x */
	//int timeouts = 0;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv))
	{
		puts("setsockopt");
		return false;
	}
	BaseSystem::Sleep(500);
	//
	// Receive the file contents and print to stdout
	//
	int nPacketSize = 40960;
	char *buf = PT_Alloc(char, nPacketSize);
	char *pData = buf;
	while(1)
	{
		// Wait to receive, nRet = NumberOfBytesReceived
		nRet = (int)recv(Socket, pData, nPacketSize, 0);
		if (nRet == SOCKET_ERROR)
		{
			//_pFile->write_bin_string("recv()");
			puts("fail to recv()");
			break;
		}

		// Did the server close the connection?
		if (nRet == 0)
			break;
		// Write to stdout
		pData += nRet;
		*pData = NULL;
		break;
	}
	
	if (nRet == SOCKET_ERROR)
	{
		return false;
	}
	
	if(nRet > 0)
		BaseFile::paser_list_seperate(buf, _data_a, "\t");
	PT_Free(buf);

	closesocket(Socket);

	return true;
}

bool BaseHttp::GetHTTP(const char* lpServerName, const char *lpFileName, BaseFile *_pFile)
{
	//
	// Use inet_addr() to determine if we're dealing with a name
	// or an address
	//
	IN_ADDR		iaHost;
	LPHOSTENT	lpHostEntry;

	iaHost.s_addr = inet_addr(lpServerName);
	if (iaHost.s_addr == INADDR_NONE)
	{
		// Wasn't an IP address string, assume it is a name
		lpHostEntry = gethostbyname(lpServerName);
	}
	else
	{
		// It was a valid IP address string
		lpHostEntry = gethostbyaddr((const char *)&iaHost, 
			sizeof(struct in_addr), AF_INET);
	}
	if (lpHostEntry == NULL)
	{
		//_pFile->write_bin_string("gethostbyname()");
		puts("fail to get hostname");
		return false;
	}


	//	
	// Create a TCP/IP stream socket
	//
	SOCKET	Socket;	

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		//_pFile->write_bin_string("socket()"); 
		puts("fail to make socket()");
		return false;
	}

	//
	// Find the port number for the HTTP service on TCP
	//
	LPSERVENT lpServEnt;
	SOCKADDR_IN saServer;

	lpServEnt = getservbyname("http", "tcp");
	if (lpServEnt == NULL)
		saServer.sin_port = htons(80);
	else
		saServer.sin_port = lpServEnt->s_port;


	//
	// Fill in the rest of the server address structure
	//
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);


	//
	// Connect the socket
	//
	int nRet;

	nRet = connect(Socket, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		//_pFile->write_bin_string("connect()");
		puts("fail to make connection");
		closesocket(Socket);
		return false;
	}


	//
	// Format the HTTP request
	//
	char szBuffer[1024];

	sprintf_s(szBuffer, 1024, "GET %s\n", lpFileName);
	////strcpy_s(szBuffer, 1024, "POST /kse_sise/kse_hyun_dailysise.jsp HTTP/1.1\r\n"\
	//				"Host: stock.koscom.co.kr\r\n"\
	//				"User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.0.1) Gecko/2008070208 Firefox/3.0.1\r\n"\
	//				"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"\
	//				"Accept-Language: en-us,en;q=0.5\r\n"\
	//				"Accept-Encoding: gzip,deflate\r\n"\
	//				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"\
	//				"Keep-Alive: 300\r\n"\
	//				"Connection: keep-alive\r\n"\
	//				"Referer: http://stock.koscom.co.kr/kse_sise/kse_hyun_dailysise.jsp\r\n"\
	//				"Cookie: BX=autikm941b4md&b=3&s=kj; JSESSIONID=47593A71AFFA1D179985B5D9BECD4259\r\n"\
	//				"Content-Type: application/x-www-form-urlencoded\r\n"\
	//				"Content-Length: 31\r\n"\
	//				"\r\n"\
	//				"code=A000660&base_date=20080215");

	nRet = (int)send(Socket, szBuffer, (int)strlen(szBuffer), 0);
	if (nRet == SOCKET_ERROR)
	{
		//_pFile->write_bin_string("send()");
		puts("fail to send()");
		closesocket(Socket);	
		return false;
	}

	struct timeval tv; /* timeval and timeout stuff added by davekw7x */
	//int timeouts = 0;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv))
	{
		puts("setsockopt");
		return false;
	}
    BaseSystem::Sleep(500);
	//
	// Receive the file contents and print to stdout
	//
	const int nPacketSize = 1024;
	char *pData = PT_Alloc(char, nPacketSize);
	while(1)
	{
		// Wait to receive, nRet = NumberOfBytesReceived
		nRet = (int)recv(Socket, pData, nPacketSize, 0);
		if (nRet == SOCKET_ERROR)
		{
			//_pFile->write_bin_string("recv()");
			puts("fail to recv()");
			continue;
		}

		// Did the server close the connection?
		if (nRet == 0)
			break;
		// Write to stdout
		_pFile->Write(pData, nRet);
	}
	PT_Free(pData);

	if (nRet == SOCKET_ERROR)
	{
		return false;
	}
	closesocket(Socket);	

	return true;
}

typedef struct __IPBlock__{
	unsigned char	cKey;
	unsigned char	cRange;
	std::vector<__IPBlock__>	*pstlVstBlock;
} ST_IPBlock;
typedef std::vector<__IPBlock__>	STLVstBlock;

static STLVstBlock	*s_pstlVstBlock = NULL;

void filter_add_ip(STLVstBlock *_stlVstBlock, unsigned char *_cIP, int _nDepth)
{
	for(UINT32 i=0; i<_stlVstBlock->size(); i++)
	{
		ST_IPBlock *pBlock	= &_stlVstBlock->at(i);
		if(pBlock->cKey == *(_cIP+_nDepth))
		{
			if(pBlock->cRange > 0)
				return; // already include ip
			if(_nDepth < 3)
				filter_add_ip((STLVstBlock*)pBlock->pstlVstBlock, _cIP, _nDepth+1);
			return;
		}else if(pBlock->cRange > 0 
			&& pBlock->cKey <= *(_cIP+_nDepth)
			&& *(_cIP+_nDepth+4) <= pBlock->cKey + pBlock->cRange)
		{
			return;// already include ip
		}else if(pBlock->cKey > *(_cIP+_nDepth))
		{
			ST_IPBlock block;
			block.cKey	= *(_cIP+_nDepth);
			block.cRange	= *(_cIP+_nDepth+4) - block.cKey;
			block.pstlVstBlock	= NULL;
			if(block.cRange == 0)
			{	
				block.cRange	= 0;
				block.pstlVstBlock	= new STLVstBlock;
				if(_nDepth < 3)
					filter_add_ip((STLVstBlock*)block.pstlVstBlock, _cIP, _nDepth+1);
			}
			_stlVstBlock->insert(_stlVstBlock->begin()+i, block);
			return;
		}
	}
	ST_IPBlock block;
	block.cKey	= *(_cIP+_nDepth);
	block.cRange	= *(_cIP+_nDepth+4) - block.cKey;
	block.pstlVstBlock	= NULL;
	if(block.cRange == 0)
	{	
		block.cRange	= 0;
		block.pstlVstBlock	= new STLVstBlock;
		if(_nDepth < 3)
			filter_add_ip((STLVstBlock*)block.pstlVstBlock, _cIP, _nDepth+1);
	}
	_stlVstBlock->push_back(block);
}

void filter_save(STLVstBlock &_stlVstBlock, BaseFile *_pFile)
{
	size_t nSize	= _stlVstBlock.size();
	_pFile->Write(&nSize, sizeof(size_t));
	_pFile->Write(&_stlVstBlock[0].cKey, sizeof(ST_IPBlock)*(int)_stlVstBlock.size());

	for(unsigned i=0; i<_stlVstBlock.size(); i++)
	{
		if(_stlVstBlock[i].pstlVstBlock)
			filter_save(*_stlVstBlock[i].pstlVstBlock, _pFile);
	}
}

void filter_load_(STLVstBlock *_pstlVstBlock, BaseFile *_pFile)
{
	size_t nSize;
	_pFile->Read(&nSize, sizeof(size_t));
	_pstlVstBlock->resize(nSize);
	_pFile->Read(&_pstlVstBlock->at(0).cKey, sizeof(ST_IPBlock)*(int)nSize);
	for(unsigned i=0; i<nSize; i++)
	{
		if(_pstlVstBlock->at(i).pstlVstBlock)
		{
			_pstlVstBlock->at(i).pstlVstBlock	= new STLVstBlock;
			filter_load_(_pstlVstBlock->at(i).pstlVstBlock, _pFile);
		}
	}
}

bool filter_check_(STLVstBlock &_stlVstBlock, unsigned char *_pcIP, int _nDepth)
{
	for(unsigned i=0; i<_stlVstBlock.size(); i++)
	{
		unsigned char cFrom, cTo, cIP;
		cFrom	= _stlVstBlock[i].cKey;
		cTo		= cFrom + _stlVstBlock[i].cRange;
		cIP		= *(_pcIP+_nDepth);
		if(cTo >= cIP)
		{
			if(cIP == cFrom && _stlVstBlock[i].pstlVstBlock && _nDepth < 3)
				return filter_check_(*_stlVstBlock[i].pstlVstBlock, _pcIP, _nDepth+1);

			if(cIP >= cFrom)
				return true;
			return false;
		}
	}
	return false;
}

void filter_free(STLVstBlock *_pstlVstBlock)
{
	for(unsigned i=0; i<_pstlVstBlock->size(); i++)
	{
		if(_pstlVstBlock->at(i).pstlVstBlock)
			filter_free(_pstlVstBlock->at(i).pstlVstBlock);
	}
	delete _pstlVstBlock;
}

bool BaseHttp::filter_make(const char *_strFilein, const char *_strFileout)
{
	BaseFile	fileFilter;

	STLVstBlock	*pstlVstBlock = new STLVstBlock;
	if(fileFilter.OpenFile(_strFilein, BaseFile::OPEN_READ))
		return false;
	fileFilter.set_asc_seperator("\t.");
	while(fileFilter.read_asc_line())
	{
		unsigned char cIP[8];
		for(unsigned i=0; i<8; i++)
		{
			int nIP;
			fileFilter.read_asc_integer(&nIP);
			cIP[i]	= (unsigned char)nIP;
		}

		filter_add_ip(pstlVstBlock, cIP, 0);
	}
	fileFilter.CloseFile();

	if(fileFilter.OpenFile(_strFileout, BaseFile::OPEN_WRITE))
		return false;
	filter_save(*pstlVstBlock, &fileFilter);
	fileFilter.CloseFile();

	filter_free(pstlVstBlock);

	return true;
}

bool BaseHttp::filter_check(unsigned char *_pcIP)
{
	if(s_pstlVstBlock)
		return filter_check_(*s_pstlVstBlock, _pcIP, 0);
	
	return true;
}

bool BaseHttp::filter_load(const char *_strFilefilter)
{
	BaseFile	fileFilter;

	if(s_pstlVstBlock)
		filter_free(s_pstlVstBlock);

	s_pstlVstBlock	= new STLVstBlock;
	if(fileFilter.OpenFile(_strFilefilter, BaseFile::OPEN_READ))
		return false;
	filter_load_(s_pstlVstBlock, &fileFilter);
	fileFilter.CloseFile();

	return true;
}

void BaseHttp::filter_close()
{
	filter_free(s_pstlVstBlock);
	s_pstlVstBlock	= NULL;
}

unsigned char *BaseHttp::get_public_ip(unsigned char *_strIP)
{
	//FILTER_100	 = 59.128.94.0	;nx server
	//FILTER_RANGE_100	= 255
	//FILTER_101	 = 221.186.67.0	;nx op
	//FILTER_RANGE_101	= 255
	//FILTER_102	 = 112.216.241.0	;nt 245
	//FILTER_RANGE_102	= 255
	//FILTER_103	 = 121.134.30.0	;nd 31
	//FILTER_RANGE_103	= 255
	//FILTER_104	 = 116.127.221.0	;nj op?
	//FILTER_RANGE_104	= 255
	//FILTER_105	 = 122.216.121.0	;nj op 59
	//FILTER_RANGE_105	= 255
	//202.136.150.145 - nt wireless
	//unsigned char cIP[7][4] = {
	//	{59,128,94,255},	// 0 nx server
	//	{221,186,67,0},		// 1 nx op
	//	{112,216,241,255},	// 2 nt
	//	{121,134,30,0},		// 3 nd 
	//	{116,127,221,44},	// 4 nj
	//	{122,216,121,5},	// 5 nj
	//	{202,136,150,145}	// 6 nt wireless
	//};
	//for(int i=0; i<7; i++)
	//{
	//	int x=0;
	//	if(filter_check(cIP[i]))
	//		x++;
	//	else
	//		x--;
	//}

	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------
	char *pBuffer = PT_Alloc(char, 100 * 1024);
	char *pBuf	= pBuffer;

	char server[255];
	char request[255];
	int	nSize = 0;

	strcpy_s(server, 255, "www.findip.kr");

	int nTryCnt = 0;
	{
		sprintf_s(request, 255, "http://www.findip.kr");
		while(0 == (nSize = BaseHttp::GetHTTP(server, request, (char**)&pBuf))
			&& nTryCnt < 4)
		{
            BaseSystem::Sleep(1000);
			nTryCnt++;
		}
	}

	if(nSize == 0)
		return NULL;

	BaseFile paser;
	paser.OpenFile((void*)pBuffer, nSize);

	paser.set_asc_seperator(">");
	paser.set_asc_deletor("\t <");

	//	<h1> 내 아이피 주소(My IP Address) : 175.253.57.99 
	while(paser.read_asc_line())
	{
		paser.read_asc_string(request, 255);
		if(strcmp(request, "h1") == 0)
		{
			paser.set_asc_seperator(":");
			paser.read_asc_string(request, 255);
			paser.read_asc_string(request, 255);

			if(_strIP)
			{
				int nIP[4];
				sscanf_s(request, "%d.%d.%d.%d", &nIP[0], &nIP[1], &nIP[2], &nIP[3]);
				for(int i=0; i<4; i++)	*(_strIP+i)	= (unsigned char)nIP[i];

				return _strIP;
			}
			paser.set_asc_seperator(">");
		}
	}
	PT_Free(pBuffer);
	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------

	return NULL;
}
