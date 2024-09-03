#pragma once

#include <atomic>

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
	void MakeDoubleInLock();
	void *m_criticalsection;
	std::atomic<int>	*m_pnUse;
	std::atomic<int>	*m_pnDouble;
	
	std::atomic<int>	*m_pnCountPushed;
	std::atomic<int>	*m_pnCountPushed2;
	std::atomic<int>	*m_puPosPush;
	std::atomic<int>	*m_puPosPop;
	
	
	UINT32				m_nSize;
	void				**m_parrayQueue;

	static BaseCircleQueue* ms_queue;
public:
	static BaseCircleQueue *stream_get();
	static BaseCircleQueue* streamSize_get();
private:

	char	m_strCalledPos[256];
};


class BraceInc
{
protected:
	std::atomic<int>* m_cnt;
	std::atomic<int>* m_double;
public:
	BraceInc(std::atomic<int>* _cnt, std::atomic<int>* _double);
	void hold();
	~BraceInc();
};

class BraceUpdate : public BraceInc
{
	void *m_criticalsection;
public:
	static void criticalInit(void *_criticalsection);
	static void criticalDestory(void *_criticalsection);
	BraceUpdate(std::atomic<int>* _cnt, std::atomic<int>* _double, void *_criticalsection);
	~BraceUpdate();
};
