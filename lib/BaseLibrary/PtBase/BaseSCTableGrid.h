#pragma once
#include "../PtBase/BaseStateFunc.h"

class BaseResGrid;

class BaseSCTableGrid :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(search_astrIf),
        STDEF_SFENUM(add_strIf),
        STDEF_SFENUM(remove_strIf),
        STDEF_SFENUM(update_strIf),
        STDEF_SFENUM(get_strIf),
        STDEF_SFENUM(equal_strIf),
        STDEF_SFENUM(getColum_strF),
        STDEF_SFENUM(getColumnBigint_strF),
        STDEF_SFENUM(gets_nIf),
        STDEF_SFENUM(setid_strIf),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCTableGrid();
    ~BaseSCTableGrid();

    PtObjectHeaderInheritance(BaseSCTableGrid);
    STDEF_SC_HEADER(BaseSCTableGrid);

    virtual int Create();
    int search_astrIf();
    int add_strIf();
    int remove_strIf();
    int update_strIf();
    int get_strIf();
    int equal_strIf();
    int getColum_strF();
    int getColumnBigint_strF();
    int gets_nIf();
    int setid_strIf();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    BaseResGrid* m_grid_p;
    int m_index; // last column index what get
};

