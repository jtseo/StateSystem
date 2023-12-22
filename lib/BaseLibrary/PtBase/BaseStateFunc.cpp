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
#include "BaseStringTable.h"

#include "BaseStateFunc.h"


PtObjectCpp(BaseStateFunc);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseStateFuncEx::s_func_hash_a;

int BaseStateFuncEx::StateFuncRegist(STLString _class_name, STLVInt *_func_hash, int _size, fnEventProcessor _func)
{
	if ((*_func_hash).size() == 0)
	{
		(*_func_hash).resize(_size);
		
		STLString func_str;
		//#SF_FuncRegistStart
		func_str = _class_name + ".Create";	(*_func_hash)[EnumCreate] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		func_str = _class_name + ".Delete";	(*_func_hash)[EnumDelete] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		func_str = _class_name + ".EventCast";	(*_func_hash)[EnumEventCast] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		func_str = _class_name + ".groupId_nIf";	(*_func_hash)[EnumgroupId_nIf] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		func_str = _class_name + ".OnStart";	(*_func_hash)[EnumOnStart] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//#SF_FuncRegistInsert

		return EnumFuncMax;
	}
	else
		return 0;
}

BaseStateFuncEx::BaseStateFuncEx()
{
	m_state_p = NULL;
	m_manager_p = NULL;
	m_resmanager_p = NULL;

	m_nObjectId = GetObjectId();
	//StateFuncRegist("BaseStateFunc", &s_func_hash_a, EnumFuncMax, BaseStateFuncEx::FunctionProcessor);
}

BaseStateFunc::BaseStateFunc()
{
	m_state_p = NULL;
	m_manager_p = NULL;
	m_resmanager_p = NULL;
	m_nObjectId = GetObjectId();
	//StateFuncRegist("BaseStateFunc", &s_func_hash_a, EnumFuncMax, BaseStateFuncEx::FunctionProcessor);
}


void BaseStateFunc::init()
{
	BaseObject::init();
}

void BaseStateFunc::release()
{
}

int BaseStateFuncEx::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH("BaseStateFuncEx");

	return s_iId;
}

int BaseStateFunc::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH("BaseStateFunc");

	return s_iId;
}

BaseStateFunc::~BaseStateFunc()
{

}

BaseStateFuncEx* BaseStateFuncEx::FunctionProcessorHeader(int _func_hash, const char* _class_name, int _obj_id
	, const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context
	, STLVInt* _func_hash_a, CreatorCallback _creator_callback, int _group_level)
{
	BaseState* state = (BaseState*)_base->m_pVoidParam;
	BaseStateFuncEx* bs_func = (BaseStateFuncEx*)state->class_get(_obj_id);
	BaseStateManager* manager = state->get_space()->get_manager();

	if (bs_func &&_func_hash == _func_hash_a->at(EnumCreate))
	{
		//state->dec_ref(bs_func);
		//bs_func->dec_ref(state);
		bs_func = NULL;
	}

	if (bs_func == NULL) {
		const void* param = NULL;
		short cnt;
		if(_event)
			_event->get_param("ParamInt64", (const void**)&param, &cnt);
		//if (!param)
		//	return NULL;
		bs_func = _creator_callback(param);
		bs_func->param_set(_base, _event, _context, state, manager, _func_hash);
		state->class_regist(_obj_id, bs_func);
	}
	else
		bs_func->param_set(_base, _event, _context, state, manager, _func_hash);

	return bs_func;
}

int BaseStateFuncEx::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	//#SF_FuncCallStart
	if (m_func_hash == _func_hash[EnumCreate])		return Create();
	else if (m_func_hash == _func_hash[EnumDelete])		return Delete();
	else if (m_func_hash == _func_hash[EnumEventCast])		return EventCast(_class_name);
	else if (m_func_hash == _func_hash[EnumgroupId_nIf])		return groupId_nIf();
	else if (m_func_hash == _func_hash[EnumOnStart])		return OnStart();
	//#SF_FuncCallInsert

	return 0;
}

void BaseStateFuncEx::param_set(const BaseDStructureValue* _base, BaseDStructureValue* _evt, BaseDStructureValue* _cxt, BaseState* _state, BaseStateManager* _manager, int _func_hash)
{
	m_state_p = _state;
	m_namekey_n = m_state_p->get_key();
	m_manager_p = _manager;
	m_param_base = _base;
	m_param_event = _evt;
	m_param_context = _cxt;
	m_state_variable = m_state_p->variable_get();
	m_func_hash = _func_hash;

	m_param_value = NULL;
	m_param_base->get_param(_func_hash, (const void**)&m_param_value, &m_param_cnt);

	m_resmanager_p = (BaseResManager*)m_manager_p->GetResManager();

}

// it is replaced by FunctionProcessorHeader
//int BaseStateFuncEx::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
//{	
//	BaseState* state = (BaseState*)_base->m_pVoidParam;
//	BaseStateFunc* bs_func = (BaseStateFunc*)state->class_get(GetObjectId());
//	if (_base->function_hash_get() == s_func_hash_a[EnumCreate]) {
//		if (bs_func) {
//			state->dec_ref(bs_func);
//			bs_func->dec_ref(state);
//		}
//		PT_OAlloc(bs_func, BaseStateFunc);
//		bs_func->param_base_set(_base);
//		bs_func->param_event_set(_event);
//		bs_func->param_context_set(_context);
//
//		return bs_func->Create();
//	}
//	
//	if (!bs_func)
//		return 0;
//
//	bs_func->param_base_set(_base);
//	bs_func->param_event_set(_event);
//	bs_func->param_context_set(_context);
//
//	return bs_func->FunctionCall("BaseStateFunc", s_func_hash_a);
//}

int BaseStateFuncEx::Create()
{
	GroupLevelSet(*((const int*)m_param_value));
	//m_state_p->inc_ref(this);
	//inc_ref(m_state_p);

	return 1;
}

int BaseStateFuncEx::Delete()
{
	//m_state_p->dec_ref(this);
	//dec_ref(m_state_p);// delete this is applied in function
	return 1;
}

//#SF_FuncDefineStart
int BaseStateFunc::Create()
{	
	GroupLevelSet(*((const int*)m_param_value));
	//m_state_p->inc_ref(this);
	//inc_ref(m_state_p);

	return 1;
}

int BaseStateFunc::Delete()
{
	//m_state_p->dec_ref(this);
	//dec_ref(m_state_p);// delete this is applied in function
	return 1;
}

BaseDStructureValue* BaseStateFuncEx::EventMakeThread(int _hash)
{
	BaseDStructureValue* evt = m_manager_p->make_event_state(_hash);

	STLMnInt stlMnGroupId;
	
	stlMnGroupId[m_group_hash] = m_group_id;
	BaseState::group_id_set(evt, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
	int nSerial = m_state_p->obj_serial_get();
	evt->set_alloc(HASH_STATE(RevStateEventCaster), &nSerial);

	return evt;
}

BaseDStructureValue* BaseStateFuncEx::EventMake(int _hash, int _group_target)
{
	BaseDStructureValue* evt = m_state_p->EventSendGet(_hash);

	STLMnInt stlMnGroupId;
	
	if(_group_target)
		stlMnGroupId[m_group_hash] = _group_target;
	else
		stlMnGroupId[m_group_hash] = m_group_id;

	BaseState::group_id_set(evt, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);
	if (!_group_target)
	{
		int nSerial = m_state_p->obj_serial_get();
		evt->set_alloc(HASH_STATE(RevStateEventCaster), &nSerial);
	}

	return evt;
}

bool BaseStateFuncEx::EventPostThread(BaseDStructureValue* _event, int _target_serial)
{
	if(_target_serial)
		_event->set_alloc(HASH_STATE(BaseEventTargetStateSerial), &_target_serial);
		
	const int* priority_pn = NULL;
	//STDEF_GETLOCAL(m_param_base, BaseStateEventCastPriority_nV, priority_pn);
	if (priority_pn)
		m_manager_p->post_systemevent(_event, 0, *priority_pn);
	else
		m_manager_p->post_systemevent(_event);
	return true;
}

bool BaseStateFuncEx::EventPost(BaseDStructureValue* _event, int _target_serial)
{
	if(_target_serial)
		_event->set_alloc(HASH_STATE(BaseEventTargetStateSerial), &_target_serial);
		
	const int* priority_pn = NULL;
	//STDEF_GETLOCAL(m_param_base, BaseStateEventCastPriority_nV, priority_pn);
	if (priority_pn)
		m_manager_p->post_systemevent(_event, 0, *priority_pn);
	else
		m_manager_p->post_systemevent(_event);

	m_state_p->EventSendReset(); // remove sent event so next casting will be sent new event.
	return true;
}

bool BaseStateFuncEx::EventPost(int _hash, int _target_serial)
{
	return EventPost(EventMake(_hash), _target_serial);
}

bool BaseStateFuncEx::EventEndPost(int _hash, int _target_serial)
{
	return EventPost(EventMake(_hash), _target_serial);
}

int BaseStateFuncEx::EventCast(STLString _class_name)
{
	const int* event_pn = NULL;
	_class_name += ".EventCast";
	if (!m_param_base->get_param(STRTOHASH(_class_name.c_str()), (const void**)&event_pn))
		return 0;

	BaseDStructureValue* evt = EventMake(*event_pn);
	EventPost(evt);

	return 1;
}

void BaseStateFuncEx::GroupIdSet(int _id)
{
	m_group_id = _id;
	m_state_p->group_id_add(m_group_hash, m_group_id);
}

bool BaseStateFuncEx::GroupLevelSet(int _level)
{
	STLString group_str = "StClassLevel";
	char buf[10];
	sprintf_s(buf, 10, "%d", _level);
	group_str += buf;
	m_group_hash = STRTOHASH(group_str.c_str());
	m_group_id = BaseStateObjIdGet();

	m_state_p->group_id_add(m_group_hash, m_group_id);
	return true;
}

void BaseStateFuncEx::GroupIdGet(STLMnInt* _group)
{
	(*_group)[m_group_hash] = m_group_id;
}

int BaseStateFuncEx::GroupIdGet()
{
	return m_group_id;
}
//#SF_FuncDefineInsert

const void* BaseStateFuncEx::paramVariableGet()
{
	const int* hash = (const int*)m_param_value;
	const void* ret;
	if(m_state_variable->get(*hash, &ret))
		return ret;
	return NULL;
}

void* BaseStateFuncEx::paramVariablePointGet()
{
	const int* hash = (const int*)m_param_value;
	const INT64 *point;
	if(!m_state_variable->get(*hash, (const void**)&point))
		return NULL;
	void *ret = (void *)*point;
	return ret;
}

bool BaseStateFuncEx::paramVariableSet(const void *_value)
{
    m_state_variable->set_alloc(*(int*)m_param_value,_value);
    return true;
}

const void* BaseStateFuncEx::paramFallowGet(int _seq, EDstType *_type)
{
	return BaseState::VariableGet(m_param_base, m_param_context, m_param_event, _seq, NULL, 0, _type);
}


bool BaseStateFuncEx::paramFallowTypeGet(int _seq, EDstType *_type)
{
	return BaseState::VariableTypeGet(m_param_base, _seq, _type);
}

bool BaseStateFuncEx::paramFallowSet(int _seq, const void* _value)
{
	return BaseState::VariableSet(m_param_base, m_param_context, m_param_event, NULL, _value, _seq);
}

void BaseStateFuncEx::variable_set(int _hash, const void* _value)
{
	m_state_variable->set_alloc(_hash, _value);
}

void* BaseStateFuncEx::paramFallowPointGet(int _seq)
{
	INT64 parampoint = *(const INT64*)paramFallowGet(_seq);
	return (void*)parampoint;
}

STLString BaseStateFuncEx::paramVariableFormatGet()
{
	const char* str = (const char*)paramVariableGet();

	char strout[10240];
	strcpy_s(strout, 10240, str);
	char* strOut2 = BaseState::VariableStringMake(m_param_base, m_param_context, m_param_event, strout, 10240);

	STLString ret_str;
	if (!strOut2)
		return ret_str;

	ret_str = strOut2;
	return ret_str;
}

STLString BaseStateFuncEx::paramFormatGet()
{
	const char* str = (const char*)m_param_value;

	char strout[10240];
	strcpy_s(strout, 10240, str);
	char* strOut2 = BaseState::VariableStringMake(m_param_base, m_param_context, m_param_event, strout, 10240);

	STLString ret_str;
	if (!strOut2)
		return ret_str;

	ret_str = strOut2;
	return ret_str;
}

STLVString BaseStateFuncEx::paramValueAStrGet()
{
	BaseFile parse;
	parse.set_asc_seperator(",");
	parse.set_asc_deletor(" ");
	parse.OpenFile((void*)m_param_value, (UINT32)strlen((const char*)m_param_value));
	parse.read_asc_line();

	STLVString ret_astr;
	STLString str;
	str = parse.read_asc_str();
	while(!str.empty())
	{
		ret_astr.push_back(str);
		str = parse.read_asc_str();
	}
	return ret_astr;
}

int BaseStateFuncEx::StateNameKeyGet()
{
	return m_namekey_n;
}

int BaseStateFuncEx::groupId_nIf()
{
	const int* objId_p = (const int*)m_param_value;
	if (*objId_p != m_group_id)
		return 0;
	return 1;
}

int BaseStateFuncEx::OnStart()
{
	return 1;
}
