#include "stdafx.h"

//#ifdef USE_FLOAT
//
//#else
//
//#define PM_E			178145
//
///**
// * For the inverse tangent calls, all approximations are valid for |t| <= 1.
// * To compute ATAN(t) for t > 1, use ATAN(t) = PI/2 - ATAN(1/t).  For t < -1,
// * use ATAN(t) = -PI/2 - ATAN(1/t).
// */
//
//const INT32	SK1 = 498;
//const INT32	SK2 = 10882;
//
///** Computes SIN(f), f is a fixed point number in radians.
// * 0 <= f <= 2PI
// */
//Fixed sin(const Fixed &value) {
//	
//	// if in range 0 to pi: nothing needs to be done.
//	// otherwise, we need to get value into that range.
//	Fixed fIn(value);
//	Fixed xDoublePI(PM_PI, true);
//	xDoublePI.m_value <<= 1;
//
//	INT32 sign2	= 1;
//	if(fIn < (Fixed)0){
//		sign2		= -1;
//		fIn.m_value	*= -1;
//		fIn %= xDoublePI;
//	}else if(fIn > xDoublePI){
//		fIn %= xDoublePI;
//	}
//
//	INT32 sign = 1;
//	// If in range -pi/4 to pi/4: nothing needs to be done.
//	// otherwise, we need to get f into that range and account for
//	// sign change.
//	if ((fIn.m_value > PM_PI_OVER_2) && (fIn.m_value <= PM_PI)) {
//		fIn.m_value = PM_PI - fIn.m_value;
//	} else if((fIn.m_value > PM_PI) && (fIn.m_value <= (PM_PI + PM_PI_OVER_2))) {
//		fIn.m_value = fIn.m_value - PM_PI;
//		sign = -1;
//	} else if(fIn.m_value > (PM_PI + PM_PI_OVER_2)) {
//		fIn.m_value = (PM_PI<<1)-fIn.m_value;
//		sign = -1;
//	}
//
//	Fixed result;
//	Fixed sqr = fIn * fIn;
//	result.m_value	= SK1;
//	result *= sqr;
//	result.m_value -= SK2;
//	result *= sqr;
//	result.m_value += (1<<16);
//	result *= fIn;
//	result.m_value	*= sign * sign2;
//	return result;
//}
//
//const INT32	CK1 = 2328;
//const INT32	CK2 = 32551;
//
///** Computes COS(f), f is a fixed point number in radians.
// * 0 <= f <= PM_PI/2
// */
//Fixed cos(const Fixed &value) {
//	Fixed	fIn(value);
//	Fixed xDoublePI(PM_PI, true);
//	xDoublePI.m_value <<= 1;
//	
//	if(fIn < (Fixed)0){
//		fIn *= (Fixed) -1;
//		fIn %= xDoublePI;
//	}else if(fIn > xDoublePI){
//		fIn %= xDoublePI;
//	}
//
//	INT32 sign = 1;
//	if ((fIn.m_value > PM_PI_OVER_2) && (fIn.m_value <= PM_PI)) {
//		fIn.m_value = PM_PI - fIn.m_value;
//		sign = -1;
//	} else if ((fIn.m_value > PM_PI_OVER_2) && (fIn.m_value <= (PM_PI + PM_PI_OVER_2))) {
//		fIn.m_value = fIn.m_value - PM_PI;
//		sign = -1;
//	} else if (fIn.m_value > (PM_PI + PM_PI_OVER_2)) {
//		fIn.m_value = (PM_PI<<1)-fIn.m_value;
//	}
//
//	Fixed sqr = fIn*fIn;
//	Fixed result(CK1, true);
//	result *= sqr;
//	result.m_value -= CK2;
//	result *= sqr;
//	result.m_value += (1<<16);
//	result.m_value *= sign;
//	return result;
//}
//
//
//    /** Computes Tan(f), f is a fixed point number in radians.
//     * 0 <= f <= PM_PI/4
//     */
//
//const INT32 TK1 = 13323;
//const INT32 TK2 = 20810;
//
//Fixed tan(const Fixed &f) {
//
//	Fixed fpIn(f), fpPI2, xPi(PM_PI, true);
//
//	fpPI2.m_value	= xPi.m_value>>1;
//	if(fpIn > fpPI2)
//	{
//		fpIn = (fpIn - (integer((fpIn - fpPI2)/xPi)+(Fixed)1)*xPi);
//	}else if(fpIn < -fpPI2)
//	{
//		fpIn = (fpIn - (integer((fpIn + fpPI2)/xPi)-(Fixed)1)*xPi);
//	}
//
//	Fixed sqr = fpIn * fpIn;
//	Fixed result(TK1, true);
//	result *= sqr;
//	result.m_value += TK2;
//	result *= sqr;
//	result.m_value += (1<<16);
//	result *= fpIn;
//	return result;
//}
//
//
//    /** Computes ArcTan(f), f is a fixed point number
//     * |f| <= 1
//     * <p>
//     * For the inverse tangent calls, all approximations are valid for |t| <= 1.
//     * To compute ATAN(t) for t > 1, use ATAN(t) = PM_PI/2 - ATAN(1/t).  For t < -1,
//     * use ATAN(t) = -PM_PI/2 - ATAN(1/t).
//     */
//
//Fixed atan (const Fixed &f) {
//
//	Fixed sqr = f*f;
//	Fixed result(1365, true);
//	result *= sqr;
//	result.m_value -= 5579;
//	result *= sqr;
//	result.m_value += 11805;
//	result *= sqr;
//	result.m_value -= 21646;
//	result *= sqr;
//	result.m_value += 65527;
//	result *= f;
//	return result;
//}
//
//const INT32 AS1 = -1228;
//const INT32 AS2 = 4866;
//const INT32 AS3 = 13901;
//const INT32 AS4 = 102939;
//
//    /** Compute ArcSin(f), 0 <= f <= 1
//     */
//
//Fixed asin (const Fixed &f) {
//	Fixed fRoot, fpIn;
//	fpIn	= f;
//	INT32	sign;
//	if(f < (Fixed)0)
//		fpIn	= -f, sign	= -1;
//	else
//		fpIn	= f, sign	= 1;
//
//	fRoot	= (Fixed)1 - fpIn;
//	fRoot	= sqrt(fRoot, 6);
//	Fixed result(AS1, true);
//	result *= fpIn;
//	result.m_value += AS2;
//	result *= fpIn;
//	result.m_value -= AS3;
//	result *= fpIn;
//	result.m_value += AS4;
//	result.m_value = PM_PI_OVER_2 - (fRoot * result).m_value;
//	result.m_value	*= sign;
//	return result;
//}
//
//
//    /** Compute ArcCos(f), 0 <= f <= 1
//     */
//
//Fixed acos(const Fixed &f) {
//	Fixed fRoot, fpIn;
//
//	fpIn	= f;
//	INT32	sign	= 1;
//	if(fpIn < (Fixed)0)
//		sign	= -1, fpIn	= -fpIn;
//
//	fRoot	= (Fixed)1 - fpIn;
//	fRoot	= sqrt(fRoot, 6);
//	Fixed result(AS1, true);
//	result *= fpIn;
//	result.m_value += AS2;
//	result *= fpIn;
//	result.m_value -= AS3;
//	result *= fpIn;
//	result.m_value += AS4;
//	result = fRoot * result;
//
//	if(sign < 0)
//		result.m_value	= PM_PI - result.m_value;
//	return result;
//}
//
//Fixed sqrt(const Fixed &value, int iDetail){
//	Fixed s;
//	s.m_value = (value.m_value + 65536) >> 1;
//	for (int i = 0; i < iDetail; i++) {
//		//converge six times
//		s.m_value = (s.m_value + (value/s).m_value) >> 1;
//	}
//	return s;
//}
//#endif