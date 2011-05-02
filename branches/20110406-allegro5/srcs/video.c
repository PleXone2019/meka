//-----------------------------------------------------------------------------
// MEKA - video.c
// Video / Miscellaenous - Code
//-----------------------------------------------------------------------------

#include "shared.h"
#include "blit.h"
#include "blitintf.h"
#include "capture.h"
#include "debugger.h"
#include "fskipper.h"
#include "inputs_i.h"
#include "inputs_t.h"
#include "palette.h"
#include "skin_bg.h"
#include "vdp.h"
#include "video.h"
#include "osd/misc.h"
#include "osd/timer.h"

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

#ifdef ARCH_DOS
extern int    _wait_for_vsync;
#endif

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void    Video_Init (void)
{
    // Allocate buffers
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_RGB_565);
    screenbuffer_1      = al_create_bitmap(MAX_RES_X + 32, MAX_RES_Y + 32);
    screenbuffer_2      = al_create_bitmap(MAX_RES_X + 32, MAX_RES_Y + 32);
    screenbuffer        = screenbuffer_1;
    screenbuffer_next   = screenbuffer_2;

    // Clear variables
    Video.res_x						= 0;
    Video.res_y						= 0;
    Video.page_flipflop				= 0;
    Video.clear_request				= FALSE;
    Video.game_area_x1				= 0;
	Video.game_area_x2				= 0;
	Video.game_area_y1				= 0;
	Video.game_area_y2				= 0;
    Video.driver					= 1;
	Video.refresh_rate_requested	= 0;
	Video.triple_buffering_activated= FALSE;
    fs_page_0 = fs_page_1 = fs_out	= NULL;
}

// CHANGE GRAPHIC MODE AND UPDATE NECESSARY VARIABLES -------------------------
static int     Video_Mode_Change (int driver, int w, int h, int v_w, int v_h, int refresh_rate, int fatal)
{
    // Attempt to avoid unnecessary resolution change (on blitter change)
    static struct
    {
        int driver;
        int w, h, v_w, v_h;
        int refresh_rate;
    } previous_mode = { -1, -1, -1, -1, -1, -1 };
    if (driver == previous_mode.driver && w == previous_mode.w && h == previous_mode.h && v_w == previous_mode.v_w && v_h == previous_mode.v_h && refresh_rate == previous_mode.refresh_rate)
    {
        Video_Mode_Update_Size ();
        return (MEKA_ERR_OK);
    }

    // We must create the larger buffers in the current depth
    // FIXME-BLIT
    if (Blit_Buffer_NativeTemp != NULL)
        al_destroy_bitmap(Blit_Buffer_NativeTemp);
    Blit_Buffer_NativeTemp = al_create_bitmap((MAX_RES_X + 32) * 2, (MAX_RES_Y + 32)*2);

    previous_mode.driver = driver;
    previous_mode.w = w;
    previous_mode.h = h;
    previous_mode.v_w = v_w;
    previous_mode.v_h = v_h;
    previous_mode.refresh_rate = refresh_rate;

    // Set new mode
	// FIXME-ALLEGRO5: Use al_set_new_display_refresh_rate()
    //request_refresh_rate (refresh_rate);
	if (g_display != NULL)
		al_destroy_display(g_display);
	g_display = al_create_display(w, h);
	if (!g_display)
    //if (set_gfx_mode (driver, w, h, v_w, v_h) != 0)
    {
		const char* error = "Unknown error";	// FIXME-ALLEGRO5: was 'allegro_error'
        if (fatal)
            Quit_Msg (Msg_Get (MSG_Error_Video_Mode), w, h, error);
        Msg (MSGT_USER, Msg_Get (MSG_Error_Video_Mode), w, h, error);
        return (MEKA_ERR_FAIL);
    }
    fs_page_0 = NULL;
    fs_page_1 = NULL;
    fs_page_2 = NULL;

#ifdef ARCH_DOS
    // Set the Allegro vsync flag to that VGA scroll do not automatically vsync
    _wait_for_vsync = FALSE;
#endif

    Video.res_x = w;
    Video.res_y = h;
    Video.refresh_rate_requested = refresh_rate;
	Video_Mode_Update_Size ();

    // Update true-color data
    Data_UpdateVideoMode();

    al_rest(100);	// FIXME-ALLEGRO5: What was that line for?

    return (MEKA_ERR_OK);
}

void    Video_Mode_Update_Size (void)
{
    int   x_fact, y_fact;

    Blitters_Get_Factors (&x_fact, &y_fact);

    // Compute game area position to be centered on the screen
    Video.game_area_x1 = (Video.res_x - cur_drv->x_res * x_fact) / 2;
    Video.game_area_y1 = (Video.res_y - cur_drv->y_res * y_fact) / 2;
    Video.game_area_x2 = (Video.res_x - Video.game_area_x1);
    Video.game_area_y2 = (Video.res_y - Video.game_area_y1);
}

void    Video_Clear (void)
{
    // Note: actual clearing will be done in blit.c
    Video.clear_request = TRUE;
}

void    Video_GUI_ChangeVideoMode (int res_x, int res_y, int depth)
{
    t_list *boxes;

    gui_mouse_show(NULL);
    g_Configuration.video_mode_gui_res_x = res_x;
    g_Configuration.video_mode_gui_res_y = res_y;
    g_Configuration.video_mode_gui_depth = depth;
    gui_set_video_mode(res_x, res_y, depth);
    if (Meka_State == MEKA_STATE_GUI)
        Video_Setup_State();
    Skins_Background_Redraw();

    // Fix position
    for (boxes = gui.boxes; boxes != NULL; boxes = boxes->next)
    {
        t_gui_box *box = boxes->elem;;
        gui_box_clip_position(box);
        box->flags |= GUI_BOX_FLAGS_DIRTY_REDRAW;
    }
}

// SWITCH FROM VIDEO MODES ----------------------------------------------------
void    Video_Setup_State (void)
{
    switch (Meka_State)
    {
    case MEKA_STATE_SHUTDOWN:
        {
			al_destroy_display(g_display);
			g_display = NULL;
            //set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);
            break;
        }
    case MEKA_STATE_FULLSCREEN: // FullScreen mode ----------------------------
        {
            int driver;
            //#ifdef ARCH_WIN32
            //   driver = Blitters.current->driver_win;
            //#else
			// FIXME-ALLEGRO5: no video driver
            driver = 0;//Blitters.current->driver;
            //#endif
	
            // FIXME-BLIT

            // Set color depth
			// FIXME-ALLEGRO5: removed call
            //set_color_depth(g_Configuration.video_mode_game_depth);

			Video.triple_buffering_activated = FALSE;
			if (g_Configuration.video_mode_game_triple_buffering)
            {
				assert(0);	// FIXME-ALLEGRO5
                if (Video_Mode_Change(
                        driver,
                        Blitters.current->res_x, Blitters.current->res_y,
                    #ifdef ARCH_WIN32
                        0, 0,
                    #else
                        0, Blitters.current->res_y * 2,
                    #endif
                        Blitters.current->refresh_rate, FALSE) != MEKA_ERR_OK)
                {
                    Meka_State = MEKA_STATE_GUI;
                    Video_Setup_State ();
                    Msg (MSGT_USER, Msg_Get (MSG_Error_Video_Mode_Back_To_GUI));
                    return;
                }
				if (fs_page_0)
				{
					al_destroy_bitmap (fs_page_0);
					fs_page_0 = NULL;
				}
				if (fs_page_1)
				{
					al_destroy_bitmap (fs_page_1);
					fs_page_1 = NULL;
				}
				if (fs_page_2)
				{
					al_destroy_bitmap (fs_page_2);
					fs_page_2 = NULL;
				}

					assert(0);
#if 0 // FIXME-ALLEGRO5
				if (gfx_capabilities & GFX_CAN_TRIPLE_BUFFER)
				{
					// Enable triple buffering
					fs_page_0 = create_video_bitmap (Video.res_x, Video.res_y);
					fs_page_1 = create_video_bitmap (Video.res_x, Video.res_y);
					fs_page_2 = create_video_bitmap (Video.res_x, Video.res_y);
					fs_out = fs_page_1;
					enable_triple_buffer();
					Video.page_flipflop = 0;
					al_set_target_bitmap(fs_page_0);
					al_clear_to_color(Border_Color);
					al_set_target_bitmap(fs_page_1);
					al_clear_to_color(Border_Color);
					al_set_target_bitmap(fs_page_2);
					al_clear_to_color(Border_Color);
					request_video_bitmap(fs_page_0);
					Video.triple_buffering_activated = TRUE;
				}
				else
				{
					// No triple buffering
					// FIXME: We allocated too much VRAM...
					fs_out = al_get_backbuffer(g_display);
					al_set_target_bitmap(fs_out);
					al_clear_to_color(Border_Color);
				}
#endif
            }
			else if (g_Configuration.video_mode_game_page_flipping)
            {
				assert(0);
#if 0 // FIXME-ALLEGRO5: page flipping
                if (Video_Mode_Change (driver,
                    Blitters.current->res_x, Blitters.current->res_y,
#ifdef ARCH_WIN32
                    0, 0,
#else
                    0, Blitters.current->res_y * 2,
#endif
                    Blitters.current->refresh_rate,
                    FALSE) != MEKA_ERR_OK)
                {
                    Meka_State = MEKA_STATE_GUI;
                    Video_Setup_State ();
                    Msg (MSGT_USER, Msg_Get (MSG_Error_Video_Mode_Back_To_GUI));
                    return;
                }
                if (fs_page_0)
				{
                    al_destroy_bitmap (fs_page_0);
					fs_page_0 = NULL;
				}
                if (fs_page_1)
				{
                    al_destroy_bitmap (fs_page_1);
					fs_page_1 = NULL;
				}
                if (fs_page_2)
                {
                    al_destroy_bitmap (fs_page_1);
                    fs_page_2 = NULL;
                }

                fs_page_0 = create_video_bitmap (Video.res_x, Video.res_y);
                fs_page_1 = create_video_bitmap (Video.res_x, Video.res_y);
                Video.page_flipflop = 0;
                fs_out = fs_page_1;
                clear_to_color (fs_page_0, Border_Color);
                clear_to_color (fs_page_1, Border_Color);
                show_video_bitmap (fs_page_0);
#endif // page flipping
            }
            else
            {
                if (Video_Mode_Change (driver,
                    Blitters.current->res_x, Blitters.current->res_y,
                    0, 0,
                    Blitters.current->refresh_rate,
                    FALSE) != MEKA_ERR_OK)
                {
                    Meka_State = MEKA_STATE_GUI;
                    Video_Setup_State ();
                    Msg (MSGT_USER, Msg_Get (MSG_Error_Video_Mode_Back_To_GUI));
                    return;
                }
                fs_out = al_get_backbuffer(g_display);
            }
            Change_Mode_Misc ();
            //Palette_Sync_All ();
            // set_gfx_mode (GFX_TEXT, 0, 0, 0, 0);
        }
        break;
    case MEKA_STATE_GUI: // Interface Mode ------------------------------------
        {
            // Revert to GUI color depth
            // FIXME-DEPTH
	        //set_color_depth(g_Configuration.video_mode_gui_depth);
            //Video_Mode_Change (g_Configuration.video_mode_gui_driver, g_Configuration.video_mode_gui_res_x, g_Configuration.video_mode_gui_res_y, 0, 0, g_Configuration.video_mode_gui_refresh_rate, TRUE);

			// FIXME-ALLEGRO5
			Video_Mode_Change (0, g_Configuration.video_mode_gui_res_x, g_Configuration.video_mode_gui_res_y, 0, 0, g_Configuration.video_mode_gui_refresh_rate, TRUE);

			gui_init_again();
            Change_Mode_Misc();
            //Palette_Sync_All ();

            gui_redraw_everything_now_once();
            gui_mouse_show (gui_buffer);
        }
        break;
    }

	// FIXME-ALLEGRO5: Use ALLEGRO_EVENT_DISPLAY_SWITCH_IN, ALLEGRO_EVENT_DISPLAY_SWITCH_OUT
    /*#ifndef ARCH_DOS
        set_display_switch_callback (SWITCH_IN,  Switch_In_Callback);
        set_display_switch_callback (SWITCH_OUT, Switch_Out_Callback);
    #endif*/

    Inputs_Init_Mouse (); // why? I forgot
}

void    Screen_Save_to_Next_Buffer(void)
{
	al_set_target_bitmap(screenbuffer_next);
	al_draw_bitmap(screenbuffer, 0, 0, 0);
}

void    Screen_Restore_from_Next_Buffer(void)
{
	al_set_target_bitmap(screenbuffer);
	al_draw_bitmap(screenbuffer_next, 0, 0, 0);
}

// REFRESH THE SCREEN ---------------------------------------------------------
// This is called when line == tsms.VDP_Line_End
void    Refresh_Screen(void)
{
    // acquire_bitmap(screen);

//#ifdef ARCH_WIN32
//    Msg (MSGT_DEBUG, "%016I64x , %016I64x", OSD_Timer_GetCyclesCurrent(), OSD_Timer_GetCyclesPerSecond());
//#else
//    Msg (MSGT_DEBUG, "%016llx , %016llx", OSD_Timer_GetCyclesCurrent(), OSD_Timer_GetCyclesPerSecond());
//#endif

    if (fskipper.Show_Current_Frame)
    {
		Capture_Update();

        if (Machine_Pause_Need_To)
            Machine_Pause();

        if (Meka_State == MEKA_STATE_GUI) // GRAPHICAL USER INTERFACE ------------
        {
            gui_update ();

            // Check if we're switching GUI off now
            if (Meka_State != MEKA_STATE_GUI)
            {
                // release_bitmap(screen);
                return;
            }

            // Msg (MSGT_DEBUG, "calling gui_redraw(), screenbuffer=%d", (screenbuffer==screenbuffer_1)?1:2);

            gui_redraw ();

            // Blit GUI screen ------------------------------------------------------
            Blit_GUI ();

            gui_mouse_show (NULL);
        }

        if (Meka_State == MEKA_STATE_FULLSCREEN) // FULLSCREEN ---------------------
        {
            if (opt.Fullscreen_Cursor)
                gui_mouse_show (screenbuffer);

            // Show current FPS -----------------------------------------------------
            if (fskipper.FPS_Display)
            {
                int x, y;
                char s [16];
                sprintf (s, "%d FPS", fskipper.FPS);
                if (cur_drv->id == DRV_GG) { x = 48; y = 24; } else { x = 8; y = 6; }
                Font_Print (F_MIDDLE, screenbuffer, s, x, y, COLOR_WHITE); // In white
                gui_status.timeleft = 0; // Force disabling the current message
            }

            // Blit emulated screen in fullscreen mode ------------------------------
            Blit_Fullscreen ();

            // Disable LightGun cursor until next screen refresh --------------------
            if (opt.Fullscreen_Cursor)
                gui_mouse_show (NULL);
        }

        // Palette update after redraw
        Palette_UpdateAfterRedraw();

        // Clear keypress queue
        Inputs_KeyPressQueue_Clear();

    } // of: if (fskipper.Show_Current_Frame)

    // Draw next image in other buffer --------------------------------------------
    if (machine & MACHINE_PAUSED)
    {
        Screen_Restore_from_Next_Buffer ();
    }
    else
    {
        // Swap buffers
        void *tmp = screenbuffer;
        screenbuffer = screenbuffer_next;
        screenbuffer_next = tmp;
        // Msg (MSGT_DEBUG, "Swap buffer. screenbuffer=%d", screenbuffer==screenbuffer_1?1:2);

        // In debugging mode, copy previously rendered buffer to new one
        // This is so the user always see the current rendering taking place over the previous one
        #ifdef MEKA_Z80_DEBUGGER
            if (Debugger.active)
                Screen_Restore_from_Next_Buffer();
        #endif
    }

    // Ask frame-skipper weither next frame should be drawn or not
    fskipper.Show_Current_Frame = Frame_Skipper();
    //if (fskipper.Show_Current_Frame == FALSE)
    //   Msg (MSGT_USER, "Skip frame!");

    // Update console (under WIN32)
    // #ifdef ARCH_WIN32
    //  ConsoleUpdate();
    // #endif

    // release_bitmap(screen);
}

// SET BORDER COLOR IN VGA MODES ----------------------------------------------
#ifdef ARCH_DOS
void    Video_VGA_Set_Border_Color(u8 idx)
{
    inp(0x3DA);
    outp(0x3C0, 0x31);
    outp(0x3C0, idx);
};
#endif

//-----------------------------------------------------------------------------

