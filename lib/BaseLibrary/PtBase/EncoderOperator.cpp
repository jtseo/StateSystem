#include "stdafx.h"
#include <assert.h>
#include "BaseMemoryPool.h"
#include "EncoderOperator.h"
#include "EncodeCodeFile.h"
//#include "seedx.h"

std::vector< _CallbackOperationFunc >	EncoderOperator::ms_stlVpOperationOrigin; // 기본 연산자 위치를 기억

PtObjectCpp(EncoderOperator);
PT_OPTCPP(EncoderOperator);

void EncoderOperator::init()
{
	if(ms_stlVpOperationOrigin.size() == 0)
	{
		ms_stlVpOperationOrigin.push_back(xor_);
		ms_stlVpOperationOrigin.push_back(add_);
		ms_stlVpOperationOrigin.push_back(not_);
		ms_stlVpOperationOrigin.push_back(sub_);
		ms_stlVpOperationOrigin.push_back(sft_l_);
		ms_stlVpOperationOrigin.push_back(sft_r_);
		ms_stlVpOperationOrigin.push_back(and_);
		ms_stlVpOperationOrigin.push_back(or_);
		ms_stlVpOperationOrigin.push_back(r_xor_);
		ms_stlVpOperationOrigin.push_back(r_add_);
		ms_stlVpOperationOrigin.push_back(r_not_);
		ms_stlVpOperationOrigin.push_back(r_sub_);
		ms_stlVpOperationOrigin.push_back(r_sft_l_);
		ms_stlVpOperationOrigin.push_back(r_sft_r_);
		ms_stlVpOperationOrigin.push_back(r_and_);
		ms_stlVpOperationOrigin.push_back(r_or_);
	}

	UINT32 i=0;
	for(;i<ms_stlVpOperationOrigin.size(); i++)
	{
		m_stlMpOperation[i] = ms_stlVpOperationOrigin[i];
	}

	m_apValue[VALUE_TEMP]	= m_nTemp;

	pEncoderFile_	= NULL;
}

void EncoderOperator::release()
{
	if(pEncoderFile_)
		PT_OFree(pEncoderFile_);
		//delete pEncoderFile_;

	pEncoderFile_ = NULL;
}

EncoderOperator::EncoderOperator(void)
{
	init();
}

void EncoderOperator::update_operation(const STLVParamRef &_rstlVOperationList)
{
	m_stlMpOperation.clear();
	UINT32 i=0;
	for(;i<_rstlVOperationList.size(); i++)
	{
		m_stlMpOperation[i] = ms_stlVpOperationOrigin[_rstlVOperationList[i]];
	}
}

EncoderOperator::~EncoderOperator(void)
{
	release();
}

void EncoderOperator::xor_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) ^ _rstlVParam[2];
}

void EncoderOperator::add_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) + _rstlVParam[2];
}

void EncoderOperator::not_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = ~(_rstlVParam[1]);
}

void EncoderOperator::sub_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) - _rstlVParam[2];
}

void EncoderOperator::sft_l_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.m_nParam			= _rstlVParam[2] & 0x3;
	_rEncoder.m_nBufferShift	= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift	<<= 8;
	_rEncoder.m_nBufferShift	|= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift	<<= _rEncoder.m_nParam;
	_rEncoder.m_nBufferShift	>>= 8;
	_rEncoder.param(_rstlVParam[0]) = (bbyte)(_rEncoder.m_nBufferShift & 0xFF);
}

void EncoderOperator::sft_r_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.m_nParam				= _rstlVParam[2] & 0x3;
	_rEncoder.m_nBufferShift		= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift		<<= 8;
	_rEncoder.m_nBufferShift		|= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift		>>= _rEncoder.m_nParam;
	_rEncoder.param(_rstlVParam[0]) = (bbyte)(_rEncoder.m_nBufferShift & 0xFF);
}

void EncoderOperator::and_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) & _rstlVParam[2];
}

void EncoderOperator::or_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) | _rstlVParam[2];
}


void EncoderOperator::r_xor_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) ^ _rEncoder.param(_rstlVParam[2]);
}

void EncoderOperator::r_add_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) + _rEncoder.param(_rstlVParam[2]);
}

void EncoderOperator::r_not_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = ~_rEncoder.param(_rstlVParam[1]);
}

void EncoderOperator::r_sub_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) - _rEncoder.param(_rstlVParam[2]);
}

void EncoderOperator::r_sft_l_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.m_nParam			= _rEncoder.param(_rstlVParam[2]) & 0x3;
	_rEncoder.m_nBufferShift	= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift	<<= 8;
	_rEncoder.m_nBufferShift	|= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift	<<= _rEncoder.m_nParam;
	_rEncoder.m_nBufferShift	>>= 8;
	_rEncoder.param(_rstlVParam[0]) = (bbyte)(_rEncoder.m_nBufferShift & 0xFF);
}

void EncoderOperator::r_sft_r_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.m_nParam			= _rEncoder.param(_rstlVParam[2]) & 0x3;
	_rEncoder.m_nBufferShift	= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift	<<= 8;
	_rEncoder.m_nBufferShift	|= _rEncoder.param(_rstlVParam[1]);
	_rEncoder.m_nBufferShift	>>= _rEncoder.m_nParam;
	_rEncoder.param(_rstlVParam[0]) = (bbyte)(_rEncoder.m_nBufferShift & 0xFF);
}

void EncoderOperator::r_and_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) & _rEncoder.param(_rstlVParam[2]);
}

void EncoderOperator::r_or_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder)
{
	_rEncoder.param(_rstlVParam[0]) = _rEncoder.param(_rstlVParam[1]) | _rEncoder.param(_rstlVParam[2]);
}

void EncoderOperator::set_input(void *_pBuffer)
{
	m_apValue[VALUE_INPUT] = (bbyte*)_pBuffer;
}

void EncoderOperator::set_output(void *_pBuffer)
{
	m_apValue[VALUE_OUTPUT] = (bbyte*)_pBuffer;
}

void EncoderOperator::set_key(void *_pKey)
{	
	m_apValue[VALUE_KEY] = (bbyte*)_pKey;
	//seed_EncRoundKey(m_cRoundKey, (bbyte*)_pKey);
}

bbyte EncoderOperator::get_length_code(bbyte _nCmd)
{
	if(_nCmd == OP_NOT || _nCmd == OP_RNOT)
		return 3;
	return 4;
}

bbyte EncoderOperator::add_index_(bbyte _nIndex, UINT32 _nDiffer)
{
	bbyte nBase;
	UINT32 nOffset = _nIndex & 0xF;//_nIndex % SIZE_BUFFER;
	nBase = _nIndex - (bbyte)nOffset;

	nOffset += _nDiffer;
	nOffset = nOffset & 0xF;//nOffset % SIZE_BUFFER;
	return nBase + (bbyte)nOffset;
}

// 명령을 실행해 입력과 키로부터 출력값을 계산해 낸다
bool EncoderOperator::run(const STLVParamRef &_rstlVCode)
{
	memset(m_nTemp, 0, SIZE_BUFFER+1);
	if(_rstlVCode.size() < 3)
	{// 최소 코드 길이보다 짧다
		return false;
	}

	STLVParamRef stlVParam;
	bbyte nCmd;
	UINT32 nLength, nCnt = 0;

	stlVParam.resize(3);
	for(int nBCount=0; nBCount < SIZE_BUFFER; nBCount++)
	{
		nCnt = 0;
		while(nCnt < _rstlVCode.size())
		{
			nCmd = _rstlVCode[nCnt];
			nCnt++;
			if(nCmd >= OP_MAX) // 존재하지 않는 명령어이다.
				return false;
			nLength = get_length_code(nCmd);

			if(_rstlVCode.size() < nCnt+nLength-1)
			{// 필요한 코드 길이보다 짧다
				return false;
			}
			UINT32 i=0;
			for(;i<nLength-1; i++)
			{
				if(_rstlVCode[nCnt] >= INDEX_COUNT)
					stlVParam[i] = nBCount;
				else if(nCmd >= OP_RXOR || i != nLength-2)
					stlVParam[i] = add_index_(_rstlVCode[nCnt], nBCount);// 명령의 인덱스는 상대 값이기 때문에 절대값으로 변환한다.
				else
					stlVParam[i] = _rstlVCode[nCnt];
				nCnt++;
			}
			m_stlMpOperation[nCmd](stlVParam, *this);
		}
	}
	return true;
}

// load encoder '_strEncoder'
// set coding function by '_stlVCoder'
// _stlVCoder에 있는 함수만 이용해서 엔코딩하고 디코딩 한다. 모두 순서대로 이용
bool EncoderOperator::set_encoder(const char *_strEncoder, STLVCoderIndex &_stlVCoder)
{
	if(!pEncoderFile_)
		PT_OAlloc(pEncoderFile_, EncodeCodeFile);
		//pEncoderFile_ = new EncodeCodeFile;

	bool bRet		= pEncoderFile_->load(_strEncoder);
	set_encoder(_stlVCoder);

	return bRet;
}

void EncoderOperator::set_encoder(STLVCoderIndex &_stlVCoder)
{
	m_stlVCoderIndex	= _stlVCoder;
	for(int i=0; i<(int)m_stlVCoderIndex.size(); i++)
	{
		if(m_stlVCoderIndex[i] >= pEncoderFile_->get_size_encoder())
			m_stlVCoderIndex[i] = m_stlVCoderIndex[i] % pEncoderFile_->get_size_encoder();
	}
}

const char *EncoderOperator::get_encoder_name(int _nIndex)
{
	return pEncoderFile_->get_encoder_name(_nIndex);
}

int	 EncoderOperator::get_size_encoder()
{
	if(pEncoderFile_)
		return pEncoderFile_->get_size_encoder();
	return 0;
}

// _nSize는 16의 배수여야 한다. 적을 경우 메모리 Over write가 발생한다.
void EncoderOperator::encode(bbyte*_pInOut, UINT32 _nSize)
{
	if(!pEncoderFile_)
		return;

	bbyte abOut[SIZE_BUFFER], *pInput;
	UINT32 nCnt = 0;

	for(;nCnt < _nSize; nCnt+=16)
	{
		int nIndex;

		pInput	= _pInOut + nCnt;
		//seed_Encrypt(pInput, m_cRoundKey);
		set_input(pInput);
		set_output(abOut);

		for(nIndex=0; nIndex<(int)m_stlVCoderIndex.size(); nIndex++)
		{
			const STLVParamRef *pCoder = pEncoderFile_->get_encoder(m_stlVCoderIndex[nIndex]);
			run(*pCoder);
			memcpy(pInput, abOut, SIZE_BUFFER);
		}
	}
}

void EncoderOperator::decode(bbyte*_pInOut, UINT32 _nSize)
{
	if(!pEncoderFile_)
		return;

	bbyte abOut[SIZE_BUFFER], *pInput;
	UINT32 nCnt = 0;

	for(;nCnt < _nSize; nCnt+=16)
	{
		int nIndex, i;
		
		pInput	= _pInOut + nCnt;
		set_input(pInput);
		set_output(abOut);

		for(i=0; i<(int)m_stlVCoderIndex.size(); i++)
		{
			nIndex = ((int)m_stlVCoderIndex.size()-1) - i;
			const STLVParamRef *pCoder = pEncoderFile_->get_decoder(m_stlVCoderIndex[nIndex]);
			run(*pCoder);
			memcpy(pInput, abOut, SIZE_BUFFER);
		}
		//seed_Decrypt(pInput, m_cRoundKey);
	}
}
