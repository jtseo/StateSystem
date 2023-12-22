#include "stdafx.h"
#include "BaseSpaceOrganizer.h"

#define CONSOL_BUILD
#ifdef _DEBUG
#ifndef CONSOL_BUILD
#include "d3dx9math.h"// Add by OJ : 2009-05-18
#include "Util/DebugContext.h"
#endif
#endif

BaseSpaceOrganizer::BaseSpaceOrganizer(int _nWidth, int _nHeight, int _nSizeUnit)
{
	m_nSizeUnit	= _nSizeUnit;

	m_nWidth	= _nWidth;
	m_nHeight	= _nHeight;
	m_pData		= NULL;

	space_reflash(_nWidth, _nHeight);
}

BaseSpaceOrganizer::~BaseSpaceOrganizer(void)
{
	if(m_pData)
		PT_Free(m_pData);
}

int BaseSpaceOrganizer::space_add(int _nWidth, int _nHeight, bbyte*_pData, int _nHash) // 공간 추가
{
	SSource source;
	source.sposBox.nPos[0]	= _nWidth-1;
	source.sposBox.nPos[1]	= _nHeight-1;
	source.nHash	= _nHash;
	source.pData	= _pData;
	m_stlVSource.push_back(source);

	return (int)m_stlVSource.size()-1;
}

int BaseSpaceOrganizer::space_find(int _nHash)
{
	for(unsigned i=0; i<m_stlVSource.size(); i++)
	{
		if(m_stlVSource[i].nHash == _nHash)
			return i;
	}
	return -1;
}

bbyte*BaseSpaceOrganizer::get_data(int _nIndex)
{
	return m_stlVSource[_nIndex].pData;
}

int BaseSpaceOrganizer::get_width(int _nIndex)
{
	return m_stlVSource[_nIndex].sposBox.nPos[0]+1;
}

int BaseSpaceOrganizer::get_height(int _nIndex)
{
	return m_stlVSource[_nIndex].sposBox.nPos[1]+1;
}

bool BaseSpaceOrganizer::space_add_(int _nIndex) // 공간 추가
{
	SBoundBox	sBox;
	sBox.nIndex	= _nIndex;
	sBox.sposBox[0].nPos[0]	= 0;
	sBox.sposBox[0].nPos[1]	= 0;
	sBox.sposBox[1].nPos[0]	= m_stlVSource[_nIndex].sposBox.nPos[0];
	sBox.sposBox[1].nPos[1]	= m_stlVSource[_nIndex].sposBox.nPos[1];

	int nIndexVacuum	= get_fit_box_(sBox);
	if(nIndexVacuum >= 0)
	{
		sBox.sposBox[0].nPos[0]	+= m_stlVsboxVacuum[nIndexVacuum].sposBox[0].nPos[0];
		sBox.sposBox[0].nPos[1]	+= m_stlVsboxVacuum[nIndexVacuum].sposBox[0].nPos[1];
		sBox.sposBox[1].nPos[0]	+= m_stlVsboxVacuum[nIndexVacuum].sposBox[0].nPos[0];
		sBox.sposBox[1].nPos[1]	+= m_stlVsboxVacuum[nIndexVacuum].sposBox[0].nPos[1];
		occupy_(sBox);
		return true;
	}

	do{
		if(m_nWidth <= m_nHeight)
		{
			m_nWidth	+= 128;
		}else{
			m_nHeight	+= 128;
		}
	}while(get_width_(sBox) > m_nWidth
		|| get_height_(sBox) > m_nHeight);

	return false;
}

int BaseSpaceOrganizer::get_size_(int _nIndexSoruce)
{
	int	nWidth	= m_stlVSource[_nIndexSoruce].sposBox.nPos[0];
	int nHeight	= m_stlVSource[_nIndexSoruce].sposBox.nPos[1];
	return nWidth*nHeight;
}

#ifdef _DEBUG
#ifndef CONSOL_BUILD
void BaseSpaceOrganizer::update_reflash()
{
	static unsigned s_nUpdatedCnt	= 1;

	if(m_pData)
		PT_Free(m_pData);

	m_pData	= PT_Alloc(bbyte, m_nWidth*m_nHeight*m_nSizeUnit);
	space_clear();

	std::vector<int>	stlVIndexs;

	int nSizeCur;
	for(unsigned i=0; i<m_stlVSource.size(); i++)
	{
		nSizeCur	= get_size_(i);
		stlVIndexs.push_back(i);
		int j	= (int)stlVIndexs.size()-2;
		for(; j>=0; j--)
		{
			if(nSizeCur > get_size_(stlVIndexs[j]))
			{
				stlVIndexs[j+1]	= stlVIndexs[j];
				stlVIndexs[j]	= i;
			}else{
				break;
			}
		}
	}

	bool bSuccess = true;
	for(unsigned i=0; i<stlVIndexs.size() && i<s_nUpdatedCnt; i++)
	{
		bSuccess	= space_add_(stlVIndexs[i]);
		if(!bSuccess)
			break;
	}

	if(bSuccess)
	{
		s_nUpdatedCnt++;
		if(s_nUpdatedCnt >= stlVIndexs.size())
			s_nUpdatedCnt	= 1;
	}
}
#endif
#endif

void BaseSpaceOrganizer::space_reflash(int _nWidth, int _nHeight)
{	
	if(m_pData)
		PT_Free(m_pData);

	if(_nWidth > 0)
		m_nWidth	= _nWidth;
	if(_nHeight > 0)
		m_nHeight	= _nHeight;

	m_pData	= PT_Alloc(bbyte, m_nWidth*m_nHeight*m_nSizeUnit);
	space_clear();

	std::vector<int>	stlVIndexs;
	
	int nSizeCur;
	for(unsigned i=0; i<m_stlVSource.size(); i++)
	{
		nSizeCur	= get_size_(i);
		stlVIndexs.push_back(i);
		int j	= (int)stlVIndexs.size()-2;
		for(; j>=0; j--)
		{
			if(nSizeCur > get_size_(stlVIndexs[j]))
			{
				stlVIndexs[j+1]	= stlVIndexs[j];
				stlVIndexs[j]	= i;
			}else{
				break;
			}
		}
	}

	bool bSuccess = true;
	for(unsigned i=0; i<stlVIndexs.size(); i++)
	{
		bSuccess	= space_add_(stlVIndexs[i]);
		if(!bSuccess)
			break;
	}

	if(!bSuccess)
		space_reflash(0, 0);
}

void BaseSpaceOrganizer::space_clear() // 초기화
{
	m_stlVsboxVacuum.clear();
	m_stlVsboxOccupied.clear();

	SBoundBox	sBox;
	sBox.sposBox[0].nPos[0]	= 0;
	sBox.sposBox[0].nPos[1]	= 0;
	sBox.sposBox[1].nPos[0]	= m_nWidth-1;
	sBox.sposBox[1].nPos[1]	= m_nHeight-1;
	m_stlVsboxVacuum.push_back(sBox);

	memset(m_pData, 0, m_nWidth*m_nHeight*m_nSizeUnit);
}

bool BaseSpaceOrganizer::space_get(int _nIndex, int *_pbound) // 설정되 공간 위치 리턴
{
	for(unsigned i=0; i<m_stlVsboxOccupied.size(); i++)
	{
		if(_nIndex == m_stlVsboxOccupied[i].nIndex)
		{
			memcpy(_pbound, &m_stlVsboxOccupied[i].sposBox[0].nPos[0], sizeof(int)*4);
			//(*(_pbound+2))++;
			//(*(_pbound+3))++;
			return true;
		}
	}

	return false;
}

int BaseSpaceOrganizer::get_size_(const SBoundBox &_sBound)	// 공간 크기 리턴
{
	int nWidth, nHeight;
	nWidth	= get_width_(_sBound);
	nHeight	= get_height_(_sBound);

	return nWidth*nHeight;
}

bool BaseSpaceOrganizer::is_enough_space_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare) // 공간이 들어 갈 수 있는지 확인
{
	if(get_width_(_sBoundIn) > get_width_(_sBoundCompare))
		return false;

	if(get_height_(_sBoundIn) > get_height_(_sBoundCompare))
		return false;

	return true;
}

int BaseSpaceOrganizer::get_fit_box_(const SBoundBox &_sBoundIn)	// 비어있는 공간에서 들어갈 수 있는 공간 중에 가장 작은 공간 리턴
{
	int nSizeCurrent	= -1;
	int	nIndexCurrent=0;
	
	for(unsigned i=0; i<m_stlVsboxVacuum.size(); i++)
	{
		if(is_enough_space_(_sBoundIn, m_stlVsboxVacuum[i]))
		{
			if(nSizeCurrent < 0 || nSizeCurrent > get_size_(m_stlVsboxVacuum[i]))
			{
				nSizeCurrent	= get_size_(m_stlVsboxVacuum[i]);
				nIndexCurrent	= i;
			}
		}
	}

	if(nSizeCurrent < 0)
		return -1;
	return (int)nIndexCurrent;
}

void BaseSpaceOrganizer::occupy_(const SBoundBox &_sBoundIn) // 들어갈 공간이 확정되고 공간을 리스트에 추가하고 빈공간 또한 다시 계산
{
	for(unsigned i=0; i<m_stlVsboxVacuum.size(); i++)
	{
		if(is_collision_(_sBoundIn, m_stlVsboxVacuum[i]))
		{
			occupy_(_sBoundIn, m_stlVsboxVacuum[i]);
			m_stlVsboxVacuum.erase(m_stlVsboxVacuum.begin()+i);
			i--;
		}
	}

	copy_space_(_sBoundIn);
	organize_vacuum_();
	
	m_stlVsboxOccupied.push_back(_sBoundIn);
}


void BaseSpaceOrganizer::occupy_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare) // 한 공간 안에 공간을 추가하고 빈공간을 계산
{
	std::vector<int>	stlVnMinX, stlVnMaxX, stlVnMinY, stlVnMaxY;

	stlVnMinX.push_back(_sBoundCompare.sposBox[0].nPos[0]);
	stlVnMinY.push_back(_sBoundCompare.sposBox[0].nPos[1]);
	stlVnMaxX.push_back(_sBoundCompare.sposBox[1].nPos[0]);
	stlVnMaxY.push_back(_sBoundCompare.sposBox[1].nPos[1]);

    stlVnMinX.push_back(_sBoundIn.sposBox[0].nPos[0]); stlVnMaxX.push_back(_sBoundIn.sposBox[0].nPos[0]-1);
    stlVnMinY.push_back(_sBoundIn.sposBox[0].nPos[1]); stlVnMaxY.push_back(_sBoundIn.sposBox[0].nPos[1]-1);
    stlVnMaxX.push_back(_sBoundIn.sposBox[1].nPos[0]); stlVnMinX.push_back(_sBoundIn.sposBox[1].nPos[0]+1);
    stlVnMaxY.push_back(_sBoundIn.sposBox[1].nPos[1]); stlVnMinY.push_back(_sBoundIn.sposBox[1].nPos[1]+1);

	for(unsigned i=1; i<stlVnMinX.size(); i++){
		if(stlVnMinX[i] <= stlVnMinX[0])
        {stlVnMinX.erase(stlVnMinX.begin()+i); i--;}
	}
	for(unsigned i=1; i<stlVnMinY.size(); i++){
		if(stlVnMinY[i] <= stlVnMinY[0])
        {stlVnMinY.erase(stlVnMinY.begin()+i); i--;}
	}
	for(unsigned i=1; i<stlVnMaxX.size(); i++){
		if(stlVnMaxX[i] >= stlVnMaxX[0])
        {stlVnMaxX.erase(stlVnMaxX.begin()+i); i--;}
	}
	for(unsigned i=1; i<stlVnMaxY.size(); i++){
		if(stlVnMaxY[i] >= stlVnMaxY[0])
        {stlVnMaxY.erase(stlVnMaxY.begin()+i); i--;}
	}

	std::vector<SBoundBox>	stlVacuum;

	for(unsigned x1=0; x1<stlVnMinX.size(); x1++)
	{
		for(unsigned y1=0; y1<stlVnMinY.size(); y1++)
		{
			for(unsigned x2=0; x2<stlVnMaxX.size(); x2++)
			{
				if(stlVnMinX[x1] >= stlVnMaxX[x2])
					continue;
				for(unsigned y2=0; y2<stlVnMaxY.size(); y2++)
				{
					if(stlVnMinY[y1] >= stlVnMaxY[y2])
						continue;
					
					SBoundBox sBox;
					sBox.sposBox[0].nPos[0]	= stlVnMinX[x1];
					sBox.sposBox[0].nPos[1]	= stlVnMinY[y1];
					sBox.sposBox[1].nPos[0]	= stlVnMaxX[x2];
					sBox.sposBox[1].nPos[1]	= stlVnMaxY[y2];

					if(is_collision_(_sBoundIn, sBox))
						continue;

					bool bInclude = false;
					for(unsigned i=0; i<stlVacuum.size(); i++)
					{
						if(is_in_(sBox, stlVacuum[i]))
						{
							bInclude	= true;
							break;
						}else if(is_in_(stlVacuum[i], sBox))
						{
							stlVacuum.erase(stlVacuum.begin()+i);
							i--;
						}
					}
					if(!bInclude)
						stlVacuum.push_back(sBox);
				}
			}
		}
	}

	for(unsigned i=0; i<stlVacuum.size(); i++)
	{
		m_stlVsboxVacuum.push_back(stlVacuum[i]);
	}
}

bool BaseSpaceOrganizer::organize_vacuum_()
{	
	for(unsigned i=0; i<m_stlVsboxVacuum.size(); i++)
	{
		for(unsigned j=0; j<m_stlVsboxVacuum.size(); j++)
		{
			if(i != j && is_in_(m_stlVsboxVacuum[i], m_stlVsboxVacuum[j]))
			{
				m_stlVsboxVacuum.erase(m_stlVsboxVacuum.begin()+i);
				i--;
				break;
			}
		}
	}

	return false;
}

bool BaseSpaceOrganizer::is_collision_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare) // 공간이 서로 충돌 하는가
{
	if(is_in_(_sBoundCompare, _sBoundIn.sposBox[0]))
		return true;

	if(is_in_(_sBoundCompare, _sBoundIn.sposBox[1]))
		return true;

	SPosition sPos;
	sPos.nPos[0]	= _sBoundIn.sposBox[0].nPos[0];
	sPos.nPos[1]	= _sBoundIn.sposBox[1].nPos[1];
	if(is_in_(_sBoundCompare, sPos))
		return true;

	sPos.nPos[0]	= _sBoundIn.sposBox[1].nPos[0];
	sPos.nPos[1]	= _sBoundIn.sposBox[0].nPos[1];
	if(is_in_(_sBoundCompare, sPos))
		return true;

	sPos.nPos[0]	= _sBoundCompare.sposBox[1].nPos[0];
	sPos.nPos[1]	= _sBoundIn.sposBox[1].nPos[1];
	if(is_in_(_sBoundCompare, sPos)
		&& is_in_(_sBoundIn, sPos))
		return true;

	sPos.nPos[0]	= _sBoundIn.sposBox[1].nPos[0];
	sPos.nPos[1]	= _sBoundCompare.sposBox[1].nPos[1];
	if(is_in_(_sBoundCompare, sPos)
		&& is_in_(_sBoundIn, sPos))
		return true;

	return false;
}

bool BaseSpaceOrganizer::is_in_(const SBoundBox &_sBoundIn, const SPosition &_sPos) // 공간이 완벽하게 들어가는가
{
	if(_sBoundIn.sposBox[0].nPos[0] > _sPos.nPos[0]
		|| _sBoundIn.sposBox[1].nPos[0] < _sPos.nPos[0])
		return false;

	if(_sBoundIn.sposBox[0].nPos[1] > _sPos.nPos[1]
		|| _sBoundIn.sposBox[1].nPos[1] < _sPos.nPos[1])
		return false;
	
	return true;
}

bool BaseSpaceOrganizer::is_in_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare) // 공간이 완벽하게 들어가는가
{
	if(!is_in_(_sBoundCompare, _sBoundIn.sposBox[0]))
		return false;

	if(!is_in_(_sBoundCompare, _sBoundIn.sposBox[1]))
		return false;

	SPosition sPos;
	sPos.nPos[0]	= _sBoundIn.sposBox[0].nPos[0];
	sPos.nPos[1]	= _sBoundIn.sposBox[1].nPos[1];
	if(!is_in_(_sBoundCompare, sPos))
		return false;

	sPos.nPos[0]	= _sBoundIn.sposBox[1].nPos[0];
	sPos.nPos[1]	= _sBoundIn.sposBox[0].nPos[1];
	if(!is_in_(_sBoundCompare, sPos))
		return false;

	return true;
}

int	BaseSpaceOrganizer::get_width_(const SBoundBox &_sBoundIn)
{
	return (_sBoundIn.sposBox[1].nPos[0] - _sBoundIn.sposBox[0].nPos[0] + 1);
}

int	BaseSpaceOrganizer::get_height_(const SBoundBox &_sBoundIn)
{
	return (_sBoundIn.sposBox[1].nPos[1] - _sBoundIn.sposBox[0].nPos[1] + 1);
}

void BaseSpaceOrganizer::copy_space_(const SBoundBox &_sBoundIn) // 공간에 데이터를 카피해 넣는다
{
	bbyte*pSource;

	int	nSizeLine	= m_nWidth * m_nSizeUnit;
	int nSizeLineSource	= get_width_(_sBoundIn) * m_nSizeUnit;
	pSource	= m_stlVSource[_sBoundIn.nIndex].pData;
	for(int y=0; y<=m_stlVSource[_sBoundIn.nIndex].sposBox.nPos[1]; y++)
	{
		int nIndexStart	= y + _sBoundIn.sposBox[0].nPos[1];
		bbyte*pStart	= m_pData + (nSizeLine * nIndexStart + _sBoundIn.sposBox[0].nPos[0] * m_nSizeUnit);
		memcpy(pStart, pSource+nSizeLineSource*y, nSizeLineSource);
	}
}

int	BaseSpaceOrganizer::get_width()
{
	return m_nWidth;
}

int BaseSpaceOrganizer::get_height()
{
	return m_nHeight;
}

bbyte*BaseSpaceOrganizer::get_pimage_data()
{
	return m_pData;
}

int BaseSpaceOrganizer::get_pixel_size()
{
	return m_nSizeUnit;
}
