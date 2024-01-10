#pragma once
#include "../PtBase/BaseStateFunc.h"
class DevPrinter :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(Print_varIf),
        STDEF_SFENUM(NameSet_strF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    DevPrinter();
    ~DevPrinter();

    PtObjectHeaderInheritance(DevPrinter);
    STDEF_SC_HEADER(DevPrinter);

    virtual int Create();
    int Print_varIf();
    int NameSet_strF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

