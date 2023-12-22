// Matrix4.cpp: implementation of the PtMatrix4 class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void PtMatrix4::SetValue(	float f11, float f12, float f13,
						float f21, float f22, float f23,
						float f31, float f32, float f33,
						float f41, float f42, float f43)
{
    m_f11 = f11; m_f12 = f12; m_f13 = f13;
    m_f21 = f21; m_f22 = f22; m_f23 = f23;
    m_f31 = f31; m_f32 = f32; m_f33 = f33;
    m_f41 = f41; m_f42 = f42; m_f43 = f43;
}

void PtMatrix4::SetRows( const PtVector3 &vecR1 , const PtVector3 &vecR2 , const PtVector3 &vecR3 , const PtVector3 &vecR4 )
{
	vecR1.GetValue((&m_f11));
	vecR2.GetValue((&m_f11)+3);
	vecR3.GetValue((&m_f11)+6);
	vecR4.GetValue((&m_f11)+9);
}

void PtMatrix4::GetRows( PtVector3 &vecR1 , PtVector3 &vecR2 , PtVector3 &vecR3 , PtVector3 &vecR4 ) const
{
	vecR1.SetValue((&m_f11));
	vecR2.SetValue((&m_f11)+3);
	vecR3.SetValue((&m_f11)+6);
	vecR4.SetValue((&m_f11)+9);
}

void PtMatrix4::MakeNormal()
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

PtMatrix4 &PtMatrix4::GetInverse(PtMatrix4 *xpm4) const
{
	xpm4->SetValue(*this);
	xpm4->MakeInverse();
	return *xpm4;
}
PtMatrix4& PtMatrix4::MakeInverse()
{
	PtMatrix4	matSrc(*this);

	float fTemp;
	for( int m=0 ; m<3 ; m++ )
		for( int r=0 ; r<m ; r++ )
		{
			fTemp = (&m_f11)[m*3+r];
			(&m_f11)[m*3+r] = (&m_f11)[r*3+m];
			(&m_f11)[r*3+m] = fTemp;
		}

	(&m_f11)[3*3+0] = -((&m_f11)[0*3+0]*matSrc[3][0] + (&m_f11)[1*3+0]*matSrc[3][1] + (&m_f11)[2*3+0]*matSrc[3][2]);
	(&m_f11)[3*3+1] = -((&m_f11)[0*3+1]*matSrc[3][0] + (&m_f11)[1*3+1]*matSrc[3][1] + (&m_f11)[2*3+1]*matSrc[3][2]);
	(&m_f11)[3*3+2] = -((&m_f11)[0*3+2]*matSrc[3][0] + (&m_f11)[1*3+2]*matSrc[3][1] + (&m_f11)[2*3+2]*matSrc[3][2]);
	return *this;
}

PtMatrix4 PtMatrix4::GetIdentity()
{
	return PtMatrix4().MakeIdentity();
}

PtMatrix4& PtMatrix4::MakeIdentity()
{
	m_f11 = 1; m_f12 = 0; m_f13 = 0;
	m_f21 = 0; m_f22 = 1; m_f23 = 0;
	m_f31 = 0; m_f32 = 0; m_f33 = 1;
	m_f41 = 0; m_f42 = 0; m_f43 = 0;
	return *this;
}

float PtMatrix4::GetRotate(int _axi) const
{
	PtMatrix4 m4;
	m4.MakeIdentity();

	int t = _axi - 1;
	if (t < 0)
		t = 2;
	PtVector3 v3, iv3, up_v3;
	GetRow(t, &v3);
	m4.GetRow(t, &iv3);
	m4.GetRow(_axi, &up_v3);
	v3[_axi] = 0;

	if (v3.DoubleLength() == 0)
		return 0;

	v3.MakeNormal();
	return v3.angle_inter(iv3, up_v3);
}

PtMatrix4 PtMatrix4::operator*(const PtMatrix4& matSrc) const{
	PtMatrix4 matRes;
	int m;
	for(m=0;m<3;m++)
	{
		matRes[m][0] = (&m_f11)[m*3+0]*matSrc[0][0] + (&m_f11)[m*3+1]*matSrc[1][0] + (&m_f11)[m*3+2]*matSrc[2][0];
		matRes[m][1] = (&m_f11)[m*3+0]*matSrc[0][1] + (&m_f11)[m*3+1]*matSrc[1][1] + (&m_f11)[m*3+2]*matSrc[2][1];
		matRes[m][2] = (&m_f11)[m*3+0]*matSrc[0][2] + (&m_f11)[m*3+1]*matSrc[1][2] + (&m_f11)[m*3+2]*matSrc[2][2];
	}
	matRes[3][0] = (&m_f11)[3*3+0]*matSrc[0][0] + (&m_f11)[m*3+1]*matSrc[1][0] + (&m_f11)[3*3+2]*matSrc[2][0] + matSrc[3][0];
	matRes[3][1] = (&m_f11)[3*3+0]*matSrc[0][1] + (&m_f11)[m*3+1]*matSrc[1][1] + (&m_f11)[3*3+2]*matSrc[2][1] + matSrc[3][1];
	matRes[3][2] = (&m_f11)[3*3+0]*matSrc[0][2] + (&m_f11)[m*3+1]*matSrc[1][2] + (&m_f11)[3*3+2]*matSrc[2][2] + matSrc[3][2];
	return matRes;
}

PtVector3 PtMatrix4::operator*(const PtVector3& vecrOp)const
{
	PtVector3 vecSrc;
	
	vecSrc.x = vecrOp.x*m_f11 + vecrOp.y*m_f21 + vecrOp.z*m_f31 + m_f41;
	vecSrc.y = vecrOp.x*m_f12 + vecrOp.y*m_f22 + vecrOp.z*m_f32 + m_f42;
	vecSrc.z = vecrOp.x*m_f13 + vecrOp.y*m_f23 + vecrOp.z*m_f33 + m_f43;
	
	return vecSrc;
}

bool PtMatrix4::operator==(const PtMatrix4 &xMatrix) const{

	if (memcmp(&m_f11, xMatrix[0], sizeof(PtMatrix4)))
		return false;
		
	return true;
}
