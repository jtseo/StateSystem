#pragma once
#include "../PtBase/BaseStateFunc.h"

class BaseSCHttpDSession:
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(ApiReturn_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCHttpDSession();
    ~BaseSCHttpDSession();

    PtObjectHeaderInheritance(BaseSCHttpDSession);
    STDEF_SC_HEADER(BaseSCHttpDSession);

    virtual int Create();
    int ApiReturn_varF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

