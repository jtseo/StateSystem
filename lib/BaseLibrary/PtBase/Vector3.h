#pragma once
// Vector3.h: interface for the PtVector3 class.
//
//////////////////////////////////////////////////////////////////////

//#if !defined(AFX_VECTOR3_H__1FBAA874_B432_4CA3_9567_F1A1EA7463C3__INCLUDED_)
//#define AFX_VECTOR3_H__1FBAA874_B432_4CA3_9567_F1A1EA7463C3__INCLUDED_

/** \brief 
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: Vector3.h,v 1.1 2009/03/13 08:54:15 jtseo Exp $
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

class PtVector2
{
public:
    PtVector2() {};
    PtVector2(float _x, float _y){
        x = _x; y = _y;
    }
	void SetValue(float _x, float _y){
        x = _x; y = _y;
	}
	float& operator[]( int iIndex );
	const float& operator[]( int iIndex )const;
	
	float x, y;
};

class PtVector3 : public PtVector2
{
public:
	typedef enum{
		Axi_X,
		Axi_Y,
		Axi_Z,
		Axi_W
	} Axi;
	float z;

    void SetInit(){x=0; y=0; z=0;}
	
	inline PtVector3(){}
	PtVector3( const float *pVec ){
		SetValue(pVec);
	}
	PtVector3( const PtVector3& vecSrc ){
		SetValue(&vecSrc.x);
	}
    PtVector3(float x, float y, float z);
	~PtVector3(){};

	void Cross(const PtVector3& vec, PtVector3 *xpV3Ret);
    PtVector3 Cross( const PtVector3& vec ) const;
    float Dot( const PtVector3& vec ) const;
	float	Length() const;
	PtVector3 &GetNormal(PtVector3 *xpv3Ret, float *pfpLength = 0) const;
	PtVector3 &MakeNormal();
	PtVector3 &Rounded(float _place)
	{
		x = rounded(x, _place);
		y = rounded(y, _place);
		z = rounded(z, _place);
		return *this;
	}
	float MakeNormal(float *xpfLength);
	PtVector3& SetValue( float fX, float fY, float fZ );
	PtVector3& SetValue(const char *_v3_str);
	void GetValue(float *xpfpValue)const{
		memcpy(xpfpValue, (&x), sizeof(float)*3);
	}
	PtVector3& SetValue(const float *xpfpValue){
		memcpy((&x), xpfpValue, sizeof(float)*3);
		return *this;
	}
	
	UINT32 GetColor();
	PtVector3& SetColor(UINT32 _nColor);
	
	bool closer(const PtVector3& vec, float _dist_f) const;
	bool AllmostEqual(const PtVector3 &vec) const;
	bool operator ==(const PtVector3 &vec) const;
	bool operator !=(const PtVector3 &vec) const{
		return !(*this == vec);
	}
	PtVector3 operator-() const;
	PtVector3& operator=(const PtVector3& vecSrc);
	
	PtVector3 &operator-=(const PtVector3 &v3);
	PtVector3 &operator+=(const PtVector3 &v3);
	PtVector3 operator-(const PtVector3 &v3)const;
	PtVector3 operator+(const PtVector3 &v3)const;
//	PtVector3& operator*=(const CMatrix3& mat);
//	PtVector3& operator*=(const CMatrix4& mat);
	PtVector3& operator/=(const PtVector3& v3);
	PtVector3& operator*=(const PtVector3& v3);

	friend PtVector3 operator*(float dVal , const PtVector3 &v);
	
	PtVector3 operator*(float dVal) const;
	PtVector3 operator/(float fOp) const;
	PtVector3 operator+(float fOp) const;
	PtVector3 operator-(float fOp) const;
	PtVector3& operator*=(float fOp);
	PtVector3& operator/=(float fOp);

	inline PtVector3& operator-=(float _op) {
		return *this = *this - _op;
	}
	inline PtVector3& operator+=(float _op) {
		return *this = *this + _op;
	}
	
	float DoubleLength(void) const{
		return (x*x+y*y+z*z);
	}

	PtVector3 &Reflect(const PtVector3 &v4Normal);
	PtVector3 &Reflect(const PtVector3 &v3Normal, float fElastic, float fFriction);
	float angle_plus(const PtVector3& v3, const PtVector3 &_up);
	float angle_inter(const PtVector3& v3, const PtVector3 &_up);
	float angle_inter(const PtVector3& _v3);

	float distance(const PtVector3& _dir, const PtVector3& _p1, PtVector3* _normal_p = NULL, float *_inline_pf = NULL);
	static bool line_intersection(const PtVector3& AP1, const PtVector3& AP2, const PtVector3& BP1, const PtVector3& BP2, PtVector3* IP); // return only on the line
	static bool line_collision(const PtVector3& AP1, const PtVector3& AP2, const PtVector3& BP1, const PtVector3& BP2, PtVector3* IP); // can return out of the line
	PtVector3 line_pos(const PtVector3& _v2, const PtVector3& _p); // a cloest point in the line from a position(_p).
	bool line_in(const PtVector3& _v2, const PtVector3& _p); // a point is in the line or not.
	float line_distance(const PtVector3* _line, float * _rate) const; // return shortest distance from line and the position of rate
	float line_distance(const PtVector3* _line, PtVector3* _collision) const; // return distance and collision position in line

	static float triangle_area_size(PtVector3 _v1, PtVector3 _v2, PtVector3 _v3);
	static bool line_continue(const PtVector3& AP1, const PtVector3& AP2, const PtVector3& BP1, const PtVector3& BP2, float _dotMargin, float _maxDist); // check two line is extended by after _maxDist

	// ==========================================
	// math functions 
	static float rounded(float _value, float _place)
	{
		float up = _value / _place;
		up += 0.5;
		int rounded = (int)up;
		up = (float)rounded;
		up *= _place;
		return up;
	}
	// =========================================
};

class PtVector4 : public PtVector3
{
public:
	PtVector4& SetValue(float _x, float _y, float _z, float _w){
		PtVector2::x	= _x;
		PtVector2::y	= _y;
		PtVector3::z	= _z;
		this->w	= _w;
		return *this;
	}

	float w;
};

typedef std::vector<PtVector2, PT_allocator<PtVector2> >							STLVVec2;
typedef std::vector<PtVector3, PT_allocator<PtVector3> >							STLVVec3;
typedef std::vector<PtVector4, PT_allocator<PtVector4> >							STLVVec4;
typedef std::vector<STLVVec3, PT_allocator<STLVVec3> >							STLVstlVVec3;
typedef std::map<STLString, PtVector3, std::less<STLString>, PT_allocator<std::pair<const STLString, PtVector3> > >		STLMstrVec3;

class PtBound
{
public:
	PtVector3 bnd[2];
	PtVector3& operator[](int iIndex);
	const PtVector3& operator[](int iIndex) const;
	void init();
	void set(PtVector3 _v1, PtVector3 _v2);
	void set(PtVector3 _v3);
	bool set(const STLVVec3& _v3_a);
	void merge(const PtBound& _bnd);
	bool merge(STLVVec3& _v3_a);
	bool collision(PtBound& _bnd, float _ext = 0);
	bool collision(PtVector3 _v3, float _ext = 0, int _axi = -1) const;
	bool collision(PtVector3* _v3_a, PtVector3* _v3Pos_p, int *_idx_p);
	bool include(PtBound& _bnd);
	bool almost(PtBound& _bnd, float _multi);
	void edge2d_get(PtVector3* _v3_a);
	void extend(float _ext);
	int size();
	bool inEdge(PtBound& _bnd, float _epsilon = _EPSILON);
	bool inEdge(const PtVector3 &_pos, float _epsilon);
	bool inEdge(int _axi, const PtVector3 &_pos, float _epsilon) const;
	bool inCorner(const PtVector3 &_pos, float _epsilon) const;
	int quadrantGet(const PtVector3 &_pos, float _epsilon = _EPSILON);

	bool collisionCenter(const STLVVec3& _vec_a, PtVector3 *_center_pv3, int *_idx_p);
	PtVector3 center();
};

typedef std::vector<PtBound, PT_allocator<PtBound> >							STLVBound;
typedef std::map<STLString, PtBound, std::less<STLString>, PT_allocator<std::pair<const STLString, PtBound> > >		STLMstrBound;
typedef std::map<int, PtBound, std::less<int>, PT_allocator<std::pair<const int, PtBound> > >		STLMnBound;

class PtPolygon
{
public:
	static bool inCheck(const PtVector3 &_v3, const STLVVec3& _polygon_a);
	static bool clockwiseCheck(const PtVector3 &_up, const STLVVec3& _polygon_a, float _epsilon = _EPSILON);
	static float length(STLVVec3 &_polygon_a);
	static float areaGet(const STLVVec3 &_polygon_a);
	static bool convexCheck(const STLVVec3 &_polygon_a, bool *_clockwise_p = NULL);
	static int inconvexPointGet(const STLVVec3 &_polygon_a, bool _clockwise, int _start);
protected:
	static float _areaConvexGet(const STLVVec3 &_polygon_a); // 
};

bool AlmostEqual(float _a, float _b, float _epsilon = _EPSILON);

class PtLine
{
public:
	PtLine() {}
	PtVector3 m_pos, m_dir;

	void set(const STLVVec3 &_pos_a);
	void set(const PtVector3 *_pos_pa);
	void set(const PtVector3 &_pos, const PtVector3 &_dir)
	{
		m_pos = _pos;
		m_dir = _dir;
	}

	PtLine &operator=(const PtLine &_line)
	{
		m_pos = _line.m_pos;
		m_dir = _line.m_dir;
		return *this;		
	}

	PtVector3 inline_closest(const PtVector3 &_pos) const;
	PtVector3 outline_closest(const PtVector3 &_pos, float *_rate_p = NULL, float *_dist_p = NULL
		, PtVector3 *_normal_p= NULL) const;
	bool collision(const PtLine &_line, PtVector3 *_pos_p) const;
	bool collisionExt(const PtLine &_line, PtVector3 *_pos_p) const;
	float distance(const PtVector3& _pos) const;

	void sideGet(PtLine *_line_p, PtVector3 _side, float _offset) const;
};

class PtCircle
{
public:
	PtVector3 m_v[2];// position[1], [1] x: radius
	
	PtCircle(){}
	PtCircle(const STLVVec3 &param_a)
	{
		m_v[0] = param_a[0];
		m_v[1] = param_a[1];
	}

	bool inCheck(const PtVector3 &_pos) const;
	bool collision(const PtCircle &_circle, STLVVec3 *_pos_pa) const;
	bool collision(const PtLine &_line, STLVVec3 *_pos_pa) const;
	bool collisionExt(const PtLine &_line, STLVVec3 *_pos_pa) const;
	void direction(const PtVector3 &_pos, PtVector3 *_dir_p) const;
	PtVector3 posGet(float _angle) const;

	PtCircle& operator=(const PtCircle& _circle) {
		m_v[0] = _circle.m_v[0];
		m_v[1] = _circle.m_v[1];
		return *this;
	}
	
	PtVector3 circle_pos(float _degree) const;
protected:
	// collision with normalized line.m_dir & line.m_pos is nearest postion
	bool _collision(const PtLine &_line, float _dist, STLVVec3 *_pos_pa) const;
};

class PtMatrix4;

class PtArc : public PtCircle
{
public:
	typedef enum _ARC_
    {
        arc_none,
        arc_cc_small,
        arc_c_small,
		arc_large,
        arc_cc_large = arc_large,
        arc_c_large,
        arc_max
    } ARC_TYPE;

	// position[1], [1] x: radius, y: start, z: end
	PtArc(){}
	PtArc(const PtCircle &_circle){ 
		m_v[0] = _circle.m_v[0];
		m_v[1] = _circle.m_v[1];
	}
	PtArc(float _r, PtVector3 _c, PtVector3 _s, PtVector3 _e);
	void set(float _r, PtVector3 _c, PtVector3 _s, PtVector3 _e);

	float angleGet(const PtVector3 &_pos) const;
	bool collision(const PtLine &_line, STLVVec3 *_pos_pa) const;
	bool collision(const PtArc &_arc, STLVVec3 *_pos_pa) const;
	bool inCheck(const PtVector3 &_pos) const;
	bool inArc(const PtVector3& _pos, float *_dist = NULL) const;
	bool onArc(const PtVector3& _pos, float _epsilon = _EPSILON) const;
	bool incheck(STLVVec3 *_pos_pa) const;
	bool inTriCheck(const PtVector3 &_pos) const;
	bool inSegment(const PtVector3 &_pos) const;
	bool biggerHalf() const;
	float distance(const PtVector3& _pos, float *_mid_p = NULL, PtVector3 *_ret_pv3 = NULL) const;
	PtVector3 startGet() const;
	PtVector3 endGet() const;
	PtVector3 middleGet() const;
	void angleOrganize();

	PtVector3 dirGet(const PtVector3 &_pos);
	void dirGet(STLVVec3 *_dir_a);

	bool sideGet(PtArc *_arc_p, float _offset);
	int typeGet(const PtVector3 &_start, float *_diameter = NULL, float _epsilon = _EPSILON, STLVVec3 *_dir_a = NULL);
	static bool ccCheck(ARC_TYPE _t) {
		if (_t == arc_cc_large || _t == arc_cc_small)
			return true;
		return false;
	}
	
	const PtArc &operator*=(const PtMatrix4 &_m4);
	void mirrorLR();
};
//#endif // !defined(AFX_VECTOR3_H__1FBAA874_B432_4CA3_9567_F1A1EA7463C3__INCLUDED_)
