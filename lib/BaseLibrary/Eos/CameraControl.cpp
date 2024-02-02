/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : CameraControl.cpp                                               *
*                                                                             *
*   Description: This is the Sample code to show the usage of EDSDK.          *
*                                                                             *
*                                                                             *
*******************************************************************************
*                                                                             *
*   Written and developed by Camera Design Dept.53                            *
*   Copyright Canon Inc. 2006-2008 All Rights Reserved                        *
*                                                                             *
*******************************************************************************/

#include "stdafx.h"
#ifndef VScriptBuild

#include <objbase.h>

#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "Camera/CameraModel.h"
#include "Camera/CameraModelLegacy.h"
#include "Camera/CameraController.h"
#include "Camera/CameraEventListener.h"
#include "CameraControl.h"

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
#include <afxwin.h>
#include <atlimage.h>

#include "../PtBase/base.h"

#include "../PtBase/Vector3.h"
#include "../PtBase/Matrix3.h"
#include "../PtBase/Matrix4.h"

#include "../PtBase/BaseObject.h"

#include "../PtBase/hashstr.h"

#include "../PtBase/BaseDStructure.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseStateManager.h"


#include <Windows.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif


CameraModel* cameraModelFactory(EdsCameraRef camera, EdsDeviceInfo deviceInfo);

CCameraControl* CCameraControl::ms_instance = NULL;


CCameraControl::CCameraControl()
{
}

CCameraControl::~CCameraControl()
{
	UnInitInstance();
}

void CCameraControl::Reset()
{
	if (ms_instance != NULL)
		delete ms_instance;
	ms_instance = NULL;
}

CCameraControl* CCameraControl::Instance()
{
	if (ms_instance == NULL)
	{
		ms_instance = new CCameraControl();
		ms_instance->InitInstance();
	}

	return ms_instance;
}
// CCameraControlApp initialization
BOOL CCameraControl::InitInstance()
{
	_controller = NULL;
	_model = NULL;
	m_eventCastEnable = true;
	m_active = false;
	EdsError	 err = EDS_ERR_OK;
	EdsCameraListRef cameraList = NULL;
	EdsUInt32	 count = 0;

	// Initialization of SDK
	err = EdsInitializeSDK();

	if(err == EDS_ERR_OK)
	{
		m_isSDKLoaded = true;
	}

	//Acquisition of camera list
	if(err == EDS_ERR_OK)
	{
		err = EdsGetCameraList(&cameraList);
	}

	//Acquisition of number of Cameras
	if(err == EDS_ERR_OK)
	{
		err = EdsGetChildCount(cameraList, &count);
		if(count == 0)
		{
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}


	//Acquisition of camera at the head of the list
	if(err == EDS_ERR_OK)
	{	
		err = EdsGetChildAtIndex(cameraList , 0 , &m_camera);
	}

	//Acquisition of camera information
	EdsDeviceInfo deviceInfo;
	if(err == EDS_ERR_OK)
	{	
		err = EdsGetDeviceInfo(m_camera , &deviceInfo);
		if(err == EDS_ERR_OK && m_camera == NULL)
		{
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}


	//Release camera list
	if(cameraList != NULL)
	{
		EdsRelease(cameraList);
	}

	//Create Camera model
	if(err == EDS_ERR_OK )
	{
		_model = cameraModelFactory(m_camera, deviceInfo);
		if(_model == NULL)
		{
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}

	if(err != EDS_ERR_OK)
	{
		::MessageBoxA(NULL,"Cannot detect camera",NULL,MB_OK);
	}

	if(err == EDS_ERR_OK )
	{
		//Create CameraController
		_controller = new CameraController();
		//Create View Dialog
		//CCameraControlDlg			view;
		
		_controller->setCameraModel(_model);
		_model->addObserver(this);
		// Send Model Event to view	
		//view.setCameraController(_controller);

		//Set Property Event Handler
		if(err == EDS_ERR_OK)
		{
			err = EdsSetPropertyEventHandler(m_camera, kEdsPropertyEvent_All, CameraEventListener::handlePropertyEvent , (EdsVoid *)_controller);
		}

		//Set Object Event Handler
		if(err == EDS_ERR_OK)
		{
			err = EdsSetObjectEventHandler(m_camera, kEdsObjectEvent_All, CameraEventListener::handleObjectEvent , (EdsVoid *)_controller);
		}

		//Set State Event Handler
		if(err == EDS_ERR_OK)
		{
			err = EdsSetCameraStateEventHandler(m_camera, kEdsStateEvent_All, CameraEventListener::handleStateEvent , (EdsVoid *)_controller);
		}

		//m_pMainWnd = &view;
		//INT_PTR nResponse = view.DoModal();

	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CCameraControl::UnInitInstance()
{

	//Release Camera
	if (m_camera != NULL)
	{
		EdsRelease(m_camera);
		m_camera = NULL;
	}

	//Termination of SDK
	if (m_isSDKLoaded)
	{
		EdsTerminateSDK();
	}

	if (_model != NULL)
	{
		delete _model;
		_model = NULL;
	}


	if (_controller != NULL)
	{
		delete _controller;
		_controller = NULL;
	}

	ms_instance = NULL;
}

void CCameraControl::update(Observable* from, CameraEvent* e)
{
	EventCastPicture(e);
	EventCastPreview(e);
	EventCastProperty(e);
}


#include <atlimage.h>
#include <vector>

std::vector<unsigned char> ExtractImageData(CImage& image, int _scale)
{
	// Prepare the container for the image data
	std::vector<unsigned char> imageData;

	// Ensure the image is not compressed and is in a 24 or 32-bit format
	if (image.GetBPP() != 24 && image.GetBPP() != 32)
	{
		return imageData;
	}
	
	int width = image.GetWidth();
	int height = image.GetHeight();
	int pitch = image.GetPitch() * -1;
	int bytesPerPixel = image.GetBPP() / 8;

	unsigned char* rawData = (unsigned char*)image.GetBits();

	int step = _scale;
	for (int y = 0; y < height; y+=step)
	{
		for (int x = 0; x < width; x+=step)
		{
			COLORREF col = image.GetPixel(x, y); // r, g, b, a
			imageData.push_back(GetRValue(col));
			imageData.push_back(GetGValue(col));
			imageData.push_back(GetBValue(col));
			imageData.push_back(255);
		}
	}

	return imageData;
}

#include "../PtBase/BaseFile.h"

void CCameraControl::EventCastPicture(CameraEvent* _evt)
{
	std::string event = _evt->getEvent();

	if (event == "DownloadComplete")
	{
		const char *filename = (const char*)_evt->getArg();

		BaseStateManager* manager = BaseStateManager::get_manager();
		BaseDStructureValue* evt = manager->make_event_state(STRTOHASH("CamRevPicture"));
		evt->set_alloc("PicturePath_strV", (void*)filename);

		manager->post_event(evt);
	}
}

#include <opencv2/opencv.hpp>

using namespace cv;

void CCameraControl::StreamFree()
{
	void* point = NULL;
	do {
		if (point)
			PT_Free(point);
		point = BaseCircleQueue::stream_get()->pop();
		BaseCircleQueue::streamSize_get()->pop();
	} while (point);
}

void CCameraControl::EventCastPreview(CameraEvent* _evt)
{
	std::string event = _evt->getEvent();

	if (event == "EvfDataChanged")
	{
		EVF_DATASET data = *static_cast<EVF_DATASET*>(_evt->getArg());

		//EdsInt32 propertyID = kEdsPropID_FocusInfo;
		//fireEvent("get_Property", &propertyID);
		EdsUInt64 size;

		unsigned char* pbyteImage = NULL;

		// Get image (JPEG) pointer.
		EdsGetPointer(data.stream, (EdsVoid**)&pbyteImage);

		if (pbyteImage != NULL && BaseCircleQueue::stream_get()->size_data() < 10)
		{
			EdsGetLength(data.stream, &size);
			std::vector<uchar> jpgData;
			jpgData.resize(size);
			memcpy(&jpgData[0], pbyteImage, size);

			Mat orgImg = imdecode(Mat(jpgData), IMREAD_COLOR);
			Mat resizeImg;
			
			int scale = 1;
			//std::vector<unsigned char> raw = ExtractImageData(image, scale);
			int w = 960;// image.GetWidth() / scale;
			int h = 640;// image.GetHeight() / scale;

			resize(orgImg, resizeImg, Size(w / 2, h / 2));
			imencode(".jpg", resizeImg, jpgData);

			int imgS = jpgData.size();
			
			BaseStateManager* manager = BaseStateManager::get_manager();
			BaseDStructureValue* evt = manager->make_event_state(STRTOHASH("CamRevPreview"));
			char *buf = PT_Alloc(char, jpgData.size());
			memcpy(buf, &jpgData[0], jpgData.size());

			BaseCircleQueue::stream_get()->push(buf);
			BaseCircleQueue::streamSize_get()->push((void*)imgS);
			//BaseFile file;
			//file.OpenFile("test.jpg", BaseFile::OPEN_WRITE);
			//file.Write(buf, imgS);
			//file.CloseFile();
			INT64 ref = (INT64) buf;
			evt->set_alloc("MemRef_nV", &ref);
			evt->set_alloc("ImageSize_nV", &imgS);
			evt->set_alloc("ImageHeight_nV", &h);
			evt->set_alloc("ImageWidth_nV", &w);

			manager->post_event(evt);
		}

		BaseSystem::Sleep(33);
		PreviewRequest();

	}
}

void CCameraControl::EventCastEnable() 
{
	m_eventCastEnable = true;
}

void CCameraControl::PreviewRequest()
{
	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return;
	// Download image data.
	ActionEvent evt("downloadEVF");
	ctr->actionPerformed(evt);
}

void CCameraControl::EventCastProperty(CameraEvent* _evt)
{
	// it's in thread
	std::string event = _evt->getEvent();

	if (event == "PropertyChanged")
	{
		EdsInt32 proeprtyID = *static_cast<EdsInt32*>(_evt->getArg());
		if (proeprtyID == kEdsPropID_Evf_OutputDevice)
		{
			CameraModel* model = getCameraModel();
			EdsUInt32 device = model->getEvfOutputDevice();

			CameraController* ctr = getCameraController();
			if (ctr == NULL)
				return;

			// PC live view has started.
			if (!m_active && (device & kEdsEvfOutputDevice_PC) != 0)
			{
				m_active = TRUE;
				// Start download of image data.
				ActionEvent evt("downloadEVF");
				ctr->actionPerformed(evt);
			}

			// PC live view has ended.
			if (m_active && (device & kEdsEvfOutputDevice_PC) == 0)
			{
				m_active = FALSE;
			}
		}
	}
}

CameraModel* cameraModelFactory(EdsCameraRef camera, EdsDeviceInfo deviceInfo)
{
	// if Legacy protocol.
	if(deviceInfo.deviceSubType == 0)
	{
		return new CameraModelLegacy(camera);
	}

	// PTP protocol.
	return new CameraModel(camera);
}

#endif