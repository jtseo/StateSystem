/*
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
 

#define FROM_ADDR    "<oj@oddeyesoft.com>"
#define TO_ADDR      "<oj@oddeyesoft.com>"
#define CC_ADDR      "<oj@oddeyesoft.com>"
 
#define FROM_MAIL "Sender Person " FROM_ADDR
#define TO_MAIL   "A Receiver " TO_ADDR
#define CC_MAIL   "John CC Smith " CC_ADDR
 
static const char *payload_text =
  "Date: Mon, 23 Nov 2023 22:54:29 +1100\r\n"
  "To: " TO_MAIL "\r\n"
  "From: " FROM_MAIL "\r\n"
  "Cc: " CC_MAIL "\r\n"
  "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a444444d@"
  "rfcpedant.example.org>\r\n"
  "Subject: SMTP example message\r\n"
  "\r\n" // empty line to divide headers from body, see RFC 5322
  "The body of the message starts here.\r\n"
  "\r\n"
  "It could be a lot of lines, could be MIME encoded, whatever.\r\n"
  "Check RFC 5322.\r\n";
 
struct upload_status {
  size_t bytes_read;
};
 
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  size_t room = size * nmemb;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
	return 0;
  }
 
  data = &payload_text[upload_ctx->bytes_read];
 
  if(data) {
	size_t len = strlen(data);
	if(room < len)
	  len = room;
	memcpy(ptr, data, len);
	upload_ctx->bytes_read += len;
 
	return len;
  }
 
  return 0;
}
 
int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx = { 0 };
 
  curl = curl_easy_init();
  if(curl) {
	curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
	//curl_easy_setopt(curl, CURLOPT_URL, "localhost:25");
	curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
 
	
	// Set login credentials for Gmail
	curl_easy_setopt(curl, CURLOPT_USERNAME, "oj@oddeyesoft.com");
	curl_easy_setopt(curl, CURLOPT_PASSWORD, "wzht enlj efcz kamh");//"wkot-cbkq-cvxa-qgnk");

	curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);
 
	recipients = curl_slist_append(recipients, TO_ADDR);
	//recipients = curl_slist_append(recipients, CC_ADDR);
	curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
	curl_easy_setopt(curl, CURLOPT_INFILESIZE, strlen(payload_text));
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
	curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
 
	res = curl_easy_perform(curl);
 
	if(res != CURLE_OK)
	  fprintf(stderr, "curl_easy_perform() failed: %s\n",
			  curl_easy_strerror(res));
 
	curl_slist_free_all(recipients);
 
	curl_easy_cleanup(curl);
  }
 
  return (int)res;
}

/*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <microhttpd.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
// #include "../PtBase/base.h"
// #include "../PtBase/BaseDefines.h"
// //#include "../PtBase/BaseFile.h"
// #include "../PtBase/BaseDStructure.h"
// #include "../PtBase/BaseObject.h"
// #include "../PtBase/BaseState.h"
// #include "../PtBase/BaseStateMain.h"
// #include "../PtBase/BaseStateManager.h"

// #include "../PtBase/BaseResResource.h"
// #include "../PtBase/BaseResManager.h"
// #include "../PtBase/BaseSystem.h"
#include "../PtBase/base.h"
#include "../PtBase/Vector3.h"
#include "../PtBase/Matrix3.h"
#include "../PtBase/Matrix4.h"
#include "../PtBase/BaseObject.h"
//#include "../PtBase/hashstr.h"
#include "../PtBase/BaseDStructure.h"
#include "../PtBase/BaseState.h"

#include "../ApStateSystem/BaseInterface.h"

#include "../PtBase/BaseFile.h"
#include "../PtBase/hashstr.h"
#include "../PtBase/BaseStateManager.h"
#include <math.h>
#include <queue>
#include <unordered_set>

#include "../PtBase/BaseSystem.h"
#include <locale.h>

//*
int main(int argc, const char *argv[])
{
	vscript_init(".", ".", ".", "localhost", 29000);
	BaseStateManager *manager = (BaseStateManager*)manager_create(".");

	//REGISTEOBJECT(BaseResFileDWG::GetObjectId(), BaseResFileDWG);
	//REGISTEOBJECT(PnDrawingTag::GetObjectId(), PnDrawingTag);
	
	int start = BaseSystem::timeGetTime();
	manager->variable_define(STRTOHASH("param1_str"), TYPE_STRING, true);
	manager->variable_define(STRTOHASH("param2_str"), TYPE_STRING, true);
		
	BaseDStructureValue *variable = manager->varialbe_global_get();
	if(argc >= 2)
		variable->set_alloc("param1_str", (const char*)argv[1]);
	if(argc >= 3)
		variable->set_alloc("param2_str", (const char*)argv[2]);
	
	while(1)
	{
		int current = BaseSystem::timeGetTime();
#ifdef DEBUG
		if(current >= start + 10 * 60 * 1000)
		{
			//s_mpool.leak_old_display(3*60*1000);
			start = current;
			//break;
		}
#endif
		//printf(".");
		
		if (s_mpool.is_terminated())
			return 1;

		if (manager->system_terminate_check())
			return 1;

		manager_update(manager);
		BaseSystem::Sleep(50);
	}

	//manager->release();
	//sleep(2);
	return 1;
}
//*/
