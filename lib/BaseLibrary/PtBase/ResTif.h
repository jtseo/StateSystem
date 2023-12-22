#pragma once
#include "BaseResResource.h"

typedef unsigned char bbyte;

class ResTif :
	public BaseResResource
{
public:
	ResTif(void);
	virtual ~ResTif(void);

	enum{
		ECARE_RES_TEXTURE	=	10000
	};

	virtual bool Load();
	virtual bool Load2();
	virtual void Clear();

	static ResTif	sm_sample;

	bbyte*get_data();
	int get_width() { return m_nWidth; }
	int get_height() { return m_nHeight; }
protected:
	bool	m_bReturnLoad1;

	bbyte*m_pData;
	int m_nWidth;
	int m_nHeight;
};
