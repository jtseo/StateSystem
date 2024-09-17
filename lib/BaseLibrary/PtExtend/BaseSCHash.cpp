#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSCHash.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseSCHash);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCHash::s_func_hash_a;
const char* BaseSCHash::s_class_name = "BaseSCHash";

int BaseSCHash::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCHash::BaseSCHash()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCHash::FunctionProcessor);
}

int BaseSCHash::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(Add_varF);
		STDEF_SFREGIST(Get_varF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCHash::~BaseSCHash()
{

}

void BaseSCHash::init()
{
	BaseStateFunc::init();
}

void BaseSCHash::release()
{
}

BaseStateFuncEx* BaseSCHash::CreatorCallback(const void* _param)
{
	BaseSCHash* bs_func;
	PT_OAlloc(bs_func, BaseSCHash);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCHash::FunctionProcessor);

	return bs_func;
}

int BaseSCHash::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(Add_varF);
		STDEF_SFFUNCALL(Get_varF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int BaseSCHash::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCHash* bs_func = (BaseSCHash*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCHash::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCHash::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

int BaseSCHash::Add_varF()
{
    const char* key = (const char*)paramVariableGet();
    const char* key2 = (const char*)paramFallowGet(0);
    const char* value = (const char*)paramFallowGet(1);

    STLString keym = key;
    if(key2)
        keym += key2;
    m_stlMhash[keym] = value;
	return 1;
}

int BaseSCHash::Get_varF()
{
    const char* key = (const char*)paramVariableGet();
    const char* key2 = (const char*)paramFallowGet(0);
    
    STLString keym = key;
    if(key2)
        keym += key2;
    
    STLMstrStr::iterator it;
    it = m_stlMhash.find(keym);
    
    if(it == m_stlMhash.end())
        return 0;
    
    paramFallowSet(1, it->second.c_str());
    return 1;
}
//#SF_functionInsert
