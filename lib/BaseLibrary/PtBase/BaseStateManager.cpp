#include "stdafx.h"
#include "BaseMemoryPool.h"
#include "BaseObject.h"
#include "BaseResResource.h"
#include "BaseState.h"
#include "BaseStateManager.h"
#include "BaseResManager.h"
#include "BaseStateSpace.h"
#include "BaseResDStructure.h"
#include "BaseResFile.h"
#include "BaseNetConnector.h"
#include "BaseResFilterIP.h"
#include "BaseFile.h"
#include "EncoderOperator.h"
#include "BaseEventHandler.h"
#include "BaseEventManager.h"

enum{
	STATE_KEY,
	STATE_NAME,
};

bool BaseStateManager::ms_terminated = false;
std::string		BaseStateManager::ms_strSavePath;// Add by OJ : 2010-07-21
//std::string		BaseStateManager::m_strRoot;// Add by OJ : 2010-07-21
//EncoderOperator	*BaseStateManager::m_pEncoder = NULL;

#define VER_ADDSERIAL
#ifdef VER_ADDSERIAL
//#define TRACE_STATE
#endif

BaseStateManagerLogic	BaseStateManagerLogic::sm_sample(NULL);
BaseStateManager		BaseStateManager::sm_sample(NULL);
BaseStateManager	*BaseStateManager::ms_manager_p;

BaseStateManagerLogic::BaseStateManagerLogic(void *_applet):m_queueAccessLock("ManagerLogic", 1000)
{	
	init(_applet);
}

BaseStateManagerLogic::~BaseStateManagerLogic(void)
{
	if(mpool_get().is_terminated())
		return;
	release();
}

BaseStateManager::BaseStateManager(void *_pApplet):BaseStateManagerLogic(_pApplet)
{	
	init(_pApplet);
}

BaseStateManager::~BaseStateManager(void)
{
	if(mpool_get().is_terminated())
		return;
	release();
}

BaseStateSpace *BaseStateManagerLogic::GetSpace(int _nSpaceID)
{
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
	{
		if(m_stlVpStateSpace[i]->space_id_get() == _nSpaceID)
			return m_stlVpStateSpace[i];
	}
	return NULL;
}

bool BaseStateManagerLogic::StateActivesGet(STLVpState *_states)
{
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
		m_stlVpStateSpace[i]->GetActiveState(_states);
	return !_states->empty();
}

BaseStateMain *BaseStateManagerLogic::GetStateMain(int _nHash, int _nSpaceID)
{
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
	{
		if(m_stlVpStateSpace[i]->space_id_get() == _nSpaceID)
			return m_stlVpStateSpace[i]->GetStateMain(_nHash);
	}
	return NULL;
}

void BaseStateManagerLogic::reload()
{
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
		m_stlVpStateSpace[i]->reset_to_reload();
	
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
		m_stlVpStateSpace[i]->reload();

	for (unsigned i = 0; i < m_stlVpStateSpace.size(); i++)
		m_stlVpStateSpace[i]->reload_start();
}

void BaseStateManagerLogic::render_after_push(FuncPostProcessor _pFunc, void *_pVoid)
{
	m_stlVFuncRenderAfter.push_back(std::make_pair(_pFunc, _pVoid));
}

void BaseStateManagerLogic::render_after_run()
{
	for(unsigned i=0; i<m_stlVFuncRenderAfter.size(); i++)
	{
		m_stlVFuncRenderAfter[i].first(m_stlVFuncRenderAfter[i].second);
	}
	m_stlVFuncRenderAfter.clear();
}

void BaseStateManagerLogic::post_processor_regist(FuncPostProcessor _pFunc, void *_pVoid)
{
	m_stlVFuncPostProcessors.push_back(std::make_pair(_pFunc, _pVoid));
}

void BaseStateManagerLogic::post_processor_run()
{
	if(m_stlVFuncPostProcessors.size() == 0)
		return;

	STLVFuncpVoid stlVFuncPostLocal;
	stlVFuncPostLocal	= m_stlVFuncPostProcessors;
	m_stlVFuncPostProcessors.clear();

	for(unsigned i=0; i<stlVFuncPostLocal.size(); i++)
	{
		stlVFuncPostLocal[i].first(stlVFuncPostLocal[i].second);
	}
}

int BaseStateManagerLogic::OnDraw()
{
	int	nRet	= 0;
//
//	if(m_pStateCurrent)
//	{
//		nRet	= m_pStateCurrent->OnDrawBefor();
//		if(!nRet)
//			return nRet;
//	}
//
//	if(m_pRndManager)
//	{
////		nRet	= ((PtRndManager*)m_pRndManager)->Draw();
//		if(!nRet)
//			return nRet;
//	}
//
//	if(m_pStateCurrent)
//	{
//		nRet	= m_pStateCurrent->OnDrawAfter();
//		if(!nRet)
//			return nRet;
//	}

	return nRet;
}

void BaseStateManagerLogic::resource_release_regist(int _nMSecLimit, void *_pRes)
{
	m_stlVReleaseMSecLimit.push_back(_nMSecLimit);
	m_stlVpReleaseRes.push_back(_pRes);
}

BaseDStructureValue *BaseStateManager::varialbe_global_get()
{
	if(m_pdsvVariableGlobal == NULL)
	{
		PT_OAlloc2(m_pdsvVariableGlobal, BaseDStructureValue, EnumGet(HASH_STATE(HASH_EnumEvent)), 1024);
		m_pdsvVariableGlobal->m_pVoidParam	= NULL;
		m_pdsvVariableGlobal->set_key(HASH_STATE(STATE_CONTEXT));
	}
	return m_pdsvVariableGlobal;
}

int BaseStateManager::variable_global(int _nHash)
{
	BaseDStructureValue *pGlobal = varialbe_global_get();
	const int *pnRet;
	if (pGlobal->get(_nHash, (const void**)&pnRet))
		return *pnRet;
	return 0;
}

void BaseStateManagerLogic::release()
{
	if (!AccessCountInc("release"))
		return;

	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
	{
		m_stlVpStateSpace[i]->dec_ref(this);
	}
	m_stlVpStateSpace.clear();

	AccessCountDec();

	if(m_eventManager_p)
		m_eventManager_p->dec_ref(this);
	m_eventManager_p = NULL;
	BaseObject::release();
}

void BaseStateManager::release()
{
	if (m_pResManager)
		((BaseResManager*)m_pResManager)->SaveReservedValue();

	BaseStateManagerLogic::release();

	if (m_pdsvVariableGlobal)
		PT_OFree(m_pdsvVariableGlobal);
	m_pdsvVariableGlobal = NULL;

	if (!AccessCountInc("Manager::release"))
		return;

	//{
	//	STLMnpBaseObject::iterator	it;
	//	it = m_stlMnpBaseTemp.begin();
	//	for(;it != m_stlMnpBaseTemp.end(); it++)
	//		PT_OFree(it->second);
	//	m_stlMnpBaseTemp.clear();
	//}
	{
		STLMnpdst::iterator it;
		it	= m_stlMnpdstEnum.begin();
		for(;it!=m_stlMnpdstEnum.end(); it++)
			PT_OFree(it->second);
		m_stlMnpdstEnum.clear();
	}

//	{
//		STLMnpStateLogic::iterator	it;
//		it	= m_stlMnpStateLogic.begin();
//		for(;it!=m_stlMnpStateLogic.end(); it++)
//			PT_OFree(it->second);
//		m_stlMnpStateLogic.clear();
//	}

	if(m_pResManager)
		((BaseObject*)m_pResManager)->dec_ref(this);

	m_pResManager	= NULL;

	if(m_pRndManager)
		((BaseObject*)m_pRndManager)->dec_ref(this);

	m_pRndManager	= NULL;
//*
	if (m_pEncoder)
	{
		PT_OFree(m_pEncoder);
		m_pEncoder = NULL;
	}
//*/
	AccessCountDec();
}

bool BaseStateManagerLogic::thread_start()
{
	/*
	if(BaseSystem::createthread(update_, 0, this) == -1){
		m_thread_run = false;
		return false;
	}
	m_thread_run = true;
	//*/
	m_thread_run = true;
	return true;
}

int BaseStateManagerLogic::init(void *_applet)
{	
	int	nRet=0;
	
	static int	s_iId = 0;
	if(s_iId == 0)
	{
		s_iId	= UniqHashStr::get_string_hash_code("BaseStateManagerLogic");
	}

    m_eventManager_p = NULL;
    if(optionGet(OPT_CreatePrivate))
    {
        //m_eventManager_p = BaseEventManager::get(this);
		PT_OAlloc(m_eventManager_p, BaseEventManager);
        m_eventManager_p->inc_ref(this);
	}
	m_eventSkip_b = false;
	m_nObjectId	= s_iId;
	m_nTimeCur	= 0;
	m_nTimeEventSequence = 0;
	m_bEventPostpond = false;

//	if(_strStateLogic)
//	{
//		BaseStateSpace	*pSpace;
//		pSpace	= new BaseStateSpace((BaseStateManager*)this, _strStateLogic, 0);
//		pSpace->inc_ref(this);
//		m_stlVpStateSpace.push_back(pSpace);
//	}

	return nRet;
}

int	BaseStateManagerLogic::actor_regist_keyvalue_func(int _nHash, FnGetKeyValue _pFunc)
{
	m_stlMnFnGetKeyValue[_nHash]	= _pFunc;
	return (int)m_stlMnFnGetKeyValue.size();
}

bool BaseStateManagerLogic::actor_get_keyvalue(const BaseDStructureValue *_pdstEvent, int *_pnValue)
{
	STLMnFnGetKeyValue::iterator	it;
	it	= m_stlMnFnGetKeyValue.find(_pdstEvent->get_key());
	if(it != m_stlMnFnGetKeyValue.end())
	{
		*_pnValue = it->second(_pdstEvent);
		return true;
	}
	return false;
}

bool BaseStateManagerLogic::AccessCountInc(const char* _strFunc)
{
	//if (BaseSystem::LFIncrement(&m_nAccessCount) > 1)
	m_queueAccessLock.push((void*)_strFunc);
	
	while(m_queueAccessLock.size_data() > 1){
		static int s_count = 0;

		s_count++;
		if(s_count > 10)
		{
			g_SendMessage(LOG_MSG_POPUP, "Dualupdate for %s PreFunc:%s PreColumn:%s", _strFunc, m_strNameOfFunc.c_str(), m_strCurColumProcessing.c_str());
			m_queueAccessLock.pop();
			s_count = 0;
			return false;
		}
		m_queueAccessLock.qsleep(10);
	}
	m_strNameOfFunc = _strFunc;
	return true;
}

void BaseStateManagerLogic::AccessCountDec()
{
	m_queueAccessLock.pop();
	m_strNameOfFunc = "";
}

void BaseStateManager::res_manager_root_set(const char *_strRootRes)
{
	if (m_pResManager == NULL)
	{
		char buff[255];
		strcpy_s(buff, 255, _strRootRes);
		int len = (int)strlen(buff);
		buff[len - 1] = 0;
		char *end = strrchr(buff, SLASH_C);
		if (end)
			* (end+1) = 0;
		m_pResManager = BaseResManager::get_manager(buff);
		((BaseObject*)m_pResManager)->inc_ref(this);
	}
}

bool state_file_is(const char *_strName) {
	char strBuffer[1024];

	strcpy_s(strBuffer, 1024, _strName);
	strcat_s(strBuffer, 1023, ".data");

	if (BaseFile::is_exist(strBuffer))
		return true;
	
	strcpy_s(strBuffer, 1024, _strName);
	strcat_s(strBuffer, 1023, ".ini");

	if (BaseFile::is_exist(strBuffer))
		return true;
	return false;
}

#include "BaseXML.h"

bool state_add(BaseDStructure *_dst,const char *_name)
{
	int key = STRTOHASH(_name);
	_dst->add_row_alloc(key, 0, &key);
	_dst->set_alloc(key, 1, _name);
	return true;
}

bool BaseStateManager::main_remove(int _hash)
{
    if(!m_dstMain.is_exist(_hash))
        return false;
    if(!m_dstMain.release(_hash))
        return false;
    if(!m_dstMain.save())
        return false;
    return true;
}

bool BaseStateManager::main_merge(int _hash, const char *_target)
{
	if(!m_dstMain.is_exist(_hash))
		return false;
	
	BaseStateMain *main_p = GetStateMain(_hash, 0);
	BaseDStructure *source_pdst = NULL;
	
	if(main_p)
		source_pdst = main_p->get_state();
	else
		source_pdst = EnumGet(_hash);
	
	if(!source_pdst)
		return false;
	
	BaseDStructure target_dst;
	
	if(!target_dst.load(_target))
		return false;
	
	STLMnstlRecord::iterator it;
	int key = target_dst.get_first_key(&it);
	
	int cnt = 0;
	while(key)
	{
		if(!source_pdst->is_exist(key))
		{
			const char *name_str;
			target_dst.get(key, STATE_NAME, (const void**)&name_str);
			
			state_add(source_pdst, name_str);
			cnt++;
		}else{
			const char *name_str;
			target_dst.get(key, STATE_NAME, (const void**)&name_str);
			const char *name_str2;
			source_pdst->get(key, STATE_NAME, (const void**)&name_str2);
			if(strncmp("Game", name_str, 4) == 0)
			{
				printf("hello");
			}
		}
		key = target_dst.get_next_key(&it);
	}
	source_pdst->save();
	
	return true;
}

bool BaseStateManager::main_add(const char *_main_str)
{
	if(strlen(_main_str) < 4)
		return false;
	//m_dstMain
	int key = STRTOHASH(_main_str);
	if(m_dstMain.is_exist(key))
		return false;
	
	state_add(&m_dstMain, _main_str);
	BaseDStructure dstMain;
	m_dstMain.save();
	return true;
}

bool BaseStateManager::main_create(const char *_main_str)
{
    if(strlen(_main_str) < 4)
        return 0;
	//m_dstMain
    int key = STRTOHASH(_main_str);
    if(m_dstMain.is_exist(key))
        return false;
    
	state_add(&m_dstMain, _main_str);
	BaseDStructure dstMain;
	state_add(&dstMain, "start_state");
	state_add(&dstMain, "state_dummy");
	state_add(&dstMain, "state_context");
	state_add(&dstMain, "state_release");
	state_add(&dstMain, "state_return");
	m_dstMain.save();
    STLString path = m_dstMain.get_name();
    path = path.substr(0, path.find_last_of('/'));
    path += "/";
    path += _main_str;
    dstMain.set_name(path.c_str());
	dstMain.save();
    STLString link = path;
    link += "Link";
    BaseDStructure dstLink;
    dstLink.set_name(link.c_str());
    dstLink.save();
	return true;
}
int BaseStateManager::load()
{
	REGISTEOBJECT(BaseResDStructure::OBJECTID_DSTRUCTURE, BaseResDStructure);
	REGISTEOBJECT(BaseResFile::GetObjectId(), BaseResFile);
	REGISTEOBJECT(BaseResFilterIP::GetObjectId(), BaseResFilterIP);
	REGISTEOBJECT(BaseXML::GetObjectId(), BaseXML);
		
	int nRet	= 0;
	if (!AccessCountInc("Manager::load"))
		return nRet;

	std::string	strPath;

	//BaseDStructure	stdMains;
	m_dstMain.release();

	strPath = m_strRoot;
	strPath += "hash.ini";
	BaseConsolFilter::load_hash(strPath.c_str());
	
	if (m_pEncoder == NULL)
	{
		//strPath = m_strRoot;
		//strPath += "encoder.cod";
		//PT_OAlloc(m_pEncoder, EncoderOperator);
		//STLVCoderIndex	stlVCoder;
		//m_pEncoder->set_encoder(strPath.c_str(), stlVCoder);
	}

	strPath	= m_strRoot;
	strPath	+= "StateList";
	g_SendMessage(LOG_MSG, "Load()%s", strPath.c_str());
	if (!m_dstMain.load(strPath.c_str()))
	{
		g_SendMessage(LOG_MSG_POPUP, "Fail to load the 'StateList'.");
		return 0;
	}

	STLMnstlRecord::iterator	it;
	int nKey;

	nKey	= m_dstMain.get_first_key(&it);
	while(nKey)
	{
		const char *strName;
		if(m_dstMain.get(nKey, STATE_NAME, (const void**)&strName))
		{
			if(strncmp("Enum", strName, 4) == 0)
			{
				EnumAdd(strName);// Add by OJ : 2010-05-10
			}
//            else if(strncmp("Logic_", strName, 6) == 0){
//				LogicAdd(strName);// Add by OJ : 2010-07-21
//			}
		}
		nKey	= m_dstMain.get_next_key(&it);
	}

	nKey = m_dstMain.get_first_key(&it);
	while (nKey)
	{
		const char* strName;
		if (m_dstMain.get(nKey, STATE_NAME, (const void**)&strName))
		{
			if (strncmp("Enum", strName, 4) == 0)
			{
				//EnumAdd(strName);// Add by OJ : 2010-05-10
			}
			else if (strncmp("Logic_", strName, 6) == 0) {
				//LogicAdd(strName);// Add by OJ : 2010-07-21
			}
			else {
				strPath = m_strRoot;
				strPath += strName;

				if (state_file_is(strPath.c_str())) {
					if (m_stlVpStateSpace.size() == 0)
					{
						BaseStateSpace* pSpace = new BaseStateSpace(this, NULL, 0);
						pSpace->inc_ref(this);
						m_stlVpStateSpace.push_back(pSpace);
					}
					for (unsigned i = 0; i < m_stlVpStateSpace.size(); i++)
					{
						m_stlVpStateSpace[i]->add_main(strPath.c_str());
					}
				}
			}
		}
		nKey = m_dstMain.get_next_key(&it);
	}

	AccessCountDec();

	BaseDStructure* pdstEnumKey;
	pdstEnumKey = EnumGet(HASH_STATE(HASH_EnumEventKey));

	STLMnstlRecord::iterator itkey = pdstEnumKey->it_begin_get();
	for (; itkey != pdstEnumKey->it_end_get(); itkey++)
	{
		const int* scan_p;
		if (pdstEnumKey->get_by_hash(itkey->first, HASH_STATE(HASH_nScanCode), (const void**)& scan_p))
		{
			m_scan2keyhash_m[*scan_p] = itkey->first;
		}
	}

	BaseDStructure::processor_init();
	return nRet;
}

void BaseStateManager::space_start(int _nSpaceID)
{
	if(m_stlVpStateSpace.size() > 0)
	{
		BaseStateSpace *pSpace = space_get(_nSpaceID);
		if(pSpace == NULL)
			pSpace	= space_create(_nSpaceID);
	
		m_stlVpStateSpace[0]->copy_space(pSpace);
		pSpace->StateStart();
	}
}

void BaseStateManager::reload()
{
	{
		STLMnpdst::iterator	it;
		it	= m_stlMnpdstEnum.begin();
		for(;it!=m_stlMnpdstEnum.end(); it++)
		{
			PT_OFree(it->second);
		}
		m_stlMnpdstEnum.clear();

//		STLMnpStateLogic::iterator	itLogic;
//		itLogic	= m_stlMnpStateLogic.begin();
//		for(;itLogic!=m_stlMnpStateLogic.end(); itLogic++)
//		{
//			PT_OFree(itLogic->second);
//		}
//		m_stlMnpStateLogic.clear();
	}
	
	{
		std::string	strPath;
		//BaseDStructure	stdMains;
		m_dstMain.release();
		strPath	= m_strRoot;
		strPath	+= "StateList";
		//BaseDStructure::static_clear();
		//BaseDStructure::init_type();
		BaseDStructure::load_define_(strPath.c_str(), 2); // reload BaseDefine.define, 2 mean reload force
		m_dstMain.load(strPath.c_str());

		STLMnstlRecord::iterator	it;
		int nKey;

		nKey	= m_dstMain.get_first_key(&it);
		while(nKey)
		{
			const char *strName;
			if(m_dstMain.get(nKey, STATE_NAME, (const void**)&strName))
			{
				if(strncmp("Enum", strName, 4) == 0)
				{
					EnumAdd(strName);// Add by OJ : 2010-05-10
				}
//                else if(strncmp("Logic_", strName, 6) == 0){
//					LogicAdd(strName);// Add by OJ : 2010-07-21
//				}
			}
			nKey	= m_dstMain.get_next_key(&it);
		}
	}

	BaseStateManagerLogic::reload();
}

int BaseStateManager::init(void *_pApplet)
{
    BaseStateManagerLogic::init(_pApplet);
	m_nDebugBlockCastMessage	= 0;
	m_queueAccessLock.init(1000);
	m_bConformer	= false;
	m_pEncoder = NULL;
	m_pdsvVariableGlobal = NULL;
	ms_terminated = false;
	//ms_enumStructureCustom_p = NULL;

	m_pApplet	= _pApplet;
	int	nRet=0;

	static int	s_iId = 0;
	if(s_iId == 0)
		s_iId	= UniqHashStr::get_string_hash_code("BaseStateManager");

	m_nObjectId	= s_iId;

	m_pRndManager	= 0;
	m_pResManager	= 0;
	
	m_stlVPushedKey.resize(1000);
	for(unsigned i=0; i<m_stlVPushedKey.size(); i++)
		m_stlVPushedKey[i]	= 0;
	//
	////m_pRndManager	= (PtRndManager*)NEW(PtRndManager);
	////m_pResManager	= (BaseResManager*)NEW(BaseResManager(_pApplet));

	////((BaseObject*)m_pResManager)->IncRef(this);
	////((BaseObject*)m_pRndManager)->IncRef(this);
	////nRet	= ((PtRndManager*)m_pRndManager)->init(_pApplet);

	return nRet;
}
//
//BaseDStructure *BaseStateManager::GetStateEvent()
//{
//	return &m_dstEvent;
//}
//
//BaseDStructure *BaseStateManager::GetStateEventKey()
//{
//	return &m_dstEventKey;
//}
void BaseStateManagerLogic::post_event_serial(BaseDStructureValue* _event_pdsv, int _serial)
{
	_event_pdsv->set_alloc(HASH_STATE(BaseEventTargetStateSerial), &_serial);
	post_event(_event_pdsv);
}

void BaseStateManagerLogic::push_event(BaseDStructureValue *_pdstEvent, int _priority_n)
{
	const unsigned	*pnTime;
	int spaceid = 0;
	_pdstEvent->set_alloc(HASH_STATE(HASH_BaseTransitionGoalSpaceID), (void*)&spaceid);
	
	unsigned nTimeOut;
	nTimeOut = m_nTimeCur + m_nTimeEventSequence;
	if(_pdstEvent->get(HASH_STATE(TIMETOPROCESS), (const void**)&pnTime))
		nTimeOut	= m_nTimeCur + *pnTime;

	//m_nTimeEventSequence++; it's commented by bellow reason.
	// Events what pushed at same time, sometimes postponed by higher priority with added sequence.
	// But higher priority event has time what later than current. but didn't intended.
	// it just added by ~Sequence values. So it should prossed and also others.
	_pdstEvent->set_alloc(HASH_STATE(TIMETOPROCESS), (void*)&nTimeOut);

#ifdef TRACE_STATE
	//if(m_nDebugBlockCastMessage == 0)
	if(0){
		int nKey	= _pdstEvent->get_key();
		int nValue	= 0;
		const int *pnValue = &nValue;
		//const int *pnId = &nValue;
		_pdstEvent->get(nKey, (void const**)&pnValue);
		if(pnValue == NULL)
			pnValue = &nValue;
		
		STLMnInt stlMnGroup;
		BaseState::group_id_get(_pdstEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);
		
		//_pdstEvent->get(HASH_STATE(HASH_BaseTransitionGoalIdentifier), (void const**)&pnId);
		//if(pnId == NULL)
		//	pnId = &nValue;
		
		char strBuf[255] = "", strCat[255] = "";
		for(STLMnInt::iterator it=stlMnGroup.begin(); it!=stlMnGroup.end(); it++)
		{
			sprintf_s(strCat, 255, "GID:%d S:%d ", it->first, it->second);
			strcat_s(strBuf, 255, strCat);
		}
		g_SendMessage(LOG_MSG, "SAction:PushEvent %s SEvent:%d EValue:%d\n", strBuf, nKey, *pnValue);
	}
#endif
	m_eventManager_p->push_event(_pdstEvent, _priority_n);
	
	if(m_queueAccessLock.size_data() == 0)
		OnUpdate(0);
}

void BaseStateManagerLogic::post_systemevent(BaseDStructureValue *_pdstEvent, int _nSpaceID, int _nPriority)
{
	push_event(_pdstEvent, _nPriority);
}

void BaseStateManagerLogic::post_event(BaseDStructureValue *_pdstEvent, int _nSpaceID, int _nPriority)
{
	BaseEventHandler::event_push(_pdstEvent, _nPriority);
}

bool BaseStateManagerLogic::OnEvent(BaseDStructureValue *_pdstEvent)
{
#ifdef TRACE_STATE
	if(m_nDebugBlockCastMessage == 0){
		int nKey	= _pdstEvent->get_key();
		int nValue	= 0;
        const int *pnValue = &nValue;//, *pnId = &nValue;
		_pdstEvent->get(nKey, (void const**)&pnValue);
		
        STLMnInt stlMnGroup;
        BaseState::group_id_get(_pdstEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);
        
        char strBuf[255] = "", strCat[255] = "";
        for(STLMnInt::iterator it=stlMnGroup.begin(); it!=stlMnGroup.end(); it++)
        {
            sprintf_s(strCat, 255, "GID:%d S:%d ", it->first, it->second);
            strcat_s(strBuf, 255, strCat);
        }
        g_SendMessage(LOG_MSG, "SAction:ProcessEvent %s SEvent:%d EValue:%d\n", strBuf, nKey, *pnValue);
        
		//_pdstEvent->get(HASH_STATE(HASH_BaseTransitionGoalIdentifier), (void const**)&pnId);
		//if(pnId == NULL)
		//	pnId = &nValue;
		//g_SendMessage(LOG_MSG, "SAction:ProcessEvent,%d SEvent:%d EValue:%d", *pnId, nKey, *pnValue);
	}
#endif
	if (!AccessCountInc("OnEvent"))
		return false;
	bool bRet;
	bRet = OnEvent_(_pdstEvent);
	PT_OFree(_pdstEvent);

	AccessCountDec();

	return bRet;
}

void BaseStateManager::state_active_show_all(void)
{
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
	{
		m_stlVpStateSpace[i]->state_active_show_all();
	}

	for (unsigned i = 0; i < m_stlVpNetManager.size(); i++)
	{
		g_SendMessage(LOG_MSG_CONSOLE, "NetManager port %d\n", ((BaseNetManager*)m_stlVpNetManager[i])->server_port_get());
		((BaseNetManager*)m_stlVpNetManager[i])->print_all_connection();
	}
}

void BaseStateManagerLogic::event_skip(bool _skip_b)
{
	m_eventSkip_b = _skip_b;
}

bool BaseStateManagerLogic::event_skip_get()
{
	return m_eventSkip_b;
}

bool BaseStateManagerLogic::OnEvent_(BaseDStructureValue *_pdstEvent)
{
	const int *pnSerial = NULL;
	_pdstEvent->get(HASH_STATE(BaseEventTargetStateSerial), (const void**)&pnSerial);

	if (pnSerial && *pnSerial != 0) // in case of target active state is already defined
	{
		BaseState *pState = GetState(*pnSerial);

#ifdef _DEBUG
		if (pState == NULL)
		{
			g_SendMessage(LOG_MSG, "OnEvent_ no state for serialnetmatch:%d", *pnSerial);
		}
		else {
			g_SendMessage(LOG_MSG, "OnEvent_ on state for serialnetmatch:%d state:%d", *pnSerial, pState->get_key());
		}
#endif

		if (pState && pState->OnEvent(_pdstEvent) == BaseState::RET_PROCESS)
			return true;
	}
	else{
		m_eventSkip_b = false;
		for (unsigned i = 0; i<m_stlVpStateSpace.size(); i++)
		{
			if (m_stlVpStateSpace[i]->OnEvent(_pdstEvent) == BaseState::RET_PROCESS)
			{	
				return true;
			}
			if (m_eventSkip_b)
				break;
		}
	}
	
	return false;
}

bool	BaseStateManagerLogic::OnUpdateSpace(unsigned _nTimeCurrent)
{
	for (unsigned i = 0; i < m_stlVpStateSpace.size(); i++) {
		m_stlVpStateSpace[i]->OnUpdate(_nTimeCurrent - m_nTimeCur);
	}
	m_nTimeCur = _nTimeCurrent;
	return true;
}

bool BaseStateManagerLogic::OnUpdateInLock(unsigned _nTimeCurrent)
{
	if(m_nTimeCur == 0)
		m_nTimeCur	= _nTimeCurrent;

	if(m_nTimeCur != 0 && _nTimeCurrent == 0)
		_nTimeCurrent	= m_nTimeCur;
	
	if(m_nTimeCur > _nTimeCurrent)
	{
		_nTimeCurrent += (-1 - m_nTimeCur);
		m_nTimeCur = 0;
	}

	unsigned nDelta = _nTimeCurrent - m_nTimeCur;
	for (int i = 0; i < (int)m_stlVReleaseMSecLimit.size(); i++)
	{
		if (m_stlVReleaseMSecLimit[i] < (int)nDelta)
		{
			PT_Free(m_stlVpReleaseRes[i]);
			m_stlVReleaseMSecLimit.erase(m_stlVReleaseMSecLimit.begin() + i);
			m_stlVpReleaseRes.erase(m_stlVpReleaseRes.begin() + i);

			i--;
			continue;
		}

		m_stlVReleaseMSecLimit[i] -= nDelta;
	}

	BaseDStructureValue *pstdEvent, *p2;

	m_stlMnpRetValue.clear();

	pstdEvent	= m_eventManager_p->get_top(_nTimeCurrent+m_nTimeEventSequence);
	p2 = pstdEvent;

	m_nTimeEventSequence = 0;
	do
	{	
		m_bEventRequestUpdate	= false;
		if(pstdEvent)
		{
			pstdEvent->ref_inc();
			if (pstdEvent->ref_get() == 1)
			{
				int x = 0;
			}
			m_eventManager_p->pop_event();
			const int* pnValue;
#ifdef TRACE_STATE
			//if(m_nDebugBlockCastMessage == 0){
			static int MassEvents[5] = { 0 };
			if (MassEvents[0] == 0) {
				MassEvents[0] = STRTOHASH("state_call");
				MassEvents[1] = STRTOHASH("state_return");

				MassEvents[2] = STRTOHASH("UxPointMoveMiddle");
				MassEvents[3] = STRTOHASH("UxPointMove");
			}
			int nKey = pstdEvent->get_key();
			int nValue = 0;
			pnValue = &nValue;
			pstdEvent->get(nKey, (void const**)& pnValue);
			if (pnValue == NULL) pnValue = &nValue;

			if(nKey != MassEvents[0] 
				&& nKey != MassEvents[1]
				&& *pnValue != MassEvents[2]
				&& *pnValue != MassEvents[3]){
				STLMnInt stlMnGroup;
				STLMnInt::const_iterator	itGroup;
				BaseState::group_id_get(pstdEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroup);
				itGroup = stlMnGroup.begin();
				if(itGroup != stlMnGroup.end())
					g_SendMessage(LOG_MSG, "SAction:PopEvent GID:%d S:%d SEvent:%d EValue:%d\n", itGroup->first, itGroup->second, nKey, *pnValue);
				else
					g_SendMessage(LOG_MSG, "SAction:PopEvent SID:0::0 SEvent:%d EValue:%d\n", nKey, *pnValue);
			}
#endif
			int nRet	= BaseState::RET_UNPROCESS;

			m_bEventStateAffected	= false;

			if(OnEvent_(pstdEvent))
				nRet	= BaseState::RET_PROCESS;
			
			if(nRet == BaseState::RET_UNPROCESS)
			{
				if(pstdEvent->get_key() == HASH_STATE(BaseStateEventGlobal))
				{
					post_event_state_unprocess(pstdEvent);
				}
			}
			
			if(m_bEventStateAffected == false)
			{
				if(pstdEvent->get_key() == HASH_STATE(HASH_BaseKeyPush_nRE))
				{
					int wParam;
					int lParam;

					//const int *pnValue;
					pstdEvent->get(HASH_STATE(HASH_nScanCode), (const void**)&pnValue);
					wParam	= *pnValue;
					pstdEvent->get(HASH_STATE(HASH_nScanCode2), (const void**)&pnValue);
					lParam	= *pnValue;

					if(lParam != 0)// ???? ???T?? ?????
						keydn_virtual(wParam, lParam);
				}else if(pstdEvent->get_key() == HASH_STATE(HASH_BaseKeyRelease_nRE))
				{
					int wParam;
					int lParam;

					pstdEvent->get(HASH_STATE(HASH_nScanCode), (const void**)&pnValue);
					wParam	= *pnValue;
					pstdEvent->get(HASH_STATE(HASH_nScanCode2), (const void**)&pnValue);
					lParam	= *pnValue;

					if(lParam != 0)// ???? ???T?? ?????
						keyup_virtual(wParam, lParam);
				}
			}

			pstdEvent->ref_dec();
			if (m_bEventPostpond)
			{
				OnUpdateSpace(_nTimeCurrent);
				m_bEventPostpond = false;
				break;
			}
			
			pstdEvent	= m_eventManager_p->get_top(_nTimeCurrent);
		}

		if(pstdEvent == NULL && _nTimeCurrent > m_nTimeCur)
		{
			OnUpdateSpace(_nTimeCurrent);
			pstdEvent	= m_eventManager_p->get_top(_nTimeCurrent);
		}
	}while(pstdEvent);

	if(m_stlMnpRetValue.size())
		return true;
	return false;
}

bool BaseStateManager::OnUpdate(unsigned _nTimeCur)
{
	g_base_idle();

	if(m_first)
	{
		StateStart();
		m_first = false;
	}
	BaseResManager::get_manager()->LoadProcess();
	
	return BaseStateManagerLogic::OnUpdate(_nTimeCur);
}

bool BaseStateManagerLogic::OnUpdate(unsigned _nTimeCurrent)
{
	if (!AccessCountInc("OnUpdate"))
		return false;
	
	bool bRet = OnUpdateInLock(_nTimeCurrent);
	post_processor_run();// Post Processor registed by State Coloms
	AccessCountDec();
	return bRet;
}

int BaseStateManagerLogic::StateStart()
{
	if (!AccessCountInc("StateStart"))
		return 0;

	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
	{
		m_stlVpStateSpace[i]->StateStart();
	}

	AccessCountDec();
	return 0;
}

bool BaseStateManager::key_is_pushed(int _nKey)
{
	return (m_stlVPushedKey[_nKey] == 1);
}

void BaseStateManager::process_event_key(int _nScanCode, bool _bPush, int _nScanCode2)
{
	if (_bPush)
	{
		if (m_stlVPushedKey[_nScanCode] == 1)
			return; // doesn't changed
	}
	else {
		if (m_stlVPushedKey[_nScanCode] == 0)
			return; // doesn't changed
	}


	int	nTargetMain, nIdt;
	nTargetMain		= 0;
	nIdt			= 0;

	int nKey;

	if(_bPush)
	{
		nKey = HASH_STATE(HASH_BaseKeyPush_nRE);
		m_stlVPushedKey[_nScanCode]	= 1;
	}else{
		nKey = HASH_STATE(HASH_BaseKeyRelease_nRE);
		m_stlVPushedKey[_nScanCode]	= 0;
	}

	STLMnInt::iterator it;
	it = m_scan2keyhash_m.find(_nScanCode);
	if (it == m_scan2keyhash_m.end())
		return;
	int nHashKey = it->second;

	BaseDStructureValue *pdsvEventKey = make_event(nKey, nHashKey, nIdt);

	if(!pdsvEventKey->get_base()->is_exist(nKey))
	{
#ifdef TRACE_STATE
		g_SendMessage(LOG_MSG, "Doesn't exist Key Event\n");
#endif
		//OutputDebugStringA("Doesn't exist Event\n");
		PT_OFree(pdsvEventKey);
		return;
	}

	pdsvEventKey->set_alloc(HASH_STATE(HASH_nScanCode), &_nScanCode);
	pdsvEventKey->set_alloc(HASH_STATE(HASH_nScanCode2), &_nScanCode2);

	int nSpaceID	= 0;
	if(m_stlVpStateSpace.size() > 0)
		nSpaceID	= m_stlVpStateSpace[0]->space_id_get();
	
	post_event(pdsvEventKey, nSpaceID);
}
//==========================================================================
// Start Update by OJ : 2010-07-21
//BaseStateManagerLogic	*BaseStateManager::LogicAdd(const char *_strFilename)
//{
//	int nHash	= UniqHashStr::get_string_hash_code(_strFilename);
//
//	STLMnpStateLogic::iterator	it;
//	it	= m_stlMnpStateLogic.find(nHash);
//	B_ASSERT(it == m_stlMnpStateLogic.end());
//
//	BaseStateManagerLogic	*pManagerLogic	= NULL;
//	if(it == m_stlMnpStateLogic.end())
//	{
//		std::string	strPath;
//
//		strPath	= m_strRoot;
//		strPath	+= _strFilename;
//
//		if (!state_file_is(strPath.c_str()))
//			return NULL;
//
//		PT_OAlloc1(pManagerLogic, BaseStateManagerLogic, strPath.c_str());
//
//		m_stlMnpStateLogic[nHash]	= pManagerLogic;
//		pManagerLogic->inc_ref(this);
//	}
//
//	return pManagerLogic;
//}


//BaseStateManagerLogic*	BaseStateManager::LogicGet(int _nHash)
//{
//	STLMnpStateLogic::iterator	it;
//	it	= m_stlMnpStateLogic.find(_nHash);
//
//	//BaseStateMain	*pMain	= NULL;
//	if(it != m_stlMnpStateLogic.end())
//	{
//		return it->second;
//	}
//
//	return NULL;
//}
//================================= End Update by OJ : 2010-07-21

//----================================================================================
BaseDStructure	*BaseStateManager::EnumAdd(const char *_strFilename)
{
	BaseDStructure	*pdstEnum=NULL;

	int nHash	= UniqHashStr::get_string_hash_code(_strFilename);

	std::string	strPath;
	strPath	= m_strRoot;
	strPath	+= _strFilename;

	if(!state_file_is(strPath.c_str()))
		return NULL;

	STLMnpdst::iterator	it;
	it	= m_stlMnpdstEnum.find(nHash);
	//B_ASSERT(it == m_stlMnpdstEnum.end());

	if(it == m_stlMnpdstEnum.end())
	{
		PT_OAlloc(pdstEnum, BaseDStructure);
		pdstEnum->load(strPath.c_str());

		m_stlMnpdstEnum[nHash]	= pdstEnum;
	}
	
	if (strcmp(_strFilename, "EnumStructure") == 0
		&& m_enumStructureCustom_p) // EnumStructure is appended from DB structure, not from file. So there should be added aftet load file.
	{
		STLMnstlRecord::iterator it2;
		short index = m_enumStructureCustom_p->get_index(HASH_STATE(BaseVariableToVariable_anV));
		int key = m_enumStructureCustom_p->get_first_key(&it2);
		while(key){
			const char* name_str = NULL;
			const int *hash_p;

			if (!m_enumStructureCustom_p->get(key, 1, (const void**)&name_str))
				continue;
			pdstEnum->set_alloc(key, 0, (void*)&key);
			pdstEnum->set_alloc(key, 1, (void*)name_str);
	
			int seq = 0;
			while (m_enumStructureCustom_p->get(key, index, (const void**)&hash_p, NULL, seq))
			{
				pdstEnum->add_row_alloc(key, index, (void*)hash_p, 1);
				seq++;
			}

			key = m_enumStructureCustom_p->get_next_key(&it2);
		}
	}

	if (strcmp(_strFilename, "EnumVariableDefine") == 0)
	{
		pdstEnum->variable_defines_for_enum(this);
	}

	return pdstEnum;
}


bool BaseStateManager::variable_set(BaseDStructureValue *_pdsvVar, int _nHash, int _nType, const void *_pValue, short _nCnt)
{
	if (!variable_define(_nHash, _nType, true))
		return false;
	_pdsvVar->set_alloc(_nHash, _pValue, _nCnt);
	return true;
}

// it's append variable on a structure.
// so you can use by BaseVariableToSturecture
// before to use this function, should defined by structure_define fuction or defined in EnumStructure.
bool BaseStateManager::structure_add(int _key, const STLVString &_var_stra)
{
	BaseDStructure* pstEnum = EnumGet(HASH_STATE(EnumStructure));

	if (!pstEnum->is_exist(_key))
		return false;

	short index = pstEnum->get_index(HASH_STATE(BaseVariableToVariable_anV));
	for (unsigned i = 0; i < _var_stra.size(); i++)
	{
		if (index < 0)
			return false;
		int hash = STRTOHASH(_var_stra[i].c_str());// "dbcolumn_~~~~~" all of variable should have variable name befor to join this structure
		pstEnum->add_row_alloc(_key, index, (void*)&hash, 1);
		m_enumStructureCustom_p->add_row_alloc(_key, index, (void*)&hash, 1);
	}
    return true;
}

// basically structure would defined at EnumStructure in the StateEditor
// but CRUD process hard to define all of table, so most of sturecture for CURD, defined by automatically.
// This function support automatic sturucture define.
bool BaseStateManager::structure_define(const char* _structure_name, const STLVString& _var_stra)
{
	int key = STRTOHASH(_structure_name);
	BaseDStructure* pstEnum = EnumGet(HASH_STATE(EnumStructure));

	if (pstEnum->is_exist(key))
		return false;

	if (m_enumStructureCustom_p == NULL)
		PT_OAlloc(m_enumStructureCustom_p, BaseDStructure);

	pstEnum->set_alloc(key, 0, (void*)&key);
	pstEnum->set_alloc(key, 1, (void*)_structure_name);

	m_enumStructureCustom_p->set_alloc(key, 0, (void*)&key);
	m_enumStructureCustom_p->set_alloc(key, 1, (void*)_structure_name);

	structure_add(key, _var_stra);
	return true;
}

bool BaseStateManager::variable_define(int _nHash, int _nType, bool bForce)
{
	if (BaseDStructure::get_index(_nHash) == -1)
	{
		const char *strName = NULL;
		BaseDStructureValue	dsvValue(EnumGet(HASH_STATE(EnumVariableDefine)), 0);
		
		if(!dsvValue.get_base()->is_exist(_nHash))
		{
			if (!bForce)
				return false;
		}
		else {
			dsvValue.set_key(_nHash);
			if (dsvValue.get(HASH_STATE(STR_NAME), (const void**)&strName))
			{
				int nLen = (int)strlen(strName);
				if (strncmp(strName + nLen - 4, "strV", 4) == 0)
					_nType = TYPE_STRING;
			}
			else {
				return false;
			}
		}

		char strBuf[1024];
		if (strName == NULL) {
			sprintf_s(strBuf, 1024, "var:%d", _nHash);
			strName = strBuf;
		}

		BaseDStructure::add_column(0, _nHash, _nType, 0, NULL, STR_COMMENT_APPENDED_VARIABLE, strName);
	}
	return true;
}

BaseDStructureValue *BaseStateManager::variable_type(int _nType, const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, BaseDStructureValue *_pdsvEvent, bool _bReceiver)
{
	STDEF_BaseState(pState);
	static int anHash[10] = { 0 };
	if (anHash[0] == 0)
	{
		anHash[0] = STRTOHASH("ContextNext");
	}
	if (_nType == HASH_STATE(Context))
		return _pdsvContext;
	else if (_nType == HASH_STATE(EventCast))
		return pState->EventSendGet();
	else if (_nType == HASH_STATE(EventCastSystem))
		return pState->EventSystemGet();
	else if (_nType == HASH_STATE(Event))
		return _pdsvEvent;
	else if (_nType == HASH_STATE(State))
		return ((BaseState*)_pdsvBase->m_pVoidParam)->variable_get();
	else if (_nType == HASH_STATE(Global))
		return varialbe_global_get();
	else if (_nType == anHash[0])// ContextNext
	{
		STLMnInt stlMnGroupId;
		pState->groupid_get_next(&stlMnGroupId, _pdsvEvent);
		BaseDStructureValue	*pContext = pState->context_reserve(stlMnGroupId);
		if (pContext == NULL)
			pContext = pState->context_get();
		return pContext;
	}
	else if (_nType == HASH_STATE(StateNext))
	{
		return ((BaseState*)_pdsvBase->m_pVoidParam)->variable_next_reserve();
	}
	else if (_nType == HASH_STATE(File)) {
		if (_bReceiver) {
			BaseDStructureValue *pdsvValue;
			PT_OAlloc2(pdsvValue, BaseDStructureValue, _pdsvContext->get_base(), 1024);
			pdsvValue->m_pVoidParam = NULL;
			pdsvValue->set_key(HASH_STATE(STATE_CONTEXT));
			return pdsvValue;
		}
		else {
			const char *strFilename;
			if (!_pdsvBase->get(HASH_STATE(BaseVariableFilename_strV), (const void **)&strFilename))
				return NULL;

			BaseDStructureValue *pdsvValue;
			PT_OAlloc2(pdsvValue, BaseDStructureValue, _pdsvContext->get_base(), 1024);
			pdsvValue->m_pVoidParam = NULL;
			pdsvValue->set_key(HASH_STATE(STATE_CONTEXT));

			STDEF_Manager(pManager);
			char strBuf[4096];
			strcpy_s(strBuf, 4096, pManager->path_get_save());
			strcat_s(strBuf, 4096, strFilename);
			pdsvValue->dump_load(this, strBuf);
			return pdsvValue;
		}
	}

	return NULL;
}

bool BaseStateManager::variable_check(BaseDStructureValue *_pdst, INT32 _nCol)
{
	if (_pdst->get_base()->get_index(_nCol) == -1)
	{
		BaseDStructureValue	dsvValue(EnumGet(HASH_STATE(EnumVariableDefine)), 0);
		dsvValue.set_key(_nCol);
		const char *strName;

		if (!dsvValue.get(HASH_STATE(STR_NAME), (const void**)&strName))
			return false;

		size_t len = strlen(strName);
		if (len > 5 && strncmp("strV", strName + len - 4, 4) == 0)
		{
			_pdst->get_base()->add_column(0, strName, TYPE_STRING);
		}
		else {
			_pdst->get_base()->add_column(0, strName, TYPE_INT32);
		}
	}
	return true;
}

bool BaseStateManager::EnumValueAdd(int _hash, const char *_value)
{
	BaseDStructure	*pdstEnum=NULL;

	STLMnpdst::iterator	it;
	it	= m_stlMnpdstEnum.find(_hash);
	
	if(it == m_stlMnpdstEnum.end())
		return false;
	
	pdstEnum	= it->second;
	if(pdstEnum->is_exist(_hash))
		return false;
	state_add(pdstEnum, _value);
	return true;
}

BaseDStructure*	BaseStateManager::EnumGet(int _nHash)
{
	BaseDStructure	*pdstEnum=NULL;

	STLMnpdst::iterator	it;
	it	= m_stlMnpdstEnum.find(_nHash);
	
	if(it != m_stlMnpdstEnum.end())
	{
		pdstEnum	= m_stlMnpdstEnum[_nHash];
	}else{
		return NULL;
	}

	return pdstEnum;
}

const void *BaseStateManager::EnumGetValue(int _nHash, int _nKey, int _nHashColum, short *_pnCnt)
{
	BaseDStructure	*pdstEnum=NULL;

	STLMnpdst::iterator	it;
	it	= m_stlMnpdstEnum.find(_nHash);

	if(it != m_stlMnpdstEnum.end())
	{
		pdstEnum	= it->second;

		BaseDStructureValue	dsvValue(pdstEnum, 0);
		dsvValue.set_key(_nKey);

		const void *pVoid;
		if(dsvValue.get(_nHashColum, (const void**)&pVoid, _pnCnt))
		{
			return pVoid;
		}
	}

	return NULL;
}

const void *BaseStateManager::EnumGetValue(int _nHash, int _nKey, int _nHashColum)
{
	BaseDStructure	*pdstEnum=NULL;

	STLMnpdst::iterator	it;
	it	= m_stlMnpdstEnum.find(_nHash);

	if(it != m_stlMnpdstEnum.end())
	{
		pdstEnum	= it->second;

		BaseDStructureValue	dsvValue(pdstEnum, 0);
		dsvValue.set_key(_nKey);

		const void *pVoid;
		if(dsvValue.get(_nHashColum, (const void**)&pVoid))
		{
			return pVoid;
		}
	}

	return NULL;
}

int	BaseStateManager::EnumGetIndex(const char *_strEnum, int _nKey)
{
	int nEnum	= UniqHashStr::get_string_hash_code(_strEnum);
	return EnumGetIndex(nEnum, _nKey);
}

int	BaseStateManager::EnumGetIndex(int _nHash, int _nKey)
{
	BaseDStructure	*pdstEnum=NULL;

	STLMnpdst::iterator	it;
	it	= m_stlMnpdstEnum.find(_nHash);

	if(it != m_stlMnpdstEnum.end())
	{
		pdstEnum	= it->second;

		BaseDStructureValue	dsvValue(pdstEnum, 0);
		dsvValue.set_key(_nKey);

		const int *pnIndex;
		if(dsvValue.get(HASH_STATE(HASH_Index_n), (const void**)&pnIndex))
		{
			return *pnIndex;
		}
	}

	return -1;
}

void BaseStateManagerLogic::post_event_state_unprocess(BaseDStructureValue *_pEvent, int _space_id)
{
	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	int nKeyStateUnprocess = HASH_STATE(HASH_BaseStateEventGlobalUnprocess);

	const int *pnEvent;
	if (_pEvent->get(nKeyState, (const void**)&pnEvent))
	{
		BaseDStructureValue *pdsvEvent = BaseStateManager::get_manager()->make_event(nKeyStateUnprocess, *pnEvent, 0);

		if (pdsvEvent == NULL)
			return;
		pdsvEvent->optionSet(OPT_EventLocal);
		push_event(pdsvEvent, _space_id);
	}
}

void BaseStateManager::post_event_int(const char *_strEvent, int _nEventValue, int _nSpaceID, int _nGroupId, int _nIdentify)
{
	int nKeyState = UniqHashStr::get_string_hash_code(_strEvent);
	BaseDStructureValue *pdsvEvent = make_event(nKeyState, _nEventValue, _nGroupId, _nIdentify);

	if (pdsvEvent == NULL)
		return;

	post_event(pdsvEvent, _nSpaceID);
}

BaseDStructureValue *BaseStateManager::make_event(int _nKey, const char *_strValue, int _nGroupId, int _nIdentify)
{
	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	BaseDStructureValue *pdsvEvent = make_event(nKeyState, STRTOHASH(_strValue), _nGroupId, _nIdentify);

	if (pdsvEvent == NULL)
		return NULL;

	pdsvEvent->set_alloc(_nKey, _strValue);
#ifdef _DEBUG
	pdsvEvent->deb_nValue = (INT64)_strValue;
#endif
	
	return pdsvEvent;
}

BaseDStructureValue *BaseStateManager::make_event(int _nKey, int _nValue, int _nGroupId, int _nIdentify)
{
	BaseDStructure *pdstEvent	= BaseStateManager::get_manager()->EnumGet(HASH_STATE(HASH_EnumEvent));

	if(pdstEvent == NULL)// StateManager destoried.
		return NULL;

	BaseDStructureValue *pdsvEvent;
	PT_OAlloc2(pdsvEvent, BaseDStructureValue, pdstEvent, 1024);

	pdsvEvent->set_key(_nKey);
	pdsvEvent->set_alloc(_nKey, &_nValue);
#ifdef _DEBUG
	pdsvEvent->deb_nValue = _nValue;
#endif
	int nBroadCast	= 0;

	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nBroadCast);
	const int *pnId;
	
    STLMnInt stlMnGroupId;
    if(!pdsvEvent->get(HASH_STATE(BaseTransitionGoalIdentifier), (const void**)&pnId) 
		|| _nIdentify != 0)
    {
		if (_nGroupId == 0)
			_nGroupId = HASH_STATE(None);
        stlMnGroupId[_nGroupId] = _nIdentify;
        BaseState::group_id_set(pdsvEvent, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
    }
    
	return pdsvEvent;
}

BaseDStructureValue *BaseStateManager::make_event_state(int _nEvent, int _nGroupId, int _nIdentify)
{
	if (BaseState::sm_stlVHash.size() == 0)
		return NULL;

	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	BaseDStructureValue *pdsvEvent = make_event(nKeyState, _nEvent, _nGroupId, _nIdentify);
	
	if (pdsvEvent == NULL)
		return NULL;

	return pdsvEvent;
}

BaseDStructureValue *BaseStateManager::make_event_state(const char *_strEvent, int _nGroupId, int _nIdentify)
{
	int nEvent = STRTOHASH(_strEvent);
	return make_event_state(nEvent, _nGroupId, _nIdentify);
}

void BaseStateManager::post_event_state(const char* _strEvent, const char* _strString)
{
	int nEvent = STRTOHASH(_strEvent);
	BaseDStructureValue* pdsvEvent;

	pdsvEvent = make_event_state(nEvent, 0, 0);
	pdsvEvent->set_alloc("TempString_strV", _strString);
	if (pdsvEvent)
		post_event(pdsvEvent, 0);
}

void BaseStateManager::post_event_state(const char *_strEvent, int _nSpaceID, int _nGroupId, int _nIdentify)
{
	int nEvent = STRTOHASH(_strEvent);
	return post_event_state(nEvent, _nSpaceID, _nGroupId, _nIdentify);
}

BaseStateManager *BaseStateManager::get_manager() {
	if (mpool_get().is_terminated())
		return NULL;

	if (ms_manager_p)
		return ms_manager_p;

	return NULL;
}

void BaseStateManager::post_event_state(int _nEvent, int _nSpaceID, int _nGroupId, int _nIdentify)
{
	BaseDStructureValue *pdsvEvent;

	pdsvEvent	= make_event_state(_nEvent, _nGroupId, _nIdentify);

	if(pdsvEvent)
		post_event(pdsvEvent, _nSpaceID);
}

void *BaseStateManager::process_event_state(int _nEvent, int _nReturnHash, int _nSpaceID, int _nGroupId, int _nIdentify)
{
	BaseDStructureValue *pdsvEvent;

	pdsvEvent = make_event_state(_nEvent, _nGroupId, _nIdentify);
	
	OnEvent(pdsvEvent);

	if(_nReturnHash == 0)
		return NULL;
	return ret_value_get(_nReturnHash);
}

void *BaseStateManagerLogic::process_event_byserial(int _nObj_serial, BaseDStructureValue *_pdsvEvent)
{
	BaseState *pState = GetState(_nObj_serial);

	if(pState)
	{
		pState->OnEvent(_pdsvEvent);
	}

	PT_OFree(_pdsvEvent);
	return NULL;
}

BaseState *BaseStateManagerLogic::GetState(int _nObjState)
{
	BaseState *pRet;

	//BaseStateSpace *pSpace	= NULL;
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
	{
		pRet = m_stlVpStateSpace[i]->GetState(_nObjState);
		if(pRet)
			return pRet;
	}
	return NULL;
}

void BaseStateManager::path_set_save(const char *_strPath)
{
	char strBuf[4096];
	strcpy_s(strBuf, 4096, _strPath);
	BaseSystem::path_fix(strBuf, 4096);
	int nLen = (int)strlen(_strPath);
	if (nLen >= 4095)
		nLen = 4094;
    if (*(strBuf + nLen - 1) != SLASH_C){
        *(strBuf + nLen) = SLASH_C;
        *(strBuf + nLen+1) = 0;
    }
	ms_strSavePath	= strBuf;
}

char *BaseStateManager::path_state_full_make(char *_strPath, int _nSize)
{
	BaseSystem::path_fix(_strPath, _nSize);
	const char *strPathCur = m_strRoot.c_str();
	int nLen = (int)strlen(strPathCur);

	if(strncmp(_strPath, "/Users", 6) == 0)
		return _strPath;

	if (strchr(_strPath, ':') != NULL)
		return _strPath;
	
	if (strncmp(_strPath, strPathCur, nLen) == 0) {
		return _strPath;
	}
	else {
		char strBuf[4096];
		strcpy_s(strBuf, 4096, strPathCur);
		if (*_strPath == '/'
			|| *_strPath == '\\') {
			strcat_s(strBuf, 4096, _strPath + 1);
		}
		else {
			strcat_s(strBuf, 4096, _strPath);
		}

		strcpy_s(_strPath, _nSize, strBuf);
		return _strPath;
	}
	return _strPath;
}

char *BaseStateManager::path_full_make(char *_strPath, int _nSize)
{
	BaseSystem::path_fix(_strPath, _nSize);
	const char *strPathCur = BaseStateManager::path_get_save();
	int nLen = (int)strlen(strPathCur);

    if(strncmp(_strPath, "/Users", 6) == 0
	   || strncmp(_strPath, "/Volumes", 8) == 0)
        return _strPath;

	if (strchr(_strPath, ':') != NULL)
		return _strPath;
    
	if (strncmp(_strPath, strPathCur, nLen) == 0) {
		return _strPath;
	}
	else {
		char strBuf[4096];
		strcpy_s(strBuf, 4096, strPathCur);
		if (*_strPath == '/'
			|| *_strPath == '\\') {
			strcat_s(strBuf, 4096, _strPath + 1);
		}
		else {
			strcat_s(strBuf, 4096, _strPath);
		}

		strcpy_s(_strPath, _nSize, strBuf);
		return _strPath;
	}
	return _strPath;
}

STLString BaseStateManager::path_get(const char* _path)
{
	STLString ret = path_get_save();
	if (*(_path + 1) == ':'
		|| *_path == '/'
		|| *_path == '\\') // full path
	{
		ret = _path;
	}else
		ret += _path;
	return ret;
}

const char *BaseStateManager::path_get_save()
{
	return ms_strSavePath.c_str();
}

void BaseStateManagerLogic::space_mask_set(STLVInt _stlVnSpaceIDMask)
{
	m_stlVnSpaceIDMask	= _stlVnSpaceIDMask;
}

BaseStateSpace *BaseStateManagerLogic::space_get(int _nSpaceID)
{
	BaseStateSpace *pSpace	= NULL;
	for(unsigned i=0; i<m_stlVpStateSpace.size(); i++)
	{
		pSpace	= m_stlVpStateSpace[i]->space_get(_nSpaceID);
		if(pSpace)
			return pSpace;
	}
	
	if(_nSpaceID == 0)// it is to create thread
	{
		PT_OAlloc3(pSpace, BaseStateSpace, (BaseStateManager*)this, NULL, _nSpaceID);
		pSpace->threadSpaceSet(true);
		m_stlVpStateSpace.push_back(pSpace);
	}

	return pSpace;
}

BaseStateSpace *BaseStateManagerLogic::space_parent_get(int _nSpaceID)
{
	int nSpaceIDParent;
	nSpaceIDParent	= space_id_get_parent_from_child_id(_nSpaceID);

	if(nSpaceIDParent == 0)
		return NULL;

	return space_get(nSpaceIDParent);
}

BaseStateSpace *BaseStateManagerLogic::space_create(int _nSpaceID)
{
	BaseStateSpace *pSpace, *pSpaceParent;
	PT_OAlloc3(pSpace, BaseStateSpace, (BaseStateManager*)this, NULL, _nSpaceID);

	pSpaceParent	= space_parent_get(_nSpaceID);

	if(pSpaceParent == NULL)
		pSpaceParent	= space_parent_create(_nSpaceID);
	
	if(pSpaceParent)
	{
		pSpaceParent->attach_child(pSpace);
	}else
	{
		m_stlVpStateSpace.push_back(pSpace);
		pSpace->inc_ref(this);
	}

	return pSpace;
}

int	BaseStateManagerLogic::space_mask_get(int _nLayer)
{
	int nRet	= 0;
	for(int i=0; i<=_nLayer; i++)
		nRet	|= m_stlVnSpaceIDMask[i];

	return nRet;
}

BaseStateSpace *BaseStateManagerLogic::space_parent_create(int _nSpaceID)
{
	int nLayer;
	BaseStateSpace *pSpace, *pSpaceParent;

	nLayer	= space_layer(_nSpaceID);

	if(nLayer < 1)
		return NULL;

	nLayer--;
	_nSpaceID	&= space_mask_get(nLayer);

	PT_OAlloc3(pSpace, BaseStateSpace, (BaseStateManager*)this, NULL, _nSpaceID);

	pSpaceParent	= space_parent_get(_nSpaceID);
	if(pSpaceParent == NULL)
		pSpaceParent	= space_parent_create(_nSpaceID);
	if(pSpaceParent){
		pSpaceParent->attach_child(pSpace);
	}else{
		m_stlVpStateSpace.push_back(pSpace);
		pSpace->inc_ref(this);
	}
	return pSpace;
}

int	BaseStateManagerLogic::space_layer(int _nSpaceID)
{
	int nTempID	= 0;//, nAccu = 0;
	for(unsigned i=0; i<m_stlVnSpaceIDMask.size(); i++)
	{
		nTempID	= m_stlVnSpaceIDMask[i] & _nSpaceID;
		if(nTempID	== 0)
			return i-1;
	}

	return -1;
}

int	BaseStateManagerLogic::space_id_get_parent_from_child_id(int _nSpaceID)
{
	int nIndex	= 0;
	
	while((m_stlVnSpaceIDMask[nIndex] & _nSpaceID) != 0
		&& m_stlVnSpaceIDMask.size() > (UINT32)nIndex+1)
		nIndex++;

	int nMask	= space_mask_get(nIndex-2);

	return (nMask & _nSpaceID);
}
//
//int BaseStateManager::serial_num_get()
//{
//	static int s_nStateSerial = 1;
//
//	return s_nStateSerial++;
//}
//
//void BaseStateManager::selector_regist(int _nSerial, int _nIdentify, int _nPriority)
//{
//	if(_nPriority == 0)
//		return;
//	m_stlMnSelectorList[_nSerial].push_back(std::pair<int, int>(_nIdentify, _nPriority));
//
//}
//
//bool BaseStateManager::selector_ready(BaseStateMain	*pMain, int _nSerial, int _nId)
//{
//	STLVpVoid stlVpStateActive;
//	if(pMain->GetState(&stlVpStateActive, 0, _nSerial))
//	{
//		for(unsigned i=0; i<stlVpStateActive.size(); i++)
//		{
//			BaseState *pStateTarget = (BaseState*)stlVpStateActive[i];
//            int nId = 0;//pStateTarget->identifier_get();
//			if(nId != 0 && nId == _nId)
//				continue;
//			if(!selector_exist(_nSerial, nId))
//				return false;
//		}
//	}
//	return true;
//}
//
//bool BaseStateManager::selector_exist(int _nSerial, int _nIdentify)
//{
//	STLMnstlVPairList::iterator	it;
//	it = m_stlMnSelectorList.find(_nSerial);
//	if(it == m_stlMnSelectorList.end())
//		return false;
//
//	if(it->second.size() > 0)
//	{// rotate Selector cause it's a first call for deal
//		for(unsigned i=0; i<it->second.size(); i++)
//		{
//			if(it->second[i].first == _nIdentify)
//				return true;
//		}
//	}
//	return false;
//}
//
//bool BaseStateManager::selector_select_dice(int _nSerial, int _nIdentify)
//{
//	STLMnstlVPairList::iterator	it;
//	it = m_stlMnSelectorList.find(_nSerial);
//	if(it == m_stlMnSelectorList.end())
//		return false;
//
//	if(it->second.size() > 0)
//	{// rotate Selector cause it's a first call for deal
//		STLVPairIntInt stlVParam;
//		stlVParam.push_back(std::pair<int, int>(HASH_STATE(SelectorSerial_nV), _nSerial));
//		int nTotal = 0;
//		for(unsigned i=0; i<it->second.size(); i++)
//			nTotal	+= it->second[i].second;
//
//		bool bRet = false;
//		int nResult	= BaseSystem::rand() % nTotal;
//		for(unsigned i=0; i<it->second.size(); i++)
//		{
//			if(!bRet && nResult < it->second[i].second)
//			{// only one _nIdentify can get true
//				it->second[i].second = -1, bRet = true;
//				post_event_state(STRTOHASH("SelectorResultSuccess"), 0, it->second[i].first, &stlVParam);
//			}else{// others is false
//				post_event_state(STRTOHASH("SelectorResultFail"), 0, it->second[i].first, &stlVParam);
//				nResult -= it->second[i].second;
//			}
//		}
//	}
//	m_stlMnSelectorList.erase(it);
//
//	return true;
//}
//
//void BaseStateManager::temp_value_set(int _nHash, BaseObject *_pObj)
//{
//	BaseObject *pData;
//	pData = temp_value_get(_nHash);
//	if(pData)
//		PT_OFree(pData);
//
//	m_stlMnpBaseTemp[_nHash] = _pObj;
//}
//
//BaseObject *BaseStateManager::temp_value_get(int _nHash)
//{
//	STLMnpBaseObject::iterator	it;
//	it = m_stlMnpBaseTemp.find(_nHash);
//	if(it != m_stlMnpBaseTemp.end())
//		return it->second;
//	return NULL;
//}

void BaseStateManagerLogic::dualupdate_block_reg_colum(const char *_strName)
{
	if(_strName == NULL)
		m_strCurColumProcessing = "";
	else
		m_strCurColumProcessing = _strName;
}

void BaseStateManager::net_manager_add(void *_pManager) {
	m_stlVpNetManager.push_back(_pManager);
}

void BaseStateManager::net_manager_delete(void *_pManager) {
	for (unsigned int i = 0; i < m_stlVpNetManager.size(); i++)
	{
		if (m_stlVpNetManager[i] == _pManager) {
			m_stlVpNetManager.erase(m_stlVpNetManager.begin() + i);
			return;
		}
	}
}

void BaseStateManager::net_manager_weakup() {
	for (unsigned int i = 0; i < m_stlVpNetManager.size(); i++)
	{
		((BaseNetManager*)m_stlVpNetManager[i])->reinit();
	}
}

STDEF_FUNC(BaseManagerStateAdd_varF)
{
	const int *hash_p;
	STDEF_GETLOCAL_R(_pdsvBase, BaseManagerStateAdd_varF, hash_p);

	STDEF_BaseState(state);
	BaseDStructureValue *variable_p = state->variable_get();
	const char *main_str;
	if(!variable_p->get(*hash_p, (void const **)&main_str))
		return 0;
	STDEF_Manager(manager);
	STLString state_name = main_str;
	state_name = state_name.substr(0, state_name.find_last_of('.'));
	if(!manager->main_add(state_name.c_str()))
		return 0;
	return 1;
}

STDEF_FUNC(BaseEnumStateAdd_strF)
{
	const char *enum_name;
	STDEF_GETLOCAL_R(_pdsvBase, BaseEnumStateAdd_strF, enum_name);

	STDEF_Manager(manager);
	const char *enum_state_str = NULL;
	int seq = 0;
	int hash = STRTOHASH(enum_name);
	do{
		enum_state_str = (const char*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, seq);
		if(enum_state_str)
			manager->EnumValueAdd(hash, enum_state_str);
		seq++;
	}while(enum_state_str);
	return 1;
}

STDEF_FUNC(BaseManagerStateCreate_varF)
{
	const int *hash_p;
	STDEF_GETLOCAL_R(_pdsvBase, BaseManagerStateCreate_varF, hash_p);

	STDEF_BaseState(state);
	BaseDStructureValue *variable_p = state->variable_get();
	const char *main_str;
	if(!variable_p->get(*hash_p, (void const **)&main_str))
		return 0;
	STDEF_Manager(manager);
	if(!manager->main_create(main_str))
		return 0;
	return 1;
}

STDEF_FUNC(BaseManagerStateMerge_varF)
{
	const int *hash_p;
	STDEF_GETLOCAL_R(_pdsvBase, BaseManagerStateMerge_varF, hash_p);

	const char* target_name = NULL, *path;
	target_name = (const char*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 0);
	path = (const char*)BaseState::VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, 1);
	STDEF_BaseState(state);
	BaseDStructureValue *variable_p = state->variable_get();
	const char *main_name;
	if(!variable_p->get(*hash_p, (void const **)&main_name))
		return 0;
	STLString fullpath = path;
	fullpath += target_name;
	fullpath = fullpath.substr(0, fullpath.find_last_of('.'));
	STDEF_Manager(manager);
	int hashMain = STRTOHASH(main_name);
	if(!manager->main_merge(hashMain, fullpath.c_str()))
		return 0;
	return 1;
}

STDEF_FUNC(BaseManagerStateRemove_varF)
{
	const int *hash_p;
	STDEF_GETLOCAL_R(_pdsvBase, BaseManagerStateRemove_varF, hash_p);

	STDEF_BaseState(state);
	BaseDStructureValue *variable_p = state->variable_get();
	const int *hashMain_p;
	if(!variable_p->get(*hash_p, (void const **)&hashMain_p))
		return 0;
	STDEF_Manager(manager);
	if(!manager->main_remove(*hashMain_p))
		return 0;
	return 1;
}

void BaseStateManagerLogic::thread_create_event(BaseDStructureValue *_pEvent, int _priority)
{
	 BaseSystem::createthread(update_, 0, _pEvent);
}

//*
DEF_ThreadCallBack(BaseStateManagerLogic::update)
{
	PT_ThreadStart(THTYPE_STATEMANAGER);
	BaseDStructureValue *evt = (BaseDStructureValue*)_pParam;

	BaseStateManagerLogic *logic = NULL;
	PT_OAlloc1(logic, BaseStateManagerLogic, NULL);
	logic->thread_start();
	BaseEventHandler::single_get(logic);
	
	const int *pnTargetMain = NULL;
	if(!evt->get(HASH_STATE(BaseTransitionGoalMain), (const void**)&pnTargetMain))
	{
		PT_OFree(evt);
        DEF_ThreadReturn;
	}
	
	BaseStateManager *manager =BaseStateManager::get_manager();
	
	const char *nameMain = (const char *)manager->EnumGetValue(STRTOHASH("EnumMains"), *pnTargetMain, HASH_STATE(STR_NAME));
	
	STLString strPath = manager->rootGet();
	strPath += nameMain;

	logic->space_get(0)->add_main(strPath.c_str());
	logic->OnEvent(evt);
	
	STLVpState states;
	do{
		//CORUM_PROFILE_SUB(UPDATE);
		logic->OnUpdate(BaseSystem::timeGetTime());

		if(!logic->StateActivesGet(&states))
		{
			break;
		}
		states.clear();
		BaseSystem::Sleep(1);
	}while(logic->thread_run_is() && !mpool_get().is_terminated());

	BaseEventHandler::release(logic);
	PT_OFree(logic);
	//CORUM_PROFILE_REPORT(NULL);

	PT_ThreadEnd(THTYPE_STATEMANAGER);

	BaseSystem::endthread();
	//_endthread();
	DEF_ThreadReturn;
}
//*/
