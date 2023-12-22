#pragma once
#include "../PtBase/BaseStateFunc.h"
class BaseSCTcp :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(open_astrF),
        STDEF_SFENUM(close_nF),
        STDEF_SFENUM(connect_astrF),
        STDEF_SFENUM(apiReqCast_varIf),
        STDEF_SFENUM(apiReturn_varF),
        STDEF_SFENUM(apiReceive_nIf),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCTcp();
    ~BaseSCTcp();

    PtObjectHeaderInheritance(BaseSCTcp);
    STDEF_SC_HEADER(BaseSCTcp);
    virtual int Create();
    int open_astrF();
    int close_nF();
    int connect_astrF();
    int apiReqCast_varIf();
    int apiReturn_varF();
    int apiReceive_nIf();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    STLString m_server;
	int	m_port;

public:
    static DEF_ThreadCallBack(update);
    void acceptCast(INT64 _socket);
    STLString serverGet() { return m_server; }
	int portGet() { return m_port; }

};

