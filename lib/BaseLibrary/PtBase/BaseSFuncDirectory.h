#pragma once

#include "BaseStateFunc.h"
class BaseSFuncDirectory :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        Enum_fileCast_nF,
        Enum_folerCreate_varF,
        Enum_folerCurrentPush_varF,
        Enum_folerCurrentPop_varF,
        STDEF_SFENUM(fileCopy_avarIf),
        STDEF_SFENUM(fileDelete_varF),
        STDEF_SFENUM(RunCLI_nF),
        STDEF_SFENUM(FilenameExtChange_varF),
        STDEF_SFENUM(DirectoryListGet_varIf),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSFuncDirectory();
    ~BaseSFuncDirectory();
    PtObjectHeaderInheritance(BaseSFuncDirectory);
    STDEF_SC_HEADER(BaseSFuncDirectory);

    virtual int Create();
    int fileCopy_avarIf();
    int fileDelete_varF();
    int RunCLI_nF();
    int FilenameExtChange_varF();
    int DirectoryListGet_varIf();
    //#SF_FuncHeaderInsert
    int fileCast_nF();
    int folerCreate_varF();
    int folerCurrentPush_varF();
    int folerCurrentPop_varF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    STLString m_pathCurrent;
    STLVString m_pathStack;

};

