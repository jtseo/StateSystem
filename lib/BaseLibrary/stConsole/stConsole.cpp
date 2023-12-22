// TestSet.cpp : Defines the entry point for the console application.
//


#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <list>
#include <queue>
#include <string>
//#include <deque>
#include <unordered_set>

#include <assert.h>
#include <map>

#include "../PtBase/base.h"

#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseSocket.h"

#include "../PtBase/BaseFile.h"
#include "../PtBase/hashstr.h"

#include "../PtBase/BaseDStructure.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseFile.h"

#include "BaseInterface.h"
#include "../PtBase/BaseEmail.h"
#include "../PtBase/BaseStateSpace.h"
#include "../PtBase/BaseStringTable.h"
#include "../PtBase/BaseStateManager.h"
#include <process.h>

#define MAX_ID_LENGTH 100

#undef STDEF_MANAGERNAME
#define STDEF_MANAGERNAME	BaseStateManager

int s_eventstack[10];

#include "../PtBase/Vector3.h"
#include "../PtBase/Matrix3.h"
#include "../PtBase/Matrix4.h"

#include "../PtBase/BaseResResource.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseStructure.h"
#include "../PtBase/BaseTime.h"

#include <conio.h>

#include "../PtBase/BaseXML.h"
#include "../StatePython/StatePython.h"

#include "../PtBase/BaseSCCurl.h"

int main()
{
	BaseSCCurl crl;

	//StatePython *python = (StatePython*)StatePython::CreatorCallback(NULL);

	//vscript_init(".", ".", ".", "192.168.105.50", 80);
	vscript_init(".", ".", ".", "localhost", 19106);
	//vscript_init(".", ".", ".", "forrelease", 19106);
	BaseStateManager* pManager = (BaseStateManager*)manager_create(".");

	//REGISTEOBJECT(BaseResFileDWG::GetObjectId(), BaseResFileDWG);
	//REGISTEOBJECT(PnResDwg2::GetObjectId(), PnResDwg2);

	//BaseConsolFilter::hide("SMain", "VScriptEditor");
	//BaseConsolFilter::hide("SAction", "PopEvent");
	BaseConsolFilter::hide("SAction", "PushEvent");
	//54.64.23.200
	BaseResManager* manager_res_p = (BaseResManager*)pManager->GetResManager();

	//PnResDwg2* dwg2 = (PnResDwg2*)manager_res_p->get_resource("EMP.dwg", PnResDwg2::GetObjectId(), NULL, NULL, 0);

	//BaseResFileDWG* dwg = (BaseResFileDWG*)manager_res_p->get_resource("test4Org.dwg", BaseResFileDWG::GetObjectId(), 0, 0, 0);
	//PnObject *obj_p = dwg->dwg_get().obj_get(10003);
	//obj_p->center();

	int key = 0;
	while (key != 'd')
	{
		manager_update(pManager);
		BaseSystem::Sleep(1);

		if (_kbhit()) {
			key = _getch();

			pManager->state_active_show_all();
		}
	}
	//*/

	manager_close(pManager);
	PT_MemDisplay();
	return 0;
}


void dispaly_column(BaseDStructureValue* _dsv, const STLString _column)
{
	int hash = STRTOHASH(_column.c_str());
	int index = BaseDStructure::get_index(hash);
	int type = BaseDStructure::get_type(index);

	char buff[255];
	int val;
	void* value = (void*)buff;
	bool bret = false;
	if (type == TYPE_INT32)
	{
		value = &val;
		bret = _dsv->get(_column.c_str(), value);
		if (bret)
			printf("%s=%d(int)\n", _column.c_str(), val);
	}
	else {
		value = buff;
		bret = _dsv->get(_column.c_str(), value);
		if (bret)
			printf("%s=%s(string)\n", _column.c_str(), buff);
	}

	if (!bret)
		printf("%s=Fail\n", buff);
}

int* data_p = NULL;
int cnt = 1024 * 10;

STDEF_FUNC(TestclientFunc_nF)
{
	const int* param_pn;
	STDEF_GETLOCAL_R(_pdsvBase, TestclientFunc_nF, param_pn);
	STDEF_Manager(pManager);

	if (*param_pn == 0)
	{
		BaseFile file;
		file.OpenFile("dump.txt");

		STLString str;
		file.Read(&str);
		
		BaseDStructureValue* evt = pManager->make_event_state("PnIDS_TextCast");
		dsv_add_string(evt, "TempString2_strV", str.c_str());

		file.CloseFile();
		pManager->post_event(evt);
	}
	else {
		BaseFile file;
		file.OpenFile("dumpRev.txt", BaseFile::OPEN_WRITE);

		STDEF_BaseState(state_p);
		BaseDStructureValue* variable = state_p->variable_get();
		
		int size = 0;
		const char* data = (const char*)dsv_get_string(variable, "TempString2_strV");
		if (data != NULL)
		{
			size = (int)strlen(data);

			file.Write(data, size);
		}
		file.CloseFile();
	}

	return 1;
}