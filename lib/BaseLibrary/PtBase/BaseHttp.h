#pragma once
#include "BaseFile.h"


class STHttpTable
{
public:
	STHttpTable(void)
	{
		nCurLine_ = 0;
	};
	virtual ~STHttpTable(void);

	const char *get_string(int _nLine, int _nRow);
	bool get_int(int _nLine, int _nRow, int *_pnOut);
	bool get_float(int _nLine, int _nRow, float *_pfOut);

	unsigned get_line();
	unsigned get_row();

	typedef std::vector<std::string*>	STLVpText;
	std::vector<STLVpText*>	stlVpstlVpArray_;
	int nCurLine_;
};

// table
// tr
// td
// brace in & out
class BaseHttp :
	public BaseFile
{
public:
	BaseHttp(void);
	virtual ~BaseHttp(void);

	bool parse(char *pDocBuffer, UINT32 _nSize);
	bool parse(char *strFilename);
	void clear();
	
	int		get_size_table();
	STHttpTable *find(char *_strFirst);
	STHttpTable *last();

	static bool InitHTTP();
	static int GetHTTP(const char* lpServerName, const char* lpFileName, char **ppDocBuffer);
	static bool GetHTTP(const char* lpServerName, const char* lpFileName, BaseFile *_pFile);
	static bool GetData(const char* _serverName, int _port, const char* _request, STLVString *_data_a);
	static void CloseHTTP();

	static unsigned char *get_public_ip(unsigned char *_strIP);

	//------------------------------------
	// IP Filter
	bool filter_make(const char *_strFilein, const char *_strFileout);
	bool filter_check(unsigned char *_pcIP);
	bool filter_load(const char *_strFilefilter);
	void filter_close();
	//------------------------------------
protected:
	std::vector<STHttpTable *> stlVpTables_;
	std::vector<STHttpTable *> stlVpTablesLoading_;


	bool parse_token_(bool _quot = false);
	void parse_comment();
	int parse_brace();
	void parse_table();
	void parse_tr();
	void parse_td();

	int get_def(const char *strToken);

	enum{
		DEF_UNKNOW,
		DEF_IN_TABLE,
		DEF_OUT_TABLE,
		DEF_IN_TR,
		DEF_OUT_TR,
		DEF_IN_TD,
		DEF_OUT_TD,
		DEF_IN_CMT,
		DEF_OUT_CMT
	};
};
