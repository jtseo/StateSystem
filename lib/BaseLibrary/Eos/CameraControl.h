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

#ifndef VScriptBuild

#include "Camera/CameraController.h"
#include "Camera/CameraModel.h"

class BaseCircleQueue;

class CCameraControl : public Observer
{
public:
	CCameraControl();
	~CCameraControl();

// Overrides
public:
	virtual BOOL InitInstance();
	void UnInitInstance();

	CameraModel*		_model;
	CameraController*	_controller;

	friend CameraController* getController();
	friend CameraModel* getModel();
	void StreamFree();

	static CCameraControl* Instance();
	static void Reset();
	void EventCastEnable();
	void PreviewRequest();
private:
	static CCameraControl* ms_instance;
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
	unsigned int m_afmode;
	int m_pictureSize[2];

	int m_currentSlot;
	STLString m_layoutPath;
	STLVVec2 m_picturePositions;
	Mat m_layoutMat;
public:
	void PictureSizeSet(int w, int h);
	bool PreviewLayoutSet(const char* _filepath, const STLVVec2& _positions);
	void PreviewSlotSet(int _slot)
	{
		m_currentSlot = _slot;
	}

private:
	int m_filterSkin;
public:
	void FilterSkinOn_nF(int _mode);
	void AFocusOn_nF();
	void AFocusOff_nF();
};

inline CameraController* getCameraController() {return CCameraControl::Instance()->_controller;}
inline CameraModel* getCameraModel() {return CCameraControl::Instance()->_model;}
#endif
