#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "DevPrinter.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(DevPrinter);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	DevPrinter::s_func_hash_a;
const char* DevPrinter::s_class_name = "DevPrinter";

int DevPrinter::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

DevPrinter::DevPrinter()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, DevPrinter::FunctionProcessor);
}

int DevPrinter::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(Print_varIf);
		STDEF_SFREGIST(NameSet_strF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

DevPrinter::~DevPrinter()
{

}

void DevPrinter::init()
{
	BaseStateFunc::init();
}

void DevPrinter::release()
{
}

BaseStateFuncEx* DevPrinter::CreatorCallback(const void* _param)
{
	DevPrinter* bs_func;
	PT_OAlloc(bs_func, DevPrinter);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, DevPrinter::FunctionProcessor);

	return bs_func;
}

int DevPrinter::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(Print_varIf);
		STDEF_SFFUNCALL(NameSet_strF);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}
int DevPrinter::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	DevPrinter* bs_func = (DevPrinter*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, DevPrinter::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int DevPrinter::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
	//GroupLevelSet(0);


	return 1;
}

int DevPrinter::Print_varIf()
{
	return 1;
}

int DevPrinter::NameSet_strF()
{
	return 1;
}

//#SF_functionInsert
