#pragma once
#include "../PtBase/BaseStateFunc.h"
class ExtendOpenCV :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(PictureRatioAdapt_varF),
        STDEF_SFENUM(PhotoPannelMake_strF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    ExtendOpenCV();
    ~ExtendOpenCV();

    PtObjectHeaderInheritance(ExtendOpenCV);
    STDEF_SC_HEADER(ExtendOpenCV);

    virtual int Create();
    int PictureRatioAdapt_varF();
    int PhotoPannelMake_strF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

