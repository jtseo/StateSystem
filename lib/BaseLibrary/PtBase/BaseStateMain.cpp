#include "stdafx.h"
#include "BaseState.h"
#include "BaseStateMain.h"
#include "BaseStateManager.h"
#include "BaseStateSpace.h"

#include "BaseResDStructure.h"
#include "BaseResManager.h"
#include "BaseFile.h"
#include "BaseUIManager.h"

BaseStateMain	BaseStateMain::sm_sample(NULL, NULL);

#define VER_ADDSERIAL
#ifdef VER_ADDSERIAL
//#define TRACE_STATE
#endif

int s_DebugDataCheckInt = 0;
int *s_pDebugDataCheckPoint = NULL;

BaseStateMain::BaseStateMain(BaseStateSpace *_pSpace, const char *_strName)
{
	init(_pSpace, _strName);
}

BaseStateMain::~BaseStateMain(void)
{
	release();
}

void BaseStateMain::release()
{
	if(mpool_get().is_terminated())
		return;
	
	for (unsigned i = 0; i < m_stlVpStateActive.size(); i++)
	{
		m_stlVpStateActive[i]->OnEnd(NULL);
		PT_OFree(m_stlVpStateActive[i]);
	}
	m_stlVpStateActive.clear();

	BaseResManager* pResManager = BaseResManager::managerReleaseGet();
	if (pResManager)
	{
		if (m_pResDstMain)
		{
			m_pResDstMain->dec_ref(this);
			if (m_pResDstMain->GetRefCount() == 1)
				pResManager->ClearResource(m_pResDstMain);

			m_pResDstMain = NULL;
		}

		if (m_pResDstLink)
		{
			m_pResDstLink->dec_ref(this);
			if (m_pResDstLink->GetRefCount() == 1)
				pResManager->ClearResource(m_pResDstLink);
			m_pResDstLink = NULL;
		}
	}

	for (unsigned i = 0; i < m_stlVReservedCntxGroupId.size(); i++)
		PT_OFree(m_stlVpReservedCntx[i]);

	m_stlVpReservedCntx.clear();
	m_stlVReservedCntxGroupId.clear();
}

bool BaseStateMain::init(BaseStateSpace *_pSpace, const char *_strName)
{
	m_bReloading	= false;
	m_pSpace		= _pSpace;
	m_bConformer	= false;
	m_nGradCloseTarget	= 0;
	m_nKeyName = 0;

	if(_pSpace == NULL)
		return false;

	m_strRecordName	= _strName;

	const char *strPost;

	strPost	= strrchr(_strName, SLASH_C);
	if(strPost == NULL)
		strPost	= _strName;
	else
		strPost++;

	
	if (load_(_strName))
	{
		set_name(strPost);
		m_nKeyName = UniqHashStr::get_string_hash_code(m_strName.c_str());

		BaseState::RegistState(m_pdstMain);
		BaseState::RegistLink(m_pdstLink);
		return true;
	}
	return false;
}

bool BaseStateMain::load_re_(const char* _strName)
{
	m_pResDstMain->dec_ref(this);
	m_pResDstLink->dec_ref(this);

	BaseResManager* mgr_res = BaseResManager::get_manager();
	mgr_res->ClearResource(m_pResDstMain);
	mgr_res->ClearResource(m_pResDstLink);
	return load_(_strName);
}

bool BaseStateMain::variableLocalDefine()
{
	int keyDefine = STRTOHASH("state_define");
	if (!m_pdstMain->is_exist(keyDefine))
		return false;

	BaseDStructureValue* dsvState = NULL;
	PT_OAlloc2(dsvState, BaseDStructureValue, m_pdstMain, 1024);

	dsvState->set_key(keyDefine);
	const int* anLink;
	short nCnt = 0;

	if (!dsvState->get(HASH_STATE(ARRAY_LINK), (const void**)&anLink, &nCnt)
		|| nCnt == 0)
	{
		PT_OFree(dsvState);
		return false;
	}
			
	for (int i = 0; i < nCnt; i++)
	{
		int nKeyLink;
		nKeyLink = *(anLink + i);
		if (!m_pdstLink->is_exist(nKeyLink))
			continue;

		BaseDStructureValue* pdsvLink = NULL;
		PT_OAlloc2(pdsvLink, BaseDStructureValue, m_pdstLink, 0);

		if (pdsvLink == NULL)
			continue;
		if (!pdsvLink->set_key(nKeyLink))
		{
			PT_OFree(pdsvLink);
			continue;
		}

		short len = 0;
		int row = 3;
		const char* name = NULL;
		if (!pdsvLink->get((const void**)&name, &len, row))
		{
			PT_OFree(pdsvLink);
			continue;
		}

		bbyte type = TYPE_INT32;
		if (strcmp(name, "VariableInt") == 0)
			type = TYPE_INT32;
		else if (strcmp(name, "VariableArrayByte") == 0)
			type = TYPE_ARRAY_BYTE;
		else if (strcmp(name, "VariableArrayInt") == 0)
			type = TYPE_ARRAY_INT32;
		else if (strcmp(name, "VariableInt64") == 0)
			type = TYPE_INT64; 
		else if (strcmp(name, "VariableArrayInt64") == 0)
			type = TYPE_ARRAY_INT64;
		else if (strcmp(name, "VariableFloat") == 0)
			type = TYPE_FLOAT;
		else if (strcmp(name, "VariableString") == 0)
			type = TYPE_STRING;
		else if (strcmp(name, "VariableArrayFloat") == 0)
			type = TYPE_ARRAY_FLOAT;
		else {
			PT_OFree(pdsvLink);
			continue;
		}

		row++;
		while (pdsvLink->get((const void**)&name, &len, row))
		{
			row++;
			int hash = STRTOHASH(name);
			int index = pdsvLink->get_index(hash);
			if (index >= 0) {
				continue;
			}
			pdsvLink->get_base()->add_column(0, hash, type, 0, 0, "state_define", name);
		}
		PT_OFree(pdsvLink);
	}
	PT_OFree(dsvState);
	return true;
}

bool BaseStateMain::load_(const char *_strName)
{
	m_pResDstMain = (BaseResDStructure*)BaseResManager::get_manager()->get_resource(_strName, BaseResDStructure::OBJECTID_DSTRUCTURE, NULL, NULL, 0);
	
	B_ASSERT(m_pResDstMain != 0);
	if (m_pResDstMain == NULL) {
		m_pdstMain = NULL;
		m_nKeyName = 0;
		return false;
	}

	m_pResDstMain->inc_ref(this);
	m_pdstMain	= m_pResDstMain->get_dst();
	char strBuf[255];
	BaseFile::change_filename(_strName, "Base", strBuf, 255);
	//m_pdstMain->load_define_append(strBuf);

	strcpy_s(strBuf, 255, _strName);
	strcat_s(strBuf, 255, "Link");

	m_pResDstLink = (BaseResDStructure*)BaseResManager::get_manager()->get_resource(strBuf, BaseResDStructure::OBJECTID_DSTRUCTURE, NULL, NULL, 0);
	B_ASSERT(m_pResDstLink != 0);
	if (m_pResDstLink == NULL)
	{
		m_nKeyName = 0;
		return false;
	}
	m_pResDstLink->inc_ref(this);
	m_pdstLink	= m_pResDstLink->get_dst();
	BaseFile::change_filename(_strName, "BaseLink", strBuf, 255);
	//m_pdstLink->load_define_append(strBuf);
	
	if(m_pSpace->threadSpaceGet())
		return true;
	
	if(!m_bReloading && m_pdstMain->is_exist(HASH_STATE(START_STATE)))
	{
		BaseState *pState;
		PT_OAlloc4(pState, BaseState, m_pSpace, m_pdstMain, HASH_STATE(START_STATE), m_pdstLink);
	
		pState->set_state_main(this);
		if (pState->context_create() == NULL)
		{
			PT_OFree(pState);
			m_nKeyName = 0;
			return false;
		}
		else {
			m_stlVpStateActive.push_back(pState);
		}

		// variable add routine for the indefendant variable define for the StateMain
		variableLocalDefine();

		return true;
	}
	return false;
}

bool BaseStateMain::restart()
{
	m_bReloading	= true;
	if (!load_re_(m_strRecordName.c_str()))
		return false;

	BaseState::RegistState(m_pdstMain);
	BaseState::RegistLink(m_pdstLink);
	m_bReloading	= false;
	return true;
}

void BaseStateMain::reload_start()
{
	STLMnInt stlMnGroupId;
	int key;
	for (unsigned i = 0; i < m_stlVpStateActive.size(); i++)
	{
		BaseState* pState = m_stlVpStateActive[i];
		key = pState->get_key();

		if (!pState->state_set_(m_pdstMain, m_pdstLink, key))
		{
			m_stlVpStateActive.erase(m_stlVpStateActive.begin() + i);
			pState->OnEnd(NULL);
			PT_OFree(pState);
			i--;
		}
		pState->regist_func_();
	}
}

void BaseStateMain::reload()
{
	restart();
}

int BaseStateMain::StateStart()
{
	if(m_stlVpStateActive.size() > 0)
	{	
		m_stlVpStateActive[0]->OnStart(NULL);
	}

	return 0;
}

void BaseStateMain::reset_to_reload()
{
	/* reloading stratigic is changed 2020/04/23 all of active state are keep the instance, and just change base state and reactive means that End and Start.
	m_stlVRecordActive.clear();
    m_StateTable.clear();

	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		m_stlVRecordActive.push_back(m_stlVpStateActive[i]->get_key());
		//m_stlVRecordIdentify.push_back(m_stlVpStateActive[i]->identifier_get());
        m_StateTable.add(NULL, m_stlVpStateActive[i]->group_id_get());
	}

	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		m_stlVpStateActive[i]->OnEnd(NULL);
		PT_OFree(m_stlVpStateActive[i]);
	}
	m_stlVpStateActive.clear();

	BaseResManager *pResManager	= BaseResManager::get_manager();
	if(pResManager)
	{
		if(m_pResDstMain)
		{
			m_pResDstMain->dec_ref(this);
			if(m_pResDstMain->GetRefCount() == 1)
				pResManager->ClearResource(m_pResDstMain);

			m_pResDstMain = NULL;
		}

		if(m_pResDstLink)
		{
			m_pResDstLink->dec_ref(this);
			if(m_pResDstLink->GetRefCount() == 1)
				pResManager->ClearResource(m_pResDstLink);
			m_pResDstLink	= NULL;
		}
	}

    
	for(unsigned i=0; i<m_stlVReservedCntxGroupId.size(); i++)
		PT_OFree(m_stlVpReservedCntx[i]);

	m_stlVpReservedCntx.clear();
	m_stlVReservedCntxGroupId.clear();
	//*/
}

void BaseStateMain::state_active_show_all(void)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
		m_stlVpStateActive[i]->state_active_show_all();
}

int BaseStateMain::OnEvent(BaseDStructureValue *_pEvent)
{
	int nKey;

	nKey	= _pEvent->get_key();

	if(nKey == HASH_STATE(STATE_CHANGE))
		return StateTransit(_pEvent);

	if(nKey == HASH_STATE(STATE_ADD))
		return StateAdd(_pEvent);

	if(nKey == HASH_STATE(STATE_RELEASE))
		return StateRelease(_pEvent);

	//==================================================================================================
	// Start Add or Update by OJ 2012-07-31 ���� 10:28:01
	// Event Actor �� ���� ���ʿ��� �� �ڵ�
	const int *pnTargetMain;
	if(_pEvent->get(HASH_STATE(BaseTransitionGoalMain), (const void**)&pnTargetMain))
	{
		if(*pnTargetMain != m_nKeyName && *pnTargetMain != 0)
			return BaseState::RET_UNPROCESS;
	}

	int nRet	= BaseState::RET_UNPROCESS;
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{	
		if(m_stlVpStateActive[i]->OnEvent(_pEvent) == BaseState::RET_PROCESS)
			nRet	= BaseState::RET_PROCESS;
	}

	// End by OJ 2012-07-31 ���� 10:28:05
	//==================================================================================================

	grade_close_();

	return nRet;
}

int BaseStateMain::StateRelease(BaseDStructureValue *_pEvent)
{
	const int *pnTargetMain;
    const INT64 *pnStatePoint;
	const BaseState *pStateTarget = NULL;

	_pEvent->get(HASH_STATE(BaseTransitionGoalMain), (const void**)&pnTargetMain);

	if(*pnTargetMain != m_nKeyName)
		return BaseState::RET_UNPROCESS;

	_pEvent->get(HASH_STATE(BaseTransitionGoalpState), (const void**)&pnStatePoint);
	pStateTarget = (BaseState*)*pnStatePoint;
	BaseState *pState	= GetStateParent(pStateTarget);

	if(pState)
	{
		pState->OnEnd(_pEvent);
		release_(pState);
		return BaseState::RET_PROCESS;
	}

	return BaseState::RET_UNPROCESS;
}

void BaseStateMain::release_(BaseState *_pState)
{
	STLVpState::iterator	it;
	it	= std::find(m_stlVpStateActive.begin(), m_stlVpStateActive.end(), _pState);
	if(it != m_stlVpStateActive.end())
	{
		BaseState *pState	= *it;
		PT_OFree(pState);
		g_BreakPoint();
		m_stlVpStateActive.erase(it);

#ifdef _DEBUG
		if(m_nKeyName == -1522711694)
		{
			bool bFind = false;
			for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
			{
				if(m_stlVpStateActive[i]->obj_serial_get() == 572)// 277 SkillVictim State 0x00010000 was added or subbed
				{
					s_pDebugDataCheckPoint = (int*)(&m_stlVpStateActive[i]);
					bFind = true;
				}
			}
			if(s_pDebugDataCheckPoint && bFind == false)
				int x=0;
			g_BreakPoint();
		}
#endif

		
	}
}


BaseDStructureValue *BaseStateMain::GetContextReservedRef(const STLMnInt &_stlMnGroupId)
{
	BaseDStructureValue *pRet	= NULL;
	pRet = GetContext(_stlMnGroupId);
	if(pRet)
		return pRet;
	for(unsigned i=0; i<m_stlVReservedCntxGroupId.size(); i++)
	{
        if(BaseState::group_id_equal(m_stlVReservedCntxGroupId[i], _stlMnGroupId))
		{
			pRet = m_stlVpReservedCntx[i];
			break;
		}
	}

	return pRet;
}

BaseDStructureValue *BaseStateMain::GetContextReserved(const STLMnInt &_stlMnGroupId)
{
	BaseDStructureValue *pRet	= NULL;
	for(unsigned i=0; i<m_stlVReservedCntxGroupId.size(); i++)
	{
		if(BaseState::group_id_equal(m_stlVReservedCntxGroupId[i], _stlMnGroupId))
		{
			pRet = m_stlVpReservedCntx[i];

			m_stlVpReservedCntx.erase(m_stlVpReservedCntx.begin()+i);
			m_stlVReservedCntxGroupId.erase(m_stlVReservedCntxGroupId.begin()+i);
			break;
		}
	}

	return pRet;
}

void	BaseStateMain::AddContextReserved(const STLMnInt &_stlMnGroupId, BaseDStructureValue *_pdsvContext)
{
    m_stlVReservedCntxGroupId.push_back(_stlMnGroupId);
	m_stlVpReservedCntx.push_back(_pdsvContext);
}


int BaseStateMain::StateAdd(BaseDStructureValue *_pEvent)
{
	const int *pnTargetMain, *pnGoalState;
    const INT64 *pnStatePoint;
	const BaseState *pStateTarget = NULL;
    STLMnInt stlMnGroupId, stlMnGroupParentId;

	_pEvent->get(HASH_STATE(BaseTransitionGoalMain), (const void**)&pnTargetMain);

	if(*pnTargetMain != m_nKeyName)
		return BaseState::RET_UNPROCESS;

	_pEvent->get(HASH_STATE(BaseTransitionGoalpState), (const void**)&pnStatePoint);
    BaseState::group_id_get(_pEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
	_pEvent->get(HASH_STATE(HASH_BaseTransitionGoal), (const void**)&pnGoalState);
    BaseState::group_id_get(_pEvent, HASH_STATE(BaseIdentifierParent_n), stlMnGroupParentId);
	pStateTarget = (BaseState*)*pnStatePoint;

	if(!m_pdstMain->is_exist(*pnGoalState))
		return BaseState::RET_UNPROCESS;

#ifdef _DEBUG
	STLMnInt::iterator it;
	int hash = STRTOHASH("StClassLevel0");
	for (it = stlMnGroupId.begin(); it != stlMnGroupId.end(); it++)
	{
		if (it->first == hash)
		{
			int x = 0;
			x++;
		}
	}
#endif

	BaseState *pState	= GetStateParent(pStateTarget);
	BaseDStructureValue *pdsvContext	= NULL, *pdsvVariable = NULL;

	if (pState)
		pdsvVariable = pState->variable_next_take();

	if(pState && pState->group_id_equal(stlMnGroupId))
		pdsvContext	= pState->context_get();
	else
	{
		pdsvContext	= GetContextReserved(stlMnGroupId);
		if(pdsvContext)
		{
			int nTime = (int)BaseSystem::timeGetTime();
			pdsvContext->set_alloc(HASH_STATE(BaseValueTimeStart), &nTime);
		}
	}
	
	DebugOutString(_pEvent, NULL, "state_add");
	PT_OAlloc4(pState, BaseState, m_pSpace, m_pdstMain, *pnGoalState, m_pdstLink);
	
	pState->set_state_main(this);
	g_BreakPoint();
	m_stlVpStateActive.push_back(pState);

#ifdef _DEBUG
	if(m_nKeyName == -1522711694)
	{
		bool bFind = false;
		for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
		{
			if(m_stlVpStateActive[i]->obj_serial_get() == 572)// 277 SkillVictim State 0x00010000 was added or subbed
			{
				bFind = true;
				bool bFirst = false;
				if(s_pDebugDataCheckPoint == NULL)
					bFirst = true;
				s_pDebugDataCheckPoint = (int*)(&m_stlVpStateActive[i]);

				if(bFirst)
					s_DebugDataCheckInt	= (int)(*s_pDebugDataCheckPoint);
			}
		}
		if(s_pDebugDataCheckPoint && bFind == false)
			int x=0;
		g_BreakPoint();
	}
#endif
	if(pdsvContext)
		pState->context_set(pdsvContext);
	else
	{
		pState->context_create();

		pdsvContext = pState->context_get();
		if(pdsvContext)
		{
			int nTime = (int)BaseSystem::timeGetTime();
			pdsvContext->set_alloc(HASH_STATE(BaseValueTimeStart), &nTime);
		}
	}
	
	if (pdsvVariable)
		pState->variable_set(pdsvVariable);
	else
		pdsvVariable = pState->variable_get();

	const int *serialCaster = NULL;
	if (_pEvent && _pEvent->get(HASH_STATE(BaseTransitionThreadCaster), (const void**)&serialCaster))// To keep caster's serial, it gonna use in the BaseStateEventGlobalReturn_nF
	{
		pdsvVariable->set_alloc(HASH_STATE(BaseTransitionThreadCaster), serialCaster);
	}
	
    BaseState::group_id_set(pState->context_get(), HASH_STATE(BaseIdentifierParent_n), stlMnGroupParentId);

	//const INT64 *pnPoint;
	//if(_pEvent->get(HASH_STATE(BaseNextTempValues), (const void**)&pnPoint))
	//{
	//	STLMnpVoid *pstlMnpTemps = (STLMnpVoid*)*pnPoint;
	//	pState->TempValueGet() = *pstlMnpTemps;
	//	delete pstlMnpTemps;
	//}

	pState->group_id_set(stlMnGroupId);
	pState->OnStart(_pEvent);

	return BaseState::RET_PROCESS;
}

int BaseStateMain::StateTransit(BaseDStructureValue *_pEvent)
{
	const int *pnTargetMain, *pnGoalState;
    const INT64 *pnStatePoint;
    const BaseState *pStateTarget = NULL;
    STLMnInt stlMnGroupId;

	_pEvent->get(HASH_STATE(BaseTransitionGoalMain), (const void**)&pnTargetMain);

	if(*pnTargetMain != m_nKeyName)
		return BaseState::RET_UNPROCESS;

	_pEvent->get(HASH_STATE(BaseTransitionGoalpState), (const void**)&pnStatePoint);
	BaseState::group_id_get(_pEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
	_pEvent->get(HASH_STATE(HASH_BaseTransitionGoal), (const void**)&pnGoalState);
	pStateTarget = (BaseState*)*pnStatePoint;
	BaseState *pState	= GetStateParent(pStateTarget);

	int nObjectSerial;
	if(pState)
	{
		BaseDStructureValue *pdsvContext, *pdsvVariable;
		pdsvContext	= pState->context_get();
		pdsvContext->ref_inc();
        stlMnGroupId	= pState->group_id_get();
		nObjectSerial = pState->obj_serial_get();

		pdsvVariable = pState->variable_next_take();
		pState->OnEnd(_pEvent);
		pState->release();

		pState->init(m_pSpace, m_pdstMain, *pnGoalState, m_pdstLink, &stlMnGroupId);
		pState->context_set(pdsvContext);
		if(pdsvVariable)
			pState->variable_set(pdsvVariable);
		pState->set_state_main(this);
		pdsvContext->ref_dec();
		pState->OnStart(_pEvent);
		return BaseState::RET_PROCESS;
	}
		
	return BaseState::RET_UNPROCESS;
}

int BaseStateMain::OnDraw()
{
	return BaseState::RET_UNPROCESS;
}

int BaseStateMain::Destory()
{
	return BaseState::RET_UNPROCESS;
}

void BaseStateMain::ContextRelease(BaseDStructureValue *_pdsvContext)
{
	if(_pdsvContext == NULL)
		return;

	//const int *pnId;
    STLMnInt stlMnGroupId;
    
    BaseState::group_id_get(_pdsvContext, HASH_STATE(BaseIdentifierParent_n), stlMnGroupId);
	
	BaseDStructureValue *pdsvContextParent;

	pdsvContextParent = GetContext(stlMnGroupId);
	if(pdsvContextParent == NULL)
		return;
/*
    // Try to record all of childs ID
	const int *anIds;
	short nCnt;
	if(pdsvContextParent->get(HASH_STATE(BaseIdentifierChilds_an), (const void **)&anIds, &nCnt))
	{
		STLVInt	stlVnIds;
		for(int i=0; i<nCnt; i++){ 
			if(anIds[i] != *pnId)
				stlVnIds.push_back(anIds[i]);
		}

		nCnt = (short)stlVnIds.size();
		if(nCnt > 0)
			pdsvContextParent->set_alloc(HASH_STATE(BaseIdentifierChilds_an), (const void*)&stlVnIds[0], nCnt);
		else
			pdsvContextParent->set_alloc(HASH_STATE(BaseIdentifierChilds_an), NULL);
	}
 //*/
}

BaseDStructureValue *BaseStateMain::GetContext(const STLMnInt &_stlMnGroupId)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		BaseState *pState	= m_stlVpStateActive[i];
		const BaseState *pStateSub	= pState;

		while(pStateSub)
		{
			if(pStateSub->group_id_equal(_stlMnGroupId))
				return pState->context_get();
			pStateSub	= pStateSub->get_sub_state();
		}
	}
	return NULL;
}

bool BaseStateMain::GetState(STLVpVoid *_pstlVpStateActive, const STLMnInt *_pstlMnGroupId, int _nSerial)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		BaseState *pState	= m_stlVpStateActive[i];
		const BaseState *pStateSub	= pState;

		while(pStateSub)
		{
            if(BaseState::group_id_variable(_pstlMnGroupId))
			{
				if(pStateSub->group_id_equal(*_pstlMnGroupId))
				{
					//if(_nSerial != 0)
					//{
					//	if((int)pStateSub->TempValueIntGet(HASH_STATE(SelectorSerial_nV)) == _nSerial)
					//		_pstlVpStateActive->push_back(pState);
					//}else
					{
						_pstlVpStateActive->push_back(pState);
					}
				}
			}else{
				//if(_nSerial != 0)
				//{
				//	if((int)pStateSub->TempValueIntGet(HASH_STATE(SelectorSerial_nV)) == _nSerial)
				//		_pstlVpStateActive->push_back(pState);
				//}else
				{
					_pstlVpStateActive->push_back(pState);
				}
			}
			pStateSub	= pStateSub->get_sub_state();
		}
	}
	return (!_pstlVpStateActive->empty());
}

BaseState *BaseStateMain::GetStateSerial(int _nObjSerialState)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		if(m_stlVpStateActive[i]->get_serial() == _nObjSerialState)
			return m_stlVpStateActive[i];
	}
	return NULL;
}

BaseState *BaseStateMain::GetState(const BaseState *_pState)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		if(m_stlVpStateActive[i] == _pState)
			return m_stlVpStateActive[i];
	}
	return NULL;
}


BaseState *BaseStateMain::GetStateParent(const BaseState *_pState)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		if(m_stlVpStateActive[i]->is_mine(_pState))
			return m_stlVpStateActive[i];
	}
	return NULL;
}

BaseState *BaseStateMain::GetStateIdentifier(int _nKeyName, const STLMnInt &_stlMnGroupId, int _nSerial)
{
	for (unsigned i = 0; i<m_stlVpStateActive.size(); i++)
	{
		BaseState *pState = m_stlVpStateActive[i];
		const BaseState *pStateSub = pState;

		while (pStateSub)
		{
			if (pStateSub->get_key() == _nKeyName)
			{
				if (pStateSub->group_id_equal(_stlMnGroupId))
				{
					//if (_nSerial != 0)
					//{
					//	if ((int)pStateSub->TempValueIntGet(HASH_STATE(SelectorSerial_nV)) == _nSerial)
					//		return pState;
					//}
					//else
					{
						return pState;
					}
				}
			}
			pStateSub = pStateSub->get_sub_state();
		}
	}
	return NULL;
}

BaseState *BaseStateMain::GetStateName(int _nKeyName, const STLMnInt *_pstlMnGroupId, int _nSerial)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		BaseState *pState	= m_stlVpStateActive[i];
		const BaseState *pStateSub	= pState;

		while(pStateSub)
		{
			if(pStateSub->get_key() == _nKeyName)
			{
                if(BaseState::group_id_variable(_pstlMnGroupId))
				{
					if(pStateSub->group_id_include(*_pstlMnGroupId))
					{
						//if(_nSerial != 0)
						//{
						//	if((int)pStateSub->TempValueIntGet(HASH_STATE(SelectorSerial_nV)) == _nSerial)
						//		return pState;
						//}else
						{
							return pState;
						}
					}
				}else{
					//if(_nSerial != 0)
					//{
					//	if((int)pStateSub->TempValueIntGet(HASH_STATE(SelectorSerial_nV)) == _nSerial)
					//		return pState;
					//}else
					{
						return pState;
					}
				}
			}
			pStateSub	= pStateSub->get_sub_state();
		}
	}
	return NULL;
}

int	BaseStateMain::get_key()
{
	return m_nKeyName;
}

BaseDStructure	*BaseStateMain::get_state()
{
	return m_pdstMain;
}

BaseDStructure	*BaseStateMain::get_link()
{
	return m_pdstLink;
}

void BaseStateMain::OnUpdate(unsigned _nTimeDelta)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		m_stlVpStateActive[i]->OnUpdate(_nTimeDelta);
	}

	grade_close_();
}

void BaseStateMain::DebugOutString(const BaseDStructureValue *_pdst,const BaseDStructureValue *_pdstLink, const char *_strState)
{
#ifdef TRACE_STATE
	if(m_pSpace->get_manager()->m_nDebugBlockCastMessage != 0)
		return;

	const int* pValue;
	if (_pdst && _pdst->get(HASH_STATE(BaseDebugHideLog), (const void**)& pValue))
		return;
	if (_pdstLink && _pdstLink->get(HASH_STATE(BaseDebugHideLog), (const void**)& pValue))
		return;

	char strBuffer[255], strTemp[255], *strSlash;
	strBuffer[0]	= NULL;

	strcat_s(strBuffer, 255, "SMain:");
	strcpy_s(strTemp, 255, m_pdstMain->get_name());
	strSlash	= strrchr(strTemp, SLASH_C);
	if(strSlash)
		strcat_s(strBuffer, 255, ++strSlash);
	else
		strcat_s(strBuffer, 255, strTemp);
	
	if(_strState)
	{
		strcat_s(strBuffer, 255, " SAction:");
		strcat_s(strBuffer, 255, _strState);
	}
	const char *strName;
	const char *strEvent;
	if(_pdst && _pdst->get(HASH_STATE(STR_NAME), (const void**)&strEvent))
	{
		strcat_s(strBuffer, 255, " SEvent:");
		strcat_s(strBuffer, 255, strEvent);
	}
	if(_pdstLink && _pdstLink->get(HASH_STATE(STR_NAME), (const void**)&strName))
	{
		strcat_s(strBuffer, 255, " SLink:");
		strcat_s(strBuffer, 255, strName);
	}
	strcat_s(strBuffer, 255, "\r\n");
	g_SendMessage(LOG_MSG, strBuffer);

	//OutputDebugStringA(strBuffer);
#endif
}

const STLVpState &BaseStateMain::get_active_state()
{
	return m_stlVpStateActive;
}

bool BaseStateMain::is_active_id(const STLMnInt *_pstlMnGroupId, int _nHash)
{
	for (unsigned i = 0; i<m_stlVpStateActive.size(); i++)
	{
		if (_nHash != 0 && m_stlVpStateActive[i]->get_key() != _nHash)
			continue;

		if (!_pstlMnGroupId)
			return true;

		if ((_nHash == 0 || m_stlVpStateActive[i]->get_key() == _nHash)
			&& (m_stlVpStateActive[i]->group_id_is_in(*_pstlMnGroupId)))
			return true;
	}
	return false;
}

bool BaseStateMain::is_active(int _nHash, const STLMnInt *_pstlMnGroupId)
{
	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		if(m_stlVpStateActive[i]->get_key() == _nHash
			&& (m_stlVpStateActive[i]->group_id_equal(*_pstlMnGroupId) || _pstlMnGroupId == NULL))
			return true;
	}
	return false;
}

bool BaseStateMain::is_conformer()
{
	return m_pSpace->is_conformer();
}

void BaseStateMain::grade_close(int _nGrade, BaseDStructureValue *_pEvent)
{
	m_nGradCloseTarget	= _nGrade;
}

void BaseStateMain::grade_close_()
{
	if(m_nGradCloseTarget == 0)
		return;

	for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
	{
		if(m_stlVpStateActive[i]->grade_get() <= m_nGradCloseTarget)
		{
			m_stlVpStateActive[i]->OnEnd(NULL);
			release_(m_stlVpStateActive[i]);
			i--;
		}
	}

	m_nGradCloseTarget	= 0;
}

void BaseStateMain::out_active(BaseState *_pState)
{
	STLVpState::iterator	it;
	it	= std::find(m_stlVpStateActive.begin(), m_stlVpStateActive.end(), _pState);
	if(it != m_stlVpStateActive.end())
	{
		BaseState *pState	= *it;
		pState->OnEnd(NULL);
		g_BreakPoint();

		m_stlVpStateActive.erase(it);

#ifdef _DEBUG
		if(m_nKeyName == -1522711694)
		{
			bool bFind = false;
			for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
			{
				if(m_stlVpStateActive[i]->obj_serial_get() == 572)// 277 SkillVictim State 0x00010000 was added or subbed
				{
					s_pDebugDataCheckPoint = (int*)(&m_stlVpStateActive[i]);
					bFind = true;
				}
			}
			if(s_pDebugDataCheckPoint && bFind == false)
				int x=0;
			g_BreakPoint();
		}
#endif
	}
}

void BaseStateMain::in_active(BaseState *_pState)
{
	g_BreakPoint();
	m_stlVpStateActive.push_back(_pState);

#ifdef _DEBUG
	if(m_nKeyName == -1522711694)
	{
		bool bFind = false;
		for(unsigned i=0; i<m_stlVpStateActive.size(); i++)
		{
			if(m_stlVpStateActive[i]->obj_serial_get() == 572)// 277 SkillVictim State 0x00010000 was added or subbed
			{
				s_pDebugDataCheckPoint = (int*)(&m_stlVpStateActive[i]);
				bFind = true;
			}
		}
		if(s_pDebugDataCheckPoint && bFind == false)
			int x=0;
		g_BreakPoint();
	}
#endif	//_pState->OnEvent();
}

void BaseStateMain::next_active_state_add(int _nKey, STLMnInt &_stlMnGroupId)
{
    STDEF_StateReserved stdStateResv;
    
    stdStateResv.nStateKey = _nKey;
    stdStateResv.stlMnGroupId = _stlMnGroupId;
    
    m_stlVResvActive.push_back(stdStateResv);
}

int BaseStateMain::_next_active_state_find(int _nKey, STLMnInt &_stlMnGroupId)
{
    for(unsigned i=0; i<m_stlVResvActive.size(); i++)
    {
        if(m_stlVResvActive[i].nStateKey == _nKey)
        {
            bool bFail = false;
            STLMnInt::iterator _it = _stlMnGroupId.begin();
            for(;_it!=_stlMnGroupId.end();_it++)
            {
                STLMnInt::iterator it = m_stlVResvActive[i].stlMnGroupId.find(_it->first);
                if(it != m_stlVResvActive[i].stlMnGroupId.end()
                   && it->second != _it->second) // second is identifier
                {
                    bFail = true;
                    break;
                }
            }
            
            if(!bFail){
                return i;
            }
        }
    }
    return -1;
}

bool BaseStateMain::next_active_state_check(int _nKey, STLMnInt &_stlMnGroupId)
{
    int nRet = _next_active_state_find(_nKey, _stlMnGroupId);
    if(nRet == -1)
        return false;
    return true;
}

bool BaseStateMain::next_active_state_del(int _nKey, STLMnInt &_stlMnGroupId)
{
    int nRet = _next_active_state_find(_nKey, _stlMnGroupId);
    if(nRet == -1)
        return false;
    m_stlVResvActive.erase(m_stlVResvActive.begin()+nRet);
    return true;
}

bool StateTableGroupId::get_group_id(int _nIndex, STLMnInt *_stlMnGroupId)
{
    if(_nIndex < 0 || _nIndex >= (int)m_stlVStateRecord.size())
        return false;
    
    *_stlMnGroupId = m_stlVStateRecord[_nIndex].stlMnGroupId;
    return true;
}

BaseDStructureValue *StateTableGroupId::get(int _nIndex)
{
	if (_nIndex < 0 || _nIndex >= (int)m_stlVStateRecord.size())
        return NULL;
    
    return m_stlVStateRecord[_nIndex].pStateValue;
}

BaseDStructureValue *StateTableGroupId::get(const STLMnInt &_stlMnGroup)
{
	for (int i = 0; i<(int)m_stlVStateRecord.size(); i++)
    {
        if(BaseState::group_id_equal(m_stlVStateRecord[i].stlMnGroupId, _stlMnGroup))
            return m_stlVStateRecord[i].pStateValue;
    }
    return NULL;
}

void StateTableGroupId::add(BaseDStructureValue *_pState, const STLMnInt &_stlMnGroup)
{
    STDEF_StateRecord stdRec;
    stdRec.pStateValue = _pState;
    stdRec.stlMnGroupId = _stlMnGroup;
    m_stlVStateRecord.push_back(stdRec);
}

void StateTableGroupId::set(BaseDStructureValue *_pState, const STLMnInt &_stlMnGroup)
{
	for (int i = 0; i<(int)m_stlVStateRecord.size(); i++)
    {
        if(BaseState::group_id_equal(m_stlVStateRecord[i].stlMnGroupId, _stlMnGroup))
        {
            m_stlVStateRecord[i].pStateValue = _pState;
            return;
        }
    }
    
    add(_pState, _stlMnGroup);
}


void StateTableGroupId::set(BaseDStructureValue *_pState, int _nIndex)
{
	if (_nIndex < 0 || _nIndex >= (int)m_stlVStateRecord.size())
        return;
    
    m_stlVStateRecord[_nIndex].pStateValue = _pState;
}

void StateTableGroupId::clear_state()
{
	for (int i = 0; i<(int)m_stlVStateRecord.size(); i++)
    {
        m_stlVStateRecord[i].pStateValue = NULL;
    }
}

void StateTableGroupId::clear()
{
    m_stlVStateRecord.clear();
}
