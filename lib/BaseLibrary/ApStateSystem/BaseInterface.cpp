//
//  BaseInterface.cpp
//  BaseLibrary
//
//  Created by Jung Tae Seo on 10/15/15.
//  Copyright © 2015 Jung Tae Seo. All rights reserved.
//

#include "pch.h"

#include "../PtBase/BaseState.h"
#include "StateManagerPnID.h"
#include "../PtBase/BaseActionStack.h"
#include "BaseInterface.h"
#include "../PtBase/BaseStringTable.h"
#include "../PtBase/BaseFile.h"
//#include "ArchiveDatabase.h"
#include "../PtBase/BaseEventHandler.h"
#include "../PtBase/BaseResResource.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseStateSpace.h"
#include "../PtBase/BaseStateManager.h"

#ifdef ANDROID
#include <android/log.h>
#endif
#define APPNAME	"StateBaseLib"

#ifndef _MAX_PATH
#define _MAX_PATH   1024
#endif

static fnEventProcessor s_fnActionStackPush = NULL;
static fnEventProcessor s_fnDebugOutEvent = NULL;

static char s_strDebugOutput[4096];
void FuncDebugOut(int _nFilter, const char* _strOut, int _nLen) {
	//const BaseDStructureValue *_pdstBase, BaseDStructureValue *_pdstEvent, BaseDStructureValue *_pdstContext, int _nState

	int i = 0;
	for (; i < 4095; i++)
	{
		if (_strOut[i] == 0) {
			s_strDebugOutput[i] = 0;
			break;
		}

		if (_strOut[i] > 127 || _strOut[i] < 32)
			s_strDebugOutput[i] = '?';
		else
			s_strDebugOutput[i] = _strOut[i];
	}
	s_strDebugOutput[i] = 0;

#ifdef ANDROID
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s", s_strDebugOutput);
#else
	if (s_fnDebugOutEvent)
		s_fnDebugOutEvent((const BaseDStructureValue*)s_strDebugOutput, NULL, NULL, _nFilter);
#endif
}

void consol_load_hash(const char* _strFile)
{
	BaseConsolFilter::load_hash(_strFile);
}

void consol_hide(const char* _strTitle, const char* _strValue)
{
	BaseConsolFilter::hide(_strTitle, _strValue);
}

void consol_show(const char* _strTitle, const char* _strValue)
{
	BaseConsolFilter::show(_strTitle, _strValue);
}

void consol_set_debugout(fnEventProcessor _fnFunc)
{
	s_fnDebugOutEvent = _fnFunc;
	g_SendMessageSetDebugOut(&FuncDebugOut);
}

void manager_language_change(int _nCountryCode)
{
	BaseStringTable::set_language(_nCountryCode, NULL);
}

/*
#include <pthread.h>

static int cnt = 0;
void __cdecl *update_(void *_param)
{
	do{
		g_SendMessage(LOG_MSG, "--------t0");
		__sync_fetch_and_add(&cnt, 1);
		g_SendMessage(LOG_MSG, "--------t1 %d", cnt);
	}while(1);
	return NULL;
}
//*/
int s_language = 0;
void vscript_init(const char* _strAppName, const char* _strRootPath, const char* _strDataPath, const char* _strConsolIP, int _nLang)
{
	//pthread_t    nThread;

	//g_SendMessage(LOG_MSG, "--------1");
	//pthread_create(&nThread, NULL, update_, 0);
	//g_SendMessage(LOG_MSG, "--------2");

	//s_fnDebugOutEvent = NULL;
	//g_SendMessageSetDebugOut(NULL);
	mpool_get().terminate(false); // befor make false, shouldn't do anything, event the g_SendMessage().
	g_SendMessage(LOG_MSG, "=========---------------========= init");
	g_SendMessage_InitSocket(_strAppName, _strConsolIP);
	StateManagerPnID::reset_manager();
	s_language = _nLang;

	BaseStateManager::path_set_save(_strDataPath);
}

void* manager_create(const char* _strRootPath)
{
	char strPath[_MAX_PATH];
	BaseSystem::tomulti2(_strRootPath, strPath, _MAX_PATH);
	size_t len = strlen(strPath);
	len--;
	//strcpy_s(strPath, _MAX_PATH, _strRootPath);
	if (strPath[len] != '\\' && strPath[len] != '/')
		strcat_s(strPath, _MAX_PATH, "/");
	BaseSystem::path_root_set(strPath);

	StateManagerPnID* pManager = StateManagerPnID::manager_create();

	BaseEventHandler::single_get(pManager);

	BaseFile conf_file;
	STLString config_path = strPath;
	bool mobile = false;
#ifdef _IOS
	mobile = true;
#endif
	config_path += "statesystemconfig.ini";
	if (!mobile && !conf_file.OpenFile(config_path.c_str(), BaseFile::OPEN_READ))
	{
		conf_file.set_asc_seperator("\t");
		char buf[255];
		char value[255];
		conf_file.read_asc_line();
		conf_file.read_asc_string(buf, 255); // read variable name ; first is VScriptPath
		conf_file.read_asc_string(value, 255); // it's path for state scripts

		config_path = strPath;
		config_path += value;

		BaseStringTable::reload();
		BaseStringTable::set_language(s_language, config_path.c_str());
		pManager->set_root_path(config_path.c_str());
		pManager->Init(NULL);// Add by OJ : 2010-02-26

		g_SendMessage(LOG_MSG, "========start: %s, %s", buf, value);
		int hash = STRTOHASH(buf);
		pManager->variable_define(hash, TYPE_STRING, true);
		pManager->varialbe_global_get()->set_alloc(hash, value); // set VScriptPath

		while (conf_file.read_asc_line())
		{
			if (conf_file.read_asc_string(buf, 255))
			{
				conf_file.read_asc_string(value, 255);

				if (strcmp(buf, "BaseProductName_strV") == 0)
					g_ApplicationSet(value);

				g_SendMessage(LOG_MSG, "========variable: %s, %s", buf, value);
				int hash = STRTOHASH(buf);
				pManager->variable_define(hash, TYPE_STRING, true);
				pManager->varialbe_global_get()->set_alloc(hash, value);
			}
		}

		conf_file.CloseFile();
	}
	else {
		strcat_s(strPath, _MAX_PATH, "State/");

		BaseStringTable::reload();
		BaseStringTable::set_language(s_language, strPath);
		pManager->set_root_path(strPath);
		pManager->Init(NULL);// Add by OJ : 2010-02-26
	}
	//marker
	//pManager->thread_start();
	return pManager;
}

void manager_close(void* _pManager)
{
	s_fnDebugOutEvent = NULL;
	s_fnActionStackPush = NULL;
	g_SendMessageSetDebugOut(NULL);
	((StateManagerPnID*)_pManager)->release_manager();

	BaseDStructure::static_clear();

	PT_MemDisplay();
	PT_Mem_END();
}

void manager_keypush(void* _manager_p, int _scan1, bool _push, int _scan2)
{
	((BaseStateManager*)_manager_p)->process_event_key(_scan1, _push, _scan2);
}

void* manager_variable_global_get(void* _pManager)
{
	return (void*)((BaseStateManager*)_pManager)->varialbe_global_get();
}

void manager_variable_define(void* _manager, const char* _name, int _type)
{
	((BaseStateManager*)_manager)->variable_define(STRTOHASH(_name), _type, true);
}

void db_column_set(const char* _column, int _type, const char* _table)
{
	//ArchiveDatabase::column_set(_column, _type, _table);
}

void manager_structure_define(void* _manager_p, const char* _table, const char* _columns)
{
	STLVString column_a;
	int len = (int)strlen(_columns) + 1;
	char* buf = PT_Alloc(char, len);
	strcpy_s(buf, len, _columns);
	BaseFile::paser_list_seperate(buf, &column_a, ",");
	((BaseStateManager*)_manager_p)->structure_define(_table, column_a);
}

void thread_end()
{
	PT_ThreadEnd(0);
}

void db_columntable_set(int _table_hash, const char* _columns)
{
	STLVString column_a;
	int len = (int)strlen(_columns) + 1;
	char* buf = PT_Alloc(char, len);
	strcpy_s(buf, len, _columns);
	BaseFile::paser_list_seperate(buf, &column_a, ",");
	//ArchiveDatabase::columntable_set(_table_hash, column_a);
}

void manager_weakup(void* _pManager)
{
	if (!_pManager)
		return;

	g_SendMessage_InitSocket(NULL, NULL);
	((BaseStateManager*)_pManager)->net_manager_weakup();
}

void* manager_event_make(void* _pManager, const char* _strEvent)
{
	if (mpool_get().is_terminated())
		return NULL;

	return ((BaseStateManager*)_pManager)->make_event_state(_strEvent);
}

void manager_update(void* _pManager)
{
	if (mpool_get().is_terminated())
		return;

	int kkk = 0;
	static int cnt = 0;
	cnt++;

	if (kkk || cnt == 3)
	{
		//PT_MemDisplay();
		kkk = 0;
	}

	((StateManagerPnID*)_pManager)->OnUpdate(BaseSystem::timeGetTime());
}

void manager_event_post(void* _pManager, void* _pdstEvent, int _space, int _priority)
{
	if (mpool_get().is_terminated())
		return;

	((StateManagerPnID*)_pManager)->post_event((BaseDStructureValue*)_pdstEvent, _space, _priority);
}

void manager_message(int _nFilter, const char* _strMsg)
{
	g_SendMessage((PtLogType)_nFilter, _strMsg);
}

void manager_event_process(void* _pManager, void* _pdstEvent)
{
	if (mpool_get().is_terminated())
		return;

	//((StateManagerPnID*)_pManager)->OnUpdate(0);
	((StateManagerPnID*)_pManager)->post_event((BaseDStructureValue*)_pdstEvent);
	//((StateManagerPnID*)_pManager)->OnUpdate(0);
}

void manager_reg_func(const char* _strName, fnEventProcessor _fnFunc, const char* _file, int _line)
{
	if (strcmp(_strName, "DebugOutput") == 0) {
		s_fnDebugOutEvent = _fnFunc;
		g_SendMessageSetDebugOut(&FuncDebugOut);
		return;
	}
	else if (strcmp(_strName, "BaseUndoStackDLLPush") == 0)
	{
		s_fnActionStackPush = _fnFunc;
		return;
	}
	g_SendMessage(LOG_MSG, "process_add %s: %p", _strName, _fnFunc);
	if (!BaseDStructure::set_event_processor(_strName, _fnFunc, _file, _line))
		BaseDStructure::processor_list_add(_strName, _fnFunc, _file, _line);
}

int manager_hash(const char* _strHash)
{
	int nHash = STRTOHASH(_strHash);
	return nHash;
}

void dsv_event_cast_reset(void* _pdsv)
{
	if (mpool_get().is_terminated())
		return;

	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_pdsv);
	BaseState* pState = (BaseState*)pdsv->m_pVoidParam;

	if (pState == NULL)
		return;

	pState->EventSendReset();
}

void* dsv_event_cast_get(void* _pdsv, int _event, bool _new)
{
	if (mpool_get().is_terminated())
		return NULL;

	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_pdsv);
	BaseState* pState = (BaseState*)pdsv->m_pVoidParam;

	if (pState == NULL)
		return NULL;

	return pState->EventSendGet(_event, _new);
}

int dsv_state_function_hash(void* _pdsv)
{
	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_pdsv);

	return pdsv->function_hash_get();
}

void* dsv_state_variable(void* _pdsv)
{
	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_pdsv);
	BaseState* pState = (BaseState*)pdsv->m_pVoidParam;

	if (pState == NULL)
		return NULL;

	return pState->variable_get();
}

int dsv_state_timelocal_get(void* _pdsv)
{

	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_pdsv);
	BaseState* pState = (BaseState*)pdsv->m_pVoidParam;

	if (pState == NULL)
		return -1;

	return pState->get_time_local();
}

bool manager_state_active(void* _manager_p, int _nKeyName, int _group, int _id, int _nSerial)
{
	if (mpool_get().is_terminated())
		return NULL;

	BaseStateManager* manager = (BaseStateManager*)_manager_p;
	STLMnInt stlMnGroupId;
	stlMnGroupId[_group] = _id;

	BaseState* state = manager->GetSpace(0)->GetStateName(_nKeyName, &stlMnGroupId, _nSerial);
	if (state == NULL)
		return false;
	return true;
}

void* manager_event_group_make(void* _manager_p, int _evt, int _groupId, int _id)
{
	if (mpool_get().is_terminated())
		return NULL;

	BaseStateManager* manager = (BaseStateManager*)_manager_p;

	int nKeyState = HASH_STATE(BaseStateEventGlobal);
	BaseDStructureValue* evt = manager->make_event(nKeyState, _evt);
	//evt->ref_inc();
	evt->set_alloc(nKeyState, &_evt);

	STLMnInt stlMnGroupId;
	stlMnGroupId[_groupId] = _id;
	BaseState::group_id_set(evt, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);

	return evt;
}

void* dsv_event_group_make(void* _dsv_p, int _evt, int _groupId, int _id)
{
	if (mpool_get().is_terminated())
		return NULL;

	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_dsv_p);
	BaseState* pState = (BaseState*)pdsv->m_pVoidParam;
	BaseStateManager* manager = pState->get_space()->get_manager();
	BaseDStructureValue* evt = pState->EventSendGet(_evt);

	STLMnInt stlMnGroupId;

	stlMnGroupId[_groupId] = _id;

	BaseState::group_id_set(evt, HASH_STATE(BaseTransitionGoalIdentifier), stlMnGroupId);

	int nSerial = pState->obj_serial_get();
	evt->set_alloc(HASH_STATE(RevStateEventCaster), &nSerial);

	return evt;
}

void dsv_group_id_set(void* _dsv_p, int _groupId, int _id)
{
	if (mpool_get().is_terminated())
		return;

	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_dsv_p);
	if (pdsv == NULL)
		return;
	BaseState* pState = (BaseState*)pdsv->m_pVoidParam;
	pState->group_id_add(_groupId, _id);
}

void* dsv_create(const char* _strMainName, const char* _strStateName)
{
	return 0;
}

const char* vscript_column_name_get_(int _nIndex)
{
	const char* str = BaseDStructure::get_type_name(_nIndex);

	if (str == NULL)
		return NULL;
	return str;
}

bool dsv_get_float(void* _pdst, const char* _strColName, float* _pfValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	if (pdst->get_local_seq(_strColName, ((void*)_pfValue)))
		return true;
	return false;
}

bool dsv_get_hash_float(void* _pdst, int _hash, float* _pfValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	const float* value = NULL;
	if (pdst->get_local_seq(_hash, ((const void**)&value)))
	{
		*_pfValue = *value;
		return true;
	}
	return false;
}

bool dsv_get_int2(void* _pdst, int _nHash, INT32* _pnLong)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	const int* pnInt = NULL;
	if (pdst->get_local_seq(_nHash, ((const void**)&pnInt)))
	{
		*_pnLong = *pnInt;
		return true;
	}
	return false;
}

bool dsv_get_int642(void* _pdst, int _key, INT64* _pnInt)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	const void* data;
	if (pdst->get_local_seq(_key, &data))
	{
		*_pnInt = *((INT64*)data);
		return true;
	}
	return false;
}

bool dsv_get_int64(void* _pdst, const char* _strColName, INT64* _pnInt)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	if (pdst->get_local_seq(_strColName, ((void*)_pnInt)))
		return true;
	return false;
}

bool dsv_get_int(void* _pdst, const char* _strColName, INT32* _pnInt)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	if (pdst->get_local_seq(_strColName, ((void*)_pnInt)))
		return true;
	return false;
}

bool dsv_set_void(void* _pdst, int _hash, const void* _pValue, INT32 _nCount)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	short cnt_s = (short)_nCount;
	pdst->set_alloc(_hash, (const void*)_pValue, cnt_s);
	return true;
}

bool dsv_set_float_array(void* _pdst, const char* _strColName, const float* _pfValue, INT32 _nCount)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	short cnt_s = (short)_nCount;
	pdst->set_alloc(_strColName, (const void*)_pfValue, cnt_s);
	return true;
}

bool dsv_get_float_array(void* _pdst, const char* _strColName, const float** _ppfFloatArray, INT32* _pnCount)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	int nHash = STRTOHASH(_strColName);
	short nCnt;
	if (pdst->get_local_seq(nHash, (const void**)_ppfFloatArray, &nCnt))
	{
		*_pnCount = (int)nCnt;
		return true;
	}
	return false;
}


bool dsv_get_int_array(void* _pdst, const char* _strColName, const INT32** _ppnIntArray, INT32* _pnCount)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	int nHash = STRTOHASH(_strColName);
	short nCnt;
	if (pdst->get_local_seq(nHash, (const void**)_ppnIntArray, &nCnt))
	{
		*_pnCount = (int)nCnt;
		return true;
	}
	return false;
}

bool dsv_set_float(void* _pdst, const char* _strColName, float _fValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	pdst->set_alloc(_strColName, (const void*)&_fValue);
	return true;
}

bool dsv_set_hash_float(void* _pdst, int _hash, float _fValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	pdst->set_alloc(_hash, (const void*)&_fValue);
	return true;
}

bool dsv_set_hash_int(void* _pdst, int _col_hash, INT32 _nValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	int nIndex = pdst->get_index(_col_hash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	B_ASSERT(type == TYPE_INT32);

	INT64 value = (INT64)_nValue;
	pdst->set_alloc(_col_hash, (const void*)&value);
	return true;

}

bool dsv_set_int(void* _pdst, const char* _strColName, INT32 _nValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	int hash = STRTOHASH(_strColName);
	int nIndex = pdst->get_index(hash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	B_ASSERT(type == TYPE_INT32);

	INT64 value = (INT64)_nValue;
	pdst->set_alloc(_strColName, (const void*)&value);
	return true;
}

bool dsv_add_int64(void* _pdst, const char* _strColName, INT64 _nValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	pdst->add_alloc(_strColName, (const void*)&_nValue);
	return true;
}

bool dsv_add_int(void* _pdst, const char* _strColName, INT32 _nValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	INT64 value = (INT64)_nValue;
	pdst->add_alloc(_strColName, (const void*)&value);
	return true;
}

bool dsv_add_float_array(void* _pdst, const char* _strColName, float* _afValue, INT32 _nCnt)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	short _shCnt = _nCnt;
	pdst->add_alloc(_strColName, (const void*)_afValue, _shCnt);
	return true;
}

bool dsv_add_int_array(void* _pdst, const char* _strColName, INT32* _anValue, INT32 _nCnt)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	short _shCnt = _nCnt;
	pdst->add_alloc(_strColName, (const void*)_anValue, _shCnt);
	return true;
}

char* static_translate(const char* _str)
{
	const char* strRet = PTGET(_str);
	int l = (int)strlen(strRet) + 1;
	char* ret = PT_Alloc(char, l);
	PT_AFree(ret);
	strcpy_s(ret, l, strRet);
	return ret;
}

char* filedialog_open(const char* _ext)
{
	char* ret = PT_Alloc(char, 4096);
	PT_AFree(ret);

	return BaseSystem::get_filenamedialogopen(_ext, ret, 4096);
}

char* filedialog_save(const char* _ext)
{
	char* ret = PT_Alloc(char, 4096);
	PT_AFree(ret);
	return BaseSystem::get_filenamedialogsave(_ext, ret, 4096);
}

void* static_variable_make_string(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, const char* _strFormat)
{
	char* ret = PT_Alloc(char, 4096);

	strcpy_s(ret, 4096, _strFormat);
	BaseState::VariableStringMake((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, ret, 4096);
	PT_AFree(ret);
	return ret;
}

INT64 static_variable_param_int64_get(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, int _nSeq, void* _pdsvDefault)
{
	const INT64* nValue = (const INT64*)BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, _nSeq, (BaseDStructureValue*)_pdsvDefault);
	if (nValue == NULL)
		return -999999;
	return *nValue;
}

int static_variable_param_int_get(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, int _nSeq, void* _pdsvDefault)
{
	const int* nValue = (const int*)BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, _nSeq, (BaseDStructureValue*)_pdsvDefault);
	if (nValue == NULL)
		return -999999;
	return *nValue;
}

int system_memory_alloc_size()
{
	return mpool_get().memory_total_size();
}

char* static_variable_param_string_get(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, int _nSeq, void* _pdsvDefault)
{
	const char* strValue = (const char*)BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, _nSeq, (BaseDStructureValue*)_pdsvDefault);

	if (strValue == NULL)
		return NULL;

	int l = (int)strlen(strValue) + 1;
	char* ret = PT_Alloc(char, l);
	PT_AFree(ret);

	strcpy_s(ret, l, strValue);
	return ret;
}

bool static_variable_param_void_set(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, int _nSeq, void* _pdsvDefault, const void* _void_p, int _cnt)
{
	short cnt = (short)_cnt;
	return BaseState::VariableSet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, (BaseDStructureValue*)_pdsvDefault, _void_p, _nSeq, cnt);
}

const void *static_variable_param_void_get(void* _pdsvBase, void* _pdsvEvent, void* _pdsvContext, int _nSeq, int *_size)
{
	return BaseState::VariableGet((const BaseDStructureValue*)_pdsvBase, (BaseDStructureValue*)_pdsvContext, (BaseDStructureValue*)_pdsvEvent, _nSeq);
}

void* dsv_get_string(void* _pdst, const char* _strColName)
{
	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_pdst);
	int hash = STRTOHASH(_strColName);

	short _nCnt = 0;
	int seq = pdsv->sequence_get_local();
	STLVpVoid stlData;
	const char* strData = NULL;
	bool bRet = pdsv->get(hash, (const void**)&strData, &_nCnt, seq);

	if (bRet) {
		stlData.push_back((void*)strData);
		int l = (int)strlen(strData);
		int len = l;
		while (l >= LIMIT_STR)
		{
			seq++;
			if (!pdsv->get(hash, (const void**)&strData, &_nCnt, seq))
				break;
			stlData.push_back((void*)strData);
			l = (int)strlen(strData);
			len += l;
		}

		char* ret = PT_Alloc(char, len + 1);
		PT_AFree(ret);

		for (int i = 0; i < (int)stlData.size(); i++) {
			int blockLen = (int)strlen((const char*)stlData[i]);
			strcpy_s(ret + i * LIMIT_STR, blockLen + 1, (const char*)stlData[i]);
		}
		return (void*)ret;
	}
	return NULL;
}

void* dsv_get_string2(void* _pdst, int _nHash)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	const char* strRet;
#ifdef _DEBUG
	int index = pdst->get_index(_nHash);
	if (index == -1)
		return NULL;
	int type = BaseDStructure::get_type(index);
	B_ASSERT(type == TYPE_STRING);
#endif
	if (pdst->get_local_seq(_nHash, (const void**)&strRet))
	{
		int l = (int)strlen(strRet) + 1;
		char* ret = PT_Alloc(char, l);
		PT_AFree(ret);

		strcpy_s(ret, l, strRet);
		return (void*)ret;
	}

	return NULL;
}

void* manager_enum_get(void* _pManager, const char* _strEnum)
{
	int nHashEnum = STRTOHASH(_strEnum);
	BaseDStructure* enum_pdst = ((StateManagerPnID*)_pManager)->EnumGet(nHashEnum);
	return enum_pdst;
}

bool manager_enum_get_int(void* _pManager, const char* _strEnum, int _nKey, int _nHashCol, INT32* _pnValue)
{
	int nHashEnum = STRTOHASH(_strEnum);
	const int* pnValue = (const int*)((StateManagerPnID*)_pManager)->EnumGetValue(nHashEnum, _nKey, _nHashCol);
	if (pnValue == NULL)
		return false;
	*_pnValue = *pnValue;
	return true;
}

void* manager_enum_get_string(void* _pManager, const char* _strEnum, int _nKey, int _nHashCol)
{
	int nHashEnum = STRTOHASH(_strEnum);
	const char* strValue = (const char*)((StateManagerPnID*)_pManager)->EnumGetValue(nHashEnum, _nKey, _nHashCol);

	if (strValue)
	{
		int l = (int)strlen(strValue) + 1;
		char* ret = PT_Alloc(char, l);
		PT_AFree(ret);

		strcpy_s(ret, l, strValue);
		return (void*)ret;
	}
	return NULL;
}

bool dsv_set_string2(void* _pdst, INT32 _nHash, const char* _strValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	int nIndex = pdst->get_index(_nHash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	B_ASSERT(type == TYPE_STRING);

	pdst->set_alloc(_nHash, (const void*)_strValue);
	return true;
}

bool dsv_set_variable2(void* _pManager, void* _pdst, int _nHash, INT32 _nValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	return ((BaseStateManager*)_pManager)->variable_set(pdst, _nHash, TYPE_INT32, (const void*)&_nValue, NULL);
}

bool dsv_set_ptr(void* _pdst, int _hash, void* _pPtr, int _cnt)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	int nIndex = pdst->get_index(_hash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	short cnt = (short)_cnt;

	INT64 nPoint = (INT64)_pPtr;
	pdst->set_alloc(_hash, (const void*)&nPoint, cnt);
	return true;
}

bool dsv_set_string(void* _pdst, const char* _strColName, const char* _strValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	int hash = STRTOHASH(_strColName);
	int nIndex = pdst->get_index(hash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	if (_strValue != NULL)
		B_ASSERT(type == TYPE_STRING);

	pdst->set_alloc(hash, (const void*)_strValue);
	return true;
}

bool dsv_add_string(void* _pdst, const char* _strColName, const char* _strValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	int hash = STRTOHASH(_strColName);
	int nIndex = pdst->get_index(hash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	B_ASSERT(type == TYPE_STRING);

	int len = (int)strlen(_strValue);
	if (len > LIMIT_STR)
	{
		char* buf = PT_Alloc(char, LIMIT_STR + 1);
		const char* start = _strValue;
		for (int i = 0; i < len / LIMIT_STR; i++)
		{
			memcpy(buf, start, LIMIT_STR);
			buf[LIMIT_STR] = NULL;
			pdst->add_alloc(_strColName, (const void*)buf);
			start = _strValue + (i + 1) * LIMIT_STR;
		}
		int mod = len % LIMIT_STR;
		if (mod > 0) {
			memcpy(buf, start, mod);
			buf[mod] = NULL;
			pdst->add_alloc(_strColName, (const void*)buf);
		}

		PT_Free(buf);
	}
	else {
		pdst->add_alloc(_strColName, (const void*)_strValue);
	}
	return true;
}

bool dsv_add_ldata(void* _pdst, const char* _strColName, const char* _data, int _size)
{
	static int count = 0;
	count++;

	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);
	int hash = STRTOHASH(_strColName);
	int nIndex = pdst->get_index(hash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	B_ASSERT(type == TYPE_ARRAY_BYTE);

	if (_size > LIMIT_STR)
	{
		const char* start = _data;
		char* buf = PT_Alloc(char, LIMIT_STR + 1);
		for (int i = 0; i < _size / LIMIT_STR; i++)
		{
			memcpy(buf, start, LIMIT_STR);
			pdst->add_alloc(_strColName, (const void*)buf, LIMIT_STR);
			start = _data + (i + 1) * LIMIT_STR;
		}
		int mod = _size % LIMIT_STR;
		if (mod > 0) {
			memcpy(buf, start, mod);
			pdst->add_alloc(_strColName, (const void*)buf, mod);
		}
		PT_Free(buf);
	}
	else {
		pdst->add_alloc(_strColName, (const void*)_data);
	}
	return true;
}

INT64 g_stream_get(INT64 _queue)
{
	if (_queue == 0)
		return 0;

	BaseCircleQueue* queue = (BaseCircleQueue*)_queue;

	INT64 data = 0, last = 0;
	do {
		if (data && last)
			g_free_data(last);
		last = data;
		data = (INT64)queue->pop();
	} while (data);

	return last;
}

INT64 g_get_alloc(int _nCnt, const char* _data)
{
	INT64 ref = mpool_get().get_alloc(_nCnt);
	if(ref != 0)
		memcpy((void*)ref, _data, _nCnt);
	return ref;
}

void g_free_data(INT64 _nRef)
{
	mpool_get().free_mem(_nRef);
}

void* g_get_ldata(INT64 _nRef, int* _pnCnt)
{
	if(!BaseCircleQueue::stream_get())
		return NULL;

	void *point = BaseCircleQueue::stream_get()->top();
	BaseCircleQueue::streamSize_get()->top();

	while (BaseCircleQueue::stream_get()->size_data() > 1)
	{
		point = BaseCircleQueue::stream_get()->pop();
		BaseCircleQueue::streamSize_get()->pop();
		PT_Free(point);
	}

	INT64 siz = (INT64)BaseCircleQueue::streamSize_get()->top();
	*_pnCnt = (int)siz;
	return BaseCircleQueue::stream_get()->top();
	//return mpool_get().get_mem(_nRef, _pnCnt);
}

const void* dsv_get_ldata(void* _pdst, INT32 _nKey, int* _pnCnt)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	const void* ret2 = NULL;
	if (!pdst->get_mass(_nKey, &ret2, _pnCnt))
	{
		return NULL;
	}

	return ret2;
}

bool dsv_get_ptr2(void* _pdst, int _hash, void** _pPtr, int* _cnt, int _seq)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	short cnt = 0;
	if (pdst->get(_hash, (const void**)_pPtr, &cnt, _seq))
	{
		if (_cnt)
			*_cnt = (int)cnt;
		return true;
	}
	return false;
}

bool dsv_get_ptr(void* _pdst, const char* _strColName, void** _pPtr)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	const INT64* pnPtr = NULL;
	if (pdst->get(STRTOHASH(_strColName), (const void**)&pnPtr))
	{
		*_pPtr = (void*)*pnPtr;
		return true;
	}
	return false;
}

bool dsv_set_variable_ptr(void* _pManager, void* _pdst, const char* _strColName, void* _pPtr)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	if (!((BaseStateManager*)_pManager)->variable_define(STRTOHASH(_strColName), TYPE_INT64, true))
		return false;

	INT64 nPoint = (INT64)_pPtr;
	pdst->set_alloc(_strColName, (const void*)&nPoint);
	return true;
}

bool dsv_set_variable(void* _pManager, void* _pdst, const char* _strColName, INT32 _nValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	return ((BaseStateManager*)_pManager)->variable_set(pdst, STRTOHASH(_strColName), TYPE_INT32, (const void*)&_nValue, NULL);
	return true;
}

bool dsv_set_variable_string(void* _pManager, void* _pdst, const char* _strColName, const char* _strValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	return ((BaseStateManager*)_pManager)->variable_set(pdst, STRTOHASH(_strColName), TYPE_STRING, _strValue, NULL);
}

bool dsv_set_variable_string2(void* _pManager, void* _pdst, INT32 _nHash, const char* _strValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	return ((BaseStateManager*)_pManager)->variable_set(pdst, _nHash, TYPE_STRING, _strValue, NULL);
}

bool dsv_add_variable_string(void* _pManager, void* _pdst, const char* _strColName, const char* _str)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	if (!((BaseStateManager*)_pManager)->variable_check(pdst, STRTOHASH(_strColName)))
		return false;
	pdst->add_alloc(_strColName, (const void*)_str);

	return true;
}

bool dsv_add_variable(void* _pManager, void* _pdst, const char* _strColName, INT32 _nValue)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	if (!((BaseStateManager*)_pManager)->variable_check(pdst, STRTOHASH(_strColName)))
		return false;
	pdst->add_alloc(_strColName, (const void*)&_nValue);

	return true;
}

bool dsv_param_get(void* _pdst, const char* _strHash, INT32* _pnSeq, INT32* _pnHashVariable)
{
	BaseDStructureValue* pdst = ((BaseDStructureValue*)_pdst);

	bool bRet = false;
	int nSeq = *_pnSeq;

	if (nSeq == 0)
		nSeq = pdst->sequence_get() + 1;

	if (STRTOHASH(_strHash) == pdst->get_colum(nSeq)) {
		const int* pnHashVariable;
		bRet = pdst->get((const void**)&pnHashVariable, NULL, nSeq);
		*_pnHashVariable = *pnHashVariable;
	}
	return bRet;
}

const void* dsv_column_get(void* _dsv_p, int _column_n, int* _hash_pn, int* _count_pn)
{
	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_dsv_p);

	static bool start = false;
	if (start)
	{
		g_SendMessage(LOG_MSG, "----- %d", _column_n);
		pdsv->dbg_mem_checkout();
	}
	*_hash_pn = pdsv->get_colum(_column_n);

	short cnt_s;
	const void* pVoid = NULL;
	if (!pdsv->get(&pVoid, &cnt_s, _column_n))
		return NULL;

	if (*_hash_pn == STRTOHASH("PnDrawLine") || start)
	{
		g_SendMessage(LOG_MSG, "-line//- %x %d: %f - %d", pdsv, _column_n, *((float*)pVoid), (int)cnt_s);
		if (*_hash_pn == STRTOHASH("PnDrawLine"))
			start = true;
		else
			start = false;
	}
	if (_count_pn)
		*_count_pn = (int)cnt_s;

	return pVoid;
}

int dsv_serial_get(void* _pdsv)
{
	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_pdsv);
	BaseState* pState = ((BaseState*)pdsv->m_pVoidParam);
	return pState->obj_serial_get();
}

void* dsv_copy_get(void* _dsv_p)
{
	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_dsv_p);

	BaseDStructureValue* pdsvEvent;
	PT_OAlloc2(pdsvEvent, BaseDStructureValue, pdsv->get_base(), 1024);

	*pdsvEvent = *pdsv;

	return pdsvEvent;
}

void dsv_delete(void* _dsv_p)
{
	BaseDStructureValue* pdsv = ((BaseDStructureValue*)_dsv_p);
	PT_OFree(pdsv);
}

void* dst_create()
{
	void* pRet = NULL;

	BaseDStructure* pdst;
	PT_OAlloc(pdst, BaseDStructure);

	pRet = pdst;

	return pRet;
}

void dst_delete(void* _pdst) {
	BaseDStructure* pdst = (BaseDStructure*)_pdst;
	PT_OFree(pdst);
}

bool dst_save(void* _pdst, const char* _strFileName, int _type)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	if (_strFileName == NULL)
		return pdst->save();
	char strBuff[4096];
	strcpy_s(strBuff, 4096, _strFileName);

	if (_type == 3)
	{
		return pdst->save(BaseStateManager::get_manager()->path_state_full_make(strBuff, 4096), 2);// skip to save define
	}
	return pdst->save(BaseStateManager::get_manager()->path_full_make(strBuff, 4096), 2);// skip to save define
}

bool dst_load(void* _pdst, const char* _strFileName, int _type)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	char strBuff[4096];
	strcpy_s(strBuff, 4096, _strFileName);

	if (_type == 3 || _type == 4)
	{
		if (_type == 4)
			_type = 0;
		return pdst->load(BaseStateManager::get_manager()->path_state_full_make(strBuff, 4096), _type);
	}
	return pdst->load(BaseStateManager::get_manager()->path_full_make(strBuff, 4096), _type);
}

int dst_get_index(INT32 _nHash)
{
	short index = BaseDStructure::get_index(_nHash);
	return (int)index;
}

void dst_clear(void* _pdst)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);

	pdst->release();
}

//set_alloc
//add_row_alloc

bool dst_add_string(void* _pdst, INT32 _nKey, INT32 _nIndex, const char* _strString)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->add_string(_nKey, _nIndex, _strString);
}

bool dst_set_string(void* _pdst, INT32 _nKey, INT32 _nIndex, const char* _strString)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	pdst->set_alloc(_nKey, _nIndex, (const void *)_strString);
	return true;
}

bool dst_add_float(void* _pdst, INT32 _nKey, INT32 _nIndex, float* _paData, INT32 _nCnt)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	pdst->add_row_alloc(_nKey, _nIndex, _paData, (short)_nCnt);
	return true;
}

bool dst_add_int(void* _pdst, INT32 _nKey, INT32 _nIndex, INT32* _paData, INT32 _nCnt)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);

	int type = BaseDStructure::get_type(_nIndex);
	B_ASSERT(type == TYPE_INT32
		|| type == TYPE_ARRAY_INT32);

	pdst->add_row_alloc(_nKey, _nIndex, _paData, (short)_nCnt);

	return true;
}

bool dst_set_int(void* _pdst, INT32 _nKey, INT32 _nIndex, INT32* _paData, INT32 _nCnt)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);

	int type = BaseDStructure::get_type(_nIndex);
	B_ASSERT(type == TYPE_INT32
		|| type == TYPE_ARRAY_INT32);

	pdst->set_alloc(_nKey, _nIndex, _paData, (short)_nCnt);
	return true;
}

bool dst_get_int_index(void* _pdst, INT32 _nKey, INT32 _nIndex, const INT32** _paData, INT32* _pnCnt)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	short _nCnt = 0;

	int type = BaseDStructure::get_type(_nIndex);
	B_ASSERT(type == TYPE_INT32
		|| type == TYPE_ARRAY_INT32
		|| type == TYPE_BYTE);

	bool bRet = pdst->get(_nKey, _nIndex, (const void**)_paData, &_nCnt);

	static int s_return;
	if (type == TYPE_BYTE)
	{
		bbyte ret = *((bbyte*)_paData);
		s_return = (int)ret;
		*_paData = &s_return;
	}

	if (_pnCnt)
		*_pnCnt = _nCnt;
	return bRet;
}

bool dst_get_int(void* _pdst, INT32 _nKey, INT32 _nSequence, const INT32** _paData, INT32* _pnCnt)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	short _nCnt = 0;
	bool bRet = pdst->get(_nKey, (const void**)_paData, &_nCnt, _nSequence);

	if (_pnCnt)
		*_pnCnt = _nCnt;
	return bRet;
}

char s_strURL[1024];
void manager_url_set(const char* _str)
{
	strcpy_s(s_strURL, 1024, _str);
}

const void* manager_url_get()
{
	return (const void*)s_strURL;
}

void* dst_get_string(void* _pdst, INT32 _nKey, INT32 _nSequence)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	short _nCnt = 0;
	STLVpVoid stlData;
	const char* strData = NULL;
	bool bRet = pdst->get(_nKey, _nSequence, (const void**)&strData, &_nCnt);

	if (bRet) {
		stlData.push_back((void*)strData);
		int l = (int)strlen(strData);
		int rpt = 1;
		int len = l;
		while (l >= LIMIT_STR)
		{
			if (!pdst->get(_nKey, _nSequence, (const void**)&strData, &_nCnt))
				break;
			stlData.push_back((void*)strData);
			l = (int)strlen(strData);
			len += l;
		}

		char* ret = PT_Alloc(char, len + 1);
		PT_AFree(ret);

		for (int i = 0; i < (int)stlData.size(); i++) {
			int blockLen = (int)strlen((const char*)stlData[i]);
			strcpy_s(ret + i * LIMIT_STR, blockLen + 1, (const char*)stlData[i]);
		}
		return (void*)ret;
	}

	return NULL;
}

char* dst_editor_string_get(void* _pdst, int _key_n, int _seq_n)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);

	char* ret = PT_Alloc(char, 4096);
	*ret = NULL;

	if (!pdst->get_string(_key_n, ret, _seq_n)) {
		PT_Free(ret);
		return NULL;
	}

	PT_AFree(ret);
	return ret;
}
bool dst_editor_string_add(void* _pdst, int _key_n, int _index_n, char* _str_value)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->add_string(_key_n, _index_n, _str_value);
}

bool dst_get_float(void* _pdst, INT32 _nKey, INT32 _nSequence, const float** _paData, INT32* _pnCnt)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	short _nCnt = 0;
	bool bRet = pdst->get(_nKey, (const void**)_paData, &_nCnt, _nSequence);

	if (_pnCnt)
		*_pnCnt = _nCnt;
	return bRet;
}

STLMnstlRecord::iterator s_itDst;

int dst_key_first_get(void* _pdst)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);

	return pdst->get_first_key(&s_itDst);
}

int dst_key_next_get(void* _pdst)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);

	return pdst->get_next_key(&s_itDst);
}

int dst_get_type_count(void* _pdst)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_type_count();
}

int dst_get_type_flag(void* _pdst, int _nIndex)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_type_flag(_nIndex);
}

bool dst_set_type_flag(void* _pdst, int _nIndex, int _nFlag)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_type_flag(_nFlag);
}

const char* dst_get_type_name(void* _pdst, int _nIndex)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	const char* ret = pdst->get_type_name(_nIndex);
	static char buf[1024];
	strcpy_s(buf, 1024, ret);
	return buf;
}

int dst_get_type(int _nIndex)
{
	return BaseDStructure::get_type(_nIndex);
}

int dst_get_type_size(void* _pdst, int _nIndex)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_type_size(_nIndex);
}

const char* dst_get_type_enum_state(void* _pdst, int _nIndex)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_type_enum_state(_nIndex);
}

const char* dst_get_type_comment(void* _pdst, int _nIndex)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_type_comment(_nIndex);
}

bool dst_set_type_enum(void* _pdst, int _nIndex, const char* _strEnum)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->set_type_enum(_nIndex, _strEnum);
}

bool dst_set_type_comment(void* _pdst, int _nIndex, const char* _strComment)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->set_type_comment(_nIndex, _strComment);
}

bool dst_type_original_check(void* _pdst, int _nHash)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->type_original_check(_nHash);
}
int dst_get_hash(int _nIndex)
{
	return BaseDStructure::get_hash(_nIndex);
}
void dst_add_row_alloc(void* _pdst, int _nKey, int _nIndex, const void* _pVoid, short _nCount)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->add_row_alloc(_nKey, _nIndex, _pVoid, _nCount);
}
int dst_get_index_seq(void* _pdst, int _nKey, int _nSequence)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_index(_nKey, _nSequence);
}
bool dst_get(void* _pdst, int _nKey, const void** _ppVoid, short* _pnCount, int _nIndexSequence)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get(_nKey, _ppVoid, _pnCount, _nIndexSequence);
}
bool dst_get_param(void* _pdst, int _nKey, int _nIndex, const void** _ppVoid, short* _pnCount, int _nIndexStartSequence)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_param(_nKey, _nIndex, _ppVoid, _pnCount, _nIndexStartSequence);
}
/*
bool dst_get(void* _pdst, int _nKey, int _nIndex, const void** _ppVoid, short* _pnCount, int _nIndexSequence)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get(_nKey, _nIndex, _ppVoid, _pnCount, _nIndexSequence);
}
//*/
void dst_set_alloc(void* _pdst, int _nKey, int _nIndex, const void* _pVoid, short _nCount)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->set_alloc(_nKey, _nIndex, _pVoid, _nCount);
}
void dst_set(void* _pdst, int _nKey, int _nIndex, void* _pVoid)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->set(_nKey, _nIndex, _pVoid);
}
bool dst_get_by_hash(void* _pdst, int _nKey, int _nHash, const void** _ppVoid)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_by_hash(_nKey, _nHash, _ppVoid);
}
bool dst_release(void* _pdst, int _nKey)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->release(_nKey);
}
bool dst_release_rows(void* _pdst, int _nKey)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->release_rows(_nKey);
}
bool dst_is_exist(void* _pdst, int _nKey)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->is_exist(_nKey);
}
bool dst_change_key(void* _pdst, int _nKey, int _nKeyto)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->change_key(_nKey, _nKeyto);
}
int dst_get_count_colum(void* _pdst, int _nKey)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_count_colum(_nKey);
}
int dst_getn_flag(void* _pdst, const char* _strFlag)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->get_flag(_strFlag);
}
char* dst_getstr_flag(void* _pdst, int _nFlag)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);

	char* ret = PT_Alloc(char, 4096);

	pdst->get_flag(ret, _nFlag);
	PT_AFree(ret);
	return ret;
}
int dst_create_key_link(void* _pdst, const char* _str_key)
{
	BaseDStructure* pdst = ((BaseDStructure*)_pdst);
	return pdst->create_key_link(_str_key);
}

bool load_asc_for_etagen(void* _pdstVoid, const char* _strFilename)
{
	BaseDStructure* _pdst = (BaseDStructure*)_pdstVoid;

	int nKeyIndex = _pdst->get_index(STRTOHASH("nKey"));
	int nIntIndex = _pdst->get_index(STRTOHASH("FileInt_anV"));
	int nStrIndex = _pdst->get_index(STRTOHASH("FileString_strV"));
	int nFloatIndex = _pdst->get_index(STRTOHASH("FileFloat_afV"));

	BaseFile file;
	if (!file.OpenFile(_strFilename, BaseFile::OPEN_READ))
	{
		int nCnt = 0;
		{
			_pdst->add_row_alloc(0, nKeyIndex, &nCnt);
			nCnt = 3;
			_pdst->add_row_alloc(3, nKeyIndex, &nCnt);

			file.read_asc_line();
			file.read_asc_integer(&nCnt);
			_pdst->add_row_alloc(0, nIntIndex, &nCnt, 1);
			_pdst->add_row_alloc(3, nIntIndex, &nCnt, 1);

			STLVInt	stlVnData;
			char strData[1024], strBuf[1024];

			// Scene Object Type
			// 1 - Ball
			// 2 - Player Home
			// 3 - Player Away
			// 5 - Primitive Circle
			// 6 - Primitive Line
			// 7 - Primitive Text

			int nMod, nValue;
			file.read_asc_line();//file.read_asc_string("{", 0);
			for (int i = 0; i < nCnt; i++)
			{ //{ {int Backnumber, int Scene Object Type, int Model Type(0: Normal, 1: Goal Keeper), int ID, int First Frame, string Player name}, {...}, ... }
				file.read_asc_line();
				file.read_asc_string(strBuf, 1024);
				file.read_asc_integer(&nValue);// anData[nStart + 0]); // backnumber
				stlVnData.push_back(nValue);
				file.read_asc_integer(&nValue);// file.write_asc_integer(nMod); // Scene type
				nMod = nValue;
				file.read_asc_integer(&nValue);// anData[nStart + 1] - nMod) / 10); // Model
				stlVnData.push_back(nMod + nValue * 10);
				file.read_asc_integer(&nValue);// anData[nStart + 2]); // ID
				stlVnData.push_back(nValue);
				file.read_asc_integer(&nValue);// anData[nStart + 3]); // First Frame
				stlVnData.push_back(nValue);

				strData[0] = 0;
				file.read_asc_string(strData, 1024);// strData, strlen(strData) + 1);// name of players, i is for index of CtrInfo
				_pdst->add_row_alloc(3, nStrIndex, strData, (short)strlen(strData) + 1);
				file.read_asc_string(strBuf, 1024);// }
			}

			_pdst->add_row_alloc(0, nIntIndex, (void*)&stlVnData.front(), (short)stlVnData.size());// Object(Ball, Players and Primitives) configuration data

			file.read_asc_line();//	file.write_asc_string("}", 0);	file.write_asc_line();
		}

		{
			nCnt = 1;
			_pdst->add_row_alloc(1, nKeyIndex, &nCnt);

			int nNumOfStep;
			file.read_asc_line();
			file.read_asc_integer(&nNumOfStep);
			_pdst->add_row_alloc(1, nIntIndex, &nNumOfStep, 1);
			file.read_asc_line(); //file.write_asc_string("{", 0);

			char strBuf[1024];
			int nValue;
			float fValue;

			for (int nStep = 0; nStep < nNumOfStep; nStep++) {
				file.read_asc_line();
				int nNumOfObject;
				file.read_asc_integer(&nNumOfObject);
				_pdst->add_row_alloc(1, nKeyIndex, &nNumOfObject);

				STLVInt	stlVnData;
				stlVnData.resize(nNumOfObject * 12);

				for (int i = 0; i < nNumOfObject; i++) {
					int nStart = i * 12;
					file.read_asc_line();
					file.read_asc_string(strBuf, 1024); //file.write_asc_string("{", 0);

					// { int id, float[2] position, int holde ball, int ball type, int show infor 
					//		, int highlighted, int length ext position { float[2] v3Pos1, float[2] v3Pos2, ...} }
					//
					file.read_asc_integer(&nValue);	stlVnData[nStart + 0] = nValue; // anData[nStart + 0]); // id
					file.read_asc_float(&fValue);	stlVnData[nStart + 1] = (int)(fValue * 1000.f); // anData[nStart + 1] / 1000.0f);
					file.read_asc_float(&fValue);	stlVnData[nStart + 2] = (int)(fValue * 1000.f); //anData[nStart + 2] / 1000.0f); // float[2] position
					file.read_asc_integer(&nValue);	stlVnData[nStart + 3] = nValue; // anData[nStart + 3]); // hold ball
					file.read_asc_integer(&nValue);	stlVnData[nStart + 4] = nValue; // anData[nStart + 4]); // ball type
					int nShowInfo = 0;
					file.read_asc_integer(&nShowInfo);
					file.read_asc_integer(&nValue); stlVnData[nStart + 5] = nValue * 2 + nShowInfo;// highlight
					int nLen;
					file.read_asc_integer(&nLen);	stlVnData[nStart + 6] = nLen; // anData[nStart + 6]); // length ext data
					file.read_asc_string(strBuf, 1024); // {
					for (int k = 0; k < nLen; k++) {
						file.read_asc_float(&fValue); stlVnData[nStart + 6 + k * 2 + 1] = (int)(fValue * 1000.f); //(float)anData[nStart + 6 + k * 2 + 1] / 1000.0f);
						file.read_asc_float(&fValue); stlVnData[nStart + 6 + k * 2 + 2] = (int)(fValue * 1000.f); //(float)anData[nStart + 6 + k * 2 + 2] / 1000.0f);
					}
					// } } in file
				}
				_pdst->add_row_alloc(1, nIntIndex, (const void*)&stlVnData.front(), (short)stlVnData.size());
			}
			file.read_asc_line(); //  }

			int nNumOfComment;
			int anFrame[2];
			file.read_asc_line(); //  
			file.read_asc_integer(&nNumOfComment);
			file.read_asc_line(); //  {
			_pdst->add_row_alloc(1, nIntIndex, (const void*)&nNumOfComment, 1);

			for (int i = 0; i < nNumOfComment; i++) {
				file.read_asc_line();
				file.read_asc_string(strBuf, 1024);
				file.read_asc_integer(&anFrame[0]);
				file.read_asc_integer(&anFrame[1]);
				file.read_asc_string(strBuf, 1024);

				_pdst->add_row_alloc(1, nIntIndex, (const void*)anFrame, 2);
				_pdst->add_row_alloc(1, nStrIndex, (const void*)strBuf, (short)strlen(strBuf) + 1);
			}
			file.read_asc_line(); // }
		}
		{
			int nNumOfData = 2, nValue;
			char strBuf[1024];

			_pdst->add_row_alloc(2, nKeyIndex, (const void*)&nNumOfData);

			file.read_asc_line();
			file.read_asc_integer(&nNumOfData);
			file.read_asc_string(strBuf, 1024);

			_pdst->add_row_alloc(2, nIntIndex, (const void*)&nNumOfData, 1);

			STLVInt	stlVnDatas;

			for (int i = 0; i < nNumOfData; i++) {
				file.read_asc_integer(&nValue); // numbers for home
				stlVnDatas.push_back(nValue);
			}
			_pdst->add_row_alloc(2, nIntIndex, (const void*)&stlVnDatas.front(), (short)stlVnDatas.size());

			file.read_asc_line();

			file.read_asc_integer(&nNumOfData);
			_pdst->add_row_alloc(2, nIntIndex, (const void*)&nNumOfData, 1);
			file.read_asc_string(strBuf, 1024);

			stlVnDatas.clear();
			for (int i = 0; i < nNumOfData; i++) {
				file.read_asc_integer(&nValue); // numbers for away
				stlVnDatas.push_back(nValue);
			}
			_pdst->add_row_alloc(2, nIntIndex, (const void*)&stlVnDatas.front(), (short)stlVnDatas.size());
			//file.write_asc_string("}", 0);
			//file.write_asc_line();
		}

		{// Camera Information
			float afData[3];
			int nValue;
			int nKey = 4;
			_pdst->add_row_alloc(4, nKeyIndex, (const void*)&nKey);

			file.read_asc_line();
			file.read_asc_float3(afData); // v3Forward
			_pdst->add_row_alloc(3, nFloatIndex, (const void*)afData, 3);

			file.read_asc_float3(afData); // v3Pos2D
			_pdst->add_row_alloc(3, nFloatIndex, (const void*)afData, 3);

			file.read_asc_float(afData); // fDist
			_pdst->add_row_alloc(3, nFloatIndex, (const void*)afData, 1);

			file.read_asc_integer(&nValue); // nStepCurrent
			_pdst->add_row_alloc(3, nIntIndex, (const void*)&nValue, 1);

			file.read_asc_float3(afData); // v3Offset
			_pdst->add_row_alloc(3, nFloatIndex, (const void*)afData, 3);

			file.read_asc_integer(&nValue); // nCamMode
			_pdst->add_row_alloc(3, nIntIndex, (const void*)&nValue, 1);
			file.read_asc_integer(&nValue); // nPlayerId
			_pdst->add_row_alloc(3, nIntIndex, (const void*)&nValue, 1);
		}
		file.CloseFile();
	}
	else {
		return false;
	}
	return true;
}

float _fmodf(float _fUp, float _fDn)
{
	float fV;

	fV = _fUp / _fDn;
	int nV = (int)fV;

	fV = _fUp - (float)nV * _fDn;

	return fV;
}

bool save_asc_for_etagen(void* _pdstVoid, const char* _strFilename)
{
	BaseDStructure* _pdst = (BaseDStructure*)_pdstVoid;

	BaseFile file;
	if (!file.OpenFile(_strFilename, BaseFile::OPEN_WRITE))
	{
		short nCnt = 0;
		const int* anData = NULL;
		{
			_pdst->get(0, (const void**)&anData, &nCnt, 1);// Num of Ctr info
			const char* strData;

			// Scene Object Type
			// 1 - Ball
			// 2 - Player Home
			// 3 - Player Away
			// 5 - Primitive Circle
			// 6 - Primitive Line
			// 7 - Primitive Text
			file.write_asc_string("#{ {int Backnumber, int Scene Object Type, int Model Type(0: Normal, 1: Goal Keeper), int ID, int First Frame, string Player name}, {...}, ... }", 0);
			file.write_asc_line();
			file.write_asc_integer(anData[0]);
			file.write_asc_line();
			file.write_asc_string("{", 0);
			file.write_asc_line();

			int nNumOfInfo = anData[0];
			int nMod;
			_pdst->get(0, (const void**)&anData, &nCnt, 2);// Object(Ball, Players and Primitives) configuration data
			// Player name
			for (int i = 0; i < nNumOfInfo; i++)
			{ //{ {int Backnumber, int Scene Object Type, int Model Type(0: Normal, 1: Goal Keeper), int ID, int First Frame, string Player name}, {...}, ... }
				file.write_asc_string("{", 0);
				int nStart = i * 4;
				file.write_asc_integer(anData[nStart + 0]); // backnumber				
				nMod = (int)_fmodf((float)anData[nStart + 1], 10.f);
				file.write_asc_integer(nMod); // Scene type
				file.write_asc_integer((anData[nStart + 1] - nMod) / 10); // Model
				file.write_asc_integer(anData[nStart + 2]); // ID
				file.write_asc_integer(anData[nStart + 3]); // First Frame

				_pdst->get(3, (const void**)&strData, &nCnt, i + 2);
				file.write_asc_string(strData, (int)strlen(strData) + 1);// name of players, i is for index of CtrInfo
				file.write_asc_string("}", 0);
				file.write_asc_line();
			}
			file.write_asc_string("}", 0);
			file.write_asc_line();
		}

		{
			_pdst->get(1, (const void**)&anData, &nCnt, 1);

			int nNumOfStep = *anData;
			const int* anData2 = NULL;
			file.write_asc_string("#{{ int id, float[3] position, int holde ball, int ball type, int show infor, int highlighted, float[3] v3Pos1, float[3] v3Pos2, float[3] v3Pos3 } {...} ...}", 0);
			file.write_asc_line();
			file.write_asc_integer(nNumOfStep);
			file.write_asc_line();
			file.write_asc_string("{", 0);
			file.write_asc_line();

			for (int nStep = 0; nStep < nNumOfStep; nStep++) {

				short nCnt2 = 0;
				int nNumOfObject;
				_pdst->get(1, (const void**)&anData2, &nCnt2, 2 + nStep * 2);
				nNumOfObject = *anData2;
				_pdst->get(1, (const void**)&anData, &nCnt2, 3 + nStep * 2);

				file.write_asc_integer(nNumOfObject);
				file.write_asc_line();

				for (int i = 0; i < nNumOfObject; i++) {
					file.write_asc_string("{", 0);

					// { int id, float[2] position, int holde ball, int ball type, int show infor 
					//		, int highlighted, int length ext position { float[2] v3Pos1, float[2] v3Pos2, ...} }
					// 
					int nStart = i * 12;
					file.write_asc_integer(anData[nStart + 0]); // id
					file.write_asc_float((float)anData[nStart + 1] / 1000.0f);
					file.write_asc_float((float)anData[nStart + 2] / 1000.0f); // float[2] position
					file.write_asc_integer(anData[nStart + 3]); // hold ball
					file.write_asc_integer(anData[nStart + 4]); // ball type
					int nShowInfo = 0;
					int nHighlight = anData[nStart + 5];
					if (anData[nStart + 5] > 1) {
						nShowInfo = 1;
						nHighlight -= 2;
					}
					file.write_asc_integer(nShowInfo); // show info
					file.write_asc_integer(nHighlight); // show info

					file.write_asc_integer(anData[nStart + 6]); // show info
					file.write_asc_string("{", 0);
					for (int k = 0; k < anData[nStart + 6]; k++) {
						file.write_asc_float((float)anData[nStart + 6 + k * 2 + 1] / 1000.0f);
						file.write_asc_float((float)anData[nStart + 6 + k * 2 + 2] / 1000.0f);
					}
					file.write_asc_string("}", 0);
					file.write_asc_string("}", 0);
					file.write_asc_line();
				}
			}
			file.write_asc_string("}", 0);
			file.write_asc_line();

			int seq = 2 + nNumOfStep * 2;
			short nCnt2;
			const char* strData;
			_pdst->get(1, (const void**)&anData2, &nCnt, seq);

			file.write_asc_string("#{ step, object index, comment }", 0);
			file.write_asc_line();
			file.write_asc_integer(anData2[0]);
			file.write_asc_line();
			file.write_asc_string("{", 0);
			file.write_asc_line();
			for (int i = 0; i < anData2[0]; i++) {
				file.write_asc_string("{", 0);
				if (_pdst->get(1, (const void**)&anData, &nCnt2, seq + i * 2 + 1)) //{time, index}
				{
					_pdst->get(1, (const void**)&strData, &nCnt2, seq + i * 2 + 2); // comment
					if (nCnt2 > 1)
					{
						file.write_asc_integer(anData[0]);
						file.write_asc_integer(anData[1]);
						file.write_asc_string(strData, 0);
					}
				}
				file.write_asc_string("}", 0);
				file.write_asc_line();
			}
			file.write_asc_string("}", 0);
			file.write_asc_line();
		}
		{
			// Unused backnumber
			_pdst->get(2, (const void**)&anData, &nCnt, 1);
			int nNumOfData = *anData;
			_pdst->get(2, (const void**)&anData, &nCnt, 2);

			file.write_asc_string("# length { unused backnumbers, ... } for home", 0);
			file.write_asc_line();
			file.write_asc_integer(nNumOfData);
			file.write_asc_string("{", 0);
			for (int i = 0; i < nNumOfData; i++) {
				file.write_asc_integer(anData[i]); // numbers for home
			}
			file.write_asc_string("}", 0);
			file.write_asc_line();

			_pdst->get(2, (const void**)&anData, &nCnt, 3);
			nNumOfData = *anData;
			file.write_asc_integer(nNumOfData);
			file.write_asc_string("{", 0);

			_pdst->get(2, (const void**)&anData, &nCnt, 4);
			for (int i = 0; i < nNumOfData; i++) {
				file.write_asc_integer(anData[i]); // numbers for away
			}
			file.write_asc_string("}", 0);
			file.write_asc_line();
		}

		{// Camera Information
			const float* afData;

			_pdst->get(4, (const void**)&afData, &nCnt, 1);
			file.write_asc_float3(afData); // v3Forward

			_pdst->get(4, (const void**)&afData, &nCnt, 2);
			file.write_asc_float3(afData); // v3Pos2D

			_pdst->get(4, (const void**)&afData, &nCnt, 3);
			file.write_asc_float(afData[0]); // fDist

			_pdst->get(4, (const void**)&anData, &nCnt, 4);
			file.write_asc_integer(anData[0]); // nStepCurrent

			_pdst->get(4, (const void**)&afData, &nCnt, 5);
			file.write_asc_float3(afData); // v3Offset

			_pdst->get(4, (const void**)&anData, &nCnt, 6);
			file.write_asc_integer(anData[0]); // nCamMode
			_pdst->get(4, (const void**)&anData, &nCnt, 7);
			file.write_asc_integer(anData[0]); // nPlayerId

			file.write_asc_line();
		}
		file.CloseFile();
	}
	else {
		return false;
	}
	return true;
}

void* base_file_create() {
	BaseFile* file_p;

	PT_OAlloc(file_p, BaseFile);
	return (void*)file_p;
}

bool base_file_open_u8(void* _bfile, const char* _filename_str)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	char fullpath[255];
	BaseSystem::tomulti(_filename_str, fullpath, 255);
	if (file_p->OpenFile(fullpath, BaseFile::OPEN_READ | BaseFile::OPEN_UTF8) == 0)
		return true;
	return false;
}

bool base_file_open(void* _bfile, const char* _filename_str, int _type)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	char* buff;
	size_t len = strlen(_filename_str) + 1;
	buff = PT_Alloc(char, len);
	BaseSystem::tomulti(_filename_str, buff, len);

	int param_open = BaseFile::OPEN_READ;
	switch (_type)
	{
	case 0:
		param_open = BaseFile::OPEN_READ | BaseFile::OPEN_UTF8;
		break;
	case 1:
		param_open = BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8;
		break;
	}
	if (file_p->OpenFile(buff, param_open) == 0) {
		PT_Free(buff);
		return true;
	}
	PT_Free(buff);
	return false;
}

bool base_file_asc_open(void* _bfile, const char* _strStream, int _size_n)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	if (file_p->OpenFile((void*)_strStream, _size_n) == 0)
		return true;
	return false;
}

void base_file_seperator_set(void* _bfile, const char* _strSeperator)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	file_p->set_asc_seperator(_strSeperator);
}

void base_file_delector_set(void* _bfile, const char* _strDelector)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	file_p->set_asc_deletor(_strDelector);
}

bool base_file_asc_write_line(void* _bfile)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	return file_p->write_asc_line();
}

bool base_file_asc_write_string(void* _bfile, const char* _str)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	//char* buff;
	//size_t len = strlen(_str)+1;
	//buff = PT_Alloc(char, len);
	//BaseSystem::toutf8(_str, buff, len);
	bool ret_b = file_p->write_asc_string(_str, 0);
	//PT_Free(buff);
	return ret_b;
}

bool base_file_asc_read_line(void* _bfile)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	return file_p->read_asc_line();
}

void* base_file_read(void* _bfile)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	static STLString str;

	if (file_p->Read(&str))
		return (void*)str.c_str();
	return NULL;

}

void* base_file_asc_read_string(void* _bfile)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	char* ret = PT_Alloc(char, 4096);

	if (file_p->read_asc_string(ret, 4096)) {
		PT_AFree(ret);
		return (void*)ret;
	}
	PT_Free(ret);
	return NULL;
}

bool base_file_asc_read_int(void* _bfile, int* _nValue)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	return file_p->read_asc_integer(_nValue);
}

bool base_file_asc_read_float(void* _bfile, float* _value_pf)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	return file_p->read_asc_float(_value_pf);
}

bool base_file_read_ushort(void* _bfile, unsigned short* _value)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	return file_p->Read((char*)_value, sizeof(short));
}

bool base_file_read_ulong(void* _bfile, unsigned long* _value)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	return file_p->Read((char*)_value, sizeof(long));
}

void* base_file_read_str(void* _bfile, int _size)
{
	BaseFile* file_p = (BaseFile*)_bfile;

	char* ret = PT_Alloc(char, _size + 1);

	if (file_p->Read(ret, _size))
	{
		ret[_size] = 0;
		PT_AFree(ret);
		return (void*)ret;
	}
	PT_Free(ret);
	return NULL;
}

bool base_file_read_uchar(void* _bfile, unsigned char* _value)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	return file_p->Read((char*)_value, 1);
}

bool base_file_read_double(void* _bfile, double* _value)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	return file_p->Read((char*)_value, sizeof(double));
}

bool base_file_read_ullong(void* _bfile, unsigned long long* _value)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	return file_p->Read((char*)_value, sizeof(long long));
}

bool base_file_seek(void* _bfile, unsigned long _offset)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	if (file_p->Seek((int)_offset) == BaseFile::MSG_SUCCESS)
		return true;
	return false;
}

bool base_file_delete(void* _bfile)
{
	BaseFile* file_p = (BaseFile*)_bfile;
	PT_OFree(file_p);
	return true;
}

void* base_malloc(int _size_n)
{
	return PT_Alloc(char, _size_n);
}

void base_free(void* _point)
{
	PT_Free(_point);
}

wchar_t* sys_towide(const char* _in, wchar_t* _out, int _size)
{
	STLString in_str = _in;
	STLWString wout = BaseSystem::towide(in_str);

	wcscpy_s(_out, _size, wout.c_str());
	return _out;
}

char* sys_toutf8(const wchar_t* _in, char* _out, int _size)
{
	STLWString in_str = _in;
	wprintf(_in);
	STLString out = BaseSystem::toutf8(in_str);
	printf("%s", out.c_str());

	strcpy_s(_out, _size, out.c_str());
	return _out;
}
