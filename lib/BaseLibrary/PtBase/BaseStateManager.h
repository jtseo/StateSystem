#ifndef Header_BaseStateManager
#define Header_BaseStateManager
#include "BaseObject.h"
#include "BaseStateMain.h"


/** \brief State Manager
 *
 * \par Project:
 * Portable 3D Engine Render
 *
 * \par File:
 * $Id: BaseStateManager.h,v 1.1 2009/03/13 07:32:55 jtseo Exp $
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
 * Create base states and manage to transit a state to another state.
 * Also message forwarding.
 *
 * \par license:
 * Copyright (c) 2008 OJ. All Rights Reserved.
 * 
 * \todo 
 *
 * \bug 
 *
 */

class EncoderOperator;	
class BaseStateMain;
class BaseStateSpace;
class BaseStateManager;
class BaseNetManager;
class BaseEventManager;

typedef std::vector<BaseStateManager*, PT_allocator<BaseStateManager*> >	STLVpStateManager;
typedef std::vector<BaseStateSpace*, PT_allocator<BaseStateSpace*> >	STLVpStateSpace;
typedef std::map<int, std::pair<BaseNetManager*, STLMnInt>, std::less<int>, PT_allocator<std::pair<const int, std::pair<BaseNetManager*, STLMnInt> > > >
			STLMKey_pSocketIDs;
typedef std::map<int, std::pair<int, int>, std::less<int>, PT_allocator<std::pair<const int, std::pair<int, int> > > >	STLMID_KeyIndex;

typedef int (*FnGetKeyValue)(const BaseDStructureValue *_pdsvEvent);
typedef std::map<int, FnGetKeyValue, std::less<int>, PT_allocator<std::pair<const int, FnGetKeyValue> > >	STLMnFnGetKeyValue;

typedef void (*FuncPostProcessor)(void *_pVoid);
typedef std::vector<std::pair<FuncPostProcessor, void*>, PT_allocator<std::pair<FuncPostProcessor, void*> > >
			STLVFuncpVoid;

typedef std::map<int, STLVPairIntInt, std::less<int>, PT_allocator<std::pair<const int, STLVPairIntInt> > >	STLMnstlVPairList;

typedef struct __stdef_socket__{
    int nServerIndex;
    int nSocketIndex;
    int nGroupId;
} STDEF_SOCKETID;
typedef std::map<int, STDEF_SOCKETID, std::less<int>, PT_allocator<std::pair<const int, STDEF_SOCKETID> > >    STLMnSocketId;

class BaseStateManagerLogic : // UI?? ??? ??????T?? ??????? ????? AI?? ?????? ???? StateManager(???T ?�????? ?????? ??? Cast???T?? ?????? ??�?)
	public BaseObject//, PTAllocT<BaseStateManagerLogic>
{
public:
	BaseStateManagerLogic(void *_applet);
	virtual ~BaseStateManagerLogic(void);
	static BaseStateManagerLogic	sm_sample;

	virtual	int init(void *_applet);
	virtual int StateStart();
	virtual int OnDraw();
	virtual void release();
	virtual BaseStateMain *GetStateMain(int _nHash, int _nSpaceID);
	BaseStateSpace *GetSpace(int _nSpaceID);
	virtual bool thread_start();
	bool StateActivesGet(STLVpState *_states);

	virtual void reload();

	//==========================================================================
	// Start Add by OJ : 2014-04-24
	// Post Processor : Some kind of Colum function has call post_event() and call OnUpdate(0)
	//   That kind of function make crash from StateSystem.
	//   So you should move that function to post processor to resolve crash problem.
public:
	void post_processor_regist(FuncPostProcessor _pFunc, void *_pVoid);
	void post_processor_run();
protected:
	STLVFuncpVoid	m_stlVFuncPostProcessors;
	// ================================================== End Add by OJ

	//==========================================================================
	// Start Add by OJ : 2014-04-24
	// Post Processor : Some kind of Colum function has call post_event() and call OnUpdate(0)
	//   That kind of function make crash from StateSystem.
	//   So you should move that function to post processor to resolve crash problem.
	
public:
	void render_after_push(FuncPostProcessor _pFunc, void *_pVoid);
	void render_after_run();
protected:
	STLVFuncpVoid	m_stlVFuncRenderAfter;
	// ================================================== End Add by OJ

	//==========================================================================
	// Start Update by OJ : 2010-10-06
	// Related on Keyboard
public:
	virtual int keyup_virtual(int /*_wParam*/, int /*_lParam*/) { return 0; };
	virtual int keydn_virtual(int /*_wParam*/, int /*_lParam*/) { return 0; };

protected:
	//================================= End Update by OJ : 2010-10-06

public:
	void *process_event_byserial(int _nObj_serial, BaseDStructureValue *_pdsvEvent);
	BaseState *GetState(int _nObj_serial);

	//==========================================================================
	// Start Update by OJ : 2010-10-07
public:
	void post_event_serial(BaseDStructureValue* _event_pdsv, int _serial);
	void push_event(BaseDStructureValue *_pdstEvent, int _priority_n);
	virtual void post_event(BaseDStructureValue *_pdstEvent, int _nSpaceID=0, int _nPriority=0);
	virtual void post_systemevent(BaseDStructureValue *_pdstEvent, int _nSpaceID=0, int _nPriority=0);
	virtual void post_event_ui(int /*_nEvent*/){}
	virtual bool OnUpdate(unsigned _nTimeCurrent);
	virtual void event_affected()
	{
		m_bEventStateAffected	= true;	
	}
	virtual void event_request_update()
	{
		m_bEventRequestUpdate	= true;
	}

	void event_postpond()
	{
		m_bEventPostpond = true;
	}

	virtual bool OnEvent(BaseDStructureValue *_pdstEvent);

protected:
	virtual bool OnEvent_(BaseDStructureValue *_pdstEvent);
	virtual bool OnUpdateInLock(unsigned _nTimeCurrent);
	bool				OnUpdateSpace(unsigned _nTimeCurrent);
	bool				m_bEventStateAffected; // Add by OJ : 2010-10-07 ???T?? ???? state_change, state_release, state_dummy ???? �?? ???
	bool				m_bEventRequestUpdate; // Add by OJ : 2010-11-29 ???T �???? ?????? OnUpdate(0)�???? ??????.
	bool				m_bEventPostpond; // skip all of event process to update system(especialy Screen update)
	BaseEventManager	*m_eventManager_p;
	//================================= End Update by OJ : 2010-10-07
protected:
	

	//==================================================================================================
	// Start Add or Update by OJ 2012-07-30 ???? 12:10:30
public:
	int		actor_regist_keyvalue_func(int _nHash, FnGetKeyValue _pFunc);
	bool	actor_get_keyvalue(const BaseDStructureValue *_pdstEvent, int *_pnValue);
protected:
	STLMnFnGetKeyValue	m_stlMnFnGetKeyValue;
	// End by OJ 2012-07-30 ???? 12:10:31
	//==================================================================================================

	//==================================================================================================
	// Start Add or Update by OJ 2012-08-10 ???? 4:25:02
	// Management Space ID
public:
	int		space_id_get_parent_from_child_id(int _nSpaceID);
	void	space_mask_set(STLVInt _stlVnSpaceIDMask);
	int		space_mask_get(int _nLayer);
	BaseStateSpace *space_get(int _nSpaceID);
	BaseStateSpace *space_parent_get(int _nSpaceID);
	BaseStateSpace *space_create(int _nSpaceID);
	BaseStateSpace *space_parent_create(int _nSpaceID);
	int		space_layer(int _nSpaceID);
protected:
	STLVInt	m_stlVnSpaceIDMask;
	// End by OJ 2012-08-10 ???? 4:25:02
	//==================================================================================================

protected:
	unsigned		m_nTimeCur;
	unsigned		m_nTimeEventSequence;
	bool			m_eventSkip_b;

	STLVpStateSpace	m_stlVpStateSpace;
public:
	void event_skip(bool _skip_b);
	bool event_skip_get();

	//===================================================================================
	// manage access count value by OJ 2013-10-24
public:
	bool AccessCountInc(const char* _strFunc);
	void AccessCountDec();
protected:
	STLString		m_strNameOfFunc;
	BaseCircleQueue	m_queueAccessLock;
	//INT32			m_nAccessCount;
	// End by OJ 2012-08-10 ???? 4:25:02
	//==================================================================================================

	//===================================================================================
	// Append return value by OJ 2013-10-24
public:
	void ret_value_set(int _nHash, void *_pRet)
	{
		m_stlMnpRetValue[_nHash] = _pRet;
	}

	void *ret_value_get(int _nHash)
	{
		STLMnpVoid::iterator	it;
		it = m_stlMnpRetValue.find(_nHash);
		if(it != m_stlMnpRetValue.end())
			return it->second;
		return NULL;
	}
protected:
	STLMnpVoid	m_stlMnpRetValue;
	//----------------------------------------------- End of OJ

	//===================================================================================
	// Append Dual update block system by OJ 2014-08-13
public:
	void dualupdate_block_reg_colum(const char *_strName);
protected:
	
	STLString	m_strCurColumProcessing;
	//----------------------------------------------- End of OJ
	//==========================================================================
	// Start Update by OJ : 2014-08-20
	// Debug fuctions
public:
	int	m_nDebugBlockCastMessage;
	int m_nManagerNumber;
	//================================= End Update by OJ : 2014-08-20
	virtual void post_event_state_unprocess(BaseDStructureValue *_pEvent, int _nSpaceID = 0);
	//==========================================================================
	// Start Update by OJ : 2019-07-11
	// Release Resource Regist(Registed res will be released after the _nMSecLimit microsecond time)
	void resource_release_regist(int _nMSecLimit, void *_pRes);
public:
	STLVInt	m_stlVReleaseMSecLimit;
	STLVpVoid	m_stlVpReleaseRes;
	//================================= End Update by OJ : 2019-07-11
	
public:
	void thread_create_event(BaseDStructureValue *_pEvent, int _priority);
	static DEF_ThreadCallBack(update);
	bool thread_run_is() { return m_thread_run; }
protected:
	bool m_thread_run;
};

typedef std::map<int, BaseStateManagerLogic*>	STLMnpStateLogic;

class BaseStateManager :
	public BaseStateManagerLogic//, public  PTAllocT<BaseStateManager>
{	
public:

	/** \brief Messages
	*/
	enum EN_MESSAGES {
			MSG_APP_START	= 0x8000,
			MSG_APP_STOP             ,
			MSG_APP_SUSPEND          ,
			MSG_APP_RESUME           ,
			MSG_APP_CONFIG           ,
			MSG_APP_HIDDEN_CONFIG    ,
			MSG_APP_BROWSE_URL       ,
			MSG_APP_BROWSE_FILE      ,
			MSG_APP_MESSAGE          ,
			MSG_ASYNC_ERROR          ,
			MSG_APP_TERMINATE        ,
			MSG_EXIT                 ,
			MSG_APP_RESTART          ,
			MSG_EXT_STOP             ,
			MSG_EXT_CREATE           ,
			MSG_EXT_RELEASE          ,
			MSG_APP_LAST_EVENT       ,
			MSG_KEY                  ,
			MSG_KEY_PRESS            ,
			MSG_KEY_RELEASE          ,
			MSG_KEY_HELD             ,
			MSG_CHAR                 ,
			MSG_UPDATECHAR           ,
			MSG_COMMAND				 ,
			MSG_CTL_TAB              ,
			MSG_CTL_SET_TITLE        ,
			MSG_CTL_SET_TEXT         ,
			MSG_CTL_ADD_ITEM         ,
			MSG_CTL_CHANGING         ,
			MSG_CTL_MENU_OPEN        ,
			MSG_CTL_SKMENU_PAGE_FULL ,
			MSG_CTL_SEL_CHANGED      ,
			MSG_CTL_TEXT_MODECHANGED ,
			MSG_DIALOG_INIT          ,
			MSG_DIALOG_START         ,
			MSG_DIALOG_END           ,
			MSG_COPYRIGHT_END        ,
			MSG_ALARM                ,
			MSG_NOTIFY               ,
			MSG_APP_NO_CLOSE         ,
			MSG_APP_NO_SLEEP         ,
			MSG_MOD_LIST_CHANGED     ,
			MSG_BUSY                 ,
			MSG_FLIP                 ,
			MSG_LOCKED               ,
			MSG_KEYGUARD             ,
			MSG_CB_CUT               ,
			MSG_CB_COPY              ,
			MSG_CB_PASTE             ,
			MSG_OEM_START            ,
			MSG_OEM_END
	};

	/** \brief Keyboard ID
	* 
	* \par Description
	* Becase some of brew phone has different keyboard message, in this class define
	* the unique IDs and match the hardware keyboard IDs. So even if a phone has different ID,
	* you don't need to update the engine or the source code.
	*
	*/
	enum EN_KEYBOARDIDS{
			KEY_0	= 0x7000,
			KEY_1,
			KEY_2,
			KEY_3,
			KEY_4,
			KEY_5,
			KEY_6,
			KEY_7,
			KEY_8,
			KEY_9,
			KEY_STAR,
			KEY_POUND,
			KEY_POWER,
			KEY_END,
			KEY_SEND,
			KEY_CLR,
			KEY_UP,
			KEY_DOWN,
			KEY_LEFT,
			KEY_RIGHT,
			KEY_SELECT,
			KEY_SOFT1,
			KEY_SOFT2,
			KEY_SOFT3,
			KEY_SOFT4,
			KEY_FUNCTION1,
			KEY_FUNCTION2,
			KEY_FUNCTION3,
			KEY_FUNCTION,
			KEY_MENU,
			KEY_INFO,
			KEY_SHIFT,
			KEY_MESSAGE,
			KEY_MUTE,
			KEY_STORE,
			KEY_RECALL,
			KEY_PUNC1,
			KEY_PUNC2,
			KEY_VOLUME_UP,
			KEY_VOLUME_DOWN,
			KEY_WEB_ACCESS,
			KEY_VOICE_MEMO,
			KEY_SPEAKER,
			KEY_TAP,    // TAP Event
			KEY_PTT,
			KEY_LSHIFT,
			KEY_RSHIFT,
			KEY_LCTRL,
			KEY_RCTRL,
			KEY_LALT,
			KEY_RALT,
			KEY_CAPLK,
			KEY_SCRLK,
			KEY_NUMLK,
			KEY_LNGHANGUL,
			KEY_LNGJUNJA,
			KEY_LNGFIN,
			KEY_LNGHANJA,
			KEY_PRSCRN,
			KEY_BREAK,
			KEY_TXINSERT,
			KEY_TXDELETE,
			KEY_TXHOME,
			KEY_TXEND,
			KEY_TXPGUP,
			KEY_TXPGDOWN,
			KEY_LAST
	};

	BaseStateManager(void *_pApplet);
	virtual ~BaseStateManager(void);
	static BaseStateManager	sm_sample;
	
	virtual	int init(void *_pApplet);
	void res_manager_root_set(const char *_strRootRes);
	int load();
	bool main_merge(int _hash, const char *_target);
	bool main_create(const char *_main_str);
	bool main_remove(int _hash);
	bool main_add(const char *_main_str);
	void post_event_int(const char *_strEvent, int _nEventValue, int _nSpaceID=0, int _nGroupId = 0, int _nIdentify=0);
//	void post_event_state(int _nEvent, int _nSpaceID, int _nIdentify, STLVPairIntInt *_pstlVParamList);
//	void post_event_state(int _nEvent, int _nSpaceID, int _nIdentify, STLVPairIntPoint *_pstlVParamList);
	void post_event_state(int _nEvent, int _nSpaceID=0, int _nGroupId = 0, int _nIdentify=0);
	void post_event_state(const char *_strEvent, int _nSpaceID=0, int _nGroupId = 0, int _nIdentify=0);
	void post_event_state(const char* _strEvent, const char* _strString);
	void *process_event_state(int _nEvent, int _nReturnHash=0, int _nSpaceID=0, int _nGroupId = 0, int _nIdentify=0);
	BaseDStructureValue *make_event(int _nKey, int _nValue, int _nGroupId = 0, int _nIdentify = 0);
	BaseDStructureValue *make_event_state(int _nEvent, int _nGroupId = 0, int _nIdentify=0);
	BaseDStructureValue *make_event_state(const char *_strEvent, int _nGroupId = 0, int _nIdentify=0);
	BaseDStructureValue *make_event(int _nKey, const char *_strValue, int _nGroupId = 0, int _nIdentify=0);
	//void post_event_add(const char *_strMain, const char *_strTargetState, const char *_strGoalState, int _nSpaceID=0, int _nIdentify=0);
	
	virtual void release();
	virtual void reload();
	
	void *GetRndManager(){return m_pRndManager;}
	void *GetResManager(){return m_pResManager;}

	static BaseStateManager *get_manager();
	
	void	*m_pApplet;
protected:
	virtual bool OnUpdate(unsigned _nTimeCurrent);
	
public:
	//==========================================================================
	// Start Update by OJ : 2010-10-06
	// Related on Keyboard
public:
	void process_event_key(int _nScanCode, bool _bPush, int _nScanCode2=0);
	bool key_is_pushed(int _nKey);

protected:
	BaseDStructure m_dstMain;
	static BaseStateManager *ms_manager_p;
	STLVInt			m_stlVPushedKey;
	STLMnInt			m_scan2keyhash_m;
	//================================= End Update by OJ : 2010-10-06
	//==========================================================================
	// Start Update by OJ : 2010-07-21
	// Enum
public:
	BaseDStructure*	EnumAdd(const char *_strFilename);
	BaseDStructure*	EnumGet(int _nHash);
	bool EnumValueAdd(int hash, const char *_value);
	const void *		EnumGetValue(int _nHash, int _nKey, int _nHashColum);
	const void *		EnumGetValue(int _nHash, int _nKey, int _nHashColum, short *_pnCnt);
	int				EnumGetIndex(int _nHash, int _nKey);
	int				EnumGetIndex(const char *_strEnum, int _nKey);
protected:
	STLMnpdst		m_stlMnpdstEnum;// Add by OJ : 2010-05-10
	BaseDStructure* m_enumStructureCustom_p;
	//================================= End Update by OJ : 2010-07-21

public:
	void space_start(int _nSpaceID);
	//==========================================================================
	// Start Update by OJ : 2010-07-21
public:
	//BaseStateManagerLogic	*LogicAdd(const char *_strFilename);
	//BaseStateManagerLogic	*LogicGet(int _nHash);
protected:
	//STLMnpStateLogic	m_stlMnpStateLogic;
	//================================= End Update by OJ : 2010-07-21

	//==========================================================================
	// Start Update by OJ : 2010-12-21
public:
	static void path_set_save(const char *_strPath);
	static const char *path_get_save();
	static STLString path_get(const char* _path);
	static char *path_full_make(char *_strPath, int _nSize);
	char *path_state_full_make(char *_strPath, int _nSize);
	const char* rootGet() { return m_strRoot.c_str(); }
protected:
	static std::string		ms_strSavePath;
	//================================= End Update by OJ : 2010-12-21
protected:

	void	*m_pRndManager;
	void	*m_pResManager;

	std::string		m_strRoot;

	//==================================================================================================
	// Start Add or Update by OJ 2012-07-17 ???? 2:54:34
public:
	void set_conformer(bool _bConformer)
	{
		m_bConformer	= _bConformer;
	}

	bool is_conformer(){return m_bConformer; }
protected:
	bool	m_bConformer;// means that server.
	// End by OJ 2012-07-17 ???? 2:54:35
	//==================================================================================================

	//==================================================================================================
	// Start Add or Update by OJ 2013-10-23
//public:
//	void temp_value_set(int _nHash, BaseObject *_pObj);
//	BaseObject *temp_value_get(int _nHash);
//protected:
//	STLMnpBaseObject	m_stlMnpBaseTemp;
	// End by OJ 
	//==================================================================================================

	//===================================================================================
	// Append Selector system by OJ 2014-05-29
public:
	//static int serial_num_get();
	//void selector_regist(int _nSerial, int _nIdentify, int _nPriority);
	//bool selector_select_dice(int _nSerial, int _nIdentify);
	//bool selector_exist(int _nSerial, int _nIdentify);
	//bool selector_ready(BaseStateMain *_pMain, int _nSerial, int _nId = 0);

protected:
	
	//STLMnstlVPairList	m_stlMnSelectorList;
	//----------------------------------------------- End of OJ

	//==========================================================================
	// Start Update by OJ : 2014-08-20
	// variable fuctions
public:
	BaseDStructureValue *varialbe_global_get();
	int variable_global(int _nHash);

	bool structure_define(const char* _structure_name, const STLVString &_var_stra);
	bool structure_add(int _key, const STLVString &_var_stra);
	bool	variable_define(int _nHash, int _nType, bool bForce = false);
	bool	variable_set(BaseDStructureValue *_pdsvVar, int _nHash, int _nType, const void *_pValue, short _nCnt = 0);
	BaseDStructureValue *variable_type(int _nType, const BaseDStructureValue *_pdsvBase, BaseDStructureValue *_pdsvContext, BaseDStructureValue *_pdsvEvent, bool _bReceiver = true);
	bool variable_check(BaseDStructureValue *_pdst, INT32 _nCol);

protected:
	BaseDStructureValue * m_pdsvVariableGlobal;
	//================================= End Update by OJ : 2014-08-20

	//===================================================================================
	// Append NetConnector Manager by OJ 2018-02-12
public:
	virtual void state_active_show_all(void);
	void net_manager_add(void *_pManager);
	void net_manager_delete(void *_pManager);
	void net_manager_weakup();
protected:
	STLVpVoid m_stlVpNetManager;
	//----------------------------------------------- End of OJ

public:
	EncoderOperator * encoder_get()
	{
		return m_pEncoder;
	}

protected:
	EncoderOperator * m_pEncoder;

	// ============================================================
	// Support Terminator
public:
	static void system_terminate()
	{
		ms_terminated = true;
	}
	static bool system_terminate_check() {
		return ms_terminated;
	}
protected:
	static bool ms_terminated;
	bool m_first = true;
	// ------------------------------------------------------------------ End of Terminator
};


#define RegistFunc(strMainState, classFunc) //\
//		{\
//		int nKey;\
//		std::string	strUI;\
//		strUI	= strMainState;\
//		nKey	= UniqHashStr::get_string_hash_code(strUI.c_str());\
//		BaseStateMain *pMain	= GetStateMain(nKey,0);\
//		\
//		if(pMain)\
//			{\
//			BaseDStructure *pdstState	= pMain->get_state();\
//			BaseDStructure *pdstLink	= pMain->get_link();\
//			\
//##classFunc::RegistState(pdstState);\
//##classFunc::RegistLink(pdstLink);\
//}\
//}

#endif 
