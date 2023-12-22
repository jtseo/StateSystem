#include "stdafx.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include "BaseResResource.h"
#include "BaseResManager.h"


unsigned		BaseResManager::ms_nThreadID	= 0;
BaseResManager *BaseResManager::pResManager_ = NULL;

bool			BaseResManager::ms_bThreadRun		= false;
unsigned		BaseResManager::ms_nReferenceCnt	= 0;
BaseCircleQueue	*BaseResManager::m_pqueueLoadRequest;
BaseCircleQueue	*BaseResManager::m_pqueueLoadFinish;

static bool s_bReleased = false;

BaseResManager* BaseResManager::managerReleaseGet(const char* _strPathRoot)
{
	return pResManager_;
}

BaseResManager	*BaseResManager::get_manager(const char *_strPathRoot)
{
	if(s_bReleased)
		return NULL;

	if(pResManager_ == NULL && _strPathRoot)
	{
		pResManager_	= new BaseResManager(_strPathRoot);
	}else if(pResManager_ == NULL)
	{
		char strRoot[255];
		BaseSystem::path_root_get(strRoot, 255, NULL);
		pResManager_	= new BaseResManager(strRoot);
	}

	return pResManager_;
}

void BaseResManager::reset_manager()
{
	Release();
	s_bReleased = false;
}

void BaseResManager::Release()
{
	if (pResManager_)
	{
		delete pResManager_;
		pResManager_ = NULL;
		s_bReleased = true;
	}
	ms_nThreadID = 0;
}

BaseResManager::BaseResManager(const char *_strPathRoot)
{
	ms_nReferenceCnt++;

	m_pqueueLoadRequest	= new BaseCircleQueue("BaseResManager:BaseResManager:Req", 20000);
	m_pqueueLoadFinish	= new BaseCircleQueue("BaseResManager:BaseResManager:Fin", 20000);
	
	if(_strPathRoot)
		strcpy_s(m_strPathRoot, 1024, _strPathRoot);

	m_bDestoried = false;

	m_pdstReservedValues	= new BaseDStructure;
	
	char strBuffer[255];
	strcpy_s(strBuffer, 255, _strPathRoot);
	//strcat_s(strBuffer, 255, "\\package\\data_s\\ResourceHeader"); only for columonline package
	if(!m_pdstReservedValues->load(strBuffer))
	{
		delete m_pdstReservedValues;
		m_pdstReservedValues	= NULL;
	}
}

void BaseResManager::SetDestory()
{
	m_bDestoried = true;
}

bool BaseResManager::GetDestoried()
{
	return m_bDestoried;
}

BaseResManager::~BaseResManager(void)
{
	ms_bThreadRun	= false;// Add or Update by OJ 2012-07-13 오후 6:01:30
	pResManager_ = NULL;

	BaseSystem::Sleep(100);

	ClearResource();

	ms_nReferenceCnt--;

	delete m_pqueueLoadFinish;
	delete m_pqueueLoadRequest;
}

void BaseResManager::LoadingResOrganize()
{
	size_t nSize	= ms_stlVpResourceLoadingStarted.size();

	for(size_t i=0; i<nSize; i++)
	{
		if(ms_stlVpResourceLoadingStarted[i]->GetState() == BaseResResource::STATE_FAIL ||
			ms_stlVpResourceLoadingStarted[i]->GetState() == BaseResResource::STATE_FINISH)
		{
			ms_stlVpResourceLoadingStarted.erase(ms_stlVpResourceLoadingStarted.begin()+i);
			nSize--;
			i--;
		}
	}
}

BaseResResource* BaseResManager::get_resource(unsigned _nType)
{
	STLMnpResource* pstlVpResource = GetResourceList(_nType);
	if (pstlVpResource && pstlVpResource->size() > 0)
		return pstlVpResource->begin()->second;
	return NULL;
}

bool BaseResManager::ClearResource(const char* _strName)
{
	STLMnpstlMnpResource::iterator it = m_stlMnpstlMnpResource.begin();
	STLMnpResource* pstlVpResource;
	BaseResResource* res;
	for (; it != m_stlMnpstlMnpResource.end(); it++)
	{
		pstlVpResource = it->second;
		res = Find(_strName, NULL, pstlVpResource);
		if (res)
		{
			ClearResource(res);
			return true;
		}
	}
	return false;
}

BaseResResource* BaseResManager::find_resource(const char* _strName, unsigned _nType, const char* _strGroupName)
{
	STLMnpResource* pstlVpResource = GetResourceList(_nType);
	if (pstlVpResource)
		return Find(_strName, _strGroupName, pstlVpResource);
	return NULL;
}

bool BaseResManager::add_resource(const char* _strName, unsigned _nType, BaseResResource* _res, const char* _strGroupName)
{
	BaseResResource* pResource = NULL;
	{// Prevent to create double resources whats have a same resource name use the critical section.
		STLMnpResource* pstlVpResource = GetResourceList(_nType);
		if (pstlVpResource)
		{
			pResource = Find(_strName, _strGroupName, pstlVpResource);
			if (pResource)
				return false;
		}
		_res->set_name(_strName);

		return Add(_strName, _nType, _res, _strGroupName);
		//LoadingResOrganize();
	}
	return false;
}

BaseResResource* BaseResManager::reload_resource(const char* _strName, unsigned _nType, _CallbackNotifierForFinish _pFunction, void* _pPram, float _fPriority, const char* _strGroupName)
{
	BaseResResource* res;

	res = find_resource(_strName, _nType);
	if (res)// the candidate dwg was update below algorithm, so it should reloaded again.
		ClearResource(res);
	res = get_resource(_strName, _nType, _pFunction, _pPram, _fPriority, _strGroupName);
	return res;
}

BaseResResource *BaseResManager::get_resource(const char *_strName, unsigned _nType, _CallbackNotifierForFinish _pFunction, void *_pPram, float _fPriority, const char *_strGroupName)
{
	if (!_strName)
		return NULL;

	BaseResResource *pResource	= NULL;
	{// Prevent to create double resources whats have a same resource name use the critical section.
		STLMnpResource *pstlVpResource = GetResourceList(_nType);
		if(pstlVpResource)
		{
			pResource	= Find(_strName, _strGroupName, pstlVpResource);
			if(pResource)
				return pResource;
		}
		pResource	= Add(_strName, _nType, _pFunction, _pPram, _fPriority, _strGroupName);
		LoadingResOrganize();
	}
	return pResource;
}

STLMnpResource *BaseResManager::GetResourceList(unsigned _nType)
{
	STLMnpstlMnpResource::iterator it = m_stlMnpstlMnpResource.find(_nType);

	if(it != m_stlMnpstlMnpResource.end())
		return it->second;

	return NULL;
}

BaseResResource *BaseResManager::Find(const char *_strName, const char *_strGroupName, STLMnpResource *_pstlVpResource)
{
	int nKey = get_key(_strName, _strGroupName);

	STLMnpResource::iterator it = _pstlVpResource->find(nKey);

	if(it == _pstlVpResource->end())
		return NULL;

	return it->second;
}

void BaseResManager::LoadProcess()
{
	BaseResResource *pRes;
	pRes = (BaseResResource*)m_pqueueLoadFinish->pop();

	if(pRes)
	{
		bool fail = false;
		if(pRes->GetRefCount() > 1)
		{
			if(pRes->Load2())
			{
				pRes->Finish(BaseResResource::STATE_FINISH);
			}else{
				fail = true;
				pRes->Finish(BaseResResource::STATE_FAIL);
			}
		}

		pRes->dec_ref(BaseResManager::get_manager());
		if (fail)
			BaseResManager::get_manager()->ClearResource(pRes);
	}
}

void BaseResManager::ThreadLoadPushFinish(BaseResResource *_pRes)
{
	m_pqueueLoadFinish->push(_pRes);// Add or Update by OJ 2012-07-13 오후 5:53:24
}

bool BaseResManager::RunningThread()
{
	return ms_bThreadRun;
}

DEF_ThreadCallBack(ThreadLoad)
//void *ThreadLoad(void *_pParam)
//DWORD WINAPI ThreadLoad( void *pParam )
{
	BaseResManager* pManager	= static_cast<BaseResManager*>(_pParam);

	BaseResResource *pRes;

	while(pManager->RunningThread())
	{
		pRes = BaseResManager::LoadingResPopFront(pManager);
		if(pRes) // BaseResManager가 Destory되고 있을 때는 더이상 Load하지 않는다.
		{
			if(pRes->GetRefCount() > 1 && !pManager->GetDestoried())
				pRes->Load();
			pManager->ThreadLoadPushFinish(pRes);// Add or Update by OJ 2012-07-13 오후 5:53:18
		}else{
            BaseSystem::Sleep(100);
		}
	}

	DEF_ThreadReturn;
}

int BaseResManager::get_key(const char *_strName, const char *_strGroupName)
{
	if(_strName == NULL)
		return 0;

	UINT32 nLen	= (UINT32)strlen(_strName);
	if(nLen == 0 || nLen >= 1024)
		return 0;

	char strBuffer[1024];
	strcpy_s(strBuffer, 1024, _strName);
	strcat_s(strBuffer, 1024, "//");
	if(_strGroupName)
		strcat_s(strBuffer, 1024, _strGroupName);
	_strlwr_s(strBuffer, 1024);
	return UniqHashStr::get_string_hash_code(strBuffer);
}

bool BaseResManager::is_resource(void *_pRes, int _nType)
{
	STLMnpResource *pstlVpResource = GetResourceList(_nType);
	if (pstlVpResource == NULL)
		return false;

	STLMnpResource::iterator it = pstlVpResource->begin();
	for (; it != pstlVpResource->end(); it++)
	{
		if (it->second == _pRes)
			return true;
	}
	return false;
}

bool	BaseResManager::Add(const char*_strName, unsigned _nType, BaseResResource* _res_p, const char* _strGroupName)
{
	if (_res_p == NULL)
		return NULL;

	_res_p->Init(this);

	_res_p->SetLoad(_strName, 0, _strGroupName);

	STLMnpResource* pstlMnpResource = GetResourceList(_nType);
	if (pstlMnpResource == NULL)
	{
		pstlMnpResource = new STLMnpResource;//(STLMnpResource*)NEW(STLMnpResource);
		m_stlMnpstlMnpResource[_nType] = pstlMnpResource;
	}

	//_res_p->Finish(BaseResResource::STATE_FINISH);

	(*pstlMnpResource)[_res_p->get_key_filename()] = _res_p;
	_res_p->inc_ref(this);

	return true;
}

BaseResResource *BaseResManager::Add(const char *_strName, unsigned _nType,_CallbackNotifierForFinish _pFunction, void *_pPram, float _fPriority, const char *_strGroupName)
{
	BaseResResource *pRes	= NULL;

	pRes	= static_cast<BaseResResource*>(BaseObject::CreateClass(_nType));

	if(pRes == NULL)
		return NULL;

	pRes->Init(this);
	if(!pRes->SetLoad(_strName, _fPriority, _strGroupName))
	{
		BaseObject::ReleaseClass(pRes);
		return NULL;
	}

	pRes->SetCallbackNotifier(_pFunction, _pPram);

	STLMnpResource *pstlMnpResource	= GetResourceList(_nType);
	if(pstlMnpResource == NULL)
	{
		pstlMnpResource	= new STLMnpResource;//(STLMnpResource*)NEW(STLMnpResource);
		m_stlMnpstlMnpResource[_nType] = pstlMnpResource;
	}

	if(_fPriority == 0) // 우선순위가 0일 경우 로딩이 될 때 까지 리턴하지 않는다.
	{
		if(pRes->Load())
		{
			pRes->Finish(BaseResResource::STATE_FINISH);

			STLMnpResource::iterator it;
			it = pstlMnpResource->find(pRes->get_key_filename());
			B_ASSERT(it == pstlMnpResource->end());

			(*pstlMnpResource)[pRes->get_key_filename()] = pRes;
			pRes->inc_ref(this);
		}else{
			BaseObject::ReleaseClass(pRes);
			pRes = NULL;
		}
	}else{
		STLMnpResource::iterator it;
		it = pstlMnpResource->find(pRes->get_key_filename());
		B_ASSERT(it == pstlMnpResource->end());

		(*pstlMnpResource)[pRes->get_key_filename()] = pRes;
		pRes->inc_ref(this);

		LoadingResPushBack(pRes, this);
	}
	
	return pRes;
}

void BaseResManager::ClearResource()
{
	LoadingResClear(this);

	STLMnpstlMnpResource::iterator itGroup = m_stlMnpstlMnpResource.begin();
	for(; itGroup!=m_stlMnpstlMnpResource.end(); itGroup++)
	{
		STLMnpResource::iterator itRes = itGroup->second->begin();
		for(; itRes != itGroup->second->end(); itRes++)
		{
#ifdef TEST_MODULE
			printf("dec ref %s, %d\n", 
				itRes->second->GetFileName(),
				itRes->second->GetRefCount());

			B_ASSERT(itRes->second->GetRefCount() == 1);
#endif
			itRes->second->dec_ref(this);
		}

		delete itGroup->second;
	}

	m_stlMnpstlMnpResource.clear();
}

void BaseResManager::SaveReservedValue()
{
	if(!m_pdstReservedValues)
		return;

	char strBuffer[255];
	strcpy_s(strBuffer, 255, m_strPathRoot);
	strcat_s(strBuffer, 255, "\\package\\data_s\\ResourceHeader");
	m_pdstReservedValues->save(strBuffer, 2);// save without to save .define, because in case of fail to save .define
									//, it bring out to permanently client crash error.
	delete m_pdstReservedValues;
	m_pdstReservedValues	= NULL;
}

void BaseResManager::ClearResourceSingleReference()
{
	STLMnpstlMnpResource::iterator itGroup = m_stlMnpstlMnpResource.begin();
	for(; itGroup!=m_stlMnpstlMnpResource.end();)
	{
		STLMnpResource::iterator itRes = itGroup->second->begin();
		for(; itRes != itGroup->second->end();)
		{
			if(itRes->second->GetRefCount() == 1)
			{
				itRes->second->dec_ref(this);
				itGroup->second->erase(itRes++);
			}else{
				itRes++;
			}
		}

		if(itGroup->second->size() == 0)
		{
			delete itGroup->second;
			m_stlMnpstlMnpResource.erase(itGroup++);
		}else{
			itGroup++;
		}
	}

}

void BaseResManager::LoadingResAddRes(BaseResResource *_pRes)
{
	m_pqueueLoadRequest->push(_pRes);// Add or Update by OJ 2012-07-13 오후 5:31:40
	ms_stlVpResourceLoadingStarted.push_back(_pRes);
}

void BaseResManager::LoadingResPushBack(BaseResResource *_pRes, BaseResManager *_pManager)
{
	_pRes->inc_ref(_pManager);
	_pManager->LoadingResAddRes(_pRes);
	
	_pRes->Finish(BaseResResource::STATE_WAIT);

	if(ms_nThreadID == 0)
	{
		ms_bThreadRun	= true;
        ms_nThreadID    = BaseSystem::createthread(&ThreadLoad_, 0, _pManager);
		//CreateThread(NULL, 0, &ThreadLoad, _pManager, 0, (LPDWORD)&ms_nThreadID);
	}
}

BaseResResource *BaseResManager::LoadingResPopFront(BaseResManager *_pManager)
{
	BaseResResource *pRes	= NULL;
	pRes	=(BaseResResource*)BaseResManager::m_pqueueLoadRequest->pop();
	if(pRes)
		pRes->Finish(BaseResResource::STATE_LOADING);

	return pRes;
}

void BaseResManager::LoadingResClear(BaseResManager *_pManager)
{
	BaseResResource	*pRes	= NULL;
	do{
		pRes	= (BaseResResource*)BaseResManager::m_pqueueLoadRequest->pop();
		if(pRes)
		{
			pRes->Finish(BaseResResource::STATE_FAIL);
			pRes->dec_ref(_pManager);
		}
	}while(pRes);
}

void BaseResManager::ClearResourceGroup(const char *_strGroupName, int _nType)
{
	std::string	strGroupName;

	STLMnpstlMnpResource::iterator itGroup = m_stlMnpstlMnpResource.find(_nType);
	if(itGroup == m_stlMnpstlMnpResource.end())
		return;

	STLMnpResource *pstlMnpResource = itGroup->second;
	STLMnpResource::iterator it = pstlMnpResource->begin();
	int nKey = get_key("/group/", _strGroupName);
	for(;it != pstlMnpResource->end();)
	{
		if(nKey == it->second->get_key_group())
		{
			it->second->dec_ref(this);
			pstlMnpResource->erase(it++);
		}else{
			it++;
		}
	}

	if(pstlMnpResource->size() == 0)
	{
		delete pstlMnpResource;
		m_stlMnpstlMnpResource.erase(itGroup);
	}
}

void BaseResManager::ClearResource(BaseResResource *_pRes)
{	
	STLMnpstlMnpResource::iterator itGroup = m_stlMnpstlMnpResource.find(_pRes->GetObjectId());

	if(itGroup != m_stlMnpstlMnpResource.end())
	{
		STLMnpResource::iterator itRes = itGroup->second->find(_pRes->get_key_filename());
		if(itRes != itGroup->second->end())
		{
			B_ASSERT(itRes->second == _pRes);
			itRes->second->dec_ref(this);
			itGroup->second->erase(itRes);
			if(itGroup->second->size() == 0)
			{
				delete itGroup->second;
				m_stlMnpstlMnpResource.erase(itGroup);
			}
		}
	}
}

int	BaseResManager::ReservedResGetValue(int _nKey, const char *_strColumName)
{
	if(!m_pdstReservedValues)
		return 0;
	const int *pnRet = NULL;
	int nIndex;

	nIndex	= m_pdstReservedValues->get_index(STRTOHASH(_strColumName));
	B_ASSERT(nIndex>=0);
	if(nIndex	< 0)
		return -1;

	m_pdstReservedValues->get(_nKey, nIndex, (const void **)&pnRet);
	return *pnRet;
}

void BaseResManager::ReservedResSetValue(int _nKey, const char *_strColumName, int _nValue)
{
	if(!m_pdstReservedValues)
		return;
	int nIndex;

	nIndex	= m_pdstReservedValues->get_index(STRTOHASH(_strColumName));

	if(!m_pdstReservedValues->is_exist(_nKey))
		m_pdstReservedValues->add_row_alloc(_nKey, 0, &_nKey);

	m_pdstReservedValues->set_alloc(_nKey, nIndex, &_nValue);
}

bool BaseResManager::ReservedResIsExist(int _nKey)
{
	if(!m_pdstReservedValues)
		return false;
	return m_pdstReservedValues->is_exist(_nKey);
}