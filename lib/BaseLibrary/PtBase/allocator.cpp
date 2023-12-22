#include "stdafx.h"

//void *new_my(size_t sz)
//{
//	return BaseMemoryPull::malloc((int)sz);
//}
//
//void delete_my(void *p)
//{
//	if(p == NULL)
//		return;
//#ifdef _DEBUG
//	G_ResCheckOut(p);
//#endif
//	BaseMemoryPull::free(p);
//}
//
//void* operator new(size_t sz)
//{
//#ifdef _DEBUG
//	return G_ResCheckIn(0, "Array", BaseMemoryPull::malloc((int)sz));
//#else
//	return BaseMemoryPull::malloc((int)sz);
//#endif
//}
//
//void operator delete(void *p)
//{
//	if(p == NULL)
//		return;
//
//#ifdef _DEBUG
//	G_ResCheckOut(p);
//#endif
//	BaseMemoryPull::free(p);
//}
//
//void* operator new[](size_t sz)
//{
//#ifdef _DEBUG
//	return G_ResCheckIn(0, "Array", BaseMemoryPull::malloc((int)sz));
//#else
//	return BaseMemoryPull::malloc((int)sz);
//#endif
//}
//
//void operator delete[](void *p)
//{
//	if(p == NULL)
//		return;
//#ifdef _DEBUG
//	G_ResCheckOut(p);
//#endif
//	BaseMemoryPull::free(p);
//}
//
//void * operator new(
//							 size_t cb,
//							 int nBlockUse,
//							 const char * szFileName,
//							 int nLine
//							 )
//{
//	return G_ResCheckIn(nLine, szFileName, BaseMemoryPull::malloc((int)cb));
//}
//
//void *operator new[](
//							   size_t cb,
//							   int nBlockUse,
//							   const char * szFileName,
//							   int nLine
//							   )
//{
//	return G_ResCheckIn(nLine, szFileName, BaseMemoryPull::malloc((int)cb));
//}
