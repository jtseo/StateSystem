#include "pch.h"
#include <deque>
#include <queue>
#include <string>
#include <vector>
#include <list>

//#include "../PtBase/Vector3.h"
//#include "../PtBase/Matrix3.h"
//#include "../PtBase/Matrix4.h"

#include "../PtBase/BaseDStructure.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseStateMain.h"
#include "StateManagerPnID.h"

#include "../PtBase/hashstr.h"
#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseStringTable.h"
#include "../PtBase/BaseCircleQueue.h"

#include "../PtBase/SPLine.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResResource.h"
#include "../PtBase/BaseResManager.h"

#include "../PtBase/BaseStringTable.h"

static bool s_bStateTerminated = false;
StateManagerPnID StateManagerPnID::sm_sample(NULL);

void StateManagerPnID::reset_manager()
{
    //BaseDStructure::static_clear();
	release_manager(true);
	s_bStateTerminated = false;
    
	BaseResManager::reset_manager();
    BaseConsolFilter::reset();
}

void StateManagerPnID::release_manager(bool _reset)
{
	if (ms_manager_p == NULL)
		return;

	s_bStateTerminated = true;
	if(ms_manager_p)
		PT_OFree(ms_manager_p);
	ms_manager_p = NULL;

	BaseResManager::Release();

	if(!_reset)
	BaseMemoryPoolMultiThread::terminate();
}
//
//void StateManagerFootball3DBoard::InitFileloader()
//{
//	BaseDStructure::set_callback_fileopen(fnCallbackFileOpen);
//	BaseDStructure::set_callback_fileclose(fnCallbackFileClose);
//}

#include "../PtBase/BaseSFuncDirectory.h"
#include "../PtBase/BaseTextEditor.h"
#include "../PtBase/BaseTextEditorText.h"
#include "../PtExtend/BaseSCJson.h"
//#include "../PtBase/BaseSFParser.h"

#ifdef _WIN32
#include "../PtExtend/DevCashReader.h"
#include "../PtExtend/DevPrinter.h"
#include "../Eos/DevCamera.h"
#endif

StateManagerPnID::StateManagerPnID(void *_applet) :BaseStateManager(_applet)
{
	BaseSFuncDirectory dir;
	BaseTextEditor te;
	BaseSCJson json;
	//BaseSFParser paser;
#ifdef _WIN32
	DevCashReader devRed;
	DevPrinter devPrt;
	DevCamera devCam;
#endif
}

void StateManagerPnID::set_root_path(const char *_strPath)
{
	char strPath[1024];
	strcpy_s(strPath, 1024, _strPath);
	BaseSystem::path_fix(strPath, 1024);
	m_strRoot = strPath;
}


int StateManagerPnID::Init(void *_pApplet)
{
	res_manager_root_set(m_strRoot.c_str());

	if (BaseState::sm_stlVHash.size() < MAX_HASH_F3DBoard)
	{
		BaseState::sm_stlVHash.resize(MAX_HASH_F3DBoard);
		BaseState::sm_stlVHash[HASH_ModeChangeDungeon] = UniqHashStr::get_string_hash_code("ModeChangeDungeon");
	}

	BaseStateManager::load();

	//StateStart(); // It's should called after finish all of set_event_processor, So it's moved to OnUpdate().
#ifdef _WIN32
	//REGISTEOBJECT(BaseResFileDWG::GetObjectId(), BaseResFileDWG);
	//REGISTEOBJECT(PnDrawingTag::GetObjectId(), PnDrawingTag);
#endif

	return 0;
}

StateManagerPnID::~StateManagerPnID(void)
{
}

bool StateManagerPnID::OnUpdate(unsigned _nTimeCurrent)
{
 	return BaseStateManager::OnUpdate(_nTimeCurrent);
}


StateManagerPnID *StateManagerPnID::manager_create()
{
	if(ms_manager_p == NULL)
	{
		StateManagerPnID *cabin = NULL;
		PT_OAlloc1(cabin, StateManagerPnID, NULL);
		ms_manager_p = cabin;
	}else{
		ms_manager_p->release();
		ms_manager_p->init(NULL);
	}
	return (StateManagerPnID*)ms_manager_p;
}

BaseDStructureValue *StateManagerPnID::create_evnet(int _nEventKey, int _nTargetMain)
{
	BaseDStructureValue *pdsvEvent = make_event(_nEventKey, _nEventKey, 0);
	
	int	nTargetMain;
	nTargetMain = _nTargetMain;
	
	pdsvEvent->set_alloc(HASH_STATE(BaseTransitionGoalMain), &nTargetMain);
	
	return pdsvEvent;
}
/*
void StateManagerFootball3DBoard::process_event(BaseDStructureValue *_pdsvEvent)
{
	BaseStateManager::post_event(_pdsvEvent);
	//OnEvent(_pdsvEvent);

	//PT_OFree(_pdsvEvent);
}
*/

STDEF_FUNC(TestclientFunc_nF)
{
	const int* param_pn;
	STDEF_GETLOCAL_R(_pdsvBase, TestclientFunc_nF, param_pn);
	STDEF_Manager(pManager);

	mpool_get().leak_old_display(*param_pn);
	return 1;
}
