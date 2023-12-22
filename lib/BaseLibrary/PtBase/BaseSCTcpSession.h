#pragma once
#include "../PtBase/BaseStateFunc.h"
class BaseSCTcpSession :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(apiEventCast_nIf),
        STDEF_SFENUM(apiReturn_varF),
        STDEF_SFENUM(apiSendFile_strF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCTcpSession();
    ~BaseSCTcpSession();

    PtObjectHeaderInheritance(BaseSCTcpSession);
    STDEF_SC_HEADER(BaseSCTcpSession);

    virtual int Create();
    int apiEventCast_nIf();
    int apiReturn_varF();
    int apiSendFile_strF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    INT64 m_socketfd;
public:
    void url_filter(STLString *_req_p);

};

