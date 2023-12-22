#pragma once
#include "BaseObject.h"

class BaseResManager;
class BaseResResource;

typedef void (*_CallbackNotifierForFinish) (void *_pParam, int _nMsg, BaseResResource *_res_p);

class BaseResResource :
	public BaseObject
{
public:
	BaseResResource();
public:
	virtual ~BaseResResource(void);

	enum{
		STATE_NULL,
		STATE_WAIT,
		STATE_LOADING,
		STATE_FINISH,
		STATE_FAIL
	};

	enum {
		OBJECTID_DSTRUCTURE = OBJECTID_RES_CUSTOM + 1,
		OBJECTID_RES_FILE
	};
	
	virtual void Init(BaseResManager *_pManager);
	virtual bool SetLoad(const char *_strFilename, float _fPriority, const char *_strGroupName);
	virtual bool Load() = 0; // Load In Thread
	virtual bool Load2() {return true;} // Load In Single Thread
	virtual void Clear() = 0;

	void SetCallbackNotifier(_CallbackNotifierForFinish _pFunction, void *_pParam);

	float	GetPriority() { 
		return m_fPriority;
	}
	void SetPriority(float _fPriority){
		m_fPriority	= _fPriority;
	}

	void Finish(int _nMsg);

	const char *GetFileName()
	{
		return m_strName.c_str();
	}

	const char *GetGroupName()
	{
		return m_strGroupName.c_str();
	}

	int GetState(){
		return m_nState;
	}

	int get_key_filename(){
		return m_nKeyFilename;
	}

	int get_key_group(){
		return m_nKeyGroup;
	}

protected:
	int			m_nState;
	STLString	m_strGroupName;
	float		m_fPriority;

	_CallbackNotifierForFinish	m_pCallbackNotifier;
	void				*m_pCallbackParam;

	BaseResManager	*m_pManager;

	int			m_nKeyFilename;
	int			m_nKeyGroup;
};
