#pragma once
#include "BaseObject.h"
#include <list>

class BaseDStructureValue;
class BaseCircleQueue;

class BaseEvent
{
public:
	int					m_nPriority;
	int					m_nSequence;
	BaseDStructureValue	*m_pdsvEvent;
	friend bool operator<(const BaseEvent &eventLeft, const BaseEvent &eventRight); // sort by process time
};
#ifdef _WIN32
typedef std::vector<BaseEvent, PT_allocator<BaseEvent> >				STLVEvent;
#else
typedef std::vector<BaseEvent>				STLVEvent;
#endif

class BaseStateManagerLogic; 

class BaseEventManager : public BaseObject
{
public:
	PtObjectHeaderInheritance(BaseEventManager);
	BaseEventManager();
	//void init();
	virtual ~BaseEventManager(void);
	virtual int dec_ref(BaseObject *_obj_p);

	void push_event_local(BaseDStructureValue *_pEvent, int _nPriority=0);
	void push_event(BaseDStructureValue *_pEvent, int _nPriority=0);
	BaseDStructureValue *get_top(unsigned _nTime);
	void pop_event();

	static void break_befor_propogate(BaseDStructureValue *_pEvent);
	static void break_befor_true(BaseDStructureValue *_pEvent);
	//static BaseEventManager *get(BaseStateManagerLogic *_manager_p);
protected:
	STLVEvent	m_stlVEvent;
	unsigned int	m_nSequence;
	
	BaseCircleQueue	*m_pstlCircleQueue; // support for multithread

	static STLMnString	m_stlMnStrBeforPropogate;
	static STLMnString	m_stlMnStrCast;
	static STLMnString	m_stlMnStrTrue;
	//BaseStateManagerLogic *m_pStateManager;
	//static BaseEventManager *ms_manager;
};
