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
		//_model->addObserver(&view);
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
