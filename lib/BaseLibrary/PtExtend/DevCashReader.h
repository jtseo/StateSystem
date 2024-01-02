#pragma once
#include "../PtBase/BaseStateFunc.h"
class DevCashReader :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(DataARead_nF),
        STDEF_SFENUM(DataAShift_nF),
        STDEF_SFENUM(DataComplete_nIf),
        STDEF_SFENUM(DataGet_varF),
        STDEF_SFENUM(DataWriteSet_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    DevCashReader();
    ~DevCashReader();

    PtObjectHeaderInheritance(DevCashReader);
    STDEF_SC_HEADER(DevCashReader);

    virtual int Create();
    int DataARead_nF();
    int DataAShift_nF();
    int DataComplete_nIf();
    int DataGet_varF();
    int DataWriteSet_varF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

