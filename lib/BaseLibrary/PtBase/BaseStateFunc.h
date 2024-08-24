#pragma once

#include "BaseObject.h"

class BaseDStructureValue;
class BaseState;
class BaseStateManager;
class BaseResManager;
class BaseStateFunc;
class BaseStateFuncEx;

typedef BaseStateFuncEx *(*CreatorCallback)(const void *_param);
// BaseStateFunc is for function registration and call by State System
// 

class BaseStateFuncEx : public BaseObject
{
protected:
	enum {
		EnumCreate,
		EnumDelete,
		EnumEventCast,
		EnumgroupId_nIf,
		EnumOnStart,
		EnumFuncMax
	};

protected:
	static int StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func);
	static BaseStateFuncEx* FunctionProcessorHeader(int _func_hash, const char* _class_name, int _obj_id
		, const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context
		, STLVInt* _func_hash_a, CreatorCallback _creator_callback, int _group_level);
	//static int FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status);
	virtual int FunctionCall(const char* _class_name, STLVInt& _func_hash);
	BaseState* m_state_p;
	BaseStateManager* m_manager_p;
	BaseResManager* m_resmanager_p;
	int m_namekey_n;

public:
	BaseStateFuncEx();
	static int GetObjectId();
	virtual int BaseStateObjIdGet() = 0;
	
	BaseDStructureValue* EventMakeThread(int _hash);
	BaseDStructureValue* EventMake(int _hash, int _group_target = 0);
	bool EventPostThread(BaseDStructureValue* _event, int _target_serial = 0);
	bool EventPost(BaseDStructureValue* _event, int _target_serial = 0);
	bool EventPost(int _hash, int _target_serial = 0);
	bool EventEndPost(int _hash, int _target_serial = 0);
	bool GroupLevelSet(int _level);
	void GroupIdSet(int _id);
	void GroupIdGet(STLMnInt* _group);
	int GroupIdGet();

	inline const BaseDStructureValue* param_base_get() { return m_param_base; }
	inline BaseDStructureValue* param_event_get() { return m_param_event; }
	inline BaseDStructureValue* param_context_get() { return m_param_context; }
	inline void param_base_set(const BaseDStructureValue* _base) { m_param_base = _base; }
	inline void param_event_set(BaseDStructureValue* _evt) { m_param_event = _evt; }
	inline void param_context_set(BaseDStructureValue* _cxt) { m_param_context = _cxt; }
	void param_set(const BaseDStructureValue* _base, BaseDStructureValue* _evt, BaseDStructureValue* _cxt, BaseState* _state, BaseStateManager* _manager, int _func_hash);

	void variable_set(int _hash, const void* _value);
protected:
	const BaseDStructureValue* m_param_base;
	BaseDStructureValue* m_param_event;
	BaseDStructureValue* m_param_context;
	BaseDStructureValue* m_state_variable;
	const void* m_param_value;
	short m_param_cnt;
	int m_param_status;
	int m_func_hash;
	int m_group_hash;
	int m_group_id;

protected:
	//#SF_FuncHeaderStart
	virtual int Create();
	virtual int Delete();
	virtual int EventCast(STLString _class_name);
	virtual int groupId_nIf();
	virtual int OnStart();
	//#SF_FuncHeaderInsert

protected:
	int StateNameKeyGet();
	const void* paramFallowGet(int _seq, EDstType *_type = NULL);
	bool paramFallowTypeGet(int _seq, EDstType *_type);
	bool paramFallowSet(int _seq, const void* _value);
	void* paramFallowPointGet(int _seq);
	const void* paramVariableGet();
	void* paramVariablePointGet();
	bool paramVariableSet(const void* _value);
	STLVString paramValueAStrGet();
	STLString paramFormatGet();
	STLString paramVariableFormatGet();

	static STLVInt	s_func_hash_a;
};

class BaseStateFunc : public BaseStateFuncEx
{
public:
	BaseStateFunc();
	virtual ~BaseStateFunc();
	
	PtObjectHeaderInheritance(BaseStateFunc);
	static int GetObjectId();
protected:
	//#SF_FuncHeaderStart
	virtual int Create();
	virtual int Delete();
	//#SF_FuncHeaderInsert

	virtual int BaseStateObjIdGet() {
		return obj_serial_get();
	}
	virtual int BaseClassIdGet() {
		return sm_sample.GetObjectId();
	}
};

typedef std::vector<BaseStateFunc*, PT_allocator<BaseStateFunc*> > STLVpSFunc;

#define STDEF_SFREGIST(name)	 func_str = _class_name + "." #name;	(*_func_hash)[Enum_##name] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__)
#define STDEF_SFFUNCALL(name) else if (m_func_hash == s_func_hash_a[Enum_##name]) return name()
#define STDEF_SFENUM(name)	Enum_##name

#define STDEF_SC_HEADER(_classname_)	\
public:\
static int GetObjectId();\
static STLVInt	s_func_hash_a;\
static const char* s_class_name;\
static int FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status);\
static int StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func);\
virtual int FunctionCall(const char* _class_name, STLVInt& _func_hash);\
static BaseStateFuncEx* CreatorCallback(const void *_param)

#define STDEF_ASC_HEADER(_classname_)	\
public:\
static int GetObjectId();\
static STLVInt	s_func_hash_a;\
static const char* s_class_name;\
static int FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status);\
static int StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func);\
virtual int FunctionCall(const char* _class_name, STLVInt& _func_hash)

#define STDEF_ASC_HEADER2(_classname_)	\
public:\
static int GetObjectId();\
static STLVInt	s_func_hash_a;\
static const char* s_class_name;\
static int StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func);\
virtual int FunctionCall(const char* _class_name, STLVInt& _func_hash)

#define STDEF_SC_CPP(_classname_)	\
STLVInt	_classname_::s_func_hash_a;\
const char* _classname_::s_class_name = #_classname_;\
int _classname_::GetObjectId()\
{\
	static int	s_iId = 0;\
	if (s_iId == 0)\
		s_iId = STRTOHASH(s_class_name);\
\
	return s_iId;\
}

#define STDEF_ASC_CPP(_aclassname_, _classname_)	\
STLVInt	_aclassname_::s_func_hash_a;\
const char* _aclassname_::s_class_name = #_classname_;\
int _aclassname_::GetObjectId()\
{\
	static int	s_iId = 0;\
	if (s_iId == 0)\
		s_iId = STRTOHASH(s_class_name);\
\
	return s_iId;\
}
