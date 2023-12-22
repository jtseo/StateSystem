#pragma once
#include "../PtBase/BaseStateFunc.h"
class BaseSCDBFile :
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
        STDEF_SFENUM(read_strF),
        STDEF_SFENUM(update_strF),
        STDEF_SFENUM(remove_strF),
        STDEF_SFENUM(create_strF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCDBFile();
    ~BaseSCDBFile();

    PtObjectHeaderInheritance(BaseSCDBFile);
    STDEF_SC_HEADER(BaseSCDBFile);

    virtual int Create();
    int open_astrF();
    int close_nF();
    int connect_astrF();
    int apiReqCast_varIf();
    int apiReturn_varF();
    int read_strF();
    int update_strF();
    int remove_strF();
    int create_strF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    
public:
    static DEF_ThreadCallBack(update);
    void acceptCast(INT64 _socket);
};

