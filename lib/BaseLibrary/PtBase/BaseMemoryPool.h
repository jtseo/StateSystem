#pragma once

#define __MEMORYPOOL_SIZE_BLOCK	64
#define __MEMORYPOOL_SIZE_LAYER	64

//*
BaseMemoryPoolMultiThread& mpool_get();

enum{
	THTYPE_ROOT				= 0,
	THTYPE_MAINTHREAD		= 100,
	THTYPE_LOGIN_SENDMAIL	= 101,
	THTYPE_PACKET_UPDATE	= 102,
	THTYPE_LOGIN_STATE		= 103,
	THTYPE_BASE_NETMANAGER	= 200,
	THTYPE_BASE_TCP			= 201,
    THTYPE_BASE_TCP_SESSION = 202,
	THTYPE_CHARGE_RECEIVE	= 300,
	THTYPE_CHARGE_SENDMAIL	= 301,
	THTYPE_DBQUERY			= 500,
	THTYPE_STATEMANAGER		= 600
};

#define PT_ThreadStart(x)	mpool_get().start(x)
#define PT_ThreadEnd(x)		mpool_get().end(x)
#define PT_Mem_END()		mpool_get().free_all()
#define PT_MemDisplay()	mpool_get().display_info()
#define PT_Update()			mpool_get().free(NULL)

#ifdef _DEBUG

#define PT_Alloc(T, count) (T*)mpool_get().malloc(sizeof(T)*(count), __FILE__, __LINE__)
#define PT_MAlloc(T)		(T*)mpool_get().malloc(sizeof(T), __FILE__, __LINE__)
#define PT_Free(T)			mpool_get().free((void*)T),T=NULL
#define PT_AFree(T)			mpool_get().auto_free((void*)T)

#define PT_OAllocS(T1, TType, st) T1	= (TType*)mpool_get().malloc(sizeof(TType), __FILE__, __LINE__), \
	::new(T1) TType(st)

#define PT_OFreeS(T) PT_Destroy(T),mpool_get().free(T)

#define PT_OAlloc(T1, TType) T1	= (TType*)mpool_get().malloc(sizeof(TType), __FILE__, __LINE__), \
	::new(T1) TType(TType::sm_sample),\
    T1->optionSet(OPT_CreatePrivate), \
	T1->init()

#define PT_OAlloc1(T1, TType, TParam) T1	= (TType*)mpool_get().malloc(sizeof(TType), __FILE__, __LINE__), \
	::new(T1) TType(TType::sm_sample),\
    T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam)

#define PT_OAlloc2(T1, TType, TParam1, TParam2)	T1 = (TType*)mpool_get().malloc(sizeof(TType), __FILE__, __LINE__), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam1, TParam2)

#define PT_OAlloc3(T1, TType, TParam1, TParam2, TParam3)	T1 = (TType*)mpool_get().malloc(sizeof(TType), __FILE__, __LINE__), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam1, TParam2, TParam3)

#define PT_OAlloc4(T1, TType, TParam1, TParam2, TParam3, TParam4)	T1 = (TType*)mpool_get().malloc(sizeof(TType), __FILE__, __LINE__), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam1, TParam2, TParam3, TParam4)

void check_debug_m_privateCreateType(int _nCheck);
#define PT_OFree(T) check_debug_m_privateCreateType(T->optionGet(OPT_CreatePrivate)), T->release(),PT_Destroy(T),mpool_get().free(T)
#define PT_AOFree(T)	mpool_get().auto_ofree(T)

#else


#define PT_OAllocS(T1, TType, st) T1	= (TType*)mpool_get().malloc(sizeof(TType)), \
	::new(T1) TType(st)

#define PT_OFreeS(T) PT_Destroy(T),mpool_get().free(T)

#define PT_Alloc(T, count)	(T*)mpool_get().malloc(sizeof(T)*(count))
#define PT_MAlloc(T)		(T*)mpool_get().malloc(sizeof(T))
#define PT_Free(T)			mpool_get().free((void*)T),T=NULL
#define PT_AFree(T)			mpool_get().auto_free((void*)T)

#define PT_OAlloc(T1, TType) T1	= (TType*)mpool_get().malloc(sizeof(TType)), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init()

#define PT_OAlloc1(T1, TType, TParam) T1	= (TType*)mpool_get().malloc(sizeof(TType)), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam)

#define PT_OAlloc2(T1, TType, TParam1, TParam2)	T1 = (TType*)mpool_get().malloc(sizeof(TType)), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam1, TParam2)

#define PT_OAlloc3(T1, TType, TParam1, TParam2, TParam3)	T1 = (TType*)mpool_get().malloc(sizeof(TType)), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam1, TParam2, TParam3)

#define PT_OAlloc4(T1, TType, TParam1, TParam2, TParam3, TParam4)	T1 = (TType*)mpool_get().malloc(sizeof(TType)), \
	::new(T1) TType(TType::sm_sample),\
T1->optionSet(OPT_CreatePrivate), \
	T1->init(TParam1, TParam2, TParam3, TParam4)

#define PT_OFree(T) T->release(),\
	PT_Destroy(T),\
	mpool_get().free(T)

#define PT_AOFree(T)	mpool_get().auto_ofree(T)
#endif

#define PT_COperator(Class) void* Class##::operator new(size_t size)\
			{	void *pRet = PT_Alloc(char, size);	return pRet;}\
			void Class##::operator delete(void* pDeleteUser)\
			{	PT_Free(pDeleteUser);}

#define PT_COperatorH static void* operator new(size_t size);\
			static void operator delete(void* pDeleteUser)

enum PtOptionType{
		OPT_NONE			= 0,
		OPT_EventLocal		= 0x01,
		OPT_CreatePrivate	= 0x02
	};

//	*/
#define PT_OPTHeader	public:\
	bool optionGet(PtOptionType _opt) { return (m_option & _opt) != 0; };\
	void optionSet(char _option);\
	static void optionSet(char *_opt_member, char _opt);\
protected:	\
	char	m_option

void pt_optionSet(char *_option_member, char _opt);
#define PT_OPTCPP(class) void class::optionSet(char _option)\
{\
	pt_optionSet(&m_option, _option);\
}
