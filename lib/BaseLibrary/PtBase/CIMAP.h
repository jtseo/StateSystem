// CIMAP.h: interface for the IMAP class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __CIMAP_H__
#define __CIMAP_H__


#include <vector>
#include <string.h>
#include <assert.h>

#ifndef _WIN32
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <errno.h>
	#include <stdio.h>
	#include <iostream>

	#include <openssl/ssl.h>

	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1

#ifndef HAVE__STRNICMP
	#define HAVE__STRNICMP
	#define _strnicmp strncasecmp
#endif

	#define OutputDebugStringA(buf)

	typedef unsigned short WORD;
	typedef int SOCKET;
	typedef struct sockaddr_in SOCKADDR_IN;
	typedef struct hostent* LPHOSTENT;
	typedef struct servent* LPSERVENT;
	typedef struct in_addr* LPIN_ADDR;
	typedef struct sockaddr* LPSOCKADDR;

	#define LINUX
#else
	#include <winsock2.h>
	#include <time.h>
	#pragma comment(lib, "ws2_32.lib")

	//Add "openssl-0.9.8l\inc32" to Additional Include Directories
	#include "openssl\ssl.h"
#endif

#include "md5.h"

#define TIME_IN_SEC		3*60		// how long client will wait for server response in non-blocking mode
#define BUFFER_SIZE_IMAP 102400	  // SendData and RecvData buffers sizes
#define MSG_SIZE_IN_MB 5		// the maximum size of the message with all attachments
#define COUNTER_VALUE	100		// how many times program will try to receive data

//const char BOUNDARY_TEXT[] = "__MESSAGE__ID__54yg6f6h6y456345";

//enum CIMAPXPriority
//{
//	XPRIORITY_HIGH = 2,
//	XPRIORITY_NORMAL = 3,
//	XPRIORITY_LOW = 4
//};

class ECIMAP
{
public:
	enum CIMAPError
	{
		CIMAP_NO_ERROR = 0,
		WSA_STARTUP = 100, // WSAGetLastError()
		WSA_VER,
		WSA_SEND,
		WSA_RECV,
		WSA_CONNECT,
		WSA_GETHOSTBY_NAME_ADDR,
		WSA_INVALID_SOCKET,
		WSA_HOSTNAME,
		WSA_IOCTLSOCKET,
		WSA_SELECT,
		BAD_IPV4_ADDR,
		UNDEF_MSG_HEADER = 200,
		UNDEF_MAIL_FROM,
		UNDEF_SUBJECT,
		UNDEF_RECIPIENTS,
		UNDEF_LOGIN,
		UNDEF_PASSWORD,
		BAD_LOGIN_PASSWORD,
		BAD_DIGEST_RESPONSE,
		BAD_SERVER_NAME,
		UNDEF_RECIPIENT_MAIL,
		COMMAND_MAIL_FROM = 300,
		COMMAND_EHLO,
		COMMAND_AUTH_PLAIN,
		COMMAND_AUTH_LOGIN,
		COMMAND_AUTH_CRAMMD5,
		COMMAND_AUTH_DIGESTMD5,
		COMMAND_DIGESTMD5,
		COMMAND_DATA,
		COMMAND_QUIT,
		COMMAND_RCPT_TO,
		MSG_BODY_ERROR,
		CONNECTION_CLOSED = 400, // by server
		SERVER_NOT_READY, // remote server
		SERVER_NOT_RESPONDING,
		SELECT_TIMEOUT,
		FILE_NOT_EXIST,
		MSG_TOO_BIG,
		BAD_LOGIN_PASS,
		UNDEF_XYZ_RESPONSE,
		LACK_OF_MEMORY,
		TIME_ERROR,
		RECVBUF_IS_EMPTY,
		SENDBUF_IS_EMPTY,
		OUT_OF_MSG_RANGE,
		COMMAND_EHLO_STARTTLS,
		SSL_PROBLEM,
		COMMAND_DATABLOCK,
		STARTTLS_NOT_SUPPORTED,
		LOGIN_NOT_SUPPORTED,
		COMMAND_LIST,
		COMMAND_MSG,
		COMMAND_COPY,
		COMMAND_DEL
	};
	ECIMAP(CIMAPError err_) : ErrorCode(err_) {}
	CIMAPError GetErrorNum(void) const {return ErrorCode;}
	std::string GetErrorText(void) const;

private:
	CIMAPError ErrorCode;
};

//enum IMAP_COMMAND
//{
//	command_INIT,
//	command_EHLO,
//	command_USER,
//	command_PASSWORD,
//	command_LIST,
//	command_MSG,
//	command_COPY,
//	command_DEL,
//	command_QUIT,
//	command_STARTTLS
//};

// TLS/SSL extension
//enum IMAP_SECURITY_TYPE
//{
//	NO_SECURITY,
//	USE_TLS,
//	USE_SSL,
//	DO_NOT_SET
//};

typedef struct tagCommand_imap
{
	SMTP_COMMAND		command;
	int					send_timeout;	 // 0 means no send is required
	int					recv_timeout;	 // 0 means no recv is required
	char				valid_reply_code[3]; // 0 means no code
	ECIMAP::CIMAPError	error;
}Command_imap;

class ST_Message{
public:
	PtObjectHeader2(ST_Message);
	PT_OPTHeader;
public:

	bool			bReaded;
	bool			bRetrieved;
	STLString		strSubject;
	STLString		strFrom;
	STLString		strTo;
	STLString		strReplyTo;
	STLString		strReturnPath;
	STLString		strSender;
	STLString		strMsg;
	STLString		strDate;
};

class CIMAP  
{
public:
	CIMAP();
	virtual ~CIMAP();
	PT_OPTHeader;
public:
	void AddRecipient(const char *email, const char *name=NULL);
	void AddBCCRecipient(const char *email, const char *name=NULL);
	void AddCCRecipient(const char *email, const char *name=NULL);    
	void AddAttachment(const char *path);   
	void AddMsgLine(const char* text);
	bool ConnectRemoteServer(const char* szServer, const unsigned short nPort_=0,
							 SMTP_SECURITY_TYPE securityType=DO_NOT_SET,
		                     bool authenticate=true, const char* login=NULL,
							 const char* password=NULL);
	void DisconnectRemoteServer();
	void DelRecipients(void);
	void DelBCCRecipients(void);
	void DelCCRecipients(void);
	void DelAttachments(void);
	void DelMsgLines(void);
	void DelMsgLine(unsigned int line);
	void ModMsgLine(unsigned int line,const char* text);
	unsigned int GetBCCRecipientCount() const;    
	unsigned int GetCCRecipientCount() const;
	unsigned int GetRecipientCount() const;    
	const char* GetLocalHostIP() const;
	const char* GetLocalHostName();
	const char* GetMsgLineText(unsigned int line) const;
	unsigned int GetMsgLines(void) const;
	const char* GetReplyTo() const;
	const char* GetMailFrom() const;
	const char* GetSenderName() const;
	const char* GetSubject() const;
	const char* GetXMailer() const;
	CSmptXPriority GetXPriority() const;
	// -------------------------------------------------
	// 2012-12-29 Start Append by oj for imap client
	//void Send();
	void Login();
	size_t GetNumOfMsg()
	{
		return m_stlVpstMessage.size();
	}
	void MoveMsg(STLVInt *_pstlVMsgs, const char *_strMsgBox);
	void SetMailBox(const char *_strMainBox)
	{
		strcpy_s(m_strMailBox, 128, _strMainBox);
	}
protected:
	char		m_strMailBox[128];
	std::vector<ST_Message*>	m_stlVpstMessage;
public:
	int GetList();

	STLVInt *GetUnreadedMsg(STLVInt *_pstlVUnreaded);

protected:

public:
	ST_Message *RetrieveMessage(int _nIndex);

	void Disconnect();
	// End of append for imap client [12/29/2012 jtseo]
	//======================================================

	void SetCharSet(const char *sCharSet);
	void SetLocalHostName(const char *sLocalHostName);
	void SetSubject(const char*);
	void SetSenderName(const char*);
	void SetSenderMail(const char*);
	void SetReplyTo(const char*);
	void SetReadReceipt(bool requestReceipt=true);
	void SetXMailer(const char*);
	void SetLogin(const char*);
	void SetPassword(const char*);
	void SetXPriority(CSmptXPriority);
	void SetIMAPServer(const char* server, const unsigned short port=0, bool authenticate=true);

private:	
	std::string m_sLocalHostName;
	std::string m_sMailFrom;
	std::string m_sNameFrom;
	std::string m_sSubject;
	std::string m_sCharSet;
	std::string m_sXMailer;
	std::string m_sReplyTo;
	bool m_bReadReceipt;
	std::string m_sIPAddr;
	std::string m_sLogin;
	std::string m_sPassword;
	std::string m_sIMAPSrvName;
	unsigned short m_iIMAPSrvPort;
	bool m_bAuthenticate;
	CSmptXPriority m_iXPriority;
	char *SendBuf;
	char *RecvBuf;
	
	SOCKET hSocket;
	bool m_bConnected;

	struct Recipient
	{
		std::string Name;
		std::string Mail;
	};

	std::vector<Recipient> Recipients;
	std::vector<Recipient> CCRecipients;
	std::vector<Recipient> BCCRecipients;
	std::vector<std::string> Attachments;
	std::vector<std::string> MsgBody;
 
	void ReceiveData(Command_imap* pEntry);
	void SendData(Command_imap* pEntry);
	void FormatHeader(char*);
	int IMAPXYZdigits();
	void SayHello();
	void SayQuit();

// TLS/SSL extension
public:
	SMTP_SECURITY_TYPE GetSecurityType() const
	{ return m_type; }
	void SetSecurityType(SMTP_SECURITY_TYPE type)
	{ m_type = type; }
	bool m_bHTML;

private:
	SMTP_SECURITY_TYPE m_type;
	SSL_CTX*      m_ctx;
	SSL*          m_ssl;

	void ReceiveResponse(Command_imap* pEntry);
	void InitOpenSSL();
	void OpenSSLConnect();
	void CleanupOpenSSL();
	void ReceiveData_SSL(SSL* ssl, Command_imap* pEntry);
	void SendData_SSL(SSL* ssl, Command_imap* pEntry);
	void StartTls();
};


#endif // __CIMAP_H__
