#include "../PtBase/stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSCHttpD.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"
#include "../PtBase/BaseSystem.h"

#include <microhttpd.h>

PtObjectCpp(BaseSCHttpD);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCHttpD::s_func_hash_a;
const char* BaseSCHttpD::s_class_name = "BaseSCHttpD";


class SessionHttp{
    MHD_Connection *connection;
    const char *jsonData;
    
    PT_OPTHeader;
public:
    static SessionHttp sm_sample;
    void init(){
        connection = NULL;
        jsonData = NULL;
    }
    virtual void release()
    {
        jsonData = NULL;
    }
    SessionHttp(){
        init();
    }
    ~SessionHttp()
    {
        release();
    }
    void JsonSet(const char *json)
    {
        jsonData = json;
    }
    void ConnectionSet(MHD_Connection *_connection)
    {
        connection = _connection;
    }
    MHD_Connection *ConnectionGet(){
        return connection;
    }
    const char *JsonGet(){
        return jsonData;
    }
    
};

PT_OPTCPP(SessionHttp);
SessionHttp SessionHttp::sm_sample;

int BaseSCHttpD::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCHttpD::BaseSCHttpD():m_disconnectionQueue("httpd Queue")
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCHttpD::FunctionProcessor);
}

int BaseSCHttpD::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(listen_astrF);
		STDEF_SFREGIST(serviceReturn_varF);
		STDEF_SFREGIST(listenStop_nF);
		STDEF_SFREGIST(sessionClose_varF);
		STDEF_SFREGIST(APIRegist_strF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCHttpD::~BaseSCHttpD()
{

}

void BaseSCHttpD::init()
{
	BaseStateFunc::init();
}

void BaseSCHttpD::release()
{
}

BaseStateFuncEx* BaseSCHttpD::CreatorCallback(const void* _param)
{
	BaseSCHttpD* bs_func;
	PT_OAlloc(bs_func, BaseSCHttpD);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCHttpD::FunctionProcessor);

	return bs_func;
}

int BaseSCHttpD::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(listen_astrF);
		STDEF_SFFUNCALL(serviceReturn_varF);
		STDEF_SFFUNCALL(listenStop_nF);
		STDEF_SFFUNCALL(sessionClose_varF);
		STDEF_SFFUNCALL(APIRegist_strF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int BaseSCHttpD::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCHttpD* bs_func = (BaseSCHttpD*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCHttpD::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCHttpD::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

int BaseSCHttpD::listen_astrF()
{
	const char* connect_str = (const char*)m_param_value;
	BaseFile paser;
	char buf[255];
	strcpy_s(buf, 255, connect_str);
	paser.openParser(buf, (UINT32)strlen(connect_str));
	paser.read_asc_line();
	m_server = paser.read_asc_str();
	if(!paser.read_asc_integer(&m_port))
		m_port = 8080;

	BaseSystem::createthread(update_, 0, this);
	m_threadrunning = true;

	return 1;
}

int BaseSCHttpD::serviceReturn_varF()
{
	struct SessionHttp *session;
	struct MHD_Response *response;
    session = (struct SessionHttp *)paramVariablePointGet();
	const char *ret = (const char*)paramFallowGet(0);
	if(session == NULL)
		return 0;
    session->JsonSet(ret);
    m_disconnectionQueue.push(session);
	return 1;
}

int BaseSCHttpD::listenStop_nF()
{
	MHD_stop_daemon(m_daemon);
	m_daemon = NULL;
	while(m_threadrunning)
		BaseSystem::Sleep(10);
	
	return 1;
}

int BaseSCHttpD::sessionClose_varF()
{
	INT64 *con = (INT64*)paramVariablePointGet();
    
    if(!con)
        return 0;
    
    m_disconnectionQueue.push(con);
	return 1;
}

int BaseSCHttpD::APIRegist_strF()
{
    const char *api = (const char*)m_param_value;
    
    m_apiSet.insert(api);
	return 1;
}
//#SF_functionInsert

#include <thread>
#include <mutex>
#include "../PtBase/hashstr.h"

std::mutex mtx;
STLMnInt s_stlMRecord;

bool checkDos(const char *url, const char *method)
{
	std::unique_lock<std::mutex> lock(mtx);

	STLString dummy = url;
	dummy += method;

	int hash = STRTOHASH(dummy.c_str());
	STLMnInt::iterator it;
	it = s_stlMRecord.find(hash);
	if (it == s_stlMRecord.end())
		s_stlMRecord[hash] = 0;

	s_stlMRecord[hash]++;

	if (s_stlMRecord[hash] > 100) {
		
		for (it = s_stlMRecord.begin(); it != s_stlMRecord.end(); it++)
			it->second--;

		return true;
	}

	return false;
}


static enum MHD_Result
answer_to_connection (void *cls, struct MHD_Connection *connection,
					  const char *url, const char *method,
					  const char *version, const char *upload_data,
					  size_t *upload_data_size, void **req_cls)
{
	if (checkDos(url, method))
	{
		printf(".");
		return MHD_NO;
	}

	SessionHttp *session;
    PT_OAlloc(session, SessionHttp);
    session->ConnectionSet(connection);
    
    PT_ThreadStart(THTYPE_BASE_TCP_SESSION);
	BaseSCHttpD* httpd = (BaseSCHttpD*)cls;
	//const char *page = "[ { \"userId\": 1, \"id\": 1, \"title\": \"provident\", \"body\": \"quia\" } ]";
	static int reqcnt = 0;
	printf("requested: %d, %s: %s\n", reqcnt++, method, url);
	
	//const char *page = "[ { \"userId\": 1, \"id\": 1, \"title\": \"provident\", \"body\": \"quia\" } ]";
	
    g_SendMessage(LOG_MSG, "TCP Request Receive %s", url);
	STLString command = "API_";
	command += (const char*)(url+1);
	//httpd->sessionAdd(connection); // in thread environment, it's not support.
    if(httpd->apiCheck(command.c_str()))
    {
        BaseDStructureValue *evt = httpd->EventMakeThread(STRTOHASH(command.c_str()));
        evt->set_point("API_connection", session);
        evt->set_alloc("API_method", method);
        evt->set_alloc("JsonParam_strV", upload_data);
        httpd->EventPostThread(evt);
        
        while(httpd->sessionDisconnectTop() != session)
            BaseSystem::Sleep(10);
        
        httpd->sessionDissconnectPop();
    }
        
    struct MHD_Response *response = NULL;
    
	if (session->JsonGet() != NULL)
	{
		response = MHD_create_response_from_buffer(strlen(session->JsonGet()), (void*)session->JsonGet(), MHD_RESPMEM_PERSISTENT);
		MHD_queue_response(connection, MHD_HTTP_OK, response);
		MHD_destroy_response(response);
	}
    
    PT_OFree(session);
    session = NULL;
    
    PT_ThreadEnd(THTYPE_BASE_TCP_SESSION);
	return MHD_YES;
}


bool BaseSCHttpD::apiCheck(const char* _api)
{
    if(m_apiSet.find(_api) == m_apiSet.end())
        return false;
    return true;
}


void *BaseSCHttpD::sessionDisconnectTop()
{
    void *point = m_disconnectionQueue.top();
    if(point == NULL)
        return NULL;
    return point;
}

void BaseSCHttpD::sessionDissconnectPop()
{
    m_disconnectionQueue.pop();
}

DEF_ThreadCallBack(BaseSCHttpD::update)
//void __cdecl BaseNetManager::update(void *_pManager)
{
	PT_ThreadStart(THTYPE_BASE_TCP);
	BaseSCHttpD* httpd = (BaseSCHttpD*)_pParam;

	struct MHD_Daemon *daemon;

	BaseFile loader;

	STLString key, cert;
	if(!loader.OpenFile("privkey.pem", BaseFile::OPEN_READ))
	{
		loader.Read(&key);
		loader.CloseFile();
	}

	if(!loader.OpenFile("fullchain.pem", BaseFile::OPEN_READ))
	{
		loader.Read(&cert);
		loader.CloseFile();
	}
	
	if(key.empty())
	{
		daemon = MHD_start_daemon (
                               MHD_USE_INTERNAL_POLLING_THREAD
								, 8080, NULL, NULL,
							   &answer_to_connection, (void *)httpd,
                               MHD_OPTION_END);
	}else{
		daemon = MHD_start_daemon (
                               MHD_USE_SSL |
								MHD_USE_INTERNAL_POLLING_THREAD
								, 8080, NULL, NULL,
							   &answer_to_connection, (void *)httpd,
                               MHD_OPTION_HTTPS_MEM_KEY, key.c_str(),
                               MHD_OPTION_HTTPS_MEM_CERT, cert.c_str(),
                               MHD_OPTION_END);
	}
	if (NULL == daemon) {
		printf("fail to open API posrt\n");
		DEF_ThreadReturn;
	}

	httpd->damonSet(daemon);
	
	do{
		BaseSystem::Sleep(10);
	}while(httpd->damonGet());
	httpd->threadStop();
    PT_ThreadEnd(THTYPE_BASE_TCP);
	DEF_ThreadReturn;
}
