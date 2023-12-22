
#include "stdafx.h"
#include "../PtBase/BaseObject.h"

#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseSystem.h"

#include "../PtBase/BaseStringTable.h"
#include "BaseJson.h"

PtObjectCpp(BaseJson);
PT_OPTCPP(BaseJson);

BaseJson::BaseJson()
{

}

BaseJson::~BaseJson()
{

}

STLString get_str(BaseFile &_paser)
{
	STLString str;
	while(str.empty())
	{
		str = _paser.read_asc_quotation();
		if(!str.empty() || _paser.get_seperator_last())
			return str;
		if(!_paser.read_asc_line())
			return str;
	}
	return str;
}

STLMstrStr::const_iterator BaseJson::begin() const
{
	return m_table_m.begin();
}

STLMstrStr::const_iterator BaseJson::end() const
{
	return m_table_m.end();
}

void BaseJson::typeSet(const STLString& _name, const char* _type)// int, float, array
{
	m_type_m[_name] = _type;
}

bool BaseJson::set(STLString _str)
{
	if(_str.empty())
		return false;
	
	BaseFile paser;
	paser.set_asc_seperator(":,{}");
	paser.set_asc_deletor("\t \n\r");
	paser.openParser((char*)_str.c_str(), (int)_str.size());
	paser.read_asc_line();

	get_str(paser);
	if(paser.get_seperator_last() != '{')
		return false;
	
	STLString name, value;

	name = get_str(paser);
	if(name.empty())
		return false;
	while(!name.empty()){
		value = get_str(paser);
		if(!value.empty())
			m_table_m[name] = value;
		name = get_str(paser);
	}
	return true;
}

bool BaseJson::get(const STLString &_name, int *_ret) // return integer of value for the variable
{
	STLString value = get(_name);
	if(value.empty())
		return false;
	
	if(sscanf(value.c_str(), "%d", _ret) == 0)
		return false;
	
	return true;
}

bool BaseJson::get(const STLString &_name, INT64 *_ret)
{
	STLString value = get(_name);
	if(value.empty())
		return false;
	
	if(sscanf(value.c_str(), "%lld", _ret) == 0)
		return false;
	
	return true;
}

STLString BaseJson::get()
{
	STLString str = "{";
	STLMstrStr::iterator it;
	it = m_table_m.begin();
	for(; it != m_table_m.end(); it++)
	{
		if(it != m_table_m.begin())
			str += ",";
		str += '\"';
		str += it->first;
		str += "\":";

		if (m_type_m[it->first] == "int")
		{
			str += it->second;
		}
		else if (m_type_m[it->first] == "float")
		{
			str += it->second;
		}
		else if (m_type_m[it->first] == "array")
		{
			str += it->second;
		} else {
			str += "\"";
			str += it->second;
			str += '\"';
		}
	}
	str += "}";
	return str;
}

STLString &BaseJson::operator[](const STLString &_name)
{
	return m_table_m[_name];
}

STLString BaseJson::operator[](const STLString &_name) const
{
	STLString none;
	STLMstrStr::const_iterator it;
	it = m_table_m.find(_name);
	if(it == m_table_m.end())
		return none;
	return it->second;
}

STLString BaseJson::get(const STLString &_name)
{
	STLString none;
	STLMstrStr::iterator it;
	it = m_table_m.find(_name);
	if(it == m_table_m.end())
		return none;
	return it->second;
}

bool BaseJson::set(const STLString &_name, const STLString &_value)
{
	m_table_m[_name] = _value;
	return true;
}
