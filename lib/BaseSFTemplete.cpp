#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSFTemplete.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseSFTemplete);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

static STLVInt	s_func_hash_a;
static const char* s_class_name = "BaseSFTemplete";

int BaseSFTemplete::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSFTemplete::BaseSFTemplete()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFTemplete::FunctionProcessor);
}

int BaseSFTemplete::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
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

BaseSFTemplete::~BaseSFTemplete()
{

}

void BaseSFTemplete::init()
{
	BaseStateFunc::init();
}

void BaseSFTemplete::release()
{
}

BaseStateFunc* BaseSFTemplete::CreatorCallback()
{
	BaseSFTemplete* bs_func;
	PT_OAlloc(bs_func, BaseSFTemplete);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFTemplete::FunctionProcessor);

	return bs_func;
}

int BaseSFTemplete::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSFTemplete* bs_func = (BaseSFTemplete*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSFTemplete::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
        if (_processor == s_func_hash_a[Enum_ext_start])        return 0;
        //STDEF_SFFUNCALL(Open_varF);
        //#SF_FuncCallInsert

		return 0;
	}

	return ret;
}

int BaseSFTemplete::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	return 1;
}

//#SF_functionInsert
