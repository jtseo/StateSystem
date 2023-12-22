// Matrix3.cpp: implementation of the PtMatrix3 class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void PtMatrix3::SetValue(	float f11, float f12, float f13, 
						float f21, float f22, float f23,
						float f31, float f32, float f33)
{
    m_f11	= f11; m_f12	= f12; m_f13	= f13;
    m_f21	= f21; m_f22	= f22; m_f23	= f23;
    m_f31	= f31; m_f32	= f32; m_f33	= f33;
}

void PtMatrix3::SetRows( const PtVector3 &vecR1 , const PtVector3 &vecR2 , const PtVector3 &vecR3)
{
	vecR1.GetValue((&m_f11));
	vecR2.GetValue((&m_f11)+3);
	vecR3.GetValue((&m_f11)+6);
}

void PtMatrix3::GetRows( PtVector3 &vecR1 , PtVector3 &vecR2 , PtVector3 &vecR3) const
{
	vecR1.SetValue((&m_f11));
	vecR2.SetValue((&m_f11)+3);
	vecR3.SetValue((&m_f11)+6);
}

void PtMatrix3::MakeNormal()
{
	PtVector3 mthV3[3];

	int i;
	for(i=0; i<3; i++)
		GetRow(i, &mthV3[i]);

	mthV3[0] = mthV3[1].Cross(mthV3[2]);
	mthV3[2] = mthV3[0].Cross(mthV3[1]);

	PtVector3	v3Normal;

	for(i=0; i<3; i++)
	{
		mthV3[i].GetNormal(&v3Normal);
		SetRow(i, v3Normal);
	}
}

PtMatrix3 PtMatrix3::GetTranspose() const
{
	PtMatrix3 matRes;

	for( int m=0 ; m<3 ; m++ )
		for( int r=0 ; r<3 ; r++ )
		{
			matRes[m*3+r] = (&m_f11)[r*3+m];
		}
	
	return matRes;
}

PtMatrix3& PtMatrix3::MakeTranspose()
{
	float fTemp;

	for( int m=0 ; m<3 ; m++ )
		for( int r=0 ; r<m ; r++ )
		{
			fTemp = (&m_f11)[m*3+r];
			(&m_f11)[m*3+r] = (&m_f11)[r*3+m];
			(&m_f11)[r*3+m] = fTemp;
		}
	
	return *this;
}

PtMatrix3 PtMatrix3::GetInverse() const
{
	PtMatrix3	matSrc(*this);
	matSrc.MakeInverse();
	return matSrc;
}
PtMatrix3& PtMatrix3::MakeInverse()
{
	PtMatrix3	matSrc(*this);
	MakeTranspose();
	return *this;
}

PtMatrix3 PtMatrix3::GetIdentity()
{
	return PtMatrix3().MakeIdentity();
}

PtMatrix3& PtMatrix3::MakeIdentity()
{
	m_f11 = 1; m_f12 = 0; m_f13 = 0;
	m_f21 = 0; m_f22 = 1; m_f23 = 0;
	m_f31 = 0; m_f32 = 0; m_f33 = 1;
	
	return *this;
}

bool PtMatrix3::operator==(const PtMatrix3 &xMatrix) const
{
	for(int i=0; i<3*3; i++)
	{
		if(*((&m_f11)+i) != xMatrix[i])
			return false;
	}
	return true;
}

PtMatrix3 PtMatrix3::operator*( const PtMatrix3& matSrc )const{
	PtMatrix3 matRes;
	for(int m=0;m<3;m++)
	{
		(&matRes.m_f11)[m*3+0] = (&m_f11)[m*3+0]*(&matRes.m_f11)[0*3+0] + (&m_f11)[m*3+1]*(&matRes.m_f11)[1*3+0] + (&m_f11)[m*3+2]*(&matRes.m_f11)[2*3+0];
		(&matRes.m_f11)[m*3+1] = (&m_f11)[m*3+0]*(&matRes.m_f11)[0*3+1] + (&m_f11)[m*3+1]*(&matRes.m_f11)[1*3+1] + (&m_f11)[m*3+2]*(&matRes.m_f11)[2*3+1];
		(&matRes.m_f11)[m*3+2] = (&m_f11)[m*3+0]*(&matRes.m_f11)[0*3+2] + (&m_f11)[m*3+1]*(&matRes.m_f11)[1*3+2] + (&m_f11)[m*3+2]*(&matRes.m_f11)[2*3+2];
	}
	return matRes;
}

PtVector3 PtMatrix3::operator*( const PtVector3& vecrOp )const{
	PtVector3 vecDest;
	vecDest.x = vecrOp.x*m_f11 + vecrOp.y*m_f21 + vecrOp.z*m_f31;
	vecDest.y = vecrOp.x*m_f12 + vecrOp.y*m_f22 + vecrOp.z*m_f32;
	vecDest.z = vecrOp.x*m_f13 + vecrOp.y*m_f23 + vecrOp.z*m_f33;
	return vecDest;
}

PtMatrix3& PtMatrix3::SetScale(const PtVector3& scale_v3)
{
	m_f11 = scale_v3.x;
	m_f22 = scale_v3.y;
	m_f33 = scale_v3.z;
	return *this;
}

PtMatrix3 &PtMatrix3::SetRotate(const PtVector3 &xv3Axi, float xpfAngle)
{
	float cc, sc, vc, a, b, c;//, d;

	a = xv3Axi[0];
	b = xv3Axi[1];
	c = xv3Axi[2];

	cc = cosf(xpfAngle);
	sc = sinf(xpfAngle);
	vc = ((float)1 - cc);
	
    m_f11 = a*a*vc+cc	; m_f12 = a*b*vc+c*sc	; m_f13 = a*c*vc-b*sc;
    m_f21 = a*b*vc-c*sc	; m_f22 = b*b*vc+cc		; m_f23 = b*c*vc+a*sc;
    m_f31 = a*c*vc+b*sc	; m_f32 = b*c*vc-a*sc	; m_f33 = c*c*vc+cc;
	
	return *this;
}
