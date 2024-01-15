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

CCameraControlApp* CCameraControlApp::ms_instance = NULL;


CCameraControlApp* CCameraControlApp::Instance()
{
	if (ms_instance == NULL)
	{
		ms_instance = new CCameraControlApp();
		ms_instance->InitInstance();
	}

	return ms_instance;
}
// CCameraControlApp initialization
BOOL CCameraControlApp::InitInstance()
{
	_controller = NULL;
	_model = NULL;
	m_refAlloc = 0;
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

void CCameraControlApp::UnInitInstance()
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

	if (m_refAlloc)
		mpool_get().free_mem(m_refAlloc);
	m_refAlloc = 0;
	ms_instance = NULL;
}

void CCameraControlApp::update(Observable* from, CameraEvent* e)
{
	EventCastPicture(e);
	EventCastPreview(e);
	EventCastProperty(e);
}

void CCameraControlApp::EventCastPicture(CameraEvent* _evt)
{

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

void CCameraControlApp::EventCastPreview(CameraEvent* _evt)
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

		if (pbyteImage != NULL && m_eventCastEnable)
		{
			EdsGetLength(data.stream, &size);

			int scale = 1;
			//std::vector<unsigned char> raw = ExtractImageData(image, scale);
			int w = 960;// image.GetWidth() / scale;
			int h = 640;// image.GetHeight() / scale;

			BaseStateManager* manager = BaseStateManager::get_manager();
			BaseDStructureValue* evt = manager->make_event_state(STRTOHASH("CamRevPreview"));
			if (m_refAlloc != 0)
				mpool_get().free_mem(m_refAlloc);
			m_refAlloc = mpool_get().get_alloc(size);
			char* buf = (char*)m_refAlloc;
			memcpy(buf, pbyteImage, size);
			evt->set_alloc("MemRef_nV", &m_refAlloc);
			evt->set_alloc("ImageHeight_nV", &h);
			evt->set_alloc("ImageWidth_nV", &w);

			m_eventCastEnable = false;
			manager->post_event(evt);
		}

	}
}

void CCameraControlApp::EventCastEnable() 
{
	m_eventCastEnable = true;

	CameraController* ctr = getCameraController();
	if (ctr == NULL)
		return;
	// Download image data.
	ActionEvent evt("downloadEVF");
	ctr->actionPerformed(evt);
}

void CCameraControlApp::EventCastProperty(CameraEvent* _evt)
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
