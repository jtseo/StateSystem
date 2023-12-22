#pragma once
#include "BaseStateFunc.h"
class BaseTextEditorText :
    public BaseStateFunc
{
protected:
    enum {
        
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(BraceSet_nF),
        STDEF_SFENUM(LineFind_varF),
        STDEF_SFENUM(WordFind_varF),
        STDEF_SFENUM(LinesAppend_varF),
        STDEF_SFENUM(WordsAppend_varF),
        STDEF_SFENUM(FileAppend_varF),
        STDEF_SFENUM(LineDelete_nF),
        STDEF_SFENUM(LineCommentMake_nF),
        STDEF_SFENUM(LineCommentRelease_nF),
        STDEF_SFENUM(WordDelete_nF),
        STDEF_SFENUM(OpenEditorVS_nF),
        STDEF_SFENUM(analysisFind_strIf),
        STDEF_SFENUM(analysisBraceGetInt_strIf),
        STDEF_SFENUM(exportFormat_strF),
        STDEF_SFENUM(seperatorPush_strF),
        STDEF_SFENUM(seperatorPop_nF),
        STDEF_SFENUM(analysisStringGet_nF),
        STDEF_SFENUM(analysisNumGet_nF),
        STDEF_SFENUM(delectorPush_strF),
        STDEF_SFENUM(delectorPop_nF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseTextEditorText();
    ~BaseTextEditorText();

    PtObjectHeaderInheritance(BaseTextEditorText);
    STDEF_SC_HEADER(BaseTextEditorText);

    virtual int Create();
    //#SF_FuncHeaderStart
    int BraceSet_nF();
    int LineFind_varF();
    int WordFind_varF();
    int LinesAppend_varF();
    int WordsAppend_varF();
    int FileAppend_varF();
    int LineDelete_nF();
    int LineCommentMake_nF();
    int LineCommentRelease_nF();
    int WordDelete_nF();
    int OpenEditorVS_nF();
    int analysisFind_strIf();
    int analysisBraceGetInt_strIf();
    int exportFormat_strF();
    int seperatorPush_strF();
    int seperatorPop_nF();
    int analysisStringGet_nF();
    int analysisNumGet_nF();
    int delectorPush_strF();
    int delectorPop_nF();
    //#SF_FuncHeaderInsert

    // user defined area from here
protected:
    BaseStateFunc* m_super;

    // support analysis
public:

protected:
    STLString   m_analysis_current_str;
    STLString   m_line_str;
    BaseFile    m_parser;
    STLVString  m_seperator_stack_a;
    STLVString  m_delector_stack_a;
    STLString   m_seperator_str;
    STLString   m_delector_str;

};

