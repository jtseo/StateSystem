#pragma once
#include "../PtBase/BaseStateFunc.h"
class BaseStateSystem :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(AppClose_strF),
        STDEF_SFENUM(MakeFront_nF),
        STDEF_SFENUM(ListFilter_strF),
        STDEF_SFENUM(KeyboardNumOn_nF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseStateSystem();
    ~BaseStateSystem();

    PtObjectHeaderInheritance(BaseStateSystem);
    STDEF_SC_HEADER(BaseStateSystem);

    virtual int Create();
    int AppClose_strF();
    int MakeFront_nF();
    int ListFilter_strF();
    int KeyboardNumOn_nF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

