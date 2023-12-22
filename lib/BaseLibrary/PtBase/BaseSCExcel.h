#pragma once
#include "../PtBase/BaseStateFunc.h"

class BaseExcelColumn
{
public:
    bool mergeUp;
    bool mergeLeft;

    STLString   text;
    PtVector3   size_v3;

    void init();
};

typedef std::vector<BaseExcelColumn, PT_allocator<BaseExcelColumn> >				STLVExcelColumn;
typedef std::vector<STLVExcelColumn, PT_allocator<STLVExcelColumn> >		    STLVVExcelColumn;

class BaseSCExcel :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(loadCheck_nIf),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCExcel();
    ~BaseSCExcel();

    PtObjectHeaderInheritance(BaseSCExcel);
    STDEF_SC_HEADER(BaseSCExcel);

    virtual int Create();
    int loadCheck_nIf();
    //#SF_FuncHeaderInsert

    // User defin area from here
    const STLVVExcelColumn& columnGet() const;
protected:
    STLVVExcelColumn    m_excel_aa;
};

