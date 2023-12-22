#pragma once

class BaseEmail
{
public:
	BaseEmail(void);
	~BaseEmail(void);

	static bool send_mail(const char *_strReceiver, const char *_strTitle, const char *_strContents, const char *_strAttachFile = NULL);
	static char *get_sequrity_string(char *_strRet, size_t _nLen, const char *_strInput);

	static void init();
protected:
	static bool is_cancel(char _chr);

public:
	static void *OpenSSLInit();
	static void OpenSSLClean();
};
