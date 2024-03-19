#pragma once

//////////////////////////////////////////////////////////////////////////

// Binary -- Block--.
// --

//////////////////////////////////////////////////////////////////////////
//#include <windows.h>

class EncoderOperator;
class CMD5;
struct md5_state_s;

#define ASC_MAX_LENGTH 10240
#define STD_STR_MIN	32
#define STD_STR_MID	255

//#define STD_TAG_SUPPORT

// version info
// Random Access read(Seek())

class BaseFile// : public PTAllocT<BaseFile>
{
public:
	void init(EncoderOperator *_pEncoder = NULL);
	void release();
	static BaseFile	sm_sample;

	BaseFile(EncoderOperator *_pEncoder = NULL);
	virtual ~BaseFile(void);

	enum{
		MSG_SUCCESS,
		MSG_ENDOFFILE,
		MSG_ERRORTOLOAD,
		MSG_ERRORTOSAVE,
		MSG_ERRORTOOPEN
	};

	enum{
		OPEN_NULL,
		OPEN_WRITE	= 0x0001,
		OPEN_READ	= 0x0002,
		OPEN_MEMORY	= 0x0004, //
		MEM_OUTDATA	= 0x0008, //by _pFileName)
								// set_size_file()
		MEM_NEWDATA = 0x0010, //
		ENCODE_001	= 0x0020, //
		OPEN_ASC	= 0x0040, //
		OPEN_NON_PACKAGE = 0x0080, // Packaging
		OPEN_RANDOM	= 0x0100, // Random format for write
		OPEN_UTF8	= 0x0200, // support for utf8 when write
		OPEN_UCS2L	= 0x0400, // support for UCS-2 Little engian when write
		OPEN_FILE	= 0x0000
	};

	bool is_exe();
	int	OpenFile(void *_pData, UINT32 _nSize);
	int	openParser(char* _pData, UINT32 _nSize);
	int	virtual OpenFile(const char *_pFileName, int _nType=OPEN_READ, void *_pData = 0, UINT32 _nSize = 0);
	int	virtual OpenFile(char *_pFileName, int _nType=OPEN_READ, void *_pData = 0, UINT32 _nSize = 0);
	static bool is_exist(const char *_pFilename);
	int SaveFile(const char *_strFilename);
	int CloseFile();
	static int copy(const char* input, const char* output);

	void *get_memory(size_t _nSize); // as close file, this point will deleted.
	char *get_point(int _nSizeSkip);// return memory point without memory copy
	int	Read(void *_pReadBuffer, int _nSize);
	int	Read(STLString *_buffer);
	int	Write(const void *_pSaveBuffer, int _nSize);
	int Seek(INT32 _nPos);
	int get_file_type();
	void set_size_file(UINT32 nSize);
	UINT32 get_size_file();

	void set_md5(CMD5 *_pMd5, md5_state_s *_pMd5State);
	UINT32 get_buffer_size();
	int	get_size_block();
	void set_comment(char _cComment);
	const char *get_asc_line();

	int get_seperator_last(){ return m_nSeperatorLast; }// Add by OJ : 2010-10-11
	int get_delector_last(){ return m_nDelecterLast; }// Add by OJ : 2010-10-11

	void *m_pVoid;
protected:
	unsigned	ReadNextBlock();
	unsigned	WriteCurrnetBlock();
	bool allocate_memoryfile(UINT32 _nSize = 1048576); //
	int write_in_file(const void *_pSaveBuffer, int _nSize);
	void init_();
	bool allocate_linememory();

	char m_dbg_filename[1024];
	char	m_cComment;		/// skip a line if start by this character(comment)
	char	*m_strFilename;
	void	*m_pfFile;
	char	*m_pbMemoryFile;
	void	*m_pMemoryOut;	// return by get_memory() and when closefile(), it will be deleted.

	bool	m_bEndOfFile;	/// --true
	int 	m_nSizeBuffer;	/// buffer--(read-- pCurrent--, write-- pbBuffer-- Current--)
	int		m_nType;		/// file type (read, write, memory, file)
	unsigned		m_nSizeBlock;	/// Disk---
	char	*m_pbBuffer;	/// current buffer: --
	unsigned char	*m_pbBufferCurrent;	/// loading point: --

	unsigned m_nSizeOfFile; /// --, memory--.
	bool	m_bDelMemoryFile; /// Close--m_pbMemoryFile--

	CMD5 *pmd5Class_;
	md5_state_s *pmd5State_;

	unsigned char strKey_[32];

	//UINT32 seed_encrypt_(unsigned char *_pBuffer, UINT32 _nSize, unsigned char *_pKey);
	//UINT32 seed_decrypt_(unsigned char *_pBuffer, UINT32 _nSize, unsigned char *_pKey);
public:
	virtual STLString mpercent_parsing(STLVPstrstr _mpercent);
	virtual void set_asc_carriage(const char *_strCarriageReturn);
	virtual void set_asc_deletor(const char *_strDelector);
	virtual void set_asc_replace(const char *_strReplace);
	virtual void set_asc_seperator(const char *_strSeperator);
	virtual void set_asc_sign(char _sign);
	virtual void push_asc_seperator(const char* _strSeperator);
	virtual bool seperator_del(char _ch);
	virtual void seperator_add(char _ch);
	virtual bool deletor_del(char _ch);
	virtual void deletor_add(char _ch);
	virtual void pop_asc_seperator();
	virtual void set_asc_tag(const char *_strTag);
	virtual bool read_asc_line();
	bool read_asc_leave_line(char *_strSource, size_t _nSize);
	bool read_asc_string( char *_strSource, const int nStrSize, bool bIgnoreSpace = false);
	bool read_asc_string(STLString *_str);
	bool read_asc_tag( char *_strSource, const int nStrSize);
	int read_asc_string2( char *_strSource, const int nStrSize);
	bool read_asc_integer( int* _pnNum );
	bool read_asc_float( float *_pfValue);
	bool read_asc_float3(float *_f3Value);
	STLString read_asc_str();
	STLString read_asc_quotation();
	const char *token_get();
	
	bool write_asc_line();
	bool write_asc_string_noseperator( const char *_strSource, const int nStrSize);
	bool write_asc_string( const char *_strSource, const int nStrSize);
	bool write_asc_integer( int _nNum );
	bool write_asc_integer64( INT64 _nNum );
	bool write_asc_float( float _fValue);
	bool write_asc_cutfloat(float _fValue);
	bool write_asc_float3(const float *_f3Value);
	bool write_asc_cutfloat3(const float *_f3Value);
	/// --
	bool write_bin_integer( const int* _nNum );
	bool write_bin_sinteger(const int* _nNum);
	bool write_bin_long( const INT32* nNum );
	bool write_bin_unsigned_long( const UINT32* _nNum );
	bool write_bin_string( const char* _strSource );
	bool write_bin_bool( const char* _bFlag );
	bool write_bin_byte( const unsigned char* _bValue );
	bool write_bin_float( const float* _fNum );
	bool write_bin_floatN(const float* _fNum, int _n);
	bool write_bin_short( const short* _nNum );
	bool write_bin_ushort( const unsigned short* _nNum );
	void write_brace_start(const char *_strComment=NULL);
	void write_brace_end();
	/// --
	bool read_bin_integer(int *_pnValue);
	bool read_bin_sinteger(int *_pnValue);
	bool read_bin_long(long *_pnValue);
	bool read_bin_unsigned_long(unsigned long *_pnValue);
	int read_bin_string( char* _strSource, const int nStrSize );
	bool read_bin_bool(char *_pcValue);
	bool read_bin_byte(unsigned char* _bValue );
	bool read_bin_float(float *_pfValue);
	bool read_bin_floatN(float* _pfValue, int _n);
	bool read_bin_short(short *_psValue);
    bool read_bin_doulbe(double *_pvalue);
	bool read_bin_ushort(unsigned short *_psValue);
	bool read_brace_start(char *_str, int _size);
	void read_brace_end();
	bool read_brace_end_check();

protected:
	bool write_asc_string_(const char* _strSource, const int nStrSize);
	virtual bool parse_quotation_();
	virtual bool parse_token_(bool _quot = false);
	bool check_delector_(char _char);
	char check_replace_(char _char);
	bool check_seperator_(char _char, int _seq);
	bool check_tag_();
	char	m_strCarriageReturn[STD_STR_MIN];
	char	m_strDelete[STD_STR_MIN];
	char	m_strReplace[STD_STR_MIN];
	char	m_strSeperator[STD_STR_MIN];
	char	m_strSeperator2[STD_STR_MIN];
	char m_sign;
	STLString *m_strToken;
	STLString* m_strTag;
	STLVPstrstr* m_mpercent_ap;
	char *m_strBufferAscLine;
	char *m_strBufferAscBlock;
#ifdef STD_TAG_SUPPORT
	STLVString	m_stlVStrTags;
#endif
	int	m_nSizeLine;
	bool m_bFirstTokenLine;
	int m_nCurCharAscBlock;
	int m_nCurCharAscLine;
	int m_nSizeAscBlock;
	int	m_nDelecterLast;
	int m_nSeperatorLast;
	
	bool m_bDeleteBuffer;

	void write_x_(float _fValue);
	void write_x_(int _nValue);
	void write_sx_(int _nValue);
	void write_x_(const char *_str, int _nBufferSize);
	int write_x_dump_(const void *_pdump, int _nBufferSize);
    void read_x_(double *_pvalue);
	void read_x_(float *_pfValue);
	bool read_x_(int *_pnValue);
	bool read_sx_(int *_pnValue);
	int read_x_(char *_str, int _nBufferSize);
	int read_x_dump_(void *_pdump, int _nBufferSize);
	void write_x_tab_(int _nDepth);
	int read_x_token_(char *_str, int _nBufferSize);
	char pase_x_hex_digit_(char cChar);
	char pase_x_hex_char_(char *pChar);
	void read_x_tail_();
	int	m_nBraceCnt;	/// ASC format-- Depth--
	std::deque<std::string, PT_allocator<std::string> > *m_stlQStack;
	char m_cBrace[2];
	char m_cReturn[2];

	int read_dump_(void *_pDump, int _nSize);
	int write_dump_(const void *_pDump, int _nSize);

public:
	bool write_binary( const INT32& nNum_ );
	bool write_binary( const short& nNum_ );
	bool write_binary( const float& nNum_ );

	bool write_binary( const char* strSource_ );

	bool read_binary( INT32& nNum_ );
	bool read_binary( short& nNum_ );
	bool read_binary( float& nNum_ );

	bool read_binary( char* strSource, int nSize_ );

	// ========================================================================
	// support parser
	static bool is_slash(char _ch);
	static char *path_cut_last(char *_strPath, int _nSize);
	static int path_r_find_slash(const char *_strPath, int _nStart);
	static bool get_path_last(const char *_strFull, char *_strPath, int _nSize);
	static bool change_filename(const char *_strSource, const char *_strFilename, char *_strRet, int _nSize);
	static bool change_fileext(const char* _strSource, const char* _ext, char* _strRet, int _nSize);
	static bool get_path(const char *_strFull, char *_strPath, int _nSize);
	static bool get_filename(const char *_strFull, char *_strFilename, int _nSize);
	static bool get_filenamefull(const char* _strFull, char* _strFilename, int _nSize);
	static bool get_filext(const char *_strFull, char *_strFilext, int _nSize);
	static STLString get_path(const STLString &_strfull);
	static STLString get_filenamefull(const STLString &_strfull);

	static int get_dec_int(const char* _dec_str);
	static int get_int(const char* _hex_str);
	static char* set_int(char* _str, int _size, int _value);
	static bool is_number(const char *_str);
	static float get_float(const char *_str);
	static INT64 get_longlong(const char *_str);
	static char *get_l2string(char *_strRet, int _nLen, INT64 _nValue);
	static char* paser_list_merge(char* _ret, int _size, const STLVString &_str_a, const char *_seperator);
	static int paser_list_seperate(const char* _in, STLVString* _str_a, const char *_seperator);
	static STLString remove(const STLString &_str, char _c);
	static STLString to_str(int _n);
	static STLString to_str(INT64 _n);
    static STLString to_str(float _f);
	static STLString to_str1(float _f);
	static STLString to_str(float _f, int _digit);
	static float round(float _f, int _digit);
	// ------------------------------------------------------------ end of parser
    
//////////////////////////////////////////////////////////////////////////
	// support for random access kind of DB file
	typedef struct{
		unsigned	nIndex;	// block index : filepos = nIndex * m_nSizeOfBlock
		unsigned	nSizeBuffer;
		bool			bDirty;
		unsigned	nLastAccessTime;
		char			*pData;
	}StFileBlock;
	typedef std::map< int, StFileBlock, std::less<int>, PT_allocator<std::pair<const int, StFileBlock> > >	STLMnFileBlock;
	STLMnFileBlock	*m_stlMnFileBlock;
	char	*get_fileblock_(UINT32 _nFilePos);
	void	release_fileblock_();
	void	read_fileblock_(StFileBlock *_pFileblock);
	void	write_fileblock_(StFileBlock *_pFileblock);
	StFileBlock		*m_pFileBlockCurrent;
	static unsigned	sm_nAccessCount;

	PT_OPTHeader;
	//==========================================================================
	// Start Update by OJ : 2010-11-02
public:
	bool is_created(){ return m_bFileCreated; } //OpenFile--
protected:
	bool	m_bFileCreated;
	//================================= End Update by OJ : 2010-11-02

	//==========================================================================
	// Start Update by OJ : 2010-11-02
public:
	
protected:
	size_t fwrite_(void *_pBuffer, int _nCnt, size_t _nSize, FILE *_pFile);
	size_t fread_(void *_pBuffer, int _nCnt, size_t _nSize, FILE *_pFile);
	EncoderOperator	*m_pEncoder;
	bool m_bExeFile;
	void exe_check();
	//================================= End Update by OJ : 2010-11-02

	bool	m_bFirstWritingSign;
	bool	m_bFirstReadingSign;
};

#ifdef TEST_MODULE

#include "KpUtil/KpUtilUnitTest.h"

class KpUnitTestBaseFile : public KpUtilUnitTest
{
public:
	KpUnitTestBaseFile(void);
	virtual ~KpUnitTestBaseFile(void);

	virtual int Init();
	virtual int Test();
	virtual int Close();
protected:
	BaseFile	*m_pFile;
};

#endif
