#pragma once
#include "../PtBase/BaseStateFunc.h"


class StateContext :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    StateContext();
    ~StateContext();
    
    PtObjectHeaderInheritance(StateContext);
    STDEF_SC_HEADER(StateContext);

    virtual int Create();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
};

