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
        STDEF_SFENUM(QRCodeMake_varF),
        STDEF_SFENUM(PhotoPannelImgAdd_varF),
        STDEF_SFENUM(Rotate_varF),
        STDEF_SFENUM(DoubleMake_varF),
        STDEF_SFENUM(FilterApply_varF),
        STDEF_SFENUM(ImageScale_fF),
        STDEF_SFENUM(ConvertBmp_varF),
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
    int QRCodeMake_varF();
    int PhotoPannelImgAdd_varF();
    int Rotate_varF();
    int DoubleMake_varF();
    int FilterApply_varF();
    int ImageScale_fF();
    int ConvertBmp_varF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    // support for text analysis
public:
    static void overlayImage(cv::Mat& background, const cv::Mat& foreground, cv::Point2i location, double alpha = 0.5, bool _overwrite = false);
protected:
};

