#pragma once
#include "BaseStateFunc.h"
class UdpChannelServer :
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
    UdpChannelServer();
    ~UdpChannelServer();

    PtObjectHeaderInheritance(UdpChannelServer);
    STDEF_SC_HEADER(UdpChannelServer);

    virtual int Create();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
protected:
};

