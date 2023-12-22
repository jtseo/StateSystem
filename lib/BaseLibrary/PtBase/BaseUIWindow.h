#pragma once
#include "BaseTree.h"

class BaseUIBoard;
class BaseSpaceOrganizer;
class BaseUIManager;

class BaseUIWindow :
	public BaseTree
{
public:
	enum{
		POS_X,
		POS_Y,
		POS_W,
		POS_H
	};

	BaseUIWindow(BaseUIManager *_pManager, const char *_strName, int _nIdentifier, int _nWinType = WIN_TYPE_Base);
	virtual ~BaseUIWindow(void);

	void updated();
	virtual void draw();
	virtual bool draw_update();
	virtual void pos_set(int _x, int _y);
	void pos_set_size(int _w, int _h);
	void pos_set(int _x, int _y, int _w, int _h);
	void pos_move(int _x, int _y);
	void pos_set(Vector4 _rect);
	void pos_zorder_set(int _z);
	void pos_set(int _nPos, int _nPosType, BaseUIWindow *_pWinRef);
	void organizer_create();
	void type_set(int _nWinType);
	int type_get();
	virtual void attach_child(BaseTree *_pChild);	
	void pos_bound_win_set(BaseUIWindow *_pWinBound)
	{
		m_pWinBound	= _pWinBound;
	}
    BaseUIWindow *window_get(int _nID);
	
	bool is_blinded();
	bool is_blind_complete();
	const Vector4 &pos_get();
	
	virtual void pos_size_update();
	virtual void pos_update();
	void pos_offset_set(int _nPosType, int _nOffset);
	void pos_global_get(Vector4 *_prect);
	void pos_ref_release(BaseUIWindow *_pWin, int _nPos);
	void pos_ref_add(BaseUIWindow *_pWinChild, int _nPos);
	void pos_get_size(int _nPos, int *_pnStart, int *_pnEnd);
	void pos_cood_set(const STLVInt &_stlVCoordinate)
	{
		m_stlVEventCod = _stlVCoordinate;
	}

	int mouse(int _id, int _x, int _y, int _nType);
	void event_cast(int _OriginHash, int _nOffsetX, int _nOffsetY, const STLVInt &_stlVIndex);
    
	int identifier_get(){
		return m_nidentifier;
	}

public:
	enum{// window type
		WIN_TYPE_Base,		// it can get base position for child windows
		WIN_TYPE_Linker		// it can't cast event, just hand over to parent window the event when comes up a event.
	};

	enum{// pos type
		POS_Static,
		POS_Dynamic,
		POS_Start,
		POS_End,
		POS_Next,
		POS_Center,
		POS_CenterFix,
		POS_Percent,
		POS_PercentFix,
		POS_MAX
	};

protected:
	BaseUIManager	*m_pManager;
	int				m_nidentifier;// window identifier for StateEventProcess
	int				m_nWinType;	// type of window for event caster, default is WIN_EVT_Caster
	
	STLVInt			m_stlVEventCod;	// event coordinate line, colume
	BaseUIWindow	*m_apPosWinRef[4];	// reference point for use update position
	BaseUIWindow	*m_pWinBound;		// set bound box, If window doesn't appear in bound box, it will not draw contents.
	int				m_anPosType[4];		// type of position (ref. // pos type enum
	int				m_anPosOffset[4];	// append offset after apply the position by pos_type
	STLVpVoid		m_astlVpPosWinChilds[4]; // when this win have updated, should call pos_update in this pointers 
	
	void pos_update_propogate(int _nPos);
	void pos_get_size_child(int _nPos, int *_pnStart, int *_pnEnd);
	
	// ==============================================================
	// control for event cast
public:
	void event_cast_disable(){
		m_bEventCastDisable	= true;
	}
	bool event_cast(){
		return m_bEventCastDisable;
	}
protected:
	bool	m_bEventCastDisable;
	//--------------------------------------------------------------
	// ==============================================================
	// control for window texture and image
public:
	struct board_vertex{
		float	v[3];
		float   c[4];
		float	t[2];
	};
	typedef struct board_bound{
		int	rect[4];
		int nTextureIndex;
	}ST_Board;
	
	BaseSpaceOrganizer *texture_space_get();
	BaseUIWindow	*texture_win();
	int texture_find(int _nHash);
	int texture_add( int w, int h, char *_pData, int _nHash);
	void texture_push(ST_Board);
protected:
	Vector4		m_rect;
	Vector2		m_sizelimit; // first value will be a size limit(witdh and height)
	int			m_nOrder;
	bool		m_bUpdate;
	std::vector<board_vertex>	m_stlVsSpriteMesh;
	std::vector<ST_Board>		m_stlVsBoards;
	BaseSpaceOrganizer			*m_pSpaceOrganizer;	// if this window should control texture resource, it will fill by instance or will be a NULL.

	void	*m_pSprite;

	void board_add_(int *_pnBoard, int _nIndexTexture);
	//--------------------------------------------------------------
};

class BaseUIWinTexture :
	public BaseUIWindow
{
public:
	BaseUIWinTexture(BaseUIManager *_pManager, const char *_strName, int _nIdentifier, int _nTextureIndex, int _nWinType);
	virtual ~BaseUIWinTexture(void);

	virtual void pos_size_update();
	virtual void pos_update();
	virtual void pos_set(int _x, int _y);
	void set_index(int _nIndex);
	bool set_index(const char *_strTextureFile);
	int get_index();
	
	virtual bool draw_update();
protected:
	int m_nTextureIndex;
};

class BaseUIWinText:
	public BaseUIWindow
{
public:
	BaseUIWinText(BaseUIManager *_pManager, const char *_strName, int _nIdentifier, int _nWinType);
	virtual ~BaseUIWinText(void);

	virtual void draw();
	void text_clear();
	void set_text(int _nFont, int _nColor, int _nOrder, int _nFlag);
	void set_color(int _nColor);
	void set_text(int _nIndex, const char *_strText);
	const char *get_text(int _nIndex);
	unsigned get_line_count();

protected:
	STLVString	m_stlVstrText;
	int m_nFont;
	int	m_nColor;
	int m_nFlag;
};
