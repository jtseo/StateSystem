// Quert.cpp: implementation of the PtQuater class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "Quater.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PtQuater& PtQuater::MakeNormal()
{
	float mag = w*w + x*x + y*y + z*z;
//#ifndef USE_FLOAT
//	if(mag.m_value <= float::_EPSILON) 
//#else
	if(mag <= _EPSILON) 
//#endif
	{
		MakeIdentity();
		return *this;
	}
	
	w/=mag;
	x/=mag;
	y/=mag;
	z/=mag;
	
	return *this;
}

PtQuater PtQuater::GetIdentity()
{
	return PtQuater((float)0, (float)1, (float)0, (float)0);
}


PtQuater&	PtQuater::SetRotate( const PtVector3 &vecAxis, float fRotateAng )
{
	PtVector3 vecRot(vecAxis);
	
//#ifndef USE_FLOAT
//	if(vecRot.Length().m_value < float::_EPSILON) vecRot = PtVector3(0,1,0);
//#else
	if(vecRot.Length() < _EPSILON) vecRot = PtVector3(0,1,0);
//#endif
	
	vecRot.MakeNormal();
	
	fRotateAng/=(float)2;
	
	w=cosf(fRotateAng);
	
	float sinVel=sinf(fRotateAng);
	
	x=vecRot.x*sinVel;
	y=vecRot.y*sinVel;
	z=vecRot.z*sinVel;
	
	return(*this);
}

void PtQuater::GetRotate( PtVector3 &vecAxis, float &fRotateAng ) const
{
	fRotateAng=acosf(w);
	
	float sinVel=sinf(fRotateAng);
	
	fRotateAng *= (float)2;
	
//#ifndef USE_FLOAT
//	if( abs( sinVel ).m_value < float::_EPSILON ) 
//#else
	if( sinVel*sinVel < _EPSILON ) 
//#endif
		vecAxis = PtVector3( 0 , 0 , 0 );
	else 
		vecAxis.SetValue( x/sinVel , y/sinVel , z/sinVel );
}

PtQuater& PtQuater::SetMatrix( const PtMatrix3& mat )
{
//#ifndef USE_FLOAT
//	float fpHalf(PM_HALF, true);
//#else
	float fpHalf(PM_HALF);
//#endif
	float tr,s;
	int i,j,k;
	int nxt[3] = { 1, 2, 0};
	
	tr = mat[0*3+0]+mat[1*3+1]+mat[2*3+2];
	if(tr > (float)0){
		s = sqrtf((float)(tr + (float)1));
		(&x)[3] = s*fpHalf;
		s = fpHalf/s;

		(&x)[0] = (mat[1*3+2] - mat[2*3+1])*s;
		(&x)[1] = (mat[2*3+0] - mat[0*3+2])*s;
		(&x)[2] = (mat[0*3+1] - mat[1*3+0])*s;
	}else{
		i = 0;
		if(mat[1*3+1] > mat[0*3+0]) i = 1;
		if(mat[2*3+2] > mat[i*3+i]) i = 2;
        j = nxt[i]; k = nxt[j];

		s = sqrtf((float)(mat[i*3+i] - ( mat[j*3+j] + mat[k*3+k])) + (float)1);

		(&x)[i] = s*fpHalf;
		s = fpHalf/s;
		(&x)[3] = (mat[j*3+k] - mat[k*3+j])*s;
		(&x)[j] = (mat[i*3+j] + mat[j*3+i])*s;
		(&x)[k] = (mat[i*3+k] + mat[k*3+i])*s;
	}

	return(*this);
}

void PtQuater::GetMatrix(PtMatrix3 *pMat ) const
{
	float s,xs,ys,zs,wx,wy,wz,xx,xy,xz,yy,yz,zz;

	s = (float)2/(x*x+y*y+z*z+w*w);

	xs = x*s;	ys = y*s;	zs = z*s;
	wx = w*xs;	wy = w*ys;	wz = w*zs;
	xx = x*xs;	xy = x*ys;	xz = x*zs;
	yy = y*ys;	yz = y*zs;	zz = z*zs;

	(*pMat)[0*3+0] = (float)1 - (yy + zz);
	(*pMat)[0*3+1] = xy + wz;
	(*pMat)[0*3+2] = xz - wy;

	(*pMat)[1*3+0] = xy - wz;
	(*pMat)[1*3+1] = (float)1 - (xx + zz);
	(*pMat)[1*3+2] = yz + wx;

	(*pMat)[2*3+0] = xz + wy;
	(*pMat)[2*3+1] = yz - wx;
	(*pMat)[2*3+2] = (float)1 - (xx + yy);
}

void SlerpInternal(const PtQuater& p, const PtQuater& q, float t, PtQuater &qt)
{
	float	to1[4];
	float	omega, cosom, sinom, scale0, scale1;

	// calc cosine
	//cosom = p[0] * q[0] + p[1] * q[1] + p[2] * q[2]
	//		   + p[3] * q[3];
	cosom = p.Dot( q );

	// adjust signs (if necessary)
	if ( cosom < (float)0 ){ cosom = -cosom; to1[0] = - q[0];
		to1[1] = - q[1];
		to1[2] = - q[2];
		to1[3] = - q[3];
	} else  {
		to1[0] = q[0];
		to1[1] = q[1];
		to1[2] = q[2];
		to1[3] = q[3];
	}

	// calculate coefficients

//#ifndef USE_FLOAT
//	if ( ((float)1 - cosom).m_value > float::_EPSILON) {
//#else
	if ( ((float)1 - cosom) > _EPSILON) {
//#endif
		// standard case (slerp)
		omega = acosf(cosom);
		sinom = sinf(omega);
		scale0 = sinf(((float)1 - t) * omega) / sinom;
		scale1 = sinf(t * omega) / sinom;

	} else {        
	// "p" and "q" quaternions are very close 
	//  ... so we can do a linear interpolation
		scale0 = (float)1 - t;
		scale1 = t;
	}
	// calculate final values
	qt[0] = scale0 * p[0] + scale1 * to1[0];
	qt[1] = scale0 * p[1] + scale1 * to1[1];
	qt[2] = scale0 * p[2] + scale1 * to1[2];
	qt[3] = scale0 * p[3] + scale1 * to1[3];
}

PtQuater PtQuater::Slerp(const PtQuater &rot0, const PtQuater &rot1, float t )
{
	PtQuater a(rot1), r;

	if((rot0-rot1).Dot(rot0-rot1) > (rot0+rot1).Dot(rot0+rot1))
		a = a * (float)-1;
	
	SlerpInternal( rot0 , a , t , r);
	
	return r;
}

bool PtQuater::operator==( const PtQuater& quaSrc ) const
{
//#ifndef USE_FLOAT
//	if( abs( x - quaSrc.x ).m_value <= float::_EPSILON && 
//		abs( y - quaSrc.y ).m_value <= float::_EPSILON && 
//		abs( z - quaSrc.z ).m_value <= float::_EPSILON &&
//		abs( w - quaSrc.w ).m_value <= float::_EPSILON
//#else
	if( ( x - quaSrc.x )*( x - quaSrc.x ) <= _EPSILON*_EPSILON &&
		( y - quaSrc.y )*( y - quaSrc.y ) <= _EPSILON*_EPSILON &&
		( z - quaSrc.z )*( z - quaSrc.z ) <= _EPSILON*_EPSILON &&
		( w - quaSrc.w )*( w - quaSrc.w ) <= _EPSILON*_EPSILON
//#endif
		) return true;
	return false;
}

float PtQuater::Dot( const PtQuater& quatSrc ) const
{
	return (x*quatSrc.x+y*quatSrc.y+z*quatSrc.z+w*quatSrc.w);
}

PtQuater PtQuater::operator+( const PtQuater& quatSrc ) const
{
	PtQuater quatTem;
	
	quatTem.x = x + quatSrc.x;
	quatTem.y = y + quatSrc.y;
	quatTem.z = z + quatSrc.z;
	quatTem.w = w + quatSrc.w;

	return quatTem;
}

PtQuater PtQuater::operator-( const PtQuater& quatSrc ) const
{
	PtQuater quatTem;
	
	quatTem.x = x - quatSrc.x;
	quatTem.y = y - quatSrc.y;
	quatTem.z = z - quatSrc.z;
	quatTem.w = w - quatSrc.w;

	return quatTem;
}

PtQuater PtQuater::operator*( const PtQuater& quatSrc ) const
{
	PtQuater quatTem;
	
	quatTem.x = x*quatSrc.x;
	quatTem.y = y*quatSrc.y;
	quatTem.z = z*quatSrc.z;
	quatTem.w = w*quatSrc.w;

	return quatTem;
}

PtQuater& PtQuater::operator*=( const PtQuater& quatSrc )
{
	x *= quatSrc.x;
	y *= quatSrc.y;
	z *= quatSrc.z;
	w *= quatSrc.w;
	
	return *this;
}

PtQuater operator*( const PtQuater& qualOp , float frOp )
{
	return PtQuater( qualOp.x*frOp , qualOp.y*frOp , qualOp.z*frOp , qualOp.w*frOp );
}

PtQuater operator*( float flOp , const PtQuater& quarOp )
{
	return PtQuater( quarOp.x*flOp , quarOp.y*flOp , quarOp.z*flOp , quarOp.w*flOp );
}

//
//PtVector3 PtQuater::RotateVector( const PtVector3& vecSrc )const
//{
//	PtQuater quaRes( *this );
//	
//	quaRes.MakeConjugate();
//	
//	quaRes *= PtQuater( vecSrc.x , vecSrc.y , vecSrc.z , 0 )*(*this);
//	
//	return PtVector3( quaRes.x , quaRes.y , quaRes.z );
//}
