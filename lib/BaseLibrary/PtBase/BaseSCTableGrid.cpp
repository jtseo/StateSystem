#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSCTableGrid.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseSCTableGrid);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCTableGrid::s_func_hash_a;
const char* BaseSCTableGrid::s_class_name = "BaseSCTableGrid";

int BaseSCTableGrid::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCTableGrid::BaseSCTableGrid()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCTableGrid::FunctionProcessor);
}

int BaseSCTableGrid::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(search_astrIf);
		STDEF_SFREGIST(add_strIf);
		STDEF_SFREGIST(remove_strIf);
		STDEF_SFREGIST(update_strIf);
		STDEF_SFREGIST(get_strIf);
		STDEF_SFREGIST(equal_strIf);
		STDEF_SFREGIST(getColum_strF);
		STDEF_SFREGIST(getColumnBigint_strF);
		STDEF_SFREGIST(gets_nIf);
		STDEF_SFREGIST(setid_strIf);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCTableGrid::~BaseSCTableGrid()
{

}

void BaseSCTableGrid::init()
{
	BaseStateFunc::init();
}

void BaseSCTableGrid::release()
{
}

BaseStateFuncEx* BaseSCTableGrid::CreatorCallback(const void* _param)
{
	BaseSCTableGrid* bs_func;
	PT_OAlloc(bs_func, BaseSCTableGrid);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCTableGrid::FunctionProcessor);

	return bs_func;
}

int BaseSCTableGrid::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(search_astrIf);
		STDEF_SFFUNCALL(add_strIf);
		STDEF_SFFUNCALL(remove_strIf);
		STDEF_SFFUNCALL(update_strIf);
		STDEF_SFFUNCALL(get_strIf);
		STDEF_SFFUNCALL(equal_strIf);
		STDEF_SFFUNCALL(getColum_strF);
		STDEF_SFFUNCALL(getColumnBigint_strF);
		STDEF_SFFUNCALL(gets_nIf);
		STDEF_SFFUNCALL(setid_strIf);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}

int BaseSCTableGrid::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCTableGrid* bs_func = (BaseSCTableGrid*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCTableGrid::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

#include "BaseResGrid.h"

int BaseSCTableGrid::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	const char* filepath = (const char*)paramFallowGet(0);

	if (!filepath)
		return 0;

	BaseResManager* res_mgr = BaseResManager::get_manager();
	m_grid_p = (BaseResGrid*)res_mgr->get_resource(filepath, BaseResGrid::GetObjectId(), 0, 0, 0);

	if (!m_grid_p)
		return 0;

	return 1;
}

int BaseSCTableGrid::search_astrIf()
{
	const char* query = (const char*)m_param_value;
	const char* search = (const char*)paramFallowGet(0);

	if (!search)
		return 0;

	BaseFile parser;
	parser.set_asc_seperator(", ");
	parser.set_asc_deletor(", ");
	parser.OpenFile((void*)query, (UINT32)strlen(query));

	parser.read_asc_line();

	STLString target = parser.read_asc_str();
	STLString label = parser.read_asc_str();

	int idx = m_grid_p->column_search(label.c_str(), search);

	if (idx == -1)
		return 0;

	int label_n = m_grid_p->label_find(target.c_str());

	if (label_n == -1)
		return 0;

	search = m_grid_p->column_get(label_n, idx);
	if (!search)
		return 0;

	paramFallowSet(0, search);

	return 1;
}
int BaseSCTableGrid::add_strIf()
{
	const char *label = (const char*)m_param_value;
	const char *data = (const char*)paramFallowGet(0);

	if(!data)
		return 0;
	
	STLVString str_a;
	m_index = (int)m_grid_p->ctid_next_get();
	BaseFile::paser_list_seperate(data, &str_a, "\t");

	STLVString set_astr;
	char buff[128];
	sprintf_s(buff, 128, "%d", m_index);
	//const char *str_v = m_grid_p->string_get(m_index, label);
	set_astr.push_back(buff);
	for(int i=0; i<str_a.size(); i++)
		set_astr.push_back(str_a[i]);

	if(!m_grid_p->strings_add(set_astr))
		return 0;
	m_grid_p->Save();
	return 1;
}
int BaseSCTableGrid::remove_strIf()
{
	const char *label = (const char*)m_param_value;
	const char *key = (const char*)paramFallowGet(0);

	if(!label || !key)
		return 0;
	int idx = m_grid_p->column_search(label, key);
	if(idx == -1)
		return 0;

	if(!m_grid_p->column_remove(idx))
		return 0;
	
	return 1;
}
int BaseSCTableGrid::update_strIf()
{
	const char *label = (const char*)m_param_value;
	const char *data = (const char*)paramFallowGet(0);
	if(!label || !data)
		return 0;

	STLVString str_a;
	BaseFile::paser_list_seperate(data, &str_a, "\t");

	STLVString set_astr;
	const char *str_v = m_grid_p->string_get(m_index, label);
	set_astr.push_back(str_v);
	for(int i=0; i<str_a.size(); i++)
		set_astr.push_back(str_a[i]);
	if(!m_grid_p->strings_set(set_astr))
		return 0;
	m_grid_p->Save();
	return 1;
}
int BaseSCTableGrid::get_strIf()
{
	const char *label = (const char*)m_param_value;
	const char *key = (const char*)paramFallowGet(0);
	int idx = m_grid_p->column_search(label, key);

	if(idx == -1)
		return 0;

	m_index = idx;
	int size_n = m_grid_p->label_count();

	STLString ret_str = "";
	for(int i=0; i<size_n; i++)
	{
		ret_str += m_grid_p->column_get(i, idx);
		ret_str += "\t";
	}

	paramFallowSet(1, ret_str.c_str());
	return 1;
}

int BaseSCTableGrid::equal_strIf()
{
	const char *label = (const char*)m_param_value;
	const char *value = (const char*)paramFallowGet(0);

	const char *str_v = m_grid_p->string_get(m_index, label);
	if(!str_v)
		return 0;
	STLString str = str_v;
	if(str != value)
		return 0;

	return 1;
}
int BaseSCTableGrid::getColum_strF()
{
	const char *label = (const char*)m_param_value;
	
	const char *str_v = m_grid_p->string_get(m_index, label);
	if(!str_v)
		return 0;
	
	paramFallowSet(0, str_v);
	return 1;
}
int BaseSCTableGrid::getColumnBigint_strF()
{
	const char *label = (const char*)m_param_value;
	
	const char *str_v = m_grid_p->string_get(m_index, label);
	if(!str_v)
		return 0;
	
	int value;
	sscanf_s(str_v, "%d", &value);
	INT64 value_64 = (INT64)value;
	paramFallowSet(0, &value_64);
	return 1;
}
int BaseSCTableGrid::gets_nIf()
{
	const int *event_p = (const int*)m_param_value;
	const char *label = (const char*)paramFallowGet(0);
	const char *search = (const char*)paramFallowGet(1);
	STLVInt idx_a = m_grid_p->columns_search(label, search);

	if(idx_a.size() == 0)
		return 0;

	int size_n = m_grid_p->label_count();
	int nTarget = m_state_p->obj_serial_get();

	for(int j=0; j<idx_a.size(); j++)
	{
		int idx = idx_a[j];
		STLString ret_str = "";
		for(int i=0; i<size_n; i++)
		{
			ret_str += m_grid_p->column_get(i, idx);
			ret_str += "\t";
		}

		BaseDStructureValue *evt = EventMake(*event_p);
		evt->set_alloc("TempString2_strV", ret_str.c_str());
		EventPost(evt, nTarget);
	}
	EventEndPost(*event_p, nTarget);

	return 1;
}
int BaseSCTableGrid::setid_strIf()
{
	const char *label = (const char*)m_param_value;
	const char *key = (const char*)paramFallowGet(0);
	int idx = m_grid_p->column_search(label, key);

	if(idx == -1)
		return 0;

	m_index = idx;
	int size_n = m_grid_p->label_count();

	STLString ret_str = "";
	for(int i=0; i<size_n; i++)
	{
		ret_str += m_grid_p->column_get(i, idx);
		ret_str += "\t";
	}

	paramFallowSet(1, ret_str.c_str());
	return 1;
}
//#SF_functionInsert
