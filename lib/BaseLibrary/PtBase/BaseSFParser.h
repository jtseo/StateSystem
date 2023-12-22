#pragma once
#include "BaseStateFunc.h"
class BaseSFParser :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(wordTake_strIf),
        STDEF_SFENUM(lineNext_nF),
        STDEF_SFENUM(labelLoad_strF),
        STDEF_SFENUM(jsonSet_varF),
        STDEF_SFENUM(jsonGet_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSFParser();
    ~BaseSFParser();

    PtObjectHeaderInheritance(BaseSFParser);
    STDEF_SC_HEADER(BaseSFParser);

    virtual int Create();
    int labelLoad_strF();
    int jsonSet_varF();
    int jsonGet_varF();
    //#SF_FuncHeaderInsert
    int wordTake_strIf();
    int lineNext_nF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    STLString   m_parsingText;
    STLVString  m_label_a;
    BaseFile    *m_parser;

};

