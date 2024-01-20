#include "stdafx.h"

#ifndef _WIN32
#include <pthread.h>

#include <sys/time.h>
#else
//#include <windows.h>

#include <MMSystem.h>
#endif

#include <assert.h>
#include <math.h>
#include <vector>

#if defined(ANDROID) || defined(_IOS)
#define DEF_MOBILE
#endif

#define MAX_THREAD	10240	//지원 하는 최대 스레드 개 수
#define SIZE_THREADID	sizeof(unsigned short)
#define SIZE_HEADER		18//6//2
#define SIZE_TAIL		1
#define SIZE_RESERV		(SIZE_HEADER+SIZE_TAIL+SIZE_THREADID)
#define SIZE_MINIMAM	8//4 update for 64 bit address system
#define	COUNT_INIT		16


#ifndef _WIN32
UINT32 timeUint32()
{
	timeval tv;
	gettimeofday(&tv, 0);
	
	return (UINT32)( ( tv.tv_sec * 1000 ) + ( tv.tv_usec / 1000 ) );
}
#endif

#include "BaseSystem.h"

BaseMemoryPoolSingle::BaseMemoryPoolSingle(void):
m_queueFree("BaseMemoryPoolSingle:Free", 102400)
{
	m_bFirstMemoryPull	= true;
	m_nAllocatedMemory	= 0;
	m_bUpdate	= true;

	m_bThreadEnd = false;

	m_bCalledClear = false;
#ifdef _DEBUG
	s_iAllocatedReserve	= 0;
	s_iAllocatedLength	= 0;
	s_nSerialNumber	= 1;
#endif
}

BaseMemoryPoolSingle::~BaseMemoryPoolSingle(void)
{
	if(m_bUpdate)
	{
		char strBuf[255], strFilename[255];

		FILE	*pfile;
		get_filename(strFilename);

		pfile	= fopen(BaseSystem::module_get_data(strBuf, 255, strFilename),"wb");

		if(pfile)
		{
			INT32 nSizeOfItem, nInitCount;
			for(INT32 i=0; i<__MEMORYPOOL_SIZE_LAYER; i++)
			{
				if(m_sMemoryLayer[i].is_init())
				{
					nInitCount	= m_sMemoryLayer[i].get_max_alloc();
					nSizeOfItem	= m_sMemoryLayer[i].get_size_item();
					nSizeOfItem -= SIZE_RESERV;
				}else{
					nInitCount	= 0;
					nSizeOfItem	= 0;
				}

				fwrite(&nSizeOfItem, sizeof(INT32), 1, pfile);
				fwrite(&nInitCount, sizeof(INT32), 1, pfile);
			}
			fclose(pfile);
		}
	}
	free_all();
}

INT32	BaseMemoryPoolSingle::get_size_(char _nSizeShift)
{
	INT32 nSquare;
	nSquare	= (INT32)pow((float)2, (float)_nSizeShift);
	return SIZE_MINIMAM*nSquare;
}

char BaseMemoryPoolBlock::get_size_shift_(INT32 _nSize)
{
	char nRet		= 0;

	INT32	nSizeTemp	= SIZE_MINIMAM;
	while(nSizeTemp < _nSize)
    {nSizeTemp <<= 1; nRet++;}

	return nRet;
}

void BaseMemoryPoolBlock::create()
{
	m_nSizeOfItem			= 0;
	m_nSizeShift			= 0;
	m_nCountOfItem			= 0;
	m_nCountAllocatedItem	= 0;
	m_pPointAllocatable		= 0; // place of allocatable, it used for last point for free place.
	BaseSystem::memset(m_paPullMemory, 0, __MEMORYPOOL_SIZE_BLOCK*sizeof(void*));
	m_nPointNull			= 0;
}

void BaseMemoryPoolBlock::init(INT32 _nSizeOfItem, INT32 _nInitCount, const char *_strFileName, int _nLine)
{	
	m_nSizeOfItem			= 0;
	m_nSizeShift			= 0;
	m_nCountOfItem			= 0;
	m_nCountAllocatedItem	= 0;
	m_pPointAllocatable		= 0; // place of allocatable, it used for last point for free place.
	BaseSystem::memset(m_paPullMemory, 0, __MEMORYPOOL_SIZE_BLOCK*sizeof(void*));
	m_nPointNull			= 0;

	m_nSizeOfItem	= _nSizeOfItem+SIZE_RESERV;
	m_nSizeShift	= get_size_shift_(_nSizeOfItem);	
	m_nSizeInit		= _nInitCount;
	m_nCountMaxAllocItem	= _nInitCount;
	m_nCountOfItem	= 0; // it's going to increase in increase_free_mem()

	static INT32 s_nCount = 0;
	s_nCount++;

	//if(m_nSizeOfItem > 30000000)
	//	printf("stoooooooooooooooooooooooooooop %d, %d", s_nCount, m_nSizeOfItem);

	increase_free_mem(_strFileName, _nLine);
}

void BaseMemoryPoolSingle::free_all()
{
	void *pMem = NULL;
	while ((pMem = m_queueFree.pop()) != NULL)
	{
		free_(pMem);
	}
#ifdef _DEBUG
	leak_display();
#endif
	for(INT32 i=0; i<__MEMORYPOOL_SIZE_LAYER; i++)
	{	
		m_sMemoryLayer[i].release();
	}
#ifndef _DEBUG
	//USER_CON_MSG(1, "All of memory cleared !! good job!!");
#endif

	m_bFirstMemoryPull	= true;
	m_nAllocatedMemory	= 0;
}

void BaseMemoryPoolSingle::init_all()
{	
	m_bFirstMemoryPull	= false;

	for(INT32 i=0; i<__MEMORYPOOL_SIZE_LAYER; i++)
	{	
		m_sMemoryLayer[i].create();
	}
}

void BaseMemoryPoolBlock::release()
{
	for(INT32 i=0; i<m_nPointNull; i++)
	{
		BaseSystem::free(m_paPullMemory[i]);
		m_paPullMemory[i]	= 0;
	}
}

void *BaseMemoryPoolBlock::get_new_item(bool *_pbUpdate, const char *_filename, int _line)
{
	if (m_nCountAllocatedItem >= m_nCountOfItem)
	{
		if (!increase_free_mem(_filename, _line))
			return NULL;
	}

	char *pNextPoint	= *((char**)m_pPointAllocatable);
	if(pNextPoint == 0 && m_nCountAllocatedItem < m_nCountOfItem)
	{
		pNextPoint	= m_pPointAllocatable+m_nSizeOfItem;
		if(m_nCountAllocatedItem < m_nCountOfItem-1)
			*((void**)pNextPoint) = 0;
		else
			pNextPoint	= 0;
	}
	char *pAlloc;
	pAlloc		= m_pPointAllocatable;
	*(pAlloc+SIZE_THREADID)		= m_nSizeShift;
	*(pAlloc+(1+SIZE_THREADID))	= m_nPointNull;	// error
	pAlloc		+= (SIZE_HEADER+SIZE_THREADID);

	m_nCountAllocatedItem++;
	if(m_nCountAllocatedItem > m_nCountMaxAllocItem)
	{
		m_nCountMaxAllocItem	= m_nCountAllocatedItem;
		if(_pbUpdate)
			*_pbUpdate	= true;
	}
	m_pPointAllocatable	= pNextPoint;

	return (void*)pAlloc;
}

void BaseMemoryPoolBlock::delete_item(void *_pItem)
{
	char *pPoint	= (char*)_pItem;
	char *pNext		= m_pPointAllocatable;
	m_pPointAllocatable	= pPoint-(SIZE_HEADER+SIZE_THREADID);
	*((void**)m_pPointAllocatable)	= pNext;
	m_nCountAllocatedItem--;
}

bool BaseMemoryPoolBlock::is_init()
{
	if(m_nSizeOfItem == 0)
		return false;
	return true;
}

bool BaseMemoryPoolBlock::increase_free_mem(const char* _filename, int _line)
{
#ifdef _DEBUG
	B_ASSERT(m_nPointNull < __MEMORYPOOL_SIZE_BLOCK);
#endif

	if(m_nPointNull > __MEMORYPOOL_SIZE_BLOCK)
	{
		g_SendMessage(LOG_MSG_POPUP, "Memory overflow blocksize %d, allocated cnt %d\n", m_nSizeOfItem, m_nCountAllocatedItem);
	}
	INT32 nSizeAllocate;

	if(m_nSizeOfItem > 500000 && m_nCountOfItem > 5)
		nSizeAllocate = 5;
	else
		nSizeAllocate	= m_nCountOfItem*2 - m_nCountOfItem;
	if(nSizeAllocate == 0)
	{
		nSizeAllocate	= m_nSizeInit;
	}
	if (m_nSizeOfItem > 902400)
	{
		int x = 0;
	}
	m_paPullMemory[m_nPointNull]	= (char*)BaseSystem::malloc(m_nSizeOfItem * nSizeAllocate);

	BaseMemoryPoolMultiThread::sm_memory_total_size += m_nSizeOfItem * nSizeAllocate;

	if(m_paPullMemory[m_nPointNull] == NULL)
	{
		char strBuf[255];
		FILE *pf;
		if(!fopen_s(&pf, BaseSystem::path_root_get(strBuf, 255, "memory_alloc_errorlog.txt"), "w"))
		{
			fprintf_s(pf, "nSizeAllocate:%d, m_nCountOfItem:%d, m_nSizeOfItem:%d\n", nSizeAllocate, m_nCountOfItem, m_nSizeOfItem);
			if(_filename)
				fprintf_s(pf, "from:%s, line:%d\n", _filename, _line);
			fclose(pf);
		}
		return false;
		//g_SendMessage(LOG_MSG_POPUP, "Memory fail to allocate %d, allocated cnt %d\n", m_nSizeOfItem, m_nCountAllocatedItem);
	}

	//g_SendMessage(LOG_MSG_POPUP, "Memory allocate block %d, allocated cnt %d\n", m_nSizeOfItem, m_nCountAllocatedItem);
#ifdef _DEBUG
	B_ASSERT(m_paPullMemory[m_nPointNull] != 0);
#endif
	m_nCountOfItem	+= nSizeAllocate;
	BaseSystem::memset(m_paPullMemory[m_nPointNull], 0, sizeof(void*));
	m_pPointAllocatable	= m_paPullMemory[m_nPointNull];

	m_nPointNull++;
	return true;
}

void BaseMemoryPoolBlock::display_info()
{
	INT32 nCountAllocated	= 0;
	for(unsigned i=0; i<__MEMORYPOOL_SIZE_BLOCK; i++)
	{
		if(m_paPullMemory[i] != NULL)
			nCountAllocated++;
	}
	if(m_nCountOfItem)
		g_SendMessage(LOG_MSG_MEM_LEAK, "BaseMemoryPoolBlock 64:%d block %d, allocated %d, reserved %d\n", nCountAllocated, m_nSizeOfItem, m_nCountAllocatedItem, m_nCountOfItem);
	//if(m_nCountOfItem)
	//	printf("BaseMemoryPullBlock 64:%d block %d, allocated %d, reserved %d\n", nCountAllocated, m_nSizeOfItem, m_nCountAllocatedItem, m_nCountOfItem);
}

#ifdef _DEBUG

void *BaseMemoryPoolSingle::leak_check_in(INT32 xiLine, const char *xpFilename, void *xpPoint)
{
//	return xpPoint;

	if(!xpFilename)
		return xpPoint;

	if(s_iAllocatedReserve == 0)
	{
		s_iAllocatedReserve		= 100;
		s_stlVpAllocatedFile	= (char**)BaseSystem::malloc(sizeof(char*)*s_iAllocatedReserve);
		s_stlVpAllocatedLine	= (INT32*)BaseSystem::malloc(sizeof(INT32)*s_iAllocatedReserve);
		s_stlVpAllocatedSerial	= (INT32*)BaseSystem::malloc(sizeof(INT32)*s_iAllocatedReserve);
		s_stlVpAllocatedPoint	= (void**)BaseSystem::malloc(sizeof(void*)*s_iAllocatedReserve);
	}

	if(s_iAllocatedReserve == s_iAllocatedLength)
	{
		s_iAllocatedReserve	*= 2;

		char	**stlVpAllocatedFile	= s_stlVpAllocatedFile;
		INT32		*stlVpAllocatedLine		= s_stlVpAllocatedLine;
		INT32		*stlVpAllocatedSerial	= s_stlVpAllocatedSerial;
		void	**stlVpAllocatedPoint	= s_stlVpAllocatedPoint;

		s_stlVpAllocatedFile	= (char**)BaseSystem::malloc(sizeof(char*)*s_iAllocatedReserve);
		s_stlVpAllocatedLine	= (INT32*)BaseSystem::malloc(sizeof(INT32)*s_iAllocatedReserve);
		s_stlVpAllocatedSerial	= (INT32*)BaseSystem::malloc(sizeof(INT32)*s_iAllocatedReserve);
		s_stlVpAllocatedPoint	= (void**)BaseSystem::malloc(sizeof(void*)*s_iAllocatedReserve);

		BaseSystem::memcpy(s_stlVpAllocatedFile, stlVpAllocatedFile, s_iAllocatedLength*sizeof(char*));
		BaseSystem::memcpy(s_stlVpAllocatedLine, stlVpAllocatedLine, s_iAllocatedLength*sizeof(INT32));
		BaseSystem::memcpy(s_stlVpAllocatedSerial, stlVpAllocatedSerial, s_iAllocatedLength*sizeof(INT32));
		BaseSystem::memcpy(s_stlVpAllocatedPoint, stlVpAllocatedPoint, s_iAllocatedLength*sizeof(void*));

		BaseSystem::free(stlVpAllocatedLine);
		BaseSystem::free(stlVpAllocatedSerial);
		BaseSystem::free(stlVpAllocatedFile);
		BaseSystem::free(stlVpAllocatedPoint);
	}

	INT32 nSerialNumber = s_nSerialNumber++;
	size_t iSize;
	iSize	= strlen(xpFilename)+1;
	char *pStr	= (char*)BaseSystem::malloc(iSize);
	strcpy_s(pStr, iSize, xpFilename);
	*(s_stlVpAllocatedFile+s_iAllocatedLength)	= pStr;
	*(s_stlVpAllocatedLine+s_iAllocatedLength)	= xiLine;
	*(s_stlVpAllocatedSerial + s_iAllocatedLength) =
#ifndef _WIN32
		timeUint32();//nSerialNumber;
#else
		timeGetTime();
#endif
	*(s_stlVpAllocatedPoint+s_iAllocatedLength)	= xpPoint;
	if(nSerialNumber == 251303
		|| nSerialNumber == 251347
		)
	{
		INT32 x=0; 
		x++;
	}
	s_iAllocatedLength++;
	return xpPoint;
}


void BaseMemoryPoolSingle::observe_push(void* _point)
{
	m_observe_m[_point] = _point;
	g_SendMessage(LOG_MSG, "observer push %x ==============================", _point);

	int serial = 0;
	for (INT32 i = s_iAllocatedLength - 1; i >= 0; i--) {
		if (*(s_stlVpAllocatedPoint + i) == _point)
		{
			serial = *(s_stlVpAllocatedSerial + i);
		}
	}
	char buf[255];
	long long templong = (long long)_point;
	unsigned long address = (unsigned long)templong;
	sprintf_s(buf, 255, "observer push #%d %lx ==============================\n", serial, address);
#ifdef WIN32
	OutputDebugStringA(buf);
#endif
}


bool BaseMemoryPoolSingle::observe_check(void* _point)
{
	DSTLMpVoid::iterator it;

	it = m_observe_m.find(_point);
	if (it != m_observe_m.end())
	{
		g_SendMessage(LOG_MSG, "observer check %x ==============================", _point);
		char buf[255];
		long long templong = (long long)_point;
		unsigned long address = (unsigned long)templong;
		sprintf_s(buf, 255, "observer check %lx = th: %d =============================\n", address, m_nThreadIndex);
#ifdef WIN32
		OutputDebugStringA(buf);
#endif
		return true;
	}
	return false;
}

void BaseMemoryPoolSingle::observe_out(void* _point)
{
	DSTLMpVoid::iterator it;

	it = m_observe_m.find(_point);
	if (it != m_observe_m.end())
	{
		g_SendMessage(LOG_MSG, "observer out %x ==============================", _point);
		char buf[255];
		long long templong = (long long)_point;
		unsigned long address = (unsigned long)templong;
		sprintf_s(buf, 255, "observer out %lx ==============================\n", address);
#ifdef WIN32
		OutputDebugStringA(buf);
#endif
		m_observe_m.erase(it);
	}
}

void BaseMemoryPoolSingle::leak_info_clear()
{
	m_bCalledClear	= true;
	if(s_iAllocatedReserve > 0)
	{
		BaseSystem::free(s_stlVpAllocatedFile);
		BaseSystem::free(s_stlVpAllocatedLine);
		BaseSystem::free(s_stlVpAllocatedSerial);
		BaseSystem::free(s_stlVpAllocatedPoint);
		s_iAllocatedReserve	= 0;
		s_iAllocatedLength	= 0;
	}
}

//#define TRACE_RELEASED_POINT

bool BaseMemoryPoolSingle::leak_check_out(void* xpPoint)
{
#ifdef TRACE_RELEASED_POINT
	int cur = BaseSystem::timeGetTime();

	for (int i = 0; i < s_delayCheckoutPoint.size(); i++)
	{
		if (s_delayCheckoutPoint[i].time < cur - 50)
		{
			BaseSystem::free(s_delayCheckoutPoint[i].file);
			s_delayCheckoutPoint.erase(s_delayCheckoutPoint.begin() + i);
			i--;
		}
	}
#endif

	//observe_out(xpPoint);
	bool	bFree = false;
	for (INT32 i = s_iAllocatedLength - 1; i >= 0; i--) {
		if (*(s_stlVpAllocatedPoint + i) == xpPoint)
		{
#ifdef TRACE_RELEASED_POINT
			STDelayCheck check;
			check.line = *(s_stlVpAllocatedLine + i);
			check.point = *(s_stlVpAllocatedPoint + i);
			check.file = *(s_stlVpAllocatedFile + i);
			check.time = cur;
			s_delayCheckoutPoint.push_back(check);
#else
			BaseSystem::free(*(s_stlVpAllocatedFile + i));
#endif

			INT32 j = s_iAllocatedLength - 1;

			*(s_stlVpAllocatedPoint + i) = *(s_stlVpAllocatedPoint + j);
			*(s_stlVpAllocatedLine + i) = *(s_stlVpAllocatedLine + j);
			*(s_stlVpAllocatedSerial + i) = *(s_stlVpAllocatedSerial + j);
			*(s_stlVpAllocatedFile + i) = *(s_stlVpAllocatedFile + j);
			s_iAllocatedLength--;
			bFree = true;
			return true;
		}
	}
	return false;
}

void BaseMemoryPoolSingle::leak_check(const void* xpPoint)
{
	bool	bFree = false;

	for (int i = (int)s_delayCheckoutPoint.size()-1; i>=0; i--)
	{
		if (s_delayCheckoutPoint[i].point == xpPoint)
		{
			char strBuff[1024];
			sprintf_s(strBuff, 1024, "%s(%d) : leak info checked. thread:%d\n", s_delayCheckoutPoint[i].file, s_delayCheckoutPoint[i].line, m_nThreadIndex);
#ifdef WIN32
			OutputDebugStringA(strBuff);
#endif
		}
	}
}

void BaseMemoryPoolSingle::leak_display()
{
	for(INT32 i=0; i<s_iAllocatedLength; i++){
		//g_SendMessage(LOG_MSG, "%s(%d) : #%d memory leak detected.\n", s_stlVpAllocatedFile[i], s_stlVpAllocatedLine[i], s_stlVpAllocatedSerial[i]);
		//printf("%s(%d) : #%d memory leak detected.\n", s_stlVpAllocatedFile[i], s_stlVpAllocatedLine[i], s_stlVpAllocatedSerial[i]);
		char strBuff[1024];
		const char *strFind = strstr(s_stlVpAllocatedFile[i], "ptallocator.h");
		if (strFind == NULL && s_stlVpAllocatedLine[i] != 43) { // because of other leak will make this leak or new operator
			sprintf_s(strBuff, 1024, "%s(%d) : #%d memory leak detected. thread:%d\n", s_stlVpAllocatedFile[i], s_stlVpAllocatedLine[i], s_stlVpAllocatedSerial[i], m_nThreadIndex);
#ifdef WIN32
			OutputDebugStringA(strBuff);
#endif
			// printf("%s", strBuff);
		}
	}

	leak_info_clear();
}


void BaseMemoryPoolSingle::leak_old_display(int _time)
{
	int start_index = 0, end_index = -1;
	INT32 current =
#ifdef _WIN32
		timeGetTime();
#else
		timeUint32();
#endif
	current -= _time;
	for(int i = s_iAllocatedLength-1; i>=0; i--)
	{
		if(s_stlVpAllocatedSerial[i] < current)
		{
			end_index = i;
			break;
		}
	}

	current -= _time;
	for(int i = end_index; i>=0; i--)
	{
		if(s_stlVpAllocatedSerial[i] < current)
		{
			start_index = i;
			break;
		}
	}

	for(INT32 i=start_index; i<end_index; i++){
		char strBuff[1024];
		const char *strFind = strstr(s_stlVpAllocatedFile[i], "ptallocator.h");
		if (strFind == NULL && s_stlVpAllocatedLine[i] != 43) { // because of other leak will make this leak or new operator
			sprintf_s(strBuff, 1024, "%s(%d) : #%d memory leak detected. thread:%d\n", s_stlVpAllocatedFile[i], s_stlVpAllocatedLine[i], s_stlVpAllocatedSerial[i], m_nThreadIndex);
#ifdef WIN32
			OutputDebugStringA(strBuff);
#endif
			// printf("%s", strBuff);
		}
	}

	//leak_info_clear();
}
#endif


void *BaseMemoryPoolSingle::malloc(size_t _nSize, const char *_strFileName, INT32 _nLine)
{
	g_BreakPoint();
	void *pPoint	= NULL;
	while((pPoint = m_queueFree.pop()) != NULL)
	{
		free_(pPoint);
	}

	if(_nSize == 0)
		return NULL;

	if(m_bFirstMemoryPull)
		init_all();
	
	INT32	nSize;
	bbyte nSizeShift;
	nSizeShift	= BaseMemoryPoolBlock::get_size_shift_((INT32)_nSize);
	nSize		= get_size_(nSizeShift);

	if(m_sMemoryLayer[nSizeShift].is_init() == false)
	{
		if(nSize >= (1<<24))
			m_sMemoryLayer[nSizeShift].init(nSize, 2, _strFileName, _nLine); // it have to change to smart system
		else
			m_sMemoryLayer[nSizeShift].init(nSize, COUNT_INIT, _strFileName, _nLine); // it have to change to smart system
	}

	bbyte*pRet;
	pRet	= (bbyte*)m_sMemoryLayer[nSizeShift].get_new_item(&m_bUpdate, _strFileName, _nLine);
	if (!pRet)
		return NULL;

	*((INT32*)(pRet-4))	= (INT32)_nSize;
	*((const unsigned char**)(pRet-12))	= (const unsigned char*)_strFileName;
	*((INT32*)(pRet-16))	= (INT32)_nLine;
	*(pRet+_nSize)	= 0xfd;

	m_nAllocatedMemory++;
	
#ifdef _DEBUG
	BaseSystem::memset(pRet, 0xce, _nSize);

	leak_check_in(_nLine, _strFileName, pRet);
#endif

	g_BreakPoint();
	return (void*)pRet;
}

void BaseMemoryPoolSingle::display_info()
{
//	if(!m_bCalledClear)
//		leak_info_clear();

	for(INT32 i=0; i<__MEMORYPOOL_SIZE_LAYER; i++)
	{	
		m_sMemoryLayer[i].display_info();
	}

#ifdef _DEBUG
	leak_display();
#endif
}

void BaseMemoryPoolSingle::index_set(unsigned short _nIndex)
{
	m_nThreadIndex	= _nIndex;
}

char *BaseMemoryPoolSingle::get_filename(char *_strFilename, INT32 _nLen)
{
	if(m_nReservedType != 0)
		sprintf_s(_strFilename, 255, "alloc_rev_%d.rev", m_nReservedType);
	else
		sprintf_s(_strFilename, 255, "alloc_%d.rev", m_nThreadIndex);

	return _strFilename;
}

INT32	BaseMemoryPoolSingle::get_reserved_type()
{
	return m_nReservedType;
}

void BaseMemoryPoolSingle::set_reserved_type(INT32 _nType)
{
	if(m_bFirstMemoryPull)
		init_all();

	m_nReservedType	= _nType;
	m_bUpdate	= true;

    return;
/*
	char strBuf[255], strFilename[255];
	get_filename(strFilename);

	FILE *pfile;
	pfile	= fopen(BaseSystem::module_get_data(strBuf, 255, strFilename), "rb");

	if(pfile)
	{
		INT32 nSizeOfItem, nInitCount;
		for(INT32 i=0; i<__MEMORYPOOL_SIZE_LAYER; i++)
		{
			fread(&nSizeOfItem, sizeof(INT32), 1, pfile);
			fread(&nInitCount, sizeof(INT32), 1, pfile);

			if(nSizeOfItem == 4)
			{
				m_bUpdate = true;
				return;
			}
			if(nSizeOfItem > 0)
				m_sMemoryLayer[i].init(nSizeOfItem, nInitCount);
		}
		fclose(pfile);

		m_bUpdate	= false;
	}else{
		m_bUpdate	= true;
	}
 //*/
}

bool BaseMemoryPoolSingle::free_add(void *_point)
{	
	if (m_bThreadEnd)
		return false;
    
    if(_point == NULL)
        return false;

	if (m_queueFree.size_data() > 1000)
	{
		//g_SendMessage(LOG_MSG, "push count %d-\n"// %s-----------------------------------\n"
		//	, m_queueFree.size_data());//, m_strCalledPos);
		//m_queueFree.push(_point);
		return false;
	}
#ifdef _DEBUG
	//observe_check(_point);
#endif
	return m_queueFree.push(_point);
}


void BaseMemoryPoolSingle::free(void *_point)
{
	void *pPoint	= NULL;
	while((pPoint = m_queueFree.pop()) != NULL)
	{
		free_(pPoint);
	}

	if(_point)
		free_(_point);
}

void BaseMemoryPoolSingle::free_(void *_point)
{
	g_BreakPoint();

	if(_point == NULL
		|| m_bFirstMemoryPull == true)
		return;
	
#ifdef _DEBUG
	if(!leak_check_out(_point))
	{
		//MessageBox(NULL, "Error: Try to free memory what doesn't allocated by memory pool.(BaseMemoryPoolSinge:free_()).", "Error", MB_OK);
		return;
	}
#endif

	bbyte nSizeShift;
	bbyte *pTemp	= (bbyte*)_point;
	nSizeShift	= *(pTemp-(SIZE_HEADER));
	//B_ASSERT(m_sMemoryLayer[nSizeShift].is_init());
	if(!m_sMemoryLayer[nSizeShift].is_init()){
#ifdef _DEBUG
        g_SendMessage(LOG_MSG_POPUP, "Error in(BaseMemoryPoolSinge:free_()).");
#endif
		return;
	}

	INT32 nSize	= *((INT32*)(pTemp-4));
	//B_ASSERT(nSize <= get_size_(nSizeShift));
	if(nSize > get_size_(nSizeShift) || nSize < 0){
#ifdef _DEBUG
		g_SendMessage(LOG_MSG_POPUP, "Error in(BaseMemoryPoolSinge:free_()).");
#endif
		return;
	}
	if(nSize <= get_size_(nSizeShift))
	{
		//B_ASSERT(*(pTemp+nSize)==(char)0xfd);
		if(*(pTemp+nSize)!=0xfd){
#ifdef _DEBUG
			g_SendMessage(LOG_MSG_POPUP, "Error in(BaseMemoryPoolSinge:free_()).");
#endif
			return;
		}
		m_sMemoryLayer[nSizeShift].delete_item(_point);
	}
#ifdef _DEBUG
	BaseSystem::memset(pTemp, 0xfd, nSize);
#endif

	m_nAllocatedMemory--;
	g_BreakPoint();

	if (m_nAllocatedMemory == 0)
	{
		//g_SendMessage(LOG_MSG, "All of memory freed.\n");
	}
}

std::map<int, BaseMemoryPoolSingle*>	*s_pstlMMemoryPoolSingle = NULL;
std::vector<BaseMemoryPoolSingle *>	*s_pstlVMemoryPoolSingle	= NULL;
std::vector<BaseMemoryPoolSingle*>* s_MemoryPoolHolded_pa = NULL;

typedef std::map<INT32, BaseCircleQueue *>	STLMpFreePoolSingle;
STLMpFreePoolSingle							*s_pstlMpFreePoolSingle;

#ifndef _WIN32
#define CRITICAL_SECTION				pthread_mutex_t
#define InitializeCriticalSection(a)	pthread_mutex_init(a, NULL)
#define DeleteCriticalSection		pthread_mutex_destroy
#define EnterCriticalSection		pthread_mutex_lock
#define LeaveCriticalSection		pthread_mutex_unlock
#define TlsGetValue					pthread_getspecific
#define TlsSetValue					pthread_setspecific
#define TlsFree						pthread_key_delete
static pthread_key_t s_dwTlsIndex;
#else
UINT32	s_dwTlsIndex	= -1;	// Thread 간 독립 메모리 공간을 할당하기 위해 사용하는 인덱스
#endif
static bool s_bInitialized	= false;
static bool s_bTerminated	= false;
CRITICAL_SECTION s_critical_section;

static UINT32 m_auto_freetime;
static BaseCircleQueue* m_auto_free_qp = NULL;// ("auto free table", 1);
static BaseCircleQueue* m_auto_ofree_qp = NULL;// ("auto O free table", 1);

BaseMemoryPoolMultiThread::BaseMemoryPoolMultiThread()
{
	if(!s_bInitialized)
		init();
}

void BaseMemoryPoolMultiThread::init()
{
	s_bTerminated	= false;
	s_bInitialized	= true;
	InitializeCriticalSection(&s_critical_section);

	m_auto_free_qp = new BaseCircleQueue("auto free table", 1);
	m_auto_free_qp = new BaseCircleQueue("auto O free table", 1);

	atexit(BaseMemoryPoolMultiThread::free_all);
	s_pstlMMemoryPoolSingle = new std::map<int, BaseMemoryPoolSingle*>;
	s_pstlVMemoryPoolSingle	= new std::vector<BaseMemoryPoolSingle*>;
	s_MemoryPoolHolded_pa = new std::vector<BaseMemoryPoolSingle*>;
	s_pstlVMemoryPoolSingle->reserve(MAX_THREAD);
	s_pstlMpFreePoolSingle	= new STLMpFreePoolSingle;
#ifdef WIN32
	s_dwTlsIndex	= TlsAlloc();
	B_ASSERT(s_dwTlsIndex != TLS_OUT_OF_INDEXES);
#else
	pthread_key_create(&s_dwTlsIndex, NULL);
#endif
	thread_main_set();
}

#include "hashstr.h"

INT32 BaseMemoryPoolMultiThread::sm_nHoldShutdownCount = 0;
INT32 BaseMemoryPoolMultiThread::sm_memory_total_size = 0;

void BaseMemoryPoolMultiThread::hold_shutdown_inc()
{
	sm_nHoldShutdownCount++;
}

void BaseMemoryPoolMultiThread::hold_shutdown_dec()
{
	sm_nHoldShutdownCount--;
}

#include "BaseStringTable.h"

void BaseMemoryPoolMultiThread::free_all()
{
	if(s_bTerminated)
		return;
#ifdef _DEBUG
	UniqHashStr::debug_release_hashtext();
#endif
	BaseStringTable::release();
	
	s_bTerminated	= true;

	while(sm_nHoldShutdownCount > 0)
		BaseSystem::Sleep(10);

	unsigned i;
	for(i=0; i<s_pstlVMemoryPoolSingle->size(); i++)
		delete s_pstlVMemoryPoolSingle->at(i);

	s_pstlVMemoryPoolSingle->clear();
	delete s_pstlVMemoryPoolSingle;
	s_pstlVMemoryPoolSingle	= NULL;
	delete s_pstlMMemoryPoolSingle;
	s_pstlMMemoryPoolSingle = NULL;
	delete s_MemoryPoolHolded_pa;
	s_MemoryPoolHolded_pa = NULL;

	STLMpFreePoolSingle::iterator	it;
	for(it=s_pstlMpFreePoolSingle->begin(); it!=s_pstlMpFreePoolSingle->end(); it++)
		delete it->second;
	delete s_pstlMpFreePoolSingle;
	s_pstlMpFreePoolSingle	= NULL;

	TlsFree(s_dwTlsIndex);
	DeleteCriticalSection(&s_critical_section);
}

BaseMemoryPoolMultiThread::~BaseMemoryPoolMultiThread(void)
{	
}

static BaseMemoryPoolSingle *s_pSingle = NULL;

bool	BaseMemoryPoolMultiThread::thread_main_check()
{
	BaseMemoryPoolSingle	*pPoolSingle = NULL;
	pPoolSingle = (BaseMemoryPoolSingle*)TlsGetValue(s_dwTlsIndex);// 쓰레드별 메모리 풀이 따로 있다.

	if (s_pSingle != NULL && s_pSingle == pPoolSingle)
		return true;
	return false;
}

int BaseMemoryPoolMultiThread::thread_index_get()
{
	BaseMemoryPoolSingle* pPoolSingle = NULL;
	pPoolSingle = (BaseMemoryPoolSingle*)TlsGetValue(s_dwTlsIndex);// 쓰레드별 메모리 풀이 따로 있다.

	return pPoolSingle->index_get();
}

bool BaseMemoryPoolMultiThread::thread_current_check(int _thread)
{
	BaseMemoryPoolSingle* pPoolSingle = NULL;
	pPoolSingle = (BaseMemoryPoolSingle*)TlsGetValue(s_dwTlsIndex);// 쓰레드별 메모리 풀이 따로 있다.

	if(_thread == pPoolSingle->index_get())
		return true;
	return false;
}

INT32 BaseMemoryPoolMultiThread::memory_total_size()
{
	return sm_memory_total_size;
}

void	BaseMemoryPoolMultiThread::thread_main_set()
{
	BaseMemoryPoolSingle	*pPoolSingle = NULL;
	pPoolSingle = (BaseMemoryPoolSingle*)TlsGetValue(s_dwTlsIndex);// 쓰레드별 메모리 풀이 따로 있다.

	if (pPoolSingle == NULL)
	{
		pPoolSingle = thread_create_single();
	}
	s_pSingle = pPoolSingle;
}

BaseMemoryPoolSingle *BaseMemoryPoolMultiThread::thread_create_single(INT32 _nReservedType)
{
	BaseMemoryPoolSingle	*pPoolSingle = NULL;
	if (_nReservedType != 0)
	{
		STLMpFreePoolSingle::iterator	it;
		lock();
		it = s_pstlMpFreePoolSingle->find(_nReservedType);
		if (it != s_pstlMpFreePoolSingle->end())
			pPoolSingle = (BaseMemoryPoolSingle*)it->second->pop();
		unlock();
	}
	else {
		lock();
		if (s_MemoryPoolHolded_pa->size() > 0)
		{
			pPoolSingle = s_MemoryPoolHolded_pa->back();
			pPoolSingle->thread_realloc();
			s_MemoryPoolHolded_pa->erase(--s_MemoryPoolHolded_pa->end());
		}
		unlock();
	}

	if (pPoolSingle == NULL)
	{
		// printf("created a new memorypoolsingle\n");
		lock();//EnterCriticalSection(&s_critical_section);
		static unsigned short thread_cnt = 0;
		pPoolSingle = new BaseMemoryPoolSingle;
		s_pstlVMemoryPoolSingle->push_back(pPoolSingle); // MAX_THREAD이상의 thread할당 될 경우 다운될 수 있다.
		thread_cnt++;
		(*s_pstlMMemoryPoolSingle)[thread_cnt] = pPoolSingle;
		pPoolSingle->index_set(thread_cnt);
		pPoolSingle->set_reserved_type(_nReservedType);
		unlock();//LeaveCriticalSection(&s_critical_section);
	}
	TlsSetValue(s_dwTlsIndex, pPoolSingle);
	return pPoolSingle;
}

BaseMemoryPoolSingle	*BaseMemoryPoolMultiThread::get_single(INT32 _nReservedType)
{
	BaseMemoryPoolSingle	*pPoolSingle	= NULL;
	pPoolSingle	= (BaseMemoryPoolSingle*)TlsGetValue(s_dwTlsIndex);// 쓰레드별 메모리 풀이 따로 있다.
	
	if(pPoolSingle == NULL)
	{
		pPoolSingle = thread_create_single(_nReservedType);
	}

	return pPoolSingle;
}

void *BaseMemoryPoolMultiThread::malloc(size_t _nSize, const char *_strFileName, INT32 _nLine)
{
#ifdef DEF_MOBILE
	auto_update();
    return BaseSystem::malloc(_nSize);
#else
	if(s_bInitialized)
		auto_update();
#endif
	if(_nSize == 0 || s_bTerminated)
		return NULL;

	if(!s_bInitialized)
		init();

	BaseMemoryPoolSingle	*pPoolSingle	= get_single();

#ifdef _DEBUG
	static char s_strBuf[255];
	static char s_nLineCnt;
	char *pRet	= (char*)pPoolSingle->malloc(_nSize, _strFileName, _nLine);
	//if(_nSize > 1024000)
	//	pPoolSingle->observe_push(pRet);
	s_nLineCnt	= _nLine;
	strcpy(s_strBuf, _strFileName);
#else
	char *pRet	= (char*)pPoolSingle->malloc(_nSize);
#endif
	if (!pRet)
		return NULL;
	
	*((unsigned short*)(pRet-(SIZE_HEADER+SIZE_THREADID)))	= pPoolSingle->index_get(); // 할당된 메모리에 쓰레드 인덱스를 넣는다.
	return (void*)pRet;
}

void BaseMemoryPoolMultiThread::observe_push(void* _point)
{
	BaseMemoryPoolSingle* pPoolSingle = get_single();
#ifdef _DEBUG
	pPoolSingle->observe_push(_point);
#endif
}


bool BaseMemoryPoolMultiThread::checkOverwrite(void* _point)
{
	if (!_point)
		return false;

	bbyte nSizeShift;
	bbyte* pTemp = (bbyte*)_point;
	nSizeShift = *(pTemp - (SIZE_HEADER));
	
	INT32 nSize = *((INT32*)(pTemp - 4));
	
	if (*(pTemp + nSize) != 0xfd) {
		return true;
	}
	return false;
}

void BaseMemoryPoolMultiThread::free(void *_point)
{
#ifdef DEF_MOBILE
	BaseSystem::free(_point);
    return;
#endif
    
#ifdef _DEBUG
	if(_point == (void*)(INT64)0xfefefefe)
	{
		B_ASSERT(0);
		return;
	}
#endif
	if(s_bTerminated)
		return;

	if(!s_bInitialized)
		init();

	BaseMemoryPoolSingle	*pPoolSingle	= get_single();

	if(_point == NULL)
	{
		pPoolSingle->free(NULL);
		return;
	}

	unsigned short	nThreadIndex;
	char	*pPoint	= (char*)_point;
	nThreadIndex	= *((unsigned short*)(pPoint-(SIZE_HEADER+SIZE_THREADID))); // 할당된 메모리에 쓰레드 인덱스가 들어있다

	if (nThreadIndex == pPoolSingle->index_get()) // 같은 쓰레드인 경우 바로 free하고 아닌경우 Queue에 넣어서 해당 스레드에서 free되도록 한다.
	{
		pPoolSingle->free(_point);
		return;
	}

	std::map<int, BaseMemoryPoolSingle*>::iterator it;
	lock();
	it = s_pstlMMemoryPoolSingle->find(nThreadIndex);
#ifdef _DEBUG
	B_ASSERT(it != s_pstlMMemoryPoolSingle->end());
#endif
	unlock();
	if (it == s_pstlMMemoryPoolSingle->end())
		return;
	
	BaseMemoryPoolSingle* pPoolSingleOther = it->second;
	
	if(!pPoolSingleOther->free_add(_point))
	{
		lock();
		pPoolSingleOther->free(_point);
		unlock();
	}
}

void BaseMemoryPoolMultiThread::end(INT32 _nReservedType)
{	
	BaseMemoryPoolSingle *pSingle	= get_single(_nReservedType);

	if (_nReservedType == 0)
	{
		lock();
		for (int i = 0; i < s_pstlVMemoryPoolSingle->size(); i++)
		{
			if (s_pstlVMemoryPoolSingle->at(i) == pSingle) {
				s_pstlVMemoryPoolSingle->erase(s_pstlVMemoryPoolSingle->begin() + i);
				break;
			}
		}
		pSingle->thread_end();
		s_MemoryPoolHolded_pa->push_back(pSingle);
		unlock();
		return;
	}
	
	STLMpFreePoolSingle::iterator	it;

	BaseCircleQueue	*pQueue	= NULL;
	lock();
	it	= s_pstlMpFreePoolSingle->find(_nReservedType);
	if(it == s_pstlMpFreePoolSingle->end())
	{
		pQueue	= new BaseCircleQueue("BaseMemoryPoolMultiThread:end", 1024);

		(*s_pstlMpFreePoolSingle)[_nReservedType]	= pQueue;
	}else{
		pQueue	= it->second;
	}
	unlock();
	pQueue->push(pSingle);
}

void BaseMemoryPoolMultiThread::start(INT32 _nReservedType)
{
	//BaseMemoryPoolSingle *pSingle	=
    get_single(_nReservedType);
}

void BaseMemoryPoolMultiThread::lock()
{
	EnterCriticalSection(&s_critical_section);
}

void BaseMemoryPoolMultiThread::unlock()
{	
	LeaveCriticalSection(&s_critical_section);
}

void BaseMemoryPoolMultiThread::terminate(bool _bTerminated)
{
	s_bTerminated	= _bTerminated;
}

bool BaseMemoryPoolMultiThread::is_terminated()
{
	return s_bTerminated;
}

void BaseMemoryPoolMultiThread::leak_check(const void* _point) 
{
#ifdef _DEBUG
	for (UINT32 i = 0; i < s_pstlVMemoryPoolSingle->size(); i++)
		s_pstlVMemoryPoolSingle->at(i)->leak_check(_point);
#endif
}

void BaseMemoryPoolMultiThread::leak_old_display(int _time)
{
#ifdef _DEBUG
	for(UINT32 i=0; i<s_pstlVMemoryPoolSingle->size(); i++)
	{
		s_pstlVMemoryPoolSingle->at(i)->leak_old_display(_time);
	}
#endif
}

void BaseMemoryPoolMultiThread::display_info()
{
	for(UINT32 i=0; i<s_pstlVMemoryPoolSingle->size(); i++)
	{
		INT32 nType;
		nType	= s_pstlVMemoryPoolSingle->at(i)->get_reserved_type();
		g_SendMessage(LOG_MSG_MEM_LEAK, "Thread type %d\n", nType);
		//printf("Thread type %d\n", nType);
		s_pstlVMemoryPoolSingle->at(i)->display_info();
	}
}

INT64 BaseMemoryPoolMultiThread::get_alloc(int _size)
{
	if (_size == 0)
		return 0;

	char* buf = PT_Alloc(char, _size+4);
	*((int*)buf) = _size;
	INT64 ref = (INT64)(buf+4);
	return ref;
}

void* BaseMemoryPoolMultiThread::get_mem(INT64 _ref, int* _size)
{
	if (_ref == 0)
		return NULL;
	char* buf = (char*)_ref;
	*_size = *((int*)(buf - 4));
	return buf;
}

void BaseMemoryPoolMultiThread::free_mem(INT64 _ref)
{
	if (_ref == 0)
		return;
	char* buf = (char*)_ref;
	buf -= 4;
	PT_Free(buf);
}

void BaseMemoryPoolMultiThread::auto_free(void *_point)
{
	m_auto_free_qp->push(_point);
	m_auto_freetime = BaseSystem::timeGetTime() + 50; // 50 mili sec later will be free
}

void BaseMemoryPoolMultiThread::auto_ofree(void *_point)
{
	m_auto_ofree_qp->push(_point);
	m_auto_freetime = BaseSystem::timeGetTime() + 50; // 50 mili  sec later will be free
}

void BaseMemoryPoolMultiThread::auto_update()
{
	if(BaseSystem::timeGetTime() < m_auto_freetime)
		return;
	
	if(m_auto_free_qp == NULL || m_auto_ofree_qp == NULL || (m_auto_free_qp->size_data() == 0
	   && m_auto_ofree_qp->size_data() == 0))
		return;
	
	void *point = NULL;
	do{
		void *p;
		point = m_auto_free_qp->pop();
		if(point == NULL)
			break;
		p = point;
		PT_Free(p);
	}while(point);
	
	if(m_auto_ofree_qp->size_data() == 0)
		return;
	
	do{
		BaseObject *p;
		point = m_auto_ofree_qp->pop();
		if(point == NULL)
			break;
		p = (BaseObject*)point;
		PT_OFree(p);
	}while(point);
}
