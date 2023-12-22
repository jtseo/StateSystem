#pragma once

//////////////////////////////////////////////////////////////////////////
// 다중 언어를 지원하기 위한 테이블로
// 영어를 키로 사용해 다른 언어로 번역된 문자열을 리턴한다.

class BaseStringTable
{
public:

	BaseStringTable(void);
	virtual ~BaseStringTable(void);

	//==========================================================================
	// Start Update by OJ : 2010-07-12
	// string utility
	static char *set_number(char *_strNumber, int _nNum); // add comma in number
	//================================= End Update by OJ : 2010-07-12

	static void reload();
	static int get_language();
	static bool	set_language(int _nLanguage, const char *_strRootPath);
	static const char *get_str(const char *_strKey);
	static char	*add_comma(char *_strRet, int _nNumber);

	static char *add_table(const char *_strString);
	static void release();
	
protected:
	static int ms_nLanguageMax, ms_nCurColumn;

	static STLVInt	ms_stlVnLanguageNumber;
	typedef std::map<int, char**>	STLMnstlVpTable;
	static STLMnstlVpTable	s_stlMnstlVpTable;
	static BaseStringTable	s_StringTable;
	static int				s_nLanguage;
	
	static void init_();
	static int	get_key(const char *_strKey);
	static void replace(char *_strSource, const char *_strTo, const char *_strFrom);
	
	static char s_strRetbuf[1024];
	// string operation
public:
	static int compare(const char* _left, const char* _right);
	static int compare2(const wchar_t* _left, const wchar_t* _right);
	static float similar(const char* _left, const char* _right);
	static float similar(const wchar_t* _left, const wchar_t* _right);
	static wchar_t* wcscut(wchar_t* _string, wchar_t _cut);
	static void cut_str(char _wChar, char* _strBuf);
	static int find(const STLString &_str, const STLString &_sub);
	static int find(const STLWString& _str, const STLWString& _sub);
	static void trim(STLWString* _str);
	static void trim(STLString* _str);
};


#define PTGET(x)	BaseStringTable::get_str(x)
#define PTCOMPARE(a, b) BaseStringTable::compare(a, b)
#define PTCOMPAREW(a,b) BaseStringTable::compare2(a, b)
#define PTSIMILAR(a, b) BaseStringTable::similar(a, b)