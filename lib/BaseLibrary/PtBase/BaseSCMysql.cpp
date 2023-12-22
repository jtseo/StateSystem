#include "stdafx.h"
#include <mysql.h>
//#include <sys/ioctl.h>
#include <inttypes.h>

//#include "../PtBase/BaseStateFunc.h"
//#include "../PtBase/BaseObject.h"

#include "BaseSCMysql.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseSystem.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"
#include "BaseJson.h"

PtObjectCpp(BaseSCMysql);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCMysql::s_func_hash_a;
const char* BaseSCMysql::s_class_name = "BaseSCMysql";

int BaseSCMysql::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCMysql::BaseSCMysql()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCMysql::FunctionProcessor);
}

int BaseSCMysql::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		STDEF_SFREGIST(querySelect_strF);
		STDEF_SFREGIST(queryUpdate_strF);
		STDEF_SFREGIST(queryDelete_strF);
		STDEF_SFREGIST(queryAdd_strF);
		STDEF_SFREGIST(querySearch_strF);
		STDEF_SFREGIST(querySelectPage_strF);
		STDEF_SFREGIST(keyAdd_astrF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCMysql::~BaseSCMysql()
{

}

void BaseSCMysql::init()
{
	BaseStateFunc::init();
}

MYSQL *s_sql = NULL;

void BaseSCMysql::release()
{
	mysql_library_end();
}

BaseStateFuncEx* BaseSCMysql::CreatorCallback(const void* _param)
{
	BaseSCMysql* bs_func;
	PT_OAlloc(bs_func, BaseSCMysql);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCMysql::FunctionProcessor);

	return bs_func;
}

int BaseSCMysql::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		STDEF_SFFUNCALL(querySelect_strF);
		STDEF_SFFUNCALL(queryUpdate_strF);
		STDEF_SFFUNCALL(queryDelete_strF);
		STDEF_SFFUNCALL(queryAdd_strF);
		STDEF_SFFUNCALL(querySearch_strF);
		STDEF_SFFUNCALL(querySelectPage_strF);
		STDEF_SFFUNCALL(keyAdd_astrF);
		//#SF_FuncCallInsert

		return 0;
	}

	return ret;
}

int BaseSCMysql::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCMysql* bs_func = (BaseSCMysql*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCMysql::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}


int BaseSCMysql::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
	
	const char *serv, *user, *pass, *db;

	serv = (const char*)paramFallowGet(0);
	user = (const char*)paramFallowGet(1);
	pass = (const char*)paramFallowGet(2);
	db = (const char*)paramFallowGet(3);
	
	if(!serv || !user || !pass || !db)
		return 0;
	
	m_server = serv;
	m_userid = user;
	m_password = pass;
	m_db = db;
	//const char ** argv = {"sqlsample", ""};
	if(mysql_library_init(0, NULL, NULL))
	{
		g_SendMessage(LOG_MSG, "Fail to initialize for mysql. %s");
		printf("mysql_library_init: %s %d",mysql_error(NULL),mysql_errno(NULL));
		return 0;
	}

	return 1;
}

typedef enum {
	DB_NONE = 0,
	DB_READ = 1,
	DB_DELETE = 2,
	DB_INSERT = 3,
	DB_UPDATE = 4
} Em_query_type;

class ParamQuery
{
public:
	PtObjectHeader(ParamQuery);
	PT_OPTHeader;
public:
	int stateSerial;
	STLString query;
	Em_query_type type; //
	BaseSCMysql *sc_mysql;
};

PtObjectCpp(ParamQuery);
PT_OPTCPP(ParamQuery);

DEF_ThreadCallBack(BaseSCMysql::update)
{
	PT_ThreadStart(THTYPE_DBQUERY);
	ParamQuery *st_query = (ParamQuery*)_pParam;
	BaseSCMysql *scSql = (BaseSCMysql*)st_query->sc_mysql;

	//do{
		//CORUM_PROFILE_SUB(UPDATE);
	MYSQL *con = s_sql;
	con = mysql_init(s_sql);

	bool err = false;
	if (con == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		err = true;
	}

	g_SendMessage(LOG_MSG, "query_start %p %s\n", st_query->query.c_str(), st_query->query.c_str());
	if (!err && mysql_real_connect(con, scSql->server_get(), scSql->user_get(), scSql->password_get(),
			scSql->db_get(), 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		err = true;
	}

	if (!err && mysql_query(con, st_query->query.c_str()))
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		err = true;
	}
	
	MYSQL_FIELD   *field;
	MYSQL_ROW    row;
	unsigned int  i;
	MYSQL_RES *res_set;

	/* the query succeeded; determine whether or not it returns data */

	int hash = STRTOHASH("DatabaseResultColumn");
	int hashEnd = STRTOHASH("DatabaseResultColumEnd");
	
	BaseDStructureValue *evt = NULL;
	if(!err)
	{
		res_set = mysql_store_result(con);
		int id = st_query->sc_mysql->obj_serial_get();
		while (res_set && (row = mysql_fetch_row (res_set)) != NULL)
		{
			evt = st_query->sc_mysql->EventMakeThread(hash);
			mysql_field_seek (res_set, 0);
			BaseJson json;
			for (i = 0; i < mysql_num_fields (res_set); i++)
			{
				
				field = mysql_fetch_field (res_set);
				if(i == 0 && IS_NUM(field->type) && strcmp(field->name, "ct_id") == 0)
				{
					INT64 ct_id;
					ct_id = BaseFile::get_longlong(row[i]);
					evt->add_alloc("dbcolumn_ct_id", &ct_id);
				}
				if(row[i])
					json[field->name] = (const char*)row[i];
			}
			evt->set_alloc("TempString_strV", json.get().c_str());
			st_query->sc_mysql->EventPostThread(evt, st_query->stateSerial);
		}
	}
	
	if(con)
		mysql_close(con);
	mysql_thread_end();
	
	evt = st_query->sc_mysql->EventMakeThread(hashEnd);
	st_query->sc_mysql->EventPostThread(evt, st_query->stateSerial);
	
	//}while(!s_mpool.is_terminated());

	//CORUM_PROFILE_REPORT(NULL);

	PT_OFree(st_query);
	
	PT_ThreadEnd(THTYPE_DBQUERY);

	BaseSystem::endthread();
	//_endthread();
	DEF_ThreadReturn;
}

int BaseSCMysql::querySelect_strF()
{
	ParamQuery *q;
	PT_OAlloc(q, ParamQuery);
	q->stateSerial = m_state_p->obj_serial_get();
	const char *query = (const char*)m_param_value;
	if(!query)
		return 0;
	const char *param = (const char*)paramFallowGet(0);
	const char *constraint = (const char*)paramFallowGet(1);
	
	STLVString mustHave;
	if(constraint)
		BaseFile::paser_list_seperate(constraint, &mustHave, ",");
	
	if(!param)
		return 0;
	STLString str = param;
	BaseJson json;
	json.set(str.c_str());
	
	q->query = query;
	STLMstrStr::const_iterator it = json.begin();
	if(it != json.end())
		q->query += " where";
	bool first = true;
	for(;it!=json.end();it++)
	{
		if(!first)
			q->query += " and";
		else
			first = false;
		
		for(int i=0; i<mustHave.size(); i++)
		{
			if(it->first == mustHave[i])
			{
				mustHave.erase(mustHave.begin()+i);
				break;
			}
		}
		
		if(key_is(it->first.c_str()))
		{
			q->query += " " + it->first;
			q->query += "=";
			q->query += it->second;
		}else if(it->first == "password"){
			q->query += " password=SHA2('";
			q->query += it->second;
			q->query += "',256)";
		}else{
			q->query += " ";
			q->query += it->first;
			q->query += " = '";
			q->query += it->second;
			q->query += "'";
		}
	}
	
	if(!mustHave.empty())
		return 0;
	
	q->sc_mysql = this;
	q->type = DB_READ;
	
	g_SendMessage(LOG_MSG, "query_lunch %p %s\n", q->query.c_str(), q->query.c_str());
	if(BaseSystem::createthread(update_, 0, q) == -1){
		return 0;
	}
	
	return 1;
}
int BaseSCMysql::queryUpdate_strF()
{
	ParamQuery *q;
	PT_OAlloc(q, ParamQuery);
	q->stateSerial = m_state_p->obj_serial_get();
	
	const char *query = (const char*)m_param_value;
	if(!query)
		return 0;
	
	const char *param = (const char*)paramFallowGet(0);
	if(!param)
		return 0;
	BaseJson json;
	json.set(param);
	
	STLString ct_id_str = json["ct_id"];
	if(ct_id_str.empty())
		return 0;
	
	q->query = query;
	STLMstrStr::const_iterator it = json.begin();
	if(it != json.end())
		q->query += " set";
	bool first = true;
	for(;it!=json.end();it++)
	{
		if(!first && it->first != "ct_id")
			q->query += " ,";
		if(it->first == "ct_id")
		{
		}else{
			first = false;
			q->query += " ";
			q->query += it->first;
			q->query += " = '";
			q->query += it->second;
			q->query += "'";
		}
	}
	
	q->query += " where ct_id = ";
	q->query += ct_id_str;
	
	q->sc_mysql = this;
	q->type = DB_UPDATE;
	
	g_SendMessage(LOG_MSG, "query_lunch %p %s\n", q->query.c_str(), q->query.c_str());
	if(BaseSystem::createthread(update_, 0, q) == -1){
		return 0;
	}
	
	return 1;
}
int BaseSCMysql::queryDelete_strF()
{
	ParamQuery *q;
	PT_OAlloc(q, ParamQuery);
	q->stateSerial = m_state_p->obj_serial_get();
	
	const char *query = (const char*)m_param_value;
	if(!query)
		return 0;
	
	const char *param = (const char*)paramFallowGet(0);
	if(!param)
		return 0;
	BaseJson json;
	json.set(param);
	
	STLString ct_id_str = json["ct_id"];
	if(ct_id_str.empty())
		return 0;
	
	q->query = query;
	q->query += " where ct_id = ";
	q->query += ct_id_str;
	
	q->sc_mysql = this;
	q->type = DB_DELETE;
	
	g_SendMessage(LOG_MSG, "query_lunch %p %s\n", q->query.c_str(), q->query.c_str());
	if(BaseSystem::createthread(update_, 0, q) == -1){
		return 0;
	}
	
	return 1;
}
int BaseSCMysql::queryAdd_strF()
{
	ParamQuery *q;
	PT_OAlloc(q, ParamQuery);
	q->stateSerial = m_state_p->obj_serial_get();
	
	const char *query = (const char*)m_param_value;
	if(!query)
		return 0;
	
	const char *param = (const char*)paramFallowGet(0);
	if(!param)
		return 0;
	BaseJson json;
	json.set(param);
	
	//STLString ct_id_str = json["ct_id"];
	//if(ct_id_str.empty())
	//	return 0;
	
	q->query = query;
	STLMstrStr::const_iterator it = json.begin();
	if(it == json.end())
		return 0;
	
	q->query += " (";
	for(;it!=json.end();it++)
	{
		if(it != json.begin())
			q->query += " ,";
		q->query += " ";
		q->query += it->first;
	}
	
	q->query += ") values (";
	it = json.begin();
	for(;it!=json.end();it++)
	{
		if(it != json.begin())
			q->query += " ,";
		q->query += "\'";
		q->query += it->second;
		q->query += "\'";
	}
	q->query += ")";
	
	q->sc_mysql = this;
	q->type = DB_INSERT;
	
	g_SendMessage(LOG_MSG, "query_lunch %p %s\n", q->query.c_str(), q->query.c_str());
	if(BaseSystem::createthread(update_, 0, q) == -1){
		return 0;
	}
	
	return 1;
}

STLString queryColumnGet(bool _key, const char *_name, const char *_word)
{
	STLString str = " ";
	str += _name;
	if(_key)
	{
		str += " = ";
		str += _word;
		str += " ";
	}else{
		str += " like '%";
		str += _word;
		str += "%' ";
	}
	return str;
}

int BaseSCMysql::querySearch_strF()
{
	ParamQuery *q;
	PT_OAlloc(q, ParamQuery);
	q->stateSerial = m_state_p->obj_serial_get();
	const char *query = (const char*)m_param_value;
	if(!query)
		return 0;
	const char *param = (const char*)paramFallowGet(0);
	if(!param)
		return 0;
	BaseJson json;
	json.set(param);
	
	q->query = query;
	STLMstrStr::const_iterator it = json.begin();
	if(it != json.end())
		q->query += " where";
	bool first = true;
	for(;it!=json.end();it++)
	{
		if(!first)
			q->query += " and";
		else
			first = false;
		q->query += queryColumnGet(key_is(it->first.c_str()), it->first.c_str(), it->second.c_str());
	}
	
	q->sc_mysql = this;
	q->type = DB_READ;
	
	g_SendMessage(LOG_MSG, "query_lunch %p %s\n", q->query.c_str(), q->query.c_str());
	if(BaseSystem::createthread(update_, 0, q) == -1){
		return 0;
	}
	
	return 1;
}

int BaseSCMysql::querySelectPage_strF()
{
	ParamQuery *q;
	PT_OAlloc(q, ParamQuery);
	q->stateSerial = m_state_p->obj_serial_get();
	const char *query = (const char*)m_param_value;
	if(!query)
		return 0;
	const char *param = (const char*)paramFallowGet(0);
	if(!param)
		return 0;
	
	const int *start = (const int*)paramFallowGet(1);
	const int *len = (const int*)paramFallowGet(2);
	STLString str = param;
	BaseJson json;
	json.set(str.c_str());
	
	STLString queryIn;
	queryIn = query;
	STLMstrStr::const_iterator it = json.begin();
	if(it != json.end())
		queryIn += " where";
	bool first = true;
	for(;it!=json.end();it++)
	{
		if(!first)
			queryIn += " and";
		else
			first = false;
		if(it->first == "ct_id")
		{
			queryIn += " ct_id = ";
			queryIn += it->second;
		}else{
			queryIn += " ";
			queryIn += it->first;
			queryIn += " = '";
			queryIn += it->second;
			queryIn += "'";
		}
	}
	
	q->query = "select R1.* from (";
	q->query += queryIn;
	q->query += ") R1 limit ";
	q->query += BaseFile::to_str(*start);
	q->query += ",";
	q->query += BaseFile::to_str(*len);
	
	q->sc_mysql = this;
	q->type = DB_READ;
	
	g_SendMessage(LOG_MSG, "query_lunch %p %s\n", q->query.c_str(), q->query.c_str());
	if(BaseSystem::createthread(update_, 0, q) == -1){
		return 0;
	}
	
	return 1;
}

bool BaseSCMysql::key_add(const char *_str)
{
	m_keys.insert(_str);
	return true;
}

bool BaseSCMysql::key_is(const char *_str)
{
	if(m_keys.find(_str) == m_keys.end())
		return false;
	return true;
}

int BaseSCMysql::keyAdd_astrF()
{
	STLVString list;
	BaseFile::paser_list_seperate((const char*)m_param_value, &list, ",");
	STLVString::iterator it = list.begin();
	for(;it != list.end(); it++)
		key_add((*it).c_str());
	return 1;
}
//#SF_functionInsert
