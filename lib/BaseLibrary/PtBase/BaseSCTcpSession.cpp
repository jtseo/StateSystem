#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#ifdef WIN32
#include <winsock.h>
#include <windows.h>
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

#include "BaseSCTcpSession.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseSCTcpSession);

#define MAX 1024

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCTcpSession::s_func_hash_a;
const char* BaseSCTcpSession::s_class_name = "BaseSCTcpSession";

int BaseSCTcpSession::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCTcpSession::BaseSCTcpSession()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCTcpSession::FunctionProcessor);
}

int BaseSCTcpSession::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(apiEventCast_nIf);
		STDEF_SFREGIST(apiReturn_varF);
		STDEF_SFREGIST(apiSendFile_strF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCTcpSession::~BaseSCTcpSession()
{

}

void BaseSCTcpSession::init()
{
	BaseStateFunc::init();
}

void BaseSCTcpSession::release()
{
}

BaseStateFuncEx* BaseSCTcpSession::CreatorCallback(const void* _param)
{
	BaseSCTcpSession* bs_func;
	PT_OAlloc(bs_func, BaseSCTcpSession);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCTcpSession::FunctionProcessor);

	return bs_func;
}

int BaseSCTcpSession::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
        //#SF_FuncCallStart
        if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
        //STDEF_SFFUNCALL(Open_varF);
        STDEF_SFFUNCALL(apiEventCast_nIf);
        STDEF_SFFUNCALL(apiReturn_varF);
		STDEF_SFFUNCALL(apiSendFile_strF);
		//#SF_FuncCallInsert
        return 0;
    }
    return ret;
}

int BaseSCTcpSession::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCTcpSession* bs_func = (BaseSCTcpSession*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCTcpSession::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCTcpSession::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	const INT64* socket_n = (const INT64*)paramFallowGet(0);

	m_socketfd = *socket_n;
	return 1;
}

int BaseSCTcpSession::apiEventCast_nIf()
{
    if (!m_socketfd)
        return 0;
#ifdef _WIN32
    SOCKET connfd = (SOCKET)m_socketfd;
#else
	int connfd = (int)m_socketfd;
#endif

    char buff[MAX];
    memset(buff, 0, MAX);

    u_long iMode = 1; // 0 is blocking mode, otrhe blocking mode
	
#ifdef WIN32
	int iResult = ioctlsocket(connfd, FIONBIO, &iMode);
#else
	int iResult = fcntl(connfd, F_SETFL, O_NONBLOCK);
#endif
    //if (iResult != NO_ERROR)
        //printf("ioctlsocket failed with error: %d\n", iResult);

    // read the message from client and copy it in buffer
    STLString stream;
    int cnt = 10;
    do {
        recv(connfd, buff, sizeof(buff), 0);

        stream += buff;
        cnt--;
        if (stream.find('\n') != STLString::npos)
            break;
        BaseSystem::Sleep(1);
    } while (cnt>=0);

    if (stream.find('\n') == STLString::npos)
        return 0;
    // print buffer which contains the client contents
//#ifdef _DEBUG
    {
        BaseFile file;
        file.OpenFile("rev_req.txt", BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8);
        file.write_asc_string(buff, MAX);
        file.write_asc_line();
        file.CloseFile();
    }
//#endif
    //printf("From client: %s\n", buff);
    url_filter(&stream);

    {
        BaseFile parser;
        parser.openParser((char*)stream.c_str(), (UINT32)stream.size());
        parser.set_asc_seperator("=/?& ");
        parser.set_asc_deletor(" /");

        parser.read_asc_line();
        STLString tag;
        if (!parser.read_asc_string(&tag))// GET
            return 0;
        if (!parser.read_asc_string(&tag))// event (SVG2DWG)
            return 0;
        STLString cmd = "PnIDS_";
        cmd += tag;
        if (!parser.read_asc_string(&tag))// filename or filename1
            return 0;
        STLString var = tag;
        if (!parser.read_asc_string(&tag))// filepath (dbcolumn_filename or param1_str
            return 0;
        STLString path = tag;

        BaseDStructureValue* evt = EventMake(STRTOHASH(cmd.c_str()));
        if (var == "filename1")
        {
            if (!parser.read_asc_string(&tag))// filename2
                return 0;
            if (!parser.read_asc_string(&tag))// filepath
                return 0;
            STLString path2 = tag;// param2_str
            evt->set_alloc("PnCompare1Filename_strV", path.c_str());
            evt->set_alloc("PnCompare2Filename_strV", path2.c_str());
        }
        else {
            evt->set_alloc("dbcolumn_filename", path.c_str());
        }
        EventPost(evt);
    }

	return 1;
}
int BaseSCTcpSession::apiReturn_varF()
{
    if (!m_socketfd)
        return 0;
#ifdef _WIN32
    SOCKET connfd = m_socketfd;
#else
	int connfd = (int)m_socketfd;
#endif

    //PnIDS_ParamJson, { 'filename':'%s' }
    const char* json_str = NULL;
    if (!m_state_variable->get(STRTOHASH("PnIDS_ParamJson"), (const void**)&json_str))
        return 0;

    char buff[255];
    SPtDateTime cur;
    BaseSystem::timeCurrent(&cur);
    STLString ret_str;
    ret_str = "HTTP/1.1 200 OK\n";
    ret_str += "Content-Type: application/json\n";
    ret_str += "Server: Microsoft-HTTPAPI/2.0\n";

    ret_str += "Date: "; //Mon, 19 Jul 2021 05:33:34 GMT\n
    ret_str += BaseTime::make_date_time_http(cur.dateTime, buff, 255);
    ret_str += "\n";
    ret_str += "Content-Length: ";

	sprintf(buff, "%zd\n\n", strlen(json_str));
    //sprintf_s(buff, "%zd\n\n", strlen(json_str));
    ret_str += buff;
    ret_str += json_str;

    // and send that buffer to client
    send(connfd, ret_str.c_str(), (int)ret_str.size(), 0);

#ifdef _WIN32
    closesocket(connfd);
#else
	close(connfd);
#endif

	return 1;
}

int hex2int(char _a)
{
    if (_a <= '9')
        return (_a - '0');

    if (_a <= 'F')
        return (_a - 'A') + 10;
    if (_a <= 'f')
        return (_a - 'a') + 10;

    return 0;
}

int hex2int(STLString _str)
{
    int ret = 0;
    int digit = (int)_str.size() - 1;
    int exp = 0;
    for (int i = digit; i >= 0; i--)
    {
        if (exp != 0)
        {
            ret += exp * hex2int(_str[i]);
            exp *= 16;
        }
        else {
            ret = hex2int(_str[i]);
            exp = 16;
        }
    }
    return ret;
}

void BaseSCTcpSession::url_filter(STLString* _req_p)
{
    STLString ret_str, parsing, code;
    parsing = *_req_p;
    do {
        size_t sep = parsing.find_first_of('%');
        if (sep == STLString::npos)
        {
            ret_str += parsing;
            break;
        }
        ret_str += parsing.substr(0, sep);

        code = parsing.substr(sep + 1, 2);
        int code_n = hex2int(code);
        ret_str += (char)code_n;

        parsing = parsing.substr(sep + 3, parsing.size());
    } while (true);
    *_req_p = ret_str;
}

int BaseSCTcpSession::apiSendFile_strF()
{
	if (!m_socketfd)
		return 0;
#ifdef _WIN32
	SOCKET connfd = m_socketfd;
#else
	int connfd = (int)m_socketfd;
#endif

	const char* filename = (const char*)m_param_value;
	STLString buf;
	BaseFile file;
	if(file.OpenFile(filename) == BaseFile::MSG_SUCCESS)
	{
		file.Read(&buf);
		file.CloseFile();
		buf += "\r\n\r\n";
		buf += "\0";
	}
	
	// and send that buffer to client
	send(connfd, buf.c_str(), (int)buf.size(), 0);

#ifdef _WIN32
	closesocket(connfd);
#else
	close(connfd);
#endif

	return 1;
}
//#SF_functionInsert
