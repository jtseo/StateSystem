#pragma once

class BaseSpaceOrganizer
{
public:
	BaseSpaceOrganizer(int _nWidth, int _nHeight, int _nSizeUnit);
	virtual ~BaseSpaceOrganizer(void);

	int space_find(int _nHash);
	int space_add(int _nWidth, int _nHeight, bbyte*_pData, int _nHash); // ���� �߰�
	void space_clear(); // �ʱ�ȭ
	bool space_get(int _nIndex, int *_pbound); // ������ ���� ��ġ ����
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

	bool	space_add_(int _nIndex); // ���� �߰�
	int		get_size_(int _nIndexSoruce);
	int		get_size_(const SBoundBox &_sBound);	// ���� ũ�� ����
	bool	is_enough_space_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // ������ ��� �� �� �ִ��� Ȯ��
	int		get_fit_box_(const SBoundBox &_sBoundIn);	// ����ִ� �������� �� �� �ִ� ���� �߿� ���� ���� ���� ����
	void	occupy_(const SBoundBox &_sBoundIn); // �� ������ Ȯ���ǰ� ������ ����Ʈ�� �߰��ϰ� ����� ���� �ٽ� ���
	bool	is_collision_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // ������ ���� �浹 �ϴ°�
	bool	is_in_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // ������ �Ϻ��ϰ� ���°�
	bool	is_in_(const SBoundBox &_sBoundIn, const SPosition &_sPos); // ������ �Ϻ��ϰ� ���°�
	void	occupy_(const SBoundBox &_sBoundIn, const SBoundBox &_sBoundCompare); // �� ���� �ȿ� ������ �߰��ϰ� ������� ���
	bool	organize_vacuum_();

	int		get_width_(const SBoundBox &_sBoundIn);
	int		get_height_(const SBoundBox &_sBoundIn);

	void	copy_space_(const SBoundBox &_sBoundIn); // ������ �����͸� ī���� �ִ´�
	
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
