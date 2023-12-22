#pragma once

#include "ZlibEntry.h"

const UINT MAX_COMMENT = 255;

// create our own fileinfo struct to hide the underlying implementation
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
class ZLIB_API	UZ_FileInfo
{
public:
	char szFileName[MAX_PATH + 1];
	char szComment[MAX_COMMENT + 1];
	
	DWORD dwVersion;  
	DWORD dwVersionNeeded;
	DWORD dwFlags;	 
	DWORD dwCompressionMethod; 
	DWORD dwDosDate;	
	DWORD dwCRC;   
	DWORD dwCompressedSize; 
	DWORD dwUncompressedSize;
	DWORD dwInternalAttrib; 
	DWORD dwExternalAttrib; 
	bool bFolder;
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
class ZLIB_API	CUnzipper  
{
public:
	/**
	 *
	 * \param szFilePath 
	 * \return 
	 */
	CUnzipper(LPCTSTR szFilePath = NULL);
	/**
	 *
	 * \return 
	 */
	virtual ~CUnzipper();
	
	// simple interface
	/**
	 *
	 * \param szFileName 
	 * \param szFolder 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	static bool __stdcall	Unzip(LPCTSTR szFileName, LPCTSTR szFolder = NULL, bool bIgnoreFilePath = FALSE);
	
	// works with prior opened zip
	/**
	 *
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	bool __stdcall	Unzip(bool bIgnoreFilePath = FALSE); // unzips to output folder or sub folder with zip name 
	/**
	 *
	 * \param szFolder 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	bool __stdcall	UnzipTo(LPCTSTR szFolder, bool bIgnoreFilePath = FALSE); // unzips to specified folder

	// extended interface
	/**
	 *
	 * \param szFilePath 
	 * \return 
	 */
	bool __stdcall	OpenZip(LPCTSTR szFilePath);
	/**
	 *
	 * \return 
	 */
	bool __stdcall	CloseZip(); // for multiple reuse
	/**
	 *
	 * \param szFolder 
	 * \return 
	 */
	bool __stdcall	SetOutputFolder(LPCTSTR szFolder); // will try to create
	
	// unzip by file index
	/**
	 *
	 * \return 
	 */
	int __stdcall	GetFileCount();
	/**
	 *
	 * \param nFile 
	 * \param info 
	 * \return 
	 */
	bool __stdcall	GetFileInfo(int nFile, UZ_FileInfo& info);
	/**
	 *
	 * \param nFile 
	 * \param szFolder 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	bool __stdcall	UnzipFile(int nFile, LPCTSTR szFolder = NULL, bool bIgnoreFilePath = FALSE);
	
	// unzip current file
	/**
	 *
	 * \param szExt 
	 * \return 
	 */
	bool __stdcall	GotoFirstFile(LPCTSTR szExt = NULL);
	/**
	 *
	 * \param szExt 
	 * \return 
	 */
	bool __stdcall	GotoNextFile(LPCTSTR szExt = NULL);
	/**
	 *
	 * \param info 
	 * \return 
	 */
	bool __stdcall	GetFileInfo(UZ_FileInfo& info);
	/**
	 *
	 * \param szFolder 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	bool __stdcall	UnzipFile(LPCTSTR szFolder = NULL, bool bIgnoreFilePath = FALSE);

	// helpers
	/**
	 *
	 * \param szFileName 
	 * \param bIgnoreFilePath 
	 * \return 
	 */
	bool __stdcall	GotoFile(LPCTSTR szFileName, bool bIgnoreFilePath = TRUE);
	/**
	 *
	 * \param nFile 
	 * \return 
	 */
	bool __stdcall	GotoFile(int nFile);
	
protected:
	void* m_uzFile;
	char m_szOutputFolder[MAX_PATH + 1];

protected:
	/**
	 *
	 * \param szFolder 
	 * \return 
	 */
	static bool __stdcall	CreateFolder(LPCTSTR szFolder);
	/**
	 *
	 * \param szFilePath 
	 * \return 
	 */
	static bool __stdcall	CreateFilePath(LPCTSTR szFilePath); // truncates from the last '\'
	/**
	 *
	 * \param szFilePath 
	 * \param dwDosDate 
	 * \return 
	 */
	static bool __stdcall	SetFileModTime(LPCTSTR szFilePath, DWORD dwDosDate);
};