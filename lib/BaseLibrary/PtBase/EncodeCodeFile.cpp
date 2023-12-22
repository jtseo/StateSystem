#include "stdafx.h"
#include "EncoderOperator.h"
#include "EncodeCodeFile.h"
#include <string.h>
#include <algorithm>

#define VERSION_ENCODE 807141	// first version ( not include extending function)

PtObjectCpp(EncodeCodeFile);

void EncodeCodeFile::init()
{
	int nKey;
	std::string strWord;

	strWord = "input";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_INPUT]		= nKey;
	m_stlMnstrReserved[nKey]		= strWord;
	strWord = "ncount";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_COUNT]		= nKey;
	m_stlMnstrReserved[nKey]		= strWord;
	strWord = "temp";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_TEMP]		= nKey;
	m_stlMnstrReserved[nKey]		= strWord;
	strWord = "output";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_OUTPUT]	= nKey;
	m_stlMnstrReserved[nKey]		= strWord;
	strWord = "key";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_KEY]	= nKey;
	m_stlMnstrReserved[nKey]	= strWord;
	strWord = "start:";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_START]	= nKey;
	m_stlMnstrReserved[nKey]	= strWord;
	strWord = "end:";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_END]	= nKey;
	m_stlMnstrReserved[nKey]	= strWord;
	strWord = "int";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_INT]	= nKey;
	m_stlMnstrReserved[nKey]	= strWord;
	strWord = "char";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_CHAR]	= nKey;
	m_stlMnstrReserved[nKey]	= strWord;
	strWord = "//";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_COMMENT]	= nKey;
	m_stlMnstrReserved[nKey]		= strWord;
	strWord = "{";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_BRACE_OPEN]	= nKey;
	m_stlMnstrReserved[nKey]			= strWord;
	strWord = "}";
	nKey = UniqHashStr::get_string_hash_code(strWord.c_str());
	m_stlMnnReserved[REV_BRACE_CLOSE]	= nKey;
	m_stlMnstrReserved[nKey]			= strWord;
}

void EncodeCodeFile::release()
{
	clear();
}

EncodeCodeFile::EncodeCodeFile(void)
{
    m_option = 0;
	init();
}

EncodeCodeFile::~EncodeCodeFile(void)
{
	release();
}

void EncodeCodeFile::compile(const char *_strFilename)
{
	int		nKey;
	char	strBuffer[255];
	bool	bStartCode = false;
	m_nLineNumber	= 0;
	std::map< int, std::string >::iterator itRev;
	const char	*strDelector = " \t;";
	set_asc_deletor(strDelector);
	const char	*strSeperator = " \t;";
	set_asc_seperator(strSeperator);

	if(!OpenFile(_strFilename, OPEN_READ))
	{
		while(read_asc_line())
		{
			m_nLineNumber++;
			read_asc_string( strBuffer, 255 );
			_strlwr_s(strBuffer, 255);

			if(strBuffer[0] == '/' || strBuffer[0] == 0)
				continue;

			nKey = UniqHashStr::get_string_hash_code(strBuffer);

			itRev = m_stlMnstrReserved.find(nKey);
			if(itRev != m_stlMnstrReserved.end())
			{
				if(m_stlMnnReserved[REV_START] == nKey)
					bStartCode = true;

				if(m_stlMnnReserved[REV_END] == nKey)
					bStartCode = false;
			}else{
				if(bStartCode)
					parse_coder_(strBuffer);
			}
		}
		CloseFile();
	}// end of if(!OpenFile(_strFilename, OPEN_READ))

	std::map< int, STLVParamRef* >	stlMnEncodes;
	std::map< int, STLVParamRef* >	stlMnDecodes;

	std::map< int, STLVParamRef* >::iterator itEn, itDe;
	itEn = m_stlMnEncodes.begin();
	while(itEn != m_stlMnEncodes.end())
	{
		itDe = m_stlMnDecodes.find(itEn->first);
		if(itDe == m_stlMnDecodes.end())
		{
			add_decoder_auto_(itEn->first);
		}
		// 추후 추가될 버전
		//add_coders_extend_(itEn->first, &stlMnEncodes, &stlMnDecodes);
		itEn++;
	}
	orgarnize_index();
}

const char *EncodeCodeFile::get_encoder_name(int _nIndex)
{
	int nHash;

	nHash	= m_stlVIndexCoder[_nIndex];
	STLMnstrReserve::iterator	it;
	it	= m_stlMnstrEncoder.find(nHash);
	if(it != m_stlMnstrEncoder.end())
		return it->second.c_str();
	return NULL;
}

void EncodeCodeFile::parse_coder_(char *_strName)
{
	char *strName;
	bool bDecoder = false;

	strName = _strName;
	if(*_strName == '~')
	{
		bDecoder	= true;
		strName		= _strName+1;
	}


	char *strBraceStart = strchr(strName, '(');
	if(!strBraceStart)
	{	
		printf("error in %d, context => %s: Function has to have a brace '('\n", m_nLineNumber, _strName);
		return;
	}
	char *strBrace = strchr(strName, ')');
	if(!strBrace)
	{
		printf("error in %d, context => %s: Function has to have a brace ')'\n", m_nLineNumber, _strName);
		return;
	}

	*strBraceStart = NULL;
	STLMnstrReserve::iterator	itRev; 
	int	nFunc = UniqHashStr::get_string_hash_code(strName);

	if(!bDecoder)
	{
		itRev = m_stlMnstrEncoder.find(nFunc);
		if(itRev != m_stlMnstrEncoder.end())
		{
			printf("error in %d, already defined function %s", m_nLineNumber, _strName);
			return;
		}
	}else{
		itRev = m_stlMnstrDecoder.find(nFunc);
		if(itRev != m_stlMnstrDecoder.end())
		{
			printf("error in %d, already defined (inverse)~function %s", m_nLineNumber, _strName);
			return;
		}
	}

	if(bDecoder)
		m_stlMnstrDecoder[nFunc]	= strName;
	else
		m_stlMnstrEncoder[nFunc]	= strName;

	bool bError = false;
	bool bStartBrace = false;
	bool bCloseBrace = false;
	char strBuffer[255];

	STLVParamRef *pstlVParam = new STLVParamRef;

	while(read_asc_line())
	{
		m_nLineNumber++;
		read_asc_string(strBuffer, 255);

		if(strBuffer[0] == '{')
		{
			if(!bStartBrace)
			{
				bStartBrace = true;
				continue;
			}else{
				break;
			}
		}else if(strBuffer[0] == '}'){
			if(!bStartBrace)
			{
				printf("error no '{' in %d\n", m_nLineNumber);
				bError = true;
			}
			bCloseBrace = true;
			break;
		}

		if(strBuffer[0] != 0)
		{
			if(!parse_coder_line_(strBuffer, pstlVParam))
			{
				delete pstlVParam;
				return;
			}
		}
	}

	if(bDecoder)
		m_stlMnDecodes[nFunc]	= pstlVParam;
	else
		m_stlMnEncodes[nFunc]	= pstlVParam;

	if(!bCloseBrace)
	{
		printf("error no '}' in %d\n", m_nLineNumber);
	}
}

bool EncodeCodeFile::parse_coder_line_(char *_strBuffer, STLVParamRef	*_pstlVParamRef)
{
	char	strBuffer[255];
	bool	bRet = true;
	bbyte nLeft, nParam1, nParam2, nCmd=0, nLength;
	if(parse_param_(_strBuffer, &nLeft) == RET_REFERENCE)
	{
		read_asc_string(_strBuffer, 255);
		if(_strBuffer[0] == '=')
		{
			read_asc_string(_strBuffer, 255);
			if(_strBuffer[0] == '~')
			{
				int nRet = parse_param_(_strBuffer+1, &nParam1);
				switch(nRet)
				{
				case RET_ERROR:		bRet	= false;					break;
				case RET_REFERENCE:	nCmd	= EncoderOperator::OP_RNOT;	break;
				case RET_NUMBER:	nCmd	= EncoderOperator::OP_NOT;	break;
				}
				nLength = 3;
				_pstlVParamRef->push_back(nCmd);
				_pstlVParamRef->push_back(nLeft);
				_pstlVParamRef->push_back(nParam1);
			}else{
				if(parse_param_(_strBuffer, &nParam1))
				{
					bool bReference = false;
					char cCmd;
					read_asc_string(strBuffer, 255);
					cCmd = strBuffer[0];
					
					read_asc_string(strBuffer, 255);
					int nRet = parse_param_(strBuffer, &nParam2);
					if(nRet != RET_ERROR)
					{
						if(nRet == RET_REFERENCE)
							bReference = true;

						switch(cCmd)
						{
						case '^': nCmd = EncoderOperator::OP_XOR; break;
						case '|': nCmd = EncoderOperator::OP_OR; break;
						case '&': nCmd = EncoderOperator::OP_AND; break;
						case '+': nCmd = EncoderOperator::OP_ADD; break;
						case '-': nCmd = EncoderOperator::OP_SUB; break;
						case '<': nCmd = EncoderOperator::OP_SFL; break;
						case '>': nCmd = EncoderOperator::OP_SFR; break;
						}

						if(bReference)
							nCmd	+= EncoderOperator::OP_RXOR;

						nLength = 4;
						_pstlVParamRef->push_back(nCmd);
						_pstlVParamRef->push_back(nLeft);
						_pstlVParamRef->push_back(nParam1);
						_pstlVParamRef->push_back(nParam2);
					}else{
						bRet = false;
					}
				}else{
					bRet	= false;
				}// end of if(parse_param_
			}// end of if(_strBuffer[0] == '~')
		}else{
			bRet = false;
		}
	}else{
		bRet = false;
	}

	if(!bRet){
		printf("error in %d, %s\n", m_nLineNumber, _strBuffer);
	}

	return bRet;
}

int	EncodeCodeFile::parse_param_(char *_strName, bbyte*_pnParam)
{
	int nRet = RET_REFERENCE;
	int nValue;

	if(_strName[0] >= '0' && _strName[0] <= '9')
	{
		nRet = RET_NUMBER;

		sscanf_s(_strName, "%d", &nValue);
		*_pnParam = (bbyte)nValue;
		return nRet;
	}

	int nOffset = 0;
	char *strBrace = strchr(_strName, '[');
	if(strBrace)
	{
		sscanf_s(strBrace+1, "%d", &nOffset);
		*strBrace = NULL;
	}

	_strlwr_s(_strName, 254);
	STLMnstrReserve::iterator	itRev; 
	int nKey = UniqHashStr::get_string_hash_code(_strName);
	
	if(m_stlMnnReserved[REV_INPUT] == nKey)
		*_pnParam	= EncoderOperator::INDEX_INPUT;
	else if(m_stlMnnReserved[REV_OUTPUT] == nKey)
		*_pnParam	= EncoderOperator::INDEX_OUTPUT;
	else if(m_stlMnnReserved[REV_KEY] == nKey)
		*_pnParam	= EncoderOperator::INDEX_KEY;
	else if(m_stlMnnReserved[REV_TEMP] == nKey)
		*_pnParam	= EncoderOperator::INDEX_TEMP;
	else if(m_stlMnnReserved[REV_COUNT] == nKey)
        {*_pnParam	= EncoderOperator::INDEX_COUNT;	nRet	= RET_NUMBER;} // count는 상수 연산자로 처리 해야 한다.
	else{
		nRet	= RET_ERROR;
		printf("error in %d, unknow value '%s'\n", m_nLineNumber, _strName);
	}
	
	*_pnParam += (bbyte)nOffset;

	return nRet;
}

bool EncodeCodeFile::is_shift_(bbyte _nCmd)
{
	switch(_nCmd)
	{
	case EncoderOperator::OP_SFL:
	case EncoderOperator::OP_SFR:
	case EncoderOperator::OP_RSFL:
	case EncoderOperator::OP_RSFR:
		return true;
	}
	return false;
}

bbyte EncodeCodeFile::get_op_inverse_(bbyte _nCmd)
{
	switch(_nCmd)
	{
	case EncoderOperator::OP_SFL:	return EncoderOperator::OP_SFR;
	case EncoderOperator::OP_SFR:	return EncoderOperator::OP_SFL;
	case EncoderOperator::OP_RSFL:	return EncoderOperator::OP_RSFR;
	case EncoderOperator::OP_RSFR:	return EncoderOperator::OP_RSFL;
	case EncoderOperator::OP_ADD:	
	case EncoderOperator::OP_SUB:
	case EncoderOperator::OP_AND:
	case EncoderOperator::OP_OR:
		printf("error to get operation inverse\n");
		break;
	}
	return _nCmd;
}

void EncodeCodeFile::add_decoder_auto_(int _nHashKeyEncode)
{
	STLVParamRef *pstlVParam;

	pstlVParam = m_stlMnEncodes[_nHashKeyEncode];
	if(pstlVParam)
	{
		UINT32 nCnt = 0;

		STLVParamRef *pstlVParamDecode = new STLVParamRef;
		while(nCnt < pstlVParam->size())
		{	
			bool bReference = false;
			bbyte nCmd	= pstlVParam->at(nCnt);	nCnt++;
			int nLength	= EncoderOperator::get_length_code(nCmd);
			nCmd = get_op_inverse_(nCmd);
			pstlVParamDecode->push_back(nCmd);

			if(nCmd >= EncoderOperator::OP_RXOR)
				bReference = true;

			bbyte nOffsetInput = 0, nLeft, nParam1, nParam2=0, nOffsetOutput = 0;
			nLeft = pstlVParam->at(nCnt); nCnt++;
			nParam1 = pstlVParam->at(nCnt); nCnt++;
			if(nLength > 3)
                {nParam2 = pstlVParam->at(nCnt); nCnt++;}

			if((nLeft & 0xF0) == EncoderOperator::INDEX_OUTPUT)
				nOffsetOutput = nLeft & 0xf;
			if((nParam1 & 0xF0) == EncoderOperator::INDEX_INPUT)
				nOffsetInput = nParam1 & 0xf;
			if(nLength > 3 && (nParam2 & 0xF0) == EncoderOperator::INDEX_INPUT
					&& bReference)
				nOffsetInput = nParam2 & 0xf;

			if((nLeft & 0xF0) == EncoderOperator::INDEX_OUTPUT)
			{
				nLeft  = (nLeft & 0xF0) | nOffsetInput;
				pstlVParamDecode->push_back(nLeft);
			}else{
				pstlVParamDecode->push_back(nLeft);
			}
			if((nParam1 & 0xF0) == EncoderOperator::INDEX_INPUT)
			{
				nParam1 = (nParam1 & 0xf0) | nOffsetOutput;
				pstlVParamDecode->push_back(nParam1);
			}else{
				pstlVParamDecode->push_back(nParam1);
			}
			if(nLength > 3)
			{
				if((nParam2 & 0xF0) == EncoderOperator::INDEX_INPUT
					&& bReference)
				{
					nParam2 = (nParam2 & 0xf0) | nOffsetOutput;
					pstlVParamDecode->push_back(nParam2);
				}else{
					pstlVParamDecode->push_back(nParam2);
				}
			}
		}// end of while
		m_stlMnDecodes[_nHashKeyEncode] = pstlVParamDecode;
	}
}

bool EncodeCodeFile::is_extendable_(bbyte _nParam)
{
	bbyte nIndex = _nParam & 0xF0;

	if(nIndex == EncoderOperator::INDEX_TEMP)
		return false;

	return true;
}

void EncodeCodeFile::add_coders_extend_(int _nHashKeyEncode, 
										std::map< int, STLVParamRef* > *_pstlMnpEncode, 
										std::map< int, STLVParamRef* > *_pstlMnpDecode)
{
	STLVParamRef *pstlVParam;

	pstlVParam = m_stlMnEncodes[_nHashKeyEncode];
	
	if(pstlVParam)
	{
		UINT32 nCnt = 0;

		pstlVParam = new STLVParamRef;
		while(nCnt < pstlVParam->size())
		{	
			bool bReference = false;
			bbyte nCmd	= pstlVParam->at(nCnt);	nCnt++;
			int nLength	= EncoderOperator::get_length_code(nCmd);

			if(nCmd >= EncoderOperator::OP_RXOR)
				bReference = true;

			bbyte nOffset = 0, nLeft, nParam1, nParam2=0;
			nLeft = pstlVParam->at(nCnt); nCnt++;
			nParam1 = pstlVParam->at(nCnt); nCnt++;
			if(nLength > 3)
                {nParam2 = pstlVParam->at(nCnt); nCnt++;}

			bbyte nDif, nEParam1, nEParam2;
			for(nDif=1;nDif < 16; nDif+=1)
			{
				nEParam1 = nParam1;
				if(is_extendable_(nParam1))
				{
					nOffset = nParam1 & 0xF;
					nOffset += nDif;
					nOffset	= nOffset & 0xF;
					nEParam1 = (nParam1 & 0xF0) & nOffset;

					pstlVParam->push_back(nCmd);
					pstlVParam->push_back(nLeft);
					pstlVParam->push_back(nEParam1);
					if(nLength > 3)
						pstlVParam->push_back(nParam2);
				}

				if(nLength > 3 && is_extendable_(nParam2) && bReference)
				{
					bbyte nDif2;
					for(nDif2=1;nDif2 < 16; nDif2+=1)
					{
						nOffset = nParam2 & 0xF;
						nOffset += nDif2;
						nOffset	= nOffset & 0xF;
						nEParam2 = (nParam2 & 0xF0) & nOffset;

						pstlVParam->push_back(nCmd);
						pstlVParam->push_back(nLeft);
						pstlVParam->push_back(nEParam1);
						pstlVParam->push_back(nEParam2);
					}
				}
			}// end of for
		}// end of while
	}

	//_pstlMnpEncode
	//_pstlMnpDecode
}

bool EncodeCodeFile::load(const char *_strFilename)
{
	clear();

	int nVersion	= VERSION_ENCODE;
	if(!OpenFile(_strFilename, OPEN_READ))
	{
		read_bin_integer(&nVersion);
		int nSize, i;
		read_bin_integer(&nSize);
		m_stlMnEncodes.clear();
		m_stlMnDecodes.clear();
		int nHash;
		char strBuffer[255];
		for(i = 0; i<nSize; i++)
		{
			read_bin_integer(&nHash);
			read_bin_string(strBuffer, 255);
			m_stlMnstrEncoder[nHash] = strBuffer;
			m_stlMnstrDecoder[nHash] = strBuffer;
		}
		//STLMnstrReserve					m_stlMnstrDecoder; encoder와 같아야 함으로 하나만 저장한다.

		read_bin_integer(&nSize);
		m_stlMnEncodes.clear();
		int nLength;
		for(i=0; i<nSize; i++)
		{
			read_bin_integer(&nHash);
			read_bin_integer(&nLength);
			STLVParamRef *pstlVParam = new STLVParamRef;
			pstlVParam->resize(nLength);
			Read(&pstlVParam->at(0), nLength);
			m_stlMnEncodes[nHash] = pstlVParam;
		}
		m_stlMnDecodes.clear();
		for(i=0; i<nSize; i++)
		{
			read_bin_integer(&nHash);
			read_bin_integer(&nLength);
			STLVParamRef *pstlVParam = new STLVParamRef;
			pstlVParam->resize(nLength);
			Read(&pstlVParam->at(0), nLength);
			m_stlMnDecodes[nHash] = pstlVParam;
		}

		CloseFile();
	}else{
		return false;
	}
	orgarnize_index();
	return true;
}

bool EncodeCodeFile::save(const char *_strFilename)
{
	int nVersion	= VERSION_ENCODE;
	if(!OpenFile(_strFilename, OPEN_WRITE))
	{
		write_bin_integer(&nVersion);
		int nSize;
		nSize = (int)m_stlMnEncodes.size();
		STLMnstrReserve::iterator itEncode;
		write_bin_integer(&nSize);
		for(itEncode = m_stlMnstrEncoder.begin(); itEncode != m_stlMnstrEncoder.end(); itEncode++)
		{
			write_bin_integer(&itEncode->first);
			write_bin_string(itEncode->second.c_str());
		}
		//STLMnstrReserve					m_stlMnstrDecoder; encoder와 같아야 함으로 하나만 저장한다.

		nSize = (int)m_stlMnEncodes.size();
		write_bin_integer(&nSize);
		std::map< int, STLVParamRef* >::iterator itParam;
		for(itParam = m_stlMnEncodes.begin(); itParam != m_stlMnEncodes.end(); itParam++)
		{
			write_bin_integer(&itParam->first);
			nSize = (int)itParam->second->size();
			write_bin_integer(&nSize);
			Write( &itParam->second->at(0), (int)itParam->second->size());
		}
		for(itParam = m_stlMnDecodes.begin(); itParam != m_stlMnDecodes.end(); itParam++)
		{
			write_bin_integer(&itParam->first);
			nSize = (int)itParam->second->size();
			write_bin_integer(&nSize);
			Write( &itParam->second->at(0), (int)itParam->second->size());
		}

		CloseFile();
	}else{
		return false;
	}
	return true;
}

void EncodeCodeFile::clear()
{
	int nSize = 0;
	nSize = (int)m_stlMnEncodes.size();
	std::map< int, STLVParamRef* >::iterator itParam;
	for(itParam = m_stlMnEncodes.begin(); itParam != m_stlMnEncodes.end(); itParam++)
	{
		delete itParam->second;
	}

	for(itParam = m_stlMnDecodes.begin(); itParam != m_stlMnDecodes.end(); itParam++)
	{
		delete itParam->second;
	}

	m_stlMnstrDecoder.clear();
	m_stlMnstrEncoder.clear();
	m_stlMnDecodes.clear();
	m_stlMnEncodes.clear();
	m_stlVIndexCoder.clear();
}

const STLVParamRef *EncodeCodeFile::get_encoder(int _nIndex)
{
	if(_nIndex >= (int)m_stlVIndexCoder.size())
	{
		_nIndex	= _nIndex % (int)m_stlVIndexCoder.size();
	}

	_nIndex	= m_stlVIndexCoder[_nIndex];
	return m_stlMnEncodes[_nIndex];
}

const STLVParamRef *EncodeCodeFile::get_decoder(int _nIndex)
{
	if(_nIndex >= (int)m_stlVIndexCoder.size())
	{
		_nIndex	= _nIndex % (int)m_stlVIndexCoder.size();
	}

	_nIndex	= m_stlVIndexCoder[_nIndex];
	return m_stlMnDecodes[_nIndex];
}

int EncodeCodeFile::get_size_encoder()
{
	return (int)m_stlMnEncodes.size();
}

int compare_int(const void *_pnParam1, const void *_pnParam2)
{
    if(*((const int*)_pnParam1) > *((const int*)_pnParam2))
        return -1;
    if(*((const int*)_pnParam1) < *((const int*)_pnParam2))
        return 1;
    return 0;
}

void EncodeCodeFile::orgarnize_index()
{
	std::map< int, STLVParamRef* >::iterator	it;
	for(it=m_stlMnEncodes.begin(); it!=m_stlMnEncodes.end(); it++)
	{
		m_stlVIndexCoder.push_back(it->first);
	}

	//sort(m_stlVIndexCoder.begin(), m_stlVIndexCoder.end(), compare_int);
    qsort(&(*m_stlVIndexCoder.begin()), m_stlVIndexCoder.size(), sizeof(int), compare_int);
}
