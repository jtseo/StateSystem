#pragma once
#include "BaseResResource.h"

class BaseResFilterIP :
	public BaseResResource
{
public:
	BaseResFilterIP(void);
	virtual ~BaseResFilterIP(void);
public:
	PtObjectHeader(BaseResFilterIP);
	static int GetObjectId();

	virtual bool Load();
	virtual bool Load2();
	virtual void Clear();

	bool check_in_ip(const char *_strIP);// IP is in list, true

protected:
	bool	m_bReturnLoad1;
	void	*m_pHttpFilter;
};
