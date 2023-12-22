#pragma once

#include "ZlibEntry.h"

/**
 * \ingroup Zlib
 *
 *
 * \par requirements
 * win98 or later\n
 * win2k or later\n
 * MFC\n
 *
 * \version 1.0
 * first version
 *
 * \date 2005-07-20
 *
 * \author 이오나
 *
 * \par license
 * 
 * \todo 
 *
 * \bug 
 *
 */
class ZLIB_API	Z_FileInfo
{
public:
	int nFileCount;
	int nFolderCount;
	DWORD dwUncompressedSize;
};

/**
 * \ingroup Zlib
 *
 *
 * \par requirements
 * win98 or later\n
 * win2k or later\n
 * MFC\n
 *
 * \version 1.0
 * first version
 *
 * \date 2005-07-20
 *
 * \author 이오나
 *
 * \par license
 * 
 * \todo 
 *
 * \bug 
 *
 */
class ZLIB_API	CZipper  
{
public:
	/**
	 *
	 * \param szFilePath 
	 * \param szRootFolder 
	 * \param bAppend 
	 * \return 
	 */
	CZipper(LPCTSTR szFilePath = NULL, LPCTSTR szRootFolder = NULL, bool bAppend = FALSE);
	/**
	 *
	 * \return 
	 */
	virtual ~CZipper();

	// simple interface
	/**
	 *
	 * \param szFilePath 
	 * \return 
	 */
	static bool __stdcall	ZipFile(LPCTSTR szFilePath); // saves as same name with .zip
	/**
	 *
	 * \param szFilePath 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	static bool __stdcall	ZipFolder(LPCTSTR szFilePath, bool bIgnoreFilePath); // saves as same name with .zip

	/**
	 *
	 * \param szFilePath 
	 * \param szZipPath 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	static bool __stdcall	ZipFolder(
		LPCTSTR szFilePath, 
		LPCTSTR szZipPath,
		bool	bIgnoreFilePath ); // saves as same name with .zip
	
	// works with prior opened zip
	/**
	 *
	 * \param szFilePath 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	bool __stdcall	AddFileToZip(LPCTSTR szFilePath, bool bIgnoreFilePath = FALSE);
	/**
	 *
	 * \param szFilePath 
	 * \param szRelFolderPath 
	 * \return 
	 */
	bool __stdcall	AddFileToZip(LPCTSTR szFilePath, LPCTSTR szRelFolderPath); // replaces path info from szFilePath with szFolder
	/**
	 *
	 * \param szFolderPath 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	bool __stdcall	AddFolderToZip(LPCTSTR szFolderPath, bool bIgnoreFilePath = FALSE);

	// extended interface
	/**
	 *
	 * \param szFilePath 
	 * \param szRootFolder 
	 * \param bAppend 
	 * \return 
	 */
	bool __stdcall	OpenZip(LPCTSTR szFilePath, LPCTSTR szRootFolder = NULL, bool bAppend = FALSE);
	/**
	 *
	 * \return 
	 */
	bool __stdcall	CloseZip(); // for multiple reuse
	/**
	 *
	 * \param info 
	 */
	void __stdcall	GetFileInfo(Z_FileInfo& info);
	
protected:
	void* m_uzFile;
	char m_szRootFolder[MAX_PATH + 1];
	Z_FileInfo m_info;

protected:
	/**
	 *
	 * \param szPath 
	 */
	void __stdcall	PrepareSourcePath(LPTSTR szPath);
};