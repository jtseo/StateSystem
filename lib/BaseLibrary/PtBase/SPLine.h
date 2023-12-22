// SPLine.h: interface for the PtSPLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLINE_H__4168368B_DD09_4EBC_BAAF_E62C165CCD1D__INCLUDED_)
#define AFX_SPLINE_H__4168368B_DD09_4EBC_BAAF_E62C165CCD1D__INCLUDED_

#include "float.h"

/** \brief 
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: SPLine.h,v 1.1 2009/03/13 08:54:15 jtseo Exp $
 *
 * \ingroup Math
 * 
 * \version 1.0
 *
 * \par History:
 * $Date: 2009/03/13 08:54:15 $\n
 * $Author: jtseo $\n
 * $Locker:  $
 *
 * \par Description:
 *
 * \par license:
 * Copyright (c) 2008 OJ. All Rights Reserved.
 * 
 * \todo 
 *
 * \bug 
 *
 */
class PtSPLine  
{
public:
	PtSPLine();
	virtual ~PtSPLine();

	static float GetBezierCurve(float *ctr, float t);
	static float GetCommonBezierCurve(float fDif, float t);
	static void Make2ExtCtrPoint(float *xpfTimes, float *xpfValues, float *xpfRets);
	static PtVector3 GetCommonBezierCurve(PtVector3 &v3From, PtVector3 &v3To, float fDif, float t);

	static PtVector3 GetBezierCurve(PtVector3* point, float t);
protected:
//	STLVfloat	m_stlVTimes;
//	STLVfloat	m_stlVValues;
};

#endif // !defined(AFX_SPLINE_H__4168368B_DD09_4EBC_BAAF_E62C165CCD1D__INCLUDED_)
