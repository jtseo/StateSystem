#pragma once
#include "../PtBase/BaseStateFunc.h"
class BaseSFTemplete :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSFTemplete();
    ~BaseSFTemplete();
    static int GetObjectId();

    PtObjectHeaderInheritance(BaseSFTemplete);

    static int FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status);
    static int StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func);
    static BaseStateFunc* CreatorCallback();

    virtual int Create();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
};

