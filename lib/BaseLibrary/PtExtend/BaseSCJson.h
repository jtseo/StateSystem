#pragma once
#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseJson.h"

class BaseSCJson:
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(Add_varF),
        STDEF_SFENUM(Get_varF),
        STDEF_SFENUM(JsonGet_varF),
        STDEF_SFENUM(MassMerge_astrF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCJson();
    ~BaseSCJson();

    PtObjectHeaderInheritance(BaseSCJson);
    STDEF_SC_HEADER(BaseSCJson);

    virtual int Create();
    int Add_varF();
    int Get_varF();
    int JsonGet_varF();
    int MassMerge_astrF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
    BaseJson    m_json;
public:
protected:
};

