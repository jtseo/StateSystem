#pragma once
#include "../PtBase/BaseStateFunc.h"

struct MHD_Daemon;

class BaseSCHttpD :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(listen_astrF),
        STDEF_SFENUM(serviceReturn_varF),
        STDEF_SFENUM(listenStop_nF),
        STDEF_SFENUM(sessionClose_varF),
        STDEF_SFENUM(APIRegist_strF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCHttpD();
    ~BaseSCHttpD();

    PtObjectHeaderInheritance(BaseSCHttpD);
    STDEF_SC_HEADER(BaseSCHttpD);

    virtual int Create();
    int listen_astrF();
    int serviceReturn_varF();
    int listenStop_nF();
    int sessionClose_varF();
    int APIRegist_strF();
    //#SF_FuncHeaderInsert

    // User defin area from here
	struct MHD_Daemon *damonGet(){ return m_daemon;}
	void damonSet(struct MHD_Daemon *_demon) { m_daemon = _demon; }
	void threadStop() { m_threadrunning = false; }
protected:
    // support for text analysis
public:
    void *sessionDisconnectTop();
    void sessionDissconnectPop();
    bool apiCheck(const char* _api);
protected:
	static DEF_ThreadCallBack(update);
	
	STLString	m_server;
	int			m_port;
	bool		m_threadrunning;
	struct MHD_Daemon *m_daemon;
	STLVpVoid	m_sessions;
    STLSString  m_apiSet;
    
    BaseCircleQueue m_disconnectionQueue;
};

