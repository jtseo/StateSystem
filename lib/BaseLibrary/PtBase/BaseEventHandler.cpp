//
//  BaseEventHandler.cpp
//  BaseLibrary
//
//  Created by jtseo on 2021/07/29.
//  Copyright © 2021 Jung Tae Seo. All rights reserved.
//

#include "stdafx.h"

#include "BaseDStructure.h"
#include "BaseState.h"
#include "BaseStateManager.h"
#include "BaseCircleQueue.h"
#include "BaseEventHandler.h"

BaseEventHandler *BaseEventHandler::ms_single = NULL;
BaseStateManagerLogic *BaseEventHandler::ms_stateManager_ap[PT_MAX_THREAD];

BaseEventHandler::BaseEventHandler()
{
	
}

BaseEventHandler::~BaseEventHandler()
{
	
}

bool BaseEventHandler::release(BaseStateManagerLogic *_manager_p)
{
	for(int i=0; i<PT_MAX_THREAD; i++){ // initialization
		if(ms_stateManager_ap[i] == _manager_p){
			ms_stateManager_ap[i] = NULL;
			return true;
		}
	}
	return false;
}

BaseEventHandler *BaseEventHandler::single_get(BaseStateManagerLogic *_manager_p)
{
	if(!ms_single){
		ms_single = new BaseEventHandler();
		for(int i=0; i<PT_MAX_THREAD; i++){ // initialization
			ms_stateManager_ap[i] = NULL;
		}
	}
	
	if(!_manager_p)
		return ms_single;
	
	int index = -1;
	int indexNew = -1;
	for(int i=0; i<PT_MAX_THREAD; i++){
		if(ms_stateManager_ap[i] == _manager_p){
			index = i;
			break;
		}
		if(ms_stateManager_ap[i] == NULL)
			indexNew = i;
	}
	if(index == -1)
	{
		if(indexNew == -1)
		{
			B_ASSERT(indexNew != -1);
			return NULL;
		}
		ms_stateManager_ap[indexNew] = _manager_p;
	}
	return ms_single;
}

void BaseEventHandler::event_push(BaseDStructureValue *_evt_pdsv, int _priority_n)
{
	_evt_pdsv->ref_inc();
	if (_evt_pdsv->ref_get() != 1)
	{
		int x = 0;
	}
	else {
		int x = 0;
	}
	for(int i=0; i<PT_MAX_THREAD; i++)
	{
		if(ms_stateManager_ap[i]){
			BaseDStructureValue *evt = NULL;
			PT_OAlloc2(evt, BaseDStructureValue, _evt_pdsv->get_base(), 1024);
			_evt_pdsv->variable_transit(evt);
			evt->set_key(_evt_pdsv->get_key());
			ms_stateManager_ap[i]->push_event(evt, _priority_n);
		}
	}
	_evt_pdsv->ref_dec();
}
