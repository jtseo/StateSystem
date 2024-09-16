#include "stdafx.h"

#ifdef WIN32
#include <winsock.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "curl/curl.h"

//#include <sys/ioctl.h>
#include <inttypes.h>

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseSystem.h"

#include "BaseSCCurl.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"
#include <iostream>

PtObjectCpp(BaseSCCurl);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCCurl::s_func_hash_a;
const char* BaseSCCurl::s_class_name = "BaseSCCurl";

int BaseSCCurl::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCCurl::BaseSCCurl()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCCurl::FunctionProcessor);
}

int BaseSCCurl::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(get_strF);
		STDEF_SFREGIST(post_strF);
		STDEF_SFREGIST(postEncode_varF);
		STDEF_SFREGIST(EmailSend_varF);
		STDEF_SFREGIST(GoogleNotification_formatF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCCurl::~BaseSCCurl()
{

}

void BaseSCCurl::init()
{
	BaseStateFunc::init();
}

void BaseSCCurl::release()
{
}

BaseStateFuncEx* BaseSCCurl::CreatorCallback(const void* _param)
{
	BaseSCCurl* bs_func;
	PT_OAlloc(bs_func, BaseSCCurl);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCCurl::FunctionProcessor);

	return bs_func;
}

int BaseSCCurl::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
        //#SF_FuncCallStart
        if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
        //STDEF_SFFUNCALL(Open_varF);
        STDEF_SFFUNCALL(get_strF);
        STDEF_SFFUNCALL(post_strF);
		STDEF_SFFUNCALL(postEncode_varF);
		STDEF_SFFUNCALL(EmailSend_varF);
		STDEF_SFFUNCALL(GoogleNotification_formatF);
		//#SF_FuncCallInsert
        return 0;
    }
    return ret;
}

int BaseSCCurl::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCCurl* bs_func = (BaseSCCurl*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCCurl::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCCurl::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

    //if (!m_manager_p->variable_define(STRTOHASH("PnIDS_ParamPoint"), TYPE_INT64))
    //    return 0;

	return 1;
}

#ifdef _WIN32
HINSTANCE hInst;
WSADATA wsaData;
#else

#endif
void mParseUrl(char* mUrl, STLString& serverName, STLString& filepath, STLString& filename);

#ifdef _WIN32
SOCKET connectToServer(char* szServerName, WORD portNum);
#else
int connectToServer(char* szServerName, short portNum);
#endif
int getHeaderLength(char* content);
bool readUrl2(char* szUrl, STLString *_data, STLString *_header);

void mParseUrl(char* mUrl, STLString& serverName, STLString& filepath, STLString& filename)
{
    STLString::size_type n;
    STLString url = mUrl;

    if (url.substr(0, 7) == "http://")
        url.erase(0, 7);

    if (url.substr(0, 8) == "https://")
        url.erase(0, 8);

    n = url.find('/');
    if (n != STLString::npos)
    {
        serverName = url.substr(0, n);
        filepath = url.substr(n);
        n = filepath.rfind('/');
        filename = filepath.substr(n + 1);
    }

    else
    {
        serverName = url;
        filepath = "/";
        filename = "";
    }
}

#ifdef _WIN32
SOCKET connectToServer(char* szServerName, WORD portNum)
#else
int connectToServer(char* szServerName, short portNum)
#endif
{
    struct hostent* hp;
    unsigned int addr;
    struct sockaddr_in server;
#ifdef _WIN32
    SOCKET conn;
#else
	int conn;
#endif
    conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _WIN32
	if (conn == INVALID_SOCKET)
        return NULL;
#else
	if (conn < 0)
		return conn;
#endif
	
	if (inet_addr(szServerName) == INADDR_NONE)
    {
        hp = gethostbyname(szServerName);
    }
    else
    {
        addr = inet_addr(szServerName);
        hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
    }

    if (hp == NULL)
    {
#ifdef _WIN32
        closesocket(conn);
        return NULL;
#else
		close(conn);
		return -1;
#endif
    }

#ifdef _WIN32
    server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
#else
	char strBuffer[255];
	unsigned add[4];
	for(int i=0; i<4; i++)
		add[i] = (unsigned char)hp->h_addr_list[0][i];
	sprintf_s(strBuffer, 255, "%u.%u.%u.%u"
			  , add[0], add[1], add[2], add[3]);
	inet_aton(strBuffer, &server.sin_addr);
#endif
    server.sin_family = AF_INET;
    server.sin_port = htons(portNum);
    if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
    {
#ifdef _WIN32
		closesocket(conn);
		return NULL;
#else
		close(conn);
		return -1;
#endif
    }
    return conn;
}

int getHeaderLength(char* content)
{
    const char* srchStr1 = "\r\n\r\n", * srchStr2 = "\n\r\n\r";
    char* findPos;
    int ofset = -1;

    findPos = strstr(content, srchStr1);
    if (findPos != NULL)
    {
        ofset = (int)(findPos - content);
        ofset += (int)strlen(srchStr1);
    }

    else
    {
        findPos = strstr(content, srchStr2);
        if (findPos != NULL)
        {
            ofset = (int)(findPos - content);
            ofset += (int)strlen(srchStr2);
        }
    }
    return ofset;
}

bool readUrl2(char* szUrl, STLString* _data, STLString* _header)
{
    const int bufSize = 512;
    char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
    char* tmpResult = NULL;
#ifdef _WIN32
    SOCKET conn;
#else
	int conn;
#endif
    STLString server, filepath, filename;
    long totalBytesRead, thisReadSize, headerLen;

    mParseUrl(szUrl, server, filepath, filename);

    ///////////// step 1, connect //////////////////////
    conn = connectToServer((char*)server.c_str(), 80);

    ///////////// step 2, send GET request /////////////
    sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
    strcpy(sendBuffer, tmpBuffer);
    strcat(sendBuffer, "\r\n");
    sprintf(tmpBuffer, "Host: %s", server.c_str());
    strcat(sendBuffer, tmpBuffer);
    strcat(sendBuffer, "\r\n");
    strcat(sendBuffer, "\r\n");
    send(conn, sendBuffer, (int)strlen(sendBuffer), 0);

    //    SetWindowText(edit3Hwnd, sendBuffer);
    printf("Buffer being sent:\n%s", sendBuffer);

    ///////////// step 3 - get received bytes ////////////////
    // Receive until the peer closes the connection
    totalBytesRead = 0;
    while (1)
    {
        memset(readBuffer, 0, bufSize);
        thisReadSize = recv(conn, readBuffer, bufSize, 0);

        if (thisReadSize <= 0)
            break;

        tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead+1);

        memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
        totalBytesRead += thisReadSize;
    }
    tmpResult[totalBytesRead] = NULL;
    *_data = tmpResult;

    headerLen = getHeaderLength(tmpResult);
    tmpResult[headerLen] = NULL;
    *_header = tmpResult;
    delete(tmpResult);

#ifdef _WIN32
    closesocket(conn);
#else
	close(conn);
#endif
    return true;
}

int BaseSCCurl::get_strF()
{
    static bool thread_start = false;

    if (thread_start)
        return 0;
    thread_start = true;
    BaseSystem::createthread(update_, 0, this);

    const char* api = (const char*)m_param_value;
	return 1;
}


DEF_ThreadCallBack(BaseSCCurl::update)
//void __cdecl BaseNetManager::update(void *_pManager)
{
    PT_ThreadStart(THTYPE_BASE_TCP);
    BaseSCCurl* scCurl = (BaseSCCurl*)_pParam;

    const int bufLen = 1024;
	STLString url = "http://stackoverflow.com";

    bool end = false;
#ifdef _WIN32
    if (WSAStartup(0x101, &wsaData) != 0)
        end = true;
#endif
	
    STLString data, header;
    if (!end)
    {
        readUrl2((char*)url.c_str(), &data, &header);
        printf("returned from readUrl\n");
        printf("data returned:\n%s", data.c_str());
        if (!data.empty())
        {   
            scCurl->event_post(data, header);
        }
#ifdef _WIN32
        WSACleanup();
#endif
    }
#ifndef _WIN32
	return NULL;
#endif
}

void BaseSCCurl::event_post(STLString& _data, STLString &_header)
{
    BaseDStructureValue* evt = EventMakeThread(STRTOHASH("Curl_return"));
    m_data = _data;
    m_header = _header;
    EventPostThread(evt);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}


bool BaseSCCurl::curlPost(const char* _data, const char *_url, const char *_autho)
{
	CURL* curl;
	CURLcode res;

	const char* jsonData = _data;

	FILE* pf = NULL;
	pf = fopen("rec_packet.txt", "a+");
	if (pf)
	{
		fputs(jsonData, pf);
		fputs("\r\n", pf);
		fclose(pf);
	}
	// Initialize libcurl
	curl = curl_easy_init();

	if (curl) {
		// Set the URL for the POST request
		//curl_easy_setopt(curl, CURLOPT_URL, "https://wgi02.archipindev.com/auth/all/");
		curl_easy_setopt(curl, CURLOPT_URL, _url);

		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		// Specify the POST data
		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		STLString authorHeader = "Authorization: Bearer ";
		authorHeader += _autho;
		headers = curl_slist_append(headers, authorHeader.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		//const char* jsonData = "{\"userId\" : \"653b5f94404e3e6dafb73846\"}";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		std::cout << "request json: " << jsonData << std::endl;
		std::string response;

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		// Perform the POST request
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		else {
			// Response received successfully
			std::cout << "Response received:\n" << response << std::endl;

			BaseDStructureValue* evt = EventMake(STRTOHASH("Curl_return"));

			int nSerial = m_state_p->obj_serial_get();
			evt->set_alloc(HASH_STATE(BaseEventTargetStateSerial), &nSerial);
			evt->set_alloc("SRBlock_strV", response.c_str());
			EventPost(evt);
		}

		// Cleanup
		curl_easy_cleanup(curl);

		// Cleanup headers
		curl_slist_free_all(headers);
	}
	if (res != CURLE_OK)
		return false;

	return true;
}


int BaseSCCurl::post_strF()
{
	CURL* curl;
	CURLcode res;

	const char* jsonData = NULL;
	const int* hash = (const int*)m_param_value;
	if (!m_state_variable->get_mass(*hash, (const void**)&jsonData))
		return 0;

	char* data;
	int len = strlen(jsonData)+1;
	data = PT_Alloc(char, len);
	memcpy(data, jsonData, len);
	const char* url = (const char*)paramFallowGet(0);
	if (url == NULL)
		return 0;
	const char* autho = (const char*)paramFallowGet(1);

	bool ret = curlPost(data, url, autho);
	PT_Free(data);
	if(!ret)
		return 0;
	return 1;
}

#include "base64.h"

int BaseSCCurl::postEncode_varF()
{
	const char* rowData = NULL;
	const int* hash = (const int*)m_param_value;
	int nSize = 0;
	if (!m_state_variable->get_mass(*hash, (const void**)&rowData, &nSize))
		return 0;

	std::string encoded = base64_encode((const unsigned char*)rowData, nSize);
	const char* url = (const char*)paramFallowGet(0);
	if (url == NULL)
		return 0;
	const char* autho = (const char*)paramFallowGet(1);

	if (!curlPost(encoded.c_str(), url, autho))
		return 0;
	return 1;
}

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	STLVString *uploads = (STLVString*)userp;

	if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	if(uploads->empty()){
		return 0;
	}
	size_t len = uploads->at(0).size();
	memcpy(ptr, uploads->at(0).c_str(), len);
	uploads->erase(uploads->begin());
		
	return len;
}

int BaseSCCurl::GoogleNotification_formatF()
{
	STLString cmd = paramFormatGet();

	STLVString list;
	BaseFile::paser_list_seperate(cmd.c_str(), &list, "|");

	STLString url = list[0];
	STLString msg = list[1];

	CURL* curl;
	CURLcode res;
	std::string readBuffer;

	// The webhook URL of your Google Chat space
	std::string webhookUrl = url.c_str();

	// JSON payload for the message you want to send
	std::string jsonData = "{\"text\": \"";
	jsonData += msg.c_str();
	jsonData += "\"}";

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl) {
		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, webhookUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); 
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Disable SSL peer verification
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // Disable SSL host verification


		// Perform the HTTP POST request
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		else {
			// Success, print the response
			std::cout << readBuffer << std::endl;
		}

		// Cleanup
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return 1;
}

int BaseSCCurl::EmailSend_varF()
{
	const char *email = (const char*)paramVariableGet();
	// email, server, id, password, mail
	const char *server = (const char*)paramFallowGet(0);
	const char *id = (const char*)paramFallowGet(1);
	const char *password = (const char*)paramFallowGet(2);
	const char *title = (const char*)paramFallowGet(3);
	const char *mail = (const char*)paramFallowGet(4);
    const char *fromName = (const char *)paramFallowGet(5);
	
	if(!email || !server || !id || !password || !title || !mail)
		return 0;
	
	BaseFile file;
	int serial = 10000;
	if(!file.OpenFile("serial.txt", BaseFile::OPEN_READ))
	{
		file.read_asc_line();
		file.read_asc_integer(&serial);
		file.CloseFile();
		serial++;
	}
	file.OpenFile("serial.txt", BaseFile::OPEN_WRITE);
	file.write_asc_integer(serial);
	file.write_asc_line();
	file.CloseFile();
	
	SPtDateTime time;
	BaseSystem::timeCurrent(&time);
	char buf[100];
	sprintf_s(buf, 100, "%d", serial);
	serial = STRTOHASH(buf);
	sprintf_s(buf, 100, "%d-%d%d", serial, time.YEAR, time.DAY);
	STLString strSerial = buf;
	
	STLVString _buffer;
	STLVString *contents = &_buffer;
	STLString str;
	str = "Date: "; str += BaseTime::make_date(time, buf, 100); str += "\r\n";	contents->push_back(str);
	str = "To: "; str += email; str += "\r\n";			contents->push_back(str);
    if(!fromName){
        str = "From: "; str += id; str += "\r\n";			
        contents->push_back(str);
    }else{
        str = "From: ";
        str += fromName;
        str += " <";
        str += id;
        str += ">\r\n";
        contents->push_back(str);
    }
	str = "Message-ID: <"; str+= strSerial.c_str(); str+="@oddeyesoft.com>\r\n";
			contents->push_back(str);
	str = "Subject: "; str+= title; str+= "\r\n";		contents->push_back(str);
	str = "\r\n"; /* empty line to divide headers from body, see RFC5322 */
			contents->push_back(str);
	str = mail;	contents->push_back(str);
	
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;

	curl = curl_easy_init();
	if(curl) {
		// Set SMTP server and port
		curl_easy_setopt(curl, CURLOPT_URL, server);
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  // 30 seconds timeout
        
		// Set login credentials for Gmail
		curl_easy_setopt(curl, CURLOPT_USERNAME, id);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

		// Sender address
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, id);

		// Recipient address
		recipients = curl_slist_append(recipients, email);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		// Message body
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, contents);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mail);
		
		// Perform the send
		res = curl_easy_perform(curl);


		
		// Check for errors
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			// Free the list of recipients
			curl_slist_free_all(recipients);
			// Cleanup
			curl_easy_cleanup(curl);
			return 0;
		}
		// Free the list of recipients
		curl_slist_free_all(recipients);
		// Cleanup
		curl_easy_cleanup(curl);

	}

	
	return 1;
}
//#SF_functionInsert
