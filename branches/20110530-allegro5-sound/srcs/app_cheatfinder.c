//-----------------------------------------------------------------------------
// MEKA - app_cheatfinder.c
// Cheat Finder - Code
//-----------------------------------------------------------------------------

#include "shared.h"
#include "app_cheatfinder.h"
#include "app_memview.h"
#include "desktop.h"
#include "g_widget.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

t_cheat_finder *	g_CheatFinder_MainInstance;
t_list *			g_CheatFinders;

static const char*	s_value_type_names[] =
{
	"8",
	"16",
	"24",
	"FLAG",
};
static int			s_value_type_bit_length[] =
{
	8,
	16,
	24,
	1,
};

//-----------------------------------------------------------------------------
// Forward declaration
//-----------------------------------------------------------------------------

static void         CheatFinder_Layout(t_cheat_finder *app, bool setup);
static void			CheatFinder_Update(t_cheat_finder* app);

static void			CheatFinder_ResetMatches(t_cheat_finder* app);
static void			CheatFinder_ReduceMatches(t_cheat_finder* app);
static void			CheatFinder_UndoReduce(t_cheat_finder* app);
static void			CheatFinder_SelectOneMatch(t_cheat_finder* app, int match_index);

static u32			CheatFinder_IndexToAddr(t_cheat_finder* app, const t_cheat_finder_match* match);
static u32			CheatFinder_ReadValue(t_cheat_finder* app, t_memory_range* mem_range, int value_index);

static void			CheatFinder_CallbackMemtypeSelect(t_widget* w);
static void			CheatFinder_CallbackValuetypeSelect(t_widget* w);
static void			CheatFinder_CallbackReset(t_widget* w);
static void			CheatFinder_CallbackReduce(t_widget* w);
static void			CheatFinder_CallbackUndoReduce(t_widget* w);
static void			CheatFinder_CallbackSelectOneMatch(t_widget* w);
static void			CheatFinder_CallbackClose(t_widget* w);

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

t_cheat_finder *	CheatFinder_New(bool register_desktop)
{
	t_cheat_finder* app = new t_cheat_finder();
	list_add(&g_CheatFinders, app);

	// Setup
	app->active = FALSE;

	// Create box
	t_frame frame;
	frame.pos.x     = 437;
	frame.pos.y     = 102;
	frame.size.x    = 330;
	frame.size.y    = 200;
	app->box = gui_box_new(&frame, "Cheat Finder");	// FIXME-LOCALIZATION
	app->box->user_data = app;
	app->box->destroy = (t_gui_box_destroy_handler)CheatFinder_Delete;

	app->memtype			= MEMTYPE_RAM;
	app->valuetype			= CHEAT_FINDER_VALUE_TYPE_8;
	app->reset_state		= true;

	// Register to desktop (applet is disabled by default)
	if (register_desktop)
		Desktop_Register_Box("CHEAT_FINDER", app->box, FALSE, &app->active);

	// Set exclusive inputs flag to avoid messing with emulation
	app->box->flags |= GUI_BOX_FLAGS_FOCUS_INPUTS_EXCLUSIVE;

	// Set tab stop
	app->box->flags |= GUI_BOX_FLAGS_TAB_STOP;

	// Layout
	CheatFinder_Layout(app, TRUE);

	// Return new instance
	return (app);
}

void	CheatFinder_Delete(t_cheat_finder *app)
{
	list_remove(&g_CheatFinders, app);
	free(app);
}

void	CheatFinder_Layout(t_cheat_finder *app, bool setup)
{
	al_set_target_bitmap(app->box->gfx_buffer);
	al_clear_to_color(COLOR_SKIN_WINDOW_BACKGROUND);

	// Add closebox widget
	if (setup)
		widget_closebox_add(app->box, CheatFinder_CallbackClose);

	// Setup Memory sections
	FontPrinter fp(F_MIDDLE);
	DrawCursor dc(v2i(5,0), F_MIDDLE);
	dc.y_spacing += 4;

	fp.Printf(dc.pos+v2i(0,4), "Memory region:");
	dc.pos.x += 90;

	if (setup)
	{
		for (int i = 0; i != MEMTYPE_MAX_; i++)
		{
			t_memory_type memtype = (t_memory_type)i;
			t_memory_range memrange;
			MemoryRange_GetDetails(memtype, &memrange);

			t_frame frame(dc.pos, v2i(30,Font_Height(F_SMALL) + 3));
			app->w_memtype_buttons[memtype] = widget_button_add(app->box, &frame, 1, CheatFinder_CallbackMemtypeSelect, WIDGET_BUTTON_STYLE_SMALL, (const char *)memrange.name, (void*)memtype);
			dc.pos.x += frame.size.x + 2;
		}
	}
	dc.NewLine();
	dc.HorizontalSeparator();

	fp.Printf(dc.pos+v2i(0,4), "Variable type:");
	dc.pos.x += 90;
	if (setup)
	{
		for (int i = 0; i != CHEAT_FINDER_VALUE_TYPE_MAX_; i++)
		{
			t_frame frame(dc.pos, v2i(30,Font_Height(F_SMALL)+3));
			app->w_valuetype_buttons[i] = widget_button_add(app->box, &frame, 1, CheatFinder_CallbackValuetypeSelect, WIDGET_BUTTON_STYLE_SMALL, (const char *)s_value_type_names[i], (void*)i);
			dc.pos.x += frame.size.x + 2;
		}
	}
	dc.NewLine();
	dc.HorizontalSeparator();

	if (setup)
	{
		t_frame frame(dc.pos, v2i(80,Font_Height(F_SMALL)+3));
		widget_button_add(app->box, &frame, 1, (t_widget_callback)CheatFinder_CallbackReset, WIDGET_BUTTON_STYLE_SMALL, "RESET SEARCH");	// FIXME-LOCALIZATION
	}

	app->matches_frame.SetPos(92,dc.pos.y);
	app->matches_frame.SetSize(app->box->frame.size - app->matches_frame.pos);
	al_draw_line(91+0.5f,dc.pos.y-2,91+0.5f,app->box->frame.size.y+1, COLOR_SKIN_WINDOW_SEPARATORS, 0);
	if (setup)
	{
		DrawCursor dc2(v2i(92+5,dc.pos.y+3),F_MIDDLE);
		dc2.NewLine();
		int h = Font_Height(F_SMALL)-2;
		for (int i = 0; i != CHEAT_FINDER_MATCHES_MAX; i++)
		{
			t_frame frame(dc2.pos, v2i(h,h));
			app->w_matches_memedit_buttons[i] = widget_button_add(app->box, &frame, 1, CheatFinder_CallbackSelectOneMatch, WIDGET_BUTTON_STYLE_SMALL, "", (void*)i);
			dc2.NewLine();
		}
	}

	dc.NewLine();
	dc.NewLine();

	fp.Printf(dc.pos+v2i(0,4), "Value:");
	dc.pos.x = 44;
	if (setup)
	{
		t_frame frame(dc.pos, v2i(42,Font_Height(F_SMALL)+3));
		app->w_custom_value = widget_inputbox_add(app->box, &frame, 6, F_MIDDLE, CheatFinder_CallbackReduce);
		widget_inputbox_set_content_type(app->w_custom_value, WIDGET_CONTENT_TYPE_DECIMAL);
	}

	dc.NewLine();
	dc.pos.y += 2;
	if (setup)
	{
		t_frame frame(dc.pos, v2i(80,Font_Height(F_SMALL)+3));
		app->w_reduce_search = widget_button_add(app->box, &frame, 1, (t_widget_callback)CheatFinder_CallbackReduce, WIDGET_BUTTON_STYLE_SMALL, "START");		// FIXME-LOCALIZATION
	}
	dc.NewLine();
	dc.pos.y += 2;
	if (setup)
	{
		t_frame frame(dc.pos, v2i(80,Font_Height(F_SMALL)+3));
		app->w_undo_reduce_search = widget_button_add(app->box, &frame, 1, (t_widget_callback)CheatFinder_CallbackUndoReduce, WIDGET_BUTTON_STYLE_SMALL, "UNDO REDUCE");		// FIXME-LOCALIZATION
		//widget_button_set_grayed_out(app->w_undo_reduce_search, false);
	}
}

void	CheatFinder_Update(t_cheat_finder* app)
{
	if (!app->active)
		return;

	al_set_target_bitmap(app->box->gfx_buffer);

	// If skin has changed, redraw everything
	if (app->box->flags & GUI_BOX_FLAGS_DIRTY_REDRAW_ALL_LAYOUT)
	{
		CheatFinder_Layout(app, FALSE);
		app->box->flags &= ~GUI_BOX_FLAGS_DIRTY_REDRAW_ALL_LAYOUT;
	}

	for (int i = 0; i != MEMTYPE_MAX_; i++)
	{
		widget_set_highlight(app->w_memtype_buttons[i], app->memtype == i);
		widget_button_set_grayed_out(app->w_memtype_buttons[i], !app->reset_state);
	}

	for (int i = 0; i != CHEAT_FINDER_VALUE_TYPE_MAX_; i++)
	{
		widget_set_highlight(app->w_valuetype_buttons[i], app->valuetype == i);
		widget_button_set_grayed_out(app->w_valuetype_buttons[i], !app->reset_state);
	}

	widget_button_set_label(app->w_reduce_search, app->reset_state ? "START" : "REDUCE");	// FIXME-LOCALIZATION
	widget_button_set_grayed_out(app->w_reduce_search, !app->reset_state && app->matches.empty());
	widget_button_set_grayed_out(app->w_undo_reduce_search, app->matches_undo.empty());

	// Always dirty (ok for a developer tool)
	app->box->flags |= GUI_BOX_FLAGS_DIRTY_REDRAW;

	al_draw_filled_rectangle(app->matches_frame.pos.x, app->matches_frame.pos.y, app->matches_frame.pos.x+app->matches_frame.size.x, app->matches_frame.pos.y+app->matches_frame.size.y, COLOR_SKIN_WINDOW_BACKGROUND);

	FontPrinter fp(F_MIDDLE);
	DrawCursor dc(app->matches_frame.pos+v2i(5,4), F_MIDDLE);

	fp.Printf(dc.pos, (app->matches.size()>1) ? "%d matches" : "%d match", app->matches.size());
	dc.NewLine();

	bool displaying_matches = false;
	if (app->matches.size() > 0)
	{
		t_memory_range memrange;
		MemoryRange_GetDetails(app->memtype, &memrange);
		if (app->matches.size() < CHEAT_FINDER_MATCHES_MAX)
		{
			const t_cheat_finder_match* match = &app->matches[0];
			for (int i = 0; i != app->matches.size(); i++, match++)
			{
				if (match->type == CHEAT_FINDER_VALUE_TYPE_FLAG)
					fp.Printf(dc.pos+v2i(12,0), "%s $%0*X bit %d ($%02X)", memrange.name, memrange.addr_hex_length, memrange.addr_start+(match->value_index>>3), match->value_index&7, 1<<(match->value_index&7));
				else
					fp.Printf(dc.pos+v2i(12,0), "%s $%0*X", memrange.name, memrange.addr_hex_length, memrange.addr_start+match->value_index);
				dc.NewLine();
			}
			displaying_matches = true;
		}
		else
		{
			fp.Printf(dc.pos, "Too many matches to display!");
		}
	}
	for (int i = 0; i != CHEAT_FINDER_MATCHES_MAX; i++)
	{
		widget_set_enabled(app->w_matches_memedit_buttons[i], displaying_matches && ((int)app->matches.size()>i));
	}

	// Request memory editor highlight?
	if (displaying_matches)
	{
		app->addresses_to_highlight_in_memory_editor.reserve(app->matches.size());
		for (int i = 0; i != app->matches.size(); i++)
		{
			const t_cheat_finder_match* match = &app->matches[i];
			u32 addr_min = CheatFinder_IndexToAddr(app, match);
			u32 addr_max = addr_min + (s_value_type_bit_length[match->type]+7)/8;
			for (u32 addr = addr_min; addr < addr_max; addr++)
				app->addresses_to_highlight_in_memory_editor.push_back(addr);
		}
	}
	else
	{
		app->addresses_to_highlight_in_memory_editor.clear();
	}
}

void CheatFinder_ResetMatches(t_cheat_finder* app)
{
	app->reset_state = true;
	app->matches.clear();
	app->matches_undo.clear();
}

static u32 CheatFinder_IndexToAddr(t_cheat_finder* app, const t_cheat_finder_match* match)
{
	u32 index = match->value_index;
	u32 addr = (match->type == CHEAT_FINDER_VALUE_TYPE_FLAG) ? (index >> 3) : index;
	return addr;
}

static u32 CheatFinder_ReadValue(t_cheat_finder* app, t_memory_range* memrange, t_cheat_finder_match* match)
{
	u32 addr = CheatFinder_IndexToAddr(app, match);
	
	u32 v;

	switch (match->type)
	{
	case CHEAT_FINDER_VALUE_TYPE_8:
		{
			v = (u32)memrange->ReadByte(addr);
			break;
		}
	case CHEAT_FINDER_VALUE_TYPE_16:
		{
			v = ((u32)memrange->ReadByte(addr) << 0) | ((u32)memrange->ReadByte(addr+1) << 8);
			break;
		}
	case CHEAT_FINDER_VALUE_TYPE_24:
		{
			v = ((u32)memrange->ReadByte(addr) << 0) | ((u32)memrange->ReadByte(addr+1) << 8) | ((u32)memrange->ReadByte(addr+2) << 16);
			break;
		}
	case CHEAT_FINDER_VALUE_TYPE_FLAG:
		{
			v = (u32)memrange->ReadByte(addr);
			if (v & (1 << (match->value_index & 7)))
				v = 1;
			else
				v = 0;
			break;
		}
	}

	return v;
}

void CheatFinder_ReduceMatches(t_cheat_finder* app)
{
	t_memory_range memrange;
	MemoryRange_GetDetails(app->memtype, &memrange);

	if (app->reset_state)
	{
		int value_max = memrange.size * (app->valuetype == CHEAT_FINDER_VALUE_TYPE_FLAG ? 8 : 1);
		if (app->valuetype == CHEAT_FINDER_VALUE_TYPE_16)
			value_max -= 1;
		if (app->valuetype == CHEAT_FINDER_VALUE_TYPE_24)
			value_max -= 2;
		app->matches.resize(value_max);
		for (int index = 0; index != value_max; index++)
		{
			t_cheat_finder_match* match = &app->matches[index];
			match->type = app->valuetype;
			match->value_index = index;
			match->last_value = CheatFinder_ReadValue(app,&memrange,match);
		}
		app->reset_state = false;
	}

	const char* custom_value_text = widget_inputbox_get_value(app->w_custom_value);
	int custom_value;
	if (sscanf(custom_value_text, "%d", &custom_value) != 1)
		return;

	if (app->matches.empty())
		return;

	// Reduce
	t_cheat_finder_match* match = &app->matches[0];

	std::vector<t_cheat_finder_match> matches_reduced;
	matches_reduced.reserve(app->matches.size());
	for (size_t i = 0; i != app->matches.size(); i++, match++)
	{
		const u32 v_old = match->last_value;
		const u32 v_cur = CheatFinder_ReadValue(app, &memrange, match);
		if (v_cur != custom_value)
		{
			// Discard
			continue;
		}
		match->last_value = v_cur;
		matches_reduced.push_back(*match);
	}
	if (app->matches.size() != matches_reduced.size())
	{
		app->matches_undo.swap(app->matches);
		app->matches.swap(matches_reduced);
	}
}

static void	CheatFinder_UndoReduce(t_cheat_finder* app)
{
	if (!app->matches_undo.empty())
	{
		app->matches.swap(app->matches_undo);
		app->matches_undo.clear();
	}
}

static void CheatFinder_SelectOneMatch(t_cheat_finder* app, int match_index)
{
	assert(match_index < (int)app->matches.size());

	const t_cheat_finder_match* match = &app->matches[match_index];
	u32 addr = CheatFinder_IndexToAddr(app, match);

	MemoryViewer_GotoAddress(MemoryViewer_MainInstance, app->memtype, addr);
}

static void CheatFinder_CallbackMemtypeSelect(t_widget* w)
{
	t_cheat_finder* app = (t_cheat_finder*)w->box->user_data;
	app->memtype = (t_memory_type)(int)w->user_data;
}

static void CheatFinder_CallbackValuetypeSelect(t_widget* w)
{
	t_cheat_finder* app = (t_cheat_finder*)w->box->user_data;
	app->valuetype = (t_cheat_finder_value_type)(int)w->user_data;
}

static void CheatFinder_CallbackReset(t_widget* w)
{
	t_cheat_finder* app = (t_cheat_finder*)w->box->user_data;
	CheatFinder_ResetMatches(app);
}

static void CheatFinder_CallbackReduce(t_widget* w)
{
	t_cheat_finder* app = (t_cheat_finder*)w->box->user_data;
	if (w != app->w_reduce_search)
		widget_button_trigger(app->w_reduce_search);
	else
		CheatFinder_ReduceMatches(app);
}

static void CheatFinder_CallbackUndoReduce(t_widget* w)
{
	t_cheat_finder* app = (t_cheat_finder*)w->box->user_data;
	CheatFinder_UndoReduce(app);
}

static void CheatFinder_CallbackSelectOneMatch(t_widget *w)
{
	t_cheat_finder* app = (t_cheat_finder*)w->box->user_data;
	const int match_index = (int)w->user_data;
	CheatFinder_SelectOneMatch(app, match_index);
}

static void	CheatFinder_CallbackClose(t_widget* w)
{
	t_cheat_finder* app = (t_cheat_finder*)w->box->user_data; // Get instance
	if (app == g_CheatFinder_MainInstance)
	{
		CheatFinder_SwitchMainInstance();
	}
	else
	{
		app->active ^= 1;
		gui_box_show(app->box, app->active, TRUE);
		if (!app->active)
		{
			// Flag GUI box for deletion
			app->box->flags |= GUI_BOX_FLAGS_DELETE;
			return;
		}
	}
}

void	CheatFinder_SwitchMainInstance()
{
	t_cheat_finder *app = g_CheatFinder_MainInstance;
	app->active ^= 1;
	gui_box_show(app->box, app->active, TRUE);
	gui_menu_inverse_check(menus_ID.tools, 5);
}

//-----------------------------------------------------------------------------

void	CheatFinders_Update()
{
	for (t_list* apps = g_CheatFinders; apps != NULL; apps = apps->next)
	{
		t_cheat_finder *app = (t_cheat_finder *)apps->elem;
		CheatFinder_Update(app);
	}
}

//-----------------------------------------------------------------------------