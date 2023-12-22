#pragma once
#include "BaseResResource.h"
#include "BaseFile.h"

class BaseResFile :
	public BaseResResource
{
public:
	BaseResFile();
	~BaseResFile();

	PtObjectHeader(BaseResFile);
	static int GetObjectId();

	virtual bool Load(); // Load In Thread
	virtual void Clear();

	int read(int _nPos, bbyte *_data, int _nSize);
protected:

	BaseFile * m_pFile;
};

