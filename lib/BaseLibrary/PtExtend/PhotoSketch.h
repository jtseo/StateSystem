﻿#pragma once
#include "../PtBase/BaseStateFunc.h"
class PhotoSketch :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(SketchStart_varF),
        STDEF_SFENUM(ThreadStop_nF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    PhotoSketch();
    ~PhotoSketch();

    PtObjectHeaderInheritance(PhotoSketch);
    STDEF_SC_HEADER(PhotoSketch);

    virtual int Create();
    //#SF_FuncHeaderInsert
    int SketchStart_varF();
    int ThreadStop_nF();
    // User defin area from here
protected:
    // support for text analysis
public:
protected:

protected:
    // support for text analysis

    STLString m_pathFrame;
    STLString m_pathPicture;
    int m_pictureSize[2];
    int m_stepCounter;
    int m_threadIdx;
    int m_sketchType;

    bool m_stop_thread;
    static DEF_ThreadCallBack(update);
public:
    int stepCount();
    int threadIdx() {
        return m_threadIdx;
    }
    bool stop_thread();
    void thread_stop() {
        m_stop_thread = true;
    }
    const char* PathFrameGet();
    const char* PathPictureGet();
    int SketchType() { return m_sketchType;  }
    int* PictureSize();
};

