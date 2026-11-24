/* NetHack 5.0  winsdl2.h */
/*
 * SDL2 window port for Powkiddy X55 (JELOS)
 * New file created November 2026
 * NetHack may be freely redistributed. See license for details.
 */

#ifndef WINSDL2_H
#define WINSDL2_H

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

/* Maximum number of windows */
#define SDL2_MAXWIN 20

/* Screen dimensions (Powkiddy X55) */
#define SDL2_SCREEN_WIDTH  1280
#define SDL2_SCREEN_HEIGHT 720

/* Cell dimensions for 80x25 grid */
#define SDL2_CELL_WIDTH  16
#define SDL2_CELL_HEIGHT 28
#define SDL2_COLS 80
#define SDL2_ROWS 25

/* Tile dimensions (maximum, for Nevanda tileset) */
#define SDL2_TILE_WIDTH  32      /* Source tile size in tileset */
#define SDL2_TILE_HEIGHT 32
#define SDL2_TILE_COLS   40      /* Tiles per row in tileset */

/* Display modes */
typedef enum {
    SDL2_DISPLAY_ASCII = 0,      /* Text-only mode */
    SDL2_DISPLAY_TILES_16,       /* 16x16 official tileset */
    SDL2_DISPLAY_TILES_32,       /* 32x32 Nevanda tileset */
    SDL2_DISPLAY_MODE_COUNT
} sdl2_display_mode;

/* Zoom levels - tile display sizes */
#define SDL2_ZOOM_LEVELS     5
#define SDL2_ZOOM_DEFAULT    2   /* Start at 48x48 (index 2) */
/* Zoom sizes: 16 (full map), 32, 48, 64, 80 pixels per tile */
extern const int sdl2_zoom_sizes[SDL2_ZOOM_LEVELS];

/* Legacy defines for compatibility - actual values come from zoom level */
#define SDL2_TILE_DISPLAY_W 48   /* Default display size (zoom level 1) */
#define SDL2_TILE_DISPLAY_H 48

/* Tile viewport (calculated dynamically based on zoom) */
#define SDL2_TILE_VIEWPORT_W 26  /* At default zoom: 1280 / 48 = 26 tiles */
#define SDL2_TILE_VIEWPORT_H 12  /* At default zoom: (720 - 28 - 84) / 48 = 12 */

/* Layout: row 0 = status, rows 1-21 = map, rows 22-24 = messages */
#define SDL2_STATUS_ROW   0
#define SDL2_STATUS_ROWS  1
#define SDL2_MAP_ROW      1
#define SDL2_MAP_ROWS     21
#define SDL2_MSG_ROW      22
#define SDL2_MSG_ROWS     3

/* Window descriptor */
typedef struct SDL2_WinDesc {
    int type;                    /* NHW_MESSAGE, NHW_STATUS, NHW_MAP, etc. */
    boolean active;              /* window is in use */

    /* Geometry (in cells) */
    int offx, offy;              /* position on screen grid */
    int width, height;           /* size in cells */
    int curx, cury;              /* cursor position within window */

    /* Text content storage (for MENU/TEXT windows) */
    char **data;                 /* array of strings */
    int *datlen;                 /* length of each string */
    int maxrow;                  /* number of rows used */
    int maxcol;                  /* max column used */

    /* Menu-specific */
    struct sdl2_menu_item *mlist; /* linked list of menu items */
    struct sdl2_menu_item *mlast; /* last item (for append) */
    int nitems;                   /* number of items */
    int npages;                   /* number of pages */
    int how;                      /* PICK_NONE, PICK_ONE, PICK_ANY */
    const char *prompt;           /* menu prompt */
    unsigned long mbehavior;      /* menu behavior flags */

    int flags;
} SDL2_WinDesc;

/* Menu item structure */
typedef struct sdl2_menu_item {
    struct sdl2_menu_item *next;
    anything identifier;
    long count;
    char *str;
    glyph_info glyphinfo;
    int attr;
    int color;
    boolean selected;
    unsigned itemflags;
    char selector;
    char gselector;
} sdl2_menu_item;

/* Map cell for buffering */
typedef struct sdl2_map_cell {
    int ch;                      /* character to display */
    int color;                   /* foreground color (0-15) */
    int attr;                    /* text attributes */
    int tileidx;                 /* tile index for graphical mode */
    boolean dirty;               /* needs redraw */
} sdl2_map_cell;

/* Topline state (from wintty.h) */
#define TOPLINE_EMPTY       0
#define TOPLINE_NEED_MORE   1
#define TOPLINE_NON_EMPTY   2
#define TOPLINE_SPECIAL     3

/* Input constants */
#define SDL2_STICK_DEADZONE  8000   /* Analog stick deadzone (out of 32767) */
#define SDL2_STICK_THRESHOLD 16000  /* Threshold for direction detection */
#define SDL2_REPEAT_DELAY    400    /* Initial repeat delay (ms) */
#define SDL2_REPEAT_RATE     80     /* Repeat rate after initial delay (ms) */

/* Direction indices for input state */
#define SDL2_DIR_UP    0
#define SDL2_DIR_DOWN  1
#define SDL2_DIR_LEFT  2
#define SDL2_DIR_RIGHT 3
#define SDL2_DIR_COUNT 4

/* Message history */
#define SDL2_MSG_HISTORY_SIZE 50

/* Global display state */
typedef struct SDL2_DisplayDesc {
    /* SDL resources */
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    int font_width, font_height;

    /* Tileset */
    SDL_Texture *tileset;        /* Current tileset texture */
    SDL_Texture *tileset_16;     /* 16x16 official tileset */
    SDL_Texture *tileset_32;     /* 32x32 Nevanda tileset */
    int tileset_cols;            /* Tiles per row in tileset (always 40) */
    boolean use_tiles;           /* Whether to use tiles for map */
    int zoom_level;              /* Current zoom level (0-4) */
    sdl2_display_mode display_mode;  /* Current display mode */
    int tile_src_w, tile_src_h;  /* Current tileset source tile dimensions */

    /* Tile viewport (for scrolling map view) */
    int view_cx, view_cy;        /* Viewport center (player position) */

    /* Screen state */
    int curx, cury;              /* current cursor position */
    int toplin;                  /* message line state */

    /* Map buffer */
    sdl2_map_cell map[ROWNO][COLNO];

    /* Message buffer */
    char toplines[BUFSZ];

    /* Status line buffers */
    char status_line1[BUFSZ];
    char status_line2[BUFSZ];

    /* Message history */
    char msg_history[SDL2_MSG_HISTORY_SIZE][BUFSZ];
    int msg_history_head;   /* Next write position */
    int msg_history_count;  /* Number of messages stored */

    /* Gamepad */
    SDL_GameController *controller;

    /* Input state for repeat handling */
    Uint32 dir_press_time[SDL2_DIR_COUNT];  /* When each direction was pressed */
    Uint32 dir_last_repeat[SDL2_DIR_COUNT]; /* Last repeat time for each dir */
    boolean dir_held[SDL2_DIR_COUNT];        /* Which directions are held */
    int last_dir_key;                        /* Last direction key returned */
} SDL2_DisplayDesc;

/* Global variables */
extern SDL2_DisplayDesc *sdl2Display;
extern SDL2_WinDesc *sdl2wins[SDL2_MAXWIN];
extern struct window_procs sdl2_procs;

/* NetHack color to SDL color mapping */
extern SDL_Color sdl2_colors[CLR_MAX];

/* Function prototypes - initialization */
extern void sdl2_init_nhwindows(int *argcp, char **argv);
extern void sdl2_exit_nhwindows(const char *str);

/* Function prototypes - window management */
extern winid sdl2_create_nhwindow(int type);
extern void sdl2_clear_nhwindow(winid window);
extern void sdl2_display_nhwindow(winid window, boolean blocking);
extern void sdl2_destroy_nhwindow(winid window);
extern void sdl2_curs(winid window, int x, int y);

/* Function prototypes - output */
extern void sdl2_putstr(winid window, int attr, const char *str);
extern void sdl2_raw_print(const char *str);
extern void sdl2_raw_print_bold(const char *str);
extern void sdl2_print_glyph(winid window, coordxy x, coordxy y,
                              const glyph_info *glyphinfo,
                              const glyph_info *bkglyphinfo);

/* Function prototypes - input */
extern int sdl2_nhgetch(void);
extern int sdl2_nh_poskey(coordxy *x, coordxy *y, int *mod);
extern char sdl2_yn_function(const char *query, const char *resp, char def);
extern void sdl2_getlin(const char *prompt, char *bufp);
extern int sdl2_get_ext_cmd(void);

/* Function prototypes - menus */
extern void sdl2_start_menu(winid window, unsigned long mbehavior);
extern void sdl2_add_menu(winid window, const glyph_info *glyphinfo,
                          const anything *identifier, char accelerator,
                          char groupacc, int attr, int clr,
                          const char *str, unsigned int itemflags);
extern void sdl2_end_menu(winid window, const char *prompt);
extern int sdl2_select_menu(winid window, int how, menu_item **menu_list);

/* Function prototypes - status */
extern void sdl2_status_init(void);
extern void sdl2_status_finish(void);
extern void sdl2_status_enablefield(int fieldidx, const char *nm,
                                    const char *fmt, boolean enable);
extern void sdl2_status_update(int fieldidx, genericptr_t ptr, int chg,
                               int percent, int color,
                               unsigned long *colormasks);

/* Helper functions */
extern void sdl2_render_text(int x, int y, const char *str, int color, int attr);
extern void sdl2_render_cell(int x, int y, int ch, int color, int attr);
extern boolean sdl2_render_tile(int map_x, int map_y, int tileidx);
extern void sdl2_refresh(void);

#endif /* WINSDL2_H */
