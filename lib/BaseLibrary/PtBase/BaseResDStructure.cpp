#include "stdafx.h"
#include "BaseResDStructure.h"

BaseResDStructure BaseResDStructure::sm_sample;

BaseResDStructure::BaseResDStructure()
{
	m_nSerialNumber	= serial_create_();	// Unique number for each objects
	m_nObjectId		= OBJECTID_DSTRUCTURE;
}

BaseResDStructure::~BaseResDStructure()
{

}

bool BaseResDStructure::Load()
{
	const char *strFilename = GetFileName();

	return m_dst.load(strFilename);
}

void BaseResDStructure::Clear()
{

}

BaseDStructure *BaseResDStructure::get_dst()
{
	return &m_dst;
}