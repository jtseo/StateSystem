#ifndef Header_BaseState
#define Header_BaseState
#include "BaseObject.h"

/** \brief Abstraction class for States
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: BaseState.h,v 1.1 2009/03/13 07:32:55 jtseo Exp $
 *
 * \ingroup State
 * 
 * \version 1.0
 *
 * \par History:
 * $Date: 2009/03/13 07:32:55 $\n
 * $Author: jtseo $\n
 * $Locker:  $
 *
 * \par Description:
 * The base of States for a special purpose.
 *
 * \par license:
 * Copyright (c) 2008 OJ. All Rights Reserved.
 * 
 * \todo 
 *
 * \bug 
 *
 */
class BaseDStructureValue;
class BaseStateManager;
class BaseStateSpace;
class BaseStateMain;
class BaseState;

#define STDEF_GET_R(from, name, value)	if(!from->get(STRTOHASH(#name),(const void**)&value)) return 0
#define STDEF_VGET_R(from, name, value)	if(!from->get(#name,value)) return 0
#define STDEF_GETS_R(from, name, value, serial)	if(!from->get(STRTOHASH(#name),(const void**)&value, NULL, serial)) return 0
#define STDEF_GETS(from, name, value, serial)	from->get(STRTOHASH(#name),(const void**)&value, NULL, serial)
#define STDEF_GETLOCAL_R(from, name, value)	if(!from->get_param(STRTOHASH(#name),(const void**)&value)) return 0;
#define STDEF_FGETLOCAL_R(from, name, value)	if(!from->get_param(BaseState::sm_stlVHash[BaseState::name],(const void**)&value)) return 0;
#define STDEF_FGETLOCAL(from, name, value)	from->get_param(BaseState::sm_stlVHash[BaseState::name],(const void**)&value)
#define STDEF_VGETLOCAL_R(from, name, value)	if(!from->get_param(#name,value)) return 0;
#define STDEF_AGETLOCAL_R(from, name, value, cnt)	if(!from->get_param(STRTOHASH(#name),(const void**)&value, &cnt)) return 0;
#define STDEF_VAGETLOCAL_R(from, name, value, cnt)	if(!from->get_param(#name,value, &cnt)) return 0;
#define STDEF_AGET_R(from, name, value, cnt)	if(!from->get(STRTOHASH(#name),(const void**)&value, &cnt)) return 0
#define STDEF_AGETS_R(from, name, value, cnt, serial)	if(!from->get(STRTOHASH(#name),(const void**)&value, &cnt, serial)) return 0
#define STDEF_GETLOCAL(from, name, value)	from->get_param(STRTOHASH(#name),(const void**)&value)
#define STDEF_VGETLOCAL(from, name, value)	from->get_param(#name,value)
#define STDEF_AGETLOCAL(from, name, value, cnt)	from->get_param(STRTOHASH(#name),(const void**)&value, &cnt)
#define STDEF_AGET(from, name, value, cnt)	from->get(STRTOHASH(#name),(const void**)&value, &cnt)
#define STDEF_AGETS(from, name, value, cnt, serial)	from->get(STRTOHASH(#name),(const void**)&value, &cnt, serial)
#define STDEF_GET(from, name, value)	from->get(STRTOHASH(#name),(const void**)&value)
#define STDEF_FGET(from, name, value)	from->get(BaseState::sm_stlVHash[name],(const void**)&value)
#define STDEF_SET(from, name, value)	from->set_alloc(STRTOHASH(#name), value)
#define STDEF_ADD(from, name, value)	from->add_alloc(STRTOHASH(#name), value)
#define STDEF_PARAM(_seq_)		((BaseState*)_pdsvBase->m_pVoidParam)->VariableGet(_pdsvBase, _pdsvContext, _pdsvEvent, _seq_)
#define STDEF_LINK(name)	//_pdstLink->set_event_processor(#name, Func##name)
#define STDEF_STATE(name)	//_pdstState->set_event_processor(#name, Func##name)
#define STDEF_FUNCH(name)	//static int Func##name(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdstEvent, BaseDStructureValue *_pdsvContext, int _nState)
//#define STDEF_FUNC(name)	int STDEF_STNAMEOFCLASS::Func##name(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdstEvent, BaseDStructureValue *_pdsvContext, int _nState)
#define STDEF_FUNC(Name)	int Func##Name(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvEvent, BaseDStructureValue *_pdsvContext, int _nState);\
	class ClassReg##Name{\
	public:\
		ClassReg##Name(){	BaseDStructure::processor_list_add(#Name, Func##Name, __FILE__, __LINE__); }\
	};\
	ClassReg##Name	classReg##Name;\
	int Func##Name(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvEvent, BaseDStructureValue *_pdsvContext, int _nState)

#define STDEF_MANAGERNAME	BaseStateManager
#define STDEF_Manager(name)		STDEF_MANAGERNAME	*name	= (STDEF_MANAGERNAME*)((BaseState*)_pdsvBase->m_pVoidParam)->get_space()->get_manager()
#define STDEF_Space(name)		BaseStateSpace	*name		= (BaseStateSpace*)((BaseState*)_pdsvBase->m_pVoidParam)->get_space()
#define STDEF_BaseState(name)	BaseState	*name	= ((BaseState*)_pdsvBase->m_pVoidParam)

typedef std::vector<BaseDStructureValue*, PT_allocator<BaseDStructureValue*> >	STLVpdstValue;
typedef std::vector<BaseState*, PT_allocator<BaseState*> >	STLVpState;
typedef int (*FnContextProcessor)(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);


//==========================================================================
// Start Update by OJ : 2011-08-11
typedef int (*FnStateUpdate)(BaseDStructureValue *_pdsvContext, BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvEvent, void*_pVoidData, UINT32 _nTimeDelta);
typedef struct __STStateUpdate__
{
	BaseDStructureValue *pdsvBase;
	BaseDStructureValue *pdsvEvent;
	void				*pVoidData;
	FnStateUpdate		fnUpdate;
} STStateUpdate;
typedef std::vector<STStateUpdate, PT_allocator<STStateUpdate> >	STLVSTStateUpdate;
//================================= End Update by OJ : 2011-08-11

class BaseState :
	public BaseObject//, public PTAllocT<BaseState>
{

public:
	enum{
		STATE_COLUM_NKEY,
		STATE_COLUM_NAME,
		STATE_COLUM_LINKS
	};
	enum{
		RET_UNPROCESS	= 0,
		RET_PROCESS,
		RET_ERR,
	};
	static BaseState	sm_sample;

	/** \brief Creator
	 *
	 * \par Purpose:
	 *
	 * \param _pManager :(in) Reference for StateManager
	 * \return 
	 * \author oj
	 * \date2005-10-31
	 */
	BaseState(BaseStateSpace* _pManager, const BaseDStructure *_pdstState, int _nKey, const BaseDStructure *_pdstLink);
	virtual ~BaseState(void);

	void init(BaseStateSpace *_pManager, const BaseDStructure *_pdstState, int _nKey, const BaseDStructure *_pdstLink, const STLMnInt *_pstlMnGroupId = NULL, int _nObjectSerial=0);
	bool state_set_(const BaseDStructure* _pdstState, const BaseDStructure* _pdstLink, int _key);
	virtual void release();
	virtual int	get_key() const;
	virtual int	get_key_sub() const;
	virtual int get_serial() const;
	bool key_is_engaged(int _nKey) const;
	bool key_is_engaged_parent(int _nKey) const;

	//BaseStateManager	*get_manager();
	BaseStateSpace		*get_space();
	BaseDStructure		*get_dstEvent();
	int	get_key_main();

	int StateReturn(BaseDStructureValue *_pdstEvent);
	int StateCall(BaseDStructureValue *_pdstEvent);

	/** \brief set parent 
	*
	* \par Purpose:
	* When you process the call stack this function support set parent
	* pointer for this, and you can use to return proecess.
	*
	* \author oj
	* \date2005-10-31
	*/
	void set_parent(BaseState *_pParent);
	BaseState *get_parent();
	bool is_mine(const BaseState *_pSub) const;
	/** \brief Start handler
	 *
	 * \par Purpose:
	 * All of state must have a Start Handler.\n
	 * When a state be activated, This function is called by the StateManager.\n
	 * \return If return is 0, success otherwise error.\n
	 *
	 * \author oj
	 * \date2005-10-31
	 */
	virtual int OnStart(BaseDStructureValue *_pdstEvent);
	void regist_func_();

	/** \brief Message Handler
	 *
	 * \par Purpose:
	 * The message forward to the current state.
	 *
	 * \param _nMsg :(in) General message(Keypress, timer etc.)
	 * \param _nParam1 :(in) Extra information( Key_1, Key_2, time handle etc.)
	 * \param _nParam2 :(in) Extra information
	 * \return 
	 * 1: The message was processed.\n
	 * 0: The message didn't be processed.\n
	 * \author oj
	 * \date2005-10-31
	 */
	virtual int OnEvent(BaseDStructureValue *_pdstEvent);

	/** \brief Message Handler
	*
	* \par Purpose:
	* Idle function
	*
	* \date2005-10-31
	*/
	virtual int OnUpdate(unsigned _nTimeDelta);
	
	/** \brief End Handler
	 *
	 * \par Purpose:
	 * All of state must have a End Handler.
	 * When a state be deactivated, This function is called by the StateManager.
	 *
	 * \return If return is 0, success otherwise error.
	 * \author oj
	 * \date2005-10-31
	 */
	virtual int OnEnd(BaseDStructureValue *_pdstEvent);
	
	/** \brief Resume Handler
	 *
	 * \par Purpose:
	 * All of state must have a Resume Handler.\n
	 * When a state be returned from the Instance state, This function is called by the StateManager.\n
	 *
	 * \return If return is 0, success otherwise error.
	 * \author oj
	 * \date2005-10-31
	 */
	virtual int OnResume(BaseDStructureValue *_pdstEvent);

	/** \brief Resume Handler
	*
	* \par Purpose:
	* All of state must have a Resume Handler.\n
	* When a state call a sub state, This function will be called .\n
	*
	* \return If return is 0, success otherwise error.
	* \author oj
	* \date2005-10-31
	*/
	virtual int OnSuspend(BaseDStructureValue *_pdstEvent);

	/** \brief Draw Befor handler
	* 
	* \a Propuse:\n
	* If you want to change someting befor to draw. You can update in this fuction.\n
	* \return
	* If return is 0, success otherwise error.\n
	*/
	virtual int OnDrawBefor()
	{
		int	nRet=0;
		return nRet;
	}

	/** \brief Handler for After Draw
	 *
	 * \par Purpose:
	 * If you want to change someting after to draw. You can update in this fuction.\n
	 *
	 * \return If return is 0, success otherwise error.\n
	 * \author oj
	 * \date2005-10-31
	 */
	virtual int OnDrawAfter()
	{
		int	nRet=0;
		return nRet;
	}

	/** \brief Return state type
	 *
	 * \par Purpose:
	 *
	 * \return 
	 * \author oj
	 * \date2005-10-31
	 */
	int	GetTypeCall()
	{
		return m_nTypeCall;
	}

	/** \brief Set state type
	 *
	 * \par Purpose:
	 *
	 * \param _nType 
	 * \author oj
	 * \date2005-10-31
	 */
	void	SetTypeCall(int _nType)
	{
		m_nTypeCall	= _nType;
	}

	BaseDStructureValue *get_state_value();
	const BaseDStructureValue *get_state_value() const;

	unsigned get_time_local(){	return m_nTimeLocal; }
	void	add_time_delay(unsigned _nTime){ m_nTimeDelay += _nTime; }

	static void init_hash();
	enum{
		STATE_SYSTEM_SIGN,
		START_STATE,
		STATE_CHANGE,
		STATE_ADD,
		STATE_RELEASE,
		STATE_DUMMY,
		STATE_CALL,
		STATE_RETURN,
		STATE_CONTEXT,
		STATE_LINKCOPY,//
		SPACE_TRANSLATE,
		BaseTransitionGoalMain,
		BaseTransitionThread,
		BaseTransitionGoalpState,		
		HASH_BaseTransitionGoalSpaceID,
		BaseTransitionGoalIdentifier,
		BaseTransitionCasterKey,
		BaseIdentifierParent_n,
		BaseIdentifierChilds_an,
		BaseTransitionNextIdentifier,
		HASH_BaseTransitionGoal,
		BaseEventTargetStateSerial,
		TIMETOPROCESS,
		N_KEY,
        BaseFalse_nIf,
		HASH_BaseLinkType,
		N_EVENT_FLASH,
		HASH_FlashUIEvent_strRE,
		ARRAY_LINK,
		STR_NAME,
		HASH_Comment,
		B_KEEP_STATE,
		HASH_BaseTimeOut_nREU,
		HASH_BaseTimeOut_varUIf,
		HASH_BaseLinkDelay,
		HASH_BaseLinkOneTime_b,
		HASH_BaseExclusiveState,
		HASH_nScanCode,
		HASH_nScanCode2,
		HASH_BaseKeyPush_nRE,
		HASH_BaseKeyRelease_nRE,
//		N_KEY_EVENT_PUSH,				// Update by Lee Seung Hee : 2010-03-16// Comment by OJ : 2010-08-05 HASH_BaseKeyPush_nRE
		HASH_ID_CHARACTER,
		HASH_ID_AutoReturnState,
		AutoReturnAfterDoResume,
		HASH_BaseLinkPriority_n,
		BaseEventPriority_nV,
		BaseLinkIndex_nV,
		HASH_BaseExclusiveLink,
		HASH_Index_n,
		HASH_BaseLinkDelay_nF,
		HASH_BaseKeyPushed_nIf,//INT32,EnumEventKey
		HASH_BaseKeyPushed_nUIf,
		HASH_BaseKeyReleased_nIf,	// Update by Lee Seung Hee : 2010-10-07 : INT32, EnumEventKey
		HASH_EnumEvent,
		HASH_EnumEventKey,
		EnumStateEvent,
		EnumStructure,
		BaseStateEventGlobal,
		BaseStateEventReturn,
		HASH_BaseStateEventGlobalUnprocess,
		HASH_BaseStateGradeSet_n,
		HASH_BaseLinkInverse,
		//ParamInt,
		BaseValueTimeStart,
		BaseValueIdentify,
		//BaseNextTempValues,
		//SelectorSerial_nV,
		FlashUIEvent_strRE,
		EnumVariableDefine,
		BaseFuncAddInverse_nIf,
		BaseVariableReferType_nV,
		BaseVariableRefer_anV,
		BaseVariableToVariable_anV,
		BaseVariableToStructure_anV,
		BaseVariableToStructure_varV,
		BaseVariableToReferVar_varV,
		BaseVariableRefer_strV,
		BaseVariableString_strV,
		BaseVariableRefRemove_nV,
		BaseVariableRefer_an64V,
		BaseVariableBreak_strV,
		BaseVariableConst_nV,
		BaseVariableConst_fV,
		BaseVariableConst_strV,
		RevTimeCurrentYear,
		RevTimeCurrentMonth,
		RevTimeCurrentDay,
		RevTimeCurrentHour,
		RevTimeCurrentMinute,
		RevTimeCurrentSecond,
		RevRandomDigit6,
		RevStateSerial,
		RevGroupStart,
		RevGroupEnd,
		Context,
		Event,
		EventCast,
		EventCastSystem,
		State,
		StateUI,
		Global,
		File,
		BaseVariableFilename_strV,
		StateNext,
        BaseStateGroupId_nV,
        None,
		Socket,
		Server,
		RevStateEventCaster,
		BaseTransitionThreadCaster,
		RevNetStateEventCaster,
		BaseVariableDefine_nV,
		BaseDebugHideLog,
		MAX_HASH
	};

	static STLVInt		sm_stlVHash;

public:
	static bool hash_append(int _nIndex, int _nHash);
	//==========================================================================
	// Start Update by OJ : 2010-07-28
	// for State Functions
public:
	static	void	RegistState(BaseDStructure *_pdstState);
	static	void	RegistLink(BaseDStructure *_pdstLink);

	static int ContextCheckIs(int _nHash, BaseDStructureValue *_pdsvContext, const void *pVoidBase, const void *_pVoidContext, short _nCnt, short _nSize);
	static int ContextCheckUp(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextCheckDown(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextCheckEqUp(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextCheckEqDown(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextCheckEqual(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextCheckNotEqual(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize);
	static int ContextCheckEqualLeft(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextSet(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	//static int ContextSetPassword(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextSetHash(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextMulti(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextAdd(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextStack(int _nHash, BaseDStructureValue* _pdsvContext, const void* pVoidBase, const void* pVoidContext, short _nCnt, short _nSize);
	static int ContextSub(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextLoopCallback(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, FnContextProcessor fnProcessor);
	static int ContextLoopCallbackInt(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, FnContextProcessor fnProcessor);
	static int VariableLoopCallbackInt(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvV1, BaseDStructureValue *_pdsvV2, FnContextProcessor fnProcessor);
	static int VariableLoopCallbackStruct(bool bConst, int _nHashOp, int _nTypeOp, const int *_pnValue, short _nCnt, const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvV1, BaseDStructureValue *_pdsvV2, FnContextProcessor fnProcessor);
	static int ContextSave(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextLoad(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);

	static int ContextFileCopy(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFileSizeGet(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFileRename(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFileExist(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFileDelete(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFolderRename(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFolderDelete(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFolderCreate(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFolderCurChange(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextFolderCurGet(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);
	static int ContextStringUpdate(int _nHash, BaseDStructureValue *_pdsvContext, const void*pVoidBase, const void*pVoidContext, short _nCnt, short _nSize);

	//================================= End Update by OJ : 2010-07-28

public:
	int	grade_get();

	BaseDStructureValue *context_get();
	void context_set(BaseDStructureValue *_pdsvContext);
	BaseDStructureValue *context_create();
	BaseDStructureValue *context_reserve(const STLMnInt &_stlMnGroupId);
	static bool context_value_int(const BaseDStructureValue *_pdsvRef, const BaseDStructureValue *_pdsvValue, int _nSequence, const void **_ppnValue, int *_pnHashOut = NULL);
	static STLMnInt group_get(BaseState* _state_p, const BaseDStructureValue* _dsvBase, BaseDStructureValue* _dsvContext, BaseDStructureValue* _dsvEvent);
	
	const BaseState	*get_sub_state()const;

	void set_state_main(BaseStateMain *_pMain);
	BaseStateMain *get_state_main();
	int make_candidate_list(STLVpState *_stlVpState, STLMnInt *_stlMnHash);
protected:

	/** \brief State Type
	*
	* \par Purpose:
	* Keep of state(Called by Instance state or not)
	*
	* \par Description:
	* 0: General Purpose State\n
	* 1: Instance State\n
	*	- Usually most of state translation is completely translated to
	*		another state, but some of special(suspend state) state they keep
	*		the current state and just process the special state(instance state)
	*		and then go back to the keeped state.
	*/
	unsigned	m_nTimeLocal;
	unsigned	m_nTimeDelay;
	int			m_nTypeCall;

	int LinkProcess(int _nIndexLink, BaseDStructureValue *_pdsvRecEvent = NULL);
public:
	BaseDStructureValue *link_get(const BaseDStructureValue *_pLink);
	int LinkProcess_(BaseDStructureValue *pLink, BaseDStructureValue *_pdsvRecEvent = NULL, bool *_linkcomplete_b = NULL);
	void LoggerSend(int _key, BaseDStructureValue* _link, const STLMnInt &_groupID_m, int _event_serial = 0);
protected:
	int	get_target_state_key(BaseDStructureValue *_pLink);// Link
	void add_link_(BaseDStructureValue *_pdsvState);
	
	//==========================================================================
	// Start Update by OJ : 2010-06-24
	STLVpdstValue		m_stlVpLinkDelayed;// Add by OJ : 2010-07-19
	STLVInt				m_stlVnLinkTimeDelayed;// Add by OJ : 2010-07-19
	//================================= End Update by OJ : 2010-06-24

	STLVpdstValue		m_stlVpLink;
	STLVpState			m_stlVpStateCandidate; // key 
	BaseDStructureValue *m_pdsvState;
	BaseDStructureValue *m_pdsvContext;	//
	const BaseDStructure		*m_pdstLink;
	BaseState			*m_pParentState;	// If this pointer got a value, when process return, call the Resume() for m_pParentState.
	BaseState			*m_pSubState;	// If this pointer got a value, all of message have to processed by substate.
	BaseState			*m_pLeafState;
	//void				*m_pManager; //!< referance for the StateManager.
	BaseStateSpace		*m_pSpace;	// reference for the BaseStateSpace what parent of this BaseState
	BaseStateMain		*m_pStateMain;

public:
	void leafStateSet(BaseState *_state_p)
	{
		if(m_pParentState){
			m_pParentState->leafStateSet(_state_p);
			return;
		}else if(this == _state_p)
			m_pLeafState = NULL;
		else
			m_pLeafState = _state_p;
	}
	int DebugOutString(const BaseDStructureValue *_pdst,const BaseDStructureValue *_pdstLink, const char *_strState);
	void state_active_show_all(void);
public:
	static int send_msg_error(PtLogType _nFileter, const char *_strFormat, ...);


	//==========================================================================
	// Start Update by OJ : 2010-11-10
public:
	void identifier_set_next(STLMnInt _stlMGroupID)
	{	
		m_stlMNextGroupID = _stlMGroupID;
	}

	STLMnInt identifier_set_next_auto(int _nGroupID);
	STLMnInt identifier_reference_next();
	STLMnInt identifier_add_next(int _nGroupID, int _nIdentifier);
	static int identifier_get_auto();
	bool groupid_get_next(STLMnInt *_pstlMnGroupId, BaseDStructureValue *_pdsvEvent);
	
    STLMnInt identifier_get_next()
	{
		return m_stlMNextGroupID;
	}
protected:
	STLMnInt	m_stlMNextGroupID;
    //int     m_nNextGroupID;
	//int		m_nNextIdentifier;// Add by OJ : 2010-11-10 Add Type
	//================================= End Update by OJ : 2010-11-10
	//==========================================================================
	// Start Update by OJ : 2010-12-10
	// Support for state changing
public:
	bool is_skip()
	{
		return (m_bSkipNextLink || m_bSuspended);
	}
	void OnEndOfOneEvent()
	{
		m_bSkipNextLink	= false;
	}
private:
	bool	m_bSkipNextLink;// Add by OJ : 2010-05-12 to excute "Exclusive Link", so if it is true, the next links will not run. then it should be changed true.
	bool	m_bSuspended;// Add by OJ : 2010-12-10 Call, Release, Translate, Return
	//================================= End Update by OJ : 2010-12-10

	//==========================================================================
	// Start Update by OJ : 2011-08-11
public:
	void UpdateFuncRegist(BaseDStructureValue *_pdsvEvent, void *_pData, FnStateUpdate _fnUpdate);
	void UpdateFuncUpdate(UINT32 _nTimeDelta);
	void UpdateFuncRelease();
protected:
	STLVSTStateUpdate	m_stlVStStateUpdate;
	//================================= End Update by OJ : 2011-08-11

	//==============================================
	// Start Add by OJ : 2013-10-22 Temperial value record for State Update Function
public:
	//static void event_state_attach_param(BaseState *_pState, const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, STLVPairIntInt *_pstlVParamList);
	static bool VariableSet(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, BaseDStructureValue *_pdsvEvent, BaseDStructureValue *_pdsvDefault, const void *_pParam, int _seq_int = 0, short _nCnt = 0);
	static const void *VariableGet(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, BaseDStructureValue *_pdsvEvent, int _nSeq, BaseDStructureValue *_pdsvDefault = NULL, int _seq_inc = 0, EDstType *_type = NULL);
	static bool VariableTypeGet(const BaseDStructureValue *_pdsvBase, int _nSeq, EDstType *_type);
	static char *VariableStringMake(const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, BaseDStructureValue *_pdsvEvent, char *_strFormat, int _nLen);
	//void TempValuePush(int _nHash, void *_pVoid);
	//void *TempValueGet(int _nHash) const;
    //int TempValueIntGet(int _nHash) const;
	//STLMnpVoid	&TempValueGet();
	BaseDStructureValue *variable_next_reserve();
	BaseDStructureValue *variable_next_take();

	void variable_set(BaseDStructureValue *_pdsvVariable);
	BaseDStructureValue *variable_get();
	BaseDStructureValue *EventSendGet(int _nEvent = 0, bool _bNew = false);
	BaseDStructureValue *EventSystemGet();
	void EventSendReset();
protected:
	BaseDStructureValue	*m_pdsvVariableNextReserve;
	BaseDStructureValue	*m_pdsvVariable;
	BaseDStructureValue *m_pdsvEventSend;
	BaseDStructureValue *m_pdsvEventSystem;
	//STLMnpVoid	m_stlMnpTemps;
	// -------------------------------- End Add by OJ

	//==============================================
	// Start Add by OJ : 2014-05-29 Add TempValue for next state
public:
	//void temp_nextvalue_push(int _nHash, void *_pVoid);
protected:
	//STLMnpVoid	m_stlMnpNextTemps;
	// -------------------------------- End Add by OJ
    
    //==============================================
    // Start Add by OJ : 2017-08-08 Add for _link operations
public:
    static bool group_id_get(const BaseDStructureValue *_pdstValue, int _nHash, STLMnInt &_stlMnGroupID);
    static void group_id_set(BaseDStructureValue *_pdstValue, int _nHash, const STLMnInt &_stlMnGroupID);
    static bool group_id_equal(const STLMnInt &_stlMnGroupID1, const STLMnInt &_stlMnGroupID2);
	static bool group_id_include(const STLMnInt &_stlMnGroupID1, const STLMnInt &_stlMnGroupID2);
    static bool group_id_variable(const STLMnInt *_pstlMnGroup);
    
    bool group_id_is_in(const STLMnInt &_stlMnGroupId) const;
	int group_id_get(int _hash) const;
    const STLMnInt &group_id_get();
    void group_id_set(const STLMnInt &_stlMnGroupId);
    bool group_id_equal(const STLMnInt &_stlMnGroupId) const;
	bool group_id_include(const STLMnInt &_stlMnGroupIdSmall) const;
	static void group_id_add(STLMnInt *_group_id_stlMn, STLMnInt &_group_id_add_stlMn);
	void group_id_add(int _nGroupId, int _nIdentifier);
    void identifier_set(int _nIdentifier, int _nGroupID = 0);
    int  identifier_get(int _nGroupID) const;
    char *group_id_get_string(char *_strBuf, int _nLen) const;
protected:
    STLMnInt    m_stlMnGroupIdentifier;
    bool _check_exculsive(BaseStateMain *_pMain,BaseDStructureValue *_pEvent,BaseDStructureValue *_pLink, int _nGoalState, STLMnInt &_stlMnGroupID, const char *_strMsg);
    // -------------------------------- End add by OJ

	//==============================================
	// Start Add by OJ : 2020-04-21 Add for class state
public:
	bool class_regist(int _class_hash, const BaseObject * _class);
	bool class_allclear();
	bool class_clear(int _class_hash);
	BaseObject* class_get(int _class_hash);
	// -------------------------------- End add by OJ

	//=================================================
	// object serial, make difference
public:
	int obj_serial_get() const{
		return m_state_serial;
	}
	void obj_serial_set(int _serial)
	{
		m_state_serial = _serial;
	}
protected:
	int m_state_serial;
	//---------------------------------
};

#define HASH_STATE(hash)	BaseState::sm_stlVHash[BaseState::hash]
#define HASH_STATE2(hash)	BaseState::sm_stlVHash[hash]

#define VARIABLE_CONTEXT_GET(pnValue) \
		int nSeq = _pdsvBase->sequence_get()+1;\
		int nHashCheck = _pdsvBase->get_colum(nSeq);\
		if(nHashCheck == BaseState::sm_stlVHash[BaseState::BaseVariableRefer_anV])\
		{\
			const int *pnHash;\
			if(_pdsvBase->get((const void**)&pnHash, NULL, nSeq))\
			{\
				_pdsvContext->get(*pnHash, (const void**)&pnValue);\
			}\
		}

#endif
