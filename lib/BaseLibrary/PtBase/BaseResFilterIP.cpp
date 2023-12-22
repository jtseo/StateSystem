#include "stdafx.h"
#include "BaseResFilterIP.h"
#include "BaseHttp.h"


PtObjectCpp(BaseResFilterIP);

BaseResFilterIP::BaseResFilterIP(void)
{
	m_nSerialNumber	= serial_create_();	// Unique number for each objects
	m_nObjectId = GetObjectId();
	m_bReturnLoad1	= false;

	m_pHttpFilter = NULL;
}

int BaseResFilterIP::GetObjectId()
{
	static int	s_iId = 0;
	if(s_iId == 0)
		s_iId	= UniqHashStr::get_string_hash_code("BaseResFilterIP");

	return s_iId;
}

BaseResFilterIP::~BaseResFilterIP(void)
{
	delete ((BaseHttp*)m_pHttpFilter);
	m_pHttpFilter	= NULL;
}

bool BaseResFilterIP::Load()
{	
	m_pHttpFilter	= new BaseHttp;

	char strBuffer[255];
	BaseSystem::path_root_get(strBuffer, 255, m_strName.c_str());
	if(!((BaseHttp*)m_pHttpFilter)->filter_load(strBuffer))
	{
		char strBuf[255];
		char *strTemp;
		strcpy_s(strBuf, 255, m_strName.c_str());
		strTemp	= strrchr(strBuf, '.');

		if(strTemp == NULL)
			return false;

		strcpy(strTemp, ".txt");
		((BaseHttp*)m_pHttpFilter)->filter_make(strBuf, m_strName.c_str());

		if(!((BaseHttp*)m_pHttpFilter)->filter_load(strBuffer))
			return false;
	}

	m_bReturnLoad1	= true;
	return true;
}

bool BaseResFilterIP::Load2()
{
	return true;
}

void BaseResFilterIP::Clear()
{

}

bool BaseResFilterIP::check_in_ip(const char *_strIP)
{
	int	anIP[4];

	sscanf_s(_strIP, "%d.%d.%d.%d", &anIP[0], &anIP[1], &anIP[2], &anIP[3]);
	unsigned char chIP[4];

	for(int i=0; i<4; i++)
		chIP[i] = (char)anIP[i];

	if(m_pHttpFilter
		&& ((BaseHttp*)m_pHttpFilter)->filter_check(chIP))
		return true;

	return false;
}
