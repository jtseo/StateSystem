#pragma once
#include "../PtBase/BaseStateFunc.h"
class PhotoSketch :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(SketchStart_varF),
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

    bool m_stop_thread;
    static DEF_ThreadCallBack(update);
public:
    int stepCount();
    int threadIdx() {
        return m_threadIdx;
    }
    bool stop_thread();
    const char* PathFrameGet();
    const char* PathPictureGet();
    int* PictureSize();
};

