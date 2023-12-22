#include "stdafx.h"
#include "BaseFile.h"
#include "BaseStringTable.h"
#include <assert.h>
#ifdef _WIN32
#ifndef _DEBUG
#include <windows.h>
#endif
#else
// define for linux

#endif
BaseStringTable::STLMnstlVpTable	BaseStringTable::s_stlMnstlVpTable;
STLVInt	BaseStringTable::ms_stlVnLanguageNumber;
BaseStringTable	BaseStringTable::s_StringTable;
int				BaseStringTable::s_nLanguage	= -1;
int				BaseStringTable::ms_nLanguageMax = 0, BaseStringTable::ms_nCurColumn = 1;
char			s_strRoot[4096];

BaseStringTable::BaseStringTable(void)
{
	//init_();
}

BaseStringTable::~BaseStringTable(void)
{	
	release();
}

void BaseStringTable::replace(char *_strSource, const char *_strTo, const char *_strFrom)
{
	size_t	nCntFrom, nCntTo;
	nCntFrom	= strlen(_strFrom);
	nCntTo		= strlen(_strTo);
	char *strStart	= strstr(_strSource, _strFrom);
	char	strBuf[4096];

	if(strStart	!= NULL)
	{
		unsigned	nCnt=0;

		while(strStart > _strSource+nCnt)
        {strBuf[nCnt]	= _strSource[nCnt]; nCnt++;}

		for(unsigned i=0; i<nCntTo; i++, nCnt++)
			strBuf[nCnt]	= _strTo[i];

		strStart	+= nCntFrom;

		while(*strStart)
		{
			strBuf[nCnt]	= *strStart;
			strStart++;
			nCnt++;
		}
		strBuf[nCnt]	= *strStart;
		strcpy_s(_strSource, 4096, strBuf);
	}
}

void BaseStringTable::release()
{
	if(mpool_get().is_terminated())
		return;

	static bool bFinish = false;
	if(bFinish)
		return;

	bFinish = true;
	BaseStringTable::STLMnstlVpTable::iterator	it;
	it	= s_stlMnstlVpTable.begin();
	for(;it!=s_stlMnstlVpTable.end(); it++)
	{
		char **ppTable;
		ppTable	= it->second;
		for(int i=0; i<ms_nLanguageMax; i++)
			if(ppTable[i])
				PT_Free(ppTable[i]);

		PT_Free(ppTable);
	}
	s_stlMnstlVpTable.clear();
}

char *BaseStringTable::add_table(const char *_strString)
{
	int nLen = (int)strlen(_strString);

	char *str	= PT_Alloc(char, nLen+1);
	strcpy_s(str, nLen, _strString);
	return str;
}

void BaseStringTable::init_()
{
	release();
	BaseFile	file;

	char strPath[4096];
	strcpy_s(strPath, 4096, s_strRoot);
	strcat_s(strPath, 4096, "languagepack.bin");
	file.set_asc_deletor("\"");
	file.set_asc_seperator("\t");

	if(!file.OpenFile(strPath, BaseFile::OPEN_READ))
	{
		char strBuffer[4096];
		file.read_asc_line();	// language type
		ms_nLanguageMax = 0;
		while (file.read_asc_string(strBuffer, 4096) != 0)
		{
			ms_nLanguageMax++;
		}

		int nNumber;
		file.read_asc_line();	// language number
		while (file.read_asc_integer(&nNumber))
			ms_stlVnLanguageNumber.push_back(nNumber);

		while(file.read_asc_line())
		{
			char *strAlloc;
			int	nLen, i=0;
			char **ppTable;
			ppTable	= PT_Alloc(char*, ms_nLanguageMax);
			while((nLen = file.read_asc_string2(strBuffer, 4096)) != 0)
			{	
				if(nLen	!= 0)
				{
					replace(strBuffer, "\n", "\\n");
					strAlloc	= PT_Alloc(char, nLen+1);
					strcpy_s(strAlloc, nLen+1, strBuffer);
				}else{
					strAlloc	= NULL;
				}
				ppTable[i]	= strAlloc;
				i++;
			}

			if(ppTable[0])
				s_stlMnstlVpTable[get_key(ppTable[0])]	= ppTable;
		}
	}

	ms_nCurColumn = 1;
}

void BaseStringTable::reload()
{
	int nLangBankup	= s_nLanguage;
	init_();
	s_nLanguage	= nLangBankup;
}

int BaseStringTable::get_language()
{
	return s_nLanguage;
}

bool BaseStringTable::set_language(int _nLanguage, const char *_strRootPath)
{
	bool bRet = false;
	if(_strRootPath)
		strcpy_s(s_strRoot, 4096, _strRootPath);

	if (s_nLanguage == -1) {
		init_();
		bRet = true;
	}
	s_nLanguage	= _nLanguage;
	for (int i = 0; i < ms_nLanguageMax; i++) {
		if (ms_stlVnLanguageNumber[i] == _nLanguage)
			ms_nCurColumn = i;
	}
	return bRet;
}

void BaseStringTable::trim(STLString* _str)
{
	int start = 0;
	while (_str->at(start) == ' ')
		start++;

	*_str = _str->c_str() + start;

	start = (int)_str->size() - 1;
	while (_str->at(start) == ' ')
		start--;

	if (start < _str->size() - 1)
		(*_str)[start] = 0;
}

void BaseStringTable::trim(STLWString* _str)
{
	int start = 0;
	while (_str->at(start) == ' ')
		start++;

	*_str = _str->c_str() + start;

	start = (int)_str->size() - 1;
	while (_str->at(start) == ' ')
		start--;

	if (start < _str->size() - 1)
		(*_str)[start] = 0;
}

int BaseStringTable::find(const STLWString& _str, const STLWString& _sub)
{
	wchar_t* buff, * sub;

	buff = PT_Alloc(wchar_t, _str.size() + 2);
	sub = PT_Alloc(wchar_t, _sub.size() + 2);

	wcscpy_s(buff, _str.size() + 2, _str.c_str());
	wcscpy_s(sub, _sub.size() + 2, _sub.c_str());

	_wcslwr(buff);
	_wcslwr(sub);

	wchar_t* find = wcsstr(buff, sub);

	int ret = (int)(find - buff);
	PT_Free(buff);
	PT_Free(sub);
	if (!find)
		return -1;

	return ret;
}

int BaseStringTable::find(const STLString& _str, const STLString& _sub)
{
	char *buff, *sub;

	buff = PT_Alloc(char, _str.size() + 2);
	sub = PT_Alloc(char, _sub.size() + 2);

	strcpy_s(buff, _str.size()+2, _str.c_str());
	strcpy_s(sub, _sub.size()+2, _sub.c_str());

	_strlwr(buff);
	_strlwr(sub);
    
	char *find = strstr(buff, sub);

	int ret = (int)(find - buff);
	PT_Free(buff);
	PT_Free(sub);
	if (!find)
		return -1;

	return ret;
}

wchar_t* BaseStringTable::wcscut(wchar_t* _string, wchar_t _cut)
{
	wchar_t buff[4096];
	wchar_t* left = buff;
	wchar_t* right = _string;
	while (*_string) {
		if (*_string != _cut) {
			*left = *_string;
			left++;
		}
		_string++;
	}
	*left = *_string;

	wcscpy(right, buff);
	return right;
}

int BaseStringTable::compare2(const wchar_t* _left, const wchar_t* _right)
{
	wchar_t buf1[1024], buf2[1024];
	wcscpy_s(buf1, 1024, _left);
	wcscpy_s(buf2, 1024, _right);

	wcscut(buf1, L' ');
	wcscut(buf2, L' ');
    
#ifdef _WIN32
	return _wcsicmp(buf1, buf2);
#else
	
	for(int i=0; buf1[i]; i++)
		buf1[i] = towupper(buf1[i]);
	for (int i = 0; buf2[i]; i++)
		buf2[i] = towupper(buf2[i]);

    return wcscmp(buf1, buf2);
#endif
    
}

float BaseStringTable::similar(const char* _left, const char* _right)
{
	char buf1[1024], buf2[1024];
	strcpy_s(buf1, 1024, _left);
	strcpy_s(buf2, 1024, _right);

	cut_str(' ', buf1);
	cut_str(' ', buf2);

	int len1, len2;
	len1 = (int)strlen(buf1);
	len2 = (int)strlen(buf2);

	for (int i = 0; i < len1; i++)
		buf1[i] = toupper(buf1[i]);
	for (int i = 0; i < len2; i++)
		buf2[i] = toupper(buf2[i]);

	float total = (float)(len1 + len2), upcnt = 0.f;

	for (int i = 0; i < len1; i++)
	{
		for (int j = 0; j < len2; j++)
		{
			if (buf1[i] == buf2[j])
			{
				buf2[j] = 0;
				upcnt += 2.f;
				break;
			}
		}
	}
	if (total == 0)
		return 1;

	return upcnt / total;
}

float BaseStringTable::similar(const wchar_t* _left, const wchar_t* _right)
{
	wchar_t buf1[1024], buf2[1024];
	wcscpy_s(buf1, 1024, _left);
	wcscpy_s(buf2, 1024, _right);

	wcscut(buf1, L' ');
	wcscut(buf2, L' ');
		
	int len1, len2;
	len1 = (int)wcslen(buf1);
	len2 = (int)wcslen(buf2);

	for(int i=0; i<len1; i++)
		buf1[i] = towupper(buf1[i]);
	for (int i = 0; i < len2; i++)
		buf2[i] = towupper(buf2[i]);

	float total = (float)(len1+len2), upcnt = 0.f;

	for (int i = 0; i < len1; i++)
	{
		for (int j = 0; j < len2; j++)
		{
			if (buf1[i] == buf2[j])
			{
				buf2[j] = 0;
				upcnt += 2.f;
				break;
			}
		}
	}

	return upcnt / total;
}

int BaseStringTable::compare(const char* _left, const char* _right)
{
	char buf1[1024], buf2[1024];
	char *end;
	unsigned cnt = 0;
	while (*_left && cnt < 1024)
	{
		if (*_left == ' ')
			_left++;
		else
			break;
		cnt++;
	}
	cnt = 0;
	while (*_right && cnt < 1024)
	{
		if (*_right == ' ')
			_right++;
		else
			break;
		cnt++;
	}
	strcpy_s(buf1, 1024, _left);
	strcpy_s(buf2, 1024, _right);

	unsigned l = (unsigned)strlen(buf1);
	if (l == 0)
		return false;
	end = buf1 + l - 1;
	while (*end == ' ' && end >= buf1) end--;
	*(end+1) = 0;

	l = (unsigned)strlen(buf2);
	if (l == 0)
		return false;
	end = buf2 + l - 1;
	while (*end == ' ' && end >= buf1) end--;
	*(end+1) = 0;
#ifdef _WIN32
	return _strcmpi(buf1, buf2);
#else
	
	for(int i=0; buf1[i]; i++)
		buf1[i] = toupper(buf1[i]);
	for (int i = 0; buf2[i]; i++)
		buf2[i] = toupper(buf2[i]);
    return strcmp(buf1, buf2);
#endif
}

void BaseStringTable::cut_str(char _wChar, char *_strBuf)
{
	char	strBuf[4096];

	int nCnt	= 0, nMov=0;
	while(*(_strBuf+nCnt) && nCnt < 4096)
	{
		if(*(_strBuf+nCnt) == _wChar)
		{
		}else{
			strBuf[nMov]	= *(_strBuf+nCnt);
			nMov++;
		}
		nCnt++;
	}

	for(int i=0; i<nMov; i++)
	{
		*(_strBuf+i)	= strBuf[i];
	}
	*(_strBuf + nMov) = 0;
}

const char *BaseStringTable::get_str(const char *_strKey)
{
	if(_strKey == NULL || *_strKey == 0)
		return _strKey;

	if(ms_nCurColumn == -1)
		init_();

	int nKey	= get_key(_strKey);
	char	**ppTable;
	ppTable	= s_stlMnstlVpTable[nKey];
	//B_ASSERT(ppTable);
	if(ppTable==NULL || ppTable[ms_nCurColumn]==NULL)
	{
#ifdef _DEBUG
		char strBuf[4096];
		FILE	*pf = NULL;
		static bool bFirst	= true;
		if(bFirst)
		{	
			//fopen_s(&pf, BaseSystem::path_root_get(strBuf, 4096, "languagepack_dbg.ini"), "w");
			bFirst	= false;
		}else{
			//fopen_s(&pf, BaseSystem::path_root_get(strBuf, 4096, "languagepack_dbg.ini"), "a+");
		}
		if(pf)
		{
			BaseSystem::tomulti(_strKey, strBuf, 4096);
			fprintf_s(pf, "%s\r\n", strBuf);
			
			fclose(pf);
		}
#endif
		return _strKey;
	}
	B_ASSERT(ms_nCurColumn < ms_nLanguageMax);

	return ppTable[ms_nCurColumn];
}

//char BaseStringTable::s_strRetbuf[1024];
//
//char *BaseStringTable::get_str(const char *_strKey)
//{
//	if(_strKey == NULL || *_strKey == 0)
//	{
//		return NULL;
//	}
//
//	char wsrBuf[1024];
//
//    BaseSystem::towide(_strKey, wsrBuf, 1024);
//	
//	char *wsrRet	= get_str(wsrBuf);
//#ifdef _DEBUG
//	int nLen = wcslen(wsrRet);
//#endif
//
//    BaseSystem::tomulti(wsrRet, s_strRetbuf, 1024);
//    
//	return s_strRetbuf;
//}

int	BaseStringTable::get_key(const char *_strKey)
{
	char strBuffer[4096];
	char *pTemp	= strBuffer;

	while(*_strKey != 0)
	{
		*pTemp	= (char)*_strKey;
		pTemp++;
		_strKey++;
	}
	*pTemp	= NULL;

	return (int)UniqHashStr::get_string_hash_code(strBuffer);
}

char *BaseStringTable::set_number(char *_strNumber, int _nNum)
{
	char strBuffer[4096];
	strBuffer[0]	= 0;

	int	nUp;

	int nCnt	= 0;
	int nComma	= 0;
	bool minus	= false;
	if(_nNum < 0)
    {minus	= true; _nNum *= -1;}

	do{
		nUp	= _nNum/10;
		if(nComma == 3)
		{
			strBuffer[nCnt]	= ',';
			nComma	= 0;
			nCnt++;
		}
		strBuffer[nCnt]	= '0'+(char)(_nNum - nUp*10);
		nCnt++;
		nComma++;
		_nNum	= nUp;
	}while(nUp != 0);

	if(minus)
	{
		strBuffer[nCnt] = '-';
		nCnt++;
	}

//	int nPos	= 0;
	for(int i=nCnt-1; i>=0; i--)
	{
		_strNumber[nCnt-i-1]	= strBuffer[i];
	}
	_strNumber[nCnt]	= 0;

	return _strNumber;
}
