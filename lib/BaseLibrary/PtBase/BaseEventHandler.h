//
//  BaseEventHandler.h
//  BaseLibrary
//
//  Created by jtseo on 2021/07/29.
//  Copyright © 2021 Jung Tae Seo. All rights reserved.
//

#ifndef BaseEventHandler_h
#define BaseEventHandler_h

//#include "BaseEventManager.h"

#define PT_MAX_THREAD	10000

class BaseStateManagerLogic;
class BaseCircleQueue;

class BaseEventHandler
{
public:
	BaseEventHandler();
	~BaseEventHandler();
	
	static BaseEventHandler *single_get(BaseStateManagerLogic *_manager_p);
	static bool release(BaseStateManagerLogic *_manager_p);
	
	static void event_push(BaseDStructureValue *_evt_pdsv, int _priority_n);
protected:
	static BaseEventHandler	*ms_single;
	static BaseStateManagerLogic	*ms_stateManager_ap[PT_MAX_THREAD];
};

#endif /* BaseEventHandler_h */
