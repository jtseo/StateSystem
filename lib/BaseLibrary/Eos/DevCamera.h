#pragma once
#include "../PtBase/BaseStateFunc.h"

class CameraEvent;

class DevCamera :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(TakeAPicture_nF),
        STDEF_SFENUM(PreviewStart_nF),
        STDEF_SFENUM(PreviewStop_nF),
        STDEF_SFENUM(EventProcess_nF),
        STDEF_SFENUM(EventCastEnale_nF),
        STDEF_SFENUM(TextImageCast_nF),
        STDEF_SFENUM(PreviewRequest_nF),
        STDEF_SFENUM(StreamFree_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    DevCamera();
    ~DevCamera();

    PtObjectHeaderInheritance(DevCamera);
    STDEF_SC_HEADER(DevCamera);

    virtual int Create();
    int TakeAPicture_nF();
    int PreviewStart_nF();
    int PreviewStop_nF();
    int EventProcess_nF();
    int EventCastEnale_nF();
    int TextImageCast_nF();
    int PreviewRequest_nF();
    int StreamFree_varF();
    //#SF_FuncHeaderInsert

protected:
    // support for text analysis

    bool m_stop_thread;
    static DEF_ThreadCallBack(update);
public:
    bool stop_thread();
protected:
};

