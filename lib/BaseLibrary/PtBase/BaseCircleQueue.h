#pragma once

#ifndef _WIN32
#include <stdatomic.h>
#endif

class atomic_cnt;

// Performance queue for multithread
// but in case of queue overflow, it make bring a critical crush.
class BaseCircleQueue
{
public:
	BaseCircleQueue(const char *_strCalledPos, int _size = 0);
	BaseCircleQueue(BaseCircleQueue &_other);
	~BaseCircleQueue(void);

	void init(UINT32 _nSize);
	void release();
	void *pop();
	void *top();// If pop thread is only one, can use this function.
	bool push(void *_pValue);
	INT32 size(){
		return m_nSize;
	}
	INT32 size_data();
	static void qsleep(int _mili);
protected:
#ifdef DEF_QLOCK
	CRITICAL_SECTION	m_lock;
#endif
	
	atomic_cnt	*m_pnCountPushed;
	atomic_cnt	*m_pnCountPushed2;
	atomic_cnt	*m_puPosPush;
	atomic_cnt	*m_puPosPop;
	
	UINT32				m_nSize;
	void				**m_parrayQueue;
	
private:

	char	m_strCalledPos[256];
};
