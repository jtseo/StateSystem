#pragma once

class BaseJson
{
public:
	PtObjectHeader(BaseJson);
	PT_OPTHeader;
public:
	BaseJson();
	virtual ~BaseJson();
	
	bool set(STLString _str); // set by brace string ==> {variable: value}
	STLString get();// return json string
	STLString get(const STLString &_name);// return value for the param variable
	bool get(const STLString &_name, int *_ret); // get integer of value for tha param variable
	bool get(const STLString &_name, INT64 *_ret);
	bool set(const STLString &_name, const STLString &_value); // set by array
	STLString &operator[](const STLString &_name);
	STLString operator[](const STLString &_name) const;
	STLMstrStr::const_iterator begin() const;
	STLMstrStr::const_iterator end() const;
	void typeSet(const STLString& _name, const char* _type);// int, float, array
protected:
	STLMstrStr	m_table_m;
	STLMstrStr	m_type_m;
};
