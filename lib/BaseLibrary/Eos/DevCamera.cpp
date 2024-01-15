#include "stdafx.h"

#include <list>
#include <vector>
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <queue>
#include <set>
#include <unordered_set>
//#include <codecvt>

#include <assert.h>
#include <algorithm>
//#include <objbase.h> 
#include <afxwin.h>

#include "../PtBase/base.h"

#include "../PtBase/Vector3.h"
#include "../PtBase/Matrix3.h"
#include "../PtBase/Matrix4.h"

#include "../PtBase/BaseObject.h"

#include "../PtBase/hashstr.h"
#include "../PtBase/BaseDStructure.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "DevCamera.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#include "CameraControl.h"

PtObjectCpp(DevCamera);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	DevCamera::s_func_hash_a;
const char* DevCamera::s_class_name = "DevCamera";

int DevCamera::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

DevCamera::DevCamera()
{
	m_nObjectId = GetObjectId();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, DevCamera::FunctionProcessor);
}

int DevCamera::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(TakeAPicture_nF);
		STDEF_SFREGIST(PreviewStart_nF);
		STDEF_SFREGIST(PreviewStop_nF);
		STDEF_SFREGIST(EventProcess_nF);
		STDEF_SFREGIST(EventCastEnale_nF);
		STDEF_SFREGIST(TextImageCast_nF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

DevCamera::~DevCamera()
{

}

void DevCamera::init()
{
	BaseStateFunc::init();
}

void DevCamera::release()
{
	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return;

	ActionEvent evt("closing");
	ctr->actionPerformed(evt);

	CCameraControlApp::Instance()->UnInitInstance();
}

BaseStateFuncEx* DevCamera::CreatorCallback(const void* _param)
{
	DevCamera* bs_func;
	PT_OAlloc(bs_func, DevCamera);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, DevCamera::FunctionProcessor);

	return bs_func;
}

int DevCamera::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(TakeAPicture_nF);
		STDEF_SFFUNCALL(PreviewStart_nF);
		STDEF_SFFUNCALL(PreviewStop_nF);
		STDEF_SFFUNCALL(EventProcess_nF);
		STDEF_SFFUNCALL(EventCastEnale_nF);
		STDEF_SFFUNCALL(TextImageCast_nF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int DevCamera::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	DevCamera* bs_func = (DevCamera*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, DevCamera::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int DevCamera::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);

	CameraController* camCtr = getCameraController();
	if(camCtr)
		camCtr->run();

    return 1;
}

int DevCamera::TakeAPicture_nF()
{
	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return 0;

	ActionEvent evt("TakePicture");
	ctr->actionPerformed(evt);

	return 1;
}

int DevCamera::PreviewStart_nF()
{
	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return 0;

	ActionEvent evt("startEVF");
	ctr->actionPerformed(evt);
	return 1;
}

int DevCamera::PreviewStop_nF()
{
	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return 0;

	ActionEvent evt("endEVF");
	ctr->actionPerformed(evt);
	return 1;
}

#include "../PtBase/BaseFile.h"

int DevCamera::EventProcess_nF()
{
	//PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	AfxPumpMessage();

	return 1;
}

int DevCamera::EventCastEnale_nF()
{
	CCameraControlApp::Instance()->EventCastEnable();
	return 1;
}

int DevCamera::TextImageCast_nF()
{
	if (m_refAlloc)
		mpool_get().free_mem(m_refAlloc);

	BaseFile file;
	if (file.OpenFile("C:\\projects\\photobooth\\prevew.jpg", BaseFile::OPEN_READ))
		return 0;
	int size = file.get_size_file();
	m_refAlloc = mpool_get().get_alloc(size);
	file.Read((void*)m_refAlloc, size);
	file.CloseFile();

	int w = 960, h = 640;

	BaseStateManager* manager = BaseStateManager::get_manager();
	BaseDStructureValue* evt = manager->make_event_state(STRTOHASH("CamRevPreview"));
	evt->set_alloc("MemRef_nV", &m_refAlloc);
	evt->set_alloc("ImageHeight_nV", &h);
	evt->set_alloc("ImageWidth_nV", &w);

	manager->post_event(evt);

	return 1;
}
//#SF_functionInsert
