// Matrix3.h: interface for the PtMatrix3 class.
//
//////////////////////////////////////////////////////////////////////

//#if !defined(AFX_MATRIX3_H__BD0E6397_0713_4CF4_BC3C_E948FCE64D87__INCLUDED_)
//#define AFX_MATRIX3_H__BD0E6397_0713_4CF4_BC3C_E948FCE64D87__INCLUDED_


/** \brief 
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: Matrix3.h,v 1.1 2009/03/13 08:54:15 jtseo Exp $
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
class PtMatrix3  
{
public:
	float  m_f11, m_f12, m_f13,
		m_f21, m_f22, m_f23,
		m_f31, m_f32, m_f33;

	PtMatrix3( float f11, float f12, float f13,
					 float f21, float f22, float f23,
					 float f31, float f32, float f33){
		SetValue(f11, f12, f13, 
			f21, f22, f23,
			f31, f32, f33);
	}
	PtMatrix3(){}
    PtMatrix3( const PtMatrix3 &mat ){
		SetValue(mat);
    }

	~PtMatrix3(){};

	PtMatrix3 &SetRotate(const PtVector3 &xv3Axi, float xpfAngle);
	PtMatrix3& SetScale(const PtVector3& scale_v3);

	PtMatrix3 &SetValue( const PtMatrix3& matSrc ){
		memcpy(&m_f11, &matSrc.m_f11, sizeof(float)*3*3);
		return *this;
	}
	PtMatrix3& SetValue( const float* pfData ){
		memcpy(&m_f11, pfData, sizeof(float)*3*3);
        return *this;
	}
	void GetValue(float *pfData)const{
		memcpy(pfData, &m_f11, sizeof(float)*3*3);
	}

	void SetValue(	float f11, float f12, float f13, 
							float f21, float f22, float f23,
							float f31, float f32, float f33);
	
	void SetRow( int iRow, const PtVector3 &vecRow ){
		vecRow.GetValue(&m_f11+iRow*3);
	}
	PtVector3 &GetRow( int iRow, PtVector3 *xpv3 ) const{
		xpv3->SetValue(&m_f11+iRow*3);
		return *xpv3;
	}
	
	void SetRows( const PtVector3 &vecR1 , const PtVector3 &vecR2 , const PtVector3 &vecR3);
	void GetRows( PtVector3 &vecR1 , PtVector3 &vecR2 , PtVector3 &vecR3) const;
	
	void MakeNormal();
	
	PtMatrix3 GetTranspose() const;
	PtMatrix3& MakeTranspose() ;
	
	PtMatrix3 GetInverse() const;
	PtMatrix3& MakeInverse();
	
	static PtMatrix3 GetIdentity();
	PtMatrix3& MakeIdentity();
	
	
	float &operator[](int i){
		return *(&m_f11+i);
	}
	const float &operator[](int i) const{
		return *(&m_f11+i);
	}
	
	PtMatrix3& operator=( const PtMatrix3& matSrc ){
		SetValue(matSrc);
		return *this;
	}
	PtMatrix3& operator*=( const PtMatrix3& matSrc ){
		*this	= *this * matSrc;
		return *this;
	}
	
	PtMatrix3 operator*( const PtMatrix3& matSrc ) const;
	PtVector3 operator*( const PtVector3& vecrOp ) const;
	friend PtVector3 operator*( const PtVector3& veclOp , const PtMatrix3& matrOp ){
		return (matrOp * veclOp);
	}
	
	bool	operator==(const PtMatrix3 &xMatrix) const;
	bool	operator!=(const PtMatrix3 &xMatrix) const{ return !(*this==xMatrix); }
};

//#endif // !defined(AFX_MATRIX3_H__BD0E6397_0713_4CF4_BC3C_E948FCE64D87__INCLUDED_)
