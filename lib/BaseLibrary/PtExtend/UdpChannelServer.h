#pragma once
#include "../PtBase/BaseStateFunc.h"

typedef enum {
    None,
    Pro,
    Dev,
    Enter,
    Max
} STD_ChannelType;

typedef struct UdpChannelInfo{
public:
    char name[50];
    char ip[20];
    STD_ChannelType type;
} STD_UdpChannel;

typedef std::vector<STD_UdpChannel, PT_allocator<STD_UdpChannel> >                STLVUdpChannel;

class UdpChannelServer :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(ChannelGet_varF),
        STDEF_SFENUM(ChannelAdd_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    UdpChannelServer();
    ~UdpChannelServer();

    PtObjectHeaderInheritance(UdpChannelServer);
    STDEF_SC_HEADER(UdpChannelServer);

    virtual int Create();
    int ChannelGet_varF();
    int ChannelAdd_varF();
    //#SF_FuncHeaderInsert

    // User defin area from heres
protected:
    // support for text analysis
    STLVUdpChannel m_stlVChannel;
    static int m_typedef[STD_ChannelType::Max];
    int m_lastChannels[STD_ChannelType::Max];
    
    STD_ChannelType TypeGet(const char* _type);
    int ChannelGet(STD_ChannelType _type, int _last);
public:
protected:
};

