#pragma once
#include "../PtBase/BaseStateFunc.h"
class BaseSCMysql :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(querySelect_strF),
        STDEF_SFENUM(queryUpdate_strF),
        STDEF_SFENUM(queryDelete_strF),
        STDEF_SFENUM(queryAdd_strF),
        STDEF_SFENUM(querySearch_strF),
        STDEF_SFENUM(querySelectPage_strF),
        STDEF_SFENUM(keyAdd_astrF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCMysql();
    ~BaseSCMysql();
    //static int GetObjectId();

    PtObjectHeaderInheritance(BaseSCMysql);
	STDEF_SC_HEADER(BaseSCMysql);

    //static int FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status);
    //static int StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func);
    //static BaseStateFuncEx* CreatorCallback(const void* _param);

    virtual int Create();
    int querySelect_strF();
    int queryUpdate_strF();
    int queryDelete_strF();
    int queryAdd_strF();
    int querySearch_strF();
    int querySelectPage_strF();
    int keyAdd_astrF();
    //#SF_FuncHeaderInsert

    // User defin area from here
	const char *db_get() { return m_db.c_str(); }
	const char *server_get() { return m_server.c_str(); }
	const char *user_get() { return m_userid.c_str(); }
	const char *password_get() { return m_password.c_str(); }
	
protected:
	STLString	m_db;
	STLString	m_server;
	STLString	m_userid;
	STLString	m_password;
	
public:
	bool key_add(const char *_str);
	bool key_is(const char *_str);
protected:
	STLSString	m_keys;
    
public:
    static DEF_ThreadCallBack(update);
};

