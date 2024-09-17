#pragma once
#include "BaseStateFunc.h"
class BaseSCHash :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(Add_varF),
        STDEF_SFENUM(Get_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCHash();
    ~BaseSCHash();

    PtObjectHeaderInheritance(BaseSCHash);
    STDEF_SC_HEADER(BaseSCHash);

    virtual int Create();
    int Add_varF();
    int Get_varF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
    STLMstrStr  m_stlMhash;
public:
protected:
};

