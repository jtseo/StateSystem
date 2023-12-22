#pragma once
#include "BaseObject.h"

class BaseUIWindow;
class BaseUIWinTexture;
class BaseSpaceOrganizer;
class BaseStatemanager;

class BaseUIManager :
	public BaseObject
{
public:
	BaseUIManager(void);
	virtual ~BaseUIManager(void);

	void release();
	
	int mouse(int _id, int _x, int _y, int _nType);
	void event_m_cast(int _nUIId, int _nHashCaster, int _nOffsetX, int _nOffsetY, const STLVInt &_stlVParam);
	//void draw();
	virtual void update();

	virtual void event_cast(int _nHash, int _nButton, int _nType, int _nIdentifier, const STLVInt &_stlVParam) = 0;
	virtual void sprite_release(void *_pSprite) = 0;
	virtual void *sprite_make(BaseSpaceOrganizer *_pSpaceOrg, void *_pSprite) = 0;
	virtual void sprite_draw(BaseUIWindow *_pWin, void *_pstlSpriteMesh, void *_pSprite, int _nOrder) = 0;
	virtual bool font_create() = 0;
	virtual int font_size_get(int _nFont) = 0;
	virtual int texture_load(BaseUIWindow *_pWin, const char *_strFile) = 0;
	virtual bool draw_text(const Vector4 &_rect, const STLVString &_stlVstrText, int _nFont, int _nColor, int _nOrder, int _nFlag) = 0;

	enum{
		MOUSE_NONE,
		MOUSE_LPUSH,
		MOUSE_LUP,		// keep PUSH+1
		MOUSE_LCLICK,	// keep UP+1
		MOUSE_LDCLICK,
		MOUSE_RPUSH,
		MOUSE_RUP,		// keep PUSH+1
		MOUSE_RCLICK,	// keep UP+1
		MOUSE_RDCLICK,
		MOUSE_MPUSH,
		MOUSE_MUP,		// keep PUSH+1
		MOUSE_MCLICK,	// keep UP+1
		MOUSE_MDCLICK,
		MOUSE_SCROLL
	};

	enum{
		SMS_PUSH,
		SMS_UP,
		SMS_MOVE,
		SMS_OVER,
		SMS_OUT,
		SMS_CLICK,
		SMS_DCLICK,
		SMS_SCROLL
	};

	enum{
		SMS_LEFT,
		SMS_RIGHT,
		SMS_MIDDLE
	};

	virtual void set_size(int _w, int _h);
	virtual void set_pos(int _x, int _y);
	virtual void move_pos(int _x, int _y)=0;
	Vector2 get_pos();
    static int serial_new();
protected:
	
	int m_nx, m_ny, m_nWidth, m_nHeight;

	//=============================================================================================
	// control for windows
protected:
	STLVpVoid		m_stlVpWindows;
public:
	void window_release(BaseUIWindow *_pWin);
	void window_release(int _nHash);
    BaseUIWindow *window_get(int _nIdentifier);

	BaseUIWinTexture *window_create_texture(const char *_strName, int _nIdentifier, const char *_strTexture, int _nWinType, BaseUIWindow *_pWinBase);
	BaseUIWindow *window_create(int _nIdentifier, const char *_strName, int _nHashUI, int _nWinType, int _nPosTypeX, int _nPosTypeY, BaseUIWindow *_pWinRefPos, BaseUIWindow *_pWinBoundBox);

	//BaseUIWindow *window_get(int _nIdentifier, int _nHash);
	BaseUIWindow *window_create_root(const char *_strName, const char *_strTexture, int _nId);
	BaseUIWindow *window_create_root(const char *_strName, int _nId);
	//---------------------------------------------------------------------------------------------
	//=============================================================================================
	// for mouse event manager
protected:
	typedef struct _ui_mouse{
		//int nCurUI;
		int x, y;// current position
		int ox, oy;// old position
		int ex, ey;// event position
		int ofx, ofy; // offset pos
		int sx, sy;// scroll x, y
		int nOldIdentifier;
        int nOldGroupID;
		int	event;
		int oEvent;
		int nIndex;
		int anEventCoord[10]; // event coordinate (first is a size of coordinate)
		int anOEventCoord[10]; // old event coordinate (first is a size of coordinate)
	} ui_mouse;
	//int m_nIdCur;
	int m_nMouseCur;

	std::vector<ui_mouse>	m_stlVMouse;

	//=============================================================================================
	// for Space manager
public:
	BaseSpaceOrganizer *space_get();
	void space_release(BaseSpaceOrganizer *_pSpace);

	static BaseUIManager *get_manager();

	//===========================================================================================
	// control of UIs(List~)
public:
	STLVStlVpVoid *list_get(int _nHash);
protected:
	typedef std::map<int, STLVStlVpVoid *>	STLMnList;
	STLMnList		m_stlMnLists;
	//-------------------------------------------------------------------------------------------

protected:

	std::vector<BaseSpaceOrganizer *>	m_stlVpFreeSpaceOrganizer;

	static BaseUIManager	*m_pManager;
public:
    
    //===========================================================================================
    // ui context
public:
    static STLVInt ui_context_get(BaseDStructureValue *_pdsvContext);
    static void ui_context_add(BaseDStructureValue *_pdsvContext, int _nUIId);
    static BaseUIWindow *ui_context_parent(BaseStateMain *_pStateMain, BaseDStructureValue *_pdsvContext, int _nHash);
    //-------------------------------------------------------------------------------------------
    
    
    //===========================================================================================
    // identifyer link
public:
    void identify_link_set(int _nUIId, int _nStateId)
    {
        m_stlMnIdState[_nUIId] = _nStateId;
    }
    int identify_link_get(int _nUIId)
    {
        STLMnInt::iterator it = m_stlMnIdState.find(_nUIId);
        if(it != m_stlMnIdState.end())
            return it->second;
        return 0;
    }
protected:
    STLMnInt    m_stlMnIdState;
    //-------------------------------------------------------------------------------------------
};
