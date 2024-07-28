#include "stdafx.h"
#include <map>
#include <vector>
#include <algorithm>

#include <assert.h>
#include "BaseDStructure.h"
#include "BaseFile.h"
#include "BaseState.h"
#include "BaseStateSpace.h"
#include "BaseStateMain.h"
#include "BaseStateManager.h"
#include "DebugContext.h"
#include "BaseEventManager.h"


#ifdef ANDROID
#include <android/log.h>
#endif
#define APPNAME	"StateBaseLib"

std::vector<int>	BaseDStructure::sm_stlVTypeSize;
STLVString	BaseDStructure::sm_stlVTypeName;

STLMnInt					BaseDStructure::sm_defineHistory_m;
STLMnInt					BaseDStructure::sm_stlMnnEventCallType;
STLMnpFuncEventProcessor	BaseDStructure::sm_stlMnpEventProcessor;
STLVTypeDefine		BaseDStructure::sm_stlVTypedef;
STLMnnTypeDefine		BaseDStructure::sm_stlMnnTypedef;
STLMnString	BaseDStructure::sm_stlMnDebugProcessorName;
STLMnString	BaseDStructure::sm_stlMnDebugProcessorFile;
STLMnInt	BaseDStructure::sm_stlMnDebugProcessorLine;
int			BaseDStructure::sm_nDefineOriginalSize = 0;
STLVString	BaseDStructure::sm_stlVClasses;

atomic_cnt	*s_lockUse = NULL;
atomic_cnt *s_lockUpdate = NULL;

#ifndef _WIN32
#define CRITICAL_SECTION				pthread_mutex_t
#endif
CRITICAL_SECTION	*s_critical_que_section = NULL;

int s_dbg_mem_value = 0;
const int* s_db_mem = NULL;

#define VER_ADDSERIAL
#ifdef _DEBUG
#define TRACE_STATE
#endif
//#pragma warning( disable : 4996 )



BaseDStructure	BaseDStructure::sm_sample;
char BaseDStructure::sm_user[255] = "default";

BaseFile *callbackFileOpen(const char *_strFilename)
{
	BaseFile *pFile;
	PT_OAlloc(pFile, BaseFile);
	if(!pFile->OpenFile(_strFilename, BaseFile::OPEN_READ))
	{
		pFile->m_pVoid	= pFile;
		return pFile;
	}
	PT_OFree(pFile);
	return NULL;
}

void callbackFileClose(void *_pFile)
{
	if(_pFile == NULL)
		return;

	BaseFile *pFile	= (BaseFile*)_pFile;

	pFile->CloseFile();
	PT_OFree(pFile);
}

fnOpenFile BaseDStructure::sm_callbackFileOpen = callbackFileOpen;
fnCloseFile	BaseDStructure::sm_callbackFileClose	= callbackFileClose;

// add_column�̳� �����͸� �߰��ϴ� �κ��� ������ �߿����� �ʱ� ������
// �ټ� ������ �˰������� ����� �� �ִ�
// �׿� get�̳� reading�˰������� ���ɿ� ����ȭ �ž� �Ѵ�.

void BaseDStructure::static_clear()
{
    sm_stlVTypeName.clear();
    sm_stlVTypedef.clear();
    sm_stlVTypeSize.clear();
    sm_stlMnnTypedef.clear();
}

BaseDStructure::BaseDStructure()
{
	m_option	= 0;
	init();
}

BaseDStructure::~BaseDStructure()
{
	release();
}

void BaseDStructure::init_type()
{
	if (sm_stlVTypeName.size() == 0)
	{
		s_lockUse = new atomic_cnt(0);
		s_lockUpdate = new atomic_cnt(0);
		s_critical_que_section = new CRITICAL_SECTION;
		
		BraceInc inc(s_lockUse, s_lockUpdate);
		BraceUpdate::criticalInit(s_critical_que_section);
		BraceUpdate updator(s_lockUse, s_lockUpdate, s_critical_que_section);
		
		sm_stlVTypeName.resize(TYPE_MAX);
		sm_stlVTypeSize.resize(TYPE_MAX);
		//sm_stlVTypeSize[TYPE_NULL] = 0;	sm_stlVTypeName[TYPE_NULL] = "NULL";
		sm_stlVTypeSize[TYPE_BOOL2] = 0;	sm_stlVTypeName[TYPE_BOOL2] = "BOOL";
		sm_stlVTypeSize[TYPE_BYTE] = 1;	sm_stlVTypeName[TYPE_BYTE] = "BYTE";
		sm_stlVTypeSize[TYPE_SHORT] = 2;	sm_stlVTypeName[TYPE_SHORT] = "SHORT";
		sm_stlVTypeSize[TYPE_INT32] = 4;	sm_stlVTypeName[TYPE_INT32] = "INT32";
		sm_stlVTypeSize[TYPE_INT64] = 8;	sm_stlVTypeName[TYPE_INT64] = "INT64";
		sm_stlVTypeSize[TYPE_FLOAT] = 4;	sm_stlVTypeName[TYPE_FLOAT] = "FLOAT";
		sm_stlVTypeSize[TYPE_VECTOR3] = 12;	sm_stlVTypeName[TYPE_VECTOR3] = "VECTOR3";
		sm_stlVTypeSize[TYPE_STRING] = 1;	sm_stlVTypeName[TYPE_STRING] = "STRING";
		sm_stlVTypeSize[TYPE_ARRAY_BYTE] = 1;	sm_stlVTypeName[TYPE_ARRAY_BYTE] = "ARRAY_BYTE";
		sm_stlVTypeSize[TYPE_ARRAY_SHORT] = 2;	sm_stlVTypeName[TYPE_ARRAY_SHORT] = "ARRAY_SHORT";
		sm_stlVTypeSize[TYPE_ARRAY_INT32] = 4;	sm_stlVTypeName[TYPE_ARRAY_INT32] = "ARRAY_INT32";
		sm_stlVTypeSize[TYPE_ARRAY_INT64] = 8;	sm_stlVTypeName[TYPE_ARRAY_INT64] = "ARRAY_INT64";
		sm_stlVTypeSize[TYPE_ARRAY_FLOAT] = 4;	sm_stlVTypeName[TYPE_ARRAY_FLOAT] = "ARRAY_FLOAT";
		sm_stlVTypeSize[TYPE_ARRAY_VECTOR3] = 12;	sm_stlVTypeName[TYPE_ARRAY_VECTOR3] = "ARRAY_VECTOR3";
	}
}

void BaseDStructure::init()
{
	release_all_data();
	m_strName[0] = 0;
	m_strName[1] = 0;

	init_type();
}

void BaseDStructure::release_all_data()
{
	for (unsigned i = 0; i < m_stlVpAllocList.size(); i++)
		PT_Free(m_stlVpAllocList[i]);

	m_stlVpAllocList.clear();
	m_stlMnData.clear();
}

void BaseDStructure::release()
{
	release_all_data();
}

bool BaseDStructure::is_exist_event_processor(const char *_strName)
{
	int nHash;
	nHash	= UniqHashStr::get_string_hash_code(_strName);
	if(sm_stlMnpEventProcessor.find(nHash) == sm_stlMnpEventProcessor.end())
		return false;
	return true;
}

bool BaseDStructure::set_event_processor(const char *_strName, fnEventProcessor	_pfuncEventProc, const char* _file, int _line, int _nCallType)
{	
	if (sm_stlMnnTypedef.size() == 0)
		return false;
	int nHash;
	nHash	= UniqHashStr::get_string_hash_code(_strName);
	int nIndex = get_index(nHash);
	BaseDStructure::sm_stlMnpEventProcessor[nHash]	= _pfuncEventProc;
	sm_stlMnDebugProcessorFile[nHash] = _file;
	sm_stlMnDebugProcessorLine[nHash] = _line;
    g_SendMessage(LOG_MSG, "set_event_processor: %s, %d\n", _strName, nHash);
    
	
	int nFlag	= 0;
	
	if (nIndex > 0)
		nFlag = get_type_flag(nIndex) & STATE_CALLTYPE;
	
	if(nFlag)// State Editor
	{
		sm_stlMnnEventCallType[nHash]	= nFlag;
	}else{
		sm_stlMnnEventCallType[nHash]	= _nCallType;

		if(nIndex > 0)
		{
			nFlag = get_type_flag(nIndex);
			set_type_flag(nIndex, _nCallType | nFlag);
		}
	}
//#ifdef _DEBUG
	sm_stlMnDebugProcessorName[nHash]	= _strName;
//#endif
	return true;
}

typedef struct{
	char				strName[255];
	char				file[255];
	int					line;
	fnEventProcessor	fProc;
}ST_ProcessorList;
typedef std::vector<ST_ProcessorList, PT_allocator<ST_ProcessorList> >	STLVProcessorList;
STLVProcessorList	*pstlVProcessorList = NULL;

void BaseDStructure::processor_list_add(const char *_strName, fnEventProcessor _fProc, const char* _file, int _line)
{
	if(pstlVProcessorList == NULL)
		pstlVProcessorList	= new STLVProcessorList;

	ST_ProcessorList	proc;
	strcpy_s(proc.strName, 255, _strName);
	strcpy_s(proc.file, 255, _file);
	proc.line = _line;
	proc.fProc	= _fProc;
	
	g_SendMessage(LOG_MSG, "processor_list_add: %s %x\n", _strName, _fProc);
#ifdef ANDROID
	__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "---------- process_add %s:%s", _file, _strName);
#endif

	pstlVProcessorList->push_back(proc);
}

void BaseDStructure::processor_init()
{
	if (pstlVProcessorList == NULL)
		return;

	for(unsigned i=0; i<pstlVProcessorList->size(); i++)
	{
		set_event_processor(pstlVProcessorList->at(i).strName, pstlVProcessorList->at(i).fProc,
			pstlVProcessorList->at(i).file, pstlVProcessorList->at(i).line);
	}
	delete pstlVProcessorList;
	pstlVProcessorList = NULL;
}

int	BaseDStructureValue::event_process(BaseDStructureValue *_pEvent,BaseDStructureValue *_pdsvContext, int _nState, int *_pnProcessCnt)
{
	if(m_pdstBase == NULL)
		return 0;

	if((_nState & BaseDStructure::STATE_UPDATE)!= 0
		&& !m_bIsGotUpdateLink)
		return 0;

	int	nRet	= 0;
	int nCountFalse	= 0;

	bool	bResult;
	int		nCallType;
	bool inverse_rule = false;
	int		nResumeColumCnt = 0;
	for(unsigned i=0; i<m_stlVBool.size(); i++)
	{
		bResult		= m_stlVBool[i].bResult;
		nCallType	= m_stlVBool[i].nCallType;
				
		if(bResult == false)
			nCountFalse++;

		if ((nCallType & _nState) == 0)
		{	
			if (bResult == false && (nCallType & BaseDStructure::STATE_CALLTYPE) == 0)
			{
				g_SendMessage(LOG_MSG_POPUP, "function has no call type error(%s)", m_stlVDebugProcessorName[i].c_str());
			}
#ifdef VER_ADDSERIAL
			if ((_nState & BaseDStructure::STATE_LINK_COMPLETE) == 0
				|| (nCallType & BaseDStructure::STATE_LINK_APPEND) == 0
				)// jtseo 2020/06/10
			{	
				continue;
			}
			if (m_stlVBool[i].nHash == HASH_STATE(BaseFuncAddInverse_nIf))
			{
				inverse_rule = !inverse_rule;
				continue;
			}
#else
				continue;
#endif
			
			int x = 0;
		}

		if (nCallType & BaseDStructure::STATE_RESUME)
			nResumeColumCnt++;

		if(bResult && // Add or Update by OJ 2012-07-19 ���� 5:33:14
			(nCallType & BaseDStructure::STATE_EVENT) != 0 
			&& (_nState & BaseDStructure::STATE_RESUME) != 0
			&& (_nState & BaseDStructure::STATE_SUSPEND) != 0
			)
		{
			bResult	= false;
			nCountFalse++;// Add or Update by OJ 2012-07-19 ���� 5:32:45
			m_stlVBool[i].bResult	= bResult;// Add or Update by OJ 2012-07-19 ���� 5:34:27
		}

		if(bResult == false 
			|| _nState == BaseDStructure::STATE_LINK_COMPLETE
			|| _nState == BaseDStructure::STATE_END
			|| _nState == BaseDStructure::STATE_RESUME
			|| _nState == BaseDStructure::STATE_SUSPEND
			|| _nState == BaseDStructure::STATE_START)
		{
			int result = event_process(i, _pEvent, _pdsvContext, _nState, _pnProcessCnt);

#ifdef VER_ADDSERIAL
			if ((nCallType & BaseDStructure::STATE_LINK_APPEND) != 0) // jtseo 2020/06/10
			{
				if (inverse_rule)
				{// Not������ ó���Ѵ�
					if (result == 0)
						result = 1;
					else
						result = 0;
				}

				if (result == 0)
				{
					BaseState* pState = (BaseState*)m_pVoidParam;
					int key_state = 0, serial = 0, key_main = 0;
					if (pState) {
						key_main = pState->get_key_main();
						key_state = pState->get_key();
						serial = pState->get_serial();
					}
					else {
						serial = 0;
					}
					logger_send(TYPE_Logger_fail, key_main, key_state, serial, get_key(), pState->group_id_get(), _pEvent ? _pEvent->serial(): 0); // send log for link fail
					return 0;
				}
			}
#endif
			if (result == 1)
				nRet = 1;
		}
	}

	if (_nState == BaseDStructure::STATE_RESUME
		|| _nState == BaseDStructure::STATE_END
		|| _nState == BaseDStructure::STATE_SUSPEND) {

	}
	else {
		if (m_stlVBool.size() == 0)
			nRet = 1;

		if (nCountFalse == 0)
			nRet = 1;
	}
	return nRet;
}

bool BaseDStructureValue::event_result_check(unsigned _nIndex)
{
	if(_nIndex > m_stlVBool.size())
		return false;
	return m_stlVBool[_nIndex].bResult;
}

int BaseDStructureValue::event_process(int _nIndex, BaseDStructureValue *_pEvent, BaseDStructureValue *_pdsvContext, int _nState, int *_pnProcessCnt)
{
	int showLog = 1;
	int nRet	= 0;
	int nCallType;

	fnEventProcessor	fnProcessor;

	m_function_hash = m_stlVBool[_nIndex].nHash;
	m_nSequenceCurrent		= m_stlVBool[_nIndex].nSequence;
	m_nSequenceCurrentLocal	= m_stlVBool[_nIndex].nSequenceLocal;
	nCallType				= m_stlVBool[_nIndex].nCallType;
	fnProcessor				= m_stlVBool[_nIndex].fnProcessor;
	m_nBoolIndexCurrent		= _nIndex;
	
	BaseEventManager::break_befor_true(_pEvent);

	if(fnProcessor == NULL)// Add or Update by OJ 2012-09-10 ���� 6:53:31
		return 0;

	if(_pnProcessCnt)
		(*_pnProcessCnt)++;
#ifdef _DEBUG	
	BaseStateManager::get_manager()->dualupdate_block_reg_colum(m_stlVDebugProcessorName[_nIndex].c_str());
#endif

#ifdef TRACE_STATE
	BaseState			*pState = (BaseState*)m_pVoidParam;
	BaseStateMain		*pMain = pState->get_state_main();
	static int s_serial = 0;
	s_serial++;

	if (pState->get_space()->get_manager()->m_nDebugBlockCastMessage == 0 && !md_LogHide) {
		char strAction[128];//, strBuffer[1024];
		switch (_nState)
		{
		case BaseDStructure::STATE_UPDATE:	strcpy_s(strAction, 128, "LUPDATE");	break;
		case BaseDStructure::STATE_EVENT:	strcpy_s(strAction, 128, "LEVENT");		break;
		case BaseDStructure::STATE_START:	strcpy_s(strAction, 128, "LSTART");		break;
		case BaseDStructure::STATE_END:		strcpy_s(strAction, 128, "LEND");		break;
		case BaseDStructure::STATE_SUSPEND:	strcpy_s(strAction, 128, "LSUSPEND");	break;
		case BaseDStructure::STATE_RESUME:	strcpy_s(strAction, 128, "LRESUME");	break;
		case BaseDStructure::STATE_LINK_COMPLETE:	strcpy_s(strAction, 128, "LCOMPT");	break;
		default:							strcpy_s(strAction, 128, "LETC");		break;
		}

		if (_nState != BaseDStructure::STATE_UPDATE)
		{
			char strBuf[1024] = "-";
			if (_pEvent) {
				int nTemp = 0;
				const int *pnValue = 0;
				_pEvent->get(_pEvent->get_key(), (const void**)&pnValue);
				if (pnValue == 0)
					pnValue = &nTemp;

				showLog = g_SendMessage(LOG_MSG,
					"SMain:%s %s SActive:%s Serial:%d SLink:%s CMT:%s SCol:%s LEvt:%d EVal:%d Col:%s(%d)",
					pMain->get_name().c_str(), pState->group_id_get_string(strBuf, 1024),
					pState->get_name().c_str(), pState->get_serial(), m_strDebugName, m_strDebugComment,
					m_stlVDebugProcessorName[_nIndex].c_str(), _pEvent->get_key(), *pnValue, strAction, s_serial
				);
			}
			else {
				showLog = g_SendMessage(LOG_MSG,
					"SMain:%s %s SActive:%s Serial:%d SLink:%s CMT:%s SCol:%s Col:%s(%d)",
					pMain->get_name().c_str(), pState->group_id_get_string(strBuf, 1024),
					pState->get_name().c_str(), pState->get_serial(), m_strDebugName, m_strDebugComment,
					m_stlVDebugProcessorName[_nIndex].c_str(), strAction, s_serial
				);
			}
		}
	}
#endif
	m_loglink_p = this;
	nRet	= fnProcessor(this, _pEvent, _pdsvContext, _nState);
#ifdef _DEBUG
	BaseStateManager::get_manager()->dualupdate_block_reg_colum(NULL);
#endif

	if((nCallType & BaseDStructure::STATE_LINK_INVERSE) != 0)
	{// Not������ ó���Ѵ�
		if(nRet == 0)
			nRet	= 1;
		else
			nRet	= 0;
	}
#ifdef TRACE_STATE
    if (pState->get_space()->get_manager()->m_nDebugBlockCastMessage == 0
        && _nState != BaseDStructure::STATE_UPDATE
		&& !md_LogHide
		&& showLog == 1)
    {
        g_SendMessage(LOG_MSG, "(%d)CRet:%d\n", s_serial, nRet);
    }
#endif

	if(nRet==1)
	{
		m_stlVBool[_nIndex].bResult	= true;
		m_bUpdated		= true;
	}

    if (g_logger()){
		logger_hand_result(m_nSequenceCurrent, nRet);
    }
	return nRet;
}

int BaseDStructureValue::event_process(BaseDStructureValue *_pEvent, BaseDStructureValue *_pdsvContext, int *_pnProcessCnt)
{
	if(m_pdstBase == NULL)
		return 0;

	if(m_bComplete)
		return 1;

	int nKeyEvent = _pEvent->get_key();
	int	nRet	= 0;

	int	nHash;
	for(unsigned i=0; i<m_stlVBool.size(); i++)
	{
		nHash	= m_stlVBool[i].nHash;
		if(nKeyEvent != nHash)
			continue;

		return event_process(i, _pEvent, _pdsvContext, BaseDStructure::STATE_EVENT, _pnProcessCnt);
	}

	return nRet;
}

void BaseDStructureValue::complete_make()
{
	m_bComplete = true;
}

bool BaseDStructureValue::is_complete(BaseDStructureValue *_pEvent, BaseDStructureValue *_pdsvContext)
{	
	bool bRetFalse = false;

	if(m_bUpdated == false)
		return m_bComplete;

	m_bUpdated	= false;
	bool bResult;
	int	nCallType;
	fnEventProcessor	fnProcessor;
	bool inverse_rule = false;

#ifdef _DEBUG
	int link = get_key();
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeoutlink0");
#endif
	for(unsigned i=0; i<m_stlVBool.size(); i++)
	{
		bResult		= m_stlVBool[i].bResult;
		nCallType	= m_stlVBool[i].nCallType;
#ifdef _DEBUG
	link = get_key();
	if(link ==-1453171329)
		g_SendMessage(LOG_MSG, "------ link for the timeoutlink1-%d %s", i, m_stlVDebugProcessorName[i].c_str());
#endif
		if(bResult == false)
		{
			return bRetFalse;
		}else{
#ifdef VER_ADDSERIAL
			continue;// jtseo 2020/06/10
#endif
			if(nCallType & BaseDStructure::STATE_LINK_APPEND)
			{// type of APPEND called in last check.

				fnProcessor				= m_stlVBool[i].fnProcessor;
				m_function_hash		= m_stlVBool[i].nHash;
				m_nSequenceCurrent		= m_stlVBool[i].nSequence;
				m_nSequenceCurrentLocal	= m_stlVBool[i].nSequenceLocal;
				m_nBoolIndexCurrent		= i;

#ifdef _DEBUG
				BaseStateManager::get_manager()->dualupdate_block_reg_colum(m_stlVDebugProcessorName[i].c_str());
#endif
				int nRet	= fnProcessor(this, _pEvent, _pdsvContext, BaseDStructure::STATE_LINK_APPEND);
#ifdef _DEBUG
				BaseStateManager::get_manager()->dualupdate_block_reg_colum(NULL);
#endif

				if(inverse_rule)
				{// Not������ ó���Ѵ�
					if(nRet == 0)
						nRet	= 1;
					else
						nRet	= 0;
				}
				BaseState			*pState = NULL;
#ifdef TRACE_STATE
				
				pState	= (BaseState*)m_pVoidParam;
				//BaseStateMain		*pMain	= pState->get_state_main();
				if(pState->get_space()->get_manager()->m_nDebugBlockCastMessage == 0
					&& !md_LogHide){
					char strAction[128];//, strBuffer[1024];
					strcpy_s(strAction, 128, "LAPPEND");

					pState	= (BaseState*)m_pVoidParam;
					BaseStateMain		*pMain	= pState->get_state_main();
                    char strBuf[1024] = "";
                    if(_pEvent)
					{
						int nTemp = 0;
						const int *pnValue = 0;
						_pEvent->get(_pEvent->get_key(), (const void**)&pnValue);
						if (pnValue == 0)
							pnValue = &nTemp;
						g_SendMessage(LOG_MSG, 
							"SMain:%s %s SActive:%s Serial:%d SLink:%s CMT:%s SCol:%s LEvt:%d EVal:%d Col:%s CRet:%d\n",
							pMain->get_name().c_str(), pState->group_id_get_string(strBuf, 1024),
							pState->get_name().c_str(), pState->get_serial(), m_strDebugName, m_strDebugComment,
							m_stlVDebugProcessorName[i].c_str(), _pEvent->get_key(), *pnValue, strAction, nRet
							);
					}else{
						g_SendMessage(LOG_MSG, 
							"SMain:%s %s SActive:%s Serial:%d SLink:%s CMT:%s SCol:%s Col:%s CRet:%d\n",
							pMain->get_name().c_str(), pState->group_id_get_string(strBuf, 1024),
							pState->get_name().c_str(), pState->get_serial(), m_strDebugName, m_strDebugComment,
							m_stlVDebugProcessorName[i].c_str(), strAction, nRet
							);
					}
				}	
#endif

                if (g_logger()){
					logger_hand_result(m_nSequenceCurrent, nRet);
                }
				if (nRet != 1)
				{
					pState = (BaseState*)m_pVoidParam;
					int key_state = 0, serial = 0, key_main = 0;
					if (pState) {
						key_main = pState->get_key_main();
						key_state = pState->get_key();
						serial = pState->get_serial();
					}
					else {
						serial = 0;
					}
					logger_send(TYPE_Logger_fail, key_main, key_state, serial, get_key(), pState->group_id_get(), _pEvent ? _pEvent->serial() : 0 ); // send log for link fail
					return bRetFalse;
				}
			}
		}
	}
	
#ifdef VER_ADDSERIAL
#else
	m_bComplete	= !bRetFalse; //true // jtseo 2020/06/10
#endif
	return !bRetFalse;
}

const BaseDStructure* BaseDStructureValue::get_base() const
{
	return m_pdstBase;
}

short	BaseDStructure::get_size_type_(bbyte _nType)
{
    if (_nType < 0 || sm_stlVTypeSize.size() <= _nType)
        return -1;

	return sm_stlVTypeSize[_nType];
}

int		BaseDStructure::get_type_flag(int _nIndex)
{
    if (_nIndex < 0 || (int)type_length() <= _nIndex)
        return -1;

	return type_get(_nIndex).nFlag;
}

const char *BaseDStructure::get_type_comment(int _nIndex) const
{
    if (_nIndex < 0 || (int)type_length() <= _nIndex)
        return NULL;
	
	char *ret = PT_Alloc(char, 255);
	strcpy_s(ret, 255, type_get(_nIndex).strComment);
	PT_AFree(ret);
	return ret;
}

const char *BaseDStructure::get_type_enum_state(int _nIndex) const
{
    if (_nIndex < 0 || (int)type_length() <= _nIndex)
        return NULL;
	char *buf = PT_Alloc(char, 255);
	PT_AFree(buf);
	strcpy_s(buf, 255, type_get(_nIndex).strEnumState);
	return buf;
}

const char *BaseDStructure::get_type_name(int _nIndex)
{
	if (_nIndex < 0 || (int)type_length() <= _nIndex)
		return NULL;

	char *ret = PT_Alloc(char, 255);
	strcpy_s(ret, 255, type_get(_nIndex).strName);
	PT_AFree(ret);
	
	return ret;
}

EDstType BaseDStructure::get_type(int _nIndex)
{
    if (_nIndex < 0 || (int)type_length() <= _nIndex)
        return TYPE_NULL;

    return (EDstType)type_get(_nIndex).nType;
}

int	BaseDStructure::get_type_size(int _nIndex)
{
    if (_nIndex < 0 || (int)type_length() <= _nIndex)
        return -1;

    B_ASSERT(type_get(_nIndex).nSize);
	return (int)type_get(_nIndex).nSize;
}

int	BaseDStructure::add_column(int _nFlag, int _nColumnHash, bbyte _nType, short _nSize, const char *_strState, const char *_strComment, const char *_strColumName)
{
	STypeDefine* define_p = NULL;
	short index;
	STypeDefine define;
	define_p = &define;
	bool overwrite = false;
	if(type_find(_nColumnHash, &index))
	{
		overwrite = true;
		//define_p = &type_get(index);
		//g_SendMessage(LOG_MSG_POPUP, "'%s' already has a colum, please check '%s.define' file", _strName, m_strName);
		//return 0;
	}
	STypeDefine &tDefine = *define_p;
	
	tDefine.nFlag = _nFlag;
	tDefine.nHash = _nColumnHash;
	strcpy_s(tDefine.strName, 255, _strColumName);
	if (_strState)
		strcpy_s(tDefine.strEnumState, 255, _strState);
	else
		tDefine.strEnumState[0] = 0;

	if (_strComment)
		strcpy_s(tDefine.strComment, 255, _strComment);
	else
		tDefine.strComment[0] = 0;

	tDefine.nType = _nType;

	if (_nSize == 0)
		_nSize = get_size_type_(_nType);
	tDefine.nSize = _nSize;

	if (!overwrite)
	{
		type_add(_nColumnHash, tDefine);
	}else{
		type_mod(index, tDefine);
	}

	return _nColumnHash;
}

void BaseDStructure::type_add(int _hash, STypeDefine _type)
{
	BraceInc inc(s_lockUse, s_lockUpdate);
	BraceUpdate updater(s_lockUse, s_lockUpdate, s_critical_que_section);

	//marker
	_type.nIndex = (short)sm_stlVTypedef.size();
	sm_stlVTypedef.push_back(_type);
	sm_stlMnnTypedef[_hash] = _type.nIndex;
}

void BaseDStructure::type_mod(int _idx, STypeDefine _type)
{
	BraceInc inc(s_lockUse, s_lockUpdate);
	sm_stlVTypedef[_idx] = _type;
}

int	BaseDStructure::add_column(int _nFlag, const char *_strName, bbyte _nType, short _nSize, const char *_strState, const char *_strComment)
{
	return add_column(_nFlag, STRTOHASH(_strName), _nType, _nSize, _strState, _strComment, _strName);
}

bool	BaseDStructure::is_define(const char *_strName)
{
	bool	bRet	= false;
	char strFilename[255];
	strcpy_s(strFilename, 255, _strName);
	char *strName = strrchr(strFilename, SLASH_C);
	if(strName)
		*(strName+1) = NULL;
	else
		*strFilename = NULL;
	strcat_s(strFilename, 255, "Basedefine.define");

	BaseFile	*pfile;
	pfile	= sm_callbackFileOpen(strFilename);
	if(pfile)
	{
		bRet	= true;
		sm_callbackFileClose(pfile);
	}
	return bRet;
}

const char *BaseDStructure::get_name() const
{
	return m_strName;
}

const char *BaseDStructure::get_filename() const
{
    const char *strPost;
    strPost	= strrchr(m_strName, SLASH_C);
    if(strPost)
        strPost++;
    else
        strPost	= m_strName;

    return strPost;
}

int		BaseDStructure::get_key() const
{
	return m_nKey;
}

void	BaseDStructure::set_name(const char *_strName)
{
	strcpy_s(m_strName, 255, _strName);
	const char *strPost;
	strPost	= strrchr(_strName, SLASH_C);
	if(strPost)
		strPost++;
	else
		strPost	= _strName;

	m_nKey	= UniqHashStr::get_string_hash_code(strPost);
}


bool	BaseDStructure::load(const char *_strName, int _bType)
{
	bool	bRet = true;

	if (_strName != NULL)
	{
		const char *strPost;
		strPost = strrchr(_strName, SLASH_C);
		if (strPost)
			strPost++;
		else
			strPost = _strName;

		m_nKey = UniqHashStr::get_string_hash_code(strPost);

		strcpy_s(m_strName, 255, _strName);
	}
	
	bRet	= load_define_(m_strName, _bType);
	if(!bRet)
		g_SendMessage(LOG_MSG, "Try to load %s bRet = %d", m_strName, bRet?1:0);
	if(!bRet)
		return bRet;
	bRet	= load_index_(m_strName, 0);
	if(!bRet)
		return bRet;

	if (_bType == 0) // normal application
	{
		//bRet = load_data_asc_(m_strName);
		bRet = load_data_(m_strName);
		if (!bRet)
		{
			bRet = load_data_asc_(m_strName);
			//bRet = load_data_(m_strName);
			if (bRet)
				bRet = save_data_(m_strName);
			//	bRet = save_data_asc_(m_strName);
		}
	}
	else if (_bType == 3) { // load in Editor
		bRet = load_data_asc_(m_strName);
		if (!bRet)
		{
			bRet = load_data_(m_strName);
			if (bRet)
				save_data_asc_(m_strName);
		}
		else {
			save_data_(m_strName);
			//STLString filename = m_strName;
			//SPtDateTime dtIni, dtDat;
			//dtIni  = BaseSystem::file_datetime_get();
		}
	}else {
		bRet	= load_data_asc_(m_strName);
	}

	if(!bRet)
		return bRet;
	return bRet;
}

PT_OPTCPP(BaseDStructureValue)
PT_OPTCPP(BaseDStructure)

bool BaseDStructure::load_define2_(BaseFile *_pFileDefine)
{


	char strBuf[255];
	int nCount, nValue;//, nBoolBit;
	_pFileDefine->read_asc_line();
	_pFileDefine->read_asc_string(strBuf, 255);
	_pFileDefine->read_asc_line();// size of bool flag
	_pFileDefine->read_asc_string(strBuf, 255);
	_pFileDefine->read_asc_integer(&nValue);
	_pFileDefine->read_asc_line();// number of data file

	nCount	= 0;

	while(_pFileDefine->read_asc_line())
	{
		int nFlag	= 0;
		char strName[128];
		bbyte	nType	= 0;
		_pFileDefine->read_asc_string(strName, 128);
		
		nFlag	= get_flag(strName);
		_pFileDefine->read_asc_string(strName, 128);

		if(strlen(strName) < 1)
			break;

		_pFileDefine->read_asc_string(strBuf, 255);
		nType	= get_type_by_name_(strBuf);
		_pFileDefine->read_asc_string(strBuf, 255);
		char strComment[255];
		_pFileDefine->read_asc_string(strComment, 255);

		if(nType != TYPE_BOOL2)
		{
			add_column(nFlag, strName, nType, 0, strBuf, strComment);
			nCount++;
		}

	}

	const char *astrReserve[] = {
		"BaseIdentifierParent_n",
		"BaseIdentifierChilds_an",
		"SelectorSerial_nV",
		"BaseValueTimeStart",
		"BaseValueIdentify",
		0
	};

	int nCnt=0;
	while(astrReserve[nCnt])
	{
		int nHash = STRTOHASH(astrReserve[nCnt]);
		if (get_index(nHash) == -1)
		{
			add_column(BaseDStructure::VALUE_CONTEXT, astrReserve[nCnt], TYPE_INT32, sizeof(int));
			nCount++;
		}
		nCnt++;
	}

	sm_nDefineOriginalSize = (int)type_length();

	return true;
}

bool	BaseDStructure::load_define_(BaseFile *_pFileDefine)
{	
	return load_define2_(_pFileDefine);
}

bool	BaseDStructure::load_define_append(const char *_strName)
{
	bool	bRet	= false;
	char strFilename[255];
	strcpy_s(strFilename, 255, _strName);
	strcat_s(strFilename, 255, ".define");

	BaseFile	*pfile;
	pfile	= sm_callbackFileOpen(strFilename);
	if(pfile)
	{
		bRet	= load_define2_(pfile);
		sm_callbackFileClose(pfile);
	}
	return bRet;
}

void BaseDStructure::classesSet(const char *_list)
{
	BaseFile::paser_list_seperate(_list, &sm_stlVClasses, ",");
}

bool	BaseDStructure::load_define_(const char *_strFilename, int _type)
{
    if(_type != 2 && !sm_stlMnnTypedef.empty())
        return true;
	bool	bRet	= false;
	char strFilename[255];
	char *strName;
	strcpy_s(strFilename, 255, _strFilename);

	strName = strrchr(strFilename, '\\');
	if(strName)
		*(strName+1) = NULL;
	else{
		strName = strrchr(strFilename, '/');

		if (strName)
			*(strName + 1) = NULL;
		else
			*strFilename = NULL;
	}

	char history[255], statepath[255];
    strcpy_s(statepath, 255, strFilename);
	strcpy_s(history, 255, strFilename);

	strcat_s(strFilename, 255, "Basedefine.define");
	strcat_s(history, 255, "BasedefineHistory.define");
	load_define_history(history);

	BaseFile	*pfile;
	pfile	= sm_callbackFileOpen(strFilename);
	if(pfile)
	{
		pfile->set_asc_seperator("\t");
		pfile->set_asc_deletor("");
		bRet	= load_define_(pfile);
		sm_callbackFileClose(pfile);
        
        char stateBase[255], buf[255];
        strcpy_s(strFilename, 255, statepath);
        strcat_s(strFilename, 255, "BaseStateClasses.define");
        pfile = sm_callbackFileOpen(strFilename);
        if(pfile)
        {
            pfile->read_asc_line();
            pfile->read_asc_string(buf, 255);
			strcpy_s(stateBase, 255, statepath);
			strcat_s(stateBase, 255, buf);
			strcat_s(stateBase, 255, ".define");
            while(pfile->read_asc_line())
            {
                pfile->read_asc_string(buf, 255);
                bRet = load_statefuncclass(stateBase, statepath, buf);
                if(!bRet)
                    return false;
            }
            
            sm_callbackFileClose(pfile);
        }
	}else{
		g_SendMessage( LOG_MSG, "fail to open define:%d %s", _type, _strFilename);
	}
	return bRet;
}

bool BaseDStructure::load_statefuncclass(const char *_basefile, const char *_path, const char *_filename)
{
    char buf[255];

	strcpy_s(buf, 255, _path);
    strcat_s(buf, 255, _filename);
    strcat_s(buf, 255, ".define");
    BaseFile *pfile, *pfilebase;
    pfile = sm_callbackFileOpen(buf);
    pfilebase = sm_callbackFileOpen(_basefile);
    if(pfile)
    {
        pfilebase->set_asc_seperator("\t");
        pfilebase->set_asc_deletor("");
        pfile->set_asc_seperator("\t");
        pfile->set_asc_deletor("");
        load_statefuncclass_(pfilebase, _filename);
        load_statefuncclass_(pfile, _filename);
        sm_callbackFileClose(pfilebase);
        sm_callbackFileClose(pfile);
    }
    return true;
}

bool BaseDStructure::load_statefuncclass_(BaseFile *_pfile, const char *_classname)
{
    char buf[255];
    _pfile->read_asc_line(); // skip filepath
    while(_pfile->read_asc_line())
    {
        int nFlag    = 0;
        char strName[128];
		bbyte    nType    = 0;
        _pfile->read_asc_string(buf, 255);
        nFlag    = get_flag(buf);
        _pfile->read_asc_string(buf, 128);
		strcpy_s(strName, 128, _classname);
		strcat_s(strName, 128, ".");
		strcat_s(strName, 128, buf);

        if(strlen(strName) < 1)
            break;

        _pfile->read_asc_string(buf, 255);
        nType    = get_type_by_name_(buf);
        _pfile->read_asc_string(buf, 255);
        char strComment[255];
        _pfile->read_asc_string(strComment, 255);

        if(nType != TYPE_BOOL2)
            add_column(nFlag, strName, nType, 0, buf, strComment);
    }
    return true;
}

bool BaseDStructure::load_define_history(const char *_history)
{
	BaseFile* file;

	sm_defineHistory_m.clear();
	file = sm_callbackFileOpen(_history);

	char buff[255];
	int hash1;// , hash2;
	if (file)
	{
		while (file->read_asc_line())
		{
			if (!file->read_asc_string(buff, 255, true))
				break;
			hash1 = STRTOHASH(buff);
			file->read_asc_string(buff, 255, true);

			STLMnInt::iterator it;
			it = sm_defineHistory_m.find(hash1);
			B_ASSERT(it == sm_defineHistory_m.end());

			sm_defineHistory_m[hash1] = STRTOHASH(buff);
		}
		sm_callbackFileClose(file);
		return true;
	}
	return false;
}

bool	BaseDStructure::load_index_(const char *_strFilename, short _nIndex)
{
	return true;
}

bool	BaseDStructure::load_data_asc_(const char *_strFilename)
{
	bool	bRet	= false;
	char strFilename[255];
	strcpy_s(strFilename, 255, _strFilename);
	strcat_s(strFilename, 255, ".ini");

	g_SendMessage(LOG_MSG, "asc loading: %s", _strFilename);
	BaseFile	file;
	if(!file.OpenFile(strFilename, BaseFile::OPEN_READ | BaseFile::OPEN_ASC))
	{
		release_all_data();
		//UINT32	nCnt;
		//file.read_bin_unsigned_long(&nCnt);
		int buff_max = 40960;
		bbyte* buffer = PT_Alloc(bbyte, buff_max);
		char name_brace[255];
		int cnt = 0;
		//short nCount;
		while(1)
		{
			cnt++;
			//if (!file.read_bin_short(&nCount))
			//	break;
			int	nKey;
			if (!file.read_brace_start(name_brace, 255))
				break;

			bool first = true;
			//for(int j=0; j<nCount; j++)
			while(file.read_brace_start(name_brace, 255))
			{
				bbyte	nType;
				short	nIndex;
				if(first)
					nIndex	= 0;
				else
					file.read_bin_short(&nIndex);

				int nHash;
				short	nSize = 0;
				bool	bVariable=false;
				
				B_ASSERT(nIndex != -1);
				nHash = STRTOHASH(name_brace);
				int idx = get_index(nHash);
				B_ASSERT(idx!=-1);
				if (idx == -1)
					g_SendMessage(LOG_MSG_POPUP, "Faile to find func '%s'. it should fixed", name_brace);
				nIndex = idx;
				nType = get_type(nIndex);
				if (nType >= TYPE_STRING)
					bVariable = true;
				else
					nSize = get_size_type_(nType);
				
				bbyte* pData = NULL;
				bbyte* pData2 = NULL;

				if (!bVariable)
				{
					pData = PT_Alloc(bbyte, nSize);
					m_stlVpAllocList.push_back(pData);
					pData2 = pData;
				}

				STLVInt int_a;
				short columnCnt = 0;
				if (nType == TYPE_STRING) {
					nSize = file.read_bin_string((char*)buffer, buff_max);
					nSize++;
				} else if (nType == TYPE_ARRAY_INT32) {
					//for (int j = 0; j<(nSize/4); j++)
					int nValue = 0;
					while (file.read_bin_sinteger(&nValue))
					{
						int_a.push_back(nValue);
					}
				} else {
					if(nType == TYPE_INT32)
						file.read_bin_sinteger((int*)pData2);
					else {
						if (bVariable)
							nSize = file.Read(buffer, nSize);// editing point for indexing : pointing by OJ 2010-08
						else
							file.Read(pData2, nSize);
					}
				}

				if (bVariable)
				{
					if(int_a.size() > 0)
						nSize = (short)int_a.size();
					if (nType == TYPE_ARRAY_INT32)
						nSize = nSize * 4;
					nSize += 2;

					pData = PT_Alloc(bbyte, nSize);
					m_stlVpAllocList.push_back(pData);
					pData2 = pData;

					pData2 += 2;
					nSize -= 2;
					BaseSystem::memcpy(pData, &nSize, sizeof(short));
					if(int_a.size()>0)
						BaseSystem::memcpy(pData2, &int_a[0], nSize);
					else
						BaseSystem::memcpy(pData2, buffer, nSize);
				}

				if(first)
					BaseSystem::memcpy(&nKey, pData, sizeof(int));
				first = false;

				if(nIndex >= 0 && (int) type_length() > nIndex)
				{
					int nHash2	= type_get(nIndex).nHash;
					int nNewIndex	= get_index(nHash2);
					if(nNewIndex >= 0){
						add_row_(&m_stlMnData[nKey], nNewIndex, pData);
					}else if((int)type_length() > nIndex
						&& type_get(nIndex).nType	== nType){
							add_row_(&m_stlMnData[nKey], nIndex, pData);
					}
				}else if(nType == TYPE_BOOL2){
					//set_(&m_stlMnData[nKey], nIndex, pData);
				}

				if(nType != TYPE_ARRAY_INT32) // array32 is already passed '}' (brace_end)
					file.read_brace_end();
			}
			
			update_linktype_(&m_stlMnData[nKey]);

			//file.read_brace_end();
		}

		PT_Free(buffer);
		bRet	= true;
		file.CloseFile();
	}
	return bRet;
}

int	BaseDStructure::get_flag(const char *_strFlag)
{
	int nRetFlag	= 0;

	BaseFile parse;
	parse.set_asc_seperator("_");
	parse.OpenFile((void*)_strFlag, (UINT32)strlen(_strFlag));
	parse.read_asc_line();

	char strBuf[255];
	while(parse.read_asc_string(strBuf, 255))
	{
		if(strcmp(strBuf, "T") == 0)		nRetFlag |= VALUE_CONTEXT;
		if(strcmp(strBuf, "SU") == 0)		nRetFlag |= STATE_UPDATE;
		if(strcmp(strBuf, "SE") == 0)		nRetFlag |= STATE_EVENT;
		if(strcmp(strBuf, "SS") == 0)		nRetFlag |= STATE_START | STATE_LINK_COMPLETE;
		if(strcmp(strBuf, "SD") == 0)		nRetFlag |= STATE_END;
		if(strcmp(strBuf, "SP") == 0)		nRetFlag |= STATE_SUSPEND;
		if(strcmp(strBuf, "SR") == 0)		nRetFlag |= STATE_RESUME;
		if(strcmp(strBuf, "LC") == 0)		nRetFlag |= STATE_LINK_COMPLETE | STATE_START;
		if(strcmp(strBuf, "LS") == 0)		nRetFlag |= STATE_LINK_START;
		if(strcmp(strBuf, "LA") == 0)		nRetFlag |= STATE_LINK_APPEND;
		if(strcmp(strBuf, "IN") == 0)		nRetFlag |= STATE_LINK_INVERSE;
	}

	if((nRetFlag & VALUE_CONTEXT) == 0)
		nRetFlag	|= LINK_LOCAL;
	return nRetFlag;
}

#pragma warning( disable : 4996 )

void	BaseDStructure::get_flag(char *_strFlag, int _nFlag)
{
	*_strFlag = NULL;
	
	if((_nFlag & VALUE_CONTEXT) != 0)	strcat(_strFlag, "T");
	else strcat(_strFlag, "L");
	if((_nFlag & STATE_UPDATE) != 0)	strcat(_strFlag, "_SU");
	if((_nFlag & STATE_EVENT) != 0)		strcat(_strFlag, "_SE");
	if((_nFlag & STATE_START) != 0)		strcat(_strFlag, "_SS");
	if((_nFlag & STATE_END) != 0)		strcat(_strFlag, "_SD");
	if((_nFlag & STATE_SUSPEND) != 0)	strcat(_strFlag, "_SP");
	if((_nFlag & STATE_RESUME) != 0)	strcat(_strFlag, "_SR");
	if((_nFlag & STATE_LINK_COMPLETE) != 0)	strcat(_strFlag, "_LC");
	if((_nFlag & STATE_LINK_START) != 0)	strcat(_strFlag, "_LS");
	if((_nFlag & STATE_LINK_APPEND) != 0)	strcat(_strFlag, "_LA");
	if((_nFlag & STATE_LINK_INVERSE) != 0)	strcat(_strFlag, "_IN");
}

bool	BaseDStructure::set_type_flag(int _nIndex, int _nFlag)
{
	BraceInc(s_lockUse, s_lockUpdate);
	if(_nIndex < 0 || _nIndex >= sm_stlVTypedef.size())
		return false;

	sm_stlVTypedef[_nIndex].nFlag = _nFlag;
	return true;
}

bool	BaseDStructure::set_type_enum(int _nIndex, const char *_strEnum)
{
	if(_nIndex < 0 || _nIndex >= (int)type_length())
		return false;

	strcpy_s(type_get(_nIndex).strEnumState, 255, _strEnum);
	return true;
}
bool	BaseDStructure::set_type_comment(int _nIndex, const char *_strComment)
{
	if(_nIndex < 0 || _nIndex >= (int)type_length())
		return false;

	strcpy_s(type_get(_nIndex).strComment, 255, _strComment);
	return true;
}

bool	BaseDStructure::load_data_(BaseFile *_pFileData)
{
	STLVInt stlVHashLocal, stlVTypeLocal, stlVSizeLocal;
	release_all_data();
	unsigned long	nCnt;
	{// load for variable define data
		_pFileData->read_bin_unsigned_long(&nCnt);
        if(nCnt > 10000 || nCnt < 0)
            return false;
        
		for(unsigned i=0; i<nCnt; i++)
		{
			int nHash;
			_pFileData->read_bin_integer(&nHash);
			stlVHashLocal.push_back(nHash);
			bbyte nType;
			_pFileData->read_bin_byte(&nType);
			stlVTypeLocal.push_back(nType);
			short nSize;
			_pFileData->read_bin_short(&nSize);
			stlVSizeLocal.push_back(nSize);
		}
	}
	_pFileData->read_bin_unsigned_long(&nCnt);
	for(unsigned k=0; k<nCnt; k++)
	{
		int	nKey;
		short nCount;
		_pFileData->read_bin_short(&nCount);
		for(int i=0; i<nCount; i++)
		{
			bbyte	nType;
			short	nIndex;
			if(i==0)
				nIndex	= 0;
			else
				_pFileData->read_bin_short(&nIndex);

			int nHash;
			short	nSize;
			bool	bVariable=false;
			if(nIndex==-1)
			{
				nHash	= 0;
				nType	= TYPE_BOOL2;
				nSize	= 0; // bool data is removed. // 2023-11-13
			}else{
				nHash	= stlVHashLocal[nIndex];//m_stlVTypedef[nIndex].nHash;
				nType	= stlVTypeLocal[nIndex];//m_stlVTypedef[nIndex].nType;
				if(nType >= TYPE_STRING)
				{
					_pFileData->read_bin_short(&nSize);
					bVariable	= true;
					nSize	+= 2;
				}else{
					nSize	= stlVSizeLocal[nIndex];//m_stlVTypedef[nIndex].nSize;
				}
			}
			bbyte*pData	= (bbyte*)PT_Alloc(bbyte, nSize);
			m_stlVpAllocList.push_back(pData);
			bbyte*pData2	= pData;
			if(bVariable)
			{
				pData2	+= 2;
				nSize	-= 2;
				BaseSystem::memcpy(pData, &nSize, sizeof(short));
			}
			_pFileData->Read(pData2, nSize);// editing point for indexing : pointing by OJ 2010-08
			if(i==0){
				BaseSystem::memcpy(&nKey, pData, sizeof(int));
#ifdef _DEBUG
				if(nKey == -2128895252)
					g_SendMessage(LOG_MSG, "loadded sample key");
#endif
			}

			if(nIndex >= 0 && (int)stlVHashLocal.size() > nIndex)
			{
				int nHash2	= stlVHashLocal[nIndex];
				int nNewIndex	= get_index(nHash2);
				if(nNewIndex >= 0){
					add_row_(&m_stlMnData[nKey], nNewIndex, pData);
				}else if((int)type_length() > nIndex
					&& type_get(nIndex).nType	== nType){
						add_row_(&m_stlMnData[nKey], nIndex, pData);
				}
			}else if(nType == TYPE_BOOL2){
				//set_(&m_stlMnData[nKey], nIndex, pData);
			}
		}
		
		update_linktype_(&m_stlMnData[nKey]);
	}
	return true;
}

bool	BaseDStructure::load_data_(const char *_strFilename)
{
	bool	bRet	= false;
	char strFilename[255];
    strcpy_s(strFilename, 255, _strFilename);
	
    const char *strPost;
    strPost	= strrchr(_strFilename, SLASH_C);
    
    if(!strPost)
        strPost = _strFilename;
    
	const char* str = strrchr(strPost, '.');

	if(str == NULL)// || strcmp(str, ".ini") == 0)
        strcat_s(strFilename, 255, ".data");
    
	BaseFile	*pfile;
	pfile	= sm_callbackFileOpen(strFilename);

	if(pfile)
	{
		bRet	= load_data_(pfile);
		sm_callbackFileClose(pfile);
	}
	return bRet;
}

bool	BaseDStructure::save(const char *_strName, int _bType)
{
	char strName[255];
	if(_strName)
		strcpy_s(strName, 255, _strName);
	else
		strcpy_s(strName, 255, m_strName);

	bool	bRet	= true;
	//if(_bType != 2)
	//{
	//	bRet	= save_define_(strName);
	//	if(!bRet)
	//		return bRet;
	//}

	if(_bType == 1)
	{
		bRet	= save_index_(strName, 0);
		if(!bRet)
			return bRet;

		//save_data_asc_(strName);
	}

	if(_bType != 2)
		save_data_asc_(strName);
	bRet	= save_data_(strName);
	if(!bRet)
		return bRet;
	return bRet;
}

bool	BaseDStructure::save_define_(const char *_strFilename)
{
	bool	bRet	= false;
	char strFilename[255], strBuf[255];
	strcpy_s(strFilename, 255, _strFilename);
	strcat_s(strFilename, 255, ".define");

	char *strName = strrchr(strFilename, SLASH_C);
	if(strName)
		*(strName+1) = NULL;
	else
		*strFilename = NULL;
	strcat_s(strFilename, 255, "Basedefine.define");

	BaseFile	file;
	file.set_asc_seperator("\t");

	if(!file.OpenFile(strFilename, BaseFile::OPEN_WRITE))
	{	
		strcpy_s(strBuf, 255, "NumOfDefine");
		file.write_asc_string(strBuf, 255);
		//file.write_asc_integer((int)type_length());
		file.write_asc_line();
		strcpy_s(strBuf, 255, "Size of Flag");
		file.write_asc_string(strBuf, 255);
		file.write_asc_integer(0);// originaly write of bool flag, but now it's not used anymore. // 2023-11-13
		file.write_asc_line();
		strcpy_s(strBuf, 255, "NumOfDataFile");
		file.write_asc_string(strBuf, 255);
		//file.write_asc_integer(1);
		file.write_asc_line();
		for(unsigned i=0; i<type_length(); i++)
		{
			STypeDefine sDefine	= type_get(i);

			if (i > 700)
			{
				if (strcmp(sDefine.strComment, STR_COMMENT_APPENDED_VARIABLE) == 0)
					continue;
			}
			get_flag(strBuf, sDefine.nFlag);
			file.write_asc_string(strBuf, 255);// Add by OJ : 2010-04-14
			file.write_asc_string(sDefine.strName, 255);
			file.write_asc_string(sm_stlVTypeName[sDefine.nType].c_str(), 30);
			file.write_asc_string(sDefine.strEnumState, 255);
			if(sDefine.strComment[0] != 0)
				file.write_asc_string(sDefine.strComment, 255);
			file.write_asc_line();
		}
		bRet	= true;
		file.CloseFile();
	}
	return bRet;
}

bool	BaseDStructure::save_index_(const char *_strFilename, short _nIndex)
{
	bool	bRet	= false;
	char strFilename[255];
	
	strcpy_s(strFilename, 255, _strFilename);
	strcat_s(strFilename, 255, "_");
	strcat_s(strFilename, 255, type_get(_nIndex).strName);
	strcat_s(strFilename, 255, ".index");

	BaseFile	file;
	if(!file.OpenFile(strFilename, BaseFile::OPEN_WRITE))
	{
		bRet	= true;
		file.CloseFile();
	}
	return bRet;
}

bool	BaseDStructure::save_data_asc_(const char *_strFilename)
{
	bool	bRet	= false;
	char strFilename[255];
	strcpy_s(strFilename, 255, _strFilename);
	strcat_s(strFilename, 255, ".ini");

	BaseFile	file;
	if(!file.OpenFile(strFilename, BaseFile::OPEN_WRITE | BaseFile::OPEN_ASC))
	{
		UINT32	nCnt=0;
		nCnt	= (UINT32)m_stlMnData.size();
		//file.write_bin_unsigned_long(&nCnt);
		STLMnstlRecord::iterator	it;
		for(it=m_stlMnData.begin();it!=m_stlMnData.end();it++)
		{
			short nCount	= (short)it->second.size();
			//file.write_bin_short(&nCount);
			if(type_get(1).nType == TYPE_STRING
				&& type_get(1).nHash == it->second[1].nHash)
				file.write_brace_start((char*)(it->second[1].pColumn)+2);
			else
				file.write_brace_start("Default");
			if(nCount > 0)
			{	
				file.write_brace_start(RES_STR_KEY);
				//file.Write(it->second[0].pColumn, type_get(0).nSize);
				file.write_bin_sinteger((int*)it->second[0].pColumn);
				file.write_brace_end();
				for(unsigned i=1; i<it->second.size(); i++)
				{
					bbyte nType = 0;
					int nHash;
					short	nIndex, nSize;
					bool bVariable	= false;
					nHash	= it->second[i].nHash;
					B_ASSERT(nHash != 0);
					type_find(nHash, &nIndex);
					file.write_brace_start(type_get(nIndex).strName);
					nType	= type_get(nIndex).nType;
					if(nType >= TYPE_STRING)
					{
						BaseSystem::memcpy(&nSize, it->second[i].pColumn, sizeof(short));
						nSize+=2;
						bVariable	= true;
					}else{
						nSize	= type_get(nIndex).nSize;
					}

					short temp = 0;
					file.write_bin_short(&temp);
					if(bVariable)// editing point for indexing : pointing by OJ 2010-08
					{
						nSize -= 2;
						if (nType == TYPE_ARRAY_INT32)
							nSize = nSize / 4;
						//file.write_bin_short(&nSize);

						if (nType == TYPE_STRING) {
							file.write_bin_string(((char*)it->second[i].pColumn) + 2);
						} else if (nType == TYPE_ARRAY_INT32) {
							int *pnArray = (int*)(((char*)it->second[i].pColumn) + 2);
							for(int j=0; j<(nSize); j++)
								file.write_bin_sinteger(pnArray+j);
						} else {
							file.Write(((char*)it->second[i].pColumn) + 2, nSize);
						}
					}else{
						if (nType == TYPE_INT32)
							file.write_bin_sinteger((int*)it->second[i].pColumn);
						else
							file.Write(it->second[i].pColumn, nSize);
					}
					file.write_brace_end();
				}
			}
			file.write_brace_end();
		}
		bRet	= true;
		file.CloseFile();
	}
	return bRet;
}

bool	BaseDStructure::save_data_(const char *_strFilename, int _type)
{
	bool	bRet	= false;
	char strFilename[255];
    
    strcpy_s(strFilename, 255, _strFilename);

	const char* str = strrchr(strFilename, '.');
	if (str == NULL || strcmp(str, ".data") != 0)
        strcat_s(strFilename, 255, ".data");
    
	BaseFile	file;
	if(!file.OpenFile(strFilename, BaseFile::OPEN_WRITE))
	{
		UINT32	nCnt=0;
		{// save for variable define data
			nCnt	= (UINT32)type_length();
			file.write_bin_unsigned_long(&nCnt);
			for(unsigned i=0; i<nCnt; i++)
			{
				int value = type_get(i).nHash; file.write_bin_integer(&value);
				bbyte b = type_get(i).nType; file.write_bin_byte(&b);
				short s = type_get(i).nSize; file.write_bin_short(&s);
			}
		}
		nCnt	= (UINT32)m_stlMnData.size();
		file.write_bin_unsigned_long(&nCnt);
		STLMnstlRecord::iterator	it;
		for(it=m_stlMnData.begin();it!=m_stlMnData.end();it++)
		{
			short nCount	= (short)it->second.size();
			file.write_bin_short(&nCount);
			if(nCount > 0)
			{	
				file.Write(it->second[0].pColumn, type_get(0).nSize);
				for(unsigned i=1; i<it->second.size(); i++)
				{
					bbyte nType;
					int nHash;
					short	nIndex, nSize;
					nHash	= it->second[i].nHash;
					if(nHash==0)
					{
						nIndex	= -1;
						nSize	= 0; // bool type of data is removed // 2023-11-23
					}else{
						nIndex	= get_index(nHash);
						nType	= type_get(nIndex).nType;
						if(nType >= TYPE_STRING)
						{
							BaseSystem::memcpy(&nSize, it->second[i].pColumn, sizeof(short));
							nSize+=2;
						}else{
							nSize	= type_get(nIndex).nSize;
						}
					}
					file.write_bin_short(&nIndex);
					file.Write(it->second[i].pColumn, nSize);// editing point for indexing : pointing by OJ 2010-08
				}
			}
		}
		bRet	= true;
		file.CloseFile();
	}
	return bRet;
}

bool BaseDStructure::release(int _nKey)
{
	STLMnstlRecord::iterator	it;
	it	= m_stlMnData.find(_nKey);
	if(it == m_stlMnData.end())
		return false;

	release_rows(_nKey);

	m_stlMnData.erase(it);
	return true;
}


bool BaseDStructure::release_rows(int _nKey)
{
	STLMnstlRecord::iterator	it;
	it	= m_stlMnData.find(_nKey);
	if(it == m_stlMnData.end())
		return false;
	STLVColumn	*pColumn;
	pColumn	= &it->second;

	for(unsigned i=0; i<pColumn->size(); i++)
	{
		STLVpVoid::iterator itAlloc;
		itAlloc	= std::find(m_stlVpAllocList.begin(), m_stlVpAllocList.end(), pColumn->at(i).pColumn);
		if(itAlloc != m_stlVpAllocList.end())
		{
			PT_Free(pColumn->at(i).pColumn);
			m_stlVpAllocList.erase(itAlloc);
		}
	}
	pColumn->clear();
	return true;
}


void	BaseDStructure::add_row_alloc(int _nKey, int _nIndex, const void *_pVoid, short _nCount)
{
	if(_nIndex != 0 && !is_exist(_nKey))
	{
		g_SendMessage(LOG_MSG_POPUP, "'%d' already has a colum, please check '%s.define' file", _nKey, m_strName);
		//assert(is_exist(_nKey));
		return;
	}
	if(_nIndex < 0)
		return;

	int nSize	= type_get(_nIndex).nSize;
	bbyte nType	= type_get(_nIndex).nType;
	if(nType == TYPE_BOOL2)
	{
		return;
	}

	if(_pVoid == NULL)
	{
		return;
	}

	bool	bValiable	= false;

	if(nType >= TYPE_STRING)
	{
		int	nColumSize	= 4;

		if(nType == TYPE_STRING)
		{
			nSize	= (int)strlen((const char*)_pVoid)+3;
		}else if(nType >= TYPE_ARRAY)
		{
			nColumSize	= sm_stlVTypeSize[nType];
			nSize	= _nCount * nColumSize + 2;
		}
		B_ASSERT(nSize < LIMIT_STR + 4);
		bValiable	= true;
	}

	char *pData	= (char*)PT_Alloc(char, nSize);
	m_stlVpAllocList.push_back(pData);

	STLMnstlRecord::iterator it;
	it	= m_stlMnData.find(_nKey);// editing point for indexing : pointing by OJ 2010-08
	if(it != m_stlMnData.end())
	{
		add_row_(&it->second, _nIndex, pData);
	}else{
		add_row_(&m_stlMnData[_nKey], _nIndex, pData);
	}

	if(bValiable)
	{
		nSize	-= 2;
		BaseSystem::memcpy(pData, &nSize, 2);
		pData	+= 2;
	}
	BaseSystem::memcpy(pData, _pVoid, nSize);
}


void	BaseDStructure::set_alloc(int _nKey, int _nIndex, const void *_pVoid, short _nCount)
{
	if(_nIndex != 0 && !is_exist(_nKey))
	{
		g_SendMessage(LOG_MSG_POPUP, "'%d' already has a colum, please check '%s.define' file", _nKey, m_strName);
		//assert(is_exist(_nKey));
		return;
	}
	if(_nIndex < 0)
		return;

	int nSize	= type_get(_nIndex).nSize;
	bbyte nType	= type_get(_nIndex).nType;
	if(nType == TYPE_BOOL2)
	{
		return;
	}

	if(_pVoid == NULL)
	{
		set_(&m_stlMnData[_nKey], _nIndex, NULL);
		return;
	}

	bool	bValiable	= false;

	if(nType >= TYPE_STRING)
	{
		int	nColumSize	= 4;

		if(nType == TYPE_STRING)
		{
			nSize	= (int)strlen((const char*)_pVoid)+3;
		}else if(nType >= TYPE_ARRAY)
		{
			nColumSize	= sm_stlVTypeSize[nType];
			nSize	= _nCount * nColumSize + 2;
		}

		B_ASSERT(nSize < LIMIT_STR + 4);
		bValiable	= true;
	}

	char *pData	= (char*)PT_Alloc(char, nSize);
	m_stlVpAllocList.push_back(pData);

	STLMnstlRecord::iterator it;
	it	= m_stlMnData.find(_nKey);// editing point for indexing : pointing by OJ 2010-08
	if(it != m_stlMnData.end())
	{
		set_(&it->second, _nIndex, NULL); // data�� ����
		set_(&it->second, _nIndex, pData);
	}else{
		set_(&m_stlMnData[_nKey], _nIndex, pData);
	}

	if(bValiable)
	{
		nSize	-= 2;
		BaseSystem::memcpy(pData, &nSize, 2);
		pData	+= 2;
	}
	BaseSystem::memcpy(pData, _pVoid, nSize);
}

void BaseDStructure::variable_defines_for_enum(void *_pManager)
{
	STLMnstlRecord::iterator it;
	it = m_stlMnData.begin();

	for (; it != m_stlMnData.end(); it++)
	{
		const int *pnType = (const int*)get_value_(&it->second, HASH_STATE(BaseVariableDefine_nV));

		if (pnType && *pnType > TYPE_BOOL2 && *pnType < TYPE_MAX) {
			((BaseStateManager*)_pManager)->variable_define(it->first, *pnType, true);
		}
	}
}

void	BaseDStructure::set(int _nKey, int _nIndex, void *_pVoid)
{
	if(_nIndex != 0 && !is_exist(_nKey))
	{
		g_SendMessage(LOG_MSG_POPUP, "'%d' already has a colum, please check '%s.define' file", _nKey, m_strName);
		//assert(is_exist(_nKey));
		return;
	}

	bbyte nType	= type_get(_nIndex).nType;
	if(nType == TYPE_BOOL2)
	{
		//short shPos	= m_stlVTypedef[_nIndex].nSize;//get_value_(_nKey, _nHash);
		//void *pFlag = get_value_(&m_stlMnData[_nKey], 0);
		//set_bool_(pFlag, (bbyte)shPos, *((bool*)_pVoid));
		return;
	}

	set_(&m_stlMnData[_nKey], _nIndex, _pVoid);
}

void	BaseDStructure::update_linktype_(STLVColumn	*_pstlVColumn)
{
	static int strHash_a[3] = {0, 0, 0};
	static int hash_a[3];
	if(strHash_a[0] == 0)
	{
		strHash_a[0] = STRTOHASH("state_release");
		strHash_a[1] = STRTOHASH("state_dummy");
		strHash_a[2] = STRTOHASH("state_return");
		
		//hash_a[0] = HASH_STATE(STATE_RELEASE);
		//hash_a[1] = HASH_STATE(STATE_DUMMY);
		//hash_a[2] = HASH_STATE(STATE_RETURN);
	}
	const char *name_str = (const char*)_pstlVColumn->at(1).pColumn;
	name_str += 2;
	int nameHash = STRTOHASH(name_str);
	bool find = false;
	int nHash = 0;
	
	for(int i=0; i<3; i++)
	{
		if(nameHash == strHash_a[i])
		{
			nHash = strHash_a[i];
			find = true;
		}
	}
	if(!find)
		return;
	
	if(_pstlVColumn->size() == 2)
		return;
	
	int hashLinkType = HASH_STATE(HASH_BaseLinkType);
	
	find = false;
	for(int i=2; i<_pstlVColumn->size(); i++)
	{
		if(_pstlVColumn->at(i).nHash == hashLinkType)
		{
			int type = *((int*)_pstlVColumn->at(i).pColumn);
			if(type != nHash)
			{
				int key = *((int*)_pstlVColumn->at(0).pColumn);
				*((int*)_pstlVColumn->at(i).pColumn) = nHash;
			}
			find = true;
			break;
		}
	}
	if(!find)
	{
		int key = *((int*)_pstlVColumn->at(0).pColumn);
		short nSize	= 4;

		char *pData	= (char*)PT_Alloc(char, nSize);
		BaseSystem::memcpy(pData, &nHash, 4);
		m_stlVpAllocList.push_back(pData);
		
		ST_BaseDColumn colum;
		colum.nHash	= hashLinkType;
		colum.pColumn = pData;
		_pstlVColumn->push_back(colum);
	}
}

void	BaseDStructure::add_row_(STLVColumn	*_pstlVColumn, int _nIndex, void *_pVoid)
{
	//assert(_pVoid);
	if(_pVoid==NULL)
	{
		g_SendMessage(LOG_MSG_POPUP, "You shouldn't add NULL colum.");
		return;
	}

	int nHash;
	nHash	= type_get(_nIndex).nHash;

	ST_BaseDColumn colum;
	colum.nHash	= nHash;
	colum.pColumn	= _pVoid;
	_pstlVColumn->push_back(colum);
}

void	BaseDStructure::set_(STLVColumn	*_pstlVColumn, int _nIndex, void *_pVoid)
{
	int nHash;
	nHash	= type_get(_nIndex).nHash;

	int nIndexRow	= get_row_sequence_(_pstlVColumn, nHash);

#ifdef _DEBUG
	if (nHash == 1351175842
		&& _pVoid == NULL)
	{
		int x = 0;
		x++;
	}
#endif
	if(nIndexRow < 0)
	{
		if(_pVoid)
		{
			ST_BaseDColumn colum;
			colum.nHash	= nHash;
			colum.pColumn	= _pVoid;
			if (_nIndex == 0) {
				_pstlVColumn->insert(_pstlVColumn->begin(), colum);
			}else if (_nIndex == 1 && _pstlVColumn->size() > 0) {
				_pstlVColumn->insert(_pstlVColumn->begin()+1, colum);
			}else {
				_pstlVColumn->push_back(colum);
			}

			
		}
		return;
	}
	
	if(_pVoid == NULL)// editing point for indexing : pointing by OJ 2010-08
	{
		if(nIndexRow >= 0)
		{
			STLVpVoid::iterator itAlloc;
			itAlloc	= std::find(m_stlVpAllocList.begin(), m_stlVpAllocList.end(), _pstlVColumn->at(nIndexRow).pColumn);
			if(itAlloc != m_stlVpAllocList.end())
			{	
				PT_Free(_pstlVColumn->at(nIndexRow).pColumn);
				m_stlVpAllocList.erase(itAlloc);
			}
			_pstlVColumn->erase(_pstlVColumn->begin()+nIndexRow);
		}
	}else{
		_pstlVColumn->at(nIndexRow).pColumn	= _pVoid;
	}
}

int		BaseDStructure::get_type_count() const
{
	return (int)type_length();
}

int	BaseDStructure::get_index(int _nKey, int _nSequence)
{
	STLMnstlRecord::const_iterator	itData;
	itData	= m_stlMnData.find(_nKey);

	if(itData==m_stlMnData.end())
		return -1;
	
	if(_nSequence < 0 || _nSequence >= (int)itData->second.size())
		return -1;

	return get_index(itData->second[_nSequence].nHash);
}

bool BaseDStructure::get_param(int _nKey, int _nIndex, const void **_ppVoid, short *_pnCount, int _nIndexStartSequence) const
{
	if (_nIndex >= (int)type_length()
		|| _nIndex < 0)
		return false;

	if (!is_exist(_nKey))
		return false;

	bbyte nType = type_get(_nIndex).nType;

	B_ASSERT(nType != TYPE_BOOL2);

	*_ppVoid = NULL;
	if (_pnCount)
		*_pnCount = 0;

	STLMnstlRecord::const_iterator	itData;
	itData = m_stlMnData.find(_nKey);

	if (itData != m_stlMnData.end())// editing point for indexing : pointing by OJ 2010-08
	{
		if (_pnCount)
			*_pnCount = 1;

		*_ppVoid = get_param_(&itData->second, type_get(_nIndex).nHash, _nIndexStartSequence);
		if (nType >= TYPE_STRING && *_ppVoid) // return �Ǿ� �� ���� ���̰� ���ϴ� ���� ��
		{
			char **ppStr;
			ppStr = (char**)_ppVoid;
			if (_pnCount)
			{
				BaseSystem::memcpy(_pnCount, *_ppVoid, sizeof(short));
				//assert(sm_stlVTypeSize[nType]);
				if (sm_stlVTypeSize[nType]>0)
					*_pnCount /= sm_stlVTypeSize[nType];
			}
			*ppStr += 2;
			return true;
		}
	}

	if (*_ppVoid)
	{
		if (_pnCount)
			*_pnCount = 1;
		return true;
	}

	if (_pnCount)
		*_pnCount = 0;
	return false;
}

bool BaseDStructure::get(int _nKey, const void **_ppVoid, short *_pnCount, int _nIndexSequence) const
{
	STLMnstlRecord::const_iterator	itData;
	itData	= m_stlMnData.find(_nKey);

	if(itData!=m_stlMnData.end())// editing point for indexing : pointing by OJ 2010-08
	{
		if(_pnCount)
			*_pnCount	= 1;

		if ((int)itData->second.size() <= _nIndexSequence
			|| _nIndexSequence < 0)
			return false;

		const ST_BaseDColumn *pStColum	= &itData->second[_nIndexSequence];
		int nIndex	= get_index(pStColum->nHash);
		int nType	= get_type(nIndex);
		*_ppVoid	= pStColum->pColumn;
		if(nType >= TYPE_STRING && *_ppVoid) // return �Ǿ� �� ���� ���̰� ���ϴ� ���� ��
		{
			char **ppStr;
			ppStr	= (char**)_ppVoid;
			if(_pnCount)
			{
				BaseSystem::memcpy(_pnCount, *_ppVoid, sizeof(short));
				if(sm_stlVTypeSize[nType]>0)
					*_pnCount	/= sm_stlVTypeSize[nType];
			}
			*ppStr	+= 2;
			return true;
		}
	}

	if(*_ppVoid)
	{
		if(_pnCount)
			*_pnCount	=	1;
		return true;
	}

	if(_pnCount)
		*_pnCount	= 0;
	return false;
}

bool BaseDStructure::get(int _nKey, int _nIndex, const void **_ppVoid, short *_pnCount, int _nIndexSequence) const
{
	if(_nIndex >= (int)type_length()
		|| _nIndex < 0)
		return false;

	if(!is_exist(_nKey))
		return false;

	bbyte nType	= type_get(_nIndex).nType;

	B_ASSERT(nType != TYPE_BOOL2);

	*_ppVoid	= NULL;
	if(_pnCount)
		*_pnCount	= 0;

	STLMnstlRecord::const_iterator	itData;
	itData	= m_stlMnData.find(_nKey);

	if(itData!=m_stlMnData.end())// editing point for indexing : pointing by OJ 2010-08
	{
		if(_pnCount)
			*_pnCount	= 1;

		*_ppVoid	= get_value_(&itData->second, type_get(_nIndex).nHash, _nIndexSequence);
		if(nType >= TYPE_STRING && *_ppVoid) // return �Ǿ� �� ���� ���̰� ���ϴ� ���� ��
		{
			char **ppStr;
			ppStr	= (char**)_ppVoid;
			if(_pnCount)
			{
				BaseSystem::memcpy(_pnCount, *_ppVoid, sizeof(short));
				//assert(sm_stlVTypeSize[nType]);
				if(sm_stlVTypeSize[nType]>0)
					*_pnCount	/= sm_stlVTypeSize[nType];
			}
			*ppStr	+= 2;
			return true;
		}
	}

	if(*_ppVoid)
	{
		if(_pnCount)
			*_pnCount	=	1;
		return true;
	}

	if(_pnCount)
		*_pnCount	= 0;
	return false;
}
bool	BaseDStructure::get_by_hash(int _nKey, int _nHash, const void **_ppVoid) const
{
	short nIndex;
	nIndex	= get_index(_nHash);

	if(nIndex < 0)
		return false;

	return get(_nKey, nIndex, _ppVoid);
}

bool	BaseDStructure::is_exist(int _nKey) const
{
	STLMnstlRecord::const_iterator	it;
	it	= m_stlMnData.find(_nKey);
	if(it == m_stlMnData.end())
		return false;
	return true;
}

bool	BaseDStructure::change_key(int _nKey, int _nKeyto)
{
	STLMnstlRecord::iterator	it, itTo;
	it	= m_stlMnData.find(_nKey);
	if(it == m_stlMnData.end())
		return false;

	if(is_exist(_nKeyto))
		return false;

	m_stlMnData[_nKeyto]	= it->second;
	m_stlMnData.erase(_nKey);
	return true;
}

int		BaseDStructure::get_first_key(STLMnstlRecord::iterator *_pit)
{
	*_pit	= m_stlMnData.begin();
	if(*_pit == m_stlMnData.end())
		return 0;
	return (*_pit)->first;
}

int		BaseDStructure::get_next_key(STLMnstlRecord::iterator *_pit)
{
    if(m_stlMnData.size() == 0)
        return 0;
    
	(*_pit)++;
	if(*_pit == m_stlMnData.end())
		return 0;
	return (*_pit)->first;
}

void	*BaseDStructure::get_value_const_(STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence)
{
	for(unsigned i=0; i<_pstlVColumn->size(); i++)
	{
		if(_pstlVColumn->at(i).nHash == _nHash)
		{
			if(_nIndexSequence == 0)
				return _pstlVColumn->at(i).pColumn;
			_nIndexSequence--;
		}
	}
	return NULL;
}

void *BaseDStructure::get_param_(const STLVColumn	*_pstlVColumn, int _nHash, int _nIndexStartSequence)
{
	int nSeqParam = _nIndexStartSequence;

	do {
		if (nSeqParam >= (int)_pstlVColumn->size())
			return NULL;

		int nIndex = get_index(_pstlVColumn->at(nSeqParam).nHash);
		int nFlag = get_type_flag(nIndex);

		if (nSeqParam != _nIndexStartSequence && (nFlag & STATE_CALLTYPE) != 0)
			return NULL;

		if (_pstlVColumn->at(nSeqParam).nHash == _nHash)
			return _pstlVColumn->at(nSeqParam).pColumn;
		nSeqParam++;
	} while (1);

	return NULL;
}

int BaseDStructure::get_mass_size(const STLVColumn* _pstlVColumn, int _hash) const
{
	int ret = 0;
	for (unsigned i = 0; i < _pstlVColumn->size(); i++)
	{
		if (_pstlVColumn->at(i).nHash == _hash)
		{
			for (unsigned j = i; j < _pstlVColumn->size(); j++)
			{
				if (_pstlVColumn->at(j).nHash == _hash)
				{
					short cnt;
					BaseSystem::memcpy(&cnt, _pstlVColumn->at(j).pColumn, sizeof(short));
					ret += cnt;
				}
			}
			return ret;
		}
	}

	return 0;
}

const void	*BaseDStructure::get_value_(const STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence)const
{
	const void* void_p = NULL;
	for(unsigned i=0; i<_pstlVColumn->size(); i++)
	{
		if(_pstlVColumn->at(i).nHash == _nHash)
		{
			if (_nIndexSequence == 0)
			{
				void_p = _pstlVColumn->at(i).pColumn;
				return void_p;
			}
			_nIndexSequence--;
		}
	}

	return NULL;
}

void	*BaseDStructure::get_value_(STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence)
{
	for(unsigned i=0; i<_pstlVColumn->size(); i++)
	{
		if(_pstlVColumn->at(i).nHash == _nHash)
		{
			if(_nIndexSequence == 0)
				return _pstlVColumn->at(i).pColumn;
			_nIndexSequence--;
		}
	}
	return NULL;
}

int		BaseDStructure::get_row_sequence_(const STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence) const
{
	for(unsigned i=0; i<_pstlVColumn->size(); i++)
	{
		if(_pstlVColumn->at(i).nHash == _nHash)
		{
			if(_nIndexSequence == 0)
				return i;
			_nIndexSequence--;
		}
	}
	return -1;
}


short	BaseDStructure::get_size_(int _nHash)
{
	short index;
	if(!type_find(_nHash, &index))
		return 0;
	
	return type_get(index).nSize;
}

STypeDefine BaseDStructure::type_get(int _index)
{
	BraceInc(s_lockUse, s_lockUpdate);
	STypeDefine def;
	def = sm_stlVTypedef[_index];
	return def;
}

size_t BaseDStructure::type_length()
{
	BraceInc(s_lockUse, s_lockUpdate);
	size_t size = sm_stlVTypedef.size();
	return size;
}

bool BaseDStructure::type_find(int _hash, short *_index_p)
{
	BraceInc(s_lockUse, s_lockUpdate);
	//marker
	STLMnnTypeDefine::const_iterator it;
	it = sm_stlMnnTypedef.find(_hash);
	if (it == sm_stlMnnTypedef.end()){
		return false;
	}
	*_index_p = it->second;
	return true;
}

bool	BaseDStructure::type_original_check(int _nHash) const
{
	short index = 0;
	if(!type_find(_nHash, &index))
		return false;
	if (index >= sm_nDefineOriginalSize)
		return false;
	return true;
}

int BaseDStructure::get_hash(int _nIndex)
{ 
	return type_get(_nIndex).nHash;
}

short	BaseDStructure::get_index(int _nHash)
{
	short index;
	if(!type_find(_nHash, &index))
	{
		STLMnInt::iterator history_it = sm_defineHistory_m.find(_nHash);
		if (history_it == sm_defineHistory_m.end())
			return -1;
		if(!type_find(history_it->second, &index))
			return -1;
	}

	return index;
}

void	BaseDStructure::sort_()
{

}

bbyte	BaseDStructure::get_type_by_name_(const char *_strName)
{
	for(bbyte i=0; i<TYPE_MAX; i++)
	{
		if(strcmp(_strName, sm_stlVTypeName[i].c_str())==0)
			return i;
	}
	B_ASSERT(0);
	return 0;
}

size_t	BaseDStructure::get_count() const
{
	return m_stlMnData.size();
}

//#pragma warning(disable: 4996)

char *set_bool(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const bool *pBool	= (const bool*)_pData;
	if(*pBool)
		strcpy(_strRet, "true");
	else
		strcpy(_strRet, "false");

	return(_strRet + strlen(_strRet));
}

char *set_byte(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure* )
{
	const bbyte*pValue = (const bbyte*)_pData;
	sprintf(_strRet, "%d", *pValue);

	return(_strRet + strlen(_strRet));
}

char *set_short(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const short *pValue = (const short*)_pData;
	sprintf(_strRet, "%d", *pValue);

	return(_strRet + strlen(_strRet));
}

char *set_int(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure* _enum_p)
{
	const int *pValue = (const int*)_pData;

	const char* str = NULL;
	if (_enum_p)
	{
		if (_enum_p->get(*pValue, 1, (const void**)&str))
			strcpy(_strRet, str);
	}
	if(str == NULL)
		sprintf(_strRet, "%d", *pValue);

	return(_strRet + strlen(_strRet));
}

char *set_int64(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const INT64 *pValue = (const INT64*)_pData;
	sprintf(_strRet, "%lld", *pValue);

	return(_strRet + strlen(_strRet));
}

char *set_float(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const float *pValue	= (const float*)_pData;
	sprintf(_strRet, "%.2f", *pValue);

	return(_strRet + strlen(_strRet));
}

char *set_vector(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const float *pValue	= (const float*)_pData;
	sprintf(_strRet, "(%.2f, %.2f, %.2f)", *pValue, *(pValue+1), *(pValue+2));

	return(_strRet + strlen(_strRet));
}

char *set_string(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const char *pValue	= (const char*)_pData;
	sprintf(_strRet, "%s", pValue);

	return(_strRet + strlen(_strRet));
}

char *set_a_byte(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const bbyte*pValue	= (const bbyte*)_pData;
	for(int i=0; i<_nCnt; i++)
	{
		_strRet	 = set_byte(pValue, _strRet, 0, NULL);
		if(i<_nCnt-1)
		{
			strcat(_strRet, " ,");
			_strRet+=2;
			pValue++;
		}
	}

	return	_strRet;
}

char *set_a_short(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const short *pValue	= (const short*)_pData;
	for(int i=0; i<_nCnt; i++)
	{
		_strRet	 = set_short(pValue, _strRet, 0, NULL);
		if(i<_nCnt-1)
		{
			strcat(_strRet, " ,");
			_strRet+=2;
			pValue++;
		}
	}

	return	_strRet;
}

char *set_a_int(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*_enum_p)
{
	const int *pValue	= (const int*)_pData;
	for(int i=0; i<_nCnt; i++)
	{
		_strRet	 = set_int(pValue, _strRet, 0, _enum_p);
		if(i<_nCnt-1)
		{
			strcat(_strRet, " ,");
			_strRet+=2;
			pValue++;
		}
	}

	return	_strRet;
}

char *set_a_int64(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const INT64 *pValue	= (const INT64*)_pData;
	for(int i=0; i<_nCnt; i++)
	{
		_strRet	 = set_int64(pValue, _strRet, 0, NULL);
		if(i<_nCnt-1)
		{
			strcat(_strRet, " ,");
			_strRet+=2;
			pValue++;
		}
	}

	return	_strRet;
}

char *set_a_float(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const float *pValue	= (const float*)_pData;
	for(int i=0; i<_nCnt; i++)
	{
		_strRet	 = set_float(pValue, _strRet, 0, NULL);
		if(i<_nCnt-1)
		{
			strcat(_strRet, " ,");
			_strRet+=2;
			pValue++;
		}
	}

	return	_strRet;
}

char *set_a_vector(const void *_pData, char *_strRet, int _nCnt, const BaseDStructure*)
{
	const float *pValue	= (const float*)_pData;
	for(int i=0; i<_nCnt; i++)
	{
		_strRet	 = set_vector(pValue, _strRet, 0, NULL);
		if(i<_nCnt-1)
		{
			strcat(_strRet, " ,");
			_strRet+=2;
			pValue+=3;
		}
	}

	return	_strRet;
}

const char *get_bool(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	bool *pBool	= (bool*)_pData;
	if(*_strIn == 'f' || *_strIn == 'F')
        {*pBool	= false; _strIn+=5;}
	else 
        {*pBool	= true; _strIn+=4;}

	return _strIn;
}

const char *get_byte(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	int nValue;
	sscanf(_strIn, "%d", &nValue);
	*((bbyte*)_pData) = (bbyte)nValue;
	while(*_strIn!='\0'&&*_strIn!=','&&*_strIn!=' ')
		_strIn++;
	if(*_strIn==',')
		_strIn++;

	return _strIn;
}

const char *get_short(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	int nValue;
	sscanf(_strIn, "%d", &nValue);
	*((short*)_pData) = (short)nValue;
	while(*_strIn!='\0'&&*_strIn!=','&&*_strIn!=' ')
		_strIn++;
	if(*_strIn==',')
		_strIn++;

	return _strIn;
}

const char *get_int(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*_enum_p)
{
	int *pValue = (int*)_pData;
	char buf[255], *buf2;
	buf2 = buf;
    *pValue = 0;
    sscanf(_strIn, "%d", pValue);
	while (*_strIn != '\0' && *_strIn != ',' && *_strIn != ' ')
	{
		*buf2 = *_strIn;
		_strIn++;
		buf2++;
	}
	*buf2 = 0;
	if(*_strIn==',')
		_strIn++;

	if (buf[0] == '0' && buf[1] == 0)
		_enum_p = NULL;

	if (_enum_p && *pValue == 0)
	{
		*pValue = STRTOHASH(buf);
	}

	return _strIn;
}

const char *get_int64(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	INT64 *pValue = (INT64*)_pData;
	sscanf(_strIn, "%lld", pValue);
	while(*_strIn!='\0'&&*_strIn!=','&&*_strIn!=' ')
		_strIn++;
	if(*_strIn==',')
		_strIn++;

	return _strIn;
}

const char *get_float(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	float *pValue	= (float*)_pData;
	sscanf(_strIn, "%f", pValue);
	while(*_strIn!='\0'&&*_strIn!=','&&*_strIn!=' ')
		_strIn++;
	if(*_strIn==',')
		_strIn++;

	return _strIn;
}

const char *get_vector(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	float *pValue	= (float*)_pData;
	sscanf(_strIn, "(%f, %f, %f)", pValue,pValue+1,pValue+2);
	while(*_strIn!='\0'&&*_strIn!=')')
		_strIn++;

	if(*_strIn == ')')
		_strIn++;
	return _strIn;
}


const char *get_string(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	char *pValue	= (char*)_pData;
	while(*_strIn!='\0')
	{
		*pValue	= *_strIn;
		_strIn++;
		pValue++;
	}
	*pValue	= *_strIn;

	return _strIn;
}

const char *get_a_byte(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	bbyte*pValue	= (bbyte*)_pData;
	*_nCnt	= 0;
	while(*_strIn!='\0')
	{
		_strIn	 = get_byte(pValue, _strIn, 0, NULL);
		while(*_strIn==','||*_strIn==' ')
			_strIn++;
		pValue++;
		(*_nCnt)++;
	}

	return	_strIn;
}

const char *get_a_short(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	short *pValue	= (short*)_pData;
	*_nCnt	= 0;
	while(*_strIn!='\0')
	{
		_strIn	 = get_short(pValue, _strIn, 0, NULL);
		while(*_strIn==','||*_strIn==' ')
			_strIn++;
		pValue++;
		(*_nCnt)++;
	}

	return	_strIn;
}

const char *get_a_int(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure* _enum_p)
{
	int *pValue	= (int*)_pData;
	*_nCnt	= 0;
	while(*_strIn!='\0')
	{
		_strIn	 = get_int(pValue, _strIn, 0, _enum_p);
		while(*_strIn==','||*_strIn==' ')
			_strIn++;
		pValue++;
		(*_nCnt)++;
	}
	return	_strIn;
}

const char *get_a_int64(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	INT64 *pValue	= (INT64*)_pData;
	*_nCnt	= 0;
	while(*_strIn!='\0')
	{
		_strIn	 = get_int64(pValue, _strIn, 0, NULL);
		while(*_strIn==','||*_strIn==' ')
			_strIn++;
		pValue++;
		(*_nCnt)++;
	}

	return	_strIn;
}

const char *get_a_float(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	float *pValue	= (float*)_pData;
	*_nCnt	= 0;
	while(*_strIn!='\0')
	{
		_strIn	 = get_float(pValue, _strIn, 0, NULL);
		while(*_strIn==','||*_strIn==' ')
			_strIn++;
		pValue++;
		(*_nCnt)++;
	}

	return	_strIn;
}

const char *get_a_vector(void *_pData, const char *_strIn, int *_nCnt, const BaseDStructure*)
{
	float *pValue	= (float*)_pData;
	*_nCnt	= 0;
	while(*_strIn!='\0')
	{
		_strIn	 = get_vector(pValue, _strIn, 0, NULL);
		while(*_strIn==','||*_strIn==' ')
			_strIn++;
		pValue+=3;
		(*_nCnt)++;
	}

	return	_strIn;
}

char*(*s_callbackSet[TYPE_MAX])(const void*, char *, int, const BaseDStructure *)	= {
	NULL,		//TYPE_NULL,
	set_bool,	//TYPE_BOOL2,
	set_byte,	//TYPE_BYTE,
	set_short,	//TYPE_SHORT,
	set_int,	//TYPE_INT32,
	set_int64,	//TYPE_INT64,
	set_float,	//TYPE_FLOAT,
	set_vector,	//TYPE_VECTOR3,
	set_string,	//TYPE_STRING,
	set_a_byte,	//TYPE_ARRAY_BYTE = TYPE_ARRAY,
	set_a_short,	//TYPE_ARRAY_SHORT,
	set_a_int,		//TYPE_ARRAY_INT32,
	set_a_int64,	//TYPE_ARRAY_INT64,
	set_a_float,	//TYPE_ARRAY_FLOAT,
	set_a_vector	//TYPE_ARRAY_VECTOR3
};

const char*(*s_callbackGet[TYPE_MAX])(void*, const char*, int *, const BaseDStructure * )	= {
	NULL,		//TYPE_NULL,
	get_bool,	//TYPE_BOOL2,
	get_byte,	//TYPE_BYTE,
	get_short,	//TYPE_SHORT,
	get_int,	//TYPE_INT32,
	get_int64,	//TYPE_INT64,
	get_float,	//TYPE_FLOAT,
	get_vector,	//TYPE_VECTOR3,
	get_string,	//TYPE_STRING,
	get_a_byte,	//TYPE_ARRAY_BYTE = TYPE_ARRAY,
	get_a_short,	//TYPE_ARRAY_SHORT,
	get_a_int,		//TYPE_ARRAY_INT32,
	get_a_int64,	//TYPE_ARRAY_INT64,
	get_a_float,	//TYPE_ARRAY_FLOAT,
	get_a_vector	//TYPE_ARRAY_VECTOR3
};

bool	BaseDStructure::get_string(int _nKey, char *_strRet, int _nSequence)
{
	const void *pData;
	short	nCnt	= 0;
	//bool	bBool;

	STLMnstlRecord::const_iterator	itRecord;
	itRecord	= m_stlMnData.find(_nKey);
	if(itRecord == m_stlMnData.end())
		return false;

	if(_nSequence < 0 || _nSequence >= (int)itRecord->second.size())
		return false;

	int nIndex;
	nIndex	= get_index(itRecord->second[_nSequence].nHash);
	if(nIndex < 0)
		return false;
	
	bbyte nType;
	nType	= type_get(nIndex).nType;

	if(nType == TYPE_BOOL2)
	{
		//if(!get(_nKey, _nIndex, &bBool))
		//	return false;

		//s_callbackSet[nType]((void*)&bBool, _strRet, nCnt);
	}else{
		if(!get(_nKey, &pData, &nCnt, _nSequence))
			return false;
		const char *enum_str = get_type_enum_state(nIndex);
		BaseDStructure* pdsvEnum = NULL;
		if(BaseStateManager::get_manager())
			pdsvEnum = BaseStateManager::get_manager()->EnumGet(STRTOHASH(enum_str));
		s_callbackSet[nType](pData, _strRet, nCnt, pdsvEnum);
	}	
	return true;
}

bool	BaseDStructure::add_string(int _nKey, int _nIndex, const char *_strIn)
{
	if(_nIndex<0 || _nIndex >= (int)type_length())
		return false;

	void *pData	= PT_Alloc(bbyte, 4096);
	bbyte nType;
	nType	= type_get(_nIndex).nType;
	int	nCnt	= 0;

	if(nType == 0)
		return false;
	
	const char* enum_str = get_type_enum_state(_nIndex);
	BaseDStructure* pdsvEnum = NULL;
	if(*enum_str != 0 && BaseStateManager::get_manager())
		pdsvEnum = BaseStateManager::get_manager()->EnumGet(STRTOHASH(enum_str));

	s_callbackGet[nType](pData, _strIn, &nCnt, pdsvEnum);
	add_row_alloc(_nKey, _nIndex, pData, (short)nCnt);

	PT_Free(pData);
	return true;
}

int BaseDStructure::create_key_link(const char *_str_key)
{
	char buff[255];
	strcpy_s(buff, 255, sm_user);
	strcat_s(buff, 255, _str_key);
	int key = STRTOHASH(buff);

	while(is_exist(key)){
		key++;
	}

	return key;
}

void BaseDStructure::get_keys(STLVInt *_pstlVConflictKey) const
{
	STLMnstlRecord::const_iterator	itRecord;
	itRecord	= m_stlMnData.begin();
	for(;itRecord!=m_stlMnData.end();itRecord++)
	{	
		int nKey	= *((int*)itRecord->second[0].pColumn);
		_pstlVConflictKey->push_back(nKey);
	}
}

int BaseDStructure::compare(const BaseDStructure &_dstTarget, STLVInt *_pstlVConflictKey, STLVInt *_pstlVBerageableKey) const
{
	int nRet	= COMPARE_EQUAL;

	int		nHash;
	short	nTIndex;
	for(unsigned i=0; i<type_length(); i++)
	{
		nHash	= type_get(i).nHash;

		nTIndex	= _dstTarget.get_index(nHash);

		if(nTIndex >= 0 && _dstTarget.get_type(nTIndex) != type_get(i).nType)
			return COMPARE_CONFLICT_DEFINE;

		if(nTIndex == -1)
			nRet	= COMPARE_INCLUDE;
	}

	if((int)type_length() < _dstTarget.get_type_count())
		nRet	= COMPARE_MERAGEABLE;

	bbyte	nType;
	short	nIndex;
	int		nSize;
	int		nHashLink	= STRTOHASH(RES_STR_ARRAYLINK);

	STLMnstlRecord::const_iterator	itRecord;
	itRecord	= m_stlMnData.begin();
	int nEqualRecordCnt	= 0;
	for(;itRecord!=m_stlMnData.end();itRecord++)
	{
		int nKey	= *((int*)itRecord->second[0].pColumn);
		if(!_dstTarget.is_exist(nKey))
		{
			if(nRet	< COMPARE_INCLUDE)
				nRet	= COMPARE_INCLUDE;
			continue;
		}

		int nCountEqual	= 0;
		bool bDif	= false;

		for(unsigned i=0; i<itRecord->second.size(); i++)
		{
			nIndex	= get_index(itRecord->second[i].nHash);
			nType	= get_type(nIndex);
			nSize	= get_type_size(nIndex);

			nTIndex	= _dstTarget.get_index(itRecord->second[i].nHash);
			if(nTIndex == -1)
			{
				return COMPARE_CONFLICT_DEFINE;
			}else{
				if(nType == TYPE_BOOL2
					&& itRecord->second[i].nHash == nHashLink)// RES_STR_ARRAYLINK �� ���� ���ε��� �ٸ� �� ������ �߰� �񱳸� �ϱ� ������ ������ �ʾƵ� �ȴ�
				{
					nCountEqual++;
				}else{
					short	nCnt;
					const void *pSrc, *pTarget;
					if(get(nKey, (const void**)&pSrc, &nCnt, i))
					{
						if(_dstTarget.get(nKey, (const void**)&pTarget, &nCnt, i))
						{
							if(memcmp(pSrc, pTarget, nSize*nCnt) != 0)
								bDif	= true;
							else
								nCountEqual++;
						}
					}
				}
			}			
		}
		int nTCnt	= _dstTarget.get_count_colum(nKey);
		if(bDif)
		{
			nRet	= COMPARE_CONFLICT_DATA;
			if(_pstlVConflictKey)
				_pstlVConflictKey->push_back(nKey);
		}else{
			if(nCountEqual < nTCnt)
			{
				if(nRet < COMPARE_MERAGEABLE)
					nRet = COMPARE_MERAGEABLE;

				if(_pstlVBerageableKey)
					_pstlVBerageableKey->push_back(nKey);
			}
		}

		if(nCountEqual == nTCnt)
			nEqualRecordCnt++;
	}
	if(nEqualRecordCnt < (int)_dstTarget.get_count())
	{
		if(nRet < COMPARE_MERAGEABLE)
			nRet = COMPARE_MERAGEABLE;
	}

	return nRet;
}

int	BaseDStructure::get_count_colum(int _nKey) const
{
	STLMnstlRecord::const_iterator	itRecord;
	itRecord	= m_stlMnData.find(_nKey);
	if(itRecord != m_stlMnData.end())
		return (int)itRecord->second.size();
	return 0;
}
//
//void BaseDStructure::merage(const BaseDStructure &_dstTarget, int _nKeyFrom, int _nKeyTo)
//{
//	if(_nKeyTo == 0)
//		_nKeyTo	= _nKeyFrom;
//
//	if(!_dstTarget.is_exist(_nKeyFrom))
//		return;
//
//	for(int nTIndex=0; nTIndex<_dstTarget.get_type_count(); nTIndex++)
//	{
//		int nHash	= _dstTarget.get_hash(nTIndex);
//		int nType	= _dstTarget.get_type(nTIndex);
//		int nIndex	= get_index(nHash);
//
//		assert(nIndex >= 0);
//
//		const void *pValue;
//		short	nCnt;
//
//		if(nIndex == 0)
//		{
//			set_alloc(_nKeyTo, nIndex, &_nKeyTo);
//			continue;
//		}
//
//		if(get(_nKeyTo, nIndex, &pValue, &nCnt))// merage�ΰ�� ���� �����Ͱ������� ���� ���� �ʴ´�
//			continue;
//
//		if(nType == TYPE_BOOL2)
//		{
//			//if(_dstTarget.get(nKey, nTIndex, &bValue))
//			//	set_alloc(nKey, nIndex, &bValue);
//		}else{
//			if(_dstTarget.get(_nKeyFrom, nTIndex, &pValue, &nCnt))
//				set_alloc(_nKeyTo, nIndex, pValue, nCnt);
//		}
//	}
//}

void BaseDStructure::replace(const BaseDStructure &_dstTarget, int _nKeyFrom, int _nKeyTo)
{
	if(_nKeyTo == 0)
		_nKeyTo	= _nKeyFrom;

	release_rows(_nKeyTo);

	BaseDStructureValue	dsvTarget(&_dstTarget);
	dsvTarget.set_key(_nKeyFrom);

	for(int nSeq=0; nSeq<dsvTarget.get_count(); nSeq++)
	{	
		int nTIndex	= dsvTarget.get_index(dsvTarget.get_colum(nSeq));
		int nHash	= _dstTarget.get_hash(nTIndex);
		int nIndex	= get_index(nHash);

		B_ASSERT(nIndex >= 0);

		const void *pValue;
		short	nCnt;

		if(nIndex == 0)// 0: Key, Key�� ��� _nKeyTo�� �ִ´� (Link�� ��� Ű�� strName�� ��ġ���� �ʴ´�)
		{	
			set_alloc(_nKeyTo, nIndex, &_nKeyTo);
			continue;
		}

		if(dsvTarget.get(&pValue, &nCnt, nSeq)){
			add_row_alloc(_nKeyTo, nIndex, pValue, nCnt);
		}
	}
}

void BaseDStructure::replace(const BaseDStructure &_dstTarget, const STLVInt *_pstlVKey, const STLVInt *_pstlVMerageableKey)
{
	STLVInt stlVKey;

	if(_pstlVKey)
		stlVKey	= *_pstlVKey;
	else
		_dstTarget.get_keys(&stlVKey);


	int		nHash;
	short	nIndex;
	for(int nTIndex=0; nTIndex<_dstTarget.get_type_count(); nTIndex++)
	{
		nHash	= _dstTarget.get_hash(nTIndex);
		nIndex	= get_index(nHash);

		if(nIndex == -1)// ���� colum�߰�
		{
			add_column(_dstTarget.get_type_flag(nTIndex), _dstTarget.get_type_name(nTIndex), _dstTarget.get_type(nTIndex)
				, _dstTarget.get_type_size(nTIndex), _dstTarget.get_type_enum_state(nTIndex)
				, _dstTarget.get_type_comment(nTIndex));
		}
	}

	for(unsigned i=0; i<stlVKey.size(); i++)
	{
		int nKey	= stlVKey[i];
		replace(_dstTarget, nKey, nKey);
	}

	if(_pstlVMerageableKey)
	{
		stlVKey	= *_pstlVMerageableKey;

		for(unsigned i=0; i<stlVKey.size(); i++)
		{
			int nKey	= stlVKey[i];

			if(!is_exist(nKey))
				replace(_dstTarget, nKey, nKey);
		}
	}
}

BaseDStructureValue	BaseDStructureValue::sm_sample(NULL);

BaseDStructureValue::BaseDStructureValue(const BaseDStructure *_pstdBase, int _nDumpSize)
{
	m_option	= 0;
	init(_pstdBase, _nDumpSize);
}

BaseDStructureValue::~BaseDStructureValue()
{
	release();
}

void BaseDStructureValue::state_set_(const BaseDStructure* _pdstBase)
{
	m_pdstBase = _pdstBase;
}

void BaseDStructureValue::init(const BaseDStructure *_pstdBase, int _nDumpSize)
{
	m_pSpace		= NULL;
	m_pVoidParam	= NULL;
	m_nSequenceCurrent	= 0;
	m_nSequenceCurrentLocal	= 0;
	m_nBoolIndexCurrent		= 0;
	m_nSizeDump	= _nDumpSize;
	m_pVoid		= PT_Alloc(bbyte, _nDumpSize);
	m_pdstBase	= _pstdBase;
	m_nSize		= 0;
	m_bUpdated	= true;
	m_bComplete	= false;
	m_nRefCount	= 0;
	m_logevent_p = NULL;
	m_logger_link_p = NULL;

	static int s_serial = 9800000;
	m_serial = s_serial++;
}

void BaseDStructureValue::sfunc_set(const STLMnpBaseObject &_sfunc_m)
{
	m_sFunc_m = _sfunc_m;
	
	STLMnpBaseObject::const_iterator it;
	it = m_sFunc_m.begin();
	for(;it!=m_sFunc_m.end();it++)
		it->second->inc_ref((BaseObject*)this);
}

void BaseDStructureValue::sfunc_transit(BaseDStructureValue *_pdsv) const
{
	_pdsv->sfunc_set(m_sFunc_m);
}

void BaseDStructureValue::sfunc_clear(int _hash)
{
	STLMnpBaseObject::iterator it;
	it = m_sFunc_m.find(_hash);

	if (it == m_sFunc_m.end())
	{
		if (_hash == -1)
			m_sFunc_m.clear();
		return;
	}

	m_sFunc_m.erase(it);
}
void BaseDStructureValue::sfunc_clear()
{
	m_sFunc_m.clear();
}
void BaseDStructureValue::sfunc_set(int _hash, BaseObject *_fclass)
{
	BaseObject *pre = sfunc_get(_hash);
	if(pre)
		pre->dec_ref((BaseObject*)this);
	
	m_sFunc_m[_hash] = _fclass;
	_fclass->inc_ref((BaseObject*)this);
}

BaseObject *BaseDStructureValue::sfunc_get(int _hash)
{	
	STLMnpBaseObject::iterator it;
	if (_hash == 0)
	{
		it = m_sFunc_m.end();
		it = m_sFunc_m.begin();
	}
	else {
		it = m_sFunc_m.find(_hash);
	}
	if(it == m_sFunc_m.end())
		return NULL;
	return it->second;
}

int BaseDStructureValue::ref_inc()
{
	m_nRefCount++;
	return m_nRefCount;
}

int BaseDStructureValue::ref_get()
{
	return m_nRefCount;
}

int BaseDStructureValue::ref_dec(void *_pStateMain)
{
	m_nRefCount--;
	if(m_nRefCount == 0)
	{
		if(_pStateMain)
		{
			BaseStateMain *pStateMain = (BaseStateMain*)_pStateMain;
			pStateMain->ContextRelease(this);
		}
		PT_OFree(this);
	}
	return m_nRefCount;
}

void BaseDStructureValue::reset_link()
{
	int nHash;
	int nCallType;
	for(unsigned i=0; i<m_stlVBool.size(); i++)
	{
		nHash	= m_stlVBool[i].nHash;
		nCallType	= m_stlVBool[i].nCallType;
		if((nCallType & BaseDStructure::STATE_LINK_START)!=0 
			|| (nCallType & BaseDStructure::STATE_START)!=0
			|| (nCallType & BaseDStructure::STATE_LINK_COMPLETE)!=0 
			|| (nCallType & BaseDStructure::STATE_LINK_APPEND)!=0)
		{
			// reset_link�� State�� �̹� Start�ǰ� �� �� �����̱� ������ 
			// Start�ÿ� �ʱ�ȭ�� Bool�� �״�� �д�
		}else{
			m_stlVBool[i].bResult	= false;
		}
	}

	m_bUpdated	= true;
	m_bComplete	= false;
}

int BaseDStructureValue::compare(const BaseDStructureValue &_dsvTarget, int _nSkip) const
{
	int nRet=0;

	short	nCnt1, nCnt2;
	const void*	pValue1;
	const void*	pValue2;
	int nIndex2;
	int nCntEqual	= 0;

	int nHashLink	= STRTOHASH(RES_STR_ARRAYLINK);
	int nHashKey	= STRTOHASH(RES_STR_KEY);

	for(int nSequence=0; nSequence<get_count(); nSequence++)
	{
		int nIndex1	= m_pdstBase->get_index(get_colum(nSequence));
		int nSize	= m_pdstBase->get_type_size(nIndex1);
		int nHash	= m_pdstBase->get_hash(nIndex1);
		int nType	= m_pdstBase->get_type(nIndex1);

		if(_nSkip == 1 && nHash == nHashKey)
			continue; // _nSkip�� 1�� ��� Key�� ������ �ʴ´�. Link�� ��� Key�� �ٸ� �� �ִ�

		if(nHash == nHashLink) // Link���� ���� ���̶� �ٸ� �� �ִ�, �߰��� �� �Ǳ� ������ ������ ���� �ʴ´�
			continue;

		if(get(&pValue1, &nCnt1, nSequence))
		{	
			nIndex2	= _dsvTarget.get_base()->get_index(nHash);
			if(nIndex2 == -1)
				return BaseDStructure::COMPARE_CONFLICT_DEFINE;

			if(nIndex2 != _dsvTarget.get_base()->get_index(get_colum(nSequence)))
				return BaseDStructure::COMPARE_CONFLICT_DATA;

			int nType2	= _dsvTarget.get_base()->get_type(nIndex2);
			if(nType != nType2)
				return BaseDStructure::COMPARE_CONFLICT_DEFINE;

			if(_dsvTarget.get(&pValue2, &nCnt2, nSequence))
			{
				if(nCnt1 != nCnt2)
					return BaseDStructure::COMPARE_CONFLICT_DATA;

				if(memcmp(pValue1, pValue2, nSize*nCnt1) != 0)
					return BaseDStructure::COMPARE_CONFLICT_DATA;
				else
					nCntEqual++;
			}else{
				if(nRet < BaseDStructure::COMPARE_INCLUDE)
					nRet	= BaseDStructure::COMPARE_INCLUDE;
			}
		}else{
			nIndex2	= _dsvTarget.get_base()->get_index(nHash);
			if(nIndex2 != -1)
			{
				if(_dsvTarget.get_by_index(nIndex2, &pValue2, &nCnt2))
				{
					if(nRet < BaseDStructure::COMPARE_MERAGEABLE)
						nRet	= BaseDStructure::COMPARE_MERAGEABLE;
				}
			}
		}
	}

	if(nRet	< BaseDStructure::COMPARE_MERAGEABLE && get_count() < _dsvTarget.get_count())
		nRet	= BaseDStructure::COMPARE_MERAGEABLE;

	return nRet;
}

bool BaseDStructureValue::regist_event_func(BaseStateSpace *_pSpace)
{
	bool	bRet	= true;
	m_pSpace	= _pSpace;
	m_stlVBool.clear();
//#ifdef _DEBUG
	m_stlVDebugProcessorName.clear();
//#endif

	const int *pnFalse;

	if (get(HASH_STATE(BaseFalse_nIf), (const void**)&pnFalse))
	{
		if (*pnFalse == 1)
		{
			m_bUpdated = true;
			m_bComplete = false;

			return false;
		}
	}

	if (get(HASH_STATE(BaseDebugHideLog), (const void**)& pnFalse))
		md_LogHide = true;
	else
		md_LogHide = false;

	m_bIsGotUpdateLink	= false;

	if(m_pdstBase)
	{
		int nFlagFunctionOperator	 = (BaseDStructure::STATE_EVENT	| BaseDStructure::STATE_LINK_START
								| BaseDStructure::STATE_LINK_APPEND	| BaseDStructure::STATE_UPDATE);
		STLMnInt	stlMnSequenceLocalCount;
		STLMnInt::iterator	itSequenceLocal;
		int nSize	= get_count();
		for(int i=0; i<nSize; i++)
		{
			int nHash	= get_colum(i);

			itSequenceLocal	= stlMnSequenceLocalCount.find(nHash);
			if(itSequenceLocal == stlMnSequenceLocalCount.end())
				stlMnSequenceLocalCount[nHash]	= 0;
			else
				stlMnSequenceLocalCount[nHash]++;

			short nIndex	= get_index(nHash);
			int	nType	= 0;
			fnEventProcessor	fnFunc	= NULL;// Add or Update by OJ 2012-09-11 ���� 11:42:02

			STLMnpFuncEventProcessor::const_iterator	it;
            it	= BaseDStructure::sm_stlMnpEventProcessor.find(nHash);
			if(it != BaseDStructure::sm_stlMnpEventProcessor.end())
			{
				fnFunc	= it->second;
				STLMnInt::const_iterator	itType;
				itType	= BaseDStructure::sm_stlMnnEventCallType.find(nHash);
				nType	= itType->second;
			}else{
				nType	= m_pdstBase->get_type_flag(nIndex) & BaseDStructure::STATE_CALLTYPE;// Add or Update by OJ 2012-09-11 ���� 11:44:33
			}

			if(fnFunc == NULL 
				&& (nType & nFlagFunctionOperator) == 0)
			{
				continue;// Add or Update by OJ 2012-09-11 ���� 12:08:45
			}

			if(fnFunc == NULL)
			{
                g_SendMessage(LOG_MSG, "regist_event_func: fail %d\n", nHash);
				bRet	= false;// Add or Update by OJ 2012-09-11 ���� 1:56:41
				continue;
			}
			
			if((nType & BaseDStructure::STATE_UPDATE)!=0)
				m_bIsGotUpdateLink	= true;

			ST_ColumnBool	stBool;
			stBool.nHash	= nHash;
			stBool.bResult	= false;
			stBool.nSequence	= i;
			stBool.nCallType	= nType;
			stBool.nSequenceLocal	= stlMnSequenceLocalCount[nHash];
			stBool.fnProcessor	= fnFunc;

			if(	((nType & BaseDStructure::STATE_LINK_COMPLETE)!=0 
				|| (nType & BaseDStructure::STATE_LINK_APPEND)!=0)
				)
				stBool.bResult	= true;
//#ifdef _DEBUG
			m_stlVDebugProcessorName.push_back(BaseDStructure::sm_stlMnDebugProcessorName.find(nHash)->second);
//#endif
			m_stlVBool.push_back(stBool);

			//==================================================================================================
			// Start Add or Update by OJ 2012-08-01 ���� 10:29:19
			STStateActor actor;
	
			actor.nKey		= stBool.nHash;
			
			{
				const int *pnInt;
				int nIndex2 = get_index(actor.nKey);
				int nSize2 = m_pdstBase->get_type_size(nIndex2);
				if(nSize2 == sizeof(int) 
					&& get(actor.nKey, (const void**)&pnInt, NULL, stBool.nSequenceLocal))
					actor.nValue = *pnInt;
				else
				{
					const char *strString;
					if(//m_pdstBase->get_type(nIndex) == BaseDStructure::TYPE_STRING
						actor.nKey == HASH_STATE(FlashUIEvent_strRE)
						&& get(actor.nKey, (const void**)&strString, NULL, stBool.nSequenceLocal))
					{
						actor.nValue = STRTOHASH(strString);
					}else
						actor.nValue = 0;
				}
			}

			if(actor.nKey	== HASH_STATE(HASH_BaseTimeOut_nREU)
				|| actor.nKey == HASH_STATE(HASH_BaseTimeOut_varUIf))
			{
				UINT32	nTimeCur = BaseSystem::timeGetTime();

				{
					BaseState *pState = (BaseState*)m_pVoidParam;
					char strBuffer[255];
					const int *timeVar = 0;
					if (actor.nKey == HASH_STATE(HASH_BaseTimeOut_varUIf))
					{
						if (!pState->variable_get()->get(actor.nValue, (const void**)&timeVar))
							continue;
						actor.nValue = *timeVar;
					}
					
					sprintf_s(strBuffer, 255, "TimeRegist TimeCur:%d TimeOut:%d", nTimeCur, actor.nValue);
					pState->DebugOutString(NULL, this, strBuffer);
				}
				actor.nType		= STStateActor::TIMEOUT;

				INT64 nTimeout = (INT64)actor.nValue;
				nTimeout = nTimeout + (INT64)nTimeCur;
				UINT32 nMax = -1;
				if(nTimeout > (INT64)nMax)
				{
					g_SendMessage(LOG_MSG_POPUP, "Time out rang over");
					continue;
				}
				actor.nValue	+= nTimeCur;
			}else if(stBool.nCallType & BaseDStructure::STATE_UPDATE)
			{
				actor.nType	= STStateActor::UPDATE;
			}else if(stBool.nCallType & BaseDStructure::STATE_EVENT){
				actor.nType	= STStateActor::HASH;
			}else{
				actor.nType	= STStateActor::NO_ACTOR; // START, END ���� Actor Event�� ������� �ʴ´�
			}

			if(actor.nType != STStateActor::NO_ACTOR)
			{
				actor.nSpaceID		= ((BaseState*)m_pVoidParam)->get_space()->space_id_get();
				actor.pdstActor		= this;
				actor.pState		= (BaseState*)m_pVoidParam;
				actor.nBoolIndex	= (int)m_stlVBool.size()-1;
				actor.nPriority = 0;
				const int* priority;
				if (get(HASH_STATE(BaseEventPriority_nV), (const void**)&priority))
					actor.nPriority = *priority;
				actor.nLinkIndex = 0;
				const int* linkindex;
				if (get(HASH_STATE(BaseLinkIndex_nV), (const void**)&linkindex))
					actor.nLinkIndex = *linkindex;
				m_pSpace->actor_event_regist(actor);
			}
			// End by OJ 2012-08-01 ���� 10:29:23
			//==================================================================================================
		}
	}
	m_bUpdated	= true;
	m_bComplete	= false;

	return bRet;
}

void BaseDStructureValue::set_clear()
{
	m_nSize = 0;
	m_stlVValueAppend.clear();
}

void BaseDStructureValue::release()
{
	for(unsigned i=0; i<m_stlVBool.size(); i++)
	{
		STStateActor actor;

		actor.nKey		= m_stlVBool[i].nHash;
		if(actor.nKey	== HASH_STATE(HASH_BaseTimeOut_nREU)
			|| actor.nKey == HASH_STATE(HASH_BaseTimeOut_varUIf))
		{
			actor.nType		= STStateActor::TIMEOUT;
		}else if(m_stlVBool[i].nCallType & BaseDStructure::STATE_UPDATE)
		{
			actor.nType	= STStateActor::UPDATE;
		}else if(m_stlVBool[i].nCallType & BaseDStructure::STATE_EVENT){
			actor.nType	= STStateActor::HASH;
		}else{
			actor.nType	= STStateActor::NO_ACTOR; // START, END ���� Actor Event�� ������� �ʴ´�
		}

		if(actor.nType != STStateActor::NO_ACTOR)
		{
			actor.pState		= (BaseState*)m_pVoidParam;
			actor.nSpaceID		= actor.pState->get_space()->space_id_get();
			actor.pdstActor		= this;
			actor.nBoolIndex	= i;

			if (m_pSpace && !m_pSpace->actor_event_release(actor))
				m_pSpace->actor_event_release(this);
		}
	}
	m_stlVBool.clear();

	if(m_pVoid)
		PT_Free(m_pVoid);
	m_pVoid = NULL;
	m_nSize = 0;
	m_stlVValueAppend.clear();

	if (m_logevent_p)
		PT_OFree(m_logevent_p);
	m_logevent_p = NULL;
	
	STLMnpBaseObject::iterator it;
	it = m_sFunc_m.begin();
	for(;it!=m_sFunc_m.end();it++)
		it->second->dec_ref((BaseObject*)this);
	m_sFunc_m.clear();
}

bool	BaseDStructureValue::dump_save(const char *_strFilename) const
{
	int nSize = get_dumppacket_size();
	void *pDump = PT_Alloc(char, nSize);
	nSize = get_dumppacket(pDump, nSize);
	
	BaseFile file;

	if (file.OpenFile(_strFilename, BaseFile::OPEN_WRITE))
		return false;

	file.Write(pDump, nSize);
	file.CloseFile();

	return true;
}

void BaseDStructureValue::dump_refragment(int _nSizeOfDump)
{
	int nSizeOfDump = _nSizeOfDump;

	char *pData = PT_Alloc(char, nSizeOfDump);
	char *pDataTemp = pData;

	int		nHash;
	short	nIndex;
	int	nSize;
	for (unsigned i = 0; i<m_stlVValueAppend.size(); i++)
	{
		char *pColumn, *pColumnFrom, *pColumnTo;
		nHash = m_stlVValueAppend[i].nHash;
		nIndex = m_pdstBase->get_index(nHash);
		nSize = m_pdstBase->get_type_size(nIndex);
		bbyte nType = m_pdstBase->get_type(nIndex);
		B_ASSERT(nType != TYPE_BOOL2);

		pColumn = (char*)m_stlVValueAppend[i].pColumn;

		BaseSystem::memcpy(pDataTemp, &nIndex, 2);	pDataTemp += 2;// Index�� ���� �ִ´�.
		m_stlVValueAppend[i].pColumn = pDataTemp; // ���ο� �޸𸮷� �����͸� �̵��Ѵ�.
		if (nType >= TYPE_STRING)// STRING���� ū Ÿ���� ����� ���ϱ� ������ �պκп� shortũ�⸸ŭ size�� ������ �ִ�
		{
			BaseSystem::memcpy(&nSize, pColumn, sizeof(short));
			nSize += sizeof(short);

			pColumnFrom = pColumn + 2;
			pColumnTo = pDataTemp + 2;
		}
		else {
			pColumnFrom = pColumn;
			pColumnTo = pDataTemp;
		}
		BaseSystem::memcpy(pDataTemp, pColumn, nSize);
		pDataTemp += nSize;

		////==================================================================================================
		//// Start Add or Update by OJ 2012-08-17 ���� 5:37:17
		//// Fast Access Point�� �ٽ� �����Ѵ�
		//if (!m_stlVpFastAccess.empty())
		//{
		//	for (unsigned k = 0; k<m_stlVpFastAccess.size(); k++)
		//	{
		//		if (m_stlVpFastAccess[k] == pColumnFrom)
		//			m_stlVpFastAccess[k] = pColumnTo;
		//	}
		//}
		//// End by OJ 2012-08-17 ���� 5:37:18
		////==================================================================================================
	}

	if (m_pVoid)
		PT_Free(m_pVoid); // ������ ���� �޸� ������ ��� �Ű�� ������ �����
	m_pVoid = pData;	// ����� �޸𸮸� �ٽ� dump point�� �����Ѵ�.
	m_nSizeDump = nSizeOfDump;
}

bool	BaseDStructureValue::dump_load(void *_pManager, const char *_strFilename)
{
	void *pDump;
	BaseFile file;

	if (file.OpenFile(_strFilename, BaseFile::OPEN_READ))
		return false;

	UINT32 nSize = file.get_size_file();
	pDump = PT_Alloc(char, nSize);
	nSize = file.Read(pDump, nSize);
	if (!set_dumppacket(_pManager, pDump, nSize))
		return false;
	file.CloseFile();
	PT_Free(pDump);

	return true;
}

bool BaseDStructureValue::set_dumppacket(void *_pManager, void *_pDump, int _nSize, bool _bPacket)
{
	if(m_nSizeDump < _nSize)
		dump_make_space_(_nSize - m_nSize);

	int	nHash;
	int nSize = 0;

	bbyte bCnt;

	bbyte*pPacket = (bbyte*)_pDump;
	bCnt = *pPacket;
	nSize += 1;

	int nKey = 0;

	for (int i = 0; i < bCnt; i++)
	{
		nHash = *((int*)(pPacket + nSize));
		bbyte bType = *(pPacket + nSize + 4);

		if (i == 0)
			nKey = nHash;
		
		if(_pManager)
			((BaseStateManager*)_pManager)->variable_define(nHash, bType, true);

		//if(bType == BaseDStructure::TYPE_STRING)
		//	g_SendMessage(LOG_MSG, "Packet Receive Colum i:%d pos:%d data:%s type:%d Hash:%d"
		//		, i, nSize, (char*)(pPacket + nSize + 7), (int)bType, nHash);
		//else
		//	g_SendMessage(LOG_MSG, "Packet Receive Colum i:%d pos:%d data:%d type:%d Hash:%d"
		//		, i, nSize, (int)(*((int*)(pPacket + nSize + 7))), (int)bType, nHash);

		nSize += 5;
		short sCnt = *((short*)(pPacket + nSize));

		add_alloc(nHash, pPacket + nSize + 2, sCnt);
		int nIndex = get_base()->get_index(nHash);

		B_ASSERT(get_base()->get_type(nIndex) == bType);

		if (get_base()->get_type(nIndex) != bType)
			return false;

		if (nIndex > 0)
			nSize += sCnt * get_base()->get_type_size(nIndex) + 2;
		else
			return false;
	}

	if (nSize < _nSize && _bPacket == false)
	{
		nKey = *((int*)(pPacket + nSize));
	}

	if(nKey != 0)
		set_key(nKey);
	return true;
}

int BaseDStructureValue::get_dumppacket_size() const
{
	return m_nSize + ((int)m_stlVValueAppend.size()+1)*5 + 10;
}

int BaseDStructureValue::get_dump_size() const
{
	return m_nSize;
}

int BaseDStructureValue::get_dumppacket_column(int _nHash, bbyte*_pPacket, int _nSizeRemain, int _nSeq) const
{
	int nRetSize = 0;
	const BaseDStructure *pBase = get_base();
	if (5 >= _nSizeRemain)
		return 0;
	int nIndex = pBase->get_index(_nHash);

	if (nIndex == -1)
		return 0;

	bbyte nType = (bbyte)pBase->get_type(nIndex);
	
	*((int*)(_pPacket)) = _nHash;
	*(_pPacket + 4) = nType;
	nRetSize += 5;

	int nSizeData = pBase->get_type_size(nIndex);
	const bbyte*pData = NULL;
	short sCnt = 1;
	if (_nSeq != -1)
		get((const void**)&pData, &sCnt, _nSeq);
	else
		get(_nHash, (const void**)&pData, &sCnt);

	if (nRetSize + sCnt*nSizeData + 2 >= _nSizeRemain)
		return 0;

	BaseSystem::memcpy(_pPacket + nRetSize, &sCnt, 2);
	BaseSystem::memcpy(_pPacket + nRetSize + 2, pData, sCnt*nSizeData);

	//if (pData != NULL)
	//{
	//	if (nType == BaseDStructure::TYPE_STRING)
	//	{
	//		char strBuf[1024];
	//		//BaseSystem::memcpy_s(strBuf, 1024, pData, 128);
 //           BaseSystem::memcpy(strBuf, pData, 128);
	//		strBuf[128] = 0;
	//		g_SendMessage(LOG_MSG, "Packet Send Colum seq:%d date:%s type:%d Hash:%d", _nSeq, strBuf, (bbyte)nType, _nHash);
	//	}
	//	else {
	//		g_SendMessage(LOG_MSG, "Packet Send Colum seq:%d date:%d type:%d Hash:%d", _nSeq, (int)(*((int*)pData)), (bbyte)nType, _nHash);
	//	}
	//}

	nRetSize += sCnt*nSizeData + 2;

	return nRetSize;
}

int BaseDStructureValue::get_dumppacket(void *_pDump, int _nSize) const
{
	bbyte*pPacket = (bbyte*)_pDump;
	int	nHash;

	int i = 0, nSize = 0;
	bbyte bCount = 0;

	const BaseDStructure *pBase = get_base();

	nSize += 1; // for define count

	//BaseSystem::memcpy(pPacket + nSize, &m_nKey, 4);
	int nColSize = get_dumppacket_column(m_nKey, pPacket + nSize, _nSize - nSize, -1);
	if (nColSize > 0) {
		nSize += nColSize;
		bCount++;
	}
	
	for (; i<get_count_appended(); i++)
	{
		nHash = get_colum(i);
		int nIndex = pBase->get_index(nHash);
		if (nIndex != -1 && nHash != m_nKey)
		{
			nSize += get_dumppacket_column(nHash, pPacket + nSize, _nSize - nSize, i);
			bCount++;
		}
	}

	if (nColSize == 0) {
		*((int*)(pPacket + nSize)) = m_nKey;
		nSize += 4;
	}

	*pPacket = bCount;

	return nSize;
}

bool BaseDStructureValue::set_key(int _nKey)
{
	if(m_pdstBase == NULL)
		return false;

#ifdef _DEBUG
	if(m_pdstBase->is_exist(_nKey) == NULL)
	{
		g_SendMessage(LOG_MSG_POPUP, "StateSystem can't find Column: '%s' in State '%s'",
			HASHTOSTR(_nKey), m_pdstBase->get_name());
	}
#endif
	B_ASSERT(m_pdstBase->is_exist(_nKey));
	if (!m_pdstBase->is_exist(_nKey))
		return false;

	m_nKey	= _nKey;
	
	const char *strName;
	if(get_by_index(1, (const void**)&strName))
		strcpy_s(m_strDebugName, 255, strName);

	strcpy_s(m_strDebugComment, 1024, " ");
	
	if (get(HASH_STATE(HASH_Comment), (const void**)&strName))
		strcpy_s(m_strDebugComment, 1024, strName);
	return true;
}

#ifdef _DEBUG
void BaseDStructureValue::set_name_debug(const char *_strName)
{
		strcpy_s(m_strDebugName, 255, _strName);
}
#endif

int BaseDStructureValue::get_key() const
{
	return m_nKey;
}

void BaseDStructureValue::add_alloc(int _nHash, const void *_pVoid, short _nCount)
{
	B_ASSERT(this != NULL && m_pdstBase != NULL);
	if(this == NULL || m_pdstBase == NULL)
		return;

	int _nIndex;
	_nIndex	= get_index(_nHash);

	if(_nIndex < 0)
		return;
	B_ASSERT(_nIndex >= 0);
	
	int nSize	= m_pdstBase->get_type_size(_nIndex);
	bbyte nType	= m_pdstBase->get_type(_nIndex);
	if(nType == TYPE_BOOL2)
	{
		return;
	}

	bool	bValiable	= false;

	if(nType >= TYPE_STRING)
	{
		int	nColumnSize	= 4;

		if(nType == TYPE_STRING)
		{
			nSize = (int)strlen((const char*)_pVoid) + 3;
		}else if(nType >= TYPE_ARRAY)
		{
			nColumnSize	= BaseDStructure::sm_stlVTypeSize[nType];
			nSize	= _nCount * nColumnSize + 2;
		}
		B_ASSERT(nSize < LIMIT_STR + 4);
		bValiable	= true;
	}

	char *pData	= NULL;
	pData	= (char*)dump_make_space_(nSize+2);

	BaseSystem::memcpy(pData, &_nIndex, 2);
	pData	+= 2;
	m_nSize	= nSize+m_nSize+2;
	B_ASSERT(m_nSize <= m_nSizeDump);

	ST_BaseDColumn stDColumn;
	stDColumn.nHash	= _nHash;
	stDColumn.pColumn	= pData;
	m_stlVValueAppend.push_back(stDColumn);

	if(bValiable)
	{
		nSize	-= 2;
		BaseSystem::memcpy(pData, &nSize, 2);
		pData	+= 2;
	}
	BaseSystem::memcpy(pData, _pVoid, nSize);
}

bool BaseDStructureValue::set_mass(int _nHash, const void* _pVoid, int _nCnt)
{
	int nIndex = get_index(_nHash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	B_ASSERT(type >= TYPE_STRING);

	if (_nCnt > LIMIT_STR)
	{
		char* buf = PT_Alloc(char, LIMIT_STR + 1);
		const char* start = (const char*)_pVoid;
		for (int i = 0; i < _nCnt / LIMIT_STR; i++)
		{
			memcpy(buf, start, LIMIT_STR);
			buf[LIMIT_STR] = NULL;
			add_alloc(_nHash, (const void*)buf, LIMIT_STR);
			start = (const char*)_pVoid + (i + 1) * LIMIT_STR;
		}
		int mod = _nCnt % LIMIT_STR;
		if (mod > 0) {
			memcpy(buf, start, mod);
			buf[mod] = NULL;
			add_alloc(_nHash, (const void*)buf, mod);
		}

		PT_Free(buf);
	}
	else {
		add_alloc(_nHash, _pVoid, _nCnt);
	}
	return true;
}

bool BaseDStructureValue::set_mass(int _nHash, const void* _pVoid)
{
	int nIndex = get_index(_nHash);
	if (nIndex == -1)
		return false;
	int type = BaseDStructure::get_type(nIndex);
	B_ASSERT(type == TYPE_STRING);

	int len = (int)strlen((const char*)_pVoid);
	if (len > LIMIT_STR)
	{
		set_alloc(_nHash, NULL);
		char* buf = PT_Alloc(char, LIMIT_STR + 1);
		const char* start = (const char*)_pVoid;
		for (int i = 0; i < len / LIMIT_STR; i++)
		{
			memcpy(buf, start, LIMIT_STR);
			buf[LIMIT_STR] = NULL;
			add_alloc(_nHash, (const void*)buf);
			start = (const char*)_pVoid + (i + 1) * LIMIT_STR;
		}
		int mod = len % LIMIT_STR;
		if (mod > 0) {
			memcpy(buf, start, mod);
			buf[mod] = NULL;
			add_alloc(_nHash, (const void*)buf);
		}

		PT_Free(buf);
	}
	else {
		set_alloc(_nHash, _pVoid);
	}
	return true;
}

void BaseDStructureValue::set_alloc(int _nHash, const void *_pVoid, short _nCount)
{
	int _nIndex;
	_nIndex	= get_index(_nHash);

#ifdef _DEBUG
	if (_nHash == 1351175842
		&& _pVoid == NULL)
	{
		int x = 0;
		x++;
	}

#endif
	B_ASSERT(_nIndex >= 0);
	B_ASSERT(_nCount >= 0);

	if(_nIndex < 0)
		return;

	int nSize	= m_pdstBase->get_type_size(_nIndex);
	bbyte nType	= m_pdstBase->get_type(_nIndex);
	if(nType == TYPE_BOOL2)
	{
		//short shPos	= nSize;
		//void *pFlag = BaseDStructure::get_value_const_(&m_stlMnValueAppend, 0);
		//m_pdstBase->set_bool_(pFlag, shPos, *((bool*)_pVoid));
		return;
	}

	bool	bValiable	= false;

	if(nType >= TYPE_STRING)
	{
		int	nColumSize	= 4;

		if(nType == TYPE_STRING && _pVoid != NULL)
		{
			nSize	= (int)strlen((const char*)_pVoid)+3;
		}else if(nType >= TYPE_ARRAY)
		{
			nColumSize	= BaseDStructure::sm_stlVTypeSize[nType];
			nSize	= _nCount * nColumSize + 2;
		}

		B_ASSERT(nSize < LIMIT_STR + 4);
		bValiable	= true;
	}
	// _nIndex(2 bbyte) + nSize(If valiable, 2) + Data(nSize)

	// editing point for indexing : pointing by OJ 2010-08
	int nSequence	= -1;
	for(unsigned i=0; i<m_stlVValueAppend.size(); i++)
	{
		if (m_stlVValueAppend[i].nHash == _nHash)
		{
			nSequence = i;

			for (unsigned j = nSequence+1; j < m_stlVValueAppend.size(); j++)
			{
				if (m_stlVValueAppend[j].nHash == _nHash)
				{
					m_stlVValueAppend.erase(m_stlVValueAppend.begin() + j); // It's gonna remove reference for variable but can't return the memory for that space.
					j--;
				}
			}
			break;
		}
	}

	if (_pVoid == NULL)
	{
		if (nSequence >= 0)
		{
			m_stlVValueAppend.erase(m_stlVValueAppend.begin() + nSequence); // It's gonna remove reference for variable but can't return the memory for that space.
		}
		return;
	}

	if(!bValiable && nSequence != -1)
	{
		BaseSystem::memcpy(m_stlVValueAppend[nSequence].pColumn, _pVoid, nSize);
		return;
	}
	// checked point
	//char *pData	= (char*)m_pVoid;
	//pData	= ((char*)m_pVoid)+m_nSize;
	char *pData	= NULL;
	char* pbackup = NULL;
	if (_pVoid >= m_pVoid && _pVoid <= (char*)m_pVoid + m_nSizeDump)
	{
		pbackup = PT_Alloc(char, nSize+2);
		BaseSystem::memcpy(pbackup, _pVoid, nSize);
		_pVoid = pbackup;
	}
	pData	= (char*)dump_make_space_(nSize+2);

	BaseSystem::memcpy(pData, &_nIndex, 2);
	pData	+= 2;
	m_nSize	= nSize+m_nSize+2;
	B_ASSERT(m_nSize <= m_nSizeDump);
	if(nSequence == -1)
	{
		ST_BaseDColumn stDColumn;
		stDColumn.nHash	= _nHash;
		stDColumn.pColumn	= pData;
		m_stlVValueAppend.push_back(stDColumn);
	}else{
		m_stlVValueAppend[nSequence].pColumn	= pData;
	}

	if(bValiable)
	{
		nSize	-= 2;
		BaseSystem::memcpy(pData, &nSize, 2);
		pData	+= 2;
	}
	BaseSystem::memcpy(pData, _pVoid, nSize);
	if (pbackup)
		PT_Free(pbackup);
}

int	BaseDStructureValue::get_base_key()const
{
	return m_pdstBase->get_key();
}

short	BaseDStructureValue::get_index(int _nHash)const
{
	if (!dbg_mem_check())
	{
		g_SendMessage(LOG_MSG, "---error--- %d", *s_db_mem);
	}
	B_ASSERT(m_pdstBase != 0);
	return m_pdstBase->get_index(_nHash);
}

void BaseDStructure::set_callback_fileclose(fnCloseFile	_fnCallback)
{
	sm_callbackFileClose	= _fnCallback;
}

void BaseDStructure::set_callback_fileopen(fnOpenFile	_fnCallback)
{
	sm_callbackFileOpen	= _fnCallback;
}

const BaseDStructureValue &BaseDStructureValue::operator=(const BaseDStructureValue &_dsvRight)
{
	m_pVoidParam	= _dsvRight.m_pVoidParam;
	m_pdstBase		= _dsvRight.m_pdstBase;

	int nSize = _dsvRight.get_dumppacket_size();
	void *pDump = PT_Alloc(char, nSize);
	nSize = _dsvRight.get_dumppacket(pDump, nSize);
	set_dumppacket(NULL, pDump, nSize);
	PT_Free(pDump);

	m_stlVBool	= _dsvRight.m_stlVBool;
	m_bUpdated	= _dsvRight.m_bUpdated;
	m_bComplete	= _dsvRight.m_bComplete;

	return *this;
}

int	BaseDStructureValue::get_count(const char *_strHash, int *_pnIndex)const
{
	const void *pValue;
	short		nSize;
	int			nHash;
	int			nIndex;
	bbyte		nType;
	nHash	= UniqHashStr::get_string_hash_code(_strHash);
	nIndex	= get_index(nHash);
	if(_pnIndex)
		*_pnIndex	= nIndex;
	nType	= m_pdstBase->get_type(nIndex);
	if(get_by_index(nIndex, &pValue, &nSize))
	{
		return nSize;
	}
	return 0;
}

bool BaseDStructureValue::get_mass(int _nHash, const void** _ppVoid, int* _pnCount)const
{
	short nCnt = 0;
	STLVpVoid stlData;
	STLVInt stlLens;
	const char* strData = NULL;
	int seq = sequence_get_local();
	int index = get_index(_nHash);
	int type = BaseDStructure::get_type(index);
	bool isStr = type == TYPE_STRING;
	bool bRet = get_by_index(get_index(_nHash), (const void**)&strData, &nCnt, seq);

	if (bRet) {
		stlData.push_back((void*)strData);
		if (isStr)
			nCnt--;
		stlLens.push_back((int)nCnt);
		int l = (int)nCnt;//(int)strlen(strData);
		int len = l;
		while (l >= LIMIT_STR)
		{
			seq++;
			if (!get_by_index(get_index(_nHash), (const void**)&strData, &nCnt, seq))
				break;
			if (isStr)
				nCnt--;
			stlData.push_back((void*)strData);
			stlLens.push_back((int)nCnt);
			l = (int)nCnt;//(int)strlen(strData);
			len += l;
		}

		char* ret = PT_Alloc(char, len + 1);
		PT_AFree(ret);

		if (_pnCount)
			*_pnCount = len;

		for (int i = 0; i < (int)stlData.size(); i++) {
			int blockLen = stlLens[i];//(int)strlen((const char*)stlData[i]);
			//strcpy_s(ret + i * LIMIT_STR, blockLen + 1, (const char*)stlData[i]);
			memcpy(ret + i * LIMIT_STR, stlData[i], blockLen + 1);
		}
		*_ppVoid = ret;
	}
	return bRet;
}

bool BaseDStructureValue::get(int _nHash, const void **_ppVoid, short *_pnCount, int _nSequence)const
{
	if (m_nSize == -33686019)
	{
		mpool_get().leak_check(this);
		return false;
	}
	return get_by_index(get_index(_nHash), _ppVoid, _pnCount, _nSequence);
}

bool BaseDStructureValue::get(void *_pRet, short *pnCnt, int _nSequence)const
{
	const void *pValue;
	short		nSize;
	bbyte		nType;
	
	if(get(&pValue, &nSize, _nSequence))
	{
		int nIndex;
		nIndex	= m_pdstBase->get_index(get_colum(_nSequence));
		nType	= m_pdstBase->get_type(nIndex);

		if(pnCnt)
			*pnCnt	= nSize;

		int nSizeType;
		if(nType != TYPE_STRING)
			nSizeType	= BaseDStructure::sm_stlVTypeSize[nType];
		else
            {nSizeType	= 1; nSize++;}

		BaseSystem::memcpy(_pRet, pValue, nSize * nSizeType);// editing point for indexing : pointing by OJ 2010-08
		return true;
	}
	if(pnCnt)
		*pnCnt	= 0;

	return false;
}

void* BaseDStructureValue::get_point(int _hash, short* pnCnt, int _nSequence) const
{
    INT64 point_n;
    if (!get_by_index(get_index(_hash), &point_n, NULL, _nSequence))
        return NULL;

    return (void*)point_n;
}

void* BaseDStructureValue::get_point(const char* _strHash, short* pnCnt, int _nSequence) const
{
    return get_point(UniqHashStr::get_string_hash_code(_strHash), pnCnt, _nSequence);
}

bool BaseDStructureValue::get_local_seq(const char *_strHash, void *_pRet, short *pnCnt)const
{
	return get_by_index(get_index(UniqHashStr::get_string_hash_code(_strHash)), _pRet, pnCnt, m_nSequenceCurrentLocal);
}

bool BaseDStructureValue::get_local_seq(int _nHash, const void **_ppVoid, short *_pnCount) const
{
	return get_by_index(get_index(_nHash), _ppVoid, _pnCount, m_nSequenceCurrentLocal);
}

bool BaseDStructureValue::get_look_up(int _nHash, const void **_ppOut, short *_pnCntOut) const
{
	int nSequence = sequence_get();
	int nHash;

	do{
		nHash = get_colum(nSequence);
		
		if(nHash == _nHash)
			return get(_ppOut, _pnCntOut, nSequence);

		nSequence--;
	}while(nSequence >= 0);

	return false;
}

bool BaseDStructureValue::get(const char *_strHash, void *_pRet, short *pnCnt, int _nSequence)const
{
	return get_by_index(get_index(UniqHashStr::get_string_hash_code(_strHash)), _pRet, pnCnt, _nSequence);
}

bool BaseDStructureValue::is_get_flag(int _nFlag)const
{
	unsigned i=0;
	for(;i<m_stlVValueAppend.size();i++)
	{
		if((m_pdstBase->get_type_flag(m_pdstBase->get_index(m_stlVValueAppend[i].nHash))
			& _nFlag) != 0)
			return true;
	}

	STLMnstlRecord::const_iterator	itRecord;
	itRecord	= m_pdstBase->m_stlMnData.find(m_nKey);
	for(i=0; i<itRecord->second.size();i++)
	{
		if((m_pdstBase->get_type_flag(m_pdstBase->get_index(itRecord->second[i].nHash))
			& _nFlag) != 0)
			return true;
	}
	return false;
}

void BaseDStructureValue::add_alloc(const char *_strHash, const void *_pIn, short _nCnt, bbyte _nType)
{
	int nHash	= UniqHashStr::get_string_hash_code(_strHash);

	return add_alloc(nHash, _pIn, _nCnt);
}

void BaseDStructureValue::set_point(int hash, void* _pIn)
{
	INT64 point_n = (INT64)_pIn;
	set_alloc(hash, (void*)&point_n);
}

void BaseDStructureValue::set_point(const char* _strHash, void* _pIn)
{
	INT64 point_n = (INT64)_pIn;
	set_alloc(_strHash, (void*)&point_n);
}

void BaseDStructureValue::set_alloc(const char *_strHash, const void *_pIn, short _nCnt, bbyte _nType)
{
	int nHash	= UniqHashStr::get_string_hash_code(_strHash);

	return set_alloc(nHash, _pIn, _nCnt);
}

int BaseDStructureValue::function_hash_get() const
{
	return m_function_hash;
}

int	BaseDStructureValue::sequence_get() const
{
	return m_nSequenceCurrent;
}

int BaseDStructureValue::bool_index_get() const
{
	return m_nBoolIndexCurrent;
}

int BaseDStructureValue::sequence_get_local() const
{
	return m_nSequenceCurrentLocal;
}

bool BaseDStructureValue::get_param(const char *_strHash, void *_pRet, short *_pnCount, int _nStartSequence)const
{
	return get_param(STRTOHASH(_strHash), _pRet, _pnCount, _nStartSequence);
}

bool BaseDStructureValue::get_param(int _nHash, void *_pRet, short *_pnCount, int _nStartSequence)const
{
	const void *pValue;
	short nSize;
	if (get_param(_nHash, &pValue, &nSize, _nStartSequence))
	{
		if (_pnCount)
			*_pnCount = nSize;

		int nSizeType, nType, nIndex;

		nIndex = get_index(_nHash);
		nType = BaseDStructure::get_type(nIndex);
		
		if (nType != TYPE_STRING)
			nSizeType = BaseDStructure::sm_stlVTypeSize[nType];
		else
        {nSizeType = 1; nSize++;}

		BaseSystem::memcpy(_pRet, pValue, nSize * nSizeType);// editing point for indexing : pointing by OJ 2010-08
		return true;
	}
	return false;
}

bool BaseDStructureValue::get_param(int _nHash, const void **_ppVoid, short *_pnCount, int _nIndexStartSequence) const
{
	if (m_pdstBase == NULL)
		return false;

	int nIndex;

	nIndex = m_pdstBase->get_index(_nHash);

	if (nIndex >= (int)m_pdstBase->get_type_count()
		|| nIndex < 0)
		return false;

	bbyte nType = m_pdstBase->get_type(nIndex);
	B_ASSERT(nType != TYPE_BOOL2);

	if (_nIndexStartSequence == 0)
		_nIndexStartSequence = m_nSequenceCurrent;

	*_ppVoid = NULL;

	if (_nIndexStartSequence < (int)m_stlVValueAppend.size())
	{
		if (_pnCount)
			*_pnCount = 0;
		*_ppVoid = m_pdstBase->get_param_(&m_stlVValueAppend, _nHash, _nIndexStartSequence);

		if (nType >= TYPE_STRING && *_ppVoid) // return �Ǿ� �� ���� ���̰� ���ϴ� ���� ��
		{
			char **ppStr;
			ppStr = (char**)_ppVoid;
			if (_pnCount)
			{
				BaseSystem::memcpy(_pnCount, *_ppVoid, sizeof(short));
				if (m_pdstBase->sm_stlVTypeSize[nType] > 0)
					*_pnCount /= m_pdstBase->sm_stlVTypeSize[nType];
			}
			*ppStr += 2;
			return true;
		}

		if (*_ppVoid)
		{
			if (_pnCount)
				*_pnCount = 1;
			return true;
		}
	}

	int nStartSeq = (int)(_nIndexStartSequence - m_stlVValueAppend.size());
	if (nStartSeq < 0)
		nStartSeq = 0;

	return m_pdstBase->get_param(m_nKey, nIndex, _ppVoid, _pnCount, nStartSeq);
}

int	BaseDStructureValue::get_mass_size(int _hash) const
{
#ifdef _DEBUG
	B_ASSERT(m_pdstBase->get_index(_hash) > 0);
#endif
	return m_pdstBase->get_mass_size(&m_stlVValueAppend, _hash);
}

bool BaseDStructureValue::get_by_index(int _nIndex, const void **_ppVoid, short *_pnCount, int _nSequence) const
{
	if(m_pdstBase == NULL)
		return false;
    
	if(_nIndex >= (int)m_pdstBase->get_type_count()
		|| _nIndex < 0)
		return false;

	//short nSize	= m_pdstBase->get_type_size(_nIndex);
	bbyte nType	= m_pdstBase->get_type(_nIndex);
	B_ASSERT(nType != TYPE_BOOL2);

	*_ppVoid	= NULL;
	if(_pnCount)
		*_pnCount	= 0;
	*_ppVoid	= m_pdstBase->get_value_(&m_stlVValueAppend, m_pdstBase->get_hash(_nIndex), _nSequence);
    
	if(nType >= TYPE_STRING && *_ppVoid) // return �Ǿ� �� ���� ���̰� ���ϴ� ���� ��
	{
		char **ppStr;
		ppStr	= (char**)_ppVoid;
		if(_pnCount)
		{
			BaseSystem::memcpy(_pnCount, *_ppVoid, sizeof(short));
			if(m_pdstBase->sm_stlVTypeSize[nType]>0)
				*_pnCount	/= m_pdstBase->sm_stlVTypeSize[nType];
		}
		*ppStr	+= 2;
		return true;
	}

	if(*_ppVoid)
	{
		if(_pnCount)
			*_pnCount	= 1;
		return true;
	}

	return m_pdstBase->get(m_nKey, _nIndex, _ppVoid, _pnCount, _nSequence);
}

bool BaseDStructureValue::get_by_index(int _nIndex, void *_pRet, short *pnCnt, int _nSequence)const
{
	if(_nIndex == -1)
		return false;

	const void *pValue;
	short		nSize;
	bbyte		nType;
	nType	= m_pdstBase->get_type(_nIndex);
	if(get_by_index(_nIndex, &pValue, &nSize, _nSequence))
	{
		if(pnCnt)
			*pnCnt	= nSize;

		int nSizeType;
		if(nType != TYPE_STRING)
			nSizeType	= BaseDStructure::sm_stlVTypeSize[nType];
        else{
            nSizeType	= 1; nSize++;
        }
		int cpyLen = nSize;
		BaseSystem::memcpy(_pRet, pValue, cpyLen * nSizeType);// editing point for indexing : pointing by OJ 2010-08
		return true;
	}
	if(pnCnt)
		*pnCnt	= 0;
	return false;
}

bool BaseDStructureValue::get(const void **_ppVoid, short *_pnCount, int _nSequence)const
{
	if(m_pdstBase == NULL)
		return false;

	if(_nSequence >= get_count()
		|| _nSequence < 0)
		return false;

	int nIndex	= m_pdstBase->get_index(get_colum(_nSequence));
	//short nSize	= m_pdstBase->get_type_size(nIndex);
	bbyte nType	= m_pdstBase->get_type(nIndex);
	B_ASSERT(nType != TYPE_BOOL2);

	*_ppVoid	= NULL;
	if(_pnCount)
		*_pnCount	= 0;

	if(_nSequence < (int)m_stlVValueAppend.size())
		*_ppVoid	= m_stlVValueAppend[_nSequence].pColumn;

	if(nType >= TYPE_STRING && *_ppVoid) // return �Ǿ� �� ���� ���̰� ���ϴ� ���� ��
	{
		char **ppStr;
		ppStr	= (char**)_ppVoid;
		if(_pnCount)
		{
			BaseSystem::memcpy(_pnCount, *_ppVoid, sizeof(short));
			if(m_pdstBase->sm_stlVTypeSize[nType]>0)
				*_pnCount	/= m_pdstBase->sm_stlVTypeSize[nType];
		}
		*ppStr	+= 2;
		return true;
	}

	if(*_ppVoid)
	{
		if(_pnCount)
			*_pnCount	= 1;
		return true;
	}

	return m_pdstBase->get(m_nKey, _ppVoid, _pnCount, (int)(_nSequence - m_stlVValueAppend.size()));
}

int	BaseDStructureValue::get_count_appended() const
{
	return (int)m_stlVValueAppend.size();
}

int BaseDStructureValue::get_count() const
{
	int nCount	= 0;

	nCount	+= (int)m_stlVValueAppend.size();

	STLMnstlRecord::const_iterator	itRecord;
	itRecord	= m_pdstBase->m_stlMnData.find(m_nKey);
	if (itRecord == m_pdstBase->m_stlMnData.end())
		return nCount;
	nCount	+= (int)itRecord->second.size();
	return nCount;
}

const void *BaseDStructureValue::dbg_mem_checkin_hash(int _seq) const
{
	const void*ret = &m_stlVValueAppend[_seq].nHash;
	dbg_mem_checkin(ret);
	return ret;
}

int BaseDStructureValue::get_colum(int _nSequence)const
{
	STLMnstlRecord::const_iterator	itRecord;
	itRecord	= m_pdstBase->m_stlMnData.find(m_nKey);

	INT32 nIndexMaxBase;
	nIndexMaxBase	= (INT32)m_stlVValueAppend.size();
	if (_nSequence < nIndexMaxBase)
	{
		return m_stlVValueAppend[_nSequence].nHash;
	}

	if(_nSequence >= (int)itRecord->second.size() + nIndexMaxBase)
		return 0;

	return itRecord->second[_nSequence - nIndexMaxBase].nHash;
}

void* BaseDStructureValue::dump_make_space_(unsigned short _nSizeMore)
{//
	if(m_nSize + _nSizeMore <= m_nSizeDump)
		return ((char*)m_pVoid)+m_nSize;

	int nSizeOfDump	= m_nSizeDump*2;
	if(nSizeOfDump < 8)//
		nSizeOfDump	= 8;

	while (nSizeOfDump < m_nSize + _nSizeMore)
	{
		if (nSizeOfDump >= LIMIT_STR)
		{
			nSizeOfDump += LIMIT_STR * 2;
		}else
			nSizeOfDump *= 2;
	}

	dump_refragment(nSizeOfDump);

	return ((char*)m_pVoid)+m_nSize;
}

const int *BaseDStructureValue::param_int_get(const BaseDStructureValue *_pdsvBase)
{
	const int *pnRet = NULL;
	const int *pnHash = NULL;
	if (!_pdsvBase->get_param(HASH_STATE(BaseVariableRefer_anV), (const void**)&pnHash))
		return NULL;
	if (!get(*pnHash, (const void**)&pnRet))
		return NULL;
	return pnRet;
}

const char	*BaseDStructureValue::param_str_get(const BaseDStructureValue *_pdsvBase)
{
	const char *strRet = NULL;
	const char *strHash = NULL;
	if(!_pdsvBase->get_param(HASH_STATE(BaseVariableRefer_strV), (const void**)&strHash))
		if(!_pdsvBase->get_param(HASH_STATE(BaseVariableString_strV), (const void**)&strHash))
			return NULL;
	int nHash = STRTOHASH(strHash);
	if (!get(nHash, (const void**)&strRet))
		return NULL;
	return strRet;
}

bool BaseDStructureValue::set_value_log_(const void* _value_p, int _hash_n)
{
	int index_n = m_pdstBase->get_index(_hash_n);
	int type_n = m_pdstBase->get_type(index_n);

	short cnt_s = 1;
	const char* str_p = (const char*)_value_p;
	char* buff = NULL;
	if (type_n >= TYPE_STRING)
	{
		BaseSystem::memcpy(&cnt_s, _value_p, sizeof(short));
		if (m_pdstBase->sm_stlVTypeSize[type_n] > 0)
			cnt_s /= m_pdstBase->sm_stlVTypeSize[type_n];

		str_p += 2;

		if (cnt_s > 100)
		{
			buff = PT_Alloc(char, 101);
			BaseSystem::memcpy(buff, str_p, 100);
			*(buff + 100) = 0;
			cnt_s = 100;
			str_p = buff;
		}
	}
	set_alloc(_hash_n, str_p, cnt_s);

	if (buff)
		PT_Free(buff);

	return TRUE;
}

bool BaseDStructureValue::set_value_(const void* _value_p, int _hash_n)
{
	int index_n = m_pdstBase->get_index(_hash_n);
	int type_n = m_pdstBase->get_type(index_n);

	short cnt_s = 1;
	const char* str_p = (const char*)_value_p;
	if (type_n >= TYPE_STRING)
	{
		BaseSystem::memcpy(&cnt_s, _value_p, sizeof(short));
		if (m_pdstBase->sm_stlVTypeSize[type_n] > 0)
			cnt_s /= m_pdstBase->sm_stlVTypeSize[type_n];

		str_p += 2;
	}
	set_alloc(_hash_n, str_p, cnt_s);
	return TRUE;
}


bool BaseDStructureValue::add_value_(const void* _value_p, int _hash_n)
{
	int index_n = m_pdstBase->get_index(_hash_n);
	int type_n = m_pdstBase->get_type(index_n);

	short cnt_s = 1;
	const char* str_p = (const char*)_value_p;
	if (type_n >= TYPE_STRING)
	{
		BaseSystem::memcpy(&cnt_s, _value_p, sizeof(short));
		if (m_pdstBase->sm_stlVTypeSize[type_n] > 0)
			cnt_s /= m_pdstBase->sm_stlVTypeSize[type_n];

		str_p += 2;
	}
	add_alloc(_hash_n, str_p, cnt_s);
	return TRUE;
}


void BaseDStructureValue::variable_copy_log(BaseDStructureValue* _dsv_p) const
{
	// Start =============================================
	// add by oj 2019/10/17
	// All of current variable will be transit to next state(add, call, transit)
	// but variable what is exist in the next variable(reserved) will not transit, mean the next variable(reserved) got hiegh priority than current variable.
	const char* value_p = NULL;

	int hash_n = 0;
	for (unsigned i = 0; i < m_stlVValueAppend.size(); i++)
	{
		hash_n = m_stlVValueAppend[i].nHash;
		value_p = (const char*)m_stlVValueAppend[i].pColumn;
		_dsv_p->set_value_log_(value_p, hash_n);
	}
	// End =============================================

}

void BaseDStructureValue::variable_copy(BaseDStructureValue* _dsv_p) const
{
	// Start =============================================
	// add by oj 2019/10/17
	// All of current variable will be transit to next state(add, call, transit)
	// but variable what is exist in the next variable(reserved) will not transit, mean the next variable(reserved) got hiegh priority than current variable.
	const char* value_p = NULL;

	int hash_n = 0;
	for (unsigned i = 0; i < m_stlVValueAppend.size(); i++)
	{
		hash_n = m_stlVValueAppend[i].nHash;
		value_p = (const char*)m_stlVValueAppend[i].pColumn;
		_dsv_p->set_value_(value_p, hash_n);
	}
	// End =============================================

}

void BaseDStructureValue::variable_transit(BaseDStructureValue *_dsv_p) const
{
	// Start =============================================
	// add by oj 2019/10/17
	// All of current variable will be transit to next state(add, call, transit)
	// but variable what is exist in the next variable(reserved) will not transit, mean the next variable(reserved) got hiegh priority than current variable.
	const char* value_p = NULL;

	STLSInt newHashs;
	int hash_n = 0;
	//_dsv_p->set_clear();
	for (unsigned i = 0; i < m_stlVValueAppend.size(); i++)
	{
		hash_n = m_stlVValueAppend[i].nHash;
		if (!_dsv_p->get(hash_n, (const void**)&value_p))
		{
			value_p = (const char*)m_stlVValueAppend[i].pColumn;
			_dsv_p->set_value_(value_p, hash_n);
			newHashs.insert(hash_n);
		}
		else {
			STLSInt::iterator it = newHashs.find(hash_n);
			if (it != newHashs.end())
			{
				value_p = (const char*)m_stlVValueAppend[i].pColumn;
				_dsv_p->add_value_(value_p, hash_n);
			}
		}
	}
	
	// End =============================================
}

bool	BaseDStructureValue::param_str_set(const BaseDStructureValue *_pdsvBase, const char *_str)
{
	const char *strHash = NULL;
	if (!_pdsvBase->get_param(HASH_STATE(BaseVariableRefer_strV), (const void**)&strHash))
		if (!_pdsvBase->get_param(HASH_STATE(BaseVariableString_strV), (const void**)&strHash))
			return false;
	int nHash = STRTOHASH(strHash);
	set_alloc(nHash, _str);
	return true;
}

bool	BaseDStructureValue::param_int_set(const BaseDStructureValue *_pdsvBase, int _nValue)
{
	const int *pnHash = NULL;
	if (!_pdsvBase->get_param(HASH_STATE(BaseVariableRefer_anV), (const void**)&pnHash))
		return false;
	set_alloc(*pnHash, &_nValue);
	return true;
}

void BaseDStructureValue::logger_state_event_cast(int _key, int _serial)
{
	m_logger_event_cast[_key] = _serial;
}

void BaseDStructureValue::logger_state_event_receive(int _key, int _serial)
{
	m_logger_event_receive[_key] = _serial;
}

void BaseDStructureValue::logger_link_set(const BaseDStructureValue* _link_p)
{
	m_logger_link_p = _link_p;
}

const BaseDStructureValue* BaseDStructureValue::logger_link_get() // if this is variable, it is variable in variable operation. (context, state, global etc)
{
	return m_logger_link_p;
}

BaseDStructureValue* BaseDStructureValue::logger_new(const char* _str)
{
	BaseDStructureValue* event = NULL;
	BaseDStructure* pdstEvent = BaseStateManager::get_manager()->EnumGet(HASH_STATE(HASH_EnumEvent));
	if (pdstEvent == NULL)// StateManager destoried.
		return NULL;
	PT_OAlloc2(event, BaseDStructureValue, pdstEvent, 1024);
	//OutputDebugString(_str);
	//OutputDebugString(m_strDebugComment);
	//mpool_get().observe_push(event);
	int event_hash = HASH_STATE(BaseStateEventGlobal);
	event->set_key(event_hash);
#ifdef _DEBUG
	event->set_name_debug(_str);
#endif
	return event;
}

void BaseDStructureValue::logger_hand_result(int _index, int _result)
{
	if (g_logger())
	{
		for (int i = 0; i < m_logger_column_result.size(); i += 2)
		{
			if (m_logger_column_result[i] == _index)
			{
				m_logger_column_result[i + 1] = _result;
				return;
			}
		}

		m_logger_column_result.push_back(_index);
		m_logger_column_result.push_back(_result);
	}
}

void BaseDStructureValue::logger_hand_variable(int _hash, const void* _value, short _cnt)
{
	if (g_logger())
	{
		if (_hash == HASH_STATE(RevGroupStart)
			|| _hash == HASH_STATE(RevGroupEnd))
			return;

		add_alloc(_hash, _value, _cnt);
	}
}

void BaseDStructureValue::logger_reset(const char * _strCmt)
{
	if (m_logevent_p)
	{
		PT_OFree(m_logevent_p);
	}

	m_logger_column_result.clear();
	STLString str = m_strDebugName;
	str += _strCmt;
	m_logevent_p = logger_new(str.c_str());
	int start = 0;
	m_logevent_p->set_alloc(HASH_STATE(RevGroupStart), (const void*)&start);;
}

void BaseDStructureValue::logger_send(int _event_index, int _keyMain, int _key, int _state_serial, int _link, const STLMnInt &_groupID_m, int _event_serial)
{
	if (g_logger())
	{
		static int hash[50] = { 0 }, event_index_msg = 11, start_log_state = 7, logger_serial = 100000;
		static int type[50];
		static BaseStateManager* s_mp = NULL;
		// marker multi thread
		BaseStateManager* mp = BaseStateManager::get_manager();

		if (hash[0] == 0)
		{
			int count = 0;
			hash[count] = STRTOHASH("VSLoggerStateKey_nV");			type[count++] = TYPE_INT32;	// 0 state key
			hash[count] = STRTOHASH("VSLoggerLinkKey_nV");			type[count++] = TYPE_INT32;	// 1 link key
			hash[count] = STRTOHASH("VSLoggerGroup_anV");			type[count++] = TYPE_ARRAY_INT32;	// 2 group type, id
			hash[count] = STRTOHASH("VSLoggerColumnResult_anV"); type[count++] = TYPE_ARRAY_INT32;	// 3 column function result
			hash[count] = STRTOHASH("VSLoggerEventCast_anV");		type[count++] = TYPE_ARRAY_INT32;	// 4 event cast =>event, serial
			hash[count] = STRTOHASH("VSLoggerSerial_nV");				type[count++] = TYPE_INT32;	// 5 logger serial
			hash[count] = STRTOHASH("VSLoggerStateSerial_nV");		type[count++] = TYPE_INT32;	// 6 state serial
			hash[count] = STRTOHASH("VSLoggerEventReceive_anV");type[count++] = TYPE_INT32;	// 7 event receive =>event, serial
			hash[count] = STRTOHASH("VSLoggerMainKey_nV");			type[count++] = TYPE_INT32;		// 8 main state key
			hash[count] = STRTOHASH("VSLoggerEventSerial_nV");			type[count++] = TYPE_INT32;		// 9 event serial
			start_log_state = count;
			hash[count++] = STRTOHASH("VSLoggerOnStart");	 // start ignore staties
			hash[count++] = STRTOHASH("VSLoggerOffStart");
			hash[count++] = STRTOHASH("VSLoggerClientSocket");
			hash[count++] = STRTOHASH("VSLoggerSession");
			hash[count++] = STRTOHASH("VSHistoryStateOn");
			hash[count++] = STRTOHASH("VSHistoryStateOff");
			hash[count++] = STRTOHASH("VSHistorySession");
			hash[count++] = STRTOHASH("UxPointNone");
			hash[count++] = STRTOHASH("UxPointPushed");
			hash[count++] = STRTOHASH("UxPointHoldWait");
			hash[count++] = STRTOHASH("UxPointMoving");
			hash[count++] = STRTOHASH("VLogMonitor");
			hash[count++] = STRTOHASH("VLogStateOn");
			hash[count++] = STRTOHASH("VLogStateOff");
			
			//hash[count++] = STRTOHASH("EditObjFocusCheck");
			event_index_msg = count;
			hash[event_index_msg + TYPE_Logger_start] = STRTOHASH("VSLoggerStateOn");
			hash[event_index_msg + TYPE_Logger_end] = STRTOHASH("VSLoggerStateOff");
			hash[event_index_msg + TYPE_Logger_dummy] = STRTOHASH("VSLoggerStateDummy");
			hash[event_index_msg + TYPE_Logger_fail] = STRTOHASH("VSLoggerStateLinkFail");
		}

		if(BaseDStructure::get_index(hash[5]) == -1)
		//if (mp != s_mp)
		{// s_mp is chagned when rerun on the editor
			for(int i=0; i<start_log_state; i++)
				mp->variable_define(hash[i], type[i], true);

			s_mp = mp;
		}

		//if(_event_index != TYPE_Logger_end)
			for (int i = start_log_state; i < event_index_msg; i++)
				if (hash[i] == _key)
				{
					logger_reset("logger_send_1");
					return;
				}

		int event_hash = hash[_event_index + event_index_msg];

		BaseDStructureValue* log_backup = NULL;

		if (!m_logevent_p)
			logger_reset("logger_send_2");

		if (_event_index == TYPE_Logger_fail) {
			log_backup = logger_new("logger_send_22");
			m_logevent_p->variable_copy_log(log_backup);
		}

		m_logevent_p->set_alloc(hash[0], (const void*)&_key);
		m_logevent_p->set_alloc(hash[1], (const void*)&_link);
		m_logevent_p->set_alloc(hash[5], (const void*)&logger_serial); logger_serial++;
		m_logevent_p->set_alloc(hash[6], (const void*)&_state_serial);
		m_logevent_p->set_alloc(hash[8], (const void*)&_keyMain);
		m_logevent_p->set_alloc(hash[9], (const void*)&_event_serial);

		//4,7
		BaseState::group_id_set(m_logevent_p, hash[4], m_logger_event_cast);
		BaseState::group_id_set(m_logevent_p, hash[7], m_logger_event_receive);
		if(m_logger_column_result.size() > 0)
			m_logevent_p->set_alloc(hash[3], (const void*) &m_logger_column_result[0], (short)m_logger_column_result.size());
		BaseState::group_id_set(m_logevent_p, hash[2], _groupID_m);
		m_logevent_p->set_alloc(HASH_STATE(RevGroupEnd), (const void*)&_link);

		m_logevent_p->set_alloc(m_logevent_p->get_key(), &event_hash);
		mp->post_systemevent(m_logevent_p);

		if (log_backup)
		{
			m_logevent_p = log_backup;
		}
		else {
			m_logevent_p = NULL;
			logger_reset("logger_send_3");
		}
	}
}

void BaseDStructureValue::dbg_mem_checkin(const void* _data) const
{
	s_db_mem = (int*)_data;
	s_dbg_mem_value = *((int*)_data);
}

bool BaseDStructureValue::dbg_mem_check() const
{
	if (s_db_mem && s_dbg_mem_value != *(s_db_mem))
		return false;
	return true;
}

void BaseDStructureValue::dbg_mem_checkout() const
{
	s_db_mem = NULL;
}
