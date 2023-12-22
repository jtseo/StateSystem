#pragma once
#include "BaseStateFunc.h"
class BaseTextEditor :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(Create_varF),
        STDEF_SFENUM(Close_varF),
        STDEF_SFENUM(classPathGet_varF),
        STDEF_SFENUM(definePathGet_varF),
        STDEF_SFENUM(functionLineNumGet_varF),
        STDEF_SFENUM(updateStart_varIf),
        STDEF_SFENUM(updateSearch_formatIf),
        STDEF_SFENUM(updateAdd_formatF),
        STDEF_SFENUM(updateEnd_nF),
        STDEF_SFENUM(classNameGet_varF),
        STDEF_SFENUM(functionNameGet_varF),
        STDEF_SFENUM(updateBraceComment_nF),
        STDEF_SFENUM(updateSearch2_formatIf),
        STDEF_SFENUM(lineCast_nF),
        STDEF_SFENUM(exportFile_varF),
        STDEF_SFENUM(seperatorPush_strF),
        STDEF_SFENUM(seperatorPop_nF),
        STDEF_SFENUM(updateAdd_vformatF),
        STDEF_SFENUM(classPathSet_varF),
        STDEF_SFENUM(definePathSet_varF),
        STDEF_SFENUM(defineClassPathGet_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseTextEditor();
    ~BaseTextEditor();

    PtObjectHeaderInheritance(BaseTextEditor);
    STDEF_SC_HEADER(BaseTextEditor);

    virtual int Create();
    int Create_varF();
    int Open_varF();
    int Close_varF();
    int classPathGet_varF();
    int definePathGet_varF();
    int functionLineNumGet_varF();
    int updateStart_varIf();
    int updateSearch_formatIf();
    int updateAdd_formatF();
    int updateEnd_nF();
    int classNameGet_varF();
    int functionNameGet_varF();
    int updateBraceComment_nF();
    int updateSearch2_formatIf();
    int lineCast_nF();
    int exportFile_varF();
    int seperatorPush_strF();
    int seperatorPop_nF();
    int updateAdd_vformatF();
    int classPathSet_varF();
    int definePathSet_varF();
    int defineClassPathGet_varF();
    //#SF_FuncHeaderInsert

    // User defin area from here
    int _updateBraceCommentInLine(STLString& _str);
    int _updateBraceComment(STLString &_str);
	int _replaceString(STLString &_loading, STLString &_target, STLString &_replace, size_t _pos);
protected:
    STLString   m_definePath;
    STLString   m_classPath;
    STLString   m_function;
    STLString   m_className;
    STLString   m_contents;
    STLVString  m_projectPath_a;

    BaseFile* m_updatefileRead;
    BaseFile* m_updatefileWrite;

    // support for text analysis
public:
    void analysisHeaderAdd(const STLString _str);
    void analysisContentsAdd(const STLString _str);
    void analysisTailAdd(const STLString _str);
protected:
    STLString   m_header_str;
    STLString   m_contents_str;
    STLString   m_tail_str;
    STLVString  m_seperator_stack_a;
    STLVString  m_delector_stack_a;
    STLString   m_seperator_str;
    STLString   m_delector_str;
};

