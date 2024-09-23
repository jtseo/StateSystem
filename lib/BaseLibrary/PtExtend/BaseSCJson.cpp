#include "stdafx.h"
//#include "../PtBase/stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSCJson.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#include "../PtBase/base64.h"
//#include <format>

PtObjectCpp(BaseSCJson);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCJson::s_func_hash_a;
const char* BaseSCJson::s_class_name = "BaseSCJson";

int BaseSCJson::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCJson::BaseSCJson()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCJson::FunctionProcessor);
}

int BaseSCJson::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(Add_varF);
		STDEF_SFREGIST(Get_varF);
		STDEF_SFREGIST(JsonGet_varF);
		STDEF_SFREGIST(MassMerge_astrF);
		STDEF_SFREGIST(clear_nF);
		STDEF_SFREGIST(JsonSet_varF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCJson::~BaseSCJson()
{

}

void BaseSCJson::init()
{
	BaseStateFunc::init();
}

void BaseSCJson::release()
{
}

BaseStateFuncEx* BaseSCJson::CreatorCallback(const void* _param)
{
	BaseSCJson* bs_func;
	PT_OAlloc(bs_func, BaseSCJson);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCJson::FunctionProcessor);

	return bs_func;
}

int BaseSCJson::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(Add_varF);
		STDEF_SFFUNCALL(Get_varF);
		STDEF_SFFUNCALL(JsonGet_varF);
		STDEF_SFFUNCALL(MassMerge_astrF);
		STDEF_SFFUNCALL(clear_nF);
		STDEF_SFFUNCALL(JsonSet_varF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int BaseSCJson::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCJson* bs_func = (BaseSCJson*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCJson::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCJson::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}


int BaseSCJson::Add_varF()
{
	const int* hash = (const int*)m_param_value;
	const char* name = (const char*)paramFallowGet(0);

	const char* data = NULL;
	int nSize = 0;
	int index = m_state_variable->get_index(*hash);
	int type = BaseDStructure::get_type(index);
	char buff[255];
	if(type == TYPE_STRING)
	{
		if (!m_state_variable->get_mass(*hash, (const void**)&data))
			return 0;
		m_json.set(name, data);
		if (strncmp(data, "[]", 2) == 0)
			m_json.typeSet(name, "array");
	}
	else if (type == TYPE_ARRAY_BYTE)
	{
		if (!m_state_variable->get_mass(*hash, (const void**)&data, &nSize))
			return 0;

		std::string encoded = base64_encode((const unsigned char*)data, nSize);
		m_json.set(name, encoded.c_str());
	}
	else if (type == TYPE_INT32)
	{
		const int *value = NULL;
		if (!m_state_variable->get(*hash, (const void**)&value))
			return 0;

		sprintf_s(buff, 255, "%d", *value);
		m_json.set(name, buff);
		m_json.typeSet(name, "int");
	}
	else if (type == TYPE_FLOAT)
	{
		const float* value = NULL;
		if (!m_state_variable->get(*hash, (const void**)&value))
			return 0;

		sprintf_s(buff, 255, "%f", *value);
		m_json.set(name, buff);
		m_json.typeSet(name, "float");
	}
	else if (type == TYPE_INT64)
	{
		const INT64* value = NULL;
		if (!m_state_variable->get(*hash, (const void**)&value))
			return 0;

		sprintf_s(buff, 255, "%lld", *value);
		m_json.set(name, buff);
		m_json.typeSet(name, "int");
	}

	return 1;
}

int BaseSCJson::Get_varF()
{
	const int* hash = (const int*)m_param_value;
	const char* name = (const char*)paramFallowGet(0);

	int idx = BaseDStructure::get_index(*hash);
	if (idx == -1)
		return 0;

	STLString nameStr = name;
	const void* value = NULL;
	STLString str;
	int v1;
	INT64 v2;
	switch (BaseDStructure::type_get(idx).nType)
	{
	case TYPE_STRING:
		str = m_json.get(nameStr);
		value = str.c_str();
		break;
	case TYPE_INT32:
		if (!m_json.get(name, &v1))
			return 0;
		value = &v1;
		break;
	case TYPE_INT64:
		if (!m_json.get(name, &v2))
			return 0;
		value = &v2;
		break;
	}
	m_state_variable->set_alloc(*hash, (const void*)value);
	return 1;
}

int BaseSCJson::JsonGet_varF()
{
	const int* hash = (const int*)m_param_value;
	STLString json = m_json.get();

	if (!m_state_variable->set_mass(*hash, (const void*)json.c_str()))
		return 0;

	return 1;
}
int BaseSCJson::MassMerge_astrF()
{
	const char* params = (const char*)m_param_value;

	STLVString param_a;
	if(BaseFile::paser_list_seperate(params, &param_a, ",") < 2)
		return 0;

	int hash1 = STRTOHASH(param_a[0].c_str());
	int hash2 = STRTOHASH(param_a[1].c_str());

	const char* buff1 = 0, *buff2 = 0;
	int nSize1, nSize2;
	if (!m_state_variable->get_mass(hash1, (const void**)&buff1, &nSize1))
		return 0;
	if (!m_state_variable->get_mass(hash2, (const void**)&buff2, &nSize2))
		return 0;


	char* ret = PT_Alloc(char, nSize1+nSize2);
	PT_AFree(ret);
	memcpy(ret, buff1, nSize1);
	memcpy(ret + nSize1, buff2, nSize2);

	m_state_variable->set_alloc(hash1, NULL);
	if (!m_state_variable->set_mass(hash1, ret, nSize1 + nSize2))
		return 0;

	std::string encoded = base64_encode((const unsigned char*)ret, nSize1 + nSize2);
	FILE* pf = NULL;
	pf = fopen("rec_packetmerge.txt", "a+");
	if (pf)
	{
		fputs(encoded.c_str(), pf);
		fprintf(pf, "%d, %d\n", nSize1, nSize2);
		fputs("", pf);
		fclose(pf);
	}

	return 1;
}

int BaseSCJson::JsonSet_varF()
{
	const char* json = (const char*)paramVariableGet();
    if(!json)
        return 0;
	if (!m_json.set(json))
		return 0;
	return 1;
}

int BaseSCJson::clear_nF()
{
	m_json.clear();
	return 1;
}
//#SF_functionInsert
