#pragma once

#include "BaseResResource.h"
#include "BaseDStructure.h"

class BaseResDStructure : public BaseResResource
{
public:
	BaseResDStructure();
	~BaseResDStructure();
	virtual bool Load(); // Load In Thread
	virtual void Clear();

	BaseDStructure *get_dst();


	static BaseResDStructure	sm_sample;
protected:

	BaseDStructure	m_dst;
};