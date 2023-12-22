#pragma once

class BaseSpaceOrganizer
{
public:
	BaseSpaceOrganizer(int _nWidth, int _nHeight, int _nSizeUnit);
	virtual ~BaseSpaceOrganizer(void);

	int space_find(int _nHash);
	int space_add(int _nWidth, int _nHeight, bbyte*_pData, int _nHash); // 공간 추가
	void space_clear(); // 초기화
	bool space_get(int _nIndex, int *_pbound); // 설정되 공간 위치 리턴
	void space_reflash(int _nWidth, int _nHeight);
	int	get_width();
	int get_height();
	int get_pixel_size();
	bbyte*get_pimage_data();
	bbyte*get_data(int _nIndex);
	int get_width(int _nIndex);
	int get_height(int _nIndex);

#ifdef _DEBUG
	void update_reflash();
#endif

	//void reflash_box_pointer(); // ??
protected:
	struct SPosition{
		int nPos[2];
	};
	struct SBoundBox{
		int			nIndex;
		SPosition	sposBox[2]; // min, max
	};

	bool	space_add_(int _nIndex); // 공간 추가
	int		get_size_(int _nIndexSoruce);
	int		get_size_(const SBoundBox &_sBound);	// 공간 크기 리턴
	bool	is_enough_space_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // 공간이 들어 갈 수 있는지 확인
	int		get_fit_box_(const SBoundBox &_sBoundIn);	// 비어있는 공간에서 들어갈 수 있는 공간 중에 가장 작은 공간 리턴
	void	occupy_(const SBoundBox &_sBoundIn); // 들어갈 공간이 확정되고 공간을 리스트에 추가하고 빈공간 또한 다시 계산
	bool	is_collision_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // 공간이 서로 충돌 하는가
	bool	is_in_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // 공간이 완벽하게 들어가는가
	bool	is_in_(const SBoundBox &_sBoundIn, const SPosition &_sPos); // 공간이 완벽하게 들어가는가
	void	occupy_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // 한 공간 안에 공간을 추가하고 비공간을 계산
	bool	organize_vacuum_();

	int		get_width_(const SBoundBox &_sBoundIn);
	int		get_height_(const SBoundBox &_sBoundIn);

	void	copy_space_(const SBoundBox &_sBoundIn); // 공간에 데이터를 카피해 넣는다
	
	struct SSource{
		SPosition	sposBox;
		bbyte*pData;
		int			nHash;
	};
	std::vector<SSource>	m_stlVSource;
	std::vector<SBoundBox>	m_stlVsboxOccupied;
	std::vector<SBoundBox>	m_stlVsboxVacuum;
	bbyte*m_pData;
	int			m_nWidth;
	int			m_nHeight;
	int			m_nSizeUnit;
};
