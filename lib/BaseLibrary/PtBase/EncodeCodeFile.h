#pragma once

#include "BaseFile.h"

class EncodeCodeFile : public BaseFile
{
public:
	PtObjectHeader2(EncodeCodeFile);
	EncodeCodeFile(void);
	~EncodeCodeFile(void);
	void compile(const char *_strFilename);
	bool load(const char *_strFilename);
	bool save(const char *_strFilename);

	int					get_size_encoder();
	const STLVParamRef *get_encoder(int _nIndex);
	const STLVParamRef *get_decoder(int _nIndex);
	const char *get_encoder_name(int _nIndex);

	void clear();
protected:
	void parse_coder_(char *_strName);
	bool parse_coder_line_(char *_strBuffer, STLVParamRef	*_pstlVParamRef);
	int	parse_param_(char *_strName, bbyte*_pnParam);
	void orgarnize_index();

	void add_decoder_auto_(int _nHashKeyEncode);
	void add_coders_extend_(int _nHashKeyEncode, 
		std::map< int, STLVParamRef* > *_pstlMnpEncode, 
		std::map< int, STLVParamRef* > *_pstlMnpDecode);
	bool is_shift_(bbyte _nCmd);
	bool is_extendable_(bbyte _nParam);
	bbyte get_op_inverse_(bbyte _nCmd);

	typedef std::map< int, std::string >	STLMnstrReserve;
	enum{
		REV_INPUT,
		REV_OUTPUT,
		REV_KEY,
		REV_TEMP,
		REV_START,
		REV_END,
		REV_COUNT,
		REV_COMMENT,
		REV_INT,
		REV_CHAR,
		REV_BRACE_OPEN,
		REV_BRACE_CLOSE
	};

	enum{
		RET_ERROR,
		RET_REFERENCE,
		RET_NUMBER
	};

	std::map< int, int >			m_stlMnnReserved;
	STLMnstrReserve					m_stlMnstrReserved;
	STLMnstrReserve					m_stlMnstrEncoder;
	STLMnstrReserve					m_stlMnstrDecoder;

	std::map< int, STLVParamRef* >	m_stlMnEncodes;
	std::map< int, STLVParamRef* >	m_stlMnDecodes;
	std::vector<int>				m_stlVIndexCoder;

	void parse_coder(char *_strName);
	int	m_nLineNumber;
};
