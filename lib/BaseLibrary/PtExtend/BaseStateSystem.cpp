#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseStateSystem.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#ifdef WIN32
#include <Windows.h>
#endif

PtObjectCpp(BaseStateSystem);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseStateSystem::s_func_hash_a;
const char* BaseStateSystem::s_class_name = "BaseStateSystem";

int BaseStateSystem::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseStateSystem::BaseStateSystem()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseStateSystem::FunctionProcessor);
}

int BaseStateSystem::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(AppClose_strF);
		STDEF_SFREGIST(MakeFront_nF);
		STDEF_SFREGIST(ListFilter_strF);
		STDEF_SFREGIST(KeyboardNumOn_nF);
		STDEF_SFREGIST(GroupIdFromEventAdd_nF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseStateSystem::~BaseStateSystem()
{

}

void BaseStateSystem::init()
{
	BaseStateFunc::init();
}

void BaseStateSystem::release()
{
}

BaseStateFuncEx* BaseStateSystem::CreatorCallback(const void* _param)
{
	BaseStateSystem* bs_func;
	PT_OAlloc(bs_func, BaseStateSystem);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseStateSystem::FunctionProcessor);

	return bs_func;
}

int BaseStateSystem::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(AppClose_strF);
		STDEF_SFFUNCALL(MakeFront_nF);
		STDEF_SFFUNCALL(ListFilter_strF);
		STDEF_SFFUNCALL(KeyboardNumOn_nF);
		STDEF_SFFUNCALL(GroupIdFromEventAdd_nF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int BaseStateSystem::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseStateSystem* bs_func = (BaseStateSystem*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseStateSystem::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseStateSystem::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

STLString s_targetApp;

#ifdef WIN32
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	const DWORD titleSize = 1024;
	TCHAR windowTitle[titleSize];

	// Get the window title.
	if (GetWindowText(hwnd, windowTitle, titleSize) > 0) {
		// Check if the window title matches the target application's title.
		if(strstr(windowTitle, s_targetApp.c_str()) != 0) {
			// Found the window. Send WM_CLOSE message.
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
	}

	return TRUE; // Continue enumerating windows.
}
#endif

int BaseStateSystem::GroupIdFromEventAdd_nF()
{
	const int *pnGroup = (const int*)m_param_value;
	
	STLMnInt groupIds;
	BaseState::group_id_get(m_param_event, HASH_STATE(BaseTransitionNextIdentifier), groupIds);
	
	if(groupIds.find(*pnGroup) == groupIds.end())
		return 0;
	
	int id = groupIds[*pnGroup];
	m_state_p->group_id_add(*pnGroup, id);
	
	return 1;
}

int BaseStateSystem::KeyboardNumOn_nF()
{
	const int* on = (const int *)m_param_value;

#ifdef WIN32
	bool curOn = (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;

	if (curOn && *on == 1)
		return 0;

	if (!curOn && *on == 0)
		return 0;

	// Simuate a key press
	keybd_event(VK_NUMLOCK, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
	// Simulate a key release
	keybd_event(VK_NUMLOCK, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
#endif
	return 1;
}

int BaseStateSystem::ListFilter_strF()
{
	const char *filter = (const char*)m_param_value;
	const char *files_str = (const char*)paramFallowGet(0);
	const char *dates_str = (const char*)paramFallowGet(1);
	
	STLVString list;
	BaseFile::paser_list_seperate(files_str, &list, ",");
	STLVString dates;
	if(dates_str)
		BaseFile::paser_list_seperate(dates_str, &dates, ",");
	
	typedef enum{
		editor,
		distributor
		//states, // *.ini !Link.ini
		//classes // *.define
	} ScriptType;
	
	ScriptType type = distributor;
	if(strcmp(filter, "editor") == 0)
		type = editor;
	
	for(int i=0; i<list.size(); i++)
	{
		bool remove = false;
		switch(type)
		{
			case editor:
				break;
			case distributor:
				if(strstr(list[i].c_str(), ".ini") != NULL)
					remove = true;
				if(strstr(list[i].c_str(), "Pos.data") != NULL)
					remove = true;
				break;
		}
		if(remove)
		{
			list.erase(list.begin()+i);

			if (dates_str)
				dates.erase(dates.begin()+i);
			i--;
		}
	}
	
	char ret[4096];
	BaseFile::paser_list_merge(ret, 4096, list, ",");
	if(!paramFallowSet(0, ret))
		return 0;
	BaseFile::paser_list_merge(ret, 4096, dates, ",");
	if(!paramFallowSet(1, ret))
		return 0;
	
	return 1;
}

int BaseStateSystem::MakeFront_nF()
{
	
#ifdef WIN32
	HWND hCur = FindWindow(NULL, "photobooth");

	// Get the thread ID of the foreground window
	DWORD foregroundThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	// Get the thread ID of the target window
	DWORD targetThreadId = GetWindowThreadProcessId(hCur, NULL);

	// Attach input to the target window thread
	AttachThreadInput(targetThreadId, foregroundThreadId, TRUE);

	// Bring the window to the foreground
	SetForegroundWindow(hCur);
	SetFocus(hCur);
	SetActiveWindow(hCur);

	// Detach input from the target window thread
	AttachThreadInput(targetThreadId, foregroundThreadId, FALSE);
#endif
	return 1;
}

int BaseStateSystem::AppClose_strF()
{
	const char* name = (const char*)m_param_value;
	s_targetApp = name;

#ifdef WIN32
	EnumWindows(EnumWindowsProc, 0);
#endif
	return 1;
}
//#SF_functionInsert
