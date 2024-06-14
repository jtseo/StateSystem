#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "../../SDK/opencv/include/opencv2/opencv.hpp"
#include "ExtendOpenCV.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"


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
		STDEF_SFREGIST(SketchPannelMake_varF);
		STDEF_SFREGIST(QRCodeMake_varF);
		STDEF_SFREGIST(PhotoPannelImgAdd_varF);
		STDEF_SFREGIST(Rotate_varF);
		STDEF_SFREGIST(DoubleMake_varF);
		STDEF_SFREGIST(FilterApply_varF);
		STDEF_SFREGIST(Resize_varF);
		STDEF_SFREGIST(ImageScale_fF);
		STDEF_SFREGIST(ConvertBmp_varF);
		STDEF_SFREGIST(PhotoVideoFrameMake_strF);
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
		STDEF_SFFUNCALL(SketchPannelMake_varF);
		STDEF_SFFUNCALL(QRCodeMake_varF);
		STDEF_SFFUNCALL(PhotoPannelImgAdd_varF);
		STDEF_SFFUNCALL(Rotate_varF);
		STDEF_SFFUNCALL(DoubleMake_varF);
		STDEF_SFFUNCALL(FilterApply_varF);
		STDEF_SFFUNCALL(Resize_varF);
		STDEF_SFFUNCALL(ImageScale_fF);
		STDEF_SFFUNCALL(ConvertBmp_varF);
		STDEF_SFFUNCALL(PhotoVideoFrameMake_strF);
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
	// cut ratio5
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

	cv::GaussianBlur(croppedImage, croppedImage, cv::Size(5, 5), 0);
	// Scale the image to the new size
	cv::Mat scaledImage;
	cv::resize(croppedImage, scaledImage, newSize, 0, 0, cv::INTER_AREA);
	//cv::flip(scaledImage, scaledImage, 1); // already filiped at EventCastPicture

	cv::imwrite(filename, scaledImage);

	return 1;
}

void ExtendOpenCV::overlayImage(cv::Mat& background, const cv::Mat& foreground, cv::Point2i location, double alpha, bool _overwrite) 
{
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
			if(_overwrite)
				opacity = 1;
			for (int c = 0; opacity > 0 && c < background.channels(); ++c) {
				unsigned char foregroundPx = foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
				background.data[y * background.step + x * background.channels() + c] =
					backgroundPx * (1. - opacity) + foregroundPx * opacity;
			}
		}
	}
}

void fillwhite(cv::Mat& background)
{
	for (int y = 0; y < background.rows; ++y) {
		for (int x = 0; x < background.cols; ++x) {
			for (int c = 0; c < background.channels(); ++c) {
				background.data[y * background.step + x * background.channels() + c] = 0xff;
			}
		}
	}
}

int ExtendOpenCV::Resize_varF()
{
	const int* size_an = (const int*)paramVariableGet();
	const char* filename = (const char*)paramFallowGet(0);
	const char* fileout = (const char*)paramFallowGet(1);

	cv::Mat img = cv::imread(filename);
	if (img.empty())
		return 0;

	cv::Size imageSize(size_an[0], size_an[1]);
	cv::Mat resizeImg = cv::Mat::zeros(imageSize, CV_8UC3);

	cv::resize(img, resizeImg, imageSize);

	cv::imwrite(fileout, resizeImg);

	return 1;
}

int ExtendOpenCV::ImageScale_fF()
{
	const float* scale = (const float*)m_param_value;
	const char* filename = (const char*)paramFallowGet(0);
	const char* fileout = (const char*)paramFallowGet(1);

	cv::Mat img = cv::imread(filename);
	if(img.empty())
		return 0;

	cv::Size imageSize(img.cols * *scale, img.rows * *scale);
	cv::Mat resizeImg = cv::Mat::zeros(imageSize, CV_8UC3);

	cv::resize(img, resizeImg, imageSize);

	cv::imwrite(fileout, resizeImg);

	return 1;
}

int ExtendOpenCV::ConvertBmp_varF()
{
	const char* filename = (const char*)paramVariableGet();
	const char* fileout = (const char*)paramFallowGet(0);
	const int* vertical = (const int*)paramFallowGet(1);
	const int* horizon = (const int*)paramFallowGet(2);
	const char* flip = (const char*)paramFallowGet(3);

	int v = 0, h = 0;
	if (vertical)
		v = *vertical;
	if (horizon)
		h = *horizon;
	cv::Mat img = cv::imread(filename);

	cv::Size imageSize(img.cols + h, img.rows + v);
	cv::Mat offsetImg = cv::Mat::zeros(imageSize, CV_8UC3);
	fillwhite(offsetImg);
	overlayImage(offsetImg, img, cv::Point2i(h, v), 1, true);
	if (flip && strncmp(flip, "y_flip", 6) == 0)
	{
		cv::flip(offsetImg, offsetImg, 1);
	}
	cv::imwrite(fileout, offsetImg);

	return 1;
}

int ExtendOpenCV::FilterApply_varF()
{
	const char *filename = (const char*)paramVariableGet();
	const int *filter = (const int*)paramFallowGet(1);
	const int *option = (const int*)paramFallowGet(2);
	
	if(!filter)
		return 0;
	STLVString listFrom, listTo;
	BaseFile::paser_list_seperate(filename, &listFrom, ",");
	
	for(int i=0; i<listFrom.size(); i++)
	{
		STLString path = "../Pictures/";
		path += listFrom[i];
		cv::Mat img = cv::imread(path.c_str(), cv::IMREAD_UNCHANGED);
		
		PtVector3 col;
		int pixel = img.channels();
		for(int x=0; x<img.cols; x++)
		{
			for(int y=0; y<img.rows; y++)
			{
				for(int i=0; i<3; i++)
					col[i] = (float)img.data[y*img.step + x * pixel+i]/255.f;
				
				switch(*filter)
				{
					case 0: // org;
						break;
					case 1: // bright;
						col *= (float)*option/100.f;
						break;
					case 2: // dark;
						col *= (float)*option/100.f;
						break;
					case 3: // gray;
					{
						float avg = 0;
						avg = col[0] +  col[1] + col[2];
						avg /= 3.f;
						for(int i=0; i<3; i++)
							col[i] = avg;
					}
						break;
					case 4:
						break;
				}
				
				for(int i=0; i<3; i++){
					if(col[i] < 0)
						col[i] = 0;
					if(col[i] > 1)
						col[i] = 1;
					img.data[y*img.step + x * pixel+i] = (int)(col[i]*255.f);
				}
			}
		}
		char buff[255];
		BaseFile::get_filename(listFrom[i].c_str(), buff, 255);
		STLString to = buff;
		if(to.find("filter") == STLString::npos)
			to += "filter";
		if(BaseFile::get_filext(listFrom[i].c_str(), buff, 255))
			to += buff;
		listTo.push_back(to);
		path = "../Pictures/";
		path += to;
		cv::imwrite(path.c_str(), img);
	}
	char namesTo[255];
	BaseFile::paser_list_merge(namesTo, 255, listTo, ",");
	if(!paramFallowSet(0, namesTo))
		return 0;
	return 1;
}

int ExtendOpenCV::DoubleMake_varF()
{
	const char *filename = (const char*)paramVariableGet();
	
	cv::Mat img = cv::imread(filename);

	if (img.cols > 1200)
		return 0;

	cv::Size newSize(img.cols*2, img.rows);
	cv::Mat doubleImg = cv::Mat::zeros(newSize, CV_8UC3);

	img.copyTo(doubleImg(cv::Rect(0, 0, img.cols, img.rows)));
	img.copyTo(doubleImg(cv::Rect(img.cols, 0, img.cols, img.rows)));

	// Save the composed image to a file
	cv::imwrite(filename, doubleImg);
	return 1;
}

int ExtendOpenCV::Rotate_varF()
{
	const char *imagePath = (const char*)paramVariableGet();
	const int *rot = (const int*)paramFallowGet(0);
	
	cv::Mat image = cv::imread(imagePath);

	if (image.empty()) {
		std::cerr << "Error: Image not found." << std::endl;
		return 0;
	}

	// Step 2: Check if the image is in portrait mode (height > width)
	if (image.rows >= image.cols)
		return 0;
	// Step 3: Rotate the image to landscape mode
	cv::Size roSize(image.size[1], image.size[0]);
	cv::Mat rotatedImage = cv::Mat::zeros(roSize, CV_8UC3);;
	
	cv::transpose(image, rotatedImage);
	cv::flip(rotatedImage, rotatedImage, 0);

	// Step 4: Save the rotated image
	if (!cv::imwrite(imagePath, rotatedImage))
		return 0;

	return 1;
}

int ExtendOpenCV::PhotoPannelImgAdd_varF()
{
	const char *pannelName = (const char*)paramVariableGet();
	const char *imageName = (const char*)paramFallowGet(0);
	const int *pos = (const int *)paramFallowGet(1);
	const int *overwrite = (const int*)paramFallowGet(2);
	const char* outName = (const char*)paramFallowGet(3);
	
	if(!imageName || !pos)
		return 0;
	
	// Load a background image with alpha channel
	cv::Mat backgroundImage = cv::imread(pannelName, cv::IMREAD_UNCHANGED);

	std::string outputFilePath = pannelName; // Path to save the final image
	if (outName)
		outputFilePath = outName;

	cv::Mat img = cv::imread(imageName);

	overlayImage(backgroundImage, img, cv::Point2i(pos[0], pos[1]), 1, *overwrite==1);

	// Save the composed image to a file
	cv::imwrite(outputFilePath, backgroundImage);
	
	return 1;
}

int ExtendOpenCV::QRCodeMake_varF()
{
	const char *url = (const char*)paramVariableGet();
	const char *filepath = (const char*)paramFallowGet(0);
	const int *size = (const int*)paramFallowGet(1);
	
	if(!url || !filepath || !size)
		return 0;
	
	std::string cmd = " -o ";
	cmd += filepath;
	cmd += " ";
	cmd += url;
	
	cv::Mat qrCodeImage;

	BaseSystem::run_shell_command("open", "qrencode", cmd.c_str(), false);
	BaseSystem::Sleep(500);
	cv::Size newSize(size[0], size[1]);
	cv::Mat imgQr = cv::imread(filepath);
	int cnt = 5;
	while (imgQr.rows == 0)
	{
		BaseSystem::Sleep(10);
		imgQr = cv::imread(filepath);
	}
	// Scale the image to the new size
	cv::Mat scaledImage;
	cv::resize(imgQr, scaledImage, newSize);

	cv::imwrite(filepath, scaledImage);
	
	return 1;
}

int ExtendOpenCV::PhotoVideoFrameMake_strF()
{
	const char* outfile = (const char*)m_param_value;
	const int* slots = (const int*)paramFallowGet(0); // slot's indexs
	const float* scale_p = (const float*)paramFallowGet(1); // scale for video
	float scale = *scale_p;

	const char* backImageName = NULL;
	if (!m_state_variable->get(STRTOHASH("PhotoFilename"), (const void**)&backImageName))
		return 0;
	m_manager_p = BaseStateManager::get_manager();
	const char* fileroot = m_manager_p->rootGet();
	STLString root = fileroot;
	STLString imgPath = fileroot;
	imgPath += "../";
	imgPath += backImageName;
	const int* photoSize = NULL;
	if (!m_state_variable->get(STRTOHASH("PhotoSize"), (const void**)&photoSize))
		return 0;
	const int* photoPoss = NULL;
	short count = 0;
	if (!m_state_variable->get(STRTOHASH("PhotoPositions"), (const void**)&photoPoss, &count))
		return 0;

	const int* pictureSize_p = NULL;
	if (!m_state_variable->get(STRTOHASH("PhotoPictureSize"), (const void**)&pictureSize_p))
		return 0;

	count /= 2;
	int frameCnt = 0;
	STLVString filenames;
	for (int i = 0; i < count; i++)
	{
		char buf[255];
		sprintf_s(buf, 255, "..\\Pictures\\slot%d\\*.jpg", slots[i]+1);
		BaseSystem::GetFileList(buf, &filenames, NULL);

		for (int j = 0; j < filenames.size(); j++)
		{
			int frame = 0;
			sscanf(filenames[j].c_str(), "img%d.jpg", &frame);
			if (frame > frameCnt)
				frameCnt = frame;
		}
	}

	// Load a background image with alpha channel
	cv::Size imageSize(photoSize[0] * scale, photoSize[1] * scale); // Desired image size
	cv::Mat backgroundImage = cv::imread(imgPath.c_str(), cv::IMREAD_UNCHANGED);
	cv::resize(backgroundImage, backgroundImage, imageSize);
	int w, h;
	w = pictureSize_p[0] * scale;
	h = pictureSize_p[1] * scale;
	if (pictureSize_p[0] < photoSize[0])
		w++;
	if (pictureSize_p[1] < photoSize[1])
		h++;
	cv::Size picSize(pictureSize_p[0]*scale, pictureSize_p[1]*scale);

	std::vector<cv::Mat> smallImages; // Vector to hold small images
	for (int frame = 1; frame <= frameCnt; frame++)
	{
		// Load small images (example)
		root = "../Pictures/";
		char buf[255];
		for (int i = 1; i <= count; i++)
		{
			sprintf_s(buf, 255, "slot%d/img%d.jpg", slots[i-1]+1, frame);
			STLString path = root + buf;
			cv::Mat frame = cv::imread(path.c_str(), cv::IMREAD_UNCHANGED);
			if (!frame.empty()) {
				cv::resize(frame, frame, picSize);
				if (smallImages.size() == count)
					smallImages[i-1] = frame;
			}
			
			if(smallImages.size() < count)
				smallImages.push_back(frame);
		}

		sprintf_s(buf, 255, "%svideo/frame%d.jpg", root.c_str(), frame);
		std::string outputFilePath = buf; // Path to save the final image

		cv::Mat image = cv::Mat::zeros(imageSize, CV_8UC3);

		// Paste small images onto the blank image at specified locations
		for (size_t i = 0; i < smallImages.size(); ++i) {
			if (smallImages[i].empty())
				continue;
			cv::Point2i location(photoPoss[i * 2] * scale, photoPoss[i * 2 + 1] * scale); // Example locations for each small image
			smallImages[i].copyTo(image(cv::Rect(location.x, location.y, smallImages[i].cols, smallImages[i].rows)));
		}

		// Overlay the background image with alpha value
		if (!backgroundImage.empty()) {
			overlayImage(image, backgroundImage, cv::Point2i(0, 0), 1); // 50% opacity for the background image
		}

		// Save the composed image to a file
		cv::imwrite(outputFilePath, image);
	}
	return 1;
}

cv::Size SizeGet(const char *_name, BaseDStructureValue *_variable)
{
	const int* array = NULL;
	cv::Size ret(0, 0);
	if (!_variable->get(STRTOHASH(_name), (const void**)&array))
		return ret;
	ret.width = array[0];
	ret.height = array[1];

	return ret;
}

const int *PositionGet(const char* _name, BaseDStructureValue* _variable)
{
	const int* array = NULL;
	if (!_variable->get(STRTOHASH(_name), (const void**)&array))
		return NULL;
	return array;
}

int ExtendOpenCV::SketchPannelMake_varF()
{
	const int* numOfPic = (const int*)paramVariableGet();
	const char* pathPicture = (const char*)paramFallowGet(0);
	const char* pathLogo = (const char*)paramFallowGet(1);
	const char* pathDate = (const char*)paramFallowGet(2);
	const char* outfile = (const char*)paramFallowGet(3);

	const int* photoPoss = PositionGet("PhotoPositions", m_state_variable);
	if (photoPoss == NULL || pathDate == NULL || pathLogo == NULL)
		return 0;

	//cv::Mat qrMat = cv::imread(pathQR);
	cv::Mat dateMat = cv::imread(pathDate);
	cv::Mat logoMat = cv::imread(pathLogo);
	if (dateMat.empty() || logoMat.empty())
		return 0;

	const int *datePos = PositionGet("PhotoDatePos", m_state_variable);
	//const int *qrPos = PositionGet("PhotoQRPos", m_state_variable);
	const int *logoPos = PositionGet("PhotoLogoPos", m_state_variable);

	cv::Size picSize = SizeGet("PhotoPictureSize", m_state_variable);
	cv::Size dateSize = SizeGet("PhotoDateSize", m_state_variable);
	//cv::Size qrSize = SizeGet("PhotoQRSize", m_state_variable);
	cv::Size logoSize = SizeGet("PhotoLogoSize", m_state_variable);
	cv::Size imageSize = SizeGet("PhotoSize", m_state_variable);// Desired image size
	std::vector<cv::Mat> smallImages; // Vector to hold small images
	// Load small images (example)
	STLString root = pathPicture;
	root += "sketch";

	for (int i = 0; i < *numOfPic; i++)
	{
		STLString path = root;
		path += ('1' + i);
		path += "/result.jpg";
	
		cv::Mat m2 = cv::imread(path.c_str());
		cv::resize(m2, m2, picSize);
		smallImages.push_back(m2);

		//smallImages.push_back(cv::imread(path.c_str(), cv::IMREAD_UNCHANGED));
	}
	std::string outputFilePath = outfile; // Path to save the final image

	cv::Mat image = cv::Mat::zeros(imageSize, CV_8UC3);
	image.setTo(cv::Scalar(255, 255, 255));
	// Paste small images onto the blank image at specified locations
	for (size_t i = 0; i < smallImages.size(); ++i) {
		cv::Point2i location(photoPoss[i * 2], photoPoss[i * 2 + 1]); // Example locations for each small image
		smallImages[i].copyTo(image(cv::Rect(location.x, location.y, smallImages[i].cols, smallImages[i].rows)));
	}

	cv::resize(dateMat, dateMat, dateSize);
	dateMat.copyTo(image(cv::Rect(datePos[0], datePos[1], dateMat.cols, dateMat.rows)));
	cv::resize(logoMat, logoMat, logoSize);
	logoMat.copyTo(image(cv::Rect(logoPos[0], logoPos[1], logoMat.cols, logoMat.rows)));
	//if (!qrMat.empty()) {
	//	cv::resize(qrMat, qrMat, qrSize);
	//	qrMat.copyTo(image(cv::Rect(qrPos[0], qrPos[1], qrMat.cols, qrMat.rows)));
	//}

	imageSize.width *= 2;
	imageSize.height *= 2;
	cv::resize(image, image, imageSize);
	// Save the composed image to a file

	if (imageSize.width > imageSize.height)
	{
		cv::transpose(image, image);
		cv::flip(image, image, 0);
	}
	cv::imwrite(outputFilePath, image);

	return 1;
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
	if (!m_state_variable->get(STRTOHASH("PhotoPictureFilteredPaths"), (const void**)&pictureNames))
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
	root = "../Pictures/";
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

	for (int i = 0; i < pictures.size(); i++)
	{
		STLString path = root;
		path += pictures[i];
		BaseSystem::file_delete(path.c_str());
	}
	return 1;
}
//#SF_functionInsert

//std::mutex matMutex;

DEF_ThreadCallBack(ExtendOpenCV::threadSave)
{
	OpenCVSave* img = (OpenCVSave*)_pParam;
	//std::lock_guard<std::mutex> lock(matMutex);

	cv::resize(img->m_image, img->m_image, img->m_imageSize);

	cv::imwrite(img->m_filename.c_str(), img->m_image);

	img->m_image.release();
	delete img;

	BaseSystem::endthread();
	DEF_ThreadReturn;
}

void ExtendOpenCV::imageSave(cv::Mat _img, cv::Size _size, STLString _filename)
{
	OpenCVSave *img = new OpenCVSave();
	img->m_image = _img;
	img->m_filename = _filename;
	img->m_imageSize = _size;
	_img.release();

	BaseSystem::createthread(threadSave_, 0, img);
}