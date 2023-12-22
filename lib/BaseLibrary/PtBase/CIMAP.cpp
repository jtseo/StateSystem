////////////////////////////////////////////////////////////////////////////////
// Original class CFastIMAP written by 
// christopher w. backen <immortal@cox.net>
// More details at: http://www.codeproject.com/KB/IP/zIMAP.aspx
// 
// Modifications introduced by Jakub Piwowarczyk:
// 1. name of the class and functions
// 2. new functions added: SendData,ReceiveData and more
// 3. authentication added
// 4. attachments added
// 5 .comments added
// 6. DELAY_IN_MS removed (no delay during sending the message)
// 7. non-blocking mode
// More details at: http://www.codeproject.com/KB/mcpp/CIMAP.aspx
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// SSL/TLS support added by John Tang by making use of OpenSSL: http://www.openssl.org/ 
// More details at: http://www.codeproject.com/KB/IP/IMAP_ssl.aspx
//
// PLAIN, CRAM-MD5 and DIGESTMD5 authentication added by David Johns
//
// Revision History:
// - Version 2.1: Updated with fixes reported as of 26 Mar 2012
//     > Fixed issue in main.cpp with referring to USE_TLS in the wrong scope discussed here: http://www.codeproject.com/Messages/4151405/Re-USE_SSL-no-member-of-CIMAP.aspx
//       - Thanks to Alan P Brown!
//     > Added modifications to allow it to compile in Debian Linux discussed here: http://www.codeproject.com/Messages/4132697/linux-port-patch.aspx
//       - Thanks to Oleg Dolgov!
//     > Added ability to change the character set, inspired by this post: http://www.codeproject.com/Messages/4238701/Re-The-subject-contains-the-Chinese-letters-could-.aspx
//       - Thanks to LeonHuang0726 and John TWC for the suggestion!
//     > Added ability to request a read receipt by calling SetReadReceipt as proposed here: http://www.codeproject.com/Messages/3938944/Disposition-Notification-To.aspx
//       - Thanks to Gospa for the suggestion!
//     > Added check for Linux when adding paths of attachments in the MIME header as suggested here: http://www.codeproject.com/Messages/4357144/portability-bug-w-attachment-name.aspx
//       - Thanks to Spike!
//     > Switched method of setting private std::string variables to use the = operator as suggested here: http://www.codeproject.com/Messages/4356937/portability-bugs-w-std-string-and-exceptions.aspx
//       - Thanks to Spike!
//     > Added SetLocalHostName function proposed here: http://www.codeproject.com/Messages/4092347/bug-fixes-GetLocalHostName-Send.aspx
//       - Thanks to jerko!
//     > Added the modifications to allow it to compile in Linux described here: http://www.codeproject.com/Messages/3878620/My-vote-of-5.aspx
//       - Thanks to korisk!
//     > Added the fix that corrects behavior when m_sNameFrom is empty described here: http://www.codeproject.com/Messages/4196071/Bug-Mail-sent-by-mail-domain-com.aspx
//       - Thanks to agenua.grupoi68!
// - Version 2.0: Updated to all fixes reported as of 23 Jun 2011:
//     > Added the m_bAuthenticate member variable to be able to disable authentication
//       even though it may be supported by the server. It defaults to true so if it is
//       not set the library will act as it would have before the addition.
//     > Added the ability to pass the security type, m_type, the new m_Authenticate flag,
//       the login and password into the ConnectRemoteServer function. If these new arguments
//       are not included in the call the function will work as it did before.
//     > Added the ability to pass the new m_Authenticate flag into the SetIMAPServer function.
//       If not provided, the function will act as it would before the addition.
//     > Added fix described here: http://www.codeproject.com/Messages/3681792/Bug-when-reading-answer.aspx
//       - Thanks to Martin Kjallman!
//     > Added fixes described here: http://www.codeproject.com/Messages/3707662/Mistakes.aspx
//       - Thanks to Karpov Andrey!
//     > Added fixes described here: http://www.codeproject.com/Messages/3587166/Re-Possible-Solution-To-Misc-EHLO-Errors.aspx
//       - Thanks to Jakub Piwowarczyk!
// - Version 1.9: Started with Revion 6 in code project http://www.codeproject.com/script/Articles/ListVersions.aspx?aid=98355
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
//#include "PtBase/base.h"
#include "BaseObject.h"

#include "CSmtp.h"
#include "CIMAP.h"
#include "base64.h"
#include "openssl/err.h"

#include <deque>
#include <map>
#include <algorithm>
#include "BaseFile.h"

#include <cassert>
#include "BaseEmail.h"

#ifndef LINUX
//Add "openssl-0.9.8l\out32" to Additional Library Directories
#pragma comment(lib, "ssleay32.lib")
#pragma comment(lib, "libeay32.lib")
#endif

#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include "base.h"
#endif


PtObjectCpp(ST_Message);

Command_imap cmd_list_imap[] = 
{
	{command_INIT,          0,     5*60,	'K',' ','G', ECIMAP::SERVER_NOT_RESPONDING},
	{command_EHLO,          5*60,  5*60,	'\0','\0','\0', ECIMAP::COMMAND_EHLO},
	{command_USER,          5*60,  5*60,	' ','O','K', ECIMAP::UNDEF_XYZ_RESPONSE},
	{command_PASSWORD,      5*60,  5*60,	' ','O','K', ECIMAP::BAD_LOGIN_PASS},
	{command_QUIT,          5*60,  5*60,	'\0','\0','\0', ECIMAP::COMMAND_QUIT},
	{command_STARTTLS,      5*60,  5*60,	'\0','\0','\0', ECIMAP::COMMAND_EHLO_STARTTLS},
	{command_LIST,		    5*60,  5*60,	' ','O','K', ECIMAP::COMMAND_LIST},
	{command_MSG,			5*60,  5*60,	' ','O','K', ECIMAP::COMMAND_MSG},
	{command_COPY,			5*60,  5*60,	' ','O','K', ECIMAP::COMMAND_COPY},
	{command_DEL,			5*60,  5*60,	' ','O','K', ECIMAP::COMMAND_DEL}
};

static Command_imap* FindCommandEntry(SMTP_COMMAND command)
{
	Command_imap* pEntry = NULL;
	for(size_t i = 0; i < sizeof(cmd_list_imap)/sizeof(cmd_list_imap[0]); ++i)
	{
		if(cmd_list_imap[i].command == command)
		{
			pEntry = &cmd_list_imap[i];
			break;
		}
	}
	assert(pEntry != NULL);
	return pEntry;
}

bool IsKeywordSupported(const char* response, const char* keyword);
unsigned char* CharToUnsignedChar(const char *strIn);

////////////////////////////////////////////////////////////////////////////////
//        NAME: CIMAP
// DESCRIPTION: Constructor of CIMAP class.
//   ARGUMENTS: none
// USES GLOBAL: none
// MODIFIES GL: m_iXPriority, m_iIMAPSrvPort, RecvBuf, SendBuf
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////

CIMAP::CIMAP()
{
	hSocket = INVALID_SOCKET;
	m_bConnected = false;
	m_iXPriority = XPRIORITY_NORMAL;
	m_iIMAPSrvPort = 0;
	m_bAuthenticate = true;

#ifndef LINUX
	// Initialize WinSock
	WSADATA wsaData;
	WORD wVer = MAKEWORD(2,2);    
	if (WSAStartup(wVer,&wsaData) != NO_ERROR)
		throw ECIMAP(ECIMAP::WSA_STARTUP);
	if (LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		WSACleanup();
		throw ECIMAP(ECIMAP::WSA_VER);
	}
#endif

	char hostname[255];
	if(gethostname((char *) &hostname, 255) == SOCKET_ERROR) throw ECIMAP(ECIMAP::WSA_HOSTNAME);
	m_sLocalHostName = hostname;
	
	if((RecvBuf = new char[BUFFER_SIZE_IMAP]) == NULL)
		throw ECIMAP(ECIMAP::LACK_OF_MEMORY);
	
	if((SendBuf = new char[BUFFER_SIZE_IMAP]) == NULL)
		throw ECIMAP(ECIMAP::LACK_OF_MEMORY);

	m_type = NO_SECURITY;
	m_ctx = NULL;
	m_ssl = NULL;
	m_bHTML = false;
	m_bReadReceipt = false;

	m_sCharSet = "US-ASCII";

	strcpy_s(m_strMailBox, 128, "inbox");
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: CIMAP
// DESCRIPTION: Destructor of CIMAP class.
//   ARGUMENTS: none
// USES GLOBAL: RecvBuf, SendBuf
// MODIFIES GL: RecvBuf, SendBuf
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
CIMAP::~CIMAP()
{
	if(m_bConnected) DisconnectRemoteServer();

	if(SendBuf)
	{
		delete[] SendBuf;
		SendBuf = NULL;
	}
	if(RecvBuf)
	{
		delete[] RecvBuf;
		RecvBuf = NULL;
	}

	CleanupOpenSSL();

#ifndef LINUX
	WSACleanup();
#endif
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: AddAttachment
// DESCRIPTION: New attachment is added.
//   ARGUMENTS: const char *Path - name of attachment added
// USES GLOBAL: Attachments
// MODIFIES GL: Attachments
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::AddAttachment(const char *Path)
{
	assert(Path);
	Attachments.insert(Attachments.end(), Path);
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: AddRecipient
// DESCRIPTION: New recipient data is added i.e.: email and name. .
//   ARGUMENTS: const char *email - mail of the recipient
//              const char *name - name of the recipient
// USES GLOBAL: Recipients
// MODIFIES GL: Recipients
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::AddRecipient(const char *email, const char *name)
{	
	if(!email)
		throw ECIMAP(ECIMAP::UNDEF_RECIPIENT_MAIL);

	Recipient recipient;
	recipient.Mail = email;
	if(name!=NULL) recipient.Name = name;
	else recipient.Name.empty();

	Recipients.insert(Recipients.end(), recipient);   
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: AddCCRecipient
// DESCRIPTION: New cc-recipient data is added i.e.: email and name. .
//   ARGUMENTS: const char *email - mail of the cc-recipient
//              const char *name - name of the ccc-recipient
// USES GLOBAL: CCRecipients
// MODIFIES GL: CCRecipients
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::AddCCRecipient(const char *email, const char *name)
{	
	if(!email)
		throw ECIMAP(ECIMAP::UNDEF_RECIPIENT_MAIL);

	Recipient recipient;
	recipient.Mail = email;
	if(name!=NULL) recipient.Name = name;
	else recipient.Name.empty();

	CCRecipients.insert(CCRecipients.end(), recipient);
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: AddBCCRecipient
// DESCRIPTION: New bcc-recipient data is added i.e.: email and name. .
//   ARGUMENTS: const char *email - mail of the bcc-recipient
//              const char *name - name of the bccc-recipient
// USES GLOBAL: BCCRecipients
// MODIFIES GL: BCCRecipients
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::AddBCCRecipient(const char *email, const char *name)
{	
	if(!email)
		throw ECIMAP(ECIMAP::UNDEF_RECIPIENT_MAIL);

	Recipient recipient;
	recipient.Mail = email;
	if(name!=NULL) recipient.Name = name;
	else recipient.Name.empty();

	BCCRecipients.insert(BCCRecipients.end(), recipient);
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: AddMsgLine
// DESCRIPTION: Adds new line in a message.
//   ARGUMENTS: const char *Text - text of the new line
// USES GLOBAL: MsgBody
// MODIFIES GL: MsgBody
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::AddMsgLine(const char* Text)
{
	MsgBody.insert(MsgBody.end(), Text);
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: DelMsgLine
// DESCRIPTION: Deletes specified line in text message.. .
//   ARGUMENTS: unsigned int Line - line to be delete
// USES GLOBAL: MsgBody
// MODIFIES GL: MsgBody
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::DelMsgLine(unsigned int Line)
{
	if(Line > MsgBody.size())
		throw ECIMAP(ECIMAP::OUT_OF_MSG_RANGE);
	MsgBody.erase(MsgBody.begin()+Line);
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: DelRecipients
// DESCRIPTION: Deletes all recipients. .
//   ARGUMENTS: void
// USES GLOBAL: Recipients
// MODIFIES GL: Recipients
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::DelRecipients()
{
	Recipients.clear();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: DelBCCRecipients
// DESCRIPTION: Deletes all BCC recipients. .
//   ARGUMENTS: void
// USES GLOBAL: BCCRecipients
// MODIFIES GL: BCCRecipients
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::DelBCCRecipients()
{
	BCCRecipients.clear();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: DelCCRecipients
// DESCRIPTION: Deletes all CC recipients. .
//   ARGUMENTS: void
// USES GLOBAL: CCRecipients
// MODIFIES GL: CCRecipients
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::DelCCRecipients()
{
	CCRecipients.clear();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: DelMsgLines
// DESCRIPTION: Deletes message text.
//   ARGUMENTS: void
// USES GLOBAL: MsgBody
// MODIFIES GL: MsgBody
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::DelMsgLines()
{
	MsgBody.clear();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: DelAttachments
// DESCRIPTION: Deletes all recipients. .
//   ARGUMENTS: void
// USES GLOBAL: Attchments
// MODIFIES GL: Attachments
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::DelAttachments()
{
	Attachments.clear();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: AddBCCRecipient
// DESCRIPTION: New bcc-recipient data is added i.e.: email and name. .
//   ARGUMENTS: const char *email - mail of the bcc-recipient
//              const char *name - name of the bccc-recipient
// USES GLOBAL: BCCRecipients
// MODIFIES GL: BCCRecipients, m_oError
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::ModMsgLine(unsigned int Line,const char* Text)
{
	if(Text)
	{
		if(Line > MsgBody.size())
			throw ECIMAP(ECIMAP::OUT_OF_MSG_RANGE);
		MsgBody.at(Line) = std::string(Text);
	}
}

void CIMAP::MoveMsg(STLVInt *_pstlVMsgs, const char *_strMsgBox)
{
	char strBuf[4096];

	Command_imap* pEntry = FindCommandEntry(command_COPY);
	sprintf(SendBuf, "tag copy ");
	for(unsigned i=0;i<_pstlVMsgs->size(); i++)
	{
		if(i+1 == _pstlVMsgs->size())
			sprintf(strBuf, "%d", _pstlVMsgs->at(i)+1);
		else
			sprintf(strBuf, "%d,", _pstlVMsgs->at(i)+1);

		strcat_s(SendBuf, 4096, strBuf);
	}
	sprintf(strBuf, " %s\r\n", _strMsgBox);
	strcat_s(SendBuf, 4096, strBuf);

	SendData(pEntry);
	ReceiveResponse(pEntry);

	pEntry = FindCommandEntry(command_DEL);
	sprintf(SendBuf, "tag store ");
	for(unsigned i=0;i<_pstlVMsgs->size(); i++)
	{
		if(i+1 == _pstlVMsgs->size())
			sprintf(strBuf, "%d", _pstlVMsgs->at(i)+1);
		else
			sprintf(strBuf, "%d,", _pstlVMsgs->at(i)+1);

		strcat_s(SendBuf, 4096, strBuf);
	}
	sprintf(strBuf, " +flags (\\deleted)\r\n");
	strcat_s(SendBuf, 4096, strBuf);

	SendData(pEntry);
	ReceiveResponse(pEntry);

}

PT_OPTCPP(CIMAP)
PT_OPTCPP(ST_Message)

int CIMAP::GetList()
{
	int	nCnt = 0;

	Command_imap* pEntry = FindCommandEntry(command_LIST);
	sprintf(SendBuf, "tag fetch 1:* flags\r\n");
	SendData(pEntry);
	ReceiveResponse(pEntry);

	BaseFile paser;

	paser.set_asc_seperator(" /\\");
	paser.set_asc_deletor(" ()");

	paser.OpenFile((void*)RecvBuf, strlen(RecvBuf));

	m_stlVpstMessage.clear();

	while(paser.read_asc_line())
	{
		ST_Message *pMsg;
		PT_OAlloc(pMsg, ST_Message);

		pMsg->bReaded	= false;
		pMsg->bRetrieved	= false;

		char strBuf[255];
		bool bMsg = false;
		int nIndex = 0;
		while(paser.read_asc_string(strBuf, 255))
		{
			if(nIndex == 0 && *strBuf == '*')
			{
				bMsg	= true;	
			}if(nIndex >= 4 && strncmp(strBuf, "Seen", 4) == 0)
				pMsg->bReaded = true;

			nIndex++;
		}

		if(bMsg)
		{
			m_stlVpstMessage.push_back(pMsg);
			nCnt++;
		}else
		{
			PT_OFree(pMsg);
		}
	}

	//RecvBuf

	return nCnt;
}

STLVInt *CIMAP::GetUnreadedMsg(STLVInt *_pstlVUnreaded)
{
	for(unsigned i=0; i<m_stlVpstMessage.size(); i++)
	{
		if(m_stlVpstMessage[i]->bReaded == false)
			_pstlVUnreaded->push_back(i);
	}

	return _pstlVUnreaded;
}

ST_Message *CIMAP::RetrieveMessage(int _nIndex)
{
	if(_nIndex < 0 
		|| _nIndex >= (int)m_stlVpstMessage.size())
		return NULL;

	if(m_stlVpstMessage[_nIndex]->bRetrieved)
		return m_stlVpstMessage[_nIndex];

	ST_Message	*pMsg;
	pMsg	= m_stlVpstMessage[_nIndex];

	try{
		g_SendMessage(LOG_MSG_FILELOG, "Start parsing ----------------------.\n");
		Command_imap* pEntry = FindCommandEntry(command_MSG);
		sprintf(SendBuf, "tag fetch %d body[header]\r\n", _nIndex+1);
		SendData(pEntry);
		ReceiveResponse(pEntry);

		BaseFile paser;

		paser.set_asc_seperator(" <>");
		paser.set_asc_deletor(" \"<>");

		//tag fetch 1 body[header]
		//* 1 FETCH (BODY[HEADER] {2132}
		//Delivered-To: kjchoose929@gmail.com
		//Received: by 10.76.11.135 with SMTP id q7csp258701oab; Tue, 4 Dec 2012
		//04:18:17 -0800 (PST)
		//Received: by 10.68.231.41 with SMTP id td9mr37806545pbc.128.1354623496973;
		//Tue, 04 Dec 2012 04:18:16 -0800 (PST)
		//Return-Path: <payment@paypal.com>
		//Received: from mx0.slc.paypal.com (mx0.slc.paypal.com. [173.0.84.225]) by
		//mx.google.com with ESMTP id xo9si2157342pbc.103.2012.12.04.04.18.15; Tue, 04
		//Dec 2012 04:18:16 -0800 (PST)
		//Received-SPF: pass (google.com: domain of payment@paypal.com designates
		//173.0.84.225 as permitted sender) client-ip=173.0.84.225;
		//Authentication-Results: mx.google.com; spf=pass (google.com: domain of
		//payment@paypal.com designates 173.0.84.225 as permitted sender)
		//smtp.mail=payment@paypal.com; dkim=pass header.i=@paypal.com
		//DomainKey-Signature: q=dns; a=rsa-sha1; c=nofws; s=dkim; d=paypal.com;
		//h=DKIM-Signature:Received:Date:Message-Id:Subject:X-MaxCode-Template:To:From:Sender:X-Email-Type-Id:X-XPT-XSL-Name:Content-Type:MIME-Version;
		//b=sRVxWGTEgui0uHwKlSmb82jMK0Vr8KV41DbTJfIdgUyljC/BH+S9b63TonMrCnte
		//SEbKmUFz5p/dFWnITZmDPkKrCk/7M5YLnqOip/rRefifPSHd3T7tanVpSl3Ecexx
		//Cc7Jge8ovXmQDCG5EpdjolrHc6smTuKtHppm6xsZipo=
		//DKIM-Signature: v=1; a=rsa-sha1; d=paypal.com; s=dkim; c=relaxed/relaxed;
		//q=dns/txt; i=@paypal.com; t=1354623495;
		//h=From:From:Subject:Date:To:MIME-Version:Content-Type;
		//bh=HTb+kEPLZEpZLaWDEtP65dC6wAI=;
		//b=9gT2JhCE1aQbKoiyHUAvKqPNEPCfXIhmrYoOEy7uvVQ4Z8Ry+Z8bV9zVydNSGTtM
		//KTuR+hWv+Lb3sHIND7y9B8XBBDT59i/BmMQ9tA9FwpdivfJNxkU0yUtFU4hHRfkJ
		//HKgyZwyEp64A/Jx+nH3S90gUyTP8I5VkJDq46XGbeUE=;
		//Received: (qmail 30024 invoked by uid 993); 4 Dec 2012 12:18:15 -0000
		//Date: Tue, 04 Dec 2012 04:18:15 -0800
		//Message-Id: <1354623495.30024@paypal.com>
		//Subject: billing mail
		//X-MaxCode-Template: email-standard-transaction-counterparty
		//To: MinJung Kim <kjchoose929@gmail.com>
		//From: "jtseo@me.com" <jtseo@me.com>
		//Sender: sendmail@paypal.com
		//X-Email-Type-Id: PP1546
		//X-XPT-XSL-Name: email_pimp/default/en_US/transaction/Recipient.xsl
		//Content-Type: multipart/alternative; boundary=--NextPart_048F8BC8A2197DE2036A
		//MIME-Version: 1.0
		//
		//)
		//tag OK Success

		paser.OpenFile((void*)RecvBuf, strlen(RecvBuf));

		char strBuffer[255];
		int nLast;
		while(paser.read_asc_line())
		{
			paser.read_asc_string(strBuffer, 255);

			if(_stricmp(strBuffer, "Subject:")==0)
			{
				paser.read_asc_string(strBuffer, 255);
				pMsg->strSubject	= strBuffer;
				paser.read_asc_string(strBuffer, 255);
				pMsg->strSubject	+= " ";
				pMsg->strSubject	+= strBuffer;

				g_SendMessage(LOG_MSG_FILELOG, "Subject: %s\n", pMsg->strSubject.c_str());
			}

			if(_stricmp(strBuffer, "Sender:")==0)
			{
				paser.read_asc_string(strBuffer, 255);
				pMsg->strSender	= strBuffer;
				g_SendMessage(LOG_MSG_FILELOG, "Sender: %s\n", pMsg->strSender.c_str());
			}

			if(_stricmp(strBuffer, "From:")==0)
			{
				while(paser.read_asc_string(strBuffer, 255))
				{
					if(strchr(strBuffer, '@'))
						pMsg->strFrom	= strBuffer;

					nLast = paser.get_seperator_last();
					if(nLast == '>')
						pMsg->strFrom	= strBuffer;
				}

				g_SendMessage(LOG_MSG_FILELOG, "From: %s\n", pMsg->strFrom.c_str());
			}

			if(_stricmp(strBuffer, "To:")==0)
			{
				paser.read_asc_string(strBuffer, 255);
				pMsg->strTo	= strBuffer;
				g_SendMessage(LOG_MSG_FILELOG, "To: %s\n", pMsg->strTo.c_str());
			}

			if(_stricmp(strBuffer, "Reply-To:")==0)
			{
				while(paser.read_asc_string(strBuffer, 255))
				{
					//char *pStart = strchr(strBuffer, '@');
					if(strchr(strBuffer, '@'))
						pMsg->strReplyTo	= strBuffer;
				}

				g_SendMessage(LOG_MSG_FILELOG, "Reply-To: %s\n", pMsg->strReplyTo.c_str());
			}

			if(strcmp(strBuffer, "Return-Path:")==0)
			{
				while(paser.read_asc_string(strBuffer, 255))
				{
					char *pStart = strchr(strBuffer, '@');
					if(strchr(strBuffer, '@'))
						pMsg->strReturnPath	= pStart;
				}

				g_SendMessage(LOG_MSG_FILELOG, "Return-Path: %s\n", pMsg->strReturnPath.c_str());
			}

			if(_stricmp(strBuffer, "Date:")==0)
			{
				paser.read_asc_leave_line(strBuffer, 255);
				pMsg->strDate	= strBuffer;

				g_SendMessage(LOG_MSG_FILELOG, "Date: %s\n", pMsg->strDate.c_str());
			}
		}


		sprintf(SendBuf, "tag fetch %d body[1]\r\n", _nIndex+1);
		SendData(pEntry);
		ReceiveResponse(pEntry);

		pMsg->strMsg	= RecvBuf;
		pMsg->bRetrieved	= true;
		//tag fetch 1 body[1]
		//* 1 FETCH (BODY[1] {1853}
		//----------------------------------------------------------------------
		//Jung Tae Seo sent you =240.10 USD
		//----------------------------------------------------------------------

		//Transaction ID: 49K41314KA702383U
		//Dear MinJung Kim,



		//Jung Tae Seo sent you a payment of =240.10 USD.

		//----------------------------------------------------------------------
		//Note from Jung Tae Seo:receivemail test



		//Get the details
		//https://www.paypal.com/kr/cgi-bin/?cmd=3D_view-a-trans&id=3D49K41314KA70238=
		//3U

		//Once you've received the payment, you can:
		//Spend the money online at thousands of stores that accept PayPal.
		//Withdraw or transfer it to your bank account (takes 2-3 days).


		//Don't see the money in your account?

		//Don't worry - sometimes it just takes a few minutes for it to show up.



		//Shipping address
		//Jung Tae Seo
		//StarVill 302, 137 BunGi
		//SongPa-Dong, SongPa-Gu
		//Seoul, Seoul=20
		//138-852
		//South Korea


		//Yours sincerely,
		//PayPal
		//=20
		//----------------------------------------------------------------------
		//Help Centre:=20
		//https://www.paypal.com/kr/cgi-bin/helpweb?cmd=3D_help
		//Resolution Centre:=20
		//https://www.paypal.com/kr/cgi-bin/?cmd=3D_complaint-view
		//Security Centre:=20
		//https://www.paypal.com/kr/security

		//Please do not reply to this email because we are not monitoring this =
		//inbox. To get in touch with us, log in to your account and click =
		//=22Contact Us=22 at the bottom of any page.

		//Copyright =A9 2012 PayPal Inc. All rights reserved.

		//Consumer advisory: PayPal Pte Ltd, the Holder of the PayPal=99 payment =
		//service stored value facility, does not require the approval of the =
		//Monetary Authority of Singapore. Consumers (users) are advised to read the =
		//terms and conditions: =
		//https://www.paypal.com/kr/sg/cgi-bin/webscr?cmd=3Dp/gen/ua/ua-outside =
		//carefully.=20
		//PayPal Email ID  PP1546

		//)
		//tag OK Success
	}
	catch(const ECIMAP&)
	{
		DisconnectRemoteServer();
		throw;
	}

	g_SendMessage(LOG_MSG_FILELOG, "End parsing +++++++++++++++++++++++++++++.\n");

	return pMsg;
}

void CIMAP::Disconnect()
{
	DisconnectRemoteServer();
}
////////////////////////////////////////////////////////////////////////////////
//        NAME: Send
// DESCRIPTION: Sending the mail. .
//   ARGUMENTS: none
// USES GLOBAL: m_sIMAPSrvName, m_iIMAPSrvPort, SendBuf, RecvBuf, m_sLogin,
//              m_sPassword, m_sMailFrom, Recipients, CCRecipients,
//              BCCRecipients, m_sMsgBody, Attachments, 
// MODIFIES GL: SendBuf 
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::Login()
{
	//char *FileBuf = NULL, *FileName = NULL;
	//FILE* hFile = NULL;

	// ***** CONNECTING TO IMAP SERVER *****

	// connecting to remote host if not already connected:
	if(hSocket==INVALID_SOCKET)
	{
		if(!ConnectRemoteServer(m_sIMAPSrvName.c_str(), m_iIMAPSrvPort, m_type, m_bAuthenticate))
			throw ECIMAP(ECIMAP::WSA_INVALID_SOCKET);
	}

	try{
		GetList();
	}
	catch(const ECIMAP&)
	{
		DisconnectRemoteServer();
		throw;
	}
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: ConnectRemoteServer
// DESCRIPTION: Connecting to the service running on the remote server. 
//   ARGUMENTS: const char *server - service name
//              const unsigned short port - service port
// USES GLOBAL: m_pCIMAPSrvName, m_iIMAPSrvPort, SendBuf, RecvBuf, m_pcLogin,
//              m_pcPassword, m_pcMailFrom, Recipients, CCRecipients,
//              BCCRecipients, m_pcMsgBody, Attachments, 
// MODIFIES GL: m_oError 
//     RETURNS: socket of the remote service
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
bool CIMAP::ConnectRemoteServer(const char* szServer, const unsigned short nPort_/*=0*/, 
								SMTP_SECURITY_TYPE securityType/*=DO_NOT_SET*/,
								bool authenticate/*=true*/, const char* login/*=NULL*/,
								const char* password/*=NULL*/)
{
	unsigned short nPort = 0;
	LPSERVENT lpServEnt;
	SOCKADDR_IN sockAddr;
	unsigned long ul = 1;
	fd_set fdwrite,fdexcept;
	timeval timeout;
	int res = 0;

	try
	{
		timeout.tv_sec = TIME_IN_SEC;
		timeout.tv_usec = 0;

		hSocket = INVALID_SOCKET;

		//if ((hSocket = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		if((hSocket = socket(AF_INET, SOCK_STREAM,0)) == INVALID_SOCKET)
			throw ECIMAP(ECIMAP::WSA_INVALID_SOCKET);

		if(nPort_ != 0)
			nPort = htons(nPort_);
		else
		{
			lpServEnt = getservbyname("mail", 0);
			if (lpServEnt == NULL)
				nPort = htons(25);
			else 
				nPort = lpServEnt->s_port;
		}
				
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = nPort;
		if((sockAddr.sin_addr.s_addr = inet_addr(szServer)) == INADDR_NONE)
		{
			LPHOSTENT host;
				
			host = gethostbyname(szServer);
			if (host)
				memcpy(&sockAddr.sin_addr,host->h_addr_list[0],host->h_length);
			else
			{
#ifdef LINUX
				close(hSocket);
#else
				closesocket(hSocket);
#endif
				throw ECIMAP(ECIMAP::WSA_GETHOSTBY_NAME_ADDR);
			}				
		}

		// start non-blocking mode for socket:
#ifdef LINUX
		if(ioctl(hSocket,FIONBIO, (unsigned long*)&ul) == SOCKET_ERROR)
#else
		if(ioctlsocket(hSocket,FIONBIO, (unsigned long*)&ul) == SOCKET_ERROR)
#endif
		{
#ifdef LINUX
			close(hSocket);
#else
			closesocket(hSocket);
#endif
			throw ECIMAP(ECIMAP::WSA_IOCTLSOCKET);
		}

		if(connect(hSocket,(LPSOCKADDR)&sockAddr,sizeof(sockAddr)) == SOCKET_ERROR)
		{
#ifdef LINUX
			if(errno != EINPROGRESS)
#else
			if(WSAGetLastError() != WSAEWOULDBLOCK)
#endif
			{
#ifdef LINUX
				close(hSocket);
#else
				closesocket(hSocket);
#endif
				throw ECIMAP(ECIMAP::WSA_CONNECT);
			}
		}
		else
			return true;

		while(true)
		{
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcept);

			FD_SET(hSocket,&fdwrite);
			FD_SET(hSocket,&fdexcept);

			if((res = select(hSocket+1,NULL,&fdwrite,&fdexcept,&timeout)) == SOCKET_ERROR)
			{
#ifdef LINUX
				close(hSocket);
#else
				closesocket(hSocket);
#endif
				throw ECIMAP(ECIMAP::WSA_SELECT);
			}

			if(!res)
			{
#ifdef LINUX
				close(hSocket);
#else
				closesocket(hSocket);
#endif
				throw ECIMAP(ECIMAP::SELECT_TIMEOUT);
			}
			if(res && FD_ISSET(hSocket,&fdwrite))
				break;
			if(res && FD_ISSET(hSocket,&fdexcept))
			{
#ifdef LINUX
				close(hSocket);
#else
				closesocket(hSocket);
#endif
				throw ECIMAP(ECIMAP::WSA_SELECT);
			}
		} // while

		FD_CLR(hSocket,&fdwrite);
		FD_CLR(hSocket,&fdexcept);

		if(securityType!=DO_NOT_SET) SetSecurityType(securityType);
		if(GetSecurityType() == USE_TLS || GetSecurityType() == USE_SSL)
		{
			InitOpenSSL();
			if(GetSecurityType() == USE_SSL)
			{
				OpenSSLConnect();
			}
		}

		Command_imap* pEntry = FindCommandEntry(command_INIT);
		ReceiveResponse(pEntry);

		m_bConnected	= true;
		// send login:
		pEntry = FindCommandEntry(command_USER);
		sprintf(SendBuf,"tag LOGIN %s %s\r\n",m_sLogin.c_str(), m_sPassword.c_str());
		SendData(pEntry);
		ReceiveResponse(pEntry);

		// send select inbox:
		pEntry = FindCommandEntry(command_USER);
		sprintf(SendBuf,"tag select %s\r\n", m_strMailBox);
		SendData(pEntry);
		ReceiveResponse(pEntry);
	}
	catch(const ECIMAP&)
	{
		if(RecvBuf[0]=='5' && RecvBuf[1]=='3' && RecvBuf[2]=='0')
			m_bConnected=false;
		DisconnectRemoteServer();
		throw;
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: DisconnectRemoteServer
// DESCRIPTION: Disconnects from the IMAP server and closes the socket
//   ARGUMENTS: none
// USES GLOBAL: none
// MODIFIES GL: none
//     RETURNS: void
//      AUTHOR: David Johns
// AUTHOR/DATE: DRJ 2010-08-14
////////////////////////////////////////////////////////////////////////////////
void CIMAP::DisconnectRemoteServer()
{
	if(m_bConnected) SayQuit();
	if(hSocket)
	{
#ifdef LINUX
		close(hSocket);
#else
		closesocket(hSocket);
#endif
	}
	hSocket = INVALID_SOCKET;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: IMAPXYZdigits
// DESCRIPTION: Converts three letters from RecvBuf to the number.
//   ARGUMENTS: none
// USES GLOBAL: RecvBuf
// MODIFIES GL: none
//     RETURNS: integer number
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
int CIMAP::IMAPXYZdigits()
{
	assert(RecvBuf);
	if(RecvBuf == NULL)
		return 0;
	return (RecvBuf[0]-'0')*100 + (RecvBuf[1]-'0')*10 + RecvBuf[2]-'0';
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: FormatHeader
// DESCRIPTION: Prepares a header of the message.
//   ARGUMENTS: char* header - formated header string
// USES GLOBAL: Recipients, CCRecipients, BCCRecipients
// MODIFIES GL: none
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void CIMAP::FormatHeader(char* header)
{
	char month[][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	size_t i;
	std::string to;
	std::string cc;
	std::string bcc;
	time_t rawtime;
	struct tm* timeinfo;

	// date/time check
	if(time(&rawtime) > 0)
		timeinfo = localtime(&rawtime);
	else
		throw ECIMAP(ECIMAP::TIME_ERROR);

	// check for at least one recipient
	if(Recipients.size())
	{
		for (i=0;i<Recipients.size();i++)
		{
			if(i > 0)
				to.append(",");
			to += Recipients[i].Name;
			to.append("<");
			to += Recipients[i].Mail;
			to.append(">");
		}
	}
	else
		throw ECIMAP(ECIMAP::UNDEF_RECIPIENTS);

	if(CCRecipients.size())
	{
		for (i=0;i<CCRecipients.size();i++)
		{
			if(i > 0)
				cc. append(",");
			cc += CCRecipients[i].Name;
			cc.append("<");
			cc += CCRecipients[i].Mail;
			cc.append(">");
		}
	}

	if(BCCRecipients.size())
	{
		for (i=0;i<BCCRecipients.size();i++)
		{
			if(i > 0)
				bcc.append(",");
			bcc += BCCRecipients[i].Name;
			bcc.append("<");
			bcc += BCCRecipients[i].Mail;
			bcc.append(">");
		}
	}
	
	// Date: <SP> <dd> <SP> <mon> <SP> <yy> <SP> <hh> ":" <mm> ":" <ss> <SP> <zone> <CRLF>
	sprintf(header,"Date: %d %s %d %d:%d:%d\r\n",	timeinfo->tm_mday,
																								month[timeinfo->tm_mon],
																								timeinfo->tm_year+1900,
																								timeinfo->tm_hour,
																								timeinfo->tm_min,
																								timeinfo->tm_sec); 
	
	// From: <SP> <sender>  <SP> "<" <sender-email> ">" <CRLF>
	if(!m_sMailFrom.size()) throw ECIMAP(ECIMAP::UNDEF_MAIL_FROM);
	 
	strcat(header,"From: ");
	if(m_sNameFrom.size()) strcat(header, m_sNameFrom.c_str());
	 
	strcat(header," <");
	strcat(header,m_sMailFrom.c_str());
	strcat(header, ">\r\n");

	// X-Mailer: <SP> <xmailer-app> <CRLF>
	if(m_sXMailer.size())
	{
		strcat(header,"X-Mailer: ");
		strcat(header, m_sXMailer.c_str());
		strcat(header, "\r\n");
	}

	// Reply-To: <SP> <reverse-path> <CRLF>
	if(m_sReplyTo.size())
	{
		strcat(header, "Reply-To: ");
		strcat(header, m_sReplyTo.c_str());
		strcat(header, "\r\n");
	}

	// Disposition-Notification-To: <SP> <reverse-path or sender-email> <CRLF>
	if(m_bReadReceipt)
	{
		strcat(header, "Disposition-Notification-To: ");
		if(m_sReplyTo.size()) strcat(header, m_sReplyTo.c_str());
		else strcat(header, m_sNameFrom.c_str());
		strcat(header, "\r\n");
	}

	// X-Priority: <SP> <number> <CRLF>
	switch(m_iXPriority)
	{
		case XPRIORITY_HIGH:
			strcat(header,"X-Priority: 2 (High)\r\n");
			break;
		case XPRIORITY_NORMAL:
			strcat(header,"X-Priority: 3 (Normal)\r\n");
			break;
		case XPRIORITY_LOW:
			strcat(header,"X-Priority: 4 (Low)\r\n");
			break;
		default:
			strcat(header,"X-Priority: 3 (Normal)\r\n");
	}

	// To: <SP> <remote-user-mail> <CRLF>
	strcat(header,"To: ");
	strcat(header, to.c_str());
	strcat(header, "\r\n");

	// Cc: <SP> <remote-user-mail> <CRLF>
	if(CCRecipients.size())
	{
		strcat(header,"Cc: ");
		strcat(header, cc.c_str());
		strcat(header, "\r\n");
	}

	if(BCCRecipients.size())
	{
		strcat(header,"Bcc: ");
		strcat(header, bcc.c_str());
		strcat(header, "\r\n");
	}

	// Subject: <SP> <subject-text> <CRLF>
	if(!m_sSubject.size()) 
		strcat(header, "Subject:  ");
	else
	{
	  strcat(header, "Subject: ");
	  strcat(header, m_sSubject.c_str());
	}
	strcat(header, "\r\n");
	
	// MIME-Version: <SP> 1.0 <CRLF>
	strcat(header,"MIME-Version: 1.0\r\n");
	if(!Attachments.size())
	{ // no attachments
		if(m_bHTML) strcat(header, "Content-Type: text/html; charset=\"");
		else strcat(header, "Content-type: text/plain; charset=\"");
		strcat(header, m_sCharSet.c_str());
		strcat(header, "\"\r\n");
		strcat(header,"Content-Transfer-Encoding: 7bit\r\n");
		strcat(SendBuf,"\r\n");
	}
	else
	{ // there is one or more attachments
		strcat(header,"Content-Type: multipart/mixed; boundary=\"");
		strcat(header,BOUNDARY_TEXT);
		strcat(header,"\"\r\n");
		strcat(header,"\r\n");
		// first goes text message
		strcat(SendBuf,"--");
		strcat(SendBuf,BOUNDARY_TEXT);
		strcat(SendBuf,"\r\n");
		if(m_bHTML) strcat(SendBuf,"Content-type: text/html; charset=");
		else strcat(SendBuf,"Content-type: text/plain; charset=");
		strcat(header, m_sCharSet.c_str());
		strcat(header, "\r\n");
		strcat(SendBuf,"Content-Transfer-Encoding: 7bit\r\n");
		strcat(SendBuf,"\r\n");
	}

	// done
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: ReceiveData
// DESCRIPTION: Receives a row terminated '\n'.
//   ARGUMENTS: none
// USES GLOBAL: RecvBuf
// MODIFIES GL: RecvBuf
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// MODIFICATION: Receives data as much as possible. Another function ReceiveResponse
//               will ensure the received data contains '\n'
// AUTHOR/DATE:  John Tang 2010-08-01
////////////////////////////////////////////////////////////////////////////////
void CIMAP::ReceiveData(Command_imap* pEntry)
{
	if(m_ssl != NULL)
	{
		ReceiveData_SSL(m_ssl, pEntry);
		return;
	}
	int res = 0;
	fd_set fdread;
	timeval time;

	time.tv_sec = pEntry->recv_timeout;
	time.tv_usec = 0;

	assert(RecvBuf);

	if(RecvBuf == NULL)
		throw ECIMAP(ECIMAP::RECVBUF_IS_EMPTY);

	FD_ZERO(&fdread);

	FD_SET(hSocket,&fdread);

	if((res = select(hSocket+1, &fdread, NULL, NULL, &time)) == SOCKET_ERROR)
	{
		FD_CLR(hSocket,&fdread);
		throw ECIMAP(ECIMAP::WSA_SELECT);
	}

	if(!res)
	{
		//timeout
		FD_CLR(hSocket,&fdread);
		throw ECIMAP(ECIMAP::SERVER_NOT_RESPONDING);
	}

	if(FD_ISSET(hSocket,&fdread))
	{
		res = recv(hSocket,RecvBuf,BUFFER_SIZE_IMAP,0);
		if(res == SOCKET_ERROR)
		{
			FD_CLR(hSocket,&fdread);
			throw ECIMAP(ECIMAP::WSA_RECV);
		}
	}

	FD_CLR(hSocket,&fdread);
	RecvBuf[res] = 0;
	if(res == 0)
	{
		throw ECIMAP(ECIMAP::CONNECTION_CLOSED);
	}
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SendData
// DESCRIPTION: Sends data from SendBuf buffer.
//   ARGUMENTS: none
// USES GLOBAL: SendBuf
// MODIFIES GL: none
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SendData(Command_imap* pEntry)
{
	if(m_ssl != NULL)
	{
		SendData_SSL(m_ssl, pEntry);
		return;
	}
	int idx = 0,res,nLeft = strlen(SendBuf);
	fd_set fdwrite;
	timeval time;

	time.tv_sec = pEntry->send_timeout;
	time.tv_usec = 0;

	assert(SendBuf);

	if(SendBuf == NULL)
		throw ECIMAP(ECIMAP::SENDBUF_IS_EMPTY);

	while(nLeft > 0)
	{
		FD_ZERO(&fdwrite);

		FD_SET(hSocket,&fdwrite);

		if((res = select(hSocket+1,NULL,&fdwrite,NULL,&time)) == SOCKET_ERROR)
		{
			FD_CLR(hSocket,&fdwrite);
			throw ECIMAP(ECIMAP::WSA_SELECT);
		}

		if(!res)
		{
			//timeout
			FD_CLR(hSocket,&fdwrite);
			throw ECIMAP(ECIMAP::SERVER_NOT_RESPONDING);
		}

		if(res && FD_ISSET(hSocket,&fdwrite))
		{
			res = send(hSocket,&SendBuf[idx],nLeft,0);
			if(res == SOCKET_ERROR || res == 0)
			{
				FD_CLR(hSocket,&fdwrite);
				throw ECIMAP(ECIMAP::WSA_SEND);
			}
			nLeft -= res;
			idx += res;
		}
	}

	OutputDebugStringA(SendBuf);
	FD_CLR(hSocket,&fdwrite);
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetLocalHostName
// DESCRIPTION: Returns local host name. 
//   ARGUMENTS: none
// USES GLOBAL: m_pcLocalHostName
// MODIFIES GL: m_oError, m_pcLocalHostName 
//     RETURNS: socket of the remote service
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
const char* CIMAP::GetLocalHostName()
{
	return m_sLocalHostName.c_str();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetRecipientCount
// DESCRIPTION: Returns the number of recipents.
//   ARGUMENTS: none
// USES GLOBAL: Recipients
// MODIFIES GL: none 
//     RETURNS: number of recipents
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
unsigned int CIMAP::GetRecipientCount() const
{
	return Recipients.size();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetBCCRecipientCount
// DESCRIPTION: Returns the number of bcc-recipents. 
//   ARGUMENTS: none
// USES GLOBAL: BCCRecipients
// MODIFIES GL: none 
//     RETURNS: number of bcc-recipents
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
unsigned int CIMAP::GetBCCRecipientCount() const
{
	return BCCRecipients.size();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetCCRecipientCount
// DESCRIPTION: Returns the number of cc-recipents.
//   ARGUMENTS: none
// USES GLOBAL: CCRecipients
// MODIFIES GL: none 
//     RETURNS: number of cc-recipents
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
unsigned int CIMAP::GetCCRecipientCount() const
{
	return CCRecipients.size();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetReplyTo
// DESCRIPTION: Returns m_pcReplyTo string.
//   ARGUMENTS: none
// USES GLOBAL: m_sReplyTo
// MODIFIES GL: none 
//     RETURNS: m_sReplyTo string
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
const char* CIMAP::GetReplyTo() const
{
	return m_sReplyTo.c_str();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetMailFrom
// DESCRIPTION: Returns m_pcMailFrom string.
//   ARGUMENTS: none
// USES GLOBAL: m_sMailFrom
// MODIFIES GL: none 
//     RETURNS: m_sMailFrom string
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
const char* CIMAP::GetMailFrom() const
{
	return m_sMailFrom.c_str();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetSenderName
// DESCRIPTION: Returns m_pcNameFrom string.
//   ARGUMENTS: none
// USES GLOBAL: m_sNameFrom
// MODIFIES GL: none 
//     RETURNS: m_sNameFrom string
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
const char* CIMAP::GetSenderName() const
{
	return m_sNameFrom.c_str();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetSubject
// DESCRIPTION: Returns m_pcSubject string.
//   ARGUMENTS: none
// USES GLOBAL: m_sSubject
// MODIFIES GL: none 
//     RETURNS: m_sSubject string
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
const char* CIMAP::GetSubject() const
{
	return m_sSubject.c_str();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetXMailer
// DESCRIPTION: Returns m_pcXMailer string.
//   ARGUMENTS: none
// USES GLOBAL: m_pcXMailer
// MODIFIES GL: none 
//     RETURNS: m_pcXMailer string
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
const char* CIMAP::GetXMailer() const
{
	return m_sXMailer.c_str();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetXPriority
// DESCRIPTION: Returns m_iXPriority string.
//   ARGUMENTS: none
// USES GLOBAL: m_iXPriority
// MODIFIES GL: none 
//     RETURNS: CIMAPXPriority m_pcXMailer
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
CSmptXPriority CIMAP::GetXPriority() const
{
	return m_iXPriority;
}

const char* CIMAP::GetMsgLineText(unsigned int Line) const
{
	if(Line > MsgBody.size())
		throw ECIMAP(ECIMAP::OUT_OF_MSG_RANGE);
	return MsgBody.at(Line).c_str();
}

unsigned int CIMAP::GetMsgLines() const
{
	return MsgBody.size();
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetCharSet
// DESCRIPTION: Allows the character set to be changed from default of US-ASCII. 
//   ARGUMENTS: const char *sCharSet 
// USES GLOBAL: m_sCharSet
// MODIFIES GL: m_sCharSet
//     RETURNS: none
//      AUTHOR: David Johns
// AUTHOR/DATE: DJ 2012-11-03
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetCharSet(const char *sCharSet)
{
    m_sCharSet = sCharSet;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetLocalHostName
// DESCRIPTION: Allows the local host name to be set externally. 
//   ARGUMENTS: const char *sLocalHostName 
// USES GLOBAL: m_sLocalHostName
// MODIFIES GL: m_sLocalHostName
//     RETURNS: none
//      AUTHOR: jerko
// AUTHOR/DATE: J 2011-12-01
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetLocalHostName(const char *sLocalHostName)
{
    m_sLocalHostName = sLocalHostName;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetXPriority
// DESCRIPTION: Setting priority of the message.
//   ARGUMENTS: CIMAPXPriority priority - priority of the message (	XPRIORITY_HIGH,
//              XPRIORITY_NORMAL, XPRIORITY_LOW)
// USES GLOBAL: none
// MODIFIES GL: m_iXPriority 
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetXPriority(CSmptXPriority priority)
{
	m_iXPriority = priority;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetReplyTo
// DESCRIPTION: Setting the return address.
//   ARGUMENTS: const char *ReplyTo - return address
// USES GLOBAL: m_sReplyTo
// MODIFIES GL: m_sReplyTo
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetReplyTo(const char *ReplyTo)
{
	m_sReplyTo = ReplyTo;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetReadReceipt
// DESCRIPTION: Setting whether to request a read receipt.
//   ARGUMENTS: bool requestReceipt - whether or not to request a read receipt
// USES GLOBAL: m_bReadReceipt
// MODIFIES GL: m_bReadReceipt
//     RETURNS: none
//      AUTHOR: David Johns
// AUTHOR/DATE: DRJ 2012-11-03
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetReadReceipt(bool requestReceipt/*=true*/)
{
	m_bReadReceipt = requestReceipt;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetSenderMail
// DESCRIPTION: Setting sender's mail.
//   ARGUMENTS: const char *EMail - sender's e-mail
// USES GLOBAL: m_sMailFrom
// MODIFIES GL: m_sMailFrom
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetSenderMail(const char *EMail)
{
	m_sMailFrom = EMail;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetSenderName
// DESCRIPTION: Setting sender's name.
//   ARGUMENTS: const char *Name - sender's name
// USES GLOBAL: m_sNameFrom
// MODIFIES GL: m_sNameFrom
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetSenderName(const char *Name)
{
	m_sNameFrom = Name;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetSubject
// DESCRIPTION: Setting subject of the message.
//   ARGUMENTS: const char *Subject - subject of the message
// USES GLOBAL: m_sSubject
// MODIFIES GL: m_sSubject
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetSubject(const char *Subject)
{
	m_sSubject = Subject;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetSubject
// DESCRIPTION: Setting the name of program which is sending the mail.
//   ARGUMENTS: const char *XMailer - programe name
// USES GLOBAL: m_sXMailer
// MODIFIES GL: m_sXMailer
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetXMailer(const char *XMailer)
{
	m_sXMailer = XMailer;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetLogin
// DESCRIPTION: Setting the login of IMAP account's owner.
//   ARGUMENTS: const char *Login - login of IMAP account's owner
// USES GLOBAL: m_sLogin
// MODIFIES GL: m_sLogin
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetLogin(const char *Login)
{
	m_sLogin = Login;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetPassword
// DESCRIPTION: Setting the password of IMAP account's owner.
//   ARGUMENTS: const char *Password - password of IMAP account's owner
// USES GLOBAL: m_sPassword
// MODIFIES GL: m_sPassword
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetPassword(const char *Password)
{
	m_sPassword = Password;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: SetIMAPServer
// DESCRIPTION: Setting the IMAP service name and port.
//   ARGUMENTS: const char* SrvName - IMAP service name
//              const unsigned short SrvPort - SMTO service port
// USES GLOBAL: m_sIMAPSrvName
// MODIFIES GL: m_sIMAPSrvName 
//     RETURNS: none
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JO 2010-0708
////////////////////////////////////////////////////////////////////////////////
void CIMAP::SetIMAPServer(const char* SrvName, const unsigned short SrvPort, bool authenticate)
{
	m_iIMAPSrvPort = SrvPort;
	m_sIMAPSrvName = SrvName;
	m_bAuthenticate = authenticate;
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetErrorText (friend function)
// DESCRIPTION: Returns the string for specified error code.
//   ARGUMENTS: CIMAPError ErrorId - error code
// USES GLOBAL: none
// MODIFIES GL: none 
//     RETURNS: error string
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
std::string ECIMAP::GetErrorText() const
{
	switch(ErrorCode)
	{
		case ECIMAP::CIMAP_NO_ERROR:
			return "";
		case ECIMAP::WSA_STARTUP:
			return "Unable to initialise winsock2";
		case ECIMAP::WSA_VER:
			return "Wrong version of the winsock2";
		case ECIMAP::WSA_SEND:
			return "Function send() failed";
		case ECIMAP::WSA_RECV:
			return "Function recv() failed";
		case ECIMAP::WSA_CONNECT:
			return "Function connect failed";
		case ECIMAP::WSA_GETHOSTBY_NAME_ADDR:
			return "Unable to determine remote server";
		case ECIMAP::WSA_INVALID_SOCKET:
			return "Invalid winsock2 socket";
		case ECIMAP::WSA_HOSTNAME:
			return "Function hostname() failed";
		case ECIMAP::WSA_IOCTLSOCKET:
			return "Function ioctlsocket() failed";
		case ECIMAP::BAD_IPV4_ADDR:
			return "Improper IPv4 address";
		case ECIMAP::UNDEF_MSG_HEADER:
			return "Undefined message header";
		case ECIMAP::UNDEF_MAIL_FROM:
			return "Undefined mail sender";
		case ECIMAP::UNDEF_SUBJECT:
			return "Undefined message subject";
		case ECIMAP::UNDEF_RECIPIENTS:
			return "Undefined at least one reciepent";
		case ECIMAP::UNDEF_RECIPIENT_MAIL:
			return "Undefined recipent mail";
		case ECIMAP::UNDEF_LOGIN:
			return "Undefined user login";
		case ECIMAP::UNDEF_PASSWORD:
			return "Undefined user password";
		case ECIMAP::BAD_LOGIN_PASSWORD:
			return "Invalid user login or password";
		case ECIMAP::BAD_DIGEST_RESPONSE:
			return "Server returned a bad digest MD5 response";
		case ECIMAP::BAD_SERVER_NAME:
			return "Unable to determine server name for digest MD5 response";
		case ECIMAP::COMMAND_MAIL_FROM:
			return "Server returned error after sending MAIL FROM";
		case ECIMAP::COMMAND_EHLO:
			return "Server returned error after sending EHLO";
		case ECIMAP::COMMAND_AUTH_PLAIN:
			return "Server returned error after sending AUTH PLAIN";
		case ECIMAP::COMMAND_AUTH_LOGIN:
			return "Server returned error after sending AUTH LOGIN";
		case ECIMAP::COMMAND_AUTH_CRAMMD5:
			return "Server returned error after sending AUTH CRAM-MD5";
		case ECIMAP::COMMAND_AUTH_DIGESTMD5:
			return "Server returned error after sending AUTH DIGEST-MD5";
		case ECIMAP::COMMAND_DIGESTMD5:
			return "Server returned error after sending MD5 DIGEST";
		case ECIMAP::COMMAND_DATA:
			return "Server returned error after sending DATA";
		case ECIMAP::COMMAND_QUIT:
			return "Server returned error after sending QUIT";
		case ECIMAP::COMMAND_RCPT_TO:
			return "Server returned error after sending RCPT TO";
		case ECIMAP::MSG_BODY_ERROR:
			return "Error in message body";
		case ECIMAP::CONNECTION_CLOSED:
			return "Server has closed the connection";
		case ECIMAP::SERVER_NOT_READY:
			return "Server is not ready";
		case ECIMAP::SERVER_NOT_RESPONDING:
			return "Server not responding";
		case ECIMAP::FILE_NOT_EXIST:
			return "File not exist";
		case ECIMAP::MSG_TOO_BIG:
			return "Message is too big";
		case ECIMAP::BAD_LOGIN_PASS:
			return "Bad login or password";
		case ECIMAP::UNDEF_XYZ_RESPONSE:
			return "Undefined xyz IMAP response";
		case ECIMAP::LACK_OF_MEMORY:
			return "Lack of memory";
		case ECIMAP::TIME_ERROR:
			return "time() error";
		case ECIMAP::RECVBUF_IS_EMPTY:
			return "RecvBuf is empty";
		case ECIMAP::SENDBUF_IS_EMPTY:
			return "SendBuf is empty";
		case ECIMAP::OUT_OF_MSG_RANGE:
			return "Specified line number is out of message size";
		case ECIMAP::COMMAND_EHLO_STARTTLS:
			return "Server returned error after sending STARTTLS";
		case ECIMAP::SSL_PROBLEM:
			return "SSL problem";
		case ECIMAP::COMMAND_DATABLOCK:
			return "Failed to send data block";
		case ECIMAP::STARTTLS_NOT_SUPPORTED:
			return "The STARTTLS command is not supported by the server";
		case ECIMAP::LOGIN_NOT_SUPPORTED:
			return "AUTH LOGIN is not supported by the server";
		case ECIMAP::COMMAND_LIST:
			return "List command error";
		case ECIMAP::COMMAND_MSG:
			return "Msg command error";
		case ECIMAP::COMMAND_COPY:
			return "Copy command error";
		case ECIMAP::COMMAND_DEL:
			return "Del command error";
		default:
			return "Undefined error id";
	}
}

void CIMAP::SayHello()
{
	Command_imap* pEntry = FindCommandEntry(command_EHLO);
	sprintf(SendBuf, "EHLO %s\r\n", GetLocalHostName()!=NULL ? m_sLocalHostName.c_str() : "domain");
	SendData(pEntry);
	ReceiveResponse(pEntry);
	m_bConnected=true;
}

void CIMAP::SayQuit()
{
	// ***** CLOSING CONNECTION *****
	
	//Command_imap* pEntry = FindCommandEntry(command_QUIT);
	//// QUIT <CRLF>
	//strcpy(SendBuf, "QUIT\r\n");
	//SendData(pEntry);
	//ReceiveResponse(pEntry);
	m_bConnected=false;
}

void CIMAP::StartTls()
{
	if(IsKeywordSupported(RecvBuf, "STARTTLS") == false)
	{
		throw ECIMAP(ECIMAP::STARTTLS_NOT_SUPPORTED);
	}
	Command_imap* pEntry = FindCommandEntry(command_STARTTLS);
	//strcpy_s(SendBuf, BUFFER_SIZE_IMAP, "STARTTLS\r\n");
	strcpy(SendBuf, "STARTTLS\r\n");
	SendData(pEntry);
	ReceiveResponse(pEntry);

	OpenSSLConnect();
}

void CIMAP::ReceiveData_SSL(SSL* ssl, Command_imap* pEntry)
{
	int res = 0;
	int offset = 0;
	fd_set fdread;
	fd_set fdwrite;
	timeval time;

	int read_blocked_on_write = 0;

	time.tv_sec = pEntry->recv_timeout;
	time.tv_usec = 0;

	assert(RecvBuf);

	if(RecvBuf == NULL)
		throw ECIMAP(ECIMAP::RECVBUF_IS_EMPTY);

	bool bFinish = false;

	while(!bFinish)
	{
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);

		FD_SET(hSocket,&fdread);

		if(read_blocked_on_write)
		{
			FD_SET(hSocket, &fdwrite);
		}

		if((res = select(hSocket+1, &fdread, &fdwrite, NULL, &time)) == SOCKET_ERROR)
		{
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			throw ECIMAP(ECIMAP::WSA_SELECT);
		}

		if(!res)
		{
			//timeout
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			throw ECIMAP(ECIMAP::SERVER_NOT_RESPONDING);
		}

		if(FD_ISSET(hSocket,&fdread) || (read_blocked_on_write && FD_ISSET(hSocket,&fdwrite)) )
		{
			while(1)
			{
				read_blocked_on_write=0;

				const int buff_len = 1024;
				char buff[buff_len];

				res = SSL_read(ssl, buff, buff_len);

				int ssl_err = SSL_get_error(ssl, res);
				if(ssl_err == SSL_ERROR_NONE)
				{
					if(offset + res > BUFFER_SIZE_IMAP - 1)
					{
						FD_ZERO(&fdread);
						FD_ZERO(&fdwrite);
						throw ECIMAP(ECIMAP::LACK_OF_MEMORY);
					}
					memcpy(RecvBuf + offset, buff, res);
					offset += res;
					if(SSL_pending(ssl))
					{
						continue;
					}
					else
					{
						bFinish = true;
						break;
					}
				}
				else if(ssl_err == SSL_ERROR_ZERO_RETURN)
				{
					bFinish = true;
					break;
				}
				else if(ssl_err == SSL_ERROR_WANT_READ)
				{
					break;
				}
				else if(ssl_err == SSL_ERROR_WANT_WRITE)
				{
					/* We get a WANT_WRITE if we're
					trying to rehandshake and we block on
					a write during that rehandshake.

					We need to wait on the socket to be 
					writeable but reinitiate the read
					when it is */
					read_blocked_on_write=1;
					break;
				}
				else
				{
					FD_ZERO(&fdread);
					FD_ZERO(&fdwrite);
					throw ECIMAP(ECIMAP::SSL_PROBLEM);
				}
			}
		}
	}

	FD_ZERO(&fdread);
	FD_ZERO(&fdwrite);
	RecvBuf[offset] = 0;
	if(offset == 0)
	{
		throw ECIMAP(ECIMAP::CONNECTION_CLOSED);
	}
}

void CIMAP::ReceiveResponse(Command_imap* pEntry)
{
	std::string line;
	//int reply_code = 0;
	bool bFinish = false;

	size_t begin = 0;
	size_t offset = 0;

	while(!bFinish)
	{
		ReceiveData(pEntry);
		line.append(RecvBuf);
		size_t len = line.length();
		
		while(1) // loop for all lines
		{
			while(offset + 1 < len)
			{
				if(line[offset] == '\r' && line[offset+1] == '\n')
					break;
				++offset;
			}
			if(offset + 1 < len) // we found a line
			{
				int nCnt=0;
				while(!bFinish)
				{
					if(pEntry->valid_reply_code[nCnt] != 0
						&& pEntry->valid_reply_code[nCnt] != line[begin+nCnt+3])
						break;

					if(nCnt == 2 || pEntry->valid_reply_code[nCnt] == 0)
						bFinish	= true;
					nCnt++;
				}

				if(bFinish)
					break;
				offset+=2;
				begin = offset;	// try to find next line
			}
			else // we haven't received the last line, so we need to receive more data 
			{
				break;
			}
		}
	}
	//strcpy_s(RecvBuf, BUFFER_SIZE_IMAP, line.c_str());
	strcpy(RecvBuf, line.c_str());
	OutputDebugStringA(RecvBuf);
	//if(reply_code != pEntry->valid_reply_code)
	//{
	//	throw ECIMAP(pEntry->error);
	//}
}

void CIMAP::SendData_SSL(SSL* ssl, Command_imap* pEntry)
{
	int offset = 0,res,nLeft = strlen(SendBuf);
	fd_set fdwrite;
	fd_set fdread;
	timeval time;

	int write_blocked_on_read = 0;

	time.tv_sec = pEntry->send_timeout;
	time.tv_usec = 0;

	assert(SendBuf);

	if(SendBuf == NULL)
		throw ECIMAP(ECIMAP::SENDBUF_IS_EMPTY);

	while(nLeft > 0)
	{
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);

		FD_SET(hSocket,&fdwrite);

		if(write_blocked_on_read)
		{
			FD_SET(hSocket, &fdread);
		}

		if((res = select(hSocket+1,&fdread,&fdwrite,NULL,&time)) == SOCKET_ERROR)
		{
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			throw ECIMAP(ECIMAP::WSA_SELECT);
		}

		if(!res)
		{
			//timeout
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			throw ECIMAP(ECIMAP::SERVER_NOT_RESPONDING);
		}

		if(FD_ISSET(hSocket,&fdwrite) || (write_blocked_on_read && FD_ISSET(hSocket, &fdread)) )
		{
			write_blocked_on_read=0;

			/* Try to write */
			res = SSL_write(ssl, SendBuf+offset, nLeft);
	          
			switch(SSL_get_error(ssl,res))
			{
			  /* We wrote something*/
			  case SSL_ERROR_NONE:
				nLeft -= res;
				offset += res;
				break;
	              
				/* We would have blocked */
			  case SSL_ERROR_WANT_WRITE:
				break;

				/* We get a WANT_READ if we're
				   trying to rehandshake and we block on
				   write during the current connection.
	               
				   We need to wait on the socket to be readable
				   but reinitiate our write when it is */
			  case SSL_ERROR_WANT_READ:
				write_blocked_on_read=1;
				break;
	              
				  /* Some other error */
			  default:	      
				FD_ZERO(&fdread);
				FD_ZERO(&fdwrite);
				throw ECIMAP(ECIMAP::SSL_PROBLEM);
			}

		}
	}

	OutputDebugStringA(SendBuf);
	FD_ZERO(&fdwrite);
	FD_ZERO(&fdread);
}

void CIMAP::InitOpenSSL()
{
	//SSL_library_init();
	//SSL_load_error_strings();
	//m_ctx = SSL_CTX_new (SSLv3_client_method());
	//if(m_ctx == NULL)
	//	throw ECIMAP(ECIMAP::SSL_PROBLEM);

	m_ctx	= (SSL_CTX*)BaseEmail::OpenSSLInit();
}

void CIMAP::OpenSSLConnect()
{
	if(m_ctx == NULL)
		throw ECIMAP(ECIMAP::SSL_PROBLEM);
	m_ssl = SSL_new (m_ctx);   
	if(m_ssl == NULL)
		throw ECIMAP(ECIMAP::SSL_PROBLEM);
	SSL_set_fd (m_ssl, (int)hSocket);
    SSL_set_mode(m_ssl, SSL_MODE_AUTO_RETRY);

	int res = 0;
	fd_set fdwrite;
	fd_set fdread;
	int write_blocked = 0;
	int read_blocked = 0;

	timeval time;
	time.tv_sec = TIME_IN_SEC;
	time.tv_usec = 0;

	SSL_set_connect_state(m_ssl);

	while(1)
	{
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);

		if(write_blocked)
			FD_SET(hSocket, &fdwrite);
		if(read_blocked)
			FD_SET(hSocket, &fdread);

		if(write_blocked || read_blocked)
		{
			write_blocked = 0;
			read_blocked = 0;
			if((res = select(hSocket+1,&fdread,&fdwrite,NULL,&time)) == SOCKET_ERROR)
			{
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				throw ECIMAP(ECIMAP::WSA_SELECT);
			}
			if(!res)
			{
				//timeout
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				throw ECIMAP(ECIMAP::SERVER_NOT_RESPONDING);
			}
		}
		res = SSL_connect(m_ssl);
		int nError = SSL_get_error(m_ssl, res);
		switch(nError)
		{
		  case SSL_ERROR_NONE:
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return;
			break;
              
		  case SSL_ERROR_WANT_WRITE:
			write_blocked = 1;
			break;

		  case SSL_ERROR_WANT_READ:
			read_blocked = 1;
			break;
              
		  default:	      
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			throw ECIMAP(ECIMAP::SSL_PROBLEM);
		}
	}
}

void CIMAP::CleanupOpenSSL()
{
	if(m_ssl != NULL)
	{
		SSL_shutdown (m_ssl);  /* send SSL/TLS close_notify */
		SSL_free (m_ssl);
		m_ssl = NULL;
	}
	//if(m_ctx != NULL)
	//{
	//	SSL_CTX_free (m_ctx);	
	//	m_ctx = NULL;
	//	ERR_free_strings();
	//	EVP_cleanup();
	//	CRYPTO_cleanup_all_ex_data();
	//}
}

