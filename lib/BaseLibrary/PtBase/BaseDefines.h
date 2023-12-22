// Defines.h: interface for the CDefines class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEFINES_H__B3218B28_CB57_44A7_82B5_C0F9FCECED7B__INCLUDED_)
#define AFX_DEFINES_H__B3218B28_CB57_44A7_82B5_C0F9FCECED7B__INCLUDED_

/** \brief 
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: Defines.h,v 1.1 2009/03/13 07:32:56 jtseo Exp $
 *
 * \ingroup PtBase
 * 
 * \version 1.0
 *
 * \par History:
 * $Date: 2009/03/13 07:32:56 $\n
 * $Author: jtseo $\n
 * $Locker:  $
 *
 * \par Description:
 *
 * \par license:
 * Copyright (c) 2008 - 2014 OJ. All Rights Reserved.
 * 
 * \todo 
 *
 * \bug 
 *
 */
#ifdef Fixed
typedef Fixed			PT_FIXED;
#else
typedef float			PT_FIXED;
#endif
typedef unsigned char	PT_BOOL;
#ifdef INT64
typedef INT32			PT_INT;
typedef INT64			PT_INT64;
#else
typedef int				PT_INT;
typedef int				PT_INT64;
#endif
#define Vector2	PtVector2
#define Vector4	PtVector4
#define Matrix3	PtMatrix3
#define Matrix	PtMatrix4
#define Quater	PtQuater

#ifndef FALSE
#define FALSE	false
#endif
#ifndef TRUE
#define TRUE	true
#endif
#ifndef NULL
#define NULL	0
#endif

int OpenPtEngine();
int ClosePtEngine();


#endif // !defined(AFX_DEFINES_H__B3218B28_CB57_44A7_82B5_C0F9FCECED7B__INCLUDED_)
