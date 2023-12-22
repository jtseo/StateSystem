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

int BaseSCHttpD::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCHttpD::BaseSCHttpD()
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
	struct MHD_Connection *connection;
	struct MHD_Response *response;
	connection = (struct MHD_Connection *)paramVariablePointGet();
	const char *ret = (const char*)paramFallowGet(0);
	if(ret == NULL)
		return 0;
	const char *page = "[ { \"userId\": 1, \"id\": 1, \"title\": \"provident\", \"body\": \"quia\" } ]";
	response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
	MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
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
	void *con = paramVariablePointGet();
	for(int i=0; i<m_sessions.size(); i++)
		if(con == m_sessions[i])
		{
			m_sessions.erase(m_sessions.begin()+i);
			return 1;
		}
	return 0;
}
//#SF_functionInsert

static enum MHD_Result
answer_to_connection (void *cls, struct MHD_Connection *connection,
					  const char *url, const char *method,
					  const char *version, const char *upload_data,
					  size_t *upload_data_size, void **req_cls)
{
	BaseSCHttpD* httpd = (BaseSCHttpD*)cls;
	//const char *page = "[ { \"userId\": 1, \"id\": 1, \"title\": \"provident\", \"body\": \"quia\" } ]";
	
	printf("requested\n");
	
	const char *page = "[ { \"userId\": 1, \"id\": 1, \"title\": \"provident\", \"body\": \"quia\" } ]";
	
	//struct MHD_Response *response;
	//response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
	//MHD_queue_response(connection, MHD_HTTP_OK, response);
	//MHD_destroy_response(response);
	STLString command = "API_";
	command += (const char*)(url+1);
	httpd->sessionAdd(connection);
	BaseDStructureValue *evt = httpd->EventMakeThread(STRTOHASH(command.c_str()));
	evt->set_point("API_connection", connection);
	evt->set_alloc("API_method", method);
	httpd->EventPostThread(evt);
	
	while(httpd->sessionGet(connection))
		BaseSystem::Sleep(10);

	return MHD_YES;
}

bool BaseSCHttpD::sessionGet(void *_session)
{
	for(int i=0;i < m_sessions.size(); i++)
		if(_session == m_sessions[i])
			return true;
	return false;
}

void BaseSCHttpD::sessionAdd(void *_session)
{
	m_sessions.push_back(_session);
}

DEF_ThreadCallBack(BaseSCHttpD::update)
//void __cdecl BaseNetManager::update(void *_pManager)
{
	PT_ThreadStart(THTYPE_BASE_TCP);
	BaseSCHttpD* httpd = (BaseSCHttpD*)_pParam;

	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, 8080, NULL, NULL,
							   &answer_to_connection, (void *)httpd, MHD_OPTION_END);
	if (NULL == daemon) {
		return NULL;
	}

	httpd->damonSet(daemon);
	
	do{
		BaseSystem::Sleep(10);
	}while(httpd->damonGet());
	httpd->threadStop();
	return NULL;
}
