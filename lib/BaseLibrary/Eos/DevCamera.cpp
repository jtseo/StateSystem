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

#include "CameraControl.h"

#include <opencv2/opencv.hpp>

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
		STDEF_SFREGIST(PicturesRateAddapt_varF);
		STDEF_SFREGIST(PhotoPrintMake_statevarF);
		STDEF_SFREGIST(PhotoPrintOutMake_strF);
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
		STDEF_SFFUNCALL(PicturesRateAddapt_varF);
		STDEF_SFFUNCALL(PhotoPrintMake_statevarF);
		STDEF_SFFUNCALL(PhotoPrintOutMake_strF);
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

int DevCamera::PreviewStart_nF()
{
	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return 0;

	ActionEvent evt("startEVF");
	ctr->actionPerformed(evt);

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

int DevCamera::PicturesRateAddapt_varF()
{
	const char* filename = (const char*)paramVariableGet();
	if (!filename)
		return 0;
	const int* sizexy = (const int*)paramFallowGet(1);
	if (!sizexy)
		return 0;
	cv::Size newSize(sizexy[0], sizexy[1]);
	float ratio = (float)sizexy[0] / (float)sizexy[1];
	// read original
	// cut ratio
	// scale image

	 // Read the image from the file
	cv::Mat image = cv::imread(filename);
	if (image.empty()) {
		return 0;
	}

	// Crop the image with the specified ratio
	int cropWidth, cropHeight;
	if (image.cols > image.rows * ratio) {
		cropHeight = image.rows;
		cropWidth = static_cast<int>(cropHeight * ratio);
	}
	else {
		cropWidth = image.cols;
		cropHeight = static_cast<int>(cropWidth / ratio);
	}
	int x = (image.cols - cropWidth) / 2;
	int y = (image.rows - cropHeight) / 2;
	cv::Rect cropRegion(x, y, cropWidth, cropHeight);
	cv::Mat croppedImage = image(cropRegion);

	// Scale the image to the new size
	cv::Mat scaledImage;
	cv::resize(croppedImage, scaledImage, newSize);

	cv::imwrite(filename, scaledImage);

	return 1;
}

int DevCamera::PhotoPrintMake_statevarF()// it will remove
{
	// empty image
	// paste pictures
	// paste backimage
	return 1;
}

void overlayImage(cv::Mat& background, const cv::Mat& foreground, cv::Point2i location, double alpha = 0.5) {
	for (int y = std::max(location.y, 0); y < background.rows; ++y) {
		int fY = y - location.y; // 0 <= fY < foreground.rows
		if (fY >= foreground.rows)
			break;

		for (int x = std::max(location.x, 0); x < background.cols; ++x) {
			int fX = x - location.x; // 0 <= fX < foreground.cols
			if (fX >= foreground.cols)
				break;

			// Blend the foreground and background pixels
			double opacity = ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3]) / 255. * alpha;
			for (int c = 0; opacity > 0 && c < background.channels(); ++c) {
				unsigned char foregroundPx = foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
				background.data[y * background.step + x * background.channels() + c] =
					backgroundPx * (1. - opacity) + foregroundPx * opacity;
			}
		}
	}
}

#include "nlohmann/json.hpp"
#include <opencv2/opencv.hpp>

//using namespace cv;
using json = nlohmann::json;
//
// parseFrame(const char* _json)
//{
//	auto frame = json::parse(_json);
//
//	fr.numPic = frame["numPicture"];
//	fr.size[0] = frame["size"]["x"];
//	fr.size[1] = frame["size"]["y"];
//	fr.picSize[0] = frame["pictureSize"]["x"];
//	fr.picSize[1] = frame["pictureSize"]["y"];
//	for (int i = 0; i < 8; i++)
//		fr.pictures[i][0] = -1;
//	for (int i = 0; i < frame["pictures"].size(); i++)
//	{
//		fr.pictures[i][0] = frame["pictures"][i]["x"];
//		fr.pictures[i][1] = frame["pictures"][i]["y"];
//	}
//	fr.qrSize[0] = frame["qrSize"]["x"];
//	fr.qrSize[1] = frame["qrSize"]["y"];
//	fr.qrPos[0] = frame["qrPos"]["x"];
//	fr.qrPos[1] = frame["qrPos"]["y"];
//	fr.price = frame["price"];
//
//	return fr;
//}

int DevCamera::PhotoPrintOutMake_strF()
{
	const char* outfile = (const char*)m_param_value;
	const char* frameType = (const char*)paramFallowGet(0);// design or origin
	const int* framNum = (const int*)paramFallowGet(1);

	cv::Size imageSize(800, 600); // Desired image size
	std::vector<cv::Mat> smallImages; // Vector to hold small images
	// Load small images (example)
	// smallImages.push_back(cv::imread("path_to_small_image_1.png", cv::IMREAD_UNCHANGED));
	// smallImages.push_back(cv::imread("path_to_small_image_2.png", cv::IMREAD_UNCHANGED));
	// Load a background image with alpha channel
	cv::Mat backgroundImage = cv::imread("path_to_background_image.png", cv::IMREAD_UNCHANGED);
	std::string outputFilePath = "output_image.png"; // Path to save the final image

	BaseFile file;
	STLString path = "../";
	path += frameType;
	path += "/double_";
	
	if (file.OpenFile(path.c_str(), BaseFile::OPEN_READ))
		return 0;
	STLString json;
	if (!file.Read(&json))
		return 0;
	auto frame = json::parse(json.c_str());
		
	cv::Mat image = cv::Mat::zeros(imageSize, CV_8UC3);

	// Paste small images onto the blank image at specified locations
	for (size_t i = 0; i < smallImages.size(); ++i) {
		cv::Point2i location(i * 100, i * 50); // Example locations for each small image
		smallImages[i].copyTo(image(cv::Rect(location.x, location.y, smallImages[i].cols, smallImages[i].rows)));
	}

	// Overlay the background image with alpha value
	if (!backgroundImage.empty()) {
		overlayImage(image, backgroundImage, cv::Point2i(0, 0), 0.5); // 50% opacity for the background image
	}

	// Save the composed image to a file
	cv::imwrite(outputFilePath, image);

	return 1;
}
//#SF_functionInsert
#endif
