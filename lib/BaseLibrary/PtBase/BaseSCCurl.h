#pragma once
#include "../PtBase/BaseStateFunc.h"
class BaseSCCurl :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(get_strF),
        STDEF_SFENUM(post_strF),
		STDEF_SFENUM(postEncode_varF),
        STDEF_SFENUM(EmailSend_varF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseSCCurl();
    ~BaseSCCurl();

    PtObjectHeaderInheritance(BaseSCCurl);
    STDEF_SC_HEADER(BaseSCCurl);
    virtual int Create();
    int get_strF();
    int post_strF();
	int postEncode_varF();
    int EmailSend_varF();
    //#SF_FuncHeaderInsert

    void event_post(STLString &_data, STLString &_header);
    void event_data_set(const char* _buff) {
        m_data = _buff;
    }
    // User defin area from here
	bool curlPost(const char* _data, const char* _url, const char* _autho);
protected:
    static DEF_ThreadCallBack(update);
    STLString   m_data, m_header;

public:

};

