// Quert.h: interface for the PtQuater class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUERT_H__48E3BFEC_3390_4E6A_8A75_B18F5E97B4A5__INCLUDED_)
#define AFX_QUERT_H__48E3BFEC_3390_4E6A_8A75_B18F5E97B4A5__INCLUDED_


/** \brief 
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: Quater.h,v 1.1 2009/03/13 08:54:15 jtseo Exp $
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
class PtQuater
{
public:
	float x, y, z, w;

	PtQuater(){}
	PtQuater( const PtQuater& quaSrc ){
		*this	= quaSrc;
	}
	PtQuater( float q0, float q1, float q2, float q3 ){
		SetValue(q0, q1, q2, q3);
	}
	PtQuater ( const PtMatrix3& matSrc ){
		SetMatrix(matSrc);
	}
	PtQuater( const PtVector3 &vecAxis, float fRadian ){
		SetRotate(vecAxis, fRadian);
	}
	
	PtQuater GetConjugate() const{
		return PtQuater( *this ).MakeConjugate();
	}
	PtQuater& MakeConjugate(){
        x = -x;	y = -y;	z = -z;
		return *this;
	}
	PtQuater& MakeNormal();
	static PtQuater GetIdentity();

	PtQuater&	MakeIdentity(){
		return SetValue(0, 1, 0, 0);
	}
	
	void GetValue( float *q0 , float *q1 , float *q2 , float *q3 ) const{
        *q0	= x; *q1	= y; *q2	= z; *q3	= w;
	}
	PtQuater&	SetValue( float q0, float q1, float q2, float q3 ){
        x	= q0; y	= q1; z	= q2; w	= q3;
		return *this;
	}
	PtQuater&	SetRotate( const PtVector3 &vecAxis, float fRotateAng );
	void GetRotate( PtVector3 &vecAxis, float &fRotateAng ) const;
	PtQuater& SetMatrix( const PtMatrix3& mat );
	void GetMatrix(PtMatrix3 *pMat ) const;
	float Dot( const PtQuater& quatSrc ) const;
	
	float& operator[](int i){
		return *((&x)+i);
	}
	const float& operator[]( int i ) const{
		return *((&x)+i);
	}

	static PtQuater Slerp(const PtQuater &rot0, const PtQuater &rot1, float t );

	PtQuater& operator=( const PtQuater& quaSrc ){
		memcpy(&x, &quaSrc.x, sizeof(float)*4);
		return *this;
	}
	PtQuater& operator=( const PtMatrix3& matSrc ){
		return SetMatrix(matSrc);
	}
	
	bool operator==( const PtQuater& quaSrc ) const;
	bool operator!=( const PtQuater& quaSrc ) const{
		return !(*this == quaSrc);
	}
	
	PtQuater operator+( const PtQuater& ) const;
	PtQuater operator-( const PtQuater& ) const;
	PtQuater operator*( const PtQuater& ) const;
	PtQuater operator/( const PtQuater& ) const;
	
	PtQuater& operator+=( const PtQuater& );
	PtQuater& operator-=( const PtQuater& );
	PtQuater& operator*=( const PtQuater& );
	PtQuater& operator/=( const PtQuater& );
	
	friend PtQuater operator*( const PtQuater& , float );
	friend PtQuater operator*( float , const PtQuater& );
	
	// directly apply to Quaternion rotation information to Vector
//	PtVector3 RotateVector( const PtVector3& vecSrc )const;
//	friend PtVector3 &operator*=(PtVector3 &xv3, const PtQuater &xqt){
//		xv3	= xqt.RotateVector(xv3);
//		return xv3;
//	}
};

#endif // !defined(AFX_QUERT_H__48E3BFEC_3390_4E6A_8A75_B18F5E97B4A5__INCLUDED_)
