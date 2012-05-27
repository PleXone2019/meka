//-----------------------------------------------------------------------------
// MEKA - gui.h
// Graphical User Interface (GUI) - Headers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Basic type
//-----------------------------------------------------------------------------

struct t_gui_box;

struct v2i
{
    int   x;
    int   y;

	v2i()
	{
		x = y = -1;
	}
	v2i(int _x, int _y)
	{
		Set(_x, _y);
	}

	void Set(int _x, int _y)
	{
		x = _x;
		y = _y;
	}

	v2i operator+(const v2i& rhs) const		{ return v2i(x+rhs.x,y+rhs.y); }
	v2i operator-(const v2i& rhs) const		{ return v2i(x-rhs.x,y-rhs.y); }

	const v2i& operator+=(const v2i& rhs)	{ x+=rhs.x; y+=rhs.y; return *this; }
};

struct t_frame
{
    v2i  pos;
    v2i  size;

	t_frame()
	{
	}

	t_frame(v2i _pos, v2i _size)
	{
		Set(_pos, _size);
	}
	void Set(v2i _pos, v2i _size)
	{
		pos = _pos;
		size = _size;
	}
	void SetPos(v2i _pos)
	{
		pos = _pos;
	}
	void SetPos(int x, int y)
	{
		pos.x = x;
		pos.y = y;
	}
	void SetSize(v2i _size)
	{
		size = _size;
	}
	void SetSize(int x, int y)
	{
		size.x = x;
		size.y = y;
	}
	v2i  GetPosEnd() const
	{
		v2i pe;
		pe.x = pos.x+size.x;
		pe.y = pos.y+size.y;
		return pe;
	}
};

struct DrawCursor
{
	v2i	pos;
	int x_base;
	int y_spacing;
	v2i viewport_min;
	v2i viewport_max;

	DrawCursor(v2i _pos, int font_id = -1);

	void NewLine()
	{
		pos.x = x_base;
		pos.y += y_spacing;
	}
	void HorizontalSeparator();
	void VerticalSeparator();
};

//-----------------------------------------------------------------------------
// Includes other GUI files
//-----------------------------------------------------------------------------

#include "g_action.h"
#include "g_box.h"
#include "g_action.h"
#include "g_mouse.h"
#include "g_update.h"

#include "g_init.h"
#include "g_menu.h"
#include "g_menu_i.h"
#include "g_menu_t.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

#define DOUBLE_CLICK_SPEED          (20)    // FIXME: this is time dependent rendered-frames!!

// Frames
#define GUI_LOOK_FRAME_PAD_HEAD1_X  (6)
#define GUI_LOOK_FRAME_PAD_HEAD2_X  (2)
#define GUI_LOOK_FRAME_PAD_X        (6)
#define GUI_LOOK_FRAME_PAD1_Y       (8)
#define GUI_LOOK_FRAME_PAD2_Y       (2)
#define GUI_LOOK_FRAME_SPACING_X    (7)
#define GUI_LOOK_FRAME_SPACING_Y    (6)
// Texts
#define GUI_LOOK_LINES_SPACING_Y    (0) /* 1 or  2 */

//-----------------------------------------------------------------------------

#define GUI_BOX_MAX                 (128)

enum t_gui_box_type
{
    GUI_BOX_TYPE_STANDARD           = 0,
    GUI_BOX_TYPE_GAME               = 1,
};

enum t_gui_box_flags
{
    GUI_BOX_FLAGS_ACTIVE                    = 0x0001,
    GUI_BOX_FLAGS_DIRTY_REDRAW_ALL_LAYOUT   = 0x0004,
    GUI_BOX_FLAGS_FOCUS_INPUTS_EXCLUSIVE    = 0x0008,   // When set and the box has focus, inputs are exclusive to this box
    GUI_BOX_FLAGS_DELETE                    = 0x0010,
    GUI_BOX_FLAGS_TAB_STOP                  = 0x0020,
};

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void    gui_redraw (void);
void    gui_redraw_everything_now_once (void);

void    GUI_DrawBackground(void);
void    GUI_RelayoutAll(void);

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

typedef void (*t_gui_box_update_handler)();
typedef void (*t_gui_box_destroy_handler)(void *);

struct t_gui_box
{
    t_frame         frame;						// Frame (position & size)
    char *          title;						// Title
    t_gui_box_type  type;                       // Type
    int				flags;                      // Flags (t_gui_box_flags) // FIXME-ENUM
    ALLEGRO_BITMAP *gfx_buffer;					// Graphics buffer holding content render
    t_list *        widgets;                    // Widgets

    // Handlers
    void            (*update)();
    void            (*destroy)(void *user_data);

    // User data
    void *          user_data;
};

struct t_gui_info
{
  bool              must_redraw;
  int               bars_height;
  int               grid_distance;
  int               dirty_x, dirty_y;
  v2i              screen;
  v2i              screen_pad;
};

struct t_gui_mouse
{
    int             x;
    int             x_prev;
    int             y;
    int             y_prev;
    int             buttons;
    int             buttons_prev;
    int             time_since_last_click;
    bool            reset_timer;

    t_gui_focus     focus;
    void *          focus_item;

    int             z_rel;      // Z Relative
    int             z_current;  // Z Current
    int             z_prev;     // Z Previous
};

struct t_gui
{
    t_list *        boxes;
    t_gui_box *     boxes_z_ordered[GUI_BOX_MAX];
    int             boxes_count;
    t_gui_info      info;
    t_gui_mouse     mouse;
};

extern t_gui        gui;

//-----------------------------------------------------------------------------
