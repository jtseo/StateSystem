#include "stdafx.h"

static STLMnString	*ps_stlMnHashTable = NULL;
static char strNull[]	= {'N','U','L','L',0};

#include "BaseFile.h"

void UniqHashStr::debug_hash_save(const char *_strFile)
{
	BaseFile file;
	file.set_asc_seperator("\t");
	if (ps_stlMnHashTable == NULL ||
		file.OpenFile(_strFile, BaseFile::OPEN_WRITE))
		return;

	STLMnString::iterator	it;
	it = ps_stlMnHashTable->begin();
	for (; it != ps_stlMnHashTable->end(); it++){
		file.write_asc_string(it->second.c_str(), 0);
		file.write_asc_integer(it->first);
		file.write_asc_line();
	}
	file.CloseFile();
}

void UniqHashStr::debug_hash_add(const char *_strString)
{
	if (ps_stlMnHashTable == NULL)
		ps_stlMnHashTable = new STLMnString;

	int nHash = get_string_hash_code(_strString);
	STLMnString::iterator	it;
	it = ps_stlMnHashTable->find(nHash);

	if (it != ps_stlMnHashTable->end())
		return;

	(*ps_stlMnHashTable)[nHash] = _strString;
}

void UniqHashStr::debug_set_hashtext(int _nHash, const char *_strString)
{
	if(ps_stlMnHashTable == NULL)
		ps_stlMnHashTable	= new STLMnString;

	STLMnString::iterator	it;
	it	= ps_stlMnHashTable->find(_nHash);

	if (it != ps_stlMnHashTable->end())
	{
		if (it->second != _strString)
		{
			g_SendMessage(LOG_MSG, "duplicate hash:%s != %s", _strString, it->second.c_str());
		}
		return;
	}

	(*ps_stlMnHashTable)[_nHash]	= _strString;

	//if(strlen(_strString) < 130)
	//	g_SendMessage(LOG_MSG, "registhash:%s hash:%d", _strString, _nHash);
}

void UniqHashStr::debug_release_hashtext()
{
	delete ps_stlMnHashTable;
}

const char *UniqHashStr::debug_get_hashtext(int _nHash)
{
	if(!ps_stlMnHashTable)
		return strNull;
	
	STLMnString::iterator	it;
	it	= ps_stlMnHashTable->find(_nHash);

	if(it != ps_stlMnHashTable->end())
		return it->second.c_str();
	return strNull;
}


UniqHashStr::UniqHashStr( const char* _pcstr )
: pcstr_(_pcstr), uhash_( get_string_hash_code(pcstr_) )
{
	//XASSERT(pcstr_);
}

UniqHashStr::~UniqHashStr()
{}

const UniqHashStr & UniqHashStr::operator=( const UniqHashStr & rhs )
{
	this->pcstr_ = rhs.pcstr_;
	this->uhash_ = rhs.uhash_;
	return *this;
}

bool operator<( const UniqHashStr & lhs, const UniqHashStr & rhs )
{
	//! 유니크하다는 전제하에 스트링 비교 생략
	return lhs.uhash_ < rhs.uhash_;
}

/**
http://www.cs.yorku.ca/~oz/hash.html
http://www.flipcode.com/cgi-bin/msg.cgi?showThread=Tip-HashString&forum=totd&id=-1
*/

int UniqHashStr::get_string_hash_code( const wchar_t* _pstr, size_t _nSize )
{
    UINT32 ch;
    UINT32 hash_result = 5381;
    UINT32 len;
    
    if(_nSize == 0)
        len = (UINT32)wcslen( _pstr );
    else
        len	= (UINT32)_nSize;

	if (*(_pstr + len - 1) == L' ')
		len--;

	if (*(_pstr) == L' ')
	{
		len--;
		_pstr++;
	}
    
    for( UINT32 i=0; i<len; ++i )
    {
        ch = (UINT32)(_pstr[i]);
        hash_result = ((hash_result<< 5) + hash_result) + ch; // hash_result * 33 + ch
    }
    
    return (int)hash_result;
}

int UniqHashStr::get_string_hash_code( const char* _pstr, size_t _nSize )
{
	UINT32 ch;
	UINT32 hash_result = 5381;
	UINT32 len;
	
	if(_nSize == 0)
		len = (UINT32)strlen( _pstr );
	else
		len	= (UINT32)_nSize;

	if (len == 0)
		return hash_result;

	if (*(_pstr + len - 1) == ' ')
		len--;

	if (len == 0)
		return hash_result;

	if (*(_pstr) == ' ')
	{
		len--;
		_pstr++;

		if (len == 0)
			return hash_result;
	}

	for( UINT32 i=0; i<len; ++i )
	{
		ch = (UINT32)(_pstr[i]);
		hash_result = ((hash_result<< 5) + hash_result) + ch; // hash_result * 33 + ch
	}

#ifdef _DEBUG
	//debug_set_hashtext(hash_result, _pstr);
#endif
	return (int)hash_result;
}
