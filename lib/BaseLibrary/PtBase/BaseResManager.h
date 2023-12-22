#pragma once
#include "BaseObject.h"

class BaseCircleQueue;
class BaseResResource;
class BaseDStructure;

typedef std::vector<BaseResResource *, PT_allocator<BaseResResource*> >	STLVpResource;

typedef std::map<int, BaseResResource*, std::less<int>, PT_allocator<std::pair<const int, BaseResResource*> > >	STLMnpResource;
typedef std::map<int, STLMnpResource*, std::less<int>, PT_allocator<std::pair<const int, STLMnpResource*> > >	STLMnpstlMnpResource;
typedef std::list<BaseResResource *, PT_allocator<BaseResResource*> > STLLpResource;

//#define RES_SUPPORT_THREAD
/** \brief Resource Manager
*
* \par Project:
* Koongpa
*
* \par File:
* $Id: BaseResManager.h,v 1.1 2009/03/13 07:32:55 jtseo Exp $
*
* \ingroup ResourceManager
* 
* \version 1.0
*
* \par History:
* $Date: 2009/03/13 07:32:55 $\n
* $Author: jtseo $\n
* $Locker:  $
*
* \par Description:
*
* \par license:
* Copyright (c) 2006 Ndoors Inc. All Rights Reserved.
* 
* \todo 
*
* \bug 
*
*/
class BaseResManager :
	public BaseObject
{
public:
	BaseResManager(const char *_strPathRoot);
public:

	enum{
		RES_NULL,
		RES_UI		= 0x0001,
		RES_DUNGEON	= 0x0002,
		RES_WORLD	= 0x0004
	};

	virtual ~BaseResManager(void);

	/** \brief Load a resource
	*
	* \par Purpose:
	* Resource를 요청 했을 때 이전에 이미 로딩된 Resource라면 그 Resource를 리턴하고 
	* 그렇지 않으면 로딩되야할 리스트에 추가하고 리턴한다. 로딩이 완료되는 시점은
	* BaseResResource::GetState()를 통해서 할 수 있고 또 _pCallbackNotifierForFinish
	* 를 셋팅하여 알 수 있다. 이 때 주의 할 점은 이 함수는 multi thread에서 호출한다는
	* 것을 잊어서는 안된다.
	*
	* \param _strName: Name of resource(filename)
	* \param _nType: class type
	* \param _pFunction: As this function finish to load, call this callback function.
	* \param _pPram: As this function finish to load, call this callback function with this param.
	* \param _fPriority: Load Priority(if value 0, load reaource and then return this function.)
	* \author oj
	* \date2006-11-30
	*/
	BaseResResource	*get_resource(const char *_strName, unsigned _nType, _CallbackNotifierForFinish _pFunction = NULL
		, void *_pPram = NULL,float _fPriority = 1, const char *_strGroupName = NULL);
	BaseResResource* reload_resource(const char* _strName, unsigned _nType, _CallbackNotifierForFinish _pFunction = NULL
		, void* _pPram = NULL, float _fPriority = 1, const char* _strGroupName = NULL);
	bool add_resource(const char* _strName, unsigned _nType, BaseResResource *_res, const char* _strGroupName = NULL);
	BaseResResource* find_resource(const char* _strName, unsigned _nType, const char* _strGroupName = NULL);

	BaseResResource* get_resource(unsigned _nType);
	bool is_resource(void *_pRes, int _nType);
	const char* root_get() { return m_strPathRoot;  }

	void ClearResource();
	void ClearResourceSingleReference(); //> 리소스 매니저만 러퍼런스 하고 있는 경우 게임 내에서는 사용하고 있지 안는 리소스 이기 때문에 지운다
	void SetDestory();
	bool GetDestoried();
	bool ClearResource(const char* _strName);
	void ClearResource(BaseResResource *_pRes);
	void ClearResourceGroup(const char *_strGroupName, int _nType);

	static void LoadProcess();
	static void LoadingResPushBack(BaseResResource *_pRes, BaseResManager *_pManager);
	static BaseResResource *LoadingResPopFront(BaseResManager *_pManager);
	static void LoadingResClear(BaseResManager *_pManager);
	static BaseResManager	*get_manager(const char *_strPathRoot=NULL);
	static BaseResManager* managerReleaseGet(const char* _strPathRoot = NULL);
	static void Release();
	static void reset_manager();
	void SaveReservedValue();

	int get_key(const char *_strName, const char *_strGroupName);

	friend class BaseResManager;
protected:

	STLMnpResource *GetResourceList(unsigned _nType);
	BaseResResource *Find(const char *_strName, const char *_strGroupName, STLMnpResource *_pstlVpResource);
	BaseResResource *Add(const char *_strName, unsigned _nType, _CallbackNotifierForFinish _pFunction, void *_pPram, float _fPriority, const char *_strGroupName);
	bool	Add(const char*_strName, unsigned _nType, BaseResResource* _res_p, const char* _strGroupName);
protected:
	STLMnpstlMnpResource	m_stlMnpstlMnpResource;

	static unsigned				ms_nReferenceCnt;
	static STLLpResource		ms_stlLpResourceLoading;
	
	STLVpResource		ms_stlVpResourceLoadingStarted;

	static unsigned				ms_nThreadID;
	
	void LoadingResOrganize();
	void LoadingResAddRes(BaseResResource *_pRes);
	
	bool	m_bDestoried;

	char	m_strPathRoot[1024];
	static BaseResManager *pResManager_;

	//==================================================================================================
	// Start Add or Update by OJ 2012-06-07 오후 6:01:57
public:
	bool	ReservedResIsExist(int _nKey);
	int		ReservedResGetValue(int _nKey, const char *_strColumName);
	void	ReservedResSetValue(int _nKey, const char *_strColumName, int _nValue);
protected:

	BaseDStructure	*m_pdstReservedValues;
	// End by OJ 2012-06-07 오후 6:01:59
	//==================================================================================================

	//==================================================================================================
	// Start Add or Update by OJ 2012-07-13 오후 5:08:37
protected:
	static BaseCircleQueue	*m_pqueueLoadRequest;
	static BaseCircleQueue	*m_pqueueLoadFinish;
	static bool		ms_bThreadRun;
public:
	void ThreadLoadPushFinish(BaseResResource *_pRes);// Add or Update by OJ 2012-07-13 오후 5:53:27
	bool RunningThread();
	// End by OJ 2012-07-13 오후 5:08:38
	//==================================================================================================
};
