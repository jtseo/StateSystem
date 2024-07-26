#pragma once
#include "BaseStateFunc.h"
class $classname :
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
    $classname();
    ~$classname();

    PtObjectHeaderInheritance($classname);
    STDEF_SC_HEADER($classname);

    virtual int Create();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

