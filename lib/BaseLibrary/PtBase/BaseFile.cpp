
#include "stdafx.h"
#include "MD52.h"
//#include "seedx.h"
#include "BaseFile.h"

#define _SIMSVR

#define FILEBLOCKLENGTH	16
#define FILEBLOCK_MAX	1024

BaseFile	BaseFile::sm_sample;
unsigned	BaseFile::sm_nAccessCount	= 0;

#include "EncoderOperator.h"

PT_OPTCPP(BaseFile)

void BaseFile::init(EncoderOperator *_pEncoder)
{
	m_pEncoder = _pEncoder;
		//STLVCoderIndex	stlVCoder;
		//char strBuf[255];
		//m_pEncoder->set_encoder(BaseSystem::get_root_path(strBuf, 255, "encoder.cod"), stlVCoder);

	m_nSizeLine = 128;
	m_strBufferAscLine = PT_Alloc(char, m_nSizeLine);
	m_strBufferAscBlock = PT_Alloc(char, ASC_MAX_LENGTH);
	/*
	m_strToken.reserve(100);
	m_strTag.reserve(100);
	m_strSeperator =  ",\t";
	m_strDelete = "";
	m_strReplace = "";
	m_strCarriageReturn = "\r\n";
	m_strTag	= "";
	//*/
	m_mpercent_ap = NULL;
	m_strToken = new STLString;
	m_strTag = new STLString;
	m_stlQStack = new std::deque<std::string, PT_allocator<std::string> >;
	m_stlMnFileBlock = new STLMnFileBlock;

	m_pMemoryOut	= NULL;
	m_pFileBlockCurrent	= NULL;
	init_();
	m_cComment = 0;//'#';
	m_sign = 0;

	memcpy(strKey_, ((void*)"0394uto#$^$^(uteo#@$[049%^L:r0pw"), 32);

	m_bFileCreated	= false;

	m_bFirstWritingSign = true;
	m_bFirstReadingSign = true;
}

void BaseFile::release()
{
	CloseFile();

	if(m_strBufferAscLine)
		PT_Free(m_strBufferAscLine);
	if(m_strBufferAscBlock)
		PT_Free(m_strBufferAscBlock);

	if (m_mpercent_ap)
		delete m_mpercent_ap;
	m_mpercent_ap = NULL;
	if (m_stlMnFileBlock)
		delete m_stlMnFileBlock;
	m_stlMnFileBlock = NULL;
	if (m_strTag)
		delete m_strTag;
	m_strTag = NULL;
	if (m_strToken)
		delete m_strToken;
	m_strToken = NULL;
	if (m_stlQStack)
		delete m_stlQStack;
	m_stlQStack = NULL;

	m_pEncoder	= NULL;
}

BaseFile::BaseFile(EncoderOperator *_pEncoder)
{
	m_option = 0;
	init(_pEncoder);
}

void BaseFile::set_md5(CMD5 *_pMd5, md5_state_s *_pMd5State)
{
	pmd5Class_ = _pMd5;
	pmd5State_ = _pMd5State;
}

void BaseFile::set_comment(char _cComment)
{
	m_cComment	= _cComment;
}

int	BaseFile::get_size_block()
{
	return m_nSizeBlock;
}

void BaseFile::init_()
{
	strcpy_s(m_strSeperator, STD_STR_MIN, ",\t");
	m_strDelete[0] = NULL;
	m_strReplace[0] = NULL;
	strcpy_s(m_strCarriageReturn, STD_STR_MIN, "\r\n");

	m_pfFile		= NULL;

	m_nType			= OPEN_NULL;		/// file type (read, write, memory, file)
	m_nSizeBlock	= 0;	/// current block size
	m_pbBuffer		= NULL;	/// current buffer: ���� ���Ϸ� ���� �ε��� ����
	m_pbBufferCurrent	= NULL;	/// loading point: �ܺη� ���� �ε� �� �� ���� �����Ѵ�.

	m_nSizeBuffer	= 0;
	m_nSizeBlock	= 1 << FILEBLOCKLENGTH;
	m_bEndOfFile	= false;
	m_nSizeOfFile	= 0;

	pmd5Class_ = NULL;
	pmd5State_ = NULL;

	m_bDelMemoryFile = false;
	m_strFilename = NULL;

	if (m_strBufferAscLine)
		m_strBufferAscLine[0] = 0;
	if (m_strBufferAscBlock)
		m_strBufferAscBlock[0] = 0;

	m_nCurCharAscBlock = ASC_MAX_LENGTH;
	m_nCurCharAscLine = 0;

	m_nSizeAscBlock = 0;
	m_bDeleteBuffer = false;
	m_bExeFile	= false;
	m_bFirstWritingSign = true;
	m_bFirstReadingSign = true;
}

bool BaseFile::is_exe()
{
	return m_bExeFile;
}

void BaseFile::set_size_file(UINT32 nSize)
{
	m_nSizeOfFile = (unsigned)nSize;
}

UINT32 BaseFile::get_size_file()
{
	return m_nSizeOfFile;
}

int	BaseFile::OpenFile(void *_pData, UINT32 _nSize)
{
#ifdef _DEBUG
	if (_nSize == 256)
	{
		int x = 0;
	}
#endif

	char *strFile = NULL;
	return OpenFile(strFile, OPEN_MEMORY, _pData, _nSize);
}

int	BaseFile::openParser(char* _pData, UINT32 _nSize)
{
	char* strFile = NULL;
	return OpenFile(strFile, OPEN_MEMORY, _pData, _nSize);
}

bool BaseFile::is_exist(const char *_pFilename)
{
	BaseFile	file;
	if(file.OpenFile(_pFilename, OPEN_READ))
		return false;
	file.CloseFile();

	return true;
}

int	BaseFile::OpenFile(const char *_pFileName, int _nType, void *_pData, UINT32 _nSize)
{
	char strBuffer[255];
	strcpy_s(strBuffer, 255, _pFileName);
	return OpenFile(strBuffer, _nType, _pData, _nSize);
}

STLString BaseFile::remove(const STLString &_str, char _c)
{
	STLString ret;
	for(char ch: _str)
	{
		if(ch != _c)
			ret += ch;
	}
	return ret;
}

int BaseFile::get_file_type()
{
	return m_nType;
}

int BaseFile::SaveFile(const char *_strFilename)
{
	BaseFile file;
	int nRet = 0;
	if(!m_pbMemoryFile)
		return 1;

	nRet	= file.OpenFile(_strFilename, OPEN_WRITE);
	if(nRet != MSG_SUCCESS)
		return nRet;

	return file.Write(m_pbMemoryFile, m_nSizeBuffer);
}

// if work well, return 0(MSG_SUCCESS)
int	BaseFile::OpenFile(char *_pFileName, int _nType, void *_pData, UINT32 _nSize)
{

#ifndef _SIMSVR
	PXPackFile	packfile;
	char *pBuffer = NULL;
	if (packfile.LoadPack(PX::cstring("koongpa.kpp"))
		&& (_nType&OPEN_MEMORY) == 0
		&& (_nType&OPEN_NON_PACKAGE) == 0
		&& (_nType&OPEN_READ) != 0)
	{
		const char *strFilename;
		const char *strFind;
		UINT32 nSize=0;

		strFilename = _pFileName;
		while((strFind = strchr(strFilename, SLASH_C)) != NULL)
			strFilename = strFind+1;

		kpp_file_info*	pinfo	=	packfile.FindFile(strFilename);

		if (NULL != pinfo)
		{
			char* pStart	=	reinterpret_cast<char*>(packfile.DumpFile(pinfo));
			if (NULL != pStart)
			{
				nSize = pinfo->uncompressed_size;
				//pBuffer = new char[nSize+1];
				pBuffer	= PT_Alloc(char, nSize+1);
				memcpy(pBuffer, pStart, nSize);
			}
		}

		packfile.Clear();

		if(pBuffer)
		{
			_pFileName = pBuffer;
			m_bDeleteBuffer = true;
			_nType |= OPEN_MEMORY;
			_nSize = nSize;
		}
	}
#endif
	m_bFileCreated	= false;
	m_nType	= _nType;
	m_nBraceCnt = 0;
	m_cBrace[0] = '{';
	m_cBrace[1] = '}';
	m_cReturn[0] = '\r';
	m_cReturn[1] = '\n';
	m_stlQStack->clear();
	m_bFirstTokenLine	= true;

	if(!(_nType & OPEN_MEMORY))
	{
		strcpy_s(m_dbg_filename, 1024, _pFileName);
		char tempName[1024];
		strcpy_s(tempName, 1024, _pFileName);
		BaseSystem::path_fix(tempName, 1024); // path fix for unix system

		if(_nType & OPEN_WRITE)
		{
			m_pfFile	= NULL;
			if(_nType & OPEN_RANDOM)
			{
				if(!fopen_s(((FILE**)&m_pfFile),tempName, "rb"))
				{
					fclose((FILE*)m_pfFile);
					if(fopen_s(((FILE**)&m_pfFile),tempName, "r+b"))
                        m_pfFile = NULL;
				}
			}
			if(m_pfFile == NULL)
			{
				if(fopen_s(((FILE**)&m_pfFile), tempName, "wb"))
                    return MSG_ERRORTOOPEN;
				m_bFileCreated	= true;
				m_bFirstWritingSign = true;
			}

			if(m_pfFile == 0)
				return MSG_ERRORTOOPEN;

			if((_nType & OPEN_RANDOM)==0)
			{
				m_pbBuffer	= PT_Alloc(char, m_nSizeBlock+1);
				m_pbBufferCurrent	= (unsigned char*)m_pbBuffer;
				m_nSizeBuffer		= 0;
			}else{
				fseek((FILE*)m_pfFile, 0, SEEK_END);
				/// ���� ũ�⸦ ��������.
				m_nSizeOfFile = (unsigned)ftell( (FILE*)m_pfFile );
				/// Ŀ���� �ٽ� �� ������ �����´�.
				fseek( (FILE*)m_pfFile, 0, SEEK_SET );

				get_fileblock_(m_nSizeOfFile);
			}
		}else{
            if(fopen_s(((FILE**)&m_pfFile),tempName, "rb"))
                return MSG_ERRORTOOPEN;
            if(m_pfFile == NULL)
                return MSG_ERRORTOOPEN;

			fseek((FILE*)m_pfFile, 0, SEEK_END);
			/// ���� ũ�⸦ ��������.
			m_nSizeOfFile = (unsigned)ftell( (FILE*)m_pfFile );
			/// Ŀ���� �ٽ� �� ������ �����´�.
			fseek( (FILE*)m_pfFile, 0, SEEK_SET );

			if(_nType & ENCODE_001)
			{
				m_nSizeBlock = m_nSizeOfFile;	
			}
			if((_nType & OPEN_RANDOM)==0)
				m_pbBuffer	= PT_Alloc(char, m_nSizeBlock+1);
			m_pbBufferCurrent	= (unsigned char*)m_pbBuffer;
			m_nSizeBuffer		= 0;
			m_bFirstReadingSign = true;

			if(ReadNextBlock() == 0)
			{
				CloseFile();
				return MSG_ENDOFFILE;
			}

			exe_check();

			//if(_nType & ENCODE_001)
			//{
			//	unsigned nResult;
			//	//_Crypto((unsigned char*)m_pbBuffer, m_nSizeBuffer, nResult, true);
			//	nResult = (unsigned)seed_decrypt_((unsigned char*)m_pbBuffer, m_nSizeBuffer, strKey_);
			//	m_nSizeBuffer = nResult;
			//}
		}

		if(!m_pfFile)
			return MSG_ERRORTOOPEN;
		else
			return MSG_SUCCESS;
	}else{
		if(_pFileName == NULL && _pData == NULL)
			return MSG_ERRORTOOPEN;

		if((_nType & OPEN_WRITE))
		{
			if(_pFileName)
			{
				size_t nSize = strlen(_pFileName);
				m_strFilename = PT_Alloc(char, nSize+1);//NEW_A(char, nSize+1);
				strcpy_s(m_strFilename, nSize+1, _pFileName);
			}

			if((_nType & MEM_OUTDATA))
			{
				m_pbMemoryFile	= (char*)_pData;
				m_nSizeBuffer	= 0;
				m_pbBuffer		= m_pbMemoryFile;
				m_pbBufferCurrent	= (unsigned char*)m_pbMemoryFile;
				m_nSizeOfFile	= (unsigned)_nSize;
			}else{
				m_pbBuffer			= NULL;
				m_pbBufferCurrent	= NULL;
				m_nSizeBuffer		= 0;

				allocate_memoryfile();
			}
		}else{
			if(_pFileName)
				m_pbBuffer	= _pFileName;
			else
				m_pbBuffer	=  (char*)_pData;

			m_pbBufferCurrent	= (unsigned char*)m_pbBuffer;
			m_nSizeBuffer		= (unsigned)_nSize;

			//if(_nType & ENCODE_001)
			//{
			//	unsigned nResult;
			//	//_Crypto((unsigned char*)m_pbBuffer, m_nSizeBuffer, nResult, true);
			//	nResult = (unsigned)seed_decrypt_((unsigned char*)m_pbBuffer, m_nSizeBuffer, strKey_);
			//	m_nSizeBuffer = nResult;
			//}
		}
	}

	if(m_pbBuffer)
		return MSG_SUCCESS;

#ifdef _DEBUG
#ifdef _SIMSVR
	char strBuffer[MAX_PATH];
	if((_nType & OPEN_MEMORY))
		sprintf_s(strBuffer, MAX_PATH, "Fail to load in memory in BaseFile.cpp");
	else
		sprintf_s(strBuffer, MAX_PATH, "Fail to load a file '%s' in BaseFile.cpp", _pFileName);

    g_SendMessage(LOG_MSG_POPUP, strBuffer);
#endif
#endif
	return MSG_ERRORTOOPEN;
}

void BaseFile::exe_check()
{
	if(m_pbBuffer)
	{
		char bufExe[4] = { 77, 90, (char)-112, 0 };
		if(memcmp(m_pbBuffer, bufExe, 4) == 0)
		{
			m_bExeFile = true;
			return;
		}	
	}
	m_bExeFile = false;
}

int BaseFile::CloseFile()
{
	if(m_pfFile)
	{
		if(m_nType & OPEN_RANDOM)
		{
			STLMnFileBlock::iterator	it;
			it	= m_stlMnFileBlock->begin();
			for(;it!=m_stlMnFileBlock->end();it++)
			{
				write_fileblock_(&it->second);
			}
			m_stlMnFileBlock->clear();
		}else if(m_nSizeBuffer > 0 && (m_nType & OPEN_WRITE))
		{
			WriteCurrnetBlock();
		}

		fclose((FILE*)m_pfFile);
		m_pfFile	= NULL;

		if(m_pbBuffer && (m_nType & OPEN_RANDOM)==0)
		{
			PT_Free(m_pbBuffer);//DEL_A(m_pbBuffer);
			m_pbBuffer	= NULL;
		}
	}else{
		if(m_strFilename) // OPEN_MEMORY, OPEN_WRITE
		{
			fopen_s(((FILE**)&m_pfFile), m_strFilename, "wb");
			m_bFileCreated	= true;

			if(m_pfFile != 0)
			{
				//unsigned nResult;
				
				//if(m_nType & ENCODE_001)
				//{
				//	if((m_nSizeOfFile % 16) != 0)
				//		allocate_memoryfile(16);

				//	//_Crypto((unsigned char*)m_pbMemoryFile, m_nSizeBuffer, nResult, false);
				//	nResult = (unsigned)seed_encrypt_((unsigned char*)m_pbMemoryFile, m_nSizeBuffer, strKey_);
				//	m_nSizeBuffer = nResult;
				//}

				fwrite_(m_pbMemoryFile, 1, m_nSizeBuffer, (FILE*)m_pfFile);
				m_nSizeBuffer		= 0;

				fclose((FILE*)m_pfFile);
			}else{
				delete m_strFilename;
				if(m_bDelMemoryFile)
					PT_Free(m_pbMemoryFile);//DEL_A(m_pbMemoryFile);

				init_();
				return MSG_ERRORTOOPEN;
			}

			delete m_strFilename;
		}else{
			// *HanMK - �޸� �����͸� ��ȣȭ �ϱ� ���� �߰���.
			//if(m_nType & OPEN_WRITE && m_nType & MEM_OUTDATA && m_nType & ENCODE_001)
			//{
			//	if((m_nSizeOfFile % 16) != 0)
			//		allocate_memoryfile(16);

			//	//_Crypto((unsigned char*)m_pbMemoryFile, m_nSizeBuffer, nResult, false);
			//	unsigned nResult;
			//	nResult = (unsigned)seed_encrypt_((unsigned char*)m_pbMemoryFile, m_nSizeBuffer, strKey_);
			//	m_nSizeBuffer = nResult;
			//}
		}

		if(m_bDelMemoryFile)
			PT_Free(m_pbMemoryFile);//DEL_A(m_pbMemoryFile);

		if(m_bDeleteBuffer)
			PT_Free(m_pbBuffer);//DEL_A(m_pbBuffer);
	}

	if(m_pMemoryOut)
	{
		PT_Free(m_pMemoryOut);
		m_pMemoryOut	= NULL;
	}

	int nSizeBuf = m_nSizeBuffer;
	bool bOutMem = (m_nType & MEM_OUTDATA) != 0;
	init_();

	if (bOutMem)
		m_nSizeBuffer = nSizeBuf;
	return MSG_SUCCESS;
}

void *BaseFile::get_memory(size_t _nSize)
{
	m_pMemoryOut	= PT_Alloc(bbyte, _nSize);
	return m_pMemoryOut;
}

int	BaseFile::Read(STLString* _buffer)
{
	_buffer->resize(get_size_file());
	return read_dump_(&_buffer->front(), get_size_file());
}

int	BaseFile::Read(void *_pReadBuffer, int _nSize)
{
	if(m_nType & OPEN_ASC)
		return read_x_dump_((char*)_pReadBuffer, _nSize);
	else
		return read_dump_(_pReadBuffer, _nSize);
}

char *BaseFile::get_point(int _nSizeSkip)
{
	char *pRetChar	= NULL;
	unsigned	nSize, nSizeLeave;

	nSizeLeave	= nSize	= 0;
	if(m_nType & OPEN_MEMORY)
	{
		if((unsigned)(m_pbBufferCurrent - (unsigned char*)m_pbBuffer) + (unsigned)_nSizeSkip > (unsigned)m_nSizeBuffer)
			_nSizeSkip = m_nSizeBuffer - (unsigned)(m_pbBufferCurrent - (unsigned char*)m_pbBuffer);

		pRetChar	= (char*)m_pbBufferCurrent;
		m_pbBufferCurrent	+= _nSizeSkip;
		nSize	= _nSizeSkip;

		if(pmd5Class_ && pmd5State_)
			pmd5Class_->append(pmd5State_, (const md5_byte_t*)pRetChar, _nSizeSkip);
	}
	if(nSize == 0)
		return NULL;

	return pRetChar;
}

int BaseFile::read_dump_(void *_pDump, int _nSize)
{
	unsigned	nSize, nSizeLeave;

	nSizeLeave	= nSize	= 0;

	if (m_bFirstReadingSign == true) {

		if (m_pbBufferCurrent[0] == 0xEF &&
			m_pbBufferCurrent[1] == 0xBB &&
			m_pbBufferCurrent[2] == 0xBF)
		{
			m_pbBufferCurrent += 3;
			m_nSizeBuffer -= 3;
			m_nType |= OPEN_UTF8;
		}

		m_bFirstReadingSign = false;
	}

	if(m_nType & OPEN_MEMORY)
	{
		//B_ASSERT(m_pbBufferCurrent);
		if((unsigned)(m_pbBufferCurrent - (unsigned char*)m_pbBuffer) + (unsigned)_nSize > (unsigned)m_nSizeBuffer)
			_nSize = m_nSizeBuffer - (unsigned)(m_pbBufferCurrent - (unsigned char*)m_pbBuffer);

		memcpy(_pDump, m_pbBufferCurrent, _nSize);
		m_pbBufferCurrent	+= _nSize;
		nSize	= _nSize;

		if(pmd5Class_ && pmd5State_)
			pmd5Class_->append(pmd5State_, (const md5_byte_t*)_pDump, _nSize);
	}else{
		//B_ASSERT(m_pfFile);
		nSizeLeave	= _nSize;

		UINT32	nSizeBlockBack;
		nSizeBlockBack	= m_nSizeBlock - (UINT32)(m_pbBufferCurrent - (unsigned char*)m_pbBuffer);

		while(nSizeLeave >= nSizeBlockBack) // �о���� ������ �� ������ �Ѿ�� ������ ������ �ε��Ѵ�.
		{
			if(nSizeLeave > (unsigned)m_nSizeBuffer)
				nSizeBlockBack	= m_nSizeBuffer; // �о���� ������ �����ϴ�

			memcpy(((char*)_pDump)+nSize, m_pbBufferCurrent, nSizeBlockBack);
			nSizeLeave	-= nSizeBlockBack;
			m_pbBufferCurrent	+= nSizeBlockBack;
			nSize				+= nSizeBlockBack;
			ReadNextBlock();

			nSizeBlockBack	= m_nSizeBuffer;
			if(m_nSizeBuffer == 0)
				break;
		}

		if(nSizeLeave > 0)
		{
			if (nSizeLeave > (unsigned)m_nSizeBuffer)
				nSizeLeave	= m_nSizeBuffer; // �о���� ������ �����ϴ�
			memcpy(((char*)_pDump)+nSize, m_pbBufferCurrent, nSizeLeave);
			m_pbBufferCurrent	+= nSizeLeave;
			nSize	+= nSizeLeave;
			m_nSizeBuffer		-= nSizeLeave;
		}
	}
	
	return nSize;
}

int BaseFile::write_dump_(const void *_pDump, int _nSize)
{
	int	nSize, nSizeLeave;

	nSizeLeave	= 0;
	nSize		= 0;

	if(m_nType & OPEN_MEMORY)
	{
		if(m_nSizeBuffer+_nSize > (int)m_nSizeOfFile && (m_nType & MEM_OUTDATA))
			return 0;

		while(m_nSizeBuffer+_nSize > (int)m_nSizeOfFile && (m_nType & MEM_OUTDATA) == 0)
		{
			if(!allocate_memoryfile())
				return 0;
		}

		memcpy(m_pbBufferCurrent, _pDump, _nSize);
		m_pbBufferCurrent	+= _nSize;
		nSize	= _nSize;
		m_nSizeBuffer	+= nSize;
	}else{
		nSize = write_in_file(_pDump, _nSize);
	}

	return nSize;
}

int	BaseFile::Write(const void *_pSaveBuffer, int _nSize)
{
	int nRet = 0;
	if(m_nType & OPEN_ASC)
		nRet = write_x_dump_(_pSaveBuffer, _nSize);
	else
		nRet = write_dump_(_pSaveBuffer, _nSize);

	if(m_nSizeOfFile == 0)
		exe_check();

	m_nSizeOfFile += _nSize;

	return nRet;
}

int BaseFile::write_in_file(const void *_pSaveBuffer, int _nSize)
{	
	if (m_bFirstWritingSign)
	{
		if (m_nType & OPEN_UTF8 && *((unsigned char*)_pSaveBuffer) != 0xef){
			*m_pbBufferCurrent = 0xef;	m_pbBufferCurrent++;
			*m_pbBufferCurrent = 0xbb;	m_pbBufferCurrent++;
			*m_pbBufferCurrent = 0xbf;	m_pbBufferCurrent++;
			m_nSizeBuffer += 3;
		}
		else if (m_nType & OPEN_UCS2L && *((unsigned char*)_pSaveBuffer) != 0xff){
			*m_pbBufferCurrent = 0xff;	m_pbBufferCurrent++;
			*m_pbBufferCurrent = 0xfe;	m_pbBufferCurrent++;
			m_nSizeBuffer += 2;
		}
		m_bFirstWritingSign = false;
	}

	unsigned nSizeLeave = 0;
	unsigned nSize = 0;

	UINT32	nSizeBlockBack;
	nSizeBlockBack	= m_nSizeBlock - (UINT32)(m_pbBufferCurrent - (unsigned char *)m_pbBuffer);

	nSizeLeave	= _nSize;
	while(nSizeLeave >= nSizeBlockBack)
	{
		memcpy(m_pbBufferCurrent, ((const char*)_pSaveBuffer)+nSize, nSizeBlockBack);

		nSize	+= nSizeBlockBack;
		m_nSizeBuffer	+= nSizeBlockBack;
		nSizeLeave	= _nSize - nSize;

		if(m_nType & OPEN_RANDOM)
		{
			m_pFileBlockCurrent->bDirty	= true;
			if((int)m_pFileBlockCurrent->nSizeBuffer < m_nSizeBuffer)
				m_pFileBlockCurrent->nSizeBuffer = m_nSizeBuffer;
			UINT32 nIndex	= m_pFileBlockCurrent->nIndex+1;
			get_fileblock_(nIndex*m_nSizeBlock);
		}else{
			if(WriteCurrnetBlock() == 0)
				break; // ���� ������ �߻��ߴ�.
		}
		nSizeBlockBack	= m_nSizeBlock;
	}
	if(nSizeLeave > 0)
	{
		memcpy(m_pbBufferCurrent, ((const char*)_pSaveBuffer)+nSize, nSizeLeave);
		nSize	+= nSizeLeave;
		m_pbBufferCurrent	+= nSizeLeave;
		m_nSizeBuffer	= (unsigned)(m_pbBufferCurrent - (unsigned char*)m_pbBuffer);
		
		if(m_nType & OPEN_RANDOM)
		{
			m_pFileBlockCurrent->bDirty	= true;
			if((int)m_pFileBlockCurrent->nSizeBuffer < m_nSizeBuffer)
				m_pFileBlockCurrent->nSizeBuffer = m_nSizeBuffer;
		}
	}
	return nSize;
}

unsigned	BaseFile::ReadNextBlock()
{
	if((m_nType & OPEN_RANDOM)!=0)
	{
		UINT32 nIndex	= 0;
		if(m_pFileBlockCurrent)
			nIndex	= m_pFileBlockCurrent->nIndex + 1;

		get_fileblock_(nIndex*m_nSizeBlock);
		return m_pFileBlockCurrent->nSizeBuffer;
	}

	m_nSizeBuffer	= (int)fread_(m_pbBuffer, 1, m_nSizeBlock, (FILE*)m_pfFile);
	m_pbBufferCurrent	= (unsigned char*)m_pbBuffer;

	if(pmd5Class_ && pmd5State_)
		pmd5Class_->append(pmd5State_, (const md5_byte_t*)m_pbBuffer, m_nSizeBuffer);
	return m_nSizeBuffer;
}

unsigned	BaseFile::WriteCurrnetBlock()
{
	if((m_nType & OPEN_RANDOM)!=0)
		return 0;

	int nRet	= (int)fwrite_(m_pbBuffer, 1, m_nSizeBuffer, (FILE*)m_pfFile);
	m_pbBufferCurrent	= (unsigned char*)m_pbBuffer;
	m_nSizeBuffer		= 0;
	return nRet;
}

BaseFile::~BaseFile(void)
{
	release();
}

int BaseFile::read_asc_string2( char *_strSource, const int nStrSize)
{
	*_strSource = NULL;
	if(parse_token_())
	{
		const char *strStart = m_strToken->c_str();
		while(*strStart == 32)
			strStart++;
		strcpy_s(_strSource, nStrSize, strStart);
		size_t nLen = strlen(_strSource);
		while(_strSource[nLen-1] == 32)
			nLen--;
		_strSource[nLen] = 0;
		return (int)nLen;
	}
	return 0;
}
#ifdef STD_TAG_SUPPORT
bool BaseFile::read_asc_tag( char *_strSource, const int nStrSize)
{
	*_strSource = NULL;
	if(m_stlVStrTags.size() == 0)
		return false;

	strcpy_s(_strSource, nStrSize, 	m_stlVStrTags[0].c_str());
	m_stlVStrTags.erase(m_stlVStrTags.begin());
	return true;
}
#endif

STLString BaseFile::read_asc_quotation()
{
	if (parse_quotation_())
		return *m_strToken;
	STLString ret;
	return ret;
}

STLString BaseFile::read_asc_str()
{
	if (parse_token_())
		return *m_strToken;
	STLString ret;
	return ret;
}

bool BaseFile::read_asc_string(STLString* _str)
{
	if (parse_token_())
	{
		*_str = m_strToken->c_str();
		return true;
	}
	return false;
}

bool BaseFile::read_asc_string( char *_strSource, const int nStrSize,bool bIgnoreSpace)
{
	*_strSource = NULL;
	if(parse_token_())
	{
		const char *strStart = m_strToken->c_str();
		while(!bIgnoreSpace && *strStart == 32)
			strStart++;
		strcpy_s(_strSource, nStrSize, strStart);
		size_t nLen = strlen(_strSource);
		while(!bIgnoreSpace && _strSource[nLen-1] == 32)
			nLen--;
		_strSource[nLen] = 0;
		return true;
	}
	return false;
}

const char *BaseFile::token_get()
{
	return m_strToken->c_str();
}

bool BaseFile::read_asc_integer( int* _pnNum )
{
	*_pnNum = 0;
	if(parse_token_())
	{	
		sscanf_s(m_strToken->c_str(), "%d", _pnNum);
		return true;
	}
	return false;
}

bool BaseFile::read_asc_float( float *_pfValue)
{
	*_pfValue = 0;
	if(parse_token_())
	{	
		sscanf_s(m_strToken->c_str(), "%f", _pfValue);
		return true;
	}
	return false;
}

bool BaseFile::read_asc_float3(float *_f3Value)
{
	bool bRet = true;
	for(int i=0; i<3; i++)
	{ 
		bRet = read_asc_float(_f3Value + i);
		if (!bRet)
			return bRet;
	}
	return true;
}

char BaseFile::check_replace_(char _char)
{
	int nCnt=0;
	while(m_strReplace[nCnt*2] != 0
		&& nCnt < ASC_MAX_LENGTH)
	{
		if(m_strReplace[nCnt*2] == _char)
		{
			return m_strReplace[nCnt*2+1];
		}
		nCnt++;
	}
	return _char;
}

bool BaseFile::check_delector_(char _char)
{
	int nCnt=0;
	while(m_strDelete[nCnt] != 0
		&& nCnt < ASC_MAX_LENGTH)
	{
		if(m_strDelete[nCnt] == _char)
		{
			m_nDelecterLast	= (int)_char;// Add by OJ : 2010-10-11
			return true;
		}
		nCnt++;
	}
	return false;
}

bool BaseFile::check_seperator_(char _char, int _seq)
{
	int nCnt=0;
	while(m_strSeperator[nCnt] != 0
		&& nCnt < ASC_MAX_LENGTH)
	{
		if(m_strSeperator[nCnt] == _char
			|| (_seq > 0 && m_sign > 0 && _char == m_sign))
		{
			m_nSeperatorLast	= (int)_char;// Add by OJ : 2010-10-11
			return true;
		}
		nCnt++;
	}

	if (m_mpercent_ap && (_char == '&' || _char == ';')) // support for & tag for xml
	{
		m_nSeperatorLast = (int)_char;
		return true;
	}

	return false;
}

bool BaseFile::check_tag_()
{
	bool bRet = false;

	if((*m_strTag)[0] == 0
		|| (*m_strTag)[1] == 0)
		return false;

	char strTag[ASC_MAX_LENGTH] = {0};

	while(m_strBufferAscLine[m_nCurCharAscLine] == (*m_strTag)[0])
	{
		m_nCurCharAscLine++;
		int nCnt = 0;
		bRet = true;
		while(m_strBufferAscLine[m_nCurCharAscLine] != (*m_strTag)[1]
			&& m_strBufferAscLine[m_nCurCharAscLine] != 0
			&& m_nCurCharAscLine < m_nSizeLine)
		{
			strTag[nCnt] = m_strBufferAscLine[m_nCurCharAscLine];
			nCnt++;
			m_nCurCharAscLine++;
		}
#ifdef STD_TAG_SUPPORT
		m_stlVStrTags.push_back(strTag);
#endif
	}
	return bRet;
}

bool BaseFile::parse_quotation_()
{
#ifdef STD_TAG_SUPPORT
	m_stlVStrTags.clear();
#endif
	return parse_token_(true);
}

bool BaseFile::parse_token_(bool _quot)
{
	m_nSeperatorLast = 0;
	if(m_nCurCharAscLine >= m_nSizeLine)
		return false;

	//while(check_seperator_(m_strBufferAscLine[m_nCurCharAscLine])
	//	&& m_strBufferAscLine[m_nCurCharAscLine] != 0
	//	&& m_nCurCharAscLine < ASC_MAX_LENGTH)
	//	m_nCurCharAscLine++;

	if(m_strBufferAscLine[m_nCurCharAscLine] == 0)
		return false;
#ifdef STD_TAG_SUPPORT
	m_stlVStrTags.clear();
	//bool	bInTag = false;
#endif
	while(check_delector_(m_strBufferAscLine[m_nCurCharAscLine])
		&& m_strBufferAscLine[m_nCurCharAscLine] != 0
		&& m_nCurCharAscLine < m_nSizeLine)
	{
		m_nCurCharAscLine++;
	}
	m_strToken->clear();
	
	bool start_quot = false;
	if(_quot && m_strBufferAscLine[m_nCurCharAscLine] == '\"')
	{
		start_quot = true;
		m_nCurCharAscLine++;
	}

	int seq = 0;
	while(m_strBufferAscLine[m_nCurCharAscLine] != 0
		  && m_nCurCharAscLine < m_nSizeLine
		  && ((start_quot && m_strBufferAscLine[m_nCurCharAscLine] != '\"')
			|| (!start_quot && !check_tag_() && !check_seperator_(m_strBufferAscLine[m_nCurCharAscLine], seq)))
		)
	{
		//if(!check_delector_(m_strBufferAscLine[m_nCurCharAscLine]))
		{
			*m_strToken += check_replace_(m_strBufferAscLine[m_nCurCharAscLine]);
		}
		m_nSeperatorLast = 0;
		m_nCurCharAscLine++;
		seq++;
	}
	
	if(start_quot && m_strBufferAscLine[m_nCurCharAscLine] == '\"')
	{
		m_nSeperatorLast = '\"';
		m_nCurCharAscLine++;
	}

	if(m_strBufferAscLine[m_nCurCharAscLine] != 0
		&& (m_sign == 0|| m_nSeperatorLast != m_sign))
		m_nCurCharAscLine++;

	if (m_mpercent_ap)
	{
		for (int i = 0; i < m_mpercent_ap->size(); i++)
		{
			if (*m_strToken == (*m_mpercent_ap)[i].first)
			{
				*m_strToken = (*m_mpercent_ap)[i].second;
			}
		}
	}

	return true;
}

bool BaseFile::allocate_linememory()
{
	char* buf = PT_Alloc(char, m_nSizeLine * 2);
	if (buf == NULL)
		return false;
	memcpy(buf, m_strBufferAscLine, m_nSizeLine);
	m_nSizeLine *= 2;
	PT_Free(m_strBufferAscLine);
	m_strBufferAscLine = buf;
	return true;
}

bool BaseFile::write_asc_line()
{
	m_bFirstTokenLine	= true;

	if (m_nCurCharAscLine + 3 >= m_nSizeLine) // support mass of line contents
		if (!allocate_linememory())
			return false;

	for(unsigned i=0; m_strCarriageReturn[i] != 0; i++)
	{
		m_strBufferAscLine[m_nCurCharAscLine] = m_strCarriageReturn[i];
		m_nCurCharAscLine++;
	}
	m_strBufferAscLine[m_nCurCharAscLine+1]	= NULL;
	
	int nRet = write_dump_(m_strBufferAscLine, m_nCurCharAscLine);
	
	if(nRet == m_nCurCharAscLine)
	{
		m_nCurCharAscLine = 0;
		return true;
	}
	return false;
}


bool BaseFile::write_asc_string_noseperator( const char *_strSource, const int nStrSize)
{
	size_t nLength = strlen(_strSource);

	while (m_nCurCharAscLine + nLength >= m_nSizeLine)
		allocate_linememory();

	strcpy_s(&m_strBufferAscLine[m_nCurCharAscLine], m_nSizeLine - m_nCurCharAscLine, _strSource);
	m_nCurCharAscLine += (int)nLength;
	return true;
}

bool BaseFile::write_asc_string_(const char* _strSource, const int nStrSize)
{
	size_t nLength = nStrSize;
	if(nStrSize == 0)
		nLength = strlen(_strSource);
	while (m_nCurCharAscLine + nLength >= m_nSizeLine)
		if (!allocate_linememory())
			return false;

	strcpy_s(&m_strBufferAscLine[m_nCurCharAscLine], m_nSizeLine - m_nCurCharAscLine, _strSource);
	m_nCurCharAscLine += (int)nLength;
	return true;
}

bool BaseFile::write_asc_string( const char *_strSource, const int /* nStrSize*/)
{
	if(!m_bFirstTokenLine)
	{
		m_strBufferAscLine[m_nCurCharAscLine] = m_strSeperator[0];
		m_nCurCharAscLine++;
	}

	m_bFirstTokenLine	= false;
	return write_asc_string_(_strSource, 0);
}

bool BaseFile::write_asc_integer64( INT64 _nNum )
{
	if(!m_bFirstTokenLine)
	{
		m_strBufferAscLine[m_nCurCharAscLine] = m_strSeperator[0];
		m_nCurCharAscLine++;
	}

	m_bFirstTokenLine	= false;
	char strBuffer[128];

	strBuffer[0] = 0;
	sprintf_s(strBuffer, 128, "%llu", _nNum);
	return write_asc_string_(strBuffer, 0);
}

bool BaseFile::write_asc_integer( int _nNum )
{
	if(!m_bFirstTokenLine)
	{
		m_strBufferAscLine[m_nCurCharAscLine] = m_strSeperator[0];
		m_nCurCharAscLine++;
	}

	m_bFirstTokenLine	= false;

	char strBuffer[128];
	strBuffer[0] = 0;
	sprintf_s(strBuffer, 128, "%d", _nNum);
	return write_asc_string_(strBuffer, 0);
}

bool BaseFile::write_asc_float3(const float *_f3Value)
{
	for (int i = 0; i < 3; i++) {
		if (!write_asc_float(_f3Value[i]))
			return false;
	}
	return true;
}

bool BaseFile::write_asc_cutfloat3(const float *_f3Value)
{
	for (int i = 0; i < 3; i++) {
		if (!write_asc_cutfloat(_f3Value[i]))
			return false;
	}
	return true;
}

bool BaseFile::write_asc_cutfloat(float _fValue)
{
	STLString str = BaseFile::to_str(_fValue, 2);
	return write_asc_string(str.c_str(), (int)str.size());
}

bool BaseFile::write_asc_float( float _fValue)
{
	if(!m_bFirstTokenLine)
	{
		m_strBufferAscLine[m_nCurCharAscLine] = m_strSeperator[0];
		m_nCurCharAscLine++;
	}

	m_bFirstTokenLine	= false;

	char strBuffer[128];

	strBuffer[0] = 0;
	sprintf_s(strBuffer, 128, "%f", _fValue);
	return write_asc_string_(strBuffer, 0);
}

void BaseFile::set_asc_carriage(const char *_strCarriageReturn)
{
	strcpy_s(m_strCarriageReturn, STD_STR_MIN, _strCarriageReturn);
}

void BaseFile::set_asc_replace(const char *_strReplace)
{
	strcpy_s(m_strReplace, STD_STR_MIN, _strReplace);
}

void BaseFile::set_asc_deletor(const char *_strDelector)
{
	strcpy_s(m_strDelete, STD_STR_MIN, _strDelector);
}

STLString BaseFile::mpercent_parsing(STLVPstrstr _mpercent)
{
	STLString ret;

	if (m_mpercent_ap)
		delete m_mpercent_ap;

	m_mpercent_ap = new STLVPstrstr;
	*m_mpercent_ap = _mpercent;

	STLString tag;
	read_asc_line();
	do
	{
		tag = read_asc_str();
		if (!tag.empty())
			ret += tag;
	} while (get_seperator_last() != 0);

	delete m_mpercent_ap;
	m_mpercent_ap = NULL;
	return ret;
}

void BaseFile::push_asc_seperator(const char* _strSeperator)
{
	strcpy_s(m_strSeperator2, STD_STR_MIN, m_strSeperator);
	strcpy_s(m_strSeperator, STD_STR_MIN, _strSeperator);
}

void BaseFile::pop_asc_seperator() 
{
	strcpy_s(m_strSeperator, STD_STR_MIN, m_strSeperator2);
}

void BaseFile::set_asc_seperator(const char *_strSeperator)
{
	strcpy_s(m_strSeperator, STD_STR_MIN, _strSeperator);
}

void BaseFile::set_asc_sign(char _sign)
{
	m_sign = _sign;
}

bool string_ch_del(char *_target, char _ch)
{
	char* _ret, * _from;
	_ret = _target;
	_from = _target;
	while (*_from != 0)
	{
		if (_from - _target >= STD_STR_MIN)
			break;
		if (*_from != _ch) {
			if (_ret != _from)
				*_ret = *_from;
			_ret++, _from++;
		}
		else
			_from++;
	}
	*_ret = *_from;
	return _ret != _from;
}

void string_ch_add(char *_target, char _ch)
{
	size_t len = strlen(_target);
	_target[len] = _ch;
	_target[len + 1] = 0;
}

bool BaseFile::deletor_del(char _ch)
{
	return string_ch_del(m_strDelete, _ch);
}

void BaseFile::deletor_add(char _ch)
{
	string_ch_add(m_strDelete, _ch);
}

bool BaseFile::seperator_del(char _ch)
{
	return string_ch_del(m_strSeperator, _ch);
}

void BaseFile::seperator_add(char _ch)
{
	string_ch_add(m_strSeperator, _ch);
}

void BaseFile::set_asc_tag(const char *_strTag)
{
	*m_strTag = _strTag;
}

bool BaseFile::read_asc_leave_line(char *_strSource, size_t _nSize)
{
	if(_nSize <= strlen(&m_strBufferAscLine[m_nCurCharAscLine]))
		return false;

	strcpy_s(_strSource, _nSize, &m_strBufferAscLine[m_nCurCharAscLine]);
	return true;
}

bool BaseFile::read_asc_line()
{
	m_nCurCharAscLine = 0;
	int nCnt=0;

	do
	{
		while(m_nCurCharAscBlock < ASC_MAX_LENGTH
			&& m_nCurCharAscBlock < m_nSizeAscBlock)
		{
#ifdef _DEBUG
			if (nCnt >= 255)
			{
				int x = 0;
			}
#endif
			if (nCnt >= m_nSizeLine) // support mass of line contents
				allocate_linememory();

			if((m_strBufferAscBlock[m_nCurCharAscBlock] == '\r'
				|| m_strBufferAscBlock[m_nCurCharAscBlock] == '\n')
				&& !check_seperator_(m_strBufferAscBlock[m_nCurCharAscBlock], 0)
				&& !check_delector_(m_strBufferAscBlock[m_nCurCharAscBlock]))
			{
				m_strBufferAscLine[nCnt] = NULL;

				while((unsigned)m_strBufferAscBlock[m_nCurCharAscBlock] < 32
					&& m_nCurCharAscBlock < ASC_MAX_LENGTH
					&& m_nCurCharAscBlock < m_nSizeAscBlock)
				{
					bool bEnter = false;
					if(m_strBufferAscBlock[m_nCurCharAscBlock] == '\n')	// \n�� �ִ� ���� ���� �߰�
						bEnter = true;

					m_nCurCharAscBlock++;
					if(m_nCurCharAscBlock >= m_nSizeAscBlock)
					{
						m_nSizeAscBlock = read_dump_( m_strBufferAscBlock, ASC_MAX_LENGTH);
						m_nCurCharAscBlock = 0;
					}

					if(bEnter)
						break;
				}

				if(m_cComment != 0 && m_strBufferAscLine[0] == m_cComment) // '#'�� �տ� ������ �̶����� �ǳ� �ڴ�
				{
					nCnt = 0;
					continue;
				}

				return true;
			}

			m_strBufferAscLine[nCnt] = m_strBufferAscBlock[m_nCurCharAscBlock];
			nCnt++;
			m_nCurCharAscBlock++;
		}

		m_nSizeAscBlock = read_dump_( m_strBufferAscBlock, ASC_MAX_LENGTH);
		if(m_nSizeAscBlock == 0 && m_nCurCharAscBlock > 0)
		{
			if (nCnt >= m_nSizeLine) // support mass of line contents
				allocate_linememory();

			m_strBufferAscLine[nCnt]	= NULL;
			m_nCurCharAscBlock			= 0;
			return true;
		}
		m_nCurCharAscBlock = 0;
	}while(m_nSizeAscBlock > 0);

	if(nCnt > 0)
	{
		if(m_cComment != 0 && m_strBufferAscLine[0] != m_cComment) // '#'�� �տ� ������ �̶����� �ǳ� �ڴ�
		{
			if (nCnt >= m_nSizeLine) // support mass of line contents
				allocate_linememory();

			m_strBufferAscLine[nCnt] = 0;
			return true;
		}
	}

	return false;
}

const char *BaseFile::get_asc_line()
{
	return m_strBufferAscLine;
}

bool BaseFile::write_bin_sinteger(const int* _nNum)
{
	if (m_nType & OPEN_ASC)
		write_sx_(*_nNum);
	else
		write_dump_(_nNum, 4);
	return true;
}

bool BaseFile::write_bin_integer( const int* _nNum )
{
	if(m_nType & OPEN_ASC)
		write_x_(*_nNum);
	else
		write_dump_( _nNum,  4 );
	return true;
}

bool BaseFile::write_bin_unsigned_long( const UINT32* _nNum )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp = (int)*_nNum;
		write_x_(nTemp);
	}else
		write_dump_( _nNum, 4 );
	return true;
}

bool BaseFile::write_bin_string( const char* _strSource )
{
	size_t nSize;
	nSize = strlen( _strSource ) + 1;

	if(m_nType & OPEN_ASC)
	{
		write_x_(_strSource, (int)nSize);
	}else{
		write_dump_( &nSize, sizeof(int) );
		write_dump_( _strSource, sizeof(char) * (int)nSize ) ;			
	}
	return true;
}

bool BaseFile::write_bin_bool( const char* _bFlag )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp = *_bFlag;
		write_x_(nTemp);
	}else
		write_dump_( _bFlag, sizeof(char) );

	return true;
}

bool BaseFile::write_bin_byte( const unsigned char* _bValue )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp = *_bValue;
		write_x_(nTemp);
	}else
		write_dump_( _bValue, sizeof(unsigned char) );

	return true;
}


bool BaseFile::read_bin_sinteger(int *_pnValue)
{
	if (m_nType & OPEN_ASC)
	{
		return read_sx_(_pnValue);
	}

	*_pnValue = 0;
	return read_dump_(_pnValue, 4) == 0 ? false : true;
}

bool BaseFile::read_bin_integer(int *_pnValue)
{
	if(m_nType & OPEN_ASC)
	{
		return read_x_(_pnValue);
	}

    *_pnValue = 0;
	return read_dump_(_pnValue, 4 )==0?false:true;
}

bool BaseFile::read_bin_unsigned_long(unsigned long *_pnValue)
{
	if(m_nType & OPEN_ASC)
	{
		read_x_((int*)_pnValue);
		return true;
	}

    *_pnValue = 0;
	return read_dump_( _pnValue, 4 )==0?false:true;
}

int BaseFile::read_bin_string( char* _strSource, const int nStrSize )
{
	int nSize;
	char str[128];
	if(m_nType & OPEN_ASC)
	{
		nSize = read_x_(_strSource, nStrSize);
	}else{
		read_dump_( &nSize, sizeof(int) );
		B_ASSERT(nSize < nStrSize);
		if(nSize > nStrSize)
			return false;
		if(nSize < 0)
			return false;

		read_dump_( str, sizeof(char) * nSize );
		str[nSize] = 0;
		strcpy_s( _strSource, nStrSize, str );
	}
	return nSize;
}

bool BaseFile::read_bin_bool(char *_pcValue)
{
	if(m_nType & OPEN_ASC)
	{
		int nValue;
		read_x_(&nValue);
		*_pcValue	= (char)nValue;
		return true;
	}
	return read_dump_( _pcValue, sizeof(char) )==0?false:true;
}

bool BaseFile::read_bin_byte( unsigned char* _bValue )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp;
		read_x_(&nTemp);
		*_bValue = (char)nTemp;
	}else
		read_dump_( _bValue, sizeof(char) );
	return true;
}

bool BaseFile::write_bin_floatN(const float* _fNum, int _n)
{
	bool ret = true;
	for (int i = 0; i < _n; i++)
		if (!write_bin_float(&_fNum[i]))
			ret = false;
	return ret;
}

bool BaseFile::write_bin_float( const float* _fNum )
{
	if(m_nType & OPEN_ASC)
	{
		write_x_(*_fNum);
	}else
		write_dump_( _fNum, sizeof(float) );
	return true;
}

bool BaseFile::read_bin_floatN(float* _fNum, int _n)
{
	bool ret = true;
	for (int i = 0; i < _n; i++)
		if (!read_bin_float(&_fNum[i]))
			ret = false;
	return ret;
}

bool BaseFile::read_bin_float(float *_pfValue)
{
	if(m_nType & OPEN_ASC)
	{
		read_x_(_pfValue);
		return true;
	}
	return read_dump_( _pfValue, sizeof(float) )==0?false:true;
}
/*
bool BaseFile::read_bin_double(double *_pvalue)
{
    if(m_nType & OPEN_ASC)
    {
        read_x_(_pvalue);
    }
    return read_dump_(_pvalue, sizeof(double))==0?false:true;
}
//*/
bool BaseFile::write_bin_long( const INT32* _nNum )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp = (int)*_nNum;
		write_x_(nTemp);
	}else
		write_dump_( _nNum,  4 );
	return true;
}

bool BaseFile::read_bin_long(long *_pnValue)
{
	if(m_nType & OPEN_ASC)
	{
		read_x_((int*)_pnValue);
		return true;
	}
    *_pnValue = 0;
	return read_dump_( _pnValue, 4 )==0?false:true;
}

bool BaseFile::write_bin_ushort( const unsigned short* _nNum )
{
	INT32 nNum;
	if(m_nType & OPEN_ASC)
	{
		int nTemp = *_nNum;
		write_x_(nTemp);
		nNum = sizeof(unsigned short);
	}else
		nNum = write_dump_( _nNum, sizeof(unsigned short) );
	return nNum==0?false:true;
}

bool BaseFile::write_bin_short( const short* _nNum )
{
	INT32 nNum;
	if(m_nType & OPEN_ASC)
	{
		int nTemp = *_nNum;
		write_x_(nTemp);
		nNum = sizeof(short);
	}else
		nNum = write_dump_( _nNum, sizeof(short) );
	return nNum==0?false:true;
}

bool BaseFile::read_bin_ushort(unsigned short *_pnShort)
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp;
		read_x_(&nTemp);
		*_pnShort = (unsigned short)nTemp;
	}else
		return (read_dump_( _pnShort, sizeof(unsigned short) ) != 0);
	return true;
}

bool BaseFile::read_bin_short( short *_pnShort)
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp;
		if(!read_x_(&nTemp))
			return false;
		*_pnShort = (short)nTemp;
	}else
		return (read_dump_( _pnShort, sizeof(short) ) != 0);
	return true;
}

bool BaseFile::allocate_memoryfile(UINT32 _nSize)
{
	if(m_nType & MEM_OUTDATA)
		return false;

	char *pOldBuffer;

	pOldBuffer = m_pbMemoryFile;

	m_nSizeOfFile += _nSize;//1048576;
	m_pbMemoryFile = PT_Alloc(char, m_nSizeOfFile);//NEW_A(char, m_nSizeOfFile); // �߰��� 1Mbyte���� ������Ų��
	memcpy(m_pbMemoryFile, pOldBuffer, m_nSizeBuffer);
	m_pbBuffer	= m_pbMemoryFile+m_nSizeBuffer;
	m_pbBufferCurrent = (unsigned char*)m_pbBuffer;

	if(m_bDelMemoryFile)
		PT_Free(pOldBuffer);//DEL_A(pOldBuffer);

	m_bDelMemoryFile = true;

	return true;
}
//
//UINT32 BaseFile::seed_encrypt_(unsigned char *_pBuffer, UINT32 _nSize, unsigned char *_pKey)
//{
//	UINT32 nRet=0;
//	UINT32 cRoundKey[32];
//
//	seed_EncRoundKey(cRoundKey, _pKey);
//	
//	do 
//	{
//		/* Encryption Algorithm */
//		seed_Encrypt(_pBuffer+nRet, cRoundKey);
//		nRet += 16;
//	} while(nRet < _nSize);
//
//	return nRet;
//}
//
//UINT32 BaseFile::seed_decrypt_(unsigned char *_pBuffer, UINT32 _nSize, unsigned char *_pKey)
//{
//	UINT32 nRet=0;
//	UINT32 cRoundKey[32];
//
//	seed_EncRoundKey(cRoundKey, _pKey);
//
//	do 
//	{
//		/* Decryption Algorithm */
//		seed_Decrypt(_pBuffer+nRet, cRoundKey);
//		nRet += 16;
//	} while(nRet < _nSize);
//
//	return nRet;
//}

bool BaseFile::write_binary( const INT32& nNum_ )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp = (int)nNum_;
		write_x_(nTemp);
		return true;
	}else
		return write_dump_( &nNum_,  sizeof(INT32) )==0?false:true;
}

bool BaseFile::write_binary( const short& nNum_ )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp = nNum_;
		write_x_(nTemp);
		return true;
	}else
		return write_dump_( &nNum_,  sizeof(short) )==0?false:true;
}

bool BaseFile::write_binary( const float& nNum_ )
{
	if(m_nType & OPEN_ASC)
	{
		write_x_(nNum_);
		return true;
	}else
		return write_dump_( &nNum_,  sizeof(float) )==0?false:true;
}

bool BaseFile::write_binary( const char* strSource_ )
{
	size_t nSize;
	nSize = strlen( strSource_ ) + 1;
	if(m_nType & OPEN_ASC)
	{
		write_x_(strSource_, (int)nSize);
	}else{
		write_dump_( &nSize, sizeof(int) );
		write_dump_( strSource_, sizeof(char) * (int)nSize );
	}
	return true;
}

bool BaseFile::read_binary( INT32& nNum_ )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp;
		read_x_(&nTemp);
		nNum_ = nTemp;
		return true;
	}else
		return read_dump_( &nNum_, sizeof(INT32) )==0?false:true;
}

bool BaseFile::read_binary( short& nNum_ )
{
	if(m_nType & OPEN_ASC)
	{
		int nTemp;
		read_x_(&nTemp);
		nNum_ = (short)nTemp;
		return true;
	}else
		return read_dump_( &nNum_, sizeof(short) )==0?false:true;
}

bool BaseFile::read_binary( float& nNum_ )
{
	if(m_nType & OPEN_ASC)
	{
		read_x_(&nNum_);
		return true;
	}else
		return read_dump_( &nNum_, sizeof(float) )==0?false:true;
}

bool BaseFile::read_binary( char* strSource, int nSize_ )
{
	if(m_nType & OPEN_ASC)
	{
		read_x_(strSource, nSize_);
	}else{
		int nSize;
		char str[1024];
		read_dump_( &nSize, sizeof(int) );
		read_dump_( str, sizeof(char) * nSize );
		
		strcpy_s( strSource, nSize_, str );
	}

	return true;
}

UINT32 BaseFile::get_buffer_size()
{
	return m_nSizeBuffer;
}

void BaseFile::write_brace_start(const char *_strComment)
{
	int nBackupCnt;
	if(m_nType & OPEN_ASC)
	{
		write_x_tab_(m_nBraceCnt);
		write_dump_(&m_cBrace[0], 1);
		nBackupCnt = m_nBraceCnt;
		m_nBraceCnt = 1;
		write_x_(_strComment, 256);
		m_nBraceCnt = nBackupCnt;

		m_stlQStack->push_back(_strComment);
	}
	m_nBraceCnt++;
}

void BaseFile::write_brace_end()
{
	int nBackupCnt;
	m_nBraceCnt--;
	B_ASSERT(m_nBraceCnt >= 0);
	if(m_nType & OPEN_ASC)
	{
		const char *strComment = m_stlQStack->back().c_str();
		write_x_tab_(m_nBraceCnt);
		write_dump_(&m_cBrace[1], 1);
		nBackupCnt = m_nBraceCnt;
		m_nBraceCnt = 1;
		write_x_(strComment, 256);
		m_nBraceCnt = nBackupCnt;

		m_stlQStack->pop_back();
	}
}

bool BaseFile::read_brace_start(char* _str, int _size)
{
	if(m_nType & OPEN_ASC)
	{
		char cTemp[2];
		do{
			if(read_dump_(cTemp, 1)==0)
				return false;

			if (cTemp[0] == m_cBrace[1])
			{
				m_nSeperatorLast = 0;
				read_x_tail_();
				m_nBraceCnt--;
				return false;
			}
			B_ASSERT(cTemp[0] == '\t' || cTemp[0] == ' ' || cTemp[0] == m_cBrace[0]);
			if (cTemp[0] != '\t' && cTemp[0] != ' ' && cTemp[0] != m_cBrace[0])
				return false;
		}while(cTemp[0] != m_cBrace[0]);
		read_bin_string(_str, _size);
		//read_x_tail_();
		//read_dump_(cTemp, 2); assert(cTemp[0] == m_cReturn[0]);
	}
	m_nBraceCnt++;
	return true;
}

bool BaseFile::read_brace_end_check()
{
	return false;
}

void BaseFile::read_brace_end()
{
	int temp = 0;
	char cTemp[2];
	char endMsg[1024];
	endMsg[0] = 0;
	int cnt = 0;
	m_nBraceCnt--;
	if(m_nType & OPEN_ASC)
	{
		B_ASSERT(m_nBraceCnt >= 0);
		do{
			read_dump_(cTemp, 1);
			B_ASSERT(cTemp[0] == '\t' || cTemp[0] == ' ' || cTemp[0] == m_cBrace[1]);
			endMsg[cnt++] = cTemp[0];
			if (cTemp[0] != '\t' && cTemp[0] != ' ' && cTemp[0] != m_cBrace[1])
				temp++;
		}while(cTemp[0] != m_cBrace[1]);
		m_nSeperatorLast = 0;
		read_x_tail_();
		//read_dump_(cTemp, 2); assert(cTemp[0] == m_cReturn[0]);
	}
	endMsg[cnt] = 0;
}

int BaseFile::read_x_token_(char *_str, int _nBufferSize)
{
	bool bStartQuote = false;
	char cTemp;
	do{
		if (read_dump_(&cTemp, 1) == 0)
			return 0;
	}while(cTemp <= ' ' || cTemp == '\t');
	if(cTemp == '\"')
	{
		read_dump_(&cTemp, 1);
		bStartQuote = true;
	}

	int nCnt = 0;
	while(!((bStartQuote && cTemp == '\"') || (!bStartQuote && (cTemp <= ' ' || cTemp == ',' || cTemp == m_cReturn[1])))){
		*(_str+nCnt) = cTemp;
		read_dump_(&cTemp, 1);
		nCnt++;

		B_ASSERT(nCnt < _nBufferSize);
		if(nCnt > _nBufferSize)
		{
			*(_str+nCnt) = 0;
			break;
		}
	}
	m_nSeperatorLast = 0;
	m_nSeperatorLast = (int)cTemp;
	
	*(_str+nCnt) = 0;

	return nCnt;
}

void BaseFile::write_sx_(int _nValue)
{
	write_x_tab_(m_nBraceCnt);
	char strBuf[128];
	sprintf_s(strBuf, 128, "%d", _nValue);
	write_dump_(strBuf, (int)strlen(strBuf));
	write_dump_(&m_cReturn, 2);
}

void BaseFile::write_x_(int _nValue)
{
	write_x_tab_(m_nBraceCnt);
	char strBuf[128];
	sprintf_s(strBuf, 128, "%u", _nValue);
	write_dump_(strBuf, (int)strlen(strBuf));
	write_dump_(&m_cReturn, 2);
}

void BaseFile::write_x_tab_(int _nDepth)
{
	char cTemp = '\t';
	for(int i=0; i<_nDepth; i++)
	{
		write_dump_(&cTemp, 1);
	}
}

void BaseFile::write_x_(const char *_str, int _nBufferSize)
{
	char *strBuf;
	strBuf = PT_Alloc(char, _nBufferSize + 10);
	write_x_tab_(m_nBraceCnt);
	//char strBuf[1024];
	sprintf_s(strBuf, _nBufferSize + 10, "\"%s\"", _str);
	write_dump_(strBuf, (int)strlen(strBuf));
	write_dump_(&m_cReturn, 2);
	PT_Free(strBuf);
}

int BaseFile::write_x_dump_(const void *_pdump, int _nBufferSize)
{
	char strBuf[1024];
	const char *pDump = (const char*)_pdump;
	int nTemp;
	write_x_tab_(m_nBraceCnt);
	for(int i=0; i<_nBufferSize; i++)
	{
		nTemp = *(pDump+i) & 0xFF;
		sprintf_s(strBuf, 1024, "%02x", nTemp);
		write_dump_(strBuf, 2);
	}
	write_dump_(&m_cReturn, 2);
	return _nBufferSize;
}

void BaseFile::read_x_tail_()
{
	if(m_nSeperatorLast == m_cReturn[1])
		return;
	
	char cTemp;
	do{
		if(read_dump_(&cTemp, 1)==0)
			return;
	}while(cTemp != m_cReturn[1]);
}

bool BaseFile::read_sx_(int *_pnValue)
{
	char strBuf[1024];
	read_x_token_(strBuf, 1024);
	read_x_tail_();
	if (*strBuf == m_cBrace[1])
	{
		m_nBraceCnt--;
		return false;
	}
	sscanf_s(strBuf, "%d", _pnValue);
	return true;
}

bool BaseFile::read_x_(int *_pnValue)
{
	char strBuf[1024];
	if(read_x_token_(strBuf, 1024) == 0)
		return false;
	read_x_tail_();
	if (*strBuf == m_cBrace[1])
	{
		m_nBraceCnt--;
		return false;
	}
	sscanf_s(strBuf, "%u", _pnValue);
	return true;
}

void BaseFile::write_x_(float _fValue)
{	
	char strBuf[1024];
	write_x_tab_(m_nBraceCnt);
	sprintf_s(strBuf, 1024, "%f", _fValue);
	write_dump_(strBuf, (int)strlen(strBuf));
	write_dump_(&m_cReturn, 2);
}

void BaseFile::read_x_(double *_pvalue)
{
    char strBuf[1024];
    read_x_token_(strBuf, 1024);
    read_x_tail_();
    *_pvalue = atof(strBuf);
}


void BaseFile::read_x_(float *_pfValue)
{
	char strBuf[1024];
	read_x_token_(strBuf, 1024);
	read_x_tail_();
	*_pfValue = (float)atof(strBuf);
}


int BaseFile::read_x_(char *_str, int _nBufferSize)
{
	int cnt = 0;
	cnt = read_x_token_(_str, _nBufferSize);
	read_x_tail_();
	return cnt;
}

char BaseFile::pase_x_hex_digit_(char cChar)
{
	char cRet;
	if(cChar-'0' > 9)
	{
		cRet = cChar - 'a' + 10;
	}else{
		cRet = cChar - '0';
	}
	return cRet;
}

char BaseFile::pase_x_hex_char_(char *pChar)
{
	char cRet;

	cRet = pase_x_hex_digit_(*pChar);
	cRet <<= 4;
	cRet |= pase_x_hex_digit_(*(pChar+1));

	return cRet;
}

int BaseFile::read_x_dump_(void *_pdump, int _nBufferSize)
{
	char strBuf[10240];
	char *pDump = (char*)_pdump;
	int nCnt = read_x_token_(strBuf, 10240);
	for(int i=0; i<nCnt/2; i++)
	{
		*(pDump+i) = pase_x_hex_char_(strBuf+i*2);
	}
	read_x_tail_();

	return nCnt/2;
}

int BaseFile::Seek(INT32 _nPos)
{
	if(m_nType & OPEN_MEMORY)
	{
		if(_nPos > m_nSizeBuffer)
			return MSG_ERRORTOLOAD;

		m_pbBufferCurrent	= (unsigned char*)m_pbBuffer + _nPos;
	}else if(m_nType & OPEN_RANDOM)
	{
		get_fileblock_(_nPos);
	}else{

		if(m_nType & OPEN_WRITE)
			WriteCurrnetBlock();

		fseek( (FILE*)m_pfFile, _nPos, SEEK_SET );

		if(m_nType & OPEN_READ)
			ReadNextBlock();
	}
	return MSG_SUCCESS;
}

void	BaseFile::write_fileblock_(StFileBlock *_pFileblock)
{
	if(_pFileblock->bDirty)
	{
		fseek( (FILE*)m_pfFile, _pFileblock->nIndex*m_nSizeBlock, SEEK_SET );
		fwrite_(_pFileblock->pData, _pFileblock->nSizeBuffer, 1, (FILE*)m_pfFile);
	}
	PT_Free(_pFileblock->pData);//delete[]	_pFileblock->pData;
}

void	BaseFile::read_fileblock_(StFileBlock *_pFileblock)
{
	fseek( (FILE*)m_pfFile, _pFileblock->nIndex*m_nSizeBlock, SEEK_SET );
	_pFileblock->pData			= PT_Alloc(char, m_nSizeBlock);//new char[m_nSizeBlock];
	_pFileblock->nSizeBuffer	= (unsigned)fread_(_pFileblock->pData, 1, m_nSizeBlock, (FILE*)m_pfFile);
	_pFileblock->bDirty			= false;
	_pFileblock->nLastAccessTime	= sm_nAccessCount++;
}

void	BaseFile::release_fileblock_()
{
	STLMnFileBlock::iterator	it, itRelease;
	it	= m_stlMnFileBlock->begin();

	StFileBlock	*pFileblock	= NULL;
	for(;it!=m_stlMnFileBlock->end(); it++)
	{
		if(pFileblock==NULL && &it->second != m_pFileBlockCurrent)
		{
			pFileblock	= &it->second;
			itRelease	= it;
		}else{
			if(pFileblock->nLastAccessTime > it->second.nLastAccessTime
				 && &it->second != m_pFileBlockCurrent)
			{
				pFileblock	= &it->second;
				itRelease	= it;
			}
		}
	}

	if(pFileblock)
	{
		write_fileblock_(pFileblock);
		m_stlMnFileBlock->erase(itRelease);
	}
}

char *BaseFile::get_fileblock_(UINT32 _nFilePos)
{
	unsigned	nIndex	= (unsigned)_nFilePos / m_nSizeBlock;
	unsigned	nOffset	= (unsigned)_nFilePos - nIndex * m_nSizeBlock;

	STLMnFileBlock::iterator	it;
	it	= m_stlMnFileBlock->find((int)nIndex);
	if(it == m_stlMnFileBlock->end())
	{
		if(m_stlMnFileBlock->size() > FILEBLOCK_MAX)
			release_fileblock_();

		m_pFileBlockCurrent	= &(*m_stlMnFileBlock)[(int)nIndex];
		m_pFileBlockCurrent->nIndex	= (int)nIndex;

		read_fileblock_(m_pFileBlockCurrent);
	}else{
		m_pFileBlockCurrent	= &it->second;
	}

	m_pFileBlockCurrent->nLastAccessTime	= sm_nAccessCount++;
	m_pbBuffer			= m_pFileBlockCurrent->pData;
	m_pbBufferCurrent	= (unsigned char*)m_pbBuffer + nOffset;
	m_nSizeBuffer		= m_pFileBlockCurrent->nSizeBuffer - nOffset;

	return (char*)m_pbBufferCurrent;
}

int path_last(const STLString& _strfull)
{
	int start = (int)_strfull.find_last_of('\\');
	if (start == STLString::npos)
		start = (int)_strfull.find_last_of('/');
	if (start == STLString::npos)
		start = (int)_strfull.size();

	return start;
}

STLString BaseFile::get_path(const STLString& _strfull)
{
	return _strfull.substr(0, path_last(_strfull)+1);
}

STLString BaseFile::get_filenamefull(const STLString& _strfull)
{
	int start = path_last(_strfull);
	if (start == _strfull.size())
		return _strfull;
	return _strfull.substr(start+1, _strfull.size());
}

bool BaseFile::get_path(const char *_strFull, char *_strPath, int _nSize)
{
	const char *str = strrchr(_strFull, SLASH_C);
	if (str == NULL)
		str = strrchr(_strFull, '/');

	if(str == NULL)
		return false;

	int nLen	= (int)(str - _strFull) + 1; // include last '/'
	strncpy_s(_strPath, _nSize, _strFull, nLen);
	_strPath[nLen]	= NULL;
	return true;
}

bool BaseFile::change_filename(const char *_strSource, const char *_strFilename, char *_strRet, int _nSize)
{
	if(get_path_last(_strSource, _strRet, _nSize))
	{
		strcat_s(_strRet, _nSize, _strFilename);
		return true;
	}

	strcpy_s(_strRet, _nSize, _strFilename);
	return true;
}

bool BaseFile::change_fileext(const char* _strSource, const char* _ext, char* _strRet, int _nSize)
{
	strcpy_s(_strRet, _nSize, _strSource);
	int l = (int)strlen(_strSource);
	char* ext = NULL;
	int i;
	for (i = l - 1; i >= 0; i--)
	{
		if (_strRet[i] == '.')
		{
			ext = _strRet + i;
			break;
		}
	}
	if (ext == NULL)
		return false;

	strcpy_s(ext, _nSize - i, _ext);
	return true;
}

int BaseFile::path_r_find_slash(const char *_strPath, int _nStart)
{
	while (!is_slash(_strPath[_nStart]) && _nStart >= 0)
		_nStart--;

	return _nStart;
}

bool BaseFile::is_slash(char _ch)
{
	if (_ch == '/' || _ch == '\\')
		return true;
	return false;
}

char *BaseFile::path_cut_last(char *_strPath, int _nSize)
{
	size_t nLen = strlen(_strPath);

	nLen--;
	if (is_slash(_strPath[nLen]))
		nLen--;
	nLen = path_r_find_slash(_strPath, (int)nLen);

	if (nLen >= 0) {
		_strPath[nLen + 1] = NULL;
		return _strPath;
	}
	return NULL;
}

bool BaseFile::get_path_last(const char *_strFull, char *_strPath, int _nSize)
{
	const char *str = strrchr(_strFull, SLASH_C);
	if(str == NULL)
		str = strrchr(_strFull, '/');
	int nLen;

	if(str == NULL)
	{
		*_strPath = NULL;
		return false;
	}else{
		nLen	= (int)(str - _strFull);
		strncpy(_strPath, _strFull, nLen+1);
		_strPath[nLen+1] = NULL;
	}
	return true;
}

bool BaseFile::get_filenamefull(const char* _strFull, char* _strFilename, int _nSize)
{
	const char* str = strrchr(_strFull, SLASH_C);
	if (str == NULL)
		str = strrchr(_strFull, '/');
	int nLen;

	if (str == NULL)
	{
		strcpy_s(_strFilename, _nSize, _strFull);
	}
	else {
		nLen = (int)(str - _strFull);
		strcpy_s(_strFilename, _nSize, str + 1);
	}
	return true;
}

bool BaseFile::get_filename(const char *_strFull, char *_strFilename, int _nSize)
{
	if (!get_filenamefull(_strFull, _strFilename, _nSize))
		return false;

	char* str = NULL;
	str	= strrchr(_strFilename, '.');
	if(str == NULL)
		return false;
	int nLen	= (int)(str - _strFilename);
	_strFilename[nLen]	= NULL;

	return true;
}

bool BaseFile::get_filext(const char *_strFull, char *_strFilext, int _nSize)
{
	const char *str = strrchr(_strFull, '.');
	int nLen;

	if(str == NULL)
	{
		strcpy_s(_strFilext, _nSize, _strFull);
	}else{
		nLen	= (int)(str - _strFull);
		strcpy_s(_strFilext, _nSize, str);
	}
	return true;
}

bool BaseFile::is_number(const char *_str)
{
	while(*_str != 0)
	{
		if((*_str < '0' || *_str > '9')
			&& *_str != '.')
			return false;
		_str++;
	}
	return true;
}

char* BaseFile::paser_list_merge(char* _ret, int _size, const STLVString& _str_a, const char* _seperator)
{
	for (int i = 0; i < _str_a.size(); i++)
	{
		if (i == 0)
			strcpy_s(_ret, _size, _str_a[i].c_str());
		else
			strcat_s(_ret, _size, _str_a[i].c_str());
	}

	return _ret;
}

int BaseFile::paser_list_seperate(const char* _in, STLVString* _str_a, const char* _seperator)
{
	BaseFile parse;
	parse.set_asc_seperator(_seperator);
	parse.OpenFile((void*)_in, (UINT32)strlen(_in));
	parse.read_asc_line();

	char buf[1024];
	while (parse.read_asc_string(buf, 1024))
		_str_a->push_back(buf);

	return (int)_str_a->size();
}

char *BaseFile::get_l2string(char *_strRet, int _nLen, INT64 _nValue)
{
	char strTemp[128];

	strTemp[0]	= 0;
	int nLen	= 0;
	INT64	nLast, nLeave;
	do{
		nLast	= _nValue;
		_nValue	/= 10;

		nLeave	= nLast - (_nValue*10);
		strTemp[nLen]	= '0'+(char)nLeave;
		nLen++;
	}while(_nValue > 0);

	for(int i=0; i<nLen; i++)
	{
		*(_strRet+nLen-i-1)	= strTemp[i];
	}
	*(_strRet+nLen)	= 0;

	return _strRet;
}
INT64 BaseFile::get_longlong(const char *_str)
{
	INT64	nRet	= 0;

	int nLen	= (int)strlen(_str);

	nRet = (INT64)(*(_str) - '0');

	if(nRet > 9 || nRet < 0)
		nRet	= 0;
#ifdef _DEBUG
	B_ASSERT(nRet >= 0);
#endif
	for(int i=1; i<nLen; i++)
	{
		INT64 nDigit	= (INT64)(*(_str+i) - '0');

		if(nDigit > 9 || nDigit < 0)
			nDigit	= 0;

		nRet	*= 10;
		nRet	+= nDigit;
	}

	return nRet;
}

int BaseFile::get_dec_int(const char* _dec_str)
{
	int nRet;
	sscanf_s(_dec_str, "%d", &nRet);
	return nRet;
}

char* BaseFile::set_int(char* _str, int _size, int _value)
{
	sprintf_s(_str, _size, "%d", _value);
	return _str;
}

int BaseFile::get_int(const char* _hex_str)
{
	int nRet;
	sscanf_s(_hex_str, "%x", &nRet);
	return nRet;
}

float BaseFile::get_float(const char *_str)
{
	float fRet;
	sscanf_s(_str, "%f", &fRet);
	return fRet;
}

size_t BaseFile::fwrite_(void *_pBuffer, int _nCnt, size_t _nSize, FILE *_pFile)
{
	size_t	nSize	= _nCnt*_nSize;
	int nPiece	= 0;
	bbyte *pBuffer;
	pBuffer	= (bbyte*)_pBuffer;
	if(m_pEncoder)
	{
		nPiece	= EncoderOperator::SIZE_BUFFER - (nSize % EncoderOperator::SIZE_BUFFER);

		if(nPiece != 0 && nPiece != EncoderOperator::SIZE_BUFFER)
			nSize	+= nPiece;
		pBuffer	= (bbyte*)PT_Alloc(char, nSize);
		if(nPiece > 0)
			memset(pBuffer+(_nCnt*_nSize), 0, nPiece);

		memcpy(pBuffer, _pBuffer, _nSize*_nCnt);

		m_pEncoder->encode(pBuffer, (UINT32)nSize);
	}
	size_t nRet	= fwrite(pBuffer, 1, nSize, _pFile);
	if(m_pEncoder)
		PT_Free(pBuffer);
	return nRet;
}

size_t BaseFile::fread_(void *_pBuffer, int _nCnt, size_t _nSize, FILE *_pFile)
{
	size_t	nSize	= _nCnt*_nSize;
	int nPiece	= 0;
	bbyte*pBuffer;
	pBuffer	= (bbyte*)_pBuffer;
	if(m_pEncoder)
	{
		nPiece	= EncoderOperator::SIZE_BUFFER - (nSize % EncoderOperator::SIZE_BUFFER);

		if(nPiece != 0 && nPiece != EncoderOperator::SIZE_BUFFER)
			nSize	+= nPiece;
		pBuffer	= (bbyte*)PT_Alloc(char, nSize);
	}
	size_t nRet	= fread(pBuffer, 1, nSize,  _pFile);
	if(m_pEncoder)
	{
		nPiece	= EncoderOperator::SIZE_BUFFER - (nRet % EncoderOperator::SIZE_BUFFER);
		B_ASSERT(nPiece == 0 || nPiece == EncoderOperator::SIZE_BUFFER);
		if(nPiece>0 && nPiece<16)
			nSize = nRet+nPiece;
		else
			nSize	= nRet;
		m_pEncoder->decode(pBuffer, (UINT32)nSize);

		memcpy(_pBuffer, pBuffer, nRet);
		PT_Free(pBuffer);
	}
	return nRet;
}


STLString BaseFile::to_str(INT64 _n)
{
	char buf[128];
#ifdef _WIN32
	sprintf_s(buf, 128, "%lld", _n);
#else
	sprintf_s(buf, 128, "%" PRId64, _n);
#endif
	STLString ret = buf;
	return ret;
}

STLString BaseFile::to_str(int _n)
{
	char buf[128];
	sprintf(buf, "%d", _n);
	STLString ret = buf;
	return ret;
}

STLString BaseFile::to_str(float _f)
{
	char buf[128];
	sprintf(buf, "%f", _f);
	STLString ret = buf;
	return ret;
}

float exp(float _f, int cnt)
{
	if (cnt == 0)
		return 1;
	float ret = _f;
	for(int i=2; i<=cnt; i++)
		ret *= _f;
	return ret;
}

float BaseFile::round(float _f, int _digit)
{
	float f = _f;
	float m = exp(10, _digit);

	f *= m;
	if (f > 0)
		f += 5.f / m;
	else
		f -= 5.f / m;
	int n = (int)f;
	f = (float)n;
	f /= m;
	return f;
}

STLString BaseFile::to_str(float _f, int _digit)
{
	_f = round(_f, _digit);
	char buf[128];
	sprintf(buf, "%g", _f);
	STLString ret = buf;
	return ret;
}

STLString BaseFile::to_str1(float _f)
{
	float f = _f;
	f *= 10.f;
	if (f > 0)
		f += 0.05f;
	else
		f -= 0.05f;
	int n = (int)f;
	f = (float)n;
	f /= 10.f;
	char buf[128];
	sprintf(buf, "%.1f", f);
	STLString ret = buf;
	return ret;
}

#ifdef TEST_MODULE

KpUnitTestBaseFile::KpUnitTestBaseFile(void)
{
	m_pFile	= NULL;
}

KpUnitTestBaseFile::~KpUnitTestBaseFile(void)
{		
}

int KpUnitTestBaseFile::Init()
{
	m_pFile	= new BaseFile();

	m_pFile->OpenFile("test.txt", BaseFile::OPEN_READ);

	return 0;
}

//#include <windows.h>

int KpUnitTestBaseFile::Test()
{
	BaseFile	kpFile;

	int		nRead;
	char	buffer[1024];

	kpFile.OpenFile("test2.txt", BaseFile::OPEN_WRITE);

	int		nCount	= 0;

	UINT32 time = BaseSystem::timeGetTime();

	while(1)
	{
		nRead	= m_pFile->Read(buffer, 1024);
		if(nRead == 0)
			break;
		nCount++;
		kpFile.Write(buffer, nRead);
	}
	kpFile.CloseFile();

	UINT32 time2 = BaseSystem::timeGetTime();

	//printf("%d\n", time2 - time);

	return 0;
}

int KpUnitTestBaseFile::Close()
{
	if(m_pFile)
		delete m_pFile;

	return 0;
}

#endif



