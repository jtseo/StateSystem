#include "stdafx.h"

#include "BaseEventManager.h"
#include "BaseDStructure.h"
#include "BaseState.h"
#include "BaseStateManager.h"

PtObjectCpp(BaseEventManager);

BaseEventManager::BaseEventManager()
{
	init();
}

void BaseEventManager::init()
{
	m_pstlCircleQueue	= new BaseCircleQueue("BaseEventManager::BaseEventManager()", 40960);
	m_nSequence = 0;
}

int BaseEventManager::dec_ref(BaseObject *_obj_p)
{
	// release the queue for the StateManager for _obj_p.

	return BaseObject::dec_ref(_obj_p);
}

BaseEventManager::~BaseEventManager(void)
{
	release();
}

void BaseEventManager::release()
{
	if(m_pstlCircleQueue)
		delete m_pstlCircleQueue;
	m_pstlCircleQueue	= NULL;
}

STLMnString BaseEventManager::m_stlMnStrBeforPropogate;
STLMnString BaseEventManager::m_stlMnStrCast;
STLMnString BaseEventManager::m_stlMnStrTrue;

void BaseEventManager::push_event_local(BaseDStructureValue *_pEvent, int _nPriority)
{
	push_event(_pEvent, _nPriority);
}

void BaseEventManager::push_event(BaseDStructureValue *_pEvent, int _nPriority)
{
	BaseEvent *pEvt;
	pEvt	= PT_MAlloc(BaseEvent);

	pEvt->m_nPriority	= _nPriority;
	pEvt->m_pdsvEvent	= _pEvent;

	_pEvent->ref_inc();
	if (_pEvent->ref_get() == 0)
	{
		int x = 0;
	}

	static int bInit = false;

	if(!bInit)
	{
		BaseDStructure *pEventState = BaseStateManager::get_manager()->EnumGet(HASH_STATE(EnumStateEvent));
		BaseDStructureValue *pdsvEvent;
		PT_OAlloc2(pdsvEvent, BaseDStructureValue, pEventState, 1024);
		
		STLMnstlRecord::iterator	it;
		int nKey;

		nKey	= pEventState->get_first_key(&it);
		while(nKey)
		{
			pdsvEvent->set_key(nKey);

			const char *strName;
			if(STDEF_GET(pdsvEvent, BaseBreakEventCast_strF, strName))
				m_stlMnStrCast[nKey] = strName;
			if(STDEF_GET(pdsvEvent, BaseBreakEventBeforPropogate_strF, strName))
				m_stlMnStrBeforPropogate[nKey] = strName;
			if(STDEF_GET(pdsvEvent, BaseBreakEventTrue_strF, strName))
				m_stlMnStrTrue[nKey] = strName;
			nKey	= pEventState->get_next_key(&it);
		}
		PT_OFree(pdsvEvent);
		bInit = true;
	}

	int nKey = _pEvent->get_key();
	if(nKey == HASH_STATE(BaseStateEventGlobal))
	{
		const int *pnKeyState;
		if(_pEvent->get(nKey, (const void**)&pnKeyState))
		{
			STLMnString::iterator	it;
			it = m_stlMnStrCast.find(*pnKeyState);
			if(it != m_stlMnStrCast.end())
            {
				STLMnInt stlMnGroup;
				STLMnInt::const_iterator	itGroup;
				BaseState::group_id_get(_pEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);
				itGroup = stlMnGroup.begin();
                g_SendMessage(LOG_MSG_POPUP, "%d:%d::%s\n", itGroup->first, itGroup->second, it->second.c_str());
			}
		}
	}

	bool ret = m_pstlCircleQueue->push(pEvt);// Thread safe code
	B_ASSERT(ret);
}

void BaseEventManager::break_befor_propogate(BaseDStructureValue *_pEvent)
{
	if(_pEvent == NULL)
		return;
	int nKey = _pEvent->get_key();
	if(nKey == HASH_STATE(BaseStateEventGlobal))
	{
		const int *pnKeyState;
		if(_pEvent->get(nKey, (const void**)&pnKeyState))
		{
			STLMnString::iterator	it;
			it = m_stlMnStrBeforPropogate.find(*pnKeyState);
			if(it != m_stlMnStrBeforPropogate.end())
			{
				STLMnInt stlMnGroup;
				STLMnInt::const_iterator	itGroup;
				BaseState::group_id_get(_pEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);
				itGroup = stlMnGroup.begin();
				g_SendMessage(LOG_MSG_POPUP, "%d:%d::%s\n", itGroup->first, itGroup->second, it->second.c_str());
			}
		}
	}
}

void BaseEventManager::break_befor_true(BaseDStructureValue *_pEvent)
{
	if(_pEvent == NULL)
		return;
	int nKey = _pEvent->get_key();
	if(nKey == HASH_STATE(BaseStateEventGlobal))
	{
		const int *pnKeyState;
		if(_pEvent->get(nKey, (const void**)&pnKeyState))
		{
			STLMnString::iterator	it;
			it = m_stlMnStrTrue.find(*pnKeyState);
			if(it != m_stlMnStrTrue.end())
			{
				STLMnInt stlMnGroup;
				STLMnInt::const_iterator	itGroup;
				BaseState::group_id_get(_pEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);
				itGroup = stlMnGroup.begin();
				g_SendMessage(LOG_MSG_POPUP, "%d:%d::%s\n", itGroup->first, itGroup->second, it->second.c_str());
			}
		}
	}
}

bool operator<(const BaseEvent &_Left, const BaseEvent &_Right)
{
	const BaseDStructureValue	*pLeft, *pRight;

	pLeft = _Left.m_pdsvEvent;
	pRight = _Right.m_pdsvEvent;

	unsigned nZero = 0;
	const unsigned *pnTimeL, *pnTimeR;

	if (!pLeft->get(HASH_STATE(TIMETOPROCESS), (const void**)&pnTimeL))
		pnTimeL = &nZero;

	if (!pRight->get(HASH_STATE(TIMETOPROCESS), (const void**)&pnTimeR))
		pnTimeR = &nZero;

	if (_Left.m_nPriority == _Right.m_nPriority)
	{
		if (*pnTimeL == *pnTimeR)
			return (_Left.m_nSequence < _Right.m_nSequence);
		return (*pnTimeL < *pnTimeR);
	}
	return (_Left.m_nPriority > _Right.m_nPriority);
}

bool compare_event(BaseEvent &_Link1, BaseEvent &_Link2)
{
	return (_Link1 < _Link2);
}

BaseDStructureValue *BaseEventManager::get_top(unsigned _nTime)
{
	if (m_stlVEvent.size() == 0)
		m_nSequence = 0;

	while(m_pstlCircleQueue->top())// Thread safe code
	{
		BaseEvent *pEvt;
		pEvt	= (BaseEvent*)m_pstlCircleQueue->pop();// Thread safe code
		pEvt->m_nSequence = m_nSequence++;
		
		int i;
		
		if (pEvt->m_nPriority > 0)
		{
			i = 0; 
			while(i < (int)m_stlVEvent.size() && compare_event(m_stlVEvent[i], *pEvt))
				i++;
			m_stlVEvent.insert(m_stlVEvent.begin() + i, *pEvt);
		}
		else {
			i = (int)m_stlVEvent.size() - 1;
			while (i > 0 && !compare_event(m_stlVEvent[i], *pEvt))
				i--;
			m_stlVEvent.insert(m_stlVEvent.begin() + (i+1), *pEvt);
		}
		PT_Free(pEvt);
	}

	if (m_stlVEvent.size() == 0)
		return NULL;

	BaseDStructureValue *pdsvEvent = m_stlVEvent.front().m_pdsvEvent;

	const unsigned	*pnTime;
	if(pdsvEvent->get(HASH_STATE(TIMETOPROCESS), (const void**)&pnTime) == false)
		return pdsvEvent;

	if(*pnTime <= _nTime)
	{
		return pdsvEvent;
	}
	return NULL;
}

void BaseEventManager::pop_event()
{
	m_stlVEvent.front().m_pdsvEvent->ref_dec();// instade of the PTOFree(m_stlEvent.front().m_pdsvEvent);
	m_stlVEvent.erase(m_stlVEvent.begin());
}
