#include "stdafx.h"

#ifndef VScriptBuild

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

#include <opencv2/opencv.hpp>

using namespace cv;

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
		STDEF_SFREGIST(PreviewRequest_nF);
		STDEF_SFREGIST(StreamFree_varF);
		STDEF_SFREGIST(PreviewLayoutPos_nF);
		STDEF_SFREGIST(FilterSkinOn_nF);
		STDEF_SFREGIST(AFocusOn_nF);
		STDEF_SFREGIST(AFocusOff_nF);
		STDEF_SFREGIST(PictureSizeSet_varF);
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

	CCameraControl::Instance()->UnInitInstance();
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
		STDEF_SFFUNCALL(PreviewRequest_nF);
		STDEF_SFFUNCALL(StreamFree_varF);
		STDEF_SFFUNCALL(PreviewLayoutPos_nF);
		STDEF_SFFUNCALL(FilterSkinOn_nF);
		STDEF_SFFUNCALL(AFocusOn_nF);
		STDEF_SFFUNCALL(AFocusOff_nF);
		STDEF_SFFUNCALL(PictureSizeSet_varF);
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

	CCameraControl::Reset();
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


void _log2(const char* strFormat, ...)
{
	char logbuf[1024];
	va_list	argptr;
	va_start(argptr, strFormat);

	vsprintf_s(logbuf, 1024, strFormat, argptr);
	va_end(argptr);

	FILE* pf = NULL;
	if (!fopen_s(&pf, "log.txt", "a+"))
	{
		fputs(logbuf, pf);
		fclose(pf);
	}
}


int DevCamera::PreviewStart_nF()
{
	CameraController* ctr = getCameraController();
	const char* filepath = (const char*)paramFallowGet(0);
	const int* positions = (const int*)paramFallowGet(1);
	const int* count = (const int*)paramFallowGet(2);
	const float* scale = (const float*)paramFallowGet(3);
	const int* blur_p = (const int*)paramFallowGet(4);
	const int* fps_p = (const int*)paramFallowGet(5);
	const float* prevScale_p = (const float*)paramFallowGet(6);
	
	if (ctr == NULL)
		return 0;

	ActionEvent evt("startEVF");
	ctr->actionPerformed(evt);

	float videoScalef = 1;
	if (scale)
		videoScalef = *scale;
	float prevScalef = 1;
	if (prevScale_p)
		prevScalef = *prevScale_p;

	int blur = 3;
	if (blur_p)
		blur = *blur_p;

	float fps = 5;
	if (fps_p)
		fps = *fps_p;

	STLVVec2 pos_av2;
	for (int i = 0; i < *count; i++)
		pos_av2.push_back(PtVector2(*(positions + i * 2)*prevScalef, *(positions + i * 2 + 1) * prevScalef));

	CCameraControl::Instance()->PreviewLayoutSet(filepath, pos_av2, videoScalef, blur, fps, prevScalef);
	m_stop_thread = false;
	BaseSystem::createthread(update_, 0, this);
	
	return 1;
}

DEF_ThreadCallBack(DevCamera::update)
//void __cdecl DevCamera::update(void *_pCam)
{
	PT_ThreadStart(THTYPE_CHARGE_RECEIVE);
	DevCamera* pCam = (DevCamera*)_pParam;

	mpool_get().hold_shutdown_inc();
	do {
		BaseSystem::Sleep(40);
		//AfxPumpMessage();
		MSG msg;
		PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	} while (!pCam->stop_thread() && !mpool_get().is_terminated());

	PT_ThreadEnd(THTYPE_CHARGE_RECEIVE);

	mpool_get().hold_shutdown_dec();
	BaseSystem::endthread();
	//_endthread();
	DEF_ThreadReturn;
}

int DevCamera::PreviewStop_nF()
{
	m_stop_thread = true;
	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return 0;

	ActionEvent evt("endEVF");
	ctr->actionPerformed(evt);
	return 1;
}

bool DevCamera::stop_thread()
{
	return m_stop_thread;
}

#include "../PtBase/BaseFile.h"

int DevCamera::EventProcess_nF()
{
	//PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	//AfxPumpMessage();

	return 1;
}

int DevCamera::EventCastEnale_nF()
{
	CCameraControl::Instance()->EventCastEnable();
	return 1;
}

int DevCamera::TextImageCast_nF()
{
	BaseFile file;
	if (file.OpenFile("C:\\projects\\photobooth\\prevew.jpg", BaseFile::OPEN_READ))
		return 0;
	int size = file.get_size_file();
	//INT64 refAlloc = mpool_get().get_alloc(size);
	//file.Read((void*)refAlloc, size);
	file.CloseFile();

	int w = 960, h = 640;

	//BaseStateManager* manager = BaseStateManager::get_manager();
	//BaseDStructureValue* evt = manager->make_event_state(STRTOHASH("CamRevPreview"));
	//evt->set_alloc("MemRef_nV", &refAlloc);
	//evt->set_alloc("ImageHeight_nV", &h);
	//evt->set_alloc("ImageWidth_nV", &w);

	//manager->post_event(evt);

	return 1;
}
int DevCamera::PreviewRequest_nF()
{
	CCameraControl::Instance()->PreviewRequest();
	return 1;
}

int DevCamera::StreamFree_varF()
{
	CCameraControl::Instance()->StreamFree();
	return 1;
}

int DevCamera::PreviewLayoutPos_nF()
{
	const int *slot = (const int*)paramVariableGet();

	CCameraControl::Instance()->PreviewSlotSet(*slot);
	return 1;
}

int DevCamera::FilterSkinOn_nF()
{
	const int* mode = (const int*)m_param_value;

	CCameraControl::Instance()->FilterSkinOn_nF(*mode);
	return 1;
}

int DevCamera::AFocusOn_nF()
{
	CCameraControl::Instance()->AFocusOn_nF();
	return 1;
}

int DevCamera::AFocusOff_nF()
{
	CCameraControl::Instance()->AFocusOff_nF();
	return 1;
}

int DevCamera::PictureSizeSet_varF()
{
	const int* size_an = (const int*)paramVariableGet();

	CCameraControl::Instance()->PictureSizeSet(size_an[0], size_an[1]);
	return 1;
}
//#SF_functionInsert
#endif
