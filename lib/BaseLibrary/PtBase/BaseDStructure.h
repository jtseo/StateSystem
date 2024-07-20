#pragma once

typedef unsigned char	bbyte;


#define VECTOR_COLUM

#define RES_STR_ARRAYLINK	"arrayLink"
#define RES_STR_NAME		"strName"
#define RES_STR_KEY			"nKey"
#define RES_STR_TIMEOUT		"BaseTimeOut_nREU"
#define RES_STR_TIMEPROCESS	"nTimeToProcess"

#define LIMIT_STR	32766

typedef struct __BaseDStructureCOLUM__
{
	int	nHash;
	void *pColumn;
}ST_BaseDColumn; 
#ifdef _WIN32
typedef std::vector<ST_BaseDColumn, PT_allocator<ST_BaseDColumn> >	STLVColumn;// Add by OJ : 2011-01-11
typedef std::map<int, STLVColumn, std::less<int>, PT_allocator<std::pair<const int, STLVColumn> > >		STLMnstlRecord;// Add by OJ : 2011-01-11
typedef std::vector<STLVColumn*, PT_allocator<STLVColumn*> >	STLVpstlRecord;
typedef std::vector<STLVpstlRecord*, PT_allocator<STLVpstlRecord*> >	STLVpstlIndexs;
#else
typedef std::vector<ST_BaseDColumn>	STLVColumn;// Add by OJ : 2011-01-11
typedef std::map<int, STLVColumn>		STLMnstlRecord;// Add by OJ : 2011-01-11
typedef std::vector<STLVColumn*>	STLVpstlRecord;
typedef std::vector<STLVpstlRecord*>	STLVpstlIndexs;
#endif
class BaseDStructure;
class BaseDStructureValue;
class BaseFile;

typedef int (*fnEventProcessor)(const BaseDStructureValue *_pdstBase, BaseDStructureValue *_pdstEvent, BaseDStructureValue *_pdstContext, int _nState);
typedef BaseFile *(*fnOpenFile)(const char *_strFilename);
typedef void (*fnCloseFile)(void *pFile);

typedef struct{
	short	nIndex;			// sequence
	int		nHash;			// hash value for strName
	int		nFlag;			// NULL: Client, R: Relay Server, L: Local Server, G: Global server
	char	strName[255];
	unsigned char	nType;			// Data Type(Bool, Int, Int63, String ...)
	short	nSize;			// Size of data, Bool has bit index and String is 0
	char	strEnumState[255];	// If you want to use enum value for this, you can use StateName for them.
	char	strComment[255];	// just comment
} STypeDefine;
typedef std::map<int, fnEventProcessor, std::less<int>, PT_allocator<std::pair<const int, fnEventProcessor> > >	STLMnpFuncEventProcessor;
typedef std::map<int, BaseDStructure*, std::less<int>, PT_allocator<std::pair<const int, BaseDStructure*> > >	STLMnpdst;
typedef std::vector<STypeDefine, PT_allocator<STypeDefine> >	STLVTypeDefine;
typedef std::map<int, short, std::less<int>, PT_allocator<std::pair<const int, short> > >	STLMnnTypeDefine;
typedef bool(*fnVariableDefine)(int _nHash, int _nType, bool _bForce);

class BaseDStructureValue;
class BaseStateSpace;

typedef enum {
	TYPE_NULL = -1,
	TYPE_BOOL2 = 1,
	TYPE_BYTE,
	TYPE_SHORT,
	TYPE_INT32,
	TYPE_INT64,
	TYPE_FLOAT,
	TYPE_VECTOR3,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_ARRAY_BYTE = TYPE_ARRAY,
	TYPE_ARRAY_SHORT,
	TYPE_ARRAY_INT32,
	TYPE_ARRAY_INT64,
	TYPE_ARRAY_FLOAT,
	TYPE_ARRAY_VECTOR3,
	TYPE_MAX
} EDstType;

class BaseDStructure// : public PTAllocT<BaseDStructure>
{
public:
	
	static	BaseDStructure	sm_sample;
	static char	sm_user[255];
	BaseDStructure();
	~BaseDStructure();
	void init();
	static void init_type();
	void release();
	void release_all_data();

	//int		del_colum(const char *_strName);
	static int		add_column(int _nFlag, int _nColumnHash, bbyte _nType, short _nSize = 0, const char *_strState = NULL, const char *_strComment = NULL, const char *_strColumnName = NULL);
	static int		add_column(int _nFlag, const char *_strName, bbyte _nType, short _nSize=0, const char *_strState=NULL, const char *_strComment=NULL);	

	enum{
		COMPARE_EQUAL,
		COMPARE_INCLUDE,
		COMPARE_MERAGEABLE,
		COMPARE_CONFLICT_DEFINE,
		COMPARE_CONFLICT_DATA,
	};

	void	get_keys(STLVInt *_pstlVConflictKey) const;
	int		compare(const BaseDStructure &_dstTarget, STLVInt *_pstlVConflictKey=NULL, STLVInt *_pstlVBerageableKey=NULL) const;
	void	replace(const BaseDStructure &_dstTarget, const STLVInt *_pstlVConflictKey=NULL, const STLVInt *_pstlVMerageableKey=NULL);
	void	replace(const BaseDStructure &_dstTarget, int _nKeyFrom, int _nKeyTo=0);
	STLMnstlRecord::iterator it_begin_get()
	{
		return m_stlMnData.begin();
	}
	STLMnstlRecord::iterator it_end_get()
	{
		return m_stlMnData.end();
	}

	//void	merage(const BaseDStructure &_dstTarget, int _nKeyFrom, int _nKeyTo=0);

    const char *get_filename() const;
	const char *get_name() const;
	int		get_key()const;
	void	set_name(const char *_strName);
	bool	is_define(const char *_strName);
	bool	load(const char *_strName = NULL, int _bType = 0);// _bType = 1: Save ini files,
	bool	load_define_append(const char *_strName);
	bool	save(const char *_strName=NULL, int _bType = 0);// _bType = 1: Save ini files, _bType = 2: pass to save '.define' file

	bool	type_original_check(int _nHash) const;
	static short	get_index(int _nHash);
	static int	get_hash(int _nIndex);
	bool		release(int _nKey);

	void		add_row_alloc(int _nKey, int _nIndex, const void *_pVoid, short _nCount=0);
	int			get_index(int _nKey, int _nSequence);
	bool		get(int _nKey, const void **_ppVoid, short *_pnCount=NULL, int _nIndexSequence=0) const;
	bool		get_param(int _nKey, int _nIndex, const void **_ppVoid, short *_pnCount = NULL, int _nIndexStartSequence = 0) const;
	bool		get(int _nKey, int _nIndex, const void **_ppVoid, short *_pnCount=NULL, int _nIndexSequence=0) const;
	bool		get_string(int _nKey, char *_strRet, int _nSequence);
	bool		add_string(int _nKey, int _nIndex, const char *_strIn);
	void		set_alloc(int _nKey, int _nIndex, const void *_pVoid, short _nCount=0);	// create memory, copy and record the position
	void		set(int _nKey, int _nIndex, void *_pVoid);		// just record the point, it can make memory error.
	bool		get_by_hash(int _nKey, int _nHash, const void **_ppVoid) const;
	bool		get(int _nKey, int _nIndex, bool *pbBool) const;	

	bool		release_rows(int _nKey);

	bool		is_exist(int _nKey)const;	// check exist value
	bool		change_key(int _nKey, int _nKeyto);

	int			get_count_colum(int _nKey) const;
	size_t		get_count() const;
	int			get_type_count() const;
	static int			get_type_flag(int _nIndex);
	static bool		set_type_flag(int _nIndex, int _nFlag);
	static const char *get_type_name(int _nIndex);
	static EDstType		get_type(int _nIndex);
	static int			get_type_size(int _nIndex);
	const char *get_type_enum_state(int _nIndex) const;
	const char *get_type_comment(int _nIndex) const;
	bool		set_type_enum(int _nIndex, const char *_strEnum);
	bool		set_type_comment(int _nIndex, const char *_strComment);

	int			get_first_key(STLMnstlRecord::iterator *_pit);
	int			get_next_key(STLMnstlRecord::iterator *_pit);
	
	static int		get_flag(const char *_strFlag);
	void	get_flag(char *_strFlag, int _nFlag);

	int		create_key_link(const char *_key_str);

	bool	is_exist_event_processor(const char *_strName);
	static bool	set_event_processor(const char *_strName, fnEventProcessor	_pfuncEventProc, const char *_file, int _line, int _nCallType	= 
		1);
		//BaseDStructureValue::STATE_UPDATE);

	enum{
		DST_EVENT_KEY,
		DST_EVENT_NAME,
		DST_EVENT_TARGET,
		DST_EVENT_INDEX,
		DST_EVENT_PARAM1,
		DST_EVENT_PARAM2,
	};

	enum{
		LINK_LOCAL			= 0x00000001,	// 'L'
		LINK_CONFORM		= 0x00000002,	// 'C'
		VALUE_CONTEXT		= 0x00000004,	// 'T'
		STATE_CONFORMTYPE	= 0x0000000F,

		STATE_CALLTYPE		= 0xFFFFFFF0,
		STATE_UPDATE		= 0x00000010,	// '?_SU'
		STATE_EVENT			= 0x00000800,	// '?_SE'
		STATE_START			= 0x00000020,	// '?_SS'
		STATE_END			= 0x00000040,	// '?_SD'
		STATE_SUSPEND		= 0x00000080,	// '?_SP'
		STATE_RESUME		= 0x00000400,	// '?_SR'
		STATE_LINK_COMPLETE	= 0x00000100,	// '?_LC'
		STATE_LINK_START	= 0x00000200,	// '?_LS'
		STATE_LINK_APPEND	= 0x00001000,	// '?_LA'
		STATE_LINK_INVERSE	= 0x00010000	// '?_IN'	// Inverse
	};

	friend	class BaseDStructureValue;

	static std::vector<int>		sm_stlVTypeSize;
	static STLVString           sm_stlVTypeName;
	static bbyte	get_type_by_name_(const char *_strName);

	static fnOpenFile	sm_callbackFileOpen;
	static fnCloseFile	sm_callbackFileClose;
	static void set_callback_fileopen(fnOpenFile	_fnCallback);
	static void set_callback_fileclose(fnCloseFile	_fnCallback);
	static bool	load_define_(const char* _strFilename, int _type);
    static bool load_statefuncclass(const char *_basefile, const char *_path, const char *_filename);
    static bool load_statefuncclass_(BaseFile *_pfile, const char *_classname);
protected:
	void	get_bool_(const void *_pFlag, short _nIndex, bool *_pbBool)const;
	void	set_bool_(void *_pFlag, short _nIndex, bool _bBool) const;
	int		get_row_sequence_(const STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence=0) const;
	void	*get_value_(STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence=0);
	static void *get_param_(const STLVColumn	*_pstlVColumn, int _nHash, int _nIndexStartSequence = 0);
	const void	*get_value_(const STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence=0)const;
	int get_mass_size(const STLVColumn* _pstlVColumn, int _hash) const;
	static void	*get_value_const_(STLVColumn	*_pstlVColumn, int _nHash, int _nIndexSequence=0);

	void	set_(STLVColumn	*_pstlVColumn, int _nIndex, void *_pVoid);
	void	add_row_(STLVColumn	*_pstlVColumn, int _nIndex, void *_pVoid);
	void	update_linktype_(STLVColumn	*_pstlVColumn);
	short	get_size_(int _nHash);
	static short	get_size_type_(bbyte _nType);
		
	void	sort_();
	//void	set_(int _nKey, int _nIndex, void *_pVoid);

	bool	save_define_(const char *_strFilename);
	bool	save_index_(const char *_strFilename, short _nIndex);
	bool	save_data_(const char *_strFilename, int _type = 0);
	bool	save_data_asc_(const char *_strFilename);

	bool	load_index_(const char *_strFilename, short _nIndex);
	bool	load_data_(const char *_strFilename);
	static bool	load_define_history(const char *_history);
	static bool	load_define_(BaseFile *_pFileDefine);
	static bool	load_define2_(BaseFile *_pFileDefine);
	bool	load_data_(BaseFile *_pFileData);
	bool	load_data_asc_(const char *_strFilename);

	STLVpVoid		m_stlVpAllocList;
	STLMnstlRecord	m_stlMnData;
public:
	static STLMnInt					sm_defineHistory_m;
	static STLMnInt					sm_stlMnnEventCallType;
	static STLMnpFuncEventProcessor	sm_stlMnpEventProcessor;
	static int					sm_nDefineOriginalSize;
	
	static STypeDefine type_get(int _index);
	static size_t type_length();
	static bool type_find(int _hash, short *_index_p);
protected:
	static STLVTypeDefine		sm_stlVTypedef;
	static STLMnnTypeDefine		sm_stlMnnTypedef;
	static STLVString			sm_stlVClasses;
	
public:
	static void classesSet(const char *_list);
	void variable_defines_for_enum(void *_pManager);
    static void static_clear();
	static void processor_list_add(const char *_strName, fnEventProcessor _fProc, const char *_file, int _line);
	static void processor_init();

protected:
//#ifdef _DEBUG
	static STLMnString	sm_stlMnDebugProcessorName;
	static STLMnString	sm_stlMnDebugProcessorFile;
	static STLMnInt	sm_stlMnDebugProcessorLine;
//#endif
	bool			m_bRet;

	char	m_strName[255];
	int		m_nKey;	

	PT_OPTHeader;
};


class BaseDStructureValue//  : public PTAllocT<BaseDStructureValue>
{
	//==========================================================================
	// Start Update by OJ : 2010-08-05
	// ������� ���� �ڵ�
//#ifdef _DEBUG
public:
	enum {
		TYPE_Logger_start,
		TYPE_Logger_end,
		TYPE_Logger_dummy,
		TYPE_Logger_fail
	};

	const char*get_name_debug()const
	{
		return m_strDebugName;
	}
//#ifdef _DEBUG
public:
	const char*get_name_bool_debug(int _nDebug)const
	{
		return m_stlVDebugProcessorName[_nDebug].c_str();
	}
	void set_name_debug(const char *_strName);
//#endif
private:
	char	m_strDebugName[255], m_strDebugComment[1024];
	STLVString	m_stlVDebugProcessorName;
	bool md_LogHide;

//#endif
	//================================= End Update by OJ : 2010-08-05

	//==========================================================================
	// Start Update by OJ : 2011-01-12
public:
	int function_hash_get() const;
	int	sequence_get() const;
	int sequence_get_local() const;
	int bool_index_get() const;
protected:
	int	m_function_hash;
	int	m_nSequenceCurrent;
	int	m_nSequenceCurrentLocal;
	int m_nBoolIndexCurrent;
	//================================= End Update by OJ : 2011-01-12
public:
	//enum{
	//	STATE_UPDATE		= 0x0001,
	//	STATE_START			= 0x0002,
	//	STATE_END			= 0x0004,
	//	STATE_SUSPEND		= 0x0008,
	//	STATE_LINK_COMPLETE	= 0x0010,
	//	STATE_LINK_START	= 0x0020,
	//	STATE_RESUME		= 0x0040,
	//	STATE_EVENT			= 0x0080,
	//	STATE_LINK_APPEND	= 0x0100
	//};
	static	BaseDStructureValue	sm_sample;

	BaseDStructureValue(const BaseDStructure *_pstdBase, int _nDumpSize = 0);
	virtual ~BaseDStructureValue();

	void init(const BaseDStructure *_pstdBase, int _nDumpSize = 0);
	void state_set_(const BaseDStructure* _pdstBase);
	bool regist_event_func(BaseStateSpace *_pSace);
	void reset_link();
	void release();

	int		get_count(const char *_strHash, int *_pnIndex=0)const;
	void set_point(const char* _strHash, void* _pIn);
	void set_point(int hash, void* _pIn);
	void	set_alloc(const char *_strHash, const void *_pIn, short nCnt = 0, bbyte _nType=TYPE_INT32);
	void	add_alloc(const char *_strHash, const void *_pIn, short nCnt = 0, bbyte _nType=TYPE_INT32);
	int		get_base_key()const;
	int		get_count() const;
	int		get_count_appended() const;
	int		get_colum(int _nSequence)const;
	
	bool	get(const void **_ppVoid, short *_pnCnt = 0, int _nSequence=0)const;
	//int		get_type_by_seq(int _nIndexSequence) const;
	bool	get(void *_pRet, short *pnCnt = 0, int _nSequence=0)const;
	bool	get(const char *_strHash, void *_pRet, short *pnCnt = 0, int _nSequence=0)const;
	bool	get(int _nHash, const void **_ppVoid, short *_pnCount=NULL, int _nSequence=0) const;
	bool	get_mass(int _nHash, const void** _ppVoid, int *_pnCount=NULL) const;
    void *get_point(int _hash, short* pnCnt = 0, int _nSequence = 0)const;
    void *get_point(const char* _strHash, short* pnCnt = 0, int _nSequence = 0)const;
	bool	get_local_seq(const char *_strHash, void *_pRet, short *pnCnt = 0)const;
	bool	get_local_seq(int _nHash, const void **_ppVoid, short *_pnCount=NULL) const;
	bool	get_by_index(int _nIndex, void *_pRet, short *_pnCount=NULL, int _nSequence=0)const;
	bool	get_by_index(int _nIndex, const void **_ppVoid, short *_pnCount=NULL, int _nSequence=0) const;
	int	get_mass_size(int _hash) const;
	bool	get_param(int _nHash, const void **_ppVoid, short *_pnCount = NULL, int _nIndexStartSequence = 0) const;
	bool	get_param(int _nHash, void *_pRet, short *_pnCount = NULL, int _nStartSequence = 0)const;
	bool	get_param(const char *_strHash, void *_pRet, short *_pnCount = NULL, int _nStartSequence = 0)const;

	bool	get_look_up(int _nHash, const void **_ppOut, short *_pnCntOut) const;

	int		compare(const BaseDStructureValue &_dsvTarget, int _nSkip = 0) const;

	int		get_key()const;
	bool	set_key(int _nKey);	// set m_nKey and remove values what stored in m_stlVValueAppend
	void	set_clear();
	//void	set_alloc(int _nIndex, void *_pVoid, short _nCount=0);	// create memory, copy and record the position
	//bool	get(int _nIndex, bool *_pbBool)const;

	void	add_alloc(int _nHash, const void *_pVoid, short _nCount=0);	// create memory, copy and record the position
	bool set_mass(int _nHash, const void *_pVoid);	// create memory, copy and record the position
	bool set_mass(int _hHash, const void* _pVoid, int _nCnt);
	void	set_alloc(int _nHash, const void *_pVoid, short _nCount=0);	// create memory, copy and record the position
	bool	get(int _nHash, bool *_pbBool)const;

	short	get_index(int _nHash) const;
	const BaseDStructure* get_base()const;
	int		event_process(int _nIndex, BaseDStructureValue *_pEvent, BaseDStructureValue *_pdsvContext, int _nState, int *_pnProcessCnt);
	int		event_process(BaseDStructureValue *_pEvent, BaseDStructureValue *_pdsvContext, int *_pnProcessCnt);	// Call for event process
	int		event_process(BaseDStructureValue *_pEvent, BaseDStructureValue *_pdsvContext, int _nState, int *_pnProcessCnt);	// Call on Idle state
	bool	event_result_check(unsigned _nIndex);
	
	void complete_make();
	bool	is_complete(BaseDStructureValue *_pEvent, BaseDStructureValue *_pdsvContext);
	bool	is_get_flag(int _nFlag)const;

	void	*m_pVoidParam;	// UIContext�� ��� FlashUIState�� ����
							
	const BaseDStructureValue &operator=(const BaseDStructureValue &_dsvRight);

	int	ref_inc();	// reference count increase
	int	ref_dec(void *_pStateMain = NULL);	// reference count decrease
	int	ref_get();
	int serial() {
		return m_serial;
	}

protected:
	BaseStateSpace	*m_pSpace;
	const BaseDStructure	*m_pdstBase;
	int		m_nKey;
	int		m_serial;
	STLVColumn	m_stlVValueAppend;
	bool	m_bRet;	// for point return
	typedef struct __ST_ColumnBool__{
		int		nHash;
		int		nSequence;
		int		nSequenceLocal;
		int		nCallType;
		bool	bResult;
		fnEventProcessor	fnProcessor;
	}ST_ColumnBool;
#ifdef _WIN32
	typedef std::vector<ST_ColumnBool, PT_allocator<ST_ColumnBool> >	STLVStColumBool;
#else
	typedef std::vector<ST_ColumnBool>	STLVStColumBool;
#endif
	STLVStColumBool	m_stlVBool;
	bool	m_bUpdated;
	bool	m_bComplete;
	bool	m_bIsGotUpdateLink;// Is got a link for update check.

private:
	int		m_nRefCount;

	PT_OPTHeader;	
	//==================================================================================================
	// Start Add or Update by OJ 2011-11-29 ���� 2:36:26
public:
	bool	set_dumppacket(void *_pManager, void *_pDump, int _nSize, bool _bPacket = false);
	int		get_dump_size() const;
	int		get_dumppacket_size() const;
	int		get_dumppacket(void *_pDump, int _nSize) const;
	void	dump_refragment(int _nSizeOfDump);
	bool	dump_save(const char *_strFilename) const;
	bool	dump_load(void *_pManager, const char *_strFilename);

private:
	void*	dump_make_space_(unsigned short _nSizeMore);
	int		get_dumppacket_column(int _nHash, bbyte*_pPacket, int _nSizeRemain, int _nSeq) const;

	void	*m_pVoid;// Dump Data
	int		m_nSize;// Size of DumpData
	int		m_nSizeDump;
	// End by OJ 2011-11-29 ���� 2:36:28
	//==================================================================================================

	//==================================================================================================
	// Start Add or Update by OJ 2017-08-29 add for variable function
public:
	//bool	variable_set(int _nHash, bbyte _nType);
	const char	*param_str_get(const BaseDStructureValue *_pdsvBase);
	bool	param_str_set(const BaseDStructureValue *_pdsvBase, const char *_str);
	bool	param_int_set(const BaseDStructureValue *_pdsvBase, int _nValue);
	const int *param_int_get(const BaseDStructureValue *_pdsvBase);
	void variable_transit(BaseDStructureValue *_dsv_p) const;
	void variable_copy(BaseDStructureValue* _dsv_p) const;
	void variable_copy_log(BaseDStructureValue* _dsv_p) const;

	bool set_value_(const void* _value_p, int _hash_n);
	bool add_value_(const void* _value_p, int _hash_n);
	bool set_value_log_(const void* _value_p, int _hash_n);
	//bool	param_variable_get(const BaseDStructureValue *_pdsvBase, int *_pnFirst, int *_pnSecond);
private:
	// End by OJ 
	//==================================================================================================

	//==================================================================================================
	// support logger
public:
	BaseDStructureValue* logger_new(const char *_str);
	void logger_hand_variable(int _hash, const void* _value, short _cnt=0);
	void logger_hand_result(int _index, int _result);
	void logger_reset(const char *_strCmt);// if this is Link type
	void logger_send(int _event_index, int _keyMain, int _key, int _state_serial, int _link, const STLMnInt &_groupID_m, int _event_serial);// if this is Link type
	BaseDStructureValue* m_logevent_p; // if this is Link type
	BaseDStructureValue* m_loglink_p; // unsed in execute_process, it's self link

	const BaseDStructureValue* logger_link_get(); // if this is variable, it is variable in variable operation. (context, state, global etc)
	void logger_link_set(const BaseDStructureValue* _link_p);

	void logger_state_event_cast(int _key, int _serial);
	void logger_state_event_receive(int _key, int _serial);
private:
	const BaseDStructureValue* m_logger_link_p; // if this is variable, it is variable in variable operation. (context, state, global etc)
	STLVInt 	m_logger_column_result;
	STLMnInt	m_logger_event_cast; // event caster or receiver record
	STLMnInt	m_logger_event_receive; // event caster or receiver record
	// end support logger =======================================================================

	// start support state class point
public:
	void sfunc_set(const STLMnpBaseObject &_sfunc_m);
	void sfunc_set(int _hash, BaseObject *_fclass);
	void sfunc_clear();
	void sfunc_clear(int _hash);
	void sfunc_transit(BaseDStructureValue *_pdsv) const;
	BaseObject *sfunc_get(int _hash);
protected:
	STLMnpBaseObject	m_sFunc_m;
	// end support state class
	// start support debug memory error =======================================================================
public:
	void dbg_mem_checkin(const void* _data) const;
	bool dbg_mem_check() const;
	void dbg_mem_checkout() const;
	const void *dbg_mem_checkin_hash(int _seq) const;
	// end support debug =======================================================================
#ifdef _DEBUG
public:
	INT64	deb_nValue;
	char	*deb_strString;
	char	deb_strBuff[255];
#endif
};

#define STR_COMMENT_APPENDED_VARIABLE	"$$#appended variable#$$"
