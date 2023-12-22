#pragma once

#include "../PtBase/BaseStateFunc.h"

class StatePython :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(Sample_nF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    StatePython();
    ~StatePython();

    PtObjectHeaderInheritance(StatePython);
    STDEF_SC_HEADER(StatePython);

    virtual int Create();
    //#SF_FuncHeaderInsert
    int Sample_nF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
};

