#include "stdafx.h"
#include "BaseResResource.h"
#include "BaseResManager.h"

BaseResResource::BaseResResource()
{
	m_nState	= STATE_NULL;
	m_fPriority	= 0;

	m_pCallbackNotifier	= NULL;
}

void BaseResResource::Init(BaseResManager *_pManager)
{
	m_pManager	= _pManager;
}

BaseResResource::~BaseResResource(void)
{
	//Clear();
}

bool BaseResResource::SetLoad(const char *_strFilename, float _fPriority, const char *_strGroupName)
{
	m_strName	= _strFilename;
	if(_strGroupName)
		m_strGroupName	= _strGroupName;
	m_fPriority		= _fPriority;
#ifdef TEST_MODULE
	printf("SetLoad %s\n", _strFilename);
#endif

	m_nKeyFilename = m_pManager->get_key(_strFilename, _strGroupName);

	if(m_nKeyFilename == 0)
		return false;

	m_nKeyGroup = 0;
	if(_strGroupName)
	{
		m_nKeyGroup =  m_pManager->get_key("/group/", _strGroupName);
	}

	return true;
}

void BaseResResource::SetCallbackNotifier(_CallbackNotifierForFinish _pFunction, void *_pParam)
{
	m_pCallbackNotifier	= _pFunction;
	m_pCallbackParam	= _pParam;
}

void BaseResResource::Finish(int _nMsg)
{
	m_nState	= _nMsg;

	if (m_pCallbackNotifier
		&& (m_nState == STATE_FINISH || m_nState == STATE_FAIL))
	{
		m_pCallbackNotifier(m_pCallbackParam, _nMsg, this);
		m_pCallbackNotifier = NULL; // block to multi call for notification
	}
}