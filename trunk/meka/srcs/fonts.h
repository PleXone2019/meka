//-----------------------------------------------------------------------------
// MEKA - fonts.h
// Fonts Tools (mostly wrapping Allegro functionalities) - Headers
//-----------------------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

enum t_font_id
{
	FONTID_NONE			= -2,
	FONTID_CUR			= -1,	// NB: Code tends to write -1 directly as an abbreviation
	FONTID_LARGE		= 0,
	FONTID_MEDIUM		= 1,
	FONTID_SMALL		= 2,
	FONTID_PROGGYTINY,
	FONTID_PROGGYSMALL,
	FONTID_PROGGYSQUARE,
	FONTID_PROGGYCLEAN,
	FONTID_PCMONO,
	FONTID_CRISP,
	FONTID_COUNT_,
};

#define         MEKA_FONT_STR_STAR      "\xC2\x80"	// 128 in UTF8
#define         MEKA_FONT_STR_CHECKED   "\xC2\x81"	// 129 in UTF8
#define         MEKA_FONT_STR_ARROW     ">"			// (not using the one stored at 130)

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

struct t_font
{
    t_font_id		id;
    ALLEGRO_FONT *	library_data;
    int				height;
};

extern t_font		Fonts[FONTID_COUNT_];
extern t_font *		FontCurrent;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void    Fonts_Init          (void);
void    Fonts_Close         (void);
void    Fonts_DeclareFont   (t_font_id font_id, ALLEGRO_FONT *library_data);

//-----------------------------------------------------------------------------

void    Font_SetCurrent     (t_font_id font_id);
void    Font_Print          (t_font_id font_id, const char *text, int x, int y, ALLEGRO_COLOR color);
void    Font_PrintCentered  (t_font_id font_id, const char *text, int x, int y, ALLEGRO_COLOR color);
int     Font_Height         (t_font_id font_id = FONTID_CUR);
int     Font_TextLength     (t_font_id font_id, const char *text);

struct FontPrinter
{
	t_font_id		font_id;
	ALLEGRO_COLOR	color;
	
	FontPrinter(t_font_id _font_id);
	FontPrinter(t_font_id _font_id, ALLEGRO_COLOR _color);

	void	Printf(v2i pos, const char* format, ...) const;
};

//-----------------------------------------------------------------------------

