#ifndef __base_define__
#define __base_define__

// Contents of base.h
//#ifndef GLOBALOVERLOADS_H
//#define GLOBALOVERLOADS_H

//#define SUPPORT_OTHER_NEW
//#include <AEEStdLib.h>
 
// SHOULD BE PUT IN EVERY FILE THAT
// DEFINES A CLASS!!!!!!!!!		

#include "Fixed.h"

void g_SendMessage_InitSocket(const char *_strName, const char *_strIP=0);
void g_ApplicationSet(const char* _strName);
void g_BreakPoint();
bool g_logger(bool _enable = false);
int g_logger_last_link(int _link);

void g_assert(bool _bTrue, const char *_strName, int _nLine);
void g_assert(int _nTrue, const char *_strName, int _nLine);

#define B_ASSERT(_bParam)	g_assert(_bParam, __FILE__, __LINE__)

typedef void (*fnDebugOut)(int nType, const char *_strOut, int _nLen);
void g_SendMessageSetDebugOut(fnDebugOut _fn);

typedef enum {
	LOG_MSG_POPUP,
	LOG_MSG,
	LOG_MSG_SYS_ERROR,
	LOG_MSG_PACKET,
	LOG_MSG_FILELOG,
	LOG_MSG_CONSOLE,
	LOG_MSG_PACKET_ERROR,
	LOG_MSG_STATE,
	LOG_MSG_STATE_ERROR,
	LOG_MSG_UI,
	LOG_MSG_UI_ERROR,
	LOG_MSG_MEM_LEAK,
	LOG_MSG_NORET,
	LOG_MSG_NOFILTER
} PtLogType;
#ifdef _DEBUG
//#include <Util/DebugConsol.h>

	int g_SendMessage(PtLogType nFilter, const char *strFormat, ... );
	void g_base_idle();

//*
//#define NEW(type) new type
//#define NEW_A(type, size)	new type[size]
//#define NEW_P(type, paramater)	new type paramater
//#define DEL_A(pParam) delete[] pParam
/*/
	#define NEW_A(type, size) (type*)G_ResCheckIn(__LINE__, __FILE__, new_my(sizeof(type)*size))
	#define NEW_P(type, paramater) (type*)G_ResCheckIn(__LINE__, __FILE__, new type paramater)
	#define NEW(type) (type*)G_ResCheckIn(__LINE__, __FILE__, new type)
	#define DEL_A(pParam) delete_my(pParam)

	void* operator new(size_t sz);
	void operator delete(void *p);
	void* operator new[](size_t sz);
	void operator delete[](void *p);

	void delete_my(void *p);
	void *new_my(size_t sz);
//*/
#else
	int g_SendMessage(PtLogType nFilter, const char *strFormat, ... );
	void g_base_idle();
	void* G_ResCheckIn(int , const char *, void *);
	bool G_ResCheckOut(void *);

	//void* operator new(size_t sz);
	//void operator delete(void *p);

	//void* operator new[](size_t sz);
	//void operator delete[](void *p);

	//#define NEW(type) new type
	//#define NEW_A(type, size)	new type[size]
	//#define NEW_P(type, paramater)	new type paramater

	//#define DEL_A(pParam) delete[] pParam
	//
	//#ifndef _MSC_VER
	//#define _MSC_VER	2000
	//#endif
#endif

//#if _MSC_VER < 1400// VS2005 lower verseion can use this
#ifndef _WIN32
#define fopen_s(file, strName, strCmd)	((*(file) = fopen(strName, strCmd)) == NULL)
#define wcscpy_s(strSrc, nSize, strTarget)	wcscpy(strSrc, strTarget)
#define sscanf_s	sscanf
#define swscanf_s   swscanf
#define strcat_s(strSrc, nSize, strTarget)	strcat(strSrc, strTarget)
#define strcpy_s(strSrc, nSize, strTarget)	strcpy(strSrc, strTarget)
#define strncpy_s(strSrc, nSize, strTarget, nLen)	strncpy(strSrc, strTarget, nLen)
#define _strlwr_s(strSrc, nSize)	strlwr(strSrc)
#define vsprintf_s(strSrc, nSize, strFmt, vaList)	vsprintf(strSrc, strFmt, vaList)
#define fprintf_s   fprintf

#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define swprintf_s swprintf
#define _strupr strupr
#define _strlwr	strlwr
#define _wcslwr wcslen

void OutputDebugString(const char *_out);

inline void sprintf_s(char *_strBuf, int _nSize, const char *_strFmt, ...)
{
	va_list  argptr;

    va_start(argptr, _strFmt);
    vsprintf(_strBuf, _strFmt, argptr);
    va_end(argptr);
}

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
#endif

#include <vector>
#include <map>
#include <set>
#include "BaseDefines.h"

#include "BaseCircleQueue.h"
#include "BaseMemoryPool3.h"

#include <cstdlib>
#include <exception>
#include <assert.h>
	//#include <new>
	//#include <xutility>
#include "BaseMemoryPool.h"

#include "PTAllocator.h"
#include "BaseTemplate.h"
#include "BaseStructure.h"
#include "BaseSystem.h"


class BaseConsolFilter{
public:
    BaseConsolFilter();
    ~BaseConsolFilter();
    
    static void reset();
    void hide_(const char *_strTitle, const char *_strValue);
    void show_(const char *_strTitle, const char *_strValue);
    
    static void hide(const char *_strTitle, const char *_strValue);
    static void show(const char *_strTitle, const char *_strValue);
    static void load_hash(const char*_strFile);
    
    bool load_filter(const char *_strFile);
    bool load_hash_(const char *_strFile);
    bool save_hash(const char *_strFile);
    const char*get_output(const char *_strOut, int _nLen);
	bool is_hide_all() {
		return m_bHideAll;
	}
protected:
    
    STLMnInt	m_stlMnStringHash;
    STLVString	m_stlVString;
    
	typedef std::map<int, STLVInt, std::less<int>, PT_allocator<std::pair<const int, STLVInt> > >	STLMnstlVValues;
    
    STLMnstlVValues	m_stlMnFilterShow;
    STLMnstlVValues	m_stlMnFilterHide;
	bool m_bHideAll;

private:
};

#endif
// EOF
