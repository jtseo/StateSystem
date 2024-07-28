#include "stdafx.h"

//#define UNREAL

#ifdef UNREAL
#include "CoreMinimal.h"
#endif

#ifdef _WIN32

	#include <winsock2.h>
	#include <windows.h>
	#include <Commdlg.h>
	#include <winsock.h>
	#include <MMSystem.h>

	#include <ws2tcpip.h>

	#include <errno.h>
	#include <stdlib.h>
	#include <tchar.h>
	#include <process.h>

	#include <codecvt>
	#include <shellapi.h>

	#include <stdio.h>
//#include <stdarg.h>
#endif

#ifndef _WIN32

#ifdef ANDROID
#include <thread>
#include <mutex>
#else
    #include <libkern/OSAtomic.h>
#endif
	#include <unistd.h>
	#include <sys/time.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdlib.h>
#include <locale.h>
#include <iostream>

#endif
#include <time.h>

#ifdef ANDROID
#include <android/log.h>
#endif
#define APPNAME	"StateBaseLib"
#include "BaseTime.h"

BaseSystem	baseSystem;
char		s_strRootPath[255] = {""};

BaseSystem::BaseSystem(void)
{
	GetCurrentPath(255, s_strRootPath);
	strcat_s(s_strRootPath, 255, SLASH);
}

char* BaseSystem::path_full_get(char* _strRetPath, size_t _nLen, const char* _strAppend)
{
	if (*_strAppend != '.')
		strcpy_s(_strRetPath, _nLen, _strAppend);
	else
		path_root_get(_strRetPath, _nLen, _strAppend);
	return _strRetPath;
}

void BaseSystem::path_root_set(char* _strRetPath)
{	
	strcpy_s(s_strRootPath, 255, _strRetPath);
	path_fix(s_strRootPath, 255);
}

char *BaseSystem::path_root_get(char *_strRetPath, size_t _nLen, const char *_strAppend)
{
	if(s_strRootPath[0] == 0)
	{
		GetCurrentPath(255, s_strRootPath);
		strcat_s(s_strRootPath, 255, SLASH);
	}
	char buffer[1024];
	strcpy_s(buffer, 1024, s_strRootPath);
	if(_strAppend)
		strcat_s(buffer, 1024, _strAppend);
	strcpy_s(_strRetPath, _nLen, buffer);
	return _strRetPath;
}

BaseSystem::~BaseSystem(void)
{
}


char *BaseSystem::module_get_path(char *_strRetPath, size_t _nLen, const char *_strAppend)
{
	char strAppend[255];
	module_get_name(strAppend, 255);
	strcat_s(strAppend, 255, _strAppend);
	path_root_get(_strRetPath, _nLen, strAppend);
	return _strRetPath;
}

char *BaseSystem::module_get_data(char *_strRetPath, size_t _nLen, const char *_strAppend)
{
    char strAppend[255];
    module_get_name(strAppend, 255);
    strcat_s(strAppend, 255, _strAppend);
    path_data_get(_strRetPath, _nLen, strAppend);
    return _strRetPath;
}

#ifndef _WIN32

#ifdef ANDROID
extern const char *__progname;
const char *
getprogname(void)
{
    return (__progname);
}
#endif

char *BaseSystem::module_get_name(char *_strRetPath, size_t _nLen)
{
	strcpy_s(_strRetPath, _nLen, getprogname());
	return _strRetPath;
}

#else

char *BaseSystem::module_get_name(char *_strRetPath, size_t _nLen)
{
	char strBuf[255];
	::GetModuleFileNameA(NULL, strBuf, 255);
	char *strHead, *strTail;
	strTail	= strrchr(strBuf, '.');
	*strTail	= NULL;
	strHead	= strrchr(strBuf, SLASH_C);
	if (strHead == NULL)
		strHead = strrchr(strBuf, '\\');
	strcpy_s(_strRetPath, _nLen, strHead+1);
	return _strRetPath;
}

#endif

#ifndef _WIN32

#ifdef ANDROID
//#include <sys/atomics.h>
#endif

char * BaseSystem::GetCurrentPath(unsigned _nSize, char *_strPath)
{
    getcwd(_strPath, _nSize);
    return _strPath;
}

#ifndef _MAC
// unit operation
INT32 BaseSystem::LFIncrement(INT32 *_pnLock)
{
#ifdef ANDROID
#ifdef _DEBUG
	if(_pnLock == NULL)
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "null access try................");
	else
		__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "try inc ................ %d", *_pnLock);
#endif
    INT32 ret = __sync_fetch_and_add(_pnLock, 1);
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "try end ................ %d", ret);
    ret++;
	return ret;
#else
    std::atomic<INT32> lockint(*_pnLock);
    int ret = lockint.fetch_add(1);
    *_pnLock = lockint;
    ret++;
	return ret;
    //return ::OSAtomicAdd32(1, (volatile int32_t*) _pnLock);
	//return ::AddAtomic(1, (SInt32*)_pnLock)+1;
#endif
}

// unit operation
INT32 BaseSystem::LFDecrement(INT32 *_pnLock)
{
#ifdef ANDROID
    __sync_fetch_and_add(_pnLock, -1);
    return *_pnLock;
#else
    std::atomic<INT32> lockint(*_pnLock);
    int ret = lockint.fetch_add(-1);
    *_pnLock = lockint;
    ret--;
    return ret;
    //return ::OSAtomicAdd32(-1, (volatile int32_t*) _pnLock);
#endif
}

 INT32 BaseSystem::LFCompareExchange(INT32 *_pnLock, INT32 _nNew, INT32 _nComperand)
{
#ifdef ANDROID
    __sync_val_compare_and_swap(_pnLock, _nComperand, _nNew);
    return *_pnLock;
#else
	return ::OSAtomicCompareAndSwap32((int)_nComperand, (int)_nNew,(volatile int32_t*) _pnLock);
#endif
	//return InterlockedCompareExchange((volatile LONG*)_pnLock, _nNew, _nComperand);
}
#endif

void BaseSystem::Sleep(UINT32 _nTime)
{
	::usleep((int)_nTime*1000);
}

UINT32 BaseSystem::timeGetTime()
{
	timeval tv;
	gettimeofday(&tv, 0);
	
	return (UINT32)( ( tv.tv_sec * 1000 ) + ( tv.tv_usec / 1000 ) );
}

void BaseSystem::WSAStartup()
{
//	WSADATA w;								/* Used to open Windows connection */
///	if (::WSAStartup(0x0101, &w) != 0)
//	{
//		fprintf(stderr, "Could not open Windows connection.\n");
//	}
}

void BaseSystem::WSACleanup()
{
//	::WSACleanup();
}

void BaseSystem::SetCurrentPath(char *_strPath)
{
	chdir(_strPath);
	//::SetCurrentDirectoryA(_strPath);
}

struct tm BaseSystem::localtime(time_t _Time)
{
	time_t tMax;
	tMax	= 31314256204;
	struct tm rTm, *pTConst;
	if(_Time < tMax)
	{
		pTConst = ::localtime(&_Time);
	}else
	{
		pTConst = ::localtime(&tMax);
	}
    memcpy(&rTm, pTConst, sizeof(rTm));
    
	return rTm;
}

char *BaseSystem::path_data_get(char *_strRetPath, size_t _nLen, const char *_strAppend)
{
    strcpy_s(_strRetPath, MAX_PATH, "/Library"); // for ios
	strcat_s(_strRetPath, _nLen, _strAppend);
	return _strRetPath;
}

int _traverse_(const char *_strPath, STLVString	*_pstlVFind, STLVString *_pstlVFolder)
{
    int nRet = 0;
    struct dirent * stFiles;
    DIR * stDirIn;
    struct stat stFileInfo;
    
	char strFullpath[1024], szFullName[1024];
    
	if(_strPath)
		strcpy_s(strFullpath, 255, _strPath);
	else
		strFullpath[0] = NULL;
    
    if ((stDirIn = opendir( strFullpath)) == NULL)
    {
        perror( strFullpath );
        return nRet;
    }
    
    while (( stFiles = readdir(stDirIn)) != NULL)
    {
        sprintf(szFullName, "%s/%s", strFullpath, stFiles -> d_name );
        
        if (lstat(szFullName, &stFileInfo) < 0)
            perror ( szFullName );
        
		if (S_ISDIR(stFileInfo.st_mode))
		{
			_pstlVFolder->push_back(stFiles->d_name);
		}else if((stFiles->d_name[0] == '.' && stFiles->d_name[1] == 0) ||
                 (stFiles->d_name[0] == '.' && stFiles->d_name[1] == '.' && stFiles->d_name[2] == 0))
		{
			int x=0;
			x++;
		}else{
			_pstlVFind->push_back(stFiles->d_name);
		}
        
	}
    closedir(stDirIn);
	return nRet;
}  // end main

#else

char * BaseSystem::GetCurrentPath(unsigned _nSize, char *_strPath)
{
    ::GetCurrentDirectoryA(_nSize, _strPath);
    return _strPath;
}
//*
// unit operation
INT32 BaseSystem::LFIncrement(INT32 *_pnLock)
{ 
	return InterlockedIncrement((volatile LONG*)_pnLock);
}

// unit operation
INT32 BaseSystem::LFDecrement(INT32 *_pnLock)
{
	return InterlockedDecrement((volatile LONG*)_pnLock);
}

INT32 BaseSystem::LFCompareExchange(INT32 *_pnLock, INT32 _nNew, INT32 _nComperand)
{
	return InterlockedCompareExchange((volatile LONG*)_pnLock, _nNew, _nComperand);
}
//*/
void BaseSystem::Sleep(UINT32 _nTime)
{
	::Sleep(_nTime);
}

UINT32 BaseSystem::timeGetTime()
{
	//return ::timeGetTime();
	static INT64	nTimeStart = 0;
	INT64	nTimeCur = (INT64) ::timeGetTime();

	if(nTimeStart == 0)
		nTimeStart = nTimeCur;

	int nRet;
	nRet = (int)(nTimeCur - nTimeStart);
	return nRet;
}

bool g_bOpenWSA = false;

void BaseSystem::WSAStartup()
{
	WSADATA w;								/* Used to open Windows connection */
	if (::WSAStartup(0x0101, &w) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
	}

	g_bOpenWSA	= true;
}

bool BaseSystem::get_ip_name(char *_strIP, int _nSize, const char *_strName)
{
	DWORD dwRetval;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	struct sockaddr_in  *sockaddr_ipv4;
	DWORD ipbufferlength = 46;

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	bool bCloseWSA	= false;
	if(!g_bOpenWSA)
	{
		WSAStartup();
		bCloseWSA	= true;
	}
	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	dwRetval = getaddrinfo(_strName, NULL, &hints, &result);
	if ( dwRetval != 0 ) {
		int retVal = (int)dwRetval;
		printf("getaddrinfo failed with error: %d\n", retVal);
		return false;
	}

	sockaddr_ipv4 = (struct sockaddr_in *) result->ai_addr;
	strcpy_s(_strIP, _nSize, inet_ntoa(sockaddr_ipv4->sin_addr) );

	printf("getaddrinfo returned success\n");

	freeaddrinfo(result);

	if(bCloseWSA)
		WSACleanup();
	return true;
}

void BaseSystem::WSACleanup()
{
	::WSACleanup();

	g_bOpenWSA	= false;
}

void BaseSystem::SetCurrentPath(char *_strPath)
{
	::SetCurrentDirectoryA(_strPath);
}

STLString *BaseSystem::GetCurrentPath(STLString *_strPath)
{
	char strPath[255];
	::GetCurrentDirectoryA(255,strPath);
	*_strPath	= strPath;
	return _strPath;
}

//typedef std::vector<WIN32_FIND_DATAA, PT_allocator<WIN32_FIND_DATAA> >		STLVFind;
int _traverse_(const char *_strPath, STLVString	*_pstlVFind, STLVString *_pstlVFolder)
{
	int nRet = 0;
    
	HANDLE handle;
	char strFullpath[1024];
	WIN32_FIND_DATAA find_data;
    
	if(_strPath)
		strcpy_s(strFullpath, 255, _strPath);
	else
		strFullpath[0] = NULL;
    
	char strWidechar[255] = "*.*";
    
	strcat_s(strFullpath, 255, strWidechar);
	handle = FindFirstFileA(strFullpath, &find_data);
    
	while(handle != INVALID_HANDLE_VALUE)
	{
		if(//(find_data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) &&
           !(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			int nHash = UniqHashStr::get_string_hash_code(find_data.cFileName);
			_pstlVFind->push_back(find_data.cFileName);
		}else if((find_data.cFileName[0] == '.' && find_data.cFileName[1] == 0)
                 || (find_data.cFileName[0] == '.' && find_data.cFileName[1] == '.' && find_data.cFileName[2] == 0))
		{
			int x=0;
			x++;
		}else{
			int nHash = UniqHashStr::get_string_hash_code(find_data.cFileName);
			_pstlVFolder->push_back(find_data.cFileName);
		}
        
		if(!FindNextFileA(handle, &find_data))
			break;
	}
    
	return nRet;
}


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

bool QueryKey(HKEY hKey, STLVString &_stlVText) 
{ 
	char    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	char    achClass[MAX_PATH] = "";  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys=0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode; 

	char  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME; 

	// Get the class name and the value count. 
	retCode = RegQueryInfoKeyA(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.
	//TCHAR	tcsLower[MAX_KEY_LENGTH];
	char	tcsLower[MAX_KEY_LENGTH];

	if (cSubKeys)
	{
		printf( "\nNumber of subkeys: %ld\n", cSubKeys);

		for (i=0; i<cSubKeys; i++) 
		{ 
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyExA(hKey, i,
				achKey, 
				&cbName, 
				NULL, 
				NULL, 
				NULL, 
				&ftLastWriteTime); 
			if (retCode == ERROR_SUCCESS) 
			{
				//_tprintf(TEXT("(%d) %s\n"), i+1, achKey);
				strcpy_s(tcsLower, MAX_KEY_LENGTH, achKey);
				_strlwr_s(tcsLower, MAX_KEY_LENGTH);
				for(unsigned j=0; j<_stlVText.size(); j++)
				{
					//STLWString wstr = BaseSystem::towide(_stlVText[j]);
					if(strstr(tcsLower, _stlVText[j].c_str()) != NULL)
					{
						return true;
					}
				}
			}
		}
	} 

	// Enumerate the key values. 

	if (cValues) 
	{
		printf( "\nNumber of values: %ld\n", cValues);

		for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
		{ 
			cchValue = MAX_VALUE_NAME; 
			achValue[0] = '\0'; 
			retCode = RegEnumValueA(hKey, i, 
				achValue, 
				&cchValue, 
				NULL, 
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS ) 
			{ 
				//_tprintf(TEXT("(%d) %s\n"), i+1, achValue); 
				strcpy_s(tcsLower, MAX_KEY_LENGTH, achKey);
				_strlwr_s(tcsLower, MAX_KEY_LENGTH);
				for(unsigned j=0; j<_stlVText.size(); j++)
				{
					//STLWString wstr = BaseSystem::towide(_stlVText[j]);
					if(strstr(tcsLower, _stlVText[j].c_str()) != NULL)
					{
						return true;
					}
				}
			} 
		}
	}
	return false;
}

int BaseSystem::get_drives(STLVString *_pstlVDrives)
{
	int nCnt;
	char *strBuff;
	char strBuffer[MAX_PATH];
	char strTemp[MAX_PATH];
	strBuff = strBuffer;
	nCnt = GetLogicalDriveStringsA(MAX_PATH, strBuff);

	_pstlVDrives->clear();

	int nLen;
	while(*strBuff != 0 && nCnt > 0)
	{
		strcpy_s(strTemp, MAX_PATH, strBuff);
		nLen	= (int)strlen(strTemp);
		*(strTemp+nLen)	= NULL;
		_pstlVDrives->push_back(strTemp);

		strBuff = strBuff+(nLen+1);
		nCnt-=(nLen+1);
	}

	return (int)_pstlVDrives->size();
}

bool BaseSystem::check_client_user(STLVString &_stlVText)
{
	bool bRet = false;
	HKEY hTestKey;

	if( RegOpenKeyEx( HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Microsoft\\Communicator"),
		0,
		KEY_READ,
		&hTestKey) == ERROR_SUCCESS)
	{
		if(QueryKey(hTestKey, _stlVText))
			bRet = true;
	}

	RegCloseKey(hTestKey);

	if( RegOpenKeyEx( HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Microsoft\\Office\\14.0\\Outlook\\Search\\Catalog"),
		0,
		KEY_READ,
		&hTestKey) == ERROR_SUCCESS)
	{
		if(QueryKey(hTestKey, _stlVText))
			bRet = true;
	}

	RegCloseKey(hTestKey);

	return bRet;
}

struct tm BaseSystem::localtime(time_t _Time)
{
	time_t tMax;
	tMax	= 31314256204;
	struct tm rTm;
	if(_Time < tMax)
	{
		::localtime_s(&rTm, &_Time);
	}else
	{
		::localtime_s(&rTm, &tMax);
	}
    
	return rTm;
}

char *BaseSystem::path_data_get(char *_strRetPath, size_t _nLen, const char *_strAppend)
{
	GetTempPathA(MAX_PATH, _strRetPath);
	strcat_s(_strRetPath, _nLen, _strAppend);
	return _strRetPath;
}
#endif

void BaseSystem::timeCurrent(SPtDateTime *_pDTime)
{
	time_t t = time(0);   // get time now
	struct tm now = localtime( t );

	_pDTime->s.sDate.s.year	= now.tm_year + 1900;
	_pDTime->s.sDate.s.month	= now.tm_mon+1;
	_pDTime->s.sDate.s.day	= now.tm_mday;

	_pDTime->s.sTime.s.nHour		= now.tm_hour;
	_pDTime->s.sTime.s.nMinute	= now.tm_min;
	_pDTime->s.sTime.s.nSecond	= now.tm_sec;
}

void BaseSystem::timeCurrent(SPtTime *_pTime)
{
	time_t t = time(0);   // get time now
	struct tm now = localtime( t );

	_pTime->s.nHour		= now.tm_hour;
	_pTime->s.nMinute	= now.tm_min;
	_pTime->s.nSecond	= now.tm_sec;
}

int BaseSystem::weekDay()
{
	time_t t = time(0);   // get time now
	struct tm now = localtime( t );

	return (int)now.tm_wday;
}

int	BaseSystem::GetFileDateList(const char *_strPath, const STLVString &_stlVFiles, STLVString *_pstlVDates)
{
	STLString pathBase = _strPath;
	
	char buf[255];
	STLString filepath;
	for(int i=0; i<_stlVFiles.size(); i++)
	{
		filepath = pathBase;
		filepath += _stlVFiles[i];
		
		SPtDateTime dt = file_datetime_get(filepath.c_str());
		
		BaseTime::make_date(dt, buf, 255);
		STLString str = buf;
		_pstlVDates->push_back(str);
	}
	
	return (int)_pstlVDates->size();
}

int	BaseSystem::GetFileList(const char *_strPath, STLVString *_pstlVFilename, STLVString *_pstlVFolders)
{
	STLVString	stlVFind, stlVFolder, stlVDate;
    
	_traverse_(_strPath, &stlVFind, &stlVFolder);
	UINT32	i;

	if(_pstlVFilename)
		for(i=0; i<stlVFind.size(); i++)
		{
			_pstlVFilename->push_back(stlVFind[i]);
		}
    
	if(_pstlVFolders)
		for(i=0; i<stlVFolder.size(); i++)
		{
			_pstlVFolders->push_back(stlVFolder[i]);
		}
	
	if(_pstlVFolders)
		for(i=0; i<stlVFolder.size(); i++)
		{
			_pstlVFolders->push_back(stlVFolder[i]);
		}
	
	return (int)(stlVFind.size() + stlVFind.size());
}

#ifdef _WIN32
unsigned BaseSystem::createthread(DEF_ThreadCallBackType, unsigned _nStackSize, void *_pParam)
{
    return (unsigned)_beginthread(_CallbackFunc, 0, _pParam);
}
void BaseSystem::endthread()
{
    _endthread();
}
#else
unsigned BaseSystem::createthread(DEF_ThreadCallBackType, unsigned _nStackSize, void *_pParam)
{
    unsigned nID = 0;
    pthread_t    nThread;
    if(pthread_create(&nThread, NULL, _CallbackFunc, _pParam) != 0)
        nID = 0;
    return nID;
}

void BaseSystem::endthread()
{
    //pthread_exit();
}

#include <ctype.h>

char* _strlwr_s( char* s, size_t _nSize)
{
    char* p = s;
    while ((*p = (int)tolower( (int)*p ))) p++;
    return s;
}
#endif

#if _MSC_VER == 1914
class std::locale::id std::codecvt<char16_t, char, struct _Mbstatet>::id;
#endif

bool _base_utf8_to_utf16(const char* _src, wchar_t *_wstr, size_t _size)
{
	wchar_t w = 0;
	int bytes = 0;
	wchar_t err = L'?';
	while(*_src){
		unsigned char c = *_src;
		if (c <= 0x7f) {//first byte
			if (bytes) {
				*_wstr = err; _wstr++;
				bytes = 0;
			}
			*_wstr = (wchar_t)c; _wstr++;
		}
		else if (c <= 0xbf) {//second/third/etc byte
			if (bytes) {
				w = ((w << 6) | (c & 0x3f));
				bytes--;
				if (bytes == 0) {
					*_wstr = w; _wstr++;
				}
			}
			else {
				*_wstr = err; _wstr++;
			}
		}
		else if (c <= 0xdf) {//2byte sequence start
			bytes = 1;
			w = c & 0x1f;
		}
		else if (c <= 0xef) {//3byte sequence start
			bytes = 2;
			w = c & 0x0f;
		}
		else if (c <= 0xf7) {//3byte sequence start
			bytes = 3;
			w = c & 0x07;
		}
		else {
			*_wstr = err; _wstr++;
			bytes = 0;
		}

		_src++;
	}
	if (bytes) {
		*_wstr = err; _wstr++;
	}
	*_wstr = NULL;
	return true;
}

STLWString _base_utf8_to_utf16(STLString src)
{	
	STLWString dest;
	dest.clear();
	wchar_t w = 0;
	int bytes = 0;
	wchar_t err = L'?';
	for (size_t i = 0; i < src.size(); i++){
		unsigned char c = (unsigned char)src[i];
		if (c <= 0x7f){//first byte
			if (bytes){
				dest.push_back(err);
				bytes = 0;
			}
			dest.push_back((wchar_t)c);
		}
		else if (c <= 0xbf){//second/third/etc byte
			if (bytes){
				w = ((w << 6)|(c & 0x3f));
				bytes--;
				if (bytes == 0)
					dest.push_back(w);
			}
			else
				dest.push_back(err);
		}
		else if (c <= 0xdf){//2byte sequence start
			bytes = 1;
			w = c & 0x1f;
		}
		else if (c <= 0xef){//3byte sequence start
			bytes = 2;
			w = c & 0x0f;
		}
		else if (c <= 0xf7){//3byte sequence start
			bytes = 3;
			w = c & 0x07;
		}
		else{
			dest.push_back(err);
			bytes = 0;
		}
	}
	if (bytes)
		dest.push_back(err);
	//auto p = reinterpret_cast<const char *>(utf8_string.data());
	//STLWString wstr;

	//try {
		//wstr = utf8_string.convert;
		//wstr = _base_conver.from_bytes(p, p + utf8_string.size());
	//}
	//catch (std::range_error err)
	//{
	//	for (int i = 0; i < utf8_string.size(); i++) // 
	//	{
	//		if (utf8_string[i] < 0) // in case of abnormal character, it can make normal by replace space (ascii 32)
	//			utf8_string[i] = 32;
	//	}

	//	try {
	//		wstr = _base_conver.from_bytes(p, p + utf8_string.size());
	//	}
	//	catch (std::range_error err)
	//	{
	//		return L"err cvt wide";
	//	}
	//}
	return dest;
}


STLWString BaseSystem::towide(const STLString &_str)
{
	STLWString wStr = _base_utf8_to_utf16(_str);


	//std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff,
	//	std::codecvt_mode::little_endian>, char16_t> cnv;
	//std::string utf8 = cnv.to_bytes(s);
	//if (cnv.converted() < s.size())
	//	throw std::runtime_error("incomplete conversion");
	return wStr;
}

wchar_t *BaseSystem::towide(const char *_str, wchar_t *_wcs, size_t _nSize)
{
#if defined(_WIN32) || defined(_MAC)
	STLWString wStr;
	wStr = _base_utf8_to_utf16(_str);
	wcscpy_s(_wcs, _nSize, wStr.c_str());
#else
	std::wstring wcsTemp(_str, _str + strlen(_str));
	wcscpy_s(_wcs, _nSize, wcsTemp.c_str());
#endif
//#ifdef _WIN32
//	MultiByteToWideChar(CP_ACP, 0, _str, -1, _wcs, (int)_nSize);
//#endif
	return _wcs;
}

char *BaseSystem::tomulti2(const char* _strU8, char* _strMulti, size_t _nSize)
{
	wchar_t u16[4096];
	B_ASSERT(_nSize < 4096);
	if (!_base_utf8_to_utf16(_strU8, u16, 4096))
		return NULL;
	tomulti((const wchar_t*)u16, _strMulti, _nSize);
	return _strMulti;
}

char *BaseSystem::tomulti(const char *_strU8, char *_strMulti, size_t _nSize)
{
#if defined(_WIN32) || defined(_MAC)
	STLWString u16;
	u16 = _base_utf8_to_utf16(_strU8);
	tomulti((const wchar_t*)u16.c_str(), _strMulti, _nSize);
#else
	strcpy_s(_strMulti, _nSize, _strU8);
#endif
	return _strMulti;
}

#if defined(_WIN32) || defined(_MAC)

#if _MSC_VER >= 1900

std::string _base_utf16_to_utf8(std::wstring utf16_string)
{
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	//auto p = reinterpret_cast<const wchar_t*>(utf16_string.data());
	//return convert.to_bytes(p, p + utf16_string.size()).c_str();

	std::string str;
	for (int i = 0; i < utf16_string.size(); i++)
		str.push_back((char)utf16_string[i]);
	return str;
}
//
//STLString _base_utf16_to_utf8(std::u16string utf16_string)
//{
//	std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
//	auto p = reinterpret_cast<const int16_t *>(utf16_string.data());
//	return convert.to_bytes(p, p + utf16_string.size()).c_str();
//}

#elif defined(_MAC)

#include <codecvt>

std::string _base_utf16_to_utf8(std::wstring utf16_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.to_bytes(utf16_string);
}

#else

std::string _base_utf16_to_utf8(std::wstring utf16_string)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(utf16_string);
}

#endif

#else

STLString _base_utf16_to_utf8(std::wstring utf16_string)
{
    STLString str(utf16_string.size(), '#');
    wcstombs(&str[0], (wchar_t*)utf16_string.c_str(), utf16_string.size());
    return str;
}

#endif // #if defined(_WIN32) || defined(_MAC)

char* BaseSystem::toutf8(const char *_multi, char* _utf8, size_t _nSize)
{
	static wchar_t wsc[1024];
    STLString u8;
    
#ifdef _WIN32
	MultiByteToWideChar(CP_ACP, 0, _multi, -1, wsc, 1024);
    u8 = _base_utf16_to_utf8(wsc).c_str();
#else
    towide(_multi, wsc, 1024);
    u8 = _multi;
#endif
	strcpy_s(_utf8, _nSize, u8.c_str());
	return _utf8;
}

STLString BaseSystem::toutf8(const STLWString &_wstr)
{
	STLString u8;
	u8 = _base_utf16_to_utf8(_wstr.c_str()).c_str();
	return u8;
}

char * BaseSystem::toutf8(const wchar_t* _wsc, char* _utf8, size_t _nSize)
{
#if defined(_WIN32) || defined(_MAC)
	STLString u8;
	u8 = _base_utf16_to_utf8(_wsc).c_str();
    strcpy_s(_utf8, _nSize, u8.c_str());
    return _utf8;
#else
    return tomulti((const wchar_t*)_wsc, _utf8, _nSize);
#endif
}

char *BaseSystem::tomulti(const wchar_t *_wsc, char *_str, size_t _nSize)
{
#ifdef _WIN32
	WideCharToMultiByte( CP_ACP, 0, _wsc, -1, _str, (int)_nSize, NULL, NULL );
#else
	std::wstring wcsTemp(_wsc);
	std::string strTemp(wcsTemp.begin(), wcsTemp.end());
	strcpy_s(_str, _nSize, strTemp.c_str());
#endif
	return _str;
}

int s_nCountRand = 0;
int BaseSystem::rand()
{
	s_nCountRand++;
	return ::rand();
}

int BaseSystem::randm()
{
	int nLow = rand();
	int nHigh = rand();
	int nRet = nLow << 5;
	nRet |= nHigh & 0x1F;
    long long nRndMax = RAND_MAX;
	long long nOther = nRndMax << 5;
	nOther |= RAND_MAX & 0x1F;

	long long nCal = (long long)nRet * 1000000l;
	nRet = (int)(nCal / nOther);
	return nRet;
}

bool s_nSRandFix = false;
void BaseSystem::srand_(unsigned _nInit)
{
	if(s_nSRandFix == false)
		srand(_nInit);
}

void BaseSystem::srand_fix()
{
	s_nSRandFix	= true;
}

void BaseSystem::srand_release()
{
	s_nSRandFix = false;
}

char *BaseSystem::path_fix(char *_strPath, int _nSize)
{
	char fromSlash = '\\';

	if (SLASH_C == '\\')
		fromSlash = '/';

	int nLen = (int)strlen(_strPath);
	if (nLen > _nSize)
		nLen = _nSize;

	for (int i = 0; i < nLen; i++) {
		if (*(_strPath + i) == fromSlash)
			*(_strPath + i) = SLASH_C;
	}
	return _strPath;
}

SPtDateTime BaseSystem::file_datetime_get(const char* _filename)
{
	struct stat t_stat;
	stat(_filename, &t_stat);
	struct tm timeinfo = localtime(t_stat.st_mtime); // or gmtime() depending on what you want
	
	SPtDateTime dt;
	dt = timeinfo;
	return dt;
}

#ifdef _WIN32

int BaseSystem::file_delete(const char *_strFile)
{
    DeleteFileA(_strFile);
    return 0;
}

int BaseSystem::folder_delete(const char *_strFolder)
{
    DeleteFileA(_strFolder);
    return 0;
}

int BaseSystem::folder_create(const char *_strFolder)
{
    CreateDirectoryA(_strFolder, NULL);
    return 0;
}

int BaseSystem::folder_change(const char *_strFolder)
{
    if(!SetCurrentDirectoryA(_strFolder))
        return -1;
    return 0;
}

int BaseSystem::folder_get(char *_strBuffer, int _nMax)
{
    if(!GetCurrentDirectoryA(_nMax, _strBuffer))
        return -1;
    return 0;
}

#else

int BaseSystem::file_delete(const char *_strFile)
{
    return remove(_strFile);
}

int BaseSystem::folder_delete(const char *_strFolder)
{
    return remove(_strFolder);
}

int BaseSystem::folder_create(const char *_strFolder)
{
    return mkdir(_strFolder, 0700);
}

int BaseSystem::folder_change(const char *_strFolder)
{
    return chdir(_strFolder);
}

int BaseSystem::folder_get(char *_strBuffer, int _nMax)
{
    if(getcwd(_strBuffer, _nMax) != NULL)
        return 0;
    return -1;
}
#endif

#ifdef _WIN32

int BaseSystem::run_shell_command(const char *_strCmd) 
{
	return system(_strCmd);
}

int BaseSystem::run_shell_command(const char* _command_str, const char* _app_str, const char* _param_str, bool _show_hide)
{
	int show_n;

	if (_show_hide)
		show_n = SW_SHOW;
	else
		show_n = SW_HIDE;

	ShellExecuteA(NULL, _command_str, _app_str, _param_str, NULL, show_n);
	return 1;
}
#else

int BaseSystem::run_shell_command(const char* _command_str, const char* _app_str, const char* _param_str, bool _show_hide)
{
    return execl(".", _app_str, _param_str);
}

int BaseSystem::run_shell_command(const char *_strCmd) {

    char *argv[10];
    
    //execvp(argv[0], argv);
#ifndef _IOS
    system(_strCmd);
#endif
	//execl(_strFile, _strCmd);
	return 0;
}

#endif

#ifdef _WIN32
char *BaseSystem::get_filenamedialogopen(const char *_strExt, char *_strRet, int _nSize)
{
	OPENFILENAMEA ofn;
	// a another memory buffer to contain the file name
	// open a file name
	if (_strExt == NULL)
		_strExt = "All\0*.*\0Scene\0*.scene\0";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = _strRet;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = (unsigned)_nSize;
	ofn.lpstrFilter = _strExt;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (!GetOpenFileNameA(&ofn))
		return NULL;

	return _strRet;
}

char *BaseSystem::get_filenamedialogsave(const char *_strExt, char *_strRet, int _nSize)
{	
	OPENFILENAMEA	ofn;
	// a another memory buffer to contain the file name
	// open a file name
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = _strRet;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = (unsigned)_nSize;
	ofn.lpstrFilter = "All\0*.*\0Scene\0*.scene\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
	if (!GetOpenFileNameA(&ofn))
		return NULL;

	return _strRet;
}

#else
char *BaseSystem::get_filenamedialogopen(const char *_strExt, char *_strRet, int _nSize)
{
	return NULL;
}

char *BaseSystem::get_filenamedialogsave(const char *_strExt, char *_strRet, int _nSize)
{
	return NULL;
}

#endif

#ifdef ANDROID
//std::mutex myMutex;
#endif

atomic_cnt::atomic_cnt(INT32 _init)
{
#ifdef _MAC
	m_pnCnt = new atomic_int(_init);
#else
#ifdef ANDROID
	m_pnCnt = new atomic_int(_init);
#else
	m_nCnt = _init;
#endif
#endif
}

atomic_cnt::~atomic_cnt()
{
#ifdef _MAC
	delete m_pnCnt;
	m_pnCnt = NULL;
#else
#ifdef ANDROID
	delete m_pnCnt;
	m_pnCnt = NULL;
#else
	m_nCnt = 0;
#endif
#endif
}

int atomic_cnt::operator++()
{
#ifdef _MAC
	INT32 ret = atomic_fetch_add_explicit(m_pnCnt, 1, memory_order_relaxed);
	ret++;
	return ret;
#else
#ifdef ANDROID
	//std::lock_guard<std::mutex> lock(myMutex);
	//int ret = ++m_nCnt;
	INT32 ret = atomic_fetch_add_explicit(m_pnCnt, 1, memory_order_relaxed);
	ret++;
	return ret;
#else
	return BaseSystem::LFIncrement(&m_nCnt);
#endif
#endif
}

int atomic_cnt::operator--()
{
#ifdef _MAC
	INT32 ret = atomic_fetch_sub_explicit(m_pnCnt, 1, memory_order_relaxed);
	ret--;
	return ret;
#else
#ifdef ANDROID
	//std::lock_guard<std::mutex> lock(myMutex);
	//int ret = --m_nCnt;
	INT32 ret = atomic_fetch_sub_explicit(m_pnCnt, 1, memory_order_relaxed);
	ret--;
	return ret;
#else
	return BaseSystem::LFDecrement(&m_nCnt);
#endif
#endif
}

int atomic_cnt::get() const
{
#ifdef _MAC
	return atomic_load_explicit(m_pnCnt, memory_order_relaxed);
#else
#ifdef ANDROID
	return atomic_load_explicit(m_pnCnt, memory_order_relaxed);
#else
	return m_nCnt;
#endif
#endif
}

#ifdef UNREAL

void* BaseSystem::memcpy(void* _p1, const void* _p2, size_t _cnt)
{
	return FMemory::Memcpy(_p1, _p2, _cnt);
}

void* BaseSystem::malloc(size_t _size)
{
	return FMemory::Malloc(_size);
}

void BaseSystem::free(void* _p)
{
	FMemory::Free(_p);
}

void BaseSystem::memset(void* _p, int _value, size_t _cnt)
{
	uint8 v = (uint8)_value;
	FMemory::Memset(_p, v, _cnt);
}

#else // else of UNREAL

void* BaseSystem::memcpy(void* _p1, const void* _p2, size_t _cnt)
{
	return ::memcpy(_p1, _p2, _cnt);
}

void* BaseSystem::malloc(size_t _size)
{
	return ::malloc(_size);
}

void BaseSystem::free(void* _p)
{
	::free(_p);
}


void BaseSystem::memset(void* _p, int _value, size_t _cnt)
{
	::memset(_p, _value, _cnt);
}


#endif // end of UNREAL
