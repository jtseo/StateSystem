// Contents of base.cpp
#include "stdafx.h"

#ifdef _WIN32
#include <winsock.h>
//#include <ws2def.h>
#endif

#include "base.h"
#include "BaseMemoryPool.h"
#include "BaseObject.h"
#include "BaseSocket.h"
#include "DebugContext.h"

#ifdef ANDROID
#include <android/log.h>
#define APPNAME "StateBaseLib"
#define TRACE_STATE
#endif

#ifdef _DEBUG
#define TRACE_STATE
#endif
// Override the default implementation (which makes use of
// malloc and free - unsupported in BREW) to use BREW's own
// MALLOC and FREE.

#ifdef _DEBUG

//
//#else
//void* operator new(size_t sz)
//{
//	return MALLOC(sz);
//}
//
//void operator delete(void *p)
//{
//	FREE(p);
//}
//
//void* operator new[](size_t sz)
//{
//	return MALLOC(sz);
//}
//
//void operator delete[](void *p)
//{
//	FREE(p);
//}
//INT32
#else

void* G_ResCheckIn(int , const char *, void *pNew)
{
	return pNew;
}

bool G_ResCheckOut(void *)
{
	return true;
}

#endif


static BaseSocket *gs_pSocket	= NULL;
static bool gs_logger_enable = false;

#include <ctime>
#include <iostream>

char s_strApplication[128];
static char log_path[255] = "";
static char s_strIP[1024] = "";
	
bool g_logger(bool _enable)
{
    if (_enable)
        gs_logger_enable = true;
    return gs_logger_enable;
}

int g_logger_last_link(int _link)
{
    static int link_last = 0;

    if (_link == 0)
        return link_last;
    else
        link_last = _link;
    return link_last;
}

void g_ApplicationSet(const char* _strName)
{
    if (_strName)
        strcpy_s(s_strApplication, 128, _strName);
}

void g_SendMessage_InitSocket(const char *_strName, const char *_strIP)
{
    if(_strIP != NULL)
    {
        if(strncmp(_strIP, "forrelease", 10) == 0)
        {
            gs_pSocket = NULL;
            return;
        }
    }

    gs_logger_enable = true;
    
	if(gs_pSocket == NULL && _strIP)
	{
		if(_strName)
			strcpy_s(s_strApplication, 128, _strName);

        gs_pSocket = new BaseSocket(false, _strIP, 16260);
        gs_pSocket->bind();

		strcpy_s(s_strIP, 1024, _strIP);
        
        FILE *pf = NULL;
        if(!fopen_s(&pf, "logpos.txt", "r"))
        {
            fgets(log_path, 255, pf);
            fclose(pf);
        }
	}
	else if(_strIP == NULL && gs_pSocket){
        gs_pSocket->weakup();
		//gs_pSocket->release();
		//delete gs_pSocket;

		//gs_pSocket = new BaseSocket(false, _strIP, 16260);
		//gs_pSocket->bind();
	}
}

#include "BaseMemoryPool.h"
#include "BaseFile.h"

#ifdef _DEBUG
extern int s_DebugDataCheckInt;
extern int *s_pDebugDataCheckPoint;
#include "BaseDStructure.h"
#endif

void g_BreakPoint()
{
#ifdef _DEBUG
	if(s_pDebugDataCheckPoint != NULL 
		&& *s_pDebugDataCheckPoint != 0xfdfdfdfd
		&& *s_pDebugDataCheckPoint != 0xcececece
		&& *s_pDebugDataCheckPoint != s_DebugDataCheckInt)
	{
		int x=0;
	}
	//static bool bLocal = false;
	//static int nCount = 100;

	//if (bLocal)
	//	return;

	//nCount--;
	//if (nCount > 0)
	//	return;

	//bLocal = true;
	//fnEventProcessor	fnFunc = NULL;
	//
	//INT32 nHash = STRTOHASH("BaseFalse_nIf");
	//STLMnpFuncEventProcessor::iterator it;
	//it = BaseDStructure::sm_stlMnpEventProcessor.find(nHash);

	//if (it != BaseDStructure::sm_stlMnpEventProcessor.end())
	//	fnFunc = it->second;

	//if (fnFunc)
	//{
	//	if(0 != fnFunc(NULL, NULL, NULL, 0))
	//		int x  = 0;
	//}
	//bLocal = false;
#endif
}


void g_assert(int _nTrue, const char *_strName, int _nLine)
{
	g_assert(_nTrue != 0, _strName, _nLine);
}


void g_assert(bool _bTrue, const char *_strName, int _nLine)
{
	if(!_bTrue)
	{
		int x=0;
		x++;
#ifdef _DEBUG
		g_SendMessage(LOG_MSG_POPUP, "Asserted in %s, %d", _strName, _nLine);
#endif
	}
}

#ifndef _WIN32
void OutputDebugStringA(const char *_out)
{
	std::cout << _out << "\n";
}
#endif

static fnDebugOut s_fnDebugOut = NULL;

void g_SendMessageSetDebugOut(fnDebugOut _fn)
{
    s_fnDebugOut = _fn;
}

BaseConsolFilter    *s_pfilter = NULL;

int _sendmsg(int _nFilter, const char *_msg)
{
    int nRet = 0;

	static int s_nLastFilter = 0;
	char strSendBuf[1150];
	strSendBuf[0] = 0;
	if (s_nLastFilter != 128) {
		strcpy_s(strSendBuf, 1150, s_strApplication);
	}

	if (s_nLastFilter != 129)
		s_nLastFilter = 0;

	strcat_s(strSendBuf, 1150, " ");
	strcat_s(strSendBuf, 1150, _msg);
	int nLen = (int)strlen(strSendBuf);
	if (nLen > 255)
	{
		int x = 0;
		x++;
	}
	UINT32 nTime;
	nTime = BaseSystem::timeGetTime();

	const char *strSendFiltered;
	if (s_pfilter && _nFilter != LOG_MSG_MEM_LEAK)
		strSendFiltered = s_pfilter->get_output(strSendBuf, nLen + 1);
	else
		strSendFiltered = strSendBuf;

	if (strSendFiltered && s_nLastFilter != 129)
	{
		if (s_fnDebugOut)
		{
			s_fnDebugOut(_nFilter, strSendFiltered, nLen + 1);
		}

		//char strOutMulti[1024];
		//BaseSystem::tomulti(strSendFiltered, strOutMulti, 1024);
		//puts(strSendFiltered);
        char out_str[1024];
        strcpy_s(out_str, 1024, strSendFiltered);
        strcat_s(out_str, 1024, "\r\n");

#ifdef ANDROID
        //__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", out_str);

#else
		OutputDebugStringA(out_str);
#endif
        //printf("%s", strOutMulti);

		if (gs_pSocket && gs_pSocket->is_connected())
		{
			//printf("%10d::%s\n", nTime, strSendBuf);
			strcpy_s(strSendBuf, 1150, strSendFiltered);
			nLen = (int)strlen(strSendBuf);
			if (_nFilter == LOG_MSG_NORET) {
                strSendBuf[nLen + 1] = -1;// 128 is change to -1 becuase error C4309: '=' truncation of constant value.
				nLen++;
				s_nLastFilter = 128;
			}
            gs_pSocket->send(strSendBuf, nLen + 1, (char*)1);
		}
		nRet = 1;
	}
	else {
		if (_nFilter == 0 && s_fnDebugOut)
		{
			s_fnDebugOut(_nFilter, strSendBuf, nLen + 1);
		}

		nRet = 0;

		s_nLastFilter = 0;
		if (_nFilter == LOG_MSG_NORET)
			s_nLastFilter = 129;
	}
		//#endif
	return nRet;
}


static BaseCircleQueue *pquMsg = NULL;

void g_base_idle()
{
	if (pquMsg == NULL) {
		pquMsg = new BaseCircleQueue("BaseMsgQueue");
		pquMsg->init(102400);
	}

	char *strThreadMsg = (char*)pquMsg->pop();
	while (strThreadMsg) {
		_sendmsg(LOG_MSG, strThreadMsg);
		strThreadMsg = (char*)pquMsg->pop();
	}
}

int g_SendMessage(PtLogType nFilter, const char* strFormat, ...)
{
    char logbuf[1024];
    if (nFilter == LOG_MSG_MEM_LEAK
        || nFilter == LOG_MSG_POPUP)
    {
        char temp[1024];
        if (strlen(strFormat) >= 1022)
        {   
            memcpy(temp, strFormat, 1022);
            temp[1023] = 0;
            strFormat = temp;
        }
        va_list	argptr;
        va_start(argptr, strFormat);
        vsprintf_s(logbuf, 1024, strFormat, argptr);
        va_end(argptr);
    }

    if (nFilter == LOG_MSG_MEM_LEAK)
    {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", logbuf);
#else
        OutputDebugStringA(logbuf);
#endif
    }
    
    if (nFilter == LOG_MSG_POPUP)
    {
#ifdef _WIN32
#ifdef _DEBUG
        MessageBoxA(NULL, logbuf, "Assert", MB_OK);
#endif
#endif
    }

#ifndef TRACE_STATE
    return 0;
#endif
    //if(!GetDebugContext().GetEnable())
    //	return 0;
    int nRet = 0;

    if (nFilter != LOG_MSG_MEM_LEAK
        && nFilter != LOG_MSG_POPUP)
    {
        va_list	argptr;
        va_start(argptr, strFormat);

        vsprintf_s(logbuf, 1024, strFormat, argptr);
        va_end(argptr);
    }

#ifdef _WIN32
	if (nFilter == LOG_MSG_CONSOLE)
	{
		// printf(logbuf);
		return 0;
	}
#endif

	if (s_pfilter && s_pfilter->is_hide_all())
		return 0;

	if (pquMsg == NULL) {
		pquMsg = new BaseCircleQueue("BaseMsgQueue", 112400);
	}

	if (!mpool_get().thread_main_check()) {
		size_t nLen = strlen(logbuf) + 2;
		char *pMsg = PT_Alloc(char, nLen);
		strcpy_s(pMsg, nLen, logbuf);
		pquMsg->push(pMsg);
		return 0;
	}

	if(nFilter == LOG_MSG_FILELOG
		//&& (GetDebugContext().GetState() >= 50)
       )
	{	
        FILE *pf = NULL;
        
		//char strFullPath[255], 
		char strDate[255];
		time_t t = time(0);   // get time now
		
		if(strlen(log_path)>1)
		{
			struct tm * now = localtime( & t );
			sprintf_s(strDate, 255, log_path,now->tm_year,now->tm_mon+1,now->tm_mday);
			if(!fopen_s(&pf, strDate, "a+"))
			{
				fprintf(pf, "%02d:%02d:%02d::%s", now->tm_hour, now->tm_min, now->tm_sec, logbuf);
				fclose(pf);
				nRet = 1;
			}
		}
	}

	char *strThreadMsg = (char*)pquMsg->pop();
	while (strThreadMsg) {
		nRet = _sendmsg(LOG_MSG, strThreadMsg);
		PT_Free(strThreadMsg);
		strThreadMsg = (char*)pquMsg->pop();
	}

	if(nFilter != LOG_MSG_NOFILTER)
		nRet = _sendmsg(nFilter, logbuf);
	//return 0;
#ifdef TRACE_STATE
	//printf("\n");
	//puts(logbuf);
#endif
	return nRet;
}

BaseConsolFilter::BaseConsolFilter()
{
	m_bHideAll = false;
}

BaseConsolFilter::~BaseConsolFilter()
{
}

void BaseConsolFilter::reset()
{
	if(s_pfilter != NULL){
        //s_pfilter->load_filter("vLogFilter.ini");
        return;
    }   
    
    s_pfilter = new BaseConsolFilter;
}

bool BaseConsolFilter::load_filter(const char *_strFile)
{
    BaseFile	file;
    file.set_asc_deletor(" \t");
    file.set_asc_seperator(" \t");
    
    m_stlMnFilterHide.clear();
    m_stlMnFilterShow.clear();
    
    if (!file.OpenFile(_strFile, BaseFile::OPEN_READ))
    {
        char strBuff[1024], strBuff2[1024], strBuff3[1024];
        
        file.read_asc_line();// ip
        file.read_asc_line();// port
        
        while (file.read_asc_line())
        {
            if (file.read_asc_string(strBuff, 1024))// show or hide
            {
                if (strBuff[0] == ';')
                    continue;
                
                int nHash, nValue;
                if (file.read_asc_string(strBuff2, 1024)) // token (SMain, SAction ~
                {
                    nHash = STRTOHASH(strBuff2);
                    if (file.read_asc_string(strBuff3, 1024)) // Value
                        nValue = STRTOHASH(strBuff3);
                    else
                        nValue = 0;
                    
                    if (file.read_asc_string(strBuff3, 1024)) // Value
                    {
                        nValue = STRTOHASH(strBuff3);
                        sprintf_s(strBuff3, 1024, "%d", nValue);
                        nValue = STRTOHASH(strBuff3);
                    }
                    
                    if (strcmp(strBuff, "show") == 0)
                    {
                        m_stlMnFilterShow[nHash].push_back(nValue);
                    }
                    else {
                        m_stlMnFilterHide[nHash].push_back(nValue);
                    }
                }
            }
        }
        
        file.CloseFile();
        return true;
    }
    return false;
}

void BaseConsolFilter::hide(const char *_strTitle, const char *_strValue)
{
    if(s_pfilter == NULL){
        reset();
    }
	
    s_pfilter->hide_(_strTitle, _strValue);
}

void BaseConsolFilter::show(const char *_strTitle, const char *_strValue)
{
    if(s_pfilter == NULL){
        reset();
    }
    
    s_pfilter->show_(_strTitle, _strValue);
}

void BaseConsolFilter::hide_(const char *_strTitle, const char *_strValue)
{
	if (strncmp(_strTitle, "all", 4) == 0) {
		m_bHideAll = true;
		return;
	}
    int nHash = STRTOHASH(_strTitle);
    int nValue;
    if(_strValue)
        nValue = STRTOHASH(_strValue);
    else
        nValue = 0;
    
    m_stlMnFilterHide[nHash].push_back(nValue);
}

void BaseConsolFilter::show_(const char *_strTitle, const char *_strValue)
{
    int nHash = STRTOHASH(_strTitle);
    int nValue;
    if(_strValue)
        nValue = STRTOHASH(_strValue);
    else
        nValue = 0;
    
    m_stlMnFilterShow[nHash].push_back(nValue);
}

void BaseConsolFilter::load_hash(const char*_strFile)
{
	if (s_pfilter == NULL)
		reset();

    if(s_pfilter)
        s_pfilter->load_hash_(_strFile);
}

bool BaseConsolFilter::load_hash_(const char *_strFile)
{
    if(s_pfilter == NULL){
        reset();
    }
    
    BaseFile	file;
    file.set_asc_seperator(",\t");
    
    m_stlMnStringHash.clear();
    m_stlVString.clear();
    
    if (!file.OpenFile(_strFile, BaseFile::OPEN_READ))
    {
        char strBuff[1024];
        while (file.read_asc_line())
        {
            if (file.read_asc_string(strBuff, 1024))
            {
                int nHash = STRTOHASH(strBuff);
                m_stlMnStringHash[nHash] = (int)m_stlVString.size();
                m_stlVString.push_back(strBuff);
                
                if (nHash == -327046163 || strcmp("BaseKeyPush_nRE", strBuff) == 0
                    || strcmp("Connect", strBuff) == 0)
                {
                    int x = 0;
                    x++;
                }
            }
        }
        
        file.CloseFile();
    }
    //savehash();
    return true;
}

bool BaseConsolFilter::save_hash(const char *_strFile)
{
    if(s_pfilter == NULL){
        reset();
    }
    
    BaseFile	file;
    
    if (!file.OpenFile(_strFile, BaseFile::OPEN_WRITE))
    {
        for (unsigned i = 0; i<m_stlVString.size(); i++)
        {
            file.write_asc_string(m_stlVString[i].c_str(), (int)m_stlVString[i].size());
            file.write_asc_integer(STRTOHASH(m_stlVString[i].c_str()));
            file.write_asc_line();
        }
        
        file.CloseFile();
    }
    return true;
}

const char *BaseConsolFilter::get_output(const char *_strOut, int _nLen)
{
    if(s_pfilter == NULL){
        reset();
    }
    
    static char s_strOutput[4096];
    BaseFile	parser;
    
    char strBuffer[2048];
    char strToken[2048];
    
	if (_nLen >= 1023)
		_nLen = 1023;

    STLVString m_value_a, m_token_a;
    strcpy_s(strBuffer, 1024, _strOut);
    parser.set_asc_deletor(" \t:");
    parser.set_asc_seperator("\t :");
    parser.OpenFile((void*)strBuffer, _nLen);
    
    parser.read_asc_line();
    bool	bParse = false;
    int		nCnt = 0;
    
    m_token_a.clear();
    m_value_a.clear();
    do {
        bParse = parser.read_asc_string(strToken, 1024);
        if (!bParse)
            break;
        m_token_a.push_back(strToken);
        m_value_a.push_back("");
        if (parser.get_seperator_last() == ':')
        {
            bParse = parser.read_asc_string(strToken, 1024);
            if (!bParse)
                break;
            m_value_a[nCnt] = strToken;
        }
        nCnt++;
    } while (1);
    
    if (m_token_a.size() >= 2 && strcmp(m_token_a[1].c_str(), "registhash") == 0)
    {
        int nHash = STRTOHASH(m_value_a[1].c_str());
        
        STLMnInt::iterator	it;
        it = m_stlMnStringHash.find(nHash);
        if (it == m_stlMnStringHash.end())
        {
            m_stlMnStringHash[nHash] = (int)m_stlVString.size();
            m_stlVString.push_back(m_value_a[1].c_str());
        }
    }
    else {
        bool bHide = false;
        STLMnInt::iterator	it;
        
        int nHash, nValue;
        
        char strBuf[255] = "";
		strBuffer[0] = 0;
        
        for (unsigned i = 0; i< m_token_a.size(); i++)
        {
            nHash = STRTOHASH(m_token_a[i].c_str());
            nValue = STRTOHASH(m_value_a[i].c_str());
            
            STLMnstlVValues::iterator	itFilter;
            itFilter = m_stlMnFilterHide.find(nHash);
            if (itFilter != m_stlMnFilterHide.end())
            {
                for (unsigned j = 0; j<itFilter->second.size(); j++)
                {
                    if (itFilter->second[j] == 0 || itFilter->second[j] == nValue)
                    {
                        bHide = true;
                        break;
                    }
                }
            }
            itFilter = m_stlMnFilterShow.find(nHash);
            if (itFilter != m_stlMnFilterShow.end())
            {
                for (unsigned j = 0; j<itFilter->second.size(); j++)
                {
                    if (itFilter->second[j] == 0 || itFilter->second[j] == nValue)
                    {
                        bHide = false;
                        break;
                    }
                }
            }
             
			strcpy_s(strBuf, 255, m_value_a[i].c_str());
            nValue = atoi(strBuf);
            it = m_stlMnStringHash.find(nValue);
            if (it != m_stlMnStringHash.end() && it->first != 5381)
				strcpy_s(strBuf, 255, m_stlVString[it->second].c_str());

			strcat_s(strBuffer, 1024, " ");
			strcat_s(strBuffer, 1024, m_token_a[i].c_str());
			strcat_s(strBuffer, 1024, ":");
			strcat_s(strBuffer, 1024, strBuf);
        }

        if (!bHide)
        {
            SPtTime	time;
            BaseSystem::timeCurrent(&time);
            sprintf_s(strBuf, 255, "%02d:%02d:%02d ", time.s.nHour, time.s.nMinute, time.s.nSecond);
            
            int nLen = (int)strlen(strBuffer);
            nLen--;
            while (nLen > 0 && (strBuffer[nLen] == '\n' || strBuffer[nLen] == '\r'))
            {strBuffer[nLen] = NULL; nLen--;}
            
            strcpy_s(s_strOutput, 1024, strBuf);
            strcat_s(s_strOutput, 1024, strBuffer);
            return s_strOutput;
        }
        return NULL;
    }

    return NULL;
}

#ifndef _WIN32

void OutputDebugString(const char *_out)
{
	//puts(_out);
	//printf(_out);
}

#endif
