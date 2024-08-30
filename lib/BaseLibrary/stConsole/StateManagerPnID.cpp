#include "pch.h"
#include <deque>
#include <queue>
#include <string>
#include <cstring>
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
	if (ms_manager_p)
		PT_OFree(ms_manager_p);
	ms_manager_p = NULL;

	BaseResManager::Release();

	if (!_reset)
		BaseMemoryPoolMultiThread::terminate();
}
//
//void StateManagerFootball3DBoard::InitFileloader()
//{
//	BaseDStructure::set_callback_fileopen(fnCallbackFileOpen);
//	BaseDStructure::set_callback_fileclose(fnCallbackFileClose);
//}

#include "../PtBase/BaseSFuncDirectory.h"
#include "../PtExtend/BaseSCJson.h"
#include "../PtExtend/BaseStateSystem.h"
#include "../PtBase/BaseSCMysql.h"
//#include "../PtBase/BaseSCCurl.h"
//#include "../PtExtend/DevCashReader.h"

#if _WIN32
#include <winsock2.h>
#endif
#include "../PtBase/BaseSCTcp.h"

StateManagerPnID::StateManagerPnID(void* _applet) :BaseStateManager(_applet)
{
	BaseSFuncDirectory dir;
	BaseSCJson json;
	BaseStateSystem bsy;
	BaseSCMysql sql;
	BaseSCTcp tcp;
	//BaseSCCurl curl;
	//DevCashReader cash;

	//BaseSFParser paser;
}

void StateManagerPnID::set_root_path(const char* _strPath)
{
	char strPath[1024];
	strcpy_s(strPath, 1024, _strPath);
	BaseSystem::path_fix(strPath, 1024);
	m_strRoot = strPath;
}

#ifdef ANDROID
#include <android/log.h>
#endif
#include <thread>
BaseCircleQueue s_queue("Queue", 5000), s_queueCmd("QueueCmd", 5000);

void threadPush(int _thread)
{
	while (s_queueCmd.size_data() < 5);
	BaseCircleQueue::qsleep(1);

	INT64 s = (INT64)s_queueCmd.pop();

	for (int i = 0; i < 1000; i++)
	{
		s++;
		while (!s_queue.push((void*)s))
			BaseCircleQueue::qsleep(1);
	}
	g_SendMessage(LOG_MSG, "pushed %lld", s);
}

void androidlog(int _d)
{
	g_SendMessage(LOG_MSG, "log %d", _d);
}

void threadPop()
{
	INT64 p = 0;
	while (p == 0)
		p = (INT64)s_queue.pop();
	do {
		g_SendMessage(LOG_MSG, "check %lld", p);
		p = (INT64)s_queue.pop();
	} while (p != 0);
}

void threadTest()
{
	androidlog(100);
	std::vector<std::thread> threads;
	s_queueCmd.push((void*)10000);
	s_queueCmd.push((void*)20000);
	s_queueCmd.push((void*)30000);
	s_queueCmd.push((void*)40000);
	s_queueCmd.push((void*)50000);
	androidlog(101);

	for (int i = 0; i < 5; i++)
	{
		androidlog(130 + i);
		threads.push_back(std::thread(threadPush, i));
	}
	androidlog(140);
	threadPop();
	androidlog(141);

	for (int i = 0; i < 5; i++)
	{
		if (threads[i].joinable())
			threads[i].join();
	}
	threads.clear();
}


int StateManagerPnID::Init(void* _pApplet)
{
	res_manager_root_set(m_strRoot.c_str());

	if (BaseState::sm_stlVHash.size() < MAX_HASH_F3DBoard)
	{
		BaseState::sm_stlVHash.resize(MAX_HASH_F3DBoard);
		BaseState::sm_stlVHash[HASH_ModeChangeDungeon] = UniqHashStr::get_string_hash_code("ModeChangeDungeon");
	}

	BaseStateManager::load();

	//threadTest();
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


StateManagerPnID* StateManagerPnID::manager_create()
{
	if (ms_manager_p == NULL)
	{
		StateManagerPnID* cabin = NULL;
		PT_OAlloc1(cabin, StateManagerPnID, NULL);
		ms_manager_p = cabin;
	}
	else {
		ms_manager_p->release();
		ms_manager_p->init(NULL);
	}
	return (StateManagerPnID*)ms_manager_p;
}

BaseDStructureValue* StateManagerPnID::create_evnet(int _nEventKey, int _nTargetMain)
{
	BaseDStructureValue* pdsvEvent = make_event(_nEventKey, _nEventKey, 0);

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
