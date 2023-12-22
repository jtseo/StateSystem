#pragma once

#define __MEMORYPOOL_SIZE_BLOCK	64
#define __MEMORYPOOL_SIZE_LAYER	64

//#define _DEBUG
//#define DEF_MOBILE

class BaseMemoryPoolBlock
{
public:
	static char	get_size_shift_(INT32 _nSize);
	void create();
	INT32	get_max_alloc()
	{
		return m_nCountMaxAllocItem;
	}
	INT32 get_size_item(){
		return m_nSizeOfItem;
	}
	void display_info();
protected:
	void init(INT32 _nSizeOfItem, INT32 _nInitCount, const char *_strFileName, int _nLine);
	void release();
	bool is_init();
	void *get_new_item(bool *_pbUpdate, const char* _filename, int _line);
	void delete_item(void *_pItem);

	void	increase_free_mem(const char* _filename, int _line);
	INT32		m_nSizeInit;
	INT32		m_nSizeOfItem;
	INT32		m_nCountOfItem;
	INT32		m_nCountAllocatedItem;
	INT32		m_nCountMaxAllocItem;
	char	m_nPointNull;
	char	m_nSizeShift;
	char	*m_pPointAllocatable; // place of allocatable, it used for last point for free place.
	char	*m_paPullMemory[__MEMORYPOOL_SIZE_BLOCK];

	friend	class BaseMemoryPoolSingle;

#ifdef _DEBUG
private:
	INT32		m_nDebugCountOfRecord;
	char	m_pDebugRecordAlloc[1000000];
#endif
};

class BaseMemoryPoolSingle
{
public:
	BaseMemoryPoolSingle();
	~BaseMemoryPoolSingle(void);

	void *malloc(size_t _nSize, const char *_strFileName=NULL, INT32 _nLine=0);
	void free(void *_point);
	bool free_add(void *_point);
#ifdef _DEBUG
	void leak_check(const void* xpPoint);
#endif

	void free_all();
	void init_all();

	void index_set(unsigned short _nIndex);
	unsigned short index_get()
	{
		return m_nThreadIndex;
	}
	void display_info();
protected:
	unsigned short m_nThreadIndex;
	void	free_(void *_point);
	INT32		get_size_(char _nSizeShift);
	char	*get_filename(char *_strFilename, INT32 _nLen=255);
	
	bool	m_bCalledClear;
	bool	m_bFirstMemoryPull;
	INT32		m_nAllocatedMemory;
	BaseCircleQueue	m_queueFree;

	bool m_bThreadEnd;
public:
	void thread_end()
	{
		m_bThreadEnd = true;
	}

	void thread_realloc()
	{
		m_bThreadEnd = false;
	}

	bool thread_terminated_is()
	{
		if (m_nAllocatedMemory == 0 && m_bThreadEnd)
			return true;
		return false;
	}
//================================================================
public:
	INT32	get_reserved_type();
	void set_reserved_type(INT32 _nType);
protected:
	INT32		m_nReservedType;// Ư�� SinglePool Type�� �Ҵ� �޸𸮸� ����� �Ѵ�.
	bool	m_bUpdate;
	//(Thread�� ������ ���� �ֱⰡ �ʹ� ������ �뷮���� ������ ���ɼ��� ���� ����
//==================================================================

	BaseMemoryPoolBlock m_sMemoryLayer[__MEMORYPOOL_SIZE_LAYER];
#ifdef _DEBUG
public:
#endif
	INT32		m_nAccessCount;
	
#ifdef _DEBUG
private:
	void *leak_check_in(INT32 xiLine, const char *xpFilename, void *xpPoint);
	bool leak_check_out(void *xpPoint);
	void leak_display();
	void leak_info_clear();
public:
	void leak_old_display(int _time);
public:
	void observe_push(void* _point);
	bool observe_check(void* _point);
	void observe_out(void* _point);
	
private:
	typedef std::map<void*, void*> DSTLMpVoid;
	DSTLMpVoid m_observe_m;

	char	**s_stlVpAllocatedFile;
	INT32		*s_stlVpAllocatedLine;
	INT32		*s_stlVpAllocatedSerial;
	void	**s_stlVpAllocatedPoint;
	INT32		s_iAllocatedReserve;
	INT32		s_iAllocatedLength;
	INT32		s_nSerialNumber;

	typedef struct __delaycheck {
		int line;
		char* file;
		void* point;
		int time;
	} STDelayCheck;
	std::vector<STDelayCheck>	s_delayCheckoutPoint;
#endif
};


class BaseMemoryPoolMultiThread
{
public:
	BaseMemoryPoolMultiThread();
	~BaseMemoryPoolMultiThread(void);

	// Application�� ����� static �޸𸮵��� �޸� ������ ����Ѵ�.
	void lock();
	// Application�� ����� static �޸𸮵��� �޸� ������ ����Ѵ�.
	void unlock();

	void *malloc(size_t _nSize, const char *_strFileName=NULL, INT32 _nLine=0);
	void free(void *_point);
	bool checkOverwrite(void* _point);
	void observe_push(void* _point);

	void end(INT32 _nReservedType);
	void start(INT32 _nReservedType);
	
	void leak_old_display(int _time);
	void display_info();
	void leak_check(const void* _point);
	static void free_all();
	static void terminate(bool _bTerminated = true);
	static bool is_terminated();

	static void hold_shutdown_inc();
	static void hold_shutdown_dec();
	bool	thread_main_check();
	void	thread_main_set();

	static INT32 memory_total_size();
	static INT32	sm_memory_total_size;
protected:
	BaseMemoryPoolSingle	*thread_create_single(INT32 _nReservedType = 0);
	BaseMemoryPoolSingle	*get_single(INT32 _nReservedType = 0);
	void init();
	static INT32	sm_nHoldShutdownCount;
	
	// =================================
	// support auto free by jtseo 2021/11/25
public:
	void auto_free(void *_point);
	void auto_ofree(void *_point);
	void auto_update();
protected:
	// ------------------------ end of auto free
};
