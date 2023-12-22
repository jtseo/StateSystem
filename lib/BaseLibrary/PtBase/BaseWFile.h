#pragma once

#include "BaseFile.h"

class BaseWFile :
	public BaseFile
{
public:
	BaseWFile(void);
	virtual ~BaseWFile(void);
	
	bool read_asc_string( wchar_t *_strSource, const int nStrSize);
	int read_asc_string2( wchar_t *_strSource, const int nStrSize);
	bool read_asc_integer( int* _pnNum );
	const wchar_t *token_get();

	bool write_asc_string( const wchar_t *_strSource, const int /* nStrSize*/);
	bool write_asc_string(const char *_strSource, const int /* nStrSize*/);
	bool write_asc_integer64(INT64 _nNum);
	bool write_asc_integer(int _nNum);
	bool write_asc_float(float _value);
	bool write_asc_line();

	virtual bool read_asc_line();
	const wchar_t *get_asc_line();

	virtual void set_asc_deletor(const char *_strDelector);
	virtual void set_asc_seperator(const char *_strSeperator);
	void set_asc_deletor(const wchar_t *_strDelector);
	void set_asc_seperator(const wchar_t *_strSeperator);
	bool check_delector_(wchar_t _char);
	bool check_seperator_(wchar_t _char);
	bool check_brace_(wchar_t _char);
	void set_asc_brace(const wchar_t *_strBrace);
	virtual void set_asc_brace(const char *_strBrace);
protected:
	virtual bool parse_token_(bool _quot = false);

	wchar_t m_wstrToken[ASC_MAX_LENGTH/2+1];
	wchar_t m_wstrBufferAscLine[ASC_MAX_LENGTH/2+1];
	wchar_t m_wstrBufferAscBlock[ASC_MAX_LENGTH/2+1];

	bool	m_bCheckUnicodePrefix;

	wchar_t m_wstrCarriageReturn[ASC_MAX_LENGTH];
	wchar_t m_wstrDelete[ASC_MAX_LENGTH];
	wchar_t m_wstrSeperator[ASC_MAX_LENGTH];
	wchar_t m_wstrBrace[ASC_MAX_LENGTH];
	STLVInt	m_stlVnLastBrace;
};
