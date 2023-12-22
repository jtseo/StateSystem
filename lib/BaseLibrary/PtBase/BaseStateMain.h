#pragma once
#include "BaseObject.h"

class BaseStateManager;
class BaseStateSpace;
class BaseState;

class BaseResDStructure;

typedef std::vector<BaseState*, PT_allocator<BaseState*> >	STLVpState;
typedef struct __std_reserved_state__{
    int nStateKey;
    STLMnInt stlMnGroupId;
} STDEF_StateReserved;
typedef std::vector<STDEF_StateReserved, PT_allocator<STDEF_StateReserved> > STLVStateReserved;
typedef std::vector<STLMnInt, PT_allocator<STLMnInt> >  STLVstlMnInt;

typedef struct __stdef_state_record__{
    BaseDStructureValue *pStateValue;
    STLMnInt    stlMnGroupId;
} STDEF_StateRecord;
typedef std::vector<STDEF_StateRecord, PT_allocator<STDEF_StateRecord> > STLVStateRecord;

class StateTableGroupId{
public:
    BaseDStructureValue *get(int _nIndex);
    BaseDStructureValue *get(const STLMnInt &_stlMnGroupId);
    bool get_group_id(int _nIndex, STLMnInt *_stlMnGroupId);
    void add(BaseDStructureValue *_pState, const STLMnInt &_stlMnGroup);
    void set(BaseDStructureValue *_pState, const STLMnInt &_stlMnGroup);
    void set(BaseDStructureValue *_pState, int _nIndex);
    void clear_state();
    void clear();
    
protected:
    STLVStateRecord m_stlVStateRecord;
};

class BaseStateMain : public BaseObject//, public PTAllocT<BaseStateMain>
{
public:
	BaseStateMain(BaseStateSpace *_pManager, const char *_strName);
	virtual ~BaseStateMain(void);
	static BaseStateMain	sm_sample;

	bool init(BaseStateSpace *_pManager, const char *_strName);
	void release();

	virtual void	OnUpdate(unsigned _nTimeDelta);
	int StateStart();
	/** \brief Release a state on the state list.
	*
	* \par Purpose:
	*
	* \param *_pState :(in) new state.
	* \return 
	* \author oj
	* \date2005-11-02
	*/
	virtual int StateRelease(BaseDStructureValue *_pEvent);

	/** \brief Add a state on the state list.
	*
	* \par Purpose:
	*
	* \param *_pState :(in) new state.
	* \return 
	* \author oj
	* \date2005-11-02
	*/
	virtual int StateAdd(BaseDStructureValue *_pEvent);

	/** \brief Transit to the _nStateID.
	*
	* \par Purpose:
	* terminate current state and then move to the _nStateID.
	*
	* \param _nStateID 
	* \return 
	* \author oj
	* \date2005-11-02
	*/
	virtual int StateTransit(BaseDStructureValue *_pEvent);

	/** \brief Drawing Handler
	*
	* \par Purpose:
	*
	* \return 
	* \author oj
	* \date2005-11-02
	*/
	virtual int OnEvent(BaseDStructureValue *_pEvent);

	virtual int OnDraw();
	
	/** \brief Destory Handler
	*
	* \par Purpose:
	* When this program terminated, this fuction is called.
	*
	* \return 
	* \author oj
	* \date2005-11-02
	*/
	virtual int Destory();
	/** \brief Get state pointer by ID.
	*
	* \param _nID 
	* \return state point
	* \author oj
	* \date2005-11-02
	*/
	//================================================================
	bool GetState(STLVpVoid *_pstlVpStateActive, const STLMnInt *_pstlMnGroupId = 0, int _nSerial = 0);
	BaseState *GetStateSerial(int _nObjSerialState);
	virtual BaseState *GetState(const BaseState *_pState);
	virtual BaseState *GetStateParent(const BaseState *_pState);
	virtual BaseState *GetStateName(int _nKeyName, const STLMnInt *_pstlMnGroupId = 0, int _nSerial = 0);
	virtual BaseState *GetStateIdentifier(int _nKeyName, const STLMnInt &_stlMnGroupId, int _nSerial = 0);
    BaseDStructureValue *GetContext(const STLMnInt &_stlMnGroupId);
	void ContextRelease(BaseDStructureValue *_pdsvContext);
	BaseDStructureValue *GetContextReserved(const STLMnInt &_stlMnGroupId);
	BaseDStructureValue *GetContextReservedRef(const STLMnInt &_stlMnGroupId);
	void	AddContextReserved(const STLMnInt &_stlMnGroupId, BaseDStructureValue *_pdsvContext);
	void state_active_show_all(void);
protected:
    STLVstlMnInt    m_stlVReservedCntxGroupId;
	STLVpdstValue	m_stlVpReservedCntx; // Make Context before to start the State.
	//===================================================================
public:

	virtual int	get_key();

	BaseDStructure	*get_state();
	BaseDStructure	*get_link();

	const STLVpState &get_active_state();
	bool is_active(int _nHash, const STLMnInt *_pstlMnGroupId = NULL);
	bool is_active_id(const STLMnInt *_pstlMnGroupId, int _nHash = 0);
	void out_active(BaseState *_pState);
	void in_active(BaseState *_pState);

	bool restart();
	void reload();
	void reload_start();
	STLString get_state_filename(){
		return m_strRecordName;
	}

	bool is_conformer();
	//==========================================================================
	// Start Update by OJ : 2010-11-11
public:
	void grade_close(int _nGrade, BaseDStructureValue *_pEvent);
	void reset_to_reload();

protected:
	void grade_close_();
	int	m_nGradCloseTarget;// lower or equal will release after update or event process
	//================================= End Update by OJ : 2010-11-11

	//==========================================================================
	// Start Update by OJ : 2014-02-25
public:
	void next_active_state_add(int _nKey, STLMnInt &_stlMnGroupId);
	bool next_active_state_check(int _nKey, STLMnInt &_stlMnGroupId);
	bool next_active_state_del(int _nKey, STLMnInt &_stlMnGroupId);
protected:
    int _next_active_state_find(int _nKey, STLMnInt &_stlMnGroupId);
    STLVStateReserved   m_stlVResvActive;   // list Active State for next turn
	//================================= End Update by OJ : 2014-02-25
	
protected:

	bool load_(const char* _strName);
	bool load_re_(const char *_strName);
	bool variableLocalDefine();

	int	m_nKeyName;
	BaseResDStructure	*m_pResDstMain;
	BaseResDStructure	*m_pResDstLink;
	BaseDStructure	*m_pdstMain;
	BaseDStructure	*m_pdstLink;
	STLVpState		m_stlVpStateActive;

	STLVInt			m_stlVRecordActive;
	//STLVInt			m_stlVRecordIdentify;
    StateTableGroupId    m_StateTable;  // update by oj 2017-08-10
	STLString		m_strRecordName;
	bool			m_bReloading;
	bool			m_bConformer;

	BaseStateSpace	*m_pSpace;
	void release_(BaseState *_pState);

	void DebugOutString(const BaseDStructureValue *_pdst,const BaseDStructureValue *_pdstLink, const char *_strState);
};

