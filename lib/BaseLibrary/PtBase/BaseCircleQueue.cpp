#include "stdafx.h"
//#include "BaseCircleQueue.h"
#include "BaseSystem.h"

#ifdef WIN32
#else

#ifdef ANDROID
#else
	//#include <libkern/OSAtomic.h>
#endif
	#include <unistd.h>
	#include <sys/time.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdlib.h>
#endif

#ifndef _WIN32
#define CRITICAL_SECTION				pthread_mutex_t
#define InitializeCriticalSection(a)	pthread_mutex_init(a, NULL)
#define DeleteCriticalSection		pthread_mutex_destroy
#define EnterCriticalSection		pthread_mutex_lock
#define LeaveCriticalSection		pthread_mutex_unlock
#define TlsGetValue					pthread_getspecific
#define TlsSetValue					pthread_setspecific
#define TlsFree						pthread_key_delete
#else

#endif

#define MAX_SOCKET	10*8
#define MAX_QUEUE 131072

void *s_buffer_a[MAX_SOCKET] = {0};
int s_buffer_use_a[MAX_SOCKET] = {0};

atomic_cnt *ps_buffer_top = NULL;

class BraceInc
{
	atomic_cnt *m_cnt;
	atomic_cnt *m_double;
public:
	BraceInc(atomic_cnt *_cnt, atomic_cnt *_double){
		m_cnt = _cnt;
		m_double = _double;
		
		while(m_double->get() > 0)
			BaseCircleQueue::qsleep(1);
		++(*m_cnt);
	}
	void hold(){
		while(m_cnt->get() > 0)
			BaseCircleQueue::qsleep(1);
	}
	~BraceInc(){
		--(*m_cnt);
	}
};

void *buff_alloc(int _size)
{
	if(ps_buffer_top == NULL)
		ps_buffer_top = new atomic_cnt(-1);
	
	if(MAX_QUEUE != _size)
		return malloc(_size);
	
	if(!s_buffer_a[0])
	{
		for(int i=0; i<MAX_SOCKET; i++)
		{
			s_buffer_a[i] = (void*)malloc(MAX_QUEUE);
			s_buffer_use_a[i] = 0;
		}
	}
	
	int top = 0, loop = 0;
	do{
		top = ++*ps_buffer_top;
		
		top %= MAX_SOCKET;
		loop++;
		if(loop >= MAX_SOCKET)
		{
			//B_ASSERT(loop < MAX_SOCKET);
			return malloc(_size);
		}
	}while(s_buffer_use_a[top] == 1);
	s_buffer_use_a[top] = 1;
	
	return s_buffer_a[top];
}

void buff_free(void *_buf)
{
	for(int i=0; i<MAX_SOCKET; i++)
	{
		if(s_buffer_a[i] == _buf)
		{
			s_buffer_use_a[i] = 0;
			return;
		}
	}
	
	//B_ASSERT(0);
	free(_buf);
}


void BaseCircleQueue::qsleep(int _mili)
{
#ifdef _WIN32
	Sleep(_mili);
#else
	usleep(_mili * 1000);
#endif
}

BaseCircleQueue::BaseCircleQueue(const char *_strCalledPos, int _size)
{
	if(_strCalledPos)
		strcpy_s(m_strCalledPos, 256, _strCalledPos);
	
	m_parrayQueue = NULL;

	m_puPosPop = NULL;
	m_puPosPush = NULL;
	m_pnCountPushed = NULL;
	m_pnCountPushed2 = NULL;
	m_criticalsection = NULL;
	
	m_pnUse = NULL;
	m_pnDouble = NULL;

	m_nSize	= 0;
	init(_size);
}

BaseCircleQueue::BaseCircleQueue(BaseCircleQueue &_other)
{
	m_puPosPop = NULL;
	m_puPosPush = NULL;
	m_pnCountPushed = NULL;
	m_pnCountPushed2 = NULL;
	
	m_pnUse = NULL;
	m_pnDouble = NULL;

	m_parrayQueue = NULL;
	m_criticalsection = NULL;
	m_nSize	= 0;
	init(_other.m_nSize);
}

void BaseCircleQueue::init(UINT32 _nSize)
{
	static int nCount = 0;
	nCount++;
	release();
	
	CRITICAL_SECTION *sec = new CRITICAL_SECTION;
	InitializeCriticalSection(sec);
	m_criticalsection = (void*)sec;
	
	if(_nSize == 0)
		_nSize = 10240;
	
	UINT32 nSizeQueue	= 128;

	while(nSizeQueue < _nSize){
		nSizeQueue	*= 2;
	}

	m_parrayQueue = (void**)buff_alloc(nSizeQueue * sizeof(void*));
	m_nSize	= nSizeQueue;
	
	if(m_puPosPush == NULL)
	{
		m_puPosPop = new atomic_cnt(-1);
		m_puPosPush = new atomic_cnt(0);
		m_pnCountPushed = new atomic_cnt(0);
		m_pnCountPushed2 = new atomic_cnt(0);
		m_pnUse = new atomic_cnt(0);
		m_pnDouble = new atomic_cnt(0);
	}
	
	memset(m_parrayQueue, 0, nSizeQueue*sizeof(void*));
}

void BaseCircleQueue::MakeDoubleInLock()
{
	if(m_pnCountPushed2->get() < m_nSize)
		return;
	
	int nSizeQueue = m_nSize * 2;
	void **backup = (void**)buff_alloc(nSizeQueue * sizeof(void*));
	
	memset(backup, 0, nSizeQueue*sizeof(void*));
	memcpy(backup, m_parrayQueue, m_nSize);
	void **torelease = m_parrayQueue;
	m_parrayQueue = backup;
	m_nSize = nSizeQueue;
	if(torelease)
		buff_free(torelease);
}

BaseCircleQueue::~BaseCircleQueue(void)
{
	release();
}

void BaseCircleQueue::release()
{
	if(m_parrayQueue)
		buff_free(m_parrayQueue);
	m_parrayQueue = NULL;
	
	if(m_puPosPush)
	{
		delete m_puPosPop;;
		delete m_puPosPush;
		delete m_pnCountPushed;
		delete m_pnCountPushed2;
		
		delete m_pnUse;
		delete m_pnDouble;
	}
	m_puPosPush = NULL;
	
	if(m_criticalsection != NULL)
	{
		CRITICAL_SECTION *sec = (CRITICAL_SECTION*)m_criticalsection;
		DeleteCriticalSection(sec);
		delete sec;
		m_criticalsection = NULL;
	}
}

void *BaseCircleQueue::top()
{
	BraceInc inc(m_pnUse, m_pnDouble);
	
	int cnt = m_pnCountPushed->get();
	if(cnt <= 0)
		return NULL;

	UINT32 nPos = (UINT32)m_puPosPop->get();
	nPos++;
	nPos	= nPos % m_nSize;

 	void *pRet	= NULL;
	pRet	= *(m_parrayQueue+nPos);
	if(pRet == 0)
	{
		do{
 			pRet	= *(m_parrayQueue+nPos);
			qsleep(1);
			g_SendMessage(LOG_MSG, "top");
		}while(pRet == NULL);
	}
	return pRet;
}

void *BaseCircleQueue::pop()
{
	BraceInc inc(m_pnUse, m_pnDouble);
	
	if(m_pnCountPushed->get() <= 0)
		return NULL;

	//if(BaseSystem::LFDecrement(&m_nCountPushed) < 0)
	if(--*m_pnCountPushed < 0)
	{// Thread 내부 에서는 m_nCountPush-- 등의 연산 결과 또한 보증 받지 못한다. 따라서 
		// Unit 연산을 사용한다.
		++*m_pnCountPushed;
        return NULL;
	}

	UINT32 nPos = (UINT32)++*m_puPosPop;
    
	nPos	= nPos % m_nSize;
    void *pRet	= NULL;
	pRet	= *(m_parrayQueue+nPos);
	if(pRet == 0)
	{
		do{
 			pRet	= *(m_parrayQueue+nPos);
			// push Thread중 나중에 시작한 Thread가 값을 넣고 m_nCountPush를 증가시켜 
			// 먼저 들어간 Thread중 아직 값을 체우지 못한 Thread가 있을 수 있기 때문에
			// 값을 넣어 줄 때 까지 기다린다.
			qsleep(1);

			g_SendMessage(LOG_MSG, "pop");
		}while(pRet == NULL);
	}
	*(m_parrayQueue+nPos)	= NULL;
	
	--*m_pnCountPushed2;
	return pRet;
}

bool BaseCircleQueue::push(void *_pValue)
{
	if(_pValue == NULL)
		return false;

	BraceInc inc(m_pnUse, m_pnDouble);
	
	//assert(m_nCountPushed < (INT32)m_nSize-1 && "CircleQueue Overflow");
	if(++*m_pnCountPushed2 >= (int)m_nSize)
	{
		--*m_pnUse;
		EnterCriticalSection((CRITICAL_SECTION*)m_criticalsection);
		++*m_pnDouble;
		inc.hold();
		//g_SendMessage(LOG_MSG_FILELOG, "--------------------CircleQueue OverFlow Size: %d %s\n", m_nSize, m_strCalledPos);
		//g_SendMessage(LOG_MSG, "--------------------CircleQueue OverFlow Size: %d\n", m_nSize);
		MakeDoubleInLock();
		//--*m_pnCountPushed2;
		--*m_pnDouble;
		LeaveCriticalSection((CRITICAL_SECTION*)m_criticalsection);
		++*m_pnUse;
		//return false;
	}

	UINT32 nPos = (UINT32)++*m_puPosPush;
	nPos--;
	nPos	= nPos % m_nSize;
	*(m_parrayQueue+nPos)	= _pValue;
	
	++*m_pnCountPushed;
	return true;
}

INT32 BaseCircleQueue::size_data()
{
	BraceInc inc(m_pnUse, m_pnDouble);
	
	if (!m_pnCountPushed)
		return 0;
	return (INT32)m_pnCountPushed->get();
}

BaseCircleQueue* BaseCircleQueue::ms_queue = NULL;
BaseCircleQueue* BaseCircleQueue::stream_get()
{
	if (ms_queue == NULL)
		ms_queue = new BaseCircleQueue(" stream ", 100);
	return ms_queue;
}

BaseCircleQueue* ms_queueSize = NULL;
BaseCircleQueue* BaseCircleQueue::streamSize_get()
{

	if (ms_queueSize == NULL)
		ms_queueSize = new BaseCircleQueue(" stream ", 100);
	return ms_queueSize;
}
