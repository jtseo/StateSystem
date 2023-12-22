#include "stdafx.h"
#include "BaseState.h"
#include "BaseNetConnector.h"
#include "BaseStateSpace.h"
#include "BaseStateMain.h"
#include "BaseStateManager.h"
#include "BaseEventManager.h"

#define VER_ADDSERIAL
#ifdef VER_ADDSERIAL
//#define TRACE_STATE
//#define STATELOG
#endif


BaseStateSpace	BaseStateSpace::sm_sample(NULL, NULL, 0);

BaseStateSpace::BaseStateSpace(BaseStateManager *_pManager, const char *_strStateLogic, int _nSpaceID)
{
	init(_pManager, _strStateLogic, _nSpaceID);
}

BaseStateSpace::~BaseStateSpace(void)
{
	release();
}

void BaseStateSpace::init(BaseStateManager *_pManager, const char *_strStateLogic, int _nSpaceID)
{
	m_pManager	= _pManager;
	m_nSpaceID	= _nSpaceID;
	m_threadSpace = false;

	if(_strStateLogic)
	{
		BaseStateMain	*pMain;
		pMain	= new BaseStateMain(this, _strStateLogic);
		if (pMain->get_key() == 0)
		{
			delete pMain;
		}
		else {
			pMain->inc_ref(this);
			m_stlVpStateMain.push_back(pMain);
		}
	}

	m_pstlvActorCurrent	= NULL;
	m_nEventActorCount	= 0;
}

void BaseStateSpace::release()
{
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
	{
		if(m_stlVpStateMain[i])
		{
			BaseStateMain* main = m_stlVpStateMain[i];
			m_stlVpStateMain.erase(m_stlVpStateMain.begin() + i);
			i--;
			main->dec_ref(this);	
		}
	}
	m_stlVpStateMain.clear();

	for(unsigned i=0; i<m_stlVpSpaceNear.size(); i++)
		m_stlVpSpaceNear[i]->dec_ref(this);	
	m_stlVpSpaceNear.clear();
}

bool BaseStateSpace::add_main(const char *_strLogin)
{
	BaseStateMain	*pMain;
	pMain	= new BaseStateMain(this, _strLogin);
	pMain->inc_ref(this);
	m_stlVpStateMain.push_back(pMain);

	return true;
}

void BaseStateSpace::copy_space(BaseStateSpace *_pSpace)
{
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
	{
		_pSpace->add_main(m_stlVpStateMain[i]->get_state_filename().c_str());
	}
}

bool compare_actor(STStateActor& a, STStateActor& b)
{	
	return (a.nPriority > b.nPriority);
}

bool BaseStateSpace::actor_event_regist(STStateActor _stStateActor)
{
	int i;
    
    if(_stStateActor.pState->get_space() != this)
    {
        int ndebug = 0;
        ndebug++;
    }
//    g_SendMessage(LOG_MSG, "Debug record %s %x, %x, %s, %s", _stStateActor.pState->get_name().c_str(), _stStateActor.pState, _stStateActor.pState->get_space(), _stStateActor.pdstActor->get_name_debug(),
//                  _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex));
    
//#ifdef TRACE_STATE
//	g_SendMessage(LOG_MSG, "actor_event_regist: SID:%d, Main:%s, Link:%s, Colum: %s, id:%d, Index:%d\n", _stStateActor.nSpaceID
//		, _stStateActor.pState->get_state_main()->get_name().c_str()
//		, _stStateActor.pdstActor->get_name_debug()
//		, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
//		, _stStateActor.pState->identifier_get()
//		, _stStateActor.nBoolIndex);
//#endif
	
	switch(_stStateActor.nType)
	{
	case STStateActor::TIMEOUT:
		i=(int)m_stlVStateActorTimer.size() - 1;
		for(;i>=0; i--)
		{
			if(m_stlVStateActorTimer[i].nValue < _stStateActor.nValue)
			{
				m_stlVStateActorTimer.insert(m_stlVStateActorTimer.begin()+i+1, _stStateActor);
				return true;
			}
		}
		m_stlVStateActorTimer.insert(m_stlVStateActorTimer.begin(), _stStateActor);
		break;
	case STStateActor::UPDATE:
		m_stlVStateActorUpdator.push_back(_stStateActor);
		break;
	case STStateActor::HASH:
		{
			STLMnpstlVSTStateActor::iterator it;
			it	= m_stlMnpstlVStateActor.find(_stStateActor.nKey);
			if(m_stlMnpstlVStateActor.end() != it)
			{	
				STLVSTStateActor& actor_a = *it->second;
			
				if (actor_a.back().nPriority >= _stStateActor.nPriority)
				{
					actor_a.push_back(_stStateActor);
				}
				else {
					int j = (int)actor_a.size()-2;
					bool inserted = false;
					while(j>=0) {
						if (actor_a[j].nPriority >= _stStateActor.nPriority)
						{
							inserted = true;
							actor_a.insert(actor_a.begin() + j + 1, _stStateActor);
							break;
						}
						j--;
					}

					if (!inserted)
						actor_a.insert(actor_a.begin(), _stStateActor);
				}
			}else{
				//==================================================================================================
				// Start Add or Update by OJ 2012-07-26 ø¿¿¸ 10:39:05
				// STLVSTStateActor∏¶ memory pool∑Œ «“¥Á
				// new STLVSTStateActor;
				static STLVSTStateActor sample;

				STLVSTStateActor *pstlVstActor;
				pstlVstActor	= (STLVSTStateActor*)mpool_get().malloc(sizeof(STLVSTStateActor), __FILE__, __LINE__);
				::new(pstlVstActor) STLVSTStateActor(sample);
				// End by OJ 2012-07-26 ø¿¿¸ 10:39:09
				//==================================================================================================

				m_stlMnpstlVStateActor[_stStateActor.nKey]	= pstlVstActor;
				m_stlMnpstlVStateActor[_stStateActor.nKey]->push_back(_stStateActor);
			}
		}
		break;
	}

	return true;
}

bool BaseStateSpace::actor_event_release(BaseDStructureValue *_pdsvLink)
{
	unsigned i;

	for(i=0;i<m_stlVStateActorTimer.size(); i++)
	{
		if(m_stlVStateActorTimer[i].pdstActor == _pdsvLink)
			m_stlVStateActorTimer.erase(m_stlVStateActorTimer.begin()+i);
	}

	return false;
}

bool BaseStateSpace::actor_event_update(STStateActor _stStateActor)
{
	unsigned i;
//#ifdef TRACE_STATE
//	g_SendMessage(LOG_MSG, "actor_event_release: SID:%d, Main:%s, Link:%s, Colum:%s, Id:%d, Index:%d\n", _stStateActor.nSpaceID
//		, _stStateActor.pState->get_state_main()->get_name().c_str()
//		, _stStateActor.pdstActor->get_name_debug()
//		, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
//		, _stStateActor.pState->identifier_get()
//		, _stStateActor.nBoolIndex);
//#endif

	switch(_stStateActor.nType)
	{
	case STStateActor::TIMEOUT:
		break;
	case STStateActor::UPDATE:
		break;
	case STStateActor::HASH:
		{
			STLMnpstlVSTStateActor::iterator it;
			it	= m_stlMnpstlVStateActor.find(_stStateActor.nKey);
			if(m_stlMnpstlVStateActor.end() != it)
			{
				STLVSTStateActor	*pstlVstStateActor;
				pstlVstStateActor	= it->second;

				bool find = false;
				int max = (int)pstlVstStateActor->size() + 1;
				for(i=0; i<pstlVstStateActor->size(); i++)
				{
					(*pstlVstStateActor)[i].nTempOrder = max - i;
					if(pstlVstStateActor->at(i).pState	== _stStateActor.pState
						&& pstlVstStateActor->at(i).nLinkIndex	== _stStateActor.nLinkIndex)
					{
						(*pstlVstStateActor)[i].nPriority = _stStateActor.nPriority;
						find = true;
					}
				}
				
				if(find){
					struct {
						bool operator()(const STStateActor &a, const STStateActor &b) const
						{
							if(a.nPriority > b.nPriority)
								return true;
							if(a.nPriority == b.nPriority)
								return a.nTempOrder > b.nTempOrder;
							return false;
						}
					} compare;
					std::sort(pstlVstStateActor->begin(), pstlVstStateActor->end(), compare);
					
					return true;
				}
			}
		}
		break;
	}

#ifdef TRACE_STATE
	if(_stStateActor.pState && _stStateActor.pdstActor)
		g_SendMessage(LOG_MSG, "fail to actor_event_update: SID:%d, Main:%s, Link:%s, Index:%d\n", _stStateActor.nSpaceID
			, _stStateActor.pState->get_state_main()->get_name().c_str()
			, _stStateActor.pdstActor->get_name_debug(), _stStateActor.nBoolIndex);
#endif
	return false;
}

bool BaseStateSpace::actor_event_release(STStateActor _stStateActor)
{
	unsigned i;
//#ifdef TRACE_STATE
//	g_SendMessage(LOG_MSG, "actor_event_release: SID:%d, Main:%s, Link:%s, Colum:%s, Id:%d, Index:%d\n", _stStateActor.nSpaceID
//		, _stStateActor.pState->get_state_main()->get_name().c_str()
//		, _stStateActor.pdstActor->get_name_debug()
//		, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
//		, _stStateActor.pState->identifier_get()
//		, _stStateActor.nBoolIndex);
//#endif

	switch(_stStateActor.nType)
	{
	case STStateActor::TIMEOUT:
		for(i=0;i<m_stlVStateActorTimer.size(); i++)
		{
			if(m_stlVStateActorTimer[i].pdstActor == _stStateActor.pdstActor
				&& m_stlVStateActorTimer[i].nBoolIndex == _stStateActor.nBoolIndex)
			{
				m_stlVStateActorTimer.erase(m_stlVStateActorTimer.begin()+i);
				return true;
			}
		}
		break;
	case STStateActor::UPDATE:
		for(i=0;i<m_stlVStateActorUpdator.size(); i++)
		{
			if(m_stlVStateActorUpdator[i].pdstActor == _stStateActor.pdstActor
				&& m_stlVStateActorUpdator[i].nBoolIndex == _stStateActor.nBoolIndex)
			{
				m_stlVStateActorUpdator.erase(m_stlVStateActorUpdator.begin()+i);
				return true;
			}
		}
		break;
	case STStateActor::HASH:
		{
			STLMnpstlVSTStateActor::iterator it;
			it	= m_stlMnpstlVStateActor.find(_stStateActor.nKey);
			if(m_stlMnpstlVStateActor.end() != it)
			{
				STLVSTStateActor	*pstlVstStateActor;
				pstlVstStateActor	= it->second;

				for(i=0; i<pstlVstStateActor->size(); i++)
				{
					if(pstlVstStateActor->at(i).pdstActor	== _stStateActor.pdstActor
						&& pstlVstStateActor->at(i).nBoolIndex	== _stStateActor.nBoolIndex)
					{
						if(m_pstlvActorCurrent == pstlVstStateActor && (int)i <= m_nEventActorCount)
								m_nEventActorCount--;// «ˆ¿Á ∞ÀªÁ ¡ﬂ ¿œ ºˆ ¿÷±‚ ∂ßπÆø° « ø‰«— ∏∏≈≠ ∞®º“ Ω√≈≤¥Ÿ.
							
						pstlVstStateActor->erase(pstlVstStateActor->begin()+i);
						if(pstlVstStateActor->size() == 0)
						{
							if(m_pstlvActorCurrent == pstlVstStateActor)
								m_pstlvActorCurrent = NULL;

							//==================================================================================================
							// Start Add or Update by OJ 2012-07-26 ø¿¿¸ 10:39:44
							// pstlVstStateActor∏¶ Memory Poolø° µ«µπ∏≤
							// delete pstlVstStateActor;
							PT_Destroy(pstlVstStateActor);
							mpool_get().free(pstlVstStateActor);

							m_stlMnpstlVStateActor.erase(it);
							// End by OJ 2012-07-26 ø¿¿¸ 10:39:47
							//==================================================================================================
						}
						return true;
					}
				}
			}
		}
		break;
	}

#ifdef TRACE_STATE
	if(_stStateActor.pState && _stStateActor.pdstActor)
		g_SendMessage(LOG_MSG, "fail to actor_event_release: SID:%d, Main:%s, Link:%s, Index:%d\n", _stStateActor.nSpaceID
			, _stStateActor.pState->get_state_main()->get_name().c_str()
			, _stStateActor.pdstActor->get_name_debug(), _stStateActor.nBoolIndex);
#endif
	return false;
}

void BaseStateSpace::post_systemevent(BaseDStructureValue *_pdstEvent, int _nPriority)
{
	m_pManager->post_systemevent(_pdstEvent, m_nSpaceID, _nPriority);
}

void BaseStateSpace::post_event(BaseDStructureValue *_pdstEvent, int _nPriority)
{
	m_pManager->post_event(_pdstEvent, m_nSpaceID, _nPriority);
}

void BaseStateSpace::thread_create_event(BaseDStructureValue *_pdstEvent, int _nPriority)
{
	m_pManager->thread_create_event(_pdstEvent, _nPriority);
}

void BaseStateSpace::state_active_show_all(void)
{
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
	{
		m_stlVpStateMain[i]->state_active_show_all();
	}
}

int BaseStateSpace::OnEvent_(BaseDStructureValue *_pEvent, const BaseState *_pStateTarget)
{
#ifdef _DEBUG
	/*if(NULL != _pEvent)
	{
		const int* nValue = NULL;

		if(_pEvent->get(_pEvent->get_key(),(const void**)&nValue))
			g_SendMessage(LOG_MSG_FILELOG,"_pEvent : base_key : %d , key : %d value : %d \n",_pEvent->get_base_key() ,
																   _pEvent->get_key(),
																   *nValue);
	}*/

#endif

	int nKey;
	nKey	= _pEvent->get_key();

	if(nKey == HASH_STATE(SPACE_TRANSLATE))
	{
		return space_translate(_pEvent);
	}

	int nRet = BaseState::RET_UNPROCESS;
	//==================================================================================================
	// Start Add or Update by OJ 2012-07-27 ø¿»ƒ 12:00:00
	if(nKey == HASH_STATE(STATE_CHANGE)
		|| nKey == HASH_STATE(STATE_ADD)
		|| nKey == HASH_STATE(STATE_RELEASE)
		|| nKey == HASH_STATE(STATE_CALL)
		|| nKey == HASH_STATE(STATE_RETURN))
	{
		for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
		{
			if(m_stlVpStateMain[i]->OnEvent(_pEvent) == BaseState::RET_PROCESS)
				nRet	= BaseState::RET_PROCESS;
		}
		return nRet;
	}

	BaseEventManager::break_befor_propogate(_pEvent);

	STLMnpstlVSTStateActor::iterator	it;
	it	= m_stlMnpstlVStateActor.find(nKey);

	if(it != m_stlMnpstlVStateActor.end())
	{
		bool bIntCheck	= false;
		int nValue	= 0;
		if(!m_pManager->actor_get_keyvalue(_pEvent, &nValue))
		{
			const int *pnInt;
			int nIndex = _pEvent->get_index(nKey);
			if(nIndex >= 0)
			{
				int nSize = _pEvent->get_base()->get_type_size(nIndex);
				if(nSize == sizeof(int) && _pEvent->get(nKey, (const void**)&pnInt))
				{
					nValue		= *pnInt;
					bIntCheck	= true;
				}
			}
		}

		m_pstlvActorCurrent	= it->second;

		STLVpState	stlVpStateChecked;

        STLMnInt stlMnGroupId;
        
        BaseState::group_id_get(_pEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
		//const int *pnTargetIdentifier;
		//int nTargetIdentifier = 0;
		//if(_pEvent->get(HASH_STATE(HASH_BaseTransitionGoalIdentifier), (const void**)&pnTargetIdentifier))
		//	nTargetIdentifier	= *pnTargetIdentifier;

		for(m_nEventActorCount=0;m_pstlvActorCurrent && m_nEventActorCount<(int)m_pstlvActorCurrent->size(); m_nEventActorCount++)
		{
			
			if(bIntCheck && nValue != (*m_pstlvActorCurrent)[m_nEventActorCount].nValue)// in case of BaseStateEventGlobal_nRE, nValue is the Event looks like 'UxPush'
				continue;

			if(_pStateTarget!=NULL && _pStateTarget != m_pstlvActorCurrent->at(m_nEventActorCount).pState)
				continue;	// In case of UI the target point has to have same point.

			if(stlMnGroupId.size() > 0 &&
				!(*m_pstlvActorCurrent)[m_nEventActorCount].pState->group_id_is_in(stlMnGroupId))
				continue;

			stlVpStateChecked.push_back((*m_pstlvActorCurrent)[m_nEventActorCount].pState);
            
			int nRetLocal;
			nRetLocal	= actor_event_process((*m_pstlvActorCurrent)[m_nEventActorCount], _pEvent, BaseDStructure::STATE_EVENT);
			if (nRetLocal == BaseState::RET_PROCESS)
			{
				nRet = nRetLocal;
			}

			if (m_pManager->event_skip_get()) { // event skip is applyed only other States, so other link what owned by this State can get this event.
				_pStateTarget = stlVpStateChecked.back();
			}

			if (m_pstlvActorCurrent == NULL)// 
			{
				printf("debug point onevt start 5 %d\n", m_nEventActorCount);

				break;
			}
			// Link∞° ªË¡¶ µ… ∞ÊøÏ m_nEventActorCount∞° ¡ŸæÓ µÈ ºˆ ¿÷¥Ÿ, also m_pstlvActorCurrent can erased in release actor.
		}

		for(unsigned i=0; i<stlVpStateChecked.size(); i++)
			stlVpStateChecked[i]->OnEndOfOneEvent();

		m_pstlvActorCurrent	= NULL;
		m_nEventActorCount	= 0;
	}
	return nRet;
}

int BaseStateSpace::OnEvent(BaseDStructureValue *_pEvent, const BaseState *_pStateTarget)
{
#ifdef _DEBUG
	/*if(_pEvent != NULL)
	{
		const int* nValue;
		if(_pEvent->get(_pEvent->get_key(),(const void**)&nValue))
		g_SendMessage(LOG_MSG_FILELOG,"_pEvent : base_key : %d , key : %d , value : %d \n",_pEvent->get_base_key() ,
															   _pEvent->get_key(),*nValue);
	}*/
	
#endif

	int nKey;
	int nRet	= BaseState::RET_UNPROCESS;

	nKey	= _pEvent->get_key();

	bool bLeafEvent	= false;
	bool bNearEvent	= false;
	const int *pnTargetSpace;
	if(_pEvent->get(HASH_STATE(HASH_BaseTransitionGoalSpaceID), (const void**)&pnTargetSpace))
	{
		if(*pnTargetSpace != m_nSpaceID && *pnTargetSpace != 0)
		{
			int nMaskedID	= *pnTargetSpace & space_mask_get();

			if(nMaskedID != m_nSpaceID) // ∞∞¿ª ∞ÊøÏ Child Space∏¶ ¿ß«—Event¿Ã¥Ÿ.
			{	
				for(unsigned i=0; i<m_stlVpSpaceNear.size(); i++)
				{
					nMaskedID	= *pnTargetSpace & m_stlVpSpaceNear[i]->space_mask_get();
					if(nMaskedID == m_stlVpSpaceNear[i]->space_id_get())
						bNearEvent	= true;
				}
				if(!bNearEvent)
					return BaseState::RET_UNPROCESS;
			}
		}else{
			bLeafEvent	= true;
		}
	}

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		int nRetChild	= BaseState::RET_UNPROCESS;
		BaseStateSpace	*pSpace	= (BaseStateSpace*)m_child_stlVp[i];
		nRetChild	= pSpace->OnEvent_(_pEvent, _pStateTarget);
		if(nRetChild == BaseState::RET_PROCESS)
			nRet = nRetChild;
	}

	//if(bNearEvent || bLeafEvent)
	if(nRet == BaseState::RET_UNPROCESS)
		nRet	= OnEvent_(_pEvent, _pStateTarget);
	
	return nRet;
}

int BaseStateSpace::actor_event_process(STStateActor &_stStateActor, BaseDStructureValue *_pEvent, int _nEventType)
{
	BaseDStructureValue	*pLink	= _stStateActor.pdstActor;
	BaseState	*pState		= _stStateActor.pState;
    

//#ifdef _COLOSSEUM
//	
//	g_SendMessage(LOG_MSG_FILELOG,"STStateActor nkey : %d \n",_stStateActor.nKey);
//
//	if(NULL != _pEvent)
//	{
//		const int* nValue = NULL;
//
//		if(_pEvent->get(_pEvent->get_key(),(const void**)&nValue))
//		g_SendMessage(LOG_MSG_FILELOG,"_pEvent : base_key : %d , key : %d value : %d \n",_pEvent->get_base_key() ,
//															   _pEvent->get_key(),
//															   *nValue);
//	}
//
//	/*if(NULL != pState)
//	{
//		g_SendMessage(LOG_MSG_FILELOG,"pState name : %s , identifier : %d \n",pState->get_name(),
//																pState->identifier_get());
//	}*/
//
//#endif


	if(_stStateActor.nKey == HASH_STATE(HASH_BaseLinkDelay_nF))
	{
#ifdef TRACE_STATE
		//if(_pEvent)
		//	g_SendMessage(LOG_MSG, "actor_event_process: SID:%d, Main:%s, Link:%s, Colum:%s, Index:%d, Event:%s\n", _stStateActor.nSpaceID
		//	, _stStateActor.pState->get_state_main()->get_name().c_str()
		//	, _stStateActor.pdstActor->get_name_debug()
		//	, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
		//	, _stStateActor.nBoolIndex
		//	, _pEvent->get_name_debug());
		//else
		//	g_SendMessage(LOG_MSG, "actor_event_process: SID:%d, Main:%s, Link:%s, Colum:%s, Index:%d, EventType:%d\n", _stStateActor.nSpaceID
		//	, _stStateActor.pState->get_state_main()->get_name().c_str()
		//	, _stStateActor.pdstActor->get_name_debug()
		//	, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
		//	, _stStateActor.nBoolIndex, _nEventType);
#endif

		//return 
		pState->LinkProcess_(pLink, _pEvent);
		return BaseState::RET_PROCESS;
	}

#ifdef _DEBUG
	int link = pLink->get_key();
	if(link ==-1453171329)
	{
		g_SendMessage(LOG_MSG, "------ link for the timeout");
	}
#endif
	if(!pState->is_skip())
	{	
#ifdef _DEBUG
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeout2");
#endif
		if(pLink->event_process(_stStateActor.nBoolIndex, _pEvent, pState->context_get(), _nEventType, NULL))
		{
#ifdef _DEBUG
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeout3");
#endif
			if(pState->get_state_value() != pLink // In case same, it's not a Link but a State;
				&& pLink->is_complete(_pEvent, pState->context_get()))
			{
#ifdef _DEBUG
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeout4");
#endif
				event_affected();// Add by OJ : 2010-10-07 ¿Ã ¿Ã∫•∆Æ∑Œ ¿Œ«ÿ StateΩ√Ω∫≈€ø° ∫Ø»≠∞° ¿÷¥Ÿ
				const int *pnTimeDelay;
				if(pLink->get(HASH_STATE(HASH_BaseLinkDelay_nF), (const void**)&pnTimeDelay))
				{
					STStateActor actor;
					actor			= _stStateActor;
					actor.nType		= STStateActor::TIMEOUT;
					actor.nKey		= HASH_STATE(HASH_BaseLinkDelay_nF);
					actor.nValue	= (UINT32)*pnTimeDelay + BaseSystem::timeGetTime();
					actor_event_regist(actor);
#ifdef TRACE_STATE
					//if(_pEvent)
					//	g_SendMessage(LOG_MSG, "actor_event_process: SID:%d, Main:%s, Link:%s, Colum:%s, id:%d, Index:%d, Event:%s\n", _stStateActor.nSpaceID
					//	, _stStateActor.pState->get_state_main()->get_name().c_str()
					//	, _stStateActor.pdstActor->get_name_debug()
					//	, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
					//	, _stStateActor.pState->identifier_get()
					//	, _stStateActor.nBoolIndex
					//	, _pEvent->get_name_debug());
					//else
					//	g_SendMessage(LOG_MSG, "actor_event_process: SID:%d, Main:%s, Link:%s, Colum:%s, id:%d, Index:%d, EventType:%d\n", _stStateActor.nSpaceID
					//	, _stStateActor.pState->get_state_main()->get_name().c_str()
					//	, _stStateActor.pdstActor->get_name_debug()
					//	, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
					//	, _stStateActor.pState->identifier_get()
					//	, _stStateActor.nBoolIndex, _nEventType);
#endif

				}else{
#ifdef TRACE_STATE
					//if(_pEvent)
					//	g_SendMessage(LOG_MSG, "actor_event_process: SID:%d, Main:%s, Link:%s, Colum:%s, id:%d, Index:%d, Event:%s\n", _stStateActor.nSpaceID
					//	, _stStateActor.pState->get_state_main()->get_name().c_str()
					//	, _stStateActor.pdstActor->get_name_debug()
					//	, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
					//	, _stStateActor.pState->identifier_get()
					//	, _stStateActor.nBoolIndex
					//	, _pEvent->get_name_debug());
					//else
					//	g_SendMessage(LOG_MSG, "actor_event_process: SID:%d, Main:%s, Link:%s, Colum:%s, id:%d, Index:%d, EventType:%d\n", _stStateActor.nSpaceID
					//	, _stStateActor.pState->get_state_main()->get_name().c_str()
					//	, _stStateActor.pdstActor->get_name_debug()
					//	, _stStateActor.pdstActor->get_name_bool_debug(_stStateActor.nBoolIndex)
					//	, _stStateActor.pState->identifier_get()
					//	, _stStateActor.nBoolIndex, _nEventType);
#endif

#ifdef _DEBUG
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeout5");
#endif
					//return
					bool linkcomplete = false;
					if(pState->LinkProcess_(pLink, _pEvent, &linkcomplete) == 0
						&& _stStateActor.nType == STStateActor::TIMEOUT)
					{
#ifdef _DEBUG
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeout6");
#endif
						return 0;
					}
#ifdef _DEBUG
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeout7");
#endif
					if (!linkcomplete)
						return 0;
					return BaseState::RET_PROCESS;
				}
			}
		}
	}


	if(_stStateActor.nType == STStateActor::TIMEOUT)
	{
		return 0;
	}
	return BaseState::RET_UNPROCESS;
}

bool BaseStateSpace::is_active_id(const STLMnInt *_pstlMnGroupId, int _nHash)
{
	bool bRet = false;
	for (unsigned i = 0; i<m_stlVpStateMain.size(); i++)
	{
		bRet = m_stlVpStateMain[i]->is_active_id(_pstlMnGroupId, _nHash);
		if (bRet)
			return bRet;
	}

	return	false;
}

BaseState* BaseStateSpace::GetStateName(int _nKeyName, const STLMnInt* _pstlMnGroupId, int _nSerial)
{
	BaseState* pRet;
	for (unsigned i = 0; i < m_stlVpStateMain.size(); i++)
	{
		pRet = m_stlVpStateMain[i]->GetStateName(_nKeyName, _pstlMnGroupId, _nSerial);
		if (pRet)
			return pRet;
	}

	return	NULL;
}

BaseState *BaseStateSpace::GetState(const BaseState *_pState)
{
	BaseState *pRet;
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
	{
		pRet = m_stlVpStateMain[i]->GetState(_pState);
		if(pRet)
			return pRet;
	}

	return	NULL;
}

BaseState *BaseStateSpace::GetState(int _nObjSerialState)
{
	BaseState *pRet;
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
	{
		pRet = m_stlVpStateMain[i]->GetStateSerial(_nObjSerialState);
		if(pRet)
			return pRet;
	}

	return	NULL;
}

BaseStateMain *BaseStateSpace::GetStateMain(int _nHash)
{
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
	{
		if(m_stlVpStateMain[i]->get_key() == _nHash)
			return m_stlVpStateMain[i];
	}

	return	NULL;
}

bool BaseStateSpace::GetActiveState(STLVpState* _stlVpState)
{
	for (unsigned i = 0; i < m_stlVpStateMain.size(); i++)
	{
		const STLVpState& stlVpState = m_stlVpStateMain[i]->get_active_state();

		for (unsigned j = 0; j < stlVpState.size(); j++)
			_stlVpState->push_back(stlVpState[j]);
	}// end of for m_stlVpStateMain
	return !_stlVpState->empty();
}

int BaseStateSpace::space_translate(const BaseDStructureValue *_pdstEvent)
{
	const int *pnTargetID;
	
	if(!_pdstEvent->get(HASH_STATE(SPACE_TRANSLATE), (const void**)&pnTargetID))
		return BaseState::RET_UNPROCESS;

	if(!m_pManager->is_conformer())
	{// Client
		m_nSpaceID	= *pnTargetID;// when apply in client, just change current id of space 
	}else{// Server
		BaseStateSpace	*pSpaceTarget;
		BaseStateMain	*pMainTarget;

        STLMnInt stlMnGroupId;
        BaseState::group_id_get(_pdstEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
		//const int *pnIdentity;
		//if(!_pdstEvent->get(HASH_STATE(HASH_BaseTransitionGoalIdentifier), (const void**)&pnIdentity))
		//	return BaseState::RET_UNPROCESS;

		STLVpBaseObject stlVpStateActive;
		pSpaceTarget	= m_pManager->GetSpace(*pnTargetID);

		// All of active state what are got same identity move to target space
		int nKeyMain;
		for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
		{
			nKeyMain	= m_stlVpStateMain[i]->get_key();

			const STLVpState &stlVpState = m_stlVpStateMain[i]->get_active_state();

			for(unsigned j=0; j<stlVpState.size(); j++)
			{
				if(stlVpState[j]->group_id_equal(stlMnGroupId))
				{	
					pMainTarget	= pSpaceTarget->GetStateMain(nKeyMain);
					if(pMainTarget)
					{
						BaseState *pState	= stlVpState[j];
						m_stlVpStateMain[i]->out_active(pState);
						pMainTarget->in_active(pState);

						j--;
					}
				}
			}
		}// end of for m_stlVpStateMain
	}// end of if(!m_pManager->is_conformer)

	return BaseState::RET_PROCESS;
}

void BaseStateSpace::OnUpdate(unsigned _nTimeDelta)
{
	//==================================================================================================
	// Start Add or Update by OJ 2012-07-30 ø¿»ƒ 4:45:34

	UINT32	nTimeCur;
	nTimeCur	= BaseSystem::timeGetTime();
	while(!m_stlVStateActorTimer.empty()
		&& (unsigned)m_stlVStateActorTimer[0].nValue <= nTimeCur)
	{
		//g_SendMessage(LOG_MSG, "SAction:TimeoutCast SActive:%d SLink:%d Time:%d", m_stlVStateActorTimer[0].pState->get_key(), m_stlVStateActorTimer[0].pdstActor->get_key(), m_stlVStateActorTimer[0].nValue);
		{
			char strBuffer[255];
					
			sprintf_s(strBuffer, 255, "TimeOut TimeCur:%d TimeRev:%d, %s, %s", nTimeCur, m_stlVStateActorTimer[0].nValue, m_stlVStateActorTimer[0].pState->get_name().c_str(), m_stlVStateActorTimer[0].pdstActor->get_name_debug());
			m_stlVStateActorTimer[0].pState->DebugOutString(NULL, m_stlVStateActorTimer[0].pdstActor, strBuffer);
		}
		if(actor_event_process(m_stlVStateActorTimer[0], NULL, BaseDStructure::STATE_UPDATE)
			== 0)
		{
			STStateActor	stStateActor;
			stStateActor	= m_stlVStateActorTimer[0];
			actor_event_release(stStateActor);
			if(stStateActor.nKey == HASH_STATE(HASH_BaseTimeOut_nREU)
				|| stStateActor.nKey == HASH_STATE(HASH_BaseTimeOut_varUIf))
			{
				const bbyte *pbOneTime = NULL;
				stStateActor.pdstActor->get(HASH_STATE(HASH_BaseLinkOneTime_b), (const void**)&pbOneTime);

				if (pbOneTime == NULL || *pbOneTime == 0){// add by oj 2016-12-09 
					const int *pnTimeout;
					stStateActor.pdstActor->get(stStateActor.nKey, (const void **)&pnTimeout);
					stStateActor.nValue = *pnTimeout + BaseSystem::timeGetTime();

					actor_event_regist(stStateActor);
				}
			}else
				actor_event_process(stStateActor, NULL, BaseDStructure::STATE_LINK_START);
		}
	}
	
	for(unsigned i=0; i<m_stlVStateActorUpdator.size(); i++)
	{
		actor_event_process(m_stlVStateActorUpdator[i], NULL, BaseDStructure::STATE_UPDATE);
	}
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
		m_stlVpStateMain[i]->OnUpdate(_nTimeDelta);


	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		BaseStateSpace	*pSpace	= (BaseStateSpace*)m_child_stlVp[i];
		pSpace->OnUpdate(_nTimeDelta);
	}
	// End by OJ 2012-07-30 ø¿»ƒ 4:45:35
	//==================================================================================================
}

int BaseStateSpace::OnDraw()
{
	int nRet = BaseState::RET_UNPROCESS;
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
		if(m_stlVpStateMain[i]->OnDraw() == BaseState::RET_PROCESS)
			nRet	= BaseState::RET_PROCESS;

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
		((BaseStateSpace*)m_child_stlVp[i])->OnDraw();

	return nRet;
}

void BaseStateSpace::reset_to_reload()
{
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
		m_stlVpStateMain[i]->reset_to_reload();

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
		((BaseStateSpace*)m_child_stlVp[i])->reset_to_reload();
}

void BaseStateSpace::reload()
{
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
		m_stlVpStateMain[i]->reload();

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
		((BaseStateSpace*)m_child_stlVp[i])->reload();
}

void BaseStateSpace::reload_start()
{
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
		m_stlVpStateMain[i]->reload_start();

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
		((BaseStateSpace*)m_child_stlVp[i])->reload_start();
}

bool compare_main(BaseStateMain *_pMain1, BaseStateMain *_pMain2)
{
	const char *_strName1, *_strName2;

	_strName1 = _pMain1->get_name().c_str();
	_strName2 = _pMain2->get_name().c_str();

	return strcmp(_strName1, _strName2) < 0;
}

int BaseStateSpace::StateStart()
{
	std::sort(m_stlVpStateMain.begin(), m_stlVpStateMain.end(), compare_main);

	int nRet = 0, nRetTemp;
	for(unsigned i=0; i<m_stlVpStateMain.size(); i++)
	{
		nRetTemp = m_stlVpStateMain[i]->StateStart();
		if(nRetTemp != 0)
			nRet = nRetTemp;
	}

	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		nRetTemp	= ((BaseStateSpace*)m_child_stlVp[i])->StateStart();
		if(nRetTemp != 0)
			nRet = nRetTemp;
	}

	return nRet;
}

BaseDStructure*	BaseStateSpace::EnumGet(int _nHash)
{
	return m_pManager->EnumGet(_nHash);
}

int	BaseStateSpace::EnumGetIndex(int _nHash, int _nKey)
{
	return m_pManager->EnumGetIndex(_nHash, _nKey);
}

int	BaseStateSpace::EnumGetIndex(const char *_strEnum, int _nKey)
{
	return m_pManager->EnumGetIndex(_strEnum, _nKey);
}

void BaseStateSpace::event_request_update()
{
	m_pManager->event_request_update();
}

void BaseStateSpace::event_affected()
{
	m_pManager->event_affected();
}

bool BaseStateSpace::key_is_pushed(int _nKey)
{
	return m_pManager->key_is_pushed(_nKey);
}

void BaseStateSpace::post_event_ui(int _nEvent)
{
	m_pManager->post_event_ui(_nEvent);
}

bool BaseStateSpace::is_conformer()
{
	return m_pManager->is_conformer();
}

bool BaseStateSpace::add_near(BaseStateSpace *_pSpace)
{
	STLVpStateSpace::iterator	it;
	it = std::find(m_stlVpSpaceNear.begin(), m_stlVpSpaceNear.end(), _pSpace);
	if(it == m_stlVpSpaceNear.end())
	{
		m_stlVpSpaceNear.push_back(_pSpace);
		_pSpace->inc_ref(this);

		_pSpace->add_near(this);
		return true;
	}
	return false;
}

void BaseStateSpace::space_mask_set(int _nMask)
{
	m_nSpaceIDMask	= _nMask;
}

int BaseStateSpace::space_mask_get()
{
	int nRetMask	= 0;
	if(get_parent())
	{
		nRetMask	= ((BaseStateSpace*)get_parent())->space_mask_get();
	}

	nRetMask |= m_nSpaceIDMask;
	return nRetMask;
}

BaseStateSpace *BaseStateSpace::space_get(int _nSpaceID)
{
	BaseStateSpace *pSpace = NULL;
	for(unsigned i=0; i<m_child_stlVp.size(); i++)
	{
		pSpace	= ((BaseStateSpace*)m_child_stlVp[i])->space_get(_nSpaceID);
		if(pSpace)
			return pSpace;
	}

	if(m_nSpaceID == _nSpaceID)
		return this;
	
	return pSpace;
}
