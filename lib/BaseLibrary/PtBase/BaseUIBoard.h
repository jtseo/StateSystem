#pragma once
#include "BaseObject.h"

class BaseUIManager;

class BaseUIBoard :
	public BaseObject
{
public:
	BaseUIBoard(BaseUIManager *_pManager, int x, int y, int w, int h, int _nTexture);
	virtual ~BaseUIBoard(void);

	void draw();
	int mouse(int _x, int _y, int _nType, int _nIdentifier, int _nGroupID);
//protected:
	int m_x, m_y, m_w, m_h;
	int m_nTexture;

	int m_count;
	int m_gap_h;
	
	int	m_nHash;
	BaseUIManager	*m_pManager;
	
	char *m_pData;

	typedef struct __stUIBoardText__{
		char	strText[1024];
		int		color;
		int		range;
		int		flag;
		void	*pFont;
	} ST_UIBoardText;

	typedef std::vector<ST_UIBoardText>	STLVUIBoardText;
	STLVUIBoardText m_stlVText;
};
