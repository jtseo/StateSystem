#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "StateContext.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(StateContext);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt    StateContext::s_func_hash_a;
const char* StateContext::s_class_name = "StateContext";

int StateContext::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

StateContext::StateContext()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, StateContext::FunctionProcessor);
}

int StateContext::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

StateContext::~StateContext()
{

}

void StateContext::init()
{
	BaseStateFunc::init();
}

void StateContext::release()
{
}

BaseStateFuncEx* StateContext::CreatorCallback(const void* _param)
{
	StateContext* bs_func;
	PT_OAlloc(bs_func, StateContext);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, StateContext::FunctionProcessor);

	return bs_func;
}

int StateContext::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
        //#SF_FuncCallStart
        if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
        //STDEF_SFFUNCALL(Open_varF);
        //#SF_FuncCallInsert
        return 0;
    }
    return ret;
}

int StateContext::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
    int _processor = _base->function_hash_get();

    StateContext* bs_func = (StateContext*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, StateContext::CreatorCallback, 0);

    int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

    return ret;
}

int StateContext::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	return 1;
}

//#SF_functionInsert
