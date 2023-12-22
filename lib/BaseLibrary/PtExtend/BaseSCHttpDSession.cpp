#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSCHttpDSession

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseSCHttpDSession

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCHttpDSession
const char* BaseSCHttpDSession

int BaseSCHttpDSession
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCHttpDSession
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCHttpDSession
}

int BaseSCHttpDSession
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(ApiReturn_varF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCHttpDSession
{

}

void BaseSCHttpDSession
{
	BaseStateFunc::init();
}

void BaseSCHttpDSession
{
}

BaseStateFuncEx* BaseSCHttpDSession
{
	BaseSCHttpDSession
	PT_OAlloc(bs_func, BaseSCHttpDSession

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCHttpDSession

	return bs_func;
}

int BaseSCHttpDSession
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(ApiReturn_varF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int BaseSCHttpDSession
{
	int _processor = _base->function_hash_get();

	BaseSCHttpDSession

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCHttpDSession
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

int *Fail*::ApiReturn_varF()
{
	return 1;
}
//#SF_functionInsert
