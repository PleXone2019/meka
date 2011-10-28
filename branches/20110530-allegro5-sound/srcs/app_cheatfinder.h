//-----------------------------------------------------------------------------
// MEKA - app_cheatfinder.h
// Cheat Finder - Headers
//-----------------------------------------------------------------------------

#include "app_memview.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

enum t_cheat_finder_value_type
{
	CHEAT_FINDER_VALUE_TYPE_8,
	CHEAT_FINDER_VALUE_TYPE_16,
	CHEAT_FINDER_VALUE_TYPE_24,
	CHEAT_FINDER_VALUE_TYPE_FLAG,
	CHEAT_FINDER_VALUE_TYPE_MAX_,
};

#define CHEAT_FINDER_MATCHES_MAX	(15)

struct t_cheat_finder_match
{
	u8		type;				// t_cheat_finder_value_type
	u16		value_index;		// index of value in memory range, typically address offset, but *8 in the case of matching 1-bit flags
	u32		last_value;
};

struct t_cheat_finder
{
	bool						active;
	t_gui_box*					box;

	t_memory_type				memtype;
	t_cheat_finder_value_type	valuetype;

	t_widget*					w_memtype_buttons[MEMTYPE_MAX_];
	t_widget*					w_valuetype_buttons[CHEAT_FINDER_VALUE_TYPE_MAX_];
	t_widget*					w_custom_value;
	t_widget*					w_reduce_search;
	t_widget*					w_undo_reduce_search;
	t_widget*					w_matches_memedit_buttons[CHEAT_FINDER_MATCHES_MAX];

	bool								reset_state;
	std::vector<t_cheat_finder_match>	matches;
	std::vector<t_cheat_finder_match>	matches_undo;
	std::vector<u32>					addresses_to_highlight_in_memory_editor;

	t_frame								matches_frame;
};

extern t_cheat_finder *		g_CheatFinder_MainInstance;
extern t_list *				g_CheatFinders;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

t_cheat_finder *			CheatFinder_New(bool register_desktop);
void                        CheatFinder_Delete(t_cheat_finder* app);
void                        CheatFinder_SwitchMainInstance(void);

void                        CheatFinders_Update(void);

//-----------------------------------------------------------------------------
