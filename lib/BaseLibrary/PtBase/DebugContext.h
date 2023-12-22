#pragma once

#define KPS_DEBUG

#include <vector>
#include <map>
#include "PTAllocator.h"
#ifndef _WIN32
#include <sys/time.h>
#endif

#ifndef __D3DX9MATH_H__
typedef struct {
	float fValue[3];
}D3DXVECTOR3;

typedef struct {
	float fValue[3];
}D3DXVECTOR2;

typedef struct {
	float fValue[4][4];
}D3DXMATRIX;
class IDirect3DDevice9;
#endif

//==========================================================================
// Start Update by ginuy : 2009-05-11 - 프로파일러

//#ifndef _WIN32
#define XDIRECTX
//#else
//#include "d3dx9math.h"// Add by OJ : 2009-05-18
//#endif

//#define PERFORMANCE_MONITORING_ON 1

#ifdef _WIN32
typedef LARGE_INTEGER tlong;
#define FORCEINLINE __forceinline
#else
typedef timeval tlong;
#define FORCEINLINE
#endif

//////////////////////////////////////////////////////////////////////////
/// Basic Performance Monitoring Stat
class C_DebugContext_BPMStat
{
public:
	float tck_tot_;   ///< 총 누적 실행 시간
	float tck_min_;   ///< 최소 실행 시간
	float tck_max_;   ///< 최대 실행 시간
	INT32 samples_;   ///< 총 호출 횟수

public:
	FORCEINLINE C_DebugContext_BPMStat() : tck_tot_(0), tck_min_(100000), tck_max_(0), samples_(0) {}
	virtual ~C_DebugContext_BPMStat() {};
	FORCEINLINE void profile( float elap )
	{
		++samples_;
		tck_tot_ += elap;
		if( elap<tck_min_ ) { tck_min_ = elap; }
		if( elap>tck_max_ ) { tck_max_ = elap; }
	}
};

//////////////////////////////////////////////////////////////////////////
/// Static Performance Monitoring Stat
class C_DebugContext_SPMStat : public C_DebugContext_BPMStat
{
public:
	explicit C_DebugContext_SPMStat( const char * name, INT32 id );

public:
	const std::string call_fn_;       ///< 함수 이름
	const int       call_id_;       ///< 할당 ID

private:
	const C_DebugContext_SPMStat & operator=( const C_DebugContext_SPMStat & ) {
        return *this;
    }
};

typedef std::vector< C_DebugContext_SPMStat* > VEC_DebugContext_StaticFCallList;

//////////////////////////////////////////////////////////////////////////
/// Dynamic Performance Monitoring Stat
class C_DebugContext_DPMStat : public C_DebugContext_BPMStat
{
public:
	float percent_;   ///< 점유율

public:
	FORCEINLINE C_DebugContext_DPMStat( int sid=-1 ) : scall_id_(sid), pparent_(0) {}
	~C_DebugContext_DPMStat() { pparent_=0; childav_.clear(); }

public:
	void report( int depth, int order, VEC_DebugContext_StaticFCallList & sfclist );

private:
	friend class C_DebugContext_PerformanceMonitor;
	typedef std::map< int, C_DebugContext_DPMStat > MAP_DebugContext_ChildAscVec;
	int         scall_id_; ///< 정적 프로파일러 아이디
	C_DebugContext_DPMStat    *pparent_;  ///< 부모 노드 포인터
	MAP_DebugContext_ChildAscVec childav_;  ///< 자식 노드
};

//////////////////////////////////////////////////////////////////////////
///
class C_DebugContext_PerformanceMonitor
{
public:
	struct DynamicProfileStarter
	{
		DynamicProfileStarter()
		{ get_perfmon()->beg_dynamic_profile(); }
		~DynamicProfileStarter()
		{ get_perfmon()->end_dynamic_profile(); }
	};
public:

	void beg_dynamic_profile();
	void end_dynamic_profile();
	void regist_flat_stat_node( C_DebugContext_SPMStat * );
	INT32 get_cur_static_id() { return (INT32)sfun_list_.size(); }
	FORCEINLINE
		void push_profile_dyna_node( int sfcall_id )
	{
		if( pdfun_cur_ )
		{
			C_DebugContext_DPMStat * pnode;
			pnode = &pdfun_cur_->childav_[sfcall_id];
			pnode->pparent_ = pdfun_cur_;
			pdfun_cur_ = pnode;
			pnode->scall_id_ = sfcall_id;
		}
	}
	FORCEINLINE
		void pop_profile_dyna_node( float elap )
	{
		if( pdfun_cur_ )
		{
			pdfun_cur_->profile( elap );
			pdfun_cur_ = pdfun_cur_->pparent_;
		}
	}

	C_DebugContext_SPMStat * get_static_stat( int sid );

public:
	static C_DebugContext_PerformanceMonitor * get_perfmon();
	void  report( const bool bFileWrite );

private:
	C_DebugContext_PerformanceMonitor();
	~C_DebugContext_PerformanceMonitor();

private:
	VEC_DebugContext_StaticFCallList sfun_list_; ///<

	C_DebugContext_DPMStat  dfun_root_;
	C_DebugContext_DPMStat *pdfun_cur_;
};

extern C_DebugContext_PerformanceMonitor * g_pefrmon;

//////////////////////////////////////////////////////////////////////////
///
class C_DebugContext_TimeProfiler
{
public:
	static void InitTimer();

	FORCEINLINE
	C_DebugContext_TimeProfiler( C_DebugContext_SPMStat& ps ) : ps_(ps), t0_( GetTicks() )
	{
		m_bDisablePerformanceCheck	= s_bDisablePerformanceCheck;

		if(!m_bDisablePerformanceCheck)
			g_pefrmon->push_profile_dyna_node( ps_.call_id_ );
	}
	FORCEINLINE
	~C_DebugContext_TimeProfiler()
	{
		if(!m_bDisablePerformanceCheck)
		{
#ifdef _WIN32
			float elap = (float)(GetTicks().QuadPart - t0_.QuadPart)/(float)tick_per_sec.QuadPart;
#else
            float elap = (float)(GetTicks().tv_sec) + (float)(GetTicks().tv_usec) / 1000000.f;
#endif
			ps_.profile( elap );
			g_pefrmon->pop_profile_dyna_node( elap );
		}
	}

	static void SetDisablePerformanceCheck(bool _bDisable)
	{
		s_bDisablePerformanceCheck	= _bDisable;
	}

private:
	const C_DebugContext_TimeProfiler & operator=( const C_DebugContext_TimeProfiler & ) {
        return *this;
    }

	FORCEINLINE
		tlong GetTicks()
	{
		tlong ticks;
#ifdef _WIN32
		QueryPerformanceCounter(&ticks);
#else
        gettimeofday(&ticks, NULL);
#endif
		return ticks;
	}


private:
	C_DebugContext_SPMStat & ps_;
	tlong     t0_;
	static tlong tick_per_sec;

	//==========================================================================
	// Start Update by OJ : 2006-07-26, 오후 5:56

	bool		m_bDisablePerformanceCheck;
	static bool	s_bDisablePerformanceCheck;

	//================================= End Update by OJ : 2006-07-26, 오후 5:56
};

#if defined(PERFORMANCE_MONITORING_ON )

#define CORUM_PROFILE_ROOT() \
	C_DebugContext_PerformanceMonitor::DynamicProfileStarter _dynamic_profile_root_
#define CORUM_PROFILE() \
	static C_DebugContext_SPMStat _static_perf_stat_(__FUNCSIG__, g_pefrmon->get_cur_static_id() ); \
	C_DebugContext_TimeProfiler _scope_time_profiler_(_static_perf_stat_)

#ifdef _WIN32
#define CORUM_PROFILE_SUB( SUB_NAME ) \
	static C_DebugContext_SPMStat _perf_stat_##SUB_NAME(#SUB_NAME" at "__FUNCSIG__, g_pefrmon->get_cur_static_id() ); \
	C_DebugContext_TimeProfiler _scope_time_profiler_##SUB_NAME(_perf_stat_##SUB_NAME)
#else
#define CORUM_PROFILE_SUB( SUB_NAME ) \
    static C_DebugContext_SPMStat _perf_stat_##SUB_NAME(__PRETTY_FUNCTION__, g_pefrmon->get_cur_static_id() ); \
    C_DebugContext_TimeProfiler _scope_time_profiler_##SUB_NAME(_perf_stat_##SUB_NAME)
#endif
#define CORUM_PROFILE_REPORT( bFileWrite ) g_pefrmon->report( bFileWrite )
#define CORUM_PROFILE_DISABLE( onoff ) C_DebugContext_TimeProfiler::SetDisablePerformanceCheck(onoff)

//#ifndef XDIRECTX
//
//#else
//#define CORUM_PROFILE_ROOT()
//#define CORUM_PROFILE()
//#define CORUM_PROFILE_REPORT()
//#define CORUM_PROFILE_SUB( SUB_NAME )
//#define CORUM_PROFILE_DISABLE( onoff )
//#endif

//////////////////////////////////////////////////////////////////////////
/* 사용법 예제 소스

-- 예제 소스 -------------------------------------------------------------
#include "profiler.hpp"
//#include <windows.h>

void func_common()
{
CORUM_PROFILE(); // 프로파일링 하고 싶은 함수의 맨위에 매크로 함수를 호출해 줍니다.
// 함수 바디에서 한번만 호출해야 합니다.
::Sleep(5);
}

void func_a_call()
{
CORUM_PROFILE();
func_common();
::Sleep(10);
}

void func_b_call()
{
CORUM_PROFILE();
func_common();
func_a_call();
func_common();
::Sleep(50);
}

void main ( void )
{
CORUM_PROFILE_ROOT(); // 동적 프로파일링의 시작부분 설정. 전체 소스에 하나만 존재해야 합니다.
// 사용하지 않으면 정적 프로파일링 정보만 표시됩니다.
for( tsi4b i=0; i<2; ++i )
{
{
CORUM_PROFILE_SUB( inner_for_1 ); // 함수 프로파일링 외에 부분적으로 사용할 경우
// 정확한 결과를 위해서는 스코핑을 해주어야 합니다.
func_common();
}
{
CORUM_PROFILE_SUB( inner_for_2 );
func_a_call();
func_b_call();
}
}
CORUM_PROFILE_REPORT(); // 최종 결과를 리포팅 합니다.
// 여러번 호출해도 되지만 마지막에 한번만 호출해주시면 됩니다.
system("PAUSE");
}

-- 결과 출력 ---------------------------------------------------------

----------------------------------------------------------------------
*-- 정적 함수 호출 프로필 5개가 등록되어 있습니다.
----------------------------------------------------------------------
int __cdecl main(void)_inner_for_1
- 호출횟수 : 2 times call
- 전체시간 : 0.016 sec
- 최소시간 : 0.000 sec
- 최대시간 : 0.016 sec
- 평균시간 : 0.008 sec
void __cdecl func_common(void)
- 호출횟수 : 10 times call
- 전체시간 : 0.080 sec
- 최소시간 : 0.000 sec
- 최대시간 : 0.016 sec
- 평균시간 : 0.008 sec
int __cdecl main(void)_inner_for_2
- 호출횟수 : 2 times call
- 전체시간 : 0.188 sec
- 최소시간 : 0.094 sec
- 최대시간 : 0.094 sec
- 평균시간 : 0.094 sec
void __cdecl func_a_call(void)
- 호출횟수 : 4 times call
- 전체시간 : 0.062 sec
- 최소시간 : 0.015 sec
- 최대시간 : 0.016 sec
- 평균시간 : 0.015 sec
void __cdecl func_b_call(void)
- 호출횟수 : 2 times call
- 전체시간 : 0.157 sec
- 최소시간 : 0.078 sec
- 최대시간 : 0.079 sec
- 평균시간 : 0.078 sec

----------------------------------------------------------------------
*-- 동적 함수 호출 프로필.
----------------------------------------------------------------------
0. 최상위 루트 노드 : 2개의 하위 프로파일.
> 점유분석 : 100% = 7.8% + 92.2% 
----------------------------------------------------------------------
.0.1. int __cdecl main(void)_inner_for_1 - 1개의 하위 프로파일.
- 호출 : 점유(7.84%) 횟수(2) 시간:총(0.016s) 최소(0.000s) 최대(0.016s) 평균(0.008s)
> 점유율 분석 : 100% = 100.0% + (0.0%)
----------------------------------------------------------------------
..0.1.1. void __cdecl func_common(void) - 0개의 하위 프로파일.
- 호출 : 점유(100.00%) 횟수(2) 시간:총(0.016s) 최소(0.000s) 최대(0.016s) 평균(0.008s)
> 점유율 분석 : 100% = (100.0%)
----------------------------------------------------------------------
.0.2. int __cdecl main(void)_inner_for_2 - 2개의 하위 프로파일.
- 호출 : 점유(92.16%) 횟수(2) 시간:총(0.188s) 최소(0.094s) 최대(0.094s) 평균(0.094s)
> 점유율 분석 : 100% = 16.5% + 83.5% + (0.0%)
----------------------------------------------------------------------
..0.2.1. void __cdecl func_a_call(void) - 1개의 하위 프로파일.
- 호출 : 점유(16.49%) 횟수(2) 시간:총(0.031s) 최소(0.015s) 최대(0.016s) 평균(0.016s)
> 점유율 분석 : 100% = 51.6% + (48.4%)
----------------------------------------------------------------------
...0.2.1.1. void __cdecl func_common(void) - 0개의 하위 프로파일.
- 호출 : 점유(51.61%) 횟수(2) 시간:총(0.016s) 최소(0.000s) 최대(0.016s) 평균(0.008s)
> 점유율 분석 : 100% = (100.0%)
----------------------------------------------------------------------
..0.2.2. void __cdecl func_b_call(void) - 2개의 하위 프로파일.
- 호출 : 점유(83.51%) 횟수(2) 시간:총(0.157s) 최소(0.078s) 최대(0.079s) 평균(0.079s)
> 점유율 분석 : 100% = 20.4% + 19.7% + (59.9%)
----------------------------------------------------------------------
...0.2.2.1. void __cdecl func_common(void) - 0개의 하위 프로파일.
- 호출 : 점유(20.38%) 횟수(4) 시간:총(0.032s) 최소(0.000s) 최대(0.016s) 평균(0.008s)
> 점유율 분석 : 100% = (100.0%)
----------------------------------------------------------------------
...0.2.2.2. void __cdecl func_a_call(void) - 1개의 하위 프로파일.
- 호출 : 점유(19.75%) 횟수(2) 시간:총(0.031s) 최소(0.015s) 최대(0.016s) 평균(0.016s)
> 점유율 분석 : 100% = 51.6% + (48.4%)
----------------------------------------------------------------------
....0.2.2.2.1. void __cdecl func_common(void) - 0개의 하위 프로파일.
- 호출 : 점유(51.61%) 횟수(2) 시간:총(0.016s) 최소(0.000s) 최대(0.016s) 평균(0.008s)
> 점유율 분석 : 100% = (100.0%)
----------------------------------------------------------------------
*/
//================================= End Update by ginuy : 2009-05-11
#else
#define CORUM_PROFILE_ROOT()
#define CORUM_PROFILE()
#define CORUM_PROFILE_REPORT()
#define CORUM_PROFILE_SUB( SUB_NAME )
#define CORUM_PROFILE_DISABLE( onoff )
#endif

class DebugContext
{
public:
	DebugContext(void);
	~DebugContext(void);

	//==========================================================================
	// Start Update by OJ : 2006-07-11, 오후 5:55
	// Debug하기 위한 Visualization 
//#ifdef KPS_DEBUG
public:
	typedef struct __EVENT_LOG__{
		UINT32	nMsg;
		UINT32	nParam1;
		UINT32	nParam2;
		UINT32	nCount;		// 입력된 횟 수
		UINT32	nTime;		// 최종 입력된 시간
		UINT32	nTimeMin;	// 가장 빠른 시간 기록
		UINT32	nTimeMax;	// 가장 긴 시간 기록
	}STEventLog;
	typedef std::vector<STEventLog>	STLVstEventLog;
	typedef std::map<UINT32, STLVstEventLog*>	STLMnpstlVstEventLog;
	int PushWinMsgLog(UINT32 _nMsg, UINT32 _wParam, INT32 _lParam);
	void PushEventLog(UINT32 _nMsg, UINT32 _nParam1, unsigned _nParam2);
	void ClearEventLog();
	bool ReportEventLog(const char *_strExt );
	void SetLocalIdentity(const char *_strIdentity);
	void SetIDDungeon(UINT32 _nID);

	friend DebugContext &GetDebugContext();

	void	IntegrityTestSetData(void *_pData, size_t _nSize);
	void	*IntegrityTestGetData();
	void	IntegrityTestCompare(void *_pData);

	int		GetTestMode();
	void	SetTestMode(int _nIndex);

	void	SetState(int _nState);
	int		GetState();

	void	SetEnable(bool _bEnable);
	bool	GetEnable();

	void	Init(int _nScreenWidth, int _nScreenHeight);
	void	Release();

	void	OffTrackmode();
	void	OnTrackmode();

	void Log(const char *_strLogFile, char *_strMsg, ...);

#ifndef XDIRECTX
	bool	WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	typedef std::vector<D3DXVECTOR3, PT_allocator<D3DXVECTOR3> >				STLVDxVector3;
	typedef std::vector<UINT32, PT_allocator<UINT32> >			STLVuInt;
	//==========================================================================
	// Start Update by OJ : 2010-05-31
public:
	void	SetMatrix(const D3DXMATRIX _mMatrix);

	void	ClearVertexs();
	void	ClearLines();
	void	ClearLinesImmedate();
	void	ClearAll();
	void	ClearAllStatic();
	void	ClearVertexsStatic();
	void	ClearLinesStatic();

	void	AddLineStrip(const D3DXVECTOR3 *_pVertexs, int _nNumOfVertex, const D3DXVECTOR3 &v3Color);
	void	AddFrame(const D3DXMATRIX &_m4, const D3DXVECTOR3 &v3Color, const float &fScale = 1);
	void	AddBoundBox(const D3DXVECTOR3 &v3Min, const D3DXVECTOR3 &v3Max, const D3DXVECTOR3 &v3Color);
	void	AddLine(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3To, const D3DXVECTOR3 &v3Color);
	void	AddDirectionLine(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3Direction, const D3DXVECTOR3 &v3Color);
	void	AddVertex(const D3DXVECTOR3 &v3Vertex, const D3DXVECTOR3 &v3Color);
	void	AddFrameStatic(const D3DXMATRIX &_m4, const D3DXVECTOR3 &v3Color, const float &fScale = 1);
	void	AddLineStatic(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3To, const D3DXVECTOR3 &v3Color);
	void	AddDirectionLineStatic(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3Direction, const D3DXVECTOR3 &v3Color);
	void	AddVertexStatic(const D3DXVECTOR3 &v3Vertex, const D3DXVECTOR3 &v3Color);
	void	LoadFrames(const char *_strFrames);

	D3DXMATRIX	GetCamera();
	void SetViewCamera(D3DXMATRIX *_pm4View);
	void SetCamera(D3DXMATRIX _m4Camera);
	void SetCameraMode(bool _bCameraMode);
	bool GetCameraMode();

	void	DrawPrimitivies(IDirect3DDevice9* _pD3DDevice, const D3DXMATRIX &_m4Projection, const D3DXMATRIX &_m4View);
	
	void	AddSprite(int *_pnBoundBox, const D3DXVECTOR3 &v3Color);
	void	AddSprite(const D3DXMATRIX &_m4, int *_pnBoundBox, const D3DXVECTOR3 &v3Color);

protected:
	STLVDxVector3		*m_pstlVVertexs;
	STLVDxVector3		*m_pstlVVertexColors;
	STLVuInt			*m_pstlVVertexTimes;

	STLVDxVector3		*m_pstlVLines;
	STLVDxVector3		*m_pstlVLineColors;
	STLVuInt			*m_pstlVLineTimes;

	STLVDxVector3		*m_pstlVLineSprites;
	STLVDxVector3		*m_pstlVLineSpriteColors;
	STLVuInt			*m_pstlVLineSpriteTimes;

	STLVDxVector3		*m_pstlVVertexsStatic;
	STLVDxVector3		*m_pstlVVertexColorsStatic;
	STLVDxVector3		*m_pstlVLinesStatic;
	STLVDxVector3		*m_pstlVLineColorsStatic;

	D3DXMATRIX			m_m4Camera;
#endif
	bool				m_bCameraView;
	//================================= End Update by OJ : 2010-05-31

protected:
//#ifdef D3DX_PI // D3DX_PI를 사용하지 않으면 DX관련 함수나 Define을 사용할 수 없다.
	STLMnpstlVstEventLog		m_stlMnpstlVstEventLog;
	char				m_strLocalIdentity[255];
	UINT32		m_nTimeReportLoopCount;
	UINT32		m_nTimeReportBefor;
	UINT32		m_nCountEvent;
	UINT32		m_nCountEventMove;
	UINT32		m_nCountKeyType;
	UINT32		m_nIDDungeon;
	int					m_nCntMacroDetect;
#ifndef XDIRECTX
	D3DXMATRIX	m_m4Matrix;

	void NevZoom(int nDelta);
	void NevPan(int nDx, int nDy);
	void NevExaminer(int nDx, int nDy);
	void NevSphere(int nDx, int nDy);
	D3DXVECTOR3 GetEarthPosition(D3DXVECTOR3 _v3Normal, D3DXVECTOR3  _v3Pos);
	int nCalcCamEnv();

	D3DXVECTOR3	g_v3Pointed;
	D3DXVECTOR3	g_v3Distance;
	D3DXVECTOR2	g_v2MouseOldPos;

	D3DXMATRIX	*m_pm4View;
#endif

	int			m_nScreenWidth, m_nScreenHeight;
	int			m_nTrackmode;
	int			m_nState;
	bool		m_bDraw;

	bool		g_bMouseMove;
	bool		g_bMouseButtonLeft;
	bool		g_bMouseButtonRight;
	bool		g_bMouseButtonMiddle;
	bool		m_bKeyAlt;
	bool		m_bKeyShift;

	// Resource 
public:
//#ifdef KPS_DEBUG
protected:

	int				m_nTestMode;
//#endif

	//================================= End Update by OJ : 2006-07-11, 오후 5:55
};
