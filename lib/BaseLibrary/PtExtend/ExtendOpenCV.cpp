#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "ExtendOpenCV.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#include <opencv2/opencv.hpp>

PtObjectCpp(ExtendOpenCV);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	ExtendOpenCV::s_func_hash_a;
const char* ExtendOpenCV::s_class_name = "ExtendOpenCV";

int ExtendOpenCV::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

ExtendOpenCV::ExtendOpenCV()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, ExtendOpenCV::FunctionProcessor);
}

int ExtendOpenCV::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(PictureRatioAdapt_varF);
		STDEF_SFREGIST(PhotoPannelMake_strF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

ExtendOpenCV::~ExtendOpenCV()
{

}

void ExtendOpenCV::init()
{
	BaseStateFunc::init();
}

void ExtendOpenCV::release()
{
}

BaseStateFuncEx* ExtendOpenCV::CreatorCallback(const void* _param)
{
	ExtendOpenCV* bs_func;
	PT_OAlloc(bs_func, ExtendOpenCV);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, ExtendOpenCV::FunctionProcessor);

	return bs_func;
}

int ExtendOpenCV::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(PictureRatioAdapt_varF);
		STDEF_SFFUNCALL(PhotoPannelMake_strF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int ExtendOpenCV::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	ExtendOpenCV* bs_func = (ExtendOpenCV*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, ExtendOpenCV::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int ExtendOpenCV::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

int ExtendOpenCV::PictureRatioAdapt_varF()
{
	const char* filename = (const char*)paramVariableGet();
	if (!filename)
		return 0;
	const int* sizexy = (const int*)paramFallowGet(0);
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

int ExtendOpenCV::PhotoPannelMake_strF()
{
	const char* outfile = (const char*)m_param_value;
	const char* frameType = (const char*)paramFallowGet(0);// design or origin
	const int* framNum = (const int*)paramFallowGet(1);

	const char* backImageName = NULL;
	if (!m_state_variable->get(STRTOHASH("PhotoFilename"), (const void**)&backImageName))
		return 0;
	const char* fileroot = m_manager_p->rootGet();
	STLString root = fileroot;
	STLString imgPath = fileroot;
	imgPath += "../";
	imgPath += backImageName;
	const char* pictureNames = NULL;
	if (!m_state_variable->get(STRTOHASH("PhotoPicturePaths"), (const void**)&pictureNames))
		return 0;
	STLVString pictures;
	BaseFile::paser_list_seperate(pictureNames, &pictures, ",");
	if (pictures.empty())
		return 0;
	const int* photoSize = NULL;
	if (!m_state_variable->get(STRTOHASH("PhotoSize"), (const void**)&photoSize))
		return 0;
	const int* photoPoss = NULL;
	if (!m_state_variable->get(STRTOHASH("PhotoPositions"), (const void**)&photoPoss))
		return 0;

	cv::Size imageSize(photoSize[0], photoSize[1]); // Desired image size
	std::vector<cv::Mat> smallImages; // Vector to hold small images
	// Load small images (example)
	root += "../frame/";
	for (int i = 0; i < pictures.size(); i++)
	{
		STLString path = root + pictures[i];
		smallImages.push_back(cv::imread(path.c_str(), cv::IMREAD_UNCHANGED));
	}
	// Load a background image with alpha channel
	cv::Mat backgroundImage = cv::imread(imgPath.c_str(), cv::IMREAD_UNCHANGED);
	std::string outputFilePath = outfile; // Path to save the final image

	cv::Mat image = cv::Mat::zeros(imageSize, CV_8UC3);

	// Paste small images onto the blank image at specified locations
	for (size_t i = 0; i < smallImages.size(); ++i) {
		cv::Point2i location(photoPoss[i * 2], photoPoss[i * 2 + 1]); // Example locations for each small image
		smallImages[i].copyTo(image(cv::Rect(location.x, location.y, smallImages[i].cols, smallImages[i].rows)));
	}

	// Overlay the background image with alpha value
	if (!backgroundImage.empty()) {
		overlayImage(image, backgroundImage, cv::Point2i(0, 0), 1); // 50% opacity for the background image
	}

	// Save the composed image to a file
	cv::imwrite(outputFilePath, image);

	return 1;
}
//#SF_functionInsert
