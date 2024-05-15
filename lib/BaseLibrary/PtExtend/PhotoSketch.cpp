#include "pch.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "PhotoSketch.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"
#include "../PhotoSketch/SketchLib.h"

#include <opencv2/opencv.hpp>
using namespace cv;

PtObjectCpp(PhotoSketch);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	PhotoSketch::s_func_hash_a;
const char* PhotoSketch::s_class_name = "PhotoSketch";

int PhotoSketch::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

PhotoSketch::PhotoSketch()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, PhotoSketch::FunctionProcessor);
}

int PhotoSketch::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(SketchStart_varF);
		//#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

PhotoSketch::~PhotoSketch()
{

}

void PhotoSketch::init()
{
	BaseStateFunc::init();
}

void PhotoSketch::release()
{
}

BaseStateFuncEx* PhotoSketch::CreatorCallback(const void* _param)
{
	PhotoSketch* bs_func;
	PT_OAlloc(bs_func, PhotoSketch);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, PhotoSketch::FunctionProcessor);

	return bs_func;
}

int PhotoSketch::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(SketchStart_varF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int PhotoSketch::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	PhotoSketch* bs_func = (PhotoSketch*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, PhotoSketch::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int PhotoSketch::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

bool PhotoSketchCallback(int _step, HBITMAP _map, void* _param_p)
{
	//STLString jpgData;
	std::vector<uchar> jpgData;
	PhotoSketch* sketch = (PhotoSketch*)_param_p;

	while (BaseCircleQueue::stream_get()->size_data() > 1)
	{
		void *point = BaseCircleQueue::stream_get()->pop();
		BaseCircleQueue::streamSize_get()->pop();
		PT_Free(point);
	}

	BITMAP bmp;
	GetObject(_map, sizeof(BITMAP), &bmp);

	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmp.bmWidth;
	bi.biHeight = -bmp.bmHeight; // Negative indicates top-down bitmap
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	cv::Mat mat(bmp.bmHeight, bmp.bmWidth, CV_8UC4); // 4 channels for BGRA
	HDC hdc = CreateCompatibleDC(NULL);
	SelectObject(hdc, _map);
	GetDIBits(hdc, _map, 0, bmp.bmHeight, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	DeleteDC(hdc);

	std::string filename = "../PhotoPicture/sketch";
	char buf2[255];
	sprintf_s(buf2, "%d", sketch->threadIdx());
	filename += buf2;
	filename += "/result";
	sprintf_s(buf2, "%d", sketch->stepCount());
	filename += buf2;
	filename += ".jpg";

	cv::imwrite(filename, mat);

	jpgData.resize(bmp.bmHeight * bmp.bmWidth * 4);
	cv::imencode(".jpg", mat, jpgData);
	int imgS = (int)jpgData.size();

	BaseStateManager* manager = BaseStateManager::get_manager();
	BaseDStructureValue* evt = manager->make_event_state(STRTOHASH("SketchStepImage"));

	unsigned char* pbyteImage = NULL;
	pbyteImage = &jpgData[0];
	char* buf = PT_Alloc(char, imgS);
	memcpy(buf, pbyteImage, imgS);

	BaseCircleQueue::stream_get()->push(buf);
	BaseCircleQueue::streamSize_get()->push((void*)imgS);
	//BaseFile file;
	//file.OpenFile("test.jpg", BaseFile::OPEN_WRITE);
	//file.Write(buf, imgS);
	//file.CloseFile();
	INT64 ref = (INT64)buf;
	evt->set_alloc("MemRef_nV", &ref);
	evt->set_alloc("ImageSize_nV", &imgS);
	evt->set_alloc("ImageHeight_nV", &bmp.bmHeight);
	evt->set_alloc("ImageWidth_nV", &bmp.bmWidth);

	manager->post_event(evt);
	
	return true;
}

int PhotoSketch::stepCount()
{
	m_stepCounter++;
	return m_stepCounter;
}

DEF_ThreadCallBack(PhotoSketch::update)
//void __cdecl PhotoSketch::update(void *_pCam)
{
	PT_ThreadStart(THTYPE_CHARGE_RECEIVE);
	PhotoSketch* pSketch = (PhotoSketch*)_pParam;

	mpool_get().hold_shutdown_inc();
	//do {
		CreateAndSaveImage(pSketch->PathFrameGet(), pSketch->PathPictureGet(), pSketch->PictureSize()[0], pSketch->PictureSize()[1], PhotoSketchCallback, pSketch);

		BaseStateManager* manager = BaseStateManager::get_manager();
		BaseDStructureValue* evt = manager->make_event_state(STRTOHASH("SketchStepFinish"));
		int count = pSketch->stepCount();
		count--;
		evt->set_alloc("TempCount_nV", &count);
		manager->post_event(evt);
	//} while (!pSketch->stop_thread() && !mpool_get().is_terminated());

	PT_ThreadEnd(THTYPE_CHARGE_RECEIVE);

	mpool_get().hold_shutdown_dec();
	BaseSystem::endthread();
	//_endthread();
	DEF_ThreadReturn;
}

const char* PhotoSketch::PathFrameGet()
{
	return m_pathFrame.c_str();
}

const char* PhotoSketch::PathPictureGet()
{
	return m_pathPicture.c_str();
}

int* PhotoSketch::PictureSize()
{
	return m_pictureSize;
}

bool PhotoSketch::stop_thread()
{
	return m_stop_thread;
}

int PhotoSketch::SketchStart_varF()
{
	const char* src = (const char*)paramFallowGet(0);
	m_stop_thread = false;
	m_pathFrame = "../PhotoFrames/port_";
	m_pathPicture = "../PhotoPicture/";
	m_pathPicture += src;
	m_pictureSize[0] = 810;
	m_pictureSize[1] = 1080;
	m_stepCounter = 0;
	m_threadIdx = 1;
	BaseSystem::createthread(update_, 0, this);

	return 1;
}

//#SF_functionInsert
