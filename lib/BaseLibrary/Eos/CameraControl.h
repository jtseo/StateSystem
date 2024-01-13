/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : CameraControl.h	                                              *
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

#pragma once

#include "Camera/CameraController.h"
#include "Camera/CameraModel.h"

// CCameraControlApp:
// CCameraControlApp dialog used for App About

class CCameraControlApp
{
public:

// Overrides
public:
	virtual BOOL InitInstance();
	void UnInitInstance();

	CameraModel*		_model;
	CameraController*	_controller;

	friend CameraController* getController();
	friend CameraModel* getModel();

	static CCameraControlApp* Instance();
private:
	static CCameraControlApp* ms_instance;
	EdsCameraRef m_camera = NULL;
	bool		 m_isSDKLoaded = false;
// Implementation
};

inline CameraController* getCameraController() {return CCameraControlApp::Instance()->_controller;}
inline CameraModel* getCameraModel() {return CCameraControlApp::Instance()->_model;}

extern CCameraControlApp theApp;
