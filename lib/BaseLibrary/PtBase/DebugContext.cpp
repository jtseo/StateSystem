
#define XDIRECTX

#include "stdafx.h"

#include "BaseMemoryPool.h"
#include "BaseObject.h"
#include "BaseFile.h"
#include "BaseSocket.h"
#include "DebugContext.h"
#include <assert.h>

#pragma warning ( push )

#ifndef XDIRECTX
#include "ZipLib/include/Zipper.h"
#endif
//==========================================================================
// Start Update by ginuy : 2009-05-11 - 프로파일러

C_DebugContext_PerformanceMonitor * g_pefrmon( C_DebugContext_PerformanceMonitor::get_perfmon() );
tlong C_DebugContext_TimeProfiler::tick_per_sec;

//////////////////////////////////////////////////////////////////////////

void F_DebugContext_DebugOut( const char* _strMessage, const bool bFileWrite = true, const char* FileName = NULL )
{
	if ( bFileWrite )
	{
		if ( 0 == FileName || 0 == FileName[ 0 ] )
			return;

		FILE* fp = NULL;

		char szPath[MAX_PATH];
        BaseSystem::module_get_name(szPath, MAX_PATH);
		char* p = strrchr(szPath, '\\');   
		szPath[p - szPath] = 0;
		strcat_s( szPath, MAX_PATH, "\\" );
		strcat_s( szPath, MAX_PATH, FileName );

		if( 0 == fopen_s( &fp, szPath, "at+" ) ) 
		{
			fprintf( fp, "%s", _strMessage );

			fclose( fp );
		}
	}
	else
	{
        puts(_strMessage);
		//OutputDebugStringA(_strMessage);
	}


	//g_kpext.IPCTrace(_strMessage);
}

bool C_DebugContext_TimeProfiler::s_bDisablePerformanceCheck = false;

void C_DebugContext_TimeProfiler::InitTimer()
{
#ifdef _WIN32
	if( 0 == QueryPerformanceFrequency(&tick_per_sec) )
	{
		assert( !"QueryPerformanceFrequency() - Timer not supported" );
		exit(0);
	}
#else
    gettimeofday(&tick_per_sec, NULL);
#endif
}

//////////////////////////////////////////////////////////////////////////

C_DebugContext_PerformanceMonitor::C_DebugContext_PerformanceMonitor()
: pdfun_cur_(0)
{
	C_DebugContext_TimeProfiler::InitTimer();
}

C_DebugContext_PerformanceMonitor::~C_DebugContext_PerformanceMonitor()
{
}

C_DebugContext_PerformanceMonitor * C_DebugContext_PerformanceMonitor::get_perfmon()
{
	static C_DebugContext_PerformanceMonitor s_pmon;
	return & s_pmon;
}

void C_DebugContext_PerformanceMonitor::beg_dynamic_profile()
{
	assert( 0==pdfun_cur_ && "KP_PROFILE_ROOT(); duplicated." );
	pdfun_cur_ = &dfun_root_;
}
void C_DebugContext_PerformanceMonitor::end_dynamic_profile()
{
	assert( &dfun_root_==pdfun_cur_ && "KP_PROFILE_ROOT(); duplicated." );
	pdfun_cur_ = 0;
}

void C_DebugContext_PerformanceMonitor::regist_flat_stat_node( C_DebugContext_SPMStat * pps )
{
	sfun_list_.push_back( pps );
}

void C_DebugContext_PerformanceMonitor::report( const bool bFileWrite )
{
    SPtDateTime dateTime;
    BaseSystem::timeCurrent(&dateTime);
	
	char FileName[ MAX_PATH ] = "";
	sprintf_s( FileName, MAX_PATH, "%d%d%d%d%d%d.txt", dateTime.s.sDate.s.year, dateTime.s.sDate.s.month, dateTime.s.sDate.s.day,
		dateTime.s.sTime.s.nHour, dateTime.s.sTime.s.nMinute, dateTime.s.sTime.s.nSecond);

	int cnt = (int)sfun_list_.size();
	char buff[512];
	F_DebugContext_DebugOut( "----------------------------------------------------------------------\n", bFileWrite, FileName );
	sprintf_s( buff, 512, "*-- static function profile registed #%d.\n", cnt );
	F_DebugContext_DebugOut( buff, bFileWrite, FileName );
	F_DebugContext_DebugOut( "----------------------------------------------------------------------\n", bFileWrite, FileName );


	char TempStr[ MAX_PATH ];
	sprintf_s( TempStr, MAX_PATH, "PerformanceMonitorcnt  = %d\r\n", cnt );
    puts(TempStr);
	//OutputDebugStringA( TempStr );


	for( size_t i = 0; i<cnt; ++i )
	{		
		C_DebugContext_SPMStat & spmstat = * sfun_list_[i];
		sprintf_s( buff, 512, "%s\n", spmstat.call_fn_.c_str() );
		F_DebugContext_DebugOut( buff, bFileWrite, FileName );
		sprintf_s( buff, 512, "- total (%.4fs) / call (%d call) = average (%.4fs)\n",
			spmstat.tck_tot_, spmstat.samples_, spmstat.tck_tot_/spmstat.samples_ );
		F_DebugContext_DebugOut( buff, bFileWrite, FileName );
		sprintf_s( buff, 512, "- min(%.4fs),  max(%.4fs)",
			spmstat.tck_min_, spmstat.tck_max_ );
		F_DebugContext_DebugOut( buff, bFileWrite, FileName );
		F_DebugContext_DebugOut( "\n----------------------------------------------------------------------\n", bFileWrite, FileName );
	}
	F_DebugContext_DebugOut( "\n----------------------------------------------------------------------\n", bFileWrite, FileName );
	if( 0<dfun_root_.childav_.size() ) dfun_root_.report( 0, 0, sfun_list_ );
	F_DebugContext_DebugOut( "----------------------------------------------------------------------\n\n", bFileWrite, FileName );
}

//////////////////////////////////////////////////////////////////////////

C_DebugContext_SPMStat::C_DebugContext_SPMStat( const char * name, INT32 id )
: call_fn_(name), call_id_((int)id)
{
	g_pefrmon->regist_flat_stat_node( this );
}

void C_DebugContext_DPMStat::report( int depth, int order, VEC_DebugContext_StaticFCallList & sfclist )
{
	static char buff[2048]={0};
	static char wspc[512]={0};
	static std::vector<int> nque;
	int cnt = (int)childav_.size();
	MAP_DebugContext_ChildAscVec::iterator itr = childav_.begin();
	MAP_DebugContext_ChildAscVec::iterator itrNext;
	MAP_DebugContext_ChildAscVec::iterator end = childav_.end();
	float ftot_tck(0);

	if( -1 == scall_id_ )
	{
		sprintf_s( buff, 2048, "*-- dynamic call profile report.\n" );
		F_DebugContext_DebugOut( buff );
		MAP_DebugContext_ChildAscVec::iterator itr2;
		MAP_DebugContext_ChildAscVec::iterator end2 = childav_.end();
		F_DebugContext_DebugOut( "----------------------------------------------------------------------\n" );
		sprintf_s( buff, 2048, "0. top root node : childs %d \n", cnt );
		F_DebugContext_DebugOut( buff );
		tck_tot_ = 0;
		if( 1>=cnt )
		{
			sprintf_s( buff, 2048, "> occupy analysis >=cnt\n" );
			F_DebugContext_DebugOut( buff );
		}
		else
		{
			sprintf_s( buff, 2048, "> occupy analysis : 100%% =" );
			F_DebugContext_DebugOut( buff );

			itr2 = childav_.begin();
			for( ; itr2 != end2; ++itr2 )
			{		


				tck_tot_ += itr2->second.tck_tot_;
				ftot_tck += itr2->second.tck_tot_;
			}
			float ftck;
			itr2 = childav_.begin();
			for( ; itr2 != end2; ++itr2 )
			{

				ftck = (float)itr2->second.tck_tot_;
				itrNext = itr2;
				sprintf_s( buff, 2048, " %.1f%% %s", (100.f*ftck)/ftot_tck, ((++itrNext) != end2 ? "+" : "") );
				F_DebugContext_DebugOut( buff );
			}
			F_DebugContext_DebugOut( "\n" );
		}
	}
	else
	{
		assert( scall_id_ < (int)sfclist.size() );
		C_DebugContext_SPMStat & snode = * sfclist[scall_id_];
		F_DebugContext_DebugOut( "----------------------------------------------------------------------\n" );

		memset( wspc, '.', depth ); wspc[depth] = 0;
		F_DebugContext_DebugOut( wspc );
		memset( wspc, ' ', depth );

		{
			std::vector<int>::iterator ibeg, iend=nque.end();
			for( ibeg=nque.begin(); ibeg!=iend; ++ibeg )
			{				
				sprintf_s( buff, 2048, "%d.", *ibeg );
				F_DebugContext_DebugOut( buff );
			}
		}

		sprintf_s( buff, 2048, "%d. %s\n", order, snode.call_fn_.c_str() );
		F_DebugContext_DebugOut( buff );

		{
			sprintf_s( buff, 2048, "%s- tot(%.4fs) / call (%d) = average (%.4fs)\n",
				wspc, tck_tot_, samples_, tck_tot_/samples_ );
			F_DebugContext_DebugOut( buff );
			//sprintf( buff, "%s- 호출 : 점유(%.2f%%) 횟수(%d)\n",
			//	wspc, (100.f*tck_tot_)/pparent_->tck_tot_, samples_ );
			//F_DebugContext_DebugOut( buff );
		}
		{
			sprintf_s( buff, 2048, "%s- min(%.4fs),  max(%.4fs)\n",
				wspc, tck_min_, tck_max_ );
			F_DebugContext_DebugOut( buff );
		}
		{
			float ftck;
			sprintf_s( buff, 2048, "%s> child call (%2d) :", wspc, cnt );
			F_DebugContext_DebugOut( buff );

			itr = childav_.begin();
			for( ; itr != end; ++itr )
			{

				ftck = (float)itr->second.tck_tot_;
				ftot_tck += ftck;
				itrNext = itr;
				sprintf_s( buff, 2048, " %.2f%% %s", (100.f*ftck)/tck_tot_, ((++itrNext) != end ? "+" : "") );
				F_DebugContext_DebugOut( buff );
			}
			sprintf_s( buff, 2048, "%s [%.2f%%]\n", 0<cnt ? "+" : "", 100.f*(tck_tot_-ftot_tck)/tck_tot_ );
			F_DebugContext_DebugOut( buff );
		}
	}

	nque.push_back( order );
	itr = childav_.begin();
	for( int i=1; itr != end; ++i, ++itr )
	{

		itr->second.report( depth+1, i, sfclist );
	}
	nque.pop_back();
}
#ifndef XDIRECTX

#endif
#pragma warning ( pop )
//================================= End Update by ginuy : 2009-05-11

DebugContext g_debugContext;

DebugContext::DebugContext(void)
{
	m_nTrackmode	= 1;
	m_nState	= 0;
	m_bDraw	= false;

	m_nTestMode = 0;
#ifndef XDIRECTX
	m_pstlVVertexs		= NULL;
	m_pstlVVertexColors	= NULL;
	m_pstlVVertexTimes	= NULL;

	m_pstlVLines			= NULL;
	m_pstlVLineColors		= NULL;
	m_pstlVLineTimes		= NULL;

	m_pstlVLineSprites		= NULL;
	m_pstlVLineSpriteColors	= NULL;
	m_pstlVLineSpriteTimes	= NULL;
	m_pm4View	= NULL;
#endif
	g_bMouseMove			= FALSE;
	g_bMouseButtonLeft		= FALSE;
	g_bMouseButtonRight		= FALSE;
	g_bMouseButtonMiddle	= FALSE;

	m_bKeyAlt	= FALSE;
	m_bKeyShift	= FALSE;
}

void DebugContext::SetTestMode(int _nIndex)
{
	m_nTestMode	= _nIndex;
}

int	DebugContext::GetTestMode()
{
	return m_nTestMode;
}

DebugContext::~DebugContext(void)
{
}

DebugContext &GetDebugContext()
{
	return g_debugContext;
}

//////////////////////////////////////////////////////////////////////////
//#ifdef KPS_DEBUG

	void DebugContext::SetState(int _nState)
	{
		m_nState	= _nState;
	}

	int DebugContext::GetState()
	{
		return m_nState;
	}
#ifndef XDIRECTX
	void DebugContext::ClearVertexs()
	{
		m_pstlVVertexColors->clear();
		m_pstlVVertexs->clear();
	}


	void DebugContext::ClearLines()
	{
//#ifdef KP_SINGLE_TEST
		//size_t nSize = m_pstlVLineColors->size();
		//UINT32 nTime = BaseSystem::timeGetTime();
		//for(unsigned i=0; i<nSize; i++)
		//{
		//	if(nTime > (*m_pstlVLineTimes)[i] + 300)
		//	{	
		//		(*m_pstlVLineColors).erase(m_pstlVLineColors->begin() + i);
		//		(*m_pstlVLines).erase(m_pstlVLines->begin() + i*2,
		//			m_pstlVLines->begin() + i*2+2);
		//		(*m_pstlVLineTimes).erase(m_pstlVLineTimes->begin() + i);
		//		i--;
		//		nSize--;
		//	}else{
		//		(*m_pstlVLineColors)[i] *= 0.99f;
		//	}
		//}
//#else
		m_pstlVLineColors->clear();
		m_pstlVLines->clear();
		m_pstlVLineTimes->clear();

		m_pstlVLineSprites->clear();
		m_pstlVLineSpriteColors->clear();
		m_pstlVLineSpriteTimes->clear();
//#endif
	}

	void DebugContext::ClearLinesImmedate()
	{
		m_pstlVLineColors->clear();
		m_pstlVLines->clear();
		m_pstlVLineTimes->clear();

		m_pstlVLineSprites->clear();
		m_pstlVLineSpriteColors->clear();
		m_pstlVLineSpriteTimes->clear();
	}

	void DebugContext::ClearAll()
	{
		ClearVertexs();
		ClearLines();
	}

	void DebugContext::ClearVertexsStatic()
	{
		m_pstlVVertexColorsStatic->clear();
		m_pstlVVertexsStatic->clear();
	}

	void DebugContext::ClearLinesStatic()
	{
		m_pstlVLineColorsStatic->clear();
		m_pstlVLinesStatic->clear();
	}

	void DebugContext::ClearAllStatic()
	{
		ClearVertexsStatic();
		ClearLinesStatic();
	}

	void DebugContext::AddLine(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3To, const D3DXVECTOR3 &v3Color)
	{
		D3DXVECTOR3	v3Temp;
		if(m_nTrackmode == 1 && m_bDraw)
		{
			D3DXVec3TransformCoord(&v3Temp, &v3From, &m_m4Matrix);
			m_pstlVLines->push_back(v3Temp);
			D3DXVec3TransformCoord(&v3Temp, &v3To, &m_m4Matrix);
			m_pstlVLines->push_back(v3Temp);
			m_pstlVLineColors->push_back(v3Color);
			m_pstlVLineTimes->push_back(BaseSystem::timeGetTime());
		}
	}

	void DebugContext::AddLineStrip(const D3DXVECTOR3 *_pVertexs, int _nNumOfVertex, const D3DXVECTOR3 &v3Color)
	{
		if(_nNumOfVertex < 2)
			return;

		for(int i=1; i<_nNumOfVertex; i++)
		{
			AddLine(*(_pVertexs+i-1), *(_pVertexs+i), v3Color);
		}

		if(_nNumOfVertex > 2)
			AddLine(*_pVertexs, *(_pVertexs+(_nNumOfVertex-1)), v3Color);
	}

	void DebugContext::AddDirectionLine(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3Direction, const D3DXVECTOR3 &v3Color)
	{
		D3DXVECTOR3	v3Temp;
		v3Temp	= v3From + v3Direction;
		AddLine(v3From, v3Temp, v3Color);
	}

	void DebugContext::AddVertex(const D3DXVECTOR3 &v3Vertex, const D3DXVECTOR3 &v3Color)
	{
		D3DXVECTOR3	v3Temp;

		if(m_nTrackmode == 1 && m_bDraw)
		{
			D3DXVec3TransformCoord(&v3Temp, &v3Vertex, &m_m4Matrix);
			m_pstlVVertexs->push_back(v3Temp);
			m_pstlVVertexColors->push_back(v3Color);
		}
	}

	void DebugContext::AddFrame(const D3DXMATRIX &_m4, const D3DXVECTOR3 &v3Color, const float &fScale)
	{
		D3DXVECTOR3	v3[4];

		memcpy(&v3[0].x, &_m4._11, sizeof(float)*3);
		memcpy(&v3[1].x, &_m4._21, sizeof(float)*3);
		memcpy(&v3[2].x, &_m4._31, sizeof(float)*3);
		memcpy(&v3[3].x, &_m4._41, sizeof(float)*3);

		AddDirectionLine(v3[3], v3[0]*fScale, D3DXVECTOR3(1, 0, 0));
		AddDirectionLine(v3[3], v3[1]*fScale, D3DXVECTOR3(0, 1, 0));
		AddDirectionLine(v3[3], v3[2]*fScale, D3DXVECTOR3(0, 0, 1));
		AddVertex(v3[3], v3Color);
	}

	void DebugContext::AddFrameStatic(const D3DXMATRIX &_m4, const D3DXVECTOR3 &v3Color, const float &fScale)
	{
		D3DXVECTOR3	v3[4];

		memcpy(&v3[0].x, &_m4._11, sizeof(float)*3);
		memcpy(&v3[1].x, &_m4._21, sizeof(float)*3);
		memcpy(&v3[2].x, &_m4._31, sizeof(float)*3);
		memcpy(&v3[3].x, &_m4._41, sizeof(float)*3);

		AddDirectionLineStatic(v3[3], v3[0]*fScale, D3DXVECTOR3(1, 0, 0));
		AddDirectionLineStatic(v3[3], v3[1]*fScale, D3DXVECTOR3(0, 1, 0));
		AddDirectionLineStatic(v3[3], v3[2]*fScale, D3DXVECTOR3(0, 0, 1));
		AddVertexStatic(v3[3], v3Color);
	}

	void DebugContext::AddLineStatic(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3To, const D3DXVECTOR3 &v3Color)
	{
		D3DXVECTOR3	v3Temp;
		if(m_nTrackmode == 1 && m_bDraw)
		{
			D3DXVec3TransformCoord(&v3Temp, &v3From, &m_m4Matrix);
			m_pstlVLinesStatic->push_back(v3Temp);
			D3DXVec3TransformCoord(&v3Temp, &v3To, &m_m4Matrix);
			m_pstlVLinesStatic->push_back(v3Temp);
			m_pstlVLineColorsStatic->push_back(v3Color);
		}
	}

	void DebugContext::AddDirectionLineStatic(const D3DXVECTOR3 &v3From, const D3DXVECTOR3 &v3Direction, const D3DXVECTOR3 &v3Color)
	{
		D3DXVECTOR3	v3Temp;
		v3Temp	= v3From + v3Direction;
		AddLineStatic(v3From, v3Temp, v3Color);
	}

	void DebugContext::AddVertexStatic(const D3DXVECTOR3 &v3Vertex, const D3DXVECTOR3 &v3Color)
	{
		D3DXVECTOR3	v3Temp;

		if(m_nTrackmode == 1 && m_bDraw)
		{
			D3DXVec3TransformCoord(&v3Temp, &v3Vertex, &m_m4Matrix);
			m_pstlVVertexsStatic->push_back(v3Temp);
			m_pstlVVertexColorsStatic->push_back(v3Color);
		}
	}

	void DebugContext::AddSprite(int *_pnBoundBox, const D3DXVECTOR3 &v3Color)
	{
		if(m_nTrackmode == 1 && m_bDraw)
		{
			D3DXVECTOR3	v3Pos[8];
			v3Pos[0].x	= (float)_pnBoundBox[0];
			v3Pos[0].y	= (float)_pnBoundBox[1];
			v3Pos[0].z	= 0;
			v3Pos[1].x	= (float)_pnBoundBox[2];
			v3Pos[1].y	= (float)_pnBoundBox[1];
			v3Pos[1].z	= 0;

			v3Pos[2].x	= (float)_pnBoundBox[2];
			v3Pos[2].y	= (float)_pnBoundBox[1];
			v3Pos[2].z	= 0;
			v3Pos[3].x	= (float)_pnBoundBox[2];
			v3Pos[3].y	= (float)_pnBoundBox[3];
			v3Pos[3].z	= 0;

			v3Pos[4].x	= (float)_pnBoundBox[2];
			v3Pos[4].y	= (float)_pnBoundBox[3];
			v3Pos[4].z	= 0;
			v3Pos[5].x	= (float)_pnBoundBox[0];
			v3Pos[5].y	= (float)_pnBoundBox[3];
			v3Pos[5].z	= 0;

			v3Pos[6].x	= (float)_pnBoundBox[0];
			v3Pos[6].y	= (float)_pnBoundBox[3];
			v3Pos[6].z	= 0;
			v3Pos[7].x	= (float)_pnBoundBox[0];
			v3Pos[7].y	= (float)_pnBoundBox[1];
			v3Pos[7].z	= 0;

			for(int i=0; i<8; i++)
			{
				m_pstlVLineSprites->push_back(v3Pos[i]);
				m_pstlVLineSpriteColors->push_back(v3Color);
				m_pstlVLineSpriteTimes->push_back(BaseSystem::timeGetTime());
			}
		}
	}

	void DebugContext::AddSprite(const D3DXMATRIX &_m4, int *_pnBoundBox, const D3DXVECTOR3 &v3Color)
	{
		if(m_nTrackmode == 1 && m_bDraw)
		{
			D3DXVECTOR3	v3Pos[8];
			v3Pos[0].x	= (float)_pnBoundBox[0];
			v3Pos[0].y	= (float)_pnBoundBox[1];
			v3Pos[0].z	= 0;
			v3Pos[1].x	= (float)_pnBoundBox[2];
			v3Pos[1].y	= (float)_pnBoundBox[1];
			v3Pos[1].z	= 0;

			v3Pos[2].x	= (float)_pnBoundBox[2];
			v3Pos[2].y	= (float)_pnBoundBox[1];
			v3Pos[2].z	= 0;
			v3Pos[3].x	= (float)_pnBoundBox[2];
			v3Pos[3].y	= (float)_pnBoundBox[3];
			v3Pos[3].z	= 0;

			v3Pos[4].x	= (float)_pnBoundBox[2];
			v3Pos[4].y	= (float)_pnBoundBox[3];
			v3Pos[4].z	= 0;
			v3Pos[5].x	= (float)_pnBoundBox[0];
			v3Pos[5].y	= (float)_pnBoundBox[3];
			v3Pos[5].z	= 0;

			v3Pos[6].x	= (float)_pnBoundBox[0];
			v3Pos[6].y	= (float)_pnBoundBox[3];
			v3Pos[6].z	= 0;
			v3Pos[7].x	= (float)_pnBoundBox[0];
			v3Pos[7].y	= (float)_pnBoundBox[1];
			v3Pos[7].z	= 0;

			for(int i=0; i<8; i++)
			{
				D3DXVec3TransformCoord(&v3Pos[i], &v3Pos[i], &_m4);

				m_pstlVLineSprites->push_back(v3Pos[i]);
				m_pstlVLineSpriteColors->push_back(v3Color);
				m_pstlVLineSpriteTimes->push_back(BaseSystem::timeGetTime());
			}
		}
	}

	void DebugContext::AddBoundBox(const D3DXVECTOR3 &_v3Min, const D3DXVECTOR3 &_v3Max, const D3DXVECTOR3 &v3Color)
	{
		D3DXVECTOR3	v3Pos[8];

		v3Pos[0]	= _v3Min;
		v3Pos[1]	= D3DXVECTOR3(_v3Max.x, _v3Min.y, _v3Min.z);
		v3Pos[2]	= D3DXVECTOR3(_v3Min.x, _v3Max.y, _v3Min.z);
		v3Pos[3]	= D3DXVECTOR3(_v3Min.x, _v3Min.y, _v3Max.z);

		v3Pos[4]	= _v3Max;
		v3Pos[5]	= D3DXVECTOR3(_v3Min.x, _v3Max.y, _v3Max.z);
		v3Pos[6]	= D3DXVECTOR3(_v3Max.x, _v3Min.y, _v3Max.z);
		v3Pos[7]	= D3DXVECTOR3(_v3Max.x, _v3Max.y, _v3Min.z);

		AddLine(v3Pos[0], v3Pos[1], v3Color);
		AddLine(v3Pos[1], v3Pos[7], v3Color);
		AddLine(v3Pos[2], v3Pos[7], v3Color);
		AddLine(v3Pos[0], v3Pos[2], v3Color);

		AddLine(v3Pos[4], v3Pos[5], v3Color);
		AddLine(v3Pos[5], v3Pos[3], v3Color);
		AddLine(v3Pos[6], v3Pos[3], v3Color);
		AddLine(v3Pos[6], v3Pos[4], v3Color);

		AddLine(v3Pos[0], v3Pos[3], v3Color);
		AddLine(v3Pos[1], v3Pos[6], v3Color);
		AddLine(v3Pos[2], v3Pos[5], v3Color);
		AddLine(v3Pos[4], v3Pos[7], v3Color);
	}

	//void DebugContext::AddOBB(const KPS_OBB &_obb, const D3DXVECTOR3 &v3Color)
	//{
	//	D3DXVECTOR3	v3Depth, v3Up, v3Cross;

	//	D3DXVECTOR3	v3Pos[8];

	//	v3Depth	= _obb.vDepth;
	//	v3Up	= _obb.vUp;
	//	D3DXVec3Cross(&v3Cross, &v3Depth, &v3Up);

	//	v3Depth	= v3Depth * _obb.vScale.z;
	//	v3Up	= v3Up * _obb.vScale.y;
	//	v3Cross	= v3Cross * _obb.vScale.x;

	//	v3Pos[0]	= _obb.vPos + v3Depth + v3Up + v3Cross;
	//	v3Pos[1]	= _obb.vPos + v3Depth + v3Up - v3Cross;
	//	v3Pos[2]	= _obb.vPos + v3Depth - v3Up - v3Cross;
	//	v3Pos[3]	= _obb.vPos + v3Depth - v3Up + v3Cross;

	//	v3Pos[4]	= _obb.vPos - v3Depth + v3Up + v3Cross;
	//	v3Pos[5]	= _obb.vPos - v3Depth + v3Up - v3Cross;
	//	v3Pos[6]	= _obb.vPos - v3Depth - v3Up - v3Cross;
	//	v3Pos[7]	= _obb.vPos - v3Depth - v3Up + v3Cross;

	//	AddVertex(_obb.vPos, v3Color);

	//	AddLine(v3Pos[0], v3Pos[1], v3Color);
	//	AddLine(v3Pos[1], v3Pos[2], v3Color);
	//	AddLine(v3Pos[2], v3Pos[3], v3Color);
	//	AddLine(v3Pos[3], v3Pos[0], v3Color);

	//	AddLine(v3Pos[4], v3Pos[5], v3Color);
	//	AddLine(v3Pos[5], v3Pos[6], v3Color);
	//	AddLine(v3Pos[6], v3Pos[7], v3Color);
	//	AddLine(v3Pos[7], v3Pos[4], v3Color);

	//	AddLine(v3Pos[0], v3Pos[4], v3Color);
	//	AddLine(v3Pos[1], v3Pos[5], v3Color);
	//	AddLine(v3Pos[2], v3Pos[6], v3Color);
	//	AddLine(v3Pos[3], v3Pos[7], v3Color);

	//	AddDirectionLine(_obb.vPos, v3Depth, v3Color);
	//}
//#endif

//const std::vector<D3DXVECTOR3> &DebugContext::GetLines()
//{
//	return *m_pstlVLines;
//}
//
//const std::vector<D3DXVECTOR3> &DebugContext::GetLineColors()
//{
//	return *m_pstlVLineColors;
//}
//
//const std::vector<D3DXVECTOR3> &DebugContext::GetVertexs()
//{
//	return *m_pstlVVertexs;
//}
//
//const std::vector<D3DXVECTOR3> &DebugContext::GetVertexColors()
//{
//	return *m_pstlVVertexColors;
//}

//#ifdef KPS_DEBUG
	void DebugContext::SetMatrix(const D3DXMATRIX _mMatrix)
	{
		m_m4Matrix	= _mMatrix;
	}

	D3DXMATRIX DebugContext::GetCamera()
	{
		return m_m4Camera;
	}


	//#ifdef KPS_DEBUG

	void DebugContext::SetCamera(D3DXMATRIX _m4Camera)
	{
		m_m4Camera	= _m4Camera;
	}

	void DebugContext::SetCameraMode(bool _bCameraMode)
	{
		m_bCameraView	= _bCameraMode;
	}

	bool DebugContext::GetCameraMode()
	{
		return m_bCameraView;
	}

#endif // #ifndef XDIRECTX
	void DebugContext::Init(int _nScreenWidth, int _nScreenHeight)
	{
		m_nScreenWidth	= _nScreenWidth;
		m_nScreenHeight	= _nScreenHeight;
#ifndef XDIRECTX
		m_pstlVVertexs		= new STLVDxVector3;// std::vector<D3DXVECTOR3>;
		m_pstlVVertexColors	= new STLVDxVector3;//std::vector<D3DXVECTOR3>;
		m_pstlVVertexTimes	= new STLVuInt;//std::vector<UINT32>;
		m_pstlVLines			= new STLVDxVector3;//std::vector<D3DXVECTOR3>;
		m_pstlVLineColors		= new STLVDxVector3;;//std::vector<D3DXVECTOR3>;
		m_pstlVLineTimes		= new STLVuInt;//std::vector<UINT32>;

		m_pstlVLineSprites			= new STLVDxVector3;// Add by OJ : 2010-05-31
		m_pstlVLineSpriteColors		= new STLVDxVector3;// Add by OJ : 2010-05-31
		m_pstlVLineSpriteTimes		= new STLVuInt;// Add by OJ : 2010-05-31

		m_pstlVVertexsStatic		= new STLVDxVector3;// std::vector<D3DXVECTOR3>;
		m_pstlVVertexColorsStatic	= new STLVDxVector3;//std::vector<D3DXVECTOR3>;
		m_pstlVLinesStatic			= new STLVDxVector3;//std::vector<D3DXVECTOR3>;
		m_pstlVLineColorsStatic		= new STLVDxVector3;;//std::vector<D3DXVECTOR3>;

		D3DXMatrixIdentity(&m_m4Matrix);
		D3DXMatrixIdentity(&m_m4Camera);
		
#endif
		m_bCameraView	= FALSE;

		m_nTimeReportLoopCount	= 0;
		m_nTimeReportBefor		= BaseSystem::timeGetTime();
		strcpy_s(m_strLocalIdentity, 255, "debug");
		m_nIDDungeon	= 100;
		m_nCountEvent	= 0;
		m_nCountEventMove	= 0;
		m_nCntMacroDetect	= 0;
	}

	void DebugContext::Release()
	{
		ClearEventLog();
#ifndef XDIRECTX
		if(m_pstlVVertexs==NULL)
			return;

		delete m_pstlVVertexs;
		delete m_pstlVVertexColors;
		delete m_pstlVVertexTimes;
		delete m_pstlVLines;
		delete m_pstlVLineColors;
		delete m_pstlVLineTimes;

		delete m_pstlVLineSprites;
		delete m_pstlVLineSpriteColors;
		delete m_pstlVLineSpriteTimes;

		delete m_pstlVVertexsStatic;
		delete m_pstlVVertexColorsStatic;
		delete m_pstlVLinesStatic;
		delete m_pstlVLineColorsStatic;
		
		m_pstlVVertexs		= NULL;
		m_pstlVVertexColors	= NULL;
		m_pstlVVertexTimes	= NULL;
		m_pstlVLines			= NULL;
		m_pstlVLineColors		= NULL;
		m_pstlVLineTimes		= NULL;

		m_pstlVLineSprites		= NULL;
		m_pstlVLineSpriteColors	= NULL;
		m_pstlVLineSpriteTimes	= NULL;

		m_pstlVVertexsStatic		= NULL;
		m_pstlVVertexColorsStatic	= NULL;
		m_pstlVLinesStatic			= NULL;
		m_pstlVLineColorsStatic		= NULL;
#endif
	}
//#endif

	void DebugContext::OffTrackmode()
	{
		m_nTrackmode	= 0;
	}

	void DebugContext::OnTrackmode()
	{
		m_nTrackmode	= 1;
	}

	void convert2char(char *_pstr, wchar_t *_wstr)
	{
		while(*_wstr != 0)
		{
			UINT32 dw	= *_wstr;
			char	hi, lo;

			hi	= (char)(0xFF & dw);
			dw >>= 8;
			lo	= (char)(0xFF & dw);

			*(_pstr++) = hi;
			if(lo != 0)
			{
				*(_pstr++)	= lo;
			}

			_wstr++;
		}
		*_pstr	= 0;
	}

	static unsigned char *s_pData = NULL;
	static size_t s_nSize;
	void DebugContext::IntegrityTestSetData(void *_pData, size_t _nSize)
	{
		if(s_pData)
		{
			PT_Free(s_pData);
			s_pData	= NULL;
		}
		s_nSize	= _nSize;
		s_pData	= PT_Alloc(unsigned char, _nSize);

		memcpy(s_pData, _pData, s_nSize);
	}

	void *DebugContext::IntegrityTestGetData()
	{
		return s_pData;
	}

	void DebugContext::IntegrityTestCompare(void *_pData)
	{
		if(memcmp(s_pData, _pData, s_nSize) != 0)
		{
			assert(!"Fail to memory compare");
		}
	}
#ifndef XDIRECTX
	// A structure for our custom vertex type
	struct CUSTOMVERTEX {
		FLOAT x, y, z; // The transformed position for the vertex
		INT32 color;        // The vertex color
	};
	#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

	D3DXVECTOR3	g_v3Pointed;
	D3DXVECTOR3	g_v3Distance;
	D3DXVECTOR2	g_v2MouseOldPos;
#endif
	bool	g_bMouseMove			= FALSE;
	bool	g_bMouseButtonLeft		= FALSE;
	bool	g_bMouseButtonRight		= FALSE;
	bool	g_bMouseButtonMiddle	= FALSE;

//
#ifndef XDIRECTX
    //using namespace std;
	void DebugContext::DrawPrimitivies(IDirect3DDevice9* _pD3DDevice, const D3DXMATRIX &_m4Projection, const D3DXMATRIX &_m4View)
	{
		D3DXMATRIX	m4Identity;
		
		//==========================================================================
		// Start Update by OJ : 2006-07-11, 오후 8:43

		// Render the Primitives
		if(GetDebugContext().GetEnable())
		{
			float	fPointSize	= 5;
			_pD3DDevice->SetRenderState(D3DRS_POINTSIZE, *((INT32*)&fPointSize));
			_pD3DDevice->SetTexture(0, NULL);
			_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
			_pD3DDevice->SetRenderState( D3DRS_ZENABLE , FALSE );
			_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
			_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

			D3DXMATRIX	m4MatrixView, m4MatrixWorld;
			//==========================================================================
			// Start Update by OJ : 2010-05-31
			// Render Sprites for 2D
			{
				D3DXMATRIX	m4Identity;
				D3DXMatrixIdentity(&m4Identity);
				_pD3DDevice->SetTransform( D3DTS_PROJECTION,  &m4Identity );
				D3DXMatrixScaling(&m4Identity, 2.f/(float)m_nScreenWidth, -2.f/(float)m_nScreenHeight, 1.f);
				_pD3DDevice->SetTransform(D3DTS_WORLD, &m4Identity);
				D3DXMatrixIdentity(&m4Identity);
				D3DXMatrixTranslation(&m4Identity
					, -1.f
					, 1.f, 0.f);
				_pD3DDevice->SetTransform(D3DTS_VIEW, &m4Identity);

				const STLVDxVector3	&stlVLines		= *m_pstlVLineSprites;
				const STLVDxVector3	&stlVLineColors	= *m_pstlVLineSpriteColors;

				if(stlVLines.size() > 0) {
					CUSTOMVERTEX *pArrayVertex = PT_Alloc(CUSTOMVERTEX, stlVLines.size());

					int i = 0;

					for( i = 0; i < (int)stlVLines.size()/2; i++) {
						(pArrayVertex+i*2)->x = stlVLines[i*2+0].x;
						(pArrayVertex+i*2)->y = stlVLines[i*2+0].y;
						(pArrayVertex+i*2)->z = stlVLines[i*2+0].z;
						(pArrayVertex+i*2)->color = RGB( stlVLineColors[i].z*255, stlVLineColors[i].y*255, stlVLineColors[i].x*255);

						if(D3DXVec3Dot(&(stlVLines[i*2+0] - stlVLines[i*2+1]), &(stlVLines[i*2+0] - stlVLines[i*2+1])) == 0) {
							(pArrayVertex+i*2+1)->x = stlVLines[i*2+1].x;
							(pArrayVertex+i*2+1)->y = stlVLines[i*2+1].y + 1.f;
							(pArrayVertex+i*2+1)->z = stlVLines[i*2+1].z;
						} else {
							(pArrayVertex+i*2+1)->x = stlVLines[i*2+1].x;
							(pArrayVertex+i*2+1)->y = stlVLines[i*2+1].y;
							(pArrayVertex+i*2+1)->z = stlVLines[i*2+1].z;
						}

						(pArrayVertex+i*2+1)->color = RGB( stlVLineColors[i].z*255, stlVLineColors[i].y*255, stlVLineColors[i].x*255);
					}

					_pD3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
					_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, stlVLines.size()/2, pArrayVertex, sizeof(CUSTOMVERTEX));

					PT_Free(pArrayVertex);
				}
			}
			//================================= End Update by OJ : 2010-05-31

			_pD3DDevice->SetTransform( D3DTS_PROJECTION,  &_m4Projection );

			_pD3DDevice->GetTransform(D3DTS_WORLD, &m4MatrixWorld);
			_pD3DDevice->GetTransform(D3DTS_VIEW, &m4MatrixView);

			D3DXMATRIX	m4Identity;

			D3DXMatrixIdentity(&m4Identity);

			_pD3DDevice->SetTransform(D3DTS_WORLD, &m4Identity);

			D3DXMATRIX g_matView = _m4View;

			if(GetDebugContext().GetCameraMode()) {
				g_matView	= GetDebugContext().GetCamera();
				D3DXMatrixInverse(&g_matView, NULL, &g_matView);
			}
			_pD3DDevice->SetTransform(D3DTS_VIEW, &g_matView);
			
			const STLVDxVector3	&stlVVertexs		= *m_pstlVVertexs;//GetDebugContext().GetVertexs();
			const STLVDxVector3	&stlVVertexColors	= *m_pstlVVertexColors;//GetDebugContext().GetVertexColors();

			if(stlVVertexs.size() > 0) {
				CUSTOMVERTEX *pArrayVertex = PT_Alloc(CUSTOMVERTEX, stlVVertexs.size());
				int i = 0;

				for( i = 0; i < (int)stlVVertexs.size(); i++) {
					(pArrayVertex+i)->x = stlVVertexs[i].x;
					(pArrayVertex+i)->y = stlVVertexs[i].y;
					(pArrayVertex+i)->z = stlVVertexs[i].z;
					(pArrayVertex+i)->color = RGB( (unsigned char)(stlVVertexColors[i].z*255),
						(unsigned char)(stlVVertexColors[i].y*255),
						(unsigned char)(stlVVertexColors[i].x*255));
				}

				_pD3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
				//_pD3DDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX );
				_pD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, stlVVertexs.size(), pArrayVertex, sizeof(CUSTOMVERTEX));

				PT_Free(pArrayVertex);
			}else{
				D3DXMATRIX	m4Frame;
				D3DXMatrixIdentity(&m4Frame);
				GetDebugContext().AddFrame(m4Frame, D3DXVECTOR3(1, 0, 0));
			}

			const STLVDxVector3	&stlVLines		= *m_pstlVLines;//GetDebugContext().GetLines();
			const STLVDxVector3	&stlVLineColors	= *m_pstlVLineColors;//GetDebugContext().GetLineColors();

			if(stlVLines.size() > 0) {
				CUSTOMVERTEX *pArrayVertex = PT_Alloc(CUSTOMVERTEX, stlVLines.size());

				int i = 0;

				for( i = 0; i < (int)stlVLines.size()/2; i++) {
					(pArrayVertex+i*2)->x = stlVLines[i*2+0].x;
					(pArrayVertex+i*2)->y = stlVLines[i*2+0].y;
					(pArrayVertex+i*2)->z = stlVLines[i*2+0].z;
					(pArrayVertex+i*2)->color = RGB( stlVLineColors[i].z*255, stlVLineColors[i].y*255, stlVLineColors[i].x*255);

					if(D3DXVec3Dot(&(stlVLines[i*2+0] - stlVLines[i*2+1]), &(stlVLines[i*2+0] - stlVLines[i*2+1])) == 0) {
						(pArrayVertex+i*2+1)->x = stlVLines[i*2+1].x;
						(pArrayVertex+i*2+1)->y = stlVLines[i*2+1].y + 1.f;
						(pArrayVertex+i*2+1)->z = stlVLines[i*2+1].z;
					} else {
						(pArrayVertex+i*2+1)->x = stlVLines[i*2+1].x;
						(pArrayVertex+i*2+1)->y = stlVLines[i*2+1].y;
						(pArrayVertex+i*2+1)->z = stlVLines[i*2+1].z;
					}

					(pArrayVertex+i*2+1)->color = RGB( stlVLineColors[i].z*255, stlVLineColors[i].y*255, stlVLineColors[i].x*255);
				}

				_pD3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
				//_pD3DDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX );
				_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, stlVLines.size()/2, pArrayVertex, sizeof(CUSTOMVERTEX));

				PT_Free(pArrayVertex);
			}
			_pD3DDevice->SetRenderState( D3DRS_ZENABLE , TRUE );
			//================================= End Update by OJ : 2006-07-11, 오후 8:43
		}

		// Render the Primitives Static
		if(GetDebugContext().GetEnable())
		{

			const STLVDxVector3	&stlVVertexs		= *m_pstlVVertexsStatic;//GetDebugContext().GetVertexs();
			const STLVDxVector3	&stlVVertexColors	= *m_pstlVVertexColorsStatic;//GetDebugContext().GetVertexColors();

			D3DXMATRIX	m4MatrixView, m4MatrixWorld;

			_pD3DDevice->SetTransform( D3DTS_PROJECTION,  &_m4Projection );

			_pD3DDevice->GetTransform(D3DTS_WORLD, &m4MatrixWorld);
			_pD3DDevice->GetTransform(D3DTS_VIEW, &m4MatrixView);

			D3DXMATRIX	m4Identity;

			D3DXMatrixIdentity(&m4Identity);

			_pD3DDevice->SetTransform(D3DTS_WORLD, &m4Identity);

			D3DXMATRIX g_matView = _m4View;

			if(GetDebugContext().GetCameraMode()) {
				g_matView	= GetDebugContext().GetCamera();
				D3DXMatrixInverse(&g_matView, NULL, &g_matView);
			}
			_pD3DDevice->SetTransform(D3DTS_VIEW, &g_matView);

			float	fPointSize	= 5;
			_pD3DDevice->SetRenderState(D3DRS_POINTSIZE, *((INT32*)&fPointSize));
			_pD3DDevice->SetTexture(0, NULL);
			_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
			_pD3DDevice->SetRenderState( D3DRS_ZENABLE , FALSE );
			_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
			_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

			if(stlVVertexs.size() > 0) {
				CUSTOMVERTEX *pArrayVertex = PT_Alloc(CUSTOMVERTEX, stlVVertexs.size());
				int i = 0;

				for( i = 0; i < (int)stlVVertexs.size(); i++) {
					(pArrayVertex+i)->x = stlVVertexs[i].x;
					(pArrayVertex+i)->y = stlVVertexs[i].y;
					(pArrayVertex+i)->z = stlVVertexs[i].z;
					(pArrayVertex+i)->color = RGB( (bbyte)(stlVVertexColors[i].z*255),
						(bbyte)(stlVVertexColors[i].y*255),
						(bbyte)(stlVVertexColors[i].x*255));
				}

				_pD3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
				//_pD3DDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX );
				_pD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, stlVVertexs.size(), pArrayVertex, sizeof(CUSTOMVERTEX));

				PT_Free(pArrayVertex);
			}else{
				D3DXMATRIX	m4Frame;
				D3DXMatrixIdentity(&m4Frame);
				GetDebugContext().AddFrame(m4Frame, D3DXVECTOR3(1, 0, 0));
			}

			const STLVDxVector3	&stlVLines		= *m_pstlVLinesStatic;//GetDebugContext().GetLines();
			const STLVDxVector3	&stlVLineColors	= *m_pstlVLineColorsStatic;//GetDebugContext().GetLineColors();

			if(stlVLines.size() > 0) {
				CUSTOMVERTEX *pArrayVertex = PT_Alloc(CUSTOMVERTEX, stlVLines.size());

				int i = 0;

				for( i = 0; i < (int)stlVLines.size()/2; i++) {
					(pArrayVertex+i*2)->x = stlVLines[i*2+0].x;
					(pArrayVertex+i*2)->y = stlVLines[i*2+0].y;
					(pArrayVertex+i*2)->z = stlVLines[i*2+0].z;
					(pArrayVertex+i*2)->color = RGB( stlVLineColors[i].z*255, stlVLineColors[i].y*255, stlVLineColors[i].x*255);

					if(D3DXVec3Dot(&(stlVLines[i*2+0] - stlVLines[i*2+1]), &(stlVLines[i*2+0] - stlVLines[i*2+1])) == 0) {
						(pArrayVertex+i*2+1)->x = stlVLines[i*2+1].x;
						(pArrayVertex+i*2+1)->y = stlVLines[i*2+1].y + 1.f;
						(pArrayVertex+i*2+1)->z = stlVLines[i*2+1].z;
					} else {
						(pArrayVertex+i*2+1)->x = stlVLines[i*2+1].x;
						(pArrayVertex+i*2+1)->y = stlVLines[i*2+1].y;
						(pArrayVertex+i*2+1)->z = stlVLines[i*2+1].z;
					}

					(pArrayVertex+i*2+1)->color = RGB( stlVLineColors[i].z*255, stlVLineColors[i].y*255, stlVLineColors[i].x*255);
				}

				_pD3DDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
				//_pD3DDevice->SetVertexShader(D3DFVF_CUSTOMVERTEX );
				_pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, stlVLines.size()/2, pArrayVertex, sizeof(CUSTOMVERTEX));

				PT_Free(pArrayVertex);
			}

			_pD3DDevice->SetRenderState( D3DRS_ZENABLE , TRUE );
			//================================= End Update by OJ : 2006-07-11, 오후 8:43
		}
	}
	void DebugContext::NevZoom(int nDelta) {
		float	fDistance = D3DXVec3Length(&g_v3Distance);

		D3DXMATRIX m4Temp;
		D3DXMatrixIdentity(&m4Temp);

		D3DXMATRIX m4Cam = GetDebugContext().GetCamera();
		;
		D3DXMatrixTranslation(&m4Temp, 0.f, 0.f,fDistance/40.f * ((float)(nDelta)/4.f));
		D3DXMatrixMultiply(&m4Cam, &m4Temp, &m4Cam);
		GetDebugContext().SetCamera(m4Cam);
	}

	void DebugContext::NevPan(int nDx, int nDy) {
		nDy *= -1;
		nDx *= -1;

		float	fDistance = D3DXVec3Length(&g_v3Distance);

		D3DXMATRIX m4Temp;
		D3DXMatrixIdentity(&m4Temp);

		D3DXMATRIX m4Cam = GetDebugContext().GetCamera();
		;
		//m4Temp.SetTranslate(fDistance/80.f * ((float)(nDx)/4.f), fDistance/80.f * ((float)(-nDy)/4.f), 0.f);
		D3DXMatrixTranslation(&m4Temp, fDistance/80.f * ((float)(nDx)/4.f), fDistance/80.f * ((float)(-nDy)/4.f), 0.f);
		D3DXMatrixMultiply(&m4Cam, &m4Temp, &m4Cam);
		GetDebugContext().SetCamera(m4Cam);
	}


	void DebugContext::NevExaminer(int nDx, int nDy) {
		float	fDistance = D3DXVec3Length(&g_v3Distance);

		D3DXVECTOR3	v3ToOrigin;

		v3ToOrigin = g_v3Pointed;
		v3ToOrigin *= -1;

		D3DXMATRIX m4Temp;

		D3DXMATRIX m4Cam = GetDebugContext().GetCamera();
		;
		m4Cam._41 += v3ToOrigin.x, m4Cam._42 += v3ToOrigin.y, m4Cam._43 += v3ToOrigin.z;

		D3DXMatrixIdentity(&m4Temp);
		D3DXMatrixRotationY(&m4Temp, D3DXToRadian((float)nDx)/3.f);
		D3DXMatrixMultiply(&m4Cam, &m4Cam, &m4Temp);

		D3DXMatrixIdentity(&m4Temp);
		//D3DXMatrixRotationX(&m4Temp, D3DXToRadian((float)nDy)/4.f);
		//D3DXMatrixMultiply(&m4Cam, &m4Temp, &m4Cam);
		D3DXVECTOR3	v3Axis(m4Cam._11, m4Cam._12, m4Cam._13);
		D3DXMatrixRotationAxis(&m4Temp, &v3Axis, D3DXToRadian((float)nDy)/4.f);
		D3DXMatrixMultiply(&m4Cam, &m4Cam, &m4Temp);

		v3ToOrigin *= -1;
		m4Cam._41 += v3ToOrigin.x, m4Cam._42 += v3ToOrigin.y, m4Cam._43 += v3ToOrigin.z;

		m4Cam._42 += fDistance/40.f * ((float)(nDy)/4.f);

		GetDebugContext().SetCamera(m4Cam);
	}

	void DebugContext::NevSphere(int nDx, int nDy) {
		//-float fDistance = D3DXVec3Length(&g_v3Distance);

		D3DXMATRIX m4Temp;
		D3DXMatrixIdentity(&m4Temp);

		D3DXMATRIX m4Cam = GetDebugContext().GetCamera();

		D3DXVECTOR3 v4Pos;
		v4Pos.x	= m4Cam._41;
		v4Pos.y	= m4Cam._42;
		v4Pos.z	= m4Cam._43;

		D3DXMatrixRotationY(&m4Temp, D3DXToRadian((float)nDx)/3.f);
		D3DXMatrixMultiply(&m4Cam, &m4Cam, &m4Temp);

		D3DXMatrixIdentity(&m4Temp);
		D3DXMatrixRotationX(&m4Temp, D3DXToRadian((float)nDy)/3.f);
		D3DXMatrixMultiply(&m4Cam, &m4Temp, &m4Cam);

		m4Cam._41	= v4Pos.x;
		m4Cam._42	= v4Pos.y;
		m4Cam._43	= v4Pos.z;

		GetDebugContext().SetCamera(m4Cam);
	}

	D3DXVECTOR3 DebugContext::GetEarthPosition(D3DXVECTOR3 _v3Normal, D3DXVECTOR3  _v3Pos) {
		D3DXVECTOR3 vec;

		if(_v3Normal.y == 0.) {
			vec.x	= 0.f;
			vec.y	= -1.f;
			vec.z	= 0.f;
			return vec;
		}

		vec.x = -_v3Pos.y*_v3Normal.x/_v3Normal.y+_v3Pos.x;
		vec.y = 0.;
		vec.z = -_v3Pos.y*_v3Normal.z/_v3Normal.y+_v3Pos.z;

		return vec;
	}

	int DebugContext::nCalcCamEnv() {
		D3DXVECTOR3	v3CamPos;
		D3DXMATRIX	m4Cam;
		m4Cam	= GetDebugContext().GetCamera();
		g_v3Distance.x	= m4Cam._31;
		g_v3Distance.y	= m4Cam._32;
		g_v3Distance.z	= m4Cam._33;

		v3CamPos.x	= m4Cam._41;
		v3CamPos.y	= m4Cam._42;
		v3CamPos.z	= m4Cam._43;
		g_v3Pointed		= GetEarthPosition(g_v3Distance, v3CamPos);

		D3DXVECTOR3	v3Dist;

		v3Dist	= g_v3Pointed - v3CamPos;
		if(D3DXVec3Length(&v3Dist) < 50) {
			g_v3Distance	*= 50;
		} else if(D3DXVec3Length(&v3Dist) > 10000) {
			g_v3Distance	*= 10000;
		} else {
			g_v3Distance	= g_v3Pointed - v3CamPos;
		}

		return 0;
	}
#endif
#ifndef XDIRECTX
	bool DebugContext::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		D3DXVECTOR2	v2MusPos;
		switch(uMsg) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			g_v2MouseOldPos.x	= LOWORD(lParam);
			g_v2MouseOldPos.y	= HIWORD(lParam);
			g_bMouseMove		= FALSE;
			nCalcCamEnv();
			m_bKeyAlt	= ((HIWORD(lParam))&KF_ALTDOWN) != 0;
			break;
		case WM_MOUSEMOVE:
	//	usPos.x	= LOWORD(lParam);
	//	v2MusPos.y	= HIWORD(lParam);
			break;
		}

		switch(uMsg) 
		{
		case WM_KEYDOWN:
			{
				m_bKeyAlt	= ((HIWORD(lParam))&KF_ALTDOWN) != 0;
				switch(wParam){
				case VK_INSERT:
					if(m_bKeyShift)
					{
						GetDebugContext().SetEnable(!GetDebugContext().GetEnable());
					}
					break;
				case VK_SHIFT:
					m_bKeyShift	= true;
					break;

				case VK_DELETE:
					if(m_bKeyShift)
						GetDebugContext().ClearAll();
					break;
				case 0x43: // 'C'
					if(m_bKeyShift) {
						if(GetDebugContext().GetCameraMode()) {
							GetDebugContext().SetCameraMode(FALSE);
						} else {
							GetDebugContext().SetCameraMode(TRUE);

							D3DXMATRIX	m4Cam;

							m4Cam	= *m_pm4View;
							D3DXMatrixInverse(&m4Cam, NULL, &m4Cam);
							GetDebugContext().SetCamera(m4Cam);
						}
					}
					break;
				}
				break;
			}
		case WM_KEYUP:
			{
				m_bKeyAlt	= ((HIWORD(lParam))&KF_ALTDOWN) != 0;
				switch(wParam){
				case VK_SHIFT:
					m_bKeyShift	= false;
					break;
				}
				break;
			}
		case WM_LBUTTONDOWN:
			g_bMouseButtonLeft	= TRUE;
			break;
		case WM_LBUTTONUP:
			g_bMouseButtonLeft	= FALSE;
			break;
		case WM_RBUTTONDOWN:
			g_bMouseButtonRight	= TRUE;
			break;
		case WM_RBUTTONUP:
			g_bMouseButtonRight	= FALSE;
			break;
		case WM_MBUTTONDOWN:
			g_bMouseButtonMiddle	= TRUE;
			break;
		case WM_MBUTTONUP:
			g_bMouseButtonMiddle	= FALSE;
			break;
		case WM_MOUSEMOVE: 
			if(g_bMouseButtonMiddle) {
				if(m_bKeyAlt != false) {
					NevExaminer( (int)(v2MusPos.x-g_v2MouseOldPos.x), (int)(v2MusPos.y-g_v2MouseOldPos.y) );
				} else {
					NevPan( (int)(v2MusPos.x-g_v2MouseOldPos.x), (int)(v2MusPos.y-g_v2MouseOldPos.y) );
				}
			}
			if(g_bMouseButtonRight) {
				NevZoom( (int)(v2MusPos.y-g_v2MouseOldPos.y) );
			}
			if(g_bMouseButtonLeft) {
				if(m_bKeyShift != false)
					NevExaminer( (int)(v2MusPos.x-g_v2MouseOldPos.x), (int)(v2MusPos.y-g_v2MouseOldPos.y) );
				else
					NevSphere( (int)(v2MusPos.x-g_v2MouseOldPos.x), (int)(v2MusPos.y-g_v2MouseOldPos.y) );
			}
			g_v2MouseOldPos	= v2MusPos;
			break;
		}

		switch(uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
			if(GetCameraMode())
				return true;
			break;
		}
		return false;
	}
#endif
#ifndef XDIRECTX
	void DebugContext::SetViewCamera(D3DXMATRIX *_pm4View)
	{
		m_pm4View	= _pm4View;
	}
#endif
	void DebugContext::Log(const char *_strLogFile, char *_strMsg, ...)
	{
		va_list vargs;
		char	strBuffer[255];
		va_start( vargs, _strMsg );
		vsprintf_s( strBuffer, 255, _strMsg, (vargs) );

		BaseFile	file;

		if(!file.OpenFile(_strLogFile, BaseFile::OPEN_RANDOM | BaseFile::OPEN_WRITE))
		{
			file.write_asc_string(strBuffer, 255);
			file.write_asc_line();
		}
		va_end( vargs );
	}


#ifndef XDIRECTX
#include <wininet.h>
#include <process.h>
	UINT __stdcall ThreadReportEventLog(void *_pParam)
	{
		char *strFilename	= (char*)_pParam;

		const char	g_szFTP_ServerURL[] = "corumpatch.gscdn.com"; 
		const int   g_iFtpPort = 21; // 본섭 포트
		const char	g_szFTP_ID[]		= "dumpupload";
		const char	g_szFTP_Password[]	= "@ejavmdjq#";

		HINTERNET	hInternet = ::InternetOpen( 
			"EventModule", 
			INTERNET_OPEN_TYPE_PRECONFIG, 
			NULL, 
			NULL, 
			0 );

		if( NULL == hInternet )
		{
			::DeleteFileA(strFilename);
			return 0;
		}

		HINTERNET	hFTP = ::InternetConnect( 
			hInternet, 
			g_szFTP_ServerURL, 
			g_iFtpPort, 
			g_szFTP_ID, 
			g_szFTP_Password, 
			INTERNET_SERVICE_FTP, 
			INTERNET_FLAG_PASSIVE, 
			0 );

		if( NULL == hFTP )
		{
			::DeleteFileA(strFilename);
			::InternetCloseHandle( hInternet );
			return 0;
		}

		char strRemote[255];
		strcpy_s(strRemote, 255, "EventLog/");
		strcat_s(strRemote, 255, strFilename);
		strcpy_s(strRemote, 255, strFilename);


		//HINTERNET hRet;
		//if(FALSE == ::FtpCommand(hFTP, TRUE, FTP_TRANSFER_TYPE_ASCII, "CWD EventLog", 0 , &hRet))
		//{
		//	int x=0;
		//	x++;
		//}

		if( FALSE == ::FtpPutFile( hFTP, strFilename, strRemote, FTP_TRANSFER_TYPE_BINARY 
			/*| INTERNET_FLAG_RELOAD*/, 0 ) )
		{
			INT32 dwError = GetLastError();

			int x=0;
			x++;
		}

		::DeleteFileA(strFilename);

		::InternetCloseHandle( hFTP );
		::InternetCloseHandle( hInternet );	
		return 0;
	}
	
	bool	DebugContext::ReportEventLog(const char *_strExt)
	{	
		BaseFile file;
		static char strFilenameFull[255];

		SYSTEMTIME	timeSystem;

		if(m_nCountEvent < 
#ifdef _DEBUG
			10)
#else
			100)
#endif
		{
			return false;
		}

		UINT32 nTimeCur	= BaseSystem::timeGetTime();
		bool	bAutoMouse	= false;
		{// 
			int	nCountDouble = 0;	// 두번이상 들어온 Event갯 수
			int nCountSingle = 0;	// 한번만 들어온 Event 갯 수

			int	nCnt;
			nCnt	= m_stlMnpstlVstEventLog.size();
			STLMnpstlVstEventLog::iterator	it;
			it	= m_stlMnpstlVstEventLog.begin();

			int nCountValiedEvent	= 0;
			int nCountKeyType		= 0;
			int nCountMouseEvent	= 0;

			for(;it!=m_stlMnpstlVstEventLog.end(); it++)
			{
				STLVstEventLog	&stlVstEvent	= *(it->second);

				for(UINT32 i=0; i<stlVstEvent.size(); i++)
				{
					if(stlVstEvent[i].nCount > 1)
						nCountDouble++;
					else 
						nCountSingle++;

					nCountValiedEvent	+= stlVstEvent[i].nCount;

					if(stlVstEvent[i].nMsg < WM_MOUSEMOVE)
						nCountKeyType++;
					else
						nCountMouseEvent+=stlVstEvent[i].nCount;

					if(nTimeCur - stlVstEvent[i].nTime > 5*60*1000)
						stlVstEvent.erase(stlVstEvent.begin()+i), i--;
				}
				if(stlVstEvent.size() == 0)
				{
					delete it->second;
					it	= m_stlMnpstlVstEventLog.erase(it);
				}
			}

			if(nCountDouble > nCountSingle)
				bAutoMouse	= true;

			if(m_nCountEventMove <= m_nCountEvent*2)
			{
				bAutoMouse	= true;
				OutputDebugString("=============Check Automouse by Moving check\n");
			}

			if(nCountValiedEvent < 
#ifdef _DEBUG
				10)
#else
				100)// 전체적인 Event개수가 적어도 오토로 가정하지 않는다
#endif
				bAutoMouse	= false;

			if(nCountKeyType > 10 && nCountMouseEvent < nCountKeyType)
				bAutoMouse	= false; // 마우스 움직임이 거의 없고 다양한 키보드를 사용한 경우 
									// 체팅하고 있는것으로 가정하고 Auto로 판정하지 않는다
		}

		if(!bAutoMouse)
		{
			OutputDebugString("=============Check Automouse Clear");
			ClearEventLog();
			return false;
		}
		OutputDebugString("=============Check Automouse\n");

		GetSystemTime(&timeSystem);

		timeSystem.wHour	+= 9;// change to Korea Time
		if(timeSystem.wHour > 23)
			timeSystem.wHour -= 24;

		sprintf_s(strFilenameFull, 255, "e-%s %04d-%02d-%02d %02d-%02d %d",
					_strExt,
					timeSystem.wYear, timeSystem.wMonth, timeSystem.wDay,
					timeSystem.wHour, timeSystem.wMinute, m_nCountEvent);
		strcat_s(strFilenameFull, 255, ".ini");

		if(!file.OpenFile(strFilenameFull, BaseFile::OPEN_WRITE))
		{
			int	nCnt;
			nCnt	= m_stlMnpstlVstEventLog.size();
			char strBuffer[255];
			sprintf_s(strBuffer, 255, "Mouse Move Count %d", m_nCountEventMove);
			file.write_asc_string(strBuffer, 255);
			file.write_asc_line();
			sprintf_s(strBuffer, 255, "Total Event Count %d", m_nCountEvent);
			file.write_asc_string(strBuffer, 255);
			file.write_asc_line();

			file.write_asc_integer(nCnt);
			file.write_asc_line();
			STLMnpstlVstEventLog::iterator	it;
			it	= m_stlMnpstlVstEventLog.begin();
			for(;it!=m_stlMnpstlVstEventLog.end(); it++)
			{
				nCnt	= it->second->size();
				file.write_asc_string("count ", 7);
				file.write_asc_integer(nCnt);

				STLVstEventLog	&stlVstEvent	= *(it->second);
				
				file.write_asc_integer(stlVstEvent[0].nMsg);
				file.write_asc_line();
				for(int i=0; i<nCnt; i++)
				{
					file.write_asc_string("\t", 2);
					file.write_asc_integer((int)stlVstEvent[i].nCount);
					file.write_asc_integer((int)stlVstEvent[i].nParam1);
					file.write_asc_integer((int)stlVstEvent[i].nParam2);
					file.write_asc_integer((int)stlVstEvent[i].nTimeMin);
					file.write_asc_integer((int)stlVstEvent[i].nTimeMax);
					file.write_asc_line();
				}
			}
			file.CloseFile();

			UINT32 nThreadID;
			CZipper::ZipFile(strFilenameFull);
			::DeleteFile(strFilenameFull);
			char strFilename[255];
			BaseFile::get_filename( strFilenameFull, strFilename, 255);
			strcpy_s(strFilenameFull, 255, strFilename);
			strcat_s(strFilenameFull, 255, ".zip");
			_beginthreadex( NULL, 0, &ThreadReportEventLog, (PVOID)strFilenameFull, 0, &nThreadID);
		}
		m_nCountEvent	= 0;
		m_nCountEventMove	= 0;

		return true;
	}

	void DebugContext::LoadFrames(const char *_strFrames)
	{
		BaseFile	file;
		D3DXMATRIX	m4Mat;
		D3DXVECTOR3	v3Color(1, 1, 1);

		if(!file.OpenFile(_strFrames, BaseFile::OPEN_READ))
		{
			while(file.read_asc_line())
			{
				for(int i=0; i<4*4; i++)
				{
					file.read_asc_float((&m4Mat._11)+i);
				}
				AddFrameStatic(m4Mat, v3Color);
			}
		}
	}

	int DebugContext::PushWinMsgLog(UINT32 _nMsg, UINT32 _wParam, INT32 _lParam)
	{
		if(m_nIDDungeon < 100) // 마을던전에서는 로그를 남기지 않는다.
			return 0;

		if(m_nTimeReportBefor > 0 && BaseSystem::timeGetTime() - m_nTimeReportBefor >
#ifdef _DEBUG
			10000) // 10 second
			//600000) // 10 minutes
#else
			//30000)	// 30 second
			//300000) // 5 minute
			600000) // 10 minute
#endif
		{
			m_nTimeReportBefor	= BaseSystem::timeGetTime();

			if(ReportEventLog(m_strLocalIdentity))
			{
				m_nCntMacroDetect++;
				return m_nCntMacroDetect;
			}
		}
		UINT32	nParam1, nParam2;
		switch(_nMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			nParam1	= LOWORD( _lParam );
			nParam2	= HIWORD( _lParam );
			PushEventLog(_nMsg, nParam1, nParam2);
			break;
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			if(_nMsg == WM_KEYUP && _wParam == 229)// 마우스 클릭후 이 메시지가 들어오는데 이유는 모름
				break;
			PushEventLog(_nMsg, _wParam, 0);
		case WM_MOUSEMOVE:
			m_nCountEventMove++;
			break;
		}

		return 0;
	}

	void DebugContext::PushEventLog(UINT32 _nMsg, UINT32 _nParam1, unsigned _nParam2)
	{
		const static int nDiffer	= 1;// Add by OJ : 2010-12-01 5 => 1로 수정
		const static int nMaxLog	= 1000;

		STLMnpstlVstEventLog::iterator	it;
		it	= m_stlMnpstlVstEventLog.find(_nMsg);

		UINT32	nTimeCur	= BaseSystem::timeGetTime();
		STLVstEventLog *pstlVstEvent;
		if(it == m_stlMnpstlVstEventLog.end())
		{
			pstlVstEvent	= new STLVstEventLog;
			m_stlMnpstlVstEventLog[_nMsg]	= pstlVstEvent;
		}else{
			pstlVstEvent	= it->second;
		}

		int nIndex	= -1;
		if(pstlVstEvent)
		{
			STEventLog	*pstEventLog	= NULL;
			for(UINT32 i=0; i<pstlVstEvent->size(); i++)
			{
				pstEventLog	= &((*pstlVstEvent)[i]);
				if(_nMsg == WM_KEYDOWN || _nMsg == WM_KEYUP
					|| _nMsg == WM_SYSKEYDOWN || _nMsg == WM_SYSKEYUP)
				{
					if(pstEventLog->nParam1 == _nParam1
						&& pstEventLog->nParam2 == _nParam2)
					{
						nIndex	= (int)i;
						break;
					}
				}else{// MOUSE Message
					if(abs((INT32)pstEventLog->nParam1 - (INT32)_nParam1) < nDiffer
						&& abs((INT32)pstEventLog->nParam2 - (INT32)_nParam2) < nDiffer )
					{
						nIndex	= (int)i;
						break;
					}
				}
			}

			char strBuffer[255];
			if(nIndex == -1)
			{
				STEventLog stEvent;
				stEvent.nMsg	= _nMsg;
				stEvent.nCount	= 0;
				stEvent.nParam1	= _nParam1;
				stEvent.nParam2	= _nParam2;
				stEvent.nTime	= nTimeCur;		// 최종 입력된 시간
				stEvent.nTimeMin	= -1;	// 가장 빠른 시간 기록
				stEvent.nTimeMax	= 0;	// 가장 긴 시간 기록

				if(pstlVstEvent->size() > nMaxLog)
				{
					pstlVstEvent->pop_back();
				}
				pstlVstEvent->push_back(stEvent);
				pstEventLog	= &((*pstlVstEvent).back());

				if(_nMsg != WM_MOUSEMOVE)
				{
					sprintf_s(strBuffer, "create ", _nMsg);
					OutputDebugString(strBuffer);
				}
			}
			if(pstEventLog)
			{
				UINT32 nTimeDiffer;
				nTimeDiffer	= nTimeCur - pstEventLog->nTime;

				if(nTimeDiffer == 0 || nTimeDiffer > 500)
				{
					pstEventLog->nCount++;
					pstEventLog->nMsg	= _nMsg;
					pstEventLog->nTime	= nTimeCur;
					if(pstEventLog->nTimeMin > nTimeDiffer && nTimeDiffer != 0)
						pstEventLog->nTimeMin	= nTimeDiffer;
					if(pstEventLog->nTimeMax < nTimeDiffer)
						pstEventLog->nTimeMax	= nTimeDiffer;

					if(_nMsg != WM_MOUSEMOVE)
					{
						sprintf_s(strBuffer, "%d %d\n", _nMsg, pstEventLog->nCount);
						OutputDebugString(strBuffer);
					}
					m_nCountEvent++;
				}
			}
		}
		//*/
	}

#endif

	void DebugContext::SetIDDungeon(UINT32 _nID)
	{
		m_nIDDungeon	= _nID;
	}

void DebugContext::ClearEventLog()
{
    m_nCntMacroDetect	= 0;
    STLMnpstlVstEventLog::iterator	it;
    it	= m_stlMnpstlVstEventLog.begin();
    for(;it!=m_stlMnpstlVstEventLog.end(); it++)
    {
        delete it->second;
    }
    m_stlMnpstlVstEventLog.clear();
}


	void DebugContext::SetLocalIdentity(const char *_strIdentity)
	{
		strcpy_s(m_strLocalIdentity, 255, _strIdentity);
	}

	void DebugContext::SetEnable(bool _bEnable)
	{
		m_bDraw	= _bEnable;
	}
	bool DebugContext::GetEnable()
	{
		return m_bDraw;
	}

//#endif

