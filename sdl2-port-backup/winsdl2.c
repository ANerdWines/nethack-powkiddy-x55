/* NetHack 5.0  winsdl2.c */
/* SDL2 window port for Powkiddy X55 (JELOS) */
/* Copyright (c) 2024 - see CLAUDE.md for project info */
/* NetHack may be freely redistributed. See license for details. */

#include "hack.h"
#include "dlb.h"
#include "func_tab.h"
#include "winsdl2.h"
#include <string.h>
#include <stdarg.h>

/* Forward declarations */
static void sdl2_delay_output(void);
static int sdl2_command_menu(void);
static void debug_log_init(void);
static void debug_log_write(const char *fmt, ...);

/* Global state */
SDL2_DisplayDesc *sdl2Display = NULL;
SDL2_WinDesc *sdl2wins[SDL2_MAXWIN] = {0};
static winid BASE_WINDOW = WIN_ERR;
static const char *sdl2_pending_extcmd = NULL;  /* For command menu -> extended cmd */

/* NetHack color palette */
SDL_Color sdl2_colors[CLR_MAX] = {
    [CLR_BLACK]          = {0x00, 0x00, 0x00, 0xFF},
    [CLR_RED]            = {0xAA, 0x00, 0x00, 0xFF},
    [CLR_GREEN]          = {0x00, 0xAA, 0x00, 0xFF},
    [CLR_BROWN]          = {0xAA, 0x55, 0x00, 0xFF},
    [CLR_BLUE]           = {0x00, 0x00, 0xAA, 0xFF},
    [CLR_MAGENTA]        = {0xAA, 0x00, 0xAA, 0xFF},
    [CLR_CYAN]           = {0x00, 0xAA, 0xAA, 0xFF},
    [CLR_GRAY]           = {0xAA, 0xAA, 0xAA, 0xFF},
    [NO_COLOR]           = {0xAA, 0xAA, 0xAA, 0xFF},
    [CLR_ORANGE]         = {0xFF, 0xAA, 0x00, 0xFF},
    [CLR_BRIGHT_GREEN]   = {0x55, 0xFF, 0x55, 0xFF},
    [CLR_YELLOW]         = {0xFF, 0xFF, 0x55, 0xFF},
    [CLR_BRIGHT_BLUE]    = {0x55, 0x55, 0xFF, 0xFF},
    [CLR_BRIGHT_MAGENTA] = {0xFF, 0x55, 0xFF, 0xFF},
    [CLR_BRIGHT_CYAN]    = {0x55, 0xFF, 0xFF, 0xFF},
    [CLR_WHITE]          = {0xFF, 0xFF, 0xFF, 0xFF},
};

/* Forward declarations for stubs */
static void sdl2_player_selection(void);
static void sdl2_askname(void);
static void sdl2_get_nh_event(void);
static void sdl2_suspend_nhwindows(const char *str);
static void sdl2_resume_nhwindows(void);
static void sdl2_nhbell(void);
static int sdl2_doprev_message(void);
static void sdl2_update_inventory(int arg);
static void sdl2_mark_synch(void);
static void sdl2_wait_synch(void);
static void sdl2_cliparound(int x, int y);
static void sdl2_putmixed(winid window, int attr, const char *str);
static void sdl2_display_file(const char *fname, boolean complain);
static void sdl2_number_pad(int state);
static void sdl2_outrip(winid window, int how, time_t when);
static void sdl2_preference_update(const char *pref);
static char *sdl2_getmsghistory(boolean init);
static void sdl2_putmsghistory(const char *msg, boolean is_restoring);
static boolean sdl2_can_suspend(void);
static char sdl2_message_menu(char let, int how, const char *mesg);
static win_request_info *sdl2_ctrl_nhwindow(winid window, int request,
                                            win_request_info *wri);

/* Window procs structure */
struct window_procs sdl2_procs = {
    WPID(sdl2),
    (WC_COLOR | WC_HILITE_PET | WC_ASCII_MAP | WC_INVERSE),
    (WC2_FULLSCREEN | WC2_STATUSLINES),
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  /* has_color */
    sdl2_init_nhwindows,
    sdl2_player_selection,
    sdl2_askname,
    sdl2_get_nh_event,
    sdl2_exit_nhwindows,
    sdl2_suspend_nhwindows,
    sdl2_resume_nhwindows,
    sdl2_create_nhwindow,
    sdl2_clear_nhwindow,
    sdl2_display_nhwindow,
    sdl2_destroy_nhwindow,
    sdl2_curs,
    sdl2_putstr,
    sdl2_putmixed,
    sdl2_display_file,
    sdl2_start_menu,
    sdl2_add_menu,
    sdl2_end_menu,
    sdl2_select_menu,
    sdl2_message_menu,
    sdl2_mark_synch,
    sdl2_wait_synch,
#ifdef CLIPPING
    sdl2_cliparound,
#endif
#ifdef POSITIONBAR
    sdl2_update_positionbar,
#endif
    sdl2_print_glyph,
    sdl2_raw_print,
    sdl2_raw_print_bold,
    sdl2_nhgetch,
    sdl2_nh_poskey,
    sdl2_nhbell,
    sdl2_doprev_message,
    sdl2_yn_function,
    sdl2_getlin,
    sdl2_get_ext_cmd,
    sdl2_number_pad,
    sdl2_delay_output,
#ifdef CHANGE_COLOR
    sdl2_change_color,
#ifdef MACOS9
    sdl2_change_background,
    sdl2_set_font_name,
#endif
    sdl2_get_color_string,
#endif
    sdl2_outrip,
    sdl2_preference_update,
    sdl2_getmsghistory,
    sdl2_putmsghistory,
    sdl2_status_init,
    sdl2_status_finish,
    sdl2_status_enablefield,
    sdl2_status_update,
    sdl2_can_suspend,
    sdl2_update_inventory,
    sdl2_ctrl_nhwindow,
};

/* Stub implementations for optional/unused functions */
void sdl2_update_positionbar(char *features) { /* no-op */ }
void sdl2_change_color(int color, long rgb, int reverse) { /* no-op */ }
void sdl2_change_background(int white_or_black) { /* no-op */ }
short sdl2_set_font_name(winid window, char *fontname) { return 0; }
char *sdl2_get_color_string(void) { return ""; }
static void sdl2_delay_output(void) { SDL_Delay(50); }

static void sdl2_player_selection(void) {
    int result;
    debug_log_write("player_selection: randomall=%d initrole=%d initrace=%d initgend=%d initalign=%d\n",
                    flags.randomall, flags.initrole, flags.initrace, flags.initgend, flags.initalign);
    debug_log_write("player_selection: plname='%s' pl_character='%s'\n", svp.plname, svp.pl_character);
    /* Use generic player selection via menus */
    result = genl_player_setup(SDL2_ROWS);
    debug_log_write("player_selection: genl_player_setup returned %d\n", result);
    debug_log_write("player_selection: after setup - initrole=%d initrace=%d initgend=%d initalign=%d\n",
                    flags.initrole, flags.initrace, flags.initgend, flags.initalign);
}

static void sdl2_askname(void) {
    sdl2_getlin("Who are you? ", svp.plname);
}

static void sdl2_get_nh_event(void) {
    /* Process any pending SDL events */
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            /* Handle quit gracefully */
            program_state.done_hup = TRUE;
        }
    }
}

static void sdl2_suspend_nhwindows(const char *str) { /* no-op on embedded */ }
static void sdl2_resume_nhwindows(void) { /* no-op */ }

static void sdl2_nhbell(void) {
    /* Could play a sound, for now just flash screen briefly */
    SDL_SetRenderDrawColor(sdl2Display->renderer, 255, 255, 255, 255);
    SDL_RenderClear(sdl2Display->renderer);
    SDL_RenderPresent(sdl2Display->renderer);
    SDL_Delay(50);
    sdl2_refresh();
}

static int sdl2_doprev_message(void) {
    int scroll = 0;
    int ch;
    int i, y;
    int max_scroll;
    char buf[BUFSZ];

    if (!sdl2Display || sdl2Display->msg_history_count == 0) {
        return 0;
    }

    max_scroll = sdl2Display->msg_history_count - (SDL2_ROWS - 2);
    if (max_scroll < 0) max_scroll = 0;

    while (1) {
        /* Clear and render history */
        SDL_SetRenderDrawColor(sdl2Display->renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl2Display->renderer);

        sdl2_render_text(0, 0, "-- Message History (B to close) --",
                        CLR_WHITE, ATR_BOLD);

        y = 2;
        for (i = 0; i < sdl2Display->msg_history_count && y < SDL2_ROWS - 1; i++) {
            int idx;
            if (i < scroll) continue;

            /* Calculate index in circular buffer (oldest first) */
            idx = (sdl2Display->msg_history_head - sdl2Display->msg_history_count + i
                   + SDL2_MSG_HISTORY_SIZE) % SDL2_MSG_HISTORY_SIZE;

            sdl2_render_text(0, y, sdl2Display->msg_history[idx], CLR_GRAY, ATR_NONE);
            y++;
        }

        snprintf(buf, sizeof(buf), "-- %d/%d --",
                scroll + 1, sdl2Display->msg_history_count);
        sdl2_render_text(0, SDL2_ROWS - 1, buf, CLR_GRAY, ATR_NONE);

        SDL_RenderPresent(sdl2Display->renderer);

        ch = sdl2_nhgetch();

        if (ch == '\033' || ch == ' ' || ch == '\r') {
            break;
        }
        if (ch == 'k' || ch == '<') {
            if (scroll > 0) scroll--;
        }
        if (ch == 'j' || ch == '>') {
            if (scroll < max_scroll) scroll++;
        }
    }

    sdl2_refresh();
    return 0;
}

static void sdl2_update_inventory(int arg) {
    /* TODO: persistent inventory update */
}

static void sdl2_mark_synch(void) { /* no-op */ }

static void sdl2_wait_synch(void) {
    sdl2_refresh();
}

static void sdl2_cliparound(int x, int y) {
    /* No scrolling needed for 80-column display */
}

static void sdl2_putmixed(winid window, int attr, const char *str) {
    /* Use generic implementation */
    genl_putmixed(window, attr, str);
}

static void sdl2_display_file(const char *fname, boolean complain) {
    dlb *f;
    char buf[BUFSZ];
    winid win;

    f = dlb_fopen(fname, "r");
    if (!f) {
        if (complain)
            pline("Cannot open %s.", fname);
        return;
    }

    win = sdl2_create_nhwindow(NHW_TEXT);
    while (dlb_fgets(buf, BUFSZ, f)) {
        char *nl = strchr(buf, '\n');
        if (nl) *nl = '\0';
        sdl2_putstr(win, ATR_NONE, buf);
    }
    dlb_fclose(f);
    sdl2_display_nhwindow(win, TRUE);
    sdl2_destroy_nhwindow(win);
}

static void sdl2_number_pad(int state) {
    /* Track number pad mode if needed */
}

static void sdl2_outrip(winid window, int how, time_t when) {
    /* Use generic RIP display */
    genl_outrip(window, how, when);
}

static void sdl2_preference_update(const char *pref) {
    /* Handle preference changes */
}

static int sdl2_msg_history_iter = 0;

static char *sdl2_getmsghistory(boolean init) {
    static char buf[BUFSZ];
    int idx;

    if (init) {
        sdl2_msg_history_iter = 0;
    }

    if (!sdl2Display || sdl2_msg_history_iter >= sdl2Display->msg_history_count) {
        return NULL;
    }

    /* Get message from circular buffer (oldest first) */
    idx = (sdl2Display->msg_history_head - sdl2Display->msg_history_count
           + sdl2_msg_history_iter + SDL2_MSG_HISTORY_SIZE) % SDL2_MSG_HISTORY_SIZE;
    strcpy(buf, sdl2Display->msg_history[idx]);
    sdl2_msg_history_iter++;
    return buf;
}

static void sdl2_putmsghistory(const char *msg, boolean is_restoring UNUSED) {
    if (!sdl2Display || !msg || !*msg) return;

    /* Store in circular buffer */
    strncpy(sdl2Display->msg_history[sdl2Display->msg_history_head],
            msg, BUFSZ - 1);
    sdl2Display->msg_history[sdl2Display->msg_history_head][BUFSZ - 1] = '\0';

    sdl2Display->msg_history_head =
        (sdl2Display->msg_history_head + 1) % SDL2_MSG_HISTORY_SIZE;

    if (sdl2Display->msg_history_count < SDL2_MSG_HISTORY_SIZE) {
        sdl2Display->msg_history_count++;
    }
}

static boolean sdl2_can_suspend(void) {
    return FALSE;
}

static char sdl2_message_menu(char let, int how, const char *mesg) {
    return genl_message_menu(let, how, mesg);
}

static win_request_info *sdl2_ctrl_nhwindow(winid window, int request,
                                            win_request_info *wri) {
    return NULL;
}

/*
 * Initialization
 */
void
sdl2_init_nhwindows(int *argcp, char **argv)
{
    int i;

    /* Allocate display descriptor */
    sdl2Display = (SDL2_DisplayDesc *) alloc(sizeof(SDL2_DisplayDesc));
    memset(sdl2Display, 0, sizeof(SDL2_DisplayDesc));

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        raw_print("SDL2: Failed to initialize SDL");
        raw_printf("SDL Error: %s", SDL_GetError());
        nh_terminate(EXIT_FAILURE);
    }

    /* Initialize SDL_ttf */
    if (TTF_Init() < 0) {
        raw_print("SDL2: Failed to initialize SDL_ttf");
        raw_printf("TTF Error: %s", TTF_GetError());
        nh_terminate(EXIT_FAILURE);
    }

    /* Create fullscreen window */
    sdl2Display->window = SDL_CreateWindow(
        "NetHack",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SDL2_SCREEN_WIDTH, SDL2_SCREEN_HEIGHT,
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN
    );
    if (!sdl2Display->window) {
        raw_printf("SDL2: Failed to create window: %s", SDL_GetError());
        nh_terminate(EXIT_FAILURE);
    }

    /* Create renderer */
    sdl2Display->renderer = SDL_CreateRenderer(
        sdl2Display->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!sdl2Display->renderer) {
        /* Fall back to software renderer */
        sdl2Display->renderer = SDL_CreateRenderer(
            sdl2Display->window, -1, SDL_RENDERER_SOFTWARE
        );
    }
    if (!sdl2Display->renderer) {
        raw_printf("SDL2: Failed to create renderer: %s", SDL_GetError());
        nh_terminate(EXIT_FAILURE);
    }

    /* Load font - try several paths */
    const char *font_paths[] = {
        "DejaVuSansMono.ttf",
        "/storage/roms/ports/nethack/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        NULL
    };

    for (i = 0; font_paths[i]; i++) {
        sdl2Display->font = TTF_OpenFont(font_paths[i], 20);
        if (sdl2Display->font) break;
    }
    if (!sdl2Display->font) {
        raw_print("SDL2: Failed to load font");
        raw_printf("TTF Error: %s", TTF_GetError());
        nh_terminate(EXIT_FAILURE);
    }

    /* Get font metrics */
    TTF_SizeText(sdl2Display->font, "M", &sdl2Display->font_width,
                 &sdl2Display->font_height);

    /* Open game controller if available */
    for (i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            sdl2Display->controller = SDL_GameControllerOpen(i);
            if (sdl2Display->controller) break;
        }
    }

    /* Initialize map buffer */
    for (i = 0; i < ROWNO; i++) {
        int j;
        for (j = 0; j < COLNO; j++) {
            sdl2Display->map[i][j].ch = ' ';
            sdl2Display->map[i][j].color = CLR_GRAY;
            sdl2Display->map[i][j].attr = ATR_NONE;
            sdl2Display->map[i][j].dirty = TRUE;
        }
    }

    /* Clear screen */
    SDL_SetRenderDrawColor(sdl2Display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl2Display->renderer);
    SDL_RenderPresent(sdl2Display->renderer);

    /* Create base window - but don't mark it active */
    BASE_WINDOW = sdl2_create_nhwindow(NHW_TEXT);
    /* Don't set active = TRUE here, it would cause stale text to render */

    iflags.window_inited = TRUE;
}

void
sdl2_exit_nhwindows(const char *str)
{
    int i;

    if (str && *str) {
        sdl2_raw_print(str);
        SDL_Delay(2000);
    }

    /* Destroy all windows */
    for (i = 0; i < SDL2_MAXWIN; i++) {
        if (sdl2wins[i]) {
            sdl2_destroy_nhwindow(i);
        }
    }

    /* Cleanup SDL */
    if (sdl2Display) {
        if (sdl2Display->controller)
            SDL_GameControllerClose(sdl2Display->controller);
        if (sdl2Display->font)
            TTF_CloseFont(sdl2Display->font);
        if (sdl2Display->renderer)
            SDL_DestroyRenderer(sdl2Display->renderer);
        if (sdl2Display->window)
            SDL_DestroyWindow(sdl2Display->window);
        free(sdl2Display);
        sdl2Display = NULL;
    }

    TTF_Quit();
    SDL_Quit();

    iflags.window_inited = FALSE;
}

/*
 * Window management
 */
winid
sdl2_create_nhwindow(int type)
{
    winid win;
    SDL2_WinDesc *newwin;

    /* Find free slot */
    for (win = 0; win < SDL2_MAXWIN; win++) {
        if (!sdl2wins[win]) break;
    }
    if (win >= SDL2_MAXWIN) {
        panic("sdl2_create_nhwindow: too many windows");
        return WIN_ERR;
    }

    /* Allocate window descriptor */
    newwin = (SDL2_WinDesc *) alloc(sizeof(SDL2_WinDesc));
    memset(newwin, 0, sizeof(SDL2_WinDesc));

    newwin->type = type;
    newwin->active = FALSE;

    /* Set default geometry based on type */
    switch (type) {
    case NHW_MESSAGE:
        newwin->offx = 0;
        newwin->offy = SDL2_MSG_ROW;
        newwin->width = SDL2_COLS;
        newwin->height = SDL2_MSG_ROWS;
        break;
    case NHW_STATUS:
        newwin->offx = 0;
        newwin->offy = SDL2_STATUS_ROW;
        newwin->width = SDL2_COLS;
        newwin->height = SDL2_STATUS_ROWS;
        break;
    case NHW_MAP:
        newwin->offx = 0;
        newwin->offy = SDL2_MAP_ROW;
        newwin->width = COLNO;
        newwin->height = ROWNO;
        break;
    case NHW_MENU:
    case NHW_TEXT:
        newwin->offx = 0;
        newwin->offy = 0;
        newwin->width = SDL2_COLS;
        newwin->height = SDL2_ROWS;
        break;
    default:
        newwin->offx = 0;
        newwin->offy = 0;
        newwin->width = SDL2_COLS;
        newwin->height = SDL2_ROWS;
        break;
    }

    sdl2wins[win] = newwin;
    return win;
}

void
sdl2_clear_nhwindow(winid window)
{
    SDL2_WinDesc *cw;
    int i, j;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];

    if (cw->type == NHW_MAP) {
        /* Clear map buffer */
        for (i = 0; i < ROWNO; i++) {
            for (j = 0; j < COLNO; j++) {
                sdl2Display->map[i][j].ch = ' ';
                sdl2Display->map[i][j].color = CLR_GRAY;
                sdl2Display->map[i][j].dirty = TRUE;
            }
        }
    } else if (cw->type == NHW_MESSAGE) {
        sdl2Display->toplines[0] = '\0';
        sdl2Display->toplin = TOPLINE_EMPTY;
    } else if (cw->type == NHW_STATUS) {
        sdl2Display->status_line1[0] = '\0';
        sdl2Display->status_line2[0] = '\0';
    }

    /* Clear text data if any */
    if (cw->data) {
        for (i = 0; i <= cw->maxrow; i++) {
            if (cw->data[i])
                free(cw->data[i]);
        }
        free(cw->data);
        cw->data = NULL;
    }
    if (cw->datlen) {
        free(cw->datlen);
        cw->datlen = NULL;
    }

    /* Clear menu items if any */
    while (cw->mlist) {
        sdl2_menu_item *item = cw->mlist;
        cw->mlist = item->next;
        if (item->str) free(item->str);
        free(item);
    }
    cw->mlast = NULL;
    cw->nitems = 0;

    cw->curx = cw->cury = 0;
    cw->maxrow = cw->maxcol = 0;
}

void
sdl2_display_nhwindow(winid window, boolean blocking)
{
    SDL2_WinDesc *cw;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];

    /* Only TEXT/MENU windows use the active flag for overlay rendering */
    if (cw->type == NHW_TEXT || cw->type == NHW_MENU) {
        cw->active = TRUE;
    }

    sdl2_refresh();

    if (blocking) {
        /* Wait for keypress */
        sdl2_nhgetch();
        /* Deactivate overlay after blocking display */
        if (cw->type == NHW_TEXT || cw->type == NHW_MENU) {
            cw->active = FALSE;
            sdl2_refresh();  /* Refresh to show map again */
        }
    }
}

void
sdl2_destroy_nhwindow(winid window)
{
    SDL2_WinDesc *cw;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];

    /* Deactivate before clearing/destroying */
    cw->active = FALSE;

    sdl2_clear_nhwindow(window);

    free(cw);
    sdl2wins[window] = NULL;
}

void
sdl2_curs(winid window, int x, int y)
{
    SDL2_WinDesc *cw;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];

    /* Convert to 0-based */
    cw->curx = x - 1;
    cw->cury = y;
}

/*
 * Output functions
 */
void
sdl2_putstr(winid window, int attr, const char *str)
{
    SDL2_WinDesc *cw;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];

    switch (cw->type) {
    case NHW_MESSAGE:
        /* Store in toplines */
        strncpy(sdl2Display->toplines, str, BUFSZ - 1);
        sdl2Display->toplines[BUFSZ - 1] = '\0';
        sdl2Display->toplin = TOPLINE_NON_EMPTY;
        /* Also store in message history */
        if (str && *str) {
            sdl2_putmsghistory(str, FALSE);
        }
        break;

    case NHW_STATUS:
        /* Store status lines - we'll combine them for display */
        if (cw->cury == 0) {
            strncpy(sdl2Display->status_line1, str, BUFSZ - 1);
            sdl2Display->status_line1[BUFSZ - 1] = '\0';
        } else if (cw->cury == 1) {
            strncpy(sdl2Display->status_line2, str, BUFSZ - 1);
            sdl2Display->status_line2[BUFSZ - 1] = '\0';
        }
        /* Don't render here - sdl2_refresh will handle it */
        break;

    case NHW_MAP:
        /* Direct output at cursor position */
        sdl2_render_text(cw->offx + cw->curx, cw->offy + cw->cury,
                         str, CLR_GRAY, attr);
        break;

    case NHW_MENU:
    case NHW_TEXT:
        /* Append to text data */
        if (!cw->data) {
            cw->data = (char **) alloc(SDL2_ROWS * sizeof(char *));
            cw->datlen = (int *) alloc(SDL2_ROWS * sizeof(int));
            memset(cw->data, 0, SDL2_ROWS * sizeof(char *));
            memset(cw->datlen, 0, SDL2_ROWS * sizeof(int));
        }
        if (cw->maxrow < SDL2_ROWS - 1) {
            int len = strlen(str);
            cw->data[cw->maxrow] = (char *) alloc(len + 1);
            strcpy(cw->data[cw->maxrow], str);
            cw->datlen[cw->maxrow] = len;
            if (len > cw->maxcol) cw->maxcol = len;
            cw->maxrow++;
        }
        break;
    }
}

void
sdl2_raw_print(const char *str)
{
    if (!str) return;

    if (sdl2Display && sdl2Display->renderer) {
        /* Render at bottom of screen */
        sdl2_render_text(0, SDL2_ROWS - 1, str, CLR_WHITE, ATR_NONE);
        sdl2_refresh();
    } else {
        /* Fallback to stderr before SDL init */
        fprintf(stderr, "%s\n", str);
    }
}

void
sdl2_raw_print_bold(const char *str)
{
    if (!str) return;

    if (sdl2Display && sdl2Display->renderer) {
        sdl2_render_text(0, SDL2_ROWS - 1, str, CLR_WHITE, ATR_BOLD);
        sdl2_refresh();
    } else {
        fprintf(stderr, "%s\n", str);
    }
}

void
sdl2_print_glyph(winid window, coordxy x, coordxy y,
                 const glyph_info *glyphinfo, const glyph_info *bkglyphinfo)
{
    int ch, color;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    /* Get character and color from glyph info */
    ch = glyphinfo->ttychar;
    color = glyphinfo->gm.sym.color;

    if (color < 0 || color >= CLR_MAX)
        color = CLR_GRAY;
    if (ch < 32 || ch > 126)
        ch = '?';

    /* Store in map buffer */
    if (y >= 0 && y < ROWNO && x >= 0 && x < COLNO) {
        sdl2Display->map[y][x].ch = ch;
        sdl2Display->map[y][x].color = color;
        sdl2Display->map[y][x].attr = ATR_NONE;
        sdl2Display->map[y][x].dirty = TRUE;
    }
}

/*
 * Rendering helpers
 */
void
sdl2_render_text(int x, int y, const char *str, int color, int attr)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect dstrect;
    SDL_Color fg;

    if (!sdl2Display || !sdl2Display->renderer || !sdl2Display->font)
        return;
    if (!str || !*str)
        return;

    /* Get color */
    if (color < 0 || color >= CLR_MAX)
        color = CLR_GRAY;
    fg = sdl2_colors[color];

    /* Handle bold by brightening */
    if (attr & ATR_BOLD) {
        fg.r = (fg.r + 255) / 2;
        fg.g = (fg.g + 255) / 2;
        fg.b = (fg.b + 255) / 2;
    }

    /* Render text */
    surface = TTF_RenderText_Blended(sdl2Display->font, str, fg);
    if (!surface) return;

    texture = SDL_CreateTextureFromSurface(sdl2Display->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    /* Calculate position */
    dstrect.x = x * sdl2Display->font_width;
    dstrect.y = y * sdl2Display->font_height;
    SDL_QueryTexture(texture, NULL, NULL, &dstrect.w, &dstrect.h);

    SDL_RenderCopy(sdl2Display->renderer, texture, NULL, &dstrect);
    SDL_DestroyTexture(texture);
}

void
sdl2_render_cell(int x, int y, int ch, int color, int attr)
{
    char buf[2] = {(char)ch, '\0'};
    sdl2_render_text(x, y, buf, color, attr);
}

void
sdl2_refresh(void)
{
    int i, j;
    SDL2_WinDesc *cw;
    boolean have_overlay = FALSE;

    if (!sdl2Display || !sdl2Display->renderer)
        return;

    /* Clear screen */
    SDL_SetRenderDrawColor(sdl2Display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl2Display->renderer);

    /* Check if there's an active overlay (TEXT/MENU window with content) */
    for (i = 0; i < SDL2_MAXWIN; i++) {
        cw = sdl2wins[i];
        if (cw && cw->active && (cw->type == NHW_TEXT || cw->type == NHW_MENU)) {
            if (cw->data && cw->maxrow > 0) {
                have_overlay = TRUE;
                break;
            }
        }
    }

    /* Only render map/message when there's no overlay taking focus */
    if (!have_overlay) {
        /* Render status at top (row 0) - combine both lines */
        {
            char combined_status[BUFSZ * 2];
            combined_status[0] = '\0';

            /* Start with line 2 (HP, Dlvl - most critical info first) */
            if (sdl2Display->status_line2[0]) {
                strncpy(combined_status, sdl2Display->status_line2,
                        sizeof(combined_status) - 1);
            }

            /* Append stats from line 1 if there's room */
            if (sdl2Display->status_line1[0]) {
                int len = strlen(combined_status);
                /* Extract just the stats part (skip name/title) */
                char *stats = strstr(sdl2Display->status_line1, "St:");
                if (stats && len < 60) {
                    /* Add separator and stats */
                    strncat(combined_status, " ", sizeof(combined_status) - len - 1);
                    strncat(combined_status, stats, sizeof(combined_status) - len - 2);
                }
            }

            combined_status[sizeof(combined_status) - 1] = '\0';
            if (combined_status[0]) {
                sdl2_render_text(0, SDL2_STATUS_ROW, combined_status,
                                 CLR_WHITE, ATR_NONE);
            }
        }

        /* Render map */
        for (i = 0; i < ROWNO; i++) {
            for (j = 0; j < COLNO; j++) {
                sdl2_map_cell *cell = &sdl2Display->map[i][j];
                if (cell->ch != ' ') {
                    sdl2_render_cell(j, i + SDL2_MAP_ROW, cell->ch,
                                     cell->color, cell->attr);
                }
            }
        }

        /* Render messages at bottom (rows 22-24) */
        /* Show recent messages from history, oldest first */
        {
            int msg_row = SDL2_MSG_ROW;
            int start_idx, msg_idx;
            int num_to_show = SDL2_MSG_ROWS;

            if (num_to_show > sdl2Display->msg_history_count)
                num_to_show = sdl2Display->msg_history_count;

            /* Calculate starting index in circular buffer */
            start_idx = sdl2Display->msg_history_head - num_to_show;
            if (start_idx < 0)
                start_idx += SDL2_MSG_HISTORY_SIZE;

            /* Render messages oldest to newest */
            for (i = 0; i < num_to_show; i++) {
                msg_idx = (start_idx + i) % SDL2_MSG_HISTORY_SIZE;
                if (sdl2Display->msg_history[msg_idx][0]) {
                    sdl2_render_text(0, msg_row + i,
                                     sdl2Display->msg_history[msg_idx],
                                     CLR_GRAY, ATR_NONE);
                }
            }

            /* Also show current message (toplines) on the last row if different */
            if (sdl2Display->toplines[0]) {
                sdl2_render_text(0, SDL2_MSG_ROW + SDL2_MSG_ROWS - 1,
                                 sdl2Display->toplines, CLR_WHITE, ATR_NONE);
            }
        }
    } else {
        /* Render overlay TEXT/MENU window */
        for (i = 0; i < SDL2_MAXWIN; i++) {
            cw = sdl2wins[i];
            if (!cw || !cw->active) continue;

            if ((cw->type == NHW_TEXT || cw->type == NHW_MENU) && cw->data) {
                for (j = 0; j < cw->maxrow && j < SDL2_ROWS; j++) {
                    if (cw->data[j]) {
                        sdl2_render_text(cw->offx, cw->offy + j,
                                         cw->data[j], CLR_GRAY, ATR_NONE);
                    }
                }
            }
        }
    }

    /* Present */
    SDL_RenderPresent(sdl2Display->renderer);
}

/*
 * Input functions - Phase 2: Enhanced gamepad support
 */

/* Direction keys for vi-style movement:
 *   y k u
 *   h . l
 *   b j n
 */
static int
sdl2_direction_key(int up, int down, int left, int right)
{
    if (up && left) return 'y';      /* up-left */
    if (up && right) return 'u';     /* up-right */
    if (down && left) return 'b';    /* down-left */
    if (down && right) return 'n';   /* down-right */
    if (up) return 'k';              /* up */
    if (down) return 'j';            /* down */
    if (left) return 'h';            /* left */
    if (right) return 'l';           /* right */
    return 0;                        /* no direction */
}

/* Get direction from analog stick */
static int
sdl2_stick_direction(void)
{
    int x, y;
    int up = 0, down = 0, left = 0, right = 0;

    if (!sdl2Display->controller)
        return 0;

    x = SDL_GameControllerGetAxis(sdl2Display->controller,
                                  SDL_CONTROLLER_AXIS_LEFTX);
    y = SDL_GameControllerGetAxis(sdl2Display->controller,
                                  SDL_CONTROLLER_AXIS_LEFTY);

    /* Apply deadzone */
    if (x > -SDL2_STICK_DEADZONE && x < SDL2_STICK_DEADZONE)
        x = 0;
    if (y > -SDL2_STICK_DEADZONE && y < SDL2_STICK_DEADZONE)
        y = 0;

    /* Determine directions */
    if (x < -SDL2_STICK_THRESHOLD) left = 1;
    if (x > SDL2_STICK_THRESHOLD) right = 1;
    if (y < -SDL2_STICK_THRESHOLD) up = 1;
    if (y > SDL2_STICK_THRESHOLD) down = 1;

    return sdl2_direction_key(up, down, left, right);
}

/* Get direction from D-pad state */
static int
sdl2_dpad_direction(void)
{
    int up = 0, down = 0, left = 0, right = 0;

    if (!sdl2Display->controller)
        return 0;

    up = SDL_GameControllerGetButton(sdl2Display->controller,
                                     SDL_CONTROLLER_BUTTON_DPAD_UP);
    down = SDL_GameControllerGetButton(sdl2Display->controller,
                                       SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    left = SDL_GameControllerGetButton(sdl2Display->controller,
                                       SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    right = SDL_GameControllerGetButton(sdl2Display->controller,
                                        SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

    return sdl2_direction_key(up, down, left, right);
}

/* Update direction held state and check for repeats */
static int
sdl2_check_direction_repeat(void)
{
    Uint32 now = SDL_GetTicks();
    int dir_key;
    int i;

    /* Get current direction from D-pad or stick */
    dir_key = sdl2_dpad_direction();
    if (!dir_key)
        dir_key = sdl2_stick_direction();

    /* Update held state */
    sdl2Display->dir_held[SDL2_DIR_UP] =
        SDL_GameControllerGetButton(sdl2Display->controller,
                                    SDL_CONTROLLER_BUTTON_DPAD_UP) ||
        (SDL_GameControllerGetAxis(sdl2Display->controller,
                                   SDL_CONTROLLER_AXIS_LEFTY) < -SDL2_STICK_THRESHOLD);
    sdl2Display->dir_held[SDL2_DIR_DOWN] =
        SDL_GameControllerGetButton(sdl2Display->controller,
                                    SDL_CONTROLLER_BUTTON_DPAD_DOWN) ||
        (SDL_GameControllerGetAxis(sdl2Display->controller,
                                   SDL_CONTROLLER_AXIS_LEFTY) > SDL2_STICK_THRESHOLD);
    sdl2Display->dir_held[SDL2_DIR_LEFT] =
        SDL_GameControllerGetButton(sdl2Display->controller,
                                    SDL_CONTROLLER_BUTTON_DPAD_LEFT) ||
        (SDL_GameControllerGetAxis(sdl2Display->controller,
                                   SDL_CONTROLLER_AXIS_LEFTX) < -SDL2_STICK_THRESHOLD);
    sdl2Display->dir_held[SDL2_DIR_RIGHT] =
        SDL_GameControllerGetButton(sdl2Display->controller,
                                    SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ||
        (SDL_GameControllerGetAxis(sdl2Display->controller,
                                   SDL_CONTROLLER_AXIS_LEFTX) > SDL2_STICK_THRESHOLD);

    /* Check if any direction is newly pressed or should repeat */
    for (i = 0; i < SDL2_DIR_COUNT; i++) {
        if (sdl2Display->dir_held[i]) {
            if (sdl2Display->dir_press_time[i] == 0) {
                /* Newly pressed */
                sdl2Display->dir_press_time[i] = now;
                sdl2Display->dir_last_repeat[i] = now;
                sdl2Display->last_dir_key = dir_key;
                return dir_key;
            } else {
                /* Check for repeat */
                Uint32 held_time = now - sdl2Display->dir_press_time[i];
                Uint32 since_repeat = now - sdl2Display->dir_last_repeat[i];

                if (held_time > SDL2_REPEAT_DELAY &&
                    since_repeat > SDL2_REPEAT_RATE) {
                    sdl2Display->dir_last_repeat[i] = now;
                    sdl2Display->last_dir_key = dir_key;
                    return dir_key;
                }
            }
        } else {
            /* Released */
            sdl2Display->dir_press_time[i] = 0;
        }
    }

    return 0;  /* No repeat this frame */
}

int
sdl2_nhgetch(void)
{
    SDL_Event event;
    int dir_key;

    while (1) {
        /* Process all pending events */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                return '\033';

            case SDL_KEYDOWN:
                /* Handle keyboard for testing/debugging */
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: return '\033';
                case SDLK_RETURN: return '\r';
                case SDLK_SPACE: return ' ';
                case SDLK_UP: return 'k';
                case SDLK_DOWN: return 'j';
                case SDLK_LEFT: return 'h';
                case SDLK_RIGHT: return 'l';
                case SDLK_PERIOD: return '.';
                case SDLK_COMMA: return ',';
                case SDLK_LESS: return '<';
                case SDLK_GREATER: return '>';
                case SDLK_COLON: return ':';
                case SDLK_SEMICOLON: return ';';
                default:
                    if (event.key.keysym.sym >= SDLK_a &&
                        event.key.keysym.sym <= SDLK_z) {
                        int c = event.key.keysym.sym;
                        /* Handle shift for capital letters */
                        if (event.key.keysym.mod & KMOD_SHIFT)
                            c = c - 'a' + 'A';
                        return c;
                    }
                    if (event.key.keysym.sym >= SDLK_0 &&
                        event.key.keysym.sym <= SDLK_9) {
                        return event.key.keysym.sym;
                    }
                    break;
                }
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                /* Face buttons */
                switch (event.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_A:
                    return '.';  /* rest/wait */
                case SDL_CONTROLLER_BUTTON_B:
                    return '\033';  /* cancel/escape */
                case SDL_CONTROLLER_BUTTON_X:
                    return ',';  /* pick up */
                case SDL_CONTROLLER_BUTTON_Y:
                    return 's';  /* search */

                /* Menu buttons */
                case SDL_CONTROLLER_BUTTON_BACK:
                    return 'i';  /* inventory */
                case SDL_CONTROLLER_BUTTON_START:
                    {
                        /* Show command menu */
                        int cmd = sdl2_command_menu();
                        if (cmd == -1) {
                            /* Special: trigger #quit extended command */
                            sdl2_pending_extcmd = "quit";
                            sdl2_refresh();
                            return '#';
                        } else if (cmd) {
                            sdl2_refresh();  /* Restore display */
                            return cmd;
                        }
                        sdl2_refresh();  /* Restore display if cancelled */
                    }
                    break;

                /* Shoulder buttons */
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    return '<';  /* go up stairs */
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    return '>';  /* go down stairs */

                /* Stick clicks */
                case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                    return 'g';  /* prefix for "go" (move until something) */
                case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                    return ':';  /* look at floor */

                /* D-pad is handled entirely by the repeat system below */
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    /* Don't handle here - let repeat system manage it */
                    break;

                default:
                    break;
                }
                break;

            case SDL_CONTROLLERDEVICEADDED:
                /* Controller connected */
                if (!sdl2Display->controller) {
                    sdl2Display->controller =
                        SDL_GameControllerOpen(event.cdevice.which);
                }
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                /* Controller disconnected */
                if (sdl2Display->controller &&
                    event.cdevice.which ==
                    SDL_JoystickInstanceID(
                        SDL_GameControllerGetJoystick(sdl2Display->controller))) {
                    SDL_GameControllerClose(sdl2Display->controller);
                    sdl2Display->controller = NULL;
                }
                break;
            }
        }

        /* Check for direction repeat (held D-pad/stick) */
        if (sdl2Display->controller) {
            dir_key = sdl2_check_direction_repeat();
            if (dir_key)
                return dir_key;
        }

        SDL_Delay(16);  /* ~60fps */
    }
}

int
sdl2_nh_poskey(coordxy *x, coordxy *y, int *mod)
{
    /* For now, just return character input */
    return sdl2_nhgetch();
}

/* Debug log file for tracing input */
static FILE *debug_log = NULL;

static void debug_log_init(void) {
    if (!debug_log) {
        debug_log = fopen("/storage/roms/ports/nethack/debug.log", "a");
        if (debug_log) {
            fprintf(debug_log, "--- Debug session started ---\n");
            fflush(debug_log);
        }
    }
}

static void debug_log_write(const char *fmt, ...) {
    va_list args;
    debug_log_init();
    if (debug_log) {
        va_start(args, fmt);
        vfprintf(debug_log, fmt, args);
        va_end(args);
        fflush(debug_log);
    }
}

char
sdl2_yn_function(const char *query, const char *resp, char def)
{
    char ch;
    char prompt_buf[BUFSZ * 2];

    debug_log_write("yn_function called: query='%s' resp='%s' def='%c'(%d)\n",
                    query ? query : "(null)",
                    resp ? resp : "(null)",
                    def ? def : '?', (int)def);

    /* Build prompt with gamepad hints */
    if (resp && strchr(resp, '?')) {
        /* Inventory/item selection prompt */
        snprintf(prompt_buf, sizeof(prompt_buf), "%s (Y=menu X=all)", query);
    } else if (resp && strchr(resp, 'y') && strchr(resp, 'n')) {
        /* Yes/no prompt */
        snprintf(prompt_buf, sizeof(prompt_buf), "%s (A=yes B=no)", query);
    } else if (!resp || !*resp) {
        /* No specific responses - likely character selection [ynaq] */
        /* Check if query contains [ynaq] pattern */
        if (strstr(query, "[ynaq]") || strstr(query, "for you?")) {
            snprintf(prompt_buf, sizeof(prompt_buf), "%s (A=yes B=no X=random)", query);
        } else {
            strncpy(prompt_buf, query, sizeof(prompt_buf) - 1);
            prompt_buf[sizeof(prompt_buf) - 1] = '\0';
        }
    } else {
        strncpy(prompt_buf, query, sizeof(prompt_buf) - 1);
        prompt_buf[sizeof(prompt_buf) - 1] = '\0';
    }

    /* Display the prompt with hints */
    sdl2_putstr(WIN_MESSAGE, ATR_NONE, prompt_buf);
    sdl2_refresh();

    while (1) {
        ch = sdl2_nhgetch();
        debug_log_write("  nhgetch returned: '%c' (%d)\n", ch ? ch : '?', (int)ch);

        /* Map gamepad buttons to useful responses */
        if (ch == '.') {
            /* A button -> 'y' for yes */
            if (resp) {
                /* If only y/n (no other options), A = yes */
                if (strcmp(resp, "yn") == 0 || strcmp(resp, "yn ") == 0 ||
                    strcmp(resp, "ynq") == 0) {
                    return 'y';
                }
                /* If 'n' is available (like ynaq prompts), return 'n' to choose manually */
                if (strchr(resp, 'n')) {
                    return 'n';
                }
            } else {
                /* No resp (character selection) - A = yes */
                return 'y';
            }
            return def;
        }
        if (ch == 's') {
            /* Y button -> '?' to show menu/help */
            ch = '?';
        }
        if (ch == ',') {
            /* X button -> '*' for inventory, 'a' for character selection */
            if (!resp || !*resp) {
                return 'a';  /* All random for character selection */
            }
            ch = '*';
        }

        /* Check if response is valid */
        if (!resp || !*resp) {
            /* Any response accepted - but map ESC to 'n' for character selection */
            if (ch == '\033') {
                ch = 'n';  /* B button = no, let me choose manually */
            }
            debug_log_write("  yn_function returning (any accepted): '%c' (%d)\n", ch ? ch : '?', (int)ch);
            return ch;
        }
        if (strchr(resp, ch)) {
            debug_log_write("  yn_function returning (valid): '%c'\n", ch);
            return ch;
        }
        if (ch == '\033') {
            /* ESC/B button - return quit/no/default */
            if (resp && strchr(resp, 'q')) {
                debug_log_write("  yn_function returning (ESC->q): 'q'\n");
                return 'q';
            }
            if (resp && strchr(resp, 'n')) {
                debug_log_write("  yn_function returning (ESC->n): 'n'\n");
                return 'n';
            }
            debug_log_write("  yn_function returning (ESC->def): '%c' (%d)\n", def ? def : '?', (int)def);
            return def;
        }
        if (ch == '\r' || ch == ' ') {
            debug_log_write("  yn_function returning (enter/space->def): '%c' (%d)\n", def ? def : '?', (int)def);
            return def;
        }
    }
}

/*
 * On-screen keyboard for text input
 */
static const char *osk_rows[] = {
    "1234567890-=",
    "qwertyuiop[]",
    "asdfghjkl;'\\",
    "zxcvbnm,./` ",
    NULL
};
static const char *osk_rows_shift[] = {
    "!@#$%^&*()_+",
    "QWERTYUIOP{}",
    "ASDFGHJKL:\"|",
    "ZXCVBNM<>?~ ",
    NULL
};

#define OSK_ROWS 4
#define OSK_COLS 12

/* Simple input for OSK - doesn't trigger command menu */
static int
sdl2_osk_input(void)
{
    SDL_Event event;

    while (1) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                return '\033';

            case SDL_CONTROLLERBUTTONDOWN:
                switch (event.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_A: return 'A';      /* type */
                case SDL_CONTROLLER_BUTTON_B: return 'B';      /* backspace */
                case SDL_CONTROLLER_BUTTON_X: return 'X';      /* space */
                case SDL_CONTROLLER_BUTTON_Y: return 'Y';      /* shift */
                case SDL_CONTROLLER_BUTTON_START: return '\r'; /* done */
                case SDL_CONTROLLER_BUTTON_BACK: return '\033';/* cancel */
                case SDL_CONTROLLER_BUTTON_DPAD_UP: return 'k';
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return 'j';
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return 'h';
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return 'l';
                default: break;
                }
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_RETURN: return '\r';
                case SDLK_ESCAPE: return '\033';
                case SDLK_BACKSPACE: return 'B';
                case SDLK_UP: return 'k';
                case SDLK_DOWN: return 'j';
                case SDLK_LEFT: return 'h';
                case SDLK_RIGHT: return 'l';
                default: break;
                }
                break;
            }
        }
        SDL_Delay(16);
    }
}

void
sdl2_getlin(const char *prompt, char *bufp)
{
    int cursor_x = 0, cursor_y = 0;
    int buf_pos = 0;
    int shifted = 0;
    int ch;
    int i, j;
    char display_buf[BUFSZ];
    SDL_Rect rect;
    SDL_Surface *surface;
    SDL_Texture *texture;
    int cell_w, cell_h, start_x, start_y;

    /* Calculate keyboard layout to fit screen */
    cell_w = (SDL2_SCREEN_WIDTH - 80) / OSK_COLS;  /* Leave margins */
    cell_h = 50;
    start_x = 40;
    start_y = SDL2_SCREEN_HEIGHT - (OSK_ROWS * cell_h) - 80;

    bufp[0] = '\0';

    while (1) {
        const char **rows = shifted ? osk_rows_shift : osk_rows;

        /* Clear screen */
        SDL_SetRenderDrawColor(sdl2Display->renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl2Display->renderer);

        /* Draw prompt at top */
        surface = TTF_RenderText_Blended(sdl2Display->font, prompt,
                                         sdl2_colors[CLR_WHITE]);
        if (surface) {
            texture = SDL_CreateTextureFromSurface(sdl2Display->renderer, surface);
            if (texture) {
                rect.x = 20; rect.y = 20;
                SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
                SDL_RenderCopy(sdl2Display->renderer, texture, NULL, &rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }

        /* Draw current input with cursor */
        snprintf(display_buf, sizeof(display_buf), "> %s_", bufp);
        surface = TTF_RenderText_Blended(sdl2Display->font, display_buf,
                                         sdl2_colors[CLR_BRIGHT_GREEN]);
        if (surface) {
            texture = SDL_CreateTextureFromSurface(sdl2Display->renderer, surface);
            if (texture) {
                rect.x = 20; rect.y = 60;
                SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
                SDL_RenderCopy(sdl2Display->renderer, texture, NULL, &rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }

        /* Draw keyboard */
        for (i = 0; i < OSK_ROWS; i++) {
            int row_len = strlen(rows[i]);
            for (j = 0; j < row_len && j < OSK_COLS; j++) {
                char key[2] = { rows[i][j], '\0' };
                int x = start_x + j * cell_w;
                int y = start_y + i * cell_h;

                /* Draw key background */
                rect.x = x + 2;
                rect.y = y + 2;
                rect.w = cell_w - 4;
                rect.h = cell_h - 4;

                if (i == cursor_y && j == cursor_x) {
                    SDL_SetRenderDrawColor(sdl2Display->renderer, 80, 80, 180, 255);
                } else {
                    SDL_SetRenderDrawColor(sdl2Display->renderer, 50, 50, 50, 255);
                }
                SDL_RenderFillRect(sdl2Display->renderer, &rect);

                /* Draw key border */
                SDL_SetRenderDrawColor(sdl2Display->renderer, 120, 120, 120, 255);
                SDL_RenderDrawRect(sdl2Display->renderer, &rect);

                /* Draw key label centered in cell */
                surface = TTF_RenderText_Blended(sdl2Display->font, key,
                    (i == cursor_y && j == cursor_x) ? sdl2_colors[CLR_WHITE]
                                                     : sdl2_colors[CLR_GRAY]);
                if (surface) {
                    texture = SDL_CreateTextureFromSurface(sdl2Display->renderer, surface);
                    if (texture) {
                        SDL_Rect dst;
                        SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
                        dst.x = x + (cell_w - dst.w) / 2;
                        dst.y = y + (cell_h - dst.h) / 2;
                        SDL_RenderCopy(sdl2Display->renderer, texture, NULL, &dst);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surface);
                }
            }
        }

        /* Draw instructions at bottom */
        const char *help = shifted ?
            "[SHIFT] A:type B:back X:space Y:unshift Start:OK Select:cancel" :
            "A:type B:back X:space Y:SHIFT Start:OK Select:cancel";
        surface = TTF_RenderText_Blended(sdl2Display->font, help,
                                         sdl2_colors[CLR_GRAY]);
        if (surface) {
            texture = SDL_CreateTextureFromSurface(sdl2Display->renderer, surface);
            if (texture) {
                rect.x = 20;
                rect.y = SDL2_SCREEN_HEIGHT - 35;
                SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
                SDL_RenderCopy(sdl2Display->renderer, texture, NULL, &rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }

        SDL_RenderPresent(sdl2Display->renderer);

        /* Get input using dedicated OSK input handler */
        ch = sdl2_osk_input();

        /* Navigation */
        if (ch == 'h' && cursor_x > 0) cursor_x--;
        if (ch == 'l' && cursor_x < OSK_COLS - 1) cursor_x++;
        if (ch == 'k' && cursor_y > 0) cursor_y--;
        if (ch == 'j' && cursor_y < OSK_ROWS - 1) cursor_y++;

        /* Type character (A button) */
        if (ch == 'A') {
            const char **r = shifted ? osk_rows_shift : osk_rows;
            if (cursor_y < OSK_ROWS && cursor_x < (int)strlen(r[cursor_y])) {
                char c = r[cursor_y][cursor_x];
                if (buf_pos < BUFSZ - 2) {
                    bufp[buf_pos++] = c;
                    bufp[buf_pos] = '\0';
                }
            }
            shifted = 0;  /* Auto-unshift after typing */
        }

        /* Backspace (B button) */
        if (ch == 'B') {
            if (buf_pos > 0) {
                bufp[--buf_pos] = '\0';
            }
        }

        /* Space (X button) */
        if (ch == 'X') {
            if (buf_pos < BUFSZ - 2) {
                bufp[buf_pos++] = ' ';
                bufp[buf_pos] = '\0';
            }
        }

        /* Shift toggle (Y button) */
        if (ch == 'Y') {
            shifted = !shifted;
        }

        /* Done (Start button) */
        if (ch == '\r') {
            SDL_Event ev;
            /* If empty, use default */
            if (buf_pos == 0) {
                strcpy(bufp, "Player");
            }
            /* Wait for Start button to be released */
            while (SDL_GameControllerGetButton(sdl2Display->controller,
                       SDL_CONTROLLER_BUTTON_START)) {
                SDL_Delay(16);
                SDL_PollEvent(&ev);  /* Keep processing events */
            }
            /* Extra delay and flush */
            SDL_Delay(150);
            while (SDL_PollEvent(&ev)) { /* flush */ }
            debug_log_write("getlin completed: name='%s'\n", bufp);
            sdl2_refresh();
            return;
        }

        /* Cancel (Select button) */
        if (ch == '\033') {
            SDL_Event ev;
            bufp[0] = '\033';
            bufp[1] = '\0';
            /* Wait for Select button to be released */
            while (SDL_GameControllerGetButton(sdl2Display->controller,
                       SDL_CONTROLLER_BUTTON_BACK)) {
                SDL_Delay(16);
                SDL_PollEvent(&ev);
            }
            SDL_Delay(150);
            while (SDL_PollEvent(&ev)) { /* flush */ }
            sdl2_refresh();
            return;
        }
    }
}

int
sdl2_get_ext_cmd(void)
{
    /* Display extended command menu using NetHack's extcmdlist */
    winid win;
    anything any;
    menu_item *selected = NULL;
    int n, i;

    /* Check for pending extended command (from command menu) */
    if (sdl2_pending_extcmd) {
        const char *cmd = sdl2_pending_extcmd;
        sdl2_pending_extcmd = NULL;  /* Clear it */
        /* Find the command in extcmdlist */
        for (i = 0; extcmdlist[i].ef_txt != NULL; i++) {
            if (strcmp(extcmdlist[i].ef_txt, cmd) == 0) {
                return i;
            }
        }
        /* Command not found - fall through to menu */
    }

    win = sdl2_create_nhwindow(NHW_MENU);
    sdl2_start_menu(win, MENU_BEHAVE_STANDARD);

    /* Add all extended commands from extcmdlist */
    for (i = 0; extcmdlist[i].ef_txt != NULL; i++) {
        /* Skip wizard-only commands for non-wizards */
        if ((extcmdlist[i].flags & WIZMODECMD) && !wizard)
            continue;
        /* Skip hidden/internal commands */
        if (extcmdlist[i].flags & CMD_NOT_AVAILABLE)
            continue;

        any = cg.zeroany;
        any.a_int = i + 1;  /* 1-based index */
        sdl2_add_menu(win, NULL, &any,
                     extcmdlist[i].ef_txt[0],  /* First letter as accelerator */
                     0, ATR_NONE, NO_COLOR,
                     extcmdlist[i].ef_txt, MENU_ITEMFLAGS_NONE);
    }

    sdl2_end_menu(win, "Extended Commands:");
    n = sdl2_select_menu(win, PICK_ONE, &selected);
    sdl2_destroy_nhwindow(win);

    if (n > 0 && selected) {
        i = selected->item.a_int - 1;  /* Convert back to 0-based */
        free(selected);
        return i;
    }

    return -1;  /* Cancelled */
}

/*
 * Command menu - accessible via Start button
 * Returns a command character or 0 if cancelled
 */
static int
sdl2_command_menu(void)
{
    static const struct {
        int cmd;  /* int to allow special values */
        const char *desc;
    } commands[] = {
        /* Movement/Actions */
        { 'o', "Open door" },
        { 'c', "Close door" },
        { 'K', "Kick something" },
        { 'l', "Loot container/ground" },
        /* Items - Weapons */
        { 'w', "Wield weapon" },
        { 'x', "Swap weapons" },
        { 'Q', "Ready/quiver ammo" },
        { 't', "Throw item" },
        { 'f', "Fire quivered ammo" },
        /* Items - Armor/Accessories */
        { 'W', "Wear armor" },
        { 'T', "Take off armor" },
        { 'P', "Put on accessory" },
        { 'R', "Remove accessory" },
        /* Items - Consumables */
        { 'e', "Eat food" },
        { 'q', "Quaff potion" },
        { 'r', "Read scroll/book" },
        { 'a', "Apply/use tool" },
        { 'z', "Zap wand" },
        /* Items - Other */
        { 'd', "Drop item" },
        { 'D', "Drop multiple" },
        { 'p', "Pay shopkeeper" },
        { 'E', "Engrave" },
        { 'C', "Name monster/item" },
        /* Game */
        { '?', "Help menu" },
        { 'v', "Version info" },
        { 'S', "Save game" },
        { -1, "Quit game" },  /* Special: triggers #quit */
        { 0, NULL }
    };

    int cursor = 0;
    int scroll_offset = 0;
    int nitems = 0;
    int visible_rows = SDL2_ROWS - 3;
    int ch;
    char buf[BUFSZ];

    /* Count items */
    while (commands[nitems].desc) nitems++;

    while (1) {
        int i, y;

        /* Render menu */
        SDL_SetRenderDrawColor(sdl2Display->renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl2Display->renderer);

        sdl2_render_text(0, 0, "-- Command Menu --", CLR_WHITE, ATR_BOLD);
        y = 2;

        for (i = scroll_offset; i < nitems && y < SDL2_ROWS - 1; i++) {
            /* Highlight current */
            if (i == cursor) {
                SDL_Rect highlight;
                highlight.x = 0;
                highlight.y = y * sdl2Display->font_height;
                highlight.w = SDL2_SCREEN_WIDTH;
                highlight.h = sdl2Display->font_height;
                SDL_SetRenderDrawColor(sdl2Display->renderer, 60, 60, 100, 255);
                SDL_RenderFillRect(sdl2Display->renderer, &highlight);
            }

            if (commands[i].cmd == -1) {
                snprintf(buf, sizeof(buf), "# - %s", commands[i].desc);
            } else {
                snprintf(buf, sizeof(buf), "%c - %s", commands[i].cmd, commands[i].desc);
            }
            sdl2_render_text(0, y, buf,
                            (i == cursor) ? CLR_WHITE : CLR_GRAY, ATR_NONE);
            y++;
        }

        snprintf(buf, sizeof(buf), "-- A: select, B: cancel --");
        sdl2_render_text(0, SDL2_ROWS - 1, buf, CLR_GRAY, ATR_NONE);

        SDL_RenderPresent(sdl2Display->renderer);

        ch = sdl2_nhgetch();

        /* Cancel */
        if (ch == '\033') {
            return 0;
        }

        /* Select current */
        if (ch == '.' || ch == '\r' || ch == ' ') {
            return commands[cursor].cmd;
        }

        /* Navigate - must check BEFORE direct selection to avoid conflicts */
        if (ch == 'k' || ch == 'K') {
            if (cursor > 0) cursor--;
            if (cursor < scroll_offset) scroll_offset = cursor;
            continue;  /* Don't fall through to direct selection */
        }
        if (ch == 'j' || ch == 'J') {
            if (cursor < nitems - 1) cursor++;
            if (cursor >= scroll_offset + visible_rows)
                scroll_offset = cursor - visible_rows + 1;
            continue;  /* Don't fall through to direct selection */
        }

        /* Page up/down */
        if (ch == '<') {
            cursor -= visible_rows;
            if (cursor < 0) cursor = 0;
            scroll_offset -= visible_rows;
            if (scroll_offset < 0) scroll_offset = 0;
            continue;
        }
        if (ch == '>') {
            cursor += visible_rows;
            if (cursor >= nitems) cursor = nitems - 1;
            scroll_offset += visible_rows;
            if (scroll_offset > nitems - visible_rows)
                scroll_offset = nitems - visible_rows;
            if (scroll_offset < 0) scroll_offset = 0;
            continue;
        }

        /* Direct selection by letter (but not navigation keys) */
        for (i = 0; i < nitems; i++) {
            if (commands[i].cmd == ch) {
                return ch;
            }
        }
    }
}

/*
 * Menu functions - stubs for Phase 1
 */
void
sdl2_start_menu(winid window, unsigned long mbehavior)
{
    SDL2_WinDesc *cw;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];
    sdl2_clear_nhwindow(window);
    cw->mbehavior = mbehavior;
}

void
sdl2_add_menu(winid window, const glyph_info *glyphinfo,
              const anything *identifier, char accelerator,
              char groupacc, int attr, int clr,
              const char *str, unsigned int itemflags)
{
    SDL2_WinDesc *cw;
    sdl2_menu_item *item;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];

    /* Allocate menu item */
    item = (sdl2_menu_item *) alloc(sizeof(sdl2_menu_item));
    memset(item, 0, sizeof(sdl2_menu_item));

    if (identifier)
        item->identifier = *identifier;
    item->selector = accelerator;
    item->gselector = groupacc;
    item->attr = attr;
    item->color = clr;
    item->str = dupstr(str);
    item->itemflags = itemflags;
    if (glyphinfo)
        item->glyphinfo = *glyphinfo;

    /* Append to list */
    item->next = NULL;
    if (cw->mlast) {
        cw->mlast->next = item;
    } else {
        cw->mlist = item;
    }
    cw->mlast = item;
    cw->nitems++;
}

void
sdl2_end_menu(winid window, const char *prompt)
{
    SDL2_WinDesc *cw;

    if (window == WIN_ERR || !sdl2wins[window])
        return;

    cw = sdl2wins[window];
    cw->prompt = prompt;
}

/* Helper: get menu item by index */
static sdl2_menu_item *
sdl2_menu_item_at(sdl2_menu_item *mlist, int index)
{
    sdl2_menu_item *item = mlist;
    int i = 0;
    while (item && i < index) {
        item = item->next;
        i++;
    }
    return item;
}

/* Helper: check if menu item is selectable */
static boolean
sdl2_menu_item_selectable(sdl2_menu_item *item)
{
    if (!item) return FALSE;
    /* Item is selectable if it has a selector key OR any identifier */
    if (item->selector) return TRUE;
    if (item->identifier.a_void) return TRUE;
    if (item->identifier.a_int) return TRUE;
    return FALSE;
}

/* Helper: find next selectable item */
static int
sdl2_menu_next_selectable(sdl2_menu_item *mlist, int current, int nitems, int dir)
{
    int next = current;
    int attempts = 0;
    sdl2_menu_item *item;

    do {
        next += dir;
        if (next < 0) next = nitems - 1;
        if (next >= nitems) next = 0;
        item = sdl2_menu_item_at(mlist, next);
        attempts++;
    } while (item && !sdl2_menu_item_selectable(item) && attempts < nitems);

    return (item && sdl2_menu_item_selectable(item)) ? next : current;
}

/* Helper: render menu with cursor and scrolling */
static void
sdl2_render_menu(SDL2_WinDesc *cw, int cursor, int scroll_offset)
{
    sdl2_menu_item *item;
    int i, y;
    int visible_rows = SDL2_ROWS - 2;  /* Leave room for prompt and footer */
    int prompt_rows = cw->prompt ? 2 : 0;
    char buf[BUFSZ];

    /* Clear and set up rendering */
    SDL_SetRenderDrawColor(sdl2Display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl2Display->renderer);

    y = 0;

    /* Render prompt */
    if (cw->prompt) {
        sdl2_render_text(0, y++, cw->prompt, CLR_WHITE, ATR_BOLD);
        y++;  /* blank line */
    }

    /* Render menu items */
    item = cw->mlist;
    i = 0;
    while (item) {
        if (i >= scroll_offset && y < SDL2_ROWS - 1) {
            int color = item->color;
            int attr = item->attr;
            char sel_mark = ' ';

            if (color <= 0 || color >= CLR_MAX)
                color = CLR_GRAY;

            /* Selection marker for PICK_ANY */
            if (item->selected)
                sel_mark = '+';

            /* Build display string */
            if (item->selector) {
                snprintf(buf, sizeof(buf), "%c %c - %s",
                         sel_mark, item->selector, item->str);
            } else {
                snprintf(buf, sizeof(buf), "      %s", item->str);
            }

            /* Highlight current cursor position */
            if (i == cursor) {
                /* Draw highlight background */
                SDL_Rect highlight;
                highlight.x = 0;
                highlight.y = y * sdl2Display->font_height;
                highlight.w = SDL2_SCREEN_WIDTH;
                highlight.h = sdl2Display->font_height;
                SDL_SetRenderDrawColor(sdl2Display->renderer, 60, 60, 100, 255);
                SDL_RenderFillRect(sdl2Display->renderer, &highlight);
                color = CLR_WHITE;
            }

            sdl2_render_text(0, y, buf, color, attr);
            y++;
        }
        item = item->next;
        i++;
    }

    /* Render footer with scroll indicator and help */
    if (cw->nitems > visible_rows - prompt_rows) {
        snprintf(buf, sizeof(buf), "-- %d-%d of %d (L1/R1: page, A: select, B: cancel) --",
                 scroll_offset + 1,
                 (scroll_offset + visible_rows - prompt_rows < cw->nitems) ?
                     scroll_offset + visible_rows - prompt_rows : cw->nitems,
                 cw->nitems);
    } else {
        snprintf(buf, sizeof(buf), "-- A: select, B: cancel --");
    }
    sdl2_render_text(0, SDL2_ROWS - 1, buf, CLR_GRAY, ATR_NONE);

    SDL_RenderPresent(sdl2Display->renderer);
}

int
sdl2_select_menu(winid window, int how, menu_item **menu_list)
{
    SDL2_WinDesc *cw;
    sdl2_menu_item *item;
    int cursor = 0;
    int scroll_offset = 0;
    int visible_rows;
    int prompt_rows;
    int ch;
    int selected = 0;

    if (window == WIN_ERR || !sdl2wins[window])
        return -1;

    cw = sdl2wins[window];
    cw->how = how;
    *menu_list = NULL;

    if (cw->nitems == 0) {
        return 0;
    }

    prompt_rows = cw->prompt ? 2 : 0;
    visible_rows = SDL2_ROWS - 2 - prompt_rows;  /* -2 for footer */

    /* Find first selectable item for initial cursor */
    if (how != PICK_NONE) {
        cursor = sdl2_menu_next_selectable(cw->mlist, -1, cw->nitems, 1);
    }

    cw->active = TRUE;

    /* Main menu loop */
    while (1) {
        sdl2_render_menu(cw, (how != PICK_NONE) ? cursor : -1, scroll_offset);

        ch = sdl2_nhgetch();

        /* Cancel */
        if (ch == '\033' || ch == 'q') {
            cw->active = FALSE;
            sdl2_refresh();
            return -1;
        }

        /* Confirm selection (for PICK_NONE or finishing PICK_ANY) */
        if (ch == '\r' || ch == ' ') {
            if (how == PICK_NONE) {
                cw->active = FALSE;
                sdl2_refresh();
                return 0;
            }
            break;  /* Done selecting for PICK_ANY */
        }

        /* A button - select current item */
        if (ch == '.') {
            item = sdl2_menu_item_at(cw->mlist, cursor);
            if (sdl2_menu_item_selectable(item)) {
                if (how == PICK_ONE) {
                    *menu_list = (menu_item *) alloc(sizeof(menu_item));
                    (*menu_list)->item = item->identifier;
                    (*menu_list)->count = -1;
                    cw->active = FALSE;
                    sdl2_refresh();
                    return 1;
                } else if (how == PICK_ANY) {
                    item->selected = !item->selected;
                }
            }
            continue;
        }

        /* Navigation */
        if (ch == 'k' || ch == 'K') {  /* Up */
            if (how != PICK_NONE) {
                cursor = sdl2_menu_next_selectable(cw->mlist, cursor, cw->nitems, -1);
                /* Adjust scroll if cursor went off screen */
                if (cursor < scroll_offset)
                    scroll_offset = cursor;
            } else {
                if (scroll_offset > 0) scroll_offset--;
            }
            continue;
        }

        if (ch == 'j' || ch == 'J') {  /* Down */
            if (how != PICK_NONE) {
                cursor = sdl2_menu_next_selectable(cw->mlist, cursor, cw->nitems, 1);
                /* Adjust scroll if cursor went off screen */
                if (cursor >= scroll_offset + visible_rows)
                    scroll_offset = cursor - visible_rows + 1;
            } else {
                if (scroll_offset < cw->nitems - visible_rows)
                    scroll_offset++;
            }
            continue;
        }

        /* Page up (L1) */
        if (ch == '<') {
            if (how != PICK_NONE) {
                int i;
                for (i = 0; i < visible_rows && cursor > 0; i++)
                    cursor = sdl2_menu_next_selectable(cw->mlist, cursor, cw->nitems, -1);
            }
            scroll_offset -= visible_rows;
            if (scroll_offset < 0) scroll_offset = 0;
            continue;
        }

        /* Page down (R1) */
        if (ch == '>') {
            if (how != PICK_NONE) {
                int i;
                for (i = 0; i < visible_rows && cursor < cw->nitems - 1; i++)
                    cursor = sdl2_menu_next_selectable(cw->mlist, cursor, cw->nitems, 1);
            }
            scroll_offset += visible_rows;
            if (scroll_offset > cw->nitems - visible_rows)
                scroll_offset = cw->nitems - visible_rows;
            if (scroll_offset < 0) scroll_offset = 0;
            continue;
        }

        /* Direct item selection by letter */
        for (item = cw->mlist; item; item = item->next) {
            if (item->selector == ch && sdl2_menu_item_selectable(item)) {
                if (how == PICK_ONE) {
                    *menu_list = (menu_item *) alloc(sizeof(menu_item));
                    (*menu_list)->item = item->identifier;
                    (*menu_list)->count = -1;
                    cw->active = FALSE;
                    sdl2_refresh();
                    return 1;
                } else if (how == PICK_ANY) {
                    item->selected = !item->selected;
                }
                break;
            }
        }
    }

    /* Count and return selected items for PICK_ANY */
    for (item = cw->mlist; item; item = item->next) {
        if (item->selected) selected++;
    }

    if (selected > 0) {
        menu_item *sel;
        *menu_list = (menu_item *) alloc(selected * sizeof(menu_item));
        sel = *menu_list;
        for (item = cw->mlist; item; item = item->next) {
            if (item->selected) {
                sel->item = item->identifier;
                sel->count = -1;
                sel++;
            }
        }
    }

    cw->active = FALSE;
    sdl2_refresh();
    return selected;
}

/*
 * Status functions - stubs for Phase 1
 */
void
sdl2_status_init(void)
{
    genl_status_init();
}

void
sdl2_status_finish(void)
{
    genl_status_finish();
}

void
sdl2_status_enablefield(int fieldidx, const char *nm,
                        const char *fmt, boolean enable)
{
    genl_status_enablefield(fieldidx, nm, fmt, enable);
}

void
sdl2_status_update(int fieldidx, genericptr_t ptr, int chg,
                   int percent, int color, unsigned long *colormasks)
{
    /* TODO: Implement proper status rendering in Phase 4 */
    genl_status_update(fieldidx, ptr, chg, percent, color, colormasks);
}
