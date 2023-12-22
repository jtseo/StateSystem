#include "stdafx.h"
#include "BaseSetString.h"

BaseSetString::BaseSetString(void)
{
}

BaseSetString::~BaseSetString(void)
{
	STLMnpString::iterator	it;
	for(it = m_stlMnpString.begin(); it != m_stlMnpString.end(); it++)
	{
		delete it->second;
	}
	m_stlMnpString.clear();
}

int BaseSetString::get_hash_lwr(const char *_str)
{
	char strBuf[255];
	strcpy_s(strBuf, 255, _str);
	_strlwr_s(strBuf, 255);
	return UniqHashStr::get_string_hash_code(strBuf);
}

void BaseSetString::add(const char *_str)
{
	int nHash	= get_hash_lwr(_str);
	STLMnpString::iterator	it;
	it	= m_stlMnpString.find(nHash);
	if(it == m_stlMnpString.end())
	{
		STLString *pstr		= new STLString(_str);//NEW_P(STLString, (_str));
		m_stlMnpString[nHash]	= pstr;
	}
}

void BaseSetString::del(const char *_str)
{
	int nHash	= get_hash_lwr(_str);
	STLMnpString::iterator	it;
	it	= m_stlMnpString.find(nHash);
	if(it != m_stlMnpString.end())
	{
		delete it->second;
		m_stlMnpString.erase(it);
	}
}

const char *BaseSetString::find(const char *_str)
{
	int nHash	= get_hash_lwr(_str);
	STLMnpString::iterator	it;
	it	= m_stlMnpString.find(nHash);
	if(it != m_stlMnpString.end())
	{
		return it->second->c_str();
	}
	return NULL;
}

size_t	BaseSetString::size()
{
	return m_stlMnpString.size();
}

void BaseSetString::get(STLVpString &_stlVpString)
{
	_stlVpString.clear();
	STLMnpString::iterator	it;
	for(it = m_stlMnpString.begin(); it != m_stlMnpString.end(); it++)
	{
		_stlVpString.push_back(it->second);
	}
}
