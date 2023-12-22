#ifndef __Fixed_Point__
#define __Fixed_Point__


#ifndef INT32

#ifdef _WIN32
typedef signed __int32      INT32;
typedef signed __int64      INT64;
typedef unsigned __int32    UINT32;
#else
#include <inttypes.h>
typedef int32_t      INT32;
typedef int64_t      INT64;
typedef unsigned int    UINT32;
#undef NULL
#define NULL 0
#endif
#endif

#define USE_FLOAT

#ifdef USE_FLOAT
	
	#define ITOX(i) ((float)(i))

	#define PM_PI			3.141593f
	#define PM_PI_OVER_2	(PM_PI*2.f)
	#define PM_HALF			(PM_PI/2.f)
	#define _EPSILON		1.0e-5f
	
//	typedef float	Fixed;
#else

//	#define ITOX(i) ((i) << 16)
//
//	#define PM_PI			205887
//	#define PM_PI_OVER_2	(PM_PI>>1)
//	#define PM_HALF			(2<<15)
//
///** \brief 
// *
// * \par Project:
// * Portable 3D Engine Render
// *
// * \par File:
// * $Id: Fixed.h,v 1.1 2009/03/13 08:54:15 jtseo Exp $
// *
// * \ingroup Math
// * 
// * \version 1.0
// *
// * \par History:
// * $Date: 2009/03/13 08:54:15 $\n
// * $Author: jtseo $\n
// * $Locker:  $
// *
// * \par Description:
// *
// * \par license:
// * Copyright (c) 2008 OJ. All Rights Reserved.
// * 
// * \todo 
// *
// * \bug 
// *
// */
//class Fixed {
//public:
//	enum {
//		EPSILON		= 0x00004,
//		PRECISION	= 16,
//		ONE			= 0x10000
//	};
//
//	Fixed(){}
//
//	Fixed(INT32 anInt, bool) {
//		m_value = anInt;
//	}
//
//#ifdef _DEBUG
//	//operator float() const{
//	//	return m_value * (1.0f/static_cast<float>(ONE));
//	//}
//
//	//Fixed(float aFloat) {
// //       m_value = static_cast<INT32>(aFloat * static_cast<float>(ONE));
// //   }
//#endif
//
//	Fixed(const Fixed& other) {
//		m_value = other.m_value;
//	}
//
//	Fixed(int anInt) {
//		m_value = anInt << PRECISION;
//	}
//
//	//operator INT32() const {
//	//	return m_value >> PRECISION;
//	//}
//
//	static Fixed FromValue(INT32 aValue) {
//		return Fixed(aValue, true);
//	}
//
//	bool operator==(const Fixed& other) const {
//		return m_value == other.m_value;
//	}
//
//	bool operator!=(const Fixed& other) const {
//		return m_value != other.m_value;
//	}
//
//	bool operator<(const Fixed& other) const {
//		return m_value < other.m_value;
//	}
//
//	bool operator>(const Fixed& other) const {
//		return m_value > other.m_value;
//	}
//
//	bool operator<=(const Fixed& other) const {
//		return m_value <= other.m_value;
//	}
//
//	bool operator>=(const Fixed& other) const {
//		return m_value >= other.m_value;
//	}
//
//	Fixed operator-() const {
//		return Fixed(-m_value, true);
//	}
//
//	Fixed operator+(const Fixed& other) const {
//		return Fixed(m_value + other.m_value, true);
//	}
//
//	Fixed operator-(const Fixed& other) const {
//		return Fixed(m_value - other.m_value, true);
//	}
//
//	Fixed operator*(const Fixed& other) const {
//		return Fixed(static_cast<INT32>(((INT64) m_value * (INT64) other.m_value)  >> PRECISION), true);
//	}
//
//	Fixed operator/(const Fixed& other) const {
//		return *this * other.inverse();
//	}
//
//	Fixed& operator=(const Fixed& other) {
//		m_value = other.m_value;
//		return *this;
//	}
//
//	Fixed& operator+=(const Fixed& other) {
//		m_value += other.m_value;
//		return *this;
//	}
//
//	Fixed& operator-=(const Fixed& other) {
//		m_value -= other.m_value;
//		return *this;
//	}
//
//	Fixed& operator*=(const Fixed& other) {
//		m_value = static_cast<INT32>(((INT64) m_value * (INT64) other.m_value)  >> PRECISION);
//		return *this;
//	}
//
//	Fixed& operator/=(const Fixed& other) {
//		return *this *= other.inverse();;
//	}
//
//	Fixed operator%(const Fixed& other) const{
//		return (*this - integer(*this / other) * other);
//	}
//
//	Fixed& operator%=(const Fixed& other){
//		*this	= (*this - integer(*this / other) * other);
//		return *this;
//	}
//
//	Fixed inverse() const{
//		return Fixed((INT32) ((((INT64)1) << 32)/(INT64)m_value), true);
//	}
//
//	INT32 Value() const {
//		return m_value;
//	}
//
////	friend inline Fixed operator -(const int&other, const Fixed &fixed){
////		return Fixed(other-fixed.m_value, true);
////	}
//#ifdef INTSUPPORT
//	bool operator==(const int& other) const {
//		return m_value == (other<<PRECISION);
//	}
//
//	bool operator!=(const int& other) const {
//		return m_value != (other<<PRECISION);
//	}
//
//	bool operator<(const int& other) const {
//		return m_value < (other<<PRECISION);
//	}
//
//	bool operator>(const int& other) const {
//		return m_value > (other<<PRECISION);
//	}
//
//	bool operator<=(const int& other) const {
//		return m_value <= (other<<PRECISION);
//	}
//
//	bool operator>=(const int& other) const {
//		return m_value >= (other<<PRECISION);
//	}
//
//	Fixed operator+(const int& other) const {
//		return Fixed(m_value + (other<<PRECISION), true);
//	}
//
//	Fixed operator-(const int& other) const {
//		return Fixed(m_value - (other<<PRECISION), true);
//	}
//
//	Fixed& operator=(const int& other) {
//		m_value = (other<<PRECISION);
//		return *this;
//	}
//
//	Fixed& operator+=(const int& other) {
//		m_value += (other<<PRECISION);
//		return *this;
//	}
//
//	Fixed& operator-=(const int& other) {
//		m_value -= (other<<PRECISION);
//		return *this;
//	}
//
//	Fixed& operator*=(const int& other) {
//		m_value = static_cast<INT32>(((INT64) m_value * (INT64) (other<<PRECISION))  >> PRECISION);
//		return *this;
//	}
//
//	Fixed operator*(const int& other) const {
//		return Fixed(static_cast<INT32>(((INT64) m_value * (INT64) (other<<PRECISION))  >> PRECISION), true);
//	}
//#endif
//
//	friend Fixed sin(const Fixed & value);
//	friend Fixed asin(const Fixed & value);
//	friend Fixed cos(const Fixed & value);
//	friend Fixed acos(const Fixed & value);
//	friend Fixed tan(const Fixed & value);
//	friend Fixed atan(const Fixed & value);
//	friend Fixed sqrt(const Fixed & value, int iDetail = 8);
//	friend Fixed invsqrt(const Fixed & value);
//	friend Fixed abs(const Fixed & value);
//	friend Fixed sgn(const Fixed & value);
//	friend Fixed fraction(const Fixed & value);
//	friend Fixed inverse(const Fixed & value);
//	friend Fixed integer(const Fixed & value);
//	
//	INT32   m_value;
//private:
//};
//
//inline Fixed abs(const Fixed & value) {
//    return value.m_value >= 0 ? value : -value;
//}
//
//inline Fixed sgn(const Fixed & value) {
//    if (value.m_value > 0) {
//        return Fixed(1);
//    } else if (value.m_value < 0) {
//        return Fixed(-1);
//    } else {
//        return Fixed(0);
//    }
//}
//
//inline Fixed fraction(const Fixed & value) {
//    return Fixed(value.m_value & ((1 << Fixed::PRECISION) - 1), true);
//}
//
//inline Fixed integer(const Fixed & value)
//{
//	if(value < (Fixed)0)
//		return (value + fraction(-value));
//	return value - fraction(value);
//}
//
//inline Fixed inverse(const Fixed & value) {
//    return value.inverse();
//}

#endif// end of USE_FLOAT

#endif