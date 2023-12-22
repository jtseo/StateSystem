#include "stdafx.h"
#include "BaseResFile.h"

BaseResFile BaseResFile::sm_sample;

BaseResFile::~BaseResFile()
{
	if(m_pFile)
		PT_OFree(m_pFile);
	m_pFile = NULL;
}

BaseResFile::BaseResFile()
{
	m_pFile = NULL;
	m_nSerialNumber = serial_create_();	// Unique number for each objects
	m_nObjectId = GetObjectId();
}

int BaseResFile::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = UniqHashStr::get_string_hash_code("BaseResFile");

	return s_iId;
}

bool BaseResFile::Load()
{
	const char *strFilename = GetFileName();

	BaseFile *pFile = NULL;
	PT_OAlloc(pFile, BaseFile);

	if (pFile->OpenFile(strFilename, BaseFile::OPEN_READ) != 0) {
		PT_OFree(pFile);
		return false;
	}

	m_pFile = pFile;

	return true;
}

void BaseResFile::Clear()
{
}

int BaseResFile::read(int _nPos, bbyte *_data, int _nSize) {
	m_pFile->Seek(_nPos);
	return m_pFile->Read(_data, _nSize);
}