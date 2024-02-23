#include "stdafx.h"

#ifdef WIN32
#include <winsock.h>
#include <windows.h>

typedef	int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#endif

//#include <sys/ioctl.h>
#include <inttypes.h>

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseSystem.h"

#include "BaseSCTcp.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"
#include "../PtBase/BaseSocket.h"

PtObjectCpp(BaseSCTcp);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCTcp::s_func_hash_a;
const char* BaseSCTcp::s_class_name = "BaseSCTcp";

int BaseSCTcp::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCTcp::BaseSCTcp()
{
    printf("printed");
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCTcp::FunctionProcessor);
}

int BaseSCTcp::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(open_astrF);
		STDEF_SFREGIST(close_nF);
		STDEF_SFREGIST(connect_astrF);
		STDEF_SFREGIST(apiReqCast_varIf);
		STDEF_SFREGIST(apiReturn_varF);
		STDEF_SFREGIST(apiReceive_nIf);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCTcp::~BaseSCTcp()
{

}

void BaseSCTcp::init()
{
	BaseStateFunc::init();
}

void BaseSCTcp::release()
{
}

BaseStateFuncEx* BaseSCTcp::CreatorCallback(const void* _param)
{
	BaseSCTcp* bs_func;
	PT_OAlloc(bs_func, BaseSCTcp);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCTcp::FunctionProcessor);

	return bs_func;
}

int BaseSCTcp::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
        //#SF_FuncCallStart
        if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
        //STDEF_SFFUNCALL(Open_varF);
        STDEF_SFFUNCALL(open_astrF);
        STDEF_SFFUNCALL(close_nF);
        STDEF_SFFUNCALL(connect_astrF);
        STDEF_SFFUNCALL(apiReqCast_varIf);
        STDEF_SFFUNCALL(apiReturn_varF);
		STDEF_SFFUNCALL(apiReceive_nIf);
		//#SF_FuncCallInsert
        return 0;
    }
    return ret;
}

int BaseSCTcp::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCTcp* bs_func = (BaseSCTcp*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCTcp::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCTcp::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

    if (!m_manager_p->variable_define(STRTOHASH("PnIDS_ParamPoint"), TYPE_INT64))
        return 0;

	return 1;
}

#define PORT 30001
int BaseSCTcp::open_astrF()
{
    const char* connect_str = (const char*)m_param_value;
	BaseFile paser;
	char buf[255];
	strcpy_s(buf, 255, connect_str);
	paser.openParser(buf, (UINT32)strlen(connect_str));
	paser.read_asc_line();
	m_server = paser.read_asc_str();
	if(!paser.read_asc_integer(&m_port))
		m_port = PORT;

    static bool thread_start = false;

    if (thread_start)
        return 0;
    thread_start = true;
    BaseSystem::createthread(update_, 0, this);

	return 1;
}

int BaseSCTcp::close_nF()
{
	return 1;
}

#define MAX 1024
#define SA struct sockaddr

DEF_ThreadCallBack(BaseSCTcp::update)
//void __cdecl BaseNetManager::update(void *_pManager)
{
    PT_ThreadStart(THTYPE_BASE_TCP);
    BaseSCTcp* ScTcp_p = (BaseSCTcp*)_pParam;

    struct sockaddr_in servaddr, cli;

#ifdef _WIN32
	WSADATA wsa;
	//printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
	//printf("Failed. Error Code : %d", WSAGetLastError());
		return;
	//return 1;
	}
	SOCKET sockfd, connfd;
#else
	int sockfd, connfd;
#endif

    
    int len;

    //printf("Initialised.\n");C
    bool end = false;
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        //exit(0);
        end = true;
    }
    //else printf("Socket successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));

	if(!BaseSocket::address_convert(&servaddr, ScTcp_p->serverGet().c_str(), ScTcp_p->portGet()))
	{
		printf("fail to make address.");
		end = true;
	}

    // Binding newly created socket to given IP and verification
    if (!end && (bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        //exit(0);
        end = true;
    }
    //else printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if (!end && (listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        //exit(0);
        end = true;
    }
    //else printf("Server listening..\n");
    len = sizeof(cli);
	if(!end)
		printf("Now listen %s:%d", ScTcp_p->serverGet().c_str(), ScTcp_p->portGet());

    // Accept the data packet from client and verification

    do {
        if (end)
            break;

		connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len);
        if (connfd < 0) {
            //printf("server acccept failed...\n");
            break;
        }
        //else printf("server acccept the client...%" PRIu64 "\n", (UINT64)connfd);

        ScTcp_p->acceptCast(connfd);
    } while (true);
    
    // After chatting close the socket
#ifdef _WIN32
    closesocket(sockfd);
#else
	close(sockfd);
	return 0;
#endif
}


DEF_ThreadCallBack(threadClient)
//void __cdecl BaseNetManager::update(void *_pManager)
{
	PT_ThreadStart(THTYPE_BASE_TCP);
	BaseSCTcp* pTcp = (BaseSCTcp*)_pParam;

	char buff[MAX];
	memset(buff, 0, MAX);

	// read the message from client and copy it in buffer
	int cnt = 0;
	STLString accum;
	do {
		cnt = recv(pTcp->socketGet(), buff, (long)sizeof(buff), 0);
		if (cnt > 0) {
			buff[cnt] = 0;
			accum += buff;
		}
		BaseSystem::Sleep(100);
	} while (cnt == 0);

	do {
		cnt = recv(pTcp->socketGet(), buff, (long)sizeof(buff), 0);
		if (cnt > 0) {
			buff[cnt] = 0;
			accum += buff;
		}
		BaseSystem::Sleep(100);
	} while (cnt > 0);

	printf("From client: %s\t To client \n", accum.c_str());

	BaseDStructureValue *evt = pTcp->EventMakeThread(STRTOHASH("TCPReceive"));
	evt->set_alloc("param1_str", accum.c_str());
	pTcp->EventPostThread(evt);

	//TCPReceive
	//param1_str
	// After chatting close the socket
#ifdef _WIN32
	closesocket(pTcp->socketGet());
#else
	close(pTcp->socketGet());
#endif

	PT_ThreadEnd(THTYPE_BASE_TCP);
}


int BaseSCTcp::connect_astrF()
{
    UINT32 timeStart = BaseSystem::timeGetTime();
    const char* connect_str = (const char*)m_param_value;

    BaseFile paser;
    paser.set_asc_seperator(":, ");
    paser.set_asc_deletor(":, ");
    paser.openParser((char*)connect_str, (UINT32)strlen(connect_str));
    paser.read_asc_line();
    char server_str[255];
    int port_n;

    paser.read_asc_string(server_str, 255);
    if (!paser.read_asc_integer(&port_n))
        return 0;

    struct sockaddr_in servaddr;
#ifdef _WIN32
    WSADATA wsa;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 0;
        //return 1;
    }
	
#endif
	
    printf("Initialised.\n");

    // socket create and verification
	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1) {
        printf("socket creation failed...\n");
        return 0;
    }
    else printf("Socket successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));
	
	struct hostent* hp;
	
	unsigned int addr;
	struct sockaddr_in server;
	if (inet_addr(server_str) == INADDR_NONE)
	{
		hp = gethostbyname(server_str);
	}
	else
	{
		addr = inet_addr(server_str);
		hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	}

	if (hp == NULL)
	{
#ifdef _WIN32
		closesocket(m_sockfd);
		return NULL;
#else
		close(m_sockfd);
		return -1;
#endif
	}

#ifdef _WIN32
	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
#else
	char strBuffer[255];
	unsigned add[4];
	for(int i=0; i<4; i++)
		add[i] = (unsigned char)hp->h_addr_list[0][i];
	sprintf_s(strBuffer, 255, "%u.%u.%u.%u"
			  , add[0], add[1], add[2], add[3]);
	inet_aton(strBuffer, &server.sin_addr);
#endif
	server.sin_family = AF_INET;
	server.sin_port = htons(port_n);
	
    if (connect(m_sockfd, (struct sockaddr*)&server, sizeof(server)))
    {
        printf("Client: connect() - Failed to connect.\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }
    else
    {
        printf("Client: connect() is OK.\n");
        printf("Client: Can start sending and receiving data...\n");
    }

	const char* packet = (const char*)paramFallowGet(0);
	if (!packet)
		return 0;

	STLString sendPacket = packet;
	sendPacket += "<EOF>";

	send(m_sockfd, sendPacket.c_str(), sendPacket.size(), 0);

	//// if msg contains "Exit" then server exit and chat ended.
	//if (strncmp("exit", buff, 4) == 0) {
	//    printf("Server Exit...\n");
	//    break;
	//}

	BaseSystem::createthread(threadClient_, 0, this);

	return 1;
}

void BaseSCTcp::acceptCast(INT64 _socket)
{
    const char* paramPoint_str = "PnIDS_ParamPoint";
    BaseDStructureValue *evt = EventMake(STRTOHASH("NetSocketOpen"));
    evt->set_alloc(paramPoint_str, (const void*)&_socket);
    EventPost(evt);
}

int BaseSCTcp::apiReqCast_varIf()
{
    return 1;
}
int BaseSCTcp::apiReturn_varF()
{
	return 1;
}

int BaseSCTcp::apiReceive_nIf()
{
	return 1;
}
//#SF_functionInsert
