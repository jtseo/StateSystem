#pragma once
#include "../PtBase/BaseStateFunc.h"

class DevCamera :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    DevCamera();
    ~DevCamera();

    PtObjectHeaderInheritance(DevCamera);
    STDEF_SC_HEADER(DevCamera);

    virtual int Create();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

