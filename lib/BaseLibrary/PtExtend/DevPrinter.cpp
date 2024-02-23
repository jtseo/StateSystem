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

#include <Windows.h>

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
		STDEF_SFREGIST(NameSet_varF);
		STDEF_SFREGIST(IsSmall_varIf);
		STDEF_SFREGIST(PaperCountGet_varF);
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


	DevPrinter devPrint;

	// Example: Load a bitmap from a file
	//HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, "test.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	//devPrint.Print("Microsoft Print to PDF", 0, hBitmap);
	//devPrint.Print("DS-RX1 (1 복사)", 1, hBitmap);
	//devPrint.Print("DS-RX1", 1, hBitmap);

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
		STDEF_SFFUNCALL(NameSet_varF);
		STDEF_SFFUNCALL(IsSmall_varIf);
		STDEF_SFFUNCALL(PaperCountGet_varF);
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

#include "DevPrint.h"

int DevPrinter::Print_varIf()
{
	// print image; param printer.
	const char* filename = (const char*)paramVariableGet();

	DevPrint(m_printer.c_str(), 1, filename);
	return 1;
}

int DevPrinter::NameSet_varF()
{
	const char* printName = (const char*)paramVariableGet();
	m_printer = printName;
	return 1;
}

int DevPrinter::IsSmall_varIf()
{
	const int* photoSize = (const int*)paramVariableGet();

	if (photoSize == NULL)
		return 0;

	int min = 0;
	min = photoSize[0];
	if (min > photoSize[1])
		min = photoSize[1];

	if (min > 600)
		return 0;

	return 1;
}

int DevPrinter::PaperCountGet_varF()
{
	const int* numOf = (const int*)paramVariableGet();
	const int* photoSize = (const int*)paramFallowGet(0);

	if (photoSize == NULL)
		return 0;

	int min = 0;
	min = photoSize[0];
	if (min > photoSize[1])
		min = photoSize[1];

	int retNumOf = *numOf;
	if (min > 600)
		retNumOf *= 2;

	paramVariableSet(&retNumOf);
	return 1;
}

int DevPrinter::NameSet_strF()
{
	const char* printName = (const char*)m_param_value;
	m_printer = printName;
	return 1;
}

//#SF_functionInsert

