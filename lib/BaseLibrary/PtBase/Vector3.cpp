// Vector3.cpp: implementation of the PtVector3 class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef USE_FLOAT
#include <math.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PtVector3::PtVector3(float x, float y, float z)
{
	this->x	= x;
	this->y	= y;
	this->z	= z;
}

void PtVector3::Cross(const PtVector3& vec, PtVector3 *xpV3Ret)
{
	xpV3Ret->x = y*vec.z-z*vec.y;
	xpV3Ret->y = z*vec.x-x*vec.z;
	xpV3Ret->z = x*vec.y-y*vec.x;
}

PtVector3 PtVector3::Cross( const PtVector3& vec ) const
{
	PtVector3 vecTem;
	vecTem.x = y*vec.z-z*vec.y;
	vecTem.y = z*vec.x-x*vec.z;
	vecTem.z = x*vec.y-y*vec.x;
	return vecTem;
}

float PtVector3::Dot( const PtVector3& vec ) const
{
	return (x*vec.x+y*vec.y+z*vec.z);
}

float PtVector3::Length() const
{
	return sqrtf( x*x + y*y + z*z );
}

PtVector3& PtVector3::SetColor(UINT32 _nColor)
{
	z	= (float)(_nColor & 0xFF);
	_nColor	>>= 8;
	y	= (float)(_nColor & 0xFF);
	_nColor	>>= 8;
	x	= (float)(_nColor & 0xFF);
	return *this;
}

UINT32 PtVector3::GetColor()
{
	UINT32 nRet;
	nRet	= 0xFF;
	nRet	<<= 8;
	nRet	|= (UINT32)x;
	nRet	<<= 8;
	nRet	|= (UINT32)y;
	nRet	<<= 8;
	nRet	|= (UINT32)z;

	return nRet;
}

PtVector3 &PtVector3::GetNormal(PtVector3 *xpv3Ret, float *pfpLength) const
{
	float	invLen;

	invLen = Length();

	if(pfpLength)
		*pfpLength	= invLen;

	if(invLen == (float)0) 
	{
		xpv3Ret->SetInit();
		return *xpv3Ret;
	}
//#ifndef USE_FLOAT
//	invLen	= invLen.inverse();
//#else
	invLen	= 1.f/invLen;
//#endif
	
	xpv3Ret->x = x * invLen;
	xpv3Ret->y = y * invLen;
	xpv3Ret->z = z * invLen;

	return *xpv3Ret;
}

PtVector3 &PtVector3::MakeNormal()
{
	return GetNormal(this);
}

float PtVector3::MakeNormal(float *xpfLength)
{
	GetNormal(this, xpfLength);
	return *xpfLength;
}

PtVector3& PtVector3::SetValue(const char* _v3_str)
{
	sscanf_s(_v3_str, "%f,%f,%f", &x, &y, &z);
	return *this;
}

PtVector3& PtVector3::SetValue( float fX, float fY, float fZ )
{
	x	= fX;
	y	= fY;
	z	= fZ;
	return *this;
}

float& PtVector2::operator[]( int iIndex )
{
	return (&x)[iIndex];
}

const float& PtVector2::operator[]( int iIndex )const
{
	return (&x)[iIndex];
}

bool AlmostEqual(float _a, float _b, float _epsilon)
{
	if (_a >= _b - _epsilon && _a <= _b + _epsilon)
		return true;
	return false;
}

bool PtVector3::closer(const PtVector3& vec, float _dist_f) const
{
	if ( AlmostEqual(x, vec.x, _dist_f) &&
		AlmostEqual(y, vec.y, _dist_f)  &&
		AlmostEqual(z, vec.z, _dist_f) ) return true;
	return false;
}

bool PtVector3::AllmostEqual(const PtVector3 &vec) const
{
//#ifndef USE_FLOAT
//	if(this->x.m_value >= vec.x.m_value - float::_EPSILON && this->x.m_value <= vec.x.m_value + float::_EPSILON &&
//		this->y.m_value >= vec.y.m_value - float::_EPSILON && this->y.m_value <= vec.y.m_value + float::_EPSILON &&
//		this->z.m_value >= vec.z.m_value - float::_EPSILON && this->z.m_value <= vec.z.m_value + float::_EPSILON) return true;
//#else
	if(AlmostEqual(x, vec.x)  &&
		AlmostEqual(y, vec.y) &&
		AlmostEqual(z, vec.z)) return true;
//#endif
	return false;
}

bool PtVector3::operator ==( const PtVector3 &vec ) const
{
	if(this->x == vec.x && 
		this->y == vec.y && this->z == vec.z) return true;
	else return false;
}
PtVector3 PtVector3::operator-() const
{
	return PtVector3(-x, -y, -z);
}
PtVector3& PtVector3::operator=( const PtVector3& vecSrc )
{
	x	= vecSrc.x;
	y	= vecSrc.y;
	z	= vecSrc.z;
	return *this;
}

//PtVector3& PtVector3::operator *=( const CMatrix3& mat )
//{
//	PtVector3 vecSrc( *this );
//	
//	x = vecSrc.x*mat.m_f11 + vecSrc.y*mat.m_f21 + vecSrc.z*mat.m_f31;
//	y = vecSrc.x*mat.m_f12 + vecSrc.y*mat.m_f22 + vecSrc.z*mat.m_f32;
//	z = vecSrc.x*mat.m_f13 + vecSrc.y*mat.m_f23 + vecSrc.z*mat.m_f33;
//	return *this;
//}
//
//PtVector3& PtVector3::operator *=( const CMatrix4& mat )
//{
//	PtVector3 vecSrc( *this );
//	
//	x = vecSrc.x*mat.m_f11 + vecSrc.y*mat.m_f21 + vecSrc.z*mat.m_f31 + (float)1*mat.m_f41;
//	y = vecSrc.x*mat.m_f12 + vecSrc.y*mat.m_f22 + vecSrc.z*mat.m_f32 + (float)1*mat.m_f42;
//	z = vecSrc.x*mat.m_f13 + vecSrc.y*mat.m_f23 + vecSrc.z*mat.m_f33 + (float)1*mat.m_f43;
//	return *this;
//}

PtVector3 PtVector3::operator *(float dVal )const
{
	PtVector3	v3Ret;
    v3Ret.x = x * dVal; v3Ret.y = y * dVal; v3Ret.z = z * dVal;
	return v3Ret;
}

PtVector3& PtVector3::operator*=( float fOp )
{
    x *= fOp; y *= fOp; z *= fOp;
	return *this;
}

PtVector3  PtVector3::operator/( float fOp )const
{
	PtVector3	v3Ret;
    v3Ret.x = x / fOp; v3Ret.y = y / fOp; v3Ret.z = z / fOp;
	return v3Ret;
}

PtVector3& PtVector3::operator/=( float fOp )
{
    x /= fOp; y /= fOp; z /= fOp;
	return *this;
}

PtVector3 PtVector3::operator+(float fOp) const
{
	PtVector3	v3Ret;
	v3Ret.x = x + fOp; v3Ret.y = y + fOp; v3Ret.z = z + fOp;
	return v3Ret;
}
PtVector3 PtVector3::operator-(float fOp) const
{
	PtVector3	v3Ret;
	v3Ret.x = x - fOp; v3Ret.y = y - fOp; v3Ret.z = z - fOp;
	return v3Ret;
}

PtVector3 &PtVector3::operator-=(const PtVector3 &v3)
{
	x	-= v3.x;
	y	-= v3.y;
	z	-= v3.z;
	return *this;
}

PtVector3& PtVector3::operator*=(const PtVector3& v3)
{
	x *= v3.x;
	y *= v3.y;
	z *= v3.z;
	return *this;
}

PtVector3& PtVector3::operator/=(const PtVector3& v3)
{
	if (v3.x != 0)
		x /= v3.x;
	if (v3.y != 0)
		y /= v3.y;
	if (v3.z != 0)
		z /= v3.z;
	return *this;
}

PtVector3 &PtVector3::operator+=(const PtVector3 &v3)
{
	x	+= v3.x;
	y	+= v3.y;
	z	+= v3.z;
	return *this;
}

PtVector3 PtVector3::operator-(const PtVector3 &v3)const
{
	PtVector3	v3Ret;
	v3Ret.x		= x - v3.x;
	v3Ret.y		= y - v3.y;
	v3Ret.z		= z - v3.z;
	return v3Ret;
}

PtVector3 PtVector3::operator+(const PtVector3 &v3)const
{
	PtVector3	v3Ret;
	v3Ret.x		= x + v3.x;
	v3Ret.y		= y + v3.y;
	v3Ret.z		= z + v3.z;
	return v3Ret;
}

PtVector3 &PtVector3::Reflect(const PtVector3 &v4Normal)
{
	PtVector3 v3, v3inv;

	v3inv = *this;
	v3inv *= (float)(-1);

	v3 = v4Normal.Cross(v3inv.Cross(v4Normal));
	v3 *= (float)(-2);
	*this = v3+v3inv;
	return *this;
}

float PtVector3::line_distance(const PtVector3* _line, PtVector3* _collision) const// return distance and collision position in line
{
	float rate;
	float dist = line_distance(_line, &rate);

	PtVector3 dif_v3 = _line[1] - _line[0];
	if (rate < 0)
	{
		*_collision = _line[0];
		dif_v3 = *this - _line[0];
		dist = dif_v3.Length();
	}
	else if (rate > 1)
	{
		*_collision = _line[1];
		dif_v3 = *this - _line[1];
		dist = dif_v3.Length();
	}
	else {
		*_collision = _line[0] + dif_v3 * rate;
	}

	return dist;
}

// this is point, _line is two point in line
float PtVector3::line_distance(const PtVector3* _line, float* _rate) const
{
	PtVector3 v = _line[1] - _line[0], p = *this - _line[0];
	PtVector3 n;
	float len;

	v.GetNormal(&n, &len);
	float dist;
	if (len == 0)
	{
		v = _line[0] - *this;
		dist = v.Length();
		if(_rate)
			*_rate = 0;
	}
	else
	{
		dist = n.Cross(p).Length();

		if (_rate)
			* _rate = n.Dot(p) / len;
	}

	return dist;
}

bool PtVector3::line_in(const PtVector3& _v2, const PtVector3& _p)
{
	PtVector3 vec, dist_v;
	vec = _v2 - *this;
	dist_v = _p - *this;

	float dot = vec.Dot(dist_v);
	if (dot < 0)
		return false;
	if (dist_v.DoubleLength() > vec.DoubleLength())
		return false;
	return true;
}

PtVector3 PtVector3::line_pos(const PtVector3& _v2, const PtVector3& _p)
{
	PtVector3 vec, dist_v;
	vec = _v2 - *this;
	vec.MakeNormal();
	dist_v = _p - *this;
	float len = vec.Dot(dist_v);
	PtVector3 pos = vec * len + *this;

	return pos;
}

float PtVector3::distance(const PtVector3& _dir, const PtVector3& _p1, PtVector3* _normal_p, float *_inline_pf)
{
	PtVector3 cross_v3, v2, dirNor_v3;
	float len_f;
	dirNor_v3 = _dir;
	dirNor_v3.MakeNormal(&len_f);

	float dist_f;
	v2 = _p1 - *this;

	cross_v3 = _dir.Cross(v2);
	cross_v3 = cross_v3.Cross(_dir);
	cross_v3.MakeNormal();

	dist_f = cross_v3.Dot(v2);

	float col_f = 0;

	col_f = dirNor_v3.Dot(v2);

	if (_inline_pf)
		* _inline_pf = 0;

	if (col_f < 0) {
		dist_f = v2.Length();
		if (_inline_pf)
			* _inline_pf = -col_f;
	}else if (col_f > len_f) {
		v2 = _p1 - (*this + _dir);
		dist_f = v2.Length();
		if (_inline_pf)
			* _inline_pf = col_f - len_f;
	}

	if (_normal_p && _normal_p->Dot(cross_v3) < 0)
		dist_f *= -1;

	return dist_f;
}

float PtVector3::angle_inter(const PtVector3& _v3)
{
	float dot = Dot(_v3);
	if (dot < -1.f)
		dot = -1.f;
	if (dot > 1.f)
		dot = 1.f;
	float angle = acosf(dot);
	return angle;
}

float PtVector3::angle_inter(const PtVector3& v3, const PtVector3& _up)
{	
	float angle = angle_inter(v3);
	PtVector3 cros = Cross(v3);
	if (cros.Dot(_up) < 0)
		angle = -angle;
	return angle;
}

float PtVector3::angle_plus(const PtVector3& v3, const PtVector3 &_up)
{
	float angle = angle_inter(v3, _up);
	if(angle < 0)
		angle += PM_PI_OVER_2;
	return angle;
}

float PtVector3::triangle_area_size(PtVector3 _p1, PtVector3 _p2, PtVector3 _p3)
{	
	PtVector3 dir = _p2 - _p1;

	float len;
	PtVector3 nor, cross;
	dir.GetNormal(&nor, &len);
	dir = nor;

	nor = _p3 - _p1;
	cross = dir.Cross(nor);
	float height = cross.Length();

	return (len * height / 2.f);
}

PtVector3 &PtVector3::Reflect(const PtVector3 &v3Normal, float fElastic, float fFriction)
{
	PtVector3 v3, v3inv, v3ElasticEle, v3FrictionEle, v3Cross;

	v3inv = *this;
	v3inv *= (float)-1;

	v3ElasticEle = v3Normal * (v3Normal.Dot(v3inv) * fElastic);
	v3Cross = v3inv.Cross(v3Normal);
	v3 = v3Normal.Cross(v3Cross);
	v3FrictionEle = v3 * ((float)-1) * fFriction;
	
	*this = v3ElasticEle + v3FrictionEle;
	return *this;
}

PtVector3 operator *( float dVal , const PtVector3 &v )
{
	PtVector3	v3Ret;

    v3Ret.x	= dVal * v.x;  v3Ret.y	= dVal * v.y; v3Ret.z	= dVal * v.z;
	return v3Ret;
}

bool PtVector3::line_collision(const PtVector3& AP1, const PtVector3& AP2, const PtVector3& BP1, const PtVector3& BP2, PtVector3* IP)
{

	float t;
	float s;
	float under = (BP2.y - BP1.y) * (AP2.x - AP1.x) - (BP2.x - BP1.x) * (AP2.y - AP1.y);
	if (under == 0)
	{
		PtVector3 v1, v2, v3, c;
		float l;
		v1 = AP2 - AP1;
		v2 = BP1 - AP1;
		v3 = BP2 - AP1;
		c = v1.Cross(v2);
		l = v1.DoubleLength();
		if (c.DoubleLength() == 0
			&& ((v2.Dot(v1) >= 0 && v2.DoubleLength() <= l)
				|| (v3.Dot(v1) >= 0 && v3.DoubleLength() <= l)))
		{
			if (v1.Dot(v2) > 0)
				* IP = BP1;
			else
				*IP = AP1;
			return true;
		}
		return false;
	}

	float _t = (BP2.x - BP1.x) * (AP1.y - BP1.y) - (BP2.y - BP1.y) * (AP1.x - BP1.x);
	float _s = (AP2.x - AP1.x) * (AP1.y - BP1.y) - (AP2.y - AP1.y) * (AP1.x - BP1.x);

	t = _t / under;
	s = _s / under;

	//if (t < 0.0 || t>1.0 || s < 0.0 || s>1.0) return false;
	if (_t == 0 && _s == 0) return false;

	IP->x = AP1.x + t * (AP2.x - AP1.x);
	IP->y = AP1.y + t * (AP2.y - AP1.y);
	IP->z = AP1.z + t * (AP2.z - AP1.z);

	return true;
}

bool PtVector3::line_continue(const PtVector3& AP1, const PtVector3& AP2, const PtVector3& BP1, const PtVector3& BP2, float _dotMargin, float _maxDist)
{
	PtVector3 dir, dir2;

	dir = AP2 - AP1;
	dir.MakeNormal();

	dir2 = BP2 - AP2;
	dir2.MakeNormal();

	if (AP2.closer(BP1, _maxDist))
		return true;

	float dot = dir.Dot(dir2);
	if (dot < 1.f - _dotMargin / 2.f
		|| 1.f + _dotMargin / 2.f < dot)
		return false;

	dir2 = BP2 - BP1;
	dot = dir.Dot(dir2);
	if (dot < 1.f - _dotMargin / 2.f
		|| 1.f + _dotMargin / 2.f < dot)
		return false;

	dir2 = BP1 - AP2;
	if (dir2.DoubleLength() > _maxDist * _maxDist)
		return false;

	return true;
}

bool PtVector3::line_intersection(const PtVector3& AP1, const PtVector3& AP2,
	const PtVector3& BP1, const PtVector3& BP2, PtVector3* IP)
{
	float t;
	float s;
	float under = (BP2.y - BP1.y) * (AP2.x - AP1.x) - (BP2.x - BP1.x) * (AP2.y - AP1.y);
	if (under == 0) // paraller
	{
		PtVector3 v1, v2, v3, c;
		float l;
		v1 = AP2 - AP1;
		v2 = BP1 - AP1;
		v3 = BP2 - AP1;
		c = v1.Cross(v2);
		l = v1.DoubleLength();
		if (c.DoubleLength() == 0 // two lines are in the same line
			&& ((v2.Dot(v1) >= 0 && v2.DoubleLength() <= l)
				|| (v3.Dot(v1) >= 0 && v3.DoubleLength() <= l)))
		{
			if (v1.Dot(v2) > 0)
				* IP = AP2; // same direction
			else
				*IP = AP1; // opsit direction
			return true;
		}
		return false;
	}

	float _t = (BP2.x - BP1.x) * (AP1.y - BP1.y) - (BP2.y - BP1.y) * (AP1.x - BP1.x);
	float _s = (AP2.x - AP1.x) * (AP1.y - BP1.y) - (AP2.y - AP1.y) * (AP1.x - BP1.x);

	t = _t / under;
	s = _s / under;

	if (t < 0.0 || t>1.0 || s < 0.0 || s>1.0) return false;
	if (_t == 0 && _s == 0) return false;

	IP->x = AP1.x + t * (AP2.x - AP1.x);
	IP->y = AP1.y + t * (AP2.y - AP1.y);
	IP->z = AP1.z + t * (AP2.z - AP1.z);
	return true;
}

void PtBound::init()
{
	bnd[0].SetInit();
	bnd[1].SetInit();
}

void PtBound::set(PtVector3 _v1, PtVector3 _v2)
{
	set(_v1);
	if (_v2.x < _v1.x)
		bnd[0].x = _v2.x;
	if (_v2.x > _v1.x)
		bnd[1].x = _v2.x;
	if (_v2.y < _v1.y)
		bnd[0].y = _v2.y;
	if (_v2.y > _v1.y)
		bnd[1].y = _v2.y;
}

void PtBound::set(PtVector3 _v3)
{
	bnd[0] = bnd[1] = _v3;
}

bool PtBound::set(const STLVVec3& _v3_a)
{
	if (_v3_a.empty())
		return false;

	bnd[0] = bnd[1] = _v3_a[0];
	for (unsigned i = 1; i < _v3_a.size(); i++)
	{
		if (bnd[0].x > _v3_a[i].x)
			bnd[0].x = _v3_a[i].x;
		if (bnd[0].y > _v3_a[i].y)
			bnd[0].y = _v3_a[i].y;
		if (bnd[1].x < _v3_a[i].x)
			bnd[1].x = _v3_a[i].x;
		if (bnd[1].y < _v3_a[i].y)
			bnd[1].y = _v3_a[i].y;
	}
	return true;
}

PtVector3& PtBound::operator[](int iIndex)
{
	return bnd[iIndex];
}

const PtVector3& PtBound::operator[](int iIndex) const
{
	return bnd[iIndex];
}

void PtBound::merge(const PtBound& _bnd)
{
	if (bnd[0].x > _bnd[0].x)
		bnd[0].x = _bnd[0].x;
	if (bnd[0].y > _bnd[0].y)
		bnd[0].y = _bnd[0].y;
	if (bnd[1].x < _bnd[1].x)
		bnd[1].x = _bnd[1].x;
	if (bnd[1].y < _bnd[1].y)
		bnd[1].y = _bnd[1].y;
}

bool PtBound::merge(STLVVec3& _v3_a)
{
	if (_v3_a.empty())
		return false;

	for (unsigned i = 0; i < _v3_a.size(); i++)
	{
		if (bnd[0].x > _v3_a[i].x)
			bnd[0].x = _v3_a[i].x;
		if (bnd[0].y > _v3_a[i].y)
			bnd[0].y = _v3_a[i].y;
		if (bnd[1].x < _v3_a[i].x)
			bnd[1].x = _v3_a[i].x;
		if (bnd[1].y < _v3_a[i].y)
			bnd[1].y = _v3_a[i].y;
	}
	return true;
}

bool PtBound::collision(PtVector3* _v3_a, PtVector3* _v3Pos_p, int* _idx_p)
{
	if (collision(_v3_a[0], 0))
	{
		if (collision(_v3_a[1], 0))
		{
			*_idx_p = 3;// all of two points are in.
			return true;
		}
		else 	*_idx_p = 0;
	}
	else if (collision(_v3_a[1], 0)) *_idx_p = 1;
	else
		return false;

	// in case just one point is in
	PtVector3 v3[2];
	v3[0] = bnd[0];
	v3[1] = v3[0];
	v3[1].x = bnd[1].x;

	if (PtVector3::line_intersection(v3[0], v3[1], _v3_a[0], _v3_a[1], _v3Pos_p))
		return true;

	v3[0] = v3[1];
	v3[1] = bnd[1];
	if (PtVector3::line_intersection(v3[0], v3[1], _v3_a[0], _v3_a[1], _v3Pos_p))
		return true;

	v3[0] = v3[1];
	v3[1].x = bnd[0].x;
	if (PtVector3::line_intersection(v3[0], v3[1], _v3_a[0], _v3_a[1], _v3Pos_p))
		return true;

	v3[0] = v3[1];
	v3[1] = bnd[0];
	if (PtVector3::line_intersection(v3[0], v3[1], _v3_a[0], _v3_a[1], _v3Pos_p))
		return true;

	return false; // couldn't happen
}

bool PtBound::collision(PtVector3 _v3, float _ext, int _axi) const
{
	if ((_axi == -1 || _axi == 0) && (_v3.x < bnd[0].x - _ext || _v3.x > bnd[1].x + _ext))
		return false;
	
	if ((_axi == -1 || _axi == 1) && (_v3.y < bnd[0].y - _ext || _v3.y > bnd[1].y + _ext))
		return false;
	return true;
}

bool PtBound::almost(PtBound& _bnd, float _multi)
{
	if (collision(_bnd))
	{
		int s1 = _bnd.size();
		PtBound bnd3 = _bnd;

		if (bnd3[0].x < bnd[0].x)
			bnd3[0].x = bnd[0].x;
		if (bnd3[0].y < bnd[0].y)
			bnd3[0].y = bnd[0].y;
		if (bnd3[1].x > bnd[1].x)
			bnd3[1].x = bnd[1].x;
		if (bnd3[1].y > bnd[1].y)
			bnd3[1].y = bnd[1].y;

		int s3 = bnd3.size();

		if (s1 > s3 && s1 < s3 * _multi)
			return true;
	}
	return false;
}

bool PtBound::include(PtBound& _bnd)
{
	if (_bnd[0].x < bnd[0].x
		|| _bnd[0].y < bnd[0].y
		|| _bnd[1].x > bnd[1].x
		|| _bnd[1].y > bnd[1].y)
		return false;
	return true;
}

bool PtBound::collision(PtBound& _bnd, float _ext)
{
	PtVector3 vec[4];
	_bnd.edge2d_get(vec);

	for (int i = 0; i < 4; i++)
	{
		if (collision(vec[i], _ext))
			return true;
	}

	edge2d_get(vec);
	for (int i = 0; i < 4; i++)
	{
		if (_bnd.collision(vec[i], _ext))
			return true;
	}

	if (_bnd[0].x <= bnd[0].x && bnd[1].x <= _bnd[1].x
		&& _bnd[0].y > bnd[0].y && bnd[1].y > _bnd[1].y)
		return true;

	if (_bnd[0].y <= bnd[0].y && bnd[1].y <= _bnd[1].y
		&& _bnd[0].x > bnd[0].x && bnd[1].x > _bnd[1].x)
		return true;

	return false;
}

void PtBound::edge2d_get(PtVector3* _v3_a)
{
	_v3_a[0] = bnd[0];
	_v3_a[1] = bnd[1];
	_v3_a[2].SetValue(bnd[0].x, bnd[1].y, 0);
	_v3_a[3].SetValue(bnd[1].x, bnd[0].y, 0);
}

void PtBound::extend(float _ext)
{
	bnd[0] -= _ext;
	bnd[1] += _ext;
}

int PtBound::size()
{
	float w, h;
	w = bnd[1].x - bnd[0].x;
	h = bnd[1].y - bnd[0].y;

	if (w == 0 && h == 0)
		return 0;

	if (w == 0)
		w = 1.f;
	if (h == 0)
		h = 1.f;

	return (int)(w * h);
}

bool PtBound::inCorner(const PtVector3 &_pos, float _epsilon) const
{
	if(bnd[0].closer(_pos, _epsilon))
		return true;
	
	if(bnd[1].closer(_pos, _epsilon))
		return true;
	
	PtVector3 corner;
	corner.x = bnd[0].x;
	corner.y = bnd[1].y;
	
	if(corner.closer(_pos, _epsilon))
		return true;
	
	corner.x = bnd[1].x;
	corner.y = bnd[0].y;
	
	if(corner.closer(_pos, _epsilon))
		return true;
	
	return false;
}

int PtBound::quadrantGet(const PtVector3 &_pos, float _epsilon)
{
	if(AlmostEqual(_pos.x, bnd[0].x, _epsilon))
		return 1;
	else if(AlmostEqual(_pos.y, bnd[1].y, _epsilon))
		return 2;
	else if(AlmostEqual(_pos.x, bnd[1].x, _epsilon))
		return 3;
	else if(AlmostEqual(_pos.y, bnd[0].y, _epsilon))
		return 4;
	return 0;
}

bool PtBound::inEdge(const PtVector3 &_pos, float _epsilon)
{
	if((AlmostEqual(bnd[0].x, _pos.x, _epsilon) 
			|| AlmostEqual(bnd[1].x, _pos.x, _epsilon) )
		&& bnd[0].y <= _pos.y && _pos.y <= bnd[1].y)
		return true;

	if((AlmostEqual(bnd[0].y, _pos.y, _epsilon) 
			|| AlmostEqual(bnd[1].y, _pos.y, _epsilon) )
		&& bnd[0].x <= _pos.x && _pos.x <= bnd[1].x)
		return true;
	
	return false;
}

bool PtBound::inEdge(int _axi, const PtVector3 &_pos, float _epsilon) const
{
	for(int i=0; i<2; i++)
	{
		if(AlmostEqual(bnd[i][_axi], _pos[_axi], _epsilon))
			return true;
	}
	return false;
}

bool PtBound::inEdge(PtBound& _bnd, float _epsilon)
{
	float w, h;
	w = _bnd[1].x - _bnd[0].x;
	h = _bnd[1].y - _bnd[0].y;

	if (w > 0)
	{
		if (h > 0)
			return false;

		if (AlmostEqual(bnd[0].y, _bnd[0].y, _epsilon) || AlmostEqual(bnd[1].y, _bnd[0].y, _epsilon))
			return true;
	}
	else if(h > 0){
		if (AlmostEqual(bnd[0].x,_bnd[0].x, _epsilon) || AlmostEqual(bnd[1].x,_bnd[0].x, _epsilon))
			return true;
	}
	return false;
}

PtVector3 PtBound::center()
{
	PtVector3 cen = bnd[0] + bnd[1];
	cen /= 2.f;
	return cen;
}

// return center of collision points with _vec_a(line) what cross this boundbox.
bool PtBound::collisionCenter(const STLVVec3& _vec_a, PtVector3* _center_pv3, int *_idx_p)
{	
	STLVVec3 vec_a;
	PtVector3 pos;
	pos = bnd[0];			vec_a.push_back(pos);
	pos.SetValue(bnd[1].x, bnd[0].y, 0);			vec_a.push_back(pos);
	pos = bnd[1];			vec_a.push_back(pos);
	pos.SetValue(bnd[0].x, bnd[1].y, 0);			vec_a.push_back(pos);
	pos = bnd[0];			vec_a.push_back(pos);

	PtVector3 ip, accu;
	accu.SetInit();
	float cnt = 0;
	for (int i = 1; i < vec_a.size(); i++)
	{
		for (int j = 1; j < _vec_a.size(); j++)
		{
			if (PtVector3::line_intersection(vec_a[i - 1], vec_a[i], _vec_a[j - 1], _vec_a[j], &ip))
			{
				if(_idx_p)
					*_idx_p = j - 1;
				accu += ip;
				cnt += 1.f;
			}
		}
	}

	if (cnt > 0)
	{
		*_center_pv3 = accu / cnt;
		return true;
	}
	return false;
}

bool PtPolygon::clockwiseCheck(const PtVector3 &_up, const STLVVec3& _polygon_a, float _epsilon)
{
	float angle = 0;
	int cnt = (int)_polygon_a.size();
	if(_polygon_a[0].closer(_polygon_a.back(), _epsilon))
		cnt--;

	if(cnt < 3)
	{ 
		return false;
	}
	
	PtVector3 pos = _polygon_a[0];
	PtVector3 dir1 = _polygon_a[1] - pos, dir_f;
	dir1.MakeNormal();
	dir_f = dir1;
	PtVector3 dir2;
	for(int i=2; i<(int)cnt; i++)
	{
		dir2 = _polygon_a[i] - pos;
		dir2.MakeNormal();
		angle += dir1.angle_inter(dir2, _up);
		dir1 = dir2;
	}

	if(angle < 0) // right hand coordinate
		return true;
	return false;
}

bool PtPolygon::inCheck(const PtVector3& _v3, const STLVVec3& _polygon_a)
{
	float accu;

	PtVector3 up(0, 0, 1);

	accu = 0.f;
	PtVector3 v1 = _polygon_a[0] - _v3, v2;
	v1.MakeNormal();
	PtVector3 first_v3 = v1;
	for (int j = 1; j < _polygon_a.size(); j++)
	{
		v2 = _polygon_a[j] - _v3;
		v2.MakeNormal();

		if (AlmostEqual(v1.Dot(v2), -1))
			return true; // on a line

		float angle = v1.angle_inter(v2, up);
		accu += angle;
		v1 = v2;
	}
	
	if(!_polygon_a[0].AllmostEqual(_polygon_a.back()))
		accu += v1.angle_inter(first_v3, up);

	if (accu < 0)
		accu *= -1;

	if (accu + 0.01 >= PM_PI_OVER_2)
		return true;

	return false;
}

float PtPolygon::length(STLVVec3 &_polygon_a)
{
	float accum = 0;
	PtVector3 len;
	for(int i=1; i<_polygon_a.size(); i++){
		len = _polygon_a[i] - _polygon_a[i-1];
		accum += len.Length();
	}
	return accum;
}

float PtPolygon::areaGet(const STLVVec3 &_polygon_a)
{
	int start = 0;
	float area = 0;
	bool clock = clockwiseCheck(PtVector3(0, 0, 1), _polygon_a);

	if(_polygon_a.size() < 3)
		return 0;
		
	STLVVec3 triangle, polygon;
	polygon = _polygon_a;
	do{
		bool find = false;
		int loopcnt = 0;	
		do{
			if(loopcnt > 1)
				return area;
			triangle.clear();

			int index = start;
			STLSInt triSet;
			for(int i=0; i<3; i++)
			{
				triSet.insert(index);
				triangle.push_back(polygon[index]);
				index++;
				if (index >= polygon.size())
					index = 0;
			}

			find = true;
			if(clock != clockwiseCheck(PtVector3(0, 0, 1), triangle))
				find = false;

			for(int i=0; i<polygon.size(); i++)
			{
				if(triSet.find(i) != triSet.end())
					continue;
				
				if(inCheck(polygon[i], triangle))
					find = false;
			}
			start++;
			if(start >= polygon.size())
			{
				start = 0;
				loopcnt++;
			}
		}while(!find);
		
		area += _areaConvexGet(triangle);
		polygon.erase(polygon.begin()+start);
		start--;
		if(start < 0)
			start = (int)polygon.size()-1;
	}while(polygon.size() > 2);
	return area;
}

bool PtPolygon::convexCheck(const STLVVec3 &_polygon_a, bool *_clockwise_p)
{
	float angle = 0;
	if(_polygon_a.size() < 3)
	{ 
		return false;
	}
	
	int overcheck[2] = {0, 0};

	PtVector3 pos = _polygon_a[0];
	PtVector3 dir1 = _polygon_a[1] - pos, dir_f;
	dir1.MakeNormal();
	dir_f = dir1;
	PtVector3 dir2, foreword(0, 0, 1);

	for(int i=2; i<(int)_polygon_a.size()-1; i++)
	{
		dir2 = _polygon_a[i] - pos;
		dir2.MakeNormal();
		float t = dir1.angle_inter(dir2, foreword); 
		if(t <= PM_PI)
			overcheck[0]++;
		else
			overcheck[1]++;
		angle += t;
		dir1 = dir2;
	}

	if(_clockwise_p){
		*_clockwise_p = false;
		if(angle < 0) // right hand coordinate
			*_clockwise_p = true;
	}

	if(angle < 0)
	{
		if(overcheck[1] == 0)
			return true;
	}else{
		if(overcheck[0] == 0)
			return true;
	}
	return false;
}

float PtPolygon::_areaConvexGet(const STLVVec3 &_polygon_a)
{
	PtVector3 pos = _polygon_a[0], dir;
	float area = 0, r, h;
	for(int i=2; i<_polygon_a.size(); i++)
	{
		dir = _polygon_a[i] - _polygon_a[i-1];
		h = pos.line_distance(&_polygon_a[i-1], &r);
		area += dir.Length() * h / 2.f;
	}
	return area;
}

int PtPolygon::inconvexPointGet(const STLVVec3 &_polygon_a, bool _clockwise, int _start)
{
	_start += 2;
	if(_polygon_a.size() < _start+1)
		return _start;
	
	PtVector3 foreword(0, 0, 1);
	if(_clockwise)
		foreword.z = -1;
	PtVector3 pre = _polygon_a[_start-1] - _polygon_a[_start-2], dir;
	pre.MakeNormal();
	float angle = 0;
	for(int i=_start; i<_polygon_a.size(); i++)
	{
		dir = _polygon_a[_start] - _polygon_a[_start-1];
		dir.MakeNormal();
		angle = pre.angle_inter(dir, foreword);
		if(angle < 0)
			return i-1;
		pre = dir;
	}
	return (int)_polygon_a.size()-1;
}

bool PtCircle::inCheck(const PtVector3 &_pos) const
{
	PtVector3 dist = _pos - m_v[0];
	//if(dist.DoubleLength() <= m_v[1].x * m_v[1].x + _EPSILON)
	if (dist.Length() <= m_v[1].x + _EPSILON)
		return true;

	return false;
}

bool PtCircle::collision(const PtCircle &_circle, STLVVec3 *_pos_pa) const
{
	PtLine line;
	PtVector3 dir, z(0, 0, 1);
	dir = _circle.m_v[0] - m_v[0];
	float l, rate, r = m_v[1].x, r2 = _circle.m_v[1].x;
	dir.MakeNormal(&l);
	
	if(l == 0)
		return false;
	
	if(r>r2)
	{
		if(r > r2+l)
			return false;
	}else{
		if(r2 > r+l)
			return false;
	}
	
	bool inv = false;
	PtCircle circle = *this;
	if(r2 > r){
		inv = true;
		float t = r2;
		r2 = r;
		r = t;
		dir *= -1;
		circle = _circle;
	}
	
	float theta = atan(r2/r);
	float k = r*cos(theta), d = 0;
	if(l <= k){
		float L = r - r2;
		rate = (l-k)/(L-k);
		d = r*rate + k*(1-rate);
	}else{
		rate = r / (r+r2);
		d = rate * l;
	}
	line.m_pos = circle.m_v[0] + dir * d;

	line.m_dir = z.Cross(dir); // line direction
	if(!circle._collision(line, d, _pos_pa))
		return false;
	
	for(int i=0; i<_pos_pa->size(); i++)
		(*_pos_pa)[i] += line.m_pos;
	return true;
}

bool PtCircle::_collision(const PtLine &_line, float _d, STLVVec3 *_pos_pa) const
{
	float r = m_v[1].x;
	if(r < _d) // no collision
		return false;

	if(r == _d) // one point collision
	{
		_pos_pa->push_back(PtVector3(0, 0, 0)); // center position
		return true;	
	}

	float l = (float)sqrt(r*r - _d*_d);
	_pos_pa->push_back(_line.m_dir*l);
	_pos_pa->push_back(_line.m_dir*(-l));
	return true;
}

bool PtCircle::collisionExt(const PtLine &_line, STLVVec3 *_pos_pa) const
{
	PtVector3	near_v3;
	float rate = 0, dist = 0;

	PtLine line;
	near_v3 = _line.outline_closest(m_v[0], &rate, &dist, &line.m_dir);
	line.m_pos = near_v3;

	if(!_collision(line, dist, _pos_pa))
		return false;

	for(int i=0; i<_pos_pa->size(); i++)
		(*_pos_pa)[i] += near_v3;
	
	if(_pos_pa->empty())
		return false;
	return true;
}

bool PtCircle::collision(const PtLine &_line, STLVVec3 *_pos_pa) const
{
	PtVector3	near_v3;
	float rate = 0, dist = 0;

	PtLine line;
	near_v3 = _line.outline_closest(m_v[0], &rate, &dist, &line.m_dir);
	line.m_pos = near_v3;

	if(!_collision(line, dist, _pos_pa))
		return false;

	float l = line.m_dir.Dot(_line.m_dir);
	for(int i=0; i<_pos_pa->size(); i++)
	{
		(*_pos_pa)[i] += near_v3;
		PtVector3 dir = _pos_pa->at(i) - _line.m_pos;
		float d = line.m_dir.Dot(dir);
		if(d >= 0 && d <= l)
			continue;

		_pos_pa->erase(_pos_pa->begin()+i);
		i--;
	}

	if(_pos_pa->empty())
		return false;
	return true;
}

PtVector3 PtCircle::circle_pos(float _degree) const
{
	PtVector3 pos(0, 0, 0);

	pos.x = cosf(_degree) * m_v[1].x;
	pos.y = sinf(_degree) * m_v[1].x;
	pos += m_v[0];

	return pos;
}

PtVector3 PtCircle::posGet(float _angle) const
{
	PtVector3 pos;
	pos = m_v[0];
	pos.x += cosf(_angle)*m_v[1].x;
	pos.y += sinf(_angle)*m_v[1].x;
	return pos;
}

void PtArc::mirrorLR()
{
	m_v[0].x *= -1;
	
	float &start = m_v[1].y;
	float &end = m_v[1].z;

	float t = start;
	start = end;
	end = t;

	start += PM_HALF;
	end += PM_HALF;

	start *= -1;
	end *= -1;

	start -= PM_HALF;
	end -= PM_HALF;

	while (start < 0)
	{
		start += PM_PI_OVER_2;
		end += PM_PI_OVER_2;
	}
}

const PtArc &PtArc::operator*=(const PtMatrix4 &_m4)
{
	m_v[0] *= _m4;
	
	float angle = _m4.GetRotate(2) * -1.f;
	m_v[1].y += angle;
	m_v[1].z += angle;
	
	angleOrganize();
	return *this;
}

int PtArc::typeGet(const PtVector3 &_start, float *_diameter, float _epsilon, STLVVec3 *_dir_a)
{
	// 1: small cc, small c, big cc, big c

	if(_dir_a)
		_dir_a->clear();

	STLVVec3 pos_a;
	pos_a.push_back(startGet());
	pos_a.push_back(endGet()); 
	int type = arc_c_small;
	if(pos_a[0].closer(_start, _epsilon))
		type = arc_cc_small;
	
	PtVector3 center = m_v[0];
	PtVector3 dir1 = center - pos_a[0];
	PtVector3 dir2 = pos_a[1] - center;
	dir1.MakeNormal();
	dir2.MakeNormal();
	float angle = dir1.angle_inter(dir2, PtVector3(0, 0, 1));
	if(angle > 0)
		type += 2;

	if(_dir_a)
	{
		PtVector3 z(0, 0, 1);
		_dir_a->push_back(dir1.Cross(z));
		_dir_a->push_back(dir2.Cross(z));
	}

	if(_diameter)
		*_diameter = m_v[1].x * 2.f;

	return type;
}

bool PtArc::sideGet(PtArc *_arc_p, float _offset)
{
	*_arc_p = *this;

	_arc_p->m_v[1].x += _offset;
	
	if(_arc_p->m_v[1].x <= 0)
		return false;

	return true;
}

float PtArc::distance(const PtVector3& _pos, float *_mid_p, PtVector3 *_ret_pv3) const
{
	float dist;
	PtVector3 dir = _pos - m_v[0];
	dir.MakeNormal(&dist);
	PtVector3 r(1, 0, 0), up(0, 0, 1);
	float degree = r.angle_inter(dir, up);

	if (degree < 0)
		degree += PM_PI_OVER_2;

	if (m_v[1].z > PM_PI_OVER_2 && degree < m_v[1].y)
		degree += PM_PI_OVER_2;

	if (degree >= m_v[1].y && degree <= m_v[1].z)
	{
		dist -= m_v[1].x;
		if (dist < 0)
			dist *= -1;
		if(_mid_p)
			*_mid_p = degree, *_ret_pv3 = m_v[0] + dir * m_v[1].x;
		return dist;
	}

	PtVector3 s, e;
	s = startGet();
	e = endGet();
	
	dir = _pos - s;
	float d = dir.Length();

	dir = _pos - e;
	dist = dir.Length();
	if (d < dist){
		if(_mid_p)
			*_mid_p = m_v[1].y, *_ret_pv3 = s;
		return d;
	}
	if(_mid_p)
		*_mid_p = m_v[1].z, *_ret_pv3 = e;
	
	return dist;
}

bool PtArc::onArc(const PtVector3& _pos, float _epsilon) const
{
	float dist = 0;
	if(!inArc(_pos, &dist))
		return false;
	
	if(m_v[1].x - _epsilon > dist
		|| dist > m_v[1].x + _epsilon)
		return false;

	return true;
}

bool PtArc::inArc(const PtVector3& _pos, float *_dist_p) const
{
	if(m_v[0].AllmostEqual(_pos))
		return true;
	
	PtVector3 dir = _pos - m_v[0];
	if (_dist_p)
		dir.MakeNormal(_dist_p);
	else
		dir.MakeNormal();

	PtVector3 r(1, 0, 0), up(0, 0, 1);
	float degree = r.angle_inter(dir, up);

	if (degree < m_v[1].y)
		degree += PM_PI_OVER_2;

	if (m_v[1].z >= PM_PI_OVER_2 && degree < m_v[1].y)
		degree += PM_PI_OVER_2;

	if (degree >= m_v[1].y && degree <= m_v[1].z)
		return true;

	return false;
}

float PtArc::angleGet(const PtVector3 &_pos) const
{
	PtVector3 dir = _pos - m_v[0];
	dir.MakeNormal();
	PtVector3 r(1, 0, 0), up(0, 0, 1);
	float degree = r.angle_inter(dir, up);

	if (degree < 0)
		degree += PM_PI_OVER_2;

	if (m_v[1].z > PM_PI_OVER_2 && degree < m_v[1].y)
		degree += PM_PI_OVER_2;

	return degree;
}

bool PtArc::inCheck(const PtVector3 &_pos) const
{
	if(!PtCircle::inCheck(_pos))
		return false;

	return inArc(_pos);
}

bool PtArc::collision(const PtArc &_arc, STLVVec3 *_pos_pa) const
{
	if(!PtCircle::collision(_arc, _pos_pa))
		return false;

	for (int i = 0; i < _pos_pa->size(); i++)
	{
		if (!inArc((*_pos_pa)[i])) {
			_pos_pa->erase(_pos_pa->begin() + i);
			i--;
		}else if(!_arc.inArc((*_pos_pa)[i])){
			_pos_pa->erase(_pos_pa->begin() + i);
			i--;
		}
	}

	return !_pos_pa->empty();
}

bool PtArc::incheck(STLVVec3 *_pos_pa) const
{
	for(int i=0; i<_pos_pa->size(); i++)
	{
		if(inCheck(_pos_pa->at(i))) // check collied position is in arc.
			continue;

		_pos_pa->erase(_pos_pa->begin()+i);
		i--;
	}

	if(_pos_pa->empty())
		return false;

	return true;
}

bool PtArc::collision(const PtLine &_line, STLVVec3 *_pos_pa) const
{
	if(!PtCircle::collision(_line, _pos_pa))
		return false;

	for (int i = 0; i < _pos_pa->size(); i++)
	{
		if (!inArc((*_pos_pa)[i])) {
			_pos_pa->erase(_pos_pa->begin() + i);
			i--;
		}
	}

	return !_pos_pa->empty();
}

bool PtArc::inSegment(const PtVector3 &_pos) const
{
	bool ret = inCheck(_pos);
	
	bool in = inTriCheck(_pos);
	
	float angle = m_v[1].z - m_v[1].y;
	
	if(angle >= PM_PI)
	{
		return ret || in;
	}
	return ret && !in;
}

// it can work when pos is in the ARC
bool PtArc::inTriCheck(const PtVector3 &_pos) const
{
	STLVVec3 tri;
	PtVector3 s = startGet(), e = endGet();
	tri.push_back(s);
	tri.push_back(e);
	tri.push_back(m_v[0]);
	
	return PtPolygon::inCheck(_pos, tri);
}

PtVector3 PtArc::startGet() const
{
	return posGet(m_v[1].y);
}

PtVector3 PtArc::endGet() const
{
	return posGet(m_v[1].z);
}

PtVector3 PtArc::middleGet() const
{
	float m = m_v[1].y + m_v[1].z;
	m /= 2.f;
	return posGet(m);
}

bool PtArc::biggerHalf() const
{
	if(m_v[1].z-m_v[1].y < PM_PI)
		return false;
	return true;
}

void PtArc::angleOrganize()
{
	float &s = m_v[1].y;
	float &e = m_v[1].z;
	
	while(s < 0)
	{
		s += PM_PI_OVER_2;
		e += PM_PI_OVER_2;
	}
	
	while(s > PM_PI_OVER_2)
	{
		s -= PM_PI_OVER_2;
		e -= PM_PI_OVER_2;
	}
}

void PtCircle::direction(const PtVector3 &_pos, PtVector3 *_dir_p) const
{
	PtVector3 z(0, 0, 1);
	PtVector3 out;
	out = _pos - m_v[0];
	out.MakeNormal();
	*_dir_p = z.Cross(out);
}

PtVector3 PtArc::dirGet(const PtVector3 &_pos)
{
	int idx = 0;
	PtVector3 pos = startGet();
	if(!pos.AllmostEqual(_pos))
		idx = 1, pos = endGet();

	PtVector3 z(0, 0, 1);
	PtVector3 dir;
	
	if(!idx)
		dir =  m_v[0] - pos;
	else
		dir = pos - m_v[0];
	dir /= m_v[1].x;
	dir = dir.Cross(z);
	dir.MakeNormal();
	
	return dir;
}

void PtArc::dirGet(STLVVec3 *_dir_a)
{
	_dir_a->clear();
	STLVVec3 pos_a;
	pos_a.push_back(startGet());
	pos_a.push_back(endGet());

	PtVector3 z(0, 0, 1);
	PtVector3 dir =  m_v[0] - pos_a[0];
	dir /= m_v[1].x;
	dir = dir.Cross(z);
	dir.MakeNormal();
	_dir_a->push_back(dir);
	dir = pos_a[1] - m_v[0];
	dir /= m_v[1].x;
	dir = dir.Cross(z);
	dir.MakeNormal();
	_dir_a->push_back(dir);
}

void PtArc::set(float _r, PtVector3 _c, PtVector3 _s, PtVector3 _e)
{
	m_v[0] = _c; // center
	m_v[1].x = _r; // radius

	PtVector3 left(1, 0, 0), forward(0, 0, 1), dir;

	dir = _s - _c;
	dir.MakeNormal();
	float start = left.angle_plus(dir, forward);

	m_v[1].y = start; // start
	dir = _e - _c;
	dir.MakeNormal();
	float end = left.angle_plus(dir, forward);
	while (end < start)
		end += PM_PI_OVER_2;
	m_v[1].z = end;  // end
}

PtArc::PtArc(float _r, PtVector3 _c, PtVector3 _s, PtVector3 _e)
{
	m_v[0] = _c; // center
	m_v[1].x = _r; // radius

	PtVector3 left(1, 0, 0), forward(0, 0, 1), dir;

	dir = _s - _c;
	dir.MakeNormal();
	float start = left.angle_plus(dir, forward);
	
	m_v[1].y = start; // start
	dir = _e - _c;
	dir.MakeNormal();
	float end = left.angle_plus(dir, forward);
	while(end < start)
		end += PM_PI_OVER_2;
	m_v[1].z = end;  // end
}

void PtLine::set(const STLVVec3 &_pos_a)
{
	m_pos = _pos_a[0];
	m_dir = _pos_a[1] - m_pos;
}

void PtLine::set(const PtVector3 *_pos_pa)
{
	m_pos = _pos_pa[0];
	m_dir = _pos_pa[1] - m_pos;
}

PtVector3 PtLine::outline_closest(const PtVector3& _pos, float *_rate_p, float *_dist_p, PtVector3 *_normal_p) const
{
	PtVector3 vec, dist_v;
	vec = m_dir;
	float l = 0;
	vec.MakeNormal(&l);
	dist_v = _pos - m_pos;
	float len = vec.Dot(dist_v);
	PtVector3 pos = vec * len + m_pos;
	if(_rate_p)
		*_rate_p = len / l;

	PtVector3 cros = vec.Cross(dist_v);
	if(_dist_p)
		*_dist_p = cros.Length();

	if(_normal_p)
		*_normal_p = vec;

	return pos;
}

float PtLine::distance(const PtVector3& _pos) const
{
	float rate = 0, dist = 0;
	PtVector3 nor;
	PtVector3 pos = outline_closest(_pos, &rate, &dist, &nor);

	if (0 <= rate && rate <= 1)
		return dist;

	PtVector3 dir;
	if (rate < 0)
	{
		dir = m_pos - _pos;
	}
	else {
		dir = m_pos + m_dir - _pos;
	}

	return dir.Length();
}

PtVector3 PtLine::inline_closest(const PtVector3& _pos) const
{
	float rate = 0;
	PtVector3 pos = outline_closest(_pos, &rate);
	PtVector3 dir = pos - m_pos;

	if(rate < 0)
		return m_pos;

	if(rate > 1)
		return m_pos + m_dir;

	return pos;
}

void PtLine::sideGet(PtLine *_line_p, PtVector3 _side, float _offset) const
{
	_line_p->m_pos = m_pos + _side * _offset;
	_line_p->m_dir = m_dir;
}

bool PtLine::collision(const PtLine &_line, PtVector3 *_pos_p) const
{
	return PtVector3::line_intersection(m_pos, m_pos+m_dir, _line.m_pos, _line.m_pos+_line.m_dir, _pos_p);
}

bool PtLine::collisionExt(const PtLine &_line, PtVector3 *_pos_p) const
{
	return PtVector3::line_collision(m_pos, m_pos+m_dir, _line.m_pos, _line.m_pos+_line.m_dir, _pos_p);
}
