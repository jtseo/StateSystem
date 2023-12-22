#pragma once

class EncodeCodeFile;
class EncoderOperator;
#ifndef bbyte
typedef unsigned char	bbyte;
#endif

typedef std::vector< bbyte >	STLVParamRef;
typedef std::vector< int >	STLVCoderIndex;
typedef void (*_CallbackOperationFunc) (const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);

// 간단한 스크립트를 컴파일하고 실행해서 암호화에 사용한다.
// 각 연산자는 해쉬번호를 가지고 있는데 이 해쉬 번호는 
// 실시간으로 바뀔 수 있다
class EncoderOperator
{
public:
	PtObjectHeader2(EncoderOperator);
	PT_OPTHeader;
public:
	EncoderOperator(void);
	virtual ~EncoderOperator(void);

	enum{
		OP_XOR,
		OP_ADD,
		OP_NOT,
		OP_SUB,
		OP_SFL,
		OP_SFR,
		OP_AND,
		OP_OR,
		OP_RXOR,
		OP_RADD,
		OP_RNOT,
		OP_RSUB,
		OP_RSFL,
		OP_RSFR,
		OP_RAND,
		OP_ROR,
		OP_MAX,
		SIZE_BUFFER		= 0x10,
		INDEX_INPUT		= 0x00,
		INDEX_OUTPUT	= 0x10,
		INDEX_KEY		= 0x20,
		INDEX_TEMP		= 0x30,
		INDEX_COUNT		= 0x40
	};

	int	 get_size_encoder();
	void set_encoder(STLVCoderIndex &_stlVCoder);
	bool set_encoder(const char *_strEncoder, STLVCoderIndex &_stlVCoder);
	void encode(bbyte*_pInOut, UINT32 _nSize);
	void decode(bbyte*_pInOut, UINT32 _nSize);
	const char *get_encoder_name(int _nIndex);

	bool run(const STLVParamRef &_rstlVCode);
	static bbyte get_length_code(bbyte _nCmd);
	void update_operation(const STLVParamRef &_rstlVOperationList);
	void set_input(void *_pBuffer);
	void set_output(void *_pBuffer);
	void set_key(void *_pKey);

	bbyte&param(bbyte _nIndex)
	{
		nBase_ = _nIndex >> 4;//_nIndex / SIZE_BUFFER;
		nOffset_ = _nIndex & 0xF;//_nIndex % SIZE_BUFFER;
		return m_apValue[nBase_][nOffset_];
	}

	bbyte m_nParam;
	int	m_nBufferShift;
private:
	EncodeCodeFile	*pEncoderFile_;
	int nBase_;
	int nOffset_;
protected:
	bbyte add_index_(bbyte _nIndex, UINT32 _nDiffer);
	// numeric functions
	static void xor_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void add_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void not_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void sub_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void sft_l_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void sft_r_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void and_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void or_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);

	// reference functions
	static void r_xor_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void r_add_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void r_not_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void r_sub_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void r_sft_l_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void r_sft_r_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void r_and_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);
	static void r_or_(const STLVParamRef &_rstlVParam, EncoderOperator &_rEncoder);

	enum{
		VALUE_INPUT		= INDEX_INPUT	>> 4,
		VALUE_OUTPUT	= INDEX_OUTPUT	>> 4,
		VALUE_KEY		= INDEX_KEY		>> 4,
		VALUE_TEMP		= INDEX_TEMP	>> 4
	};
	
	STLVCoderIndex	m_stlVCoderIndex;
	bbyte*m_apValue[5];
	bbyte m_nTemp[SIZE_BUFFER+1];
	UINT32 m_cRoundKey[32];
	std::map< int, _CallbackOperationFunc >			m_stlMpOperation; // 실시간으로 변하는 연산자 기억
	static std::vector< _CallbackOperationFunc >	ms_stlVpOperationOrigin; // 기본 연산자 위치를 기억
};
