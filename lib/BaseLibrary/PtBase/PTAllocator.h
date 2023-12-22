// xmemory internal header (from <memory>)
#pragma once

//#ifndef _PT_XMEMORY_
//#define _PT_XMEMORY_
//#ifndef RC_INVOKED

#ifdef  _MSC_VER
#pragma pack(push,_CRT_PACKING)
#pragma warning(push,3)

#pragma warning(disable: 4100)
#endif  /* _MSC_VER */

#ifndef _FARQ	/* specify standard memory model */
#define _FARQ
#define _PDFT	ptrdiff_t
#define _SIZT	size_t
#endif /* _FARQ */

#define PT_CPOINTER_X(T, A)		\
	typename A::template rebind<T>::other::const_pointer
#define PT_CREFERENCE_X(T, A)	\
	typename A::template rebind<T>::other::const_reference
#define PT_POINTER_X(T, A)	\
	typename A::template rebind<T>::other::pointer
#define PT_REFERENCE_X(T, A)	\
	typename A::template rebind<T>::other::reference

// TEMPLATE FUNCTION _Allocate
template<class _Ty> inline
_Ty _FARQ *PT_Allocate(_SIZT _Count, _Ty _FARQ *)
{	// check for integer overflow
	if (_Count <= 0)
		_Count = 0;
	else if (((_SIZT)(-1) / _Count) < sizeof (_Ty))
		//__asm int 3;
		assert(0);// 2003지원을 위한 코드
		//_THROW_NCEE(std::bad_alloc, NULL);

	// allocate storage for _Count elements of type _Ty
	//return ((_Ty _FARQ *) G_ResCheckIn(__LINE__, __FILE__, ::operator new(_Count * sizeof (_Ty))));
	return (_Ty _FARQ *) PT_Alloc(_Ty, _Count);
}

#define _CONSTRUCTOR(val, ty, ptr)	(ptr)->ty(val)
// TEMPLATE FUNCTION _Construct
template<class _T1,
class _T2> inline
	void PT_Construct(_T1 _FARQ *_Ptr, const _T2& _Val)
{	// construct object at _Ptr with value _Val
	void _FARQ *_Vptr = _Ptr;
	::new (_Vptr) _T1(_Val);
	//G_ResCheckIn(__LINE__, __FILE__, ::new (_Vptr) _T1(_Val));
	//_Vptr	= PT_MAlloc(_T1);
	//memcpy(_Vptr, &_Val, sizeof(_T1));
	//_CONSTRUCTOR(_Val, _T1, _Ptr);
}

// TEMPLATE FUNCTION _Destroy
template<class _Ty> inline
void PT_Destroy(_Ty _FARQ *_Ptr)
{	// destroy object at _Ptr
	//_DESTRUCTOR(_Ty, _Ptr);
	((_Ty*)_Ptr)->~_Ty();
}

template<> inline
void PT_Destroy(char _FARQ *_ptr)
{	// destroy a char (do nothing)
	PT_Free(_ptr);
}

template<> inline
void PT_Destroy(wchar_t _FARQ * _ptr)
{	// destroy a wchar_t (do nothing)
	PT_Free(_ptr);
}


// TEMPLATE CLASS _Allocator_base
template<class _Ty>
struct PT_Allocator_base
{	// base class for generic allocators
	typedef _Ty value_type;
};

// TEMPLATE CLASS _Allocator_base<const _Ty>
template<class _Ty>
struct PT_Allocator_base<const _Ty>
{	// base class for generic allocators for const _Ty
	typedef _Ty value_type;
};

// TEMPLATE CLASS allocator
template<class _Ty>
class PT_allocator
	: public PT_Allocator_base<_Ty>
{	// generic allocator for objects of class _Ty
public:
	typedef PT_Allocator_base<_Ty> _Mybase;
	typedef typename _Mybase::value_type value_type;
	typedef value_type _FARQ *pointer;
	typedef value_type _FARQ& reference;
	typedef const value_type _FARQ *const_pointer;
	typedef const value_type _FARQ& const_reference;

	typedef _SIZT size_type;
	typedef _PDFT difference_type;

	template<class _Other>
	struct rebind
	{	// convert an allocator<_Ty> to an allocator <_Other>
		typedef PT_allocator<_Other> other;
	};

	pointer address(reference _Val) const
	{	// return address of mutable _Val
		return (&_Val);
	}

	const_pointer address(const_reference _Val) const
	{	// return address of nonmutable _Val
		return (&_Val);
	}
#ifdef WIN32
	PT_allocator() //_THROW0()
#else
	PT_allocator()
#endif
	{	// construct default allocator (do nothing)
	}

#ifdef WIN32
	PT_allocator(const PT_allocator<_Ty>&) //_THROW0()
#else
	PT_allocator(const PT_allocator<_Ty>&)
#endif
	{	// construct by copying (do nothing)
	}

	template<class _Other>
#ifdef WIN32
	PT_allocator(const PT_allocator<_Other>&) //_THROW0()
#else
	PT_allocator(const PT_allocator<_Other>&)
#endif
	{	// construct from a related allocator (do nothing)
	}

	template<class _Other>
	PT_allocator<_Ty>& operator=(const PT_allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}

	void deallocate(pointer _Ptr, size_type)
	{	// deallocate object at _Ptr, ignore size
		//::operator delete(_Ptr);
		PT_Free(_Ptr);
	}

	pointer allocate(size_type _Count)
	{	// allocate array of _Count elements
		return (PT_Allocate(_Count, (pointer)0));
	}

	pointer allocate(size_type _Count, const void _FARQ *)
	{	// allocate array of _Count elements, ignore hint
		return (allocate(_Count));
	}

	void construct(pointer _Ptr, const _Ty& _Val)
	{	// construct object at _Ptr with value _Val
		PT_Construct(_Ptr, _Val);
	}

	void destroy(pointer _Ptr)
	{	// destroy object at _Ptr
		PT_Destroy(_Ptr);
	}

#ifdef WIN32
	_SIZT max_size() const //_THROW0()
#else
	_SIZT max_size() const
#endif
	{	// estimate maximum array size
		_SIZT _Count = (_SIZT)(-1) / sizeof (_Ty);
		return (0 < _Count ? _Count : 1);
	}
};

// allocator TEMPLATE OPERATORS
template<class _Ty,
class _Other> inline
#ifdef WIN32
	bool operator==(const PT_allocator<_Ty>&, const PT_allocator<_Other>&) //_THROW0()
#else
	bool operator==(const PT_allocator<_Ty>&, const PT_allocator<_Other>&)
#endif
{	// test for allocator equality (always true)
	return (true);
}

template<class _Ty,
class _Other> inline
#ifdef WIN32
	bool operator!=(const PT_allocator<_Ty>&, const PT_allocator<_Other>&) //_THROW0()
#else
	bool operator!=(const PT_allocator<_Ty>&, const PT_allocator<_Other>&)
#endif
{	// test for allocator inequality (always false)
	return (false);
}

// CLASS allocator<void>
template<> class PT_allocator<void>
{	// generic allocator for type void
public:
	typedef void _Ty;
	typedef _Ty _FARQ *pointer;
	typedef const _Ty _FARQ *const_pointer;
	typedef _Ty value_type;

	template<class _Other>
	struct rebind
	{	// convert an allocator<void> to an allocator <_Other>
		typedef PT_allocator<_Other> other;
	};

#ifdef WIN32
	PT_allocator() //_THROW0()
#else
	PT_allocator()
#endif
	{	// construct default allocator (do nothing)
	}

#ifdef WIN32
	PT_allocator(const PT_allocator<_Ty>&) //_THROW0()
#else
	PT_allocator(const PT_allocator<_Ty>&)
#endif
	{	// construct by copying (do nothing)
	}

	template<class _Other>
#ifdef WIN32
	PT_allocator(const PT_allocator<_Other>&) //_THROW0()
#else
	PT_allocator(const PT_allocator<_Other>&)
#endif
	{	// construct from related allocator (do nothing)
	}

	template<class _Other>
	PT_allocator<_Ty>& operator=(const PT_allocator<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}
};

// TEMPLATE FUNCTION _Destroy_range
template<class _Ty,
class _Alloc> inline
	void PT_Destroy_range(_Ty *_First, _Ty *_Last, _Alloc& _Al)
{	// destroy [_First, _Last)
	PT_Destroy_range(_First, _Last, _Al, _Ptr_cat(_First, _Last));
}
/*
#ifdef WIN32
template<class _Ty,
class _Alloc> inline
	void PT_Destroy_range(_Ty *_First, _Ty *_Last, _Alloc& _Al,
	std::_Nonscalar_ptr_iterator_tag)
{	// destroy [_First, _Last), arbitrary type
	for (; _First != _Last; ++_First)
		_Al.destroy(_First);
}

template<class _Ty,
class _Alloc> inline
	void PT_Destroy_range(_Ty *_First, _Ty *_Last, _Alloc& _Al,
	std::_Scalar_ptr_iterator_tag)
{	// destroy [_First, _Last), scalar type (do nothing)
}
#endif
//*/
#ifdef  _MSC_VER
#pragma warning(default: 4100)

#pragma warning(pop)
//#pragma pack(pop)
#endif  /* _MSC_VER */

//#endif /* RC_INVOKED */
//#endif /* _XMEMORY_ */

/*
* Copyright (c) 1992-2005 by P.J. Plauger.  ALL RIGHTS RESERVED.
* Consult your license regarding permissions and restrictions.
*/

/*
* This file is derived from software bearing the following
* restrictions:
*
* Copyright (c) 1994
* Hewlett-Packard Company
*
* Permission to use, copy, modify, distribute and sell this
* software and its documentation for any purpose is hereby
* granted without fee, provided that the above copyright notice
* appear in all copies and that both that copyright notice and
* this permission notice appear in supporting documentation.
* Hewlett-Packard Company makes no representations about the
* suitability of this software for any purpose. It is provided
* "as is" without express or implied warranty.
V4.05:0009 */
