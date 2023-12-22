#include "stdafx.h"

#include "BaseResResource.h"
#include "BaseState.h"
#include "BaseStateManager.h"
#include "BaseResManager.h"
#include "BaseStateSpace.h"

#include "BaseUIManager.h"
#include "BaseUIWindow.h"
#include "BaseSpaceOrganizer.h"
#include <assert.h>
#include "ResTif.h"

BaseUIManager	*BaseUIManager::m_pManager	= NULL;

BaseUIManager *BaseUIManager::get_manager()
{
	return m_pManager;
}

int s_nUISerial = 90000;
int BaseUIManager::serial_new()
{
    return s_nUISerial++;
}

BaseUIManager::BaseUIManager(void)
{
	REGISTEOBJECT(ResTif::ECARE_RES_TEXTURE, ResTif);

    m_stlVMouse.resize(20);
    memset(&m_stlVMouse[0], 0, sizeof(ui_mouse));
    m_nx = 0;
    m_ny = 0;
}

void BaseUIManager::release()
{	
	for(unsigned i=0; i<m_stlVpWindows.size(); i++)
		((BaseUIWindow*)m_stlVpWindows[i])->dec_ref(this);

	m_stlVpWindows.clear();

	for(unsigned i=0; i<m_stlVpFreeSpaceOrganizer.size(); i++)
		delete m_stlVpFreeSpaceOrganizer[i];
	m_stlVpFreeSpaceOrganizer.clear();

	STLMnList::iterator	it;
	it	= m_stlMnLists.begin();
	for(;it!=m_stlMnLists.end(); it++)
		delete it->second;

	m_stlMnLists.clear();
}

BaseUIManager::~BaseUIManager(void)
{
	release();
}

void BaseUIManager::window_release(int _nUIId)
{
	for(unsigned i=0; i<m_stlVpWindows.size(); i++)
	{
		BaseUIWindow *pWin = (BaseUIWindow*)m_stlVpWindows[i];

		if(pWin->identifier_get() == _nUIId)
        {
			pWin->dec_ref(this);
			m_stlVpWindows.erase(m_stlVpWindows.begin()+i);
            break;
		}
	}
}

void BaseUIManager::window_release(BaseUIWindow *_pWin)
{
	_pWin->dec_ref(this);
	m_stlVpWindows.erase(std::find(m_stlVpWindows.begin(), m_stlVpWindows.end(), _pWin));
}

BaseUIWindow *BaseUIManager::window_get(int _nIdentifier)
{
	if(_nIdentifier == 0 && m_stlVpWindows.size() > 0)
		return (BaseUIWindow*)m_stlVpWindows[0];

    BaseUIWindow *pRet = NULL;
	for(unsigned i=0; i<m_stlVpWindows.size(); i++)
	{
        pRet = ((BaseUIWindow*)m_stlVpWindows[i])->window_get(_nIdentifier);
        if(pRet)
            return pRet;
	}
	return NULL;
}

BaseUIWindow *BaseUIManager::window_create_root(const char *_strName, const char *_strTexture, int _nId)
{
	BaseUIWinTexture *pWin = new BaseUIWinTexture(this, _strName, _nId, -1, BaseUIWindow::WIN_TYPE_Base);
	
	pWin->organizer_create();
	pWin->inc_ref(this);

	int nTex = texture_load(pWin, _strTexture);
	if(nTex != -1)
	{
		pWin->set_index(nTex);
	}
	m_stlVpWindows.push_back(pWin);

	return pWin;
}

BaseUIWindow *BaseUIManager::window_create_root(const char *_strName, int _nId)
{
	BaseUIWindow *pWin = new BaseUIWindow(this, _strName, _nId);
	
	pWin->organizer_create();
	pWin->inc_ref(this);
	m_stlVpWindows.push_back(pWin);

	return pWin;
}

void BaseUIManager::update()
{
	for(unsigned i=0; i<m_stlVpWindows.size(); i++)
	{
		((BaseUIWindow*)m_stlVpWindows[i])->draw();
	}
}

int BaseUIManager::mouse(int _id, int _x, int _y, int _nType)
{
    if(_id >= (int)m_stlVMouse.size())
        return 0;
    
	int nRet = 0;

	if(_nType == MOUSE_SCROLL)
	{
		m_stlVMouse[_id].sx	= _x;
		m_stlVMouse[_id].sy	= _y;

		_x	= m_stlVMouse[_id].x;
		_y	= m_stlVMouse[_id].y;
	}else{
		m_stlVMouse[_id].x	= _x;
		m_stlVMouse[_id].y	= _y;
	}
	m_stlVMouse[_id].event	= _nType;

	m_nMouseCur	= _id;

	int lx, ly;

	lx = _x - m_nx;
	ly = _y - m_ny;

	for(int i=(int)m_stlVpWindows.size()-1; i>=0; i--)
	{
		//m_nIdCur	= ((BaseUIWindow*)m_stlVpWindows[i])->identifier_get();
		nRet = ((BaseUIWindow*)m_stlVpWindows[i])->mouse(_id, lx, ly, _nType);
		if(nRet != 0)
			break;
	}
	
	if(nRet == 0 && m_stlVpWindows.size() > 0)
	{
		//m_nIdCur	= ((BaseUIWindow*)m_stlVpWindows[0])->identifier_get();
		STLVInt stlVParam;
		event_m_cast(0, 0, 0, 0, stlVParam);
	}

	m_stlVMouse[_id].ox	= _x;
	m_stlVMouse[_id].oy	= _y;

	return nRet;
}

Vector2 BaseUIManager::get_pos()
{
	Vector2 pos;
	pos.x	= (float)m_nx;
	pos.y	= (float)m_ny;

	return pos;
}

void BaseUIManager::set_size(int _w, int _h)
{
	m_nWidth	= _w;
	m_nHeight	= _h;
}

void BaseUIManager::set_pos(int _x, int _y)
{
	m_nx	= _x;
	m_ny	= _y;
}

void BaseUIManager::event_m_cast(int _nUIId, int _nHashCaster, int _nOffsetX, int _nOffsetY, const STLVInt &_stlVParam)
{
	int x, y, button = 0;
	bool bCast = false;

	x = m_stlVMouse[m_nMouseCur].x;
	y = m_stlVMouse[m_nMouseCur].y;

	m_stlVMouse[m_nMouseCur].ofx	= _nOffsetX;
	m_stlVMouse[m_nMouseCur].ofy	= _nOffsetY;
	
	m_stlVMouse[m_nMouseCur].anEventCoord[0] = (int)_stlVParam.size();
	for(int i=0; i<(int)_stlVParam.size(); i++)
	{
		m_stlVMouse[m_nMouseCur].anEventCoord[i+1]	= _stlVParam[i];
	}

	switch(m_stlVMouse[m_nMouseCur].event)
	{
	case MOUSE_LPUSH:
	case MOUSE_LUP:
		button = 0;
		break;
	case MOUSE_RPUSH:
	case MOUSE_RUP:
		button = 1;
		break;
	case MOUSE_MPUSH:
	case MOUSE_MUP:
		button = 2;
		break;
	}

	int nOldEvent = m_stlVMouse[m_nMouseCur].oEvent;
	int nEventTarget = _nUIId;
	int nOldUI = m_stlVMouse[m_nMouseCur].nOldIdentifier;
	int nOldIndex = m_stlVMouse[m_nMouseCur].nIndex;

	if(nOldEvent == MOUSE_RPUSH
		|| nOldEvent == MOUSE_LPUSH
		|| nOldEvent == MOUSE_MPUSH)
		nEventTarget = nOldUI;

	switch(m_stlVMouse[m_nMouseCur].event)
	{
	case MOUSE_NONE:
		event_cast(nEventTarget, button, SMS_MOVE, _nUIId, _stlVParam);
		break;
	case MOUSE_SCROLL:
		event_cast(nEventTarget, button, SMS_SCROLL, _nUIId, _stlVParam);
		break;
	case MOUSE_RPUSH:
	case MOUSE_MPUSH:
	case MOUSE_LPUSH:
		m_stlVMouse[m_nMouseCur].ex	= m_stlVMouse[m_nMouseCur].x;
		m_stlVMouse[m_nMouseCur].ey	= m_stlVMouse[m_nMouseCur].y;
		m_stlVMouse[m_nMouseCur].oEvent = m_stlVMouse[m_nMouseCur].event;
		event_cast(_nUIId, button, SMS_PUSH, _nUIId, _stlVParam);
		break;
	case MOUSE_RUP:
	case MOUSE_MUP:
	case MOUSE_LUP:
		if(m_stlVMouse[m_nMouseCur].oEvent == m_stlVMouse[m_nMouseCur].event-1
			&& m_stlVMouse[m_nMouseCur].nOldIdentifier == _nUIId)// Befor is Push
		{
			int nLx, nLy;
			nLx	= x - m_stlVMouse[m_nMouseCur].ex;
			nLy	= y - m_stlVMouse[m_nMouseCur].ey;

			int nLen;
			nLen	= nLx*nLx + nLy*nLy;

			if(nLen <= 1)
			{
                bCast = true; event_cast(nEventTarget, button, SMS_CLICK, _nUIId, _stlVParam);// Send click event
				g_SendMessage(LOG_MSG, "Cast Click message ID:%d, Hash:%d\n", _nUIId, nEventTarget);
			}
		}
	
		if(!bCast)
			event_cast(nEventTarget, button, SMS_UP, _nUIId, _stlVParam);

		m_stlVMouse[m_nMouseCur].oEvent = m_stlVMouse[m_nMouseCur].event;
		break;
	}

	m_stlVMouse[m_nMouseCur].ox = m_stlVMouse[m_nMouseCur].x;
	m_stlVMouse[m_nMouseCur].oy = m_stlVMouse[m_nMouseCur].y;

	STLVInt	stlVEventCoordinateOld;
	for(int i=1; i<m_stlVMouse[m_nMouseCur].anOEventCoord[0]+1; i++)
	{
		stlVEventCoordinateOld.push_back(m_stlVMouse[m_nMouseCur].anOEventCoord[i]);
	}
	int nOldIdentifier;
	nOldIdentifier	= m_stlVMouse[m_nMouseCur].nOldIdentifier;

	if(nOldEvent == MOUSE_LPUSH)
		return; // Event target can't change without mouse release.
	
	if(nOldUI != 0 && (nOldUI != _nUIId || nOldIndex != _nHashCaster))
	{
		event_cast(nOldUI, 0, SMS_OUT, nOldIdentifier, stlVEventCoordinateOld);
		event_cast(_nUIId, 0, SMS_OVER, _nUIId, _stlVParam);
	}

	m_stlVMouse[m_nMouseCur].anOEventCoord[0] = (int)_stlVParam.size();
	for(int i=0; i<(int)_stlVParam.size(); i++)
	{
		m_stlVMouse[m_nMouseCur].anOEventCoord[i+1]	= _stlVParam[i];
	}
	m_stlVMouse[m_nMouseCur].nOldIdentifier	= _nUIId;
	//m_stlVMouse[m_nMouseCur].nCurUI = _nUIId;
	m_stlVMouse[m_nMouseCur].nIndex = _nHashCaster;
}

BaseSpaceOrganizer *BaseUIManager::space_get()
{
	BaseSpaceOrganizer *pSpace = NULL;
	if(m_stlVpFreeSpaceOrganizer.size() > 0)
	{
		pSpace	= m_stlVpFreeSpaceOrganizer[0];
		m_stlVpFreeSpaceOrganizer.erase(m_stlVpFreeSpaceOrganizer.begin());
		return pSpace;
	}

	pSpace = new BaseSpaceOrganizer(512, 512, 4);
	return pSpace;
}

void BaseUIManager::space_release(BaseSpaceOrganizer *_pSpace)
{
	m_stlVpFreeSpaceOrganizer.push_back(_pSpace);		
}

STLVStlVpVoid *BaseUIManager::list_get(int _nHash)
{
	STLMnList::iterator	it;
	it = m_stlMnLists.find(_nHash);

	if(it != m_stlMnLists.end())
	{
		return it->second;
	}

	STLVStlVpVoid *pstlVpWin = new STLVStlVpVoid;
	m_stlMnLists[_nHash]	= pstlVpWin;

	return pstlVpWin;
}

const char *texture_path_get(const char *_strTexture)
{	
//	static char strPath[255];
//	if(_strTexture)
//	{
//		if(strncmp(_strTexture, "./", 2) == 0)
//			_strTexture += 2;
//
//		_strTexture = BaseSystem::get_root_path(strPath, 255, _strTexture);
//		for(int i=0; i<255; i++)
//		{
//			if(strPath[i] == '/')
//				strPath[i] = SLASH_C;
//		}
//	}
	return _strTexture;
}

BaseUIWindow *BaseUIManager::window_create(int _nIdentifier, const char *_strName, int _nHashUI, int _nWinType, int _nPosTypeX, int _nPosTypeY, BaseUIWindow *_pWinRefPos, BaseUIWindow *_pWinBoundBox)
	// _strName can same with _nHashUI but in list window has different name , it include coordinate index
{
	const char *strName, *strNameTif;

	strName	= (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), _nHashUI, STRTOHASH("strName"));
	if(strName == NULL)
		return NULL;

	strNameTif	= (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), _nHashUI, STRTOHASH("UITextureFilename_strV"));
	
	BaseUIWindow *pWinChild;
	
	if(strNameTif)
	{
		strNameTif = texture_path_get(strNameTif);
		pWinChild	= window_create_texture(_strName, _nIdentifier, strNameTif, _nWinType, _pWinRefPos);
	}else{
		const int *pnSize;
		short	nCnt = 0;
		pnSize	= (const int*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), _nHashUI, STRTOHASH("ScreenSize"), &nCnt);
		
		if(nCnt == 2)
		{
			pWinChild	= new BaseUIWindow(this, _strName, _nIdentifier, _nWinType);
			pWinChild->pos_set(0, 0, *(pnSize+0), *(pnSize+1));
		}else if(nCnt == 4){
			pWinChild	= new BaseUIWindow(this, _strName, _nIdentifier, _nWinType);
			pWinChild->pos_set(*(pnSize+0), *(pnSize+1), *(pnSize+2), *(pnSize+3));
		}else if(nCnt == 7){
			pWinChild	= new BaseUIWinText(this, _strName, _nIdentifier, _nWinType);
			pWinChild->pos_set(*(pnSize+0), *(pnSize+1), *(pnSize+2), *(pnSize+3));
			((BaseUIWinText*)pWinChild)->set_text(*(pnSize+4), -1, *(pnSize+5), *(pnSize+6));
		}else{
			pWinChild	= new BaseUIWindow(this, _strName, _nIdentifier, _nWinType);
		}
	}

	if(_pWinRefPos)
	{
		pWinChild->pos_set(BaseUIWindow::POS_X, _nPosTypeX, _pWinRefPos);
		pWinChild->pos_set(BaseUIWindow::POS_Y, _nPosTypeY, _pWinRefPos);
	}

	if(_pWinBoundBox)
		pWinChild->pos_bound_win_set(_pWinBoundBox);

	return pWinChild;
}

BaseUIWinTexture *BaseUIManager::window_create_texture(const char *_strName, int _nIdentifier, const char *_strTexture, int _nWinType, BaseUIWindow *_pWinBase)
{
	int nTIndex;

    BaseUIWindow *pWin	= _pWinBase;//window_get(_nIdentifier);

	if(pWin == NULL)
		return NULL;

	BaseUIWinTexture *pTex = NULL;

	nTIndex = texture_load(pWin, _strTexture);
	if(nTIndex == -1)
	{	
		return NULL;
	}

	pTex = new BaseUIWinTexture(this, _strName, _nIdentifier, nTIndex, _nWinType);
	return pTex;
}

STLVInt BaseUIManager::ui_context_get(BaseDStructureValue *_pdsvContext)
{
    const int *apUI;
    short   nCnt;
    STLVInt stlVUIs;
    
    if(STDEF_AGET(_pdsvContext, UIBoardHolder_anC, apUI, nCnt))
    {
        for(int i=0; i<nCnt; i++)
            stlVUIs.push_back(apUI[i]);
    }
    
    return stlVUIs;
}

void BaseUIManager::ui_context_add(BaseDStructureValue *_pdsvContext, int _nUIId)
{
    STLVInt stlVpUI = ui_context_get(_pdsvContext);
    stlVpUI.push_back(_nUIId);
    _pdsvContext->set_alloc("UIBoardHolder_anC", (const void*)&stlVpUI[0], (short)stlVpUI.size());
}

BaseUIWindow *BaseUIManager::ui_context_parent(BaseStateMain *_pStateMain, BaseDStructureValue *_pdsvContext, int _nHash)
{
    STLVInt stlVIds = BaseUIManager::ui_context_get(_pdsvContext);
	for(unsigned i=0; i<stlVIds.size(); i++)
    {
        BaseUIWindow *pWinTemp = NULL;
        pWinTemp = BaseUIManager::get_manager()->window_get(stlVIds[i]);
        if(pWinTemp &&
           (_nHash == 0 || STRTOHASH(pWinTemp->get_name().c_str()) == _nHash) )
            return pWinTemp;
        
    }
    
    //int *pnIdStateParent;
    STLMnInt stlMnGroupId;
    BaseState::group_id_get(_pdsvContext, HASH_STATE(BaseIdentifierParent_n), stlMnGroupId);
    //STDEF_GET_R(_pdsvContext, BaseIdentifierParent_n, pnIdStateParent);
    _pdsvContext = _pStateMain->GetContext(stlMnGroupId);
    
    if(_pdsvContext)
        return ui_context_parent(_pStateMain, _pdsvContext, _nHash);
    return NULL;
}

STDEF_FUNC(UIBoardCreate_nF)
{
	int nHash;
	if(!_pdsvBase->get("UIBoardCreate_nF", &nHash, NULL, _pdsvBase->sequence_get_local()))
		return 0;

	const char *strName;
	const char *strNameTif;

	strName	= (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), nHash, STRTOHASH("strName"));
	if(strName == NULL)
		return 0;

	strNameTif	= (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), nHash, STRTOHASH("UITextureFilename_strV"));
	
    int nUIId, nStateId;

	BaseState *pState = (BaseState*)_pdsvBase->m_pVoidParam;

    nStateId = pState->identifier_get( HASH_STATE(StateUI) );
    nUIId	= BaseUIManager::serial_new();
	
	BaseUIManager *pUIManager = BaseUIManager::get_manager();
	
	if(strNameTif)
		strNameTif = texture_path_get(strNameTif);

    BaseUIWindow	*pWinChild = NULL;
    BaseUIWindow *pWinParent = NULL;
    const int *pnHashBase;
    BaseStateMain *pStateMain = pState->get_state_main();
    //int ofx=0, ofy=0;
    if(_pdsvBase->get_look_up(STRTOHASH("UIBoardBase_nV"), (const void**)&pnHashBase, NULL))
    {
        pWinParent = BaseUIManager::ui_context_parent(pStateMain, _pdsvContext, *pnHashBase);
    }else{
        //int *pnIdStateParent;
        STLMnInt stlMnGroupId;
        BaseState::group_id_get(_pdsvContext, HASH_STATE(BaseIdentifierParent_n), stlMnGroupId);
        
        //STDEF_GET_R(_pdsvContext, BaseIdentifierParent_n, pnIdStateParent);
        BaseDStructureValue *pdsvContext = pStateMain->GetContext(stlMnGroupId);
        
        if(pdsvContext)
            pWinParent = BaseUIManager::ui_context_parent(pStateMain, pdsvContext, 0);
        
        if(pWinParent == NULL)
        {
            pWinParent = pUIManager->window_get(0);
            pWinChild->type_set(BaseUIWindow::WIN_TYPE_Base);
        }
    }
    
	if(strNameTif)
		pWinChild	= pUIManager->window_create_texture(strName, nUIId, strNameTif, BaseUIWindow::WIN_TYPE_Base, pWinParent);
	else
		pWinChild	= new BaseUIWindow(pUIManager, strName, nUIId, BaseUIWindow::WIN_TYPE_Base);

	const int *pnPos;
	short nCnt;
	if(!_pdsvBase->get(STRTOHASH("UIBoardCreatePos_anV"), (const void**)&pnPos, &nCnt, _pdsvBase->sequence_get_local()))
		return 0;

	pWinParent->attach_child(pWinChild);
	pWinChild->pos_set(BaseUIWindow::POS_X, BaseUIWindow::POS_Static, pWinParent);
	pWinChild->pos_set(BaseUIWindow::POS_Y, BaseUIWindow::POS_Static, pWinParent);
	pWinChild->pos_set(BaseUIWindow::POS_W, BaseUIWindow::POS_Static, pWinParent);
	pWinChild->pos_set(BaseUIWindow::POS_H, BaseUIWindow::POS_Static, pWinParent);
	
	if(nCnt >= 5)
	{
		if(*(pnPos+2) == 0 && *(pnPos+3) == 0)
		{
			pWinChild->pos_set(*pnPos, *(pnPos+1));
		}else{
			pWinChild->pos_set(*pnPos, *(pnPos+1), *(pnPos+2), *(pnPos+3));
		}

		if(*(pnPos+4) == 1)
			pWinChild->event_cast_disable();
	}else if(nCnt >= 4)
		pWinChild->pos_set(*pnPos, *(pnPos+1), *(pnPos+2), *(pnPos+3));
	else if(nCnt >= 2)
		pWinChild->pos_set(*pnPos, *(pnPos+1));

	//if(nCnt >= 6 && strNameTif == NULL)
	//	pWin->board_add(strName, *pnPos+ofx, *(pnPos+1)+ofy, *(pnPos+2), *(pnPos+3), *(pnPos+4), *(pnPos+5));//x y w h cnt gap_h

    BaseUIManager::ui_context_add(_pdsvContext, nUIId);
    pUIManager->identify_link_set(nUIId, nStateId);
    //pState->identifier_set(nUIId);
	return 1;
}

STDEF_FUNC(UIBoardUpdate_nF)
{
	int nHash;
	if(!_pdsvBase->get("UIBoardUpdate_nF", &nHash, NULL, _pdsvBase->sequence_get_local()))
		return 0;

	int nHashBase;
	if(!_pdsvBase->get("UIBoardBase_nV", &nHashBase, NULL, _pdsvBase->sequence_get_local()))
		return 0;
	
	const char *strNameTif;

	strNameTif	= (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), nHash, STRTOHASH("UITextureFilename_strV"));
	if(strNameTif == NULL)
		return 0;

	if(strNameTif)
		strNameTif = texture_path_get(strNameTif);

	BaseUIWinTexture *pTexture = NULL;
    STLVInt stlVIds = BaseUIManager::ui_context_get(_pdsvContext);
    for(unsigned i=0; i<stlVIds.size(); i++)
    {
        BaseUIWindow *pWinTemp = NULL;
        pWinTemp = BaseUIManager::get_manager()->window_get(stlVIds[i]);
        if(pWinTemp && STRTOHASH(pWinTemp->get_name().c_str()) == nHashBase)
        {
            pTexture = (BaseUIWinTexture*)pWinTemp;
            break;
        }
    }
    
    if(!pTexture)
        return 0;
    
	int nTex;
	nTex = BaseUIManager::get_manager()->texture_load(pTexture, strNameTif);
	if(nTex >= 0)
	{
        pTexture->set_index(nTex);
        pTexture->pos_move(0, 0);
		return 1;
	}
	return 0;
}


STDEF_FUNC(UIBoardMouseMove_nF)
{
    const int *pnHash;
    STDEF_GET_R(_pdsvBase, UIBoardMouseMove_nF, pnHash);
    
    BaseUIWindow *pWin = NULL;
    STLVInt stlVIds = BaseUIManager::ui_context_get(_pdsvContext);
	for(unsigned i=0; i<stlVIds.size(); i++)
    {
        BaseUIWindow *pWinTemp = NULL;
        pWinTemp = BaseUIManager::get_manager()->window_get(stlVIds[i]);
        if(pWinTemp && STRTOHASH(pWinTemp->get_name().c_str()) == *pnHash)
        {
            pWin = pWinTemp;
            break;
        }
    }
    
    if(!pWin)
        return 0;
    
    const int *anMove;
    short nCnt;
    STDEF_AGET_R(_pdsvEvent, BaseMouseDelta_anV, anMove, nCnt);
    pWin->pos_move(anMove[0], anMove[1]);
    
    return 1;
}

STDEF_FUNC(UIWindowPos_anF)
{
	const int *pnPos;
	short nCnt;
	STDEF_AGET_R(_pdsvBase, UIWindowPos_anF, pnPos, nCnt);

	int nIdentifier;

	BaseState *pState = (BaseState*)_pdsvBase->m_pVoidParam;

	nIdentifier	= pState->identifier_get(HASH_STATE(StateUI));
	BaseUIWindow *pWin;
	
	BaseUIManager *pUIManager = (BaseUIManager *)BaseUIManager::get_manager();
	pWin = pUIManager->window_get(nIdentifier);
	pWin->pos_set(*pnPos, *(pnPos+1));

	if(nCnt > 2 && *(pnPos+2) != 0)
		pWin->event_cast_disable();

	return 1;
}

STDEF_FUNC(MouseEvent_nF)
{
	short nCnt;
	const int *pnValue;

	STDEF_AGET_R(_pdsvBase, MouseEvent_Values_anV, pnValue, nCnt);

	short nCnt2;
	const int *pnEValue;
	STDEF_AGET_R(_pdsvEvent, MouseEvent_Values_anV, pnEValue, nCnt2);

	if(nCnt < 2)
		return 0;
	if(nCnt >= 4 && nCnt2 >= 7 && *(pnValue+3) != *(pnEValue+7))
		return 0;
	if(nCnt >= 3 && nCnt2 >= 6 && *(pnValue+2) != *(pnEValue+6))
		return 0;

	if(*pnValue == *pnEValue && *(pnValue+1) == *(pnEValue+1))
		return 1;
	//MouseEvent_Values_anV
	return 0;
}

STDEF_FUNC(MouseEventCheckUI_nF)
{
	const int *pnHashTarget;
	STDEF_GET_R(_pdsvBase, MouseEventCheckUI_nF, pnHashTarget);
	//==========================================================================================
	// Check Control
	const int *panValues;
	short nCnt;
	int nX=0, nY=0;
	bool bControl = false;
	if(_pdsvEvent->get(STRTOHASH("MouseEvent_Values_anV"), (const void**)&panValues, &nCnt))
	{
		if(nCnt >= 8)
		{
			nX	= *(panValues+6);
			nY	= *(panValues+7);
			bControl	= true;
		}
	}
	//-------------------------------------------------------------------------------------------
	BaseUIWinTexture *pTexture;
	BaseUIManager *pUIManager = (BaseUIManager *)BaseUIManager::get_manager();

	int nIdentifier;
	int nHash;

	if(!_pdsvEvent->get("MouseEvent_nF", &nHash))
		return 0;

	if(!_pdsvEvent->get("nTargetIdentifier", &nIdentifier))
		return 0;

	const char *strName, *strNameTif;
	strName		= (const char *)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), nHash, STRTOHASH("strName"));
	strNameTif	= (const char *)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), *pnHashTarget, STRTOHASH("UITextureFilename_strV"));

	if(strNameTif)
		strNameTif = texture_path_get(strNameTif);
    
    STDEF_BaseState(pState);
	if(!bControl)
        pTexture = (BaseUIWinTexture*)BaseUIManager::ui_context_parent(pState->get_state_main(), _pdsvContext, *pnHashTarget);
	else{
		STLVStlVpVoid *pstlVstlVControl = pUIManager->list_get(nHash);
		pTexture = (BaseUIWinTexture*)(*pstlVstlVControl)[nY][nX];
	}

	if(pTexture == NULL)
		return 0;

	int nTex;
	nTex = pUIManager->texture_load(pTexture, strNameTif);
	
	if(pTexture->get_index() != nTex)
		return 0;

	return 1;
}

STDEF_FUNC(UIWindowRelease_nF)
{
	int nHash;
	if(!_pdsvBase->get("UIWindowRelease_nF", &nHash))
		return 0;

	BaseUIManager *pUIManager = (BaseUIManager *)BaseUIManager::get_manager();
	
    STLVInt stlVIds = BaseUIManager::ui_context_get(_pdsvContext);
	for(unsigned i=0; i<stlVIds.size(); i++)
    {
        BaseUIWindow *pWin;
        pWin = pUIManager->window_get(stlVIds[i]);
        if(pWin && STRTOHASH(pWin->get_name().c_str()) == nHash)
            pUIManager->window_release(stlVIds[i]);
    }
	return 1;
}

STDEF_FUNC(UIWindowCreate_nF)
{
	int nHash;
	if(!_pdsvBase->get("UIWindowCreate_nF", &nHash))
		return 0;

	const char *strNameTif;
	const char *strName;
    strName		= (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), nHash, STRTOHASH("strName"));
	strNameTif	= (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), nHash, STRTOHASH("UITextureFilename_strV"));
	if(strNameTif == NULL)
		return 0;

	const int *pnSize;
	short nCnt;
	pnSize	= (const int*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"), nHash, STRTOHASH("UIScreenSize_anV"), &nCnt);
	if(pnSize == NULL)
		return 0;

	//BaseUIWindow	*puiWindow = new BaseUIWindow(BaseUIManager::get_manager(), 0, 0, *(pnSize+0), *(pnSize+1));

    int nUIId, nStateId;
    nUIId = BaseUIManager::serial_new();
    
    BaseState *pState = (BaseState*)_pdsvBase->m_pVoidParam;
    
    nStateId = pState->identifier_get(HASH_STATE(StateUI));
    
    BaseUIManager *pUIManager = (BaseUIManager *)BaseUIManager::get_manager();
	BaseUIWindow	*puiWindow = pUIManager->window_create_root(strName, strNameTif, nUIId);

    BaseUIManager::ui_context_add(_pdsvContext, nUIId);
	if(nCnt >= 4)
		puiWindow->pos_set(*(pnSize+0), *(pnSize+1), *(pnSize+2), *(pnSize+3));	
	if(nCnt >= 6)
		puiWindow->pos_zorder_set(*(pnSize+5));
    
    pUIManager->identify_link_set(nUIId, nStateId);
	
	return 1;
}

STDEF_FUNC(UIWindowMoveMouse_nF)
{
	short nCnt;
	const int *pnEValue;

	const int *pnHash;
	STDEF_GET_R(_pdsvBase, UIWindowMoveMouse_nF, pnHash);
	STDEF_AGET_R(_pdsvEvent, MouseEvent_Values_anV, pnEValue, nCnt);

	if(nCnt < 4)
		return 0;

	if(*pnHash == STRTOHASH("default"))
	{
		BaseUIManager::get_manager()->move_pos(*(pnEValue+2), *(pnEValue+3));
		return 1;
	}

	int nIdentifier;
	BaseState *pState = (BaseState*)_pdsvBase->m_pVoidParam;
	nIdentifier	= pState->identifier_get(HASH_STATE(StateUI));

	BaseUIWindow *pWin;
	pWin = BaseUIManager::ui_context_parent(pState->get_state_main(), _pdsvContext, *pnHash);

	pWin->pos_move(*(pnEValue+2), *(pnEValue+3));
	return 1;
}

int BaseUIManager::texture_load(BaseUIWindow *_pWin, const char *_strFile)
{
	int nTex = -1;
	int nHash = STRTOHASH(_strFile);

	nTex = _pWin->texture_find(nHash);

	if(nTex == -1)
	{
		ResTif	*pTif = (ResTif*)BaseResManager::get_manager()->get_resource(_strFile, ResTif::ECARE_RES_TEXTURE, NULL, NULL, 0);
		if(pTif == NULL)
		{
			char strBuf[255];
			sprintf_s(strBuf, 255, "Fail to load texture %s", _strFile);
			g_SendMessage(LOG_MSG_POPUP, strBuf);
			return -1;
		}
		nTex	=  _pWin->texture_add(pTif->get_width(), pTif->get_height(), (char*)pTif->get_data(), nHash);
	}
		
	return nTex;
}

STDEF_FUNC(UIBoardListCreate_nF)
{
	const int *pnHash;
	const int *pnRowGap;
	const int *pnColHash;
	const int *pnColType;
	const int *pnColGap;
	int nRowGap = 0;

	STDEF_GET_R(_pdsvBase, UIBoardListCreate_nF, pnHash);
	if(STDEF_GET(_pdsvBase, UIBoardListCreateRowGap_nV, pnRowGap))
		nRowGap = *pnRowGap;

	//int nLocalCnt	= 0;
	//_pdsvBase->get()
	while(STDEF_GET(_pdsvBase, UIBoardListCreateCol_nV, pnColHash))
	{
		STDEF_GET(_pdsvBase, UIBoardListCreateColType_nV, pnColType);
		STDEF_GET(_pdsvBase, UIBoardListCreateColGap_nV, pnColGap);
	}
	return 1;
}

STDEF_FUNC(UIListMake_nF)
{
	const int *pnHashBase, *pnCount, *pnHashUI, *pnHashUILine;

	STDEF_GET_R(_pdsvBase, UIListMake_nF, pnHashBase);	// Base window, use to bound box and start position
	STDEF_GET_R(_pdsvBase, UIListMakeCount_nV, pnCount); // Number of List 
	STDEF_GET_R(_pdsvBase, UIListMakeLineUI_nV, pnHashUILine); // Line window just define size

	int nHash, nHashColum, nHashColumGap;
	nHashColum		= STRTOHASH("UIListMakeItem_nV"); // Element Window format for a colum
	nHashColumGap	= STRTOHASH("UIListMakeItemGap_aV"); // Gap of in between colum and next colum
	int nSequence	= _pdsvBase->sequence_get(); // position of Base window colum in Dynamic data structure
	nSequence	+= 2; // Skip UIListMakeCount_nV, UIListMakeLineUI_nV
	short	shCnt = 0;
	const char *strName;

	strName = (const char*)BaseStateManager::get_manager()->EnumGetValue(STRTOHASH("EnumUI"),*pnHashBase, STRTOHASH("strName"));

    int nIdentifier;
	BaseState *pState = (BaseState*)_pdsvBase->m_pVoidParam;

	nIdentifier	= pState->identifier_get(HASH_STATE(StateUI));
	BaseUIWindow *pWin;

	BaseUIManager *pUIManager = (BaseUIManager *)BaseUIManager::get_manager();
    pWin = BaseUIManager::ui_context_parent(pState->get_state_main(), _pdsvContext, *pnHashBase);
	
	char strBuf[255];
	if(pWin == NULL)
	{
		sprintf_s(strBuf, 255, "Error to get UI %s", strName);
		g_SendMessage(LOG_MSG_POPUP, strBuf);
	}

	STLVInt	stlVnEnumUI;
	STLVInt stlVnEnumUIGap;
	int	nGapX	= 0;
	int nGapY	= 0;
	do{
		nSequence++;
		nHash = _pdsvBase->get_colum(nSequence); // get UIListMakeItem_nV to get colum, it support multi UIListMakeItem_nV
		if(nHash == 0)
			break;

		if(nHash == nHashColum) // Always colum can get another type, so we should check it by the hash value
		{
			if(_pdsvBase->get((const void **)&pnHashUI, &shCnt, nSequence))
			{	
				stlVnEnumUI.push_back(*pnHashUI); // record colums
				stlVnEnumUIGap.push_back(nGapX);
				nGapX= 0;
			}
		}else if(nHash == nHashColumGap)
		{
			const int *pnGap;
			if(_pdsvBase->get((const void **)&pnGap, &shCnt, nSequence))
			{	
				nGapX	= *pnGap;
				if(shCnt > 1)
					nGapY	= *(pnGap+1);
			}
		}
	}while(nHash != 0);

	STLVStlVpVoid *pstlVstlVpWin;

	pstlVstlVpWin	= pUIManager->list_get(*pnHashBase);

	char strLine[255];

	STLVInt	stlVCoordinate;
	stlVCoordinate.resize(2);
	BaseUIWindow	*pLinePre = 0;
	// All of child window has WIN_TYPE_Link
	for(int y=0; y<*pnCount; y++)
	{
		BaseUIWindow	*pLine;
		STLVpVoid	stlVpWin;		

		sprintf_s(strLine, 255, "%sLine%03d", strName, y);
		// create window
		// first window has pos_type: POS_TYPE_START
		// This window is line window to include colums, so it have no image(texture), it has just dynamic bound box defend on colums.
		// Fist : POS_TYPE_Start, POS_TYPE_Start
		// next : POS_TYPE_Start, POS_TYPE_Next
		if(y==0)
		{
			pLine = pUIManager->window_create(nIdentifier, strLine, *pnHashUILine, BaseUIWindow::WIN_TYPE_Linker, BaseUIWindow::POS_Start, BaseUIWindow::POS_Start, pWin, pWin);
			pLine->pos_set(BaseUIWindow::POS_W, BaseUIWindow::POS_Static, NULL);
			pLine->pos_set(BaseUIWindow::POS_H, BaseUIWindow::POS_Dynamic, NULL);
			pWin->attach_child(pLine);
		}else{
			pLine = pUIManager->window_create(nIdentifier, strLine, *pnHashUILine, BaseUIWindow::WIN_TYPE_Linker, BaseUIWindow::POS_Start, BaseUIWindow::POS_Next, pLinePre, pWin);
			pLine->pos_set(BaseUIWindow::POS_W, BaseUIWindow::POS_Static, NULL);
			pLine->pos_set(BaseUIWindow::POS_H, BaseUIWindow::POS_Dynamic, NULL);
			pLine->pos_offset_set(BaseUIWindow::POS_Y, nGapY);
			pWin->attach_child(pLine);
		}

		BaseUIWindow *pWinColumPre = NULL;
		for(int x=0; x<(int)stlVnEnumUI.size(); x++)
		{
			stlVCoordinate[0] = x;
			stlVCoordinate[1] = y;

			char strNameColum[255];
			BaseUIWindow *pWinColum = NULL;
			sprintf_s(strNameColum, 255, "%sColum%02d%02d", strName, x, y);
			// Firat: POS_TYPE_Start, POS_TYPE_CENTER
			// Next: POS_TYPE_Next, POS_TYPE_CENTER
			if(x==0)
			{
				pWinColum	= pUIManager->window_create(nIdentifier, strNameColum, stlVnEnumUI[x], BaseUIWindow::WIN_TYPE_Linker, BaseUIWindow::POS_Start, BaseUIWindow::POS_Center, pLine, pWin);
				pLine->attach_child(pWinColum);
			}else{
				pWinColum	= pUIManager->window_create(nIdentifier, strNameColum, stlVnEnumUI[x], BaseUIWindow::WIN_TYPE_Linker, BaseUIWindow::POS_Next, BaseUIWindow::POS_Center, pWinColumPre, pWin);
				pWinColum->pos_offset_set(BaseUIWindow::POS_X, stlVnEnumUIGap[x]);
				pWinColumPre->attach_child(pWinColum);
			}

			pWinColum->pos_cood_set(stlVCoordinate);
			stlVpWin.push_back(pWinColum);
			pWinColumPre = pWinColum;
		}
		
		pLine->event_cast_disable();
		pLine->pos_update();
		pLinePre	= pLine;
		pstlVstlVpWin->push_back(stlVpWin);
	}
	return 1;
}

STDEF_FUNC(BaseMouse_anIF)
{
    const int *anRequest, *anEvent;
    short   nCntReq, nCntEvt;
    STDEF_AGET_R(_pdsvBase, BaseMouse_anIF, anRequest, nCntReq);
    STDEF_AGET_R(_pdsvEvent, BaseMouse_anIF, anEvent, nCntEvt);
    
    for(int i=0; i<nCntReq; i++)
    {
        bool bFind = false;
        for(int j=0; j<nCntEvt; j++)
        {
            if(anRequest[i] == anEvent[j])// Push, Over, Up, Out, Button_1, Button_2, Button_3, Button_4
            {
                bFind = true;
                break;
            }
        }
        if(!bFind)
            return 0;
    }
    return 1;
}
