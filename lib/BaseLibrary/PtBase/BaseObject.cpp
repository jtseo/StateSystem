#include "stdafx.h"
#include "BaseState.h"
#include "BaseStateMain.h"
#include "BaseStateSpace.h"
#include "BaseStateManager.h"

int	BaseObject::ms_nSerialCounter	= 0;

#ifdef _DEBUG
//#include "common/xsync.h"
#endif

//PtObjectCpp(BaseObject);

BaseObject::BaseObject(void)
{
	m_option	= 0;
	init();
}

int BaseObject::serial_create_()
{
	return ms_nSerialCounter++;
}

void BaseObject::init(int _nObjectSerial)
{
	if (_nObjectSerial == 0)
		m_nSerialNumber = serial_create_();	// Unique number for each objects
	else
		m_nSerialNumber = _nObjectSerial;
	m_nObjectId	= OBJECTID_BASEOBJECT;
	m_nRefCont	= 0;
#ifdef _DEBUG
	//	m_pCriticalSection	= NULL;
#endif
}

BaseObject::~BaseObject(void)
{
#ifdef _DEBUG
	//if(m_pCriticalSection != NULL)
	//{
	//	delete ((CriticalSection*)m_pCriticalSection);
	//	m_pCriticalSection	= NULL;
	//}
#endif
}

void BaseObject::release()
{
}

MAPObjectFactory BaseObject::ms_mapObjectFactory;

void BaseObject::RegistObject(UINT32 _id, CreateObjectFunc _pFunc)
{
	MAPObjectFactory::iterator it = ms_mapObjectFactory.find(_id);
	if (it != ms_mapObjectFactory.end())
		return;

	ms_mapObjectFactory[_id]	= _pFunc;
}

BaseObject* BaseObject::CreateClass(UINT32 _id)
{
	MAPObjectFactoryIt	it;
	it	= ms_mapObjectFactory.find(_id);

	if(it == ms_mapObjectFactory.end())
	{
		return NULL;
	}

	BaseObject *pRet = (it->second)();
	pRet->SetObjectId((int)_id);

	return pRet;
}

void BaseObject::ReleaseClass(BaseObject*_pClass)
{
	PT_OFree(_pClass);
}

#ifdef _DEBUG

void BaseObject::Notifydisconnect(BaseObject *_pObject)
{
	int x=0;
	x++;
	B_ASSERT(false);
}

void BaseObject::inc_ref(BaseObject *_pRefer)
{
	m_stlVpRef.push_back(_pRefer);
	m_nRefCont++;
}

int BaseObject::dec_ref(BaseObject *_pRefer)
{
	STLVpBaseObject::iterator it;

	it	= std::find(m_stlVpRef.begin(), m_stlVpRef.end(), _pRefer);

	if(it != m_stlVpRef.end())
	{
		m_stlVpRef.erase(it);
		m_nRefCont--;

		if(m_nRefCont == 0)
		{
			if(optionGet(OPT_CreatePrivate))
				PT_OFree(this);
			else
				delete this;
			return 0;
		}
	}else{
		int x=0;
		x++;
		B_ASSERT(false);
	}

	return m_nRefCont;
}

#else // in release mode

void BaseObject::inc_ref(BaseObject *_pRefer)
{
	m_nRefCont++;
}

int BaseObject::dec_ref(BaseObject *_pRefer)
{
	m_nRefCont--;
	if(m_nRefCont == 0)
	{
		if(optionGet(OPT_CreatePrivate))
			PT_OFree(this);
		else
			delete this;
		return 0;
	}
	
	return m_nRefCont;
}

#endif

STDEF_FUNC(ClassBaseObjRegist)
{
	STDEF_BaseState(state);

	int id = 0;
	if (!_pdsvEvent->get("TempInt_nV", (void*)&id))
		return 0;

	STLMnInt stlMGroup = state->group_id_get();

	static int hash_group = 0;
	if (hash_group == 0)
		hash_group = STRTOHASH("HashIDObject");

	STLMnInt::iterator it;
	it = stlMGroup.find(hash_group);
	if (it == stlMGroup.end())
		return 0;

	const void* class_point = 0;
	if (!_pdsvEvent->get(it->second, (const void**)&class_point))
		return 0;

	state->class_regist(it->second, (const BaseObject*)class_point);
	return 1;
}

PT_OPTCPP(BaseObject)
