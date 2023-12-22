#include "pch.h"

//#define PY_SSIZE_T_CLEAN
//#include <Python.h>

#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseStateSpace.h"
#include "../PtBase/BaseStringTable.h"

#include "StatePython.h"


PtObjectCpp(StatePython);
// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	StatePython::s_func_hash_a;
const char* StatePython::s_class_name = "StatePython";

int StatePython::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

StatePython::StatePython()
{
	printf("create python");
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, StatePython::FunctionProcessor);
}

int StatePython::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(Sample_nF);
		//#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

StatePython::~StatePython()
{

}

void StatePython::init()
{
	BaseStateFunc::init();
}

void StatePython::release()
{
}

BaseStateFuncEx* StatePython::CreatorCallback(const void* _param)
{
	StatePython* bs_func;
	PT_OAlloc(bs_func, StatePython);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, StatePython::FunctionProcessor);

	printf("create call back");

	return bs_func;
}

int StatePython::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(Sample_nF);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}

int StatePython::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	StatePython* bs_func = (StatePython*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, StatePython::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int StatePython::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	return 1;
}

int StatePython::Sample_nF()
{

	return 0;
}
//#SF_functionInsert
