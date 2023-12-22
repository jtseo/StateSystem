#pragma once
#include "BaseTree.h"

class BaseStateManager;
class BaseState;

class BaseStateMain;
typedef std::vector<BaseStateMain*>				STLVpStateMain;

//==================================================================================================
// Start Add or Update by OJ 2012-07-20 오후 6:07:04
// to decrease propagation time, use event hash table.
// to connect width target BaseState.
typedef struct {
	enum{
		NO_ACTOR,
		TIMEOUT,
		HASH,
		UPDATE
	};
	int					nType;
	int					nSpaceID;
	int					nKey;// Hash에서 사용됨(State Key)
	UINT32		nValue;// Timeout Event에서는 Timeout값, HASH에서는 Value를 사용해 빠른 검색 사용
	// 0일 경우 검색 하지 않고 모두 Test함
	int					nBoolIndex;// BaseDStructureValue의 BoolIndex를 말한다. 빨리 접근하기 위해 갖은 값
	int					nPriority; // BaseEventPriority_nV in link
	int					nLinkIndex; // use to find link from state system.
	BaseState			*pState; //pdstActor->m_pVoidParam이 BaseState를 가르킨다
	BaseDStructureValue	*pdstActor;
	int					nTempOrder;
}STStateActor;

class BaseStateSpace;
typedef std::vector<BaseStateSpace*, PT_allocator<BaseStateSpace*> >	STLVpStateSpace;
typedef std::vector<STStateActor, PT_allocator<STStateActor> >	STLVSTStateActor;
typedef std::map<int, STLVSTStateActor*, std::less<int>, PT_allocator<std::pair<const int, STLVSTStateActor*> > >	STLMnpstlVSTStateActor;
// End by OJ 2012-07-20 오후 6:07:08
//==================================================================================================

// Define Specific Space
// Client can't have more than one space.
// Server can have more than one space but they couldn't share event.
// because they are other space. only same space client can share event.
class BaseStateSpace : public BaseTree
{
public:
	BaseStateSpace(BaseStateManager *_pManager, const char *_strStateLogic, int _nSpaceID);
	virtual ~BaseStateSpace(void);

	static BaseStateSpace	sm_sample;
	
	void init(BaseStateManager *_pManager, const char *_strStateLogic, int _nSpaceID);
	void release();

	bool add_main(const char *_strLogin);
	bool add_near(BaseStateSpace *_pSpace);

	int OnEvent(BaseDStructureValue *_pEvent, const BaseState *_pStateTarget=NULL);
	int OnEvent_(BaseDStructureValue *_pEvent, const BaseState *_pStateTarget=NULL);
	virtual BaseStateMain *GetStateMain(int _nHash);
	BaseState *GetState(int _nObjSerialState);
	BaseState *GetState(const BaseState *_pState);
	BaseState* GetStateName(int _nKeyName, const STLMnInt* _pstlMnGroupId, int _nSerial);
	
	bool GetActiveState(STLVpState *_stlState);
	bool is_active_id(const STLMnInt *_pstlMnGroupId, int _nHash = 0);
	int space_translate(const BaseDStructureValue *_pdstEvent);// In Server: translate all of identified ActiveState to target Space. \
														  // In Client: change current state id.
	void state_active_show_all(void);

	virtual void post_event(BaseDStructureValue *_pdstEvent, int _nPriority=0);
	void post_systemevent(BaseDStructureValue *_pdstEvent, int _nPriority=0);
	void thread_create_event(BaseDStructureValue *_pdstEvent, int _nPriority=0);
	//==================================================================================================
	// Start Add or Update by OJ 2012-08-08 오후 5:18:23

	int space_id_get(){ return m_nSpaceID; }
	void space_id_set(int _nSpaceID) { m_nSpaceID = _nSpaceID; }
	void space_mask_set(int _nMask);
	int space_mask_get();
	BaseStateSpace *space_get(int _nSpaceID);
protected:
	int	m_nSpaceIDMask;

	// End by OJ 2012-08-08 오후 5:18:26
	//==================================================================================================
public:
	void copy_space(BaseStateSpace *_pSpace);

	virtual void	OnUpdate(unsigned _nTimeDelta);
	virtual int OnDraw();
	void reload();
	void reset_to_reload();
	void reload_start();
	int StateStart();
	BaseStateManager *get_manager()	{
		return m_pManager;
	}
	
	//==================================================================================================
	// Start Add or Update by OJ 2012-07-18 오전 10:13:14
	// connect to StateManager
public:
	BaseDStructure*	EnumGet(int _nHash);
	int				EnumGetIndex(int _nHash, int _nKey);
	int				EnumGetIndex(const char *_strEnum, int _nKey);
	void			event_request_update();
	void			event_affected();
	bool			key_is_pushed(int _nKey);
	void			post_event_ui(int _nEvent);
	bool			is_conformer();
	// End by OJ 2012-07-18 오전 10:13:18
	//==================================================================================================


	//==================================================================================================
	// Start Add or Update by OJ 2012-07-20 오후 6:10:21
public:
	int actor_event_process(STStateActor &_stStateActor, BaseDStructureValue *_pEvent, int _nEventType);
	bool actor_event_regist(STStateActor _stStateActor);
	bool actor_event_release(STStateActor _stStateActor);
	bool actor_event_release(BaseDStructureValue *_pdsvLink);
	bool actor_event_update(STStateActor _stStateActor);
protected:	
	int			m_nEventActorCount;
	STLVSTStateActor		*m_pstlvActorCurrent;
	STLMnpstlVSTStateActor	m_stlMnpstlVStateActor;
	STLVSTStateActor		m_stlVStateActorTimer;
	STLVSTStateActor		m_stlVStateActorUpdator;
	// End by OJ 2012-07-20 오후 6:10:22
	//==================================================================================================

	//==================================================================================================
	// Start Add or Update by OJ 2023-11-09
public:
	void threadSpaceSet(bool _b) { m_threadSpace = true; }
	bool threadSpaceGet() { return m_threadSpace; }
protected:
	bool m_threadSpace;
	// End by OJ
	//==================================================================================================

protected:
	STLVpStateSpace		m_stlVpSpaceNear;
	STLVpStateMain		m_stlVpStateMain;
	BaseStateManager	*m_pManager;

public:
	enum{
		SPACETYPE_NULL,
		SPACETYPE_DUNGEON,
		SPACETYPE_GUILD,
		SPACETYPE_PARTY
	};
protected:
	int					m_nSpaceID;// (m_nSpaceID / 1,000,000) = SpaceType, 
					// (m_nSpaceID % 1,000,000) / 100 = DungeonID, 
					// (m_nSpaceID % 1,000,000) % 100 = LayerID
};
