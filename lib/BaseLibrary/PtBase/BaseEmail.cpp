#include "StdAfx.h"
#include "BaseEmail.h"

#include "CSmtp.h"
#include "CIMAP.h"
#include <iostream>
#include "base64.h"
#include "openssl/err.h"

CRITICAL_SECTION s_critical_section_openssl;

static char strCancel[10] = "";

BaseEmail::BaseEmail(void)
{
	if(strCancel[0] != 0)
		return;

	strCancel[0]	= '@';
	strCancel[0]	= '|';// |?><
	strCancel[1]	= 'a';
	strCancel[1]	= '?';
	strCancel[2]	= '?';
	strCancel[2]	= '>';
	strCancel[3]	= '<';
	strCancel[4]	= 0;
	InitializeCriticalSection(&s_critical_section_openssl);
}

void BaseEmail::init()
{
	if(strCancel[0] != 0)
		return;

	strCancel[0]	= '@';
	strCancel[0]	= '|';// |?><
	strCancel[1]	= 'a';
	strCancel[1]	= '?';
	strCancel[2]	= '?';
	strCancel[2]	= '>';
	strCancel[3]	= '<';
	strCancel[4]	= 0;
	InitializeCriticalSection(&s_critical_section_openssl);
}

BaseEmail::~BaseEmail(void)
{
}

bool BaseEmail::send_mail(const char *_strReceiver, const char *_strTitle, const char *_strContents, const char *_strAttachFile)
{
	bool bError = false;

//	try
//	{
		CSmtp mail;

#define test_gmail_ssl 
//		tls

#if defined(test_gmail_tls)
		mail.SetSMTPServer("smtp.mail.yahoo.com", 587);
		//mail.SetSecurityType(USE_TLS);
#elif defined(test_gmail_ssl)
		mail.SetSMTPServer("smtp.gmail.com",465);
		mail.SetSecurityType(USE_SSL);
#elif defined(test_hotmail_TLS)
		mail.SetSMTPServer("smtp.live.com",25);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_aol_tls)
		mail.SetSMTPServer("smtp.aol.com",587);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_yahoo_ssl)
		mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
		mail.SetSecurityType(USE_SSL);
#endif
		char strBuffer[255];

		//mail.SetLogin(get_sequrity_string(strBuffer, 255, "crotr@udm$ffogr@efvgetrr@.grmtayiul*.hcgofmh"));//"no.reply@yahoo.com corumforever@gmail.com"
		//mail.SetPassword(get_sequrity_string(strBuffer, 255, "Rdjesgehojdglrqdrsnd3e2r"));//"zhfna919" vorxhfl&!%715
		//mail.SetSenderMail(get_sequrity_string(strBuffer, 255, "crotr@udm$ffogr@efvgetrr@.grmtayiul*.hcgofmh"));//"corumforever@gmail.com"
		//mail.SetReplyTo(get_sequrity_string(strBuffer, 255, "crotr@udm$ffogr@efvgetrr@.grmtayiul*.hcgofmh"));//"corumforever@gmail.com"

		mail.SetLogin(get_sequrity_string(strBuffer, 255,      "ahdgmfien5@youdld,emynebsvocfdts.fcgojmk"));//"no.reply@yahoo.com corumforever@gmail.com"
		mail.SetPassword(get_sequrity_string(strBuffer, 255, "Sdoecsbgohatrjdh!g)r03"));//"zhfna919" vorxhfl&!%715
		mail.SetSenderMail(get_sequrity_string(strBuffer, 255, "ahdgmfien5@youdld,emynebsvocfdts.fcgojmk"));//"corumforever@gmail.com"
		mail.SetReplyTo(get_sequrity_string(strBuffer, 255, "ahdgmfien5@youdld,emynebsvocfdts.fcgojmk"));//"corumforever@gmail.com"

		//mail.SetLogin(get_sequrity_string(strBuffer, 255, "bdde.tnyourie4p6l8y9@0y9a0hpo;ol.kcjoumi"));//"no.reply@yahoo.com corumforever@gmail.com"
		//mail.SetPassword(get_sequrity_string(strBuffer, 255, "Gdodeedspgsh!r)30d"));//"zhfna919" vorxhfl&!%715
		//mail.SetSenderMail(get_sequrity_string(strBuffer, 255, "bdde.tnyourie4p6l8y9@0y9a0hpo;ol.kcjoumi"));//"corumforever@gmail.com"
		//mail.SetReplyTo(get_sequrity_string(strBuffer, 255, "bdde.tnyourie4p6l8y9@0y9a0hpo;ol.kcjoumi"));//"corumforever@gmail.com"
		
		mail.SetSenderName("Oddeyesoft");
		mail.SetSubject(_strTitle);
		if (_strAttachFile) {
			mail.AddAttachment(_strAttachFile);
		}
		mail.AddRecipient(_strReceiver);
		mail.SetXPriority(XPRIORITY_NORMAL);
		mail.SetXMailer("The SocBoard (v2.02)");
		mail.SetCharSet("UTF-8");
		mail.AddMsgLine(_strContents);
		mail.m_bHTML	= true;
		mail.Send();

//	}
//	catch(ECSmtp e)
//	{
//		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
//		bError = true;
//	}
//	if(!bError)
//		std::cout << "Mail was send successfully.\n";
//	else
//		return false;
	return true;
}

bool BaseEmail::is_cancel(char _chr)
{
	if(strCancel[0] == 0)
		init();

	char *strTemp	= strCancel;
	while(*strTemp)
	{
		if(*strTemp == _chr)
			return true;
		strTemp++;
	}
	return false;
}

char *BaseEmail::get_sequrity_string(char *_strRet, size_t _nLen, const char *_strInput)
{
	char	*strTemp;
	size_t nCnt=0;

	strTemp	= _strRet;
	while(nCnt < _nLen && *_strInput != 0)
	{
		if(!is_cancel(*_strInput))
		{
			*strTemp	= *_strInput;
			_strInput++;
			if(is_cancel(*_strInput))
				_strInput++;
			_strInput++;
			strTemp++;
			nCnt++;
			continue;
		}
		_strInput++;
	}

	*strTemp	= 0;

	return _strRet;
}

SSL_CTX	*s_pCTX	= NULL;

void *BaseEmail::OpenSSLInit()
{
	EnterCriticalSection(&s_critical_section_openssl);
	
	if(s_pCTX)
	{
		LeaveCriticalSection(&s_critical_section_openssl);
		return s_pCTX;
	}

	SSL_library_init();
	SSL_load_error_strings();
	//s_pCTX = SSL_CTX_new (SSLv3_client_method());
	s_pCTX = SSL_CTX_new(SSLv23_client_method());
	//if(m_ctx == NULL)
	//	throw ECIMAP(ECIMAP::SSL_PROBLEM);

	LeaveCriticalSection(&s_critical_section_openssl);

	return s_pCTX;
}


void BaseEmail::OpenSSLClean()
{
	if(s_pCTX == NULL)
		return;

	if(s_pCTX != NULL)
	{
		SSL_CTX_free (s_pCTX);	
		s_pCTX	= NULL;
		ERR_free_strings();
		EVP_cleanup();
		CRYPTO_cleanup_all_ex_data();
	}

	DeleteCriticalSection(&s_critical_section_openssl);
}