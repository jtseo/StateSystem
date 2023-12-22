// Matrix4.h: interface for the PtMatrix4 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIX4_H__BF0BFD99_5FC0_440C_954A_6BEABE87823C__INCLUDED_)
#define AFX_MATRIX4_H__BF0BFD99_5FC0_440C_954A_6BEABE87823C__INCLUDED_


/** \brief 
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: Matrix4.h,v 1.1 2009/03/13 08:54:15 jtseo Exp $
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
class PtMatrix4
{
public:
	float  m_f11, m_f12, m_f13,
		m_f21, m_f22, m_f23,
		m_f31, m_f32, m_f33,
		m_f41, m_f42, m_f43;

	PtMatrix4( float f11, float f12, float f13,
					 float f21, float f22, float f23,
					 float f31, float f32, float f33,
					 float f41, float f42, float f43){
		SetValue(f11, f12, f13,
					 f21, f22, f23,
					 f31, f32, f33,
					 f41, f42, f43);
	}

	void Get4x4Matrix(float *_pMat)
	{
		memcpy(_pMat, (&m_f11), sizeof(float)*3); *(_pMat+3)	= 0;
		memcpy((_pMat+4*1), (&m_f21), sizeof(float)*3); *(_pMat+4*1+3)	= 0;
		memcpy((_pMat+4*2), (&m_f31), sizeof(float)*3); *(_pMat+4*2+3)	= 0;
		memcpy((_pMat+4*3), (&m_f41), sizeof(float)*3); *(_pMat+4*3+3)	= 1;
	}

	void Set4x4Matrix(float *_pMat)
	{
		memcpy((&m_f11), _pMat, sizeof(float)*3);
		memcpy((&m_f21), (_pMat+4*1), sizeof(float)*3);
		memcpy((&m_f31), (_pMat+4*2), sizeof(float)*3);
		memcpy((&m_f41), (_pMat+4*3), sizeof(float)*3);
	}

	PtMatrix4(){}
    PtMatrix4( const PtMatrix4 &mat ){
		memcpy((&m_f11), &mat.m_f11, sizeof(float)*3*4);
	}
	~PtMatrix4(){};

	PtMatrix4& SetValue( const PtMatrix4& matSrc ){
		memcpy((&m_f11), &matSrc.m_f11, sizeof(float)*3*4);
		return *this;
	}
	PtMatrix4& SetValue( const float* pfData ){
		memcpy((&m_f11), pfData, sizeof(float)*3*4);
		return *this;
	}
	void GetValue(float *pfData){
		memcpy(pfData, (&m_f11), sizeof(float)*3*4);
	}

	void SetValue(	float f11, float f12, float f13,
							float f21, float f22, float f23,
							float f31, float f32, float f33,
							float f41, float f42, float f43);
	
	
	void SetRow(int iRow, const PtVector3 &vecRow){
		vecRow.GetValue((&m_f11)+iRow*3);
	}
	PtVector3 &GetRow( int iRow, PtVector3 *xpv3) const{
		xpv3->SetValue((&m_f11)+iRow*3);
		return *xpv3;
	}
	void SetRows( const PtVector3 &vecR1 , const PtVector3 &vecR2 , const PtVector3 &vecR3 , const PtVector3 &vecR4 );
	void GetRows( PtVector3 &vecR1 , PtVector3 &vecR2 , PtVector3 &vecR3 , PtVector3 &vecR4 ) const;
	
	void MakeNormal();
	
	PtMatrix4 &GetInverse(PtMatrix4 *xpm4) const;
	PtMatrix4& MakeInverse();
	
	static PtMatrix4 GetIdentity();
	PtMatrix4& MakeIdentity();
	
	PtMatrix4& operator=(const PtMatrix4& matSrc){
		SetValue(matSrc);
		return *this;
	}
	PtMatrix4& operator*=(const PtMatrix4& matSrc){
		*this	= *this * matSrc;
		return *this;
	}
	
	float *operator[](int i){
		return (&m_f11 + i*3);
	}
	const float *operator[](int i) const{
		return (&m_f11 + i*3);
	}
	
	PtMatrix4 operator*(const PtMatrix4& matSrc)const;
	PtVector3 operator*(const PtVector3& vecrOp)const;

	friend PtVector3 operator*(const PtVector3& vecrOp, const PtMatrix4 &mat){
		return (mat*vecrOp);
	}
	friend PtVector3 &operator*=(PtVector3& vecrOp, const PtMatrix4 &mat){
		vecrOp	= vecrOp * mat;
		return vecrOp;
	}
	
	bool	operator==(const PtMatrix4 &xMatrix) const;
	bool	operator!=(const PtMatrix4 &xMatrix) const{ return !(*this==xMatrix); }

	float GetRotate(int _axi) const;
	
	PtMatrix4 &SetRotate(const PtVector3 &xv3Axi, float xpfAngle){
		((PtMatrix3*)this)->SetRotate(xv3Axi, xpfAngle);
		return *this;
	}

	PtMatrix4& SetScale(const PtVector3& _scale_v3) {
		((PtMatrix3*)this)->SetScale(_scale_v3);
		return *this;
	}
};

#endif // !defined(AFX_MATRIX4_H__BF0BFD99_5FC0_440C_954A_6BEABE87823C__INCLUDED_)
