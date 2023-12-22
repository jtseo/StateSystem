#include "stdafx.h"
#include "BaseResGrid.h"
#include "../PtBase/BaseFile.h"

#include "../PtBase/BaseState.h"
#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateSpace.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseResManager.h"

#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseResGrid);

BaseResGrid::BaseResGrid(void)
{
	m_nSerialNumber = serial_create_();	// Unique number for each objects
	m_nObjectId = GetObjectId();
}

int BaseResGrid::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0) {
		s_iId = UniqHashStr::get_string_hash_code("BaseResGrid");
	}
	else {
		static bool first = true;
		if (first) {
			first = false;
			REGISTEOBJECT(BaseResGrid::GetObjectId(), BaseResGrid);
		}
	}

	return s_iId;
}

BaseResGrid::~BaseResGrid(void)
{
}

bool BaseResGrid::Save()
{
	BaseFile	file;

	char path[255];
	BaseSystem::path_full_get(path, 255, m_strName.c_str());
	if (file.OpenFile(path, BaseFile::OPEN_WRITE))
		return false;

	file.set_asc_seperator("\t");
	file.set_asc_deletor("");

	for (int i = 0; i < m_label.size(); i++)
		file.write_asc_string(m_label[i].c_str(), 0);
	file.write_asc_line();

	for (int i = 0; i < m_grid_a.size(); i++)
	{
		for (int j = 0; j < m_grid_a[i].column_a.size(); j++)
			file.write_asc_string(m_grid_a[i].column_a[j].c_str(), 0);
		file.write_asc_line();
	}

	file.CloseFile();
	return true;
}

bool BaseResGrid::LoadJson()
{
	BaseFile	file;

	char path[255];

	m_ctid_new = -1;
	BaseSystem::path_full_get(path, 255, m_strName.c_str());
	if (file.OpenFile(path, BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
		return false;

	file.set_asc_seperator(":,{}");
	file.set_asc_deletor("\t \n\r");

	file.read_asc_line(); // label
	BaseResGridColumn column;

	STLString	label, tag;

	int cnt = 0;
	do {
		label = file.read_asc_quotation();
		cnt++;
	} while (label.empty() && cnt < 2);

	if (label.empty())
		return false;

	do {
		m_label.push_back(label);
		tag = file.read_asc_quotation();
		column.column_a.push_back(tag);
		label = file.read_asc_quotation();
	} while (!label.empty());

	m_grid_a.push_back(column);
	file.CloseFile();
	return true;
}

bool BaseResGrid::Load()
{	
	if (m_strName.substr(m_strName.find_last_of('.')) == ".json")
	{
		return LoadJson();
	}
	BaseFile	file;

	char path[255];

	m_ctid_new = -1;
	BaseSystem::path_full_get(path, 255, m_strName.c_str());
	if (!file.OpenFile(path, BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
	{
		file.set_asc_seperator("\t");
		file.set_asc_deletor("");

		file.read_asc_line(); // label
		BaseResGridColumn column;

		STLString	label, tag;
		while (file.read_asc_string(&tag))
		{
			label = tag;
			m_label.push_back(label);
		}

		while (file.read_asc_line())
		{
			for (unsigned i = 0; i < m_label.size(); i++) {
				tag = "";
				file.read_asc_string(&tag);
				column.column_a.push_back(tag);
			}
			m_grid_a.push_back(column);
			column.column_a.clear();
		}

		file.CloseFile();
		return true;
	}
	return false;
}

bool BaseResGrid::strings_set(const STLVString &_str_a)
{
	for (int i = 0; i < m_grid_a.size(); i++)
	{
		if (_str_a[0] == m_grid_a[i].column_a[0])
		{
			m_grid_a[i].column_a = _str_a;
			return true;
		}
	}

	BaseResGridColumn column;
	column.column_a = _str_a;
	m_grid_a.push_back(column);
	return false;
}

STLString BaseResGrid::label_get(int _i)
{
	STLString ret;

	if (_i < 0 || _i >= m_label.size())
		return ret;

	return m_label[_i];
}

int BaseResGrid::column_count()
{
	return (int)m_grid_a.size();
}

const char* BaseResGrid::column_get(int _label_index, int _column_index)
{
	if (m_grid_a.size() <= _column_index)
		return NULL;

	if (m_grid_a[_column_index].column_a.size() <= _label_index)
		return NULL;

	return m_grid_a[_column_index].column_a[_label_index].c_str();
}

int BaseResGrid::column_get(const char* _label_name, const char* _search)
{
	if (*_search == 0)
		return -1;

	int label = label_find(_label_name);

	if (label < 0)
		return -1;

	STLString search = _search;
	for (unsigned i = 0; i < m_grid_a.size(); i++)
	{
		if (m_grid_a[i].column_a[label] == search)
			return i;
	}
	return -1;
}

STLVInt BaseResGrid::columns_search(const char *_label_name, const char* _search)
{
	STLVInt ret_a;
	if (*_search == 0)
		return ret_a;

	int label = label_find(_label_name);

	if (label < 0)
		return ret_a;

	STLString search = _search;
	for (unsigned i = 0; i < m_grid_a.size(); i++)
	{
		int start = BaseStringTable::find(m_grid_a[i].column_a[label], search);
		if(start >= 0)
			ret_a.push_back(i);
	}
	return ret_a;
}

int BaseResGrid::column_search(const char* _label_name, const char* _search)
{
	if (*_search == 0)
		return -1;

	int label = label_find(_label_name);

	if (label < 0)
		return -1;

	STLString search = _search;
	for (unsigned i = 0; i < m_grid_a.size(); i++)
	{
		int start = BaseStringTable::find(m_grid_a[i].column_a[label], search);

		if (start >= 0)
			return i;
	}
	return -1;
}

bool BaseResGrid::column_remove(int _column_index)
{
	if(m_grid_a.size() <= _column_index || _column_index < 0)
		return false;

	m_grid_a.erase(m_grid_a.begin()+_column_index);
	return true;
}


int BaseResGrid::label_add(const char* _label_name)
{
	m_label.push_back(_label_name);
	return (int)m_label.size() - 1;
}

int BaseResGrid::label_find(const char* _label_name)
{
	if (*_label_name == 0 || m_label.empty())
		return -1;

	for (unsigned i = 0; i < m_label.size(); i++)
	{
		if (PTCOMPARE(m_label[i].c_str(), _label_name) == 0)
			return i;
	}

	return -1;
}



PtVector3 BaseResGrid::vector_get(int _column, const char* _label_name)
{
	PtVector3 col(1, 1, 1);

	if (_column < 0 || m_grid_a.size() <= _column)
		return col;

	int label = label_find(_label_name);

	if (label < 0)
		return col;

	float fValue;
	sscanf_s(m_grid_a[_column].column_a[label].c_str(), "%f", &fValue);		col.x = fValue / 255.f;	
	if (label + 1 >= m_label.size())
		return col;
	sscanf_s(m_grid_a[_column].column_a[label +1].c_str(), "%f", &fValue);		col.y = fValue / 255.f;
	if (label + 2 >= m_label.size())
		return col;
	sscanf_s(m_grid_a[_column].column_a[label +2].c_str(), "%f", &fValue);		col.z = fValue / 255.f;
	return col;
}

bool BaseResGrid::strings_add(STLVString _str_a)
{
	if (m_label.size() == 0)
		return false;
	BaseResGridColumn column;

	for (int i = 0; i < m_label.size(); i++)
	{
		if (i < _str_a.size())
			column.column_a.push_back(_str_a[i]);
		else
			column.column_a.push_back("");
	}
	m_grid_a.push_back(column);
	return true;
}

bool BaseResGrid::string_set(int _column, const char* _label_name, const char *_value)
{
	if (_column < 0 || m_grid_a.size() <= _column)
		return false;

	int label = label_find(_label_name);

	if (label < 0)
		return false;
	m_grid_a[_column].column_a[label] = _value;
	return true;
}

const char* BaseResGrid::string_get(int _column, const char* _label_name)
{
	if (_column < 0 || m_grid_a.size() <= _column)
		return NULL;

	int label = label_find(_label_name);

	if (label < 0)
		return NULL;
	return m_grid_a[_column].column_a[label].c_str();
}

const char* BaseResGrid::filename_get(const char* _name)
{
	for (unsigned i = 0; i < m_grid_a.size(); i++)
	{	
		if (PTCOMPARE(m_grid_a[i].column_a[2].c_str(),_name) == 0)
			return m_grid_a[i].column_a[3].c_str();
	}
	return NULL;
}

INT64 BaseResGrid::ctid_next_get()
{
	if(m_ctid_new == -1)
	{
		for(int i=0; i<m_grid_a.size(); i++)
		{
			int value = 0;
			sscanf_s(m_grid_a[i].column_a[0].c_str(), "%d", &value);
			if(m_ctid_new < value)
				m_ctid_new = value;
		}
	}
	m_ctid_new++;
	return m_ctid_new;
}

bool BaseResGrid::VariableSet(const char* _variable, const STLString& _value_str)
{
	int column = column_get("variable", _variable);
	if (column < 0) {
		STLVString str_a;
		str_a.push_back(_variable);
		strings_add(str_a);
	}

	return string_set(column, "value", _value_str.c_str());
}

bool BaseResGrid::VariableGet(const char* _variable, STLString& _value_str)
{
	int column = column_get("variable", _variable);
	if (column < 0)
		return false;

	const char *ret = string_get(column, "value");
	if (!ret)
		return false;
	_value_str = ret;
	return true;
}

bool BaseResGrid::VariableSet(const char* _variable, float _value_f)
{
	char buff[255];
	sprintf_s(buff, 255, "%f", _value_f);
	STLString str = buff;
	return VariableSet(_variable, str);
}

bool BaseResGrid::VariableGet(const char* _variable, float& _value_f)
{
	STLString str;
	bool ret = VariableGet(_variable, str);
	if (!ret)
		return ret;
	sscanf_s(str.c_str(), "%f", &_value_f);
	return true;
}

bool BaseResGrid::VariableSet(const char* _variable, int _value_n)
{
	char buff[255];
	sprintf_s(buff, 255, "%d", _value_n);
	STLString str = buff;
	return VariableSet(_variable, str);
}

bool BaseResGrid::VariableGet(const char* _variable, int& _value_n)
{
	STLString str;
	bool ret = VariableGet(_variable, str);
	if (!ret)
		return ret;
	sscanf_s(str.c_str(), "%d", &_value_n);
	return true;
}
