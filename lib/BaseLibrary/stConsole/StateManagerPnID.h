#pragma once
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseEventManager.h"
#include "../PtBase/BaseState.h"

class BaseCircleQueue;

class StateManagerPnID : public BaseStateManager
{
public:
	static StateManagerPnID  sm_sample;
	
	StateManagerPnID(void *_applet);
	~StateManagerPnID(void);

	virtual bool OnUpdate(unsigned _nTimeCurrent);

	virtual int Init(void *_pApplet);
	static void InitFileloader();
	
	void	set_root_path(const char *_strPath);
	static StateManagerPnID *get_manager();
	static void release_manager(bool _reset = false);
	static void reset_manager(); // reset to restart manager.

	BaseDStructureValue *create_evnet(int _nEventKey, int _nTargetMain = 0);
	//void process_event(BaseDStructureValue *_pdsvEvent);
	void post_event_model(int _nArmor, int _nUserIndex);
	static StateManagerPnID *manager_create();
public:

	enum {
		HASH_ModeChangeDungeon = BaseState::MAX_HASH,
		MAX_HASH_F3DBoard,
	};
};

#define HASH_STATE_F3DBoard(hash)	BaseState::sm_stlVHash[StateManagerPnID::hash]
