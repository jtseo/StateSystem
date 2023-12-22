#pragma once

#ifdef _WIN32
#define DEF_ThreadCallBack(callbackFunc)    void __cdecl callbackFunc##_(void *_pParam)
#define DEF_ThreadCallBackType  void (__cdecl *_CallbackFunc)(void*)
#define DEF_ThreadReturn	return

#define SLASH   "\\"
#define SLASH_C   '\\'

//#define SLASH   "/"
//#define SLASH_C   '/'

#else
#define DEF_ThreadCallBack(callbackFunc)    void* callbackFunc##_(void*_pParam)
#define DEF_ThreadCallBackType  void *(*_CallbackFunc)(void*)
#define DEF_ThreadReturn    return NULL

#define CRITICAL_SECTION				pthread_mutex_t
#define InitializeCriticalSection(a)	pthread_mutex_init(a, NULL)
#define DeleteCriticalSection		pthread_mutex_destroy
#define EnterCriticalSection		pthread_mutex_lock
#define LeaveCriticalSection		pthread_mutex_unlock
#define TlsGetValue					pthread_getspecific
#define TlsSetValue					pthread_setspecific
#define TlsFree						pthread_key_delete

#define SLASH   "/"
#define SLASH_C   '/'

#endif
class BaseSystem
{
public:
	BaseSystem(void);
	~BaseSystem(void);
	#ifndef _MAC
	static INT32 LFIncrement(INT32 *_pnLock);
	static INT32 LFDecrement(INT32 *_pnLock);
	static INT32 LFCompareExchange(INT32 *_pnLock, INT32 _nNew, INT32 _nComperand);
	#endif
	static void Sleep(UINT32 _nTime);
	static UINT32 timeGetTime();
	static void *memcpy(void* _p1, const void* _p2, size_t _cnt);
	static void *malloc(size_t _size);
	static void memset(void* _p, int _value, size_t _cnt);
	static void free(void* _p);

	static void SetCurrentPath(char *_strPath);
	static STLString *GetCurrentPath(STLString *_strPath);
    static char *GetCurrentPath(unsigned _nSize, char *_strPath);

	static void WSAStartup();
	static void WSACleanup();
	static struct tm localtime(time_t _Time);
	static void timeCurrent(SPtTime *_pTime);
	static void timeCurrent(SPtDateTime *_pDTime);
	static int weekDay();
	static int	GetFileList(const char *_strPath, STLVString *_pstlVFilename, STLVString *_pstlVFolders);
	static bool check_client_user(STLVString &_stlVText);
	static bool get_ip_name(char *_strIP, int _nSize, const char *_strName);
	static char* path_full_get(char* _strRetPath, size_t _nLen, const char* _strAppend);
	static void path_root_set(char* _strRetPath);
	static char *path_root_get(char *_strRetPath, size_t _nLen, const char *_strAppend);
	static char *path_data_get(char *_strRetPath, size_t _nLen, const char *_strAppend);
	static char *module_get_name(char *_strRetPath, size_t _nLen);
	static char *module_get_path(char *_strRetPath, size_t _nLen, const char *_strAppend);
    static char *module_get_data(char *_strRetPath, size_t _nLen, const char *_strAppend);
	static int get_drives(STLVString *_pstlVDrives);
	static int run_shell_command(const char *_strCmd);
	static int run_shell_command(const char* _command_str, const char* _app_str, const char* _param_str, bool _show_hide);
	static char *get_filenamedialogopen(const char *_strExt, char *_strRet, int _nSize);
	static char *get_filenamedialogsave(const char *_strExt, char *_strRet, int _nSize);
    
    static unsigned createthread(DEF_ThreadCallBackType, unsigned _nStackSize, void *_pParam);
    static void endthread();
	static STLWString towide(const STLString &);
    static wchar_t *towide(const char *_str, wchar_t *_wcs, size_t _nSize);
	static char *tomulti(const wchar_t *_wsc, char *_str, size_t _nSize);
#ifdef _WIN32
	static char *tomulti(const char16_t *_wsc, char *_str, size_t _nSize);
#endif
	static char *tomulti(const char *_strU8, char *_strMulti, size_t _nSize);
	static char *tomulti2(const char* _strU8, char* _strMulti, size_t _nSize);
	static STLString toutf8(const STLWString &_wstr);
	static char *toutf8(const wchar_t*_wsc, char* _utf8, size_t _nSize);
	static char* toutf8(const char *_multi, char* _utf8, size_t _nSize);
	static int rand();
	static int randm(); // a million random
	static void srand_(unsigned _nInit);
	static void srand_fix();
	static void srand_release();
    
	static char *path_fix(char *_strPath, int _nSize);
	static SPtDateTime file_datetime_get(const char* _filename);
    static int file_delete(const char *_strFile);
    static int folder_delete(const char *_strFolder);
    static int folder_create(const char *_strFolder);
    static int folder_change(const char *_strFolder);
    static int folder_get(char *_strBuffer, int _nMax);
};

#define PT_SRAND() \
		static bool bSrandFlag = true; \
		if(bSrandFlag)	\
		{	\
			BaseSystem::srand_( (unsigned)time( NULL ) );	\
			bSrandFlag = false;	\
		}

#ifndef _WIN32
char* _strlwr_s( char* s, size_t _nSize);
#endif


class atomic_cnt
{
public:	
	atomic_cnt(INT32 _init);
	~atomic_cnt();
	
	int operator++();
	int operator--();
	int get() const;
#ifndef _MAC
#ifdef	ANDROID
	//CRITICAL_SECTION	m_lock;
	atomic_int* m_pnCnt;
#endif
	INT32		m_nCnt;	// Decrease after pop
#else
	atomic_int	*m_pnCnt;
#endif
};
