#include "stdafx.h"
#include "BaseObject.h"
#include "BaseState.h"
#include "BaseStateMain.h"
#include "BaseStateManager.h"
#include "BaseStateSpace.h"

#include "BaseFile.h"
#include "BaseResFilterIP.h"
#include "BaseResManager.h"
#include "BaseTime.h"
#include "BaseStateSpace.h"
#include "BaseStringTable.h"
//#include "KISA_SHA256.h"

STLVInt	BaseState::sm_stlVHash;
BaseState	BaseState::sm_sample(NULL, NULL, 0, NULL);

int	s_nIdentifier = 10000;

#define VER_ADDSERIAL

#ifdef _DEBUG
#define TRACE_STATE
#else
#ifdef ANDROID
#define TRACE_STATE
#endif
#endif

#define STDEF_STNAMEOFCLASS	BaseState
#undef STDEF_MANAGERNAME
#define STDEF_MANAGERNAME	BaseStateManager

void BaseState::RegistState(BaseDStructure* _pdstState)
{
}

void BaseState::RegistLink(BaseDStructure* _pdstLink)
{
}

int BaseState::send_msg_error(PtLogType _nFilter, const char* _strFormat, ...)
{
	char logbuf[1024];

	va_list	argptr;
	va_start(argptr, _strFormat);
	//if( 0==argptr ) return 0;
	vsprintf_s(logbuf, 1024, _strFormat, argptr);
	va_end(argptr);

	g_SendMessage(_nFilter, logbuf);

	return 0;
}

bool compare_link(BaseDStructureValue* _pLink1, BaseDStructureValue* _pLink2)
{
	const int* pnPriority1, * pnPriority2;
	int nPr1, nPr2;

	if (!_pLink1->get(HASH_STATE(HASH_BaseLinkPriority_n), (const void**)& pnPriority1))
		nPr1 = 0;
	else
		nPr1 = *pnPriority1;

	if (!_pLink2->get(HASH_STATE(HASH_BaseLinkPriority_n), (const void**)& pnPriority2))
		nPr2 = 0;
	else
		nPr2 = *pnPriority2;

	return (nPr1 > nPr2);
}

bool BaseState::state_set_(const BaseDStructure* _pdstState, const BaseDStructure* _pdstLink, int _key)
{
	m_pdstLink = _pdstLink;

	if (_pdstState)
	{
		BaseDStructureValue* pState = NULL;
		PT_OAlloc2(pState, BaseDStructureValue, _pdstState, 1024);

		pState->m_pVoidParam = this;
		if (!pState->set_key(_key))
		{
			PT_OFree(pState);
			return false;
		}
		m_pdsvState = pState;
		if (g_logger()) {
			m_pdsvState->logger_reset("state_set_");
		}
	}

	unsigned	i;
	for (i = 0; i < m_stlVpLink.size(); i++)
	{
		int nCnt = 0;
		PT_OFree(m_stlVpLink[i]);
	}
	m_stlVpLink.clear();

	if (m_pdsvVariable)
		m_pdsvVariable->state_set_(_pdstState);

	if (m_pdsvVariableNextReserve)
		m_pdsvVariableNextReserve->state_set_(_pdstState);

	if (m_pdsvContext)
		m_pdsvContext->state_set_(_pdstState);

	for (i = 0; i < m_stlVpStateCandidate.size(); i++)
	{
	}
	
	if (m_pdsvState)
	{
//#ifdef TRACE_STATE
#ifdef _DEBUG
		const char* strName;
		if (m_pdsvState->get(HASH_STATE(STR_NAME), (const void**)&strName))
		{
			m_strName = strName;
		}
#endif
//#endif
		add_link_(m_pdsvState);
	}
	std::sort(m_stlVpLink.begin(), m_stlVpLink.end(), compare_link);

	if (m_pSubState)
		m_pSubState->state_set_(_pdstState, _pdstLink, _key);
	return true;
}

void BaseState::init(BaseStateSpace* _pSpace, const BaseDStructure* _pdstState, int _nKey, const BaseDStructure* _pdstLink, const STLMnInt* _pstlMnGroupId, int _nObjectSerial)
{
	BaseObject::init(_nObjectSerial);
	m_state_serial = m_nSerialNumber;
	m_pStateMain = NULL;
	m_pdsvState = NULL;
	m_pdsvContext = NULL;
	m_bSkipNextLink = false;
	m_bSuspended = false;
	m_pdsvEventSend = NULL;
	m_pdsvEventSystem = NULL;
	m_pdsvVariable = NULL;
	m_pdsvVariableNextReserve = NULL;
	m_pSpace = _pSpace;
	
	if (sm_stlVHash.size() == 0)
	{
		init_hash();
	}

	m_pParentState = NULL;
	m_pSubState = NULL;
	m_pLeafState = NULL;
	if (_pstlMnGroupId)
		m_stlMnGroupIdentifier = *_pstlMnGroupId;
	m_nTimeLocal = 0;
	m_nTimeDelay = 0;
	m_stlVpLinkDelayed.clear();
	m_stlVnLinkTimeDelayed.clear();

	state_set_(_pdstState, _pdstLink, _nKey);
}

void BaseState::add_link_(BaseDStructureValue* _pdsvState)
{
	const int* anLink;
	short nCnt = 0;
	if (_pdsvState->get(sm_stlVHash[ARRAY_LINK], (const void**)& anLink, &nCnt)
		&& nCnt > 0)
	{
		for (int i = 0; i < nCnt; i++)
		{
			int nKeyLink;
			nKeyLink = *(anLink + i);
			if (!m_pdstLink->is_exist(nKeyLink))
				continue;

			BaseDStructureValue* pdsvLink;
			PT_OAlloc2(pdsvLink, BaseDStructureValue, m_pdstLink, 0);

			pdsvLink->set_key(nKeyLink);

			const int* pnKey;
			if (pdsvLink->get(sm_stlVHash[HASH_BaseLinkType], (const void**)& pnKey)
				&& *pnKey == sm_stlVHash[STATE_LINKCOPY]
				&& !pdsvLink->get(sm_stlVHash[BaseFalse_nIf], (const void**)& pnKey)) // 같은 기능이 많은 Link들을 한 State에 모아서 재활용 하기 위해 사용한다
			{
				const char* strNameKey;
				if (!pdsvLink->get(sm_stlVHash[STR_NAME], (const void**)& strNameKey))
					continue;
				nKeyLink = STRTOHASH(strNameKey);
				if (!m_pdsvState->get_base()->is_exist(nKeyLink))
					continue;

				BaseDStructureValue* pdsvLinkCopy;
				PT_OAlloc2(pdsvLinkCopy, BaseDStructureValue, m_pdsvState->get_base(), 0);

				pdsvLinkCopy->set_key(nKeyLink);
				add_link_(pdsvLinkCopy);// 해당 State의 Link를 내것 처럼 가저와 쓴다

				PT_OFree(pdsvLinkCopy);
				PT_OFree(pdsvLink);
				continue;
			}

			pdsvLink->m_pVoidParam = this;
			
			m_stlVpLink.push_back(pdsvLink);
		}
	}
}

void BaseState::release()
{
	unsigned	i;
	for (i = 0; i < m_stlVpLink.size(); i++)
	{
		int nCnt = 0;
		//m_stlVpLink[i]->event_process(NULL, context_get(), BaseDStructure::STATE_END, &nCnt);
		PT_OFree(m_stlVpLink[i]);
	}
	m_stlVpLink.clear();
	if (m_pdsvState)
	{
		int nCnt = 0;
		//m_pdsvState->event_process(NULL, context_get(), BaseDStructure::STATE_END, &nCnt);
		PT_OFree(m_pdsvState);
		m_pdsvState = NULL;
	}

	if (m_pdsvVariable)
	{
		PT_OFree(m_pdsvVariable);
		m_pdsvVariable = NULL;
	}

	if (m_pdsvVariableNextReserve)
	{
		PT_OFree(m_pdsvVariableNextReserve);
		m_pdsvVariableNextReserve = NULL;
	}

	if (m_pSubState)
		PT_OFree(m_pSubState);
	m_pSubState = NULL;

	if (m_pdsvContext)
		m_pdsvContext->ref_dec();
	m_pdsvContext = NULL;

	for (i = 0; i < m_stlVpStateCandidate.size(); i++)
	{
		PT_OFree(m_stlVpStateCandidate[i]);
	}
	m_stlVpStateCandidate.clear();
}

BaseState::BaseState(BaseStateSpace* _pSpace, const BaseDStructure* _pdstState, int _nKey, const BaseDStructure* _pdstLink)
{
	m_pdsvContext = NULL;
	init(_pSpace, _pdstState, _nKey, _pdstLink);
}

BaseState::~BaseState(void)
{
	release();
}

void BaseState::variable_set(BaseDStructureValue* _pdsvVariable)
{
	if (m_pdsvVariable != NULL)
		PT_OFree(m_pdsvVariable);

	m_pdsvVariable = _pdsvVariable;
	m_pdsvVariable->m_pVoidParam = this;
	m_pdsvVariable->set_key(HASH_STATE(STATE_CONTEXT));
}

BaseDStructureValue* BaseState::variable_get()
{
	if (m_pdsvVariable == NULL)
	{
		PT_OAlloc2(m_pdsvVariable, BaseDStructureValue, m_pdsvState->get_base(), 0);
		m_pdsvVariable->m_pVoidParam = this;
		m_pdsvVariable->set_key(HASH_STATE(STATE_CONTEXT));
	}
	return m_pdsvVariable;
}

BaseDStructureValue* BaseState::get_state_value()
{
	return m_pdsvState;
}

void BaseState::init_hash()
{
	sm_stlVHash.resize(MAX_HASH);

	sm_stlVHash[TIMETOPROCESS] = STRTOHASH(RES_STR_TIMEPROCESS);
	sm_stlVHash[N_KEY] = STRTOHASH(RES_STR_KEY);
	sm_stlVHash[ARRAY_LINK] = STRTOHASH(RES_STR_ARRAYLINK);
	sm_stlVHash[STR_NAME] = STRTOHASH(RES_STR_NAME);
	sm_stlVHash[HASH_BaseTimeOut_nREU] = STRTOHASH(RES_STR_TIMEOUT);
	sm_stlVHash[HASH_BaseTimeOut_varUIf] = STRTOHASH("BaseTimeOut_varUIf");
	sm_stlVHash[STATE_SYSTEM_SIGN] = STRTOHASH("STATE_SYSTEM_SIGN");
	sm_stlVHash[START_STATE] = STRTOHASH("start_state");
	sm_stlVHash[STATE_CHANGE] = STRTOHASH("state_change");
	sm_stlVHash[STATE_ADD] = STRTOHASH("state_add");
	sm_stlVHash[STATE_RELEASE] = STRTOHASH("state_release");
	sm_stlVHash[STATE_DUMMY] = STRTOHASH("state_dummy");
	sm_stlVHash[STATE_CALL] = STRTOHASH("state_call");
	sm_stlVHash[STATE_RETURN] = STRTOHASH("state_return");
	sm_stlVHash[STATE_CONTEXT] = STRTOHASH("state_context");
	sm_stlVHash[STATE_LINKCOPY] = STRTOHASH("state_linkcopy");
	sm_stlVHash[SPACE_TRANSLATE] = STRTOHASH("space_translate");
	sm_stlVHash[BaseTransitionGoalMain] = STRTOHASH("BaseTransitionGoalMain");
	sm_stlVHash[BaseTransitionThread] = STRTOHASH("BaseTransitionThread");
	sm_stlVHash[BaseTransitionGoalpState] = STRTOHASH("BaseTransitionGoalpState");
	sm_stlVHash[HASH_BaseTransitionGoalSpaceID] = STRTOHASH("BaseTransitionGoalSpaceID");
	sm_stlVHash[BaseTransitionGoalIdentifier] = STRTOHASH("BaseTransitionGoalIdentifier");
	sm_stlVHash[BaseIdentifierParent_n] = STRTOHASH("BaseIdentifierParent_n");
	sm_stlVHash[BaseIdentifierChilds_an] = STRTOHASH("BaseIdentifierChilds_an");
	sm_stlVHash[BaseTransitionNextIdentifier] = STRTOHASH("BaseTransitionNextIdentifier");
	sm_stlVHash[BaseTransitionCasterKey] = STRTOHASH("BaseTransitionCasterKey");
	sm_stlVHash[HASH_BaseTransitionGoal] = STRTOHASH("BaseTransitionGoal");
	sm_stlVHash[BaseEventTargetStateSerial] = STRTOHASH("BaseEventTargetStateSerial");
	sm_stlVHash[BaseFalse_nIf] = STRTOHASH("BaseFalse_nIf");
	sm_stlVHash[HASH_BaseLinkType] = STRTOHASH("BaseLinkType");
	sm_stlVHash[N_EVENT_FLASH] = STRTOHASH("nEventFlash");
	sm_stlVHash[HASH_FlashUIEvent_strRE] = STRTOHASH("FlashUIEvent_strRE");
	sm_stlVHash[HASH_Comment] = STRTOHASH("Comment");
	sm_stlVHash[B_KEEP_STATE] = STRTOHASH("bKeepState");
	sm_stlVHash[HASH_BaseLinkDelay] = STRTOHASH("BaseLinkDelay");
	sm_stlVHash[HASH_BaseLinkOneTime_b] = STRTOHASH("BaseLinkOneTime_b");
	sm_stlVHash[HASH_BaseExclusiveState] = STRTOHASH("BaseExclusiveState");
	sm_stlVHash[HASH_nScanCode] = STRTOHASH("nScanCode");
	sm_stlVHash[HASH_nScanCode2] = STRTOHASH("nScanCode2");
	sm_stlVHash[HASH_BaseKeyPush_nRE] = STRTOHASH("BaseKeyPush_nRE");
	sm_stlVHash[HASH_BaseKeyRelease_nRE] = STRTOHASH("BaseKeyRelease_nRE");
	sm_stlVHash[HASH_ID_CHARACTER] = STRTOHASH("ID_CHARACTER");
	sm_stlVHash[HASH_ID_AutoReturnState] = STRTOHASH("BaseSetAutoReturnState_nF");
	sm_stlVHash[AutoReturnAfterDoResume] = STRTOHASH("BaseSetAutoReturnAfterDoResume");
	sm_stlVHash[HASH_BaseLinkPriority_n] = STRTOHASH("BaseLinkPriority_n");
	sm_stlVHash[BaseEventPriority_nV] = STRTOHASH("BaseEventPriority_nV");
	sm_stlVHash[BaseLinkIndex_nV] = STRTOHASH("BaseLinkIndex_nV");
	sm_stlVHash[HASH_BaseExclusiveLink] = STRTOHASH("BaseExclusiveLink");
	sm_stlVHash[HASH_Index_n] = STRTOHASH("Index_n");
	sm_stlVHash[HASH_BaseLinkDelay_nF] = STRTOHASH("BaseLinkDelay_nF");
	sm_stlVHash[HASH_BaseKeyPushed_nIf] = STRTOHASH("BaseKeyPushed_nIf");
	sm_stlVHash[HASH_BaseKeyPushed_nUIf] = STRTOHASH("BaseKeyPushed_nUIf");
	sm_stlVHash[HASH_BaseKeyReleased_nIf] = STRTOHASH("BaseKeyReleased_nIf");
	sm_stlVHash[HASH_EnumEvent] = STRTOHASH("EnumEvent");
	sm_stlVHash[HASH_EnumEventKey] = STRTOHASH("EnumEventKey");
	sm_stlVHash[EnumStateEvent] = STRTOHASH("EnumStateEvent");
	sm_stlVHash[EnumStructure] = STRTOHASH("EnumStructure");
	sm_stlVHash[BaseStateEventGlobal] = STRTOHASH("BaseStateEventGlobal");
	sm_stlVHash[BaseStateEventReturn] = STRTOHASH("BaseStateEventReturn");
	sm_stlVHash[HASH_BaseStateEventGlobalUnprocess] = STRTOHASH("BaseStateEventGlobalUnprocess");
	sm_stlVHash[HASH_BaseStateGradeSet_n] = STRTOHASH("BaseStateGradeSet_n");
	sm_stlVHash[HASH_BaseLinkInverse] = STRTOHASH("BaseLinkInverse");
	//sm_stlVHash[ParamInt]			= STRTOHASH("ParamInt");
	//sm_stlVHash[BaseNextTempValues]	= STRTOHASH("BaseNextTempValues");
	//sm_stlVHash[SelectorSerial_nV]	= STRTOHASH("SelectorSerial_nV");
	sm_stlVHash[FlashUIEvent_strRE] = STRTOHASH("FlashUIEvent_strRE");
	sm_stlVHash[BaseValueTimeStart] = STRTOHASH("BaseValueTimeStart");
	sm_stlVHash[BaseValueIdentify] = STRTOHASH("BaseValueIdentify");
	sm_stlVHash[EnumVariableDefine] = STRTOHASH("EnumVariableDefine");
	sm_stlVHash[BaseFuncAddInverse_nIf] = STRTOHASH("BaseFuncAddInverse_nIf");
	sm_stlVHash[BaseVariableRefer_anV] = STRTOHASH("BaseVariableRefer_anV");
	sm_stlVHash[BaseVariableReferType_nV] = STRTOHASH("BaseVariableReferType_nV");
	sm_stlVHash[BaseVariableRefer_strV] = STRTOHASH("BaseVariableRefer_strV");
	sm_stlVHash[BaseVariableString_strV] = STRTOHASH("BaseVariableString_strV");
	sm_stlVHash[BaseVariableRefRemove_nV] = STRTOHASH("BaseVariableRefRemove_nV");
	sm_stlVHash[BaseVariableToVariable_anV] = STRTOHASH("BaseVariableToVariable_anV"); 
	sm_stlVHash[BaseVariableToStructure_anV] = STRTOHASH("BaseVariableToStructure_anV");
	sm_stlVHash[BaseVariableToStructure_varV] = STRTOHASH("BaseVariableToStructure_varV");
	sm_stlVHash[BaseVariableToReferVar_varV] = STRTOHASH("BaseVariableToReferVar_varV");
	sm_stlVHash[BaseVariableRefer_an64V] = STRTOHASH("BaseVariableRefer_an64V");
	sm_stlVHash[BaseVariableBreak_strV] = STRTOHASH("BaseVariableBreak_strV");
	sm_stlVHash[BaseVariableConst_nV] = STRTOHASH("BaseVariableConst_nV");
	sm_stlVHash[BaseVariableConst_fV] = STRTOHASH("BaseVariableConst_fV");
	sm_stlVHash[BaseVariableConst_strV] = STRTOHASH("BaseVariableConst_strV");
	sm_stlVHash[RevTimeCurrentYear] = STRTOHASH("RevTimeCurrentYear");
	sm_stlVHash[RevTimeCurrentMonth] = STRTOHASH("RevTimeCurrentMonth");
	sm_stlVHash[RevTimeCurrentDay] = STRTOHASH("RevTimeCurrentDay");
	sm_stlVHash[RevRandomDigit6] = STRTOHASH("RevRandomDigit6");
	sm_stlVHash[RevTimeCurrentHour] = STRTOHASH("RevTimeCurrentHour");
	sm_stlVHash[RevTimeCurrentMinute] = STRTOHASH("RevTimeCurrentMinute");
	sm_stlVHash[RevTimeCurrentSecond] = STRTOHASH("RevTimeCurrentSecond");
	sm_stlVHash[RevStateSerial] = STRTOHASH("RevStateSerial");
	sm_stlVHash[RevGroupStart] = STRTOHASH("RevGroupStart");
	sm_stlVHash[RevGroupEnd] = STRTOHASH("RevGroupEnd");
	sm_stlVHash[Context] = STRTOHASH("Context");
	sm_stlVHash[Event] = STRTOHASH("Event");
	sm_stlVHash[EventCast] = STRTOHASH("EventCast");
	sm_stlVHash[EventCastSystem] = STRTOHASH("EventCastSystem");
	sm_stlVHash[State] = STRTOHASH("State");
	sm_stlVHash[StateUI] = STRTOHASH("StateUI");
	sm_stlVHash[Global] = STRTOHASH("Global");
	sm_stlVHash[File] = STRTOHASH("File");
	sm_stlVHash[BaseVariableFilename_strV] = STRTOHASH("BaseVariableFilename_strV");
	sm_stlVHash[StateNext] = STRTOHASH("StateNext");
	sm_stlVHash[BaseStateGroupId_nV] = STRTOHASH("BaseStateGroupId_nV");
	sm_stlVHash[None] = STRTOHASH("None");
	sm_stlVHash[Socket] = STRTOHASH("Socket");
	sm_stlVHash[Server] = STRTOHASH("Server");
	sm_stlVHash[RevStateEventCaster] = STRTOHASH("RevStateEventCaster");
	sm_stlVHash[BaseTransitionThreadCaster] = STRTOHASH("BaseTransitionThreadCaster");
	sm_stlVHash[RevNetStateEventCaster] = STRTOHASH("RevNetStateEventCaster");
	sm_stlVHash[BaseVariableDefine_nV] = STRTOHASH("BaseVariableDefine_nV");
	sm_stlVHash[BaseDebugHideLog] = STRTOHASH("BaseDebugHideLog");
	//======================================================================================= Option Start
}

bool BaseState::hash_append(int _nIndex, int _nHash)
{
	if (_nHash == 0 || _nIndex >= (int)sm_stlVHash.size())
		return false;

	if (sm_stlVHash[_nIndex] != 0)
	{
		B_ASSERT(sm_stlVHash[_nIndex] == 0);
		return false;
	}

	sm_stlVHash[_nIndex] = _nHash;
	return true;
}


const BaseDStructureValue* BaseState::get_state_value() const
{
	return m_pdsvState;
}

void BaseState::set_parent(BaseState* _pParent)
{
	m_pParentState = _pParent;
}

BaseState* BaseState::get_parent()
{
	BaseState* pRet;
	if (m_pParentState)
	{
		pRet = m_pParentState->get_parent();
		if (pRet)
			return pRet;
		return m_pParentState;
	}
	return NULL;
}

bool BaseState::is_mine(const BaseState* _pState) const
{
	if (_pState == this)
		return true;

	if (m_pSubState)
		return m_pSubState->is_mine(_pState);

	return false;
}

void BaseState::regist_func_()
{
	m_pdsvState->regist_event_func(m_pSpace);
	
	for (unsigned i = 0; i < m_stlVpLink.size(); i++)
	{
		int link_hash = m_stlVpLink[i]->get_key();

		if (!m_stlVpLink[i]->regist_event_func(m_pSpace))
		{
			PT_OFree(m_stlVpLink[i]);
			m_stlVpLink.erase(m_stlVpLink.begin() + i);// Add or Update by OJ 2012-09-11 오후 1:58:37
			i--;
		}
		else
		{
			if (g_logger())
				m_stlVpLink[i]->logger_reset("regist_func_");
		}
	}
	if(m_pSubState)
		m_pSubState->regist_func_();
}

int BaseState::OnStart(BaseDStructureValue* _pdsvEvent)
{
	m_bSuspended = false;
	m_nTimeLocal = 0;
	DebugOutString(_pdsvEvent, NULL, "OnStart");

	STLMnInt	stlMnHash;
	if (m_pStateMain->is_conformer())
		make_candidate_list(&m_stlVpStateCandidate, &stlMnHash);

	int nCnt = 0;
	if (g_logger()) {
		m_pdsvState->logger_reset("OnStart");
		m_pdsvState->logger_send(BaseDStructureValue::TYPE_Logger_start, get_key_main(), get_key(), get_serial(), 0, m_stlMnGroupIdentifier, _pdsvEvent ? _pdsvEvent->serial() : 0);
	}

	m_pStateMain->next_active_state_del(get_key(), m_stlMnGroupIdentifier);

	regist_func_();
	m_pdsvState->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_START, &nCnt);
	
	for (unsigned i = 0; i < m_stlVpLink.size(); i++)
	{
		int nProcessCnt = 0;
		int link_hash = m_stlVpLink[i]->get_key();
		
		if (m_stlVpLink[i]->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_LINK_START, &nProcessCnt)
			&& m_stlVpLink[i]->is_complete(_pdsvEvent, context_get()))
		{
			if (LinkProcess(i, _pdsvEvent))
				i--;// Add by OJ 2010-04-09 return 값이 true일 경우 Link가 지워진 경우 이다.

			if (m_bSkipNextLink || m_bSuspended)
			{
				m_bSkipNextLink = false;// Add by OJ : 2018-10-30 It should set false to allow next event process in Space Actor(fast rutine).
					//
				break; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
			}
		}
	}// end of for loop

	return RET_UNPROCESS;
}

int BaseState::OnUpdate(unsigned _nTimeDelta)
{
	if (m_pLeafState)
		return m_pLeafState->OnUpdate(_nTimeDelta);

	if (m_bSuspended)// 이미 Call, Translate, Return, Release등의 처리 이벤트를 발생 시켰다
		return RET_UNPROCESS;

	UpdateFuncUpdate(_nTimeDelta);

	//==================================================================================================
	// Start Add or Update by OJ 2012-08-01 오전 11:28:25

	//for(unsigned i=0; i<m_stlVpLinkDelayed.size(); i++)
	//{
	//	if(m_stlVnLinkTimeDelayed[i] <= (int)_nTimeDelta)
	//	{
	//		LinkProcess_(m_stlVpLinkDelayed[i]);

	//		if(m_bSkipNextLink)
	//			return RET_PROCESS; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
	//	}else{
	//		m_stlVnLinkTimeDelayed[i] -= _nTimeDelta;
	//	}
	//}

	m_nTimeLocal += _nTimeDelta;

	// End by OJ 2012-08-01 오전 11:28:30
	//==================================================================================================
//	int nCnt=0;
//	m_pdsvState->event_process(context_get(), BaseDStructure::STATE_UPDATE, &nCnt);
//
//	for(unsigned i=0; i<m_stlVpLink.size(); i++)
//	{
//		int nProcessCnt	= 0;
//#ifdef TRACE_STATE
//		char strNameLink[255];
//		strcpy_s(strNameLink, 255, m_stlVpLink[i]->get_name_debug());
//#endif
//		if(m_stlVpLink[i]->event_process(context_get(), BaseDStructure::STATE_UPDATE, &nProcessCnt)
//			&& m_stlVpLink[i]->is_complete(NULL, context_get()))
//		{
//			if(LinkProcess(i))
//				i--;// Add by OJ 2010-04-09 return 값이 true일 경우 Link가 지워진 경우 이다.
//
//
//			if(m_bSkipNextLink)
//			{
//				break; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
//			}
//		}
//
//	}// end of for loop

	return RET_UNPROCESS;
}

BaseDStructureValue* BaseState::link_get(const BaseDStructureValue* _pLink)
{
	for (unsigned i = 0; i < m_stlVpLink.size(); i++)
	{
		if (m_stlVpLink[i] == _pLink)
			return m_stlVpLink[i];
	}
	return NULL;
}

int BaseState::LinkProcess(int _nIndexLink, BaseDStructureValue* _pdsvRecEvent)
{
	BaseDStructureValue* pLink;
	pLink = m_stlVpLink[_nIndexLink];

	{// Delay Link Check
		const int* pnTimeDelay;
		if (pLink->get(sm_stlVHash[HASH_BaseLinkDelay_nF], (const void**)& pnTimeDelay))
		{
			m_stlVpLinkDelayed.push_back(pLink);
			m_stlVnLinkTimeDelayed.push_back(*pnTimeDelay);
			return 0;
		}
	}

	return LinkProcess_(pLink, _pdsvRecEvent);
}

bool BaseState::groupid_get_next(STLMnInt* _pstlMnGroupId, BaseDStructureValue* _pdsvEvent) {
	if (m_stlMNextGroupID.size() > 0)
	{
		*_pstlMnGroupId = m_stlMnGroupIdentifier;
		BaseState::group_id_add(_pstlMnGroupId, m_stlMNextGroupID);
		return true;
	}
	else if (_pdsvEvent != NULL) {
		return group_id_get(_pdsvEvent, HASH_STATE(BaseTransitionNextIdentifier), *_pstlMnGroupId);
	}
	return false;
}

int BaseState::group_id_get(int _hash) const
{
	STLMnInt::const_iterator it;
	it = m_stlMnGroupIdentifier.find(_hash);

	if (it == m_stlMnGroupIdentifier.end())
		return 0;
	return it->second;
}

const STLMnInt& BaseState::group_id_get()
{
	return m_stlMnGroupIdentifier;
}

void BaseState::group_id_set(const STLMnInt& _stlMnGroupId)
{
	m_stlMnGroupIdentifier = _stlMnGroupId;
}

bool BaseState::group_id_is_in(const STLMnInt& _stlMnGroupId) const
{
	STLMnInt::const_iterator it = _stlMnGroupId.begin();
	bool bFind = false;

	for (; it != _stlMnGroupId.end(); it++)
	{
		if (it->first == HASH_STATE(None) && it->second == 0)
			continue;
		bFind = false;

		STLMnInt::const_iterator itIn = m_stlMnGroupIdentifier.begin();
		for (; itIn != m_stlMnGroupIdentifier.end(); itIn++)
		{
			if (it->first == itIn->first && (it->second == itIn->second || it->second == -1))
				bFind = true;
		}

		if (bFind == false)
			return false;
	}
	return true;
}

char* BaseState::group_id_get_string(char* _strBuf, int _nLen) const
{
	char strCat[255];

	STLMnInt::const_iterator it = m_stlMnGroupIdentifier.begin();
	for (; it != m_stlMnGroupIdentifier.end(); it++)
	{
		sprintf_s(strCat, 255, "GID:%d S:%d ", it->first, it->second);
		strcat_s(_strBuf, _nLen, strCat);
	}
	return _strBuf;
}

void BaseState::group_id_add(STLMnInt* _group_id_stlMn, STLMnInt& _group_id_add_stlMn)
{
	STLMnInt::iterator it;
	for (it = _group_id_add_stlMn.begin(); it != _group_id_add_stlMn.end(); it++)
	{
		(*_group_id_stlMn)[it->first] = it->second;
	}
}

void BaseState::group_id_add(int _nGroupId, int _nIdentifier)
{
	m_stlMnGroupIdentifier[_nGroupId] = _nIdentifier;
}


bool BaseState::group_id_include(const STLMnInt& _stlMnGroupIdSmall) const
{
	return group_id_include(_stlMnGroupIdSmall, m_stlMnGroupIdentifier);
}

bool BaseState::group_id_equal(const STLMnInt& _stlMnGroupId) const
{
	return group_id_equal(_stlMnGroupId, m_stlMnGroupIdentifier);
}

bool BaseState::group_id_variable(const STLMnInt* _pstlMnGroup)
{
	if (_pstlMnGroup == NULL)
		return false;

	STLMnInt::const_iterator it = _pstlMnGroup->begin();
	for (; it != _pstlMnGroup->end(); it++)
	{
		if (it->second != 0)
			return true;
	}
	return false;
}

bool BaseState::group_id_include(const STLMnInt& _stlMnGroupIDSmall, const STLMnInt& _stlMnGroupIDBig)
{
	int nHitCnt = 0, nSearchCnt = 0;

	STLMnInt::const_iterator it = _stlMnGroupIDSmall.begin();
	for (; it != _stlMnGroupIDSmall.end(); it++)
	{
		STLMnInt::const_iterator it2 = _stlMnGroupIDBig.find(it->first);
		if (it2 != _stlMnGroupIDBig.end()
			&& it->second == it2->second)
			nHitCnt++;

		nSearchCnt++;
	}

	if (nSearchCnt == nHitCnt)
		return true;
	return false;
}

bool BaseState::group_id_equal(const STLMnInt& _stlMnGroupID1, const STLMnInt& _stlMnGroupID2)
{
	int nHitCnt = 0, nSearchCnt = 0;

	STLMnInt::const_iterator it = _stlMnGroupID1.begin();
	for (; it != _stlMnGroupID1.end(); it++)
	{
		STLMnInt::const_iterator it2 = _stlMnGroupID2.find(it->first);
		if (it2 != _stlMnGroupID2.end()
			&& it->second == it2->second)
			nHitCnt++;

		nSearchCnt++;
	}

	if (nSearchCnt == nHitCnt
		&& nHitCnt == _stlMnGroupID2.size())
		return true;
	return false;
}

void BaseState::group_id_set(BaseDStructureValue* _pdstValue, int _nHash, const STLMnInt& _stlMnGroupID)
{
	if (_stlMnGroupID.size() == 0)
		return;

	int* pnIds;
	pnIds = PT_Alloc(int, _stlMnGroupID.size() * 2);
	STLMnInt::const_iterator it;
	it = _stlMnGroupID.begin();
	int i = 0;
	for (; it != _stlMnGroupID.end(); it++) {
		pnIds[i * 2] = it->first; // first is group id
		pnIds[i * 2 + 1] = it->second; // second is identifier
		i++;
	}
	short nCnt = (short)(_stlMnGroupID.size() * 2);
	_pdstValue->set_alloc(_nHash, pnIds, nCnt);

	PT_Free(pnIds);
}

bool BaseState::group_id_get(const BaseDStructureValue* _pdstValue, int _nHash, STLMnInt& _stlMnGroupID)
{
	const int* pnIds;
	short nCnt;
	_stlMnGroupID.clear();
	if (_pdstValue->get(_nHash, (const void**)& pnIds, &nCnt))
	{
		for (int i = 0; i < nCnt / 2; i++)
			_stlMnGroupID[pnIds[i * 2]] = pnIds[i * 2 + 1]; // group id is first
		return true;
	}
	return false;
}

bool BaseState::_check_exculsive(BaseStateMain* _pMain, BaseDStructureValue* _pEvent, BaseDStructureValue* _pLink, int _nGoalState, STLMnInt& _stlMnGroupID, const char* _strMsg)
{
	BaseState* pActive = _pMain->GetStateName((int)_nGoalState, &_stlMnGroupID); // first is group id

	if (pActive || _pMain->next_active_state_check((int)_nGoalState, _stlMnGroupID))
	{
		DebugOutString(_pEvent, _pLink, _strMsg);
		return false;
	}
	return true;
}

void BaseState::LoggerSend(int _key, BaseDStructureValue *_link, const STLMnInt& _groupID_m, int _event_serial)
{
	if (!g_logger())
		return;

	int key_link = 0;
	if (_link)
		key_link = _link->get_key();

	if (_key == sm_stlVHash[STATE_ADD]
		|| _key == sm_stlVHash[STATE_CALL]
		|| _key == sm_stlVHash[STATE_CHANGE])
	{
		_link->logger_send(BaseDStructureValue::TYPE_Logger_dummy, get_key_main(), get_key(), get_serial(), key_link, _groupID_m, _event_serial);
		if (_key == sm_stlVHash[STATE_CHANGE])
		{
			_link->logger_send(BaseDStructureValue::TYPE_Logger_end, get_key_main(), get_key(), get_serial(), 0, _groupID_m, _event_serial);
			if (m_pParentState)
				m_pParentState->LoggerSend(sm_stlVHash[STATE_RELEASE], m_pdsvState, _groupID_m, _event_serial);
		}
	}
	else if (_key == sm_stlVHash[STATE_DUMMY])
	{
		_link->logger_send(BaseDStructureValue::TYPE_Logger_dummy, get_key_main(), get_key(), get_serial(), key_link, _groupID_m, _event_serial);
	}
	else if (_key == sm_stlVHash[STATE_RELEASE]
		|| _key == sm_stlVHash[STATE_RETURN])
	{
		_link->logger_send(BaseDStructureValue::TYPE_Logger_end, get_key_main(), get_key(), obj_serial_get(), key_link, _groupID_m, _event_serial);
		if (m_pParentState && _key == sm_stlVHash[STATE_RELEASE])
			m_pParentState->LoggerSend(sm_stlVHash[STATE_RELEASE], m_pdsvState, _groupID_m, _event_serial);
	}
}

int BaseState::LinkProcess_(BaseDStructureValue* pLink, BaseDStructureValue* _pdsvRecEvent, bool* _linkcomplete_b)
{
	STLMnInt    stlMnGroupIds;
	INT32 nTargetMain, nGoalState, nKeyStateCurrent;
	nTargetMain = m_pdsvState->get_base()->get_key();
	nKeyStateCurrent = m_pdsvState->get_key();

	int link = pLink->get_key();
#ifdef _DEBUG
	if(link == 465824612)
	{
		g_SendMessage(LOG_MSG, "------ link for the timeout");
	}
#endif
	if (_linkcomplete_b)
		*_linkcomplete_b = false;

	bool	bPosted = false;

	const int* pnTargetMain = NULL;
	const int* createthread = NULL;
	if (pLink->get(HASH_STATE(BaseTransitionGoalMain), (const void**)&pnTargetMain))
	{
		nTargetMain = *pnTargetMain;
		pLink->get(HASH_STATE(BaseTransitionThread), (const void**)&createthread);
	}

	BaseStateMain* pMain = m_pSpace->GetStateMain((int)nTargetMain);
	BaseStateManager* pManager = m_pSpace->get_manager();

	const bbyte* pbExclusive = NULL;
	int nKey = sm_stlVHash[STATE_CHANGE];

	m_pdsvEventSystem = pManager->make_event(nKey, nKey, 0);

	nGoalState = get_target_state_key(pLink);
	if (nGoalState)
	{
		const int* pnKey;

		if (!group_id_get(pLink, sm_stlVHash[BaseTransitionGoalIdentifier], stlMnGroupIds))
			stlMnGroupIds = m_stlMnGroupIdentifier;

		if(pLink->get(sm_stlVHash[HASH_BaseLinkType], (const void**)& pnKey))
			nKey = *pnKey;
		else{
			int x=0;
			x++;
		}

		bPosted = true;

		if (pLink->get(sm_stlVHash[HASH_BaseExclusiveState], (const void**)& pbExclusive))
		{// 배타적 Link는 오직 하나의 ActiveState를 허용한다.
			bPosted = _check_exculsive(pMain, m_pdsvEventSystem, pLink, nGoalState, stlMnGroupIds, "Block(Exclusive-by_id");
		}
	}

	bool bOneTime = true;
	const bbyte* pbOneTime;
	if (pLink->get(sm_stlVHash[HASH_BaseLinkOneTime_b], (const void**)& pbOneTime))
	{// OneTimeEvent가 True일 때 이 링크는 한번만 수행 될 수 있다
		bOneTime = (*pbOneTime != 0);
	}
	else {
		if (nKey == sm_stlVHash[STATE_ADD]
			|| nKey == sm_stlVHash[STATE_DUMMY]
			|| nKey == sm_stlVHash[STATE_CALL])
			bOneTime = false;
	}

	if (nKey != sm_stlVHash[STATE_ADD] && nKey != sm_stlVHash[STATE_DUMMY])
	{
		m_stlVpLinkDelayed.clear();
		m_stlVnLinkTimeDelayed.clear();
	}
	
	int nRet = 0;
	if (!bPosted)
	{
		PT_OFree(m_pdsvEventSystem);
	}
	else {
		//m_nNextIdentifier	= 0;// Add by OJ : 2010-11-10
		BaseDStructureValue* pdsvContext = context_get();
		m_pdsvEventSystem->set_clear();
		m_pdsvEventSystem->set_key(nKey);

		if (0 == pLink->event_process(_pdsvRecEvent, pdsvContext, BaseDStructure::STATE_LINK_COMPLETE, NULL)) // Link媛 紐⑤몢 True????Complete瑜??섑뻾?쒕떎.
		{ // jtseo 2020/06/10
#ifdef VER_ADDSERIAL
			PT_OFree(m_pdsvEventSystem);
			return 0;
#endif
		}
#ifdef VER_ADDSERIAL
		else	// jtseo 2020/06/10
			pLink->complete_make();
#endif
		if (_linkcomplete_b)
			*_linkcomplete_b = true;

		const void* pnExclusiveLink;
		if (pLink->get(sm_stlVHash[HASH_BaseExclusiveLink], &pnExclusiveLink))
			m_bSkipNextLink = true; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
		else
			m_bSkipNextLink = false;

		EventSendReset();// Event what unused will remove by oj 2015-01-02

		LoggerSend(nKey, pLink, stlMnGroupIds, _pdsvRecEvent ? _pdsvRecEvent->serial() : 0);

		if (m_stlMNextGroupID.size() > 0)
		{
			BaseState::group_id_add(&stlMnGroupIds, m_stlMNextGroupID);
			m_stlMNextGroupID.clear();// Add by OJ
		}

		DebugOutString(m_pdsvEventSystem, pLink, "PostEvent");
		if (nKey == sm_stlVHash[STATE_DUMMY])
		{// Dummy Link는 Event를 보내지 않는다.
			PT_OFree(m_pdsvEventSystem);
		}
		else {
			//short   nIdentifierNextCnt;
			//const int *pnIdentifierNext;
			if (_pdsvRecEvent && nKey == sm_stlVHash[STATE_ADD]) {
				STLMnInt	stlMnGroupTemp;
				if (group_id_get(_pdsvRecEvent, sm_stlVHash[BaseTransitionNextIdentifier], stlMnGroupTemp))
					BaseState::group_id_add(&stlMnGroupIds, stlMnGroupTemp);
			}

			if (pbExclusive)
			{
				bPosted = _check_exculsive(pMain, m_pdsvEventSystem, pLink, nGoalState, stlMnGroupIds, "Block(Exclusive-by event id");
			}

			if (!bPosted)
			{
				PT_OFree(m_pdsvEventSystem);
			}
			else {
				BaseState* pThis = this;
				INT64 nTargetPoint = (INT64)pThis;
				if(nKey != HASH_STATE(STATE_RELEASE))
					pMain->next_active_state_add((int)nGoalState, stlMnGroupIds);

				//m_pdsvEventSystem->set_alloc(sm_stlVHash[N_KEY], &nKey);
				m_pdsvEventSystem->set_alloc(sm_stlVHash[HASH_BaseTransitionGoal], &nGoalState);
				m_pdsvEventSystem->set_alloc(sm_stlVHash[BaseTransitionGoalMain], &nTargetMain);
				m_pdsvEventSystem->set_alloc(sm_stlVHash[BaseTransitionGoalpState], &nTargetPoint);
				group_id_set(m_pdsvEventSystem, sm_stlVHash[BaseTransitionGoalIdentifier], stlMnGroupIds);
				group_id_set(m_pdsvEventSystem, sm_stlVHash[BaseIdentifierParent_n], m_stlMnGroupIdentifier);
				m_pdsvEventSystem->set_alloc(sm_stlVHash[BaseTransitionCasterKey], &nKeyStateCurrent);

				const int* pnSerialCaster;
				BaseDStructureValue* variable_state = variable_get();
				if (_pdsvRecEvent && STDEF_GET(_pdsvRecEvent, RevStateEventCaster, pnSerialCaster))// To keep caster's serial, it gonna use in the BaseStateEventGlobalReturn_nF
				{	
					BaseDStructureValue* variable_state_next = variable_next_reserve();
					variable_state_next->set_alloc(HASH_STATE(RevStateEventCaster), pnSerialCaster);
				}
				else if (group_id_equal(stlMnGroupIds) == false
					&& STDEF_GET(variable_state, RevStateEventCaster, pnSerialCaster))
				{
					BaseDStructureValue* variable_state_next = variable_next_reserve();
					variable_state_next->set_alloc(HASH_STATE(RevStateEventCaster), pnSerialCaster);
				}

				if (_pdsvRecEvent && STDEF_GET(_pdsvRecEvent, RevNetStateEventCaster, pnSerialCaster))// To keep caster's serial, it gonna use in the BaseNetStateEventReturn_nF
				{
					BaseDStructureValue* variable_state_next = variable_next_reserve();
					variable_state_next->set_alloc(HASH_STATE(RevNetStateEventCaster), pnSerialCaster);
				}
				else if (group_id_equal(stlMnGroupIds) == false
					&& STDEF_GET(variable_state, RevNetStateEventCaster, pnSerialCaster))
				{
					BaseDStructureValue* variable_state_next = variable_next_reserve();
					variable_state_next->set_alloc(HASH_STATE(RevNetStateEventCaster), pnSerialCaster);
				}

				if(createthread == NULL)
					m_pSpace->post_systemevent(m_pdsvEventSystem, 100);
				else{
					int nSerial = obj_serial_get();
					m_pdsvEventSystem->set_alloc(HASH_STATE(BaseTransitionThreadCaster), &nSerial);
					m_pSpace->thread_create_event(m_pdsvEventSystem, 100);
				}
				
				if (nKey == sm_stlVHash[STATE_CALL]
					|| nKey == sm_stlVHash[STATE_RETURN]
					|| nKey == sm_stlVHash[STATE_RELEASE]
					|| nKey == sm_stlVHash[STATE_CHANGE])
				{
					m_bSkipNextLink = true;
					m_bSuspended = true;
				}
			}// end of (!bPosted)
		}
	}

	pLink->reset_link();
	if (bOneTime)
	{
		for (unsigned i = 0; i < m_stlVpLink.size(); i++)
		{
			if (pLink == m_stlVpLink[i])
			{
				m_stlVpLink.erase(m_stlVpLink.begin() + i);
				break;
			}
		}
		PT_OFree(pLink);

		nRet = 1; // add by oj 2010-04-09 link is deleted
	}

	return nRet;
}

void BaseState::state_active_show_all(void)
{
	if (m_pLeafState)
	{
		DebugOutString(NULL, NULL, "OnActiveSuspended");
		return m_pLeafState->state_active_show_all();
	}

	DebugOutString(NULL, NULL, "OnActiveCheck");
}

int BaseState::OnEvent(BaseDStructureValue* _pdsvEvent)
{
	if (m_pLeafState)
		return m_pLeafState->OnEvent(_pdsvEvent);

	//DebugOutString(_pdsvEvent, NULL, "OnEvent");

	int nKey = _pdsvEvent->get_key();

	if (nKey == HASH_STATE(STATE_CALL))
		return StateCall(_pdsvEvent);

	if (nKey == HASH_STATE(STATE_RETURN))
		return StateReturn(_pdsvEvent);

	int nRet = RET_UNPROCESS;
	//==================================================================================================
	// Start Add or Update by OJ 2012-08-01 오전 11:30:11
	// erase because actor event
	//	int nCnt=0;
	//	m_pdsvState->event_process(_pdsvEvent, context_get(), &nCnt);
	//	
	BaseDStructureValue* pLink;

	for (unsigned i = 0; i < m_stlVpLink.size(); i++)
	{
		pLink = m_stlVpLink[i];
		int nProcessCnt = 0;
#ifdef _DEBUG
		char strNameLink[255];
		strcpy_s(strNameLink, 255, pLink->get_name_debug());
		const char* strComment;
		if (pLink->get(STRTOHASH("Comment"), (const void**)& strComment))
			strcat_s(strNameLink, 255, strComment);
#endif

		bool bRet = false;
		if (pLink->event_process(_pdsvEvent, context_get(), &nProcessCnt))
		{
			nRet = RET_PROCESS; bRet = true;
		}

		if (bRet && pLink->is_complete(_pdsvEvent, context_get()))
		{
			m_pSpace->event_affected();// Add by OJ : 2010-10-07 이 이벤트로 인해 State시스템에 변화가 있다
			// Translate State
			m_bSkipNextLink = false;
			if (LinkProcess(i, _pdsvEvent))
				i--;// Add by OJ 2010-04-09 return 값이 true일 경우 Link가 지워진 경우 이다.

			if (m_bSkipNextLink)
			{
				m_bSkipNextLink = false; // Add by OJ : 2018-10-30 It should set false to allow next event process in Space Actor(fast rutine).
					//
				break; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
			}
		}

	}
	// End by OJ 2012-08-01 오전 11:30:18
	//==================================================================================================
	return nRet;
}

int BaseState::StateCall(BaseDStructureValue* _pdsvEvent)
{
	const int* pnGoalState;
	const INT64* pnStatePoint;
	const BaseState* pStateTarget = NULL;
	STLMnInt stlMnGroupId;

	_pdsvEvent->get(HASH_STATE(BaseTransitionGoalpState), (const void**)& pnStatePoint);
	group_id_get(_pdsvEvent, sm_stlVHash[BaseState::BaseTransitionGoalIdentifier], stlMnGroupId);
	_pdsvEvent->get(HASH_STATE(HASH_BaseTransitionGoal), (const void**)& pnGoalState);

	pStateTarget = (BaseState*)* pnStatePoint;
	if (pStateTarget != NULL
		&& pStateTarget != this)
		return BaseState::RET_UNPROCESS;

	//if (group_id_variable(&stlMnGroupId) &&
	//	!group_id_equal(stlMnGroupId))
	//	return BaseState::RET_UNPROCESS;
	const BaseDStructure* pStateMain;
	pStateMain = m_pdsvState->get_base();
	if (!pStateMain->is_exist(*pnGoalState))
		return BaseState::RET_UNPROCESS;

	OnSuspend(_pdsvEvent);

	BaseState* pState = m_pStateMain->GetStateParent(pStateTarget);
	BaseDStructureValue* pdsvContext = NULL, * pdsvVariable = NULL;

	pdsvVariable = pState->variable_next_take();

	PT_OAlloc4(m_pSubState, BaseState, m_pSpace, pStateMain, *pnGoalState, m_pdstLink);
	m_pSubState->obj_serial_set(obj_serial_get());
	m_pSubState->set_parent(this);
	leafStateSet(m_pSubState);
	m_pSubState->set_state_main(m_pStateMain);

	if (pState->group_id_equal(stlMnGroupId))
		pdsvContext = pState->context_get();
	else
	{
		pdsvContext = m_pStateMain->GetContextReserved(stlMnGroupId);

		if (pdsvContext)
		{
			INT32 nTime = BaseSystem::timeGetTime();
			pdsvContext->set_alloc(HASH_STATE(BaseValueTimeStart), &nTime);
		}
	}

	if (pdsvContext) {
		m_pSubState->context_set(pdsvContext);
	}
	else
	{
		m_pSubState->context_create();
		pdsvContext = m_pSubState->context_get();
		if (pdsvContext)
		{
			INT32 nTime = BaseSystem::timeGetTime();
			pdsvContext->set_alloc(HASH_STATE(BaseValueTimeStart), &nTime);
		}
	}
	if (pdsvVariable)
		m_pSubState->variable_set(pdsvVariable);
	m_pSubState->group_id_set(stlMnGroupId);

	m_pSubState->OnStart(_pdsvEvent);

	return BaseState::RET_PROCESS;
}

int BaseState::StateReturn(BaseDStructureValue* _pdsvEvent)
{
	const INT64* pnStatePoint;
	const BaseState* pStateTarget = NULL;
	STLMnInt stlMnGroupId;

	_pdsvEvent->get(HASH_STATE(BaseTransitionGoalpState), (const void**)& pnStatePoint);
	BaseState::group_id_get(_pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
	pStateTarget = (BaseState*)* pnStatePoint;
	if (pStateTarget != NULL
		&& pStateTarget != this)
		return BaseState::RET_UNPROCESS;
	
	//if (group_id_variable(&stlMnGroupId) &&
	//	!group_id_equal(stlMnGroupId))
	//	return BaseState::RET_UNPROCESS;
	if (m_pParentState == NULL)
	{
		g_SendMessage(LOG_MSG_POPUP, "try to return without call in %s", m_strName.c_str());
		return BaseState::RET_UNPROCESS;
	}

	int nKeyStateCurrent = m_pdsvState->get_key();
	OnEnd(_pdsvEvent);
	_pdsvEvent->set_alloc(sm_stlVHash[BaseTransitionCasterKey], &nKeyStateCurrent);

	m_pParentState->OnResume(_pdsvEvent);
	// from here 
	// should not update member variable 
	// case this pointer is removed in OnResume();
	return BaseState::RET_PROCESS;
}

int BaseState::OnEnd(BaseDStructureValue* _pdsvEvent)
{
	if (m_pLeafState)
		m_pLeafState->OnEnd(_pdsvEvent);
	
	for (unsigned i = 0; i < m_stlVpLink.size(); i++)
	{
		int nProcessCnt = 0;
		if (m_stlVpLink[i]->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_END, &nProcessCnt)
			&& m_stlVpLink[i]->is_complete(_pdsvEvent, context_get()))
		{
			if (LinkProcess(i, _pdsvEvent))
				i--;// Add by OJ 2010-04-09 return 값이 true일 경우 Link가 지워진 경우 이다.

			if (m_bSkipNextLink || m_bSuspended)
			{
				m_bSkipNextLink = false;// Add by OJ : 2018-10-30 It should set false to allow next event process in Space Actor(fast rutine).
				//
				break; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
			}
		}
	}// end of for loop

	DebugOutString(_pdsvEvent, NULL, "OnEnd");
	UpdateFuncRelease();

	int nProcessCnt = 0;
	m_pdsvState->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_END, &nProcessCnt);

	PT_OFree(m_pdsvState);
	m_pdsvState = NULL;

	EventSendReset();

	return RET_UNPROCESS;
}

int BaseState::OnResume(BaseDStructureValue* _pdsvEvent)
{
	m_bSkipNextLink = false;
	m_bSuspended = false;
	DebugOutString(_pdsvEvent, NULL, "OnResume");
	int nCnt = 0;
	m_pdsvState->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_RESUME, &nCnt);
	PT_OFree(m_pSubState);
	m_pSubState = NULL;

	leafStateSet(this);

	const void* pValue;
	if (m_pParentState
		&& m_pdsvState->get(sm_stlVHash[HASH_ID_AutoReturnState], &pValue))
	{
		int nKeyStateCurrent = m_pdsvState->get_key();
		
		m_pdsvState->logger_send(BaseDStructureValue::TYPE_Logger_end, get_key_main(), get_key(), obj_serial_get(), 0, group_id_get(), _pdsvEvent ? _pdsvEvent->serial() : 0);
		OnEnd(_pdsvEvent);
		_pdsvEvent->set_alloc(sm_stlVHash[BaseTransitionCasterKey], &nKeyStateCurrent);
		m_pParentState->OnResume(_pdsvEvent);
		m_pParentState = NULL;
	}
	else {
		for (unsigned i = 0; i < m_stlVpLink.size(); i++)
		{
			int nProcessCnt = 0;
			if (m_stlVpLink[i]->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_RESUME, &nProcessCnt)
				&& m_stlVpLink[i]->is_complete(_pdsvEvent, context_get()))
			{
				if (LinkProcess(i, _pdsvEvent))
					i--;// Add by OJ 2010-04-09 return 값이 true일 경우 Link가 지워진 경우 이다.

				if (m_bSkipNextLink || m_bSuspended)
				{
					m_bSkipNextLink = false;// Add by OJ : 2018-10-30 It should set false to allow next event process in Space Actor(fast rutine).
					//
					break; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
				}
			}
		}// end of for loop
	}
	if (m_pParentState
		&& m_pdsvState->get(sm_stlVHash[AutoReturnAfterDoResume], &pValue))
	{
		int nKeyStateCurrent = m_pdsvState->get_key();
		LoggerSend(sm_stlVHash[STATE_RELEASE], m_pdsvState, group_id_get(), _pdsvEvent ? _pdsvEvent->serial() : 0);
		OnEnd(_pdsvEvent);
		_pdsvEvent->set_alloc(sm_stlVHash[BaseTransitionCasterKey], &nKeyStateCurrent);
		m_pParentState->OnResume(_pdsvEvent);
		m_pParentState = NULL;
	}

	return RET_UNPROCESS;
}

int BaseState::OnSuspend(BaseDStructureValue* _pdsvEvent)
{
	//DebugOutString(_pdsvEvent, NULL, "OnSuspend");
	for (unsigned i = 0; i < m_stlVpLink.size(); i++)
	{
		int nProcessCnt = 0;
		if (m_stlVpLink[i]->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_SUSPEND, &nProcessCnt)
			&& m_stlVpLink[i]->is_complete(_pdsvEvent, context_get()))
		{
			if (LinkProcess(i, _pdsvEvent))
				i--;// Add by OJ 2010-04-09 return 값이 true일 경우 Link가 지워진 경우 이다.

			if (m_bSkipNextLink || m_bSuspended)
			{
				m_bSkipNextLink = false;// Add by OJ : 2018-10-30 It should set false to allow next event process in Space Actor(fast rutine).
				//
				break; // Add by OJ : 2010-05-12 BaseExclusiveLink일경우 다른 링크는 체크하지 않는다
			}
		}
	}// end of for loop

	int nCnt = 0;
	m_pdsvState->event_process(_pdsvEvent, context_get(), BaseDStructure::STATE_SUSPEND, &nCnt);
	return RET_UNPROCESS;
}

void BaseState::identifier_set(int _nIdentifier, int _nGroupId)
{
	m_stlMnGroupIdentifier.clear();
	if (_nGroupId == 0)
		_nGroupId = HASH_STATE(None);
	m_stlMnGroupIdentifier[_nGroupId] = _nIdentifier;
}

int  BaseState::identifier_get(int _nGroupId) const
{
	if (_nGroupId == 0)
		_nGroupId = HASH_STATE(None);

	STLMnInt::const_iterator it = m_stlMnGroupIdentifier.find(_nGroupId);
	if (it != m_stlMnGroupIdentifier.end())
		return it->second;

	return 0;
}

bool BaseState::key_is_engaged(int _nKey) const
{
	if (m_pdsvState->get_key() == _nKey)
		return true;

	if (m_pSubState)
		if (m_pSubState->key_is_engaged(_nKey))
			return true;

	if (m_pParentState)
		if (m_pParentState->key_is_engaged_parent(_nKey))
			return true;

	return false;
}

bool BaseState::key_is_engaged_parent(int _nKey) const
{
	if (m_pdsvState->get_key() == _nKey)
		return true;

	if (m_pParentState)
		if (m_pParentState->key_is_engaged_parent(_nKey))
			return true;

	return false;
}

int	BaseState::get_key() const
{
	//if(m_pSubState)
	//	return m_pSubState->get_key();

	return m_pdsvState->get_key();
}

int	BaseState::get_key_sub() const
{
	if (m_pLeafState)
		return m_pLeafState->get_key_sub();

	return m_pdsvState->get_key();
}

int BaseState::get_serial() const
{
	if (m_pLeafState)
		return m_pLeafState->get_serial();

	return obj_serial_get();
}

BaseDStructure* BaseState::get_dstEvent()
{
	return m_pSpace->EnumGet(sm_stlVHash[HASH_EnumEvent]);
}

int	BaseState::get_key_main()
{
	if (m_pdsvState == NULL
		|| m_pdsvState->get_base() == NULL)
		return 0;

	return m_pdsvState->get_base()->get_key();
}

bool BaseState::VariableSet(const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvContext, BaseDStructureValue* _pdsvEvent, BaseDStructureValue* _pdsvDefault, const void* _pParam, int _seq_int, short _nCnt)
{
	BaseDStructureValue* pdsvVariable = _pdsvDefault;
	if (pdsvVariable == NULL)
	{
		if ((BaseState*)_pdsvBase->m_pVoidParam)
			pdsvVariable = ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();
		else
			pdsvVariable = _pdsvContext;
	}

	int nHash;
	for (int i = _pdsvBase->sequence_get() + _seq_int + 1; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if (nHash == HASH_STATE(BaseVariableReferType_nV))
		{
			STDEF_Manager(pManager);
			const int* pnHash;
			if (!_pdsvBase->get((const void**)& pnHash, NULL, i))
				break;

			pdsvVariable = pManager->variable_type(*pnHash, _pdsvBase, _pdsvContext, _pdsvEvent);
		}
		else if (nHash == HASH_STATE(BaseVariableRefRemove_nV))
		{
			pdsvVariable->set_alloc(nHash, NULL);
		}
		else if (nHash == HASH_STATE(BaseVariableRefer_anV))
		{
			const int* pnHash;
			if (!_pdsvBase->get((const void**)& pnHash, NULL, i))
				break;

			pdsvVariable->set_alloc(*pnHash, _pParam, _nCnt);
			return true;
		}
		else {
			break;
		}
	}
	return false;
}

bool BaseState::VariableTypeGet(const BaseDStructureValue *_pdsvBase, int _nSeq, EDstType *_type)
{
	int nSeqLocal = 0;
	int nHash;
	for (int i = _pdsvBase->sequence_get() + 1; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if(_type)
			*_type = _pdsvBase->get_base()->get_type(nIndex);
		
		if (nHash == HASH_STATE(BaseVariableReferType_nV))
		{
		}
		else if (nHash == HASH_STATE(BaseVariableRefRemove_nV))
		{
		}
		else if (nHash == HASH_STATE(BaseVariableRefer_anV))
		{
			const int* pnHash;
			if (!_pdsvBase->get((const void**)& pnHash, NULL, i))
				break;

			nIndex = _pdsvBase->get_index(*pnHash);
			if(_type)
				*_type = _pdsvBase->get_base()->get_type(nIndex);
			
			if (_nSeq == nSeqLocal)
				return true;
			nSeqLocal++;
		}
		else if (nHash == HASH_STATE(BaseVariableConst_strV)
			|| nHash == HASH_STATE(BaseVariableConst_nV)
			|| nHash == HASH_STATE(BaseVariableConst_fV))
		{
			if (_nSeq == nSeqLocal)
				return true;
			nSeqLocal++;
		}
		else {
			break;
		}
	}
	return false;
}

const void* BaseState::VariableGet(const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvContext, BaseDStructureValue* _pdsvEvent, int _nSeq, BaseDStructureValue* _pdsvDefault, int _seq_inc, EDstType *_type)
{
	BaseDStructureValue* pdsvVariable = _pdsvDefault;
	if (pdsvVariable == NULL)
	{
		if ((BaseState*)_pdsvBase->m_pVoidParam)
			pdsvVariable = ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();
		else
			pdsvVariable = _pdsvContext;
	}

	int nSeqLocal = 0;
	int nHash;
	for (int i = _pdsvBase->sequence_get() + 1 + _seq_inc; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if(_type)
			*_type = _pdsvBase->get_base()->get_type(nIndex);
		
		if (nHash == HASH_STATE(BaseVariableReferType_nV))
		{
			const int* pnHash;
			if (!_pdsvBase->get((const void**)& pnHash, NULL, i))
				break;

			STDEF_Manager(pManager);
			pdsvVariable = pManager->variable_type(*pnHash, _pdsvBase, _pdsvContext, _pdsvEvent);
		}
		else if (nHash == HASH_STATE(BaseVariableRefRemove_nV))
		{
			pdsvVariable->set_alloc(nHash, NULL);
		}
		else if (nHash == HASH_STATE(BaseVariableRefer_anV))
		{
			const int* pnHash;
			if (!_pdsvBase->get((const void**)& pnHash, NULL, i))
				break;

			nIndex = _pdsvBase->get_index(*pnHash);
			if(_type)
				*_type = _pdsvBase->get_base()->get_type(nIndex);
			
			const char* strValue;
			if (pdsvVariable->get(*pnHash, (const void**)& strValue))
				if (_nSeq == nSeqLocal)
					return (const void*)strValue;
			nSeqLocal++;
		}
		else if (nHash == HASH_STATE(BaseVariableConst_strV)
			|| nHash == HASH_STATE(BaseVariableConst_nV)
			|| nHash == HASH_STATE(BaseVariableConst_fV))
		{
			const char* strValue;
			if (!_pdsvBase->get((const void**)& strValue, NULL, i))
				break;

			if (_nSeq == nSeqLocal)
				return (const void*)strValue;
			nSeqLocal++;
		}
		else {
			break;
		}
	}
	return NULL;
}

int _formatGet(const char* _strIn, int* _pnType, char* _strOut) // the _strIn should start by '%'
{
	if (*_strIn != '%')
		return 0;
	int nCnt = 1;

	*_pnType = 0;
	*_strOut = *_strIn;
	_strOut++;

	do {
		switch (*(++_strIn)) {
		case 'd':			*_pnType = 1;			nCnt++;			break;
		case 'f':			*_pnType = 2;			nCnt++;			break;
		case 's':			*_pnType = 3;			nCnt++;			break;
		case '%':			*_pnType = 4;			nCnt++;			break;
		default:
			nCnt++;
			*_strOut = *_strIn;
			_strOut++;
			break;
		}
	} while (*_pnType == 0);

	*_strOut = *_strIn;
	_strOut++;
	*_strOut = 0;

	return nCnt;
}

int* reserved_value(const BaseDStructureValue* _pdsvBase, int _nHash, int* _pnType)
{
	static int anHash[10] = { 0,0 };
	if (anHash[0] == 0)
	{
		//		anHash[0]	= STRTOHASH("RevStateSerial");
		anHash[1] = STRTOHASH("RevIdentify");
		anHash[2] = STRTOHASH("RevDateCurrent");
		anHash[3] = STRTOHASH("RevDatetimeCurrent");
	}
	static int nRet;
	int* pnRet = NULL;
	//if(_nHash == anHash[0])// RevStateSerial
	//{
	//	STDEF_BaseState(pState);
	//	nRet = (int)pState->TempValueIntGet(HASH_STATE(SelectorSerial_nV));
	//	pnRet = &nRet;
	//}else 
	if (_nHash == anHash[1])// RevIdentify
	{
		const int* pnGroupId;
		STDEF_BaseState(pState);
		if (STDEF_FGET(_pdsvBase, BaseState::BaseStateGroupId_nV, pnGroupId))
			nRet = pState->identifier_get(*pnGroupId);
		else
			nRet = pState->identifier_get(HASH_STATE(None));
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == HASH_STATE(RevTimeCurrentYear)) {
		SPtDateTime	tmCur;	BaseSystem::timeCurrent(&tmCur); nRet = tmCur.s.sDate.s.year;
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == HASH_STATE(RevTimeCurrentMonth)) {
		SPtDateTime	tmCur;	BaseSystem::timeCurrent(&tmCur); nRet = tmCur.s.sDate.s.month;
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == HASH_STATE(RevRandomDigit6)) {
		char* strTempString = PT_Alloc(char, 10);

		BaseSystem::srand_(BaseSystem::timeGetTime());
		for (int i = 0; i < 6; i++)
		{
			strTempString[i] = '0' + BaseSystem::rand() % 10;
		}
		strTempString[6] = 0;
		pnRet = (int*)strTempString;
		*_pnType = TYPE_STRING;

		STDEF_Manager(pManager);
		pManager->resource_release_regist(10000, (void*)strTempString);// it will be released after 10 sec.
	}
	else if (_nHash == HASH_STATE(RevTimeCurrentDay)) {
		SPtDateTime	tmCur;	BaseSystem::timeCurrent(&tmCur); nRet = tmCur.s.sDate.s.day;
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == HASH_STATE(RevTimeCurrentHour)) {
		SPtDateTime	tmCur;	BaseSystem::timeCurrent(&tmCur); nRet = tmCur.s.sTime.s.nHour;
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == HASH_STATE(RevTimeCurrentMinute)) {
		SPtDateTime	tmCur;	BaseSystem::timeCurrent(&tmCur); nRet = tmCur.s.sTime.s.nMinute;
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == HASH_STATE(RevTimeCurrentSecond)) {
		SPtDateTime	tmCur;	BaseSystem::timeCurrent(&tmCur); nRet = tmCur.s.sTime.s.nSecond;
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == HASH_STATE(RevStateSerial)) {
		STDEF_BaseState(pState);
		nRet = pState->obj_serial_get();
		pnRet = &nRet;
		*_pnType = TYPE_INT32;
	}
	else if (_nHash == anHash[2]) {// RevDateCurrent
		SPtDateTime	tmCur;
		BaseSystem::timeCurrent(&tmCur);
		static char strDate[20];
		BaseTime::make_date(tmCur.s.sDate, strDate, 20);
		pnRet = (int*)strDate;
		*_pnType = TYPE_STRING;
	}
	else if (_nHash == anHash[3]) { //RevDatetimeCurrent
		SPtDateTime	tmCur;
		BaseSystem::timeCurrent(&tmCur);
		static char strDate[20];
		//printf("Time: %d, %d, %d\n", tmCur.s.sTime.s.nHour, tmCur.s.sTime.s.nMinute, tmCur.s.sTime.s.nSecond);
		BaseTime::make_date(tmCur, strDate, 20);
		//printf("TimeResult: %s %d\n", strDate, strlen(strDate));
		pnRet = (int*)strDate;
		*_pnType = TYPE_STRING;
	}
	return pnRet;
}

char* BaseState::VariableStringMake(const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvContext, BaseDStructureValue* _pdsvEvent, char* _strFormat, int _nLen)
{
	BaseDStructureValue* pdsvVariable = ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();

	if (pdsvVariable == NULL)
		return _strFormat;

	STLVInt	stlVnParam;
	STLVString	stlVstrParam;

	int nHash;
	for (int i = _pdsvBase->sequence_get() + 1; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		//int nIndex2;
		//nIndex = _pdsvBase->get_index(nHash);
		//if (nHash == STRTOHASH("BaseStateSelectorSerial_nV"))
		//{
		//	nValue = BaseStateManager::serial_num_get();
		//	stlVnParam.push_back(nValue);
		//}
		//else 
		if (nHash == HASH_STATE(BaseVariableReferType_nV))
		{
			const int* pnHash;
			if (!_pdsvBase->get((const void**)& pnHash, NULL, i))
				break;

			STDEF_Manager(pManager);
			pdsvVariable = pManager->variable_type(*pnHash, _pdsvBase, _pdsvContext, _pdsvEvent);
		}
		else if (nHash == HASH_STATE(BaseVariableRefer_anV))
		{
			const int* pnHash;
			if (!_pdsvBase->get((const void**)& pnHash, NULL, i))
				break;
			const int* pnValue;
			int nIndex = pdsvVariable->get_base()->get_index(*pnHash);
			if (nIndex >= 0 && pdsvVariable->get_base()->get_type(nIndex) == TYPE_STRING)
			{
				if (pdsvVariable->get(*pnHash, (const void**)& pnValue))
					stlVstrParam.push_back((const char*)pnValue);
				else
					stlVstrParam.push_back((const char*)"*Fail*");
			}
			else
			{
				if (pdsvVariable->get(*pnHash, (const void**)& pnValue))
					stlVnParam.push_back(*pnValue);
				else {
					int nInt;
					pnValue = reserved_value(_pdsvBase, *pnHash, &nInt);
					if (nInt == TYPE_STRING) {
						stlVstrParam.push_back((const char*)pnValue);
					}
					else if (pnValue != NULL) {
						stlVnParam.push_back(*pnValue);
					}
					else {
						stlVstrParam.push_back((const char*)"*Fail*");
						stlVnParam.push_back(-999999);
					}
				}
			}
		}
		else if (nHash == HASH_STATE(BaseVariableRefRemove_nV))
		{
			pdsvVariable->set_alloc(nHash, NULL);
		}
		else if (nHash == HASH_STATE(BaseVariableRefer_strV)
				|| nHash == HASH_STATE(BaseVariableString_strV))
		{
			const char* strHash;
			if (!_pdsvBase->get((const void**)& strHash, NULL, i))
				break;

			const char* strValue;
			if (pdsvVariable->get(STRTOHASH(strHash), (const void**)& strValue))
				stlVstrParam.push_back(strValue);
		}
		else {
			break;
		}

		if (pdsvVariable == NULL)
			break;
	}

	char* strBuffer = PT_Alloc(char, _nLen)
		, * strTemp = PT_Alloc(char, _nLen);

	strcpy_s(strBuffer, _nLen, _strFormat);

	int nFormatLen = (int)strlen(strBuffer);
	_strFormat[0] = 0;

	int nNCnt = 0, nStrCnt = 0;
	int nCnt = 0, nFCnt = 0;
	while (nCnt < nFormatLen) {
		if (strBuffer[nCnt] == '\\')
		{
			nCnt++;
			_strFormat[nFCnt++] = strBuffer[nCnt]; _strFormat[nFCnt] = 0;
			nCnt++;
		}
		else if (strBuffer[nCnt] == '%')
		{
			int nLenFormat = 0, nType = 0;
			char strFormat[100];
			nLenFormat = _formatGet(&strBuffer[nCnt], &nType, strFormat);

			switch (nType) {
			case 0:
				PT_Free(strBuffer);
				PT_Free(strTemp);
				return NULL;
			case 1:
				if ((int)stlVnParam.size() <= nNCnt) {
					PT_Free(strBuffer);
					PT_Free(strTemp);
					return NULL;
				}
				sprintf_s(strTemp, _nLen, strFormat, stlVnParam[nNCnt++]);
				strcat_s(_strFormat, _nLen, strTemp);
				nFCnt += (int)strlen(strTemp);
				break;
			case 2:
				if ((int)stlVnParam.size() <= nNCnt) {
					PT_Free(strBuffer);
					PT_Free(strTemp);
					return NULL;
				}
				sprintf_s(strTemp, _nLen, strFormat, (float)stlVnParam[nNCnt++] * 0.01f);
				strcat_s(_strFormat, _nLen, strTemp);
				nFCnt += (int)strlen(strTemp);
				break;
			case 3:
				if ((int)stlVstrParam.size() <= nStrCnt) {
					PT_Free(strBuffer);
					PT_Free(strTemp);
					return NULL;
				}
				strcat_s(_strFormat, _nLen, stlVstrParam[nStrCnt].c_str());
				nFCnt += (int)stlVstrParam[nStrCnt++].length();
				break;
			case 4:
				_strFormat[nFCnt++] = strBuffer[nCnt]; _strFormat[nFCnt] = 0;
				break;
			}
			nCnt += nLenFormat;
		}
		else {
			_strFormat[nFCnt++] = strBuffer[nCnt]; _strFormat[nFCnt] = 0;
			nCnt++;
		}
	}

	PT_Free(strBuffer);
	PT_Free(strTemp);

	return _strFormat;
}

/*
void BaseState::event_state_attach_param(BaseState *_pState, const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, STLVPairIntInt *_pstlVParamList)
{
	int nHash, nValue;
	for(int i = _pdsvBase->sequence_get()+1; i<_pdsvBase->get_count(); i++)
	{
		nHash	= _pdsvBase->get_colum(i);
		int nIndex;
		nIndex	= _pdsvBase->get_index(nHash);
		if((_pdsvBase->get_base()->get_type_flag(nIndex) & VALUE_CONTEXT) != 0)
		{
			const int *pnValue;
			if(_pdsvContext->get(nHash, (const void**)&pnValue))
				_pstlVParamList->push_back(std::pair<int, int>(nHash, *pnValue));
		}else if(nHash == STRTOHASH("BaseStateSelectorSerial_nV"))
		{
			nValue = BaseStateManager::serial_num_get();
			_pstlVParamList->push_back(std::pair<int, int>(HASH_STATE(SelectorSerial_nV), nValue));
		}else if(nHash == STRTOHASH("BaseStateParamInt_nV"))
		{
			if(!_pdsvBase->get(&nValue, NULL, i))
				break;
			_pstlVParamList->push_back(std::pair<int, int>(HASH_STATE(ParamInt), nValue));
		}else if(nHash == STRTOHASH("BaseStateParamTemp_nV")){
			if(!_pdsvBase->get(&nValue, NULL, i))
				break;
			INT64 n64Param = (INT64)_pState->TempValueGet(nValue);
			int nParam =(int)n64Param;
			_pstlVParamList->push_back(std::pair<int, int>(nValue, nParam));
		}else if(nHash == HASH_STATE(BaseVariableRefer_anV))
		{
			const int *pnHash;
			if(!_pdsvBase->get((const void**)&pnHash, NULL, i))
				break;
			const int *pnValue;
			if(_pdsvContext->get(*pnHash, (const void**)&pnValue))
				_pstlVParamList->push_back(std::pair<int, int>(*pnHash, *pnValue));
		}else{
			break;
		}
	}

	BaseDStructureValue *pEvent = _pState->EventSendGet();
	for(int i = 0; i<pEvent->get_count(); i++)
	{
		nHash	= pEvent->get_colum(i);
		if(nHash == HASH_STATE(N_KEY)
			|| nHash == HASH_STATE(HASH_BaseStateEventGlobal)
			|| nHash == HASH_STATE(HASH_BaseTransitionGoalMain)
			|| nHash == HASH_STATE(HASH_BaseTransitionGoalIdentifier))
			continue;
		int nIndex;
		nIndex	= pEvent->get_index(nHash);
		if(pEvent->get_base()->get_type(nIndex) != TYPE_INT32)
			continue;

		const int *pnValue;
		if(pEvent->get((const void**)&pnValue, NULL, i))
			_pstlVParamList->push_back(std::pair<int, int>(nHash, *pnValue));
	}
}
//*/

#define LOG_MAX 512

int BaseState::DebugOutString(const BaseDStructureValue* _pdst, const BaseDStructureValue* _pdstLink, const char* _strState)
{
	int nRet = 0;
	//*
#ifdef TRACE_STATE
	if (m_pSpace->get_manager()->m_nDebugBlockCastMessage != 0)
		return nRet;

	const int* pValue;
	if (m_pdsvState->get(HASH_STATE(BaseDebugHideLog), (const void**)&pValue))
		return nRet;

	char strBuffer[LOG_MAX], strTemp[LOG_MAX];
	strBuffer[0] = NULL;

	if (m_pdsvState && m_pdsvState->get_base())
	{
		strcpy_s(strTemp, LOG_MAX, m_pdsvState->get_base()->get_filename());
		strcat_s(strBuffer, LOG_MAX, "SMain:");
		strcat_s(strBuffer, LOG_MAX, strTemp);
	}

	if (_strState)
	{
		char strBuf[LOG_MAX];
		STLMnInt::const_iterator it = m_stlMnGroupIdentifier.begin();
		for (; it != m_stlMnGroupIdentifier.end(); it++) {
			sprintf_s(strBuf, LOG_MAX, " GID:%d S:%d", it->first, it->second);
			strcat_s(strBuffer, LOG_MAX, strBuf);
		}
	}

	const char* strName;
	if (m_pdsvState->get(sm_stlVHash[STR_NAME], (const void**)&strName))
	{
		strcat_s(strBuffer, LOG_MAX, " SActive:");
		strcat_s(strBuffer, LOG_MAX, strName);
	}

	if (_strState)
	{
		strcat_s(strBuffer, LOG_MAX, " SAction:");
		strcat_s(strBuffer, LOG_MAX, _strState);
	}
	
	sprintf_s(strTemp, LOG_MAX, " Serial:%d", get_serial());
	strcat_s(strBuffer, LOG_MAX, strTemp);

	const char* strEvent;
	if (_pdst && _pdst->get(sm_stlVHash[STR_NAME], (const void**)& strEvent))
	{
		strcat_s(strBuffer, LOG_MAX, " SEvent:");
		strcat_s(strBuffer, LOG_MAX, strEvent);
	}

	if (_pdstLink && _pdstLink->get(HASH_STATE(BaseDebugHideLog), (const void**)& pValue))
		return nRet;

	if (_pdstLink && _pdstLink->get(sm_stlVHash[STR_NAME], (const void**)& strName))
	{
		strcat_s(strBuffer, LOG_MAX, " SLink:");
		strcat_s(strBuffer, LOG_MAX, strName);
	}
	if (_pdstLink && _pdstLink->get(sm_stlVHash[HASH_Comment], (const void**)& strName))
	{
		strcat_s(strBuffer, LOG_MAX, "(");
		strcat_s(strBuffer, LOG_MAX, strName);
		strcat_s(strBuffer, LOG_MAX, ")");
	}

	strcat_s(strBuffer, LOG_MAX, "\r\n");
	nRet = g_SendMessage(LOG_MSG, strBuffer);

	//static BaseFile file;

	//static bool bFirst = true;

	//if(bFirst)
	//{
	//	file.OpenFile("log.txt", BaseFile::OPEN_WRITE);
	//	bFirst	= false;
	//}
	//file.write_asc_string(strBuffer, 255);
	//file.write_asc_line();
	//OutputDebugStringA(strBuffer);
	//printf(strBuffer);
#endif
	//*/
	return nRet;
}

//BaseStateManager *BaseState::get_manager()
//{
//	return (BaseStateManager*)m_pManager;
//}

BaseStateSpace* BaseState::get_space()
{
	return m_pSpace;
}

STDEF_FUNC(BaseKeyPushed_nIf)
{
	const int* pnKey;
	if (_pdsvBase->get(HASH_STATE(HASH_BaseKeyPushed_nIf), (const void**)& pnKey))
	{
		STDEF_Space(pSpace);

		BaseDStructure* pdstEnumKey;
		pdstEnumKey = pSpace->EnumGet(HASH_STATE(HASH_EnumEventKey));

		const int* pnScanCodeLink;
		if (pdstEnumKey->get_by_hash(*pnKey, HASH_STATE(HASH_nScanCode), (const void**)& pnScanCodeLink))
		{
			if (pSpace->key_is_pushed(*pnScanCodeLink))
				return 1;
		}
	}
	return 0;
}

STDEF_FUNC(BaseKeyPushed_nUIf)
{
	const int* pnKey;
	if (_pdsvBase->get(HASH_STATE(HASH_BaseKeyPushed_nUIf), (const void**)& pnKey))
	{
		STDEF_Space(pSpace);

		BaseDStructure* pdstEnumKey;
		pdstEnumKey = pSpace->EnumGet(HASH_STATE(HASH_EnumEventKey));

		const int* pnScanCodeLink;
		if (pdstEnumKey->get_by_hash(*pnKey, HASH_STATE(HASH_nScanCode), (const void**)& pnScanCodeLink))
		{
			if (pSpace->key_is_pushed(*pnScanCodeLink))
				return 1;
		}
	}
	return 0;
}

STDEF_FUNC(BaseKeyReleased_nIf)
{
	const int* pnKey;
	if (_pdsvBase->get(HASH_STATE(HASH_BaseKeyReleased_nIf), (const void**)& pnKey))
	{
		STDEF_Space(pSpace);

		BaseDStructure* pdstEnumKey;
		pdstEnumKey = pSpace->EnumGet(HASH_STATE(HASH_EnumEventKey));

		const int* pnScanCodeLink;
		if (pdstEnumKey->get_by_hash(*pnKey, HASH_STATE(HASH_nScanCode), (const void**)& pnScanCodeLink))
			if (pSpace->key_is_pushed(*pnScanCodeLink) == false)
				return 1;
	}
	return 0;
}

STDEF_FUNC(BaseSleep_nF)
{
	const int *milisec;
	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseSleep_nF"), (const void**)&milisec))
		return 0;

	BaseSystem::Sleep(*milisec);
	return 1;
}

STDEF_FUNC(BaseTimeOut_nREU)
{
	if (_pdsvBase == NULL)
		return 0;

	return 1;
}

STDEF_FUNC(BaseTimeOut_varUIf)
{
	if (_pdsvBase == NULL)
		return 0;
	return 1;
}

BaseDStructureValue* BaseState::variable_next_take()
{
	if (m_pLeafState)
		return m_pLeafState->variable_next_take();

	if (m_pdsvVariable == NULL
		&& m_pdsvVariableNextReserve == NULL)
		return NULL;

	BaseDStructureValue* pdsv = variable_next_reserve();
	
	m_pdsvVariableNextReserve = NULL;

	if (m_pdsvVariable){
		m_pdsvVariable->variable_transit(pdsv);
		m_pdsvVariable->sfunc_transit(pdsv);
	}
	return pdsv;
}

BaseDStructureValue* BaseState::variable_next_reserve()
{
	if (m_pdsvVariableNextReserve == NULL)
		PT_OAlloc2(m_pdsvVariableNextReserve, BaseDStructureValue, m_pdsvState->get_base(), 1024);

	return m_pdsvVariableNextReserve;
}

BaseDStructureValue* BaseState::context_reserve(const STLMnInt& _stlMnGroupId)
{
	BaseDStructureValue* pdsvContextReserve;
	pdsvContextReserve = m_pStateMain->GetContextReservedRef(_stlMnGroupId);
	if (pdsvContextReserve)
		return pdsvContextReserve;

	PT_OAlloc2(pdsvContextReserve, BaseDStructureValue, m_pdsvState->get_base(), 1024);
	pdsvContextReserve->m_pVoidParam = NULL;
	pdsvContextReserve->set_key(HASH_STATE(STATE_CONTEXT));

	m_pStateMain->AddContextReserved(_stlMnGroupId, pdsvContextReserve);
	return pdsvContextReserve;
}

BaseDStructureValue* BaseState::context_create()
{
	B_ASSERT(!m_pdsvContext);

	if (m_pdsvContext)
		m_pdsvContext->ref_dec();

	PT_OAlloc2(m_pdsvContext, BaseDStructureValue, m_pdsvState->get_base(), 1024);

	int nKey = sm_stlVHash[STATE_CONTEXT];
	m_pdsvContext->m_pVoidParam = this;
	if (!m_pdsvContext->get_base()->is_exist(nKey))
	{
		g_SendMessage(LOG_MSG_POPUP, "%s should have 'state_context' state", m_strName.c_str());
		B_ASSERT(m_pdsvContext->get_base()->is_exist(nKey));
	
		PT_OFree(m_pdsvContext);
		m_pdsvContext = NULL;
		return NULL;
	}

	m_pdsvContext->ref_inc();
	m_pdsvContext->set_key(nKey);
	return m_pdsvContext;
}

void BaseState::context_set(BaseDStructureValue* _pdsvContext)
{
	if (_pdsvContext == NULL)
		return;
	B_ASSERT(!m_pdsvContext);

	if (m_pdsvContext)
		m_pdsvContext->ref_dec();

	m_pdsvContext = _pdsvContext;
	if (m_pdsvContext->m_pVoidParam == NULL)
		m_pdsvContext->m_pVoidParam = this;

	m_pdsvContext->ref_inc();
}

BaseDStructureValue* BaseState::context_get()
{
	if (m_pParentState)
		return m_pParentState->context_get();
	return m_pdsvContext;
}

const BaseState* BaseState::get_sub_state()const
{
	return m_pSubState;
}

STDEF_FUNC(BaseKeyPush_nRE)
{
	const int* pnScanCode;
	if (_pdsvEvent->get(HASH_STATE(HASH_nScanCode), (const void**)& pnScanCode))
	{
		const int* pnEnumKeyLink;
		const int* pnScanCodeLink;
		if (_pdsvBase->get(HASH_STATE(HASH_BaseKeyPush_nRE), (const void**)& pnEnumKeyLink))
		{
			STDEF_Space(pSpace);

			BaseDStructure* pdstEnumKey;
			pdstEnumKey = pSpace->EnumGet(HASH_STATE(HASH_EnumEventKey));

			if (pdstEnumKey->get_by_hash(*pnEnumKeyLink, HASH_STATE(HASH_nScanCode), (const void**)& pnScanCodeLink))
			{
				if (*pnScanCodeLink == *pnScanCode)
					return 1;
			}
		}
	}
	return 0;
}

STDEF_FUNC(BaseKeyRelease_nRE)
{
	const int* pnScanCode;
	if (_pdsvEvent->get(HASH_STATE(HASH_nScanCode), (const void**)& pnScanCode))
	{
		const int* pnEnumKeyLink;
		const int* pnScanCodeLink;
		if (_pdsvBase->get(HASH_STATE(HASH_BaseKeyRelease_nRE), (const void**)& pnEnumKeyLink))
		{
			STDEF_Space(pSpace);

			BaseDStructure* pdstEnumKey;
			pdstEnumKey = pSpace->EnumGet(HASH_STATE(HASH_EnumEventKey));

			if (pdstEnumKey->get_by_hash(*pnEnumKeyLink, HASH_STATE(HASH_nScanCode), (const void**)& pnScanCodeLink))
			{
				if (*pnScanCodeLink == *pnScanCode)
					return 1;
			}
		}
	}
	return 0;
}

STDEF_FUNC(BaseValueCheckContext_bF)
{
	bbyte nCompare;
	if (!_pdsvBase->get("BaseValueCheckContext_bF", &nCompare))
		return 0;

	const char* strFilename;
	if (_pdsvBase->get(STRTOHASH("BaseValueFileLoad_strV"), (const void**)& strFilename))
	{
		_pdsvContext->set_alloc(STRTOHASH("ContextFileSaveName_strV"), strFilename);
		BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextLoad);
	}

	int	nRet = 1;
	int	nHash;
	bool	bFound = false;
	for (int i = 0; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if ((_pdsvBase->get_base()->get_type_flag(nIndex) & BaseDStructure::VALUE_CONTEXT) != 0)
		{
			const void* pVoid;
			const void* pVoid2;
			short nCount = 0;
			short nCount2 = 0;
			short nSize = 0;

			nSize = _pdsvBase->get_base()->get_type_size(nIndex);

			if (_pdsvBase->get(nHash, &pVoid, &nCount))
				if (_pdsvContext->get(nHash, &pVoid2, &nCount2))
				{
					bFound = true;
					if (memcmp(pVoid, pVoid2, nCount * nSize) != 0)
					{
						if (nCompare == 1)
							return 0;
					}
					else {
						if (nCompare == 2)
							return 0;
					}
				}
		}
	}

	if (nCompare == 3)
	{
		if (!bFound)
			return 1;
		else
			return 0;
	}

	return nRet;
}

bool BaseState::context_value_int(const BaseDStructureValue* _pdsvRef, const BaseDStructureValue* _pdsvValue, int _nSequence, const void** _ppnValue, int* _pnHashOut)
{
	int nHash = _pdsvRef->get_colum(_nSequence);
	if (nHash == 0)
		return false;

	if (_pnHashOut)
		* _pnHashOut = nHash;

	int nIndex;
	nIndex = _pdsvRef->get_index(nHash);
	if ((_pdsvRef->get_base()->get_type_flag(nIndex) & BaseDStructure::VALUE_CONTEXT) != 0)
	{
		if (_pdsvValue->get(nHash, _ppnValue))
			return true;
	}
	return false;
}

STDEF_FUNC(BaseValueEventParamIntToContext_nF)
{
	const int* pnValue;
	int i, nHash, nCnt = 0;
	i = _pdsvBase->sequence_get();
	i++;	// skip current colum, cause it's Context function.

	for (; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if ((_pdsvBase->get_base()->get_type_flag(nIndex) & BaseDStructure::VALUE_CONTEXT) != 0)
		{
			if (_pdsvEvent->get(nHash, (const void**)& pnValue))
			{
				_pdsvContext->set_alloc(nHash, pnValue);
			}
			//else if(STDEF_GETS(_pdsvEvent, ParamInt, pnValue, nCnt))
			//{
			//	_pdsvContext->set_alloc(nHash, pnValue);
			//}
			else {
				return 0;
			}
			nCnt++;
		}
		else { // if current colum is not a Context, finish to serach.
			break;
		}
	}

	return 1;
}

STDEF_FUNC(BaseValueToContext_bF)
{
	return BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextSet);
}

void BaseState::set_state_main(BaseStateMain* _pMain)
{
	m_pStateMain = _pMain;
}

BaseStateMain* BaseState::get_state_main()
{
	return m_pStateMain;
}

int BaseState::make_candidate_list(STLVpState* _stlVpState, STLMnInt* _pstlMnHash)
{
	int nHash = m_pdsvState->get_key();

	STLMnInt::iterator it = _pstlMnHash->find(nHash);
	if (it != _pstlMnHash->end())
		return 0;

	(*_pstlMnHash)[nHash] = nHash;

	for (unsigned i = 0; i < m_stlVpLink.size(); i++)
	{
		nHash = get_target_state_key(m_stlVpLink[i]);

		if (nHash == 0)
			continue;

		if (m_stlVpLink[i]->is_get_flag(BaseDStructure::LINK_CONFORM))
		{
			BaseState* pState;
			PT_OAlloc4(pState, BaseState, m_pSpace, m_pdsvState->get_base(), nHash, m_pdstLink);

			_stlVpState->push_back(pState);
		}
		else {
			BaseState* pState;
			PT_OAlloc4(pState, BaseState, m_pSpace, m_pdsvState->get_base(), nHash, m_pdstLink);

			pState->make_candidate_list(_stlVpState, _pstlMnHash);
			PT_OFree(pState);
		}
	}
	return (int)_stlVpState->size();
}

int	BaseState::get_target_state_key(BaseDStructureValue* _pLink)// Link가 지정하는 키를 리턴한다.
{
	const char* strNameNext;

	int	nRetState = 0;
	if (_pLink->get(HASH_STATE(STR_NAME), (const void**)& strNameNext))
		nRetState = STRTOHASH(strNameNext);

	return nRetState;
}
//
//STDEF_FUNC(BaseStateNoactiveSerialCheck_nF)
//{
//	const int *pnState;
//
//	STDEF_GETLOCAL_R(_pdsvBase, BaseStateNoactiveSerialCheck_nF, pnState);
//	int nSerial;
//
//	BaseState *pState = (BaseState*)_pdsvBase->m_pVoidParam;
//    INT64 n64 = (INT64)pState->TempValueGet(HASH_STATE(SelectorSerial_nV));
//	nSerial = (int)n64;
//	BaseStateMain	*pMain;
//	pMain	= pState->get_state_main();
//
//	BaseState *pStateSerial = pMain->GetStateName(*pnState, NULL, nSerial);
//	if(pStateSerial == NULL)
//		return 1;
//
//	return 0;
//}

int state_active_count(int _key, BaseState *_state, STLMnInt group_m)
{
	BaseStateMain* pMain;
	pMain = _state->get_state_main();

	const STLVpState& stlVState = pMain->get_active_state();

	int nCount = 0;
	for (unsigned i = 0; i < stlVState.size(); i++)
	{
		if (stlVState[i]->get_key() == _key)
		{
			if (group_m.size() == 0)
				nCount++;
			else if (stlVState[i]->group_id_include(group_m))
				nCount++;
		}
	}

	return nCount;
}

STDEF_FUNC(BaseStateActiveCount_nF)
{
	const int* pnStateKey;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateActiveCount_nF, pnStateKey);

	const int* groupId_n = NULL;
	STDEF_GETLOCAL(_pdsvBase, BaseStateActiveCountGroupId_nV, groupId_n);
	STDEF_BaseState(pState);

	STLMnInt group_m;
	if (groupId_n != NULL) {
		STLMnInt::const_iterator it;
		it = pState->group_id_get().find(*groupId_n);
		if (it != pState->group_id_get().end()) {
			group_m[*groupId_n] = it->second;;
		}
	}

	int count = state_active_count(*pnStateKey, pState, group_m);

	if (BaseState::VariableSet(_pdsvBase, _pdsvContext, _pdsvEvent, NULL, (void*)& count, 1))
		return 1;

	return 0;
}

STDEF_FUNC(BaseStateActiveCheckGlobal_astrIf)
{
	const char* strStates;
	STLVInt	stlVnStateKeys;

	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseStateActiveCheckGlobal_astrIf"), (const void**)& strStates))
		return 0;

	BaseFile paser;

	int nSize = (int)strlen(strStates);
	paser.OpenFile((void*)strStates, nSize);

	paser.set_asc_deletor(" ");
	paser.set_asc_seperator(",");
	paser.read_asc_line();

	STDEF_BaseState(pState);
	BaseStateMain* pMain;
	pMain = pState->get_state_main();

	char strBuf[255];
	while (paser.read_asc_string(strBuf, 255))
	{
		if (*strBuf == 0)
			break;

		if (pMain->GetStateName(STRTOHASH(strBuf), NULL) == NULL)
			return 0;

		*strBuf = 0;
	}

	return 1;
}

STDEF_FUNC(BaseStateActiveCheck_astrIf)
{
	const char* strStates;
	STLVInt	stlVnStateKeys;

	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseStateActiveCheck_astrIf"), (const void**)& strStates))
		return 0;

	BaseFile paser;

	int nSize = (int)strlen(strStates);
	paser.OpenFile((void*)strStates, nSize);

	paser.set_asc_deletor(" ");
	paser.set_asc_seperator(",");
	paser.read_asc_line();

	STDEF_BaseState(pState);
	BaseStateMain* pMain;
	pMain = pState->get_state_main();

	STLMnInt stlMnGroup = BaseState::group_get(pState, _pdsvBase, _pdsvContext, _pdsvEvent);
	char strBuf[255];
	while (paser.read_asc_string(strBuf, 255))
	{
		if (*strBuf == 0)
			break;

		if (pMain->GetStateName(STRTOHASH(strBuf), &stlMnGroup) == NULL)
			return 0;

		*strBuf = 0;
	}

	return 1;
}

STDEF_FUNC(BaseStateInactiveCheckGlobal_astrIf)
{
	const char* strStates;
	STLVInt	stlVnStateKeys;

	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseStateInactiveCheckGlobal_astrIf"), (const void**)& strStates))
		return 0;

	BaseFile paser;

	int nSize = (int)strlen(strStates);
	paser.OpenFile((void*)strStates, nSize);

	paser.set_asc_deletor(" ");
	paser.set_asc_seperator(",");
	paser.read_asc_line();

	STDEF_BaseState(pState);
	BaseStateMain* pMain;
	pMain = pState->get_state_main();

	char strBuf[255];
	while (paser.read_asc_string(strBuf, 255))
	{
		if (*strBuf == 0)
			break;

		if (pMain->GetStateName(STRTOHASH(strBuf), NULL))
			return 0;
	}

	return 1;
}

STLMnInt BaseState::group_get(BaseState* _state_p, const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvContext, BaseDStructureValue* _pdsvEvent)
{
	STLMnInt stlMnGroup;
	const int* pnGroup;
	short nCnt;
	if (STDEF_AGETLOCAL(_pdsvBase, BaseStateEventCastWidthIdGroup_anV, pnGroup, nCnt))
	{
		for (int i = 0; i < nCnt; i++)
		{
			int nId = 0;
			const int* pnId = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvEvent, 1);
			if (pnId == NULL)
			{
				nId = _state_p->identifier_get(pnGroup[i]);
				if (nId != 0)
					pnId = &nId;
			}

			if (pnId == NULL)
				stlMnGroup = _state_p->group_id_get();
			else
				stlMnGroup[pnGroup[i]] = *pnId;
		}
	}
	else {
		stlMnGroup = _state_p->group_id_get();
	}

	return stlMnGroup;
}

STDEF_FUNC(BaseStateInactiveCheck_astrIf)
{
	const char* strStates;
	STLVInt	stlVnStateKeys;

	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseStateInactiveCheck_astrIf"), (const void**)& strStates))
		return 0;

	BaseFile paser;

	int nSize = (int)strlen(strStates);
	paser.OpenFile((void*)strStates, nSize);

	paser.set_asc_deletor(" ");
	paser.set_asc_seperator(",");
	paser.read_asc_line();

	STDEF_BaseState(pState);
	BaseStateMain* pMain;
	pMain = pState->get_state_main();

	STLMnInt stlMnGroup = BaseState::group_get(pState, _pdsvBase, _pdsvContext, _pdsvEvent);
	char strBuf[255];
	while (paser.read_asc_string(strBuf, 255))
	{
		if (*strBuf == 0)
			break;

		if (pMain->GetStateName(STRTOHASH(strBuf), &stlMnGroup))
			return 0;
	}

	return 1;
}

STDEF_FUNC(BaseStateGradeCloseDN_nF)
{
	int nGrade;

	if (!_pdsvBase->get_local_seq("BaseStateGradeCloseDN_nF", &nGrade))
		return 0;

	BaseStateMain* pMain;
	pMain = ((BaseState*)_pdsvBase->m_pVoidParam)->get_state_main();

	pMain->grade_close(nGrade, _pdsvEvent);
	return 1;
}

STDEF_FUNC(BaseEnd_nIf)
{	
	return 1;
}

STDEF_FUNC(BaseSuspend_nIf)
{
	return 1;
}


STDEF_FUNC(BaseResume_nF)
{
	int nKeyCasterReq;

	if (_pdsvBase->get_local_seq("BaseResumeFrom_nV", &nKeyCasterReq)) {
		const int* pnKey;
		if (!_pdsvEvent->get_local_seq(HASH_STATE(BaseTransitionCasterKey), (const void**)& pnKey))
			return 0;

		if (*pnKey != nKeyCasterReq)
			return 0;
	}

	if (_pdsvBase->get_local_seq("BaseResumeFromNot_nV", &nKeyCasterReq)) {
		const int* pnKey;
		if (!_pdsvEvent->get_local_seq(HASH_STATE(BaseTransitionCasterKey), (const void**)& pnKey))
			return 0;

		if (*pnKey == nKeyCasterReq)
			return 0;
	}

	return 1;
}

STDEF_FUNC(BasePreviousStateCheck_nIf)
{
	int nKeyCasterReq;

	if (!_pdsvBase->get_local_seq("BasePreviousStateCheck_nIf", &nKeyCasterReq))
		return 0;

	const int* pnKey;
	if (!_pdsvBase->get_local_seq(HASH_STATE(BaseTransitionCasterKey), (const void**)& pnKey))
		return 0;

	if (*pnKey == nKeyCasterReq)
		return 1;

	return 0;
}

STDEF_FUNC(BaseStart_nIf)
{
	return 1;
}

STDEF_FUNC(BaseValueAddToC)
{
	return BaseState::ContextLoopCallbackInt(_pdsvBase, _pdsvContext, BaseState::ContextAdd);
}

STDEF_FUNC(BaseValueSubToC)
{
	return BaseState::ContextLoopCallbackInt(_pdsvBase, _pdsvContext, BaseState::ContextSub);
}

STDEF_FUNC(BaseValueCheckContextUp)
{
	const char* strFilename;
	if (_pdsvBase->get(STRTOHASH("BaseValueFileLoad_strV"), (const void**)& strFilename))
	{
		_pdsvContext->set_alloc(STRTOHASH("ContextFileSaveName_strV"), strFilename);
		BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextLoad);
	}

	return BaseState::ContextLoopCallbackInt(_pdsvBase, _pdsvContext, BaseState::ContextCheckUp);
}

STDEF_FUNC(BaseValueCheckContextDown)
{
	const char* strFilename;
	if (_pdsvBase->get(STRTOHASH("BaseValueFileLoad_strV"), (const void**)& strFilename))
	{
		_pdsvContext->set_alloc(STRTOHASH("ContextFileSaveName_strV"), strFilename);
		BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextLoad);
	}

	return BaseState::ContextLoopCallbackInt(_pdsvBase, _pdsvContext, BaseState::ContextCheckDown);
}
//
//STDEF_FUNC(BaseValueSubTempParamToC)
//{
//	int	nHash;
//	
//	int i;
//	i = _pdsvBase->sequence_get();
//	i++;	// skip current colum, cause it's Context function.
//	
//	STDEF_BaseState(pState);
//	INT64 nAppend = (INT64)pState->TempValueGet(HASH_STATE(ParamInt));
//	const int *pnValue;
//	for(; i<_pdsvBase->get_count(); i++)
//	{
//		if(BaseState::context_value_int(_pdsvBase, _pdsvContext, i, (const void**)&pnValue, &nHash))
//		{
//			int nValue = *pnValue - (int)nAppend;
//			_pdsvContext->set_alloc(nHash, &nValue);
//		}else
//			break;
//	}
//	return 1;
//}

int BaseState::ContextCheckIs(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	return 1;
}

int BaseState::ContextCheckUp(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)		return 0;

	//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
	//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, pVoidContext, _nCnt);

	if (_nSize == 0)
	{
#ifdef _DEBUG
		//g_SendMessage(LOG_MSG, "CheckEqual %s, %s", (const char*)pVoidBase, (const char*)pVoidContext);
#endif
		const char* strLeft = (const char*)pVoidContext;
		const char* strRight = (const char*)pVoidBase;

		if (BaseStringTable::find(strLeft, strRight) >= 0)
			return 1;
		return 0;
	}
	else {
		if (*((int*)pVoidBase) < *((int*)pVoidContext))
			return 1;
	}
	return 0;
}

int BaseState::ContextCheckDown(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)		return 0;

	if (*((int*)pVoidBase) > * ((int*)pVoidContext))
		return 1;
	return 0;
}

int BaseState::ContextCheckEqUp(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)		return 0;

	//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
	//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, pVoidContext, _nCnt);

	if (*((int*)pVoidBase) <= *((int*)pVoidContext))
		return 1;
	return 0;
}

int BaseState::ContextCheckEqDown(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)		return 0;
	
	//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
	//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, pVoidContext, _nCnt);

	if (*((int*)pVoidBase) >= *((int*)pVoidContext))
		return 1;
	return 0;
}

int BaseState::ContextCheckEqualLeft(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)		return 0;

	//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
	//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, pVoidContext, _nCnt);

	if (_nSize == 0)
	{
#ifdef _DEBUG
		//g_SendMessage(LOG_MSG, "CheckEqual %s, %s", (const char*)pVoidBase, (const char*)pVoidContext);
#endif
		const char* strLeft = (const char*)pVoidContext;
		const char* strRight = (const char*)pVoidBase;

		if (strncmp(strRight, "NULL", 3) == 0)
		{
			if (*(strLeft) == 0) // means that the string exist but the content is null
				return 1;
			else
				return 0;
		}

		if (strLeft == NULL)
			return 0;

		size_t nLen = strlen(strRight);
		if (strncmp(strRight, strLeft, nLen) == 0)
			return 1;
	}
	else {
		if (memcmp(pVoidBase, pVoidContext, _nCnt * _nSize) == 0)
			return 1;
	}
	return 0;
}

int BaseState::ContextCheckNotEqual(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL
		|| pVoidBase == NULL)		return 0;

	int ret = ContextCheckEqual(_nHash, _pdsvContext, pVoidBase, pVoidContext, _nCnt, _nSize);

	if (ret == 0)
		return 1;
	return 0;
}

int BaseState::ContextCheckEqual(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL
		|| pVoidBase == NULL)		return 0;

	if (_nSize == 0)
	{
#ifdef _DEBUG
		if (_nSize == 0 &&
			strncmp((const char*)pVoidBase, "moved", 5) == 0)
		{
			int i = 0;
			i++;
			//g_SendMessage(LOG_MSG, "Set %s", (const char*)pVoidBase);
		}
#endif
		if (strncmp((const char*)pVoidBase, "NULL", 3) == 0)
		{
			if (*((const char*)pVoidContext) == 0)
				return 1;
			else
				return 0;
		}

		//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p) {
		//	static char buff[4096];
		//	strcpy_s(buff, 4096, (const char*)pVoidBase);
		//	strcat_s(buff, 4096, (const char*)"==");
		//	strcat_s(buff, 4096, (const char*)pVoidContext);
		//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, buff);
		//}
#ifdef _DEBUG
		g_SendMessage(LOG_MSG, "---Equal: %s==$s", (const char*)pVoidBase, (const char*)pVoidContext);
#endif
		if (strcmp((const char*)pVoidBase, (const char*)pVoidContext) == 0)
			return 1;
	}
	else {
		//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
		//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, pVoidContext, _nCnt);

		if (memcmp(pVoidBase, pVoidContext, _nCnt * _nSize) == 0)
			return 1;
	}
	return 0;
}

int copy(const char* input, const char* output) {

	BaseFile filer, filew;

	char buffer[4096];
	int nRead = 0;

	if (!filer.OpenFile(input, BaseFile::OPEN_READ))
	{
		if (!filew.OpenFile(output, BaseFile::OPEN_WRITE))
		{
			nRead = filer.Read(buffer, 4096);
			while (nRead > 0)
			{
				filew.Write(buffer, nRead);
				nRead = filer.Read(buffer, 4096);
			}
			nRead = (int)filew.get_size_file();
			filew.CloseFile();
		}
		else {
			filer.CloseFile();
			return 0;
		}
		filer.CloseFile();
	}
	else {
		return 0;
	}
	return nRead;
}

int BaseState::ContextFileCopy(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	const char* strFile1 = (const char*)pVoidBase,
		* strFile2 = (const char*)pVoidContext;

	char strPath1[1024], strPath2[1024];
	BaseSystem::tomulti(strFile1, strPath1, 1024);
	BaseSystem::tomulti(strFile2, strPath2, 1024);

	if (copy(BaseStateManager::get_manager()->path_full_make(strPath1, 1024), BaseStateManager::get_manager()->path_full_make(strPath2, 1024)) > 0)
		return 1;
	return 0;
}

int BaseState::ContextFileSizeGet(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	const char* strFile1 = (const char*)pVoidBase;

	if (strlen(strFile1) >= 1023)
		return 0;

	char strPath1[1024];
	BaseSystem::tomulti(strFile1, strPath1, 1024);

	BaseFile file;
	if (file.OpenFile(BaseStateManager::get_manager()->path_full_make(strPath1, 1024), BaseFile::OPEN_READ))
		return 0;
	int nSize = file.get_size_file();

	_pdsvContext->set_alloc(_nHash, &nSize);
	return 1;
}

int BaseState::ContextFileRename(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	const char* strFile1 = (const char*)pVoidBase,
		* strFile2 = (const char*)pVoidContext;

	char strPath1[1024], strPath2[1024];
	strcpy_s(strPath1, 1024, strFile1);
	strcpy_s(strPath2, 1024, strFile2);

	if (rename(BaseStateManager::get_manager()->path_full_make(strPath1, 1024), BaseStateManager::get_manager()->path_full_make(strPath2, 1024)) == 0)
		return 1;
	return 0;
}

int BaseState::ContextFileExist(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	char strBuf[1024];
	const char* strFile = (const char*)pVoidBase;

	if (_nSize != 0) {
		sprintf_s(strBuf, 1024, "%d", *((const int*)pVoidContext));
		strFile = strBuf;
	}
	else if (strFile == NULL)
		strFile = (const char*)pVoidContext;
	char strPath1[1024];
	BaseSystem::tomulti(strFile, strPath1, 1024);
	//strcpy_s(strPath1, 1024, strFile);

	if (BaseFile::is_exist(BaseStateManager::get_manager()->path_full_make(strPath1, 1024)))
		return 1;
	return 0;
}

int BaseState::ContextFileDelete(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	char strBuf[1024];
	const char* strFile = (const char*)pVoidBase;
	if (_nSize != 0) {
		sprintf_s(strBuf, 1024, "%d", *((const int*)pVoidContext));
		strFile = strBuf;
	}
	else if (strFile == NULL)
		strFile = (const char*)pVoidContext;
	char strPath1[1024];
	strcpy_s(strPath1, 1024, strFile);

	BaseSystem::file_delete(BaseStateManager::get_manager()->path_full_make(strPath1, 1024));
	return 1;
}

int BaseState::ContextFolderRename(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	const char* strFile1 = (const char*)pVoidBase,
		* strFile2 = (const char*)pVoidContext;
	char strPath1[1024], strPath2[1024];
	strcpy_s(strPath1, 1024, strFile1);
	strcpy_s(strPath2, 1024, strFile2);

	if (rename(BaseStateManager::get_manager()->path_full_make(strPath1, 1024), BaseStateManager::get_manager()->path_full_make(strPath2, 1024)) == 0)
		return 1;
	return 0;
}

int BaseState::ContextFolderDelete(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	char strBuf[1024];
	const char* strFile = (const char*)pVoidBase;
	if (_nSize != 0) {
		sprintf_s(strBuf, 1024, "%d", *((const int*)pVoidContext));
		strFile = strBuf;
	}
	else if (strFile == NULL)
		strFile = (const char*)pVoidContext;
	char strPath1[1024];
	strcpy_s(strPath1, 1024, strFile);

	BaseSystem::folder_delete(BaseStateManager::get_manager()->path_full_make(strPath1, 1024));
	return 1;
}

int BaseState::ContextFolderCreate(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	char strBuf[1024];
	const char* strFile = (const char*)pVoidBase;
	if (_nSize != 0) {
		sprintf_s(strBuf, 1024, "%d", *((const int*)pVoidContext));
		strFile = strBuf;
	}
	else if (strFile == NULL)
		strFile = (const char*)pVoidContext;
	char strPath1[4096];
	strcpy_s(strPath1, 4096, strFile);

	BaseSystem::folder_create(BaseStateManager::get_manager()->path_full_make(strPath1, 1024));
	return 1;
}

int BaseState::ContextFolderCurChange(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	char strBuf[4096];
	const char* strFile = (const char*)pVoidBase;
	if (_nSize != 0) {
		sprintf_s(strBuf, 1024, "%d", *((const int*)pVoidContext));
		strFile = strBuf;
	}
	else if (strFile == NULL)
		strFile = (const char*)pVoidContext;

	strcpy_s(strBuf, 4096, BaseStateManager::get_manager()->path_get_save());

	if (strcmp(strFile, "..") == 0) {
		BaseFile::path_cut_last(strBuf, 4096);
	}
	else {
		strcat_s(strBuf, 4096, strFile);
		strcat_s(strBuf, 4096, SLASH);
	}
	BaseStateManager::get_manager()->path_set_save(strBuf);
	return 1;
}

int BaseState::ContextFolderCurGet(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	char strBuf[4096];
	strcpy_s(strBuf, 4096, BaseStateManager::get_manager()->path_get_save());

	_pdsvContext->set_alloc(_nHash, strBuf);
	return 1;
}

int BaseState::ContextSetHash(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	int nHash = STRTOHASH((const char*)pVoidBase);
	_pdsvContext->set_alloc(_nHash, &nHash);
	return 1;
}

//int BaseState::ContextSetPassword(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
//{
//	char strNull[10] = "";
//
//	if (pVoidBase == NULL || strncmp((const char*)pVoidBase, "NULL", 4) == 0)
//	{
//		pVoidBase = strNull;
//		_nCnt = 1;
//	}
//
//	if (_nSize == 0)
//	{
//		BaseFile* pEncode;
//		PT_OAlloc1(pEncode, BaseFile, NULL); // BaseStateManager::encoder_get());
//
//		const char* strRight = (const char*)pVoidBase;
//		size_t nLen = strlen(strRight);
//
//		if (nLen > 30)
//			return 0;
//
//		BYTE strBuf[256];
//		int nSize = 0;
//		//{
//		//	pEncode->OpenFile((char*)NULL, BaseFile::OPEN_WRITE | BaseFile::ENCODE_001 | BaseFile::OPEN_MEMORY | BaseFile::MEM_OUTDATA
//		//		, (void*)strBuf, 256);
//
//		//	pEncode->write_asc_string(strRight, (int)nLen);
//		//	pEncode->write_asc_line();
//
//		//	pEncode->CloseFile();
//		//	int nSize = pEncode->get_buffer_size();
//		//  PT_OFree(pEncode);
//		//}
//
//		{
//			SHA256_Encrpyt((const BYTE*)strRight, (UINT)nLen, strBuf);
//			nSize = 32;
//		}		
//
//		strBuf[nSize] = 0;
//
//
//		char strOut[256];
//		for (int i = 0; i < nSize; i++)
//		{
//			unsigned x = (unsigned)strBuf[i];
//			sprintf_s(strOut + i * 2, 3, "%02x", x);
//		}
//		strOut[nSize * 2] = 0;
//		_pdsvContext->set_alloc(_nHash, (void*)strOut, _nCnt);
//
//		if (g_logger())
//			_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, (void*)strOut, _nCnt);
//
//		g_SendMessage(LOG_MSG, "SetPass_from'%s'to'%s'", strRight, strOut);
//		return 1;
//	}
//#ifdef _DEBUG
//	//if (_nSize == 0)
//	//g_SendMessage(LOG_MSG, "Set %s", (const char*)pVoidBase);
//#endif
//	_pdsvContext->set_alloc(_nHash, pVoidBase, _nCnt);
//	return 1;
//}

int BaseState::ContextSet(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	char strNull[10] = "";

	if (pVoidBase == NULL || strncmp((const char*)pVoidBase, "NULL", 4) == 0)
	{
		pVoidBase = strNull;
		_nCnt = 1;
	}
#ifdef _DEBUG
	if (_nSize == 0 &&
			strncmp((const char*)pVoidBase, "moved", 5) == 0)
	{
		int i = 0;
		i++;
		//g_SendMessage(LOG_MSG, "Set %s", (const char*)pVoidBase);
	}
#endif

	_pdsvContext->set_alloc(_nHash, pVoidBase, _nCnt);

	return 1;
}

int BaseState::ContextStringUpdate(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	static char s_strTarget[4096] = "";

	if (pVoidBase == NULL || strncmp((const char*)pVoidBase, "NULL", 4) == 0)
	{
		if (pVoidContext)
			strcpy_s(s_strTarget, 4096, (const char*)pVoidContext);
	}
	else {
		if (!pVoidContext)
			return 0;

		char strTemp[4096], * strHead;
		const char* strTail;
		strcpy_s(strTemp, 4096, (const char*)pVoidContext);
		strHead = strstr(strTemp, s_strTarget);
		if (!strHead)
			return 0;

		*strHead = NULL;
		strTail = ((const char*)pVoidContext) + strlen(strTemp) + strlen(s_strTarget);
		strcat_s(strTemp, 4096, (const char*)pVoidBase);
		strcat_s(strTemp, 4096, strTail);
		_pdsvContext->set_alloc(_nHash, strTemp);

		//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
		//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, strTemp);
	}
	return 1;
}

int BaseState::ContextMulti(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)
		return 0;
	int nValue;
	nValue = *((int*)pVoidContext) * *((int*)pVoidBase);
	_pdsvContext->set_alloc(_nHash, &nValue);

	//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
	//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, &nValue);

	return 1;
}

int BaseState::ContextStack(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)
		return 0;

	if(_nCnt > 0) // _nCnt is the length right param, but now only '1' is allowed
		_nCnt = 1;
	short cnt = 0;
	if (_nSize == 0) // if param is string
	{
		return 0;
	}
	
	const int *value_ap = NULL;

	_pdsvContext->get(_nHash, (const void**)&value_ap, &cnt);

	if (_nCnt == 0)
		return 0;

	int* valueout_ap = NULL;

	if (_nSize == 4)
	{
		valueout_ap = PT_Alloc(int, (cnt + _nCnt));
		for (int i = 0; i < cnt; i++)
			*(valueout_ap + i) = *(value_ap + i);
		for (int i = 0; i < _nCnt; i++)
			*(valueout_ap + cnt + i) = *(((int*)pVoidBase) + i);
		_pdsvContext->set_alloc(_nHash, valueout_ap, cnt + _nCnt);
	}
	else if(_nSize == 8){
		valueout_ap = (int*)PT_Alloc(INT64, (cnt + _nCnt));
		for (int i = 0; i < cnt; i++)
			*(((INT64*)valueout_ap) + i) = *(((INT64*)value_ap) + i);
		for (int i = 0; i < _nCnt; i++)
			*(((INT64*)valueout_ap) + (cnt + i)) = *(((INT64*)pVoidBase) + i);
		_pdsvContext->set_alloc(_nHash, valueout_ap, cnt + _nCnt);
	}

	//if (g_logger() && _pdsvContext->logger_link_get()->m_logevent_p)
	//	_pdsvContext->logger_link_get()->m_logevent_p->logger_hand_variable(_nHash, valueout_ap, cnt+_nCnt);

	if (valueout_ap)
		PT_Free(valueout_ap);

	return 1;
}


int BaseState::ContextAdd(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)
		return 0;

	if (_nSize == 0) // if param is string
	{
		STLString strBuffer;

		strBuffer = (const char*)pVoidContext;
		//strBuffer += " ";
		if (pVoidBase != NULL && *((const char*)pVoidBase) != 0)
			strBuffer += (const char*)pVoidBase;
		else
			strBuffer += ",";
#ifdef _DEBUG
		//g_SendMessage(LOG_MSG, "Add from: %s to: %s", (const char*)pVoidContext, strBuffer.c_str());
#endif
		_pdsvContext->set_alloc(_nHash, strBuffer.c_str());


		return 1;
	}
	else {
		int nValue;
		nValue = *((int*)pVoidContext) + *((int*)pVoidBase);
		_pdsvContext->set_alloc(_nHash, &nValue);
	}
	return 1;
}

int BaseState::ContextSub(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	if (pVoidContext == NULL)
		return 0;

	if (_nSize == 0) // if param is string
	{	
		STLString buff, befor, after, sub;
		buff = (const char*)pVoidContext;
		sub = (const char*)pVoidBase;
		size_t find = buff.find(sub);
		if (find == STLString::npos)
		{
			find = BaseStringTable::find(buff, sub);
			if(find == STLString::npos)
				return 0;
		}

		befor = buff.substr(0, find);
		after = buff.substr(find + sub.size(), buff.size());
		befor += after;
		_pdsvContext->set_alloc(_nHash, befor.c_str());

		return 1;
	}
	else {
		int nValue;
		nValue = *((int*)pVoidContext) - *((int*)pVoidBase);
		_pdsvContext->set_alloc(_nHash, &nValue);

	}
	return 1;
}

int BaseState::ContextLoopCallback(const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvContext, FnContextProcessor fnProcessor)
{
	int	nRet = 0;
	int	nHash;

	int i;
	i = _pdsvBase->sequence_get();
	i++;	// skip current colum, cause it's Context function.
	//short nCnt;

	for (; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if ((_pdsvBase->get_base()->get_type_flag(nIndex) & BaseDStructure::VALUE_CONTEXT) != 0)
		{
			const void* pVoid;
			const void* pVoid2;
			short nCount = 0;
			short nCount2 = 0;
			short nSize = 0;

			nSize = _pdsvBase->get_base()->get_type_size(nIndex);

			if (_pdsvBase->get(&pVoid, &nCount, i))
			{
				if (!_pdsvContext->get(nHash, &pVoid2, &nCount2))
					pVoid2 = NULL;

				_pdsvContext->logger_link_set(_pdsvBase);
				if (_pdsvBase->get_base()->get_type(nIndex) == TYPE_STRING
					&& strcmp((const char*)pVoid, "NULL") == 0)
				{
					if (fnProcessor(nHash, _pdsvContext, "", pVoid2, 1, nSize) != 0)
						nRet = 1;
					else
						return 0;
				}
				else {
					if (fnProcessor(nHash, _pdsvContext, pVoid, pVoid2, nCount, nSize) != 0)
						nRet = 1;
					else
						return 0;
				}
			}
		}
		else { // if current colum is not a Context, finish to serach.
			break;
		}
	}

	return nRet;
}

int BaseState::ContextLoopCallbackInt(const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvContext, FnContextProcessor fnProcessor)
{
	int	nRet = 0;
	int	nHash;

	int i;
	i = _pdsvBase->sequence_get();
	i++;	// skip current colum, cause it's Context function.
	short nCnt;

	for (; i < _pdsvBase->get_count(); i++)
	{
		nHash = _pdsvBase->get_colum(i);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if ((_pdsvBase->get_base()->get_type_flag(nIndex) & BaseDStructure::VALUE_CONTEXT) != 0)
		{
			int nType;
			nType = _pdsvBase->get_base()->get_type(nIndex);

			const void* pVoid;
			const void* pVoid2 = NULL;

			switch (nType)
			{
			case TYPE_INT32:
				_pdsvContext->get(nHash, &pVoid2);
				break;
			case TYPE_STRING:
			{
				if (_pdsvContext->get(nHash, &pVoid2))
				{
					int nHash2 = STRTOHASH((const char*)pVoid2);
					if (!_pdsvContext->get(nHash2, &pVoid2))
						pVoid2 = NULL;
				}
			}
			break;
			}

			_pdsvContext->logger_link_set(_pdsvBase);
			if (_pdsvBase->get(&pVoid, &nCnt, i))
			{
				if (fnProcessor(nHash, _pdsvContext, pVoid, pVoid2, 1, 4) != 0)
					nRet = 1;
				else
					return 0;
			}
		}
		else {// if current colum is not a Context, finish to serach.
			break;
		}
	}

	return nRet;
}

int BaseState::VariableLoopCallbackStruct(bool bConst, int _nHashOp, int _nTypeOp, const int* _pnValue, short _nCnt
	, const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvV1, BaseDStructureValue* _pdsvV2, FnContextProcessor fnProcessor)
{
	char strBuffer[255];
	const int* pnValue2 = NULL;
	INT64 nAddress2 = (INT64)_pdsvV2;
	INT64	nValue1;

	int	nRet = 0;
	bool    bDebugView = false;

	int	nHash = 0, nHash2 = 0;
	short nCntCompare = 1;
	nValue1 = 0;

	if (_nTypeOp == TYPE_STRING) {
		BaseFile file;
		file.set_asc_seperator(", ");

		file.OpenFile((void*)_pnValue, (int)strlen((char*)_pnValue));
		if (file.read_asc_line()) {
			file.read_asc_string(strBuffer, 255);
			nHash = STRTOHASH(strBuffer);
			int nLen1, nLen2;
			nLen2 = (int)strlen((const char*)_pnValue);
			nLen1 = (int)strlen(strBuffer);
			if (nLen2 > nLen1 + 2) {
				pnValue2 = (const int*)(((const char*)_pnValue) + strlen(strBuffer) + 2);
				pnValue2 = (const int*)PTGET((const char*)pnValue2);
			}
			else {
				pnValue2 = NULL;
			}
		}
	}
	else {
		nHash = _pnValue[0];
		if (_nCnt >= 2)
			nValue1 = _pnValue[1];
	}

	if (_nHashOp == HASH_STATE(BaseVariableToReferVar_varV)) // receive is refer variable, so should changed to the real variable
	{
		const char* ref_hash_str;
		if (!_pdsvV1->get(nHash, (const void**)&ref_hash_str))
			return 2;// continue;
		nHash = STRTOHASH(ref_hash_str); // it's the real variable
		_nHashOp = HASH_STATE(BaseVariableToVariable_anV); // fallowed process is not different with BaseVariableToVariable.
	}

	if (_nHashOp == HASH_STATE(BaseVariableBreak_strV))
	{
		g_SendMessage(LOG_MSG_POPUP, (const char*)_pnValue);
		bDebugView = true;
		return 2;// continue;
	}

	if ((int)nValue1 != 0 && // nValue1 is sender.
			_nHashOp == HASH_STATE(BaseVariableToVariable_anV))
	{
		int nIndex = _pdsvV1->get_base()->get_index((int)nValue1);// If reference variable is string, receiveing variable should be a string.
		if (nIndex >= 0 && _pdsvV1->get_base()->get_type(nIndex) == TYPE_STRING)
			_nTypeOp = TYPE_STRING;
	}

	int nIndex = _pdsvV1->get_base()->get_index(nHash);

	if (nIndex != -1)
	{
		int type = BaseDStructure::get_type(nIndex);
		if (type == TYPE_STRING)
			_nTypeOp = TYPE_STRING;
		else if(type == TYPE_INT64)
			_nTypeOp = TYPE_INT64;
	}

	int nSize = 0;
	if (_nHashOp == HASH_STATE(BaseVariableToVariable_anV))
	{
		int nInt = 0;

		if (_pdsvV2)
		{
			if ((int)nValue1 == 0) {
				_pdsvV2->get(nHash, (const void**)&pnValue2, &nCntCompare);
				nHash2 = nHash;
			}
			else
			{
				nHash2 = nValue1;
				if (_pdsvV2->get((int)nValue1, (const void**)&pnValue2, &nCntCompare)
					&& _nTypeOp == TYPE_INT64)
				{
					int index = BaseDStructure::get_index((int)nValue1);
					int type = BaseDStructure::get_type(index);
					if (type == TYPE_INT32)
					{
						nValue1 = 0L;
						memcpy(&nValue1, pnValue2, 4);
						pnValue2 = (const int*)&nValue1;
					}
				}
			}

		}
		else {
			_pdsvV2 = _pdsvV1; // if _pdsvV2 is null, use the same variable group.

			if ((int)nValue1 == 0) {
				_pdsvV1->get(nHash, (const void**)&pnValue2, &nCntCompare);
				nHash2 = nHash;
			}
			if (pnValue2 != NULL)
				nAddress2 = (INT64)_pdsvV1;
		}

		if (pnValue2 == NULL)
		{
			if ((int)nValue1 != 0) {
				_pdsvV1->get((int)nValue1, (const void**)&pnValue2, &nCntCompare);
				nHash2 = nValue1;
			}
			if (pnValue2 != NULL)
				nAddress2 = (INT64)_pdsvV1;
		}

		if (pnValue2 == NULL) {
			pnValue2 = reserved_value(_pdsvBase, (int)nValue1, &nInt);
			if (nInt == TYPE_STRING) {
				_nTypeOp = TYPE_STRING;

				if (bDebugView)
				{
					g_SendMessage(LOG_MSG, "Reserved:%d = '%s'", (int)nValue1, (const char*)pnValue2);
				}
			}
			nAddress2 = 1;
		}

		if (_nTypeOp == TYPE_STRING)
		{
			int nIndex2 = _pdsvV1->get_base()->get_index((int)nValue1);
			if (nInt == TYPE_INT32 || (nIndex2 >= 0 && _pdsvV1->get_base()->get_type(nIndex2) == TYPE_INT32))
			{
				if (pnValue2 == NULL) {
#ifdef _DEBUG
					g_SendMessage(LOG_MSG_POPUP, "Fail to get value %d in BaseVariableToVariable_anV %d, %d", nValue1, nHash, nValue1);
#endif
					return 2;// continue;
				}
				sprintf_s(strBuffer, 255, "%d", *pnValue2);
				pnValue2 = (const int*)strBuffer;
			}
		}

		if (pnValue2 == NULL) {
#ifdef _DEBUG
			g_SendMessage(LOG_MSG, "Fail to get value %d in BaseVariableToVariable_anV %d, %d", nValue1, nHash, nValue1);
#endif
			return 2;// continue;
		}
	}
	else if (_nHashOp == HASH_STATE(BaseVariableRefRemove_nV))
	{
		_pdsvV1->set_alloc(nHash, NULL);
		return 2;// continue;
	}
	else if (_nHashOp == HASH_STATE(BaseVariableConst_nV)
			|| _nHashOp == HASH_STATE(BaseVariableConst_fV)) {
		nHash = 0;
		pnValue2 = NULL;
		nAddress2 = 2;
	}
	else if (_nHashOp == HASH_STATE(BaseVariableConst_strV)) {
		nHash = 0;
		pnValue2 = NULL;
		_nTypeOp = TYPE_STRING;
		nAddress2 = 2;
	}
	else if (_nCnt > 2 && _nTypeOp == TYPE_INT32){
		pnValue2 = &_pnValue[1];
		nCntCompare = _nCnt-1;
	}
	else if (_nTypeOp != TYPE_STRING) {
		pnValue2 = (int*)&nValue1; nCntCompare = 1;
		nAddress2 = 3;
	}

	if (!bConst && nIndex == -1)
	{
		if (_nCnt == 1) {
			_pnValue = reserved_value(_pdsvBase, nHash, &_nTypeOp);
			if (_pnValue)
				bConst = true;
		}


		if (fnProcessor == BaseState::ContextFileSizeGet)
			_nTypeOp = TYPE_INT32;

		STDEF_Manager(pManager);
		bool force_define = false;
		if (_nHashOp == HASH_STATE(BaseVariableString_strV))
			force_define = true;
		if (!bConst && !pManager->variable_define(nHash, _nTypeOp, force_define))
			return 2;// continue;

		nIndex = BaseDStructure::get_index(nHash);
	}

	if (!bConst
		&& fnProcessor != BaseState::ContextSet
		//&& fnProcessor != BaseState::ContextSetPassword
		&& fnProcessor != BaseState::ContextFileSizeGet
		&& fnProcessor != BaseState::ContextFolderCurGet
		&& fnProcessor != BaseState::ContextStack
		&& !_pdsvV1->get(nHash, (const void**)& _pnValue, &nCntCompare))
	{
		if (fnProcessor == BaseState::ContextCheckIs)
			return 0;
		return 2;// continue;
	}

	if (nIndex > 0)
		nSize = BaseDStructure::get_type_size(nIndex);

	int nValueTemp = 0;
	if (_nTypeOp == TYPE_STRING)
	{
		if (fnProcessor == BaseState::ContextSet &&
			_pdsvV1->get_base()->get_type(nIndex) == TYPE_INT32)// receive type is string and giver is a int, convert to string.
		{
			int len = (int)strlen((const char*)pnValue2);
			bool num = true;

			for (int i = 0; i < len; i++)
			{
				char ch = *(((const char*)pnValue2) + i);
				if ('0' > ch || ch > '9') 
				{
					num = false;
					break;
				}
			}
			if (num)
				sscanf_s((const char*)pnValue2, "%d", &nValueTemp);
			else
				nValueTemp = STRTOHASH((const char*)pnValue2);
			pnValue2 = &nValueTemp;
		}
		nSize = 0;

		if (bDebugView)
		{
			if (fnProcessor == ContextSetHash) {
				g_SendMessage(LOG_MSG, "Hash:%d = hashset %llu-P2:'%s'", nHash, nAddress2, (const char*)pnValue2);
			}
			else if (pnValue2 && _pnValue) {
				if (!_pnValue)
					g_SendMessage(LOG_MSG, "Hash1:%d = P1:'%s'", nHash, (const char*)_pnValue);
				printf("Hash2:%d = %llu-P2:'%s'", nHash, nAddress2, (const char*)pnValue2);
				g_SendMessage(LOG_MSG, "Hash2:%d = %llu-P2:'%s'", nHash, nAddress2, (const char*)pnValue2);
			}
			else if (pnValue2) {
				g_SendMessage(LOG_MSG, "Hash:%d = %llu-P2:'%s'", nHash, nAddress2, (const char*)pnValue2);
			}
		}
	}
	else {
		if (bDebugView)
		{
			if (pnValue2 && _pnValue)
				g_SendMessage(LOG_MSG, "Hash:%d = P1:'%d' %llu-P2:'%d'", nHash, *_pnValue, nAddress2, *pnValue2);
			else if (pnValue2)
				g_SendMessage(LOG_MSG, "Hash:%d = %llu-P2:'%d'", nHash, nAddress2, *pnValue2);
		}
	}

	_pdsvV1->logger_link_set(_pdsvBase);
	if (fnProcessor(nHash, _pdsvV1, pnValue2, _pnValue, nCntCompare, nSize) != 0) {
		nRet = 1;
		int seq = 1;
		while (fnProcessor == ContextSet && nCntCompare >= LIMIT_STR && _pdsvV2 != NULL && nHash2 != 0)
		{
			if (!_pdsvV2->get(nHash2, (const void**)&pnValue2, &nCntCompare, seq++))
				break;
			_pdsvV1->add_alloc(nHash, pnValue2, nCntCompare);
		}
		if (bDebugView && _pdsvV1->get(nHash, (const void**)& _pnValue, &nCntCompare))
		{
			if (_nTypeOp == TYPE_STRING) {
				printf("result:%d = %llu-P1:'%s'", nHash, (INT64)_pdsvV1, (const char*)_pnValue);
				g_SendMessage(LOG_MSG, "result:%d = %llu-P1:'%s'", nHash, (INT64)_pdsvV1, (const char*)_pnValue);
			}
			else {
				g_SendMessage(LOG_MSG, "result:%d = %llu-P1:'%d'", nHash, (INT64)_pdsvV1, *_pnValue);
			}
		}

		if (g_logger() && _pdsvV1->logger_link_get()->m_logevent_p)
		{
			short index = _pdsvV1->get_index(nHash);
			int type = BaseDStructure::get_type(index);
			if (type == TYPE_ARRAY_BYTE)
			{
				int ha = STRTOHASH("nScanCode2");
				int size = _pdsvV1->get_mass_size(nHash);
				_pdsvV1->logger_link_get()->m_logevent_p->logger_hand_variable(ha, (const void*)&size);
			}
			else {
				_pdsvV1->logger_link_get()->m_logevent_p->logger_hand_variable(nHash, _pnValue, _nCnt);
			}
		}
	}
	else
		return 0;
	return nRet;
}

int BaseState::VariableLoopCallbackInt(const BaseDStructureValue* _pdsvBase, BaseDStructureValue* _pdsvV1, BaseDStructureValue* _pdsvV2, FnContextProcessor fnProcessor)
{
	int	nRet = 0, nHashOp;

	const int* pnValue = NULL;
	//INT64 nAddress2 = (INT64)_pdsvV2;
	int i;
	i = _pdsvBase->sequence_get();
	i++;	// skip current colum, cause it's Context function.
	short nCnt;

	STLVInt	stlVnStructure;

	for (; i < _pdsvBase->get_count(); i++)
	{
		int nTypeOp = TYPE_INT32;
		bool bConst = false;
		nCnt = 1;

		nHashOp = _pdsvBase->get_colum(i);

		if (nHashOp == HASH_STATE(BaseVariableConst_nV)
			|| nHashOp == HASH_STATE(BaseVariableConst_strV)
			|| nHashOp == HASH_STATE(BaseVariableConst_fV))
			bConst = true;

		if (nHashOp == HASH_STATE(BaseVariableRefer_strV)
			|| nHashOp == HASH_STATE(BaseVariableString_strV)
			|| nHashOp == HASH_STATE(BaseVariableBreak_strV))
			nTypeOp = TYPE_STRING;

		if (nHashOp != HASH_STATE(BaseVariableRefer_anV)
			&& nHashOp != HASH_STATE(BaseVariableToVariable_anV)
			&& nHashOp != HASH_STATE(BaseVariableRefRemove_nV)
			&& nHashOp != HASH_STATE(BaseVariableRefer_an64V)
			&& nHashOp != HASH_STATE(BaseVariableToStructure_anV)
			&& nHashOp != HASH_STATE(BaseVariableToStructure_varV)
			&& nHashOp != HASH_STATE(BaseVariableToReferVar_varV)
			&& !bConst && nTypeOp == TYPE_INT32
			)
			break;

		if (!_pdsvBase->get((const void**)& pnValue, &nCnt, i))
			continue;

		if (nHashOp == HASH_STATE(BaseVariableToStructure_anV)
			|| nHashOp == HASH_STATE(BaseVariableToStructure_varV))
		{
			STDEF_Manager(pManager);
			BaseDStructure* pstEnum = pManager->EnumGet(HASH_STATE(EnumStructure));

			int hashVar = 0;
			if (nHashOp == HASH_STATE(BaseVariableToStructure_varV)) // structure name is in the variable table in first param
			{
				int varHash = *((int*)pnValue);
				const char* structHash_str = NULL;
				if (!_pdsvV1->get(varHash, (const void**)&structHash_str))
					continue; // fail to get structure variable
				hashVar = STRTOHASH(structHash_str);
				pnValue = &hashVar; //  use this structure // might be, it's defined  by source code or script, escpecialy db table name
			}

			nHashOp = HASH_STATE(BaseVariableToVariable_anV);
			bConst = false;
			nCnt = 1;
			const int* pnHashValue = NULL;
			int nSeq = 2;
			do {
				if (!pstEnum->get(*pnValue, (const void**)& pnHashValue, NULL, nSeq))
					break;

				nRet = VariableLoopCallbackStruct(bConst, nHashOp, nTypeOp, pnHashValue, nCnt
					, _pdsvBase, _pdsvV1, _pdsvV2, fnProcessor);

				if (nRet == 0)
					break;
				nSeq++;
			} while (1);
		}
		else {
			nRet = VariableLoopCallbackStruct(bConst, nHashOp, nTypeOp, pnValue, nCnt
				, _pdsvBase, _pdsvV1, _pdsvV2, fnProcessor);
		}

		if (nRet == 0)
			return nRet;
		if (nRet == 2) // 2 mean's that continue;
			nRet = 0;
	}

	return nRet;
}

int	BaseState::grade_get()
{
	const int* pnGrade;
	if (!m_pdsvState->get(sm_stlVHash[HASH_BaseStateGradeSet_n], (const void**)& pnGrade))
		return 0;

	return *pnGrade;
}

STDEF_FUNC(BaseStateEventGlobalVariableGot_nIf)
{
	const int* hash;
	STDEF_GET_R(_pdsvBase, BaseStateEventGlobalVariableGot_nIf, hash);
	const void* void_p = NULL;
	if (_pdsvEvent->get(*hash, &void_p))
		return 1;
	return 0;
}

STDEF_FUNC(BaseEventGlobalPassCast_nF)
{
	const int* type_pn;
	STDEF_GETLOCAL_R(_pdsvBase, BaseEventGlobalPassCast_nF, type_pn);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	STDEF_Manager(manager);

	BaseDStructureValue* pdsvV1, * pdsvV2 = NULL;
	pdsvV1 = manager->variable_type(*type_pn, _pdsvBase, _pdsvContext, _pdsvEvent);

	BaseDStructure* pdstEvent = manager->EnumGet(HASH_STATE(HASH_EnumEvent));
	if (pdstEvent == NULL)// StateManager destoried.
		return 0;

	BaseDStructureValue* pdsvEvent;
	PT_OAlloc2(pdsvEvent, BaseDStructureValue, pdstEvent, 1024);

	pdsvV1->variable_transit(pdsvEvent);
	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	pdsvEvent->set_key(nKeyState);
	pdsvEvent->set_alloc(HASH_STATE(TIMETOPROCESS), NULL);

	STLMnInt stlMnGroup = BaseState::group_get(pState, _pdsvBase, _pdsvContext, _pdsvEvent);
	BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);


	int nSerial = pState->obj_serial_get();
	pdsvEvent->set_alloc(HASH_STATE(RevStateEventCaster), &nSerial);
	const int* priority_pn = NULL;
	STDEF_GETLOCAL(_pdsvBase, BaseStateEventCastPriority_nV, priority_pn);
	if (priority_pn)
		pSpace->post_event(pdsvEvent, *priority_pn);
	else
		pSpace->post_event(pdsvEvent);
	return 1;
}

STDEF_FUNC(BaseStateEventGlobal)
{
	int nEventHash;

	if (!_pdsvEvent->get("BaseStateEventGlobal", &nEventHash))
	{
		B_ASSERT(_pdsvEvent->get("BaseStateEventGlobal", &nEventHash));
		return 0;
	}

	const int* pnEventHashCheck;

	STDEF_GETLOCAL_R(_pdsvBase, BaseStateEventGlobal, pnEventHashCheck);

	if (nEventHash == *pnEventHashCheck)
	{
		int nHash, nSeq;
		const int* pnValue;
		nSeq = _pdsvBase->sequence_get() + 1;
		nHash = _pdsvBase->get_colum(nSeq);
		int nIndex;
		nIndex = _pdsvBase->get_index(nHash);
		if (_pdsvBase->get((const void**)& pnValue, NULL, nSeq) && nHash == STRTOHASH("BaseStateParam_nV"))
		{
			STDEF_Manager(pManager);
			BaseDStructureValue	dsvValue(pManager->EnumGet(STRTOHASH("EnumStateEvent")), 0);
			dsvValue.set_key(*pnEventHashCheck);

			int	nRet = 0;
			for (int i = 0; i < dsvValue.get_count(); i++)
			{
				nHash = dsvValue.get_colum(i);
				if (nHash == HASH_STATE(N_KEY) ||
					nHash == HASH_STATE(STR_NAME))
					continue;

				const int* pnParamEvt;
				if (!_pdsvEvent->get(nHash, (const void**)& pnParamEvt))
					return 0;

				if (*pnValue != *pnParamEvt)
					return 0;

				nRet = 1;
			}

			if (nRet == 0)
				return 0;
		}

		if (g_logger())
		{
			static int hash = 0;
			if (hash == 0)
				hash = STRTOHASH("VSLoggerEventSerial_nV");
			const int *serial = NULL;
			if (_pdsvEvent->get(hash, (const void**)&serial) && _pdsvBase->m_loglink_p)
				_pdsvBase->m_loglink_p->logger_state_event_receive(nEventHash, *serial);
		}
		return 1;
	}

	return 0;
}

STDEF_FUNC(BaseStateEventGlobalUnprocess)
{
	int nEventHash;

	if (!_pdsvEvent->get("BaseStateEventGlobalUnprocess", &nEventHash))
	{
		B_ASSERT(_pdsvEvent->get("BaseStateEventGlobalUnprocess", &nEventHash));
		return 0;
	}

	int nEventHashCheck;

	if (!_pdsvBase->get_local_seq("BaseStateEventGlobalUnprocess", &nEventHashCheck))
	{
		B_ASSERT(_pdsvBase->get_local_seq("BaseStateEventGlobalUnprocess", &nEventHashCheck));
		return 0;
	}

	if (nEventHash == nEventHashCheck)
	{
		return 1;
	}

	return 0;
}

STDEF_FUNC(BaseStateParent_nSE)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateParent_nSE, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);

	int nKeyMain;//, nState = 0;
	nKeyMain = pState->get_key_main();

	STLMnInt stlMnGroupParent;
	BaseState::group_id_get(_pdsvEvent, STRTOHASH("BaseIdentifierParent_an"), stlMnGroupParent);

	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nKeyMain);
	BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupParent);

	//STLVPairIntInt stlVParamList;
	//BaseState::event_state_attach_param(pState, _pdsvBase, _pdsvContext, &stlVParamList);

	//for(int i=0; i<(int)stlVParamList.size(); i++)
	//	pdsvEvent->add_alloc(stlVParamList[i].first, &stlVParamList[i].second);

	pSpace->post_systemevent(pdsvEvent);
	return 1;
}

STDEF_FUNC(BaseStateCastToEventValue_strF)
{
	const char* strHash;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateCastToEventValue_strF, strHash);
	int nHash = STRTOHASH(strHash);

	const int* pnId;
	if (!_pdsvEvent->get(nHash, (const void**)& pnId))
		return 0;

	const int* pnEventHash;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateCastToEventValue_nV, pnEventHash);
	STDEF_Manager(pManager);
	pManager->post_event_state(*pnEventHash, 0, 0, *pnId);

	return 1;
}

STDEF_FUNC(BaseStateIdentify_nSE)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateIdentify_nSE, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);

	int nKeyMain;//, nState = 0;

	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nKeyMain);
	BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), pState->group_id_get());

	//STLVPairIntInt stlVParamList;
	//BaseState::event_state_attach_param(pState, _pdsvBase, _pdsvContext, &stlVParamList);

	//for(int i=0; i<(int)stlVParamList.size(); i++)
	//	pdsvEvent->add_alloc(stlVParamList[i].first, &stlVParamList[i].second);

	pSpace->post_event(pdsvEvent);
	pState->EventSendReset(); // remove sent event so next casting will be sent new event.
	return 1;
}

STDEF_FUNC(BaseStateChild_nSE)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateChild_nSE, pnEvent);

	const int* pnIdChild;
	short nCnt;
	STDEF_AGET_R(_pdsvContext, BaseIdentifierChilds_an, pnIdChild, nCnt);

	STDEF_Space(pSpace);
	BaseDStructureValue* pdsvEvent;
	int nKeyMain;
	BaseState* pState = (BaseState*)_pdsvBase->m_pVoidParam;

	nKeyMain = pState->get_key_main();

	for (int i = 0; i < nCnt; i++)
	{
		pdsvEvent = pState->EventSendGet(*pnEvent);
		int nChild = pnIdChild[i];

		pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nKeyMain);
		pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalIdentifier), &nChild);

		//STLVPairIntInt stlVParamList;
		//BaseState::event_state_attach_param(pState, _pdsvBase, _pdsvContext, &stlVParamList);

		//for(int i=0; i<(int)stlVParamList.size(); i++)
		//	pdsvEvent->add_alloc(stlVParamList[i].first, &stlVParamList[i].second);

		pSpace->post_systemevent(pdsvEvent);
	}
	return 1;
}

STDEF_FUNC(BaseStateEventMouseCast_nF)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateEventMouseCast_nF, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);
	int nBroadCast = 0;

	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nBroadCast);

	pSpace->post_event(pdsvEvent);
	pState->EventSendReset(); // remove sent event so next casting will be sent new event.
	return 1;
}

STDEF_FUNC(BaseEventThreadReturn_nF)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseEventThreadReturn_nF, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);
	BaseDStructureValue* variable_state = pState->variable_get();
	int nBroadCast = 0;

	const int* pnTarget = NULL;
	STDEF_GET_R(variable_state, BaseTransitionThreadCaster, pnTarget);

	pdsvEvent->set_alloc(HASH_STATE(BaseEventTargetStateSerial), pnTarget);
	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nBroadCast);

	pSpace->post_event(pdsvEvent);
	pState->EventSendReset(); // remove sent event so next casting will be sent new event.
	return 1;
}

STDEF_FUNC(BaseStateEventGlobalReturn_nF)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateEventGlobalReturn_nF, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);
	BaseDStructureValue* variable_state = pState->variable_get();
	int nBroadCast = 0;

	const int* pnTarget;
	if (_pdsvEvent == NULL || !STDEF_GET(_pdsvEvent, RevStateEventCaster, pnTarget))
		STDEF_GET_R(variable_state, RevStateEventCaster, pnTarget);

	pdsvEvent->set_alloc(HASH_STATE(BaseEventTargetStateSerial), pnTarget);
	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nBroadCast);

	g_SendMessage(LOG_MSG, "BaseStateEventGlobalReturn_nF,return_serial=%d,event=%d", *pnTarget,*pnEvent);
	//STLVPairIntInt stlVParamList;
	//BaseState::event_state_attach_param(pState, _pdsvBase, _pdsvContext, &stlVParamList);

	//for (int i = 0; i<(int)stlVParamList.size(); i++)
	//	pdsvEvent->add_alloc(stlVParamList[i].first, &stlVParamList[i].second);

	pSpace->post_event(pdsvEvent);
	pState->EventSendReset(); // remove sent event so next casting will be sent new event.
	return 1;
}


STDEF_FUNC(BaseStateEventCastWithIdentifier_nF)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateEventCastWithIdentifier_nF, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);

	int nSerial = pState->obj_serial_get();
	pdsvEvent->set_alloc(HASH_STATE(RevStateEventCaster), &nSerial);

	STLMnInt stlMnGroup = BaseState::group_get(pState, _pdsvBase, _pdsvContext, _pdsvEvent);
	BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);

	pSpace->post_event(pdsvEvent);
	pSpace->post_event_ui(*pnEvent);

	pState->EventSendReset(); // remove sent event so next casting will be sent new event.
	return 1;
}

STDEF_FUNC(BaseStateEventPostpond_nF)
{
	STDEF_Manager(manager_p);

	manager_p->event_postpond();
	return 1;
}

STDEF_FUNC(BaseStateEventGlobalCast_nF)
{
	const int* pnEvent;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateEventGlobalCast_nF, pnEvent);

	STDEF_Space(pSpace);
	STDEF_BaseState(pState);
	BaseDStructureValue* pdsvEvent = pState->EventSendGet(*pnEvent);

	int nSerial = pState->obj_serial_get();
	pdsvEvent->set_alloc(HASH_STATE(RevStateEventCaster), &nSerial);
	const int *priority_pn = NULL;
	STDEF_GETLOCAL(_pdsvBase, BaseStateEventCastPriority_nV, priority_pn);
	if(priority_pn)
		pSpace->post_event(pdsvEvent, *priority_pn);
	else
		pSpace->post_event(pdsvEvent);
	pSpace->post_event_ui(*pnEvent);

	pState->EventSendReset(); // remove sent event so next casting will be sent new event.
	return 1;
}

int BaseState::ContextSave(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	const char* strFilename;
	if (!_pdsvContext->get(STRTOHASH("ContextFileSaveName_strV"), (const void**)& strFilename))
		return 0;

	char strFilenamePath[255];
	strcpy_s(strFilenamePath, 255, BaseStateManager::get_manager()->path_get_save());
	strcat_s(strFilenamePath, 255, strFilename);
	BaseDStructure* pdstFile;
	PT_OAlloc(pdstFile, BaseDStructure);
	if (!pdstFile->load(strFilenamePath))
	{
		pdstFile->add_column(0, "nKey", TYPE_INT32);
		pdstFile->add_column(0, "strName", TYPE_STRING);

		int nKey = 0;
		pdstFile->set_alloc(0, 0, &nKey);
		pdstFile->set_alloc(0, 1, "SaveContext");
	}

	const char* strNameContext;
	int nIndex = _pdsvContext->get_index(_nHash);
	strNameContext = _pdsvContext->get_base()->get_type_name(nIndex);

	if (pdstFile->get_index(_nHash) == -1)
	{
		pdstFile->add_column(BaseDStructure::VALUE_CONTEXT, strNameContext
			, _pdsvContext->get_base()->get_type(nIndex)
			, _pdsvContext->get_base()->get_type_size(nIndex));
	}

	pdstFile->set_alloc(0, pdstFile->get_index(_nHash), pVoidContext, _nCnt);
#ifdef _DEBUG
	pdstFile->save(NULL);
#else
	pdstFile->save(NULL, true);
#endif
	PT_OFree(pdstFile);
	return 1;
}

int BaseState::ContextLoad(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize)
{
	const char* strFilename;
	if (!_pdsvContext->get(STRTOHASH("ContextFileSaveName_strV"), (const void**)& strFilename))
		return 0;

	char strFilenamePath[255];
	strcpy_s(strFilenamePath, 255, BaseStateManager::get_manager()->path_get_save());
	strcat_s(strFilenamePath, 255, strFilename);

	BaseDStructure* pdstFile;
	PT_OAlloc(pdstFile, BaseDStructure);
	if (!pdstFile->load(strFilenamePath))
		return 0;

	const char* strNameContext;
	int nIndex = _pdsvContext->get_index(_nHash);
	strNameContext = _pdsvContext->get_base()->get_type_name(nIndex);

	if (pdstFile->get_index(_nHash) == -1)
	{
		pdstFile->add_column(BaseDStructure::VALUE_CONTEXT, strNameContext
			, _pdsvContext->get_base()->get_type(nIndex)
			, _pdsvContext->get_base()->get_type_size(nIndex));
	}

	const void* pValue;
	short	nCnt;
	if (pdstFile->get(0, pdstFile->get_index(_nHash), &pValue, &nCnt))
	{
		_pdsvContext->set_alloc(_nHash, pValue, nCnt);
	}
	PT_OFree(pdstFile);
	return 1;
}

STDEF_FUNC(BaseValueFileSave_strF)
{
	const char* strFilename;
	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseValueFileSave_strF"), (const void**)& strFilename))
		return 0;

	_pdsvContext->set_alloc(STRTOHASH("ContextFileSaveName_strV"), strFilename);

	int nRet = BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextSave);
	return nRet;
}

STDEF_FUNC(BaseValueFileLoad_strF)
{
	const char* strFilename;
	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseValueFileLoad_strF"), (const void**)& strFilename))
		return 0;

	_pdsvContext->set_alloc(STRTOHASH("ContextFileSaveName_strV"), strFilename);

	int nRet = BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextLoad);
	return nRet;
}

STDEF_FUNC(BaseValueFileKeep_strF)
{
	const char* strFilename;
	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseValueFileKeep_strF"), (const void**)& strFilename))
		return 0;

	_pdsvContext->set_alloc(STRTOHASH("ContextFileSaveName_strV"), strFilename);
	int nRet = 0;

	if (_nState == BaseDStructure::STATE_START)
	{
		nRet = BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextLoad);
	}
	else if (_nState == BaseDStructure::STATE_END) {
		nRet = BaseState::ContextLoopCallback(_pdsvBase, _pdsvContext, BaseState::ContextSave);
	}

	return nRet;
}

//==========================================================================
// Start  by OJ : 2011-08-12
// 특정 함수와 변수를 등록해 Update될 때 마다 호출되도록 한다.
// 파라메터 _pData는 PT_Alloc또는 PT_MAlloc로 할당한 메모리 공간이어야 한다
void BaseState::UpdateFuncRegist(BaseDStructureValue* _pdsvEvent, void* _pData, FnStateUpdate _fnUpdate)
{
	STStateUpdate	stUpdate;

	PT_OAlloc2(stUpdate.pdsvBase, BaseDStructureValue, m_pdsvState->get_base(), 1024);
	*stUpdate.pdsvBase = *m_pdsvState;

	if (_pdsvEvent)
	{
		PT_OAlloc2(stUpdate.pdsvEvent, BaseDStructureValue, _pdsvEvent->get_base(), _pdsvEvent->get_dump_size());
		*stUpdate.pdsvEvent = *_pdsvEvent;
	}
	else {
		stUpdate.pdsvEvent = NULL;
	}

	stUpdate.pVoidData = _pData;
	stUpdate.fnUpdate = _fnUpdate;

	m_stlVStStateUpdate.push_back(stUpdate);
}

void BaseState::UpdateFuncUpdate(UINT32 _nTimeDelta)
{
	unsigned i;
	BaseDStructureValue* pdsvContext = context_get();
	for (i = 0; i < m_stlVStStateUpdate.size(); i++)
		m_stlVStStateUpdate[i].fnUpdate(pdsvContext, m_stlVStStateUpdate[i].pdsvBase, m_stlVStStateUpdate[i].pdsvEvent, m_stlVStStateUpdate[i].pVoidData, _nTimeDelta);
}

void BaseState::UpdateFuncRelease()
{
	unsigned i;
	for (i = 0; i < m_stlVStStateUpdate.size(); i++)
	{
		//if(m_stlVStStateUpdate[i].pVoidData)
		//	PT_Free(m_stlVStStateUpdate[i].pVoidData); // void date should freed in BaseDStructure::STATE_END event.
		if (m_stlVStStateUpdate[i].pdsvBase)
			PT_OFree(m_stlVStStateUpdate[i].pdsvBase);
		if (m_stlVStStateUpdate[i].pdsvEvent)
			PT_OFree(m_stlVStStateUpdate[i].pdsvEvent);
	}
	m_stlVStStateUpdate.clear();
}
//
//void BaseState::temp_nextvalue_push(int _nHash, void *_pVoid)
//{
//	m_stlMnpNextTemps[_nHash] = _pVoid;
//}
//
//void BaseState::TempValuePush(int _nHash, void *_pVoid)
//{
//	m_stlMnpTemps[_nHash]	= _pVoid;
//}
//
//STLMnpVoid	&BaseState::TempValueGet()
//{
//	return m_stlMnpTemps;
//}
//
//void *BaseState::TempValueGet(int _nHash) const
//{
//	STLMnpVoid::const_iterator	it;
//	it	= m_stlMnpTemps.find(_nHash);
//	if(it == m_stlMnpTemps.end())
//		return NULL;
//	return it->second;
//}
//
//
//int BaseState::TempValueIntGet(int _nHash) const
//{
//    STLMnpVoid::const_iterator	it;
//	it	= m_stlMnpTemps.find(_nHash);
//	if(it == m_stlMnpTemps.end())
//		return 0;
//    INT64 n64Ret = (INT64)it->second;
//	return (int)n64Ret;
//}

#include "BaseMemoryPool.h"

BaseDStructureValue* BaseState::EventSystemGet()
{
	return m_pdsvEventSystem;
}

BaseDStructureValue* BaseState::EventSendGet(int _nEvent, bool _bNew)
{
	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	if (m_pdsvEventSend == NULL || _bNew)
	{
		m_pdsvEventSend = m_pSpace->get_manager()->make_event(nKeyState, _nEvent);
		//m_pdsvEventSend->ref_inc();

		if (g_logger())
		{
			static int hash = 0, serial = 900000;
			if (hash == 0) {
				hash = STRTOHASH("VSLoggerEventSerial_nV");
				m_pSpace->get_manager()->variable_define(hash, TYPE_INT32, true);
			}
			if(BaseDStructure::get_index(hash) == -1)
				m_pSpace->get_manager()->variable_define(hash, TYPE_INT32, true);

			serial++;
			m_pdsvEventSend->set_alloc(hash, &serial);
		}
	}
	if (_nEvent != 0)
		m_pdsvEventSend->set_alloc(nKeyState, &_nEvent);
	return m_pdsvEventSend;
}

void BaseState::EventSendReset()
{
	if (m_pdsvEventSend)
	{
		m_pdsvEventSend->ref_inc();
		m_pdsvEventSend->ref_dec();
		m_pdsvEventSend = NULL;
	}
}

int BaseState::identifier_get_auto()
{
	return s_nIdentifier++;
}

STLMnInt BaseState::identifier_set_next_auto(int _nGroupId)
{
	m_stlMNextGroupID[_nGroupId] = s_nIdentifier++;
	return m_stlMNextGroupID;
}

STLMnInt BaseState::identifier_reference_next()
{
	STLMnInt::iterator it, itFind;
	it = m_stlMnGroupIdentifier.begin();

	for (; it != m_stlMnGroupIdentifier.end(); it++)
	{
		if (m_stlMNextGroupID.find(it->first) == m_stlMNextGroupID.end()) {
			m_stlMNextGroupID[it->first] = it->second;
		}
	}
	return m_stlMNextGroupID;
}

STLMnInt BaseState::identifier_add_next(int _nGroupID, int _nIdentifier)
{
	m_stlMNextGroupID[_nGroupID] = _nIdentifier;
	return m_stlMNextGroupID;
}

bool BaseState::class_clear(int _class_hash)
{
	if (m_pSubState)
		m_pSubState->class_clear(_class_hash);

	BaseDStructureValue* variable_p = variable_get();
	if (variable_p == NULL)
		return false;
	variable_p->sfunc_clear(_class_hash);
	return true;
}

bool BaseState::class_allclear()
{
	if (m_pSubState)
		m_pSubState->class_allclear();

	BaseDStructureValue* variable_p = variable_get();
	if (variable_p == NULL)
		return false;
	variable_p->sfunc_clear();
	return true;
}

bool BaseState::class_regist(int _class_hash, const BaseObject* _class)
{
	BaseDStructureValue* variable_p = variable_get();
	if (variable_p == NULL)
		return false;
	variable_p->sfunc_set(_class_hash, (BaseObject*)_class);
	return true;
}

BaseObject* BaseState::class_get(int _class_hash)
{
	BaseDStructureValue* variable_p = variable_get();
	return variable_p->sfunc_get(_class_hash);
}

STDEF_FUNC(BaseIPFilterIn_strF)
{
	const char* strFilename, * strIP;
	STDEF_GETLOCAL_R(_pdsvBase, BaseIPFilterIn_strF, strFilename);
	STDEF_GETLOCAL_R(_pdsvContext, BaseIPFilterIP_strV, strIP);
	BaseResFilterIP* pFilter = (BaseResFilterIP*)BaseResManager::get_manager()->get_resource
	(strFilename, BaseResFilterIP::GetObjectId(), NULL, NULL, 0);

	if (pFilter->check_in_ip(strIP))
		return 1;

	return 0;
}

STDEF_FUNC(BaseIPFilterOut_strF)
{
	const char* strFilename, * strIP;
	STDEF_GETLOCAL_R(_pdsvBase, BaseIPFilterOut_strF, strFilename);
	STDEF_GETLOCAL_R(_pdsvContext, BaseIPFilterIP_strV, strIP);
	BaseResFilterIP* pFilter = (BaseResFilterIP*)BaseResManager::get_manager()->get_resource
	(strFilename, BaseResFilterIP::GetObjectId(), NULL, NULL, 0);

	if (pFilter->check_in_ip(strIP))
		return 0;

	return 1;

	return 1;
}

void dateTwo(const char* _strFormat, SPtDate* _ptmStart, SPtDate* _ptmEnd)
{
	BaseFile	paser;

	paser.set_asc_seperator("-:/~");
	paser.set_asc_deletor(" \t");
	paser.OpenFile((void*)_strFormat, (UINT32)strlen(_strFormat));

	SPtDate	tmStart, tmEnd;
	memset(&tmStart, 0, sizeof(SPtDate));
	memset(&tmEnd, 0, sizeof(SPtDate));
	int nValue;

	if (paser.read_asc_line())
	{
		if (paser.read_asc_integer(&nValue))
		{
			tmStart.s.year = nValue;
			if (paser.get_seperator_last() != '~')
			{
				if (paser.read_asc_integer(&nValue))
				{
					tmStart.s.month = nValue;
					if (paser.get_seperator_last() != '~')
					{
						if (paser.read_asc_integer(&nValue))
							tmStart.s.day = nValue;
					}
				}
			}
		}
	}

	if (paser.read_asc_integer(&nValue))
	{
		tmEnd.s.year = nValue;
		if (paser.read_asc_integer(&nValue))
		{
			tmEnd.s.month = nValue;
			if (paser.read_asc_integer(&nValue))
				tmEnd.s.day = nValue;
		}
	}

	*_ptmStart = tmStart;
	*_ptmEnd = tmEnd;
}


void timeTwo(const char* _strFormat, SPtTime* _ptmStart, SPtTime* _ptmEnd)
{
	BaseFile	paser;

	paser.set_asc_seperator("-:/~");
	paser.set_asc_deletor(" \t");
	paser.OpenFile((void*)_strFormat, (UINT32)strlen(_strFormat));

	SPtTime	tmStart, tmEnd;
	memset(&tmStart, 0, sizeof(SPtTime));
	memset(&tmEnd, 0, sizeof(SPtTime));
	int nValue;

	if (paser.read_asc_line())
	{
		if (paser.read_asc_integer(&nValue))
		{
			tmStart.s.nHour = nValue;
			if (paser.get_seperator_last() != '~')
			{
				if (paser.read_asc_integer(&nValue))
				{
					tmStart.s.nMinute = nValue;
					if (paser.get_seperator_last() != '~')
					{
						if (paser.read_asc_integer(&nValue))
							tmStart.s.nSecond = nValue;
					}
				}
			}
		}
	}

	if (paser.read_asc_integer(&nValue))
	{
		tmEnd.s.nHour = nValue;
		if (paser.read_asc_integer(&nValue))
		{
			tmEnd.s.nMinute = nValue;
			if (paser.read_asc_integer(&nValue))
				tmEnd.s.nSecond = nValue;
		}
	}

	*_ptmStart = tmStart;
	*_ptmEnd = tmEnd;
}

STDEF_FUNC(BaseDateTimeEvent_strRE)
{
	const char* strDateTime;
	STDEF_GETLOCAL_R(_pdsvBase, BaseDateTimeEvent_strRE, strDateTime);

	SPtDateTime tm;
	BaseTime::pase_loop_datetime(strDateTime, &tm);
	SPtDateTime	tmCur;
	BaseSystem::timeCurrent(&tmCur);
	STDEF_Space(pSpace);
	STDEF_BaseState(pState);

	STStateActor	actor;
	actor.nKey = STRTOHASH("BaseDateTimeEvent_strRE");
	actor.nBoolIndex = _pdsvBase->bool_index_get();
	actor.pdstActor = pState->link_get(_pdsvBase);
	actor.pState = NULL;

	if (_nState == BaseDStructure::STATE_LINK_START)
	{
		if (tm < tmCur)
			return 0;

		INT64 nTimeout = (INT64)(tm - tmCur);
		nTimeout = nTimeout * 1000 + (INT64)BaseSystem::timeGetTime();
		UINT32 nMax = -1;
		if (nTimeout > (INT64)nMax)
		{
			g_SendMessage(LOG_MSG_POPUP, "Time out rang over");
			return 0;
		}

		actor.nType = STStateActor::HASH;
		pSpace->actor_event_release(actor);

		actor.nValue = (UINT32)nTimeout;
		actor.nType = STStateActor::TIMEOUT;
		actor.nSpaceID = pSpace->space_id_get();
		actor.pState = pState;
		pSpace->actor_event_regist(actor);
		return 0;
	}

	//if(tm <= tmCur)
	return 1;
}

STDEF_FUNC(BaseDateIn_strF)
{
	const char* strDateTime;

	STDEF_GETLOCAL_R(_pdsvBase, BaseDateIn_strF, strDateTime);
	SPtDate tmStart, tmEnd;
	dateTwo(strDateTime, &tmStart, &tmEnd);
	SPtDateTime	tmCur;
	BaseSystem::timeCurrent(&tmCur);

	if (tmCur.s.sDate >= tmStart
		&& tmCur.s.sDate <= tmEnd)
		return 1;

	return 0;
}

STDEF_FUNC(BaseDateOut_strF)
{
	const char* strDateTime;

	STDEF_GETLOCAL_R(_pdsvBase, BaseDateOut_strF, strDateTime);
	SPtDate tmStart, tmEnd;
	dateTwo(strDateTime, &tmStart, &tmEnd);
	SPtDateTime	tmCur;
	BaseSystem::timeCurrent(&tmCur);

	if (tmCur.s.sDate >= tmStart
		&& tmCur.s.sDate <= tmEnd)
		return 0;

	return 1;
}


STDEF_FUNC(BaseDateTimeIn_strF)
{
	const char* strDateTime;

	STDEF_GETLOCAL_R(_pdsvBase, BaseDateTimeIn_strF, strDateTime);
	SPtTime tmStart, tmEnd;
	timeTwo(strDateTime, &tmStart, &tmEnd);
	SPtTime	tmCur;
	BaseSystem::timeCurrent(&tmCur);

	if (tmCur >= tmStart
		&& tmCur <= tmEnd)
		return 1;

	return 0;
}

STDEF_FUNC(BaseDateTimeOut_strF)
{
	const char* strDateTime;

	STDEF_GETLOCAL_R(_pdsvBase, BaseDateTimeOut_strF, strDateTime);
	SPtTime tmStart, tmEnd;
	timeTwo(strDateTime, &tmStart, &tmEnd);
	SPtTime	tmCur;
	BaseSystem::timeCurrent(&tmCur);

	if (tmCur >= tmStart
		&& tmCur <= tmEnd)
		return 0;

	return 1;
}

STDEF_FUNC(BaseDateDayIn_anF)
{
	const int* pnwDay;
	short nCnt = 0;

	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseDateDayIn_anF"), (const void**)& pnwDay, &nCnt))
		return 0;

	int nwDayCur;

	nwDayCur = BaseSystem::weekDay();
	for (int i = 0; i < nCnt; i++)
	{
		if (nwDayCur == *(pnwDay + i))
			return 1;
	}

	return 0;
}

STDEF_FUNC(BaseDateDayOut_anF)
{
	const int* pnwDay;
	short nCnt = 0;

	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseDateDayOut_anF"), (const void**)& pnwDay, &nCnt))
		return 0;

	int nwDayCur;

	nwDayCur = BaseSystem::weekDay();
	for (int i = 0; i < nCnt; i++)
	{
		if (nwDayCur == *(pnwDay + i))
			return 0;
	}

	return 1;
}

STDEF_FUNC(BaseRandom_fIf)
{
	const float* pfRandom;
	STDEF_GETLOCAL_R(_pdsvBase, BaseRandom_fIf, pfRandom);

	int nRandom = (int)(*pfRandom * RAND_MAX / 100.f);

	int nRndSrc = BaseSystem::rand() + 1;

	if (nRandom > nRndSrc)
		return 1;
	return 0;
}

STDEF_FUNC(BaseStartApplCompareDateOver_nIf)
{
	const int* pnDay;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStartApplCompareDateOver_nIf, pnDay);

	const char* strDateFile = "english_p.t_1_0_daterecord.ini";
	char strFile[255];

	BaseSystem::path_data_get(strFile, 255, strDateFile);

	BaseFile	file;

	SPtDateTime datetime;
	char strCurDate[255];

	if (!file.is_exist(strFile))
	{
		char strDate[255];

		BaseSystem::timeCurrent(&datetime);
		BaseTime::make_date_time(datetime.dateTime, strDate);

		if (!file.OpenFile(strFile, BaseFile::OPEN_WRITE))
		{
			file.write_asc_string(strDate, 255);
			file.write_asc_line();
		}
		return 0;
	}
	else {
		if (!file.OpenFile(strFile, BaseFile::OPEN_READ))
		{
			file.read_asc_line();
			file.read_asc_string(strCurDate, 255);
		}
	}

	datetime.dateTime = BaseTime::parse_date_time(strCurDate);

	for (int i = 0; i < *pnDay; i++)
		datetime.s.sDate.date = BaseTime::day_increase(datetime.s.sDate.date);

	SPtDateTime datecur;
	BaseSystem::timeCurrent(&datecur);
	if (datecur > datetime)
		return 1;

	return 0;
}

STDEF_FUNC(BaseFalse_nIf)
{
	return 0;
}

STDEF_FUNC(BaseStateReturn_nF)
{
	STDEF_Manager(pManager);
	const int* pnRetValue;
	const int* pnHash;
	STDEF_GETLOCAL_R(_pdsvEvent, BaseStateEventGlobal, pnHash);
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateReturn_nF, pnRetValue);

	pManager->ret_value_set(*pnHash, (void*)pnRetValue);
	return 1;
}

//STDEF_FUNC(BaseStateReturnSet_nF)

STDEF_FUNC(BaseStateReturnAdd_nF)
{
	STDEF_Manager(pManager);
	const int* pnHash;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateReturnAdd_nF, pnHash);

	const INT64* pnValue;
	if (!STDEF_GETLOCAL(_pdsvBase, BaseStateReturnSetState_nV, pnValue))
	{
		if (!STDEF_GETLOCAL(_pdsvBase, BaseStateReturnSet_vV, pnValue))
		{
			STDEF_GETLOCAL_R(_pdsvBase, BaseStateReturnSet_nV, pnValue);
		}
		else {
			const int* pnVariable = (const int*)pnValue;
			if (!_pdsvContext->get(*pnVariable, (const void**)& pnValue))
				return 0;
		}
		INT64 nCurValue = (INT64)pManager->ret_value_get(*pnHash);
		nCurValue += *pnValue;
		pManager->ret_value_set(*pnHash, (void*)nCurValue);
	}
	else {
		INT64 nCurValue = (INT64)pManager->ret_value_get(*pnHash);
		nCurValue += *pnValue;
		pManager->ret_value_set(*pnHash, (void*)nCurValue);
		STDEF_GETLOCAL(_pdsvBase, BaseStateReturnSet_nV, pnValue);
		//nCurValue = (int)pManager->ret_value_get(HASH_STATE(ParamInt));
		//nCurValue += (int)*pnValue;
		//pManager->ret_value_set(HASH_STATE(ParamInt), (void*)nCurValue);
	}
	return 1;
}

STDEF_FUNC(BaseStateReturnSet_nF)
{
	STDEF_Manager(pManager);
	const int* pnHash;
	STDEF_GETLOCAL_R(_pdsvBase, BaseStateReturnSet_nF, pnHash);

	const INT64* pnValue;
	if (!STDEF_GETLOCAL(_pdsvBase, BaseStateReturnSetState_nV, pnValue))
	{
		STDEF_GETLOCAL_R(_pdsvBase, BaseStateReturnSet_nV, pnValue);
		pManager->ret_value_set(*pnHash, (void*)* pnValue);
	}
	else {
		pManager->ret_value_set(*pnHash, (void*)* pnValue);
		//STDEF_GETLOCAL(_pdsvBase, BaseStateReturnSet_nV, pnValue);
		//pManager->ret_value_set(HASH_STATE(ParamInt), (void*) *pnValue);
	}
	return 1;
}
//
//STDEF_FUNC(BaseStateSerial_nIf)
//{
//	const int *pnParamSerial;
//	if(!_pdsvEvent->get(HASH_STATE(SelectorSerial_nV), (const void**)&pnParamSerial))
//		return 0;
//
//	BaseState			*pState	= (BaseState*)_pdsvBase->m_pVoidParam;
//	
//	int nSerial = (int)pState->TempValueIntGet(HASH_STATE(SelectorSerial_nV));
//	if(nSerial == *pnParamSerial)
//		return 1;
//
//	return 0;
//}
//
//STDEF_FUNC(BaseStateParamInt_nIf)
//{
//	const int *pnParam;
//	STDEF_GETLOCAL_R(_pdsvBase, BaseStateParamInt_nIf, pnParam);
//
//	int nParamEvent;
//	if(!_pdsvEvent->get("ParamInt", &nParamEvent))
//		return 0;
//
//	if(*pnParam == nParamEvent)
//		return 1;
//
//	return 0;
//}

//STDEF_FUNC(BaseStateParamInt_anIf)
//{
//	const int *pnParams;
//	short nCount;
//
//	STDEF_AGET_R(_pdsvBase, BaseStateParamInt_anIf, pnParams,nCount);
//	int nParamEvent;
//	if(!_pdsvEvent->get("ParamInt", &nParamEvent))
//		return 0;
//
//	for(short i = 0; i < nCount; ++i)
//	{
//		if(nParamEvent == pnParams[i])
//		{
//			return 1;
//		}
//	}
//
//	return 0;
//}
//
//STDEF_FUNC(BaseStateParamInt_nIfMore)
//{
//	const int *pnParam;
//	STDEF_GETLOCAL_R(_pdsvBase, BaseStateParamInt_nIfMore, pnParam);
//
//	int nParamEvent;
//	if(!_pdsvEvent->get("ParamInt", &nParamEvent))
//		return 0;
//
//	if(*pnParam < nParamEvent)
//		return 1;
//
//	return 0;
//}
//
//STDEF_FUNC(BaseStateParamInt_nIfLess)
//{
//	const int *pnParam;
//	STDEF_GETLOCAL_R(_pdsvBase, BaseStateParamInt_nIfLess, pnParam);
//
//	int nParamEvent;
//	if(!_pdsvEvent->get("ParamInt", &nParamEvent))
//		return 0;
//
//	if(*pnParam > nParamEvent)
//		return 1;
//
//	return 0;
//}

STDEF_FUNC(BaseVariableIdentifierAdd_nF)
{
	const int* pnGroup;
	STDEF_GETLOCAL_R(_pdsvBase, BaseVariableIdentifierAdd_nF, pnGroup);

	const int* pnId = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvEvent);
	if (pnId == NULL)
		return 0;

	STDEF_BaseState(pState);

	pState->group_id_add(*pnGroup, *pnId);
	return 1;
}

STDEF_FUNC(BaseIdentifierNextAutoSet_nF)
{
	const int* pnGroupId;
	BaseState* pState = (BaseState*)_pdsvBase->m_pVoidParam;

	STDEF_GETLOCAL_R(_pdsvBase, BaseIdentifierNextAutoSet_nF, pnGroupId);
	pState->identifier_set_next_auto(*pnGroupId);

	return 1;
}
//
//STDEF_FUNC(BaseEventToStateTemp_nF)
//{
//	const int *pnKey;
//	const INT64 *pnValue;
//	STDEF_GETLOCAL_R(_pdsvBase, BaseEventToStateTemp_nF, pnKey);
//	if(!_pdsvEvent->get(*pnKey, (const void **)&pnValue))
//		return 0;
//	BaseState			*pState	= (BaseState*)_pdsvBase->m_pVoidParam;
//	
//	pState->TempValuePush(*pnKey, (void*)*pnValue);
//	return 1;
//}
//
//STDEF_FUNC(BaseEventToStateNextTemp_nF)
//{
//	const int *pnKey;
//	const INT64 *pnValue;
//	STDEF_GETLOCAL_R(_pdsvBase, BaseEventToStateNextTemp_nF, pnKey);
//	if(!_pdsvEvent->get(*pnKey, (const void **)&pnValue))
//		return 0;
//	BaseState			*pState	= (BaseState*)_pdsvBase->m_pVoidParam;
//	pState->temp_nextvalue_push(*pnKey, (void*)*pnValue);
//	return 1;
//}

STDEF_FUNC(BaseStateActiveShowAll_nF)
{
#ifdef _DEBUG
	BaseStateManager::get_manager()->state_active_show_all();
#endif
	return 1;
}

STDEF_FUNC(BasePrintString_strF)
{
	const char* strOut;
	STDEF_GETLOCAL_R(_pdsvBase, BasePrintString_strF, strOut);

	char strOut2[4096];

	strcpy_s(strOut2, 4096, strOut);
	BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strOut2, 4096);

	g_SendMessage(LOG_MSG, strOut2);
	printf("%s\n", strOut2);
	return 1;
}

#define STDEF_VARIABLE_ACCUM(colum, func, pManager) \
	const int *panTypes;\
	short	nCnt;\
	if(!_pdsvBase->get_local_seq(STRTOHASH(#colum),(const void**)&panTypes, &nCnt)) return 0;\
	\
	BaseDStructureValue	*pdsvV1, *pdsvV2 = NULL;\
	pdsvV1	= pManager->variable_type(panTypes[0], _pdsvBase, _pdsvContext, _pdsvEvent);\
	if(nCnt > 1){\
		pdsvV2	= pManager->variable_type(panTypes[1], _pdsvBase, _pdsvContext, _pdsvEvent, false);\
	}\
	if(pdsvV1 == NULL)\
		return 0;\
	\
	int nRet = BaseState::VariableLoopCallbackInt(_pdsvBase, pdsvV1, pdsvV2, BaseState::Context##func);\
	if(nCnt >= 1 && panTypes[0] == HASH_STATE(File)){\
		const char *strFilename;\
		char strBuf[4096];\
		if (!_pdsvBase->get(HASH_STATE(BaseVariableFilename_strV), (const void**)&strFilename))\
			return 0;\
		strcpy_s(strBuf, 4096, BaseStateManager::get_manager()->path_get_save());\
		strcat_s(strBuf, 4096, strFilename);\
		pdsvV1->dump_save(strBuf);\
		PT_OFree(pdsvV1);\
	}\
	if (nCnt >= 2 && panTypes[1] == HASH_STATE(File)) {\
				PT_OFree(pdsvV2);\
	}\
	return nRet;

#define STDEF_VARIABLE_BOOL(colum, func, pManager) \
const int *panTypes;\
	short	nCnt;\
	if(!_pdsvBase->get_local_seq(STRTOHASH(#colum),(const void**)&panTypes, &nCnt)) return 0;\
	\
	BaseDStructureValue	*pdsvV1, *pdsvV2 = NULL;\
	if(nCnt == 1){\
		pdsvV1	= pManager->variable_type(panTypes[0], _pdsvBase, _pdsvContext, _pdsvEvent);\
	}else{\
		pdsvV1	= pManager->variable_type(panTypes[0], _pdsvBase, _pdsvContext, _pdsvEvent);\
		pdsvV2	= pManager->variable_type(panTypes[1], _pdsvBase, _pdsvContext, _pdsvEvent, false);\
	}\
	if(pdsvV1 == NULL)\
		return 0;\
	\
	int nRet = BaseState::VariableLoopCallbackInt(_pdsvBase, pdsvV1, pdsvV2, BaseState::Context##func);\
	if(nCnt >= 1 && panTypes[0] == HASH_STATE(File)){\
		const char *strFilename;\
		char strBuf[4096];\
		if (!_pdsvBase->get(HASH_STATE(BaseVariableFilename_strV), (const void**)&strFilename))\
			return 0;\
		strcpy_s(strBuf, 4096, BaseStateManager::get_manager()->path_get_save());\
		strcat_s(strBuf, 4096, strFilename);\
		pdsvV1->dump_save(strBuf);\
		PT_OFree(pdsvV1);\
	}\
	if (nCnt >= 2 && panTypes[1] == HASH_STATE(File)) {\
				PT_OFree(pdsvV2);\
	}\
	return nRet;

STDEF_FUNC(BaseVariableMoveGroup_anF)
{
	const int* panTypes;
	short	nCnt;
	STDEF_AGETLOCAL_R(_pdsvBase, BaseVariableMoveGroup_anF, panTypes, nCnt);
	STDEF_Manager(pManager);
	BaseDStructureValue* pdsvV1, * pdsvV2 = NULL;
	pdsvV1 = pManager->variable_type(panTypes[0], _pdsvBase, _pdsvContext, _pdsvEvent);
	if (nCnt > 1) {
		pdsvV2 = pManager->variable_type(panTypes[1], _pdsvBase, _pdsvContext, _pdsvEvent, false);
	}
	if (pdsvV1 == NULL || pdsvV2 == NULL)
		return 0;

	int	nHash;

	const void* pnValue;
	int i = 0;
	bool	bStart = false;

	for (; i < pdsvV2->get_count_appended(); i++)
	{
		nHash = pdsvV2->get_colum(i);
		if (!bStart && nHash != HASH_STATE(RevGroupStart))
			continue;

		bStart = true;

		if (pdsvV2->get(&pnValue, &nCnt, i))
		{
			pdsvV1->set_alloc(nHash, NULL);
			pdsvV1->set_alloc(nHash, pnValue, nCnt);

			//if (g_logger() && _pdsvBase->m_logevent_p)
			//	_pdsvBase->m_logevent_p->logger_hand_variable(nHash, pnValue, nCnt);
		}

		if (nHash == HASH_STATE(RevGroupEnd))
			break;
	}

	return 1;
}

STDEF_FUNC(BaseVariableFileExist_anIf)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableFileExist_anIf, FileExist, pManager);
}

STDEF_FUNC(BaseVariableFileSizeGet_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFileSizeGet_anF, FileSizeGet, pManager);
}

STDEF_FUNC(BaseVariableFileCopy_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFileCopy_anF, FileCopy, pManager);
}

STDEF_FUNC(BaseVariableFileRename_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFileRename_anF, FileRename, pManager);
}

STDEF_FUNC(BaseVariableFileDelete_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFileDelete_anF, FileDelete, pManager);
}

STDEF_FUNC(BaseVariableFolderRename_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFolderRename_anF, FolderRename, pManager);
}

STDEF_FUNC(BaseVariableFolderDelete_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFolderDelete_anF, FolderDelete, pManager);
}

STDEF_FUNC(BaseVariableFolderCreate_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFolderCreate_anF, FolderCreate, pManager);
}

STDEF_FUNC(BaseVariableFolderCurChange_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFolderCurChange_anF, FolderCurChange, pManager);
}

STDEF_FUNC(BaseVariableFolderCurGet_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableFolderCurGet_anF, FolderCurGet, pManager);
}

STDEF_FUNC(BaseVariableSetHash_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableSetHash_anF, SetHash, pManager);
}

STDEF_FUNC(BaseVariableStringUpdate_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableStringUpdate_anF, StringUpdate, pManager);
}

STDEF_FUNC(BaseVariableSetFormat_astrF)
{
	const char* str_a;
	STDEF_GETLOCAL_R(_pdsvBase, BaseVariableSetFormat_astrF, str_a);

	BaseDStructureValue* pVariable = ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();

	char var[255];
	int cnt = 0;
	while (*str_a != ',')
	{
		var[cnt] = *str_a;
		str_a++;
		cnt++;
	}
	var[cnt] = 0;
	str_a++;// skip ','
	while (*str_a == ' ') // skip space
		str_a++;

	int hash = STRTOHASH(var);

	int nLen = (int)strlen(str_a) + 1024;
	char* strOut = PT_Alloc(char, nLen);
	strcpy_s(strOut, nLen, str_a);
	char* strOut2 = BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strOut, nLen);

	pVariable->set_alloc(hash, (const void*)strOut2);
	PT_Free(strOut);

	if (strOut2 == NULL)
		return 0;
	return 1;
}

STDEF_FUNC(BaseVariableSetString_nF)
{
	const char* strBuffer;
	const int* pnHash;
	STDEF_GETLOCAL_R(_pdsvBase, BaseVariableSetString_nF, pnHash);

	BaseDStructureValue* pVariable = ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();

	if (!pVariable->get(*pnHash, (const void**)& strBuffer))
		return 0;

	int nLen = (int)strlen(strBuffer) + 1024;
	char* strOut = PT_Alloc(char, nLen);
	strcpy_s(strOut, nLen, strBuffer);
	char* strOut2 = BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strOut, nLen);

	pVariable->set_alloc(*pnHash, (const void*)strOut2);
	PT_Free(strOut);

	if (strOut2 == NULL)
		return 0;
	return 1;
}
//
//STDEF_FUNC(BaseVariableSetPassword_anF)
//{
//	STDEF_Manager(pManager);
//	STDEF_VARIABLE_ACCUM(BaseVariableSetPassword_anF, SetPassword, pManager);
//}

STDEF_FUNC(BaseVariableSet_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableSet_anF, Set, pManager);
}

STDEF_FUNC(BaseVariablePass_anF)
{
	const int* panTypes;
	short	nCnt;
	STDEF_Manager(pManager);

	if (!_pdsvBase->get_local_seq(STRTOHASH("BaseVariablePass_anF"), (const void**)&panTypes, &nCnt))
		return 0; 

	if (nCnt < 2)
		return 0;

	BaseDStructureValue* pdsvV1, * pdsvV2 = NULL;
	pdsvV1 = pManager->variable_type(panTypes[0], _pdsvBase, _pdsvContext, _pdsvEvent);
	pdsvV2 = pManager->variable_type(panTypes[1], _pdsvBase, _pdsvContext, _pdsvEvent, false);

	pdsvV2->variable_transit(pdsvV1);
	pdsvV1->set_alloc(HASH_STATE(TIMETOPROCESS), NULL);
	return 1;
}

STDEF_FUNC(BaseVariableSet_anF_Add)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableSet_anF_Add, Set, pManager);
}

STDEF_FUNC(BaseVariableIs_anIf)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableIs_anIf, CheckIs, pManager);
}

STDEF_FUNC(BaseVariableStack_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableStack_anF, Stack, pManager);
}

STDEF_FUNC(BaseVariableAdd_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableAdd_anF, Add, pManager);
}

STDEF_FUNC(BaseVariableMulti_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableMulti_anF, Multi, pManager);
}

STDEF_FUNC(BaseVariableSub_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_ACCUM(BaseVariableSub_anF, Sub, pManager);
}

STDEF_FUNC(BaseVariableNotEqual_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableNotEqual_anF, CheckNotEqual, pManager);
}

STDEF_FUNC(BaseVariableEqual_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableEqual_anF, CheckEqual, pManager);
}

STDEF_FUNC(BaseVariableEqualLeft_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableEqualLeft_anF, CheckEqualLeft, pManager);
}

STDEF_FUNC(BaseVariableBigger_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableBigger_anF, CheckUp, pManager);
}

STDEF_FUNC(BaseVariableEqSmaller_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableEqSmaller_anF, CheckEqDown, pManager);
}

STDEF_FUNC(BaseVariableEqBigger_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableEqBigger_anF, CheckEqUp, pManager);
}

STDEF_FUNC(BaseVariableSmaller_anF)
{
	STDEF_Manager(pManager);
	STDEF_VARIABLE_BOOL(BaseVariableSmaller_anF, CheckDown, pManager);
}

STDEF_FUNC(BaseBreakStart_strF)
{
	const char* strBuffer;
	STDEF_GETLOCAL_R(_pdsvBase, BaseBreakStart_strF, strBuffer);

	STDEF_BaseState(pState);
	STLMnInt stlMnGroupId;
	stlMnGroupId = pState->group_id_get();
	STLMnInt::iterator it = stlMnGroupId.begin();
	char strBuf[255], strCat[255] = { 0 };
	for (; it != stlMnGroupId.end(); it++)
	{
		sprintf_s(strBuf, 255, ":%d:%d", it->first, it->second);
		strcat_s(strCat, 255, strBuf);
	}
	g_SendMessage(LOG_MSG_FILELOG, "%s::%s\n", strCat, strBuffer);
	return 1;
}

STDEF_FUNC(BaseTimeGet_varF)
{
	const int* pnHashVariable = NULL;

	STDEF_GETLOCAL_R(_pdsvBase, BaseTimeGet_varF, pnHashVariable);
	if (pnHashVariable == NULL)
		return 0;

	STDEF_BaseState(pState);
	
	int time = BaseSystem::timeGetTime();
	BaseDStructureValue *variables = pState->variable_get();
	variables->set_alloc(*pnHashVariable, &time);
	return 1;
}

STDEF_FUNC(BaseBreakAppend_strF)
{
	const char* strBuffer;
	STDEF_GETLOCAL_R(_pdsvBase, BaseBreakAppend_strF, strBuffer);
	STDEF_BaseState(pState);
	STLMnInt stlMnGroupId;
	stlMnGroupId = pState->group_id_get();
	STLMnInt::iterator it = stlMnGroupId.begin();
	char strBuf[255], strCat[255] = { 0 };
	for (; it != stlMnGroupId.end(); it++)
	{
		sprintf_s(strBuf, 255, ":%d:%d", it->first, it->second);
		strcat_s(strCat, 255, strBuf);
	}
	g_SendMessage(LOG_MSG_FILELOG, "%s::%s\n", strCat, strBuffer);
	return 1;
}

STDEF_FUNC(BaseBreakComplete_strF)
{
	const char* strBuffer;
	STDEF_GETLOCAL_R(_pdsvBase, BaseBreakComplete_strF, strBuffer);
	STDEF_BaseState(pState);
	STLMnInt stlMnGroupId;
	stlMnGroupId = pState->group_id_get();
	STLMnInt::iterator it = stlMnGroupId.begin();
	char strBuf[255], strCat[255] = { 0 };
	for (; it != stlMnGroupId.end(); it++)
	{
		sprintf_s(strBuf, 255, ":%d:%d", it->first, it->second);
		strcat_s(strCat, 255, strBuf);
	}
	g_SendMessage(LOG_MSG_FILELOG, "%s::%s\n", strCat, strBuffer);
	return 1;
}

STDEF_FUNC(BaseMessagebox_strF)
{
	const char* strBuffer;
	STDEF_GETLOCAL_R(_pdsvBase, BaseMessagebox_strF, strBuffer);

	char strOut2[4096];

	strcpy_s(strOut2, 4096, strBuffer);
	BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strOut2, 4096);

	g_SendMessage(LOG_MSG_POPUP, strOut2);
	return 1;
}

STDEF_FUNC(BaseConsolHoldEnter_strF)
{
	const char* strBuffer;
	STDEF_GETLOCAL_R(_pdsvBase, BaseConsolHoldEnter_strF, strBuffer);

	char strOut[4096];
	strcpy_s(strOut, 4096, strBuffer);
	char* strOut2 = BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strOut, 4096);

	if (strOut2)
		printf("%s", strOut2);
	getchar();
	return 1;
}

STDEF_FUNC(BaseEventGroupHas_nIf)
{
	const int* pnGroup;
	STDEF_GETLOCAL_R(_pdsvBase, BaseEventGroupHas_nIf, pnGroup);
	const int* pnId = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvEvent);
	
	STLMnInt stlMnGroupId;
	BaseState::group_id_get(_pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);

	STLMnInt::iterator it;
	it = stlMnGroupId.find(*pnGroup);

	if (it == stlMnGroupId.end())
		return 0;

	if (pnId && it->second != *pnId)
		return 0;

	return 1;
}

STDEF_FUNC(BaseIdentifierCheckExist_nIf)
{
	const int* pnGroup;

	STDEF_GETLOCAL_R(_pdsvBase, BaseIdentifierCheckExist_nIf, pnGroup);
	const int* pnId = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvEvent);
	if (pnId == NULL)
		return 0;

	STDEF_Space(pSpace);

	STLMnInt stlMnGroup;

	stlMnGroup[*pnGroup] = *pnId;
	if (pSpace->is_active_id(&stlMnGroup))
		return 1;
	return 0;
}

STDEF_FUNC(BaseVariableSetByIdentifier_nF)
{
	const int* pnGroup, * pnHashVariable;

	STDEF_GETLOCAL_R(_pdsvBase, BaseVariableSetByIdentifier_nF, pnGroup);
	STDEF_FGETLOCAL_R(_pdsvBase, BaseVariableRefer_anV, pnHashVariable);

	STDEF_BaseState(pState);
	int nId = pState->identifier_get(*pnGroup);

	if (nId)
	{
		STDEF_Manager(pManager);
		BaseDStructureValue* pdsvEvent = pState->EventSendGet();
		pManager->variable_define(*pnHashVariable, TYPE_INT32);
		pdsvEvent->set_alloc(*pnHashVariable, &nId);
		return 1;
	}

	return 0;
}

STDEF_FUNC(BaseStateEventGlobalSetIdentifier_nF)
{
	const int* pnGroup;

	STDEF_GETLOCAL_R(_pdsvBase, BaseStateEventGlobalSetIdentifier_nF, pnGroup);

	int nId = 0;
	const int* pnId = NULL;
	STDEF_BaseState(pState);
	pnId = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvEvent);
	if (pnId == NULL)
	{
		nId = pState->identifier_get(*pnGroup);
		if (nId != 0)
			pnId = &nId;
	}

	if (pnId) {
		STLMnInt stlMnGroup;
		BaseDStructureValue* pdsvEvent = pState->EventSendGet();
		stlMnGroup[*pnGroup] = *pnId;
		BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);
		return 1;
	}


	return 0;
}

STDEF_FUNC(BaseIdentifierSetByEventVariable_nF)
{
	const int* pnGroup;

	STDEF_GETLOCAL_R(_pdsvBase, BaseIdentifierSetByEventVariable_nF, pnGroup);
	const int* pnId = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvEvent);
	if (pnId == NULL)
		return 0;
	//STDEF_FGETLOCAL_R(_pdsvBase, BaseVariableRefer_anV, pnHashVariable);

	STDEF_BaseState(pState);

	pState->group_id_add(*pnGroup, *pnId);
	return 1;
}

STDEF_FUNC(BaseIdentifierNextSetByEventVariable_nF)
{
	const int* pnGroup;

	STDEF_GETLOCAL_R(_pdsvBase, BaseIdentifierNextSetByEventVariable_nF, pnGroup);
	const int* pnId = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, _pdsvEvent);
	//STDEF_FGETLOCAL_R(_pdsvBase, BaseVariableRefer_anV, pnHashVariable);

	STDEF_BaseState(pState);
	// -779358450
	if (pnId)
	{
		pState->identifier_reference_next();
		pState->identifier_add_next(*pnGroup, *pnId);
		return 1;
	}
	return 0;
}

STDEF_FUNC(BaseTimeInAdditionCheck_nIf)
{
	const int* pnTimeIn;
	STDEF_GETLOCAL_R(_pdsvBase, BaseTimeInAdditionCheck_nIf, pnTimeIn);

	{
		BaseState* pState = ((BaseState*)_pdsvBase->m_pVoidParam);

		if (pState->get_time_local() <= (UINT32)* pnTimeIn)
			return 1;
	}
	return 0;
}

STDEF_FUNC(BaseTimeOutAdditionCheck_nIf)
{
	const int* pnTimeIn;
	STDEF_GETLOCAL_R(_pdsvBase, BaseTimeOutAdditionCheck_nIf, pnTimeIn);

	{
		BaseState* pState = ((BaseState*)_pdsvBase->m_pVoidParam);

		if (pState->get_time_local() > (UINT32)* pnTimeIn)
			return 1;
	}
	return 0;
}

STDEF_FUNC(BaseConsolCmdShell_nF)
{
	const int* show_pn;
	STDEF_GETLOCAL_R(_pdsvBase, BaseConsolCmdShell_nF, show_pn);

	int show_n = 1;
	int seq_n = 0;
	char opr_str[10], cmd_str[1024] = "\0", param_str[2024] = "\0";
	const char* param1_str;
	const char* strValue = (const char*)BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, seq_n);
	strcpy_s(opr_str, 10, strValue);	seq_n++;
	strValue = (const char*)BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, seq_n);
	strcpy_s(cmd_str, 1024, strValue); seq_n++;
	param1_str = (const char*)BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, seq_n);
	seq_n++;
	strValue = (const char*)BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, seq_n);
	
	if (strValue == NULL && param1_str != NULL)
		strcpy_s(param_str, 2024, param1_str);
	else if (strValue != NULL && param1_str != NULL)
		sprintf_s(param_str, 2024, "\"%s\" \"%s\"", param1_str, strValue);

	char param2[1024];
	BaseSystem::tomulti(param_str, param2, 1024);

	STLString fullpath = "\"";
	fullpath += BaseStateManager::path_get(cmd_str);
	fullpath += "\"";
	printf("\n%s, %s, %s\n", opr_str, fullpath.c_str(), param2);
	BaseSystem::run_shell_command(opr_str, fullpath.c_str(), param2, *show_pn != 0);

	STDEF_Manager(pManager);

	pManager->post_event_state("Submit");
	return 1;
}

STDEF_FUNC(BaseConsolCmdRun_strvF)
{
	const char* strBuffer;
	STDEF_GETLOCAL_R(_pdsvBase, BaseConsolCmdRun_strvF, strBuffer);

	char strOut[4096];
	strcpy_s(strOut, 4096, strBuffer);
	char* strOut2 = BaseState::VariableStringMake(_pdsvBase, _pdsvContext, _pdsvEvent, strOut, 4096);

	int nRet = 0;
	nRet = BaseSystem::run_shell_command(strOut);

	STDEF_Manager(pManager);

	if (nRet == 1)
		pManager->post_event_state("Submit");
	else
		pManager->post_event_state("Cancel");
	return nRet;
}

STDEF_FUNC(BaseStateCurrentCheckIs_anF)
{
	const int* pnStates;
	short	nCnt;
	STDEF_AGETLOCAL_R(_pdsvBase, BaseStateCurrentCheckIs_anF, pnStates, nCnt);
	STDEF_BaseState(pState);

	for (int i = 0; i < nCnt; i++) {
		if (pState->key_is_engaged(pnStates[i]))
			return 1;
	}

	return 0;
}

STDEF_FUNC(BaseStateCurrentCheckIsnt_anF)
{
	const int* pnStates;
	short	nCnt;
	STDEF_AGETLOCAL_R(_pdsvBase, BaseStateCurrentCheckIsnt_anF, pnStates, nCnt);
	STDEF_BaseState(pState);

	for (int i = 0; i < nCnt; i++) {
		if (pState->key_is_engaged(pnStates[i]))
			return 0;
	}

	return 1;
}

STDEF_FUNC(BaseExclusiveEvent)
{
	STDEF_Manager(pManager);

	pManager->event_skip(true);

	return 1;
}

void reload_state(void* _data)
{
	// marker multi thread
	BaseStateManager::get_manager()->reload();
}

STDEF_FUNC(BaseStateSystemReload_nF)
{
	STDEF_Manager(manager);

	manager->post_processor_regist(reload_state, NULL);
	return 1;
}

STDEF_FUNC(BaseFile2PathCopy_avarF)
{
	const int* hash_a = NULL;
	short cnt;
	STDEF_AGETLOCAL_R(_pdsvBase, BaseFile2PathCopy_avarF, hash_a, cnt);

	if (cnt < 2)
		return 0;

	char filename[255], pathTo[512];

	const char* path, *path2;
	STDEF_BaseState(state);
	BaseDStructureValue* variable = state->variable_get();
	if (!variable->get(hash_a[0], (const void**)&path))
		return 0;
	if (!variable->get(hash_a[1], (const void**)&path2))
		return 0;

	BaseFile::get_filename(path, filename, 255);
	strcpy_s(pathTo, 512, path2);
	strcat_s(pathTo, 512, filename);
	BaseFile::get_filext(path, filename, 255);
	strcat_s(pathTo, 512, filename);

	variable->set_alloc(hash_a[0], pathTo);

	char buff[512];
	BaseSystem::tomulti(path, buff, 512);
	BaseSystem::tomulti(pathTo, pathTo, 512);
	if(copy(buff, pathTo) == 0)
		return 0;

	return 1;
}

STDEF_FUNC(BaseFilepathNameCut_varIf)//	INT32	EnumVariableDefine	cut only filenameand ext
{
	const int* hash = 0;
	STDEF_GETLOCAL_R(_pdsvBase, BaseFilepathNameCut_varIf, hash);
	STDEF_BaseState(state);
	BaseDStructureValue* var = state->variable_get();

	const char* path;
	if(!var->get(*hash, (const void**)&path))
		return 0;

	char buf[MAX_PATH];
	strcpy_s(buf, MAX_PATH, path);
	BaseFile::path_cut_last(buf, MAX_PATH);
	var->set_alloc(*hash, buf);

	return 1;
}

STDEF_FUNC(BaseFilepathPathExtCut_varIf)//	INT32	EnumVariableDefine	cut only filenameand ext
{
	const int* hash = 0;
	STDEF_GETLOCAL_R(_pdsvBase, BaseFilepathPathExtCut_varIf, hash);
	STDEF_BaseState(state);
	BaseDStructureValue* var = state->variable_get();

	const char* path;
	if (!var->get(*hash, (const void**)&path))
		return 0;

	char buf[MAX_PATH];
	strcpy_s(buf, MAX_PATH, path);
	BaseFile::get_filename(path, buf, MAX_PATH);
	var->set_alloc(*hash, buf);

	return 1;
}

STDEF_FUNC(BaseFilepathPathCut_varIf)//	INT32	EnumVariableDefine	cut only path
{
	const int* hash = 0;
	STDEF_GETLOCAL_R(_pdsvBase, BaseFilepathPathCut_varIf, hash);
	STDEF_BaseState(state);
	BaseDStructureValue* var = state->variable_get();

	const char* path;
	if (!var->get(*hash, (const void**)&path))
		return 0;

	char buf[MAX_PATH];
	strcpy_s(buf, MAX_PATH, path);
	BaseFile::get_filenamefull(path, buf, MAX_PATH);
	var->set_alloc(*hash, buf);

	return 1;
}

STDEF_FUNC(BaseFuncAddInverse_nIf)
{
	return 1;
}

STDEF_FUNC(TestAdd_nF)
{
	const int* value = NULL;
	STDEF_GETLOCAL_R(_pdsvBase, TestAdd_nF, value);

	return *value;
}

STDEF_FUNC(BaseSystemSavePathSet_v1_nF)
{
	const char* path = (const char*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0);

	if (!path)
		return 0;

	STDEF_Manager(manager);
	manager->path_set_save(path);
	return 1;
}

STDEF_FUNC(BaseVariableFileLoad_astrF)
{
	const char* str_a;
	STDEF_GETLOCAL_R(_pdsvBase, BaseVariableFileLoad_astrF, str_a);
	STLVString var_astr;

	char buf[255];
	strcpy_s(buf, 255, str_a);
	BaseFile::paser_list_seperate(buf, &var_astr, ",");

	if (var_astr.size() < 2)
		return 0;

	BaseDStructureValue* pVariable = ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();

	int hash = STRTOHASH(var_astr[0].c_str());

	BaseFile file;
	if (file.OpenFile(var_astr[1].c_str(), BaseFile::OPEN_READ))
		return 0;

	UINT32 size_n = file.get_size_file();
	char* buff = PT_Alloc(char, (int)size_n+1);
	*(buff + size_n) = 0;
	
	file.Read(buff, size_n);

	file.CloseFile();


	int index = BaseDStructure::get_index(hash);

	if (BaseDStructure::type_get(index).nType >= TYPE_STRING)
	{
		pVariable->set_mass(hash, buff, size_n);
	}
	else {
		pVariable->set_alloc(hash, buff, size_n);
	}

	PT_Free(buff);
	return 1;
}

STDEF_FUNC(BaseVariableFileSave_astrF)
{
	const char* str_a;
	STDEF_GETLOCAL_R(_pdsvBase, BaseVariableFileSave_astrF, str_a);
	STLVString var_astr;

	char buf[255];
	strcpy_s(buf, 255, str_a);
	BaseFile::paser_list_seperate(buf, &var_astr, ",");

	if (var_astr.size() < 2)
		return 0;

	BaseDStructureValue* pVariable = ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();

	int hash = STRTOHASH(var_astr[0].c_str());

	const char *buff = NULL;
	short cnt = 0;
	int cnt2 = 0;

	bool largeable = pVariable->get_base()->type_get(pVariable->get_base()->get_index(hash)).nType >= TYPE_STRING;

	BaseFile file;

	if (file.OpenFile(var_astr[1].c_str(), BaseFile::OPEN_WRITE))
		return 0;

	if (largeable)
	{
		if (!pVariable->get_mass(hash, (const void**)&buff, &cnt2))
			return 0;
	}
	else
	{
		if (!pVariable->get(hash, (const void**)&buff, &cnt))
			return 0;
	}

	if(largeable)
		file.Write(buff, cnt2);
	else
		file.Write(buff, (int)cnt);

	file.CloseFile();
		return 1;

}

STDEF_FUNC(BaseDebugLeakCheck_nF)
{
	static int cnt = 0;
	int kkk = 0;
	cnt++;
	if (kkk || cnt == 1 || cnt == 2 || cnt == 5)
	{
		//PT_MemDisplay();
	}
		return 1;
}

STDEF_FUNC(BaseSystemTerminate_nF)
{
	STDEF_Manager(pManager);
	pManager->system_terminate();
	return 1;
}

STDEF_FUNC(BaseMemoryFree_varF)
{
	const int* hash;
	STDEF_GETLOCAL_R(_pdsvBase, BaseMemoryFree_varF, hash);

	STDEF_BaseState(state_p);
	BaseDStructureValue* variable_pdsv = state_p->variable_get();
	const INT64* refer;// = (const INT64*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, variable_pdsv);
	if (!variable_pdsv->get(*hash, (const void**)&refer))
		return 0;
	if (!refer)
		return 0;

	mpool_get().free_mem(*refer);
	return 1;
}

STDEF_FUNC(BaseEventPriorityUpdate_nF)
{
	const int* linkIndex_pn;
	STDEF_GETLOCAL_R(_pdsvBase, BaseEventPriorityUpdate_nF, linkIndex_pn);
	
	STDEF_BaseState(state_p);
	BaseDStructureValue* variable_pdsv = state_p->variable_get();
	const int *priority_pn = (const int*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0, variable_pdsv);
	
	STStateActor	actor;
	actor.nType = STStateActor::HASH;
	actor.nKey = STRTOHASH("BaseStateEventGlobal");
	actor.nPriority = *priority_pn;
	actor.nLinkIndex = *linkIndex_pn;
	actor.pState = state_p;
	actor.pdstActor = NULL;//state_p->link_get(_pdsvBase);
	
	STDEF_Space(space_p);
	if(!space_p->actor_event_update(actor))
		return 0;
	
	return 1;
}
