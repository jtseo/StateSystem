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

class CCameraControlApp : public Observer
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
	void EventCastEnable();
private:
	static CCameraControlApp* ms_instance;
	EdsCameraRef m_camera = NULL;
	bool		 m_isSDKLoaded = false;

	void update(Observable* from, CameraEvent* e);

// Implementation
    // User defin area from here
	void EventCastPicture(CameraEvent* _evt);
	void EventCastPreview(CameraEvent* _evt);
	void EventCastProperty(CameraEvent* _evt);
	bool m_active;
	bool m_eventCastEnable;
	INT64 m_refAlloc = 0;
};

inline CameraController* getCameraController() {return CCameraControlApp::Instance()->_controller;}
inline CameraModel* getCameraModel() {return CCameraControlApp::Instance()->_model;}
