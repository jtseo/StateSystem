#include "stdafx.h"
#include "BaseWFile.h"

#ifndef _DEBUG
#ifdef _WIN32
#include <windows.h>
#else

#include <stdio.h>
#include <wchar.h>
#endif
#endif
#include <locale>

#ifndef ANDROID
//#include <codecvt>
#endif

BaseWFile::BaseWFile(void)
{
	m_bCheckUnicodePrefix	= true;

	wcscpy_s(m_wstrSeperator, ASC_MAX_LENGTH, L",\t");
	wcscpy_s(m_wstrDelete, ASC_MAX_LENGTH, L"");
	wcscpy_s(m_wstrCarriageReturn, ASC_MAX_LENGTH, L"\r\n");
	m_wstrBrace[0]	= NULL;
}

BaseWFile::~BaseWFile(void)
{
}

bool BaseWFile::read_asc_line()
{
	m_nCurCharAscLine = 0;
	int nCnt=0;

	do
	{
		while(m_nCurCharAscBlock < ASC_MAX_LENGTH/2
			&& m_nCurCharAscBlock < m_nSizeAscBlock)
		{
			check_brace_(m_wstrBufferAscBlock[m_nCurCharAscBlock]);

			if(m_stlVnLastBrace.empty() 
				&& (unsigned)m_wstrBufferAscBlock[m_nCurCharAscBlock] < 32
				&& !check_seperator_(m_wstrBufferAscBlock[m_nCurCharAscBlock])
				&& !check_delector_(m_wstrBufferAscBlock[m_nCurCharAscBlock]))
			{
				m_wstrBufferAscLine[nCnt] = NULL;

				while((unsigned)m_wstrBufferAscBlock[m_nCurCharAscBlock] < 32
					&& m_nCurCharAscBlock < ASC_MAX_LENGTH/2
					&& m_nCurCharAscBlock < m_nSizeAscBlock)
				{
					bool bEnter = false;
					if(m_wstrBufferAscBlock[m_nCurCharAscBlock] == '\n')	// \n만 있는 행을 위해 추가
						bEnter = true;

					m_nCurCharAscBlock++;
					if(m_nCurCharAscBlock >= m_nSizeAscBlock)
					{
						m_nSizeAscBlock = read_dump_( m_wstrBufferAscBlock, ASC_MAX_LENGTH)/2;
						m_nCurCharAscBlock = 0;
					}

					if(bEnter)
						break;
				}

				if(m_cComment != 0 && m_wstrBufferAscLine[0] == m_cComment) // '#'이 앞에 있으면 이라인은 건너 뛴다
				{
					nCnt = 0;
					continue;
				}
				return true;
			}

			m_wstrBufferAscLine[nCnt] = m_wstrBufferAscBlock[m_nCurCharAscBlock];
			nCnt++;
			m_nCurCharAscBlock++;
		}

		m_nSizeAscBlock = read_dump_( m_wstrBufferAscBlock, ASC_MAX_LENGTH)/2;
		if(m_nSizeAscBlock == 0 && m_nCurCharAscBlock > 0)
		{
			m_wstrBufferAscLine[nCnt]	= NULL;
			m_nCurCharAscBlock			= 0;
			return true;
		}
		m_nCurCharAscBlock = 0;
		if(m_bCheckUnicodePrefix)
		{
			B_ASSERT(m_wstrBufferAscBlock[0]==0xFeFF);
			if(m_wstrBufferAscBlock[0]!=0xFeFF)
			{
				m_bCheckUnicodePrefix	= true;
				return false;
			}
			m_nCurCharAscBlock++;
			m_bCheckUnicodePrefix	= false;
		}
	}while(m_nSizeAscBlock > 0);

	if(nCnt > 0)
	{
		if(m_cComment != 0 && m_wstrBufferAscLine[0] != m_cComment) // '#'이 앞에 있으면 이라인은 건너 뛴다
		{
			m_wstrBufferAscLine[nCnt] = 0;
			return true;
		}
	}

	return false;
}

const wchar_t *BaseWFile::token_get()
{
	return m_wstrToken;
}

bool BaseWFile::read_asc_integer( int* _pnNum )
{
	*_pnNum = 0;
	if(parse_token_())
	{	
		swscanf_s(m_wstrToken, L"%d", _pnNum);
		return true;
	}
	return false;
}


bool BaseWFile::read_asc_string( wchar_t *_strSource, const int nStrSize)
{
	*_strSource = NULL;
	if(parse_token_())
	{
		wchar_t *strStart = m_wstrToken;
		while(*strStart == 32)
			strStart++;
		wcscpy_s(_strSource, nStrSize, strStart);
		size_t nLen = wcslen(_strSource);
		while(_strSource[nLen-1] == 32)
			nLen--;
		_strSource[nLen] = 0;
		return true;
	}
	return false;
}

bool BaseWFile::write_asc_string(const char *_strSource, const int _nStrSize)
{
	//std::wstring wcsTemp(_strSource, _strSource + strlen(_strSource));
	//return write_asc_string(wcsTemp.c_str(), _nStrSize);

	wchar_t	wcsBuffer[ASC_MAX_LENGTH];
	BaseSystem::towide(_strSource, wcsBuffer, _nStrSize);
	//MultiByteToWideChar(CP_ACP, 0, _strSource, -1, wcsBuffer, ASC_MAX_LENGTH);
	
	//MultiByteToWideChar(CP_ACP, 0, _strSource, -1, wcsBuffer, ASC_MAX_LENGTH);
	//MultiByteToWideChar(CP_OEMCP, 0, _strSource, -1, wcsBuffer, ASC_MAX_LENGTH);
	//MultiByteToWideChar(CP_THREAD_ACP, 0, _strSource, -1, wcsBuffer, ASC_MAX_LENGTH);
	//MultiByteToWideChar(CP_UTF7, 0, _strSource, -1, wcsBuffer, ASC_MAX_LENGTH);
	//MultiByteToWideChar(CP_UTF8, 0, _strSource, -1, wcsBuffer, ASC_MAX_LENGTH);
	return write_asc_string(wcsBuffer, _nStrSize);
}

//std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
//return myconv.to_bytes(str);

bool BaseWFile::write_asc_integer64(INT64 _nNum)
{	
	wchar_t strBuffer[ASC_MAX_LENGTH];

	strBuffer[0] = 0;
	swprintf_s(strBuffer, ASC_MAX_LENGTH, L"%llu", _nNum);
	return write_asc_string(strBuffer, ASC_MAX_LENGTH);
}

bool BaseWFile::write_asc_float(float _value)
{
	wchar_t strBuffer[ASC_MAX_LENGTH];

	strBuffer[0] = 0;
	swprintf_s(strBuffer, ASC_MAX_LENGTH, L"%f", _value);
	return write_asc_string(strBuffer, ASC_MAX_LENGTH);
}

bool BaseWFile::write_asc_integer(int _nNum)
{
	wchar_t strBuffer[ASC_MAX_LENGTH];

	strBuffer[0] = 0;
	swprintf_s(strBuffer, ASC_MAX_LENGTH, L"%d", _nNum);
	return write_asc_string(strBuffer, ASC_MAX_LENGTH);
}

bool BaseWFile::write_asc_string( const wchar_t *_strSource, const int /* nStrSize*/)
{
	if (m_bFirstWritingSign)
	{
		m_nType |= OPEN_UCS2L;
	}
	
	if(!m_bFirstTokenLine)
	{
		m_wstrBufferAscLine[m_nCurCharAscLine] = m_wstrSeperator[0];
		m_nCurCharAscLine++;
	}
	
	m_bFirstTokenLine	= false;

	if(ASC_MAX_LENGTH - m_nCurCharAscLine <= 0)
		return false;

	size_t nLength = wcslen(_strSource);
	if(m_nCurCharAscLine+nLength >= ASC_MAX_LENGTH)
		return false;
	wcscpy_s(&m_wstrBufferAscLine[m_nCurCharAscLine], ASC_MAX_LENGTH - m_nCurCharAscLine, _strSource);
	m_nCurCharAscLine += (int)nLength;
	return true;
}


int BaseWFile::read_asc_string2( wchar_t *_strSource, const int nStrSize)
{
	*_strSource = NULL;
	if(parse_token_())
	{
		wchar_t *strStart = m_wstrToken;
		while(*strStart == 32)
			strStart++;
		wcscpy_s(_strSource, nStrSize, strStart);
		size_t nLen = wcslen(_strSource);
		while(_strSource[nLen-1] == 32)
			nLen--;
		_strSource[nLen] = 0;
		return (int)nLen;
	}
	return 0;
}

void BaseWFile::set_asc_deletor(const wchar_t *_strDelector)
{
	wcscpy_s(m_wstrDelete, ASC_MAX_LENGTH, _strDelector);
}

void BaseWFile::set_asc_seperator(const wchar_t *_strSeperator)
{
	wcscpy_s(m_wstrSeperator, ASC_MAX_LENGTH, _strSeperator);
}

void BaseWFile::set_asc_brace(const wchar_t *_strBrace)
{
	wcscpy_s(m_wstrBrace, ASC_MAX_LENGTH, _strBrace);
}

void BaseWFile::set_asc_brace(const char *_strBrace)
{
    std::wstring wcsTemp(_strBrace, _strBrace+strlen(_strBrace));
    wcscpy_s(m_wstrBrace, ASC_MAX_LENGTH, wcsTemp.c_str());
}

void BaseWFile::set_asc_deletor(const char *_strDelector)
{
    std::wstring wcsTemp(_strDelector, _strDelector+strlen(_strDelector));
    wcscpy_s(m_wstrDelete, ASC_MAX_LENGTH, wcsTemp.c_str());
	//MultiByteToWideChar(CP_ACP, 0, _strDelector, -1, m_wstrDelete, ASC_MAX_LENGTH);
}

void BaseWFile::set_asc_seperator(const char *_strSeperator)
{
    std::wstring wcsTemp(_strSeperator, _strSeperator+strlen(_strSeperator));
    wcscpy_s(m_wstrSeperator, ASC_MAX_LENGTH, wcsTemp.c_str());
	//MultiByteToWideChar(CP_ACP, 0, _strSeperator, -1, m_wstrSeperator, ASC_MAX_LENGTH);
}

bool BaseWFile::check_delector_(wchar_t _char)
{
	int nCnt=0;
	while(m_wstrDelete[nCnt] != 0
		&& nCnt < ASC_MAX_LENGTH)
	{
		if(m_wstrDelete[nCnt] == _char)
			return true;
		nCnt++;
	}
	return false;
}

bool BaseWFile::check_seperator_(wchar_t _char)
{
	int nCnt=0;
	while(m_wstrSeperator[nCnt] != 0
		&& nCnt < ASC_MAX_LENGTH)
	{
		if(m_wstrSeperator[nCnt] == _char)
			return true;
		nCnt++;
	}
	return false;
}

bool BaseWFile::check_brace_(wchar_t _char)
{
	if(!m_stlVnLastBrace.empty() && m_wstrBrace[m_stlVnLastBrace.back()+1] == _char)
	{
		m_stlVnLastBrace.pop_back();
		return true;
	}

	int nCnt=0;
	while(m_wstrBrace[nCnt] != 0
		&& nCnt < ASC_MAX_LENGTH)
	{
		if(m_wstrBrace[nCnt] == _char)
		{
			m_stlVnLastBrace.push_back(nCnt);
			return true;
		}
		nCnt+=2;
	}
	return false;
}

bool BaseWFile::parse_token_(bool _quot)
{
	if(m_nCurCharAscLine >= ASC_MAX_LENGTH/2)
		return false;

	//while(check_seperator_(m_wstrBufferAscLine[m_nCurCharAscLine])
	//	&& m_wstrBufferAscLine[m_nCurCharAscLine] != 0
	//	&& m_nCurCharAscLine < ASC_MAX_LENGTH/2)
	//	m_nCurCharAscLine++;

	if(m_wstrBufferAscLine[m_nCurCharAscLine] == 0)
		return false;

	while(check_delector_(m_wstrBufferAscLine[m_nCurCharAscLine])
		&& m_wstrBufferAscLine[m_nCurCharAscLine] != 0
		&& m_nCurCharAscLine < ASC_MAX_LENGTH/2)
	{
		m_nCurCharAscLine++;
	}

	int nCnt = 0;
	while(!check_seperator_(m_wstrBufferAscLine[m_nCurCharAscLine])
		&& m_wstrBufferAscLine[m_nCurCharAscLine] != 0
		&& m_nCurCharAscLine < ASC_MAX_LENGTH/2)
	{
		if(!check_delector_(m_wstrBufferAscLine[m_nCurCharAscLine]))
		{
			m_wstrToken[nCnt] = m_wstrBufferAscLine[m_nCurCharAscLine];
			nCnt++;
		}
		m_nCurCharAscLine++;
	}

	if(m_wstrBufferAscLine[m_nCurCharAscLine] != 0)
		m_nCurCharAscLine++;
	m_wstrToken[nCnt] = 0;

	for(int i=0; i<nCnt; i++)
		m_strToken[i]	= (char)m_wstrToken[i];
	return true;
}

const wchar_t *BaseWFile::get_asc_line()
{
	return m_wstrBufferAscLine;
}

bool BaseWFile::write_asc_line()
{
	m_bFirstTokenLine = true;
	if (m_nCurCharAscLine < ASC_MAX_LENGTH - 3)
	{
		for (unsigned i = 0; i<wcslen(m_wstrCarriageReturn); i++)
		{
			m_wstrBufferAscLine[m_nCurCharAscLine] = m_wstrCarriageReturn[i];
			m_nCurCharAscLine++;
		}
		m_wstrBufferAscLine[m_nCurCharAscLine + 1] = NULL;
	}

	int nRet = write_dump_(m_wstrBufferAscLine, m_nCurCharAscLine*2);

	if (nRet == m_nCurCharAscLine*2)
	{
		m_nCurCharAscLine = 0;
		return true;
	}
	return false;
}
