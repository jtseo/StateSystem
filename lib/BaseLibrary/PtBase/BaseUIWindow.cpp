#include "stdafx.h"
#include "BaseSpaceOrganizer.h"
#include "BaseUIWindow.h"
#include "BaseUIBoard.h"
#include "BaseState.h"
#include "BaseUIManager.h"

//#include "../../../SDK/Dx9/Include/d3dx9math.h"
#include "DebugContext.h"

BaseUIWindow::BaseUIWindow(BaseUIManager *_pManager, const char *_strName, int _nIdentifier, int _nWinType)
{
	m_bEventCastDisable	= false;
	m_pManager	= _pManager;
	m_pSprite	= NULL;
	m_bUpdate	= false;
	m_nWinType	= _nWinType;
	m_pWinBound	= NULL;

	m_pSpaceOrganizer	= NULL;
	m_nidentifier	= _nIdentifier;
	m_strName		= _strName;

	m_rect.SetValue(0, 0, 0, 0);
	m_nOrder	= 0;

	m_sizelimit.SetValue(0, 0);
		
	for(int i=0; i<4; i++)
	{
		m_apPosWinRef[i]	= NULL;
		m_anPosType[i]		= POS_Static;
		m_anPosOffset[i]	= 0;
	}
}

void BaseUIWindow::type_set(int _nWinType)
{
	m_nWinType	= _nWinType;
}

int BaseUIWindow::type_get()
{
	return m_nWinType;
}

BaseUIWindow::~BaseUIWindow(void)
{
	if(m_pSpaceOrganizer)
		m_pManager->space_release(m_pSpaceOrganizer);
	m_pSpaceOrganizer	= NULL;

	if(m_pSprite)
		m_pManager->sprite_release(m_pSprite);
	m_pSprite = NULL;
}

void BaseUIWindow::organizer_create()
{
	if(m_pSpaceOrganizer)
		return;

	m_pSpaceOrganizer = m_pManager->space_get();
}

BaseUIWindow *BaseUIWindow::texture_win()
{
	if(m_pSpaceOrganizer)
		return this;

	if(get_parent())
		return ((BaseUIWindow*)get_parent())->texture_win();

	return NULL;
}

void BaseUIWindow::updated()
{
	m_bUpdate = true;
}

void BaseUIWindow::pos_update_propogate(int _nPos)
{
	BaseUIWindow *pWin = texture_win();

	if(pWin)
		pWin->updated();

	//for(unsigned i=0; i<m_astlVpPosWinChilds[_nPos].size(); i++)
	//	((BaseUIWindow*)m_astlVpPosWinChilds[_nPos][i])->pos_update();
}

void BaseUIWindow::pos_set(int _x, int _y)
{
	if(m_rect.x != _x)
	{
		m_rect.x = (float)_x;
		pos_update_propogate(POS_X);
	}

	if(m_rect.y != _y)
	{
		m_rect.y = (float)_y;
		pos_update_propogate(POS_Y);
	}
}

void BaseUIWindow::pos_set_size(int _w, int _h)
{
	if(m_rect.z != _w)
	{
		if(m_sizelimit.x == 0)
			m_sizelimit.x = (float)_w;

		m_rect.z = (float)_w;
		pos_update_propogate(POS_W);
	}

	if(m_rect.w != _h)
	{
		if(m_sizelimit.y == 0)
			m_sizelimit.y = (float)_h;

		m_rect.w = (float)_h;
		pos_update_propogate(POS_H);
	}
}

void BaseUIWindow::pos_offset_set(int _nPosType, int _nOffset)
{
	m_anPosOffset[_nPosType] = _nOffset;
}

void BaseUIWindow::pos_set(int _x, int _y, int _w, int _h)
{
	pos_set(_x, _y);
	pos_set_size(_w, _h);
}

void BaseUIWindow::pos_set(Vector4 _rect)
{
	pos_set((int)_rect.x, (int)_rect.y);
	pos_set_size((int)_rect.z, (int)_rect.w);
}

void BaseUIWindow::pos_move(int _x, int _y)
{
	int x, y;
	x	= (int)m_rect.x + _x;
	y	= (int)m_rect.y + _y;

	pos_set(x, y);
}

void BaseUIWindow::pos_zorder_set(int _z)
{
	m_nOrder = _z;
}

void BaseUIWindow::pos_set(int _nPos, int _nPosType, BaseUIWindow *_pWinRef)
{
	m_anPosType[_nPos] = _nPosType;
	
	if(m_apPosWinRef[_nPos])
		m_apPosWinRef[_nPos]->pos_ref_release(this, _nPos);

	if(_pWinRef)
	{
		m_apPosWinRef[_nPos]	= _pWinRef;
		_pWinRef->pos_ref_add(this, _nPos);
	}
}

void BaseUIWindow::pos_ref_add(BaseUIWindow *_pWinChild, int _nPos)
{
	m_astlVpPosWinChilds[_nPos].push_back(_pWinChild);
}

const Vector4 &BaseUIWindow::pos_get()
{
	return m_rect;
}

void BaseUIWindow::pos_get_size(int _nPos, int *_pnStart, int *_pnEnd)
{
	if(m_child_stlVp.empty())
	{
		switch(_nPos)
		{
		case POS_X:
		case POS_W:
			*_pnStart	= (int)m_rect.x;
			*_pnEnd		= (int)(m_rect.x + m_rect.z);
			break;
		case POS_Y:
		case POS_H:
			*_pnStart	= (int)m_rect.y;
			*_pnEnd		= (int)(m_rect.y + m_rect.w);
			break;
		}
		return;
	}

	pos_get_size_child(_nPos, _pnStart, _pnEnd);

	if(m_nWinType == WIN_TYPE_Base)// convert to global position
	{
		switch(_nPos)
		{
		case POS_X:
		case POS_W:
			*_pnStart	+= (int)m_rect.x;
			*_pnEnd		+= (int)m_rect.x;
			break;
		case POS_Y:
		case POS_H:
			*_pnStart	= +(int)m_rect.y;
			*_pnEnd		= +(int)m_rect.y;
			break;
		}
	}

	switch(_nPos)// convine coordinate
	{
	case POS_X:
	case POS_W:
		if(*_pnStart > m_rect.x)
			*_pnStart = (int)m_rect.x;
		if(*_pnEnd < m_rect.x+m_rect.z)
			*_pnEnd = (int)(m_rect.x + m_rect.z);
		break;
	case POS_Y:
	case POS_H:
		if(*_pnStart > m_rect.y)
			*_pnStart = (int)m_rect.y;
		if(*_pnEnd < m_rect.y+m_rect.w)
			*_pnEnd = (int)(m_rect.y + m_rect.w);
		break;
	}
}

void BaseUIWindow::pos_get_size_child(int _nPos, int *_pnStart, int *_pnEnd)
{
	bool bFirst = true;
	int nStart = 0, nEnd = 0;

	*_pnStart	= 0;
	*_pnEnd		= 0;

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		((BaseUIWindow*)m_child_stlVp[i])->pos_get_size(_nPos, &nStart, &nEnd);
		if(bFirst)
		{
			*_pnStart	= nStart;
			*_pnEnd		= nEnd;
		}else{
			if(nStart < *_pnStart)
				*_pnStart	= nStart;
			if(nEnd > *_pnEnd)
				*_pnEnd		= nEnd;
		}
	}
}
	
void BaseUIWindow::pos_update()
{
	int nValue;	// update value
	int anRef[3];// 0: target start, 1: target width, 2: this width

	for(int nPos=0; nPos<4; nPos++)
	{
		nValue = (int)m_rect[nPos];
		if(m_apPosWinRef[nPos])
		{
			Vector4 rect;
			const Vector4 &rect_ = m_apPosWinRef[nPos]->pos_get();
			rect = rect_;
			if(m_apPosWinRef[nPos] == get_parent() &&
				m_apPosWinRef[nPos]->type_get() == BaseUIWindow::WIN_TYPE_Base)
			{
				rect.x = rect.y = 0;
			}
			switch(nPos)
			{
			case POS_X:	anRef[0]	= (int)rect.x;	anRef[1]	= (int)rect.z;	anRef[2]	= (int)m_rect.z;	break;
			case POS_Y:	anRef[0]	= (int)rect.y;	anRef[1]	= (int)rect.w;	anRef[2]	= (int)m_rect.w;	break;
			case POS_W:	anRef[0]	= (int)rect.z;	anRef[1]	= 0;			anRef[2]	= 0;		break;
			case POS_H:	anRef[0]	= (int)rect.w;	anRef[1]	= 0;			anRef[2]	= 0;		break;
			}
		}else{
			memset(anRef, 0, sizeof(int)*3);
		}

		switch(m_anPosType[nPos])
		{
		//case POS_Static:	nValue	= anRef[0];	break;
		case POS_Dynamic:
			{
				int s, e;
				pos_get_size_child(nPos, &s, &e);
				nValue	= (e - s);
			}
			break;
		case POS_Start:		nValue	= anRef[0];								break;
		case POS_End:		nValue	= anRef[0] + anRef[1] - anRef[2];		break;
		case POS_Next:		nValue	= anRef[0] + anRef[1] + 1;				break;
		case POS_Center:	nValue	= anRef[0] + anRef[1]/2 - anRef[2]/2;	break;
		case POS_CenterFix:	nValue	= anRef[0] + anRef[1]/2;				break;
		case POS_Percent:	nValue	= (anRef[0] + anRef[1] - anRef[2])*m_anPosOffset[nPos]/100;	break;
		case POS_PercentFix:		nValue	= (anRef[0] + anRef[1])*m_anPosOffset[nPos]/100;	break;
		}

		if(m_anPosType[nPos] == POS_Start || m_anPosType[nPos] == POS_End
			|| m_anPosType[nPos] == POS_Next
			|| m_anPosType[nPos] == POS_Center || m_anPosType[nPos] == POS_CenterFix)
		{
			nValue += m_anPosOffset[nPos];
		}

		if(nValue != m_rect[nPos]
			&& (nPos < 2 || nValue >= m_sizelimit[nPos-2]))
		{
			m_rect[nPos] = (float)nValue;
			pos_update_propogate(nPos);
		}
	}

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		((BaseUIWindow*)m_child_stlVp[i])->pos_update();
	}
}

void BaseUIWindow::pos_ref_release(BaseUIWindow *_pWin, int _nPos)
{
	for(unsigned i=0; i<m_astlVpPosWinChilds[_nPos].size(); i++)
	{
		if(m_astlVpPosWinChilds[_nPos][i] == _pWin)
		{
			m_astlVpPosWinChilds[_nPos].erase(m_astlVpPosWinChilds[_nPos].begin()+i);
			break;
		}
	}
}

int BaseUIWindow::mouse(int _id, int _x, int _y, int _nType)
{
	bool bPropogate	= false;
	bool bCast		= false;
	int nHash = 0;

	int nOffsetX, nOffsetY;

	nOffsetX	= _x - (int)m_rect.x;
	nOffsetY	= _y - (int)m_rect.y;

	if(_x >= m_rect.x && _y >= m_rect.y && _x <= m_rect.x + m_rect.z && _y <= m_rect.y+m_rect.w)
	{
		bPropogate	= true;
		bCast		= true;
	}

	if(m_nWinType == WIN_TYPE_Linker){
		bPropogate	= true;
	}else{
		_x	-= (int)m_rect.x;
		_y	-= (int)m_rect.y;
	}

	if(bPropogate)
	{
		for(unsigned i=0; i<m_child_stlVp.size(); i++)
		{
			int inv = (int)m_child_stlVp.size() - i - 1;
			nHash = ((BaseUIWindow*)m_child_stlVp[inv])->mouse(_id, _x, _y, _nType);
			if(nHash != 0)
				return nHash;
		}
	}

	if(bCast && !m_bEventCastDisable)
	{
		nHash = STRTOHASH(get_name().c_str());
		event_cast(nHash, nOffsetX, nOffsetY, m_stlVEventCod);
	}

	return nHash;
}

void BaseUIWindow::event_cast(int _OriginHash, int _nOffsetX, int _nOffsetY, const STLVInt &_stlVIndex)
{
	if(get_parent() && m_nWinType == WIN_TYPE_Linker)
	{
		((BaseUIWindow*)get_parent())->event_cast(_OriginHash, _nOffsetX, _nOffsetY, _stlVIndex);
		return;
	}

	m_pManager->event_m_cast(m_nidentifier, _OriginHash, _nOffsetX, _nOffsetY, _stlVIndex);
}

int BaseUIWindow::texture_find(int _nHash)
{
	BaseSpaceOrganizer	 *pSpace;

	pSpace	 = texture_space_get();

	if(pSpace)
		return pSpace->space_find(_nHash);

	return -1;
}

int BaseUIWindow::texture_add( int w, int h, char *_pData, int _nHash)
{
	BaseSpaceOrganizer	 *pSpace;

	pSpace	 = texture_space_get();

	if(pSpace)
		return pSpace->space_add(w, h, (bbyte*)_pData, _nHash);

	return -1;
}

BaseSpaceOrganizer *BaseUIWindow::texture_space_get()
{
	if(m_pSpaceOrganizer)
		return m_pSpaceOrganizer;

	if(get_parent())
		return ((BaseUIWindow*)get_parent())->texture_space_get();
	return NULL;
}

void BaseUIWindow::texture_push(ST_Board _stBoard)
{
	if(m_nWinType == WIN_TYPE_Base)
	{
		_stBoard.rect[0] += (int)m_rect.x;
		_stBoard.rect[1] += (int)m_rect.y;
		_stBoard.rect[2] += (int)m_rect.x;
		_stBoard.rect[3] += (int)m_rect.y;
	}

	if(m_pSpaceOrganizer)
		m_stlVsBoards.push_back(_stBoard);

	if(get_parent())
		((BaseUIWindow*)get_parent())->texture_push(_stBoard);
}

bool BaseUIWindow::draw_update()
{	
	for(unsigned i=0; i<m_child_stlVp.size(); i++)
		((BaseUIWindow*)m_child_stlVp[i])->draw_update();

	return true;
}

BaseUIWindow *BaseUIWindow::window_get(int _nID)
{
    if(_nID == identifier_get())
        return this;
    
    BaseUIWindow *pRet = NULL;
    for(unsigned i=0; i<m_child_stlVp.size(); i++)
    {
        pRet = ((BaseUIWindow*)m_child_stlVp[i])->window_get(_nID);
        if(pRet)
            return pRet;
    }
    
    return NULL;
}

void BaseUIWindow::draw()
{
	if(m_bUpdate)
	{
		m_stlVsBoards.clear();
		{
			draw_update();
		}
		
		m_stlVsSpriteMesh.clear();
		{
			m_pSpaceOrganizer->space_reflash(0, 0);
		}

		{
			for(unsigned i=0; i<m_stlVsBoards.size(); i++)
				board_add_(m_stlVsBoards[i].rect, m_stlVsBoards[i].nTextureIndex);
		}

		{
			m_pSprite = m_pManager->sprite_make(m_pSpaceOrganizer, m_pSprite);
		}

		m_bUpdate = false;
	}

	if(m_pSprite != NULL)
	{
		m_pManager->sprite_draw(this, &m_stlVsSpriteMesh, m_pSprite, m_nOrder);
	}
	
	for(unsigned i=0; i<m_child_stlVp.size(); i++)
		((BaseUIWindow*)m_child_stlVp[i])->draw();
}

void BaseUIWindow::board_add_(int *_pnBoard, int _nIndexTexture)
{
    float fColor[4] = {1, 1, 1, 1};
	//int	_nColor = 0xFFFFFFFF;
	int	nUV[4];
	m_pSpaceOrganizer->space_get(_nIndexTexture, nUV);
	
	if(_nIndexTexture == -1)
	{
		nUV[0]	= 0;
		nUV[1]	= 0;
		nUV[2]	= m_pSpaceOrganizer->get_width();
		nUV[3]	= m_pSpaceOrganizer->get_height();
	}

	float fUV[4];
	fUV[0]	= (float)nUV[0];
	fUV[1]	= (float)nUV[1];
	fUV[2]	= (float)nUV[2];
	fUV[3]	= (float)nUV[3];

	///*
	//if(fUV[2] - fUV[0] > 0)
		fUV[2]++;
	//if(fUV[3] - fUV[1] > 0)
		fUV[3]++;
	///*/
	//if(fUV[2] - fUV[0] > 0)
	//	fUV[0]	-= 0.5f;//, fUV[2]	+= 0.5f;
	//if(fUV[3] - fUV[1] > 0)
	//	fUV[1]	-= 0.5f;//, fUV[3]	+= 0.5f;
	////*/

	PtVector3	v3UvCal(0.f, 0.f, 0.f);
	PtVector3	v3Pos[2];
	v3Pos[0].x	= (float)_pnBoard[0];
	v3Pos[0].y	= (float)_pnBoard[3];
	v3Pos[0].z	= 0;

	v3Pos[1].x	= (float)_pnBoard[2];
	v3Pos[1].y	= (float)_pnBoard[1];
	v3Pos[1].z	= 0;

	PtMatrix4	m4;
	m4.MakeIdentity();
	//m4.Set4x4Matrix(_pm4);
	
	PtMatrix4	m4Inv;
	m4.GetInverse(&m4Inv);
	m4Inv.m_f41	= 0;
	m4Inv.m_f42	= 0;
	
	m4Inv.MakeNormal();
	//PtVector3	v3PosCal(0.5f, 0.5f, 0.f);
    PtVector3	v3PosCal(0.5f, -0.5f, 0.f);
	//v3UvCal		= m4Inv * v3UvCal; // 바운드 박스 가로 줄이 나오지 않는 문제가 있어 삭제

	board_vertex	sVer;
	PtVector3	&v3	= *((PtVector3*)&sVer.v[0]);
	sVer.v[0]	= v3Pos[0].x;
	sVer.v[1]	= v3Pos[0].y;
	sVer.v[2]	= 0;
    memcpy(&sVer.c[0], &fColor[0], sizeof(float)*4);
	sVer.t[0]	= (fUV[0]+v3UvCal.x)/(float)m_pSpaceOrganizer->get_width();
	sVer.t[1]	= (fUV[1]+v3UvCal.y)/(float)m_pSpaceOrganizer->get_height();
	v3			= m4 * v3;
	v3			-= v3PosCal;
	m_stlVsSpriteMesh.push_back(sVer);

    sVer.v[0]	= v3Pos[1].x;
    sVer.v[1]	= v3Pos[0].y;
    sVer.v[2]	= 0;
    memcpy(&sVer.c[0], &fColor[0], sizeof(float)*4);
    sVer.t[0]	= (fUV[2]+v3UvCal.x)/(float)m_pSpaceOrganizer->get_width();
    sVer.t[1]	= (fUV[1]+v3UvCal.y)/(float)m_pSpaceOrganizer->get_height();
    v3			= m4 * v3;
    v3			-= v3PosCal;
    m_stlVsSpriteMesh.push_back(sVer);
    
    sVer.v[0]	= v3Pos[1].x;
    sVer.v[1]	= v3Pos[1].y;
    sVer.v[2]	= 0;
    memcpy(&sVer.c[0], &fColor[0], sizeof(float)*4);
    sVer.t[0]	= (fUV[2]+v3UvCal.x)/(float)m_pSpaceOrganizer->get_width();
    sVer.t[1]	= (fUV[3]+v3UvCal.y)/(float)m_pSpaceOrganizer->get_height();
    v3			= m4 * v3;
    v3			-= v3PosCal;
    m_stlVsSpriteMesh.push_back(sVer);

	sVer.v[0]	= v3Pos[0].x;
	sVer.v[1]	= v3Pos[0].y;
    sVer.v[2]	= 0;
    memcpy(&sVer.c[0], &fColor[0], sizeof(float)*4);
	sVer.t[0]	= (fUV[0]+v3UvCal.x)/(float)m_pSpaceOrganizer->get_width();
	sVer.t[1]	= (fUV[1]+v3UvCal.y)/(float)m_pSpaceOrganizer->get_height();
	v3			= m4 * v3;
	v3			-= v3PosCal;
	m_stlVsSpriteMesh.push_back(sVer);

	sVer.v[0]	= v3Pos[1].x;
	sVer.v[1]	= v3Pos[1].y;
    sVer.v[2]	= 0;
    memcpy(&sVer.c[0], &fColor[0], sizeof(float)*4);
	sVer.t[0]	= (fUV[2]+v3UvCal.x)/(float)m_pSpaceOrganizer->get_width();
	sVer.t[1]	= (fUV[3]+v3UvCal.y)/(float)m_pSpaceOrganizer->get_height();
	v3			= m4 * v3;
	v3			-= v3PosCal;
	m_stlVsSpriteMesh.push_back(sVer);

	sVer.v[0]	= v3Pos[0].x;
	sVer.v[1]	= v3Pos[1].y;
    sVer.v[2]	= 0;
    memcpy(&sVer.c[0], &fColor[0], sizeof(float)*4);
    sVer.t[0]	= (fUV[0]+v3UvCal.x)/(float)m_pSpaceOrganizer->get_width();
	sVer.t[1]	= (fUV[3]+v3UvCal.y)/(float)m_pSpaceOrganizer->get_height();
	v3			= m4 * v3;
	v3			-= v3PosCal;
	m_stlVsSpriteMesh.push_back(sVer);
}

BaseUIWinTexture::BaseUIWinTexture(BaseUIManager *_pManager, const char *_strName, int _nIdentifier, int _nTextureIndex, int _nWinType)
	: BaseUIWindow(_pManager, _strName, _nIdentifier, _nWinType)
{
	m_nTextureIndex	= _nTextureIndex;
}

BaseUIWinTexture::~BaseUIWinTexture(void)
{
}

void BaseUIWinTexture::set_index(int _nIndex)
{
	if(m_nTextureIndex == _nIndex)
		return;

	BaseUIWindow *pWin = NULL;
	pWin = texture_win();

	if(pWin)
		pWin->updated();

	m_nTextureIndex	= _nIndex;
}

void BaseUIWindow::pos_size_update()
{
}

void BaseUIWindow::attach_child(BaseTree *_pTree)
{
	BaseTree::attach_child(_pTree);

	((BaseUIWindow*)_pTree)->pos_size_update();
	((BaseUIWindow*)_pTree)->pos_update();
}

void BaseUIWinTexture::pos_size_update()
{
	BaseSpaceOrganizer *pSpace = texture_space_get();
	if(pSpace)
	{
		int w, h;
		w	= pSpace->get_width(m_nTextureIndex);
		h	= pSpace->get_height(m_nTextureIndex);
		BaseUIWindow::pos_set_size(w, h);
	}
}

void BaseUIWinTexture::pos_update()
{
	BaseUIWindow::pos_update();

	int nValue;	// update value
	int anRef[3];// 0: target start, 1: target width, 2: this width
	
	BaseSpaceOrganizer *pSpace = texture_space_get();
	int width=0, height=0;
	if(pSpace)
	{
		width	= pSpace->get_width(m_nTextureIndex);
		height	= pSpace->get_height(m_nTextureIndex);
	}

	for(int nPos=2; nPos<4; nPos++)
	{
		nValue = (int)m_rect[nPos];

		switch(nPos)
		{
		case POS_W:	anRef[0]	= (int)width;	anRef[1]	= 0;			anRef[2]	= 0;		break;
		case POS_H:	anRef[0]	= (int)height;	anRef[1]	= 0;			anRef[2]	= 0;		break;
		}

		switch(m_anPosType[nPos])
		{
		case POS_Static:	nValue	= anRef[0];	break;
		}

		if(nValue != m_rect[nPos])
		{
			m_rect[nPos] = (float)nValue;
			pos_update_propogate(nPos);
		}
	}
}

void BaseUIWinTexture::pos_set(int _x, int _y)
{
	BaseUIWindow::pos_set(_x, _y);

	BaseSpaceOrganizer *pSpace = texture_space_get();
	if(pSpace)
	{
		int w, h;
		w	= pSpace->get_width(m_nTextureIndex);
		h	= pSpace->get_height(m_nTextureIndex);
		BaseUIWindow::pos_set_size(w, h);
	}
}

bool BaseUIWinTexture::set_index(const char *_strTextureFile)
{
	int nTex = m_pManager->texture_load(this, _strTextureFile);

	if(nTex == -1)
		return false;

	set_index(nTex);
	return true;
}

int BaseUIWinTexture::get_index()
{
	return m_nTextureIndex;
}

void BaseUIWindow::pos_global_get(Vector4 *_prect)
{
	void *pParent	= get_parent();

	if(pParent)
	{
		if(((BaseUIWindow*)get_parent())->type_get() == WIN_TYPE_Base)
		{
			Vector4 rect = ((BaseUIWindow*)get_parent())->pos_get();
			_prect->x	+= rect.x;
			_prect->y	+= rect.y;
		}
		((BaseUIWindow*)get_parent())->pos_global_get(_prect);
	}
}

bool BaseUIWindow::is_blinded()
{
	if(m_pWinBound)
	{
		Vector4 bound = m_pWinBound->pos_get();
		m_pWinBound->pos_global_get(&bound);
		Vector4 win_rect = m_rect;
		pos_global_get(&win_rect);
	
		if(bound.x > win_rect.x
			|| bound.y > win_rect.y
			|| bound.x+bound.z < win_rect.x+win_rect.z
			|| bound.y+bound.w < win_rect.y+win_rect.w)
				return true;
	}

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		if(((BaseUIWindow*)m_child_stlVp[i])->is_blinded())
			return true;
	}
	return false;
}

bool BaseUIWindow::is_blind_complete()
{
	if(m_pWinBound)
	{
		Vector4 bound = m_pWinBound->pos_get();
		m_pWinBound->pos_global_get(&bound);
		Vector4 win_rect = m_rect;
		pos_global_get(&win_rect);
		if(bound.x > win_rect.x+win_rect.z
			|| bound.y > win_rect.y+win_rect.w
			|| bound.x+bound.z < win_rect.x
			|| bound.y+bound.w < win_rect.y)
		{	
			return true;
		}
	}
	return false;
}

bool BaseUIWinTexture::draw_update()
{
	if(is_blind_complete())
		return false;

	ST_Board stBoard;
	stBoard.rect[0] = (int)m_rect.x;
	stBoard.rect[1] = (int)m_rect.y;
	stBoard.rect[2] = (int)(m_rect.x+m_rect.z);
	stBoard.rect[3] = (int)(m_rect.y+m_rect.w);
	stBoard.nTextureIndex	= m_nTextureIndex;

	if(m_pSpaceOrganizer)
	{
		m_stlVsBoards.push_back(stBoard);
	}else{
		if(get_parent())
			((BaseUIWindow*)get_parent())->texture_push(stBoard);
	}

	if(!BaseUIWindow::draw_update())
		return false;

	return true;
}

BaseUIWinText::BaseUIWinText(BaseUIManager *_pManager, const char *_strName, int _nIdentifier, int _nWinType)
	: BaseUIWindow(_pManager, _strName, _nIdentifier, _nWinType)
{
}
BaseUIWinText::~BaseUIWinText(void)
{
}

void BaseUIWinText::set_color(int _nColor)
{
	m_nColor = _nColor;
}

void BaseUIWinText::set_text(int _nIndex, const char *_strText)
{
	if(_nIndex > 50){
		B_ASSERT(_nIndex <= 50);
		return;
	}

	if(_strText == NULL && _nIndex < (int)m_stlVstrText.size())
	{
		m_stlVstrText.erase(m_stlVstrText.begin()+_nIndex);
	}else if(_strText != NULL){
		if(_nIndex >= (int)m_stlVstrText.size()){
			while(_nIndex >= (int)m_stlVstrText.size())
				m_stlVstrText.push_back("");
		}

		m_stlVstrText[_nIndex]	= _strText;
	}

	int h = (int)m_pManager->font_size_get(m_nFont) * 2 * (int)m_stlVstrText.size();

	pos_set_size((int)m_rect.z, h);
}

void BaseUIWinText::text_clear()
{
	m_stlVstrText.clear();
}

void BaseUIWinText::set_text(int _nFont, int _nColor, int _nOrder, int _nFlag)
{
	m_nFont		= _nFont;
	m_nColor	= _nColor;
	m_nOrder	= _nOrder;
	m_nFlag		= _nFlag;
}

const char *BaseUIWinText::get_text(int _nIndex)
{
	if(_nIndex >= (int)m_stlVstrText.size())
		return NULL;

	return m_stlVstrText[_nIndex].c_str();
}

void BaseUIWinText::draw()
{
	BaseUIWindow::draw();

	Vector4 rect;
	rect = m_rect;
	pos_global_get(&rect);

	m_pManager->draw_text(rect, m_stlVstrText, m_nFont, m_nColor, m_nOrder, m_nFlag);
}

unsigned BaseUIWinText::get_line_count()
{
	return (unsigned)m_stlVstrText.size();
}
