#pragma once

class BaseObject;
typedef BaseObject *(*CreateObjectFunc)();

typedef std::vector<BaseObject*, PT_allocator<BaseObject*> >	STLVpBaseObject;
typedef std::map<int, BaseObject*, std::less<int>, PT_allocator<std::pair<const int, BaseObject*> > >	STLMnpBaseObject;
typedef std::map<UINT32, CreateObjectFunc, std::less<UINT32>, PT_allocator<std::pair<const UINT32, CreateObjectFunc> > >	MAPObjectFactory;
typedef MAPObjectFactory::iterator MAPObjectFactoryIt;

#define REGISTEOBJECT(id,s)	BaseObject::RegistObject(id, &CreateObjectTemplate<s>)
#define CREATEOBJECT(id) BaseObject::CreateClass(id)

#define PtObjectHeader(_class_)	static _class_ sm_sample;\
	void init(){}\
	virtual void release(){}
#define PtObjectHeaderInheritance(_class_)	static _class_ sm_sample;\
	void init();\
	virtual void release()
#define PtObjectHeader2(_class_)	static _class_ sm_sample;\
	void init();\
	virtual void release();
#define PtObjectCpp(_class_)	_class_ _class_::sm_sample

//private:
//	static MAPObjectFactory		ms_mapObjectFactory;
//public:
//	static void RegistObject(UINT32 _id, CreateObjectFunc _pFunc);
//	static BaseObject* CreateClass(UINT32 _id);
//	static void ReleaseClass(BaseObject*_pClass);


class BaseObject// : public PTAllocT<BaseObject>
{
public:
	BaseObject(void);
	void init(int _nObjectSerial=0);
	virtual void release();
	static int serial_create_();
	//PtObjectHeader(BaseObject);

private:
	static MAPObjectFactory		ms_mapObjectFactory;
public:
	static void RegistObject(UINT32 _id, CreateObjectFunc _pFunc);
	static BaseObject* CreateClass(UINT32 _id);
	static void ReleaseClass(BaseObject*_pClass);

public:
	virtual ~BaseObject(void);

	int	GetRefCount()
	{
		return m_nRefCont;
	}
	virtual void inc_ref(BaseObject *xpRefer);
	virtual int dec_ref(BaseObject *xpRefer);

	const STLString &get_name() const{
		return m_strName;
	}

	int GetObjectId(){
		return m_nObjectId;
	}

	void SetObjectId(int _nId){
		m_nObjectId	= _nId;
	}
	void set_name(STLString _str){
		m_strName	= _str;
//#ifdef _DEBUG
        m_cstrName  = m_strName.c_str();
//#endif
	}
private:
	static int	ms_nSerialCounter;
protected:
	int			m_nSerialNumber;	// Unique number for each objects
	int			m_nObjectId;  // Unique number for class id
	STLString	m_strName;
	int			m_nRefCont;

public:
	int obj_serial_get() const { return m_nSerialNumber; }
#ifdef _DEBUG

public:
	virtual void Notifydisconnect(BaseObject *_pObject);
protected:
	STLVpBaseObject	m_stlVpRef;
//	void	*m_pCriticalSection;
#endif
    const   char    *m_cstrName;
	
    PT_OPTHeader;
};

template<typename classtype>
BaseObject *CreateObjectTemplate()
{
	BaseObject *pObject;
	
    PT_OAlloc(pObject, classtype);
    
	return pObject;
}

enum{
	OBJECTID_BASEOBJECT		= 0x00000001, // reserved identity number ranged( 0~ 0x0FFFFFFF)
	OBJECTID_RES_MANAGER,
	OBJECTID_RES_RESOURCE,
	OBJECTID_RES_CUSTOM		= 0x10000000,
	OBJECTID_SCN_OBJECT		= 0x20000000,
	OBJECTID_STATE_OBJECT	= 0x30000000,
	OBJECTID_DEVICE			= 0x40000000,
	OBJECTID_SERVER			= 0x50000000,
	OBJECTID_CLIENT			= 0x60000000,
	OBJECTID_NETWORK		= 0x70000000,
	OBJECTID_RENDER			= 0x80000000,
	OBJECTID_2D_INTERFACE	= 0x90000000
};

