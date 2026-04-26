#include "m11_game_view.h"

#include "asset_status_m12.h"
#include "fs_portable_compat.h"
#include "m11_v2_vertical_slice_assets.h"
#include "render_sdl_m11.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration: set by M11_GameView_Draw to give nested draw
 * helpers access to the current game state for asset-backed rendering. */
static const M11_GameViewState* g_drawState = NULL;

/*
 * M11 in-game palette indices.
 *
 * The framebuffer is interpreted by render_sdl_m11.c through the DM PC VGA
 * palette (vga_palette_pc34_compat.c; source: VIDEODRV.C / ReDMCSB G8149,
 * G8151–G8156).  That palette is NOT standard CGA/EGA: it is a 16-entry
 * DAC-programmed palette where the numeric slots carry different roles
 * than the IBM VGA 16-color legacy palette.
 *
 * The enum below was originally authored with CGA/EGA semantics (e.g. 2 =
 * green, 7 = light gray, 13 = magenta) which, under the DM VGA palette,
 * produced very wrong in-game colors (index 7 = pure green – the "toxic
 * green floor" seen in V1 screenshots – index 2 = light gray, index 1 =
 * mid gray, etc.).
 *
 * We fix the palette path at the source by binding each symbolic color
 * name to the correct DM PC VGA slot, matching VIDEODRV.C:
 *
 *   0  Black         1  Gray          2  Light Gray    3  Brown
 *   4  Cyan          5  Dark Brown    6  Dark Green    7  Green
 *   8  Red           9  Orange/Gold  10  Tan/Skin     11  Yellow
 *  12  Dark Gray    13  Silver       14  Blue         15  White
 *
 * This preserves M10 semantics (data layer untouched), keeps M11
 * interaction/rendering intact, and only corrects which palette slot each
 * existing call site already asked for.  Every existing name keeps its
 * intended color meaning; only the stored index changes.
 */
enum {
    M11_COLOR_BLACK = 0,        /* DM PC VGA slot 0  — Black          */
    M11_COLOR_NAVY = 14,        /* DM PC VGA slot 14 — Blue           */
    M11_COLOR_GREEN = 6,        /* DM PC VGA slot 6  — Dark Green     */
    M11_COLOR_CYAN = 4,         /* DM PC VGA slot 4  — Cyan           */
    M11_COLOR_RED = 8,          /* DM PC VGA slot 8  — Red            */
    M11_COLOR_BROWN = 3,        /* DM PC VGA slot 3  — Brown          */
    M11_COLOR_LIGHT_GRAY = 2,   /* DM PC VGA slot 2  — Light Gray     */
    M11_COLOR_DARK_GRAY = 12,   /* DM PC VGA slot 12 — Dark Gray      */
    M11_COLOR_LIGHT_BLUE = 14,  /* DM PC VGA slot 14 — Blue (sole blue)*/
    M11_COLOR_LIGHT_GREEN = 7,  /* DM PC VGA slot 7  — Green          */
    M11_COLOR_LIGHT_CYAN = 4,   /* DM PC VGA slot 4  — Cyan (invariant)*/
    M11_COLOR_LIGHT_RED = 9,    /* DM PC VGA slot 9  — Orange/Gold    */
    M11_COLOR_MAGENTA = 10,     /* DM PC VGA slot 10 — Tan/Skin       */
    M11_COLOR_YELLOW = 11,      /* DM PC VGA slot 11 — Yellow         */
    M11_COLOR_WHITE = 15,       /* DM PC VGA slot 15 — White          */
    M11_COLOR_GRAY = 1,         /* DM PC VGA slot 1  — Gray           */
    M11_COLOR_DARK_BROWN = 5,   /* DM PC VGA slot 5  — Dark Brown     */
    M11_COLOR_ORANGE = 9,       /* DM PC VGA slot 9  — Orange/Gold    */
    M11_COLOR_SILVER = 13       /* DM PC VGA slot 13 — Silver         */
};

/*
 * Pass 40 — source-anchored DM1 viewport constants (documentation-only).
 *
 * DM1 PC 3.4 viewport, from the local ReDMCSB dump:
 *   DEFS.H:1997  #define C112_BYTE_WIDTH_VIEWPORT 112   (4-bpp => 224 px)
 *   DEFS.H:2003  #define C136_HEIGHT_VIEWPORT     136
 *   COORD.C:81   int16_t G2067_i_ViewportScreenX = 0;
 *   COORD.C:82   int16_t G2068_i_ViewportScreenY = 33;
 *
 * The DM1-original viewport rectangle on the 320x200 framebuffer is
 * therefore (x=0, y=33, w=224, h=136).  Normal V1 now uses that
 * source-faithful rectangle directly; the old smaller Firestaff
 * viewport remains only for debug/prototype HUD mode.
 */
enum {
    M11_DM1_VIEWPORT_X = 0,    /* COORD.C G2067_i_ViewportScreenX        */
    M11_DM1_VIEWPORT_Y = 33,   /* COORD.C G2068_i_ViewportScreenY        */
    M11_DM1_VIEWPORT_W = 224,  /* DEFS.H C112_BYTE_WIDTH_VIEWPORT * 2    */
    M11_DM1_VIEWPORT_H = 136   /* DEFS.H C136_HEIGHT_VIEWPORT            */
};

enum {
    M11_VIEWPORT_X = M11_DM1_VIEWPORT_X,
    M11_VIEWPORT_Y = M11_DM1_VIEWPORT_Y,
    M11_VIEWPORT_W = M11_DM1_VIEWPORT_W,
    M11_VIEWPORT_H = M11_DM1_VIEWPORT_H,
    M11_CONTROL_STRIP_X = 14,
    M11_CONTROL_STRIP_Y = 165,
    M11_CONTROL_STRIP_W = 88,
    M11_CONTROL_STRIP_H = 14,
    M11_PROMPT_STRIP_X = 104,
    M11_PROMPT_STRIP_Y = 165,
    M11_PROMPT_STRIP_W = 202,
    M11_PROMPT_STRIP_H = 14,
    M11_PARTY_PANEL_X = 12,
    M11_PARTY_PANEL_Y = 160,
    /* Legacy / V2-mode dimensions.  V2's pre-baked four-slot HUD
     * sprite (m11_v2_party_hud_four_slot_base, 302x28) encodes
     * these values directly: 4 * 77 - 6 = 302 (= step 77 * 3 +
     * slot 71 + 13 padding), and the active overlay is 71x28.
     * In V1 original-faithful mode these are overridden at the
     * use site by the M11_V1_PARTY_SLOT_* constants below.  See
     * pass 41 (parity-evidence/pass41_status_box_stride.md). */
    M11_PARTY_SLOT_W = 71,
    M11_PARTY_SLOT_H = 28,
    M11_PARTY_SLOT_STEP = 77,
    /* DM1 PC 3.4 source-anchored champion status-box geometry
     * for V1 original-faithful mode.  STEP == DEFS.H:2157
     * C69_CHAMPION_STATUS_BOX_SPACING (69).  W == graphic
     * C007_GRAPHIC_STATUS_BOX frame width (67, verified by the
     * M11 asset-loader probe invariant INV_GV_205 which asserts
     * graphic 7 is 67x29).  Pass 41 landed these as the active
     * V1 values via m11_party_slot_step() / m11_party_slot_w().
     * Ref: V1_BLOCKERS.md §5 / PARITY_MATRIX_DM1_V1.md §1. */
    M11_V1_PARTY_SLOT_W    = 67,
    M11_V1_PARTY_SLOT_STEP = 69,
    /* DM1 PC 3.4 source-anchored champion status-box bar-graph
     * geometry for V1 original-faithful mode.  Derived from the
     * ZONES.H layout table recovered in pass 47 tooling
     * (zones_h_reconstruction.json, GRAPHICS.DAT entry 696) and
     * from ReDMCSB CHAMDRAW.C F0287_CHAMPION_DrawBarGraphs.
     *
     * Zone hierarchy (relative to the 67x29 C007 status-box frame
     * whose origin is the slot (x, y)):
     *   zone 150 (dims 67x29) -> zone 151 (type 1, origin 0,0)
     *     -> zone 183 (dims 24x29) at offset (0, 0)
     *       -> zone 187 (type 1, origin 43, 0) — bar-graph region
     *         -> zone 191 (dims 4x25) at offset 0, 0
     *           -> zone 195 (type 7, bottom-center, offset 5, 26)  "HP"
     *           -> zone 199 (type 7, bottom-center, offset 12, 26) "stamina"
     *           -> zone 203 (type 7, bottom-center, offset 19, 26) "mana"
     *
     * Each bar is 4 px wide x 25 px tall, bottom-anchored inside a
     * 4x25 container.  The three bars span x = 43..62 (HP centered
     * at 43+5-2 = 46 column range; 4-wide; etc.) — see helper
     * m11_v1_bar_graph_x() below for the exact per-bar origin.
     *
     * Pass 43: replaces the invented horizontal 59x{2,1,1} strip at
     * slot-bottom with the source-faithful vertical 4x25 bars.
     * Ref: V1_BLOCKERS.md §7; PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md
     * Pass 35 §2.7; parity-evidence/pass43_bar_graphs.md. */
    M11_V1_BAR_GRAPH_REGION_X = 43,
    M11_V1_BAR_GRAPH_REGION_Y = 0,
    M11_V1_BAR_GRAPH_REGION_W = 24,
    M11_V1_BAR_GRAPH_REGION_H = 29,
    M11_V1_BAR_CONTAINER_W   = 4,
    M11_V1_BAR_CONTAINER_H   = 25,
    M11_V1_BAR_HP_CX         = 5,   /* zone 195 d1 */
    M11_V1_BAR_STAMINA_CX    = 12,  /* zone 199 d1 */
    M11_V1_BAR_MANA_CX       = 19,  /* zone 203 d1 */
    /* DM1 status-box hand slot zones from layout-696:
     * C211/C213/C215/C217 ready hand  = parent champion box + (4,10)
     * C212/C214/C216/C218 action hand = parent champion box + (24,10)
     * Parent dimensions are 16x16; GRAPHICS.DAT slot-box bitmaps are
     * 18x18 and intentionally overdraw the border like F0291. */
    M11_V1_STATUS_READY_HAND_X = 4,
    M11_V1_STATUS_ACTION_HAND_X = 24,
    M11_V1_STATUS_HAND_Y = 10,
    M11_V1_STATUS_HAND_ZONE_W = 16,
    M11_V1_STATUS_HAND_ZONE_H = 16,
    /* DM1 status-box name zones from layout-696:
     * C159..C162 clear zones are 43x7 at status-box origin + (0,0).
     * C163..C166 text zones are type-18 children at +1, clipped to
     * 42x7.  F0292 clears C159+n to C01 dark gray and centers the
     * champion name in C163+n. */
    M11_V1_STATUS_NAME_CLEAR_X = 0,
    M11_V1_STATUS_NAME_CLEAR_Y = 0,
    M11_V1_STATUS_NAME_CLEAR_W = 43,
    M11_V1_STATUS_NAME_CLEAR_H = 7,
    M11_V1_STATUS_NAME_TEXT_X = 1,
    M11_V1_STATUS_NAME_TEXT_Y = 0,
    M11_V1_STATUS_NAME_TEXT_W = 42,
    M11_V1_STATUS_NAME_TEXT_H = 7,
    M11_V1_BAR_BOTTOM_OFFSET = 26,  /* zones 195/199/203 d2 (bottom anchor) */
    M11_UTILITY_PANEL_X = 218,
    M11_UTILITY_PANEL_Y = 28,
    M11_UTILITY_PANEL_W = 86,
    M11_UTILITY_PANEL_H = 42,
    M11_UTILITY_BUTTON_Y = 56,
    M11_UTILITY_BUTTON_H = 10,
    M11_UTILITY_INSPECT_X = 222,
    M11_UTILITY_INSPECT_W = 22,
    M11_UTILITY_SAVE_X = 250,
    M11_UTILITY_SAVE_W = 22,
    M11_UTILITY_LOAD_X = 278,
    M11_UTILITY_LOAD_W = 22
};

enum {
    M11_QUICKSAVE_HEADER_SIZE = 16
};

/* DM1 action-hand icon cell geometry (duplicated as forward decls
 * for M11_GameView_HandlePointer, which needs these before the
 * #defines that originally introduced them near the drawing code
 * lower down in this translation unit).  The downstream #defines
 * remain authoritative — keep these two blocks in sync.  Ref:
 * ReDMCSB MENUS.C F0386_MENUS_DrawActionIcon. */
enum {
    M11_DM_ACTION_ICON_CELL_Y_FWD    = 86,
    M11_DM_ACTION_ICON_CELL_H_FWD    = 35,
    M11_DM_ACTION_ICON_CELL_W_FWD    = 20,
    M11_DM_ACTION_ICON_CELL_STEP_FWD = 22,
    M11_DM_ACTION_ICON_CELL_X0_FWD   = 233,
    /* Action-menu row geometry forward declarations so
     * M11_GameView_HandlePointer can hit-test the three
     * DM1 action rows while the authoritative definitions
     * (M11_DM_ACTION_MENU_ROW_*) live next to the renderer
     * later in the file.  Ref: ReDMCSB ACTIDRAW.C
     * F0387_MENUS_DrawActionArea menu-mode zones 85/86/87. */
    M11_DM_ACTION_MENU_AREA_X_FWD      = 224,
    M11_DM_ACTION_MENU_AREA_W_FWD      = 87,
    M11_DM_ACTION_MENU_ROW_Y0_FWD      = 58,
    M11_DM_ACTION_MENU_ROW_STEP_FWD    = 11,
    M11_DM_ACTION_MENU_ROW_H_FWD       = 9
};

static const unsigned char g_m11_quicksave_magic[8] = {
    'F', 'S', 'M', '1', '1', 'Q', 'S', '1'
};

typedef struct {
    char ch;
    unsigned char rows[7];
} M11_Glyph;

typedef struct {
    int scale;
    int tracking;
    unsigned char color;
    int shadowDx;
    int shadowDy;
    unsigned char shadowColor;
} M11_TextStyle;

static const M11_Glyph g_font[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
    {'/', {1, 1, 2, 4, 8, 16, 16}},
    {'>', {1, 2, 4, 8, 4, 2, 1}},
    {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'B', {30, 17, 17, 30, 17, 17, 30}},
    {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'D', {30, 17, 17, 17, 17, 17, 30}},
    {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'F', {31, 16, 16, 30, 16, 16, 16}},
    {'G', {14, 17, 16, 23, 17, 17, 14}},
    {'H', {17, 17, 17, 31, 17, 17, 17}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'J', {7, 2, 2, 2, 18, 18, 12}},
    {'K', {17, 18, 20, 24, 20, 18, 17}},
    {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 17, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 17, 21, 27, 17}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}}
};

static const M11_TextStyle g_text_small = {1, 1, M11_COLOR_WHITE, 0, 0, M11_COLOR_BLACK};
static const M11_TextStyle g_text_shadow = {1, 1, M11_COLOR_WHITE, 1, 1, M11_COLOR_BLACK};
static const M11_TextStyle g_text_title = {2, 1, M11_COLOR_YELLOW, 1, 1, M11_COLOR_BLACK};

/* Thread-local-ish pointer to the original DM1 font for the current
 * render pass.  Set at the top of M11_GameView_Render and cleared
 * after.  Allows m11_draw_text to automatically use the original
 * font when assets are available without changing every call site. */
static const M11_FontState* g_activeOriginalFont = NULL;

static void m11_put_pixel(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          unsigned char color) {
    if (!framebuffer) {
        return;
    }
    if (x < 0 || y < 0 || x >= framebufferWidth || y >= framebufferHeight) {
        return;
    }
    framebuffer[(y * framebufferWidth) + x] = color;
}

static void m11_fill_rect(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char color) {
    int yy;
    int xx;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                          x + xx, y + yy, color);
        }
    }
}

static void m11_draw_rect(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char color) {
    int i;
    for (i = 0; i < w; ++i) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + i, y, color);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + i, y + h - 1, color);
    }
    for (i = 0; i < h; ++i) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y + i, color);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + w - 1, y + i, color);
    }
}

static void m11_hatch_rect(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x,
                           int y,
                           int w,
                           int h) {
    int yy;
    for (yy = y; yy < y + h; ++yy) {
        int xx;
        for (xx = x; xx < x + w; ++xx) {
            /* DM1 PC VGA VIDEODRV.C F8155_VIDRV_06_HatchScreenBox:
             * leave odd (x^y) pixels untouched and force even checker
             * pixels to black. */
            if (((xx ^ yy) & 1) == 0) {
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                              xx, yy, M11_COLOR_BLACK);
            }
        }
    }
}

static void m11_draw_hline(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x1,
                           int x2,
                           int y,
                           unsigned char color) {
    int x;
    if (x2 < x1) {
        int swap = x1;
        x1 = x2;
        x2 = swap;
    }
    for (x = x1; x <= x2; ++x) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y, color);
    }
}

static void m11_draw_vline(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x,
                           int y1,
                           int y2,
                           unsigned char color) {
    int y;
    if (y2 < y1) {
        int swap = y1;
        y1 = y2;
        y2 = swap;
    }
    for (y = y1; y <= y2; ++y) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y, color);
    }
}

static const M11_Glyph* m11_find_glyph(char ch) {
    size_t i;
    unsigned char uch = (unsigned char)ch;
    if (uch >= 'a' && uch <= 'z') {
        ch = (char)(uch - ('a' - 'A'));
    }
    for (i = 0; i < sizeof(g_font) / sizeof(g_font[0]); ++i) {
        if (g_font[i].ch == ch) {
            return &g_font[i];
        }
    }
    return &g_font[0];
}

static void m11_draw_glyph(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x,
                           int y,
                           char ch,
                           const M11_TextStyle* style,
                           int drawShadow) {
    const M11_Glyph* glyph = m11_find_glyph(ch);
    int row;
    int col;
    int yy;
    int xx;
    const M11_TextStyle* s = style ? style : &g_text_small;
    for (row = 0; row < 7; ++row) {
        for (col = 0; col < 5; ++col) {
            if ((glyph->rows[row] & (1 << (4 - col))) == 0) {
                continue;
            }
            for (yy = 0; yy < s->scale; ++yy) {
                for (xx = 0; xx < s->scale; ++xx) {
                    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                                  x + col * s->scale + xx + (drawShadow ? s->shadowDx : 0),
                                  y + row * s->scale + yy + (drawShadow ? s->shadowDy : 0),
                                  drawShadow ? s->shadowColor : s->color);
                }
            }
        }
    }
}

/* Forward declaration for original-font text renderer */
static void m11_draw_text_original(
    const M11_FontState* font,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int x,
    int y,
    const char* text,
    const M11_TextStyle* style);

static void m11_draw_text(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          const char* text,
                          const M11_TextStyle* style) {
    int cursor = x;
    size_t i;
    const M11_TextStyle* s = style ? style : &g_text_small;
    if (!text) {
        return;
    }
    /* Use original DM1 font when available */
    if (g_activeOriginalFont && M11_Font_IsLoaded(g_activeOriginalFont)) {
        m11_draw_text_original(g_activeOriginalFont, framebuffer,
            framebufferWidth, framebufferHeight, x, y, text, style);
        return;
    }
    for (i = 0; text[i] != '\0'; ++i) {
        m11_draw_glyph(framebuffer, framebufferWidth, framebufferHeight,
                       cursor, y, text[i], s, 1);
        m11_draw_glyph(framebuffer, framebufferWidth, framebufferHeight,
                       cursor, y, text[i], s, 0);
        cursor += (5 * s->scale) + s->tracking;
    }
}

static int m11_measure_text_pixels(const char* text,
                                   const M11_TextStyle* style) {
    const M11_TextStyle* s = style ? style : &g_text_small;
    size_t len = text ? strlen(text) : 0;
    if (!text || len == 0) return 0;
    if (g_activeOriginalFont && M11_Font_IsLoaded(g_activeOriginalFont)) {
        return M11_Font_MeasureString(text);
    }
    return (int)(len * (size_t)((5 * s->scale) + s->tracking));
}

static void m11_draw_text_centered_in_rect(unsigned char* framebuffer,
                                           int framebufferWidth,
                                           int framebufferHeight,
                                           int x,
                                           int y,
                                           int w,
                                           const char* text,
                                           const M11_TextStyle* style) {
    int textW = m11_measure_text_pixels(text, style);
    int drawX = x + ((w - textW) / 2);
    if (drawX < x) drawX = x;
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  drawX, y, text, style);
}

static int m11_dialog_source_c469_text_y_for_lines(int lineCount) {
    enum {
        SOURCE_TEXT_HEIGHT = 6,
        SOURCE_TEXT_PAD = 1,
        SOURCE_TEXT_BASELINE = 6,
        SOURCE_LINE_STEP = 8
    };
    int zoneX, zoneY, zoneW, zoneH;
    int count = lineCount < 1 ? 1 : lineCount;
    int block = count * (SOURCE_TEXT_HEIGHT + (SOURCE_TEXT_PAD * 2) - 1 + 1);
    int relativeY;
    (void)M11_GameView_GetV1DialogMessageZone(1, &zoneX, &zoneY, &zoneW, &zoneH);
    (void)zoneX;
    (void)zoneW;
    relativeY = zoneY + ((zoneH - (block - (SOURCE_TEXT_PAD * 2))) / 2) +
                    SOURCE_TEXT_BASELINE - 1;
    (void)SOURCE_LINE_STEP;
    return M11_VIEWPORT_Y + relativeY;
}

static int m11_dialog_source_c471_text_y_for_lines(int lineCount) {
    enum {
        SOURCE_TEXT_HEIGHT = 6,
        SOURCE_TEXT_PAD = 1,
        SOURCE_TEXT_BASELINE = 6
    };
    int zoneX, zoneY, zoneW, zoneH;
    int count = lineCount < 1 ? 1 : lineCount;
    int block = count * (SOURCE_TEXT_HEIGHT + (SOURCE_TEXT_PAD * 2) - 1 + 1);
    int relativeY;
    (void)M11_GameView_GetV1DialogMessageZone(2, &zoneX, &zoneY, &zoneW, &zoneH);
    (void)zoneX;
    (void)zoneW;
    relativeY = zoneY + ((zoneH - (block - (SOURCE_TEXT_PAD * 2))) / 2) +
                    SOURCE_TEXT_BASELINE - 1;
    return M11_VIEWPORT_Y + relativeY;
}

int M11_GameView_GetV1DialogSingleChoiceMessageTextY(int lineCount) {
    return m11_dialog_source_c469_text_y_for_lines(lineCount);
}

int M11_GameView_GetV1DialogMultiChoiceMessageTextY(int lineCount) {
    return m11_dialog_source_c471_text_y_for_lines(lineCount);
}

int M11_GameView_GetV1DialogMessageZone(int choiceCount,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH) {
    int x, y, w, h;
    if (choiceCount > 1) {
        /* C471_ZONE_DIALOG: parent C470 bottom/right (188,36), child top/left (112,32). */
        x = 112; y = 32; w = 77; h = 5;
    } else {
        /* C469_ZONE_DIALOG: parent C468 bottom/right (188,73), child top/left (112,49). */
        x = 112; y = 49; w = 77; h = 25;
    }
    if (outX) *outX = x;
    if (outY) *outY = y;
    if (outW) *outW = w;
    if (outH) *outH = h;
    return 1;
}

int M11_GameView_GetV1DialogMessageWidth(int choiceCount) {
    int w = 0;
    (void)M11_GameView_GetV1DialogMessageZone(choiceCount, NULL, NULL, &w, NULL);
    return w;
}

int M11_GameView_GetV1DialogChoiceTextZoneId(int choiceCount,
                                              int choiceIndex) {
    if (choiceIndex < 0) return 0;
    switch (choiceCount) {
        case 1:
            return choiceIndex == 0 ? 462 : 0;
        case 2:
            if (choiceIndex == 0) return 463;
            if (choiceIndex == 1) return 462;
            return 0;
        case 3:
            if (choiceIndex == 0) return 463;
            if (choiceIndex == 1) return 466;
            if (choiceIndex == 2) return 467;
            return 0;
        default:
            if (choiceCount < 4) return 0;
            if (choiceIndex == 0) return 464;
            if (choiceIndex == 1) return 465;
            if (choiceIndex == 2) return 466;
            if (choiceIndex == 3) return 467;
            return 0;
    }
}

int M11_GameView_GetV1DialogChoiceTextZone(int choiceCount,
                                            int choiceIndex,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH) {
    int zoneId = M11_GameView_GetV1DialogChoiceTextZoneId(choiceCount, choiceIndex);
    int x = 0, y = 0, w = 0;
    switch (zoneId) {
        case 462:
            x = 16; y = 110; w = 192;
            break;
        case 463:
            x = 16; y = 73; w = 192;
            break;
        case 464:
            x = 16; y = 73; w = 86;
            break;
        case 465:
            x = 123; y = 73; w = 86;
            break;
        case 466:
            x = 16; y = 110; w = 86;
            break;
        case 467:
            x = 123; y = 110; w = 86;
            break;
        default:
            return 0;
    }
    if (outX) *outX = x;
    if (outY) *outY = y;
    if (outW) *outW = w;
    if (outH) *outH = 7;
    return 1;
}

int M11_GameView_GetV1DialogChoiceButtonZoneId(int choiceCount,
                                                int choiceIndex) {
    if (choiceIndex < 0) return 0;
    switch (choiceCount) {
        case 1:
            return choiceIndex == 0 ? 456 : 0;
        case 2:
            if (choiceIndex == 0) return 457;
            if (choiceIndex == 1) return 456;
            return 0;
        case 3:
            if (choiceIndex == 0) return 457;
            if (choiceIndex == 1) return 460;
            if (choiceIndex == 2) return 461;
            return 0;
        default:
            if (choiceCount < 4) return 0;
            if (choiceIndex == 0) return 458;
            if (choiceIndex == 1) return 459;
            if (choiceIndex == 2) return 460;
            if (choiceIndex == 3) return 461;
            return 0;
    }
}

int M11_GameView_GetV1DialogChoiceHitZone(int choiceCount,
                                           int choiceIndex,
                                           int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH) {
    int zoneId = M11_GameView_GetV1DialogChoiceButtonZoneId(choiceCount, choiceIndex);
    int x = 0, y = 0, w = 0;
    switch (zoneId) {
        case 456:
            x = 16; y = 104; w = 192;
            break;
        case 457:
            x = 16; y = 67; w = 192;
            break;
        case 458:
            x = 16; y = 67; w = 86;
            break;
        case 459:
            x = 123; y = 67; w = 86;
            break;
        case 460:
            x = 16; y = 104; w = 86;
            break;
        case 461:
            x = 123; y = 104; w = 86;
            break;
        default:
            return 0;
    }
    if (outX) *outX = x;
    if (outY) *outY = y;
    if (outW) *outW = w;
    if (outH) *outH = 17;
    return 1;
}

static int m11_dialog_source_message_width_for_choices(int choiceCount) {
    return M11_GameView_GetV1DialogMessageWidth(choiceCount);
}

static int m11_dialog_source_split_two_lines(const char* text,
                                             char* line1,
                                             size_t line1Size,
                                             char* line2,
                                             size_t line2Size,
                                             int zoneWidth) {
    int textW;
    int maxW;
    int best = -1;
    int i;
    if (!text || !line1 || !line2 || line1Size == 0 || line2Size == 0) return 0;
    if (zoneWidth <= 0) zoneWidth = 77;
    textW = m11_measure_text_pixels(text, &g_text_shadow);
    maxW = zoneWidth - (zoneWidth >> 3);
    if (textW <= maxW) {
        snprintf(line1, line1Size, "%s", text);
        line2[0] = '\0';
        return 1;
    }
    if (maxW > textW - (textW >> 3)) {
        maxW = textW - (textW >> 3);
    }
    for (i = 1; text[i] != '\0'; ++i) {
        if (text[i] == ' ') {
            char tmp[128];
            size_t n = (size_t)i < sizeof(tmp) - 1 ? (size_t)i : sizeof(tmp) - 1;
            memcpy(tmp, text, n);
            tmp[n] = '\0';
            if (m11_measure_text_pixels(tmp, &g_text_shadow) <= maxW) {
                best = i;
            }
        }
    }
    if (best <= 0) {
        best = maxW / 6;
        if (best < 1) best = 1;
    }
    if ((size_t)best >= line1Size) best = (int)line1Size - 1;
    memcpy(line1, text, (size_t)best);
    line1[best] = '\0';
    snprintf(line2, line2Size, "%s", text + best + (text[best] == ' ' ? 1 : 0));
    return 2;
}

static void m11_draw_dialog_choice_text(unsigned char* framebuffer,
                                        int framebufferWidth,
                                        int framebufferHeight,
                                        int viewportX,
                                        int viewportY,
                                        int width,
                                        const char* text) {
    M11_TextStyle choiceStyle = g_text_shadow;
    choiceStyle.color = M11_COLOR_WHITE;
    choiceStyle.shadowColor = M11_COLOR_BROWN;
    if (!text || text[0] == '\0') return;
    m11_draw_text_centered_in_rect(framebuffer,
                                   framebufferWidth,
                                   framebufferHeight,
                                   M11_VIEWPORT_X + viewportX,
                                   M11_VIEWPORT_Y + viewportY,
                                   width,
                                   text,
                                   &choiceStyle);
}

static void m11_draw_dialog_choices_source(const M11_GameViewState* state,
                                           unsigned char* framebuffer,
                                           int framebufferWidth,
                                           int framebufferHeight) {
    int i;
    if (!state || state->dialogChoiceCount <= 0) return;
    for (i = 0; i < state->dialogChoiceCount && i < 4; ++i) {
        int x, y, w;
        if (!M11_GameView_GetV1DialogChoiceTextZone(state->dialogChoiceCount,
                                                    i, &x, &y, &w, NULL)) {
            continue;
        }
        m11_draw_dialog_choice_text(framebuffer, framebufferWidth, framebufferHeight,
                                    x, y, w, state->dialogChoices[i]);
    }
}

static int m11_dialog_choice_at_point(const M11_GameViewState* state,
                                      int x,
                                      int y) {
    int vx = x - M11_VIEWPORT_X;
    int vy = y - M11_VIEWPORT_Y;
    int i;
    if (!state || state->dialogChoiceCount <= 0) return 0;
    if (vx < 0 || vy < 0 || vx >= M11_VIEWPORT_W || vy >= M11_VIEWPORT_H) return 0;
    for (i = 0; i < state->dialogChoiceCount && i < 4; ++i) {
        int hx, hy, hw, hh;
        if (!M11_GameView_GetV1DialogChoiceHitZone(state->dialogChoiceCount,
                                                   i, &hx, &hy, &hw, &hh)) {
            continue;
        }
        if (vx >= hx && vx < hx + hw && vy >= hy && vy < hy + hh) {
            return i + 1;
        }
    }
    return 0;
}

/* Draw text using the original DM1 font when available, with shadow.
 * Falls back to the builtin hardcoded font otherwise. */
static void m11_draw_text_original(
    const M11_FontState* font,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int x,
    int y,
    const char* text,
    const M11_TextStyle* style)
{
    const M11_TextStyle* s = style ? style : &g_text_small;
    if (!text) return;
    if (font && M11_Font_IsLoaded(font)) {
        /* Shadow pass */
        if (s->shadowDx != 0 || s->shadowDy != 0) {
            M11_Font_DrawString(font, framebuffer, framebufferWidth,
                framebufferHeight, x + s->shadowDx, y + s->shadowDy,
                text, s->shadowColor, -1, s->scale);
        }
        /* Foreground pass */
        M11_Font_DrawString(font, framebuffer, framebufferWidth,
            framebufferHeight, x, y, text, s->color, -1, s->scale);
    } else {
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      x, y, text, style);
    }
}

static const char* m11_direction_name(int dir) {
    switch (dir) {
        case DIR_NORTH: return "NORTH";
        case DIR_EAST: return "EAST";
        case DIR_SOUTH: return "SOUTH";
        case DIR_WEST: return "WEST";
        default: return "UNKNOWN";
    }
}

static const char* m11_source_name(M11_GameSourceKind sourceKind) {
    switch (sourceKind) {
        case M11_GAME_SOURCE_CUSTOM_DUNGEON:
            return "CUSTOM";
        case M11_GAME_SOURCE_DIRECT_DUNGEON:
            return "DIRECT";
        default:
            return "BUILTIN";
    }
}

static M11_AudioMarker m11_audio_marker_from_emission(const struct TickEmission_Compat* emission) {
    if (!emission) {
        return M11_AUDIO_MARKER_NONE;
    }
    switch (emission->kind) {
        case EMIT_PARTY_MOVED:
            return M11_AUDIO_MARKER_FOOTSTEP;
        case EMIT_DOOR_STATE:
            return M11_AUDIO_MARKER_DOOR;
        case EMIT_DAMAGE_DEALT:
        case EMIT_KILL_NOTIFY:
        case EMIT_CHAMPION_DOWN:
            return M11_AUDIO_MARKER_COMBAT;
        case EMIT_SPELL_EFFECT:
            return M11_AUDIO_MARKER_SPELL;
        case EMIT_SOUND_REQUEST:
            switch (emission->payload[0]) {
                case 1:
                case 2:
                    return M11_AUDIO_MARKER_DOOR;
                case 3:
                    return M11_AUDIO_MARKER_FOOTSTEP;
                case 4:
                    return M11_AUDIO_MARKER_COMBAT;
                case 6:
                    return M11_AUDIO_MARKER_SPELL;
                default:
                    return M11_AUDIO_MARKER_CREATURE;
            }
        default:
            break;
    }
    return M11_AUDIO_MARKER_NONE;
}

static void m11_audio_emit_source_sound(M11_GameViewState* state,
                                        int soundIndex,
                                        M11_AudioMarker fallbackMarker) {
    if (!state) {
        return;
    }
    (void)M11_Audio_EmitSoundIndex(&state->audioState,
                                   soundIndex,
                                   fallbackMarker);
}

static void m11_audio_emit_for_emission(M11_GameViewState* state,
                                        const struct TickEmission_Compat* emission) {
    M11_AudioMarker marker;
    if (!state || !emission) {
        return;
    }
    marker = m11_audio_marker_from_emission(emission);
    if (marker == M11_AUDIO_MARKER_NONE) {
        return;
    }
    if (emission->kind == EMIT_SOUND_REQUEST) {
        (void)M11_Audio_EmitSoundIndex(&state->audioState,
                                       emission->payload[0],
                                       marker);
    } else {
        (void)M11_Audio_EmitMarker(&state->audioState, marker);
    }
    state->audioEventCount += 1;
}

/* m11_join_path replaced by FSP_JoinPath from fs_portable_compat. */

static void m11_write_u32_le(unsigned char* dst, uint32_t value) {
    if (!dst) {
        return;
    }
    dst[0] = (unsigned char)(value & 0xFFu);
    dst[1] = (unsigned char)((value >> 8) & 0xFFu);
    dst[2] = (unsigned char)((value >> 16) & 0xFFu);
    dst[3] = (unsigned char)((value >> 24) & 0xFFu);
}

static uint32_t m11_read_u32_le(const unsigned char* src) {
    if (!src) {
        return 0;
    }
    return ((uint32_t)src[0]) |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

int M11_GameView_GetQuickSavePath(const M11_GameViewState* state,
                                  char* out,
                                  size_t outSize) {
    const char* envPath;
    const char* sourceId = "dm1";
    int rc;

    if (!out || outSize == 0U) {
        return 0;
    }

    envPath = getenv("FIRESTAFF_QUICKSAVE_PATH");
    if (envPath && envPath[0] != '\0') {
        rc = snprintf(out, outSize, "%s", envPath);
        return rc > 0 && (size_t)rc < outSize;
    }

    if (state && state->sourceId[0] != '\0') {
        sourceId = state->sourceId;
    }

    rc = snprintf(out, outSize, "firestaff-%s-quicksave.sav", sourceId);
    return rc > 0 && (size_t)rc < outSize;
}

static int m11_resolve_builtin_dungeon_path(char* out,
                                            size_t outSize,
                                            const char* dataDir,
                                            const char* gameId) {
    if (!out || !dataDir || !gameId) {
        return 0;
    }
    if (strcmp(gameId, "dm1") == 0) {
        return FSP_JoinPath(out, outSize, dataDir, "DUNGEON.DAT");
    }
    if (strcmp(gameId, "csb") == 0) {
        return FSP_JoinPath(out, outSize, dataDir, "CSB.DAT");
    }
    if (strcmp(gameId, "dm2") == 0) {
        return FSP_JoinPath(out, outSize, dataDir, "DM2DUNGEON.DAT");
    }
    return 0;
}

static uint8_t m11_forward_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_NORTH;
        case DIR_EAST: return CMD_MOVE_EAST;
        case DIR_SOUTH: return CMD_MOVE_SOUTH;
        case DIR_WEST: return CMD_MOVE_WEST;
        default: return CMD_NONE;
    }
}

static uint8_t m11_backward_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_SOUTH;
        case DIR_EAST: return CMD_MOVE_WEST;
        case DIR_SOUTH: return CMD_MOVE_NORTH;
        case DIR_WEST: return CMD_MOVE_EAST;
        default: return CMD_NONE;
    }
}

static uint8_t m11_strafe_left_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_WEST;
        case DIR_EAST: return CMD_MOVE_NORTH;
        case DIR_SOUTH: return CMD_MOVE_EAST;
        case DIR_WEST: return CMD_MOVE_SOUTH;
        default: return CMD_NONE;
    }
}

static uint8_t m11_strafe_right_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_EAST;
        case DIR_EAST: return CMD_MOVE_SOUTH;
        case DIR_SOUTH: return CMD_MOVE_WEST;
        case DIR_WEST: return CMD_MOVE_NORTH;
        default: return CMD_NONE;
    }
}

static unsigned char m11_tile_color(int elementType) {
    switch (elementType) {
        case DUNGEON_ELEMENT_WALL: return M11_COLOR_DARK_GRAY;
        case DUNGEON_ELEMENT_CORRIDOR: return M11_COLOR_LIGHT_GRAY;
        case DUNGEON_ELEMENT_PIT: return M11_COLOR_BROWN;
        case DUNGEON_ELEMENT_STAIRS: return M11_COLOR_YELLOW;
        case DUNGEON_ELEMENT_DOOR: return M11_COLOR_LIGHT_RED;
        case DUNGEON_ELEMENT_TELEPORTER: return M11_COLOR_LIGHT_CYAN;
        case DUNGEON_ELEMENT_FAKEWALL: return M11_COLOR_MAGENTA;
        default: return M11_COLOR_BLACK;
    }
}

static int m11_map_square_base(const struct DungeonDatState_Compat* dungeon,
                               int mapIndex) {
    int i;
    int base = 0;
    if (!dungeon || !dungeon->maps || mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }
    for (i = 0; i < mapIndex; ++i) {
        base += (int)dungeon->maps[i].width * (int)dungeon->maps[i].height;
    }
    return base;
}

static int m11_get_square_byte(const struct GameWorld_Compat* world,
                               int mapIndex,
                               int mapX,
                               int mapY,
                               unsigned char* outSquare) {
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int index;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded || !outSquare) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return 0;
    }
    tiles = &world->dungeon->tiles[mapIndex];
    index = mapX * (int)map->height + mapY;
    if (!tiles->squareData || index < 0 || index >= tiles->squareCount) {
        return 0;
    }
    *outSquare = tiles->squareData[index];
    return 1;
}

static unsigned short m11_raw_next_thing(const struct DungeonThings_Compat* things,
                                         unsigned short thing) {
    int type;
    int index;
    const unsigned char* raw;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (type < 0 || type >= 16 || !things->rawThingData[type] || index < 0 || index >= things->thingCounts[type]) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + (index * s_thingDataByteCount[type]);
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static unsigned short m11_get_first_square_thing(const struct GameWorld_Compat* world,
                                                 int mapIndex,
                                                 int mapX,
                                                 int mapY) {
    int base;
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) {
        return THING_ENDOFLIST;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return THING_ENDOFLIST;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return THING_ENDOFLIST;
    }
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) {
        return THING_ENDOFLIST;
    }
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return THING_ENDOFLIST;
    }
    return world->things->squareFirstThings[squareIndex];
}

static int m11_count_square_things(const struct GameWorld_Compat* world,
                                   int mapIndex,
                                   int mapX,
                                   int mapY) {
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);
    int count = 0;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && count < 32) {
        ++count;
        thing = m11_raw_next_thing(world->things, thing);
    }
    return count;
}

typedef struct {
    int total;
    int groups;
    int items;
    int sensors;
    int textStrings;
    int teleporters;
    int projectiles;
    int explosions;
    int doors;
} M11_SquareThingSummary;

struct M11_ViewportCell;

/* ── DM1 random ornament computation (F0169/F0170 from ReDMCSB) ──
 * Floor ornaments in DM1 are assigned per-square using a deterministic
 * pseudorandom algorithm seeded by the dungeon header's OrnamentRandomSeed.
 * The algorithm hashes map coordinates + map index/dimensions to produce
 * a per-square ornament index.  If the square allows random ornaments
 * (bit 3 of the corridor square byte) and the map has floor ornaments,
 * the square gets a floor ornament ordinal (1-based, 0 = none).
 * Ref: ReDMCSB DUNGEON.C F0169_DUNGEON_GetRandomOrnamentIndex,
 *      F0170_DUNGEON_GetRandomOrnamentOrdinal,
 *      F0172_DUNGEON_SetSquareAspect (corridor case). */

/* Compute the floor ornament ordinal for a corridor/pit/stairs/teleporter
 * square.  Returns 1-based ordinal, or 0 if none.
 * This mirrors F0172_DUNGEON_SetSquareAspect's floor ornament path.
 * Two sources of floor ornaments:
 *  1. Random floor ornaments based on MASK0x0008 bit in the square byte
 *  2. Sensor things with ornamentOrdinal > 0 (explicit placement)
 * Ref: ReDMCSB DUNGEON.C F0172 lines ~2189-2196. */
static int m11_compute_floor_ornament_ordinal(
    const M11_GameViewState* state,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned char square) {
    int elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    const struct DungeonMapDesc_Compat* map;
    int randomAllowed;
    int randomFloorOrnCount;
    unsigned short ornSeed;
    unsigned int value2;
    int idx;
    int ordinal = 0;

    /* Floor ornaments only appear on corridor, pit, stairs, teleporter */
    if (elementType != DUNGEON_ELEMENT_CORRIDOR &&
        elementType != DUNGEON_ELEMENT_PIT &&
        elementType != DUNGEON_ELEMENT_STAIRS &&
        elementType != DUNGEON_ELEMENT_TELEPORTER) {
        return 0;
    }
    if (!state || !state->world.dungeon ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }

    map = &state->world.dungeon->maps[mapIndex];
    ornSeed = state->world.dungeon->header.ornamentRandomSeed;
    randomFloorOrnCount = (int)map->randomFloorOrnamentCount;

    /* Random floor ornament from square byte bit 3 (MASK0x0008) */
    randomAllowed = (square & 0x08) != 0;
    if (randomAllowed && randomFloorOrnCount > 0) {
        /* value2 = 3000 + (mapIndex << 6) + mapWidth + mapHeight
         * Ref: ReDMCSB F0170 call from F0172 */
        value2 = (unsigned int)(3000 + (mapIndex << 6) +
                                (int)map->width + (int)map->height);
        idx = (int)((((((unsigned int)(2000 + (mapX << 5) + mapY)) * 31417u) >> 1) +
                     (value2 * 11u) + ornSeed) >> 2) % 30;
        if (idx < randomFloorOrnCount) {
            ordinal = idx + 1; /* 1-based */
        }
    }

    /* Sensor-placed floor ornaments override random ones.
     * Scan for sensor things with ornamentOrdinal on this square.
     * Ref: ReDMCSB F0172 — C0_SENSOR_FLOOR_ORNAMENT_ORDINAL path. */
    if (state->world.things && state->world.things->sensors) {
        unsigned short scanThing = m11_get_first_square_thing(
            &state->world, mapIndex, mapX, mapY);
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_SENSOR) {
                int sIdx = THING_GET_INDEX(scanThing);
                if (sIdx >= 0 && sIdx < state->world.things->sensorCount) {
                    /* Floor ornament sensor: ornamentOrdinal > 0 on
                     * non-wall squares indicates a floor ornament.
                     * Use the same field we use for wall ornaments. */
                    int sOrd = (int)state->world.things->sensors[sIdx].ornamentOrdinal;
                    if (sOrd > 0) {
                        ordinal = sOrd;
                        break;
                    }
                }
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    return ordinal;
}

/* ── DM1 Creature Aspect Data (G0219_as_Graphic558_CreatureAspects) ──
 * Each creature type has an aspect descriptor that controls:
 *  - FirstNativeBitmapRelativeIndex: offset from the creature graphic set base
 *  - FirstDerivedBitmapIndex: index into derived bitmap cache
 *  - CoordinateSet (upper 4 bits of CoordinateSet_TransparentColor)
 *  - TransparentColor (lower 4 bits)
 *  - ReplacementColorSetIndices (color 9 in low nibble, color 10 in high)
 *
 * The coordinate set selects which of 11 pre-defined viewport position
 * tables to use for placing the creature at each depth/sub-cell.
 *
 * We additionally carry the GraphicInfo field from the CREATURE_INFO
 * table (G0243_as_Graphic559_CreatureInfo[].GraphicInfo, 16 bits).
 * GraphicInfo controls which poses have dedicated bitmaps
 * (side/back/attack) and the flip semantics used when falling back to
 * the front bitmap.  Without these flags the renderer was blindly
 * indexing into every creature's native+derived bitmap sequence and
 * producing unrelated sprites for any pose beyond the front — this
 * table fixes that by letting the pose lookup fall back to front
 * whenever the per-creature flag is clear.
 *
 * Values extracted from ReDMCSB DEFS.H / disassembly of DM1 PC v3.4
 * (see firestaff_extracted_frontends_probe.c G0243_as_Graphic559_CreatureInfo).
 * The front/side/attack width+height fields exist only in later versions
 * (S10+) and are not needed for our rendering — we use GRAPHICS.DAT
 * bitmap dimensions directly.
 *
 * Format: { FirstNativeBitmapRelativeIndex, FirstDerivedBitmapIndex,
 *           CoordinateSet_TransparentColor, ReplacementColorSetIndices,
 *           GraphicInfo }
 */
typedef struct {
    unsigned short firstNativeBitmapRelativeIndex;
    unsigned short firstDerivedBitmapIndex;
    unsigned char  coordinateSet_transparentColor;
    unsigned char  replacementColorSetIndices;
    unsigned short graphicInfo;
} M11_CreatureAspect;

#define M11_CREATURE_COORD_SET(a) (((a)->coordinateSet_transparentColor >> 4) & 0x0F)
#define M11_CREATURE_TRANSPARENT_COLOR(a) ((a)->coordinateSet_transparentColor & 0x0F)
#define M11_CREATURE_REPL_COLOR9(a) ((a)->replacementColorSetIndices & 0x0F)
#define M11_CREATURE_REPL_COLOR10(a) (((a)->replacementColorSetIndices >> 4) & 0x0F)

/* Creature GraphicInfo masks — source-backed from ReDMCSB DEFS.H.
 * Bit 0..1: MASK0x0003_ADDITIONAL (reserved extra derived bitmaps; only
 *           value 1 is ever meaningful, values 2/3 are unused).
 * Bit 2:    MASK0x0004_FLIP_NON_ATTACK — flip front bitmap for non-attack
 *           poses when no side/back bitmap exists.
 * Bit 3:    MASK0x0008_SIDE — dedicated side bitmap exists.
 * Bit 4:    MASK0x0010_BACK — dedicated back bitmap exists.
 * Bit 5:    MASK0x0020_ATTACK — dedicated attack bitmap exists.
 * Bit 6:    unreferenced.
 * Bit 7:    MASK0x0080_SPECIAL_D2_FRONT — alternate front bitmap at D2.
 * Bit 8:    MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT — the alternate
 *           D2 front uses the flipped front bitmap.
 * Bit 9:    MASK0x0200_FLIP_ATTACK — flip attack bitmap.
 * Bit 10:   MASK0x0400_FLIP_DURING_ATTACK — flip sprite during attack.
 * Bit 11:   unreferenced.
 * Bits 12-13: M052_MAXIMUM_HORIZONTAL_OFFSET (creature wobble range).
 * Bits 14-15: M053_MAXIMUM_VERTICAL_OFFSET.
 */
#define M11_CREATURE_GI_MASK_ADDITIONAL          0x0003u
#define M11_CREATURE_GI_MASK_FLIP_NON_ATTACK     0x0004u
#define M11_CREATURE_GI_MASK_SIDE                0x0008u
#define M11_CREATURE_GI_MASK_BACK                0x0010u
#define M11_CREATURE_GI_MASK_ATTACK              0x0020u
#define M11_CREATURE_GI_MASK_SPECIAL_D2_FRONT    0x0080u
#define M11_CREATURE_GI_MASK_D2_FRONT_IS_FLIPPED 0x0100u
#define M11_CREATURE_GI_MASK_FLIP_ATTACK         0x0200u
#define M11_CREATURE_GI_MASK_FLIP_DURING_ATTACK  0x0400u
#define M11_CREATURE_GI_ADDITIONAL(gi)            ((gi) & M11_CREATURE_GI_MASK_ADDITIONAL)
#define M11_CREATURE_GI_MAX_HORIZONTAL_OFFSET(gi) (((gi) >> 12) & 0x0003u)
#define M11_CREATURE_GI_MAX_VERTICAL_OFFSET(gi)   (((gi) >> 14) & 0x0003u)

/* ── Source-backed native/derived bitmap-count helpers (ReDMCSB) ──
 * These mirror ReDMCSB's F097_xxxx_DUNGEONVIEW_LoadGraphics_COPYPROTECTIONF
 * native-bitmap allocation loop (DUNVIEW.C ~L555-L585) and
 * F460_xxxx_START_CalculateDerivedBitmapCacheSizes (START.C ~L165-L205)
 * derived-bitmap allocation loop.
 *
 * Native bitmap sequence per creature (variable length):
 *   [Front] [Side?] [Back?] [SpecialD2?] [Attack?] [AdditionalFront x N?]
 *   where:
 *     - Front is always present (1 slot)
 *     - Side present iff MASK0x0008_SIDE
 *     - Back present iff MASK0x0010_BACK
 *     - SpecialD2 slot present iff (MASK0x0080_SPECIAL_D2_FRONT
 *       && !MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT) for the
 *       C06_COMPILE_DM10aEN..DM13bFR versions (DM1 Atari ST family).
 *       Guarded by BUG0_00 note in ReDMCSB: this slot is allocated but
 *       never read by F1512-render ("Useless code").  The allocation
 *       still consumes a bitmap-index position.
 *     - Attack present iff MASK0x0020_ATTACK
 *     - AdditionalFront present iff (ADDITIONAL > 0 &&
 *       !MASK0x0004_FLIP_NON_ATTACK); count = ADDITIONAL
 *
 * Derived bitmap sequence per creature (variable length):
 *   [FrontD3] [FrontD2] [SideD3 SideD2?] [BackD3 BackD2?] [AttackD3 AttackD2?]
 *     [AdditionalFrontD1 AdditionalFrontD3 AdditionalFrontD2 x N?]
 *   No SpecialD2 extra slot in the derived cache (only F097 allocates
 *   that in the native list).  Additional fronts get 3 derived slots
 *   each (D1-cache, D3, D2) regardless of FLIP_NON_ATTACK.
 */
static int m11_creature_native_bitmap_count_from_gi(unsigned int gi) {
    int count = 1; /* Front is always present */
    int hasSpecialD2 = 0;
    int additional = (int)M11_CREATURE_GI_ADDITIONAL(gi);
    if (gi & M11_CREATURE_GI_MASK_SIDE)   count += 1;
    if (gi & M11_CREATURE_GI_MASK_BACK)   count += 1;
    hasSpecialD2 = ((gi & M11_CREATURE_GI_MASK_SPECIAL_D2_FRONT) != 0) &&
                   ((gi & M11_CREATURE_GI_MASK_D2_FRONT_IS_FLIPPED) == 0);
    if (hasSpecialD2) count += 1;
    if (gi & M11_CREATURE_GI_MASK_ATTACK) count += 1;
    if (additional && !(gi & M11_CREATURE_GI_MASK_FLIP_NON_ATTACK)) {
        count += additional;
    }
    return count;
}

static int m11_creature_derived_bitmap_count_from_gi(unsigned int gi) {
    int count = 2; /* Front D3 + Front D2 always present */
    int additional = (int)M11_CREATURE_GI_ADDITIONAL(gi);
    if (gi & M11_CREATURE_GI_MASK_SIDE)   count += 2;
    if (gi & M11_CREATURE_GI_MASK_BACK)   count += 2;
    if (gi & M11_CREATURE_GI_MASK_ATTACK) count += 2;
    /* Additional fronts each get D1 cache + D3 + D2 derived slots */
    count += additional * 3;
    return count;
}

static const M11_CreatureAspect s_creatureAspects[27] = {
    /* Fields: firstNative, firstDerived, coordSet_transparent, replColors, graphicInfo
     * graphicInfo values reproduced verbatim from ReDMCSB
     * G0243_as_Graphic559_CreatureInfo[].GraphicInfo. */
    /* Type  0: GiantScorpion — no side/back/attack bitmaps. */
    { 0,   495, 0x1D, 0x01, 0x0482 },
    /* Type  1: SwampSlime — no side/back/attack bitmaps. */
    { 4,   507, 0x0B, 0x20, 0x0480 },
    /* Type  2: Giggler — back bitmap only. */
    { 6,   519, 0x0B, 0x00, 0x4510 },
    /* Type  3: PainRat — back + attack bitmaps. */
    { 10,  531, 0x24, 0x31, 0x04B4 },
    /* Type  4: Ruster — no side/back/attack bitmaps. */
    { 12,  543, 0x14, 0x34, 0x0701 },
    /* Type  5: Screamer — no side/back/attack bitmaps. */
    { 16,  555, 0x18, 0x34, 0x0581 },
    /* Type  6: Rockpile — side bitmap only. */
    { 19,  567, 0x0D, 0x00, 0x070C },
    /* Type  7: GhostRive — no side/back/attack bitmaps. */
    { 21,  579, 0x04, 0x00, 0x0300 },
    /* Type  8: WaterElemental — attack bitmap only. */
    { 23,  591, 0x04, 0x00, 0x5864 },
    /* Type  9: Couatl — no side/back/attack bitmaps. */
    { 25,  603, 0x14, 0x00, 0x0282 },
    /* Type 10: StoneGolem — no side/back/attack bitmaps. */
    { 29,  615, 0x04, 0x00, 0x1480 },
    /* Type 11: Mummy — no side/back/attack bitmaps. */
    { 33,  627, 0x14, 0x00, 0x18C6 },
    /* Type 12: Skeleton — no side/back/attack bitmaps. */
    { 35,  639, 0x04, 0x00, 0x1280 },
    /* Type 13: MagentaWorm — attack bitmap only. */
    { 39,  651, 0x1D, 0x20, 0x14A2 },
    /* Type 14: Trolin — side + back + attack bitmaps. */
    { 43,  663, 0x04, 0x30, 0x05B8 },
    /* Type 15: GiantWasp — no side/back/attack bitmaps. */
    { 47,  675, 0x14, 0x78, 0x0381 },
    /* Type 16: Antman — no side/back/attack bitmaps. */
    { 51,  687, 0x04, 0x65, 0x0680 },
    /* Type 17: Vexirk — attack bitmap only. */
    { 55,  699, 0x24, 0x00, 0x04A0 },
    /* Type 18: AnimatedArmour — no side/back/attack bitmaps. */
    { 59,  711, 0x04, 0x00, 0x0280 },
    /* Type 19: Materializer — attack bitmap only. */
    { 63,  723, 0x0D, 0xA9, 0x4060 },
    /* Type 20: RedDragon — side + back bitmaps (no attack). */
    { 67,  735, 0x14, 0x65, 0x10DE },
    /* Type 21: Oitu — no side/back/attack bitmaps. */
    { 69,  747, 0x14, 0xA9, 0x0082 },
    /* Type 22: Demon — no side/back/attack bitmaps. */
    { 73,  759, 0x04, 0xCB, 0x1480 },
    /* Type 23: LordChaos — side + attack bitmaps. */
    { 77,  771, 0x14, 0x00, 0x78AA },
    /* Type 24: LordOrder — side bitmap only. */
    { 81,  783, 0x14, 0xCB, 0x068A },
    /* Type 25: GreyLord — side + attack bitmaps. */
    { 85,  795, 0x14, 0xCB, 0x78AA },
    /* Type 26: LordChaosRedDragon — side + attack bitmaps. */
    { 86,  807, 0x14, 0xCB, 0x78AA }
};

/* ── DM1 Creature Viewport Coordinate Sets (G0224) ──
 * Exact front-cell viewport coordinates extracted from original item 558
 * documentation (Graphic558 / G0224). Each entry is {centerX, bottomY}
 * in the original 224×136 viewport.
 *
 * We only encode the 3 coordinate sets actually referenced by the DM1
 * creature aspect table in this renderer (sets 0, 1 and 2).
 *
 * For set 0 / set 2 the five slots map to c1..c5.
 * For set 1 the five slots map to c6..c10.
 * Depth index mapping here is 0=D1, 1=D2, 2=D3.
 */
static const unsigned char s_creatureFrontCoordSets[3][3][5][2] = {
    /* Depth 0 (D1) */
    {
        /* Set 0: c1..c5 */  {{ 83,103 }, {141,103 }, {148,119 }, { 76,119 }, {109,111 }},
        /* Set 1: c6..c10 */ {{ 81,119 }, {142,119 }, {111,105 }, {111,111 }, {111,119 }},
        /* Set 2: c1..c5 */  {{ 83, 94 }, {141, 94 }, {148, 98 }, { 76, 98 }, {109, 96 }}
    },
    /* Depth 1 (D2) */
    {
        /* Set 0: c1..c5 */  {{ 92, 81 }, {131, 81 }, {132, 90 }, { 91, 90 }, {111, 85 }},
        /* Set 1: c6..c10 */ {{ 91, 90 }, {132, 90 }, {111, 83 }, {111, 85 }, {111, 90 }},
        /* Set 2: c1..c5 */  {{ 92, 73 }, {131, 73 }, {132, 78 }, { 91, 78 }, {111, 75 }}
    },
    /* Depth 2 (D3) */
    {
        /* Set 0: c1..c5 */  {{ 95, 70 }, {127, 70 }, {129, 75 }, { 93, 75 }, {111, 72 }},
        /* Set 1: c6..c10 */ {{ 94, 75 }, {128, 75 }, {111, 70 }, {111, 72 }, {111, 75 }},
        /* Set 2: c1..c5 */  {{ 95, 59 }, {127, 59 }, {129, 61 }, { 93, 61 }, {111, 60 }}
    }
};

void M11_GameView_GetCreatureFrontSlotPoint(int coordSet,
                                            int depthIndex,
                                            int visibleCount,
                                            int slotIndex,
                                            int* outCenterX,
                                            int* outBottomY) {
    int pointIndex = 4;
    if (outCenterX) *outCenterX = 111;
    if (outBottomY) *outBottomY = (depthIndex <= 0) ? 111 : (depthIndex == 1 ? 85 : 72);
    if (coordSet < 0 || coordSet > 2 || depthIndex < 0 || depthIndex > 2 ||
        slotIndex < 0 || slotIndex > 3) {
        return;
    }

    if (coordSet == 1) {
        if (visibleCount <= 1) {
            pointIndex = 4; /* c10 */
        } else {
            pointIndex = slotIndex < 2 ? slotIndex : 4; /* c6/c7, clamp extras */
        }
    } else {
        if (visibleCount <= 1) {
            pointIndex = 4; /* c5 */
        } else {
            pointIndex = slotIndex;
            if (pointIndex > 3) pointIndex = 3;
        }
    }

    if (outCenterX) *outCenterX = (int)s_creatureFrontCoordSets[depthIndex][coordSet][pointIndex][0];
    if (outBottomY) *outBottomY = (int)s_creatureFrontCoordSets[depthIndex][coordSet][pointIndex][1];
}

static int m11_creature_front_point_index(int coordSet,
                                          int visibleCount,
                                          int slotIndex) {
    int pointIndex = 4;
    if (slotIndex < 0) slotIndex = 0;
    if (slotIndex > 3) slotIndex = 3;
    if (coordSet == 1) {
        if (visibleCount > 1) {
            pointIndex = slotIndex < 2 ? slotIndex : 4;
        }
    } else if (visibleCount > 1) {
        pointIndex = slotIndex;
        if (pointIndex > 3) pointIndex = 3;
    }
    return pointIndex;
}

static int m11_c3200_creature_zone_point(int coordSet,
                                         int depthIndex,
                                         int visibleCount,
                                         int slotIndex,
                                         int* outX,
                                         int* outY) {
    /* Layout-696 C3200_ZONE_ source points for center-lane creature
     * placement.  We bind the three coordinate sets used by DM1 creature
     * aspects and the D1/D2/D3 center groups.  Values are viewport-local
     * center-X / bottom-Y, matching F0115's creature blit anchor. */
    static const short kC3200Center[3][3][5][2] = {
        {
            {{ 83,106}, {141,106}, {148,119}, { 76,119}, {112,111}},
            {{ 92, 83}, {131, 83}, {132, 90}, { 91, 90}, {112, 85}},
            {{ 97, 67}, {125, 67}, {129, 72}, { 95, 72}, {112, 72}}
        },
        {
            {{ 81,119}, {142,119}, {112,105}, {112,111}, {112,119}},
            {{ 91, 90}, {132, 90}, {112, 83}, {112, 85}, {112, 89}},
            {{ 94, 73}, {128, 73}, {112, 70}, {112, 70}, {112, 73}}
        },
        {
            {{ 83, 79}, {141, 79}, {148, 85}, { 76, 85}, {112, 81}},
            {{ 92, 65}, {131, 65}, {132, 67}, { 91, 67}, {112, 66}},
            {{ 95, 59}, {127, 59}, {129, 61}, { 93, 61}, {112, 60}}
        }
    };
    int pointIndex;
    if (coordSet < 0 || coordSet > 2) return 0;
    if (depthIndex < 0) depthIndex = 0;
    if (depthIndex > 2) depthIndex = 2;
    pointIndex = m11_creature_front_point_index(coordSet, visibleCount, slotIndex);
    if (outX) *outX = (int)kC3200Center[coordSet][depthIndex][pointIndex][0];
    if (outY) *outY = (int)kC3200Center[coordSet][depthIndex][pointIndex][1];
    return 1;
}

static int m11_c3200_creature_side_zone_point(int coordSet,
                                              int depthIndex,
                                              int sideHint,
                                              int visibleCount,
                                              int slotIndex,
                                              int* outX,
                                              int* outY) {
    /* Layout-696 C3200 side groups.  For each coordinate set the 65
     * records are arranged as depth bands; within each band center is
     * followed by left and right side-cell groups. */
    static const short kC3200Side[3][3][2][5][2] = {
        {
            {{{-54,103}, { 18,108}, {  1,119}, {-99,119}, {-21,111}},
             {{207,109}, {277,103}, {321,119}, {223,119}, {244,111}}},
            {{{  4, 83}, { 46, 83}, { 35, 90}, {-20, 90}, { 20, 85}},
             {{177, 83}, {224, 83}, {243, 90}, {189, 90}, {205, 85}}},
            {{{ 35, 68}, { 63, 68}, { 58, 72}, { 27, 72}, { 45, 72}},
             {{157, 68}, {181, 67}, {200, 72}, {166, 72}, {179, 72}}}
        },
        {
            {{{-97,119}, {  1,119}, {-16,105}, {-30,111}, {-21,119}},
             {{223,119}, {321,119}, {239,105}, {253,111}, {246,119}}},
            {{{-20, 90}, { 35, 90}, { 25, 83}, { 20, 85}, { 26, 89}},
             {{189, 90}, {243, 90}, {199, 83}, {205, 85}, {203, 89}}},
            {{{ 28, 73}, { 58, 73}, { 49, 70}, { 45, 70}, { 49, 73}},
             {{166, 73}, {198, 73}, {175, 70}, {179, 70}, {178, 73}}}
        },
        {
            {{{-54, 79}, { 18, 79}, {  1, 85}, {-99, 85}, {-21, 81}},
             {{207, 79}, {277, 79}, {321, 85}, {223, 85}, {244, 81}}},
            {{{ -1, 65}, { 46, 65}, { 35, 67}, {-20, 67}, { 20, 66}},
             {{177, 65}, {224, 65}, {243, 67}, {189, 67}, {205, 66}}},
            {{{ 39, 59}, { 63, 59}, { 58, 61}, { 25, 61}, { 45, 60}},
             {{159, 59}, {185, 59}, {201, 61}, {166, 61}, {179, 60}}}
        }
    };
    int sideIndex;
    int pointIndex;
    if (coordSet < 0 || coordSet > 2) return 0;
    if (depthIndex < 0) depthIndex = 0;
    if (depthIndex > 2) depthIndex = 2;
    sideIndex = sideHint < 0 ? 0 : 1;
    pointIndex = m11_creature_front_point_index(coordSet, visibleCount, slotIndex);
    if (outX) *outX = (int)kC3200Side[coordSet][depthIndex][sideIndex][pointIndex][0];
    if (outY) *outY = (int)kC3200Side[coordSet][depthIndex][sideIndex][pointIndex][1];
    return 1;
}

/* Query the creature aspect's coordinate set index (0-10) for a type. */
static int m11_creature_coordinate_set(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return M11_CREATURE_COORD_SET(&s_creatureAspects[creatureType]);
}

/* Query the creature's transparent color index from aspect data. */
static int m11_creature_transparent_color(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return M11_CREATURE_TRANSPARENT_COLOR(&s_creatureAspects[creatureType]);
}

static int m11_thing_is_item(int thingType) {
    switch (thingType) {
        case THING_TYPE_WEAPON:
        case THING_TYPE_ARMOUR:
        case THING_TYPE_SCROLL:
        case THING_TYPE_POTION:
        case THING_TYPE_CONTAINER:
        case THING_TYPE_JUNK:
            return 1;
        default:
            return 0;
    }
}

static void m11_summarize_square_things(const struct GameWorld_Compat* world,
                                        int mapIndex,
                                        int mapX,
                                        int mapY,
                                        M11_SquareThingSummary* outSummary) {
    M11_SquareThingSummary summary;
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);

    memset(&summary, 0, sizeof(summary));
    while (thing != THING_ENDOFLIST && thing != THING_NONE && summary.total < 32) {
        int thingType = THING_GET_TYPE(thing);
        ++summary.total;
        switch (thingType) {
            case THING_TYPE_DOOR:
                ++summary.doors;
                break;
            case THING_TYPE_TELEPORTER:
                ++summary.teleporters;
                break;
            case THING_TYPE_TEXTSTRING:
                ++summary.textStrings;
                break;
            case THING_TYPE_SENSOR:
                ++summary.sensors;
                break;
            case THING_TYPE_GROUP:
                ++summary.groups;
                break;
            case THING_TYPE_PROJECTILE:
                ++summary.projectiles;
                break;
            case THING_TYPE_EXPLOSION:
                ++summary.explosions;
                break;
            default:
                if (m11_thing_is_item(thingType)) {
                    ++summary.items;
                }
                break;
        }
        thing = m11_raw_next_thing(world->things, thing);
    }

    /* V1 projectile-cycle visibility: runtime-only projectiles and
     * explosions spawned via F0810 / F0821 (action-menu projectile
     * rows) are not in the dungeon-thing linked list.  The viewport
     * renderer and side-pane code gate their sprites on
     * summary.projectiles / summary.explosions, so we fold the
     * GameWorld runtime lists into the same totals here so the
     * newly-spawned projectile is drawn at its current cell from
     * the first tick, and keeps being drawn at the cell it has
     * travelled to after each F0811 advance.  Matches DM1 where
     * a thrown arrow / fireball appears on the square it is flying
     * through for each visible frame of the cast animation.
     *
     * Ref: ReDMCSB DUNGEON.C viewport projectile scan walks the
     * ACTIVE_PROJECTILE list per tick in exactly the same way. */
    if (world) {
        int i;
        for (i = 0; i < world->projectiles.count
                    && i < PROJECTILE_LIST_CAPACITY; ++i) {
            const struct ProjectileInstance_Compat* p =
                &world->projectiles.entries[i];
            if (p->slotIndex < 0) continue;
            if (p->mapIndex == mapIndex && p->mapX == mapX
                    && p->mapY == mapY) {
                ++summary.projectiles;
                ++summary.total;
            }
        }
        for (i = 0; i < world->explosions.count
                    && i < EXPLOSION_LIST_CAPACITY; ++i) {
            const struct ExplosionInstance_Compat* e =
                &world->explosions.entries[i];
            if (e->slotIndex < 0) continue;
            if (e->mapIndex == mapIndex && e->mapX == mapX
                    && e->mapY == mapY) {
                ++summary.explosions;
                ++summary.total;
            }
        }
    }

    if (outSummary) {
        *outSummary = summary;
    }
}

/* Pass 42: V1-chrome-mode reroute.  In the original DM1 PC 3.4
 * presentation, short status strings are surfaced via the TEXT.C
 * message log, not via a dedicated "status lozenge" or "inspect
 * readout" surface (neither exists in DM1).  Firestaff's 82
 * m11_set_status sites and 68 m11_set_inspect_readout sites
 * (PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md Pass 35 §2.2 / §2.4) are
 * rerouted here: the invented surfaces are still written (they
 * stay visible under showDebugHUD for diagnostics), but when V1
 * chrome mode is enabled and the content is player-facing, the
 * strings are additionally pushed into the rolling message log
 * so the rerouted multi-line surface at the bottom of the screen
 * can pick them up.
 *
 * The player-facing filter is the same suppress-list used by the
 * existing v1 bottom-line scan (m11_v1_message_is_player_facing)
 * so no debug chatter leaks into the log.  To avoid duplicate
 * entries we track the last rerouted payload on the state and
 * skip the push when the new payload is byte-identical to the
 * previous one (per-surface). */

/* Forward declarations for helpers defined later in this
 * translation unit that the Pass 42 chrome reroute needs. */
static int m11_v1_chrome_mode_enabled(void);
static int m11_v1_message_is_player_facing(const char* stripped);
static const M11_LogEntry* m11_log_entry_at(const M11_MessageLog* log, int reverseIndex);

static int m11_chrome_reroute_is_player_facing_pass42(const char* text) {
    if (!text || text[0] == '\0') {
        return 0;
    }
    return m11_v1_message_is_player_facing(text);
}

/* Pass 42: return 1 when any of the last N message-log entries
 * already contains the rerouted payload's key phrase.  This
 * prevents the chrome reroute from double-logging events that a
 * m11_log_event call immediately before already surfaced (e.g.
 * stair transitions, pit falls, spell feedback all emit both an
 * m11_log_event and a m11_set_status with overlapping wording).
 *
 * Matching is substring-based on the outcome half of the payload
 * (text after " - " for status, text after ": " for inspect).
 * A 3-entry lookback covers all call sites where a log_event is
 * immediately followed by a status/inspect in DM1 flows (we never
 * observe more than one intervening entry between the two). */
static int m11_chrome_reroute_already_in_log_pass42(const M11_GameViewState* state,
                                                    const char* text) {
    const char* key = text;
    const char* split;
    int lookback;
    if (!state || !text || text[0] == '\0') {
        return 0;
    }
    /* Extract the "outcome" part of the payload (after " - " for
     * status, after ": " for inspect).  If neither separator is
     * present, match on the whole payload. */
    split = strstr(text, " - ");
    if (split && split[3] != '\0') {
        key = split + 3;
    } else {
        split = strstr(text, ": ");
        if (split && split[2] != '\0') {
            key = split + 2;
        }
    }
    if (!key || key[0] == '\0') {
        return 0;
    }
    /* Look back up to 3 entries; if any contains the key substring
     * (case-sensitive, matches tick-prefixed log_event entries
     * verbatim), treat the reroute as already surfaced. */
    for (lookback = 0; lookback < 3 && lookback < state->messageLog.count; ++lookback) {
        const M11_LogEntry* entry = m11_log_entry_at(&state->messageLog, lookback);
        if (!entry || entry->text[0] == '\0') {
            continue;
        }
        if (strstr(entry->text, key) != NULL) {
            return 1;
        }
    }
    return 0;
}

static void m11_chrome_reroute_push_pass42(M11_GameViewState* state,
                                           unsigned char color,
                                           const char* text,
                                           char* lastSlot,
                                           size_t lastSlotSize) {
    if (!state || !text || text[0] == '\0') {
        return;
    }
    if (!m11_chrome_reroute_is_player_facing_pass42(text)) {
        return;
    }
    /* Suppress if this reroute duplicates a recent m11_log_event
     * entry (avoids double-logging for call sites where both
     * pathways fire, e.g. stair transitions, pit falls, spells). */
    if (m11_chrome_reroute_already_in_log_pass42(state, text)) {
        return;
    }
    if (lastSlot && lastSlotSize > 0 && strcmp(lastSlot, text) == 0) {
        return;
    }
    M11_MessageLog_Push(&state->messageLog, text, color);
    if (lastSlot && lastSlotSize > 0) {
        snprintf(lastSlot, lastSlotSize, "%s", text);
    }
}

static void m11_set_status(M11_GameViewState* state,
                           const char* action,
                           const char* outcome) {
    if (!state) {
        return;
    }
    snprintf(state->lastAction, sizeof(state->lastAction), "%s", action ? action : "NONE");
    snprintf(state->lastOutcome, sizeof(state->lastOutcome), "%s", outcome ? outcome : "");

    /* Pass 42: reroute into message log when V1 chrome mode is on. */
    if (m11_v1_chrome_mode_enabled()) {
        char payload[96];
        const char* a = action ? action : "";
        const char* o = outcome ? outcome : "";
        if (a[0] != '\0' && o[0] != '\0') {
            snprintf(payload, sizeof(payload), "%s - %s", a, o);
        } else if (a[0] != '\0') {
            snprintf(payload, sizeof(payload), "%s", a);
        } else {
            snprintf(payload, sizeof(payload), "%s", o);
        }
        m11_chrome_reroute_push_pass42(state, M11_COLOR_YELLOW, payload,
                                       state->chromeRerouteLastStatus,
                                       sizeof(state->chromeRerouteLastStatus));
    }
}

static void m11_set_inspect_readout(M11_GameViewState* state,
                                    const char* title,
                                    const char* detail) {
    if (!state) {
        return;
    }
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s",
             title ? title : "NO FOCUS");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail), "%s",
             detail ? detail : "PRESS ENTER ON A REAL FRONT-CELL TARGET");

    /* Pass 42: reroute player-facing inspect readouts into the
     * message log when V1 chrome mode is on.  The invented
     * two-line surface is already debug-only; this gives the
     * strings a DM1-style surface in V1 mode. */
    if (m11_v1_chrome_mode_enabled()) {
        char payload[96];
        const char* t = title ? title : "";
        const char* d = detail ? detail : "";
        if (t[0] != '\0' && d[0] != '\0') {
            snprintf(payload, sizeof(payload), "%s: %s", t, d);
        } else if (t[0] != '\0') {
            snprintf(payload, sizeof(payload), "%s", t);
        } else {
            snprintf(payload, sizeof(payload), "%s", d);
        }
        m11_chrome_reroute_push_pass42(state, M11_COLOR_LIGHT_CYAN, payload,
                                       state->chromeRerouteLastInspect,
                                       sizeof(state->chromeRerouteLastInspect));
    }
}

static void m11_refresh_hash(M11_GameViewState* state) {
    uint32_t hash = 0;
    if (!state) {
        return;
    }
    if (F0891_ORCH_WorldHash_Compat(&state->world, &hash) == 1) {
        state->lastWorldHash = hash;
    }
}

static int m11_inspect_front_cell(M11_GameViewState* state);
static int m11_front_cell_has_attack_target(const M11_GameViewState* state);
static int m11_front_cell_is_door(const M11_GameViewState* state);
static int m11_front_cell_mirror_ordinal(const M11_GameViewState* state);
static int m11_get_front_cell(const M11_GameViewState* state, struct M11_ViewportCell* outCell);
static int m11_viewport_cell_is_wall_free(int elementType);
static int m11_toggle_front_door(M11_GameViewState* state);
static int m11_apply_tick(M11_GameViewState* state,
                          uint8_t command,
                          const char* actionLabel);
static void m11_check_party_death(M11_GameViewState* state);
static M12_MenuInput m11_pointer_viewport_input(const M11_GameViewState* state,
                                                int x,
                                                int y);
static void m11_get_active_champion_label(const M11_GameViewState* state,
                                          char* out,
                                          size_t outSize);
static void m11_format_champion_name(const unsigned char* raw,
                                     char* out,
                                     size_t outSize);
static void m11_format_champion_title(const unsigned char* raw,
                                      char* out,
                                      size_t outSize);
static int m11_endgame_source_skill_level(const M11_GameViewState* state, int championIndex, int baseSkillIndex);
static const struct ChampionState_Compat* m11_get_active_champion(const M11_GameViewState* state);
static int m11_cycle_active_champion(M11_GameViewState* state);
static int m11_set_active_champion(M11_GameViewState* state, int championIndex);

static int m11_point_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && y >= ry && x < (rx + rw) && y < (ry + rh);
}

static int m11_point_in_utility_button(int x,
                                       int y,
                                       int buttonX,
                                       int buttonWidth) {
    return m11_point_in_rect(x, y,
                             buttonX,
                             M11_UTILITY_BUTTON_Y,
                             buttonWidth,
                             M11_UTILITY_BUTTON_H);
}

static int m11_v2_vertical_slice_enabled(void) {
    static int cached = -1;
    const char* env;
    if (cached >= 0) {
        return cached;
    }
    env = getenv("FIRESTAFF_V2_VERTICAL_SLICE");
    cached = (env && env[0] != '\0' && strcmp(env, "0") != 0) ? 1 : 0;
    return cached;
}

/* Pass 42: V1 chrome-mode switch.
 *
 * Closes V1_BLOCKERS.md §6 ("Firestaff-invented UI chrome ...") by
 * exposing a single cached toggle that the renderer and the
 * notification helpers consult.  When enabled (the default in V1
 * mode) Firestaff-invented surfaces that have no DM1 PC 3.4
 * equivalent are either suppressed entirely or reduced to the
 * source-faithful message-log surface at the bottom of the screen.
 *
 * The following surfaces are affected in V1-chrome mode:
 *
 *   - Control strip at (14, 165, 88, 14)  -> SUPPRESSED
 *     (Firestaff-invented per PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md
 *      Pass 35 §2.6; no DM1 equivalent.)
 *   - Status-lozenge + inspect-readout writes (m11_set_status /
 *     m11_set_inspect_readout) are additionally rerouted into the
 *     rolling message log when the content is player-facing
 *     (m11_v1_message_is_player_facing).  The invented rendering
 *     surfaces for those strings (utility panel overlay, focus
 *     card) are already debug-only (showDebugHUD), so the reroute
 *     gives the notifications a visible path in V1 while the
 *     chrome remains hidden.
 *   - Bottom message surface is expanded from a single line at
 *     y=149 to a multi-line log region (up to 3 lines at y=149,
 *     157, 165) stepping by 8 px, matching TEXT.C message-log
 *     stride.  This is the source-faithful reroute target for the
 *     status/inspect notifications.
 *
 * The following surfaces are NOT touched by pass 42 (they either
 * already match DM1 source or are tracked as separate blockers):
 *
 *   - Viewport rectangle (V1_BLOCKERS.md §4; depends on pass 42
 *     chrome removal *and* pass 47b ZONES.H parse before the
 *     viewport can be bound to M11_DM1_VIEWPORT_*).
 *   - Champion HP/stamina/mana numeric -> bar graph (§7, pass 43).
 *   - Spell rune text -> C011 blit (§8, pass 44).
 *   - Font atlas (§9, pass 45).
 *   - VGA palette (§10, pass 46).
 *
 * Default: V1 chrome mode ON.  Disable via FIRESTAFF_V1_CHROME=0
 * (kept for A/B measurement and compat with any legacy tooling
 * that expected the control-strip overlay).  V2 vertical-slice
 * mode forces V1 chrome OFF so the pre-baked HUD sprite layout is
 * not visually stripped; V2 is not on the V1 parity path
 * (V1_BLOCKERS.md scope notes).
 *
 * Ref: V1_BLOCKERS.md §6; PARITY_V1_TEXT_VS_GRAPHICS_AUDIT.md
 * Pass 35 §2.2, §2.4, §2.5, §2.6; PARITY_MATRIX_DM1_V1.md §4;
 * parity-evidence/pass42_chrome_reduction.md.
 */
static int m11_v1_chrome_mode_enabled(void) {
    static int cached = -1;
    const char* env;
    if (cached >= 0) {
        return cached;
    }
    /* V2 vertical-slice mode is not on the V1 parity path.  Force
     * V1 chrome mode OFF when V2 is enabled so the pre-baked HUD
     * sprite composition remains intact. */
    if (m11_v2_vertical_slice_enabled()) {
        cached = 0;
        return cached;
    }
    env = getenv("FIRESTAFF_V1_CHROME");
    if (env && env[0] != '\0' && strcmp(env, "0") == 0) {
        cached = 0;
    } else {
        cached = 1;
    }
    return cached;
}



/* Pass 41: mode-aware champion status-box stride / width.
 *
 * In V1 original-faithful mode (the default; V2 vertical slice not
 * enabled) the runtime uses the DM1 DEFS.H-anchored stride of 69 px
 * (C69_CHAMPION_STATUS_BOX_SPACING) and a slot width of 67 px (the
 * C007_GRAPHIC_STATUS_BOX frame width).  This closes the +8 px per
 * slot drift recorded in pass 34 (`parity-evidence/
 * pass34_sidepanel_rectangle_table.md` §3).
 *
 * In V2 vertical-slice mode the pre-baked 302x28 four-slot HUD
 * sprite (m11_v2_party_hud_four_slot_base) and 71x28 active
 * overlay (m11_v2_party_hud_four_slot_active_overlay) assume the
 * legacy 77/71 geometry, so we keep that here.  V2 mode is not on
 * the V1 parity path (see V1_BLOCKERS.md §4 scope notes).
 *
 * Ref: V1_BLOCKERS.md §5; DEFS.H:2157; Pass 41 evidence file
 * parity-evidence/pass41_status_box_stride.md. */
static int m11_party_slot_step(void) {
    return m11_v2_vertical_slice_enabled()
        ? (int)M11_PARTY_SLOT_STEP
        : (int)M11_V1_PARTY_SLOT_STEP;
}

static int m11_party_slot_w(void) {
    return m11_v2_vertical_slice_enabled()
        ? (int)M11_PARTY_SLOT_W
        : (int)M11_V1_PARTY_SLOT_W;
}

/* Pass 43: DM1 PC 3.4 source-faithful bar-graph mode switch.
 *
 * Default: ON in V1 original-faithful mode; OFF under V2
 * vertical-slice (which composites a pre-baked HUD sprite that
 * includes its own legacy horizontal strip).  Opt-out env var
 * FIRESTAFF_V1_BAR_GRAPHS=0 for A/B measurement only.
 *
 * When ON, the champion status-box renders three vertical bars
 * (HP/stamina/mana) using the ZONES.H-anchored geometry captured
 * in the M11_V1_BAR_* enum above, matching the visible behavior
 * of CHAMDRAW.C F0287_CHAMPION_DrawBarGraphs: blank portion is
 * darkest-gray (C12), filled portion is the per-champion color
 * (G0046_auc_Graphic562_ChampionColor[championIndex], reconstructed
 * as M11_GameView_GetV1ChampionBarColor() below), bar drains from the top.
 *
 * Ref: V1_BLOCKERS.md §7; firestaff_pc34_core_amalgam.c
 * CHAMDRAW.C F0287; zones_h_reconstruction.json records 187..206;
 * parity-evidence/pass43_bar_graphs.md. */
static int m11_v1_bar_graphs_enabled(void) {
    static int cached = -1;
    const char* env;
    if (cached >= 0) {
        return cached;
    }
    if (m11_v2_vertical_slice_enabled()) {
        cached = 0;
        return cached;
    }
    env = getenv("FIRESTAFF_V1_BAR_GRAPHS");
    if (env && env[0] != '\0' && strcmp(env, "0") == 0) {
        cached = 0;
    } else {
        cached = 1;
    }
    return cached;
}

/* Pass 43: per-champion color reconstruction.
 *
 * In DM1 PC 3.4 the champion color table lives inside graphic 562
 * (G0046_auc_Graphic562_ChampionColor, 4 entries of palette index
 * bytes).  Extracting the exact bytes from GRAPHICS.DAT entry 562
 * is out of pass-43 scope (blocker §15 / graphic 562 content map
 * is a separate item).  The exact four palette indices are still
 * recoverable from the local ReDMCSB DATA.C declaration for
 * G0046_auc_Graphic562_ChampionColor, which defines
 * `{ 7, 11, 8, 14 }`.  Those bytes are mirrored here directly so
 * the visible bar colors match the source while the geometry stays
 * bound to the recovered ZONES.H layout.
 *
 * Honesty trail: when graphic 562 itself is decoded semantically,
 * this helper can swap from the DATA.C mirror to the extracted
 * bytes without changing the rendered output. */
int M11_GameView_GetV1ChampionBarColor(int championIndex) {
    static const unsigned char colorTable[4] = {
        (unsigned char)M11_COLOR_LIGHT_GREEN, /* DATA.C G0046[0] = 7  */
        (unsigned char)M11_COLOR_YELLOW,      /* DATA.C G0046[1] = 11 */
        (unsigned char)M11_COLOR_RED,         /* DATA.C G0046[2] = 8  */
        (unsigned char)M11_COLOR_LIGHT_BLUE   /* DATA.C G0046[3] = 14 */
    };
    if (championIndex < 0 || championIndex > 3) {
        return M11_COLOR_SILVER; /* C13_COLOR_LIGHTEST_GRAY */
    }
    return (int)colorTable[championIndex];
}

int M11_GameView_GetV1StatusBarBlankColor(void) {
    return M11_COLOR_DARK_GRAY;
}

/* Pass 43: return the top-left pixel coordinate inside a 67x29
 * status-box frame for the bar container at the given statIndex
 * (0 = HP, 1 = stamina, 2 = mana).  Computed once here so tests
 * and the probe can reproduce the exact pixel placement.
 *
 * Zone algebra (type 7 = bottom-center anchor within 4x25
 * container, container anchored at region top-left):
 *   container_top_y  = region_y + region_h - container_h
 *                    = 0 + 29 - 25 = 4         [inside status box]
 *   container_left_x = region_x + d1 - container_w/2 - offset
 *                    ≈ region_x + d1 - 2
 *                    = 43 + d1 - 2
 *
 * For the three bars d1 ∈ {5, 12, 19}, yielding container
 * left edges at x = 46, 53, 60 inside the 67-wide status box.
 * The bar container height is 25 and its bottom edge is at
 * y = container_top_y + container_h = 29 (flush with the
 * 29-tall status-box frame bottom). */
static int m11_v1_bar_graph_x(int statIndex) {
    int d1;
    switch (statIndex) {
        case 0: d1 = (int)M11_V1_BAR_HP_CX;      break;
        case 1: d1 = (int)M11_V1_BAR_STAMINA_CX; break;
        case 2: d1 = (int)M11_V1_BAR_MANA_CX;    break;
        default: return -1;
    }
    /* Center the 4-wide bar on d1 inside region-x.  With
     * COORD.C type-7 centering and (container_w+1)/2 = 2, the
     * offset is d1 - 2. */
    return (int)M11_V1_BAR_GRAPH_REGION_X + d1 - 2;
}

static int m11_v1_bar_graph_y_top(void) {
    /* container_top = region_y + region_h - container_h */
    return (int)M11_V1_BAR_GRAPH_REGION_Y +
           (int)M11_V1_BAR_GRAPH_REGION_H -
           (int)M11_V1_BAR_CONTAINER_H;
}

static int m11_v1_status_hand_x(int handIndex) {
    return handIndex == 0
        ? (int)M11_V1_STATUS_READY_HAND_X
        : (int)M11_V1_STATUS_ACTION_HAND_X;
}

static void m11_blit_v2_slice_asset(const M11_V2SliceAsset* asset,
                                    unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    int dstX,
                                    int dstY,
                                    int transparentZero) {
    int y;
    int x;
    if (!asset || !asset->pixels || !framebuffer) {
        return;
    }
    for (y = 0; y < (int)asset->height; ++y) {
        for (x = 0; x < (int)asset->width; ++x) {
            unsigned char px = asset->pixels[y * (int)asset->width + x];
            if (transparentZero && px == 0U) {
                continue;
            }
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                          dstX + x, dstY + y, px);
        }
    }
}

static const struct ChampionState_Compat* m11_get_active_champion(const M11_GameViewState* state) {
    int index;

    if (!state) {
        return NULL;
    }

    index = state->world.party.activeChampionIndex;
    if (index < 0 || index >= CHAMPION_MAX_PARTY) {
        return NULL;
    }
    if (index >= state->world.party.championCount || !state->world.party.champions[index].present) {
        return NULL;
    }

    return &state->world.party.champions[index];
}

/* ================================================================
 * Message log — ring buffer for event history
 * ================================================================ */

void M11_MessageLog_Push(M11_MessageLog* log, const char* text, unsigned char color) {
    M11_LogEntry* entry;
    if (!log || !text) {
        return;
    }
    entry = &log->entries[log->writeIndex];
    snprintf(entry->text, M11_MESSAGE_MAX_LENGTH, "%s", text);
    entry->color = color;
    log->writeIndex = (log->writeIndex + 1) % M11_MESSAGE_LOG_CAPACITY;
    if (log->count < M11_MESSAGE_LOG_CAPACITY) {
        ++log->count;
    }
}

int M11_GameView_GetMessageLogCount(const M11_GameViewState* state) {
    return state ? state->messageLog.count : 0;
}

const char* M11_GameView_GetMessageLogEntry(const M11_GameViewState* state, int reverseIndex) {
    int idx;
    if (!state || reverseIndex < 0 || reverseIndex >= state->messageLog.count) {
        return NULL;
    }
    idx = (state->messageLog.writeIndex - 1 - reverseIndex + M11_MESSAGE_LOG_CAPACITY) % M11_MESSAGE_LOG_CAPACITY;
    return state->messageLog.entries[idx].text;
}

static const M11_LogEntry* m11_log_entry_at(const M11_MessageLog* log, int reverseIndex) {
    int idx;
    if (!log || reverseIndex < 0 || reverseIndex >= log->count) {
        return NULL;
    }
    idx = (log->writeIndex - 1 - reverseIndex + M11_MESSAGE_LOG_CAPACITY) % M11_MESSAGE_LOG_CAPACITY;
    return &log->entries[idx];
}

/* V1 whitelist: only surface genuinely player-facing events as the
 * single bottom-screen message. Debug-like narration (LOADED, PARTY
 * MOVED, SPELL PANEL OPENED, RUNE <x> (<n>), DOOR STATE CHANGED, etc.)
 * is suppressed in V1 so the screen reads like classic DM rather than
 * an event log. Matching is done on the raw log text AFTER tick-prefix
 * stripping. */
static int m11_v1_message_is_player_facing(const char* stripped) {
    static const char* const kSuppress[] = {
        "DUNGEON MASTER LOADED",
        "CHAOS STRIKES BACK LOADED",
        "DUNGEON MASTER II LOADED",
        "PARTY MOVED TO",
        "PARTY MOVED",
        "SPELL PANEL OPENED",
        "SPELL CLEARED",
        "RUNE ",                 /* "RUNE LO (96)" */
        "DOOR STATE CHANGED",
        "IDLE TICK",
        "GAME VIEW NOT STARTED",
        "GAME DATA LOADED",
        "FACING UPDATED",
        "TURN IGNORED",
        "MOVEMENT BLOCKED",
        "STRIKE COMMITTED",
        "SPELL COMMITTED",
        " FADES",
        " OUT OF BOUNDS",
        " COLLIDES IN FLIGHT",
        "CAST SPELL #",
        NULL
    };
    int i;
    if (!stripped || stripped[0] == '\0') {
        return 0;
    }
    for (i = 0; kSuppress[i] != NULL; ++i) {
        if (strstr(stripped, kSuppress[i]) != NULL) {
            return 0;
        }
    }
    return 1;
}

static void m11_log_event(M11_GameViewState* state, unsigned char color, const char* fmt, ...) {
    char buf[M11_MESSAGE_MAX_LENGTH];
    va_list ap;
    if (!state) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    M11_MessageLog_Push(&state->messageLog, buf, color);
}

/* ================================================================
 * Explored cell tracking
 * ================================================================ */

static void m11_mark_explored(M11_GameViewState* state) {
    unsigned int cell;
    if (!state || !state->active) {
        return;
    }
    cell = (unsigned int)(state->world.party.mapX * 32 + state->world.party.mapY);
    if (cell < 1024U) {
        state->exploredBits[cell / 32U] |= (1U << (cell % 32U));
    }
}

static int m11_is_explored(const M11_GameViewState* state, int mapX, int mapY) {
    unsigned int cell;
    if (!state) {
        return 0;
    }
    cell = (unsigned int)(mapX * 32 + mapY);
    if (cell >= 1024U) {
        return 0;
    }
    return (state->exploredBits[cell / 32U] & (1U << (cell % 32U))) != 0;
}

/* ================================================================
 * Level transitions
 * ================================================================ */

/*
 * Thin UI/status delegation around the compat-owned stairs resolver
 * F0705_MOVEMENT_ResolveStairsTransition_Compat (MOVESENS.C stairs branch).
 * World mutation + level clamp live entirely in compat; this helper only
 * applies the mutation to the world, resets explored bits, and emits the
 * M11 status/log/inspect lines.
 */
static int m11_try_stairs_transition(M11_GameViewState* state) {
    struct StairsTransitionResult_Compat stairs;
    const struct DungeonMapDesc_Compat* targetMap;

    if (!state || !state->active || !state->world.dungeon) {
        return 0;
    }

    if (!F0705_MOVEMENT_ResolveStairsTransition_Compat(state->world.dungeon,
                                                       &state->world.party,
                                                       &stairs)) {
        /* Not on stairs at all. */
        return 0;
    }
    if (!stairs.transitioned) {
        /* On stairs but the target level is out of range. */
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: STAIRS LEAD NOWHERE",
                      (unsigned int)state->world.gameTick);
        return 0;
    }

    state->world.party.mapIndex = stairs.toMapIndex;
    state->world.party.mapX = stairs.newMapX;
    state->world.party.mapY = stairs.newMapY;
    state->world.party.direction = stairs.newDirection;

    memset(state->exploredBits, 0, sizeof(state->exploredBits));
    m11_mark_explored(state);
    m11_refresh_hash(state);
    targetMap = &state->world.dungeon->maps[stairs.toMapIndex];
    if (stairs.stairUp) {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: ASCENDED TO LEVEL %d",
                      (unsigned int)state->world.gameTick,
                      stairs.toMapIndex + 1);
        m11_set_status(state, "STAIRS", "ASCENDED TO PREVIOUS LEVEL");
    } else {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: DESCENDED TO LEVEL %d",
                      (unsigned int)state->world.gameTick,
                      stairs.toMapIndex + 1);
        m11_set_status(state, "STAIRS", "DESCENDED TO NEXT LEVEL");
    }
    snprintf(state->inspectTitle, sizeof(state->inspectTitle),
             "LEVEL %d", stairs.toMapIndex + 1);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "MAP %dx%d, ENTERED FROM STAIRS",
             (int)targetMap->width, (int)targetMap->height);
    return 1;
}

/* ================================================================
 * Post-move environment transitions now belong to compat/runtime.
 * This M11 shim exists only for probe coverage of the legacy public entry.
 * ================================================================ */

static int m11_apply_post_move_environment_from_compat(M11_GameViewState* state) {
    struct PostMoveResolution_Compat resolution;
    int i;

    if (!state || !state->active || !state->world.dungeon) {
        return 0;
    }
    memset(&resolution, 0, sizeof(resolution));
    if (!F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
            state->world.dungeon,
            state->world.things,
            &state->world.party,
            state->world.gameTick,
            &resolution)) {
        return 0;
    }
    if (!resolution.transitioned) {
        return 0;
    }

    state->world.party.mapX = resolution.finalMapX;
    state->world.party.mapY = resolution.finalMapY;
    state->world.party.direction = resolution.finalDirection;
    state->world.party.mapIndex = resolution.finalMapIndex;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (resolution.championFallDamage[i] > 0 &&
            state->world.party.champions[i].present &&
            state->world.party.champions[i].hp.current > 0) {
            int hp = (int)state->world.party.champions[i].hp.current - resolution.championFallDamage[i];
            state->world.party.champions[i].hp.current = (int16_t)((hp > 0) ? hp : 0);
        }
    }
    memset(state->exploredBits, 0, sizeof(state->exploredBits));
    m11_mark_explored(state);
    m11_refresh_hash(state);
    if (resolution.pitCount > 0) {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: FELL INTO PIT! LEVEL %d",
                      (unsigned int)state->world.gameTick,
                      state->world.party.mapIndex + 1);
        m11_set_status(state, "PIT", "FELL TO NEXT LEVEL");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "PIT FALL");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "DROPPED TO LEVEL %d, POSITION (%d,%d)",
                 state->world.party.mapIndex + 1,
                 state->world.party.mapX,
                 state->world.party.mapY);
    }
    if (resolution.teleporterCount > 0) {
        m11_log_event(state, M11_COLOR_LIGHT_CYAN, "T%u: TELEPORTED TO MAP %d (%d,%d)",
                      (unsigned int)state->world.gameTick,
                      state->world.party.mapIndex + 1,
                      state->world.party.mapX,
                      state->world.party.mapY);
        m11_set_status(state, "TELEPORT", "PARTY TRANSPORTED");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "TELEPORTER");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "ARRIVED AT LEVEL %d, POSITION (%d,%d)",
                 state->world.party.mapIndex + 1,
                 state->world.party.mapX,
                 state->world.party.mapY);
    }
    return 1;
}

static int m11_check_post_move_transitions(M11_GameViewState* state) {
    return m11_apply_post_move_environment_from_compat(state);
}

int M11_GameView_CheckPostMoveTransitions(M11_GameViewState* state) {
    return m11_check_post_move_transitions(state);
}

/* ================================================================
 * Item name helpers
 * ================================================================ */

static const char* const s_weaponTypeNames[] = {
    "EYE OF TIME", "STORMRING", "TORCH", "FLAMITT",
    "STAFF OF CLAWS", "BOLT BLADE", "FURY", "THE FIRESTAFF",
    "DAGGER", "FALCHION", "SWORD", "RAPIER",
    "SABRE", "SAMURAI SWORD", "DELTA", "DIAMOND EDGE",
    "VORPAL BLADE", "THE INQUISITOR", "AXE", "HARDCLEAVE",
    "MACE", "MACE OF ORDER", "MORNING STAR", "CLUB",
    "STONE CLUB", "BOW", "CROSSBOW", "ARROW",
    "SLAYER", "SLING", "ROCK", "POISON DART",
    "THROWING STAR", "STICK", "STAFF", "WAND",
    "TEOWAND", "YEW STAFF", "STAFF OF MANAR", "SNAKE STAFF",
    "THE CONDUIT", "DRAGON SPIT", "SCEPTRE OF LYF", "HORN OF FEAR",
    "SPEED BOW", "THE FIRESTAFF"
};

static const char* const s_armourTypeNames[] = {
    "CAPE", "CLOAK OF NIGHT", "BARBARIAN HIDE", "SANDALS",
    "LEATHER BOOTS", "ELVEN BOOTS", "LEATHER JERKIN", "LEATHER PANTS",
    "SUEDE BOOTS", "BLUE PANTS", "GHI", "GHI TROUSERS",
    "CALISTA", "CROWN OF NERRA", "BEZERKER HELM", "HELMET",
    "BASINET", "NETA SHIRT", "CHAINMAIL", "PLATE MAIL",
    "MITHRAL MAIL", "MITHRAL HOSEN", "LEG MAIL", "FOOT PLATE",
    "SMALL SHIELD", "WOODEN SHIELD", "LARGE SHIELD", "SHIELD OF LYTE",
    "SHIELD OF DARC", "DEXHELM"
};

static const char* const s_potionTypeNames[] = {
    "MON POTION", "UM POTION", "DES POTION", "VEN POTION",
    "SAR POTION", "ZO POTION", "ROS POTION", "KU POTION",
    "DANE POTION", "NETA POTION", "BRO POTION", "MA POTION",
    "YA POTION", "EE POTION", "VI POTION", "WATER FLASK",
    "EMPTY FLASK"
};

static const char* const s_junkTypeNames[] = {
    "COMPASS", "TORCH", "WATERSKIN", "JEWEL SYMAL",
    "ILLUMULET", "ASHES", "BONES", "SAR COIN",
    "GOLD COIN", "IRON KEY", "KEY OF B", "SOLID KEY",
    "SQUARE KEY", "TOURQUOISE KEY", "CROSS KEY", "ONYX KEY",
    "SKELETON KEY", "GOLD KEY", "WINGED KEY", "TOPAZ KEY",
    "SAPPHIRE KEY", "EMERALD KEY", "RUBY KEY", "RA KEY",
    "MASTER KEY", "BOULDER", "BLUE GEM", "ORANGE GEM",
    "GREEN GEM", "APPLE", "CORN", "BREAD",
    "CHEESE", "SCREAMER SLICE", "WORM ROUND", "DRUMSTICK",
    "DRAGON STEAK", "GEM OF AGES", "EKKHARD CROSS", "MOONSTONE",
    "THE HELLION", "PENDANT FERAL", "MAGICAL BOX", "MIRROR OF DAWN",
    "ROPE", "RABBIT FOOT", "CORBAMITE", "CHOKER",
    "LOCK PICKS", "MAGNIFIER", "ZOKATHRA SPELL", "EMPTY FLASK"
};

/* Scroll names not needed yet — all scrolls are just "SCROLL" */

static const char* const s_containerTypeNames[] = {
    "CHEST", "OPEN CHEST", "OPEN CHEST"
};

/* Creature type name table (C00..C26, matching g_profiles order). */
static const char* const s_creatureTypeNames[27] = {
    "GIANT SCORPION",  /* C00 */
    "SWAMP SLIME",     /* C01 */
    "GIGGLER",         /* C02 */
    "WIZARD EYE",      /* C03 */
    "PAIN RAT",        /* C04 */
    "RUSTER",          /* C05 */
    "SCREAMER",        /* C06 */
    "ROCKPILE",        /* C07 */
    "GHOST",           /* C08 */
    "STONE GOLEM",     /* C09 */
    "MUMMY",           /* C10 */
    "BLACK FLAME",     /* C11 */
    "SKELETON",        /* C12 */
    "COUATL",          /* C13 */
    "VEXIRK",          /* C14 */
    "MAGENTA WORM",    /* C15 */
    "TROLIN",          /* C16 */
    "GIANT WASP",      /* C17 */
    "ANIMATED ARMOUR", /* C18 */
    "MATERIALIZER",    /* C19 */
    "WATER ELEMENTAL", /* C20 */
    "OITU",            /* C21 */
    "DEMON",           /* C22 */
    "LORD CHAOS",      /* C23 */
    "RED DRAGON",      /* C24 */
    "LORD ORDER",      /* C25 */
    "GREY LORD"        /* C26 */
};

static const char* m11_creature_name(int creatureType) {
    if (creatureType >= 0 && creatureType < 27) {
        return s_creatureTypeNames[creatureType];
    }
    return "UNKNOWN CREATURE";
}

static void m11_get_item_name(const struct DungeonThings_Compat* things,
                              unsigned short thingId,
                              char* out,
                              size_t outSize) {
    int thingType;
    int thingIndex;
    if (!out || outSize == 0U) {
        return;
    }
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) {
        snprintf(out, outSize, "NOTHING");
        return;
    }
    thingType = THING_GET_TYPE(thingId);
    thingIndex = THING_GET_INDEX(thingId);
    switch (thingType) {
        case THING_TYPE_WEAPON:
            if (things->weapons && thingIndex >= 0 && thingIndex < things->weaponCount) {
                int subtype = things->weapons[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_weaponTypeNames) / sizeof(s_weaponTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_weaponTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "WEAPON %d", thingIndex);
            return;
        case THING_TYPE_ARMOUR:
            if (things->armours && thingIndex >= 0 && thingIndex < things->armourCount) {
                int subtype = things->armours[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_armourTypeNames) / sizeof(s_armourTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_armourTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "ARMOUR %d", thingIndex);
            return;
        case THING_TYPE_POTION:
            if (things->potions && thingIndex >= 0 && thingIndex < things->potionCount) {
                int subtype = things->potions[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_potionTypeNames) / sizeof(s_potionTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_potionTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "POTION %d", thingIndex);
            return;
        case THING_TYPE_JUNK:
            if (things->junks && thingIndex >= 0 && thingIndex < things->junkCount) {
                int subtype = things->junks[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_junkTypeNames) / sizeof(s_junkTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_junkTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "JUNK %d", thingIndex);
            return;
        case THING_TYPE_SCROLL:
            snprintf(out, outSize, "SCROLL");
            return;
        case THING_TYPE_CONTAINER:
            if (things->containers && thingIndex >= 0 && thingIndex < things->containerCount) {
                int subtype = things->containers[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_containerTypeNames) / sizeof(s_containerTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_containerTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "CONTAINER %d", thingIndex);
            return;
        default:
            snprintf(out, outSize, "%s %d",
                     F0505_DUNGEON_GetThingTypeName_Compat(thingType), thingIndex);
            return;
    }
}

/* ================================================================
 * Thing chain manipulation: remove an item from a square,
 * prepend an item to a square
 * ================================================================ */

static void m11_set_raw_next_thing(struct DungeonThings_Compat* things,
                                   unsigned short thingId,
                                   unsigned short newNext) {
    int type;
    int index;
    unsigned char* raw;
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) {
        return;
    }
    type = THING_GET_TYPE(thingId);
    index = THING_GET_INDEX(thingId);
    if (type < 0 || type >= 16 || !things->rawThingData[type] ||
        index < 0 || index >= things->thingCounts[type]) {
        return;
    }
    raw = things->rawThingData[type] + (index * s_thingDataByteCount[type]);
    raw[0] = (unsigned char)(newNext & 0xFFu);
    raw[1] = (unsigned char)((newNext >> 8) & 0xFFu);
}

/* Remove a specific thing from a square's chain. Returns 1 if removed. */
static int m11_unlink_thing_from_square(struct GameWorld_Compat* world,
                                        int mapIndex,
                                        int mapX,
                                        int mapY,
                                        unsigned short target) {
    int base;
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    unsigned short current;
    unsigned short prev;
    int safety = 0;

    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return 0;
    }
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) {
        return 0;
    }
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    current = world->things->squareFirstThings[squareIndex];
    prev = THING_ENDOFLIST;

    while (current != THING_ENDOFLIST && current != THING_NONE && safety < 64) {
        if (current == target) {
            unsigned short next = m11_raw_next_thing(world->things, current);
            if (prev == THING_ENDOFLIST) {
                world->things->squareFirstThings[squareIndex] = next;
            } else {
                m11_set_raw_next_thing(world->things, prev, next);
            }
            m11_set_raw_next_thing(world->things, current, THING_ENDOFLIST);
            return 1;
        }
        prev = current;
        current = m11_raw_next_thing(world->things, current);
        ++safety;
    }
    return 0;
}

/* Prepend a thing to a square's chain. */
static int m11_prepend_thing_to_square(struct GameWorld_Compat* world,
                                       int mapIndex,
                                       int mapX,
                                       int mapY,
                                       unsigned short thingId) {
    int base;
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    unsigned short oldFirst;

    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return 0;
    }
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) {
        return 0;
    }
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    oldFirst = world->things->squareFirstThings[squareIndex];
    m11_set_raw_next_thing(world->things, thingId, oldFirst);
    world->things->squareFirstThings[squareIndex] = thingId;
    return 1;
}

/* Find the first item-type thing on a square. */
static unsigned short m11_find_first_item_on_square(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);
    int safety = 0;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
        if (m11_thing_is_item(THING_GET_TYPE(thing))) {
            return thing;
        }
        thing = m11_raw_next_thing(world->things, thing);
        ++safety;
    }
    return THING_NONE;
}

/* Find the first empty backpack/pouch slot for a champion. */
static int m11_find_empty_slot(const struct ChampionState_Compat* champ) {
    int slot;
    if (!champ || !champ->present) {
        return -1;
    }
    /* Prefer hands first, then pouches, then backpack */
    if (champ->inventory[CHAMPION_SLOT_HAND_LEFT] == THING_NONE) {
        return CHAMPION_SLOT_HAND_LEFT;
    }
    if (champ->inventory[CHAMPION_SLOT_HAND_RIGHT] == THING_NONE) {
        return CHAMPION_SLOT_HAND_RIGHT;
    }
    for (slot = CHAMPION_SLOT_POUCH_1; slot <= CHAMPION_SLOT_POUCH_2; ++slot) {
        if (champ->inventory[slot] == THING_NONE) {
            return slot;
        }
    }
    for (slot = CHAMPION_SLOT_BACKPACK_1; slot <= CHAMPION_SLOT_BACKPACK_8; ++slot) {
        if (champ->inventory[slot] == THING_NONE) {
            return slot;
        }
    }
    return -1;
}

/* Count items in a champion's inventory. */
int M11_GameView_CountChampionItems(const M11_GameViewState* state, int championIndex) {
    int count = 0;
    int slot;
    const struct ChampionState_Compat* champ;
    if (!state || championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }
    champ = &state->world.party.champions[championIndex];
    if (!champ->present) {
        return 0;
    }
    for (slot = 0; slot < CHAMPION_SLOT_COUNT; ++slot) {
        if (champ->inventory[slot] != THING_NONE) {
            ++count;
        }
    }
    return count;
}

int M11_GameView_GetSkillLevel(const M11_GameViewState* state,
                               int championIndex,
                               int skillIndex) {
    if (!state || !state->active) return -1;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return -1;
    if (!state->world.party.champions[championIndex].present) return -1;
    if (skillIndex < 0 || skillIndex >= CHAMPION_SKILL_COUNT) return -1;
    return F0848_LIFECYCLE_ComputeSkillLevel_Compat(
        &state->world.lifecycle.champions[championIndex],
        skillIndex, 0);
}

/* ================================================================
 * Pickup: take first item from current cell -> champion inventory
 * ================================================================ */

int M11_GameView_PickupItem(M11_GameViewState* state) {
    unsigned short item;
    int targetSlot;
    struct ChampionState_Compat* champ;
    char itemName[48];
    char champName[16];

    if (!state || !state->active || state->partyDead) {
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "PICKUP", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "PICKUP", "CHAMPION CANNOT ACT");
        return 0;
    }

    item = m11_find_first_item_on_square(
        &state->world,
        state->world.party.mapIndex,
        state->world.party.mapX,
        state->world.party.mapY);
    if (item == THING_NONE) {
        m11_set_status(state, "PICKUP", "NOTHING TO PICK UP");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "EMPTY FLOOR");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "NO ITEMS ON THIS CELL");
        return 0;
    }

    targetSlot = m11_find_empty_slot(champ);
    if (targetSlot < 0) {
        m11_set_status(state, "PICKUP", "INVENTORY FULL");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "HANDS FULL");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "DROP SOMETHING FIRST (P KEY)");
        return 0;
    }

    if (!m11_unlink_thing_from_square(&state->world,
                                      state->world.party.mapIndex,
                                      state->world.party.mapX,
                                      state->world.party.mapY,
                                      item)) {
        m11_set_status(state, "PICKUP", "CHAIN ERROR");
        return 0;
    }

    champ->inventory[targetSlot] = item;
    m11_get_item_name(state->world.things, item, itemName, sizeof(itemName));
    m11_format_champion_name(champ->name, champName, sizeof(champName));

    m11_log_event(state, M11_COLOR_LIGHT_GREEN, "T%u: %s TOOK %s",
                  (unsigned int)state->world.gameTick, champName, itemName);
    m11_set_status(state, "PICKUP", "ITEM TAKEN");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "PICKED UP");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "%s -> %s SLOT %d", itemName, champName, targetSlot);
    m11_refresh_hash(state);
    return 1;
}

/* ================================================================
 * Drop: take item from champion hand/slot -> current cell
 * ================================================================ */

int M11_GameView_DropItem(M11_GameViewState* state) {
    struct ChampionState_Compat* champ;
    unsigned short item = THING_NONE;
    int dropSlot = -1;
    int slot;
    char itemName[48];
    char champName[16];

    if (!state || !state->active || state->partyDead) {
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "DROP", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "DROP", "CHAMPION CANNOT ACT");
        return 0;
    }

    /* Drop from hands first (right, then left), then last backpack item */
    if (champ->inventory[CHAMPION_SLOT_HAND_RIGHT] != THING_NONE) {
        dropSlot = CHAMPION_SLOT_HAND_RIGHT;
    } else if (champ->inventory[CHAMPION_SLOT_HAND_LEFT] != THING_NONE) {
        dropSlot = CHAMPION_SLOT_HAND_LEFT;
    } else {
        for (slot = CHAMPION_SLOT_BACKPACK_8; slot >= CHAMPION_SLOT_BACKPACK_1; --slot) {
            if (champ->inventory[slot] != THING_NONE) {
                dropSlot = slot;
                break;
            }
        }
        if (dropSlot < 0) {
            for (slot = CHAMPION_SLOT_POUCH_2; slot >= CHAMPION_SLOT_POUCH_1; --slot) {
                if (champ->inventory[slot] != THING_NONE) {
                    dropSlot = slot;
                    break;
                }
            }
        }
    }

    if (dropSlot < 0) {
        m11_set_status(state, "DROP", "NOTHING TO DROP");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "EMPTY HANDS");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "PICK SOMETHING UP FIRST (G KEY)");
        return 0;
    }

    item = champ->inventory[dropSlot];
    champ->inventory[dropSlot] = THING_NONE;

    if (!m11_prepend_thing_to_square(&state->world,
                                     state->world.party.mapIndex,
                                     state->world.party.mapX,
                                     state->world.party.mapY,
                                     item)) {
        /* Restore if chain operation fails */
        champ->inventory[dropSlot] = item;
        m11_set_status(state, "DROP", "CHAIN ERROR");
        return 0;
    }

    m11_get_item_name(state->world.things, item, itemName, sizeof(itemName));
    m11_format_champion_name(champ->name, champName, sizeof(champName));

    m11_log_event(state, M11_COLOR_YELLOW, "T%u: %s DROPPED %s",
                  (unsigned int)state->world.gameTick, champName, itemName);
    m11_set_status(state, "DROP", "ITEM DROPPED");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "DROPPED");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "%s FROM %s SLOT %d", itemName, champName, dropSlot);
    m11_refresh_hash(state);
    return 1;
}

/* Forward declarations for spell casting */
/* Forward declaration — public, see header */

/* ================================================================
 * Spell casting UI
 * ================================================================ */

/* DM1 rune names per row. Row 0 = power runes, rows 1-3 = element/form/class. */
static const char* const g_rune_names[4][6] = {
    { "LO",  "UM",  "ON",  "EE",  "PAL", "MON" },
    { "YA",  "VI",  "OH",  "FUL", "DES", "ZO"  },
    { "VEN", "EW",  "KATH","IR",  "BRO", "GOR" },
    { "KU",  "ROS", "DAIN","NETA","RA",  "SAR" }
};

/* Encode a rune symbol from (row, column) into the DM1 byte value.
 * Per SYMBOL.C:F0399: runeValue = 0x60 + 6*row + column. */
static int m11_encode_rune(int row, int col) {
    if (row < 0 || row > 3 || col < 0 || col > 5) return -1;
    return 0x60 + 6 * row + col;
}

enum {
    M11_SPELL_LABEL_CELL_W = 14,
    M11_SPELL_LABEL_CELL_H = 13,
    M11_SPELL_LABEL_AVAILABLE_Y = 13,
    M11_SPELL_LABEL_SELECTED_Y = 26
};

static void m11_get_rune_abbrev(int row, int col, char out[3]) {
    const char* src;
    if (!out) {
        return;
    }
    out[0] = '?';
    out[1] = '?';
    out[2] = '\0';
    if (row < 0 || row >= 4 || col < 0 || col >= 6) {
        return;
    }
    src = g_rune_names[row][col];
    if (!src || src[0] == '\0') {
        return;
    }
    out[0] = src[0];
    out[1] = src[1] ? src[1] : ' ';
}

int M11_GameView_GetV1SpellAreaLinesGraphicId(void) {
    /* C011_GRAPHIC_MENU_SPELL_AREA_LINES; kept near rune-cell blitter so
     * the source graphic id is probe-visible before the asset enum block. */
    return 11;
}

int M11_GameView_GetV1SpellAvailableSymbolParentZoneId(int symbolIndex) {
    if (symbolIndex < 0 || symbolIndex >= 6) return 0;
    /* Source layout-696 C245..C250 parent zones for available spell symbols. */
    return 245 + symbolIndex;
}

int M11_GameView_GetV1SpellAvailableSymbolZoneId(int symbolIndex) {
    if (!M11_GameView_GetV1SpellAvailableSymbolParentZoneId(symbolIndex)) return 0;
    /* Source layout-696 C255..C260 draw zones for available spell symbols. */
    return 255 + symbolIndex;
}

int M11_GameView_GetV1SpellChampionSymbolZoneId(int symbolIndex) {
    if (symbolIndex < 0 || symbolIndex >= 4) return 0;
    /* Source layout-696 C261..C264 selected/champion spell-symbol zones. */
    return 261 + symbolIndex;
}

int M11_GameView_GetV1SpellCastZoneId(void) {
    /* Source layout-696 C252_ZONE_SPELL_AREA_CAST_SPELL. */
    return 252;
}

int M11_GameView_GetV1SpellRecantZoneId(void) {
    /* Source layout-696 C254_ZONE_SPELL_AREA_RECANT_SYMBOL. */
    return 254;
}

int M11_GameView_GetV1SpellLabelCellSourceZone(int selectedLine,
                                                int* outX,
                                                int* outY,
                                                int* outW,
                                                int* outH) {
    if (outX) *outX = 0;
    if (outY) *outY = selectedLine ? M11_SPELL_LABEL_SELECTED_Y
                                   : M11_SPELL_LABEL_AVAILABLE_Y;
    if (outW) *outW = M11_SPELL_LABEL_CELL_W;
    if (outH) *outH = M11_SPELL_LABEL_CELL_H;
    return 1;
}

static int m11_blit_spell_label_cell(const M11_GameViewState* state,
                                     unsigned char* framebuffer,
                                     int framebufferWidth,
                                     int framebufferHeight,
                                     int dstX,
                                     int dstY,
                                     int selectedLine) {
    const M11_AssetSlot* slot;
    int srcX, srcY, srcW, srcH;
    if (!state || !state->assetsAvailable) {
        return 0;
    }
    (void)M11_GameView_GetV1SpellLabelCellSourceZone(selectedLine,
                                                     &srcX, &srcY,
                                                     &srcW, &srcH);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)M11_GameView_GetV1SpellAreaLinesGraphicId());
    if (!slot || (int)slot->width != M11_SPELL_LABEL_CELL_W ||
        (int)slot->height != (M11_SPELL_LABEL_CELL_H * 3)) {
        return 0;
    }
    M11_AssetLoader_BlitRegion(slot,
                               srcX,
                               srcY,
                               srcW,
                               srcH,
                               framebuffer,
                               framebufferWidth,
                               framebufferHeight,
                               dstX,
                               dstY,
                               -1);
    return 1;
}

int M11_GameView_OpenSpellPanel(M11_GameViewState* state) {
    if (!state || !state->active || state->partyDead) return 0;
    state->spellPanelOpen = 1;
    state->spellRuneRow = 0;
    memset(&state->spellBuffer, 0, sizeof(state->spellBuffer));
    m11_log_event(state, M11_COLOR_LIGHT_BLUE, "T%u: SPELL PANEL OPENED",
                  (unsigned int)state->world.gameTick);
    return 1;
}

int M11_GameView_CloseSpellPanel(M11_GameViewState* state) {
    if (!state) return 0;
    state->spellPanelOpen = 0;
    state->spellRuneRow = 0;
    memset(&state->spellBuffer, 0, sizeof(state->spellBuffer));
    return 1;
}

int M11_GameView_EnterRune(M11_GameViewState* state, int symbolIndex) {
    int runeValue;
    if (!state || !state->active || state->partyDead) return 0;
    if (!state->spellPanelOpen) return 0;
    if (symbolIndex < 0 || symbolIndex > 5) return 0;
    if (state->spellBuffer.runeCount >= 4) return 0;

    runeValue = m11_encode_rune(state->spellRuneRow, symbolIndex);
    if (runeValue < 0) return 0;

    state->spellBuffer.runes[state->spellBuffer.runeCount] = runeValue;
    state->spellBuffer.runeCount++;
    state->spellRuneRow++;

    m11_log_event(state, M11_COLOR_WHITE, "T%u: RUNE %s (%d)",
                  (unsigned int)state->world.gameTick,
                  g_rune_names[state->spellRuneRow - 1][symbolIndex],
                  runeValue);

    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "RUNE ENTERED");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "%s — %d OF 4 SYMBOLS",
             g_rune_names[state->spellRuneRow - 1][symbolIndex],
             state->spellBuffer.runeCount);

    /* Auto-close panel after 4 runes (full sequence) */
    if (state->spellRuneRow >= 4) {
        state->spellRuneRow = 3; /* clamp display row */
    }
    return 1;
}

int M11_GameView_ClearSpell(M11_GameViewState* state) {
    if (!state || !state->active) return 0;
    state->spellRuneRow = 0;
    memset(&state->spellBuffer, 0, sizeof(state->spellBuffer));
    if (state->spellPanelOpen) {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: SPELL CLEARED",
                      (unsigned int)state->world.gameTick);
    }
    return 1;
}

int M11_GameView_CastSpell(M11_GameViewState* state) {
    struct ChampionState_Compat* champ;
    struct SpellCastRequest_Compat req;
    struct SpellDefinition_Compat spell;
    struct SpellEffect_Compat effect;
    uint32_t packed = 0;
    int tableIndex = -1;
    int manaCost = 0;
    int failureReason = 0;
    char champName[16];

    if (!state || !state->active || state->partyDead) return 0;
    if (state->spellBuffer.runeCount < 2) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: NEED AT LEAST 2 RUNES",
                      (unsigned int)state->world.gameTick);
        m11_set_status(state, "CAST", "NOT ENOUGH RUNES");
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "CAST", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "CAST", "CHAMPION CANNOT ACT");
        return 0;
    }

    m11_format_champion_name(champ->name, champName, sizeof(champName));

    /* Encode the rune sequence */
    memset(&spell, 0, sizeof(spell));
    if (!F0750_MAGIC_EncodeRuneSequence_Compat(&state->spellBuffer, &packed)) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — INVALID RUNES",
                      (unsigned int)state->world.gameTick, champName);
        m11_set_status(state, "CAST", "INVALID RUNE SEQUENCE");
        M11_GameView_ClearSpell(state);
        state->spellPanelOpen = 0;
        return 0;
    }

    /* Look up spell in table */
    if (!F0752_MAGIC_LookupSpellInTable_Compat(packed, &tableIndex, &spell)) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — MEANINGLESS SPELL",
                      (unsigned int)state->world.gameTick, champName);
        m11_set_status(state, "CAST", "UNKNOWN SPELL");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "SPELL FAILED");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "NO MATCHING SPELL IN TABLE");
        M11_GameView_ClearSpell(state);
        state->spellPanelOpen = 0;
        return 1; /* consumed the input even though spell failed */
    }

    /* Compute mana cost */
    F0753_MAGIC_ComputeManaCost_Compat(&state->spellBuffer, &manaCost);
    if ((int)champ->mana.current < manaCost) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — NOT ENOUGH MANA (%d/%d)",
                      (unsigned int)state->world.gameTick, champName,
                      (int)champ->mana.current, manaCost);
        m11_set_status(state, "CAST", "OUT OF MANA");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "NEED MANA");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "COST %d, HAVE %d", manaCost, (int)champ->mana.current);
        M11_GameView_ClearSpell(state);
        state->spellPanelOpen = 0;
        return 1;
    }

    /* Build a cast request */
    memset(&req, 0, sizeof(req));
    req.championIndex = state->world.party.activeChampionIndex;
    req.currentMana = (int)champ->mana.current;
    req.maximumMana = (int)champ->mana.maximum;
    req.skillLevelForSpell = (spell.skillIndex >= 0 && spell.skillIndex < CHAMPION_SKILL_COUNT)
                                  ? champ->skillLevels[spell.skillIndex] : 0;
    req.statisticWisdom = champ->attributes[CHAMPION_ATTR_WISDOM];
    req.luckCurrent = state->world.magic.luckCurrent;
    req.partyDirection = state->world.party.direction;
    req.partyMapIndex = state->world.party.mapIndex;
    req.partyMapX = state->world.party.mapX;
    req.partyMapY = state->world.party.mapY;
    req.hasEmptyFlaskInHand = 0; /* TODO: check inventory */
    req.hasMagicMapInHand = 0;
    req.gameTimeTicksLow = (int)(state->world.gameTick & 0x7FFFFFFF);
    req.spellTableIndex = tableIndex;
    req.rawSymbolsPacked = (int)packed;

    /* Validate the cast */
    memset(&effect, 0, sizeof(effect));
    if (!F0754_MAGIC_ValidateCastRequest_Compat(&req, &spell,
                                                 state->spellBuffer.runes[0] > 0
                                                     ? (state->spellBuffer.runes[0] - 0x60) / 6 + 1
                                                     : 1,
                                                 &state->world.masterRng,
                                                 &failureReason)) {
        const char* failMsg = "NEEDS MORE PRACTICE";
        if (failureReason == SPELL_FAILURE_MEANINGLESS_SPELL) failMsg = "MEANINGLESS SPELL";
        else if (failureReason == SPELL_FAILURE_NEEDS_FLASK_IN_HAND) failMsg = "NEED FLASK";
        else if (failureReason == SPELL_FAILURE_NEEDS_MAGIC_MAP) failMsg = "NEED MAGIC MAP";
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — %s",
                      (unsigned int)state->world.gameTick, champName, failMsg);
        m11_set_status(state, "CAST", failMsg);
    }

    /* Deduct mana */
    champ->mana.current = (uint16_t)((int)champ->mana.current - manaCost);

    /* Issue CMD_CAST_SPELL through the tick system */
    {
        struct TickInput_Compat input;
        memset(&input, 0, sizeof(input));
        input.tick = state->world.gameTick;
        input.command = CMD_CAST_SPELL;
        input.commandArg1 = (uint8_t)state->world.party.activeChampionIndex;
        input.commandArg2 = (uint8_t)tableIndex;
        input.reserved = (uint8_t)(state->spellBuffer.runes[0] > 0
                                       ? (state->spellBuffer.runes[0] - 0x60) / 6 + 1
                                       : 1);
        memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
        F0884_ORCH_AdvanceOneTick_Compat(&state->world, &input, &state->lastTickResult);
        state->lastWorldHash = state->lastTickResult.worldHashPost;
        M11_GameView_ProcessTickEmissions(state);
    }

    m11_log_event(state, M11_COLOR_GREEN, "T%u: %s CAST SPELL #%d (COST %d MANA)",
                  (unsigned int)state->world.gameTick, champName, tableIndex, manaCost);
    m11_set_status(state, "CAST", "SPELL COMMITTED");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s CASTS", champName);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "SPELL #%d, COST %d MANA, %d REMAINING",
             tableIndex, manaCost, (int)champ->mana.current);

    M11_GameView_ClearSpell(state);
    state->spellPanelOpen = 0;
    m11_refresh_hash(state);
    return 1;
}

/* ================================================================
 * Food / water drain, rest recovery, champion death
 * ================================================================ */

static void m11_apply_survival_drain(M11_GameViewState* state) {
    int i;
    if (!state || !state->active) {
        return;
    }
    /* Every 6 ticks, drain 1 food and 1 water from each champion */
    if ((state->world.gameTick % 6U) != 0U) {
        return;
    }
    for (i = 0; i < state->world.party.championCount; ++i) {
        struct ChampionState_Compat* champ = &state->world.party.champions[i];
        if (!champ->present) {
            continue;
        }
        if (champ->hp.current == 0) {
            continue; /* dead */
        }
        if (champ->food > 0) {
            --champ->food;
        }
        if (champ->water > 0) {
            --champ->water;
        }
        /* Starvation/dehydration damage */
        if (champ->food == 0 && champ->water == 0 && champ->hp.current > 0) {
            if (champ->hp.current > 1) {
                --champ->hp.current;
            }
        }
    }
}

static void m11_apply_rest_recovery(M11_GameViewState* state) {
    int i;
    if (!state || !state->active || !state->resting) {
        return;
    }
    /* Every 4 ticks while resting, recover 1 HP, 2 stamina, 1 mana */
    if ((state->world.gameTick % 4U) != 0U) {
        return;
    }
    for (i = 0; i < state->world.party.championCount; ++i) {
        struct ChampionState_Compat* champ = &state->world.party.champions[i];
        if (!champ->present || champ->hp.current == 0) {
            continue;
        }
        if (champ->hp.current < champ->hp.maximum) {
            ++champ->hp.current;
        }
        if (champ->stamina.current + 2 <= champ->stamina.maximum) {
            champ->stamina.current += 2;
        } else {
            champ->stamina.current = champ->stamina.maximum;
        }
        if (champ->mana.current < champ->mana.maximum) {
            ++champ->mana.current;
        }
    }
}

static void m11_check_party_death(M11_GameViewState* state) {
    int i;
    int anyAlive = 0;
    if (!state || !state->active) {
        return;
    }
    for (i = 0; i < state->world.party.championCount; ++i) {
        if (state->world.party.champions[i].present &&
            state->world.party.champions[i].hp.current > 0) {
            anyAlive = 1;
            break;
        }
    }
    if (!anyAlive && state->world.party.championCount > 0) {
        state->partyDead = 1;
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: ALL CHAMPIONS HAVE FALLEN",
                      (unsigned int)state->world.gameTick);
        m11_set_status(state, "DEATH", "PARTY WIPED");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "GAME OVER");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "THE DUNGEON CLAIMS YOUR SOULS. LOAD A SAVE OR RETURN TO MENU.");
    }
}

/* ================================================================
 * Creature AI simulation (M11-layer)
 *
 * The M10 tick orchestrator treats TIMELINE_EVENT_CREATURE_TICK as a
 * no-op (v1 stub). This M11-layer simulation provides simple creature
 * behavior so the game feels alive:
 *   - Creatures within sight range move toward the party (one step per
 *     movementTicks interval).
 *   - Creatures on the party square deal autonomous damage.
 *   - Creature death removes the group from the map.
 * All manipulation happens on M11-owned world state — no M10 files
 * are modified.
 * ================================================================ */

/* Find a creature group thing on a specific square. */
static unsigned short m11_find_group_on_square(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);
    int safety = 0;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) {
            return thing;
        }
        thing = m11_raw_next_thing(world->things, thing);
        ++safety;
    }
    return THING_NONE;
}

/* Locate a group by thing-id: scan all squares on the given map.
 * Returns 1 if found, filling outX/outY. */
static int m11_find_group_position(
    const struct GameWorld_Compat* world,
    int mapIndex,
    unsigned short groupThing,
    int* outX,
    int* outY) {
    const struct DungeonMapDesc_Compat* map;
    int mx, my, base, idx;
    unsigned short thing;
    int safety;
    if (!world || !world->dungeon || !world->things ||
        !world->things->squareFirstThings) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) return 0;
    for (mx = 0; mx < (int)map->width; ++mx) {
        for (my = 0; my < (int)map->height; ++my) {
            idx = base + mx * (int)map->height + my;
            if (idx < 0 || idx >= world->things->squareFirstThingCount) continue;
            thing = world->things->squareFirstThings[idx];
            safety = 0;
            while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
                if (thing == groupThing) {
                    *outX = mx;
                    *outY = my;
                    return 1;
                }
                thing = m11_raw_next_thing(world->things, thing);
                ++safety;
            }
        }
    }
    return 0;
}

/*
 * Check whether a square is walkable for a creature.
 *
 * Pass 39: delegates to the shared compat owner
 * F0707_MOVEMENT_IsSquarePassableForContext_Compat with
 * MOVEMENT_PASS_CTX_CREATURE so creature and party walkability share
 * one source-faithful decoder for every element type.  The creature
 * context rejects stairs (DM1 creatures cannot traverse stairs per
 * GROUP.C / F0264_MOVE_IsSquareAccessibleForCreature), which matches
 * the pre-pass-39 behaviour for stairs and therefore prevents the
 * pass-30 stairs regression while still unifying every other element.
 *
 * Destroyed doors (state 5) are now walkable for creatures, matching
 * F0706 / party rules; previously the custom M11 path rejected them
 * which was a divergence from the shared compat path.
 */
static int m11_square_walkable_for_creature(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    if (!world || !world->dungeon) return 0;
    return F0707_MOVEMENT_IsSquarePassableForContext_Compat(
        world->dungeon, mapIndex, mapX, mapY,
        MOVEMENT_PASS_CTX_CREATURE);
}

/* Move a creature one step toward the party. Returns 1 if moved. */
static int m11_creature_try_move(
    M11_GameViewState* state,
    unsigned short groupThing,
    int groupX,
    int groupY,
    int* outNewX,
    int* outNewY) {
    int partyX = state->world.party.mapX;
    int partyY = state->world.party.mapY;
    int mapIdx = state->world.party.mapIndex;
    int dx = 0, dy = 0;
    int bestX = groupX, bestY = groupY;
    int bestDist;
    int candidates[4][2];
    int candidateCount = 0;
    int i;

    /* Compute desired direction */
    if (partyX > groupX) dx = 1;
    else if (partyX < groupX) dx = -1;
    if (partyY > groupY) dy = 1;
    else if (partyY < groupY) dy = -1;

    /* Already on party square? Don't move. */
    if (groupX == partyX && groupY == partyY) {
        *outNewX = groupX;
        *outNewY = groupY;
        return 0;
    }

    /* Try primary direction first, then lateral, then diagonal */
    if (dx != 0) {
        candidates[candidateCount][0] = groupX + dx;
        candidates[candidateCount][1] = groupY;
        candidateCount++;
    }
    if (dy != 0) {
        candidates[candidateCount][0] = groupX;
        candidates[candidateCount][1] = groupY + dy;
        candidateCount++;
    }
    if (dx != 0 && dy != 0) {
        candidates[candidateCount][0] = groupX + dx;
        candidates[candidateCount][1] = groupY + dy;
        candidateCount++;
    }
    /* Lateral fallback: if only one axis differs, try the other axis */
    if (dx == 0 && dy != 0) {
        candidates[candidateCount][0] = groupX + 1;
        candidates[candidateCount][1] = groupY;
        candidateCount++;
    }
    if (dy == 0 && dx != 0) {
        candidates[candidateCount][0] = groupX;
        candidates[candidateCount][1] = groupY + 1;
        candidateCount++;
    }

    bestDist = abs(partyX - groupX) + abs(partyY - groupY);
    for (i = 0; i < candidateCount; ++i) {
        int cx = candidates[i][0];
        int cy = candidates[i][1];
        int dist;
        if (!m11_square_walkable_for_creature(&state->world, mapIdx, cx, cy)) {
            continue;
        }
        /* Don't step onto a square that already has another group */
        if (m11_find_group_on_square(&state->world, mapIdx, cx, cy) != THING_NONE) {
            /* Allow stepping onto the party square even if another group is there */
            if (cx != partyX || cy != partyY) continue;
        }
        dist = abs(partyX - cx) + abs(partyY - cy);
        if (dist < bestDist) {
            bestDist = dist;
            bestX = cx;
            bestY = cy;
        }
    }

    if (bestX == groupX && bestY == groupY) {
        *outNewX = groupX;
        *outNewY = groupY;
        return 0; /* couldn't move */
    }

    /* Move: unlink from old square, prepend to new square */
    if (!m11_unlink_thing_from_square(&state->world, mapIdx, groupX, groupY, groupThing)) {
        *outNewX = groupX;
        *outNewY = groupY;
        return 0;
    }
    if (!m11_prepend_thing_to_square(&state->world, mapIdx, bestX, bestY, groupThing)) {
        /* Failed to place — put it back */
        m11_prepend_thing_to_square(&state->world, mapIdx, groupX, groupY, groupThing);
        *outNewX = groupX;
        *outNewY = groupY;
        return 0;
    }
    *outNewX = bestX;
    *outNewY = bestY;
    return 1;
}

/* Deal autonomous creature damage to the party. */
static void m11_creature_attack_party(
    M11_GameViewState* state,
    const struct DungeonGroup_Compat* group) {
    const struct CreatureBehaviorProfile_Compat* profile;
    int creatureCount;
    int totalDamage;
    int targetChamp;
    int i;
    struct ChampionState_Compat* champ;
    int baseDmg;

    if (!state || !group) return;

    profile = CREATURE_GetProfile_Compat(group->creatureType);
    creatureCount = (int)group->count + 1;  /* count field is 0-based */
    baseDmg = profile ? profile->baseAttack : 10;

    /* Each creature in the group attacks for baseDmg / 3 (simplified) */
    totalDamage = 0;
    for (i = 0; i < creatureCount; ++i) {
        if (group->health[i] > 0) {
            /* Simple damage: baseAttack / 3, minimum 1 */
            int dmg = baseDmg / 3;
            if (dmg < 1) dmg = 1;
            totalDamage += dmg;
        }
    }

    if (totalDamage <= 0) return;

    /* Target the active champion, or first alive champion */
    targetChamp = state->world.party.activeChampionIndex;
    if (targetChamp < 0 || targetChamp >= state->world.party.championCount ||
        !state->world.party.champions[targetChamp].present ||
        state->world.party.champions[targetChamp].hp.current == 0) {
        targetChamp = -1;
        for (i = 0; i < state->world.party.championCount; ++i) {
            if (state->world.party.champions[i].present &&
                state->world.party.champions[i].hp.current > 0) {
                targetChamp = i;
                break;
            }
        }
    }
    if (targetChamp < 0) return;

    champ = &state->world.party.champions[targetChamp];
    if ((int)champ->hp.current > totalDamage) {
        champ->hp.current -= (unsigned short)totalDamage;
    } else {
        champ->hp.current = 0;
    }

    {
        char champName[16];
        m11_format_champion_name(champ->name, champName, sizeof(champName));
        m11_log_event(state, M11_COLOR_LIGHT_RED,
                      "T%u: %s HIT BY %s FOR %d",
                      (unsigned int)state->world.gameTick,
                      champName,
                      m11_creature_name(group->creatureType),
                      totalDamage);
    }

    /* Trigger visual feedback for the attack */
    M11_GameView_NotifyDamageFlash(state, group->creatureType);
}

/* Process one creature group: check distance, maybe move, maybe attack. */
static void m11_process_one_creature_group(
    M11_GameViewState* state,
    unsigned short groupThing,
    int groupIndex) {
    const struct CreatureBehaviorProfile_Compat* profile;
    struct DungeonGroup_Compat* group;
    int groupX, groupY;
    int dist;
    int anyAlive;
    int i;

    if (!state || !state->world.things ||
        groupIndex < 0 || groupIndex >= state->world.things->groupCount) {
        return;
    }
    group = &state->world.things->groups[groupIndex];
    profile = CREATURE_GetProfile_Compat(group->creatureType);
    if (!profile) return;

    /* Check if any creature in the group is alive */
    anyAlive = 0;
    for (i = 0; i < (int)group->count + 1; ++i) {
        if (group->health[i] > 0) {
            anyAlive = 1;
            break;
        }
    }
    if (!anyAlive) return;

    /* Find where this group is on the map */
    if (!m11_find_group_position(&state->world,
                                 state->world.party.mapIndex,
                                 groupThing,
                                 &groupX, &groupY)) {
        return; /* not on current map */
    }

    dist = abs(state->world.party.mapX - groupX) +
           abs(state->world.party.mapY - groupY);

    /* If on same square as party: attack */
    if (dist == 0) {
        /* Attack cadence: only attack every attackTicks ticks */
        if (profile->attackTicks > 0 &&
            (state->world.gameTick % (uint32_t)profile->attackTicks) == 0u) {
            m11_creature_attack_party(state, group);
        }
        return;
    }

    /* Movement check: within sight range? */
    if (dist > profile->sightRange && dist > profile->smellRange) {
        return; /* too far to detect party */
    }

    /* Movement cadence: only move every movementTicks ticks */
    if (profile->movementTicks > 0 &&
        (state->world.gameTick % (uint32_t)profile->movementTicks) != 0u) {
        return;
    }

    /* Try to move toward the party */
    {
        int newX, newY;
        if (m11_creature_try_move(state, groupThing, groupX, groupY, &newX, &newY)) {
            /* Check if creature arrived at party square */
            if (newX == state->world.party.mapX &&
                newY == state->world.party.mapY) {
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s REACHES THE PARTY!",
                              (unsigned int)state->world.gameTick,
                              m11_creature_name(group->creatureType));
            }
        }
    }
}

/* Scan all groups on the current map and process their AI. */
static void m11_process_creature_ticks(M11_GameViewState* state) {
    int mapIdx;
    int i;
    const struct DungeonMapDesc_Compat* map;
    int base, mx, my, idx;
    unsigned short thing;
    int safety;

    if (!state || !state->active || state->partyDead) return;
    if (!state->world.dungeon || !state->world.things ||
        !state->world.things->squareFirstThings) return;

    mapIdx = state->world.party.mapIndex;
    if (mapIdx < 0 || mapIdx >= (int)state->world.dungeon->header.mapCount) return;
    map = &state->world.dungeon->maps[mapIdx];
    base = m11_map_square_base(state->world.dungeon, mapIdx);
    if (base < 0) return;

    /* Scan all squares on the current map for groups */
    for (mx = 0; mx < (int)map->width; ++mx) {
        for (my = 0; my < (int)map->height; ++my) {
            idx = base + mx * (int)map->height + my;
            if (idx < 0 || idx >= state->world.things->squareFirstThingCount) continue;
            thing = state->world.things->squareFirstThings[idx];
            safety = 0;
            while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) {
                    int groupIdx = THING_GET_INDEX(thing);
                    m11_process_one_creature_group(state, thing, groupIdx);
                    break; /* one group per square for simplicity */
                }
                thing = m11_raw_next_thing(state->world.things, thing);
                ++safety;
            }
        }
    }

    (void)i; /* suppress unused warning */
}

/* ================================================================
 * XP award and level-up processing
 *
 * When EMIT_DAMAGE_DEALT fires, the active champion earns combat XP
 * through the M10 lifecycle layer.  If the accumulated experience
 * crosses a level threshold, F0850 applies stat boosts and we log
 * the level-up.
 * ================================================================ */
static void m11_award_combat_xp(M11_GameViewState* state,
                                int championIndex,
                                int damage) {
    struct ChampionLifecycleState_Compat* lc;
    struct LevelUpMarker_Compat marker;
    int xpAmount;
    int skillIndex;
    char name[16];

    if (!state) return;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return;
    if (!state->world.party.champions[championIndex].present) return;

    lc = &state->world.lifecycle.champions[championIndex];
    memset(&marker, 0, sizeof(marker));

    /* XP proportional to damage, minimum 1.
     * The lifecycle layer uses 20-skill indices: LIFECYCLE_SKILL_SWING
     * is the default melee sub-skill under Fighter. */
    xpAmount = damage;
    if (xpAmount < 1) xpAmount = 1;
    skillIndex = LIFECYCLE_SKILL_SWING;

    if (F0851_LIFECYCLE_AwardCombatXP_Compat(
            lc, championIndex, skillIndex, xpAmount,
            state->world.party.mapIndex, /* map difficulty */
            state->world.gameTick,
            state->world.lifecycle.lastCreatureAttackTime,
            &state->world.masterRng,
            &marker)) {
        /* Sync base skill level back to Phase 10 party state.
         * The lifecycle uses 20 sub-skills; Phase 10 only tracks the
         * 4 base classes (Fighter/Ninja/Priest/Wizard). */
        int baseIdx = (skillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        if (baseIdx >= 0 && baseIdx < CHAMPION_SKILL_COUNT) {
            int newLevel = F0848_LIFECYCLE_ComputeSkillLevel_Compat(
                lc, baseIdx, 0);
            if (newLevel > 0) {
                state->world.party.champions[championIndex].skillLevels[baseIdx] =
                    (unsigned short)newLevel;
            }
        }

        /* Check for level-up */
        if (marker.newLevel > marker.previousLevel && marker.newLevel > 0) {
            m11_format_champion_name(
                state->world.party.champions[championIndex].name,
                name, sizeof(name));
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s LEVELED UP! (%s %d -> %d)",
                          (unsigned int)state->world.gameTick,
                          name,
                          (baseIdx == LIFECYCLE_SKILL_FIGHTER)  ? "FIGHTER" :
                          (baseIdx == LIFECYCLE_SKILL_NINJA)    ? "NINJA"   :
                          (baseIdx == LIFECYCLE_SKILL_PRIEST)   ? "PRIEST"  :
                          (baseIdx == LIFECYCLE_SKILL_WIZARD)   ? "WIZARD"  :
                          "SKILL",
                          marker.previousLevel, marker.newLevel);
        }
    }
}

static void m11_award_magic_xp(M11_GameViewState* state,
                               int championIndex,
                               int spellKind,
                               int power) {
    struct ChampionLifecycleState_Compat* lc;
    struct LevelUpMarker_Compat marker;
    int skillIndex;
    char name[16];

    if (!state) return;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return;
    if (!state->world.party.champions[championIndex].present) return;

    lc = &state->world.lifecycle.champions[championIndex];
    memset(&marker, 0, sizeof(marker));

    /* Map spell kind to lifecycle sub-skill: Heal (13) for potions,
     * Fire (16) for projectile/combat magic. */
    if (spellKind == C1_SPELL_KIND_POTION_COMPAT) {
        skillIndex = LIFECYCLE_SKILL_HEAL;
    } else {
        skillIndex = LIFECYCLE_SKILL_FIRE;
    }

    if (F0852_LIFECYCLE_AwardMagicXP_Compat(
            lc, championIndex, skillIndex, power > 0 ? power : 1,
            state->world.party.mapIndex,
            state->world.gameTick,
            state->world.lifecycle.lastCreatureAttackTime,
            &state->world.masterRng,
            &marker)) {
        int baseIdx = (skillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        if (baseIdx >= 0 && baseIdx < CHAMPION_SKILL_COUNT) {
            int newLevel = F0848_LIFECYCLE_ComputeSkillLevel_Compat(
                lc, baseIdx, 0);
            if (newLevel > 0) {
                state->world.party.champions[championIndex].skillLevels[baseIdx] =
                    (unsigned short)newLevel;
            }
        }
        if (marker.newLevel > marker.previousLevel && marker.newLevel > 0) {
            m11_format_champion_name(
                state->world.party.champions[championIndex].name,
                name, sizeof(name));
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s LEVELED UP! (%s %d -> %d)",
                          (unsigned int)state->world.gameTick,
                          name,
                          (baseIdx == LIFECYCLE_SKILL_PRIEST)  ? "PRIEST"  :
                          (baseIdx == LIFECYCLE_SKILL_WIZARD)  ? "WIZARD"  :
                          "SKILL",
                          marker.previousLevel, marker.newLevel);
        }
    }
}

/* ================================================================
 * Kill XP bonus
 *
 * When a creature is killed (EMIT_KILL_NOTIFY), the active champion
 * receives a bonus XP award proportional to the slain creature's
 * baseHealth.  This rewards actually defeating enemies, not just
 * scratching them.
 * ================================================================ */
static void m11_award_kill_xp(M11_GameViewState* state,
                              int creatureType) {
    const struct CreatureBehaviorProfile_Compat* profile;
    int champIdx;
    int xpBonus;

    if (!state) return;
    profile = CREATURE_GetProfile_Compat(creatureType);
    champIdx = state->world.party.activeChampionIndex;
    if (champIdx < 0 || champIdx >= CHAMPION_MAX_PARTY) return;
    if (!state->world.party.champions[champIdx].present) return;

    /* XP bonus = baseHealth / 2, minimum 5 */
    xpBonus = profile ? profile->baseHealth / 2 : 10;
    if (xpBonus < 5) xpBonus = 5;

    m11_award_combat_xp(state, champIdx, xpBonus);

    {
        char name[16];
        m11_format_champion_name(
            state->world.party.champions[champIdx].name,
            name, sizeof(name));
        m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                      "T%u: %s EARNS %d KILL XP (%s)",
                      (unsigned int)state->world.gameTick,
                      name, xpBonus,
                      m11_creature_name(creatureType));
    }
}

/* ================================================================
 * Potion / flask use
 *
 * The active champion drinks a potion or water flask from a hand
 * slot.  Potions apply stat effects based on their DM1 type:
 *   - VEN (type 3): antidote / restore stamina
 *   - KU  (type 7): heal HP
 *   - ZO  (type 5): restore mana
 *   - MON (type 0): shield / temporary defense (logged only)
 *   - DES (type 2): poison (damage)
 *   - VI  (type 14): restore all stats partially
 *   - WATER FLASK (type 15): restore water
 *   - EMPTY FLASK (type 16): no effect
 *   - Others: generic small heal
 * The potion thing is consumed (replaced by EMPTY FLASK subtype 16
 * if doNotDiscard is set, or removed from inventory otherwise).
 * ================================================================ */

/* DM1 potion type indices (match s_potionTypeNames order) */
enum {
    M11_POTION_MON  = 0,
    M11_POTION_UM   = 1,
    M11_POTION_DES  = 2,
    M11_POTION_VEN  = 3,
    M11_POTION_SAR  = 4,
    M11_POTION_ZO   = 5,
    M11_POTION_ROS  = 6,
    M11_POTION_KU   = 7,
    M11_POTION_DANE = 8,
    M11_POTION_NETA = 9,
    M11_POTION_BRO  = 10,
    M11_POTION_MA   = 11,
    M11_POTION_YA   = 12,
    M11_POTION_EE   = 13,
    M11_POTION_VI   = 14,
    M11_POTION_WATER_FLASK = 15,
    M11_POTION_EMPTY_FLASK = 16
};

static void m11_apply_potion_effect(M11_GameViewState* state,
                                    struct ChampionState_Compat* champ,
                                    int potionType,
                                    int power,
                                    const char* champName) {
    int amount;
    if (!state || !champ) return;

    /* Power scales the effect: 1..255, typical potion power is 20-80 */
    amount = (power > 0) ? (int)power / 4 + 1 : 5;

    switch (potionType) {
        case M11_POTION_KU: /* Heal HP */
            if ((int)champ->hp.current + amount > (int)champ->hp.maximum) {
                champ->hp.current = champ->hp.maximum;
            } else {
                champ->hp.current += (uint16_t)amount;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s HEALED %d HP",
                          (unsigned int)state->world.gameTick,
                          champName, amount);
            break;

        case M11_POTION_VEN: /* Antidote / restore stamina */
            if ((int)champ->stamina.current + amount * 2 > (int)champ->stamina.maximum) {
                champ->stamina.current = champ->stamina.maximum;
            } else {
                champ->stamina.current += (uint16_t)(amount * 2);
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s STAMINA RESTORED +%d",
                          (unsigned int)state->world.gameTick,
                          champName, amount * 2);
            break;

        case M11_POTION_ZO: /* Restore mana */
            if ((int)champ->mana.current + amount > (int)champ->mana.maximum) {
                champ->mana.current = champ->mana.maximum;
            } else {
                champ->mana.current += (uint16_t)amount;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s MANA RESTORED +%d",
                          (unsigned int)state->world.gameTick,
                          champName, amount);
            break;

        case M11_POTION_DES: /* Poison — damages the drinker */
            if ((int)champ->hp.current > amount) {
                champ->hp.current -= (uint16_t)amount;
            } else {
                champ->hp.current = 0;
            }
            m11_log_event(state, M11_COLOR_LIGHT_RED,
                          "T%u: %s POISONED! -%d HP",
                          (unsigned int)state->world.gameTick,
                          champName, amount);
            break;

        case M11_POTION_VI: /* Restore all stats partially */
            if ((int)champ->hp.current + amount / 2 <= (int)champ->hp.maximum) {
                champ->hp.current += (uint16_t)(amount / 2);
            } else {
                champ->hp.current = champ->hp.maximum;
            }
            if ((int)champ->stamina.current + amount <= (int)champ->stamina.maximum) {
                champ->stamina.current += (uint16_t)amount;
            } else {
                champ->stamina.current = champ->stamina.maximum;
            }
            if ((int)champ->mana.current + amount / 2 <= (int)champ->mana.maximum) {
                champ->mana.current += (uint16_t)(amount / 2);
            } else {
                champ->mana.current = champ->mana.maximum;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s VITALITY RESTORED",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        case M11_POTION_WATER_FLASK: /* Restore water */
            if (champ->water + amount * 8 > 255) {
                champ->water = 255;
            } else {
                champ->water += (unsigned char)(amount * 8);
            }
            m11_log_event(state, M11_COLOR_LIGHT_BLUE,
                          "T%u: %s DRINKS WATER",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        case M11_POTION_EMPTY_FLASK:
            m11_log_event(state, M11_COLOR_YELLOW,
                          "T%u: %s — FLASK IS EMPTY",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        case M11_POTION_MON: /* Shield — log only for now */
            m11_log_event(state, M11_COLOR_CYAN,
                          "T%u: %s FEELS SHIELDED",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        default: /* Generic small heal for unhandled types */
            if ((int)champ->hp.current + amount / 2 <= (int)champ->hp.maximum) {
                champ->hp.current += (uint16_t)(amount / 2);
            } else {
                champ->hp.current = champ->hp.maximum;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s DRINKS POTION (+%d HP)",
                          (unsigned int)state->world.gameTick,
                          champName, amount / 2);
            break;
    }
}

int M11_GameView_UseItem(M11_GameViewState* state) {
    struct ChampionState_Compat* champ;
    unsigned short item = THING_NONE;
    int useSlot = -1;
    int thingType;
    int thingIndex;
    char itemName[48];
    char champName[16];

    if (!state || !state->active || state->partyDead) {
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "USE", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "USE", "CHAMPION CANNOT ACT");
        return 0;
    }

    /* Check right hand first, then left hand */
    if (champ->inventory[CHAMPION_SLOT_HAND_RIGHT] != THING_NONE) {
        useSlot = CHAMPION_SLOT_HAND_RIGHT;
    } else if (champ->inventory[CHAMPION_SLOT_HAND_LEFT] != THING_NONE) {
        useSlot = CHAMPION_SLOT_HAND_LEFT;
    }
    if (useSlot < 0) {
        m11_set_status(state, "USE", "HANDS ARE EMPTY");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "NOTHING TO USE");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "PICK UP A POTION OR FLASK FIRST (G KEY)");
        return 0;
    }

    item = champ->inventory[useSlot];
    thingType = THING_GET_TYPE(item);
    thingIndex = THING_GET_INDEX(item);
    m11_get_item_name(state->world.things, item, itemName, sizeof(itemName));
    m11_format_champion_name(champ->name, champName, sizeof(champName));

    if (thingType == THING_TYPE_POTION) {
        /* Drink the potion */
        if (state->world.things->potions &&
            thingIndex >= 0 && thingIndex < state->world.things->potionCount) {
            struct DungeonPotion_Compat* pot =
                &state->world.things->potions[thingIndex];
            int potType = (int)pot->type;
            int potPower = (int)pot->power;

            if (potType == M11_POTION_EMPTY_FLASK) {
                m11_set_status(state, "USE", "FLASK IS EMPTY");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                         "EMPTY FLASK");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "NOTHING TO DRINK");
                return 0;
            }

            m11_apply_potion_effect(state, champ, potType, potPower, champName);

            /* Award priest XP for drinking a potion */
            m11_award_magic_xp(state,
                               state->world.party.activeChampionIndex,
                               C1_SPELL_KIND_POTION_COMPAT,
                               potPower > 0 ? potPower / 10 + 1 : 1);

            /* Consume: if doNotDiscard, convert to empty flask;
             * otherwise remove from inventory */
            if (pot->doNotDiscard) {
                pot->type = M11_POTION_EMPTY_FLASK;
                pot->power = 0;
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: %s NOW HOLDS AN EMPTY FLASK",
                              (unsigned int)state->world.gameTick,
                              champName);
            } else {
                champ->inventory[useSlot] = THING_NONE;
            }

            m11_set_status(state, "USE", "POTION CONSUMED");
            snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                     "DRANK %s", itemName);
            snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                     "%s USED %s (POWER %d)",
                     champName, itemName, potPower);
            m11_check_party_death(state);
            m11_refresh_hash(state);
            return 1;
        }
        m11_set_status(state, "USE", "POTION DATA ERROR");
        return 0;
    }

    /* Junk type: waterskin (type 2) can be drunk */
    if (thingType == THING_TYPE_JUNK) {
        if (state->world.things->junks &&
            thingIndex >= 0 && thingIndex < state->world.things->junkCount) {
            int junkType = (int)state->world.things->junks[thingIndex].type;
            if (junkType == 2) { /* WATERSKIN */
                if (champ->water + 40 > 255) {
                    champ->water = 255;
                } else {
                    champ->water += 40;
                }
                m11_log_event(state, M11_COLOR_LIGHT_BLUE,
                              "T%u: %s DRINKS FROM WATERSKIN",
                              (unsigned int)state->world.gameTick,
                              champName);
                m11_set_status(state, "USE", "DRANK WATER");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                         "WATERSKIN");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "%s QUENCHED THIRST", champName);
                m11_refresh_hash(state);
                return 1;
            }
        }
    }

    /* Food items: apple(29), corn(30), bread(31), cheese(32),
     * screamer slice(33), worm round(34), drumstick(35), dragon steak(36) */
    if (thingType == THING_TYPE_JUNK) {
        if (state->world.things->junks &&
            thingIndex >= 0 && thingIndex < state->world.things->junkCount) {
            int junkType = (int)state->world.things->junks[thingIndex].type;
            if (junkType >= 29 && junkType <= 36) {
                /* Eat food: restore food stat */
                int foodAmount = 20 + (junkType - 29) * 5;
                if (champ->food + foodAmount > 255) {
                    champ->food = 255;
                } else {
                    champ->food += (unsigned char)foodAmount;
                }
                /* Remove from hand */
                if (state->world.things->junks[thingIndex].doNotDiscard) {
                    m11_log_event(state, M11_COLOR_YELLOW,
                                  "T%u: %s ATE %s (KEEPS ITEM)",
                                  (unsigned int)state->world.gameTick,
                                  champName, itemName);
                } else {
                    champ->inventory[useSlot] = THING_NONE;
                }
                m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                              "T%u: %s ATE %s (+%d FOOD)",
                              (unsigned int)state->world.gameTick,
                              champName, itemName, foodAmount);
                m11_set_status(state, "USE", "FOOD CONSUMED");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                         "ATE %s", itemName);
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "%s FEELS LESS HUNGRY (+%d)",
                         champName, foodAmount);
                m11_refresh_hash(state);
                return 1;
            }
        }
    }

    m11_set_status(state, "USE", "CANNOT USE THIS ITEM");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s", itemName);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "THIS ITEM CANNOT BE USED DIRECTLY");
    return 0;
}

/* Forward declaration: defined later in file, alongside the other
 * projectile-action helpers.  Drives F0811 for each live projectile
 * once per orchestrator tick from inside ProcessTickEmissions. */
static void m11_advance_projectiles_v1(M11_GameViewState* state);

/* Forward decl for explosion advance; implementation lives with other
 * projectile-action helpers.  Drives F0822 for each live explosion
 * once per orchestrator tick from inside ProcessTickEmissions so the
 * post-detonation aftermath progresses through its frame sequence
 * rather than freezing at the first burst frame. */
static void m11_advance_explosions_v1(M11_GameViewState* state);

void M11_GameView_ProcessTickEmissions(M11_GameViewState* state) {
    int i;
    if (!state) {
        return;
    }
    for (i = 0; i < state->lastTickResult.emissionCount; ++i) {
        const struct TickEmission_Compat* e = &state->lastTickResult.emissions[i];
        m11_audio_emit_for_emission(state, e);
        switch (e->kind) {
            case EMIT_DAMAGE_DEALT: {
                int champIdx = state->world.party.activeChampionIndex;
                int dmgDealt = (int)e->payload[2];
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: DAMAGE %d DEALT",
                              (unsigned int)state->world.gameTick,
                              dmgDealt);
                /* Award combat XP to the active champion */
                m11_award_combat_xp(state, champIdx, dmgDealt);
                /* Trigger GRAPHICS.DAT graphic 14 viewport overlay */
                M11_GameView_NotifyCreatureHit(state, dmgDealt);
                break;
            }
            case EMIT_KILL_NOTIFY: {
                /* payload[0] = creature type (if available), else -1 */
                int cType = (int)e->payload[0];
                m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                              "T%u: %s DEFEATED",
                              (unsigned int)state->world.gameTick,
                              (cType >= 0 && cType < 27)
                                  ? m11_creature_name(cType)
                                  : "ENEMY");
                /* Award kill XP bonus */
                m11_award_kill_xp(state, cType >= 0 ? cType : 0);
                break;
            }
            case EMIT_XP_AWARD:
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: EXPERIENCE GAINED",
                              (unsigned int)state->world.gameTick);
                break;
            case EMIT_CHAMPION_DOWN: {
                int champIdx = (int)e->payload[0];
                char name[16];
                if (champIdx >= 0 && champIdx < CHAMPION_MAX_PARTY &&
                    state->world.party.champions[champIdx].present) {
                    m11_format_champion_name(
                        state->world.party.champions[champIdx].name,
                        name, sizeof(name));
                } else {
                    snprintf(name, sizeof(name), "CHAMPION");
                }
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s HAS FALLEN",
                              (unsigned int)state->world.gameTick, name);
                break;
            }
            case EMIT_PARTY_MOVED:
                m11_log_event(state, M11_COLOR_LIGHT_GRAY,
                              "T%u: PARTY MOVED TO %d,%d",
                              (unsigned int)state->world.gameTick,
                              state->world.party.mapX,
                              state->world.party.mapY);
                break;
            case EMIT_PARTY_FELL:
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: FELL INTO PIT! LEVEL %d",
                              (unsigned int)state->world.gameTick,
                              (int)e->payload[0] + 1);
                m11_set_status(state, "PIT", "FELL TO NEXT LEVEL");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle), "PIT FALL");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "DROPPED TO LEVEL %d, POSITION (%d,%d)",
                         (int)e->payload[0] + 1,
                         (int)e->payload[1],
                         (int)e->payload[2]);
                break;
            case EMIT_PARTY_TELEPORTED:
                m11_log_event(state, M11_COLOR_LIGHT_CYAN,
                              "T%u: TELEPORTED TO MAP %d (%d,%d)",
                              (unsigned int)state->world.gameTick,
                              (int)e->payload[0] + 1,
                              (int)e->payload[1],
                              (int)e->payload[2]);
                m11_set_status(state, "TELEPORT", "PARTY TRANSPORTED");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle), "TELEPORTER");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "ARRIVED AT LEVEL %d, POSITION (%d,%d)",
                         (int)e->payload[0] + 1,
                         (int)e->payload[1],
                         (int)e->payload[2]);
                break;
            case EMIT_DOOR_STATE:
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: DOOR STATE CHANGED",
                              (unsigned int)state->world.gameTick);
                break;
            case EMIT_SPELL_EFFECT: {
                /* payload[0]=champIdx, [1]=spellKind, [2]=spellType, [3]=power */
                int sChamp = (int)e->payload[0];
                int sKind = (int)e->payload[1];
                int sType = (int)e->payload[2];
                int sPow  = (int)e->payload[3];
                const char* kindStr = "SPELL";
                if (sKind == C2_SPELL_KIND_PROJECTILE_COMPAT) kindStr = "PROJECTILE";
                else if (sKind == C3_SPELL_KIND_OTHER_COMPAT) kindStr = "ENCHANTMENT";
                else if (sKind == C1_SPELL_KIND_POTION_COMPAT) kindStr = "POTION";
                m11_log_event(state, M11_COLOR_CYAN,
                              "T%u: %s EFFECT APPLIED (TYPE %d, POWER %d)",
                              (unsigned int)state->world.gameTick,
                              kindStr, sType, sPow);
                /* Award magic XP to the casting champion */
                m11_award_magic_xp(state, sChamp, sKind, sPow);
                break;
            }
            case EMIT_GAME_WON:
                if (!state->gameWon) {
                    state->gameWon = 1;
                    state->gameWonTick = state->world.gameTick;
                    m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                                  "T%u: THE QUEST IS COMPLETE!",
                                  (unsigned int)state->world.gameTick);
                    m11_set_status(state, "ENDGAME", "VICTORY!");
                    snprintf(state->inspectTitle,
                             sizeof(state->inspectTitle), "VICTORY");
                    snprintf(state->inspectDetail,
                             sizeof(state->inspectDetail),
                             "LORD CHAOS IS DEFEATED. THE FIRESTAFF IS RESTORED.");
                }
                break;
            case EMIT_PARTY_DEAD:
                if (!state->partyDead) {
                    state->partyDead = 1;
                    m11_log_event(state, M11_COLOR_LIGHT_RED,
                                  "T%u: ALL CHAMPIONS HAVE FALLEN",
                                  (unsigned int)state->world.gameTick);
                    m11_set_status(state, "DEATH", "PARTY WIPED");
                    snprintf(state->inspectTitle,
                             sizeof(state->inspectTitle), "GAME OVER");
                    snprintf(state->inspectDetail,
                             sizeof(state->inspectDetail),
                             "THE DUNGEON CLAIMS YOUR SOULS. LOAD A SAVE OR RETURN TO MENU.");
                }
                break;
            default:
                break;
        }
    }

    /* V1 projectile cycle: after the M10 orchestrator has advanced
     * one tick and its emissions have been processed, step each live
     * projectile via F0811 so the cast visibly travels and detonates
     * instead of freezing at its spawn cell.  Runs exactly once per
     * orchestrator tick; no-op when no projectiles are live. */
    m11_advance_projectiles_v1(state);

    /* V1 explosion cycle: after projectiles have stepped (and may have
     * spawned new first-frame explosions on impact), advance every
     * live explosion one frame via F0822.  This turns the post-
     * detonation aftermath into the classic DM1 multi-frame progression
     * (poison/smoke clouds decay and dissipate; fireball/lightning
     * emit their damage action on the first advance and despawn the
     * slot) rather than leaving a single frozen burst frame. */
    m11_advance_explosions_v1(state);
}

void M11_GameView_Init(M11_GameViewState* state) {
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    (void)M11_Audio_Init(&state->audioState);
    /* V1 presentation: debug HUD off by default, opt-in via env */
    {
        const char* dbg = getenv("FIRESTAFF_DEBUG_HUD");
        state->showDebugHUD = (dbg && dbg[0] == '1') ? 1 : 0;
    }
    state->candidateMirrorOrdinal = -1;
    m11_set_status(state, "BOOT", "GAME VIEW NOT STARTED");
    m11_set_inspect_readout(state, "NO FOCUS", "PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL");
}

void M11_GameView_Shutdown(M11_GameViewState* state) {
    if (!state) {
        return;
    }
    if (state->assetsAvailable) {
        M11_AssetLoader_Shutdown(&state->assetLoader);
        state->assetsAvailable = 0;
    }
    if (state->active) {
        F0883_WORLD_Free_Compat(&state->world);
    }
    M11_Audio_Shutdown(&state->audioState);
    memset(state, 0, sizeof(*state));
}

int M11_GameView_Start(M11_GameViewState* state, const M11_GameLaunchSpec* spec) {
    char dungeonPath[M11_GAME_VIEW_PATH_CAPACITY];
    if (!state || !spec || !spec->title) {
        return 0;
    }
    if (spec->sourceKind == M11_GAME_SOURCE_DIRECT_DUNGEON) {
        if (!spec->dungeonPath || spec->dungeonPath[0] == '\0') {
            return 0;
        }
        snprintf(dungeonPath, sizeof(dungeonPath), "%s", spec->dungeonPath);
    } else if (spec->dungeonPath && spec->dungeonPath[0] != '\0') {
        snprintf(dungeonPath, sizeof(dungeonPath), "%s", spec->dungeonPath);
    } else if (!spec->dataDir || spec->dataDir[0] == '\0' ||
               !m11_resolve_builtin_dungeon_path(dungeonPath,
                                                sizeof(dungeonPath),
                                                spec->dataDir,
                                                spec->gameId)) {
        return 0;
    }
    {
        int savedDebugHUD = state->showDebugHUD;
        M11_GameView_Shutdown(state);
        M11_GameView_Init(state);
        state->showDebugHUD = savedDebugHUD;
    }
    if (F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, 0xF1A5U, &state->world) != 1) {
        m11_set_status(state, "BOOT", "FAILED TO LOAD DUNGEON.DAT");
        return 0;
    }
    state->mirrorCatalogAvailable = 0;
    memset(&state->mirrorCatalog, 0, sizeof(state->mirrorCatalog));
    if (state->world.things &&
        F0652_CHAMPION_BuildMirrorCatalog_Compat(state->world.things,
                                                 &state->mirrorCatalog) > 0) {
        state->mirrorCatalogAvailable = 1;
    }
    state->active = 1;
    state->startedFromLauncher = 1;
    state->sourceKind = spec->sourceKind;
    snprintf(state->title, sizeof(state->title), "%s", spec->title);
    snprintf(state->sourceId, sizeof(state->sourceId), "%s",
             spec->sourceId ? spec->sourceId : "launcher");
    snprintf(state->dungeonPath, sizeof(state->dungeonPath), "%s", dungeonPath);

    /* Try to open GRAPHICS.DAT from the same directory as the dungeon file */
    {
        char graphicsDatPath[M11_GAME_VIEW_PATH_CAPACITY];
        size_t dungeonLen = strlen(dungeonPath);
        size_t slashPos = dungeonLen;
        while (slashPos > 0 && dungeonPath[slashPos - 1] != '/' && dungeonPath[slashPos - 1] != '\\') {
            --slashPos;
        }
        if (slashPos > 0 && slashPos + 13 < sizeof(graphicsDatPath)) {
            memcpy(graphicsDatPath, dungeonPath, slashPos);
            memcpy(graphicsDatPath + slashPos, "GRAPHICS.DAT", 13);
            if (M11_AssetLoader_Init(&state->assetLoader, graphicsDatPath)) {
                state->assetsAvailable = 1;
                /* Try to load the original DM1 font from GRAPHICS.DAT */
                M11_Font_Init(&state->originalFont);
                if (M11_Font_LoadFromGraphicsDat(
                        &state->originalFont,
                        state->assetLoader.fileState,
                        state->assetLoader.runtimeState)) {
                    state->originalFontAvailable = 1;
                }
            }
        }
    }

    m11_refresh_hash(state);
    m11_mark_explored(state);
    m11_log_event(state, M11_COLOR_YELLOW, "T0: %s LOADED", spec->title);
    m11_set_status(state, "BOOT", "GAME DATA LOADED");
    m11_set_inspect_readout(state, "READY", "CLICK CENTER TO ADVANCE OR READ, CLICK SIDES TO TURN, TAB PICKS THE FRONT CHAMPION");
    return 1;
}

int M11_GameView_OpenSelectedMenuEntry(M11_GameViewState* state,
                                       const M12_StartupMenuState* menuState) {
    const M12_MenuEntry* entry;
    M11_GameLaunchSpec spec;
    int rendererBackend = M12_RENDERER_BACKEND_AUTO;
    if (!state || !menuState) {
        return 0;
    }
    if (menuState->launchRequested) {
        M12_LaunchIntent intent = M12_StartupMenu_GetLaunchIntent(menuState);
        if (!intent.valid) {
            return 0;
        }
        entry = M12_StartupMenu_GetEntry(menuState, menuState->activatedIndex);
        rendererBackend = intent.rendererBackend;
    } else {
        entry = M12_StartupMenu_GetEntry(menuState, menuState->selectedIndex);
        rendererBackend = M12_StartupMenu_GetRendererBackend(menuState);
    }
    if (!entry || entry->kind != M12_MENU_ENTRY_GAME || !entry->available) {
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.rendererBackend = rendererBackend;
    spec.title = entry->title;
    spec.gameId = entry->gameId;
    spec.dataDir = M12_AssetStatus_GetDataDir(&menuState->assetStatus);
    spec.sourceId = entry->gameId;
    spec.sourceKind = (entry->sourceKind == M12_MENU_SOURCE_CUSTOM_DUNGEON)
                          ? M11_GAME_SOURCE_CUSTOM_DUNGEON
                          : M11_GAME_SOURCE_BUILTIN_CATALOG;
    return M11_GameView_Start(state, &spec);
}

int M11_GameView_StartDm1(M11_GameViewState* state, const char* dataDir) {
    M11_GameLaunchSpec spec;
    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.dataDir = dataDir;
    spec.sourceId = "dm1";
    spec.rendererBackend = M12_RENDERER_BACKEND_AUTO;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    return M11_GameView_Start(state, &spec);
}

int M11_GameView_QuickSave(M11_GameViewState* state) {
    char path[M11_GAME_VIEW_PATH_CAPACITY];
    FILE* file = NULL;
    unsigned char* blob = NULL;
    int blobSize;
    int bytesWritten = 0;
    unsigned char header[M11_QUICKSAVE_HEADER_SIZE];

    if (!state || !state->active) {
        return 0;
    }
    if (!M11_GameView_GetQuickSavePath(state, path, sizeof(path))) {
        m11_set_status(state, "SAVE", "SAVE PATH TOO LONG");
        return 0;
    }

    blobSize = F0899_WORLD_SerializedSize_Compat(&state->world);
    if (blobSize <= 0) {
        m11_set_status(state, "SAVE", "SERIALISE SIZE FAILED");
        return 0;
    }

    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) {
        m11_set_status(state, "SAVE", "OUT OF MEMORY");
        return 0;
    }
    if (!F0897_WORLD_Serialize_Compat(&state->world, blob, blobSize, &bytesWritten) ||
        bytesWritten != blobSize) {
        free(blob);
        m11_set_status(state, "SAVE", "SERIALISE FAILED");
        return 0;
    }

    memcpy(header, g_m11_quicksave_magic, sizeof(g_m11_quicksave_magic));
    m11_write_u32_le(header + 8, (uint32_t)blobSize);
    m11_write_u32_le(header + 12, state->lastWorldHash);

    file = fopen(path, "wb");
    if (!file) {
        free(blob);
        m11_set_status(state, "SAVE", "OPEN FAILED");
        return 0;
    }
    if (fwrite(header, 1U, sizeof(header), file) != sizeof(header) ||
        fwrite(blob, 1U, (size_t)blobSize, file) != (size_t)blobSize ||
        fclose(file) != 0) {
        free(blob);
        m11_set_status(state, "SAVE", "WRITE FAILED");
        return 0;
    }
    free(blob);

    m11_set_status(state, "SAVE", "QUICKSAVE WRITTEN");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "SAVE SLOT READY");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "F9 RESTORES TICK %u FROM %s",
             (unsigned int)state->world.gameTick,
             path);
    return 1;
}

int M11_GameView_QuickLoad(M11_GameViewState* state) {
    char path[M11_GAME_VIEW_PATH_CAPACITY];
    FILE* file = NULL;
    long fileSizeLong;
    int blobSize;
    unsigned char header[M11_QUICKSAVE_HEADER_SIZE];
    unsigned char* blob = NULL;
    struct GameWorld_Compat loadedWorld;
    uint32_t expectedHash;
    uint32_t loadedHash = 0;

    if (!state || !state->active) {
        return 0;
    }
    if (!M11_GameView_GetQuickSavePath(state, path, sizeof(path))) {
        m11_set_status(state, "LOAD", "SAVE PATH TOO LONG");
        return 0;
    }

    file = fopen(path, "rb");
    if (!file) {
        m11_set_status(state, "LOAD", "NO QUICKSAVE FOUND");
        return 0;
    }
    if (fread(header, 1U, sizeof(header), file) != sizeof(header) ||
        memcmp(header, g_m11_quicksave_magic, sizeof(g_m11_quicksave_magic)) != 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE HEADER INVALID");
        return 0;
    }

    blobSize = (int)m11_read_u32_le(header + 8);
    expectedHash = m11_read_u32_le(header + 12);
    if (blobSize <= 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE SIZE INVALID");
        return 0;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE SEEK FAILED");
        return 0;
    }
    fileSizeLong = ftell(file);
    if (fileSizeLong < (long)M11_QUICKSAVE_HEADER_SIZE ||
        fileSizeLong != (long)(M11_QUICKSAVE_HEADER_SIZE + blobSize) ||
        fseek(file, (long)M11_QUICKSAVE_HEADER_SIZE, SEEK_SET) != 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE LENGTH MISMATCH");
        return 0;
    }

    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) {
        fclose(file);
        m11_set_status(state, "LOAD", "OUT OF MEMORY");
        return 0;
    }
    if (fread(blob, 1U, (size_t)blobSize, file) != (size_t)blobSize || fclose(file) != 0) {
        free(blob);
        m11_set_status(state, "LOAD", "READ FAILED");
        return 0;
    }

    memset(&loadedWorld, 0, sizeof(loadedWorld));
    if (!F0898_WORLD_Deserialize_Compat(&loadedWorld, blob, blobSize, NULL) ||
        !F0891_ORCH_WorldHash_Compat(&loadedWorld, &loadedHash) ||
        loadedHash != expectedHash) {
        F0883_WORLD_Free_Compat(&loadedWorld);
        free(blob);
        m11_set_status(state, "LOAD", "SAVE VERIFY FAILED");
        return 0;
    }
    free(blob);

    F0883_WORLD_Free_Compat(&state->world);
    state->world = loadedWorld;
    memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
    m11_refresh_hash(state);
    m11_set_status(state, "LOAD", "QUICKSAVE RESTORED");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "RESTORED");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "TICK %u HASH %08X RELOADED FROM %s",
             (unsigned int)state->world.gameTick,
             (unsigned int)state->lastWorldHash,
             path);
    return 1;
}

M11_GameInputResult M11_GameView_AdvanceIdleTick(M11_GameViewState* state) {
    if (!state || !state->active) {
        return M11_GAME_INPUT_IGNORED;
    }
    /* No idle ticks during overlays, endgame, or dialog. */
    if (state->gameWon || state->dialogOverlayActive ||
        state->mapOverlayActive || state->inventoryPanelActive) {
        return M11_GAME_INPUT_IGNORED;
    }
    if (!m11_apply_tick(state, CMD_NONE, "WAIT")) {
        return M11_GAME_INPUT_IGNORED;
    }
    return M11_GAME_INPUT_REDRAW;
}

static int m11_apply_tick(M11_GameViewState* state,
                          uint8_t command,
                          const char* actionLabel) {
    struct TickInput_Compat input;
    struct PartyState_Compat beforeParty;
    uint32_t beforeTick;
    uint32_t beforeHash;
    if (!state || !state->active) {
        return 0;
    }
    memset(&input, 0, sizeof(input));
    beforeParty = state->world.party;
    beforeTick = state->world.gameTick;
    beforeHash = state->lastWorldHash;
    input.tick = state->world.gameTick;
    input.command = command;
    if (command == CMD_ATTACK) {
        input.commandArg1 = (uint8_t)(state->world.party.activeChampionIndex >= 0
                                          ? state->world.party.activeChampionIndex
                                          : 0);
        input.commandArg2 = (uint8_t)state->world.party.direction;
    }
    memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
    {
        int orchResult = F0884_ORCH_AdvanceOneTick_Compat(
            &state->world, &input, &state->lastTickResult);
        if (orchResult == ORCH_FAIL) {
            m11_set_status(state, actionLabel, "TICK REJECTED");
            return 0;
        }
        /* ORCH_PARTY_DEAD and ORCH_GAME_WON are handled via emissions
         * below — the tick was still successfully processed. */
    }
    state->lastWorldHash = state->lastTickResult.worldHashPost;

    /* Process emissions into the message log (also sets gameWon/partyDead
     * via EMIT_GAME_WON / EMIT_PARTY_DEAD handling). */
    M11_GameView_ProcessTickEmissions(state);

    /* Apply survival mechanics */
    m11_apply_survival_drain(state);
    m11_apply_rest_recovery(state);

    /* Torch fuel burn-down */
    M11_GameView_UpdateTorchFuel(state);

    /* Creature AI: movement and autonomous damage */
    m11_process_creature_ticks(state);

    /* Advance animation frame counters and decay timers */
    M11_GameView_TickAnimation(state);

    m11_check_party_death(state);

    /* Mark current cell as explored */
    m11_mark_explored(state);

    /* Post-move pit/teleporter ownership now lives in compat/runtime. */

    if (command == CMD_ATTACK) {
        char champion[16];
        int attackRoll = 0;
        int i;
        m11_get_active_champion_label(state, champion, sizeof(champion));
        for (i = 0; i < state->lastTickResult.emissionCount; ++i) {
            const struct TickEmission_Compat* emission = &state->lastTickResult.emissions[i];
            if (emission->kind == EMIT_DAMAGE_DEALT) {
                attackRoll = emission->payload[2];
                break;
            }
        }
        m11_set_status(state, actionLabel, "STRIKE COMMITTED");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s ATTACKS", champion);
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "FRONT CONTACT, ROLL %d, TICK %u",
                 attackRoll,
                 (unsigned int)state->world.gameTick);
    } else if (command == CMD_TURN_LEFT || command == CMD_TURN_RIGHT) {
        if (state->world.party.direction != beforeParty.direction) {
            m11_set_status(state, actionLabel, "FACING UPDATED");
        } else {
            m11_set_status(state, actionLabel, "TURN IGNORED");
        }
    } else if (command == CMD_NONE) {
        if (state->world.gameTick != beforeTick || state->lastWorldHash != beforeHash) {
            m11_set_status(state, actionLabel, "IDLE TICK ADVANCED");
        } else {
            m11_set_status(state, actionLabel, "IDLE TICK HELD");
        }
    } else if (state->world.party.mapIndex != beforeParty.mapIndex) {
        /* Map changed via pit or teleporter — status already set by transition handler */
        (void)0;
    } else if (state->world.party.mapX != beforeParty.mapX ||
               state->world.party.mapY != beforeParty.mapY) {
        m11_set_status(state, actionLabel, "PARTY MOVED");
    } else {
        m11_set_status(state, actionLabel, "MOVEMENT BLOCKED");
    }
    return 1;
}


int M11_GameView_GetMirrorCatalogCount(const M11_GameViewState* state) {
    if (!state || !state->mirrorCatalogAvailable) return 0;
    return state->mirrorCatalog.count;
}

int M11_GameView_GetMirrorNameByOrdinal(const M11_GameViewState* state,
                                        int mirrorOrdinal,
                                        char* outName,
                                        int outSize) {
    if (!state || !state->mirrorCatalogAvailable) {
        if (outName && outSize > 0) outName[0] = '\0';
        return 0;
    }
    return F0660_CHAMPION_MirrorCatalogGetName_Compat(&state->mirrorCatalog,
                                                      mirrorOrdinal,
                                                      outName,
                                                      outSize);
}

int M11_GameView_GetMirrorTitleByOrdinal(const M11_GameViewState* state,
                                         int mirrorOrdinal,
                                         char* outTitle,
                                         int outSize) {
    if (!state || !state->mirrorCatalogAvailable) {
        if (outTitle && outSize > 0) outTitle[0] = '\0';
        return 0;
    }
    return F0661_CHAMPION_MirrorCatalogGetTitle_Compat(&state->mirrorCatalog,
                                                       mirrorOrdinal,
                                                       outTitle,
                                                       outSize);
}

int M11_GameView_RecruitChampionByMirrorOrdinal(M11_GameViewState* state,
                                                int mirrorOrdinal) {
    if (!state || !state->active || !state->mirrorCatalogAvailable) return 0;
    return F0673_CHAMPION_MirrorCatalogRecruitOrdinalIfAbsent_Compat(
        &state->mirrorCatalog, mirrorOrdinal, &state->world.party);
}

int M11_GameView_RecruitChampionByMirrorName(M11_GameViewState* state,
                                             const char* name) {
    if (!state || !state->active || !state->mirrorCatalogAvailable) return 0;
    return F0674_CHAMPION_MirrorCatalogRecruitNameIfAbsent_Compat(
        &state->mirrorCatalog, name, &state->world.party);
}

int M11_GameView_GetFrontMirrorOrdinal(const M11_GameViewState* state) {
    return m11_front_cell_mirror_ordinal(state);
}

int M11_GameView_SelectFrontMirrorCandidate(M11_GameViewState* state) {
    int mirrorOrdinal;
    char mirrorName[16];
    char mirrorTitle[32];

    mirrorName[0] = '\0';
    mirrorTitle[0] = '\0';
    if (!state || !state->active) return 0;
    mirrorOrdinal = m11_front_cell_mirror_ordinal(state);
    if (mirrorOrdinal < 0) return 0;
    if (state->world.party.championCount >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "MIRROR", "PARTY FULL");
        m11_set_inspect_readout(state, "CHAMPION MIRROR",
                                "NO ROOM FOR ANOTHER CHAMPION");
        return 0;
    }

    state->candidateMirrorOrdinal = mirrorOrdinal;
    state->candidateMirrorPanelActive = 1;
    (void)M11_GameView_GetMirrorNameByOrdinal(state, mirrorOrdinal,
                                              mirrorName, sizeof(mirrorName));
    (void)M11_GameView_GetMirrorTitleByOrdinal(state, mirrorOrdinal,
                                               mirrorTitle, sizeof(mirrorTitle));
    m11_set_status(state, "MIRROR", "RESURRECT OR REINCARNATE");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle),
             "MIRROR: %s", mirrorName[0] ? mirrorName : "CHAMPION");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "%s%s%s — CHOOSE RESURRECT, REINCARNATE, OR CANCEL",
             mirrorName[0] ? mirrorName : "CHAMPION",
             mirrorTitle[0] ? ", " : "",
             mirrorTitle[0] ? mirrorTitle : "");
    return 1;
}

int M11_GameView_ConfirmMirrorCandidate(M11_GameViewState* state,
                                        int reincarnate) {
    int result;
    char mirrorName[16];

    mirrorName[0] = '\0';
    if (!state || !state->active || !state->candidateMirrorPanelActive ||
        state->candidateMirrorOrdinal < 0) {
        return 0;
    }
    if (state->world.party.championCount >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "MIRROR", "PARTY FULL");
        return 0;
    }
    (void)M11_GameView_GetMirrorNameByOrdinal(state, state->candidateMirrorOrdinal,
                                              mirrorName, sizeof(mirrorName));
    result = M11_GameView_RecruitChampionByMirrorOrdinal(
        state, state->candidateMirrorOrdinal);
    if (result == 1) {
        state->candidateMirrorPanelActive = 0;
        state->candidateMirrorOrdinal = -1;
        m11_refresh_hash(state);
        m11_set_status(state, "MIRROR", reincarnate ? "REINCARNATED" : "RESURRECTED");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                 "%s JOINS", mirrorName[0] ? mirrorName : "CHAMPION");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "%s ADDED TO THE PARTY FROM THE SOURCE MIRROR RECORD",
                 mirrorName[0] ? mirrorName : "CHAMPION");
    }
    return result;
}

int M11_GameView_CancelMirrorCandidate(M11_GameViewState* state) {
    if (!state || !state->active || !state->candidateMirrorPanelActive) {
        return 0;
    }
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    m11_set_status(state, "MIRROR", "CANCELLED");
    m11_set_inspect_readout(state, "CHAMPION MIRROR",
                            "SELECTION CANCELLED");
    return 1;
}

M11_GameInputResult M11_GameView_HandleInput(M11_GameViewState* state,
                                             M12_MenuInput input) {
    uint8_t command = CMD_NONE;
    const char* label = "NONE";
    if (!state || !state->active) {
        return M11_GAME_INPUT_IGNORED;
    }

    /* Dialog overlay: any input dismisses it, then return to gameplay. */
    if (state->dialogOverlayActive) {
        if (input == M12_MENU_INPUT_BACK) {
            M11_GameView_DismissDialogOverlay(state);
            m11_set_status(state, "RETURN", "BACK TO LAUNCHER");
            return M11_GAME_INPUT_RETURN_TO_MENU;
        }
        if (input == M12_MENU_INPUT_ACCEPT && state->dialogChoiceCount > 0) {
            state->dialogSelectedChoice = 1;
            M11_GameView_DismissDialogOverlay(state);
            return M11_GAME_INPUT_REDRAW;
        }
        if (input != M12_MENU_INPUT_NONE) {
            M11_GameView_DismissDialogOverlay(state);
            return M11_GAME_INPUT_REDRAW;
        }
        return M11_GAME_INPUT_IGNORED;
    }

    /* Source-backed champion mirror panel: CLIKCHAM routes
     * C160/C161/C162 through the resurrect/reincarnate/cancel panel.
     * M11 maps ACTION/ACCEPT to the default resurrect choice for now
     * and BACK to cancel; the public confirm API keeps probes explicit. */
    if (state->candidateMirrorPanelActive) {
        if (input == M12_MENU_INPUT_BACK) {
            return M11_GameView_CancelMirrorCandidate(state)
                       ? M11_GAME_INPUT_REDRAW
                       : M11_GAME_INPUT_IGNORED;
        }
        if (input == M12_MENU_INPUT_ACCEPT || input == M12_MENU_INPUT_ACTION) {
            return M11_GameView_ConfirmMirrorCandidate(state, 0)
                       ? M11_GAME_INPUT_REDRAW
                       : M11_GAME_INPUT_IGNORED;
        }
        return M11_GAME_INPUT_IGNORED;
    }

    /* Endgame: only ESC (return to menu) and quickload accepted. */
    if (state->gameWon) {
        if (input == M12_MENU_INPUT_BACK) {
            m11_set_status(state, "RETURN", "BACK TO LAUNCHER");
            return M11_GAME_INPUT_RETURN_TO_MENU;
        }
        return M11_GAME_INPUT_IGNORED;
    }

    /* Map/inventory toggle.
     *
     * The full-screen map overlay is a Firestaff convenience/debug
     * surface, not a source-backed DM1 V1 UI (see
     * PARITY_MATRIX_DM1_V1.md pass 102).  Keep it available to debug
     * HUD sessions, but ignore the M-key in default V1 chrome mode so
     * normal parity play cannot enter an invented screen. */
    if (input == M12_MENU_INPUT_MAP_TOGGLE) {
        if (m11_v1_chrome_mode_enabled() && !state->showDebugHUD) {
            return M11_GAME_INPUT_IGNORED;
        }
        state->inventoryPanelActive = 0;
        M11_GameView_ToggleMapOverlay(state);
        return M11_GAME_INPUT_REDRAW;
    }
    if (input == M12_MENU_INPUT_INVENTORY_TOGGLE) {
        state->mapOverlayActive = 0;
        M11_GameView_ToggleInventoryPanel(state);
        return M11_GAME_INPUT_REDRAW;
    }

    /* While an overlay is open, block gameplay input; ESC/BACK closes it. */
    if (state->mapOverlayActive || state->inventoryPanelActive) {
        if (input == M12_MENU_INPUT_BACK) {
            state->mapOverlayActive = 0;
            state->inventoryPanelActive = 0;
            return M11_GAME_INPUT_REDRAW;
        }
        /* In inventory panel, accept up/down/left/right to navigate slots */
        if (state->inventoryPanelActive) {
            if (input == M12_MENU_INPUT_UP && state->inventorySelectedSlot > 0) {
                state->inventorySelectedSlot--;
                return M11_GAME_INPUT_REDRAW;
            }
            if (input == M12_MENU_INPUT_DOWN && state->inventorySelectedSlot < 20) {
                state->inventorySelectedSlot++;
                return M11_GAME_INPUT_REDRAW;
            }
        }
        return M11_GAME_INPUT_IGNORED;
    }

    switch (input) {
        case M12_MENU_INPUT_UP:
            command = m11_forward_command_for_direction(state->world.party.direction);
            label = "FORWARD";
            break;
        case M12_MENU_INPUT_DOWN:
            command = m11_backward_command_for_direction(state->world.party.direction);
            label = "BACKSTEP";
            break;
        case M12_MENU_INPUT_LEFT:
            command = CMD_TURN_LEFT;
            label = "TURN LEFT";
            break;
        case M12_MENU_INPUT_RIGHT:
            command = CMD_TURN_RIGHT;
            label = "TURN RIGHT";
            break;
        case M12_MENU_INPUT_STRAFE_LEFT:
            command = m11_strafe_left_command_for_direction(state->world.party.direction);
            label = "STRAFE LEFT";
            break;
        case M12_MENU_INPUT_STRAFE_RIGHT:
            command = m11_strafe_right_command_for_direction(state->world.party.direction);
            label = "STRAFE RIGHT";
            break;
        case M12_MENU_INPUT_ACCEPT:
            if (m11_inspect_front_cell(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_ACTION:
            if (m11_front_cell_has_attack_target(state)) {
                command = CMD_ATTACK;
                label = "ATTACK";
                break;
            }
            if (m11_front_cell_is_door(state)) {
                if (m11_toggle_front_door(state)) {
                    return M11_GAME_INPUT_REDRAW;
                }
                return M11_GAME_INPUT_IGNORED;
            }
            command = CMD_NONE;
            label = "WAIT";
            break;
        case M12_MENU_INPUT_CYCLE_CHAMPION:
            if (m11_cycle_active_champion(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_REST_TOGGLE:
            state->resting = !state->resting;
            if (state->resting) {
                m11_log_event(state, M11_COLOR_LIGHT_BLUE, "T%u: RESTING",
                              (unsigned int)state->world.gameTick);
                m11_set_status(state, "REST", "PARTY IS RESTING");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle), "RESTING");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "HP AND STAMINA RECOVER SLOWLY. PRESS R AGAIN TO WAKE.");
            } else {
                m11_log_event(state, M11_COLOR_LIGHT_BLUE, "T%u: WOKE UP",
                              (unsigned int)state->world.gameTick);
                m11_set_status(state, "REST", "PARTY AWAKE");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle), "AWAKE");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "REST ENDED. RESUME EXPLORING.");
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_USE_STAIRS:
            if (m11_try_stairs_transition(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            m11_set_status(state, "STAIRS", "NO STAIRS HERE");
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_PICKUP_ITEM:
            if (M11_GameView_PickupItem(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_DROP_ITEM:
            if (M11_GameView_DropItem(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_SPELL_RUNE_1:
        case M12_MENU_INPUT_SPELL_RUNE_2:
        case M12_MENU_INPUT_SPELL_RUNE_3:
        case M12_MENU_INPUT_SPELL_RUNE_4:
        case M12_MENU_INPUT_SPELL_RUNE_5:
        case M12_MENU_INPUT_SPELL_RUNE_6: {
            int runeIdx = (int)(input - M12_MENU_INPUT_SPELL_RUNE_1);
            if (!state->spellPanelOpen) {
                M11_GameView_OpenSpellPanel(state);
            }
            if (M11_GameView_EnterRune(state, runeIdx)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        }
        case M12_MENU_INPUT_SPELL_CAST:
            if (state->spellPanelOpen && state->spellBuffer.runeCount >= 2) {
                M11_GameView_CastSpell(state);
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_SPELL_CLEAR:
            if (state->spellPanelOpen) {
                M11_GameView_ClearSpell(state);
                M11_GameView_CloseSpellPanel(state);
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_USE_ITEM:
            if (M11_GameView_UseItem(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_BACK:
            m11_set_status(state, "RETURN", "BACK TO LAUNCHER");
            return M11_GAME_INPUT_RETURN_TO_MENU;
        case M12_MENU_INPUT_NONE:
        default:
            return M11_GAME_INPUT_IGNORED;
    }
    if (!m11_apply_tick(state, command, label)) {
        return M11_GAME_INPUT_IGNORED;
    }
    return M11_GAME_INPUT_REDRAW;
}

M11_GameInputResult M11_GameView_HandlePointer(M11_GameViewState* state,
                                               int x,
                                               int y,
                                               int primaryButton) {
    int slot;

    if (!state || !state->active || !primaryButton) {
        return M11_GAME_INPUT_IGNORED;
    }

    /* Click dismisses dialog overlay. */
    if (state->dialogOverlayActive) {
        int choice = m11_dialog_choice_at_point(state, x, y);
        if (choice > 0) {
            state->dialogSelectedChoice = choice;
        }
        M11_GameView_DismissDialogOverlay(state);
        return M11_GAME_INPUT_REDRAW;
    }

    /* In endgame, clicks are ignored (use ESC). */
    if (state->gameWon) {
        return M11_GAME_INPUT_IGNORED;
    }

    /* The procedural utility-panel buttons (inspect / save /
     * load / back header) exist only in the debug HUD.  In V1
     * mode the right column shows the authentic DM1 action +
     * spell frames, so clicks in y=28..66 must fall through to
     * the DM1 action-menu and icon-cell hit tests below instead
     * of being intercepted by invisible hotspots.  Without this
     * gate a click on action-row 0 (y=58..66) lands inside the
     * invisible save/load button row (y=56..65) and is silently
     * swallowed. */
    if (state->showDebugHUD) {
        if (m11_point_in_utility_button(x, y, M11_UTILITY_INSPECT_X, M11_UTILITY_INSPECT_W)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_ACCEPT);
        }
        if (m11_point_in_utility_button(x, y, M11_UTILITY_SAVE_X, M11_UTILITY_SAVE_W)) {
            if (M11_GameView_QuickSave(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        }
        if (m11_point_in_utility_button(x, y, M11_UTILITY_LOAD_X, M11_UTILITY_LOAD_W)) {
            if (M11_GameView_QuickLoad(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        }
        if (m11_point_in_rect(x, y,
                              M11_UTILITY_PANEL_X,
                              M11_UTILITY_PANEL_Y,
                              M11_UTILITY_PANEL_W,
                              12)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_BACK);
        }
    }

    /* DM1 action-hand icon cell hits — clicking a champion's
     * action cell activates them (F0389_MENUS_SetActingChampion)
     * and switches the right-column action area into menu mode.
     * Clicking the acting champion's cell again clears it
     * (F0388_MENUS_ClearActingChampion).  Hit rectangles are
     * resolved through M11_GameView_GetV1ActionIconCellZone() so
     * click geometry stays locked to layout-696 C089..C092, matching
     * F0386_MENUS_DrawActionIcon geometry.  Only active
     * in V1 mode once the authentic frames have rendered, mirroring
     * the visible-cells gate used in m11_draw_utility_panel. */
    if (!state->showDebugHUD) {
        int slotHit;

        /* When in action-menu mode, clicks on the three action
         * rows drive F0391_MENUS_DidClickTriggerAction: the
         * chosen action is resolved against the ActionList, the
         * action is performed (within our bounded model), and
         * the acting champion is always cleared afterwards — even
         * when the row is empty (ACTION_NONE) the click still
         * falls through to the icon-cell test below so the user
         * can switch champions without first hitting an icon.
         *
         * Row hit geometry matches ACTIDRAW.C F0387 zones 85/86/87
         * (y0=58, step=11, h=9) within the 87×45 action area at
         * x=224..310.  We test the rows BEFORE the icon cells so
         * a click at the bottom of an action row (which is above
         * the icon cells at y=86) is resolved as a row-click first.
         *
         * Ref: ReDMCSB MENU.C F0391_MENUS_DidClickTriggerAction. */
        if (state->actingChampionOrdinal != 0) {
            int rowIdx;
            for (rowIdx = 0; rowIdx < 3; ++rowIdx) {
                int rowX, rowY, rowW, rowH;
                if (!M11_GameView_GetV1ActionMenuRowZone(
                        rowIdx, &rowX, &rowY, &rowW, &rowH) ||
                    !m11_point_in_rect(x, y, rowX, rowY, rowW, rowH)) {
                    continue;
                }
                (void)M11_GameView_TriggerActionRow(state, rowIdx);
                return M11_GAME_INPUT_REDRAW;
            }
        }

        for (slotHit = 0; slotHit < CHAMPION_MAX_PARTY; ++slotHit) {
            int cellX, cellY, cellW, cellH;
            if (!M11_GameView_GetV1ActionIconCellZone(
                    slotHit, &cellX, &cellY, &cellW, &cellH)) {
                continue;
            }
            if (!m11_point_in_rect(x, y, cellX, cellY, cellW, cellH)) {
                continue;
            }
            /* Toggle: click on already-acting champion clears. */
            if (state->actingChampionOrdinal ==
                    (unsigned int)(slotHit + 1)) {
                M11_GameView_ClearActingChampion(state);
                return M11_GAME_INPUT_REDRAW;
            }
            if (M11_GameView_SetActingChampion(state, slotHit)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        }
    }

    if (m11_point_in_rect(x, y,
                          M11_PROMPT_STRIP_X,
                          M11_PROMPT_STRIP_Y,
                          M11_PROMPT_STRIP_W,
                          M11_PROMPT_STRIP_H) ||
        m11_point_in_rect(x, y, 218, 106, 86, 34)) {
        return M11_GameView_HandleInput(state, M12_MENU_INPUT_ACCEPT);
    }

    if (m11_point_in_rect(x, y,
                          M11_VIEWPORT_X,
                          M11_VIEWPORT_Y,
                          M11_VIEWPORT_W,
                          M11_VIEWPORT_H)) {
        return M11_GameView_HandleInput(state, m11_pointer_viewport_input(state, x, y));
    }

    if (m11_point_in_rect(x, y,
                          M11_CONTROL_STRIP_X,
                          M11_CONTROL_STRIP_Y,
                          M11_CONTROL_STRIP_W,
                          M11_CONTROL_STRIP_H)) {
        if (m11_point_in_rect(x, y, 18, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_LEFT);
        }
        if (m11_point_in_rect(x, y, 35, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_UP);
        }
        if (m11_point_in_rect(x, y, 52, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_RIGHT);
        }
        if (m11_point_in_rect(x, y, 69, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_DOWN);
        }
        if (m11_point_in_rect(x, y, 86, 167, 12, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_ACTION);
        }
    }

    {
        int slotStep = m11_party_slot_step();
        int slotW    = m11_party_slot_w();
        for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
            int slotX = M11_PARTY_PANEL_X + slot * slotStep;
            if (m11_point_in_rect(x, y,
                                  slotX,
                                  M11_PARTY_PANEL_Y + 20,
                                  slotW,
                                  M11_PARTY_SLOT_H - 20)) {
                if (m11_set_active_champion(state, slot)) {
                    return M11_GAME_INPUT_REDRAW;
                }
                return M11_GAME_INPUT_IGNORED;
            }
        }
    }

    return M11_GAME_INPUT_IGNORED;
}

static void m11_draw_party_arrow(unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int size,
                                 int direction,
                                 unsigned char color) {
    int i;
    int cx = x + size / 2;
    int cy = y + size / 2;
    for (i = 0; i < size / 3; ++i) {
        switch (direction) {
            case DIR_NORTH:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx, y + 2 + i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx - i, y + 3 + i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx + i, y + 3 + i, color);
                break;
            case DIR_EAST:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + size - 3 - i, cy, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + size - 4 - i, cy - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + size - 4 - i, cy + i, color);
                break;
            case DIR_SOUTH:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx, y + size - 3 - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx - i, y + size - 4 - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx + i, y + size - 4 - i, color);
                break;
            case DIR_WEST:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 2 + i, cy, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 3 + i, cy - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 3 + i, cy + i, color);
                break;
            default:
                break;
        }
    }
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} M11_ViewRect;

/* Forward declaration for floor ornament rendering (defined after door ornaments). */
static int m11_draw_floor_ornament(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int fbW, int fbH,
                                   const M11_ViewRect* rect,
                                   int ornamentOrdinal,
                                   int depthIndex,
                                   int sideHint);

enum {
    M11_MAX_CELL_CREATURES = 4, /* DM1 supports up to 4 creature groups per square */
    M11_MAX_CELL_ITEMS    = 4  /* DM1 scatters up to 4 floor items visibly */
};

typedef struct M11_ViewportCell {
    int valid;
    int mapX;
    int mapY;
    int relForward;
    int relSide;
    unsigned char square;
    int elementType;
    int thingCount;
    unsigned short firstThing;
    int doorState;
    int doorType;
    int doorVertical;
    int hasDoorThing;
    int creatureType; /* -1 if no creature, else creature type index (0-26) */
    /* All creature group types on this square (up to 4, -1 terminated) */
    int creatureTypes[M11_MAX_CELL_CREATURES];
    int creatureCountsPerGroup[M11_MAX_CELL_CREATURES]; /* creature count per group (count+1) */
    int creatureDirections[M11_MAX_CELL_CREATURES]; /* DungeonGroup_Compat.direction per group */
    int creatureGroupCount; /* number of valid entries in creatureTypes[] */
    /* First floor item info for sprite rendering (legacy single-item) */
    int firstItemThingType;    /* THING_TYPE_WEAPON..JUNK, or -1 if no item */
    int firstItemSubtype;      /* weapon/armour/potion/junk subtype, or -1 */
    /* All visible floor items (up to 4) for multi-item scatter */
    int floorItemTypes[M11_MAX_CELL_ITEMS];    /* THING_TYPE_*, -1 sentinel */
    int floorItemSubtypes[M11_MAX_CELL_ITEMS]; /* subtype per item, -1 sentinel */
    int floorItemCells[M11_MAX_CELL_ITEMS];    /* relative cell 0..3 */
    int floorItemCount; /* number of valid entries in floorItem arrays */
    /* Wall/door ornament ordinal from thing data (0-15, -1 if none) */
    int wallOrnamentOrdinal;
    int doorOrnamentOrdinal;
    /* First projectile graphic index (416-438) from GRAPHICS.DAT, or -1 */
    int firstProjectileGfxIndex;
    int firstProjectileSubtype;
    /* First projectile direction relative to party facing.
     * 0 = moving away from party, 1 = moving right, 2 = toward party,
     * 3 = moving left.  -1 if no projectile or direction unknown.
     * Used for DM1-faithful projectile sprite mirroring. */
    int firstProjectileRelDir;
    /* First projectile sub-cell position (0-3, or -1 if none).
     * In DM1, projectiles occupy a specific sub-cell (quadrant) within
     * the tile.  0=NW, 1=NE, 2=SW, 3=SE (absolute).  The relative
     * sub-cell position (relative to party facing) determines the
     * projectile's viewport offset within the cell face area.
     * Ref: ReDMCSB DUNVIEW.C F115 — M11_CELL(P141_T_Thing) positioning. */
    int firstProjectileCell;
    /* DM1 F0115/F0791 flip flags for first projectile bitmap:
     * bit0 = horizontal, bit1 = vertical. */
    int firstProjectileFlipFlags;
    /* Floor ornament ordinal for this square (1-based ordinal, 0 = none).
     * In DM1, corridor/pit/stair/teleporter squares may have floor
     * ornaments assigned via random generation or sensor things.
     * Ref: ReDMCSB DUNGEON.C F169 — SquareAspect[C4_FLOOR_ORNAMENT_ORDINAL]. */
    int floorOrnamentOrdinal;
    /* First explosion type (0-50), or -1 */
    int firstExplosionType;
    /* First explosion currentFrame (0..maxFrames-1), or -1 if none. */
    int firstExplosionFrame;
    /* First explosion maxFrames (from Phase17_ExplosionMaxFrames), or
     * 0 if none.  Used by the viewport burst renderer to modulate
     * size/intensity across the classic DM1 multi-frame aftermath. */
    int firstExplosionMaxFrames;
    /* First explosion residual attack (0..255), or -1 if none.  Poison
     * cloud and smoke decay this per frame; the burst renderer uses it
     * to fade the cloud as it dissipates. */
    int firstExplosionAttack;
    M11_SquareThingSummary summary;
} M11_ViewportCell;

static void m11_draw_creature_cue(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  int x,
                                  int y,
                                  int w,
                                  int h,
                                  int depthIndex) {
    int bodyW = w / 3;
    int bodyH = h / 3;
    int bodyX;
    int bodyY;
    int eyeY;
    unsigned char color = depthIndex == 0 ? M11_COLOR_LIGHT_GREEN : M11_COLOR_GREEN;

    if (bodyW < 4 || bodyH < 5) {
        return;
    }

    bodyX = x + (w - bodyW) / 2;
    bodyY = y + (h - bodyH) / 2;
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX + bodyW / 4, bodyY - 2, bodyW / 2, 3, color);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX, bodyY, bodyW, bodyH, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                   bodyX + 1, bodyY + bodyH, bodyY + bodyH + 2, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                   bodyX + bodyW - 2, bodyY + bodyH, bodyY + bodyH + 2, color);
    eyeY = bodyY + 2;
    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX + bodyW / 3, eyeY, M11_COLOR_BLACK);
    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX + (bodyW * 2) / 3, eyeY, M11_COLOR_BLACK);
}

static void m11_draw_item_cue(unsigned char* framebuffer,
                              int framebufferWidth,
                              int framebufferHeight,
                              int x,
                              int y,
                              int w,
                              int h,
                              int count) {
    int items = count > 3 ? 3 : count;
    int i;
    int baseY = y + h - 6;

    for (i = 0; i < items; ++i) {
        int cx = x + w / 2 - 8 + i * 7;
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx, baseY - 2, M11_COLOR_WHITE);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx - 1, baseY - 1, M11_COLOR_WHITE);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx + 1, baseY - 1, M11_COLOR_WHITE);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      cx - 1, baseY, 3, 2, M11_COLOR_YELLOW);
    }
}

/* Draw a GRAPHICS.DAT-backed projectile sprite in the given area.
 * relativeDir: projectile direction relative to party facing (0-3), or -1.
 *   0 = flying away, 1 = flying right, 2 = flying toward, 3 = flying left.
 * relativeCell: projectile sub-cell relative to party facing (0-3), or -1.
 *   0 = back-left, 1 = back-right, 2 = front-left, 3 = front-right.
 *   When >= 0, the sprite is offset within the face area to match the
 *   DM1 quadrant-based projectile positioning.  When -1, centered.
 * In DM1, projectile sprites are mirrored horizontally when the relative
 * direction is 1 (right-bound); left-bound (3) and forward/backward (0,2)
 * use the normal sprite orientation.
 * Ref: ReDMCSB DUNVIEW.C F115 — projectile coordinate lookup uses
 * G218_aaaauc_Graphic558_ObjectCoordinateSets to position projectiles
 * within the viewport cell based on their sub-cell.
 * Returns 1 if a real sprite was drawn, 0 for fallback. */
static int m11_projectile_source_scale_units(int depthIndex, int relativeCell) {
    /* DUNVIEW.C G0215_auc_Graphic558_ProjectileScales:
     *   32 D1 back/native, 27 D2 front, 21 D2 back,
     *   18 D3 front, 14 D3 back, 12 D4 front, 9 D4 back.
     * Values are scale units out of 32. The normal V1 pass draws D1..D3;
     * center sub-cells 0/1 are the back row and 2/3 are the front row. */
    static const unsigned char kProjectileScales[7] = {32, 27, 21, 18, 14, 12, 9};
    int frontRow = (relativeCell < 0) ? 1 : (relativeCell >= 2);
    int idx;
    if (depthIndex <= 0) return kProjectileScales[0];
    idx = depthIndex * 2 - (frontRow ? 1 : 0);
    if (idx < 1) idx = 1;
    if (idx > 6) idx = 6;
    return kProjectileScales[idx];
}

static unsigned int m11_projectile_aspect_graphic_info(int aspectIndex);
static int m11_object_source_scale_index(int depthIndex, int relativeCell);

static void m11_blit_scaled_flip(const M11_AssetSlot* slot,
                                 unsigned char* framebuffer,
                                 int fbW,
                                 int fbH,
                                 int dstX,
                                 int dstY,
                                 int dstW,
                                 int dstH,
                                 int transparentColor,
                                 int flipHorizontal,
                                 int flipVertical) {
    int dy;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        int dx;
        if (flipVertical) sy = (int)slot->height - 1 - sy;
        if (fbY < 0 || fbY >= fbH) continue;
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (flipHorizontal) sx = (int)slot->width - 1 - sx;
            if (fbX < 0 || fbX >= fbW) continue;
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) continue;
            framebuffer[fbY * fbW + fbX] = pixel;
        }
    }
}

static int m11_projectile_aspect_flip_flags(int aspectIndex,
                                            int relativeDir,
                                            int relativeCell,
                                            int mapX,
                                            int mapY) {
    unsigned int info = m11_projectile_aspect_graphic_info(aspectIndex);
    int aspectType = (int)(info & 0x0003u);
    int flags = 0;
    if (relativeDir < 0) relativeDir = 0;
    relativeDir &= 3;
    if (aspectType == 3) return 0;

    if (aspectType == 0) {
        int parityVertical = ((mapX + mapY) & 1) ? 1 : 0;
        if (relativeDir == 1 || relativeDir == 3) {
            if (relativeCell == 0 || relativeCell == 2) flags |= 0x01;
            if (parityVertical) flags |= 0x02;
            else flags ^= 0x01;
        } else {
            if (parityVertical && relativeCell < 2) flags |= 0x02;
        }
    } else if (relativeDir == 1) {
        flags |= 0x01;
    }
    if ((info & 0x0010u) && relativeDir == 3) {
        flags |= 0x01;
    }
    return flags;
}

static int m11_c2900_projectile_zone_point(int scaleIndex,
                                           int relativeCell,
                                           int* outX,
                                           int* outY) {
    /* Layout-696 C2900_ZONE_ table used by DUNVIEW.C F0115 for
     * projectile placement.  Five projectile scale buckets × four
     * view cells, parallel to the C2500 object table but higher in
     * the viewport (projectiles fly through the cell center rather
     * than lying on the floor). */
    static const short kC2900[5][4][2] = {
        {{  0,  0}, {  0,  0}, {129, 47}, { 95, 47}},
        {{  0,  0}, {  0,  0}, { 62, 47}, { 25, 47}},
        {{  0,  0}, {  0,  0}, {200, 47}, {162, 47}},
        {{  0,  0}, {  0,  0}, {  2, 47}, {-35, 47}},
        {{  0,  0}, {  0,  0}, {258, 47}, {202, 47}}
    };
    int zx;
    int zy;
    if (scaleIndex < 0) scaleIndex = 0;
    if (scaleIndex > 4) scaleIndex = 4;
    if (relativeCell < 0 || relativeCell > 3) return 0;
    zx = (int)kC2900[scaleIndex][relativeCell][0];
    zy = (int)kC2900[scaleIndex][relativeCell][1];
    if (zx == 0 && zy == 0) return 0;
    if (outX) *outX = zx;
    if (outY) *outY = zy;
    return 1;
}

static int m11_draw_projectile_sprite(const M11_GameViewState* state,
                                      unsigned char* framebuffer,
                                      int framebufferWidth,
                                      int framebufferHeight,
                                      int x, int y, int w, int h,
                                      int gfxIndex, int depthIndex,
                                      int relativeDir,
                                      int relativeCell,
                                      int flipFlags) {
    const M11_AssetSlot* slot;
    int drawW, drawH, drawX, drawY;
    int scale;
    if (!state || !state->assetsAvailable || gfxIndex < 454 ||
        gfxIndex >= 486) return 0;
    (void)relativeDir;
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, (unsigned int)gfxIndex);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    scale = m11_projectile_source_scale_units(depthIndex, relativeCell);
    drawW = (int)slot->width * scale / 32;
    drawH = (int)slot->height * scale / 32;
    if (drawW < 3) drawW = 3;
    if (drawH < 3) drawH = 3;
    if (drawW > w) drawW = w;
    if (drawH > h) drawH = h;
    /* DM1 sub-cell projectile positioning.
     * In the original, each projectile is drawn at a specific quadrant
     * of the viewport cell face.  The relative cell (0-3) maps to:
     *   0 = back-left:   upper-left quadrant
     *   1 = back-right:  upper-right quadrant
     *   2 = front-left:  lower-left quadrant
     *   3 = front-right: lower-right quadrant
     * We offset the sprite center toward the appropriate quadrant.
     * At greater depth the offset is reduced proportionally. */
    if (relativeCell >= 0 && relativeCell <= 3) {
        int zoneX = 0;
        int zoneY = 0;
        int scaleIndex = m11_object_source_scale_index(depthIndex, relativeCell);
        int qx = w / 4;  /* quarter-width offset */
        int qy = h / 4;  /* quarter-height offset */
        if (x >= M11_VIEWPORT_X && y >= M11_VIEWPORT_Y &&
            m11_c2900_projectile_zone_point(scaleIndex, relativeCell, &zoneX, &zoneY)) {
            drawX = M11_VIEWPORT_X + zoneX - drawW / 2;
            drawY = M11_VIEWPORT_Y + zoneY - drawH / 2;
        } else {
            /* Reduce offset at depth to match perspective convergence */
            if (depthIndex >= 1) { qx = qx * 2 / 3; qy = qy * 2 / 3; }
            if (depthIndex >= 2) { qx = qx / 2;     qy = qy / 2; }
            switch (relativeCell) {
                case 0: /* back-left: upper-left */
                    drawX = x + (w / 2 - qx) - drawW / 2;
                    drawY = y + (h / 2 - qy) - drawH / 2;
                    break;
                case 1: /* back-right: upper-right */
                    drawX = x + (w / 2 + qx) - drawW / 2;
                    drawY = y + (h / 2 - qy) - drawH / 2;
                    break;
                case 2: /* front-left: lower-left */
                    drawX = x + (w / 2 - qx) - drawW / 2;
                    drawY = y + (h / 2 + qy) - drawH / 2;
                    break;
                default: /* front-right: lower-right */
                    drawX = x + (w / 2 + qx) - drawW / 2;
                    drawY = y + (h / 2 + qy) - drawH / 2;
                    break;
            }
        }
        /* Clamp to face bounds */
        if (drawX < x) drawX = x;
        if (drawY < y) drawY = y;
        if (drawX + drawW > x + w) drawX = x + w - drawW;
        if (drawY + drawH > y + h) drawY = y + h - drawH;
    } else {
        drawX = x + (w - drawW) / 2;
        drawY = y + (h - drawH) / 2;
    }
    /* DM1 draws object/projectile bitmaps through F0791 with
     * C10_COLOR_FLESH as the transparent key. Fireball native graphics
     * have palette index 10 in their border, so keying on black leaves a
     * visible square backing. */
    if (flipFlags & 0x02) {
        m11_blit_scaled_flip(slot, framebuffer, framebufferWidth, framebufferHeight,
                             drawX, drawY, drawW, drawH,
                             M11_COLOR_MAGENTA,
                             (flipFlags & 0x01) ? 1 : 0,
                             1);
    } else if (flipFlags & 0x01) {
        M11_AssetLoader_BlitScaledMirror(slot, framebuffer, framebufferWidth,
                                         framebufferHeight,
                                         drawX, drawY, drawW, drawH,
                                         M11_COLOR_MAGENTA);
    } else {
        M11_AssetLoader_BlitScaled(slot, framebuffer, framebufferWidth,
                                   framebufferHeight,
                                   drawX, drawY, drawW, drawH,
                                   M11_COLOR_MAGENTA);
    }
    return 1;
}

/* DM1 explosion-type -> explosion aspect index (0..3).
 * Ref: ReDMCSB DUNVIEW.C F0141 explosion draw loop (lines 1850-1878):
 *   FIREBALL / LIGHTNING_BOLT / REBIRTH_STEP2  -> C0_EXPLOSION_ASPECT_FIRE    (0)
 *   POISON_BOLT / POISON_CLOUD                 -> C2_EXPLOSION_ASPECT_POISON  (2)
 *   SMOKE                                      -> C3_EXPLOSION_ASPECT_SMOKE   (3)
 *   everything else (HARM_NON_MATERIAL, OPEN_DOOR, dispell, etc.)
 *                                              -> C1_EXPLOSION_ASPECT_SPELL   (1)
 *
 * Returns -1 for FLUXCAGE (not drawn through this path in DM1) and for
 * REBIRTH_STEP1 (handled separately in the original). */
static int m11_explosion_type_to_aspect(int expType) {
    if (expType < 0) return -1;
    if (expType == C050_EXPLOSION_FLUXCAGE) return -1;
    if (expType == C100_EXPLOSION_REBIRTH_STEP1) return -1;
    if (expType == C000_EXPLOSION_FIREBALL
            || expType == C002_EXPLOSION_LIGHTNING_BOLT
            || expType == C101_EXPLOSION_REBIRTH_STEP2) {
        return 0; /* FIRE */
    }
    if (expType == C007_EXPLOSION_POISON_CLOUD) {
        /* In DM1 POISON_BOLT (type 6) also maps to POISON aspect, but
         * our V1 header does not define C006.  The POISON_CLOUD branch
         * covers the only on-viewport explosion variant we spawn. */
        return 2; /* POISON */
    }
    if (expType == C040_EXPLOSION_SMOKE) {
        return 3; /* SMOKE */
    }
    return 1; /* SPELL (HARM_NON_MATERIAL / OPEN_DOOR / default) */
}

/* DM1 explosion aspect -> native GRAPHICS.DAT index.
 * Ref: ReDMCSB DUNVIEW.C F0136 F0675_DUNGEONVIEW_GetScaledBitmap
 *   bitmap index = M614_GRAPHIC_FIRST_EXPLOSION (=486)
 *                + min(aspectIndex, C2_EXPLOSION_ASPECT_POISON)
 * Aspect 0 (fire)   -> 486
 * Aspect 1 (spell)  -> 487
 * Aspect 2 (poison) -> 488
 * Aspect 3 (smoke)  -> 488 (with smoke palette change on pixel values 6,7). */
enum { M11_GFX_FIRST_EXPLOSION = 486 };
static int m11_explosion_aspect_to_gfx(int aspect) {
    if (aspect < 0) return -1;
    if (aspect >= 2) return M11_GFX_FIRST_EXPLOSION + 2;
    return M11_GFX_FIRST_EXPLOSION + aspect;
}

/* Draw a DM1 explosion bitmap from GRAPHICS.DAT.
 *
 * Replaces the previous cue-style palette-rect bloom with the real
 * explosion bitmap frames Fontanel ships in GRAPHICS.DAT at indices
 * 486 (fire), 487 (spell) and 488 (poison; also reused for smoke
 * with palette changes).
 *
 * The DM1 original calls F0675_DUNGEONVIEW_GetScaledBitmap to request a
 * cached scaled copy of the bitmap at a depth-specific pixel size.  We
 * achieve the same visible result by loading the native bitmap once and
 * blitting it with M11_AssetLoader_BlitScaled at a face-local size
 * chosen from the depth slot, then modulating that size across
 * currentFrame / maxFrames so the bitmap blooms on frame 0 and fades
 * on later frames (fireball / lightning / dispell) or decays with the
 * residual attack (poison / smoke), matching how F0822 progresses
 * the aftermath tick-by-tick.
 *
 * Smoke reuses the poison bitmap with DM1's palette substitution
 * G0212_auc_Graphic558_PaletteChanges_Smoke {0,1,2,3,4,5,12,1,...}
 * which only changes pixel values 6 (-> 12 = DARK_GRAY) and 7 (-> 1 =
 * NAVY); all other indices are identity.  We implement those two
 * remaps via BlitScaledReplace's two replacement slots.
 *
 * Returns 1 if a real bitmap was blit, 0 if the loader is not ready
 * (in which case callers draw the fallback palette-rect cue). */
static int m11_draw_explosion_sprite(const M11_GameViewState* state,
                                     unsigned char* framebuffer,
                                     int framebufferWidth,
                                     int framebufferHeight,
                                     int x, int y, int w, int h,
                                     int expType,
                                     int frame,
                                     int maxFrames,
                                     int attack,
                                     int depthIndex) {
    const M11_AssetSlot* slot;
    int aspect;
    int gfxIndex;
    int baseScale;
    int scale;
    int drawW, drawH, drawX, drawY;
    int isSmoke;
    if (!state || !state->assetsAvailable) return 0;
    aspect = m11_explosion_type_to_aspect(expType);
    if (aspect < 0) return 0;
    gfxIndex = m11_explosion_aspect_to_gfx(aspect);
    if (gfxIndex < 0) return 0;
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)gfxIndex);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* Depth-based base scale: fills most of the face at depth 0, smaller
     * at greater depths.  Matches the DM1 look where a point-blank
     * fireball swallows the viewport cell while a distant one is a
     * small orange blob. */
    baseScale = (depthIndex == 0) ? 100
               : (depthIndex == 1) ? 70
               : 45;
    scale = baseScale;

    /* Per-frame bloom/fade modulation for the one-shot aspects
     * (fire / spell).  DM1's F0822 ADVANCES the explosion frame
     * counter and despawns one-shots after maxFrames; we bloom on
     * frame 0, peak on frame 1, and fade on frame 2+. */
    if (aspect == 0 || aspect == 1) {
        if (frame >= 0 && maxFrames > 0) {
            if (frame == 0)      scale = (baseScale * 110) / 100; /* 110% */
            else if (frame == 1) scale = baseScale;                /* 100% */
            else                 scale = (baseScale * 65) / 100;  /*  65% */
        }
    } else if (aspect == 2 || aspect == 3) {
        /* Poison cloud / smoke: size tracks residual attack instead of
         * the frame index.  F0822 decays attack by 3 (poison) or 40
         * (smoke) per tick, so the cloud naturally shrinks as it
         * dissipates.  Clamp to [60%, 115%] of baseScale. */
        if (attack >= 0) {
            int a = attack > 200 ? 200 : attack;
            int pct = 60 + (a * 55) / 200;
            if (pct < 60)  pct = 60;
            if (pct > 115) pct = 115;
            scale = (baseScale * pct) / 100;
        }
    }

    drawW = (int)slot->width  * scale / 100;
    drawH = (int)slot->height * scale / 100;
    /* Minimum visible footprint so we never collapse to a single pixel. */
    if (drawW < 4) drawW = 4;
    if (drawH < 4) drawH = 4;
    /* Clamp to the face box so we do not spill into neighbouring cells. */
    if (drawW > w) drawW = w;
    if (drawH > h) drawH = h;
    drawX = x + (w - drawW) / 2;
    drawY = y + (h - drawH) / 2;

    isSmoke = (aspect == 3);
    if (isSmoke) {
        /* DM1 G0212_auc_Graphic558_PaletteChanges_Smoke identity-maps
         * everything except pixel 6 -> 12 (DARK_GRAY) and 7 -> 1
         * (NAVY).  BlitScaledReplace has two replacement slots; use
         * them for exactly those two. */
        M11_AssetLoader_BlitScaledReplace(
            slot, framebuffer, framebufferWidth, framebufferHeight,
            drawX, drawY, drawW, drawH,
            /*transparentColor*/ 0,
            /*replSrc9*/ 6,  /*replDst9*/ M11_COLOR_DARK_GRAY,
            /*replSrc10*/ 7, /*replDst10*/ M11_COLOR_NAVY);
    } else {
        M11_AssetLoader_BlitScaled(
            slot, framebuffer, framebufferWidth, framebufferHeight,
            drawX, drawY, drawW, drawH, /*transparentColor*/ 0);
    }
    return 1;
}

/* Draw a pit visual effect: darkened floor area with depth gradient.
 * In DM1, pits appear as dark openings in the floor.  We darken the
 * lower portion of the cell face to simulate the pit opening. */
static void m11_draw_pit_effect(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                int x, int y, int w, int h,
                                int depthIndex) {
    int pitY = y + h / 3;
    int pitH = h - h / 3;
    (void)depthIndex;
    /* Draw dark pit opening on the floor area */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x + 2, pitY, w - 4, pitH - 2, M11_COLOR_BLACK);
    /* Outline for definition — dark gray border around the pit */
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x + 1, pitY - 1, w - 2, pitH, M11_COLOR_DARK_GRAY);
    /* Inner shadow lines for depth perception */
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   x + 3, x + w - 4, pitY + 2, M11_COLOR_NAVY);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   x + 4, x + w - 5, pitY + pitH - 4, M11_COLOR_NAVY);
}

/* Draw a teleporter visual effect: colored shimmer/rift.
 * In DM1, teleporters have a distinctive visual presence in the
 * viewport — a shimmering effect.  We render a colored frame with
 * an inner glow pattern. */
static void m11_draw_teleporter_effect(unsigned char* framebuffer,
                                       int framebufferWidth,
                                       int framebufferHeight,
                                       int x, int y, int w, int h,
                                       int depthIndex) {
    int cx = x + w / 2;
    int cy = y + h / 2;
    int rw = w * 2 / 3;
    int rh = h * 2 / 3;
    unsigned char color = depthIndex == 0 ? M11_COLOR_LIGHT_CYAN :
                          (depthIndex == 1 ? M11_COLOR_CYAN : M11_COLOR_NAVY);
    /* Outer shimmer frame */
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  cx - rw / 2, cy - rh / 2, rw, rh, color);
    /* Inner cross pattern */
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   cx - rw / 4, cx + rw / 4, cy, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                   cx, cy - rh / 4, cy + rh / 4, color);
    /* Corner dots for shimmer */
    if (rw > 8 && rh > 8) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                      cx - rw / 3, cy - rh / 3, M11_COLOR_WHITE);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                      cx + rw / 3, cy + rh / 3, M11_COLOR_WHITE);
    }
}

static void m11_draw_effect_cue(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                int x,
                                int y,
                                int w,
                                int h,
                                const M11_ViewportCell* cell,
                                int depthIndex) {
    int cx = x + w / 2;
    int cy = y + h / 2;
    if (!cell) {
        return;
    }
    /* Projectile sprite from GRAPHICS.DAT, or fallback crosshair */
    if (cell->summary.projectiles > 0) {
        if (!g_drawState ||
            !m11_draw_projectile_sprite(g_drawState, framebuffer,
                                        framebufferWidth, framebufferHeight,
                                        x, y, w, h,
                                        cell->firstProjectileGfxIndex,
                                        depthIndex,
                                        cell->firstProjectileRelDir,
                                        cell->firstProjectileCell,
                                        cell->firstProjectileFlipFlags)) {
            /* Fallback: cyan crosshair */
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           cx - 3, cx + 3, cy, M11_COLOR_LIGHT_CYAN);
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                           cx, cy - 3, cy + 3, M11_COLOR_LIGHT_CYAN);
        }
    }
    /* Teleporter shimmer effect */
    if (cell->summary.teleporters > 0) {
        m11_draw_teleporter_effect(framebuffer, framebufferWidth, framebufferHeight,
                                   x, y, w, h, depthIndex);
    }
    /* Pit darkness effect */
    if (cell->elementType == DUNGEON_ELEMENT_PIT) {
        m11_draw_pit_effect(framebuffer, framebufferWidth, framebufferHeight,
                            x, y, w, h, depthIndex);
    }
    /* DM1 explosion-type-specific viewport visual effects.
     *
     * Primary path: load the real DM1 explosion bitmap from
     * GRAPHICS.DAT at index 486 (fire), 487 (spell) or 488 (poison /
     * smoke) and blit it with depth-scaled, frame-modulated size so
     * the post-detonation aftermath looks like the classic DM1 burst
     * rather than a palette-rect cue.  This replaces pass 25's cue-
     * style fill with source-backed bitmap frames while preserving
     * the F0822 advance/fade/despawn lifecycle from pass 25.
     *
     * Ref: ReDMCSB DUNVIEW.C F0136 F0675_DUNGEONVIEW_GetScaledBitmap
     *      and F0141 explosion draw loop (lines 1842-1878) selecting
     *      EXPLOSION_ASPECT {FIRE, SPELL, POISON, SMOKE} from the
     *      explosion Type.
     *
     * Fallback: when no GRAPHICS.DAT loader is available (headless
     * probes using synthetic worlds) we keep the palette-rect cue so
     * probes keep asserting visible content at the cell. */
    if (cell->summary.explosions > 0) {
        unsigned char expColor;
        int expType = cell->firstExplosionType;
        int baseR   = 5 + (depthIndex == 0 ? 2 : 0);
        int expR    = baseR;
        int frame   = cell->firstExplosionFrame;
        int maxF    = cell->firstExplosionMaxFrames;
        int atk     = cell->firstExplosionAttack;
        int drewBitmap = 0;
        if (g_drawState) {
            drewBitmap = m11_draw_explosion_sprite(
                g_drawState, framebuffer, framebufferWidth,
                framebufferHeight, x, y, w, h,
                expType, frame, maxF, atk, depthIndex);
        }
        /* Cue-style palette-rect fallback for probes / headless
         * environments without a GRAPHICS.DAT loader.  Real builds
         * take the bitmap path above, matching classic DM1. */
        if (!drewBitmap) {
            /* Multi-frame aftermath progression (DM1-style bloom-then-fade):
             *   - Fireball / lightning / dispell: short ~3-frame bloom that
             *     peaks at frame 1 and fades by frame 2+ (radius 120/90/60%
             *     of base).
             *   - Poison cloud / smoke: large cloud whose radius tracks the
             *     residual attack (larger when thick, smaller as it decays).
             *   - Other types: single frame at base radius. */
            if (frame >= 0 && maxF > 0) {
                if (expType == C000_EXPLOSION_FIREBALL
                        || expType == C002_EXPLOSION_LIGHTNING_BOLT
                        || expType == C003_EXPLOSION_HARM_NON_MATERIAL) {
                    if (frame == 0)      expR = (baseR * 6) / 5;  /* 120% */
                    else if (frame == 1) expR = baseR;            /* 100% */
                    else                 expR = (baseR * 3) / 5;  /* 60%  */
                    if (expR < 2) expR = 2;
                } else if (expType == C007_EXPLOSION_POISON_CLOUD
                        || expType == C040_EXPLOSION_SMOKE) {
                    /* Radius tracks residual attack: full at spawn, shrinks
                     * as F0822 decays the cloud.  Attack starts up to 255
                     * and decays by 3 (poison) or 40 (smoke) per frame;
                     * normalise to baseR range. */
                    if (atk > 0) {
                        int num = (baseR * (atk > 128 ? 128 : atk)) / 64;
                        expR = baseR / 2 + num / 2;
                        if (expR < baseR / 2) expR = baseR / 2;
                        if (expR > baseR + 2) expR = baseR + 2;
                    }
                }
            }
            if (expType == C002_EXPLOSION_LIGHTNING_BOLT) {
                /* Lightning/energy: cyan-white flash that dims on fade. */
                unsigned char cross = (frame >= 1) ? M11_COLOR_LIGHT_CYAN
                                                   : M11_COLOR_WHITE;
                expColor = M11_COLOR_LIGHT_CYAN;
                m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                               cx - expR, cx + expR, cy, cross);
                m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                               cx, cy - expR, cy + expR, cross);
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
            } else if (expType == C007_EXPLOSION_POISON_CLOUD) {
                /* Poison cloud: green haze that thins as the cloud decays. */
                int coreR = expR / 2;
                int thinning = (atk >= 0 && atk < 40);
                expColor = thinning ? M11_COLOR_DARK_GRAY : M11_COLOR_GREEN;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
                if (!thinning && coreR > 0) {
                    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  cx - coreR, cy - coreR,
                                  coreR * 2 + 1, coreR * 2 + 1, M11_COLOR_LIGHT_GREEN);
                }
            } else if (expType == C040_EXPLOSION_SMOKE) {
                /* Smoke: dark-gray cloud that thins and fades. */
                expColor = (atk >= 0 && atk < 80) ? M11_COLOR_DARK_GRAY
                                                  : M11_COLOR_LIGHT_GRAY;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx - expR, cy - expR,
                              expR * 2 + 1, expR * 2 + 1, expColor);
            } else if (expType >= 0 && expType <= 7) {
                /* Fire/fireball explosions: orange-red burst with yellow core
                 * that shrinks on fade frames. */
                int coreR = expR / 2;
                if (frame >= 0 && maxF > 0 && frame >= maxF - 1) coreR = 0;
                expColor = (frame >= 2) ? M11_COLOR_BROWN : M11_COLOR_LIGHT_RED;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
                if (coreR > 0) {
                    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  cx - coreR, cy - coreR,
                                  coreR * 2 + 1, coreR * 2 + 1, M11_COLOR_YELLOW);
                }
            } else {
                /* All other explosion types: generic magenta burst */
                expColor = M11_COLOR_MAGENTA;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx - expR / 2, cy - expR / 2,
                              expR + 1, expR + 1, expColor);
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
            }
        } else {
            /* Silence unused-variable warnings: baseR / expR / expColor
             * only matter in the cue-fallback branch; the bitmap path
             * already draws the explosion. */
            (void)baseR; (void)expR; (void)expColor;
            (void)frame; (void)maxF; (void)atk; (void)expType;
        }
    }
    if (cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      cx - 4, cy - 4, 9, 9, M11_COLOR_MAGENTA);
    }
}

static void m11_append_phrase(char* out,
                              size_t outSize,
                              const char* phrase) {
    size_t used;
    if (!out || outSize == 0U || !phrase || phrase[0] == '\0') {
        return;
    }
    used = strlen(out);
    if (used > 0U && used + 2U < outSize) {
        snprintf(out + used, outSize - used, ", ");
        used = strlen(out);
    }
    if (used + 1U < outSize) {
        snprintf(out + used, outSize - used, "%s", phrase);
    }
}

static void m11_append_count_phrase(char* out,
                                    size_t outSize,
                                    int count,
                                    const char* singular,
                                    const char* plural) {
    char chunk[32];
    if (count <= 0 || !singular || !plural) {
        return;
    }
    snprintf(chunk, sizeof(chunk), "%d %s", count, count == 1 ? singular : plural);
    m11_append_phrase(out, outSize, chunk);
}

static void m11_format_square_summary(const M11_ViewportCell* cell,
                                      char* out,
                                      size_t outSize) {
    char extras[96];
    const char* base;

    if (!out || outSize == 0U) {
        return;
    }
    if (!cell || !cell->valid) {
        snprintf(out, outSize, "VOID");
        return;
    }

    extras[0] = '\0';
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        m11_append_phrase(extras, sizeof(extras),
                          (cell->doorState == 0 || cell->doorState == 5) ? "OPEN" : "SHUT");
    }
    m11_append_count_phrase(extras, sizeof(extras), cell->summary.groups, "FOE", "FOES");
    m11_append_count_phrase(extras, sizeof(extras), cell->summary.items, "ITEM", "ITEMS");
    m11_append_count_phrase(extras, sizeof(extras),
                            cell->summary.projectiles + cell->summary.explosions,
                            "EFFECT", "EFFECTS");
    m11_append_count_phrase(extras, sizeof(extras), cell->summary.sensors, "SENSOR", "SENSORS");
    m11_append_count_phrase(extras, sizeof(extras),
                            cell->summary.textStrings + cell->summary.teleporters,
                            "MARK", "MARKS");

    base = F0503_DUNGEON_GetElementName_Compat(cell->elementType);
    if (extras[0] != '\0') {
        snprintf(out, outSize, "%s, %s", base, extras);
    } else {
        snprintf(out, outSize, "%s", base);
    }
}

static void m11_format_square_things(unsigned short firstThing,
                                     int squareThingCount,
                                     char* out,
                                     size_t outSize) {
    int thingType;
    if (!out || outSize == 0U) {
        return;
    }
    if (firstThing == THING_ENDOFLIST || firstThing == THING_NONE || squareThingCount <= 0) {
        snprintf(out, outSize, "QUIET FLOOR");
        return;
    }
    thingType = THING_GET_TYPE(firstThing);
    switch (thingType) {
        case THING_TYPE_GROUP:
            snprintf(out, outSize, "%d CREATURE %s",
                     squareThingCount,
                     squareThingCount == 1 ? "CONTACT" : "CONTACTS");
            break;
        case THING_TYPE_PROJECTILE:
        case THING_TYPE_EXPLOSION:
            snprintf(out, outSize, "%d LIVE EFFECT %s",
                     squareThingCount,
                     squareThingCount == 1 ? "AHEAD" : "STACKED");
            break;
        case THING_TYPE_TEXTSTRING:
            snprintf(out, outSize, "TEXT PLAQUE");
            break;
        case THING_TYPE_SENSOR:
            snprintf(out, outSize, "TRIGGER TILE");
            break;
        case THING_TYPE_TELEPORTER:
            snprintf(out, outSize, "TELEPORT FIELD");
            break;
        default:
            snprintf(out, outSize, "%d FLOOR %s",
                     squareThingCount,
                     squareThingCount == 1 ? "ITEM" : "ITEMS");
            break;
    }
}

static void m11_get_food_water_average(const struct PartyState_Compat* party,
                                       int* outFood,
                                       int* outWater) {
    int slot;
    int foodTotal = 0;
    int waterTotal = 0;
    int count = 0;
    if (!party) {
        if (outFood) {
            *outFood = 0;
        }
        if (outWater) {
            *outWater = 0;
        }
        return;
    }
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        if (!party->champions[slot].present) {
            continue;
        }
        foodTotal += party->champions[slot].food;
        waterTotal += party->champions[slot].water;
        ++count;
    }
    if (outFood) {
        *outFood = count > 0 ? foodTotal / count : 0;
    }
    if (outWater) {
        *outWater = count > 0 ? waterTotal / count : 0;
    }
}

static void m11_direction_vectors(int direction,
                                  int* outForwardX,
                                  int* outForwardY,
                                  int* outRightX,
                                  int* outRightY) {
    if (!outForwardX || !outForwardY || !outRightX || !outRightY) {
        return;
    }
    switch (direction) {
        case DIR_NORTH:
            *outForwardX = 0;
            *outForwardY = -1;
            *outRightX = 1;
            *outRightY = 0;
            break;
        case DIR_EAST:
            *outForwardX = 1;
            *outForwardY = 0;
            *outRightX = 0;
            *outRightY = 1;
            break;
        case DIR_SOUTH:
            *outForwardX = 0;
            *outForwardY = 1;
            *outRightX = -1;
            *outRightY = 0;
            break;
        case DIR_WEST:
        default:
            *outForwardX = -1;
            *outForwardY = 0;
            *outRightX = 0;
            *outRightY = -1;
            break;
    }
}

/* ── Per-map ornament index cache ──
 * Load the wall and door ornament index tables for a given map from
 * DUNGEON.DAT metadata.  In the original, the metadata follows the
 * square tile data: creatureTypeCount bytes of creature type indices,
 * then wallOrnamentCount bytes of wall ornament graphic indices,
 * then floorOrnamentCount bytes of floor ornament graphic indices,
 * then doorOrnamentCount bytes of door ornament graphic indices.
 * Ref: ReDMCSB DUNGEON.C F0173_DUNGEON_ReadMapData_CPSE.
 * We read these lazily and cache in the game view state. */
static void m11_ensure_ornament_cache(M11_GameViewState* state, int mapIndex) {
    const struct DungeonDatState_Compat* dun;
    const struct DungeonMapDesc_Compat* m;
    long rawDataFileOffset;
    long mapFileOffset;
    int squareCount;
    unsigned char metaBuf[128]; /* enough for all metadata bytes */
    int metaSize;
    int offset;
    FILE* fp;
    int i;
    int totalColumns;
    long thingDataTotalBytes;

    if (!state || !state->world.dungeon || !state->world.dungeon->loaded) return;
    dun = state->world.dungeon;
    if (mapIndex < 0 || mapIndex >= (int)dun->header.mapCount) return;
    if (mapIndex >= (int)32) return;
    if (state->ornamentCacheLoaded[mapIndex]) return;

    m = &dun->maps[mapIndex];
    squareCount = (int)m->width * (int)m->height;

    /* Compute metadata size: creature types + wall ornaments (+ inscription slot) +
     * floor ornaments + door ornaments. */
    metaSize = (int)m->creatureTypeCount + (int)m->wallOrnamentCount + 1 +
               (int)m->floorOrnamentCount + (int)m->doorOrnamentCount;
    if (metaSize <= 0 || metaSize > (int)sizeof(metaBuf)) {
        state->ornamentCacheLoaded[mapIndex] = 1;
        return;
    }

    /* Reconstruct the DUNGEON.DAT path from the GRAPHICS.DAT path
     * stored in the asset loader (same directory). */
    {
        char datPath[512];
        const char* gfxPath = state->assetLoader.graphicsDatPath;
        const char* lastSlash;
        int dirLen;
        if (!state->assetsAvailable || gfxPath[0] == '\0') {
            state->ornamentCacheLoaded[mapIndex] = 1;
            return;
        }
        /* Find directory portion of GRAPHICS.DAT path */
        lastSlash = strrchr(gfxPath, '/');
        if (!lastSlash) lastSlash = strrchr(gfxPath, '\\');
        if (lastSlash) {
            dirLen = (int)(lastSlash - gfxPath);
        } else {
            dirLen = 0;
        }
        if (dirLen > 0) {
            snprintf(datPath, sizeof(datPath), "%.*s/DUNGEON.DAT", dirLen, gfxPath);
        } else {
            snprintf(datPath, sizeof(datPath), "DUNGEON.DAT");
        }
        fp = fopen(datPath, "rb");
        if (!fp && dirLen > 0) {
            snprintf(datPath, sizeof(datPath), "%.*s/dungeon.dat", dirLen, gfxPath);
            fp = fopen(datPath, "rb");
        }
        if (!fp) {
            state->ornamentCacheLoaded[mapIndex] = 1;
            return;
        }
    }

    /* Compute raw data section file offset (same formula as
     * F0501_DUNGEON_LoadTileData_Compat). */
    totalColumns = 0;
    for (i = 0; i < (int)dun->header.mapCount; ++i) {
        totalColumns += dun->maps[i].width;
    }
    thingDataTotalBytes = 0;
    for (i = 0; i < 16; ++i) {
        thingDataTotalBytes += (long)dun->header.thingCounts[i] *
                               (long)s_thingDataByteCount[i];
    }
    rawDataFileOffset = DUNGEON_HEADER_SIZE +
                        (long)dun->header.mapCount * DUNGEON_MAP_DESC_SIZE +
                        (long)totalColumns * 2 +
                        (long)dun->header.squareFirstThingCount * 2 +
                        thingDataTotalBytes;
    mapFileOffset = rawDataFileOffset + (long)m->rawMapDataByteOffset;

    /* Seek past square data to the metadata area */
    if (fseek(fp, mapFileOffset + (long)squareCount, SEEK_SET) != 0) {
        fclose(fp);
        state->ornamentCacheLoaded[mapIndex] = 1;
        return;
    }

    if ((int)fread(metaBuf, 1, (size_t)metaSize, fp) != metaSize) {
        fclose(fp);
        state->ornamentCacheLoaded[mapIndex] = 1;
        return;
    }
    fclose(fp);

    /* Parse: skip creature type bytes, then read wall ornament indices.
     * Each byte is a global ornament index used by the rendering engine.
     * Ref: ReDMCSB DUNGEON.C F0173_DUNGEON_ReadMapData_CPSE —
     *   CopyBytes(MapMetaData + CreatureTypeCount, G0261, WallOrnamentCount)
     *   G0261[InscriptionWallOrnamentIndex] = C0_WALL_ORNAMENT_INSCRIPTION */
    offset = (int)m->creatureTypeCount;
    for (i = 0; i < 16; ++i) {
        if (i < (int)m->wallOrnamentCount + 1 && (offset + i) < metaSize) {
            state->wallOrnamentIndices[mapIndex][i] = (int)metaBuf[offset + i];
        } else {
            state->wallOrnamentIndices[mapIndex][i] = i; /* identity fallback */
        }
    }
    offset += (int)m->wallOrnamentCount + 1; /* +1 for inscription ornament */

    /* Read floor ornament indices.
     * Ref: ReDMCSB DUNGEON.C F173 — G262_auc_CurrentMapFloorOrnamentIndices.
     * Each byte is a global floor ornament graphic set index.  In the
     * original, floor ornaments start at graphic index 247 and each
     * ornament has 6 perspective variants (D3L..D1R).
     * Floor ornament graphic = 247 + globalIndex * 6 + viewOffset. */
    for (i = 0; i < 16; ++i) {
        if (i < (int)m->floorOrnamentCount && (offset + i) < metaSize) {
            state->floorOrnamentIndices[mapIndex][i] = (int)metaBuf[offset + i];
        } else {
            state->floorOrnamentIndices[mapIndex][i] = -1; /* no ornament */
        }
    }
    offset += (int)m->floorOrnamentCount;

    /* Read door ornament indices */
    for (i = 0; i < 16; ++i) {
        if (i < (int)m->doorOrnamentCount && (offset + i) < metaSize) {
            state->doorOrnamentIndices[mapIndex][i] = (int)metaBuf[offset + i];
        } else {
            state->doorOrnamentIndices[mapIndex][i] = i; /* identity fallback */
        }
    }

    state->ornamentCacheLoaded[mapIndex] = 1;
}

static int m11_projectile_subtype_to_graphic_index(int subtype) {
    /* ReDMCSB F0142 + DUNVIEW.C G0210_as_Graphic558_ProjectileAspects.
     * M613_GRAPHIC_FIRST_PROJECTILE is 454.  Spell/explosion-like
     * projectile subtypes map to projectile aspect native graphics, not
     * to the older object/weapon graphics around 416. */
    switch (subtype) {
        case PROJECTILE_SUBTYPE_FIREBALL:
            return 454 + 28; /* C10_PROJECTILE_ASPECT_EXPLOSION_FIREBALL */
        case PROJECTILE_SUBTYPE_SLIME:
            return 454 + 30; /* C12_PROJECTILE_ASPECT_EXPLOSION_SLIME */
        case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:
            return 454 + 9;  /* C03_PROJECTILE_ASPECT_EXPLOSION_LIGHTNING_BOLT */
        case PROJECTILE_SUBTYPE_POISON_BOLT:
        case PROJECTILE_SUBTYPE_POISON_CLOUD:
            return 454 + 31; /* C13_PROJECTILE_ASPECT_EXPLOSION_POISON */
        case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
        case PROJECTILE_SUBTYPE_OPEN_DOOR:
            return 454 + 29; /* C11_PROJECTILE_ASPECT_EXPLOSION_DEFAULT */
        case PROJECTILE_SUBTYPE_KINETIC_ARROW:
        default:
            return 454;      /* first projectile aspect / safe kinetic fallback */
    }
}

static int m11_projectile_subtype_to_aspect_index(int subtype) {
    switch (subtype) {
        case PROJECTILE_SUBTYPE_FIREBALL: return 10;
        case PROJECTILE_SUBTYPE_SLIME: return 12;
        case PROJECTILE_SUBTYPE_LIGHTNING_BOLT: return 3;
        case PROJECTILE_SUBTYPE_POISON_BOLT:
        case PROJECTILE_SUBTYPE_POISON_CLOUD: return 13;
        case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
        case PROJECTILE_SUBTYPE_OPEN_DOOR: return 11;
        case PROJECTILE_SUBTYPE_KINETIC_ARROW:
        default: return 0;
    }
}

static int m11_projectile_aspect_first_native(int aspectIndex) {
    static const unsigned char kFirstNative[14] = {
        0,3,6,9,11,14,17,20,23,26,28,29,30,31
    };
    if (aspectIndex < 0 || aspectIndex >= 14) return -1;
    return (int)kFirstNative[aspectIndex];
}

static unsigned int m11_projectile_aspect_graphic_info(int aspectIndex) {
    static const unsigned short kGraphicInfo[14] = {
        0x0011,0x0011,0x0010,0x0112,0x0011,0x0010,0x0010,
        0x0011,0x0011,0x0012,0x0103,0x0103,0x0103,0x0103
    };
    if (aspectIndex < 0 || aspectIndex >= 14) return 0u;
    return (unsigned int)kGraphicInfo[aspectIndex];
}

static int m11_projectile_aspect_bitmap_delta(int aspectIndex, int relativeDir) {
    unsigned int info = m11_projectile_aspect_graphic_info(aspectIndex);
    int aspectType = (int)(info & 0x0003u);
    if (relativeDir < 0) relativeDir = 0;
    relativeDir &= 3;
    if (aspectType == 3) return 0;
    if (relativeDir == 1 || relativeDir == 3) {
        return aspectType == 2 ? 1 : 2;
    }
    if (aspectType >= 2) return 0;
    if (aspectType == 1 && relativeDir != 0) return 0;
    return 1;
}

static int m11_projectile_aspect_to_graphic_index(int aspectIndex, int relativeDir) {
    int first = m11_projectile_aspect_first_native(aspectIndex);
    if (first < 0) return -1;
    return 454 + first + m11_projectile_aspect_bitmap_delta(aspectIndex, relativeDir);
}

static int m11_sample_viewport_cell(const M11_GameViewState* state,
                                    int relForward,
                                    int relSide,
                                    M11_ViewportCell* outCell) {
    int fx;
    int fy;
    int rx;
    int ry;
    int mapX;
    int mapY;
    unsigned char square = 0;
    unsigned short firstThing = THING_ENDOFLIST;
    M11_ViewportCell cell;

    memset(&cell, 0, sizeof(cell));
    cell.relForward = relForward;
    cell.relSide = relSide;
    cell.elementType = DUNGEON_ELEMENT_WALL;
    cell.firstThing = THING_ENDOFLIST;
    cell.doorState = -1;
    cell.doorType = 0;
    cell.creatureType = -1;
    { int ci; for (ci = 0; ci < M11_MAX_CELL_CREATURES; ++ci) { cell.creatureTypes[ci] = -1; cell.creatureCountsPerGroup[ci] = 0; cell.creatureDirections[ci] = -1; } }
    cell.creatureGroupCount = 0;
    cell.firstItemThingType = -1;
    cell.firstItemSubtype = -1;
    { int fi; for (fi = 0; fi < M11_MAX_CELL_ITEMS; ++fi) { cell.floorItemTypes[fi] = -1; cell.floorItemSubtypes[fi] = -1; cell.floorItemCells[fi] = 0; } }
    cell.floorItemCount = 0;
    cell.wallOrnamentOrdinal = -1;
    cell.doorOrnamentOrdinal = -1;
    cell.firstProjectileGfxIndex = -1;
    cell.firstProjectileSubtype = -1;
    cell.firstProjectileRelDir = -1;
    cell.firstProjectileCell = -1;
    cell.firstProjectileFlipFlags = 0;
    cell.floorOrnamentOrdinal = 0;
    cell.firstExplosionType = -1;
    cell.firstExplosionFrame = -1;
    cell.firstExplosionMaxFrames = 0;
    cell.firstExplosionAttack = -1;

    if (!state || !state->active) {
        if (outCell) {
            *outCell = cell;
        }
        return 0;
    }

    m11_direction_vectors(state->world.party.direction, &fx, &fy, &rx, &ry);
    mapX = state->world.party.mapX + relForward * fx + relSide * rx;
    mapY = state->world.party.mapY + relForward * fy + relSide * ry;
    cell.mapX = mapX;
    cell.mapY = mapY;

    if (!m11_get_square_byte(&state->world,
                             state->world.party.mapIndex,
                             mapX,
                             mapY,
                             &square)) {
        if (outCell) {
            *outCell = cell;
        }
        return 0;
    }

    firstThing = m11_get_first_square_thing(&state->world,
                                            state->world.party.mapIndex,
                                            mapX,
                                            mapY);
    cell.valid = 1;
    cell.square = square;
    cell.elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    cell.thingCount = m11_count_square_things(&state->world,
                                              state->world.party.mapIndex,
                                              mapX,
                                              mapY);
    cell.firstThing = firstThing;
    m11_summarize_square_things(&state->world,
                                state->world.party.mapIndex,
                                mapX,
                                mapY,
                                &cell.summary);

    if (cell.elementType == DUNGEON_ELEMENT_DOOR) {
        cell.doorState = square & 0x07;
        cell.doorVertical = (square & 0x08) != 0;
        if (firstThing != THING_ENDOFLIST && firstThing != THING_NONE &&
            THING_GET_TYPE(firstThing) == THING_TYPE_DOOR &&
            state->world.things && state->world.things->doors) {
            int doorIndex = THING_GET_INDEX(firstThing);
            if (doorIndex >= 0 && doorIndex < state->world.things->doorCount) {
                cell.hasDoorThing = 1;
                cell.doorType = state->world.things->doors[doorIndex].type & 1;
                cell.doorVertical = state->world.things->doors[doorIndex].vertical;
            }
        }
    }

    /* Extract all creature group types on this square (up to 4).
     * DM1 can have multiple creature groups on a single square; the
     * viewport draws them stacked/offset.  We also keep the legacy
     * single creatureType field pointing at the first group. */
    if (cell.summary.groups > 0 && state->world.things && state->world.things->groups) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_GROUP) {
                int gIdx = THING_GET_INDEX(scanThing);
                if (gIdx >= 0 && gIdx < state->world.things->groupCount &&
                    cell.creatureGroupCount < M11_MAX_CELL_CREATURES) {
                    int ct = (int)state->world.things->groups[gIdx].creatureType;
                    if (cell.creatureGroupCount == 0) {
                        cell.creatureType = ct; /* legacy compat */
                    }
                    cell.creatureTypes[cell.creatureGroupCount] = ct;
                    cell.creatureCountsPerGroup[cell.creatureGroupCount] =
                        (int)state->world.things->groups[gIdx].count + 1;
                    cell.creatureDirections[cell.creatureGroupCount] =
                        (int)state->world.things->groups[gIdx].direction;
                    cell.creatureGroupCount++;
                }
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract floor item types and subtypes for sprite rendering.
     * Collect up to M11_MAX_CELL_ITEMS items for multi-item scatter.
     * The first one also populates the legacy single-item fields. */
    if (cell.summary.items > 0 && state->world.things) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE &&
               scanSafety < 64 && cell.floorItemCount < M11_MAX_CELL_ITEMS) {
            int tType = THING_GET_TYPE(scanThing);
            int tIdx = THING_GET_INDEX(scanThing);
            if (m11_thing_is_item(tType)) {
                int itemSubtype = -1;
                switch (tType) {
                    case THING_TYPE_WEAPON:
                        if (state->world.things->weapons && tIdx >= 0 && tIdx < state->world.things->weaponCount)
                            itemSubtype = (int)state->world.things->weapons[tIdx].type;
                        break;
                    case THING_TYPE_ARMOUR:
                        if (state->world.things->armours && tIdx >= 0 && tIdx < state->world.things->armourCount)
                            itemSubtype = (int)state->world.things->armours[tIdx].type;
                        break;
                    case THING_TYPE_POTION:
                        if (state->world.things->potions && tIdx >= 0 && tIdx < state->world.things->potionCount)
                            itemSubtype = (int)state->world.things->potions[tIdx].type;
                        break;
                    case THING_TYPE_JUNK:
                        if (state->world.things->junks && tIdx >= 0 && tIdx < state->world.things->junkCount)
                            itemSubtype = (int)state->world.things->junks[tIdx].type;
                        break;
                    case THING_TYPE_SCROLL:
                        itemSubtype = 0;
                        break;
                    case THING_TYPE_CONTAINER:
                        if (state->world.things->containers && tIdx >= 0 && tIdx < state->world.things->containerCount)
                            itemSubtype = (int)state->world.things->containers[tIdx].type;
                        break;
                    default:
                        itemSubtype = 0;
                        break;
                }
                if (cell.floorItemCount == 0) {
                    cell.firstItemThingType = tType;
                    cell.firstItemSubtype = itemSubtype;
                }
                cell.floorItemTypes[cell.floorItemCount] = tType;
                cell.floorItemSubtypes[cell.floorItemCount] = itemSubtype;
                cell.floorItemCells[cell.floorItemCount] = (((int)(scanThing >> 14)) - state->world.party.direction) & 3;
                cell.floorItemCount++;
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract door ornament ordinal */
    if (cell.elementType == DUNGEON_ELEMENT_DOOR && cell.hasDoorThing &&
        state->world.things && state->world.things->doors) {
        int doorIdx = THING_GET_INDEX(firstThing);
        if (doorIdx >= 0 && doorIdx < state->world.things->doorCount) {
            int ord = (int)state->world.things->doors[doorIdx].ornamentOrdinal;
            if (ord > 0) cell.doorOrnamentOrdinal = ord;
        }
    }

    /* Extract wall ornament ordinal from sensors on this square.
     * In DM, wall ornaments are placed via sensor things whose
     * ornamentOrdinal field specifies the graphic to display. */
    if (state->world.things && state->world.things->sensors) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_SENSOR) {
                int sIdx = THING_GET_INDEX(scanThing);
                if (sIdx >= 0 && sIdx < state->world.things->sensorCount) {
                    int ord = (int)state->world.things->sensors[sIdx].ornamentOrdinal;
                    if (ord > 0) {
                        cell.wallOrnamentOrdinal = ord;
                        break;
                    }
                }
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract first projectile graphic index from GRAPHICS.DAT.
     * Source-backed spell/magic projectiles use M613=454 plus the
     * F0142/G0210 projectile-aspect native offset. */
    if (cell.summary.projectiles > 0 && state->world.things &&
        state->world.things->projectiles) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_PROJECTILE) {
                int pIdx = THING_GET_INDEX(scanThing);
                if (pIdx >= 0 && pIdx < state->world.things->projectileCount) {
                    int slot = (int)state->world.things->projectiles[pIdx].slot;
                    cell.firstProjectileSubtype = slot;
                    cell.firstProjectileGfxIndex = m11_projectile_subtype_to_graphic_index(slot);
                }
                break;
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Runtime-only projectile fallback.  When the summary reports a
     * projectile but the things-linked-list has no matching slot
     * (because the projectile was spawned into world.projectiles via
     * F0810 rather than written into the dungeon-things chain), pick
     * the graphic index from the first runtime projectile at this
     * cell.  Runtime subtypes are mapped through the same projectile
     * aspect table path.  This is what actually lets the player see
     * the projectile both when it first appears and at every tick of
     * its travel. */
    if (cell.summary.projectiles > 0 && cell.firstProjectileGfxIndex < 0) {
        int pi;
        for (pi = 0; pi < state->world.projectiles.count; ++pi) {
            const struct ProjectileInstance_Compat* rp =
                &state->world.projectiles.entries[pi];
            if (rp->slotIndex < 0) continue;
            if (rp->mapIndex != state->world.party.mapIndex) continue;
            if (rp->mapX != mapX || rp->mapY != mapY) continue;
            cell.firstProjectileSubtype = rp->projectileSubtype;
            cell.firstProjectileGfxIndex =
                m11_projectile_subtype_to_graphic_index(rp->projectileSubtype);
            break;
        }
    }

    /* Extract first projectile direction from the runtime projectile list.
     * Match a runtime ProjectileInstance_Compat to this cell's map position
     * and compute the direction relative to party facing.
     * In DM1, projectile sprites are horizontally mirrored when the
     * missile's relative direction is 1 (right) vs 3 (left).  Missiles
     * heading toward (2) or away (0) from the party use the normal sprite.
     * Ref: ReDMCSB VIEWPORT.C projectile rendering direction logic. */
    if (cell.firstProjectileGfxIndex >= 0) {
        int pi;
        int partyMap = state->world.party.mapIndex;
        int partyDir = state->world.party.direction;
        for (pi = 0; pi < state->world.projectiles.count; ++pi) {
            const struct ProjectileInstance_Compat* rp = &state->world.projectiles.entries[pi];
            if (rp->slotIndex < 0) continue;
            if (rp->mapIndex == partyMap && rp->mapX == mapX && rp->mapY == mapY) {
                cell.firstProjectileRelDir = (rp->direction - partyDir) & 3;
                if (cell.firstProjectileSubtype >= 0) {
                    int aspectIndex = m11_projectile_subtype_to_aspect_index(
                        cell.firstProjectileSubtype);
                    cell.firstProjectileGfxIndex = m11_projectile_aspect_to_graphic_index(
                        aspectIndex,
                        cell.firstProjectileRelDir);
                    cell.firstProjectileFlipFlags = m11_projectile_aspect_flip_flags(
                        aspectIndex,
                        cell.firstProjectileRelDir,
                        (rp->cell - partyDir) & 3,
                        mapX,
                        mapY);
                }
                /* Extract sub-cell position (0-3) for DM1-faithful
                 * projectile viewport offset.  The cell field in the
                 * runtime data is absolute (0=NW,1=NE,2=SW,3=SE).
                 * We store the relative sub-cell: rotate by party
                 * facing so 0 = back-left, 1 = back-right,
                 * 2 = front-left, 3 = front-right from the party's
                 * perspective.  This matches DM1's view-cell mapping.
                 * Ref: ReDMCSB DUNVIEW.C L0139_i_Cell = M21_NORMALIZE(
                 *   A0126_i_ViewCell + P142_i_Direction). */
                cell.firstProjectileCell = (rp->cell - partyDir) & 3;
                break;
            }
        }
    }

    /* Extract first explosion type */
    if (cell.summary.explosions > 0 && state->world.things &&
        state->world.things->explosions) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_EXPLOSION) {
                int eIdx = THING_GET_INDEX(scanThing);
                if (eIdx >= 0 && eIdx < state->world.things->explosionCount) {
                    cell.firstExplosionType = (int)state->world.things->explosions[eIdx].type;
                    /* Dungeon-thing explosions do not carry a live frame
                     * counter in M10's compact layout; seed the render
                     * fields so the single-frame burst still draws. */
                    cell.firstExplosionFrame     = 0;
                    cell.firstExplosionMaxFrames = 1;
                    cell.firstExplosionAttack    = -1;
                }
                break;
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Runtime-only explosion fallback.  An explosion spawned via the
     * V1 projectile-impact path (F0821_EXPLOSION_Create_Compat) lives
     * in world.explosions rather than the dungeon thing chain.  Pick
     * the type from the first runtime explosion at this cell so the
     * viewport's type-specific burst visual still lands, and pull
     * currentFrame / maxFrames / attack so the burst renderer can
     * bloom / fade across the classic DM1 multi-frame aftermath. */
    if (cell.summary.explosions > 0 && cell.firstExplosionType < 0) {
        int ei;
        for (ei = 0; ei < state->world.explosions.count; ++ei) {
            const struct ExplosionInstance_Compat* re =
                &state->world.explosions.entries[ei];
            if (re->slotIndex < 0) continue;
            if (re->mapIndex != state->world.party.mapIndex) continue;
            if (re->mapX != mapX || re->mapY != mapY) continue;
            cell.firstExplosionType      = re->explosionType;
            cell.firstExplosionFrame     = re->currentFrame;
            cell.firstExplosionMaxFrames = re->maxFrames > 0 ? re->maxFrames : 1;
            cell.firstExplosionAttack    = re->attack;
            break;
        }
    }

    /* Extract floor ornament ordinal from random generation or sensor things.
     * In DM1, corridor/pit/stairs/teleporter squares can have floor ornaments
     * assigned via the deterministic random algorithm or sensor-placed overrides.
     * Ref: ReDMCSB DUNGEON.C F0172_DUNGEON_SetSquareAspect. */
    if (cell.valid && m11_viewport_cell_is_wall_free(cell.elementType)) {
        cell.floorOrnamentOrdinal = m11_compute_floor_ornament_ordinal(
            state, state->world.party.mapIndex, mapX, mapY, square);
    }

    if (outCell) {
        *outCell = cell;
    }
    return 1;
}

/* Helper: return 1 for element types that can have floor ornaments. */
static int m11_viewport_cell_is_wall_free(int elementType) {
    return elementType == DUNGEON_ELEMENT_CORRIDOR ||
           elementType == DUNGEON_ELEMENT_PIT ||
           elementType == DUNGEON_ELEMENT_STAIRS ||
           elementType == DUNGEON_ELEMENT_TELEPORTER;
}

static int m11_viewport_cell_is_open(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return 0;
    }
    if (cell->elementType == DUNGEON_ELEMENT_WALL ||
        cell->elementType == DUNGEON_ELEMENT_FAKEWALL) {
        return 0;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        return cell->doorState == 0 || cell->doorState == 5;
    }
    return 1;
}

static int m11_viewport_cell_is_wall_like(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return 0;
    }
    return cell->elementType == DUNGEON_ELEMENT_WALL ||
           cell->elementType == DUNGEON_ELEMENT_FAKEWALL;
}

static int m11_build_front_text_readout(const M11_GameViewState* state,
                                        const M11_ViewportCell* cell,
                                        char* outTitle,
                                        size_t outTitleSize,
                                        char* outDetail,
                                        size_t outDetailSize) {
    unsigned short thing;

    if (!state || !cell || !cell->valid || !outTitle || !outDetail) {
        return 0;
    }

    thing = cell->firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE) {
        int thingType = THING_GET_TYPE(thing);
        int thingIndex = THING_GET_INDEX(thing);
        if (thingType == THING_TYPE_TEXTSTRING && state->world.things &&
            state->world.things->textStrings && thingIndex >= 0 &&
            thingIndex < state->world.things->textStringCount) {
            const struct DungeonTextString_Compat* textThing = &state->world.things->textStrings[thingIndex];
            char decoded[96];
            if (state->world.things->textData && state->world.things->textDataWordCount > 0 &&
                F0507_DUNGEON_DecodeTextAtOffset_Compat(state->world.things->textData,
                                                        state->world.things->textDataWordCount,
                                                        textThing->textDataWordOffset,
                                                        decoded,
                                                        sizeof(decoded)) >= 0 &&
                decoded[0] != '\0') {
                snprintf(outTitle, outTitleSize, "TEXT PLAQUE");
                snprintf(outDetail, outDetailSize, "%s", decoded);
                return 1;
            }
            snprintf(outTitle, outTitleSize, "TEXT PLAQUE");
            snprintf(outDetail, outDetailSize, "TEXT OFFSET %u (DECODE UNAVAILABLE)",
                     (unsigned int)textThing->textDataWordOffset);
            return 1;
        }
        thing = m11_raw_next_thing(state->world.things, thing);
    }

    return 0;
}

static int m11_inspect_front_cell(M11_GameViewState* state) {
    M11_ViewportCell frontCell;
    char title[64];
    char detail[128];

    if (!state || !state->active) {
        return 0;
    }

    memset(&frontCell, 0, sizeof(frontCell));
    if (!m11_sample_viewport_cell(state, 1, 0, &frontCell) || !frontCell.valid) {
        m11_set_status(state, "INSPECT", "VOID AHEAD");
        m11_set_inspect_readout(state, "VOID", "TURN TO RE-ENTER THE MAP");
        return 1;
    }

    if (m11_build_front_text_readout(state, &frontCell,
                                     title, sizeof(title),
                                     detail, sizeof(detail))) {
        int mirrorOrdinal = m11_front_cell_mirror_ordinal(state);
        if (mirrorOrdinal >= 0 &&
            M11_GameView_SelectFrontMirrorCandidate(state) == 1) {
            return 1;
        }
        m11_set_status(state, "INSPECT", "READ TEXT");
        m11_set_inspect_readout(state, title, detail);
        /* Show a dialog overlay so the text plaque is prominently
         * displayed; the player dismisses it with any key. */
        M11_GameView_ShowDialogOverlay(state, detail);
        return 1;
    }

    if (frontCell.elementType == DUNGEON_ELEMENT_DOOR) {
        snprintf(title, sizeof(title), "FRONT DOOR");
        snprintf(detail, sizeof(detail), "%s, STATE %d, %s",
                 frontCell.doorVertical ? "VERTICAL" : "HORIZONTAL",
                 frontCell.doorState,
                 m11_viewport_cell_is_open(&frontCell) ? "OPEN ENOUGH TO CROSS" : "STILL CLOSED");
        m11_set_status(state, "INSPECT", "DOOR CHECK");
        m11_set_inspect_readout(state, title, detail);
        return 1;
    }

    if (frontCell.summary.groups > 0) {
        snprintf(title, sizeof(title), "CREATURE CONTACT");
        snprintf(detail, sizeof(detail), "%d GROUP TARGET%s IN FRONT CELL",
                 frontCell.summary.groups,
                 frontCell.summary.groups == 1 ? "" : "S");
        m11_set_status(state, "INSPECT", "ENEMY SPOTTED");
        m11_set_inspect_readout(state, title, detail);
        return 1;
    }

    if (frontCell.summary.items > 0 || frontCell.summary.projectiles > 0 ||
        frontCell.summary.explosions > 0 || frontCell.summary.sensors > 0 ||
        frontCell.summary.teleporters > 0 || frontCell.summary.textStrings > 0 ||
        frontCell.elementType == DUNGEON_ELEMENT_PIT ||
        frontCell.elementType == DUNGEON_ELEMENT_STAIRS ||
        frontCell.elementType == DUNGEON_ELEMENT_TELEPORTER) {
        snprintf(title, sizeof(title), "%s",
                 F0503_DUNGEON_GetElementName_Compat(frontCell.elementType));
        m11_format_square_summary(&frontCell, detail, sizeof(detail));
        m11_set_status(state, "INSPECT", "FRONT CELL READOUT");
        m11_set_inspect_readout(state, title, detail);
        return 1;
    }

    return 0;
}

static int m11_front_cell_mirror_ordinal(const M11_GameViewState* state) {
    M11_ViewportCell frontCell;
    unsigned short thing;

    if (!state || !state->active || !state->mirrorCatalogAvailable ||
        !m11_get_front_cell(state, &frontCell) || !frontCell.valid ||
        !state->world.things || !state->world.things->textStrings) {
        return -1;
    }

    thing = frontCell.firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE) {
        int thingType = THING_GET_TYPE(thing);
        int thingIndex = THING_GET_INDEX(thing);
        if (thingType == THING_TYPE_TEXTSTRING && thingIndex >= 0 &&
            thingIndex < state->world.things->textStringCount) {
            int mirrorOrdinal = F0676_CHAMPION_MirrorCatalogGetOrdinalForTextStringIndex_Compat(
                &state->mirrorCatalog, thingIndex);
            if (mirrorOrdinal >= 0) {
                return mirrorOrdinal;
            }
        }
        thing = m11_raw_next_thing(state->world.things, thing);
    }
    return -1;
}

static int m11_get_front_cell(const M11_GameViewState* state, M11_ViewportCell* outCell) {
    M11_ViewportCell frontCell;

    memset(&frontCell, 0, sizeof(frontCell));
    if (!state || !state->active || !m11_sample_viewport_cell(state, 1, 0, &frontCell) || !frontCell.valid) {
        if (outCell) {
            *outCell = frontCell;
        }
        return 0;
    }
    if (outCell) {
        *outCell = frontCell;
    }
    return 1;
}

static int m11_front_cell_has_attack_target(const M11_GameViewState* state) {
    M11_ViewportCell frontCell;

    if (!m11_get_front_cell(state, &frontCell)) {
        return 0;
    }
    return frontCell.summary.groups > 0;
}

static int m11_front_cell_is_door(const M11_GameViewState* state) {
    M11_ViewportCell frontCell;

    if (!m11_get_front_cell(state, &frontCell)) {
        return 0;
    }
    return frontCell.valid && frontCell.elementType == DUNGEON_ELEMENT_DOOR;
}

/*
 * Thin shim over F0715_DOOR_ResolveToggleAction_Compat +
 * F0713_DOOR_BuildAnimationEvent_Compat: compat decides whether to open
 * or close, the viewport schedules a TIMELINE_EVENT_DOOR_ANIMATE event
 * on the world timeline and then runs the tick orchestrator.  The
 * animating intermediate states (1..3) are owned by the compat layer
 * via F0712_DOOR_StepAnimation_Compat inside F0887 (Pass 38), not by
 * this M11 shim.
 */
static int m11_toggle_front_door(M11_GameViewState* state) {
    M11_ViewportCell frontCell;
    struct DoorToggleResult_Compat action;
    int doorEffect;
    int currentState = -1;
    int finalState;

    if (!state || !state->active || !m11_get_front_cell(state, &frontCell) ||
        !frontCell.valid || frontCell.elementType != DUNGEON_ELEMENT_DOOR) {
        return 0;
    }

    if (!F0715_DOOR_ResolveToggleAction_Compat(state->world.dungeon,
                                               state->world.party.mapIndex,
                                               frontCell.mapX,
                                               frontCell.mapY,
                                               &action)) {
        return 0;
    }

    if (action.kind == DOOR_ACTION_DESTROYED) {
        m11_set_status(state, "DOOR", "DOOR DESTROYED");
        m11_set_inspect_readout(state, "FRONT DOOR", "DESTROYED, NO LONGER BLOCKING THE PASSAGE");
        return 1;
    }

    if (action.kind != DOOR_ACTION_OPEN && action.kind != DOOR_ACTION_CLOSE) {
        return 0;
    }

    /*
     * Convert OPEN/CLOSE into a DOOR_EFFECT_SET/CLEAR and schedule an
     * animation event.  The action.newDoorState (0 or 4) carried over
     * from Pass 31 is still the final target; it is no longer applied
     * directly.
     */
    finalState = action.newDoorState;
    if (!F0714_DOOR_ResolveAnimationEffect_Compat(
            state->world.dungeon,
            state->world.party.mapIndex,
            frontCell.mapX,
            frontCell.mapY,
            DOOR_EFFECT_TOGGLE,
            &doorEffect,
            &currentState)) {
        /* Already at target (e.g. state==0 on OPEN / state==4 on CLOSE) —
         * nothing to animate; still surface the status line so the UI
         * reflects the user's intent. */
        if (finalState == 0) {
            m11_set_status(state, "DOOR", "DOOR OPENED");
            m11_set_inspect_readout(state, "FRONT DOOR", "OPEN, PASSAGE CLEAR, CLICK CENTER OR PRESS UP TO CROSS");
        } else {
            m11_set_status(state, "DOOR", "DOOR CLOSED");
            m11_set_inspect_readout(state, "FRONT DOOR", "SHUT, ENTER INSPECTS, SPACE TOGGLES AGAIN");
        }
        return 1;
    }

    {
        struct TimelineEvent_Compat animEvent;
        if (F0713_DOOR_BuildAnimationEvent_Compat(
                state->world.party.mapIndex,
                frontCell.mapX,
                frontCell.mapY,
                doorEffect,
                state->world.gameTick,
                &animEvent)) {
            /* fireAtTick = gameTick + 1 — dispatches on the tick below. */
            (void)F0721_TIMELINE_Schedule_Compat(&state->world.timeline, &animEvent);
        }
    }

    /*
     * Advance the world by one tick through the orchestrator so the
     * scheduled TIMELINE_EVENT_DOOR_ANIMATE fires.  On each such tick
     * F0887_ORCH_DispatchTimelineEvents_Compat calls the Pass-38 step
     * handler, which walks the door state by one (4 -> 3, 3 -> 2, …)
     * and reschedules the event for the next tick until the final
     * state is reached.
     */
    {
        struct TickInput_Compat input;
        memset(&input, 0, sizeof(input));
        input.tick    = state->world.gameTick;
        input.command = CMD_NONE;
        memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
        (void)F0884_ORCH_AdvanceOneTick_Compat(
            &state->world, &input, &state->lastTickResult);
    }
    state->lastWorldHash = state->lastTickResult.worldHashPost;
    M11_GameView_ProcessTickEmissions(state);
    m11_refresh_hash(state);
    state->lastTickResult.worldHashPost = state->lastWorldHash;

    /*
     * Status / inspect text reflects the player's *intent*, not the
     * current in-flight state; the actual pixel-level animation is
     * expressed by the EMIT_DOOR_STATE stream consumed by
     * M11_GameView_ProcessTickEmissions.  This matches the original:
     * TIMELINE.C does not emit text when it rattles a closing door.
     */
    if (finalState == 0) {
        m11_set_status(state, "DOOR", "DOOR OPENING");
        m11_set_inspect_readout(state, "FRONT DOOR", "OPENING, PASSAGE CLEARING, CLICK CENTER OR PRESS UP TO CROSS");
    } else {
        m11_set_status(state, "DOOR", "DOOR CLOSING");
        m11_set_inspect_readout(state, "FRONT DOOR", "CLOSING, WAIT FOR RATTLE, SPACE TOGGLES AGAIN");
    }
    return 1;
}

static M12_MenuInput m11_pointer_viewport_input(const M11_GameViewState* state,
                                                int x,
                                                int y) {
    M11_ViewportCell frontCell;
    int localX;
    int localY;

    if (!state || !state->active) {
        return M12_MENU_INPUT_NONE;
    }

    localX = x - M11_VIEWPORT_X;
    localY = y - M11_VIEWPORT_Y;

    if (localX < M11_VIEWPORT_W / 3) {
        if (localY >= (M11_VIEWPORT_H * 2) / 3) {
            return M12_MENU_INPUT_STRAFE_LEFT;
        }
        return M12_MENU_INPUT_LEFT;
    }
    if (localX >= (M11_VIEWPORT_W * 2) / 3) {
        if (localY >= (M11_VIEWPORT_H * 2) / 3) {
            return M12_MENU_INPUT_STRAFE_RIGHT;
        }
        return M12_MENU_INPUT_RIGHT;
    }
    if (!m11_get_front_cell(state, &frontCell)) {
        return M12_MENU_INPUT_ACCEPT;
    }
    if (localY >= (M11_VIEWPORT_H * 2) / 3) {
        if (frontCell.summary.groups > 0) {
            return M12_MENU_INPUT_ACTION;
        }
        if (m11_viewport_cell_is_open(&frontCell) &&
            frontCell.summary.items == 0 &&
            frontCell.summary.sensors == 0 &&
            frontCell.summary.textStrings == 0 &&
            frontCell.summary.projectiles == 0 &&
            frontCell.summary.explosions == 0 &&
            frontCell.elementType != DUNGEON_ELEMENT_PIT) {
            return M12_MENU_INPUT_UP;
        }
    }
    return M12_MENU_INPUT_ACCEPT;
}

static unsigned char m11_viewport_fill_color(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return M11_COLOR_DARK_GRAY;
    }
    switch (cell->elementType) {
        case DUNGEON_ELEMENT_WALL:
            return M11_COLOR_DARK_GRAY;
        case DUNGEON_ELEMENT_CORRIDOR:
            return M11_COLOR_BLACK;
        case DUNGEON_ELEMENT_PIT:
            return M11_COLOR_RED;
        case DUNGEON_ELEMENT_STAIRS:
            return M11_COLOR_GREEN;
        case DUNGEON_ELEMENT_DOOR:
            return M11_COLOR_BROWN;
        case DUNGEON_ELEMENT_TELEPORTER:
            return M11_COLOR_MAGENTA;
        case DUNGEON_ELEMENT_FAKEWALL:
            return M11_COLOR_LIGHT_GRAY;
        default:
            return M11_COLOR_BLACK;
    }
}

static unsigned char m11_feature_accent_color(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return M11_COLOR_DARK_GRAY;
    }
    switch (cell->elementType) {
        case DUNGEON_ELEMENT_DOOR: return M11_COLOR_LIGHT_RED;
        case DUNGEON_ELEMENT_STAIRS: return M11_COLOR_YELLOW;
        case DUNGEON_ELEMENT_PIT: return M11_COLOR_BROWN;
        case DUNGEON_ELEMENT_TELEPORTER: return M11_COLOR_LIGHT_CYAN;
        case DUNGEON_ELEMENT_FAKEWALL: return M11_COLOR_MAGENTA;
        case DUNGEON_ELEMENT_CORRIDOR: return M11_COLOR_LIGHT_GRAY;
        default: return M11_COLOR_WHITE;
    }
}

static void m11_draw_wall_contents(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const M11_ViewRect* rect,
                                   const M11_ViewportCell* cell,
                                   int depthIndex);

/* NOTE: g_drawState forward-declared near file top. */

/* Forward declarations for asset-backed rendering helpers */
static int m11_draw_door_frame_asset(const M11_GameViewState* state,
                                     unsigned char* framebuffer,
                                     int fbW, int fbH,
                                     const M11_ViewRect* rect,
                                     int depthIndex);
static int m11_draw_door_side_asset(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW, int fbH,
                                    int x, int y, int w, int h,
                                    int depthIndex);
static int m11_draw_stairs_asset(const M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 int fbW, int fbH,
                                 const M11_ViewRect* rect,
                                 int depthIndex, int stairUp);
static int m11_draw_creature_sprite(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW, int fbH,
                                    int x, int y, int w, int h,
                                    int creatureType, int depthIndex,
                                    int creatureDir);
static int m11_draw_item_sprite(const M11_GameViewState* state,
                                unsigned char* framebuffer,
                                int fbW, int fbH,
                                int x, int y, int w, int h,
                                int thingType, int subtype,
                                int relativeCell, int pileIndex,
                                int depthIndex);
static int m11_draw_wall_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW, int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex);
static int m11_draw_door_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW, int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex);

static void m11_draw_wall_face(unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight,
                               const M11_ViewRect* rect,
                               const M11_ViewportCell* cell,
                               int depthIndex) {
    int inset = 6 + depthIndex * 4;
    int faceX = rect->x + inset;
    int faceY = rect->y + inset / 2;
    int faceW = rect->w - inset * 2;
    int faceH = rect->h - inset;
    unsigned char accent = m11_feature_accent_color(cell);

    if (faceW < 8 || faceH < 8) {
        return;
    }

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  faceX, faceY, faceW, faceH,
                  cell->elementType == DUNGEON_ELEMENT_WALL ? M11_COLOR_DARK_GRAY : m11_viewport_fill_color(cell));
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  faceX, faceY, faceW, faceH, accent);

    switch (cell->elementType) {
        case DUNGEON_ELEMENT_DOOR:
            /* Try real door frame graphics first */
            if (g_drawState &&
                m11_draw_door_frame_asset(g_drawState, framebuffer,
                                          framebufferWidth, framebufferHeight,
                                          rect, depthIndex)) {
                break; /* Real asset drawn successfully */
            }
            /* Fallback: primitive door lines */
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + faceW / 2, faceY + 3, faceY + faceH - 4, M11_COLOR_YELLOW);
            if (cell->doorState > 0 && cell->doorState < 5) {
                int shutHeight = (faceH - 8) * cell->doorState / 5;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              faceX + 3, faceY + faceH - 4 - shutHeight,
                              faceW - 6, shutHeight, M11_COLOR_BLACK);
            }
            break;
        case DUNGEON_ELEMENT_STAIRS:
            /* Try real stair graphics first */
            if (g_drawState) {
                int stairUp = (cell->square & 0x01);
                if (m11_draw_stairs_asset(g_drawState, framebuffer,
                                          framebufferWidth, framebufferHeight,
                                          rect, depthIndex, stairUp)) {
                    break;
                }
            }
            /* Fallback: primitive stair lines */
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + 4, faceX + faceW - 5, faceY + faceH - 5, M11_COLOR_YELLOW);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + 8, faceX + faceW - 9, faceY + faceH - 10, M11_COLOR_YELLOW);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + 12, faceX + faceW - 13, faceY + faceH - 15, M11_COLOR_YELLOW);
            break;
        case DUNGEON_ELEMENT_PIT:
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + faceH / 2,
                          faceW - 10, faceH / 3, M11_COLOR_BLACK);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + faceH / 2,
                          faceW - 10, faceH / 3, M11_COLOR_BROWN);
            break;
        case DUNGEON_ELEMENT_TELEPORTER:
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + 4, faceW - 10, faceH - 8, M11_COLOR_LIGHT_CYAN);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 10, faceY + 8, faceW - 20, faceH - 16, M11_COLOR_WHITE);
            break;
        case DUNGEON_ELEMENT_FAKEWALL:
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + 4, faceW - 10, faceH - 8, M11_COLOR_MAGENTA);
            break;
        case DUNGEON_ELEMENT_CORRIDOR:
        default:
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 4, faceY + 4, faceW - 8, faceH - 8, M11_COLOR_DARK_GRAY);
            break;
    }

    /* Draw wall ornament on wall-type cells (sensor-placed ornaments) */
    if (cell->elementType == DUNGEON_ELEMENT_WALL && cell->wallOrnamentOrdinal >= 0) {
        M11_ViewRect ornRect;
        ornRect.x = faceX;
        ornRect.y = faceY;
        ornRect.w = faceW;
        ornRect.h = faceH;
        if (g_drawState) {
            m11_draw_wall_ornament(g_drawState, framebuffer,
                                   framebufferWidth, framebufferHeight,
                                   &ornRect, cell->wallOrnamentOrdinal, depthIndex);
        }
    }

    /* Draw door ornament on door-type cells */
    if (cell->elementType == DUNGEON_ELEMENT_DOOR && cell->doorOrnamentOrdinal >= 0) {
        M11_ViewRect ornRect;
        ornRect.x = faceX;
        ornRect.y = faceY;
        ornRect.w = faceW;
        ornRect.h = faceH;
        if (g_drawState) {
            m11_draw_door_ornament(g_drawState, framebuffer,
                                   framebufferWidth, framebufferHeight,
                                   &ornRect, cell->doorOrnamentOrdinal, depthIndex);
        }
    }

    if (m11_viewport_cell_is_open(cell)) {
        m11_draw_wall_contents(framebuffer, framebufferWidth, framebufferHeight,
                               rect, cell, depthIndex);
    } else if (cell->thingCount > 0) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      faceX + faceW / 2 - 1, faceY + faceH / 2 - 1,
                      3, 3, M11_COLOR_WHITE);
    }
}

static void m11_draw_wall_contents(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const M11_ViewRect* rect,
                                   const M11_ViewportCell* cell,
                                   int depthIndex) {
    int inset = 6 + depthIndex * 4;
    int faceX = rect->x + inset;
    int faceY = rect->y + inset / 2;
    int faceW = rect->w - inset * 2;
    int faceH = rect->h - inset;

    if (!cell || !m11_viewport_cell_is_open(cell) || faceW < 8 || faceH < 8) {
        return;
    }

    /* ── DM1-faithful Z-order: floor ornaments → floor items → creatures → projectiles ──
     * In the original, floor ornaments are drawn first (painted on the
     * floor texture), then floor items (lying on the ground), then
     * creatures (standing above the items), then projectiles and
     * explosions (in flight, topmost layer).
     * Ref: ReDMCSB DUNVIEW.C F115 — per-cell draw loop. */

    /* Layer 0: Floor ornaments (painted on the floor, below everything) */
    if (cell->floorOrnamentOrdinal > 0 && g_drawState) {
        M11_ViewRect floorRect;
        floorRect.x = faceX;
        floorRect.y = faceY;
        floorRect.w = faceW;
        floorRect.h = faceH;
        m11_draw_floor_ornament(g_drawState, framebuffer,
                                framebufferWidth, framebufferHeight,
                                &floorRect, cell->floorOrnamentOrdinal,
                                depthIndex, 0 /* center cell */);
    }

    /* Layer 1: Floor items (lowest physical objects — on the ground) */
    if (cell->floorItemCount > 0) {
        int ii;
        int itemsToShow = cell->floorItemCount;
        for (ii = 0; ii < itemsToShow; ++ii) {
            if (cell->floorItemTypes[ii] < 0) continue;
            if (!g_drawState ||
                !m11_draw_item_sprite(g_drawState, framebuffer,
                                      framebufferWidth, framebufferHeight,
                                      faceX + 2, faceY + 2, faceW - 4, faceH - 4,
                                      cell->floorItemTypes[ii],
                                      cell->floorItemSubtypes[ii],
                                      cell->floorItemCells[ii], ii,
                                      depthIndex)) {
                /* Fallback cue only for the first item to avoid clutter */
                if (ii == 0) {
                    m11_draw_item_cue(framebuffer, framebufferWidth, framebufferHeight,
                                      faceX + 2, faceY + 2, faceW - 4, faceH - 4,
                                      cell->summary.items);
                }
            }
        }
    }

    /* Layer 2: Creatures (standing on the floor, above items) */
    if (cell->creatureGroupCount > 0) {
        int gi;
        int groupCount = cell->creatureGroupCount;
        int slotW = (groupCount > 1) ? (faceW - 8) * 2 / (groupCount + 1) : faceW - 8;
        int slotH = faceH - 10;
        int stepX = (groupCount > 1) ? ((faceW - 8) - slotW) / (groupCount - 1) : 0;
        for (gi = 0; gi < groupCount; ++gi) {
            int cx = faceX + 4 + gi * stepX;
            int cy = faceY + 5;
            int countInGroup = cell->creatureCountsPerGroup[gi];
            int visibleDups, di;
            if (cell->creatureTypes[gi] < 0) continue;
            visibleDups = countInGroup;
            if (visibleDups > 4) visibleDups = 4;
            if (visibleDups < 1) visibleDups = 1;
            if (visibleDups == 1) {
                int coordSet = m11_creature_coordinate_set(cell->creatureTypes[gi]);
                int zoneX = 0;
                int zoneY = 0;
                if (m11_c3200_creature_zone_point(coordSet,
                                                  depthIndex < 3 ? depthIndex : 2,
                                                  1, 0, &zoneX, &zoneY)) {
                    cx = M11_VIEWPORT_X + zoneX - slotW / 2;
                    cy = M11_VIEWPORT_Y + zoneY - slotH;
                    if (cx < faceX) cx = faceX;
                    if (cy < faceY) cy = faceY;
                    if (cx + slotW > faceX + faceW) cx = faceX + faceW - slotW;
                    if (cy + slotH > faceY + faceH) cy = faceY + faceH - slotH;
                }
                if (!g_drawState ||
                    !m11_draw_creature_sprite(g_drawState, framebuffer,
                                              framebufferWidth, framebufferHeight,
                                              cx, cy, slotW, slotH,
                                              cell->creatureTypes[gi], depthIndex,
                                              cell->creatureDirections[gi])) {
                    m11_draw_creature_cue(framebuffer, framebufferWidth, framebufferHeight,
                                          cx, cy, slotW, slotH, depthIndex);
                }
            } else {
                int dupOffX[4], dupOffY[4];
                int dupW = slotW * 3 / 4;
                int dupH = slotH * 3 / 4;
                int ofsX = (slotW - dupW) / 2;
                int ofsY = (slotH - dupH) / 2;
                if (ofsX < 1) ofsX = 1;
                if (ofsY < 1) ofsY = 1;
                /* DM1 creature front-cell positioning from layout-696
                 * C3200 coordinates. The stored points are center X and
                 * bottom Y in the 224x136 viewport, so convert them into the
                 * local face rectangle and then anchor the duplicate sprite
                 * by its bottom center. */
                {
                    int coordSet = m11_creature_coordinate_set(cell->creatureTypes[gi]);
                    int dIdx = depthIndex < 3 ? depthIndex : 2;
                    if (coordSet >= 0 && coordSet <= 2) {
                        for (di = 0; di < visibleDups && di < 4; ++di) {
                            int origCenterX;
                            int origBottomY;
                            int localCenterX;
                            int localBottomY;
                            (void)m11_c3200_creature_zone_point(coordSet, dIdx,
                                                                 visibleDups, di,
                                                                 &origCenterX,
                                                                 &origBottomY);
                            localCenterX = (origCenterX * faceW) / 224;
                            localBottomY = (origBottomY * faceH) / 136;
                            dupOffX[di] = localCenterX - dupW / 2;
                            dupOffY[di] = localBottomY - dupH;
                            if (dupOffX[di] < 0) dupOffX[di] = 0;
                            if (dupOffY[di] < 0) dupOffY[di] = 0;
                            if (dupOffX[di] + dupW > slotW + ofsX * 2)
                                dupOffX[di] = slotW + ofsX * 2 - dupW;
                            if (dupOffY[di] + dupH > slotH + ofsY * 2)
                                dupOffY[di] = slotH + ofsY * 2 - dupH;
                        }
                    } else {
                        /* Fallback: grid layout */
                        dupOffX[0] = 0;        dupOffY[0] = 0;
                        dupOffX[1] = ofsX * 2; dupOffY[1] = 0;
                        dupOffX[2] = 0;        dupOffY[2] = ofsY * 2;
                        dupOffX[3] = ofsX * 2; dupOffY[3] = ofsY * 2;
                    }
                }
                for (di = 0; di < visibleDups; ++di) {
                    int dx = cx + dupOffX[di];
                    int dy = cy + dupOffY[di];
                    if (!g_drawState ||
                        !m11_draw_creature_sprite(g_drawState, framebuffer,
                                                  framebufferWidth, framebufferHeight,
                                                  dx, dy, dupW, dupH,
                                                  cell->creatureTypes[gi], depthIndex,
                                                  cell->creatureDirections[gi])) {
                        m11_draw_creature_cue(framebuffer, framebufferWidth, framebufferHeight,
                                              dx, dy, dupW, dupH, depthIndex);
                    }
                }
            }
            if (countInGroup > 1 && slotW >= 12 && slotH >= 12) {
                char countStr[4];
                int badgeX = cx + slotW - 8;
                int badgeY = cy + slotH - 8;
                snprintf(countStr, sizeof(countStr), "%d", countInGroup);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              badgeX - 1, badgeY - 1, 9, 9, M11_COLOR_BLACK);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              badgeX, badgeY, countStr, &g_text_small);
            }
        }
    }

    /* Layer 3: Projectiles and explosions (in flight, topmost) */
    m11_draw_effect_cue(framebuffer, framebufferWidth, framebufferHeight,
                        faceX + 3, faceY + 3, faceW - 6, faceH - 6, cell,
                        depthIndex);
}

/* Known GRAPHICS.DAT indices for rendering assets in CSB PC 3.4. */
enum {
    /* Wall texture sets (256x32) */
    M11_GFX_WALL_SET_0 = 42,
    M11_GFX_WALL_SET_1 = 43,
    M11_GFX_WALL_SET_2 = 44,
    M11_GFX_WALL_SET_3 = 45,

    /* Floor tiles (32x32) */
    M11_GFX_FLOOR_SET_0 = 76,
    M11_GFX_FLOOR_SET_1 = 77,

    /* Legacy Firestaff misnomer: graphic 0 is not a DM1 dungeon
     * viewport background; it is a UI/panel graphic.  Normal V1 must
     * build the viewport from the original floor/ceiling strips and
     * DRAWVIEW wall/object overlays instead. */
    M11_GFX_VIEWPORT_BG = 0,
    M11_GFX_VIEWPORT_BG_ALT = 17,

    /* DM1 floor/ceiling panels.
     * ReDMCSB I34E DEFS.H:
     *   M650_GRAPHIC_FLOOR_SET_0_FLOOR   = 78 (224x97)
     *   M651_GRAPHIC_FLOOR_SET_0_CEILING = 79 (224x39)
     * DUNVIEW.C F0128 draws ceiling into C700 and floor into C701. */
    M11_GFX_FLOOR_PANEL   = 78,  /* 224x97 */
    M11_GFX_CEILING_PANEL = 79,  /* 224x39 */

    /* Door frame graphics at depth (perspective-scaled sizes) */
    M11_GFX_DOOR_FRAME_D3   = 70,  /*  36x49  — far depth */
    M11_GFX_DOOR_FRAME_D3W  = 71,  /*  83x49  — far depth wide */
    M11_GFX_DOOR_PILLAR_D3  = 72,  /*   8x52  — thin pillar */
    M11_GFX_DOOR_FRAME_D2   = 73,  /*  78x74  — mid depth */
    M11_GFX_DOOR_FRAME_D1   = 74,  /*  60x111 — near depth */
    M11_GFX_DOOR_FRAME_D0   = 75,  /*  33x136 — nearest side strip */
    M11_GFX_DOOR_SIDE_D0    = 86,  /*  32x123 */
    M11_GFX_DOOR_SIDE_D1    = 87,  /*  25x94  */
    M11_GFX_DOOR_SIDE_D2    = 88,  /*  18x65  */
    M11_GFX_DOOR_SIDE_D3    = 89,  /*  10x42  */
    M11_GFX_DOOR_FRAME_TOP_D1 = 91, /* 102x4 */
    M11_GFX_DOOR_FRAME_TOP_D2 = 92, /* 70x3 */
    M11_GFX_DOOR_SET0_D3    = 246, /* 44x38 */
    M11_GFX_DOOR_SET0_D2    = 247, /* 64x61 */
    M11_GFX_DOOR_SET0_D1    = 248, /* 96x88 */
    M11_GFX_DOOR_BUTTON_BASE = 453, /* 8x9 */
    M11_GFX_DOOR_MASK_DESTROYED = 439, /* M649_GRAPHIC_DOOR_MASK_DESTROYED */

    /* DM1 floor pit graphics. */
    M11_GFX_FLOOR_PIT_D3L2 = 49,
    M11_GFX_FLOOR_PIT_D3L  = 50,
    M11_GFX_FLOOR_PIT_D3C  = 51,
    M11_GFX_FLOOR_PIT_D2L  = 52,
    M11_GFX_FLOOR_PIT_D2C  = 53,
    M11_GFX_FLOOR_PIT_D1L  = 54,
    M11_GFX_FLOOR_PIT_D1C  = 55,
    M11_GFX_FLOOR_PIT_D0L  = 56,
    M11_GFX_FLOOR_PIT_D0C  = 57,
    M11_GFX_FLOOR_PIT_INVISIBLE_D2L = 58,
    M11_GFX_FLOOR_PIT_INVISIBLE_D2C = 59,
    M11_GFX_FLOOR_PIT_INVISIBLE_D1L = 60,
    M11_GFX_FLOOR_PIT_INVISIBLE_D1C = 61,
    M11_GFX_FLOOR_PIT_INVISIBLE_D0L = 62,
    M11_GFX_FLOOR_PIT_INVISIBLE_D0C = 63,

    /* DM1 wall-set 0 stairs graphics.  Source: M645_GRAPHIC_FIRST_STAIRS=108. */
    M11_GFX_DM1_STAIRS_UP_FRONT_D3L    = 108,
    M11_GFX_DM1_STAIRS_UP_FRONT_D3C    = 109,
    M11_GFX_DM1_STAIRS_UP_FRONT_D2L    = 110,
    M11_GFX_DM1_STAIRS_UP_FRONT_D2C    = 111,
    M11_GFX_DM1_STAIRS_UP_FRONT_D1L    = 112,
    M11_GFX_DM1_STAIRS_UP_FRONT_D1C    = 113,
    M11_GFX_DM1_STAIRS_UP_FRONT_D0C_L  = 114,
    M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L  = 115,
    M11_GFX_DM1_STAIRS_DOWN_FRONT_D3C  = 116,
    M11_GFX_DM1_STAIRS_DOWN_FRONT_D2L  = 117,
    M11_GFX_DM1_STAIRS_DOWN_FRONT_D2C  = 118,
    M11_GFX_DM1_STAIRS_DOWN_FRONT_D1L  = 119,
    M11_GFX_DM1_STAIRS_DOWN_FRONT_D1C  = 120,
    M11_GFX_DM1_STAIRS_DOWN_FRONT_D0C_L= 121,
    M11_GFX_DM1_STAIRS_SIDE_D2L        = 122,
    M11_GFX_DM1_STAIRS_UP_SIDE_D1L     = 123,
    M11_GFX_DM1_STAIRS_DOWN_SIDE_D1L   = 124,
    M11_GFX_DM1_STAIRS_SIDE_D0L        = 125,

    /* DM1 field/teleporter graphics.  Source: DEFS.H C070..C077. */
    M11_GFX_DM1_FIELD_MASK_BASE        = 70,
    M11_GFX_DM1_FIELD_TELEPORTER       = 76,
    M11_GFX_DM1_FIELD_FLUXCAGE         = 77,

    /* DM1 wall-set 0 wall graphics.  These are the source GRAPHICS.DAT
     * wall panels that DUNVIEW.C draws into layout-696 zones C702..C717. */
    M11_GFX_WALLSET0_D0R    = 93,  /* 33x136 -> C717_ZONE_WALL_D0R */
    M11_GFX_WALLSET0_D0L    = 94,  /* 33x136 -> C716_ZONE_WALL_D0L */
    M11_GFX_WALLSET0_D1R    = 95,  /* 60x111 -> C714_ZONE_WALL_D1R */
    M11_GFX_WALLSET0_D1L    = 96,  /* 60x111 -> C713_ZONE_WALL_D1L */
    M11_GFX_WALLSET0_D3C    = 107, /* 70x49  -> C704_ZONE_WALL_D3C */
    M11_GFX_WALLSET0_D2C    = 102, /* 106x74 -> C709_ZONE_WALL_D2C */
    M11_GFX_WALLSET0_D1C    = 97,  /* 160x111 -> C712_ZONE_WALL_D1C */
    M11_GFX_WALLSET0_D2R2   = 98,  /* 8x52   -> C708_ZONE_WALL_D2R2 */
    M11_GFX_WALLSET0_D2L2   = 99,  /* 8x52   -> C707_ZONE_WALL_D2L2 */
    M11_GFX_WALLSET0_D2R    = 100, /* 78x74  -> C711_ZONE_WALL_D2R */
    M11_GFX_WALLSET0_D2L    = 101, /* 78x74  -> C710_ZONE_WALL_D2L */
    M11_GFX_WALLSET0_D3R2   = 103, /* 44x49  -> C703_ZONE_WALL_D3R2 */
    M11_GFX_WALLSET0_D3L2   = 104, /* 44x49  -> C702_ZONE_WALL_D3L2 */
    M11_GFX_WALLSET0_D3R    = 105, /* 83x49  -> C706_ZONE_WALL_D3R */
    M11_GFX_WALLSET0_D3L    = 106, /* 83x49  -> C705_ZONE_WALL_D3L */

    /* Creature sprites.  DM1/PC 3.4 uses
     * M618_GRAPHIC_FIRST_CREATURE=584 plus G0219 first-native offsets. */
    M11_GFX_CREATURE_BASE   = 584,
    M11_GFX_CREATURE_SETS_A = 27,
    M11_GFX_CREATURE_BASE_B = 584,
    M11_GFX_CREATURE_SETS_B = 0,

    /* Item viewport sprites (small icons shown in corridor).
     * In CSB/DM, object list entries map thing types to graphic indices.
     * The mapping uses a base index per thing type; subtypes offset from
     * there.  Graphic indices 267+ hold the floor item icons. */
    M11_GFX_ITEM_SPRITE_BASE = 498, /* M612_GRAPHIC_FIRST_OBJECT */
    M11_GFX_ITEM_WEAPON_BASE = 498,
    M11_GFX_ITEM_ARMOUR_BASE = 498,
    M11_GFX_ITEM_SCROLL_BASE = 500,
    M11_GFX_ITEM_POTION_BASE = 501,
    M11_GFX_ITEM_CONTAINER_BASE = 498,
    M11_GFX_ITEM_JUNK_BASE   = 498,
    M11_GFX_ITEM_SPRITE_END  = 584, /* before M618 creature family */

    /* Projectile viewport sprites (M613_GRAPHIC_FIRST_PROJECTILE=454).
     * In DM1 these are the small flying-object graphics drawn in the
     * corridor when a missile is in flight.  Ref: ReDMCSB DEFS.H
     * M613 and DUNVIEW.C G0210_as_Graphic558_ProjectileAspects. */
    M11_GFX_PROJECTILE_BASE = 454,
    M11_GFX_PROJECTILE_COUNT = 32,
    M11_GFX_PROJECTILE_END  = 486,

    /* Wall ornament graphics.  DM1/PC 3.4 uses
     * M615_GRAPHIC_FIRST_WALL_ORNAMENT=259; each global wall ornament
     * has two native graphics. */
    M11_GFX_WALL_ORNAMENT_BASE = 259,
    M11_GFX_WALL_ORNAMENTS_PER_SET = 2,
    M11_GFX_DOOR_ORNAMENT_BASE = 441, /* M617_GRAPHIC_FIRST_DOOR_ORNAMENT */
    M11_GFX_DOOR_ORNAMENTS_PER_SET = 16,

    /* Floor ornament graphics.
     * In DM1/PC 3.4, regular map floor ornaments start at graphic
     * index 385 and each
     * ornament has 6 perspective variants for the 6 visible floor
     * positions (D3L, D3C, D3R, D2L, D2C, D2R).
     * Graphic index = 385 + globalOrnamentIndex * 6 + G0191 increment.
     * Ref: ReDMCSB DEFS.H M616_GRAPHIC_FIRST_FLOOR_ORNAMENT.
     * Footprints are the special pre-base ornament at 379..384. */
    M11_GFX_FLOOR_ORNAMENT_FOOTPRINTS_BASE = 379,
    M11_FLOOR_ORNAMENT_FOOTPRINTS_INDEX = 15,
    M11_GFX_FLOOR_ORNAMENT_BASE = 385,
    M11_GFX_FLOOR_ORNAMENT_VARIANTS = 6,

    /* Stair graphics */
    M11_GFX_STAIRS_DOWN_D2 = 93,  /* 33x136 */
    M11_GFX_STAIRS_DOWN_D1 = 95,  /* 60x111 */
    M11_GFX_STAIRS_UP_D2   = 94,  /* 33x136 */
    M11_GFX_STAIRS_UP_D1   = 96,  /* 60x111 */

    /* Champion icon strip (graphic 28 in original CSB/DM).
     * Horizontal strip of 4×4 = 16 directional icons, each 19×14.
     * Icon index = ((championCell - partyDirection) & 3) * 4 + frame.
     * Used for champion visual in the inventory portrait and HUD. */
    M11_GFX_CHAMPION_ICONS = 28,
    M11_CHAMPION_ICON_W    = 19,
    M11_CHAMPION_ICON_H    = 14,

    /* Spell area background (graphic 9 in original CSB/DM).
     * Drawn as the grid backdrop behind the 6 rune symbol buttons.
     * 87×25 in GRAPHICS.DAT.
     * Ref: DEFS.H line 2216 C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND.
     * Graphic 11 (C011_GRAPHIC_MENU_SPELL_AREA_LINES, 14×39) holds
     * the rune symbol line overlays drawn on top of this background. */
    M11_GFX_SPELL_AREA_BG = 9,
    M11_GFX_SPELL_AREA_LINES = 11,

    /* Action area background (graphic 10 in original CSB/DM).
     * 87×45 in GRAPHICS.DAT. Ref: C010_GRAPHIC_MENU_ACTION_AREA. */
    M11_GFX_ACTION_AREA = 10,

    /* Empty panel background (graphic 20 in original CSB/DM).
     * 144×73 in GRAPHICS.DAT. Ref: C020_GRAPHIC_PANEL_EMPTY. */
    M11_GFX_PANEL_EMPTY = 20,

    /* Slot box graphics (18×18 each in GRAPHICS.DAT).
     * Used by DrawIconInSlotBox (ReDMCSB INVNTORY.C / OBJECT.C).
     * Ref: DEFS.H lines 2193-2196. */
    M11_GFX_SLOT_BOX_NORMAL      = 33, /* C033_GRAPHIC_SLOT_BOX_NORMAL */
    M11_GFX_SLOT_BOX_WOUNDED     = 34, /* C034_GRAPHIC_SLOT_BOX_WOUNDED */
    M11_GFX_SLOT_BOX_ACTING_HAND = 35, /* C035_GRAPHIC_SLOT_BOX_ACTING_HAND */

    /* Champion status box frames (67×29 each in GRAPHICS.DAT).
     * Ref: DEFS.H lines 2171-2172, 2197-2199. */
    M11_GFX_STATUS_BOX           =  7, /* C007_GRAPHIC_STATUS_BOX */
    M11_GFX_STATUS_BOX_DEAD      =  8, /* C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION */

    /* Champion portrait strip (256×87 in GRAPHICS.DAT).
     * 8 columns × 3 rows of 32×29 portraits.
     * Ref: DEFS.H line 2186, M027_PORTRAIT_X, M028_PORTRAIT_Y. */
    M11_GFX_CHAMPION_PORTRAITS   = 26, /* C026_GRAPHIC_CHAMPION_PORTRAITS */
    M11_PORTRAIT_W               = 32,
    M11_PORTRAIT_H               = 29,

    /* Food / water / poisoned labels.
     * Ref: DEFS.H lines 2190-2192. */
    M11_GFX_FOOD_LABEL           = 30, /* C030_GRAPHIC_FOOD_LABEL (34×9) */
    M11_GFX_WATER_LABEL          = 31, /* C031_GRAPHIC_WATER_LABEL (46×9) */
    M11_GFX_POISONED_LABEL       = 32, /* C032_GRAPHIC_POISONED_LABEL (96×15) */

    /* Party shield border overlays (67×29 each in GRAPHICS.DAT).
     * Drawn on top of champion status box when party effect is active.
     * Ref: DEFS.H lines 2197-2199. */
    M11_GFX_BORDER_PARTY_SHIELD      = 37, /* C037 */
    M11_GFX_BORDER_PARTY_FIRESHIELD  = 38, /* C038 */
    M11_GFX_BORDER_PARTY_SPELLSHIELD = 39, /* C039 */

    /* Damage indicator overlays.
     * C014: 88×45, drawn on viewport when creature takes damage.
     * C015: 45×7,  drawn on champion status box for non-inventory champion.
     * C016: 32×29, drawn on inventory view for the active champion.
     * Ref: ReDMCSB CHAMPION.C F0291 / MELEE.C display code. */
    M11_GFX_DAMAGE_TO_CREATURE        = 14, /* C014_GRAPHIC_DAMAGE_TO_CREATURE (88×45) */
    M11_GFX_DAMAGE_TO_CHAMPION_SMALL  = 15, /* C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL (45×7) */
    M11_GFX_DAMAGE_TO_CHAMPION_BIG    = 16  /* C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG (32×29) */
};

#define M11_GFX_DIALOG_BOX 17 /* C000_GRAPHIC_DIALOG_BOX, viewport-sized 224×136 */

/* ================================================================
 * Asset-backed rendering helpers for GRAPHICS.DAT integration.
 *
 * These functions load specific graphics from the asset loader and
 * blit them into the framebuffer at the correct viewport positions.
 * Each function gracefully falls back to the existing primitive
 * rendering when assets are unavailable.
 * ================================================================ */

/* Apply depth-based light dimming to a rectangular region.
 * depthIndex 0 = nearest (bright), 2 = far (dim).
 *
 * V1-faithful approach: instead of remapping palette indices through a
 * subjective dim_table, we set the per-pixel VGA palette brightness
 * level in the upper 4 bits of each framebuffer byte.  This matches
 * the original game, which achieves depth darkness by programming
 * different RGB values into the VGA DAC registers for the same colour
 * indices — same index, darker palette.  The RGBA conversion stage
 * in render_sdl_m11 reads the per-pixel level and looks up the
 * correct brightness from G9010_auc_VgaPaletteAll_Compat.
 *
 * Each depth pass adds 1 to the palette level, clamped to
 * M11_PALETTE_LEVELS-1 (the darkest).  Palette level 0 is brightest
 * (normal light), level 5 is the darkest. */
static void m11_apply_depth_dimming(unsigned char* framebuffer,
                                    int fbW,
                                    int fbH,
                                    int rx,
                                    int ry,
                                    int rw,
                                    int rh,
                                    int depthIndex) {
    int yy, xx;
    if (depthIndex <= 0) return;
    for (yy = ry; yy < ry + rh && yy < fbH; ++yy) {
        if (yy < 0) continue;
        for (xx = rx; xx < rx + rw && xx < fbW; ++xx) {
            unsigned char raw;
            int idx;
            int currentLevel;
            int newLevel;
            if (xx < 0) continue;
            raw = framebuffer[yy * fbW + xx];
            idx = M11_FB_DECODE_INDEX(raw);
            currentLevel = M11_FB_DECODE_LEVEL(raw);
            newLevel = currentLevel + depthIndex;
            if (newLevel >= M11_PALETTE_LEVELS) {
                newLevel = M11_PALETTE_LEVELS - 1;
            }
            framebuffer[yy * fbW + xx] = M11_FB_ENCODE(idx, newLevel);
        }
    }
}

/* Draw the source-backed DM1 floor/ceiling base for the 224x136
 * viewport.  ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF draws
 * graphic 79 (ceiling, 224x39) into C700_ZONE_VIEWPORT_CEILING_AREA
 * and graphic 78 (floor, 224x97) into C701_ZONE_VIEWPORT_FLOOR_AREA.
 *
 * Older Firestaff code incorrectly used graphic 0 as a full viewport
 * background; that asset is a UI/panel graphic, which is why the
 * dungeon looked like a random room/control panel instead of DM1. */
static void m11_blit_viewport_region_maybe_flip(const M11_AssetSlot* slot,
                                                int srcX,
                                                int srcY,
                                                int srcW,
                                                int srcH,
                                                unsigned char* framebuffer,
                                                int fbW,
                                                int fbH,
                                                int dstX,
                                                int dstY,
                                                int flipHorizontal) {
    int y;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        srcW <= 0 || srcH <= 0) {
        return;
    }
    if (!flipHorizontal) {
        M11_AssetLoader_BlitRegion(slot, srcX, srcY, srcW, srcH,
                                   framebuffer, fbW, fbH, dstX, dstY, -1);
        return;
    }
    if (srcX < 0 || srcY < 0 ||
        srcX + srcW > (int)slot->width ||
        srcY + srcH > (int)slot->height) {
        return;
    }
    for (y = 0; y < srcH; ++y) {
        int fbY = dstY + y;
        int x;
        if (fbY < 0 || fbY >= fbH) continue;
        for (x = 0; x < srcW; ++x) {
            int fbX = dstX + x;
            int sx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbW) continue;
            sx = srcX + (srcW - 1 - x);
            pixel = slot->pixels[(srcY + y) * (int)slot->width + sx];
            framebuffer[fbY * fbW + fbX] = pixel;
        }
    }
}

static void m11_draw_viewport_background(const M11_GameViewState* state,
                                         unsigned char* framebuffer,
                                         int fbW,
                                         int fbH,
                                         int vpX,
                                         int vpY,
                                         int vpW,
                                         int vpH) {
    if (state->assetsAvailable) {
        const M11_AssetSlot* ceilingSlot = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, M11_GFX_CEILING_PANEL);
        const M11_AssetSlot* floorSlot = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, M11_GFX_FLOOR_PANEL);
        if (ceilingSlot && floorSlot &&
            ceilingSlot->width == 224 && ceilingSlot->height == 39 &&
            floorSlot->width == 224 && floorSlot->height == 97 &&
            vpW == 224 && vpH == 136) {
            int parityFlip = ((state->world.party.mapX +
                               state->world.party.mapY +
                               state->world.party.direction) & 1) ? 1 : 0;
            /* ReDMCSB F0128 alternates horizontal flip between ceiling
             * and floor based on (mapX + mapY + direction) parity.  This
             * breaks repeated dither seams in the 224-wide viewport base
             * and matches the original CPSF draw order. */
            m11_blit_viewport_region_maybe_flip(ceilingSlot,
                                                0, 0, 224, 39,
                                                framebuffer, fbW, fbH,
                                                vpX, vpY,
                                                parityFlip ? 0 : 1);
            m11_blit_viewport_region_maybe_flip(floorSlot,
                                                0, 0, 224, 97,
                                                framebuffer, fbW, fbH,
                                                vpX, vpY + 39,
                                                parityFlip ? 1 : 0);
            return;
        }
    }
    /* Fallback: solid fills.
     *
     * Classic DM1 draws the ceiling strip as black (it is simply unlit
     * space above the visible corridor) and the floor strip as the
     * darkest stone gray when no asset is available.  The pre-correction
     * behaviour filled the ceiling with NAVY (slot 14 = 0,0,255 pure
     * blue) which read as a neon sky band, far from DM's unlit black
     * ceilings.  Ref: ReDMCSB DUNVIEW.C ceiling/floor strip zones. */
    m11_fill_rect(framebuffer, fbW, fbH,
                  vpX + 2, vpY + 2, vpW - 4, vpH / 2, M11_COLOR_BLACK);
    m11_fill_rect(framebuffer, fbW, fbH,
                  vpX + 2, vpY + vpH / 2, vpW - 4, vpH / 2 - 2,
                  M11_COLOR_DARK_GRAY);
}

typedef struct M11_DM1WallFrontBlit {
    int depthIndex;
    int relForward;
    int relSide;
    int graphicIndex;
    int dstX;
    int dstY;
    int width;
    int height;
} M11_DM1WallFrontBlit;

typedef struct M11_DM1ZoneBlit {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} M11_DM1ZoneBlit;

static unsigned int m11_wallset_graphic_index_for_state(const M11_GameViewState* state,
                                                        unsigned int wallSet0GraphicIndex) {
    int wallSet = 0;
    if (wallSet0GraphicIndex < (unsigned int)M11_GFX_DOOR_SIDE_D0 ||
        wallSet0GraphicIndex > (unsigned int)M11_GFX_DM1_STAIRS_SIDE_D0L) {
        return wallSet0GraphicIndex;
    }
    if (state && state->world.dungeon && state->world.dungeon->maps &&
        state->world.party.mapIndex >= 0 &&
        state->world.party.mapIndex < (int)state->world.dungeon->header.mapCount) {
        wallSet = (int)state->world.dungeon->maps[state->world.party.mapIndex].wallSet;
    }
    if (wallSet < 0) wallSet = 0;
    return (unsigned int)(M11_GFX_DOOR_SIDE_D0 + wallSet * 40 +
                          ((int)wallSet0GraphicIndex - M11_GFX_DOOR_SIDE_D0));
}

static int m11_draw_dm1_wall_blit_with_transparency(const M11_GameViewState* state,
                                                    unsigned char* framebuffer,
                                                    int fbW,
                                                    int fbH,
                                                    const M11_DM1WallFrontBlit* blit,
                                                    int transparentColor) {
    const M11_AssetSlot* slot;
    unsigned int graphicIndex;
    if (!state || !state->assetsAvailable || !blit) {
        return 0;
    }
    graphicIndex = m11_wallset_graphic_index_for_state(state,
                                                       (unsigned int)blit->graphicIndex);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                graphicIndex);
    if (!slot || slot->width != blit->width || slot->height != blit->height) {
        return 0;
    }
    M11_AssetLoader_BlitRegion(slot,
                               0, 0, blit->width, blit->height,
                               framebuffer, fbW, fbH,
                               M11_VIEWPORT_X + blit->dstX,
                               M11_VIEWPORT_Y + blit->dstY,
                               transparentColor);
    return 1;
}

static int m11_draw_dm1_front_wall_blit(const M11_GameViewState* state,
                                        unsigned char* framebuffer,
                                        int fbW,
                                        int fbH,
                                        const M11_DM1WallFrontBlit* blit) {
    return m11_draw_dm1_wall_blit_with_transparency(state, framebuffer,
                                                    fbW, fbH, blit, -1);
}

static int m11_draw_dm1_zone_blit(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW,
                                  int fbH,
                                  const M11_DM1ZoneBlit* blit,
                                  int transparentColor) {
    const M11_AssetSlot* slot;
    if (!state || !state->assetsAvailable || !blit) {
        return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                m11_wallset_graphic_index_for_state(state,
                                    (unsigned int)blit->graphicIndex));
    if (!slot || slot->width <= 0 || slot->height <= 0) {
        return 0;
    }
    if (blit->srcX < 0 || blit->srcY < 0 ||
        blit->srcX + blit->width > slot->width ||
        blit->srcY + blit->height > slot->height) {
        return 0;
    }
    M11_AssetLoader_BlitRegion(slot,
                               blit->srcX, blit->srcY,
                               blit->width, blit->height,
                               framebuffer, fbW, fbH,
                               M11_VIEWPORT_X + blit->dstX,
                               M11_VIEWPORT_Y + blit->dstY,
                               transparentColor);
    return 1;
}

static int m11_draw_dm1_zone_blit_maybe_flip(const M11_GameViewState* state,
                                             unsigned char* framebuffer,
                                             int fbW,
                                             int fbH,
                                             const M11_DM1ZoneBlit* blit,
                                             int transparentColor,
                                             int flipHorizontal) {
    const M11_AssetSlot* slot;
    int y;
    if (!flipHorizontal) {
        return m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH, blit, transparentColor);
    }
    if (!state || !state->assetsAvailable || !blit || !framebuffer) {
        return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                m11_wallset_graphic_index_for_state(state,
                                    (unsigned int)blit->graphicIndex));
    if (!slot || !slot->loaded || !slot->pixels || slot->width <= 0 || slot->height <= 0) {
        return 0;
    }
    if (blit->srcX < 0 || blit->srcY < 0 ||
        blit->srcX + blit->width > slot->width ||
        blit->srcY + blit->height > slot->height) {
        return 0;
    }
    for (y = 0; y < blit->height; ++y) {
        int x;
        int fbY = M11_VIEWPORT_Y + blit->dstY + y;
        if (fbY < 0 || fbY >= fbH) {
            continue;
        }
        for (x = 0; x < blit->width; ++x) {
            int fbX = M11_VIEWPORT_X + blit->dstX + x;
            int sx = blit->srcX + (blit->width - 1 - x);
            int sy = blit->srcY + y;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbW) {
                continue;
            }
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) {
                continue;
            }
            framebuffer[fbY * fbW + fbX] = pixel;
        }
    }
    return 1;
}

static int m11_draw_dm1_field_zone(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int fbW,
                                   int fbH,
                                   int dstX,
                                   int dstY,
                                   int dstW,
                                   int dstH,
                                   int baseStartUnit,
                                   int transparentColor,
                                   int maskIndexAndFlip) {
    const M11_AssetSlot* field;
    const M11_AssetSlot* mask = NULL;
    int maskFlip = 0;
    int y;
    if (!state || !state->assetsAvailable || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return 0;
    }
    field = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                 M11_GFX_DM1_FIELD_TELEPORTER);
    if (!field || !field->loaded || !field->pixels || field->width <= 0 || field->height <= 0) {
        return 0;
    }
    if (maskIndexAndFlip != 255) {
        int maskIndex = maskIndexAndFlip & 0x7f;
        maskFlip = (maskIndexAndFlip & 0x80) ? 1 : 0;
        mask = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                    M11_GFX_DM1_FIELD_MASK_BASE + maskIndex);
        if (!mask || !mask->loaded || !mask->pixels || mask->width <= 0 || mask->height <= 0) {
            mask = NULL;
        }
    }
    for (y = 0; y < dstH; ++y) {
        int fbY = M11_VIEWPORT_Y + dstY + y;
        int x;
        if (fbY < 0 || fbY >= fbH) {
            continue;
        }
        for (x = 0; x < dstW; ++x) {
            int fbX = M11_VIEWPORT_X + dstX + x;
            int sx;
            int sy;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbW) {
                continue;
            }
            if (mask) {
                int mx = x * (int)mask->width / dstW;
                int my = y * (int)mask->height / dstH;
                unsigned char maskPixel;
                if (maskFlip) {
                    mx = (int)mask->width - 1 - mx;
                }
                maskPixel = mask->pixels[my * (int)mask->width + mx];
                if (maskPixel == 0) {
                    continue;
                }
            }
            sx = (x + (baseStartUnit * 16)) % (int)field->width;
            sy = y % (int)field->height;
            pixel = field->pixels[sy * (int)field->width + sx];
            if ((transparentColor & 0x7f) != 0x7f && pixel == (unsigned char)(transparentColor & 0x7f)) {
                continue;
            }
            framebuffer[fbY * fbW + fbX] = pixel;
        }
    }
    return 1;
}

static void m11_blit_scaled_palette_map(const M11_AssetSlot* slot,
                                        unsigned char* framebuffer,
                                        int fbW,
                                        int fbH,
                                        int dstX,
                                        int dstY,
                                        int dstW,
                                        int dstH,
                                        int transparentColor,
                                        const unsigned char paletteMap[16]) {
    int dy;
    if (!slot || !slot->loaded || !slot->pixels || !framebuffer ||
        dstW <= 0 || dstH <= 0) {
        return;
    }
    for (dy = 0; dy < dstH; ++dy) {
        int sy = dy * (int)slot->height / dstH;
        int fbY = dstY + dy;
        int dx;
        if (fbY < 0 || fbY >= fbH) {
            continue;
        }
        for (dx = 0; dx < dstW; ++dx) {
            int sx = dx * (int)slot->width / dstW;
            int fbX = dstX + dx;
            unsigned char pixel;
            if (fbX < 0 || fbX >= fbW) {
                continue;
            }
            pixel = slot->pixels[sy * (int)slot->width + sx];
            if (transparentColor >= 0 && pixel == (unsigned char)transparentColor) {
                continue;
            }
            if (paletteMap) {
                pixel = paletteMap[pixel & 0x0f];
            }
            framebuffer[fbY * fbW + fbX] = pixel;
        }
    }
}

static int m11_dm1_door_panel_graphic(const M11_GameViewState* state,
                                      const M11_ViewportCell* cell,
                                      int depthIndex) {
    int mapIdx;
    int doorSet = 0;
    int doorType = 0;
    int depthOffset;
    if (!state || !state->world.dungeon || !cell || depthIndex < 0 || depthIndex > 2) {
        return -1;
    }
    mapIdx = state->world.party.mapIndex;
    if (mapIdx < 0 || mapIdx >= state->world.dungeon->header.mapCount) {
        return -1;
    }
    doorType = cell->doorType & 1;
    doorSet = doorType ?
        (int)state->world.dungeon->maps[mapIdx].doorSet1 :
        (int)state->world.dungeon->maps[mapIdx].doorSet0;
    /* ReDMCSB F0095_DUNGEONVIEW_LoadDoorSet:
     *   D3 = M633 + doorSet*3 + 0
     *   D2 = M633 + doorSet*3 + 1
     *   D1 = M633 + doorSet*3 + 2 */
    depthOffset = 2 - depthIndex;
    return M11_GFX_DOOR_SET0_D3 + doorSet * 3 + depthOffset;
}

static int m11_dm1_scaled_dimension(int dimension, int scale) {
    return ((dimension * scale) + (scale >> 1)) >> 5;
}

static int m11_dm1_door_ornament_info(const M11_GameViewState* state,
                                      int ornamentOrdinal,
                                      int* outGraphicIndex,
                                      int* outCoordSet) {
    static const unsigned char kDoorOrnCoordSet[12] = {
        0, 1, 1, 1, 0, 1, 2, 1, 1, 1, 1, 1
    };
    int mapIdx;
    int ordIdx;
    int globalIdx;
    if (!state || !state->world.dungeon || ornamentOrdinal <= 0 ||
        !outGraphicIndex || !outCoordSet) {
        return 0;
    }
    mapIdx = state->world.party.mapIndex;
    ordIdx = ornamentOrdinal - 1;
    m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);
    if (mapIdx >= 0 && mapIdx < 32 && state->ornamentCacheLoaded[mapIdx] && ordIdx < 16) {
        globalIdx = state->doorOrnamentIndices[mapIdx][ordIdx];
    } else {
        globalIdx = ordIdx;
    }
    if (globalIdx < 0) {
        return 0;
    }
    *outGraphicIndex = M11_GFX_DOOR_ORNAMENT_BASE + globalIdx;
    *outCoordSet = (globalIdx >= 0 && globalIdx < 12) ? kDoorOrnCoordSet[globalIdx] : 1;
    return 1;
}

static void m11_draw_dm1_door_ornament_on_panel(const M11_GameViewState* state,
                                                unsigned char* framebuffer,
                                                int fbW,
                                                int fbH,
                                                const M11_DM1ZoneBlit* panel,
                                                int depthIndex,
                                                int ornamentOrdinal) {
    static const unsigned char kOrnD3Palette[16] = {
        0, 12, 1, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 2, 0, 13
    };
    static const unsigned char kOrnD2Palette[16] = {
        0, 1, 2, 3, 4, 3, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15
    };
    static const int kAnchorX[3][3] = {
        {28, 42, 63}, /* coordinate set 0: right-aligned anchors */
        {15, 22, 33}, /* coordinate set 1: left/top-ish anchors */
        {34, 50, 75}  /* coordinate set 2: right/bottom anchors */
    };
    static const int kAnchorY[3][3] = {
        {13, 17, 22},
        {23, 35, 53},
        {37, 53, 80}
    };
    const M11_AssetSlot* slot;
    int graphicIndex;
    int coordSet;
    int scale;
    int viewIndex;
    int ornW;
    int ornH;
    int relX;
    int relY;
    if (!state || !state->assetsAvailable || !panel || ornamentOrdinal <= 0) {
        return;
    }
    if (depthIndex < 0 || depthIndex > 2) {
        return;
    }
    if (!m11_dm1_door_ornament_info(state, ornamentOrdinal, &graphicIndex, &coordSet)) {
        return;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)graphicIndex);
    if (!slot || slot->width <= 0 || slot->height <= 0) {
        return;
    }
    if (coordSet < 0 || coordSet > 2) {
        coordSet = 1;
    }
    scale = (depthIndex == 0) ? 32 : ((depthIndex == 1) ? 21 : 14);
    viewIndex = (depthIndex == 0) ? 2 : ((depthIndex == 1) ? 1 : 0);
    ornW = m11_dm1_scaled_dimension(slot->width, scale);
    ornH = m11_dm1_scaled_dimension(slot->height, scale);
    if (ornW <= 0 || ornH <= 0) {
        return;
    }
    if (coordSet == 0) {
        relX = kAnchorX[coordSet][viewIndex] - ornW;
        relY = kAnchorY[coordSet][viewIndex];
    } else if (coordSet == 1) {
        relX = kAnchorX[coordSet][viewIndex];
        relY = kAnchorY[coordSet][viewIndex] - ornH;
    } else {
        relX = kAnchorX[coordSet][viewIndex] - ornW;
        relY = kAnchorY[coordSet][viewIndex] - ornH;
    }
    m11_blit_scaled_palette_map(slot,
                                framebuffer, fbW, fbH,
                                M11_VIEWPORT_X + panel->dstX + relX,
                                M11_VIEWPORT_Y + panel->dstY + relY,
                                ornW, ornH,
                                9,
                                depthIndex == 2 ? kOrnD3Palette :
                                    (depthIndex == 1 ? kOrnD2Palette : NULL));
}

static void m11_draw_dm1_destroyed_door_mask_on_panel(const M11_GameViewState* state,
                                                      unsigned char* framebuffer,
                                                      int fbW,
                                                      int fbH,
                                                      const M11_DM1ZoneBlit* panel) {
    const M11_AssetSlot* slot;
    if (!state || !state->assetsAvailable || !panel) {
        return;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                M11_GFX_DOOR_MASK_DESTROYED);
    if (!slot || slot->width <= 0 || slot->height <= 0) {
        return;
    }
    /* F0111 draws C15_DOOR_ORNAMENT_DESTROYED_MASK into the temporary
     * door bitmap before blitting the door.  Graphic 439 is the full-size
     * mask; scale it to the already-resolved visible door panel zone. */
    M11_AssetLoader_BlitScaled(slot,
                               framebuffer, fbW, fbH,
                               M11_VIEWPORT_X + panel->dstX,
                               M11_VIEWPORT_Y + panel->dstY,
                               panel->width,
                               panel->height,
                               9);
}

static void m11_draw_dm1_front_walls(const M11_GameViewState* state,
                                     unsigned char* framebuffer,
                                     int fbW,
                                     int fbH,
                                     const M11_ViewportCell cells[3][3]) {
    static const M11_DM1WallFrontBlit kFrontBlits[3] = {
        /* Resolved from layout 696 via COORD.C F0635_ semantics:
         *   graphic 97  -> C712_ZONE_WALL_D1C: x=32,y=9,w=160,h=111
         *   graphic 102 -> C709_ZONE_WALL_D2C: x=59,y=19,w=106,h=74
         *   graphic 107 -> C704_ZONE_WALL_D3C: x=77,y=25,w=70,h=49 */
        {0, 1, 0, M11_GFX_WALLSET0_D1C, 32, 9, 160, 111},
        {1, 2, 0, M11_GFX_WALLSET0_D2C, 59, 19, 106, 74},
        {2, 3, 0, M11_GFX_WALLSET0_D3C, 77, 25, 70, 49}
    };
    int depth;
    int occluded = 0;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (depth = 0; depth < 3; ++depth) {
        if (occluded) {
            break;
        }
        if (m11_viewport_cell_is_wall_like(&cells[depth][1])) {
            (void)m11_draw_dm1_front_wall_blit(state, framebuffer, fbW, fbH,
                                               &kFrontBlits[depth]);
            occluded = 1;
        }
    }
}

static void m11_draw_dm1_floor_pits(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH) {
    typedef struct M11_DM1PitSpec {
        int relForward;
        int relSide;
        M11_DM1ZoneBlit blit;
        M11_DM1ZoneBlit invisibleBlit;
        int hasInvisibleBlit;
    } M11_DM1PitSpec;
    static const M11_DM1PitSpec kPits[] = {
        {3, -2, {M11_GFX_FLOOR_PIT_D3L2, 0, 0, 0,   66, 22, 10}, {0}, 0},
        {3,  2, {M11_GFX_FLOOR_PIT_D3L2, 0, 0, 202, 66, 22, 10}, {0}, 0},
        {3, -1, {M11_GFX_FLOOR_PIT_D3L,  0, 0, 4,   65, 78, 8},  {0}, 0},
        {3,  0, {M11_GFX_FLOOR_PIT_D3C,  0, 0, 79,  65, 64, 8},  {0}, 0},
        {3,  1, {M11_GFX_FLOOR_PIT_D3L,  0, 0, 142, 65, 78, 8},  {0}, 0},
        {2, -1, {M11_GFX_FLOOR_PIT_D2L,  1, 0, 0,   76, 71, 13}, {M11_GFX_FLOOR_PIT_INVISIBLE_D2L, 1,0,0,   76,71,12}, 1},
        {2,  0, {M11_GFX_FLOOR_PIT_D2C,  0, 0, 66,  77, 92, 12}, {M11_GFX_FLOOR_PIT_INVISIBLE_D2C, 0,0,66,  77,92,12}, 1},
        {2,  1, {M11_GFX_FLOOR_PIT_D2L,  0, 0, 153, 76, 71, 13}, {M11_GFX_FLOOR_PIT_INVISIBLE_D2L, 0,0,153, 76,71,12}, 1},
        {1, -1, {M11_GFX_FLOOR_PIT_D1L,  3, 0, 0,   94, 54, 24}, {M11_GFX_FLOOR_PIT_INVISIBLE_D1L, 3,0,0,   94,49,24}, 1},
        {1,  0, {M11_GFX_FLOOR_PIT_D1C,  0, 0, 43,  94, 139,24}, {M11_GFX_FLOOR_PIT_INVISIBLE_D1C, 0,0,41,  94,144,24},1},
        {1,  1, {M11_GFX_FLOOR_PIT_D1L,  0, 0, 169, 94, 55, 24}, {M11_GFX_FLOOR_PIT_INVISIBLE_D1L, 0,0,174, 94,50,24}, 1},
        {0, -1, {M11_GFX_FLOOR_PIT_D0L,  4, 0, 0,   126,20, 10}, {M11_GFX_FLOOR_PIT_INVISIBLE_D0L, 4,0,0,   126,14,10}, 1},
        {0,  0, {M11_GFX_FLOOR_PIT_D0C,  0, 0, 27,  127,170,9},  {M11_GFX_FLOOR_PIT_INVISIBLE_D0C, 0,0,25,  127,174,9},1},
        {0,  1, {M11_GFX_FLOOR_PIT_D0L,  0, 0, 200, 126,24, 10}, {M11_GFX_FLOOR_PIT_INVISIBLE_D0L, 0,0,206, 126,18,10}, 1}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kPits) / sizeof(kPits[0]); ++i) {
        M11_ViewportCell cell;
        if (!m11_sample_viewport_cell(state, kPits[i].relForward, kPits[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_PIT) {
            continue;
        }
        if (cell.square & 0x04) { /* MASK0x0004_PIT_INVISIBLE */
            if (!kPits[i].hasInvisibleBlit) {
                continue;
            }
            (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH,
                                         &kPits[i].invisibleBlit, 10);
        } else {
            (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH,
                                         &kPits[i].blit, 10);
        }
    }
}

static void m11_draw_dm1_floor_ornaments(const M11_GameViewState* state,
                                         unsigned char* framebuffer,
                                         int fbW,
                                         int fbH) {
    typedef struct M11_DM1FloorOrnSpec {
        int relForward;
        int relSide;
        int increment;
        int flipHorizontal;
        M11_DM1ZoneBlit blit;
    } M11_DM1FloorOrnSpec;
    static const M11_DM1FloorOrnSpec kOrnaments[] = {
        {3,-2,0,0,{0,39,0,0,   66,1, 6}},
        {3, 2,0,1,{0,0, 0,223, 66,1, 6}},
        {3,-1,0,0,{0,0, 0,32,  67,40,6}},
        {3, 0,1,0,{0,0, 0,99,  67,26,6}},
        {3, 1,0,1,{0,0, 0,153, 67,40,6}},
        {2,-1,2,0,{0,0, 0,1,   77,60,11}},
        {2, 0,3,0,{0,0, 0,91,  77,42,11}},
        {2, 1,2,1,{0,0, 0,167, 77,57,11}},
        {1,-1,4,0,{0,0, 0,0,   96,25,21}},
        {1, 0,5,0,{0,0, 0,81,  94,62,23}},
        {1, 1,4,1,{0,0, 0,199, 96,25,21}}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kOrnaments) / sizeof(kOrnaments[0]); ++i) {
        M11_ViewportCell cell;
        M11_DM1ZoneBlit blit;
        int localIdx;
        int mapIdx;
        int ornGlobalIdx = -1;
        if (!m11_sample_viewport_cell(state, kOrnaments[i].relForward, kOrnaments[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.floorOrnamentOrdinal <= 0) {
            continue;
        }
        mapIdx = state->world.party.mapIndex;
        localIdx = cell.floorOrnamentOrdinal - 1;
        m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);
        if (mapIdx >= 0 && mapIdx < 32 &&
            state->ornamentCacheLoaded[mapIdx] &&
            localIdx >= 0 && localIdx < 16) {
            ornGlobalIdx = state->floorOrnamentIndices[mapIdx][localIdx];
        } else {
            ornGlobalIdx = localIdx;
        }
        if (ornGlobalIdx < 0) {
            continue;
        }
        blit = kOrnaments[i].blit;
        blit.graphicIndex =
            (ornGlobalIdx == M11_FLOOR_ORNAMENT_FOOTPRINTS_INDEX
                 ? M11_GFX_FLOOR_ORNAMENT_FOOTPRINTS_BASE
                 : M11_GFX_FLOOR_ORNAMENT_BASE +
                       ornGlobalIdx * M11_GFX_FLOOR_ORNAMENT_VARIANTS) +
            kOrnaments[i].increment;
        (void)m11_draw_dm1_zone_blit_maybe_flip(state, framebuffer, fbW, fbH,
                                                &blit, 10,
                                                kOrnaments[i].flipHorizontal);
    }
}

static void m11_draw_dm1_wall_ornaments(const M11_GameViewState* state,
                                        unsigned char* framebuffer,
                                        int fbW,
                                        int fbH) {
    typedef struct M11_DM1WallOrnSpec {
        int relForward;
        int relSide;
        int nativeOffset;
        int flipHorizontal;
        M11_DM1ZoneBlit blit;
    } M11_DM1WallOrnSpec;
    static const M11_DM1WallOrnSpec kWallOrnaments[] = {
        {3,-2,0,1,{0,0,0,26,  21,10,42}},
        {3, 2,0,1,{0,0,0,187, 22,10,42}},
        {3,-1,0,0,{0,0,0,80,  22,10,42}},
        {3, 1,0,1,{0,0,0,134, 22,10,42}},
        {3,-1,1,0,{0,0,0,0,   16,90,56}},
        {3, 0,1,0,{0,0,0,67,  16,90,56}},
        {3, 1,1,0,{0,0,0,135, 16,89,56}},
        {2,-1,0,0,{0,0,0,66,  24,10,42}},
        {2, 1,0,1,{0,0,0,149, 24,10,42}},
        {2,-1,1,0,{0,35,0,0,  19,55,56}},
        {2, 0,1,0,{0,0,0,67,  19,90,56}},
        {2, 1,1,0,{0,0,0,169, 19,55,56}},
        {1,-1,0,0,{0,0,0,50,  28,10,42}},
        {1, 1,0,1,{0,0,0,165, 28,10,42}},
        {1, 0,1,0,{0,0,0,67,  22,90,56}}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kWallOrnaments) / sizeof(kWallOrnaments[0]); ++i) {
        M11_ViewportCell cell;
        M11_DM1ZoneBlit blit;
        int localIdx;
        int mapIdx;
        int ornGlobalIdx = -1;
        if (!m11_sample_viewport_cell(state, kWallOrnaments[i].relForward, kWallOrnaments[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_WALL || cell.wallOrnamentOrdinal <= 0) {
            continue;
        }
        mapIdx = state->world.party.mapIndex;
        localIdx = cell.wallOrnamentOrdinal - 1;
        m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);
        if (mapIdx >= 0 && mapIdx < 32 && state->ornamentCacheLoaded[mapIdx] &&
            localIdx >= 0 && localIdx < 16) {
            ornGlobalIdx = state->wallOrnamentIndices[mapIdx][localIdx];
        } else {
            ornGlobalIdx = localIdx;
        }
        if (ornGlobalIdx < 0) {
            continue;
        }
        blit = kWallOrnaments[i].blit;
        blit.graphicIndex = M11_GFX_WALL_ORNAMENT_BASE + ornGlobalIdx * 2 +
            kWallOrnaments[i].nativeOffset;
        (void)m11_draw_dm1_zone_blit_maybe_flip(state, framebuffer, fbW, fbH,
                                                &blit, 10,
                                                kWallOrnaments[i].flipHorizontal);
    }
}

static int m11_dm1_stairs_front_facing(const M11_GameViewState* state,
                                       const M11_ViewportCell* cell) {
    int northSouth;
    if (!state || !cell) {
        return 1;
    }
    /* ReDMCSB/DEFS.H: stairs bit 0x08 marks north/south orientation. */
    northSouth = (cell->square & 0x08) ? 1 : 0;
    return northSouth ?
        (state->world.party.direction == DIR_NORTH || state->world.party.direction == DIR_SOUTH) :
        (state->world.party.direction == DIR_EAST || state->world.party.direction == DIR_WEST);
}

static void m11_draw_dm1_stairs(const M11_GameViewState* state,
                                unsigned char* framebuffer,
                                int fbW,
                                int fbH) {
    typedef struct M11_DM1StairSpec {
        int relForward;
        int relSide;
        int frontOnly;
        int sideOnly;
        int upGfx;
        int downGfx;
        M11_DM1ZoneBlit upBlit;
        M11_DM1ZoneBlit downBlit;
    } M11_DM1StairSpec;
    static const M11_DM1StairSpec kStairs[] = {
        {3,-2,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   0,0,0,   25,63,45}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   0,0,0,   25,75,41}},
        {3, 2,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   0,0,161, 25,63,45}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   0,0,149, 25,75,41}},
        {3,-1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   0,0,14,  26,63,45}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   0,0,13,  28,75,41}},
        {3, 0,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D3C,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D3C,   {M11_GFX_DM1_STAIRS_UP_FRONT_D3C,   0,0,78,  25,68,46}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D3C,   0,0,75,  25,74,49}},
        {3, 1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D3L,   0,0,147, 26,63,45}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D3L,   0,0,133, 28,75,41}},
        {2,-1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D2L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D2L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D2L,   1,0,0,   24,59,62}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D2L,   0,0,0,   20,61,62}},
        {2, 0,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D2C,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D2C,   {M11_GFX_DM1_STAIRS_UP_FRONT_D2C,   0,0,62,  20,100,63},{M11_GFX_DM1_STAIRS_DOWN_FRONT_D2C,   0,0,63,  24,98,61}},
        {2, 1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D2L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D2L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D2L,   0,0,165, 20,59,62}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D2L,   0,0,164, 24,60,62}},
        {1,-1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D1L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D1L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D1L,   0,0,0,   9, 32,100},{M11_GFX_DM1_STAIRS_DOWN_FRONT_D1L,   0,0,0,   17,32,91}},
        {1, 0,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D1C,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D1C,   {M11_GFX_DM1_STAIRS_UP_FRONT_D1C,   0,0,32,  9, 160,100},{M11_GFX_DM1_STAIRS_DOWN_FRONT_D1C,   0,0,35,  17,152,92}},
        {1, 1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D1L,   M11_GFX_DM1_STAIRS_DOWN_FRONT_D1L,   {M11_GFX_DM1_STAIRS_UP_FRONT_D1L,   0,0,192, 9, 32,100},{M11_GFX_DM1_STAIRS_DOWN_FRONT_D1L,   0,0,192, 18,32,91}},
        {0,-1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D0C_L, M11_GFX_DM1_STAIRS_DOWN_FRONT_D0C_L, {M11_GFX_DM1_STAIRS_UP_FRONT_D0C_L, 0,0,0,   58,30,44}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D0C_L, 0,0,0,   76,30,60}},
        {0, 1,1,0,M11_GFX_DM1_STAIRS_UP_FRONT_D0C_L, M11_GFX_DM1_STAIRS_DOWN_FRONT_D0C_L, {M11_GFX_DM1_STAIRS_UP_FRONT_D0C_L, 0,0,194, 58,30,44}, {M11_GFX_DM1_STAIRS_DOWN_FRONT_D0C_L, 0,0,194, 76,30,60}},
        {2,-1,0,1,M11_GFX_DM1_STAIRS_SIDE_D2L,       M11_GFX_DM1_STAIRS_SIDE_D2L,         {M11_GFX_DM1_STAIRS_SIDE_D2L,       0,0,60,  55,8, 5},  {M11_GFX_DM1_STAIRS_SIDE_D2L,       0,0,60,  55,8, 5}},
        {2, 1,0,1,M11_GFX_DM1_STAIRS_SIDE_D2L,       M11_GFX_DM1_STAIRS_SIDE_D2L,         {M11_GFX_DM1_STAIRS_SIDE_D2L,       0,0,156, 56,8, 5},  {M11_GFX_DM1_STAIRS_SIDE_D2L,       0,0,156, 56,8, 5}},
        {1,-1,0,1,M11_GFX_DM1_STAIRS_UP_SIDE_D1L,    M11_GFX_DM1_STAIRS_DOWN_SIDE_D1L,    {M11_GFX_DM1_STAIRS_UP_SIDE_D1L,    0,0,32,  58,20,43}, {M11_GFX_DM1_STAIRS_DOWN_SIDE_D1L,  0,0,32,  62,20,39}},
        {1, 1,0,1,M11_GFX_DM1_STAIRS_UP_SIDE_D1L,    M11_GFX_DM1_STAIRS_DOWN_SIDE_D1L,    {M11_GFX_DM1_STAIRS_UP_SIDE_D1L,    0,0,172, 57,20,43}, {M11_GFX_DM1_STAIRS_DOWN_SIDE_D1L,  0,0,172, 62,20,39}},
        {0,-1,0,1,M11_GFX_DM1_STAIRS_SIDE_D0L,       M11_GFX_DM1_STAIRS_SIDE_D0L,         {M11_GFX_DM1_STAIRS_SIDE_D0L,       0,0,0,   73,16,13}, {M11_GFX_DM1_STAIRS_SIDE_D0L,       0,0,0,   73,16,13}},
        {0, 1,0,1,M11_GFX_DM1_STAIRS_SIDE_D0L,       M11_GFX_DM1_STAIRS_SIDE_D0L,         {M11_GFX_DM1_STAIRS_SIDE_D0L,       0,0,208, 73,16,13}, {M11_GFX_DM1_STAIRS_SIDE_D0L,       0,0,208, 73,16,13}}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kStairs) / sizeof(kStairs[0]); ++i) {
        M11_ViewportCell cell;
        int frontFacing;
        int stairUp;
        if (!m11_sample_viewport_cell(state, kStairs[i].relForward, kStairs[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_STAIRS) {
            continue;
        }
        frontFacing = m11_dm1_stairs_front_facing(state, &cell);
        if ((kStairs[i].frontOnly && !frontFacing) || (kStairs[i].sideOnly && frontFacing)) {
            continue;
        }
        stairUp = (cell.square & 0x01) ? 1 : 0;
        (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH,
                                     stairUp ? &kStairs[i].upBlit : &kStairs[i].downBlit,
                                     0);
    }
}

static void m11_draw_dm1_teleporter_fields(const M11_GameViewState* state,
                                           unsigned char* framebuffer,
                                           int fbW,
                                           int fbH) {
    typedef struct M11_DM1FieldSpec {
        int relForward;
        int relSide;
        int dstX;
        int dstY;
        int dstW;
        int dstH;
        int baseStartUnit;
        int transparentColor;
        int maskIndexAndFlip;
    } M11_DM1FieldSpec;
    static const M11_DM1FieldSpec kFields[] = {
        /* ReDMCSB G2035 view-square -> G0188 field-aspect mapping,
         * resolved through layout-696 wall zones C702..C717. */
        {3,-2,0,   25,36, 49,0x3f,0x0a,0x00},
        {3, 2,188, 25,36, 49,0x3f,0x0a,0x80},
        {3,-1,7,   25,83, 49,0x3f,0x0a,0x01},
        {3, 0,77,  25,70, 49,0x3f,0x8a,0xff},
        {3, 1,134, 25,83, 49,0x3f,0x0a,0x81},
        {2,-2,0,   24,8,  52,0x3f,0x0a,0x02},
        {2, 2,216, 24,8,  52,0x3f,0x0a,0x82},
        {2,-1,0,   19,78, 74,0x3f,0x0a,0x03},
        {2, 0,59,  19,106,74,0x3c,0x8a,0xff},
        {2, 1,146, 19,78, 74,0x3f,0x0a,0x83},
        {1,-1,0,   9, 60, 111,0x3f,0x0a,0x04},
        {1, 0,32,  9, 160,111,0x3d,0x8a,0xff},
        {1, 1,164, 9, 60, 111,0x3f,0x0a,0x84},
        {0,-1,0,   0, 33, 136,0x3f,0x0a,0x05},
        {0, 0,0,   0, 224,136,0x3b,0x8a,0xff},
        {0, 1,191, 0, 33, 136,0x3f,0x0a,0x85}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kFields) / sizeof(kFields[0]); ++i) {
        M11_ViewportCell cell;
        if (!m11_sample_viewport_cell(state, kFields[i].relForward, kFields[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_TELEPORTER) {
            continue;
        }
        if ((cell.square & 0x04) == 0 || (cell.square & 0x08) == 0) {
            continue;
        }
        (void)m11_draw_dm1_field_zone(state, framebuffer, fbW, fbH,
                                      kFields[i].dstX, kFields[i].dstY,
                                      kFields[i].dstW, kFields[i].dstH,
                                      kFields[i].baseStartUnit,
                                      kFields[i].transparentColor,
                                      kFields[i].maskIndexAndFlip);
    }
}

static void m11_draw_dm1_side_walls(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH) {
    static const M11_DM1WallFrontBlit kSideBlits[] = {
        /* Far to near, matching the first source-bound subset of
         * DUNVIEW.C F0097/F012x wall-zone order.  The relForward/relSide
         * coordinates name the viewed square; dst rects are layout-696
         * F0635_-resolved viewport zones. */
        {3, 3, -2, M11_GFX_WALLSET0_D3L2, 0,   25, 44, 49},
        {3, 3,  2, M11_GFX_WALLSET0_D3R2, 180, 25, 44, 49},
        {3, 3, -1, M11_GFX_WALLSET0_D3L,  7,   25, 83, 49},
        {3, 3,  1, M11_GFX_WALLSET0_D3R,  134, 25, 83, 49},
        {2, 2, -2, M11_GFX_WALLSET0_D2L2, 0,   24, 8,  52},
        {2, 2,  2, M11_GFX_WALLSET0_D2R2, 216, 24, 8,  52},
        {2, 2, -1, M11_GFX_WALLSET0_D2L,  0,   19, 78, 74},
        {2, 2,  1, M11_GFX_WALLSET0_D2R,  146, 19, 78, 74},
        {1, 1, -1, M11_GFX_WALLSET0_D1L,  0,   9,  60, 111},
        {1, 1,  1, M11_GFX_WALLSET0_D1R,  164, 9,  60, 111},
        {0, 0, -1, M11_GFX_WALLSET0_D0L,  0,   0,  33, 136},
        {0, 0,  1, M11_GFX_WALLSET0_D0R,  191, 0,  33, 136}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kSideBlits) / sizeof(kSideBlits[0]); ++i) {
        M11_ViewportCell cell;
        if (!m11_sample_viewport_cell(state,
                                      kSideBlits[i].relForward,
                                      kSideBlits[i].relSide,
                                      &cell)) {
            continue;
        }
        if (m11_viewport_cell_is_wall_like(&cell)) {
            (void)m11_draw_dm1_wall_blit_with_transparency(state,
                                                           framebuffer,
                                                           fbW,
                                                           fbH,
                                                           &kSideBlits[i],
                                                           M11_COLOR_MAGENTA);
        }
    }
}

static void m11_draw_dm1_center_doors(const M11_GameViewState* state,
                                      unsigned char* framebuffer,
                                      int fbW,
                                      int fbH,
                                      const M11_ViewportCell cells[3][3]) {
    static const M11_DM1ZoneBlit kD1C[] = {
        {M11_GFX_DOOR_FRAME_TOP_D1, 0, 0, 61, 12, 102, 4},
        {M11_GFX_DOOR_SIDE_D1,     0, 0, 44, 13, 25, 94},
        {M11_GFX_DOOR_SIDE_D1,     0, 0, 155, 13, 25, 94}
    };
    static const M11_DM1ZoneBlit kD2C[] = {
        {M11_GFX_DOOR_FRAME_TOP_D2, 0, 0, 77, 21, 70, 3},
        {M11_GFX_DOOR_SIDE_D2,     0, 0, 65, 21, 18, 65},
        {M11_GFX_DOOR_SIDE_D2,     0, 0, 141, 21, 18, 65}
    };
    static const M11_DM1ZoneBlit kD3C[] = {
        {M11_GFX_DOOR_SIDE_D3, 0, 0, 82, 27, 10, 42},
        {M11_GFX_DOOR_SIDE_D3, 0, 0, 132, 27, 10, 42}
    };
    static const M11_DM1ZoneBlit kDoorPanels[3][4] = {
        {
            {M11_GFX_DOOR_SET0_D1, 0, 0, 64, 16, 96, 86},
            {M11_GFX_DOOR_SET0_D1, 0, 65, 64, 15, 96, 23},
            {M11_GFX_DOOR_SET0_D1, 0, 43, 64, 15, 96, 45},
            {M11_GFX_DOOR_SET0_D1, 0, 21, 64, 15, 96, 67}
        },
        {
            {M11_GFX_DOOR_SET0_D2, 0, 0, 80, 24, 64, 59},
            {M11_GFX_DOOR_SET0_D2, 0, 44, 80, 24, 64, 17},
            {M11_GFX_DOOR_SET0_D2, 0, 29, 80, 24, 64, 32},
            {M11_GFX_DOOR_SET0_D2, 0, 14, 80, 24, 64, 47}
        },
        {
            {M11_GFX_DOOR_SET0_D3, 0, 0, 90, 30, 44, 38},
            {M11_GFX_DOOR_SET0_D3, 0, 27, 90, 29, 44, 11},
            {M11_GFX_DOOR_SET0_D3, 0, 17, 90, 29, 44, 21},
            {M11_GFX_DOOR_SET0_D3, 0, 7,  90, 29, 44, 31}
        }
    };
    static const struct { const M11_DM1ZoneBlit* blits; size_t count; } kByDepth[3] = {
        {kD1C, sizeof(kD1C) / sizeof(kD1C[0])},
        {kD2C, sizeof(kD2C) / sizeof(kD2C[0])},
        {kD3C, sizeof(kD3C) / sizeof(kD3C[0])}
    };
    int depth;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (depth = 0; depth < 3; ++depth) {
        size_t i;
        const M11_ViewportCell* cell = &cells[depth][1];
        if (!cell->valid || cell->elementType != DUNGEON_ELEMENT_DOOR ||
            m11_viewport_cell_is_open(cell)) {
            continue;
        }
        for (i = 0; i < kByDepth[depth].count; ++i) {
            (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH,
                                         &kByDepth[depth].blits[i], 10);
        }
        {
            int panelState = (cell->doorState >= 1 && cell->doorState <= 3) ?
                cell->doorState : 0;
            M11_DM1ZoneBlit panel = kDoorPanels[depth][panelState];
            int panelGraphic = m11_dm1_door_panel_graphic(state, cell, depth);
            if (panelGraphic >= 0) {
                panel.graphicIndex = panelGraphic;
            }
            (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH,
                                         &panel, 10);
        }
        break;
    }
}

static void m11_draw_dm1_center_door_ornaments(const M11_GameViewState* state,
                                               unsigned char* framebuffer,
                                               int fbW,
                                               int fbH,
                                               const M11_ViewportCell cells[3][3]) {
    static const M11_DM1ZoneBlit kDoorPanels[3][4] = {
        {
            {M11_GFX_DOOR_SET0_D1, 0, 0, 64, 16, 96, 86},
            {M11_GFX_DOOR_SET0_D1, 0, 65, 64, 15, 96, 23},
            {M11_GFX_DOOR_SET0_D1, 0, 43, 64, 15, 96, 45},
            {M11_GFX_DOOR_SET0_D1, 0, 21, 64, 15, 96, 67}
        },
        {
            {M11_GFX_DOOR_SET0_D2, 0, 0, 80, 24, 64, 59},
            {M11_GFX_DOOR_SET0_D2, 0, 44, 80, 24, 64, 17},
            {M11_GFX_DOOR_SET0_D2, 0, 29, 80, 24, 64, 32},
            {M11_GFX_DOOR_SET0_D2, 0, 14, 80, 24, 64, 47}
        },
        {
            {M11_GFX_DOOR_SET0_D3, 0, 0, 90, 30, 44, 38},
            {M11_GFX_DOOR_SET0_D3, 0, 27, 90, 29, 44, 11},
            {M11_GFX_DOOR_SET0_D3, 0, 17, 90, 29, 44, 21},
            {M11_GFX_DOOR_SET0_D3, 0, 7,  90, 29, 44, 31}
        }
    };
    static const int kAnchorX[3][3] = {
        {28, 42, 63}, /* coordinate set 0: right-aligned anchors */
        {15, 22, 33}, /* coordinate set 1: left/top-ish anchors */
        {34, 50, 75}  /* coordinate set 2: right/bottom anchors */
    };
    static const int kAnchorY[3][3] = {
        {13, 17, 22},
        {23, 35, 53},
        {37, 53, 80}
    };
    int depth;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (depth = 0; depth < 3; ++depth) {
        const M11_ViewportCell* cell = &cells[depth][1];
        const M11_AssetSlot* slot;
        int graphicIndex;
        int coordSet;
        int panelState;
        M11_DM1ZoneBlit panel;
        int scale = (depth == 0) ? 32 : ((depth == 1) ? 21 : 14);
        int viewIndex = (depth == 0) ? 2 : ((depth == 1) ? 1 : 0);
        int ornW, ornH, relX, relY;
        if (!cell->valid || cell->elementType != DUNGEON_ELEMENT_DOOR ||
            m11_viewport_cell_is_open(cell) || cell->doorOrnamentOrdinal <= 0) {
            continue;
        }
        if (!m11_dm1_door_ornament_info(state, cell->doorOrnamentOrdinal,
                                        &graphicIndex, &coordSet)) {
            continue;
        }
        slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                    (unsigned int)graphicIndex);
        if (!slot || slot->width <= 0 || slot->height <= 0) {
            continue;
        }
        if (coordSet < 0 || coordSet > 2) {
            coordSet = 1;
        }
        ornW = m11_dm1_scaled_dimension(slot->width, scale);
        ornH = m11_dm1_scaled_dimension(slot->height, scale);
        if (ornW <= 0 || ornH <= 0) {
            continue;
        }
        panelState = (cell->doorState >= 1 && cell->doorState <= 3) ? cell->doorState : 0;
        panel = kDoorPanels[depth][panelState];
        if (coordSet == 0) {
            relX = kAnchorX[coordSet][viewIndex] - ornW;
            relY = kAnchorY[coordSet][viewIndex];
        } else if (coordSet == 1) {
            relX = kAnchorX[coordSet][viewIndex];
            relY = kAnchorY[coordSet][viewIndex] - ornH;
        } else {
            relX = kAnchorX[coordSet][viewIndex] - ornW;
            relY = kAnchorY[coordSet][viewIndex] - ornH;
        }
        M11_AssetLoader_BlitScaled(slot,
                                   framebuffer, fbW, fbH,
                                   M11_VIEWPORT_X + panel.dstX + relX,
                                   M11_VIEWPORT_Y + panel.dstY + relY,
                                   ornW, ornH,
                                   9);
        break;
    }
}

static void m11_draw_dm1_center_destroyed_door_masks(const M11_GameViewState* state,
                                                    unsigned char* framebuffer,
                                                    int fbW,
                                                    int fbH,
                                                    const M11_ViewportCell cells[3][3]) {
    static const M11_DM1ZoneBlit kDoorPanels[3] = {
        {M11_GFX_DOOR_SET0_D1, 0, 0, 64, 16, 96, 86},
        {M11_GFX_DOOR_SET0_D2, 0, 0, 80, 24, 64, 59},
        {M11_GFX_DOOR_SET0_D3, 0, 0, 90, 30, 44, 38}
    };
    int depth;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (depth = 0; depth < 3; ++depth) {
        const M11_ViewportCell* cell = &cells[depth][1];
        if (!cell->valid || cell->elementType != DUNGEON_ELEMENT_DOOR ||
            cell->doorState != 5) {
            continue;
        }
        m11_draw_dm1_destroyed_door_mask_on_panel(state, framebuffer, fbW, fbH,
                                                  &kDoorPanels[depth]);
        break;
    }
}

static void m11_draw_dm1_center_door_buttons(const M11_GameViewState* state,
                                             unsigned char* framebuffer,
                                             int fbW,
                                             int fbH,
                                             const M11_ViewportCell cells[3][3]) {
    static const unsigned char kButtonD3Palette[16] = {
        0, 0, 12, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 1, 0, 2
    };
    static const unsigned char kButtonD2Palette[16] = {
        0, 12, 1, 3, 4, 3, 6, 7, 5, 9, 10, 11, 0, 2, 14, 13
    };
    static const M11_DM1ZoneBlit kButtons[3] = {
        /* C1950_ZONE_DOOR_BUTTON + C3_VIEW_DOOR_BUTTON_D1C */
        {M11_GFX_DOOR_BUTTON_BASE, 0, 0, 167, 43, 8, 9},
        /* D2 uses 20/32 scaled derived bitmap. */
        {M11_GFX_DOOR_BUTTON_BASE, 0, 0, 150, 42, 5, 5},
        /* D3 uses 16/32 scaled derived bitmap. */
        {M11_GFX_DOOR_BUTTON_BASE, 0, 0, 137, 41, 4, 4}
    };
    int depth;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (depth = 0; depth < 3; ++depth) {
        const M11_ViewportCell* cell = &cells[depth][1];
        const M11_AssetSlot* slot;
        if (!cell->valid || cell->elementType != DUNGEON_ELEMENT_DOOR ||
            m11_viewport_cell_is_open(cell) || !cell->hasDoorThing) {
            continue;
        }
        if (!state->world.things || !state->world.things->doors) {
            continue;
        }
        {
            int doorIdx = THING_GET_INDEX(cell->firstThing);
            if (doorIdx < 0 || doorIdx >= state->world.things->doorCount ||
                !state->world.things->doors[doorIdx].button) {
                continue;
            }
        }
        slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                    M11_GFX_DOOR_BUTTON_BASE);
        if (!slot || slot->width <= 0 || slot->height <= 0) {
            continue;
        }
        m11_blit_scaled_palette_map(slot,
                                    framebuffer, fbW, fbH,
                                    M11_VIEWPORT_X + kButtons[depth].dstX,
                                    M11_VIEWPORT_Y + kButtons[depth].dstY,
                                    kButtons[depth].width,
                                    kButtons[depth].height,
                                    10,
                                    depth == 2 ? kButtonD3Palette :
                                        (depth == 1 ? kButtonD2Palette : NULL));
        break;
    }
}

static void m11_draw_dm1_d3r_door_button(const M11_GameViewState* state,
                                         unsigned char* framebuffer,
                                         int fbW,
                                         int fbH) {
    static const unsigned char kButtonD3Palette[16] = {
        0, 0, 12, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 1, 0, 2
    };
    M11_ViewportCell cell;
    const M11_AssetSlot* slot;
    if (!state || !state->assetsAvailable) {
        return;
    }
    if (!m11_sample_viewport_cell(state, 3, 1, &cell)) {
        return;
    }
    if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_DOOR ||
        m11_viewport_cell_is_open(&cell) || !cell.hasDoorThing ||
        !state->world.things || !state->world.things->doors) {
        return;
    }
    {
        int doorIdx = THING_GET_INDEX(cell.firstThing);
        if (doorIdx < 0 || doorIdx >= state->world.things->doorCount ||
            !state->world.things->doors[doorIdx].button) {
            return;
        }
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                M11_GFX_DOOR_BUTTON_BASE);
    if (!slot || slot->width <= 0 || slot->height <= 0) {
        return;
    }
    /* C1950_ZONE_DOOR_BUTTON + C0_VIEW_DOOR_BUTTON_D3R, scaled 16/32. */
    m11_blit_scaled_palette_map(slot,
                                framebuffer, fbW, fbH,
                                M11_VIEWPORT_X + 197,
                                M11_VIEWPORT_Y + 39,
                                4, 4,
                                10,
                                kButtonD3Palette);
}

typedef struct M11_DM1SideDoorSpec {
    int relForward;
    int relSide;
    int depthIndex;
    M11_DM1ZoneBlit panel;
    M11_DM1ZoneBlit frameA;
    M11_DM1ZoneBlit frameB;
    int frameCount;
} M11_DM1SideDoorSpec;

static void m11_draw_dm1_side_doors(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH) {
    static const M11_DM1SideDoorSpec kSpecs[] = {
        /* D3L2/R2 doors have only the clipped door panel in DUNVIEW.C. */
        {3, -2, 2, {M11_GFX_DOOR_SET0_D3, 35, 0, 0,   28, 9,  38}, {0}, {0}, 0},
        {3,  2, 2, {M11_GFX_DOOR_SET0_D3, 0,  0, 210, 28, 14, 38}, {0}, {0}, 0},
        /* D3L/R side doors: side frame pair + clipped D3 panel. */
        {3, -1, 2, {M11_GFX_DOOR_SET0_D3, 1,  0, 30,  29, 43, 38},
                   {M11_GFX_DOOR_FRAME_D3W, 0, 0, 16, 27, 16, 43},
                   {M11_GFX_DOOR_FRAME_D3W, 0, 0, 73, 27, 16, 43}, 2},
        {3,  1, 2, {M11_GFX_DOOR_SET0_D3, 0,  0, 151, 29, 43, 38},
                   {M11_GFX_DOOR_FRAME_D3W, 0, 0, 147, 27, 16, 43},
                   {M11_GFX_DOOR_FRAME_D3W, 0, 0, 192, 26, 16, 43}, 2},
        /* D2L/R: top frame + clipped D2 panel. */
        {2, -1, 1, {M11_GFX_DOOR_SET0_D2, 4, 0, 0,   24, 60, 59},
                   {M11_GFX_DOOR_FRAME_TOP_D2, 0, 0, 6, 22, 70, 3}, {0}, 1},
        {2,  1, 1, {M11_GFX_DOOR_SET0_D2, 0, 0, 164, 23, 60, 59},
                   {M11_GFX_DOOR_FRAME_TOP_D2, 0, 0, 160, 22, 64, 3}, {0}, 1},
        /* D1L/R: top frame + clipped D1 panel. */
        {1, -1, 0, {M11_GFX_DOOR_SET0_D1, 64, 0, 0,   18, 32, 86},
                   {M11_GFX_DOOR_FRAME_TOP_D1, 0, 0, 0, 14, 102, 4}, {0}, 1},
        {1,  1, 0, {M11_GFX_DOOR_SET0_D1, 0,  0, 192, 18, 32, 86},
                   {M11_GFX_DOOR_FRAME_TOP_D1, 0, 0, 122, 14, 102, 4}, {0}, 1}
    };
    static const int kDoorOpenSrcY[3][4] = {
        {0, 65, 43, 21}, /* D1 */
        {0, 44, 29, 14}, /* D2 */
        {0, 27, 17, 7}   /* D3 */
    };
    static const int kDoorOpenHeight[3][4] = {
        {0, 23, 45, 67}, /* D1 */
        {0, 17, 32, 47}, /* D2 */
        {0, 11, 21, 31}  /* D3 */
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kSpecs) / sizeof(kSpecs[0]); ++i) {
        M11_ViewportCell cell;
        M11_DM1ZoneBlit panel;
        int panelGraphic;
        if (!m11_sample_viewport_cell(state, kSpecs[i].relForward, kSpecs[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_DOOR ||
            m11_viewport_cell_is_open(&cell)) {
            continue;
        }
        if (kSpecs[i].frameCount >= 1) {
            (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH,
                                         &kSpecs[i].frameA, 10);
        }
        if (kSpecs[i].frameCount >= 2) {
            (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH,
                                         &kSpecs[i].frameB, 10);
        }
        panel = kSpecs[i].panel;
        panelGraphic = m11_dm1_door_panel_graphic(state, &cell, kSpecs[i].depthIndex);
        if (panelGraphic >= 0) {
            panel.graphicIndex = panelGraphic;
        }
        if (cell.doorState >= 1 && cell.doorState <= 3) {
            panel.srcY = kDoorOpenSrcY[kSpecs[i].depthIndex][cell.doorState];
            panel.height = kDoorOpenHeight[kSpecs[i].depthIndex][cell.doorState];
            if (kSpecs[i].depthIndex == 0) {
                panel.dstY = 17;
            } else if (kSpecs[i].depthIndex == 2) {
                panel.dstY = (kSpecs[i].relSide == -1 || kSpecs[i].relSide == 1) ? 29 : 28;
                if (kSpecs[i].relSide == -1) {
                    panel.srcX = 0;
                    panel.dstX = 30;
                    panel.width = 44;
                } else if (kSpecs[i].relSide == 1) {
                    panel.srcX = 0;
                    panel.dstX = 150;
                    panel.width = 44;
                }
            }
        }
        (void)m11_draw_dm1_zone_blit(state, framebuffer, fbW, fbH, &panel, 10);
    }
}

static void m11_draw_dm1_side_door_ornaments(const M11_GameViewState* state,
                                             unsigned char* framebuffer,
                                             int fbW,
                                             int fbH) {
    static const M11_DM1SideDoorSpec kSpecs[] = {
        {3, -2, 2, {M11_GFX_DOOR_SET0_D3, 35, 0, 0,   28, 9,  38}, {0}, {0}, 0},
        {3,  2, 2, {M11_GFX_DOOR_SET0_D3, 0,  0, 210, 28, 14, 38}, {0}, {0}, 0},
        {3, -1, 2, {M11_GFX_DOOR_SET0_D3, 1,  0, 30,  29, 43, 38}, {0}, {0}, 0},
        {3,  1, 2, {M11_GFX_DOOR_SET0_D3, 0,  0, 151, 29, 43, 38}, {0}, {0}, 0},
        {2, -1, 1, {M11_GFX_DOOR_SET0_D2, 4, 0, 0,   24, 60, 59}, {0}, {0}, 0},
        {2,  1, 1, {M11_GFX_DOOR_SET0_D2, 0, 0, 164, 23, 60, 59}, {0}, {0}, 0},
        {1, -1, 0, {M11_GFX_DOOR_SET0_D1, 64, 0, 0,   18, 32, 86}, {0}, {0}, 0},
        {1,  1, 0, {M11_GFX_DOOR_SET0_D1, 0,  0, 192, 18, 32, 86}, {0}, {0}, 0}
    };
    static const int kDoorOpenSrcY[3][4] = {
        {0, 65, 43, 21},
        {0, 44, 29, 14},
        {0, 27, 17, 7}
    };
    static const int kDoorOpenHeight[3][4] = {
        {0, 23, 45, 67},
        {0, 17, 32, 47},
        {0, 11, 21, 31}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kSpecs) / sizeof(kSpecs[0]); ++i) {
        M11_ViewportCell cell;
        M11_DM1ZoneBlit panel;
        int panelGraphic;
        if (!m11_sample_viewport_cell(state, kSpecs[i].relForward, kSpecs[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_DOOR ||
            m11_viewport_cell_is_open(&cell) || cell.doorOrnamentOrdinal <= 0) {
            continue;
        }
        panel = kSpecs[i].panel;
        panelGraphic = m11_dm1_door_panel_graphic(state, &cell, kSpecs[i].depthIndex);
        if (panelGraphic >= 0) {
            panel.graphicIndex = panelGraphic;
        }
        if (cell.doorState >= 1 && cell.doorState <= 3) {
            panel.srcY = kDoorOpenSrcY[kSpecs[i].depthIndex][cell.doorState];
            panel.height = kDoorOpenHeight[kSpecs[i].depthIndex][cell.doorState];
            if (kSpecs[i].depthIndex == 0) {
                panel.dstY = 17;
            } else if (kSpecs[i].depthIndex == 2) {
                panel.dstY = (kSpecs[i].relSide == -1 || kSpecs[i].relSide == 1) ? 29 : 28;
                if (kSpecs[i].relSide == -1) {
                    panel.srcX = 0;
                    panel.dstX = 30;
                    panel.width = 44;
                } else if (kSpecs[i].relSide == 1) {
                    panel.srcX = 0;
                    panel.dstX = 150;
                    panel.width = 44;
                }
            }
        }
        m11_draw_dm1_door_ornament_on_panel(state, framebuffer, fbW, fbH,
                                            &panel, kSpecs[i].depthIndex,
                                            cell.doorOrnamentOrdinal);
    }
}

static void m11_draw_dm1_side_destroyed_door_masks(const M11_GameViewState* state,
                                                   unsigned char* framebuffer,
                                                   int fbW,
                                                   int fbH) {
    static const M11_DM1SideDoorSpec kSpecs[] = {
        {3, -2, 2, {M11_GFX_DOOR_SET0_D3, 35, 0, 0,   28, 9,  38}, {0}, {0}, 0},
        {3,  2, 2, {M11_GFX_DOOR_SET0_D3, 0,  0, 210, 28, 14, 38}, {0}, {0}, 0},
        {3, -1, 2, {M11_GFX_DOOR_SET0_D3, 1,  0, 30,  29, 43, 38}, {0}, {0}, 0},
        {3,  1, 2, {M11_GFX_DOOR_SET0_D3, 0,  0, 151, 29, 43, 38}, {0}, {0}, 0},
        {2, -1, 1, {M11_GFX_DOOR_SET0_D2, 4, 0, 0,   24, 60, 59}, {0}, {0}, 0},
        {2,  1, 1, {M11_GFX_DOOR_SET0_D2, 0, 0, 164, 23, 60, 59}, {0}, {0}, 0},
        {1, -1, 0, {M11_GFX_DOOR_SET0_D1, 64, 0, 0,   18, 32, 86}, {0}, {0}, 0},
        {1,  1, 0, {M11_GFX_DOOR_SET0_D1, 0,  0, 192, 18, 32, 86}, {0}, {0}, 0}
    };
    size_t i;
    if (!state || !state->assetsAvailable) {
        return;
    }
    for (i = 0; i < sizeof(kSpecs) / sizeof(kSpecs[0]); ++i) {
        M11_ViewportCell cell;
        M11_DM1ZoneBlit panel;
        int panelGraphic;
        if (!m11_sample_viewport_cell(state, kSpecs[i].relForward, kSpecs[i].relSide, &cell)) {
            continue;
        }
        if (!cell.valid || cell.elementType != DUNGEON_ELEMENT_DOOR || cell.doorState != 5) {
            continue;
        }
        panel = kSpecs[i].panel;
        panelGraphic = m11_dm1_door_panel_graphic(state, &cell, kSpecs[i].depthIndex);
        if (panelGraphic >= 0) {
            panel.graphicIndex = panelGraphic;
        }
        m11_draw_dm1_destroyed_door_mask_on_panel(state, framebuffer, fbW, fbH, &panel);
    }
}

/* Draw a real door frame from GRAPHICS.DAT at the given depth.
 * depthIndex 0 = nearest, 2 = farthest.
 * Falls back to the primitive line-based door rendering if unavailable. */
static int m11_draw_door_frame_asset(const M11_GameViewState* state,
                                     unsigned char* framebuffer,
                                     int fbW,
                                     int fbH,
                                     const M11_ViewRect* rect,
                                     int depthIndex) {
    unsigned int doorIdx;
    const M11_AssetSlot* slot;
    if (!state->assetsAvailable) return 0;
    /* Select the depth-appropriate door frame graphic */
    switch (depthIndex) {
        case 0: doorIdx = M11_GFX_DOOR_FRAME_D1; break;
        case 1: doorIdx = M11_GFX_DOOR_FRAME_D2; break;
        case 2: doorIdx = M11_GFX_DOOR_FRAME_D3; break;
        default: return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, doorIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    /* Center the door frame graphic in the face rect */
    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               rect->x + 2, rect->y + 2,
                               rect->w - 4, rect->h - 4, 0);
    return 1;
}

/* Draw door side pillars from GRAPHICS.DAT. */
static int m11_draw_door_side_asset(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH,
                                    int x,
                                    int y,
                                    int w,
                                    int h,
                                    int depthIndex) {
    unsigned int sideIdx;
    const M11_AssetSlot* slot;
    if (!state->assetsAvailable || w < 4 || h < 8) return 0;
    switch (depthIndex) {
        case 0: sideIdx = M11_GFX_DOOR_SIDE_D0; break;
        case 1: sideIdx = M11_GFX_DOOR_SIDE_D1; break;
        case 2: sideIdx = M11_GFX_DOOR_SIDE_D2; break;
        default: return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, sideIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               x, y, w, h, 0);
    return 1;
}

/* Map an item thing type + subtype to a GRAPHICS.DAT graphic index.
 * Returns the graphic index, or 0 if no mapping exists. */
static unsigned int m11_item_sprite_index(int thingType, int subtype) {
    static const unsigned char kObjectInfoAspect[180] = {
        1,0,67,67,67,67,67,67,2,2,2,2,2,2,2,2,2,2,68,68,
        68,68,80,38,38,35,37,11,12,12,39,17,12,12,12,12,12,12,12,42,
        12,13,13,21,21,33,43,44,14,45,16,46,11,47,48,49,50,11,31,31,
        11,11,11,51,32,30,65,45,82,23,23,23,55,8,24,24,24,24,69,24,
        24,69,7,7,57,23,23,29,69,69,24,24,53,53,9,9,9,54,54,10,
        54,19,19,19,19,9,19,52,20,22,56,10,52,20,22,56,10,52,20,22,
        56,10,52,19,22,81,84,34,6,15,15,40,41,4,83,4,18,18,18,18,
        18,18,18,18,62,62,62,62,62,62,62,62,76,3,60,61,27,28,25,26,
        71,70,5,66,15,15,58,59,59,79,63,64,72,73,74,75,77,78,74,41
    };
    static const unsigned char kObjectAspectFirstNative[85] = {
         0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
        49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
        65, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 82,
        84, 85, 86, 87, 88
    };
    int objectInfoIndex;
    int aspectIndex;
    if (subtype < 0) subtype = 0;
    switch (thingType) {
        case THING_TYPE_WEAPON:
            if (subtype > 45) subtype = 0;
            objectInfoIndex = 23 + subtype;
            break;
        case THING_TYPE_ARMOUR:
            if (subtype > 57) subtype = 0;
            objectInfoIndex = 69 + subtype;
            break;
        case THING_TYPE_SCROLL:
            objectInfoIndex = 0;
            break;
        case THING_TYPE_POTION:
            if (subtype > 20) subtype = 0;
            objectInfoIndex = 2 + subtype;
            break;
        case THING_TYPE_CONTAINER:
            if (subtype > 0) subtype = 0;
            objectInfoIndex = 1 + subtype;
            break;
        case THING_TYPE_JUNK:
            if (subtype > 52) subtype = 0;
            objectInfoIndex = 127 + subtype;
            break;
        default:
            return 0;
    }
    if (objectInfoIndex < 0 || objectInfoIndex >= 180) return 0;
    aspectIndex = kObjectInfoAspect[objectInfoIndex];
    if (aspectIndex < 0 || aspectIndex >= 85) return 0;
    /* G0209_as_Graphic558_ObjectAspects[].FirstNativeBitmapRelativeIndex
     * has gaps for aspects with alcove/right-facing alternates; do not
     * synthesize it as aspectIndex+1. */
    return M11_GFX_ITEM_SPRITE_BASE + (unsigned int)kObjectAspectFirstNative[aspectIndex];
}

static int m11_item_aspect_index(int thingType, int subtype) {
    static const unsigned char kObjectInfoAspect[180] = {
        1,0,67,67,67,67,67,67,2,2,2,2,2,2,2,2,2,2,68,68,
        68,68,80,38,38,35,37,11,12,12,39,17,12,12,12,12,12,12,12,42,
        12,13,13,21,21,33,43,44,14,45,16,46,11,47,48,49,50,11,31,31,
        11,11,11,51,32,30,65,45,82,23,23,23,55,8,24,24,24,24,69,24,
        24,69,7,7,57,23,23,29,69,69,24,24,53,53,9,9,9,54,54,10,
        54,19,19,19,19,9,19,52,20,22,56,10,52,20,22,56,10,52,20,22,
        56,10,52,19,22,81,84,34,6,15,15,40,41,4,83,4,18,18,18,18,
        18,18,18,18,62,62,62,62,62,62,62,62,76,3,60,61,27,28,25,26,
        71,70,5,66,15,15,58,59,59,79,63,64,72,73,74,75,77,78,74,41
    };
    int objectInfoIndex;
    if (subtype < 0) subtype = 0;
    switch (thingType) {
        case THING_TYPE_WEAPON:
            if (subtype > 45) subtype = 0;
            objectInfoIndex = 23 + subtype;
            break;
        case THING_TYPE_ARMOUR:
            if (subtype > 57) subtype = 0;
            objectInfoIndex = 69 + subtype;
            break;
        case THING_TYPE_SCROLL:
            objectInfoIndex = 0;
            break;
        case THING_TYPE_POTION:
            if (subtype > 20) subtype = 0;
            objectInfoIndex = 2 + subtype;
            break;
        case THING_TYPE_CONTAINER:
            if (subtype > 0) subtype = 0;
            objectInfoIndex = 1 + subtype;
            break;
        case THING_TYPE_JUNK:
            if (subtype > 52) subtype = 0;
            objectInfoIndex = 127 + subtype;
            break;
        default:
            return -1;
    }
    if (objectInfoIndex < 0 || objectInfoIndex >= 180) return -1;
    return (int)kObjectInfoAspect[objectInfoIndex];
}

static int m11_object_source_scale_units(int scaleIndex) {
    /* DUNVIEW.C G2030_auc_ObjectScales: source object scale units for
     * the five distance/cell scale buckets used by F0115. */
    static const unsigned char kObjectScales[5] = {27, 21, 18, 14, 12};
    if (scaleIndex < 0) scaleIndex = 0;
    if (scaleIndex > 4) scaleIndex = 4;
    return kObjectScales[scaleIndex];
}

static int m11_object_source_scale_index(int depthIndex, int relativeCell) {
    /* F0115 object path:
     *   D1/native uses scale bucket 0.
     *   deeper rows use (viewDepth * 2) - 1 - (viewCell >> 1).
     * relativeCell 0/1 = back row, 2/3 = front row. */
    int frontRow = relativeCell >= 2;
    int idx;
    if (depthIndex <= 0) return 0;
    idx = depthIndex * 2 - (frontRow ? 1 : 0);
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return idx;
}

static void m11_object_source_pile_shift_indices(int pileIndex,
                                                 int* outXIndex,
                                                 int* outYIndex) {
    /* DUNVIEW.C G0217_aauc_Graphic558_ObjectPileShiftSetIndices. */
    static const unsigned char kPileShiftIndices[16][2] = {
        {2,5}, {0,6}, {5,7}, {3,0},
        {7,1}, {1,2}, {6,3}, {3,3},
        {5,5}, {2,6}, {7,7}, {1,0},
        {3,1}, {6,2}, {1,3}, {5,3}
    };
    if (pileIndex < 0) pileIndex = 0;
    pileIndex &= 0x0F;
    if (outXIndex) *outXIndex = (int)kPileShiftIndices[pileIndex][0];
    if (outYIndex) *outYIndex = (int)kPileShiftIndices[pileIndex][1];
}

static int m11_object_source_shift_value(int shiftSet, int shiftIndex) {
    /* DUNVIEW.C G0223_aac_Graphic558_ShiftSets. */
    static const signed char kShiftSets[3][8] = {
        { 0, 1, 2, 3, 0,-3,-2,-1},
        { 0, 1, 1, 2, 0,-2,-1,-1},
        { 0, 1, 1, 1, 0,-1,-1,-1}
    };
    if (shiftSet < 0) shiftSet = 0;
    if (shiftSet > 2) shiftSet = 2;
    if (shiftIndex < 0) shiftIndex = 0;
    if (shiftIndex > 7) shiftIndex = 7;
    return (int)kShiftSets[shiftSet][shiftIndex];
}

static int m11_c2500_object_zone_point(int scaleIndex,
                                       int relativeCell,
                                       int* outX,
                                       int* outY) {
    /* Layout-696 C2500_ZONE_ table used by DUNVIEW.C F0115 for
     * non-alcove object/creature placement with
     * MASK0x8000_SHIFT_OBJECTS_AND_CREATURES.  Five scale buckets
     * (G2030) × four view cells.  Entries whose source coordinates are
     * 0,0 are intentionally unusable for that distance/cell. */
    static const short kC2500[5][4][2] = {
        {{  0,  0}, {  0,  0}, {127, 70}, { 98, 70}},
        {{  0,  0}, {  0,  0}, { 62, 70}, { 25, 70}},
        {{  0,  0}, {  0,  0}, {200, 70}, {162, 70}},
        {{  0,  0}, {  0,  0}, {  2, 70}, {-35, 70}},
        {{  0,  0}, {  0,  0}, {258, 70}, {222, 70}}
    };
    int zx;
    int zy;
    if (scaleIndex < 0) scaleIndex = 0;
    if (scaleIndex > 4) scaleIndex = 4;
    if (relativeCell < 0 || relativeCell > 3) return 0;
    zx = (int)kC2500[scaleIndex][relativeCell][0];
    zy = (int)kC2500[scaleIndex][relativeCell][1];
    if (zx == 0 && zy == 0) return 0;
    if (outX) *outX = zx;
    if (outY) *outY = zy;
    return 1;
}

static unsigned int m11_object_aspect_graphic_info(int aspectIndex) {
    /* DUNVIEW.C G0209_as_Graphic558_ObjectAspects[].GraphicInfo. */
    static const unsigned char kGraphicInfo[85] = {
        0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00
    };
    if (aspectIndex < 0 || aspectIndex >= 85) return 0u;
    return (unsigned int)kGraphicInfo[aspectIndex];
}

static int m11_object_aspect_coordinate_set(int aspectIndex) {
    /* DUNVIEW.C G0209_as_Graphic558_ObjectAspects[].CoordinateSet. */
    static const unsigned char kCoordinateSet[85] = {
        0,1,1,1,1,1,0,0,0,1,0,1,1,0,2,1,
        1,0,1,2,2,1,2,0,0,1,1,1,1,0,1,1,
        1,0,1,1,0,0,1,1,0,0,1,1,0,2,1,1,
        1,1,1,0,0,1,0,0,0,0,1,1,1,1,1,1,
        1,0,1,1,1,0,0,0,0,1,1,1,0,1,1,1,
        1,0,1,1,0
    };
    if (aspectIndex < 0 || aspectIndex >= 85) return 0;
    return (int)kCoordinateSet[aspectIndex];
}

static int m11_creature_source_palette_change(int depthPaletteIndex,
                                              int paletteIndex) {
    /* DUNVIEW.C G0221/G0222 creature palette-change tables for D3/D2
     * derived bitmaps. depthPaletteIndex 0=D3, 1=D2. */
    static const unsigned char kCreaturePaletteD3[16] = {
        0,12,1,3,4,3,0,6,3,0,0,11,0,2,0,13
    };
    static const unsigned char kCreaturePaletteD2[16] = {
        0,1,2,3,4,3,6,7,5,0,0,11,12,13,14,15
    };
    if (paletteIndex < 0) paletteIndex = 0;
    if (paletteIndex > 15) paletteIndex = 15;
    return depthPaletteIndex == 0 ? (int)kCreaturePaletteD3[paletteIndex]
                                  : (int)kCreaturePaletteD2[paletteIndex];
}

/* Resolve an inventory thingId to a GRAPHICS.DAT item sprite index.
 * Returns the graphic index suitable for M11_AssetLoader_Load, or 0
 * if the thing type/subtype cannot be mapped.  This is the inventory
 * counterpart of the viewport floor-item m11_item_sprite_index(). */
static unsigned int m11_inventory_thing_sprite_index(
    const struct DungeonThings_Compat* things,
    unsigned short thingId) {
    int thingType, thingIndex, subtype;
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) return 0;
    thingType  = THING_GET_TYPE(thingId);
    thingIndex = THING_GET_INDEX(thingId);
    subtype = 0;
    switch (thingType) {
        case THING_TYPE_WEAPON:
            if (things->weapons && thingIndex >= 0 && thingIndex < things->weaponCount)
                subtype = things->weapons[thingIndex].type;
            break;
        case THING_TYPE_ARMOUR:
            if (things->armours && thingIndex >= 0 && thingIndex < things->armourCount)
                subtype = things->armours[thingIndex].type;
            break;
        case THING_TYPE_SCROLL:
            return M11_GFX_ITEM_SCROLL_BASE;
        case THING_TYPE_POTION:
            if (things->potions && thingIndex >= 0 && thingIndex < things->potionCount)
                subtype = things->potions[thingIndex].type;
            break;
        case THING_TYPE_CONTAINER:
            if (things->containers && thingIndex >= 0 && thingIndex < things->containerCount)
                subtype = things->containers[thingIndex].type;
            break;
        case THING_TYPE_JUNK:
            if (things->junks && thingIndex >= 0 && thingIndex < things->junkCount)
                subtype = things->junks[thingIndex].type;
            break;
        default:
            return 0;
    }
    return m11_item_sprite_index(thingType, subtype);
}

/* Draw an item sprite from GRAPHICS.DAT at the given viewport position.
 * Falls back to the primitive item cue if the asset is unavailable.
 * Returns 1 if a real sprite was drawn. */
static int m11_draw_item_sprite(const M11_GameViewState* state,
                                unsigned char* framebuffer,
                                int fbW,
                                int fbH,
                                int x,
                                int y,
                                int w,
                                int h,
                                int thingType,
                                int subtype,
                                int relativeCell,
                                int pileIndex,
                                int depthIndex) {
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int spriteW, spriteH;
    int drawW, drawH, drawX, drawY;
    int scaleIndex;
    int shiftSet;
    int shiftXIndex = 0;
    int shiftYIndex = 0;
    int aspectIndex;
    int useMirror;

    if (!state || !state->assetsAvailable || thingType < 0) return 0;
    gfxIdx = m11_item_sprite_index(thingType, subtype);
    if (gfxIdx == 0 || gfxIdx >= M11_GFX_ITEM_SPRITE_END) return 0;
    aspectIndex = m11_item_aspect_index(thingType, subtype);
    useMirror = (aspectIndex >= 0 &&
                 (m11_object_aspect_graphic_info(aspectIndex) & 0x0001u) &&
                 (relativeCell == 1 || relativeCell == 3)) ? 1 : 0;

    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    spriteW = (int)slot->width;
    spriteH = (int)slot->height;

    /* Source-derived object scaling. DM1 uses G2030 units out of 32 for
     * object scale buckets; this keeps the current center-cell renderer
     * aligned with the future C2500 zone pass instead of using invented
     * depth percentages. */
    scaleIndex = m11_object_source_scale_index(depthIndex, relativeCell);
    drawW = (int)slot->width * m11_object_source_scale_units(scaleIndex) / 32;
    drawH = (drawW * spriteH) / spriteW;
    if (drawH > h) {
        drawH = h;
        drawW = (drawH * spriteW) / spriteH;
    }
    if (drawW > w) {
        drawW = w;
        drawH = (drawW * spriteH) / spriteW;
    }
    if (drawW < 3 || drawH < 3) return 0;

    /* Source-derived pile shift. F0115 uses G0217 to pick X/Y shift
     * indices and G0223 to turn those indices into pixel offsets for the
     * current distance bucket. We still draw into the temporary center
     * rectangle, but the scatter order now follows the DM1 table. */
    {
        int halfW = (w - drawW) / 2;
        int cellX = (relativeCell == 1 || relativeCell == 3) ? (w / 6) : -(w / 6);
        int cellY = (relativeCell >= 2) ? 2 : -2;
        int zoneX = 0;
        int zoneY = 0;
        shiftSet = (scaleIndex + 1) >> 1;
        if (shiftSet > 2) shiftSet = 2;
        m11_object_source_pile_shift_indices(pileIndex, &shiftXIndex, &shiftYIndex);
        if (x >= M11_VIEWPORT_X && y >= M11_VIEWPORT_Y &&
            m11_c2500_object_zone_point(scaleIndex, relativeCell, &zoneX, &zoneY)) {
            drawX = M11_VIEWPORT_X + zoneX - (drawW / 2) +
                    m11_object_source_shift_value(shiftSet, shiftXIndex);
            drawY = M11_VIEWPORT_Y + zoneY - drawH +
                    m11_object_source_shift_value(shiftSet, shiftYIndex);
        } else {
            drawX = x + halfW + cellX + m11_object_source_shift_value(shiftSet, shiftXIndex);
            drawY = y + h - drawH - 2 + cellY + m11_object_source_shift_value(shiftSet, shiftYIndex);
        }
        if (drawX < x) drawX = x;
        if (drawY < y) drawY = y;
        if (drawX + drawW > x + w) drawX = x + w - drawW;
        if (drawY + drawH > y + h) drawY = y + h - drawH;
    }

    if (useMirror) {
        M11_AssetLoader_BlitScaledMirror(slot, framebuffer, fbW, fbH,
                                         drawX, drawY, drawW, drawH, 10);
    } else {
        M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                                   drawX, drawY, drawW, drawH, 10);
    }
    return 1;
}

/* Draw a wall ornament from GRAPHICS.DAT on the wall face.
 * Uses the map's wall set and the cell's ornament ordinal.
 * Returns 1 if a real ornament was drawn. */
static int m11_draw_wall_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW,
                                  int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex) {
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int ornW, ornH, drawX, drawY;
    int mapIdx;
    int ornGlobalIdx;

    if (!state || !state->assetsAvailable || ornamentOrdinal < 0) return 0;
    if (!state->world.dungeon) return 0;

    mapIdx = state->world.party.mapIndex;

    /* Ensure the per-map ornament index cache is loaded.
     * Cast away const: the cache is a lazy-init optimization that
     * doesn't change observable game state. */
    m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);

    /* Resolve the per-map ordinal to a global ornament index via
     * the cached G0261 table.  Fall back to ordinal if the cache
     * couldn't be loaded (identity mapping). */
    if (mapIdx >= 0 && mapIdx < (int)32 &&
        state->ornamentCacheLoaded[mapIdx] &&
        ornamentOrdinal < 16) {
        ornGlobalIdx = state->wallOrnamentIndices[mapIdx][ornamentOrdinal];
    } else {
        /* Fallback: direct mapping (old behaviour) */
        int wallSet = 0;
        if (mapIdx >= 0 && mapIdx < (int)state->world.dungeon->header.mapCount) {
            wallSet = (int)state->world.dungeon->maps[mapIdx].wallSet;
        }
        ornGlobalIdx = wallSet * 16 + ornamentOrdinal;
    }
    gfxIdx = (unsigned int)(M11_GFX_WALL_ORNAMENT_BASE +
                            ornGlobalIdx * M11_GFX_WALL_ORNAMENTS_PER_SET);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* DM1-faithful wall ornament depth scaling.
     * The original uses specific perspective-scaled ornament sizes at
     * each depth level.  We approximate these as fractions of the wall
     * face rect, derived from ReDMCSB viewport geometry:
     *   depth 0 (nearest): ornament at ~50% of face
     *   depth 1 (mid):     ornament at ~38% of face
     *   depth 2 (far):     ornament at ~28% of face
     *   depth 3 (farthest): ornament at ~20% of face */
    {
        /* Scale numerator/denominator pairs per depth */
        static const int s_ornScaleNum[4] = { 50, 38, 28, 20 };
        int scaleIdx = depthIndex < 4 ? depthIndex : 3;
        ornW = rect->w * s_ornScaleNum[scaleIdx] / 100;
        ornH = (ornW * (int)slot->height) / (int)slot->width;
        if (ornH > rect->h * s_ornScaleNum[scaleIdx] / 100) {
            ornH = rect->h * s_ornScaleNum[scaleIdx] / 100;
            ornW = (ornH * (int)slot->width) / (int)slot->height;
        }
    }
    if (ornW < 3 || ornH < 3) return 0;

    drawX = rect->x + (rect->w - ornW) / 2;
    drawY = rect->y + (rect->h - ornH) / 2;

    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               drawX, drawY, ornW, ornH, 0);
    return 1;
}

/* Draw a door ornament from GRAPHICS.DAT on the door face.
 * Returns 1 if a real ornament was drawn. */
static int m11_draw_door_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW,
                                  int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex) {
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int ornW, ornH, drawX, drawY;
    int mapIdx;
    int ornGlobalIdx;

    if (!state || !state->assetsAvailable || ornamentOrdinal < 0) return 0;
    if (!state->world.dungeon) return 0;

    mapIdx = state->world.party.mapIndex;

    /* Ensure ornament cache and resolve via per-map door ornament table */
    m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);

    if (mapIdx >= 0 && mapIdx < (int)32 &&
        state->ornamentCacheLoaded[mapIdx] &&
        ornamentOrdinal < 16) {
        ornGlobalIdx = state->doorOrnamentIndices[mapIdx][ornamentOrdinal];
    } else {
        /* Fallback: direct mapping */
        int doorSet = 0;
        if (mapIdx >= 0 && mapIdx < (int)state->world.dungeon->header.mapCount) {
            doorSet = (int)state->world.dungeon->maps[mapIdx].doorSet0;
        }
        ornGlobalIdx = doorSet * M11_GFX_DOOR_ORNAMENTS_PER_SET + ornamentOrdinal;
    }
    gfxIdx = (unsigned int)(M11_GFX_DOOR_ORNAMENT_BASE + ornGlobalIdx);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* DM1-faithful door ornament depth scaling.
     * Door ornaments scale with perspective similarly to wall ornaments
     * but are slightly smaller (centered on the door panel).
     *   depth 0: ~40%, depth 1: ~30%, depth 2: ~22%, depth 3: ~16% */
    {
        static const int s_doorOrnScaleNum[4] = { 40, 30, 22, 16 };
        int scaleIdx = depthIndex < 4 ? depthIndex : 3;
        ornW = rect->w * s_doorOrnScaleNum[scaleIdx] / 100;
        ornH = (ornW * (int)slot->height) / (int)slot->width;
        if (ornH > rect->h * s_doorOrnScaleNum[scaleIdx] / 100) {
            ornH = rect->h * s_doorOrnScaleNum[scaleIdx] / 100;
            ornW = (ornH * (int)slot->width) / (int)slot->height;
        }
    }
    if (ornW < 3 || ornH < 3) return 0;

    drawX = rect->x + (rect->w - ornW) / 2;
    drawY = rect->y + (rect->h - ornH) / 2;

    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               drawX, drawY, ornW, ornH, 0);
    return 1;
}

/* Draw a floor ornament from GRAPHICS.DAT at the given viewport position.
 * In DM1, floor ornaments are drawn on the floor area of open cells
 * (corridor, pit, stairs, teleporter).  Each ornament graphic set has
 * 6 perspective variants indexed by the visible floor position:
 *   0=D3L, 1=D3C, 2=D3R, 3=D2L, 4=D2C, 5=D2R
 * For D1 (depth 0) cells, DM1 uses variant 4 (D2C) scaled up.
 * The global ornament index is resolved via the per-map floor ornament
 * index table (G0261 analog for floor ornaments).
 * Returns 1 if a real ornament was drawn.
 * Ref: ReDMCSB DUNVIEW.C F115 floor ornament rendering path. */
static int m11_draw_floor_ornament(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int fbW,
                                   int fbH,
                                   const M11_ViewRect* rect,
                                   int ornamentOrdinal,
                                   int depthIndex,
                                   int sideHint) {
    static const unsigned char kFloorOrnD3Palette[16] = {
        0, 12, 1, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 2, 14, 13
    };
    static const unsigned char kFloorOrnD2Palette[16] = {
        0, 1, 2, 3, 4, 3, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15
    };
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int ornW, ornH, drawX, drawY;
    int mapIdx;
    int ornGlobalIdx;
    int variant;

    if (!state || !state->assetsAvailable || ornamentOrdinal <= 0) return 0;
    if (!state->world.dungeon) return 0;

    mapIdx = state->world.party.mapIndex;

    /* Ensure the per-map ornament index cache is loaded. */
    m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);

    /* Resolve per-map ordinal (1-based) to global ornament index.
     * ornamentOrdinal - 1 gives the 0-based per-map index. */
    {
        int localIdx = ornamentOrdinal - 1;
        if (mapIdx >= 0 && mapIdx < (int)32 &&
            state->ornamentCacheLoaded[mapIdx] &&
            localIdx >= 0 && localIdx < 16) {
            ornGlobalIdx = state->floorOrnamentIndices[mapIdx][localIdx];
        } else {
            ornGlobalIdx = localIdx; /* fallback: identity mapping */
        }
    }
    if (ornGlobalIdx < 0) return 0;

    /* Select the source native-bitmap increment through
     * G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements:
     *   D3L2/R2/D3L/D3R -> 0, D3C -> 1
     *   D2L/D2R -> 2, D2C -> 3
     *   D1L/D1R -> 4, D1C -> 5
     * The old 0..5 lateral mapping accidentally used object/creature-like
     * variants and pointed at the wrong GRAPHICS.DAT art. */
    if (depthIndex >= 2) {
        variant = (sideHint == 0) ? 1 : 0;
    } else if (depthIndex == 1) {
        variant = (sideHint == 0) ? 3 : 2;
    } else {
        variant = (sideHint == 0) ? 5 : 4;
    }

    gfxIdx = (unsigned int)((ornGlobalIdx == M11_FLOOR_ORNAMENT_FOOTPRINTS_INDEX
                                ? M11_GFX_FLOOR_ORNAMENT_FOOTPRINTS_BASE
                                : M11_GFX_FLOOR_ORNAMENT_BASE +
                                      ornGlobalIdx * M11_GFX_FLOOR_ORNAMENT_VARIANTS) +
                            variant);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* Scale ornament to fit the floor area of the cell rect.
     * Floor ornaments occupy the bottom portion of the cell face.
     *   depth 0: ~60% width, placed in bottom 40% of face
     *   depth 1: ~50% width, placed in bottom 35%
     *   depth 2: ~40% width, placed in bottom 30% */
    {
        static const int s_floorOrnScaleW[3] = { 60, 50, 40 };
        static const int s_floorOrnScaleH[3] = { 40, 35, 30 };
        int scaleIdx = depthIndex < 3 ? depthIndex : 2;
        int maxW = rect->w * s_floorOrnScaleW[scaleIdx] / 100;
        int maxH = rect->h * s_floorOrnScaleH[scaleIdx] / 100;
        ornW = maxW;
        ornH = (ornW * (int)slot->height) / (int)slot->width;
        if (ornH > maxH) {
            ornH = maxH;
            ornW = (ornH * (int)slot->width) / (int)slot->height;
        }
    }
    if (ornW < 3 || ornH < 3) return 0;

    /* Center horizontally in the cell, place at the bottom (floor area) */
    drawX = rect->x + (rect->w - ornW) / 2;
    drawY = rect->y + rect->h - ornH - 2;

    /* Side offset: shift toward the inner corridor edge */
    if (sideHint < 0) drawX += rect->w / 6;
    else if (sideHint > 0) drawX -= rect->w / 6;

    m11_blit_scaled_palette_map(slot, framebuffer, fbW, fbH,
                                drawX, drawY, ornW, ornH, 0,
                                depthIndex >= 2 ? kFloorOrnD3Palette :
                                    (depthIndex == 1 ? kFloorOrnD2Palette : NULL));
    return 1;
}

/* Get the current map's wall set index for wall texture selection.
 * Returns 0 (default set) if dungeon data is unavailable. */
static int m11_current_map_wall_set(const M11_GameViewState* state) {
    if (!state || !state->world.dungeon ||
        state->world.party.mapIndex < 0 ||
        state->world.party.mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    return (int)state->world.dungeon->maps[state->world.party.mapIndex].wallSet;
}

/* Get the current map's floor set index for floor texture selection. */
static int m11_current_map_floor_set(const M11_GameViewState* state) {
    if (!state || !state->world.dungeon ||
        state->world.party.mapIndex < 0 ||
        state->world.party.mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    return (int)state->world.dungeon->maps[state->world.party.mapIndex].floorSet;
}

/* Draw stair graphics from GRAPHICS.DAT at the given depth. */
static int m11_draw_stairs_asset(const M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 int fbW,
                                 int fbH,
                                 const M11_ViewRect* rect,
                                 int depthIndex,
                                 int stairUp) {
    unsigned int stairIdx;
    const M11_AssetSlot* slot;
    if (!state->assetsAvailable) return 0;
    if (depthIndex == 0 || depthIndex == 1) {
        stairIdx = stairUp ? M11_GFX_STAIRS_UP_D1 : M11_GFX_STAIRS_DOWN_D1;
    } else {
        stairIdx = stairUp ? M11_GFX_STAIRS_UP_D2 : M11_GFX_STAIRS_DOWN_D2;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, stairIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               rect->x + 2, rect->y + 2,
                               rect->w - 4, rect->h - 4, 0);
    return 1;
}

/* Map a creature type index (0-26) to a GRAPHICS.DAT sprite set.
 * Returns the base index for the 3-sprite set (small/mid/large),
 * or 0 if no sprite is available for this type.
 * The mapping covers 8 creature graphic sets: 4 at base 246, 4 at base 439.
 * Creature types are mapped round-robin to the available sets. */
/* Forward declaration for extended creature sprite draw. */
static int m11_draw_creature_sprite_ex(const M11_GameViewState* state,
                                       unsigned char* framebuffer,
                                       int fbW, int fbH,
                                       int x, int y, int w, int h,
                                       int creatureType,
                                       int depthIndex,
                                       int sideHint,
                                       int creatureDir);

/* ── DM1 Creature Replacement Color Sets (G0220_as_Graphic558) ──
 * In the original VGA driver, VIDRV_12_SetCreatureReplacementColors
 * remaps palette entries 9 and 10 to creature-specific colors before
 * drawing.  Each set index (0-12) maps to a target palette index for
 * color 9 and another for color 10.
 *
 * Values reconstructed from ReDMCSB VIDEODRV.C and visual matching
 * against original DM1 PC v3.4 screenshots.
 *
 * Index 0 is unused (indicates no replacement).  Indices 1-12 are
 * assigned to creature types via replacementColorSetIndices in the
 * aspect table.
 */
static const unsigned char s_replColor9[13] = {
    0,  /* 0: unused */
    4,  /* 1: GiantScorpion  — dark red / brown */
    2,  /* 2: Giggler        — dark green */
    1,  /* 3: PainRat        — dark blue */
    4,  /* 4: Ruster         — dark red / brown */
    5,  /* 5: GhostRive      — dark magenta */
    3,  /* 6: Couatl         — dark cyan */
    6,  /* 7: Mummy          — brown */
    5,  /* 8: MagentaWorm    — dark magenta */
    6,  /* 9: Trolin         — brown */
    4,  /* 10: Antman        — dark red */
    1,  /* 11: Vexirk        — dark blue */
   12   /* 12: Demon         — light red */
};
static const unsigned char s_replColor10[13] = {
    0,  /* 0: unused */
   14,  /* 1: GiantScorpion  — yellow */
   14,  /* 2: Giggler        — yellow */
   12,  /* 3: PainRat        — light red */
   14,  /* 4: Ruster         — yellow */
   15,  /* 5: GhostRive      — white */
    9,  /* 6: Couatl         — light blue */
   14,  /* 7: Mummy          — yellow */
   13,  /* 8: MagentaWorm    — light magenta */
   14,  /* 9: Trolin         — yellow */
   12,  /* 10: Antman        — light red */
   13,  /* 11: Vexirk        — light magenta */
   14   /* 12: Demon         — yellow */
};

/* Query replacement palette indices for a creature type.
 * Returns 1 if the creature uses replacement colors, 0 if not. */
static int m11_creature_replacement_colors(int creatureType,
                                           int* outReplDst9,
                                           int* outReplDst10) {
    int setIdx9, setIdx10;
    if (creatureType < 0 || creatureType >= 27) return 0;
    setIdx9  = M11_CREATURE_REPL_COLOR9(&s_creatureAspects[creatureType]);
    setIdx10 = M11_CREATURE_REPL_COLOR10(&s_creatureAspects[creatureType]);
    if (setIdx9 == 0 && setIdx10 == 0) return 0; /* no replacement */
    if (outReplDst9) {
        *outReplDst9 = (setIdx9 > 0 && setIdx9 < 13)
                     ? (int)s_replColor9[setIdx9] : 9;
    }
    if (outReplDst10) {
        *outReplDst10 = (setIdx10 > 0 && setIdx10 < 13)
                      ? (int)s_replColor10[setIdx10] : 10;
    }
    return 1;
}

enum {
    M11_CREATURE_POSE_FRONT = 0,
    M11_CREATURE_POSE_SIDE = 1,
    M11_CREATURE_POSE_BACK = 2,
    M11_CREATURE_POSE_ATTACK = 3
};

/* Return the GRAPHICS.DAT index for a creature sprite pose.
 * Ref: ReDMCSB DEFS.H creature aspect layout.
 * Native D1 bitmaps start at graphic 446 and are ordered:
 *   Front D1, Side D1, Back D1, Attack D1, Additional Front D1...
 * Derived D2/D3 bitmaps start at FirstDerivedBitmapIndex and are ordered:
 *   Front D3, Front D2, Side D3, Side D2, Back D3, Back D2,
 *   Attack D3, Attack D2.
 *
 * Creatures do not all have every pose bitmap.  The CREATURE_INFO
 * GraphicInfo field flags which poses have dedicated bitmaps:
 *   MASK0x0008_SIDE  — side bitmap present
 *   MASK0x0010_BACK  — back bitmap present
 *   MASK0x0020_ATTACK — attack bitmap present
 * When a flag is clear the original engine draws the FRONT bitmap for
 * that pose (optionally flipped via the FLIP_* bits).  Without that
 * fallback the renderer indexes into the next creature's bitmap set or
 * unrelated graphic slots — a well-known fidelity bug we fix here.
 */
static unsigned int m11_creature_sprite_for_pose(int creatureType,
                                                 int depthIndex,
                                                 int pose) {
    static const unsigned int kFirstNativeCreatureGraphic = 584;
    static const unsigned char s_nativePoseOffset[4] = {0, 1, 2, 3};
    static const unsigned char s_derivedPoseOffset[4][2] = {
        {0, 1}, /* front  D3/D2 */
        {2, 3}, /* side   D3/D2 */
        {4, 5}, /* back   D3/D2 */
        {6, 7}  /* attack D3/D2 */
    };
    const M11_CreatureAspect* aspect;
    unsigned int gi;
    int dIdx;

    if (creatureType < 0 || creatureType > 26) return 0;
    if (pose < M11_CREATURE_POSE_FRONT || pose > M11_CREATURE_POSE_ATTACK) {
        pose = M11_CREATURE_POSE_FRONT;
    }
    aspect = &s_creatureAspects[creatureType];
    gi = (unsigned int)aspect->graphicInfo;

    /* Source-backed fallback: if the creature lacks a dedicated bitmap
     * for the requested pose, fall back to FRONT.  This matches the
     * original DM1 engine behavior in F0115_DUNGEONVIEW_DrawObjects-
     * CreaturesProjectilesExplosions_CPSEF. */
    if (pose == M11_CREATURE_POSE_SIDE &&
        !(gi & M11_CREATURE_GI_MASK_SIDE)) {
        pose = M11_CREATURE_POSE_FRONT;
    } else if (pose == M11_CREATURE_POSE_BACK &&
               !(gi & M11_CREATURE_GI_MASK_BACK)) {
        pose = M11_CREATURE_POSE_FRONT;
    } else if (pose == M11_CREATURE_POSE_ATTACK &&
               !(gi & M11_CREATURE_GI_MASK_ATTACK)) {
        pose = M11_CREATURE_POSE_FRONT;
    }

    if (depthIndex <= 0) {
        return kFirstNativeCreatureGraphic +
               (unsigned int)aspect->firstNativeBitmapRelativeIndex +
               (unsigned int)s_nativePoseOffset[pose];
    }
    dIdx = (depthIndex >= 2) ? 0 : 1; /* derived order is D3, then D2 */
    return (unsigned int)aspect->firstDerivedBitmapIndex +
           (unsigned int)s_derivedPoseOffset[pose][dIdx];
}

static int m11_creature_relative_facing(int creatureDir, int partyDir) {
    if (creatureDir < 0 || partyDir < 0) return 2;
    return (creatureDir - partyDir) & 3;
}

static int m11_creature_pose_for_view(int relFacing, int attacking) {
    if (attacking && relFacing == 2) {
        return M11_CREATURE_POSE_ATTACK;
    }
    switch (relFacing & 3) {
        case 0: return M11_CREATURE_POSE_BACK;
        case 1:
        case 3: return M11_CREATURE_POSE_SIDE;
        default: return M11_CREATURE_POSE_FRONT;
    }
}

static int m11_creature_pose_mirror(int relFacing, int pose) {
    if (pose == M11_CREATURE_POSE_SIDE) {
        return (relFacing & 3) == 1;
    }
    return 0;
}

/* Source-backed mirror selection.  When a pose falls back to FRONT
 * because the creature lacks a dedicated bitmap, the original engine
 * consults MASK0x0004_FLIP_NON_ATTACK / MASK0x0200_FLIP_ATTACK to decide
 * whether to mirror the front bitmap.  relFacing==1 means the creature
 * is facing the party from its right — the engine flips for that case
 * when the corresponding flag is set.  See ReDMCSB DUNGEON.C
 * F0178_GROUP_GetCreatureAspect / F0115 orientation logic. */
static int m11_creature_pose_mirror_with_info(int creatureType,
                                              int relFacing,
                                              int pose,
                                              int attacking) {
    unsigned int gi;
    if (creatureType < 0 || creatureType > 26) {
        return m11_creature_pose_mirror(relFacing, pose);
    }
    gi = (unsigned int)s_creatureAspects[creatureType].graphicInfo;

    if (pose == M11_CREATURE_POSE_SIDE) {
        if (gi & M11_CREATURE_GI_MASK_SIDE) {
            /* Dedicated side bitmap: mirror when creature is facing
             * from the party's right. */
            return (relFacing & 3) == 1;
        }
        /* Side fell back to front.  Use FLIP_NON_ATTACK to decide. */
        if (gi & M11_CREATURE_GI_MASK_FLIP_NON_ATTACK) {
            return (relFacing & 3) == 1;
        }
        return 0;
    }

    if (pose == M11_CREATURE_POSE_BACK) {
        /* Back never mirrors in the original; if fell back to front via
         * MASK0x0010_BACK clear, still do not mirror. */
        return 0;
    }

    if (pose == M11_CREATURE_POSE_ATTACK) {
        if (gi & M11_CREATURE_GI_MASK_ATTACK) {
            /* Dedicated attack bitmap: honor FLIP_ATTACK on that bitmap. */
            if ((gi & M11_CREATURE_GI_MASK_FLIP_ATTACK) &&
                !(gi & M11_CREATURE_GI_MASK_FLIP_DURING_ATTACK)) {
                return (relFacing & 3) == 1;
            }
            return 0;
        }
        /* Attack fell back to front. */
        if (attacking && (gi & M11_CREATURE_GI_MASK_FLIP_ATTACK)) {
            return (relFacing & 3) == 1;
        }
        return 0;
    }

    return 0;
}

/* Draw a creature sprite from GRAPHICS.DAT at the given viewport position.
 * depthIndex 0 = near (large sprite), 1 = mid, 2 = far (small sprite).
 * sideHint: 0 = center, -1 = left side cell, +1 = right side cell.
 * When sideHint != 0 the sprite is drawn mirrored so creatures
 * appear to face inward toward the corridor center.
 * Returns 1 if a real sprite was drawn, 0 if fallback needed. */
static int m11_draw_creature_sprite(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH,
                                    int x,
                                    int y,
                                    int w,
                                    int h,
                                    int creatureType,
                                    int depthIndex,
                                    int creatureDir) {
    return m11_draw_creature_sprite_ex(state, framebuffer, fbW, fbH,
                                       x, y, w, h, creatureType,
                                       depthIndex, 0, creatureDir);
}

static int m11_draw_creature_sprite_ex(const M11_GameViewState* state,
                                       unsigned char* framebuffer,
                                       int fbW,
                                       int fbH,
                                       int x,
                                       int y,
                                       int w,
                                       int h,
                                       int creatureType,
                                       int depthIndex,
                                       int sideHint,
                                       int creatureDir) {
    unsigned int spriteIdx;
    const M11_AssetSlot* slot;
    int spriteW, spriteH;
    int drawW, drawH, drawX, drawY;
    int useAttackPose = 0;
    int useMirror = 0;
    int hasReplColors = 0;
    int replDst9 = 9, replDst10 = 10;
    int relFacing;
    int pose;

    if (!state->assetsAvailable || creatureType < 0) return 0;

    /* Bounded DM1 fidelity pass:
     * choose the native/derived creature bitmap from original aspect data
     * using group facing vs party facing. This lands true front/side/back
     * pose selection and front-facing attack selection instead of reusing
     * the same generic sprite for every view. */
    if (state->attackCueTimer > 0 &&
        state->attackCueCreatureType == creatureType &&
        depthIndex == 0) {
        useAttackPose = 1;
    }
    relFacing = m11_creature_relative_facing(creatureDir,
                                             state->world.party.direction);
    pose = m11_creature_pose_for_view(relFacing, useAttackPose);
    spriteIdx = m11_creature_sprite_for_pose(creatureType, depthIndex, pose);
    if (spriteIdx == 0) return 0;

    /* Query replacement colors from the creature aspect data.
     * In DM1, creatures that share the same graphic set are
     * differentiated by replacing palette indices 9 and 10 with
     * creature-specific colors during compositing.
     * Ref: ReDMCSB VIDEODRV.C VIDRV_12_SetCreatureReplacementColors. */
    hasReplColors = m11_creature_replacement_colors(creatureType,
                                                    &replDst9, &replDst10);

    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, spriteIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    spriteW = (int)slot->width;
    spriteH = (int)slot->height;

    /* Mirror only when the original pose needs it.
     * Side pose orientation comes from creature direction relative to the
     * party, not from which pane the creature happens to occupy.
     * When the pose fell back to FRONT because no dedicated bitmap
     * exists, the GraphicInfo FLIP_NON_ATTACK/FLIP_ATTACK flags decide
     * whether the front bitmap should be mirrored for this view. */
    useMirror = m11_creature_pose_mirror_with_info(creatureType,
                                                   relFacing, pose,
                                                   useAttackPose);

    /* Scale to fit within the face rect while preserving aspect ratio.
     * DM1 perspective fidelity: side-cell creatures are drawn smaller
     * than center-cell creatures at the same depth.  In the original,
     * side panes are narrower (roughly 60-70% of center width) and
     * creatures shrink proportionally.  We apply a 70% scale factor
     * for side cells to match the corridor perspective illusion.
     * At depth 1+ the side panes are even narrower, so we apply an
     * additional 80% reduction per extra depth step for side cells. */
    {
        int maxW = w - 4;
        int maxH = h - 4;
        if (sideHint != 0) {
            /* DM1 side-cell perspective proportion: ~70% of center */
            maxW = maxW * 70 / 100;
            maxH = maxH * 70 / 100;
            /* Further reduce at greater depth for side cells */
            if (depthIndex >= 1) {
                maxW = maxW * 80 / 100;
                maxH = maxH * 80 / 100;
            }
            if (depthIndex >= 2) {
                maxW = maxW * 80 / 100;
                maxH = maxH * 80 / 100;
            }
        }
        drawW = maxW;
        drawH = (drawW * spriteH) / spriteW;
        if (drawH > maxH) {
            drawH = maxH;
            drawW = (drawH * spriteW) / spriteH;
        }
    }
    if (drawW < 4 || drawH < 4) return 0;

    /* DM1 side-cell positioning: offset toward the corridor center.
     * Left-side creatures shift right, right-side shift left, so they
     * appear at the inner edge of the side pane.  Center creatures
     * remain centered. */
    if (sideHint < 0) {
        /* Left side: push toward right (inner) edge */
        drawX = x + w - drawW - 2;
    } else if (sideHint > 0) {
        /* Right side: push toward left (inner) edge */
        drawX = x + 2;
    } else {
        drawX = x + (w - drawW) / 2;
    }
    drawY = y + (h - drawH) / 2;

    /* Attack cue should keep creature identity stable. Use a subtle
     * forward lunge on the same sprite instead of swapping graphic sets. */
    if (useAttackPose) {
        int lungeW = drawW / 8;
        int lungeH = drawH / 10;
        if (lungeW < 1) lungeW = 1;
        if (lungeH < 1) lungeH = 1;
        if (drawW + lungeW <= w) {
            drawX -= lungeW / 2;
            drawW += lungeW;
        }
        if (drawH + lungeH <= h) {
            drawY -= lungeH;
            drawH += lungeH;
        }
    }

    /* Idle animation: on frame 1, bob the creature up by 1-2 pixels
     * to give a breathing / pulsing effect.  Frame 0 is the base pose.
     * Attack pose skips the bob for a snappier look. */
    if (!useAttackPose) {
        int animFrame = M11_GameView_CreatureAnimFrame(state, creatureType);
        if (animFrame == 1) {
            int bob = (depthIndex == 0) ? 2 : 1;
            drawY -= bob;
        }
    }

    /* Composite the creature sprite with transparent color keying and
     * optional replacement color remapping.
     *
     * In DM1, each creature type specifies its transparent (key) color
     * index via the CoordinateSet_TransparentColor field.  Most creatures
     * use color index 10, but some use 0 (black) or other indices.
     * Ref: ReDMCSB DEFS.H M072_TRANSPARENT_COLOR.
     *
     * Creature types that share graphic sets are differentiated by
     * replacing palette indices 9 and 10 with creature-specific colors.
     * This is how e.g. PainRat and Mummy share set 0 but look different. */
    {
        int transpColor = m11_creature_transparent_color(creatureType);
        if (hasReplColors) {
            if (useMirror) {
                M11_AssetLoader_BlitScaledMirrorReplace(
                    slot, framebuffer, fbW, fbH,
                    drawX, drawY, drawW, drawH, transpColor,
                    9, replDst9, 10, replDst10);
            } else {
                M11_AssetLoader_BlitScaledReplace(
                    slot, framebuffer, fbW, fbH,
                    drawX, drawY, drawW, drawH, transpColor,
                    9, replDst9, 10, replDst10);
            }
        } else {
            if (useMirror) {
                M11_AssetLoader_BlitScaledMirror(slot, framebuffer, fbW, fbH,
                                                 drawX, drawY, drawW, drawH, transpColor);
            } else {
                M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                                           drawX, drawY, drawW, drawH, transpColor);
            }
        }
    }
    return 1;
}

/* Draw the outer UI frame border using ceiling/floor panels from
 * GRAPHICS.DAT. Returns 1 if real assets were used. */
static int m11_draw_ui_frame_assets(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH) {
    const M11_AssetSlot* ceilSlot;
    const M11_AssetSlot* floorSlot;
    if (m11_v2_vertical_slice_enabled()) {
        m11_blit_v2_slice_asset(&m11_v2_viewport_frame_base,
                                framebuffer, fbW, fbH,
                                0, 16, 1);
        return 1;
    }
    if (!state->assetsAvailable) return 0;

    ceilSlot = M11_AssetLoader_Load(
        (M11_AssetLoader*)&state->assetLoader, M11_GFX_CEILING_PANEL);
    floorSlot = M11_AssetLoader_Load(
        (M11_AssetLoader*)&state->assetLoader, M11_GFX_FLOOR_PANEL);

    if (ceilSlot && ceilSlot->width > 0 && ceilSlot->height > 0) {
        /* Blit the ceiling panel at the top of the viewport area */
        M11_AssetLoader_BlitScaled(ceilSlot, framebuffer, fbW, fbH,
                                   12, 24, 196, 14, -1);
    }
    if (floorSlot && floorSlot->width > 0 && floorSlot->height > 0) {
        /* Blit the floor panel below the viewport */
        M11_AssetLoader_BlitScaled(floorSlot, framebuffer, fbW, fbH,
                                   12, 142, 196, 6, -1);
    }
    return (ceilSlot != NULL || floorSlot != NULL) ? 1 : 0;
}

static void m11_draw_corridor_frame(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    const M11_ViewRect* outer,
                                    const M11_ViewRect* inner,
                                    int depthIndex) {
    unsigned char wallShade = depthIndex == 0 ? M11_COLOR_DARK_GRAY : M11_COLOR_BLACK;
    unsigned char edge = depthIndex == 0 ? M11_COLOR_LIGHT_GRAY : M11_COLOR_DARK_GRAY;
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, outer->y, outer->w, inner->y - outer->y, wallShade);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, inner->y + inner->h, outer->w,
                  (outer->y + outer->h) - (inner->y + inner->h), M11_COLOR_BROWN);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, inner->y, inner->x - outer->x, inner->h, wallShade);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  inner->x + inner->w, inner->y,
                  (outer->x + outer->w) - (inner->x + inner->w), inner->h, wallShade);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, outer->y, outer->w, outer->h, edge);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  inner->x, inner->y, inner->w, inner->h, M11_COLOR_LIGHT_GRAY);
}

static void m11_draw_side_feature(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  const M11_ViewRect* outer,
                                  const M11_ViewRect* inner,
                                  const M11_ViewportCell* cell,
                                  int side,
                                  int depthIndex) {
    int paneX;
    int paneW;
    int paneY = inner->y + 3;
    int paneH = inner->h - 6;
    unsigned char accent;

    if (!cell || !cell->valid || paneH <= 4) {
        return;
    }

    if (side < 0) {
        paneX = outer->x + 4;
        paneW = (inner->x - outer->x) - 8;
    } else {
        paneX = inner->x + inner->w + 4;
        paneW = (outer->x + outer->w) - paneX - 4;
    }
    if (paneW <= 4) {
        return;
    }

    accent = m11_feature_accent_color(cell);
    if (m11_viewport_cell_is_open(cell)) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH, M11_COLOR_BLACK);
        if (cell->elementType == DUNGEON_ELEMENT_TELEPORTER) {
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + 1, paneY + 1, paneW - 2, paneH - 2, M11_COLOR_LIGHT_CYAN);
        } else if (cell->elementType == DUNGEON_ELEMENT_PIT) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + 1, paneY + paneH / 2, paneW - 2, paneH / 3, M11_COLOR_BROWN);
        }
    } else {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH,
                      depthIndex == 0 ? M11_COLOR_DARK_GRAY : M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH, accent);
    }

    /* Side-pane wall ornaments: DM1 renders wall ornaments on side walls */
    if (cell->elementType == DUNGEON_ELEMENT_WALL && cell->wallOrnamentOrdinal >= 0 && g_drawState) {
        M11_ViewRect sideWallOrnRect;
        sideWallOrnRect.x = paneX;
        sideWallOrnRect.y = paneY;
        sideWallOrnRect.w = paneW;
        sideWallOrnRect.h = paneH;
        m11_draw_wall_ornament(g_drawState, framebuffer,
                               framebufferWidth, framebufferHeight,
                               &sideWallOrnRect, cell->wallOrnamentOrdinal,
                               depthIndex);
    }

    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        /* Try real door side pillar graphic first */
        if (!g_drawState ||
            !m11_draw_door_side_asset(g_drawState, framebuffer,
                                      framebufferWidth, framebufferHeight,
                                      paneX, paneY, paneW, paneH, depthIndex)) {
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                           paneX + paneW / 2, paneY + 2, paneY + paneH - 3, M11_COLOR_YELLOW);
        }
        /* Draw door ornament on side-visible doors.
         * DM1 renders door ornaments on the side-view door frame
         * when the door has a decorative element (button, keyhole, etc.). */
        if (cell->doorOrnamentOrdinal >= 0 && g_drawState) {
            M11_ViewRect sideOrnRect;
            sideOrnRect.x = paneX;
            sideOrnRect.y = paneY;
            sideOrnRect.w = paneW;
            sideOrnRect.h = paneH;
            m11_draw_door_ornament(g_drawState, framebuffer,
                                   framebufferWidth, framebufferHeight,
                                   &sideOrnRect, cell->doorOrnamentOrdinal,
                                   depthIndex);
        }
        /* Re-draw the accent border on top so the door element
         * accent colour stays visible in probe invariants. */
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH, accent);
    }

    if (m11_viewport_cell_is_open(cell)) {
        /* Floor ornaments in side cells (below items and creatures) */
        if (cell->floorOrnamentOrdinal > 0 && g_drawState) {
            M11_ViewRect sideFloorRect;
            sideFloorRect.x = paneX;
            sideFloorRect.y = paneY;
            sideFloorRect.w = paneW;
            sideFloorRect.h = paneH;
            m11_draw_floor_ornament(g_drawState, framebuffer,
                                    framebufferWidth, framebufferHeight,
                                    &sideFloorRect, cell->floorOrnamentOrdinal,
                                    depthIndex, side);
        }
        /* Multi-creature stacking with duplication in side cells.
         * Duplicate sprites with vertical offsets in narrow pane. */
        if (cell->creatureGroupCount > 0) {
            int gi;
            int groupCount = cell->creatureGroupCount;
            int slotH = (groupCount > 1)
                ? (paneH - 2) * 2 / (groupCount + 1)
                : paneH - 2;
            int stepY = (groupCount > 1)
                ? ((paneH - 2) - slotH) / (groupCount - 1)
                : 0;
            for (gi = 0; gi < groupCount; ++gi) {
                int cy = paneY + 1 + gi * stepY;
                int countInGroup = cell->creatureCountsPerGroup[gi];
                int visibleDups, di;
                if (cell->creatureTypes[gi] < 0) continue;
                visibleDups = countInGroup;
                if (visibleDups > 3) visibleDups = 3;
                if (visibleDups < 1) visibleDups = 1;
                if (visibleDups == 1) {
                    int coordSet = m11_creature_coordinate_set(cell->creatureTypes[gi]);
                    int zoneX = 0;
                    int zoneY = 0;
                    int drawPaneX = paneX + 1;
                    int drawPaneY = cy;
                    int drawPaneW = paneW - 2;
                    int drawSideHint = side;
                    if (m11_c3200_creature_side_zone_point(coordSet,
                                                           depthIndex < 3 ? depthIndex : 2,
                                                           side,
                                                           1, 0,
                                                           &zoneX, &zoneY)) {
                        drawPaneX = M11_VIEWPORT_X + zoneX - drawPaneW / 2;
                        drawPaneY = M11_VIEWPORT_Y + zoneY - slotH;
                        drawSideHint = 0;
                    }
                    if (!g_drawState ||
                        !m11_draw_creature_sprite_ex(g_drawState, framebuffer,
                                                     framebufferWidth, framebufferHeight,
                                                     drawPaneX, drawPaneY,
                                                     drawPaneW, slotH,
                                                     cell->creatureTypes[gi], depthIndex,
                                                     drawSideHint,
                                                     cell->creatureDirections[gi])) {
                        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      paneX + paneW / 2 - 1, cy + slotH / 2 - 2,
                                      3, 5, depthIndex == 0 ? M11_COLOR_LIGHT_GREEN : M11_COLOR_GREEN);
                    }
                } else {
                    int dupH = slotH * 2 / 3;
                    int ofsY = (slotH - dupH) / (visibleDups > 1 ? visibleDups - 1 : 1);
                    if (ofsY < 1) ofsY = 1;
                    for (di = 0; di < visibleDups; ++di) {
                        int dy = cy + di * ofsY;
                        int coordSet = m11_creature_coordinate_set(cell->creatureTypes[gi]);
                        int zoneX = 0;
                        int zoneY = 0;
                        int drawPaneX = paneX + 1;
                        int drawPaneY = dy;
                        int drawPaneW = paneW - 2;
                        int drawSideHint = side;
                        if (m11_c3200_creature_side_zone_point(coordSet,
                                                               depthIndex < 3 ? depthIndex : 2,
                                                               side,
                                                               visibleDups, di,
                                                               &zoneX, &zoneY)) {
                            drawPaneX = M11_VIEWPORT_X + zoneX - drawPaneW / 2;
                            drawPaneY = M11_VIEWPORT_Y + zoneY - dupH;
                            drawSideHint = 0;
                        }
                        if (!g_drawState ||
                            !m11_draw_creature_sprite_ex(g_drawState, framebuffer,
                                                         framebufferWidth, framebufferHeight,
                                                         drawPaneX, drawPaneY,
                                                         drawPaneW, dupH,
                                                         cell->creatureTypes[gi], depthIndex,
                                                         drawSideHint,
                                                         cell->creatureDirections[gi])) {
                            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                          paneX + paneW / 2 - 1, dy + dupH / 2 - 2,
                                          3, 5, depthIndex == 0 ? M11_COLOR_LIGHT_GREEN : M11_COLOR_GREEN);
                        }
                    }
                }
                if (countInGroup > 1 && paneW >= 10 && slotH >= 10) {
                    char countStr[4];
                    int badgeX = paneX + paneW - 9;
                    int badgeY = cy + slotH - 8;
                    snprintf(countStr, sizeof(countStr), "%d", countInGroup);
                    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  badgeX - 1, badgeY - 1, 9, 9, M11_COLOR_BLACK);
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  badgeX, badgeY, countStr, &g_text_small);
                }
            }
        }
        /* ── Side-pane item sprites ──
         * DM1 draws item sprites in side cells similarly to front cells,
         * just smaller to fit the narrower pane. */
        if (cell->floorItemCount > 0) {
            int ii;
            int itemArea = paneH / 3;
            int itemBaseY = paneY + paneH - itemArea - 2;
            for (ii = 0; ii < cell->floorItemCount; ++ii) {
                if (cell->floorItemTypes[ii] < 0) continue;
                if (!g_drawState ||
                    !m11_draw_item_sprite(g_drawState, framebuffer,
                                          framebufferWidth, framebufferHeight,
                                          paneX + 1, itemBaseY,
                                          paneW - 2, itemArea,
                                          cell->floorItemTypes[ii],
                                          cell->floorItemSubtypes[ii],
                                          cell->floorItemCells[ii], ii,
                                          depthIndex + 1)) {
                    if (ii == 0) {
                        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      paneX + paneW / 2 - 2, paneY + paneH - 4,
                                      5, 2, M11_COLOR_YELLOW);
                    }
                }
            }
        } else if (cell->summary.items > 0) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + paneW / 2 - 2, paneY + paneH - 4,
                          5, 2, M11_COLOR_YELLOW);
        }
        /* Side-pane projectile sprites: real GRAPHICS.DAT sprites */
        if (cell->summary.projectiles > 0) {
            int projArea = paneH / 3;
            int projY = paneY + (paneH - projArea) / 2;
            if (projArea < 6) projArea = 6;
            if (!g_drawState ||
                !m11_draw_projectile_sprite(g_drawState, framebuffer,
                                            framebufferWidth, framebufferHeight,
                                            paneX + 1, projY,
                                            paneW - 2, projArea,
                                            cell->firstProjectileGfxIndex,
                                            depthIndex + 1,
                                            cell->firstProjectileRelDir,
                                            cell->firstProjectileCell,
                                            cell->firstProjectileFlipFlags)) {
                int pcx = paneX + paneW / 2;
                int pcy = paneY + paneH / 2;
                m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                               pcx - 2, pcx + 2, pcy, M11_COLOR_LIGHT_CYAN);
                m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                               pcx, pcy - 2, pcy + 2, M11_COLOR_LIGHT_CYAN);
            }
        }
        if (cell->summary.explosions > 0 && cell->summary.projectiles == 0) {
            int ecx = paneX + paneW / 2;
            int ecy = paneY + paneH / 2;
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          ecx - 1, ecy - 1, 3, 3, M11_COLOR_LIGHT_RED);
        }
    }
}

static unsigned char m11_focus_color(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return M11_COLOR_DARK_GRAY;
    }
    if (cell->summary.groups > 0 || cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        return M11_COLOR_LIGHT_RED;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR && !m11_viewport_cell_is_open(cell)) {
        return M11_COLOR_YELLOW;
    }
    if (cell->summary.items > 0 || cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        return M11_COLOR_LIGHT_CYAN;
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT ||
        cell->elementType == DUNGEON_ELEMENT_TELEPORTER ||
        cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        return M11_COLOR_MAGENTA;
    }
    return M11_COLOR_LIGHT_GREEN;
}

static void m11_format_lane_label(const M11_ViewportCell* cell,
                                  const char* prefix,
                                  char* out,
                                  size_t outSize) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!cell || !cell->valid) {
        snprintf(out, outSize, "%s VOID", prefix ? prefix : "?");
        return;
    }
    if (cell->summary.groups > 0) {
        int totalCreatures = 0;
        int gi;
        for (gi = 0; gi < cell->creatureGroupCount && gi < M11_MAX_CELL_CREATURES; ++gi) {
            totalCreatures += cell->creatureCountsPerGroup[gi];
        }
        if (totalCreatures > 0) {
            snprintf(out, outSize, "%s %dC", prefix ? prefix : "?", totalCreatures);
        } else {
            snprintf(out, outSize, "%s %dG", prefix ? prefix : "?", cell->summary.groups);
        }
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        snprintf(out, outSize, "%s DOOR", prefix ? prefix : "?");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT) {
        snprintf(out, outSize, "%s PIT", prefix ? prefix : "?");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        snprintf(out, outSize, "%s STEP", prefix ? prefix : "?");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_TELEPORTER) {
        snprintf(out, outSize, "%s TELE", prefix ? prefix : "?");
        return;
    }
    if (cell->summary.items > 0) {
        snprintf(out, outSize, "%s ITEM", prefix ? prefix : "?");
        return;
    }
    if (cell->summary.sensors > 0 || cell->summary.textStrings > 0 ||
        cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        snprintf(out, outSize, "%s MARK", prefix ? prefix : "?");
        return;
    }
    if (m11_viewport_cell_is_open(cell)) {
        snprintf(out, outSize, "%s OPEN", prefix ? prefix : "?");
        return;
    }
    snprintf(out, outSize, "%s WALL", prefix ? prefix : "?");
}

static void m11_draw_lane_chip(unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight,
                               int x,
                               int y,
                               int w,
                               int h,
                               const M11_ViewportCell* cell,
                               const char* prefix) {
    char label[16];
    unsigned char border = m11_focus_color(cell);
    unsigned char fill = (!cell || !cell->valid || !m11_viewport_cell_is_open(cell))
                             ? M11_COLOR_DARK_GRAY
                             : M11_COLOR_BLACK;

    m11_format_lane_label(cell, prefix, label, sizeof(label));
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, fill);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, border);
    if (cell && cell->valid) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      x + 2, y + 2, 4, h - 4, border);
    }
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  x + 9, y + 2, label, &g_text_small);
}

static void m11_format_depth_label(const M11_ViewportCell* cell,
                                   int depth,
                                   char* out,
                                   size_t outSize) {
    char lane[16];
    char depthPrefix[2];

    if (!out || outSize == 0U) {
        return;
    }
    depthPrefix[0] = (char)('1' + depth);
    depthPrefix[1] = '\0';
    m11_format_lane_label(cell, depthPrefix, lane, sizeof(lane));
    snprintf(out, outSize, "%s", lane);
}

static void m11_draw_depth_chip(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                int x,
                                int y,
                                int w,
                                int h,
                                const M11_ViewportCell* cell,
                                int depth) {
    char label[16];
    unsigned char border = m11_focus_color(cell);
    unsigned char fill = (!cell || !cell->valid || !m11_viewport_cell_is_open(cell))
                             ? M11_COLOR_DARK_GRAY
                             : M11_COLOR_BLACK;

    m11_format_depth_label(cell, depth, label, sizeof(label));
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, fill);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, border);
    if (cell && cell->valid) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      x + 2, y + 2, w - 4, 2, border);
    }
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  x + 4, y + 2, label, &g_text_small);
}

static void m11_draw_focus_brackets(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    const M11_ViewRect* rect,
                                    const M11_ViewportCell* cell) {
    int x1;
    int x2;
    int y1;
    int y2;
    int arm = 10;
    unsigned char color;

    if (!rect || !cell || !cell->valid) {
        return;
    }

    x1 = rect->x + 8;
    y1 = rect->y + 7;
    x2 = rect->x + rect->w - 9;
    y2 = rect->y + rect->h - 8;
    if (x2 - x1 < 12 || y2 - y1 < 12) {
        return;
    }

    color = m11_focus_color(cell);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x1, x1 + arm, y1, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x1, y1, y1 + arm, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x2 - arm, x2, y1, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x2, y1, y1 + arm, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x1, x1 + arm, y2, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x1, y2 - arm, y2, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x2 - arm, x2, y2, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x2, y2 - arm, y2, color);

    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                  rect->x + rect->w / 2, rect->y + rect->h / 2, color);
}

static int m11_tick_has_emission_kind(const struct TickResult_Compat* tick,
                                      uint8_t kind) {
    int i;
    if (!tick) {
        return 0;
    }
    for (i = 0; i < tick->emissionCount; ++i) {
        if (tick->emissions[i].kind == kind) {
            return 1;
        }
    }
    return 0;
}

static unsigned char m11_feedback_color(const M11_GameViewState* state,
                                        const M11_ViewportCell* aheadCell) {
    if (!state) {
        return M11_COLOR_LIGHT_CYAN;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_DAMAGE_DEALT)) {
        return M11_COLOR_LIGHT_RED;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_DOOR_STATE)) {
        return M11_COLOR_YELLOW;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_SOUND_REQUEST)) {
        return M11_COLOR_MAGENTA;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_PARTY_MOVED)) {
        if (state->lastAction[0] != '\0' && strstr(state->lastAction, "TURN") != NULL) {
            return M11_COLOR_YELLOW;
        }
        return M11_COLOR_LIGHT_CYAN;
    }
    if (state->lastOutcome[0] != '\0' && strstr(state->lastOutcome, "BLOCKED") != NULL) {
        return M11_COLOR_YELLOW;
    }
    return m11_focus_color(aheadCell);
}

static void m11_build_feedback_summary(const M11_GameViewState* state,
                                       char* out,
                                       size_t outSize) {
    const struct TickResult_Compat* tick;
    const struct TickEmission_Compat* emission;
    int i;
    int first = 1;

    if (!out || outSize == 0U) {
        return;
    }
    out[0] = '\0';
    if (!state) {
        return;
    }

    tick = &state->lastTickResult;
    for (i = 0; i < tick->emissionCount; ++i) {
        char chunk[32];
        emission = &tick->emissions[i];
        chunk[0] = '\0';
        switch (emission->kind) {
            case EMIT_DAMAGE_DEALT:
                snprintf(chunk, sizeof(chunk), "HIT %d", (int)emission->payload[2]);
                break;
            case EMIT_PARTY_MOVED:
                if (state->lastAction[0] != '\0' && strstr(state->lastAction, "TURN") != NULL) {
                    snprintf(chunk, sizeof(chunk), "TURN %s",
                             m11_direction_name(state->world.party.direction));
                } else {
                    snprintf(chunk, sizeof(chunk), "STEP %d:%d",
                             state->world.party.mapX,
                             state->world.party.mapY);
                }
                break;
            case EMIT_DOOR_STATE:
                snprintf(chunk, sizeof(chunk), "DOOR %d:%d",
                         (int)emission->payload[0],
                         (int)emission->payload[1]);
                break;
            case EMIT_SOUND_REQUEST:
                snprintf(chunk, sizeof(chunk), "FX %d", (int)emission->payload[0]);
                break;
            case EMIT_CHAMPION_DOWN:
                snprintf(chunk, sizeof(chunk), "DOWN %d", (int)emission->payload[0] + 1);
                break;
            case EMIT_KILL_NOTIFY:
                snprintf(chunk, sizeof(chunk), "KILL");
                break;
            default:
                break;
        }
        if (chunk[0] == '\0') {
            continue;
        }
        if (!first) {
            snprintf(out + strlen(out), outSize - strlen(out), " | ");
        }
        snprintf(out + strlen(out), outSize - strlen(out), "%s", chunk);
        first = 0;
    }

    if (out[0] == '\0') {
        if (state->lastOutcome[0] != '\0') {
            snprintf(out, outSize, "%s", state->lastOutcome);
        } else {
            snprintf(out, outSize, "READY");
        }
    }
}

static void m11_draw_feedback_strip(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    const M11_GameViewState* state,
                                    const M11_ViewportCell* aheadCell) {
    char summary[96];
    unsigned char color;

    if (!state) {
        return;
    }

    color = m11_feedback_color(state, aheadCell);
    m11_build_feedback_summary(state, summary, sizeof(summary));

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  24, 130, 172, 10, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  24, 130, 172, 10, color);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  26, 132, 6, 6, color);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  36, 131, summary, &g_text_small);
}

static void m11_draw_viewport_feedback_frame(unsigned char* framebuffer,
                                             int framebufferWidth,
                                             int framebufferHeight,
                                             const M11_GameViewState* state,
                                             const M11_ViewportCell* aheadCell) {
    unsigned char color = m11_feedback_color(state, aheadCell);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  10, 22, 200, 122, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   82, 138, 145, color);
}

static void m11_draw_arrow_glyph(unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int direction,
                                 unsigned char color) {
    switch (direction) {
        case DIR_NORTH:
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x, x + 6, y + 1, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 1, x + 5, y + 2, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 2, x + 4, y + 3, color);
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 3, y + 3, y + 8, color);
            break;
        case DIR_SOUTH:
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 3, y, y + 5, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 2, x + 4, y + 5, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 1, x + 5, y + 6, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x, x + 6, y + 7, color);
            break;
        case DIR_EAST:
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 5, y, y + 6, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x, x + 5, y + 3, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 2, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 4, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 6, y + 3, color);
            break;
        case DIR_WEST:
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 1, y, y + 6, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 1, x + 6, y + 3, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 2, y + 2, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 2, y + 4, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y + 3, color);
            break;
        default:
            break;
    }
}

static void m11_draw_control_button(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    int x,
                                    int y,
                                    int w,
                                    int h,
                                    const char* label,
                                    int iconDirection,
                                    unsigned char border,
                                    unsigned char fill) {
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, fill);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, border);
    if (iconDirection >= 0) {
        m11_draw_arrow_glyph(framebuffer, framebufferWidth, framebufferHeight,
                             x + 2, y + 2, iconDirection, border);
    }
    if (label && label[0] != '\0') {
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      x + 11, y + 2, label, &g_text_small);
    }
}

static void m11_draw_control_strip(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const M11_ViewportCell* aheadCell) {
    unsigned char accent = m11_focus_color(aheadCell);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_CONTROL_STRIP_X, M11_CONTROL_STRIP_Y,
                  M11_CONTROL_STRIP_W, M11_CONTROL_STRIP_H, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_CONTROL_STRIP_X, M11_CONTROL_STRIP_Y,
                  M11_CONTROL_STRIP_W, M11_CONTROL_STRIP_H, accent);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            18, 167, 15, 10, "", DIR_WEST,
                            M11_COLOR_YELLOW, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            35, 167, 15, 10, "", DIR_NORTH,
                            M11_COLOR_LIGHT_CYAN, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            52, 167, 15, 10, "", DIR_EAST,
                            M11_COLOR_YELLOW, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            69, 167, 15, 10, "", DIR_SOUTH,
                            M11_COLOR_LIGHT_CYAN, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            86, 167, 12, 10, "", -1,
                            accent, M11_COLOR_BLACK);
    /* V1: avoid explicit hotkey text on controls; draw a small action pip. */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  90, 170, 4, 4, M11_COLOR_YELLOW);
}

static const char* m11_focus_badge_label(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return "VOID";
    }
    if (cell->summary.groups > 0) {
        return "CONTACT";
    }
    if (cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        return "DANGER";
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        return m11_viewport_cell_is_open(cell) ? "ENTRY" : "BARRIER";
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT ||
        cell->elementType == DUNGEON_ELEMENT_TELEPORTER ||
        cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        return "HAZARD";
    }
    if (cell->summary.items > 0 || cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        return "INTERACT";
    }
    if (m11_viewport_cell_is_open(cell)) {
        return "CLEAR";
    }
    return "STONE";
}

static void m11_draw_focus_card(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                const M11_GameViewState* state,
                                const M11_ViewportCell* aheadCell) {
    unsigned char accent = m11_focus_color(aheadCell);
    const char* badge = m11_focus_badge_label(aheadCell);

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 106, 86, 34, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 106, 86, 34, accent);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  222, 110, 26, 8, accent);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  224, 111, badge, &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  222, 121, state->inspectTitle, &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  222, 129, state->inspectDetail, &g_text_small);
}

/* ================================================================
 * Light-level computation
 *
 * Combines magical light (FUL spell, MAGIC_TORCH), lit torches in
 * champion hand slots, and the ILLUMULET junk item to produce a
 * 0..255 light level that drives viewport depth dimming.
 *
 * Light sources (additive, clamped to 255):
 *   - magicalLightAmount from world.magic (FUL/OH spell chain)
 *   - Each lit TORCH weapon in a hand slot:  +80
 *   - ILLUMULET junk in any hand slot:       +50
 *   - FLAMITT weapon (index 3) lit:          +100
 * ================================================================ */

#define M11_LIGHT_TORCH_BONUS       80
#define M11_LIGHT_ILLUMULET_BONUS   50
#define M11_LIGHT_FLAMITT_BONUS     100
#define M11_LIGHT_MAX               255

/* Torch weapon sub-type index in s_weaponTypeNames[] */
#define M11_WEAPON_SUBTYPE_TORCH    2
#define M11_WEAPON_SUBTYPE_FLAMITT  3
/* ILLUMULET junk sub-type index in s_junkTypeNames[] */
#define M11_JUNK_SUBTYPE_ILLUMULET  4

static int m11_compute_light_level(const M11_GameViewState* state) {
    int light;
    int ci;
    if (!state) {
        return 0;
    }

    /* Start with the magical light amount from the spell system. */
    light = state->world.magic.magicalLightAmount;
    if (light < 0) light = 0;

    /* Scan each champion's hand slots for light-emitting items. */
    for (ci = 0; ci < CHAMPION_MAX_PARTY; ++ci) {
        const struct ChampionState_Compat* champ = &state->world.party.champions[ci];
        int slot;
        if (!champ->present) continue;

        for (slot = CHAMPION_SLOT_HAND_LEFT; slot <= CHAMPION_SLOT_HAND_RIGHT; ++slot) {
            unsigned short thing = champ->inventory[slot];
            int thingType, thingIndex;
            if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;

            thingType = THING_GET_TYPE(thing);
            thingIndex = THING_GET_INDEX(thing);

            if (thingType == THING_TYPE_WEAPON &&
                state->world.things &&
                thingIndex >= 0 && thingIndex < state->world.things->weaponCount) {
                const struct DungeonWeapon_Compat* w = &state->world.things->weapons[thingIndex];
                if (w->type == M11_WEAPON_SUBTYPE_TORCH && w->lit) {
                    /* Scale light by remaining fuel fraction */
                    int fuel = (thingIndex >= 0 && thingIndex < M11_TORCH_FUEL_CAPACITY)
                               ? state->torchFuel[thingIndex] : M11_TORCH_INITIAL_FUEL;
                    int bonus = (M11_LIGHT_TORCH_BONUS * fuel) / M11_TORCH_INITIAL_FUEL;
                    if (bonus < 1 && fuel > 0) bonus = 1;
                    light += bonus;
                }
                if (w->type == M11_WEAPON_SUBTYPE_FLAMITT && w->lit) {
                    int fuel = (thingIndex >= 0 && thingIndex < M11_TORCH_FUEL_CAPACITY)
                               ? state->torchFuel[thingIndex] : M11_FLAMITT_INITIAL_FUEL;
                    int bonus = (M11_LIGHT_FLAMITT_BONUS * fuel) / M11_FLAMITT_INITIAL_FUEL;
                    if (bonus < 1 && fuel > 0) bonus = 1;
                    light += bonus;
                }
            }

            if (thingType == THING_TYPE_JUNK &&
                state->world.things &&
                thingIndex >= 0 && thingIndex < state->world.things->junkCount) {
                const struct DungeonJunk_Compat* j = &state->world.things->junks[thingIndex];
                if (j->type == M11_JUNK_SUBTYPE_ILLUMULET) {
                    light += M11_LIGHT_ILLUMULET_BONUS;
                }
            }
        }
    }

    if (light > M11_LIGHT_MAX) light = M11_LIGHT_MAX;
    return light;
}

/* Query the current party light level (0..255). Exposed for probes. */
int M11_GameView_GetLightLevel(const M11_GameViewState* state) {
    return m11_compute_light_level(state);
}

/* ================================================================
 * Torch fuel burn-down
 * ================================================================ */

int M11_GameView_GetTorchFuel(const M11_GameViewState* state, int weaponIndex) {
    if (!state || weaponIndex < 0 || weaponIndex >= M11_TORCH_FUEL_CAPACITY) {
        return 0;
    }
    return state->torchFuel[weaponIndex];
}

void M11_GameView_UpdateTorchFuel(M11_GameViewState* state) {
    int ci;
    if (!state || !state->active || !state->world.things) {
        return;
    }

    /* Scan all champion hand slots for lit torches/flamitts. */
    for (ci = 0; ci < CHAMPION_MAX_PARTY; ++ci) {
        struct ChampionState_Compat* champ = &state->world.party.champions[ci];
        int slot;
        if (!champ->present) continue;

        for (slot = CHAMPION_SLOT_HAND_LEFT; slot <= CHAMPION_SLOT_HAND_RIGHT; ++slot) {
            unsigned short thing = champ->inventory[slot];
            int thingType, thingIndex;
            if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;

            thingType = THING_GET_TYPE(thing);
            thingIndex = THING_GET_INDEX(thing);

            if (thingType != THING_TYPE_WEAPON) continue;
            if (thingIndex < 0 || thingIndex >= state->world.things->weaponCount) continue;
            if (thingIndex >= M11_TORCH_FUEL_CAPACITY) continue;

            {
                struct DungeonWeapon_Compat* w = &state->world.things->weapons[thingIndex];
                int isTorch = (w->type == M11_WEAPON_SUBTYPE_TORCH);
                int isFlamitt = (w->type == M11_WEAPON_SUBTYPE_FLAMITT);

                if (!isTorch && !isFlamitt) continue;
                if (!w->lit) continue;

                /* Initialize fuel on first encounter */
                if (!state->torchFuelInitialized[thingIndex]) {
                    state->torchFuel[thingIndex] = isTorch
                        ? M11_TORCH_INITIAL_FUEL
                        : M11_FLAMITT_INITIAL_FUEL;
                    state->torchFuelInitialized[thingIndex] = 1;
                }

                /* Burn one unit of fuel */
                state->torchFuel[thingIndex]--;

                /* Log when fuel is low */
                if (state->torchFuel[thingIndex] == (M11_TORCH_INITIAL_FUEL / 4) && isTorch) {
                    m11_log_event(state, M11_COLOR_YELLOW,
                                  "T%u: TORCH DIMS",
                                  (unsigned int)state->world.gameTick);
                }
                if (state->torchFuel[thingIndex] == (M11_FLAMITT_INITIAL_FUEL / 4) && isFlamitt) {
                    m11_log_event(state, M11_COLOR_YELLOW,
                                  "T%u: FLAMITT DIMS",
                                  (unsigned int)state->world.gameTick);
                }

                /* Extinguish when fuel is exhausted */
                if (state->torchFuel[thingIndex] <= 0) {
                    state->torchFuel[thingIndex] = 0;
                    state->torchFuelInitialized[thingIndex] = 0;
                    w->lit = 0;
                    m11_log_event(state, M11_COLOR_LIGHT_RED,
                                  "T%u: %s BURNS OUT",
                                  (unsigned int)state->world.gameTick,
                                  isTorch ? "TORCH" : "FLAMITT");
                }
            }
        }
    }
}

/* Classic-DM right column geometry (320x200 screen).
 *
 *   ZONE_ACTION_AREA = (224, 45, 311, 89)   — 88x45, origin of graphic 10
 *     (C010_GRAPHIC_MENU_ACTION_AREA, 87x45 in GRAPHICS.DAT).
 *   ZONE_SPELL_AREA  = (233, 90, 319, 125)  — spell area backdrop below
 *     action area.  Graphic 9 (C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND,
 *     87x25) is used when the spell casting UI is active; we reuse it
 *     here as the structural backdrop strip so the right column
 *     presents the authentic carved-panel frame rather than empty black
 *     space above the party HUD.  Reference: ReDMCSB DEFS.H:2216,
 *     F0387_MENUS_DrawActionArea and F0394_MENUS_SetMagicCasterAndDrawSpellArea.
 */
#define M11_DM_ACTION_AREA_X    224
#define M11_DM_ACTION_AREA_Y     45
#define M11_DM_ACTION_AREA_W     87
#define M11_DM_ACTION_AREA_H     45
#define M11_DM_SPELL_AREA_X     224
#define M11_DM_SPELL_AREA_Y      90
#define M11_DM_SPELL_AREA_W      87
#define M11_DM_SPELL_AREA_H      25

/* Try to blit GRAPHICS.DAT graphic `gfxIdx` at its native size anchored
 * at (x,y).  Returns 1 on success, 0 if the asset was unavailable or
 * had unexpected dimensions. */
static int m11_blit_panel_asset_native(const M11_GameViewState* state,
                                       unsigned char* framebuffer,
                                       int framebufferWidth,
                                       int framebufferHeight,
                                       unsigned int gfxIdx,
                                       int expectedW,
                                       int expectedH,
                                       int x,
                                       int y) {
    const M11_AssetSlot* slot;
    if (!state || !state->assetsAvailable) {
        return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width <= 0 || slot->height <= 0) {
        return 0;
    }
    if ((int)slot->width != expectedW || (int)slot->height != expectedH) {
        return 0;
    }
    M11_AssetLoader_BlitRegion(slot, 0, 0, expectedW, expectedH,
                               framebuffer, framebufferWidth, framebufferHeight,
                               x, y, -1);
    return 1;
}

/* ---------------------------------------------------------------
 * DM1 action-hand icon cells (F0386_MENUS_DrawActionIcon).
 *
 * In the classic DM1 idle state (G509_B_ActionAreaContainsIcons = TRUE,
 * no champion is "acting"), the action area is cleared to black and
 * each living party champion's action-hand object is drawn as a
 * 16×16 icon inside a cyan-filled cell.  The cell geometry from
 * ReDMCSB MENUS.C F0386_MENUS_DrawActionIcon:
 *
 *   L1184_s_Box.X1 = (championIndex * 22) + 233;
 *   L1184_s_Box.X2 = X1 + 19;      -> cell is 20 wide (X1..X2 inclusive)
 *   L1184_s_Box.Y1 = 86;
 *   L1184_s_Box.Y2 = 120;          -> cell is 35 tall (Y1..Y2 inclusive)
 *   FillBox cell, C04_COLOR_CYAN          (cyan if alive, black if dead)
 *   Icon inner box X1+2, Y1=95  –  X2-2, Y2=110    -> 16×16 icon inset
 *   Blit 16×16 action-hand object icon into inner box
 *
 * The four cells span x=233..321 (overflowing the 224..311 action-area
 * frame on the right by 7 px, which is authentic DM1 behaviour on the
 * 320-wide logical screen — the rightmost cell hugs the right edge of
 * the screen).  Vertically the cells cover y=86..120, which straddles
 * the bottom rows of the action-area frame (y=86..89), the whole
 * spell-area frame (y=90..114), and spills slightly below it
 * (y=115..120).  DM1 draws over the frames in the same way: icon
 * mode fills the action area black first (effectively hiding the
 * frame) and then paints the cyan icon cells.  We keep the
 * authentic ceec03b frame blits in place as the static right-column
 * chrome, and paint the icon cells on top, matching DM1's
 * frame-then-icons stacking order as observed in live
 * screenshots of the idle action area.
 *
 * Ref: ReDMCSB SOURCE/ENGINE/MENUS.C F0386_MENUS_DrawActionIcon,
 *      F0387_MENUS_DrawActionArea (icon-mode branch).
 * --------------------------------------------------------------- */
#define M11_DM_ACTION_ICON_CELL_Y      86
#define M11_DM_ACTION_ICON_CELL_H      35   /* Y1..Y2 inclusive = 120-86+1 */
#define M11_DM_ACTION_ICON_CELL_W      20   /* X1..X2 inclusive = 19 + 1  */
#define M11_DM_ACTION_ICON_CELL_STEP   22
#define M11_DM_ACTION_ICON_CELL_X0    233
#define M11_DM_ACTION_ICON_INNER_X_OFF  2
#define M11_DM_ACTION_ICON_INNER_Y     95
#define M11_DM_ACTION_ICON_INNER_W     16
#define M11_DM_ACTION_ICON_INNER_H     16
#define M11_DM_OBJECT_ICON_GRAPHIC_BASE 42
#define M11_DM_OBJECT_ICONS_PER_GRAPHIC 32
#define M11_DM_OBJECT_ICON_EMPTY_HAND 201

int M11_GameView_GetV1ObjectIconSourceZone(int iconIndex,
                                           int* outGraphicIndex,
                                           int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH) {
    int localIndex;
    if (iconIndex < 0) return 0;
    localIndex = iconIndex % M11_DM_OBJECT_ICONS_PER_GRAPHIC;
    if (outGraphicIndex) {
        *outGraphicIndex = M11_DM_OBJECT_ICON_GRAPHIC_BASE +
                           (iconIndex / M11_DM_OBJECT_ICONS_PER_GRAPHIC);
    }
    if (outX) *outX = (localIndex & 0x0F) * M11_DM_ACTION_ICON_INNER_W;
    if (outY) *outY = (localIndex >> 4) * M11_DM_ACTION_ICON_INNER_H;
    if (outW) *outW = M11_DM_ACTION_ICON_INNER_W;
    if (outH) *outH = M11_DM_ACTION_ICON_INNER_H;
    return 1;
}

int M11_GameView_MapV1ActionIconPaletteColor(int colorIndex,
                                             int applyActionPalette) {
    if (applyActionPalette && (colorIndex & 0x0F) == M11_COLOR_DARK_GRAY) {
        return M11_COLOR_CYAN;
    }
    return colorIndex;
}

int M11_GameView_ShouldHatchV1ActionIconCells(const M11_GameViewState* state) {
    if (!state) return 0;
    return state->candidateMirrorOrdinal > 0 ||
           state->candidateMirrorPanelActive ||
           state->resting;
}

int M11_GameView_GetV1ActionIconCellBackdropColor(const M11_GameViewState* state,
                                                  int championSlot) {
    const struct ChampionState_Compat* champ;
    if (!state || championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY ||
        championSlot >= state->world.party.championCount) {
        return -1;
    }
    champ = &state->world.party.champions[championSlot];
    if (!champ->present) return -1;
    return champ->hp.current == 0 ? M11_COLOR_BLACK : M11_COLOR_CYAN;
}

static int m11_draw_dm_object_icon_index(const M11_GameViewState* state,
                                         unsigned char* framebuffer,
                                         int framebufferWidth,
                                         int framebufferHeight,
                                         int iconIndex,
                                         int dstX,
                                         int dstY,
                                         int applyActionPalette) {
    const M11_AssetSlot* slot;
    int graphicIndex;
    int srcX;
    int srcY;
    int y;
    if (!state || !state->assetsAvailable || !framebuffer || iconIndex < 0) {
        return 0;
    }
    if (!M11_GameView_GetV1ObjectIconSourceZone(iconIndex,
                                                &graphicIndex,
                                                &srcX,
                                                &srcY,
                                                NULL,
                                                NULL)) {
        return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)graphicIndex);
    if (!slot || !slot->loaded || !slot->pixels ||
        srcX + M11_DM_ACTION_ICON_INNER_W > (int)slot->width ||
        srcY + M11_DM_ACTION_ICON_INNER_H > (int)slot->height) {
        return 0;
    }
    for (y = 0; y < M11_DM_ACTION_ICON_INNER_H; ++y) {
        int x;
        int fbY = dstY + y;
        if (fbY < 0 || fbY >= framebufferHeight) continue;
        for (x = 0; x < M11_DM_ACTION_ICON_INNER_W; ++x) {
            int fbX = dstX + x;
            unsigned char px;
            if (fbX < 0 || fbX >= framebufferWidth) continue;
            px = slot->pixels[(srcY + y) * (int)slot->width + srcX + x];
            /* ACTIDRAW.C applies G0498 palette changes only for
             * action-area object icons: color 12 is remapped to C04 cyan.
             * Inventory slot icons use F0038_OBJECT_DrawIconInSlotBox and
             * are blitted directly, without this palette rewrite. */
            px = (unsigned char)M11_GameView_MapV1ActionIconPaletteColor(
                px, applyActionPalette);
            framebuffer[fbY * framebufferWidth + fbX] = px;
        }
    }
    return 1;
}

static int m11_draw_dm_dialog_backdrop(const M11_GameViewState* state,
                                       unsigned char* framebuffer,
                                       int framebufferWidth,
                                       int framebufferHeight) {
    const M11_AssetSlot* slot;
    if (!state || !state->assetsAvailable || !framebuffer) return 0;
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)M11_GameView_GetV1DialogBackdropGraphicId());
    if (!slot || !slot->loaded || !slot->pixels ||
        (int)slot->width != M11_VIEWPORT_W ||
        (int)slot->height != M11_VIEWPORT_H) {
        return 0;
    }
    M11_AssetLoader_Blit(slot, framebuffer, framebufferWidth, framebufferHeight,
                         M11_VIEWPORT_X, M11_VIEWPORT_Y, 0);
    return 1;
}

static int m11_copy_dm_dialog_patch(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    int srcX,
                                    int srcY,
                                    int patchW,
                                    int patchH,
                                    int dstX,
                                    int dstY) {
    const M11_AssetSlot* slot;
    int x, y;
    if (!state || !state->assetsAvailable || !framebuffer) return 0;
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)M11_GameView_GetV1DialogBackdropGraphicId());
    if (!slot || !slot->loaded || !slot->pixels) return 0;
    if (patchW <= 0) patchW = (int)slot->width;
    if (patchH <= 0) patchH = (int)slot->height;
    for (y = 0; y < patchH; ++y) {
        int sy = srcY + y;
        int dy = M11_VIEWPORT_Y + dstY + y;
        if (sy < 0 || sy >= (int)slot->height || dy < 0 || dy >= framebufferHeight) continue;
        for (x = 0; x < patchW; ++x) {
            int sx = srcX + x;
            int dx = M11_VIEWPORT_X + dstX + x;
            if (sx < 0 || sx >= (int)slot->width || dx < 0 || dx >= framebufferWidth) continue;
            framebuffer[dy * framebufferWidth + dx] = slot->pixels[sy * (int)slot->width + sx] & 0x0F;
        }
    }
    return 1;
}

static void m11_apply_dm_dialog_choice_patch(const M11_GameViewState* state,
                                             unsigned char* framebuffer,
                                             int framebufferWidth,
                                             int framebufferHeight) {
    int sx, sy, w, h, dx, dy;
    if (!state) return;
    if (!M11_GameView_GetV1DialogChoicePatchZone(state->dialogChoiceCount,
                                                 &sx, &sy, &w, &h,
                                                 &dx, &dy)) {
        return;
    }
    m11_copy_dm_dialog_patch(state, framebuffer, framebufferWidth, framebufferHeight,
                             sx, sy, w, h, dx, dy);
}

/* Which inventory slot a champion is currently treating as the
 * action hand.  ReDMCSB stores this in G407.Champions[i].Slots[
 * C01_SLOT_ACTION_HAND], with the MASK0x8000_ACTION_HAND bit of
 * Attributes selecting which physical hand maps to it.  Our compat
 * layer does not yet mirror that bit, so fall back to the DM1
 * default for a fresh party: right hand first, then left hand. */
static unsigned short m11_get_action_hand_thing(
    const struct ChampionState_Compat* champ) {
    unsigned short t;
    if (!champ) return THING_NONE;
    /* Prefer the CHAMPION_SLOT_ACTION_HAND alias if populated. */
    t = champ->inventory[CHAMPION_SLOT_ACTION_HAND];
    if (t != THING_NONE && t != THING_ENDOFLIST) return t;
    /* DM1 default acting hand is the right hand. */
    t = champ->inventory[CHAMPION_SLOT_HAND_RIGHT];
    if (t != THING_NONE && t != THING_ENDOFLIST) return t;
    return champ->inventory[CHAMPION_SLOT_HAND_LEFT];
}

/* Source-backed ActionSetIndex lookups extracted from
 * G0237_as_Graphic559_ObjectInfo (ReDMCSB DUNGLOB.C, 180 entries).
 *
 * F0386_MENUS_DrawActionIcon checks:
 *   if (G0237_as_Graphic559_ObjectInfo[ObjectInfoIndex].ActionSetIndex)
 *     -> blit the object icon from GRAPHICS.DAT
 *   else
 *     -> fill the 16x16 icon bitmap with C04_COLOR_CYAN (no icon)
 *
 * Items with ActionSetIndex==0 (food, most junk, potions, armour,
 * chests, scrolls) therefore appear as a PLAIN CYAN cell in the
 * classic DM1 action area — the object icon is intentionally
 * suppressed.  Only items with a defined ActionSet (weapons,
 * shields, bombs with combat actions, magical boxes, rope, coins,
 * magical rings) get their icon drawn.
 *
 * Without this gating, our V1 renderer previously drew any item's
 * sprite into the cell, which caused e.g. bread, bones, and keys
 * to show where DM1 would show an empty cyan cell.  This table
 * restores the authentic F0386 behaviour, matching the source
 * exactly.  Table is indexed by (thing-type, subtype) via
 * m11_action_set_index_for_thing().
 *
 * Ref: ReDMCSB DUNGLOB.C G0237_as_Graphic559_ObjectInfo[180] and
 *      MENUS.C F0386_MENUS_DrawActionIcon. */
static const unsigned char M11_ACTION_SET_INDEX_POTION[20] = {
    /* idx 2..21 in ObjectInfo: Mon, Um, Des, Ven, Sar, Zo, Ros, Ku,
     * Dane, Neta, Bro, Ma, Ya, Ee, Vi, Water, Kath, Pew, Ra, Ful.
     * Only Ven (subtype 3) and Ful (subtype 19) have ActionSet 42. */
     0, 0, 0, 42, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 42
};
static const unsigned char M11_ACTION_SET_INDEX_WEAPON[46] = {
    /* idx 23..68 in ObjectInfo: Eye Of Time..Firestaff Complete.
     * Verbatim ActionSetIndex column from G0237_as_Graphic559_ObjectInfo. */
    43,  7,  5,  6,  8,  9, 10, 11, 12, 13,
    13, 14, 15, 15, 16, 17, 18, 19, 20, 21,
    22, 22, 23, 24, 24, 27, 27, 26, 26, 27,
    42, 40, 42,  5,  5, 28, 29, 30, 31, 32,
    33,  5, 35, 36, 27,  1
};
static const unsigned char M11_ACTION_SET_INDEX_ARMOUR[58] = {
    /* idx 69..126 in ObjectInfo.  Mostly 0; only shield-class armour
     * entries (Buckler..Shield of Darc) have ActionSet 41 (BLOCK). */
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0, 41, 41,
    41, 41,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 41,  0,  0,  0,  0, 41,  0,  0,
     0,  0, 41,  0,  0,  0,  0,  0
};
static const unsigned char M11_ACTION_SET_INDEX_JUNK[53] = {
    /* idx 127..179 in ObjectInfo: Compass, Water, Jewel Symal...
     * Non-zero entries: Copper/Silver/Gold Coin (ActionSet 37),
     * Magical Box Blue/Green (38), Rope (39). */
     0,  0,  0,  0,  0,  0, 37, 37, 37,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 38, 38,  0, 39,  0,  0,  0,  0,
     0,  0,  0
};

/* Resolve the F0386-relevant ActionSetIndex for a thing held in an
 * action hand.  Returns 0 for items DM1 paints as a plain cyan icon
 * cell (ActionSetIndex==0), non-zero for items whose icon DM1 blits
 * into the cell.  thing=THING_NONE/ENDOFLIST returns 0 here because
 * empty-hand icon selection is handled explicitly by
 * m11_draw_dm_action_icon_cells() using C201_ICON_ACTION_ICON_EMPTY_HAND. */
static unsigned int m11_action_set_index_for_thing(
    const struct DungeonThings_Compat* things,
    unsigned short thingId) {
    int thingType, thingIndex, subtype;
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) return 0;
    thingType  = THING_GET_TYPE(thingId);
    thingIndex = THING_GET_INDEX(thingId);
    switch (thingType) {
        case THING_TYPE_SCROLL:
            /* ObjectInfo index 0: Scroll, ActionSetIndex=0. */
            return 0;
        case THING_TYPE_CONTAINER:
            /* ObjectInfo idx 1..: Chest, all ActionSetIndex=0. */
            return 0;
        case THING_TYPE_POTION:
            if (!things->potions || thingIndex < 0 ||
                thingIndex >= things->potionCount) return 0;
            subtype = things->potions[thingIndex].type;
            if (subtype < 0 || subtype >= (int)(sizeof(M11_ACTION_SET_INDEX_POTION) /
                                                sizeof(M11_ACTION_SET_INDEX_POTION[0])))
                return 0;
            return M11_ACTION_SET_INDEX_POTION[subtype];
        case THING_TYPE_WEAPON:
            if (!things->weapons || thingIndex < 0 ||
                thingIndex >= things->weaponCount) return 0;
            subtype = things->weapons[thingIndex].type;
            if (subtype < 0 || subtype >= (int)(sizeof(M11_ACTION_SET_INDEX_WEAPON) /
                                                sizeof(M11_ACTION_SET_INDEX_WEAPON[0])))
                return 0;
            return M11_ACTION_SET_INDEX_WEAPON[subtype];
        case THING_TYPE_ARMOUR:
            if (!things->armours || thingIndex < 0 ||
                thingIndex >= things->armourCount) return 0;
            subtype = things->armours[thingIndex].type;
            if (subtype < 0 || subtype >= (int)(sizeof(M11_ACTION_SET_INDEX_ARMOUR) /
                                                sizeof(M11_ACTION_SET_INDEX_ARMOUR[0])))
                return 0;
            return M11_ACTION_SET_INDEX_ARMOUR[subtype];
        case THING_TYPE_JUNK:
            if (!things->junks || thingIndex < 0 ||
                thingIndex >= things->junkCount) return 0;
            subtype = things->junks[thingIndex].type;
            if (subtype < 0 || subtype >= (int)(sizeof(M11_ACTION_SET_INDEX_JUNK) /
                                                sizeof(M11_ACTION_SET_INDEX_JUNK[0])))
                return 0;
            return M11_ACTION_SET_INDEX_JUNK[subtype];
        default:
            return 0;
    }
}

static int m11_object_info_index_for_thing(const struct DungeonThings_Compat* things,
                                           unsigned short thingId) {
    int thingType, thingIndex, subtype;
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) return -1;
    thingType  = THING_GET_TYPE(thingId);
    thingIndex = THING_GET_INDEX(thingId);
    switch (thingType) {
        case THING_TYPE_SCROLL:
            return 0;
        case THING_TYPE_CONTAINER:
            if (!things->containers || thingIndex < 0 ||
                thingIndex >= things->containerCount) return -1;
            subtype = things->containers[thingIndex].type;
            if (subtype < 0 || subtype > 0) subtype = 0;
            return 1 + subtype;
        case THING_TYPE_POTION:
            if (!things->potions || thingIndex < 0 ||
                thingIndex >= things->potionCount) return -1;
            subtype = things->potions[thingIndex].type;
            if (subtype < 0 || subtype > 20) subtype = 0;
            return 2 + subtype;
        case THING_TYPE_WEAPON:
            if (!things->weapons || thingIndex < 0 ||
                thingIndex >= things->weaponCount) return -1;
            subtype = things->weapons[thingIndex].type;
            if (subtype < 0 || subtype > 45) subtype = 0;
            return 23 + subtype;
        case THING_TYPE_ARMOUR:
            if (!things->armours || thingIndex < 0 ||
                thingIndex >= things->armourCount) return -1;
            subtype = things->armours[thingIndex].type;
            if (subtype < 0 || subtype > 57) subtype = 0;
            return 69 + subtype;
        case THING_TYPE_JUNK:
            if (!things->junks || thingIndex < 0 ||
                thingIndex >= things->junkCount) return -1;
            subtype = things->junks[thingIndex].type;
            if (subtype < 0 || subtype > 52) subtype = 0;
            return 127 + subtype;
        default:
            return -1;
    }
}

static int m11_object_icon_index_for_thing(const M11_GameViewState* state,
                                           const struct DungeonThings_Compat* things,
                                           unsigned short thingId) {
    static const unsigned char kObjectInfoType[180] = {
         30,144,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,
        166,167,195, 16, 18,  4, 14, 20, 23, 25, 27, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
         61, 62, 63, 64, 65, 66,135,143, 28, 80, 81, 82,112,114, 67, 83, 68, 84, 69, 70,
         85, 86, 71, 87,119, 72, 88,113, 89, 73, 74, 90,103,104, 96, 97, 98,105,106,108,
        107, 75, 91, 76, 92, 99,115,100, 77, 93,116,109,101, 78, 94,117,110,102, 79, 95,
        118,111,140,141,142,194,196,  0,  8, 10, 12,146,147,125,126,127,176,177,178,179,
        180,181,182,183,184,185,186,187,188,189,190,191,128,129,130,131,168,169,170,171,
        172,173,174,175,120,121,122,123,124,132,133,134,136,137,138,139,192,193,197,198
    };
    int objectInfoIndex = m11_object_info_index_for_thing(things, thingId);
    int iconIndex;
    if (objectInfoIndex < 0 || objectInfoIndex >= 180) return -1;
    iconIndex = (int)kObjectInfoType[objectInfoIndex];
    if (THING_GET_TYPE(thingId) == THING_TYPE_WEAPON) {
        int thingIndex = THING_GET_INDEX(thingId);
        if (things->weapons && thingIndex >= 0 && thingIndex < things->weaponCount) {
            const struct DungeonWeapon_Compat* weapon = &things->weapons[thingIndex];
            if (iconIndex == 4 && weapon->lit) {
                static const unsigned char kChargeCountToTorchIconOffset[16] = {
                    0,1,1,1,2,2,2,2,3,3,3,3,3,3,3,3
                };
                iconIndex += kChargeCountToTorchIconOffset[weapon->chargeCount & 0x0F];
            } else if (weapon->chargeCount &&
                       (iconIndex == 14 || iconIndex == 16 || iconIndex == 18 ||
                        iconIndex == 20 || iconIndex == 23 || iconIndex == 25)) {
                iconIndex += 1;
            }
        }
    } else if (THING_GET_TYPE(thingId) == THING_TYPE_SCROLL) {
        int thingIndex = THING_GET_INDEX(thingId);
        if (things->scrolls && thingIndex >= 0 && thingIndex < things->scrollCount &&
            iconIndex == 30 && things->scrolls[thingIndex].closed) {
            iconIndex += 1;
        }
    } else if (THING_GET_TYPE(thingId) == THING_TYPE_JUNK) {
        int thingIndex = THING_GET_INDEX(thingId);
        if (things->junks && thingIndex >= 0 && thingIndex < things->junkCount) {
            const struct DungeonJunk_Compat* junk = &things->junks[thingIndex];
            if (iconIndex == 0 && state) {
                iconIndex += state->world.party.direction & 0x03;
            } else if (junk->chargeCount &&
                       (iconIndex == 8 || iconIndex == 10 || iconIndex == 12)) {
                iconIndex += 1;
            }
        }
    }
    return iconIndex;
}

/* ---------------------------------------------------------------
 * DM1 ActionSet table (G0489_as_Graphic560_ActionSets[44]).
 *
 * Verbatim ActionIndices[0..2] column copied out of ReDMCSB
 * MENU.C.  Each entry is a 3-tuple of action-name indices that
 * F0387_MENUS_DrawActionArea (menu-mode branch) prints into
 * zones 85, 86 and 87.  0xFF = C0xFF_ACTION_NONE (empty row,
 * rendered as blank space; also short-circuits the 2-/1-action
 * zone selection in F0387).
 *
 * Entry 0 is the all-none sentinel (matches items with
 * ActionSetIndex == 0 after F0389 aborts).  Entry 2 is the
 * empty-hand set (PUNCH, KICK, WAR CRY) — this is the default
 * when a living champion with no action-hand object is activated,
 * per F0389_MENUS_SetActingChampion.  Entries 1 and 3..43 follow
 * the item's declared ActionSetIndex.
 *
 * Ref: ReDMCSB MENU.C lines 46..86 — "ACTION_SET
 * G0489_as_Graphic560_ActionSets[44] = { ... };".  ReDMCSB stores
 * 6 bytes per entry (action[0..2] + properties + useless byte);
 * only the action-index triple is needed for F0387's menu-mode
 * text rendering, so we store just that here. */
static const unsigned char M11_ACTION_SET_ACTIONS[44][3] = {
    /* 0 */  {255, 255, 255},
    /* 1 */  { 27,  43,  35},
    /* 2 */  {  6,   7,   8}, /* Empty hand: PUNCH, KICK, WAR CRY */
    /* 3 */  {  0,   0,   0},
    /* 4 */  {  0,   0,   0},
    /* 5 */  { 13, 255, 255},
    /* 6 */  { 13,  20, 255},
    /* 7 */  { 13,  23, 255},
    /* 8 */  { 28,  41,  22},
    /* 9 */  { 16,   2,  23},
    /* 10 */ {  2,  25,  20},
    /* 11 */ { 17,  41,  34},
    /* 12 */ { 42,   9,  28},
    /* 13 */ { 13,  17,   2},
    /* 14 */ { 16,  17,  15},
    /* 15 */ { 28,  17,  25},
    /* 16 */ {  2,  25,  15},
    /* 17 */ {  9,   2,  29},
    /* 18 */ { 16,  29,  24},
    /* 19 */ { 13,  15,  19},
    /* 20 */ { 13,   2,  25},
    /* 21 */ {  2,  29,  19},
    /* 22 */ { 13,  30,  31},
    /* 23 */ { 13,  31,  25},
    /* 24 */ { 42,  30, 255},
    /* 25 */ {  0,   0,   0},
    /* 26 */ { 42,   9, 255},
    /* 27 */ { 32, 255, 255},
    /* 28 */ { 37,  33,  36},
    /* 29 */ { 37,  33,  34},
    /* 30 */ { 17,  38,  21},
    /* 31 */ { 13,  21,  34},
    /* 32 */ { 36,  37,  41},
    /* 33 */ { 13,  23,  39},
    /* 34 */ { 13,  17,  40},
    /* 35 */ { 17,  36,  38},
    /* 36 */ {  4, 255, 255},
    /* 37 */ {  5, 255, 255},
    /* 38 */ { 11, 255, 255},
    /* 39 */ { 10, 255, 255},
    /* 40 */ { 42,   9, 255},
    /* 41 */ {  1,  12, 255},
    /* 42 */ { 42, 255, 255},
    /* 43 */ {  6,  11, 255}
};

/* DM1 action-name strings (G0490_ac_Graphic560_ActionNames).
 *
 * Verbatim order from ReDMCSB MENU.C X431_I34E build — the
 * null-delimited string "N\0BLOCK\0CHOP\0X\0BLOW HORN\0...\0FUSE"
 * parsed out into a flat array.  Index 0 is "N" (placeholder used
 * by action set 41 to print just the letter N — "Firestaff"
 * activation).  Index 3 and 26 are the DM1 deprecated-entry
 * placeholder "X".  255 is C0xFF_ACTION_NONE which
 * F0384_MENUS_GetActionName returns as an empty string.
 *
 * These names are what F0387 prints in cyan-on-black into zones
 * 85, 86, 87 during menu-mode.  They are fixed-case UPPERCASE
 * strings in the original; we keep them exactly as in the source. */
static const char* const M11_ACTION_NAMES[44] = {
    /* 0  */ "N",
    /* 1  */ "BLOCK",
    /* 2  */ "CHOP",
    /* 3  */ "X",
    /* 4  */ "BLOW HORN",
    /* 5  */ "FLIP",
    /* 6  */ "PUNCH",
    /* 7  */ "KICK",
    /* 8  */ "WAR CRY",
    /* 9  */ "STAB",
    /* 10 */ "CLIMB DOWN",
    /* 11 */ "FREEZE LIFE",
    /* 12 */ "HIT",
    /* 13 */ "SWING",
    /* 14 */ "STAB",
    /* 15 */ "THRUST",
    /* 16 */ "JAB",
    /* 17 */ "PARRY",
    /* 18 */ "HACK",
    /* 19 */ "BERZERK",
    /* 20 */ "FIREBALL",
    /* 21 */ "DISPELL",
    /* 22 */ "CONFUSE",
    /* 23 */ "LIGHTNING",
    /* 24 */ "DISRUPT",
    /* 25 */ "MELEE",
    /* 26 */ "X",
    /* 27 */ "INVOKE",
    /* 28 */ "SLASH",
    /* 29 */ "CLEAVE",
    /* 30 */ "BASH",
    /* 31 */ "STUN",
    /* 32 */ "SHOOT",
    /* 33 */ "SPELLSHIELD",
    /* 34 */ "FIRESHIELD",
    /* 35 */ "FLUXCAGE",
    /* 36 */ "HEAL",
    /* 37 */ "CALM",
    /* 38 */ "LIGHT",
    /* 39 */ "WINDOW",
    /* 40 */ "SPIT",
    /* 41 */ "BRANDISH",
    /* 42 */ "THROW",
    /* 43 */ "FUSE"
};

const char* M11_GameView_GetActionName(unsigned char actionIndex) {
    if (actionIndex == 0xFF) return "";
    if (actionIndex >= 44) return "";
    return M11_ACTION_NAMES[actionIndex];
}

/* Resolve the ActionSet index for a given champion slot's action
 * hand.  Returns 2 (empty-hand set: PUNCH, KICK, WAR CRY) when the
 * hand is empty (DM1 F0389_MENUS_SetActingChampion fallback), the
 * object's ActionSetIndex from the source table when held, or 0
 * when the object has ActionSetIndex==0 (DM1 aborts F0389 entirely
 * — we still emit a sentinel 0 so the caller renders three NONE
 * rows).  DM1 explicitly returns BEFORE setting G0506 in that
 * last case, so the caller must guard against it. */
static unsigned int m11_resolve_action_set_for_champion(
        const M11_GameViewState* state,
        int championIndex) {
    const struct ChampionState_Compat* champ;
    unsigned short handThing;
    unsigned int setIdx;
    if (!state) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    if (championIndex >= state->world.party.championCount) return 0;
    champ = &state->world.party.champions[championIndex];
    if (!champ->present) return 0;
    if (champ->hp.current == 0) return 0;
    handThing = m11_get_action_hand_thing(champ);
    if (handThing == THING_NONE || handThing == THING_ENDOFLIST) {
        /* DM1 F0389: "Actions Punch, Kick and War Cry". */
        return 2;
    }
    if (!state->world.things) return 0;
    setIdx = m11_action_set_index_for_thing(state->world.things, handThing);
    /* If ActionSetIndex == 0, DM1 F0389 returns without setting
     * the acting ordinal.  We propagate 0 here so callers can
     * treat it as "not activatable". */
    return setIdx;
}

unsigned int M11_GameView_GetActingChampionOrdinal(const M11_GameViewState* state) {
    if (!state) return 0;
    return state->actingChampionOrdinal;
}

int M11_GameView_SetActingChampion(M11_GameViewState* state, int championIndex) {
    unsigned int setIdx;
    if (!state) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    if (championIndex >= state->world.party.championCount) return 0;
    if (!state->world.party.champions[championIndex].present) return 0;
    if (state->world.party.champions[championIndex].hp.current == 0) return 0;
    /* DM1 F0389 aborts when the action-hand item has
     * ActionSetIndex==0 (scrolls, food, most junk).  Mirror that
     * here so the action-menu mode never appears for items that
     * can't actually be used. */
    setIdx = m11_resolve_action_set_for_champion(state, championIndex);
    if (setIdx == 0) return 0;
    /* DM1 stores an ORDINAL (1-based), matching G0506. */
    state->actingChampionOrdinal = (unsigned int)(championIndex + 1);
    return 1;
}

void M11_GameView_ClearActingChampion(M11_GameViewState* state) {
    if (!state) return;
    state->actingChampionOrdinal = 0;
}

int M11_GameView_GetActingActionIndices(const M11_GameViewState* state,
                                        unsigned char outIndices[3]) {
    int idx;
    unsigned int setIdx;
    if (!state || !outIndices) return 0;
    if (state->actingChampionOrdinal == 0) return 0;
    idx = (int)state->actingChampionOrdinal - 1;
    if (idx < 0 || idx >= CHAMPION_MAX_PARTY) return 0;
    setIdx = m11_resolve_action_set_for_champion(state, idx);
    if (setIdx == 0 || setIdx >= 44) return 0;
    outIndices[0] = M11_ACTION_SET_ACTIONS[setIdx][0];
    outIndices[1] = M11_ACTION_SET_ACTIONS[setIdx][1];
    outIndices[2] = M11_ACTION_SET_ACTIONS[setIdx][2];
    return 1;
}

/* DM1 melee-style action flags.  When an action-name index appears
 * in this table it represents a damaging melee-contact strike that
 * classic DM resolves against the creature in the party's front
 * cell.  The original F0391 -> F0407 chain dispatches every action
 * through a larger G0492_ac_Graphic560_ActionDamage table and the
 * per-skill logic in F0407_MENUS_IsActionPerformed; for the bounded
 * V1 slice we only need to know which names are melee-contact so
 * the click fires a CMD_ATTACK tick and the creature in front takes
 * damage via the existing orchestrator path.
 *
 * Non-melee actions (WAR CRY, BLOCK, BLOW HORN, PARRY, LIGHT, HEAL,
 * CALM, CLIMB DOWN, FIREBALL, LIGHTNING, SHOOT, THROW, etc.) still
 * emit the DM1 "CHAMPION: ACTION" log line and clear the acting
 * champion; they simply do not advance the tick as a contact
 * strike.  This keeps the visible recognisability honest — the
 * player sees the expected feedback — without fabricating effects
 * we don't yet model.
 *
 * Ref: ReDMCSB MENU.C G0492_ac_Graphic560_ActionDamage (non-zero
 * entries) cross-referenced against the ActionSet tables. */
static int m11_action_is_melee_contact(unsigned char actionIndex) {
    switch (actionIndex) {
        /* CHOP=2, PUNCH=6, KICK=7, STAB=9 or 14, HIT=12,
         * SWING=13, THRUST=15, JAB=16, HACK=18, BERZERK=19,
         * MELEE=25, SLASH=28, CLEAVE=29, BASH=30, STUN=31. */
        case 2:  case 6:  case 7:  case 9:  case 12:
        case 13: case 14: case 15: case 16: case 18:
        case 19: case 25: case 28: case 29: case 30:
        case 31:
            return 1;
        default:
            return 0;
    }
}

/* Bounded non-melee action effects for the V1 slice.
 *
 * F0391 -> F0407 dispatches every menu action through a large
 * switch in the original.  The bounded V1 slice below implements
 * the subset of non-melee actions whose effects already have
 * source-backed storage in our M10 model (MagicState,
 * GameWorld.freezeLifeTicks, ChampionState.hp / mana) or are pure
 * player-facing log cues identical to the original game.
 *
 * Each handler mirrors the matching F0407 case as literally as
 * the V1 state model allows; behaviour that needs subsystems we
 * have not yet grounded (projectile spells, F0401 frighten,
 * thrown objects, flux/fuse, thieves-eye event) falls through to
 * the generic "time passes" tick advance below so the action row
 * still visibly behaves — stamina drains, disabled-ticks accrue
 * via the orchestrator, creatures move — instead of being a
 * silent log-only stub.
 *
 * Ref: ReDMCSB MENU.C F0407_MENUS_IsActionPerformed,
 *      ACTIDRAW.C F0391_MENUS_DidClickTriggerAction.
 *
 * Returns 1 when the handler bound a real effect (caller reports
 * AL1245_B_ActionPerformed=TRUE), 0 otherwise.  Either way the
 * log line has already been emitted by the caller. */

/* ---------------------------------------------------------------
 * Bounded projectile spawn helper for action-menu projectile/spell
 * rows (FIREBALL / LIGHTNING / DISPELL / INVOKE / SHOOT / THROW).
 *
 * F0407's projectile-spell / shoot / throw branches all funnel
 * through F0327_CHAMPION_IsProjectileSpellCast or F0326_CHAMPION_
 * ShootProjectile / F0328_CHAMPION_IsObjectThrown, which in turn
 * call into PROJEXPL.C's F0212_PROJECTILE_Create and push a
 * TIMELINE_EVENT_PROJECTILE_MOVE.  The v1 source-backed route is
 * the same: F0810_PROJECTILE_Create_Compat populates a slot in
 * GameWorld.projectiles and returns the first-move timeline event,
 * which we schedule via F0721.  The viewport renderer already
 * walks world.projectiles (see the runtime projectile scan around
 * the firstProjectileGfxIndex path in m11_sample_viewport_cell)
 * and draws the sprite at its map cell.
 *
 * Even though the per-tick TIMELINE_EVENT_PROJECTILE_MOVE handler
 * in F0887 is a v1 no-op (the queue pops the event but does not
 * yet step the projectile), the visible, recognisable DM effect
 * still lands: the player sees the correct projectile sprite (from
 * GRAPHICS.DAT 416..438) appear at the party's cell facing their
 * direction, the required mana is deducted, the DM1-style log
 * line and audio cue fire, and the action menu closes.  This
 * matches what classic DM showed the player in the first animation
 * frame of the spell — exactly the bounded slice asked for.
 *
 * Returns 1 on success (slot allocated), 0 on list-full / invalid.
 *
 * Ref: ReDMCSB MENU.C F0407 C020/C021/C023/C027/C032/C042 cases,
 *      PROJEXPL.C F0212_PROJECTILE_Create. */
static int m11_spawn_action_projectile(M11_GameViewState* state,
                                       int championIndex,
                                       int subtype,
                                       int category,
                                       int kineticEnergy,
                                       int impactAttack,
                                       int attackTypeCode) {
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat firstMove;
    int slot = -1;
    if (!state) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    memset(&input, 0, sizeof(input));
    input.category           = category;
    input.subtype            = subtype;
    input.ownerKind          = PROJECTILE_OWNER_CHAMPION;
    input.ownerIndex         = championIndex;
    input.mapIndex           = state->world.party.mapIndex;
    input.mapX               = state->world.party.mapX;
    input.mapY               = state->world.party.mapY;
    input.cell               = championIndex & 3;
    input.direction          = state->world.party.direction & 3;
    input.kineticEnergy      = kineticEnergy;
    input.attack             = impactAttack;
    input.stepEnergy         = 1;
    input.currentTick        = (int)state->world.gameTick;
    input.poisonAttack       = (subtype == PROJECTILE_SUBTYPE_POISON_BOLT ||
                                subtype == PROJECTILE_SUBTYPE_POISON_CLOUD)
                               ? impactAttack : 0;
    input.attackTypeCode     = attackTypeCode;
    input.potionPower        = 0;
    input.firstMoveGraceFlag = 1;
    memset(&firstMove, 0, sizeof(firstMove));
    if (!F0810_PROJECTILE_Create_Compat(&input, &state->world.projectiles,
                                        &slot, &firstMove)) {
        return 0;
    }
    /* Schedule the first-move event so it rides the existing
     * timeline path.  The F0887 handler pops it as a v1 no-op,
     * but scheduling keeps the queue honest for the eventual real
     * handler without requiring any M10 changes. */
    (void)F0721_TIMELINE_Schedule_Compat(&state->world.timeline, &firstMove);
    return 1;
}

/* Compute projectile-spell kinetic/attack parameters for the
 * action-menu projectile rows, mirroring F0407's per-case values.
 *
 * F0407 layout (core amalgam lines ~9535..9555):
 *   LIGHTNING: kineticEnergy=180, explosion=LIGHTNING_BOLT
 *   DISPELL:   kineticEnergy=150, explosion=HARM_NON_MATERIAL
 *   FIREBALL:  kineticEnergy=150, explosion=FIREBALL
 *   SPIT:      kineticEnergy=250, explosion=FIREBALL
 *
 * Impact attack in the original comes from F0026-bounded
 *   (powerOrdinal+2) * (4 + (skill<<1))
 * with powerOrdinal treated as 3 (mid-power) for action rows.
 * For V1 we substitute the wizard skill level via lifecycle
 * lookup — same arithmetic as F0756. */
static int m11_action_projectile_impact_attack(const M11_GameViewState* state,
                                               int championIndex) {
    int skillLevel = M11_GameView_GetSkillLevel(state, championIndex,
                                                CHAMPION_SKILL_WIZARD);
    int raw;
    if (skillLevel < 0) skillLevel = 0;
    /* powerOrdinal=3 approximates the "medium" cast strength that
     * DM1 action-menu projectile rows land with a default power
     * rune (ordinal 3 = Um/Ro).  The +2 offset and <<1 match
     * F0756 / F0026 exactly. */
    raw = (3 + 2) * (4 + (skillLevel << 1));
    if (raw < 21)  raw = 21;
    if (raw > 255) raw = 255;
    return raw;
}

/* ---------------------------------------------------------------
 * V1 projectile tick advance — drives F0811 per live projectile.
 *
 * M10 ships F0811_PROJECTILE_Advance_Compat (mirror of
 * F0219_PROJECTILE_ProcessEvents48To49) fully implemented and
 * verified by the M10 projectile probe: cell flip, cross-cell
 * step, kinetic/energy decay, wall / door / boundary / fluxcage
 * hit classification, impact-attack computation, explosion spawn
 * on magical hit, and next-move rescheduling.  The V1 orchestrator
 * dispatcher (F0887) currently pops TIMELINE_EVENT_PROJECTILE_MOVE
 * as a no-op — M10 policy forbids editing that dispatch — so the
 * per-tick advance has to be driven from the V1 game-view layer
 * itself, which is what this function does.
 *
 * Each active projectile is advanced once per game tick as long
 * as its `scheduledAtTick` has arrived.  The function builds a
 * CellContentDigest_Compat from the dungeon tile data, party
 * position and live creature-AI state, calls F0811, then applies
 * the returned new state back to the slot.  On impact it despawns
 * the slot via F0813, optionally spawns a source-backed explosion
 * via F0821, applies damage to any DungeonGroup at the dest via
 * F0738 for HIT_CREATURE, decrements champion HP for HIT_CHAMPION,
 * and emits DM1-style log cues so the player sees the cast through
 * to its recognisable conclusion instead of only the first frame.
 *
 * M10 functions are CALLED here, never edited; the M10 orchestrator
 * dispatch path (F0887) and M10 per-tick behaviour remain untouched.
 *
 * Ref: ReDMCSB PROJEXPL.C F0219_PROJECTILE_ProcessEvents48To49
 *      (per-tick advance), F0220_EXPLOSION_ProcessEvent25 (per-
 *      tick explosion; v1 explosion-advance is covered by spawning
 *      the explosion into world.explosions so it is rendered by
 *      the viewport for one visible burst frame). */

/* Helper: which direction does cell X lie in?  Destination cell on a
 * cross-cell step is determined by F0811 internally; what we need is
 * the destination SQUARE (mapX, mapY) adjacent to the source square
 * in the projectile's direction.  Exact mirror of Fontanel step
 * table: NORTH=-Y, EAST=+X, SOUTH=+Y, WEST=-X. */
static void m11_projectile_step(int dir, int* dx, int* dy) {
    switch (dir & 3) {
        case 0: *dx = 0;  *dy = -1; break;  /* NORTH */
        case 1: *dx = +1; *dy =  0; break;  /* EAST  */
        case 2: *dx = 0;  *dy = +1; break;  /* SOUTH */
        case 3: *dx = -1; *dy =  0; break;  /* WEST  */
        default:*dx = 0;  *dy =  0; break;
    }
}

/* Build a CellContentDigest_Compat capturing everything F0811
 * needs to know about the source and destination squares.  Returns
 * 1 on success, 0 if the source square cannot be sampled (should
 * never happen for a live projectile).  Populates destIsMapBoundary
 * on off-map destinations so F0811 classifies them as HIT_WALL. */
static int m11_build_projectile_digest(
    const struct GameWorld_Compat* world,
    const struct ProjectileInstance_Compat* p,
    int otherProjectileIndex,
    struct CellContentDigest_Compat* out) {
    unsigned char sourceSq = 0;
    unsigned char destSq = 0;
    int dx = 0, dy = 0;
    int destX, destY;
    const struct DungeonMapDesc_Compat* map;
    int i;
    if (!world || !p || !out) return 0;
    if (!world->dungeon || p->mapIndex < 0
        || p->mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    map = &world->dungeon->maps[p->mapIndex];
    if (!m11_get_square_byte(world, p->mapIndex, p->mapX, p->mapY,
                             &sourceSq)) return 0;
    m11_projectile_step(p->direction, &dx, &dy);
    destX = p->mapX + dx;
    destY = p->mapY + dy;

    memset(out, 0, sizeof(*out));
    out->sourceMapIndex   = p->mapIndex;
    out->sourceMapX       = p->mapX;
    out->sourceMapY       = p->mapY;
    out->sourceSquareType = (sourceSq >> 5) & 7;

    /* Check for other projectiles on source square (to detect
     * in-flight collisions when two projectiles converge). */
    for (i = 0; i < world->projectiles.count; ++i) {
        const struct ProjectileInstance_Compat* q = &world->projectiles.entries[i];
        if (i == otherProjectileIndex) continue;
        if (q->slotIndex < 0) continue;
        if (q->mapIndex == p->mapIndex && q->mapX == p->mapX
                && q->mapY == p->mapY) {
            out->sourceHasOtherProjectile = 1;
            break;
        }
    }

    /* Map boundary check. */
    if (destX < 0 || destY < 0
        || destX >= (int)map->width || destY >= (int)map->height) {
        out->destMapIndex      = p->mapIndex;
        out->destMapX          = destX;
        out->destMapY          = destY;
        out->destIsMapBoundary = 1;
        out->destSquareType    = PROJECTILE_ELEMENT_WALL;
        out->destDoorState     = PROJECTILE_DOOR_STATE_NONE;
        out->destTeleporterNewDirection = -1;
        return 1;
    }
    if (!m11_get_square_byte(world, p->mapIndex, destX, destY, &destSq)) {
        out->destMapIndex      = p->mapIndex;
        out->destMapX          = destX;
        out->destMapY          = destY;
        out->destIsMapBoundary = 1;
        out->destSquareType    = PROJECTILE_ELEMENT_WALL;
        out->destDoorState     = PROJECTILE_DOOR_STATE_NONE;
        out->destTeleporterNewDirection = -1;
        return 1;
    }

    out->destMapIndex  = p->mapIndex;
    out->destMapX      = destX;
    out->destMapY      = destY;
    out->destSquareType = (destSq >> 5) & 7;
    out->destTeleporterNewDirection = -1;

    /* Door state — DM1 encodes door state in the low bits of the
     * square attribute byte.  0 = open, 4 = fully closed. */
    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int doorAttr = destSq & 0x07;
        if (doorAttr == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (doorAttr <= 4) {
            out->destDoorState = doorAttr;  /* CLOSED_ONE_FOURTH..CLOSED_FULL */
        } else if (doorAttr == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
            out->destDoorIsDestroyed = 1;
        } else {
            out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
        }
        /* Magical projectiles pass through normal doors; v1 leaves
         * the per-door MASK0x0002 flag unset so F0816 uses its
         * per-subtype override only. */
        out->destDoorAllowsProjectilePassThrough = 0;
    } else {
        out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
    }

    /* Party presence on destination square. */
    if (world->party.mapIndex == p->mapIndex
            && world->party.mapX == destX
            && world->party.mapY == destY) {
        out->destHasChampion       = 1;
        out->destPartyDirection    = world->party.direction & 3;
        /* Party occupies all 4 sub-cells for F0811's cell-mask gate. */
        out->destChampionCellMask  = 0x0F;
    }

    /* Creature group on destination square (via AI state slots).  v1
     * uses cellMask=0x0F so a projectile that enters a creature's
     * square resolves as HIT_CREATURE regardless of which sub-cell
     * the creature occupies.  NEEDS DISASSEMBLY REVIEW: per-sub-cell
     * hit mask from DungeonGroup_Compat.cells; kept full until the
     * V1 creature-drawing pass grounds sub-cell positioning. */
    for (i = 0; i < world->creatureAICount
                && i < GAMEWORLD_CREATURE_AI_CAPACITY; ++i) {
        const struct CreatureAIState_Compat* ai = &world->creatureAI[i];
        if (ai->groupMapIndex == p->mapIndex
                && ai->groupMapX == destX
                && ai->groupMapY == destY) {
            const struct CreatureBehaviorProfile_Compat* profile =
                CREATURE_GetProfile_Compat(ai->creatureType);
            out->destHasCreatureGroup = 1;
            out->destCreatureType     = ai->creatureType;
            out->destCreatureCellMask = 0x0F;
            out->destCreatureIsNonMaterial = (profile != NULL)
                && ((profile->attributes
                     & CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            break;
        }
    }

    /* Other projectiles on destination square. */
    for (i = 0; i < world->projectiles.count; ++i) {
        const struct ProjectileInstance_Compat* q = &world->projectiles.entries[i];
        if (i == otherProjectileIndex) continue;
        if (q->slotIndex < 0) continue;
        if (q->mapIndex == p->mapIndex && q->mapX == destX
                && q->mapY == destY) {
            out->destHasOtherProjectile = 1;
            break;
        }
    }

    return 1;
}

/* Short DM1-style projectile subtype name for log cues. */
static const char* m11_projectile_subtype_name(int subtype) {
    switch (subtype) {
        case PROJECTILE_SUBTYPE_FIREBALL:          return "FIREBALL";
        case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:    return "LIGHTNING BOLT";
        case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL: return "DISPELL";
        case PROJECTILE_SUBTYPE_POISON_BOLT:       return "POISON BOLT";
        case PROJECTILE_SUBTYPE_POISON_CLOUD:      return "POISON CLOUD";
        case PROJECTILE_SUBTYPE_OPEN_DOOR:         return "MAGIC";
        case PROJECTILE_SUBTYPE_SLIME:             return "SLIME";
        case PROJECTILE_SUBTYPE_KINETIC_ARROW:     return "MISSILE";
        default:                                   return "PROJECTILE";
    }
}

/* Apply bounded V1 impact side-effects: explosion spawn, creature
 * damage, champion damage, and DM1-style log cue.  Designed to
 * touch only data the M10 layer already owns (world.explosions via
 * F0821, world.things->groups via F0738, party champion HP direct
 * write) and to NEVER edit M10 code paths.  Returns nothing. */
static void m11_projectile_apply_impact(
    M11_GameViewState* state,
    const struct ProjectileInstance_Compat* p,
    const struct ProjectileTickResult_Compat* r) {
    const char* name = m11_projectile_subtype_name(p->projectileSubtype);

    /* Explosion spawn — magical hits on fireball / lightning /
     * harm-non-material / poison-* subtypes.  F0820 populated
     * r->outExplosion for us; push it into world.explosions via
     * F0821_EXPLOSION_Create_Compat so the viewport's
     * type-specific burst colour renders on the impact cell. */
    if (r->emittedExplosion) {
        struct ExplosionCreateInput_Compat eIn;
        struct TimelineEvent_Compat eFirst;
        int eSlot = -1;
        memset(&eIn, 0, sizeof(eIn));
        eIn.explosionType         = r->outExplosion.explosionType;
        eIn.attack                = r->outExplosion.attack;
        eIn.mapIndex              = r->outExplosion.mapIndex;
        eIn.mapX                  = r->outExplosion.mapX;
        eIn.mapY                  = r->outExplosion.mapY;
        eIn.cell                  = r->outExplosion.cell;
        eIn.centered              = r->outExplosion.centered;
        eIn.poisonAttack          = r->outExplosion.poisonAttack;
        eIn.currentTick           = (int)state->world.gameTick;
        eIn.ownerKind             = r->outExplosion.ownerKind;
        eIn.ownerIndex            = r->outExplosion.ownerIndex;
        eIn.creatorProjectileSlot = r->outExplosion.creatorProjectileSlot;
        (void)F0821_EXPLOSION_Create_Compat(&eIn, &state->world.explosions,
                                            &eSlot, &eFirst);
    }

    /* Apply damage on HIT_CREATURE to the DungeonGroup at the impact
     * cell.  We find the group thing at (destMap, destX, destY), use
     * F0738_COMBAT_ApplyDamageToGroup_Compat on creature slot 0 with
     * the impact attack as damage.  DM1's group damage scatters hits
     * across live sub-cells; v1 settles for slot 0 so projectiles
     * that reach a creature square visibly chip or kill the creature
     * rather than vanishing silently.  F0738 is a pure M10 mutator
     * and is not modified here. */
    if (r->resultKind == PROJECTILE_RESULT_HIT_CREATURE
            && r->emittedCombatAction) {
        unsigned short groupThing = m11_find_group_on_square(
            &state->world, p->mapIndex,
            r->newMapX != 0 || r->newMapY != 0 ? r->newMapX : p->mapX,
            r->newMapX != 0 || r->newMapY != 0 ? r->newMapY : p->mapY);
        if (groupThing != THING_NONE && groupThing != THING_ENDOFLIST
                && state->world.things) {
            int gIdx = THING_GET_INDEX(groupThing);
            if (gIdx >= 0 && gIdx < state->world.things->groupCount) {
                struct DungeonGroup_Compat* g = &state->world.things->groups[gIdx];
                struct CombatResult_Compat res;
                int outcome = 0;
                int slotI;
                memset(&res, 0, sizeof(res));
                res.damageApplied = r->outAction.rawAttackValue;
                for (slotI = 0; slotI < 4; ++slotI) {
                    if (g->health[slotI] > 0) {
                        F0738_COMBAT_ApplyDamageToGroup_Compat(
                            &res, g, slotI, &outcome);
                        break;
                    }
                }
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s HITS %s",
                              (unsigned int)state->world.gameTick,
                              name,
                              m11_creature_name((int)g->creatureType));
                return;
            }
        }
        m11_log_event(state, M11_COLOR_LIGHT_RED,
                      "T%u: %s STRIKES CREATURE",
                      (unsigned int)state->world.gameTick, name);
        return;
    }

    if (r->resultKind == PROJECTILE_RESULT_HIT_CHAMPION
            && r->emittedCombatAction) {
        int ci = r->outAction.defenderSlotOrCreatureIndex;
        if (ci >= 0 && ci < CHAMPION_MAX_PARTY
                && state->world.party.champions[ci].present) {
            int hp = (int)state->world.party.champions[ci].hp.current;
            int dmg = r->outAction.rawAttackValue;
            if (dmg < 0) dmg = 0;
            if (dmg > hp) dmg = hp;
            state->world.party.champions[ci].hp.current =
                (unsigned short)(hp - dmg);
            m11_log_event(state, M11_COLOR_LIGHT_RED,
                          "T%u: %s HITS PARTY FOR %d",
                          (unsigned int)state->world.gameTick, name, dmg);
        } else {
            m11_log_event(state, M11_COLOR_LIGHT_RED,
                          "T%u: %s HITS PARTY",
                          (unsigned int)state->world.gameTick, name);
        }
        return;
    }

    switch (r->resultKind) {
        case PROJECTILE_RESULT_HIT_WALL:
            m11_log_event(state, M11_COLOR_YELLOW,
                          "T%u: %s HITS WALL",
                          (unsigned int)state->world.gameTick, name);
            break;
        case PROJECTILE_RESULT_HIT_DOOR:
            m11_log_event(state, M11_COLOR_YELLOW,
                          "T%u: %s HITS DOOR",
                          (unsigned int)state->world.gameTick, name);
            break;
        case PROJECTILE_RESULT_HIT_FLUXCAGE:
            m11_log_event(state, M11_COLOR_MAGENTA,
                          "T%u: %s ABSORBED BY FLUXCAGE",
                          (unsigned int)state->world.gameTick, name);
            break;
        case PROJECTILE_RESULT_HIT_OTHER_PROJECTILE:
            m11_log_event(state, M11_COLOR_LIGHT_CYAN,
                          "T%u: %s COLLIDES IN FLIGHT",
                          (unsigned int)state->world.gameTick, name);
            break;
        case PROJECTILE_RESULT_DESPAWN_ENERGY:
            m11_log_event(state, M11_COLOR_DARK_GRAY,
                          "T%u: %s FADES",
                          (unsigned int)state->world.gameTick, name);
            break;
        case PROJECTILE_RESULT_DESPAWN_BOUNDS:
            m11_log_event(state, M11_COLOR_DARK_GRAY,
                          "T%u: %s OUT OF BOUNDS",
                          (unsigned int)state->world.gameTick, name);
            break;
        default:
            break;
    }
}

/* Public per-tick advance: iterate live projectiles, drive F0811
 * once per projectile whose scheduled tick has arrived, apply the
 * new state or despawn + impact side-effects.  Called from
 * M11_GameView_ProcessTickEmissions so every orchestrator tick
 * (movement, attack, rest, action menu) steps active projectiles
 * through the world exactly once.  Idempotent on empty lists. */
static void m11_advance_projectiles_v1(M11_GameViewState* state) {
    int i;
    uint32_t now;
    if (!state || !state->active) return;
    /* No dungeon = no reliable cell-content digest.  This is the
     * case for probe harnesses that exercise action-menu dispatch
     * without wiring a full dungeon; for those the correct V1
     * behaviour is to leave projectile slots undisturbed so the
     * probe can still verify spawn side-effects.  In real gameplay
     * the dungeon is always loaded before projectiles can fire. */
    if (!state->world.dungeon || !state->world.dungeon->tilesLoaded) return;
    now = state->world.gameTick;

    for (i = 0; i < PROJECTILE_LIST_CAPACITY; ++i) {
        struct ProjectileInstance_Compat* p = &state->world.projectiles.entries[i];
        struct ProjectileInstance_Compat newState;
        struct ProjectileTickResult_Compat result;
        struct CellContentDigest_Compat digest;

        if (p->slotIndex < 0) continue;
        if ((uint32_t)p->scheduledAtTick > now) continue;

        if (!m11_build_projectile_digest(&state->world, p, i, &digest)) {
            /* Can't classify the digest this tick (e.g. transient
             * dungeon state).  Skip rather than despawn so the slot
             * stays visible and can be advanced on a later tick. */
            continue;
        }

        if (!F0811_PROJECTILE_Advance_Compat(p, &digest, now,
                                             &state->world.masterRng,
                                             &newState, &result)) {
            F0813_PROJECTILE_Despawn_Compat(&state->world.projectiles, i);
            continue;
        }

        if (result.despawn) {
            /* Apply impact side effects (explosion, damage, log cue)
             * before freeing the slot so the cue references the
             * right projectile type. */
            m11_projectile_apply_impact(state, p, &result);
            F0813_PROJECTILE_Despawn_Compat(&state->world.projectiles, i);
        } else {
            /* Commit the flown state.  F0811 fills outNewState with
             * the updated cell/map/direction/energy; we also advance
             * scheduledAtTick per F0825 semantics (1 tick on party
             * map, 3 ticks off-map).  The timeline queue itself is
             * untouched — M10 policy. */
            newState.scheduledAtTick = (int)now +
                ((newState.mapIndex == (int)state->world.partyMapIndex) ? 1 : 3);
            *p = newState;
        }
    }
}

/* Probe-visible wrapper so game_view_probe.c can drive the advance
 * deterministically without needing to replay an orchestrator tick. */
void M11_GameView_AdvanceProjectilesOnce(M11_GameViewState* state) {
    m11_advance_projectiles_v1(state);
}

/* Build a CellContentDigest_Compat for an explosion's current cell so
 * F0822_EXPLOSION_Advance_Compat can classify champion / creature-group
 * presence and door state at the AoE target square.  Returns 1 on
 * success.  Mirrors m11_build_projectile_digest but centred on the
 * explosion's cell (source == dest for a stationary explosion). */
static int m11_build_explosion_digest(
    const struct GameWorld_Compat* world,
    const struct ExplosionInstance_Compat* e,
    struct CellContentDigest_Compat* out) {
    unsigned char sq = 0;
    int hasSquare = 0;
    int i;
    if (!world || !e || !out) return 0;
    if (!world->dungeon || e->mapIndex < 0
        || e->mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    /* Explosions can land on wall / boundary cells (projectile
     * detonation on impact with a wall), so a missing squareByte is
     * fine — classify as a wall-cell burst and let F0822 run with a
     * minimal digest.  Matches ReDMCSB where a fireball bursting on
     * a wall still progresses through F0822's frame logic. */
    hasSquare = m11_get_square_byte(world, e->mapIndex, e->mapX, e->mapY, &sq);

    memset(out, 0, sizeof(*out));
    out->sourceMapIndex    = e->mapIndex;
    out->sourceMapX        = e->mapX;
    out->sourceMapY        = e->mapY;
    out->sourceSquareType  = hasSquare ? ((sq >> 5) & 7) : PROJECTILE_ELEMENT_WALL;
    out->destMapIndex      = e->mapIndex;
    out->destMapX          = e->mapX;
    out->destMapY          = e->mapY;
    out->destSquareType    = hasSquare ? ((sq >> 5) & 7) : PROJECTILE_ELEMENT_WALL;
    out->destIsMapBoundary = hasSquare ? 0 : 1;
    out->destTeleporterNewDirection = -1;
    if (!hasSquare) {
        /* Off-map / unreadable cell: no champion, no creature, no door;
         * just let F0822 despawn a one-shot or decay a persistent type. */
        return 1;
    }

    if (out->destSquareType == PROJECTILE_ELEMENT_DOOR) {
        int doorAttr = sq & 0x07;
        if (doorAttr == 0) {
            out->destDoorState = PROJECTILE_DOOR_STATE_OPEN;
        } else if (doorAttr <= 4) {
            out->destDoorState = doorAttr;
        } else if (doorAttr == 5) {
            out->destDoorState = PROJECTILE_DOOR_STATE_DESTROYED;
            out->destDoorIsDestroyed = 1;
        } else {
            out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
        }
    } else {
        out->destDoorState = PROJECTILE_DOOR_STATE_NONE;
    }

    /* Party presence on the explosion cell. */
    if (world->party.mapIndex == e->mapIndex
            && world->party.mapX == e->mapX
            && world->party.mapY == e->mapY) {
        out->destHasChampion      = 1;
        out->destPartyDirection   = world->party.direction & 3;
        out->destChampionCellMask = 0x0F;
    }

    /* Creature group on the explosion cell. */
    for (i = 0; i < world->creatureAICount
                && i < GAMEWORLD_CREATURE_AI_CAPACITY; ++i) {
        const struct CreatureAIState_Compat* ai = &world->creatureAI[i];
        if (ai->groupMapIndex == e->mapIndex
                && ai->groupMapX == e->mapX
                && ai->groupMapY == e->mapY) {
            const struct CreatureBehaviorProfile_Compat* profile =
                CREATURE_GetProfile_Compat(ai->creatureType);
            out->destHasCreatureGroup = 1;
            out->destCreatureType     = ai->creatureType;
            out->destCreatureCellMask = 0x0F;
            out->destCreatureIsNonMaterial = (profile != NULL)
                && ((profile->attributes
                     & CREATURE_ATTR_MASK_NON_MATERIAL) != 0);
            break;
        }
    }

    return 1;
}

/* Apply the per-frame combat actions that F0822 emitted for this
 * explosion advance.  Mirrors the bounded V1 damage path used for
 * projectile impacts: champion HP direct write for party hits,
 * F0738_COMBAT_ApplyDamageToGroup_Compat for creature-group hits.
 * Pure data mutation, no M10 edits. */
static void m11_explosion_apply_tick_result(
    M11_GameViewState* state,
    const struct ExplosionInstance_Compat* e,
    const struct ExplosionTickResult_Compat* r) {
    const char* typeName = "EXPLOSION";
    if (e->explosionType == C000_EXPLOSION_FIREBALL)           typeName = "FIREBALL";
    else if (e->explosionType == C002_EXPLOSION_LIGHTNING_BOLT) typeName = "LIGHTNING";
    else if (e->explosionType == C007_EXPLOSION_POISON_CLOUD)   typeName = "POISON CLOUD";
    else if (e->explosionType == C040_EXPLOSION_SMOKE)          typeName = "SMOKE";

    /* Champion damage. */
    if (r->emittedCombatActionPartyCount > 0) {
        int ci = r->outActionParty.defenderSlotOrCreatureIndex;
        int dmg = r->outActionParty.rawAttackValue;
        if (ci >= 0 && ci < CHAMPION_MAX_PARTY
                && state->world.party.champions[ci].present) {
            int hp = (int)state->world.party.champions[ci].hp.current;
            if (dmg < 0) dmg = 0;
            if (dmg > hp) dmg = hp;
            state->world.party.champions[ci].hp.current =
                (unsigned short)(hp - dmg);
            m11_log_event(state, M11_COLOR_LIGHT_RED,
                          "T%u: %s BURNS PARTY FOR %d",
                          (unsigned int)state->world.gameTick,
                          typeName, dmg);
        }
    }

    /* Creature group damage. */
    if (r->emittedCombatActionGroupCount > 0 && state->world.things) {
        unsigned short groupThing = m11_find_group_on_square(
            &state->world,
            r->outActionGroup.targetMapIndex,
            r->outActionGroup.targetMapX,
            r->outActionGroup.targetMapY);
        if (groupThing != THING_NONE && groupThing != THING_ENDOFLIST) {
            int gIdx = THING_GET_INDEX(groupThing);
            if (gIdx >= 0 && gIdx < state->world.things->groupCount) {
                struct DungeonGroup_Compat* g = &state->world.things->groups[gIdx];
                struct CombatResult_Compat res;
                int outcome = 0;
                int slotI;
                memset(&res, 0, sizeof(res));
                res.damageApplied = r->outActionGroup.rawAttackValue;
                for (slotI = 0; slotI < 4; ++slotI) {
                    if (g->health[slotI] > 0) {
                        F0738_COMBAT_ApplyDamageToGroup_Compat(
                            &res, g, slotI, &outcome);
                        break;
                    }
                }
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s SEARS %s",
                              (unsigned int)state->world.gameTick,
                              typeName,
                              m11_creature_name((int)g->creatureType));
            }
        }
    }
}

/* V1 explosion tick advance — drives F0822 per live explosion whose
 * scheduledAtTick has arrived.  Called once per orchestrator tick
 * (right after the projectile advance) so a fireball detonation
 * progresses through its frame sequence instead of freezing at the
 * first burst frame.  The persistent types (poison cloud / smoke)
 * reduce attack per frame and despawn when attack drops below the
 * type-specific threshold; one-shot types (fireball / lightning)
 * emit damage on the first advance and despawn immediately.  Idle
 * on empty lists.  Mirrors ReDMCSB TIMELINE.C's EXPLOSION_ADVANCE
 * timeline-event dispatch to F0822. */
static void m11_advance_explosions_v1(M11_GameViewState* state) {
    int i;
    uint32_t now;
    if (!state || !state->active) return;
    if (!state->world.dungeon || !state->world.dungeon->tilesLoaded) return;
    now = state->world.gameTick;

    for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
        struct ExplosionInstance_Compat* e = &state->world.explosions.entries[i];
        struct ExplosionInstance_Compat newState;
        struct ExplosionTickResult_Compat result;
        struct CellContentDigest_Compat digest;

        if (e->reserved0 == 0) continue;
        if ((uint32_t)e->scheduledAtTick > now) continue;

        if (!m11_build_explosion_digest(&state->world, e, &digest)) {
            continue;
        }

        if (!F0822_EXPLOSION_Advance_Compat(e, &digest, now,
                                             &state->world.masterRng,
                                             &newState, &result)) {
            F0824_EXPLOSION_Despawn_Compat(&state->world.explosions, i);
            continue;
        }

        /* Apply AoE effects before the slot is reused on despawn. */
        m11_explosion_apply_tick_result(state, e, &result);

        if (result.despawn) {
            F0824_EXPLOSION_Despawn_Compat(&state->world.explosions, i);
        } else {
            /* Commit advanced state (new frame, possibly reduced attack,
             * possibly promoted rebirth step 1 -> step 2).  Re-schedule
             * per F0826: 1 tick default, 5 ticks for REBIRTH_STEP1
             * -> STEP2.  newAttack already reflects F0822's decay for
             * poison/smoke. */
            int delay = (e->explosionType == C100_EXPLOSION_REBIRTH_STEP1)
                ? 5 : 1;
            newState.reserved0 = 1; /* preserve occupied flag */
            newState.slotIndex = i;
            newState.scheduledAtTick = (int)now + delay;
            *e = newState;
        }
    }
}

/* Probe-visible wrapper so game_view_probe.c can drive the explosion
 * advance deterministically without replaying an orchestrator tick. */
void M11_GameView_AdvanceExplosionsOnce(M11_GameViewState* state) {
    m11_advance_explosions_v1(state);
}

int M11_GameView_CountCellProjectiles(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    M11_SquareThingSummary summary;
    m11_summarize_square_things(world, mapIndex, mapX, mapY, &summary);
    return summary.projectiles;
}

int M11_GameView_CountCellExplosions(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    M11_SquareThingSummary summary;
    m11_summarize_square_things(world, mapIndex, mapX, mapY, &summary);
    return summary.explosions;
}

int M11_GameView_GetProjectileSourceScaleUnits(int depthIndex,
                                               int relativeCell) {
    return m11_projectile_source_scale_units(depthIndex, relativeCell);
}

int M11_GameView_GetProjectileAspectFirstNative(int aspectIndex) {
    return m11_projectile_aspect_first_native(aspectIndex);
}

unsigned int M11_GameView_GetProjectileAspectGraphicInfo(int aspectIndex) {
    return m11_projectile_aspect_graphic_info(aspectIndex);
}

int M11_GameView_GetProjectileAspectBitmapDelta(int aspectIndex, int relativeDir) {
    return m11_projectile_aspect_bitmap_delta(aspectIndex, relativeDir);
}

int M11_GameView_GetProjectileGraphicForAspect(int aspectIndex, int relativeDir) {
    return m11_projectile_aspect_to_graphic_index(aspectIndex, relativeDir);
}

int M11_GameView_GetProjectileAspectFlipFlags(int aspectIndex,
                                              int relativeDir,
                                              int relativeCell,
                                              int mapX,
                                              int mapY) {
    return m11_projectile_aspect_flip_flags(aspectIndex,
                                            relativeDir,
                                            relativeCell,
                                            mapX,
                                            mapY);
}

unsigned int M11_GameView_GetObjectSpriteIndex(int thingType, int subtype) {
    return m11_item_sprite_index(thingType, subtype);
}

int M11_GameView_GetObjectSourceScaleUnits(int scaleIndex) {
    return m11_object_source_scale_units(scaleIndex);
}

int M11_GameView_GetObjectSourceScaleIndex(int depthIndex, int relativeCell) {
    return m11_object_source_scale_index(depthIndex, relativeCell);
}

int M11_GameView_GetC2500ObjectZonePoint(int scaleIndex,
                                         int relativeCell,
                                         int* outX,
                                         int* outY) {
    return m11_c2500_object_zone_point(scaleIndex, relativeCell, outX, outY);
}

int M11_GameView_GetC2900ProjectileZonePoint(int scaleIndex,
                                             int relativeCell,
                                             int* outX,
                                             int* outY) {
    return m11_c2900_projectile_zone_point(scaleIndex, relativeCell, outX, outY);
}

int M11_GameView_GetWallSetGraphicIndex(int wallSet, int wallSet0GraphicIndex) {
    if (wallSet < 0) wallSet = 0;
    if (wallSet0GraphicIndex < M11_GFX_DOOR_SIDE_D0 ||
        wallSet0GraphicIndex > M11_GFX_DM1_STAIRS_SIDE_D0L) {
        return wallSet0GraphicIndex;
    }
    return M11_GFX_DOOR_SIDE_D0 + wallSet * 40 +
           (wallSet0GraphicIndex - M11_GFX_DOOR_SIDE_D0);
}

int M11_GameView_GetViewportRect(int* outX, int* outY, int* outW, int* outH) {
    if (outX) *outX = M11_VIEWPORT_X;
    if (outY) *outY = M11_VIEWPORT_Y;
    if (outW) *outW = M11_VIEWPORT_W;
    if (outH) *outH = M11_VIEWPORT_H;
    return 1;
}

int M11_GameView_GetObjectIconIndexForThing(const M11_GameViewState* state,
                                            unsigned short thingId) {
    if (!state || !state->world.things) return -1;
    return m11_object_icon_index_for_thing(state, state->world.things, thingId);
}

int M11_GameView_GetC3200CreatureZonePoint(int coordSet,
                                           int depthIndex,
                                           int visibleCount,
                                           int slotIndex,
                                           int* outX,
                                           int* outY) {
    return m11_c3200_creature_zone_point(coordSet, depthIndex,
                                         visibleCount, slotIndex,
                                         outX, outY);
}

int M11_GameView_GetC3200CreatureSideZonePoint(int coordSet,
                                               int depthIndex,
                                               int sideHint,
                                               int visibleCount,
                                               int slotIndex,
                                               int* outX,
                                               int* outY) {
    return m11_c3200_creature_side_zone_point(coordSet, depthIndex,
                                              sideHint, visibleCount,
                                              slotIndex, outX, outY);
}

void M11_GameView_GetObjectPileShiftIndices(int pileIndex,
                                            int* outXIndex,
                                            int* outYIndex) {
    m11_object_source_pile_shift_indices(pileIndex, outXIndex, outYIndex);
}

int M11_GameView_GetObjectShiftValue(int shiftSet, int shiftIndex) {
    return m11_object_source_shift_value(shiftSet, shiftIndex);
}

unsigned int M11_GameView_GetObjectAspectGraphicInfo(int aspectIndex) {
    return m11_object_aspect_graphic_info(aspectIndex);
}

int M11_GameView_GetObjectAspectCoordinateSet(int aspectIndex) {
    return m11_object_aspect_coordinate_set(aspectIndex);
}

int M11_GameView_ObjectUsesFlipOnRight(int thingType, int subtype,
                                       int relativeCell) {
    int aspectIndex = m11_item_aspect_index(thingType, subtype);
    if (aspectIndex < 0) return 0;
    return ((m11_object_aspect_graphic_info(aspectIndex) & 0x0001u) &&
            (relativeCell == 1 || relativeCell == 3)) ? 1 : 0;
}

int M11_GameView_GetCreaturePaletteChange(int depthPaletteIndex,
                                          int paletteIndex) {
    return m11_creature_source_palette_change(depthPaletteIndex, paletteIndex);
}

static int m11_perform_non_melee_action(M11_GameViewState* state,
                                        int championIndex,
                                        unsigned char chosen,
                                        const char* champName) {
    struct ChampionState_Compat* champ;
    if (!state) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    if (championIndex >= state->world.party.championCount) return 0;
    champ = &state->world.party.champions[championIndex];
    if (!champ->present) return 0;

    switch (chosen) {
        case 5: {
            /* FLIP (F0407 C005_ACTION_FLIP): M005_RANDOM(2) ->
             * "IT COMES UP HEADS." or "IT COMES UP TAILS.".
             * Pure player-facing cue.  We draw the bit from the
             * world RNG so the outcome is deterministic for a
             * given game seed, matching the original's RNG use. */
            int roll = F0732_COMBAT_RngRandom_Compat(&state->world.masterRng, 2);
            m11_log_event(state, M11_COLOR_LIGHT_CYAN,
                          "T%u: IT COMES UP %s.",
                          (unsigned int)state->world.gameTick,
                          roll ? "HEADS" : "TAILS");
            return 1;
        }
        case 36: {
            /* HEAL (F0407 C036_ACTION_HEAL): transfer mana into
             * HP up to the healing cap derived from the champion's
             * HEAL skill level.  The original loop is:
             *     healCap = min(10, skillLevel)
             *     while (mana > 0 && missing > 0) {
             *         amount = min(missing, healCap)
             *         hp += amount; mana -= 2; missing -= amount;
             *     }
             * We mirror that arithmetic against the V1 champion
             * stat model.  Ref: F0407 case C036_ACTION_HEAL. */
            int missing;
            int healCap;
            int skillLevel;
            int healedTotal = 0;
            if (champ->hp.current >= champ->hp.maximum) {
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: %s IS ALREADY AT FULL HEALTH",
                              (unsigned int)state->world.gameTick,
                              champName);
                return 0;
            }
            if (champ->mana.current == 0) {
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s HAS NO MANA TO HEAL",
                              (unsigned int)state->world.gameTick,
                              champName);
                return 0;
            }
            skillLevel = M11_GameView_GetSkillLevel(state, championIndex,
                                                    CHAMPION_SKILL_PRIEST);
            if (skillLevel < 0) skillLevel = 0;
            healCap = skillLevel;
            if (healCap > 10) healCap = 10;
            if (healCap < 1) healCap = 1;
            missing = (int)champ->hp.maximum - (int)champ->hp.current;
            while (champ->mana.current > 0 && missing > 0) {
                int amount = (missing < healCap) ? missing : healCap;
                champ->hp.current = (unsigned short)(champ->hp.current + amount);
                healedTotal += amount;
                missing -= amount;
                if (champ->mana.current >= 2) {
                    champ->mana.current = (unsigned short)(champ->mana.current - 2);
                } else {
                    champ->mana.current = 0;
                    break;
                }
            }
            if (healedTotal > 0) {
                m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                              "T%u: %s HEALED %d HP",
                              (unsigned int)state->world.gameTick,
                              champName, healedTotal);
                return 1;
            }
            return 0;
        }
        case 38: {
            /* LIGHT (F0407 C038_ACTION_LIGHT):
             *     MagicalLightAmount += LightPowerToLightAmount[2]
             *     F0404_MENUS_CreateEvent70_Light(-2, 2500)
             *     F0405_MENUS_DecrementCharges(champion)
             *
             * The original lit-decay event is not yet wired into
             * our timeline, so we add the light amount directly
             * and let the existing light-level path pick it up.
             * The decay will be applied when the MagicState
             * timeline wiring lands; in the meantime the player
             * sees the viewport brighten, which is the
             * recognisable effect.  LightPowerToLightAmount[2] is
             * 6 in the bounded runtime-dynamics table. */
            state->world.magic.magicalLightAmount += 6;
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s CREATES MAGICAL LIGHT",
                          (unsigned int)state->world.gameTick,
                          champName);
            return 1;
        }
        case 11: {
            /* FREEZE LIFE (F0407 C011_ACTION_FREEZE_LIFE):
             * advance G0407_s_Party.FreezeLifeTicks by 70 for the
             * common weapon case (blue=30, green=125 branches
             * require junk-box typing we do not currently model
             * per-thing).  Capped at 200 as in F0407. */
            int32_t prev = state->world.freezeLifeTicks;
            int32_t next = prev + 70;
            if (next > 200) next = 200;
            state->world.freezeLifeTicks = next;
            /* Mirror into MagicState.freezeLifeTicks so the light
             * / creature-ai sides observe the freeze consistently. */
            state->world.magic.freezeLifeTicks = next;
            m11_log_event(state, M11_COLOR_LIGHT_BLUE,
                          "T%u: %s FREEZES TIME (%d TICKS)",
                          (unsigned int)state->world.gameTick,
                          champName, next - prev);
            return 1;
        }
        case 33: /* SPELLSHIELD */
        case 34: /* FIRESHIELD */ {
            /* F0407 C033_ACTION_SPELLSHIELD / C034_ACTION_FIRESHIELD:
             *     IsPartySpellOrFireShieldSuccessful(..., 280, TRUE)
             * which increases magicState.spellShieldDefense or
             * fireShieldDefense by a fixed amount.  The V1 slice
             * applies the shield boost directly so the player
             * sees the effect on the party HUD; creature damage
             * mitigation routes through the existing MagicState
             * reads in m11_draw_party_hud. */
            if (chosen == 33) {
                state->world.magic.spellShieldDefense += 6;
                m11_log_event(state, M11_COLOR_LIGHT_BLUE,
                              "T%u: %s RAISES SPELL SHIELD",
                              (unsigned int)state->world.gameTick,
                              champName);
            } else {
                state->world.magic.fireShieldDefense += 6;
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s RAISES FIRE SHIELD",
                              (unsigned int)state->world.gameTick,
                              champName);
            }
            return 1;
        }
        case 1:  /* BLOCK */
        case 17: /* PARRY */ {
            /* F0407 routes these through the action-disabled /
             * stamina path only (G0491 ActionDisabledTicks +
             * G0494 ActionStamina).  In V1 the defensive posture
             * is visible as "time passes" — stamina drains via
             * the orchestrator's survival pass, and the menu
             * closes.  Emit a bounded defensive log so the
             * player sees the champion is guarding. */
            m11_log_event(state, M11_COLOR_LIGHT_GRAY,
                          "T%u: %s TAKES A DEFENSIVE STANCE",
                          (unsigned int)state->world.gameTick,
                          champName);
            return 0;
        }
        case 8:  /* WAR CRY */
        case 4:  /* BLOW HORN */
        case 37: /* CALM */
        case 41: /* BRANDISH */
        case 22: /* CONFUSE */ {
            /* F0407 routes these through F0401_MENUS_IsGroupFrightenedByAction
             * against the creature group in front of the party.
             * The group-frighten model is not yet wired in V1;
             * what IS recognisable to the player is the sound and
             * the log cue.  Emit an audio marker (reuses the
             * creature/combat cue buffers) and a descriptive
             * log line that names the action's intent. */
            const char* verb;
            switch (chosen) {
                case 8:  verb = "LETS OUT A WAR CRY"; break;
                case 4:  verb = "BLOWS THE HORN"; break;
                case 37: verb = "TRIES TO CALM THE BEAST"; break;
                case 41: verb = "BRANDISHES A FEARSOME WEAPON"; break;
                case 22: verb = "ATTEMPTS TO CONFUSE THE FOE"; break;
                default: verb = "SHOUTS"; break;
            }
            m11_log_event(state, M11_COLOR_YELLOW,
                          "T%u: %s %s",
                          (unsigned int)state->world.gameTick,
                          champName, verb);
            if (chosen == 8) {
                /* Pass 55: ReDMCSB I34E maps WAR CRY to sound event 17
                 * (M619_SOUND_WAR_CRY / GRAPHICS.DAT SND3 item 707). */
                m11_audio_emit_source_sound(state, 17, M11_AUDIO_MARKER_CREATURE);
            } else if (chosen == 4) {
                /* Pass 55: ReDMCSB I34E maps BLOW HORN to sound event 18
                 * (M620_SOUND_BLOW_HORN / GRAPHICS.DAT SND3 item 704). */
                m11_audio_emit_source_sound(state, 18, M11_AUDIO_MARKER_CREATURE);
            } else {
                /* CALM / BRANDISH / CONFUSE are still V1-slice cues only;
                 * keep the honest marker fallback until their original
                 * runtime sound requests are captured or source-backed. */
                (void)M11_Audio_EmitMarker(&state->audioState,
                                           M11_AUDIO_MARKER_CREATURE);
            }
            return 0;
        }
        case 32: /* SHOOT */ {
            /* F0407 case C032_ACTION_SHOOT validates the action-
             * hand is a bow/sling class and the ready-hand is
             * matching ammunition; on failure it sets
             * G0513_i_ActionDamage = CM2_DAMAGE_NO_AMMUNITION and
             * returns AL1245_B_ActionPerformed=FALSE.  The
             * WeaponInfo class-check is outside the V1 slice, but
             * when the ready-hand holds something we can honour
             * the bounded DM1 effect by spawning a kinetic
             * projectile carrying that thing, mirroring
             * F0326_CHAMPION_ShootProjectile's essential step:
             * the player sees a projectile sprite leave the party
             * cell in the facing direction.  When the ready-hand
             * is empty we emit the authentic "NO AMMUNITION" cue. */
            unsigned short readyThing = champ->inventory[CHAMPION_SLOT_HAND_LEFT];
            int skillShoot = M11_GameView_GetSkillLevel(state, championIndex,
                                                        CHAMPION_SKILL_FIGHTER);
            int shootAttack;
            int spawned;
            if (readyThing == THING_NONE || readyThing == THING_ENDOFLIST) {
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s HAS NO AMMUNITION",
                              (unsigned int)state->world.gameTick,
                              champName);
                return 0;
            }
            if (skillShoot < 0) skillShoot = 0;
            /* F0407 SHOOT damage: (ShootAttack + SkillLevel) << 1;
             * without the WeaponInfo table we approximate
             * ShootAttack=20 which is the mid-class bow attack in
             * G0238.  Skill scaling matches the original shift. */
            shootAttack = (20 + skillShoot) << 1;
            if (shootAttack > 255) shootAttack = 255;
            spawned = m11_spawn_action_projectile(state, championIndex,
                                                  PROJECTILE_SUBTYPE_KINETIC_ARROW,
                                                  PROJECTILE_CATEGORY_KINETIC,
                                                  120, shootAttack,
                                                  COMBAT_ATTACK_NORMAL);
            m11_log_event(state, M11_COLOR_YELLOW,
                          "T%u: %s SHOOTS",
                          (unsigned int)state->world.gameTick,
                          champName);
            /* Pass 55: ReDMCSB I34E maps party melee/shoot/throw to
             * sound event 13 (M563_SOUND_COMBAT_ATTACK...). */
            m11_audio_emit_source_sound(state, 13, M11_AUDIO_MARKER_COMBAT);
            return spawned;
        }
        case 20:   /* FIREBALL */
        case 21:   /* DISPELL */
        case 23: { /* LIGHTNING */
            /* F0407 cases C020_ACTION_FIREBALL / C021_ACTION_DISPELL /
             * C023_ACTION_LIGHTNING.  Each picks a magical subtype,
             * a fixed kineticEnergy, decrements mana by the
             * skill-scaled required amount and spawns a projectile
             * via F0327_CHAMPION_IsProjectileSpellCast, which
             * bottoms out in F0212_PROJECTILE_Create.  The V1
             * source-backed route uses F0810 with the same subtype
             * mapping: FIREBALL -> 0x80, LIGHTNING_BOLT -> 0x82,
             * HARM_NON_MATERIAL -> 0x83.  Mana cost and skill path
             * mirror F0407: 7 - min(6, wizardSkill). */
            int subtype;
            int kinetic;
            int attackType;
            const char* verb;
            int impact;
            int manaCost;
            int skillWiz = M11_GameView_GetSkillLevel(state, championIndex,
                                                     CHAMPION_SKILL_WIZARD);
            int actualEnergy;
            int spawned;
            if (skillWiz < 0) skillWiz = 0;
            manaCost = 7 - (skillWiz > 6 ? 6 : skillWiz);
            if (manaCost < 1) manaCost = 1;
            switch (chosen) {
                case 20:
                    subtype    = PROJECTILE_SUBTYPE_FIREBALL;
                    kinetic    = 150;
                    attackType = COMBAT_ATTACK_FIRE;
                    verb       = "CASTS FIREBALL";
                    break;
                case 21:
                    subtype    = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
                    kinetic    = 150;
                    attackType = COMBAT_ATTACK_MAGIC;
                    verb       = "CASTS DISPELL";
                    break;
                case 23:
                default:
                    subtype    = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
                    kinetic    = 180;
                    attackType = COMBAT_ATTACK_LIGHTNING;
                    verb       = "CASTS LIGHTNING";
                    break;
            }
            /* F0407: if CurrentMana < RequiredMana, scale
             * kineticEnergy down proportionally and cap cost at
             * available mana (the "under-powered cast" path). */
            if ((int)champ->mana.current < manaCost) {
                if (manaCost > 0) {
                    actualEnergy = (int)champ->mana.current * kinetic / manaCost;
                } else {
                    actualEnergy = kinetic;
                }
                if (actualEnergy < 2) actualEnergy = 2;
                manaCost = (int)champ->mana.current;
            } else {
                actualEnergy = kinetic;
            }
            if (manaCost > 0) {
                if ((int)champ->mana.current >= manaCost) {
                    champ->mana.current = (uint16_t)((int)champ->mana.current - manaCost);
                } else {
                    champ->mana.current = 0;
                }
            }
            impact = m11_action_projectile_impact_attack(state, championIndex);
            spawned = m11_spawn_action_projectile(state, championIndex,
                                                  subtype,
                                                  PROJECTILE_CATEGORY_MAGICAL,
                                                  actualEnergy, impact,
                                                  attackType);
            m11_log_event(state,
                          chosen == 23 ? M11_COLOR_LIGHT_CYAN :
                          chosen == 21 ? M11_COLOR_LIGHT_BLUE :
                                         M11_COLOR_LIGHT_RED,
                          "T%u: %s %s",
                          (unsigned int)state->world.gameTick,
                          champName, verb);
            (void)M11_Audio_EmitMarker(&state->audioState,
                                       M11_AUDIO_MARKER_COMBAT);
            return spawned;
        }
        case 27: { /* INVOKE */
            /* F0407 case C027_ACTION_INVOKE: kineticEnergy =
             * RANDOM(128)+100 and the explosion type is chosen
             * from a 6-way random switch —
             *   0 -> POISON_BOLT
             *   1 -> POISON_CLOUD
             *   2 -> HARM_NON_MATERIAL
             *   default (3..5) -> FIREBALL
             * Each routes through the projectile-spell path with
             * the same mana/skill machinery as FIREBALL et al. */
            int subtype;
            int attackType;
            int roll;
            int energyRoll;
            int kinetic;
            int manaCost;
            int skillWiz = M11_GameView_GetSkillLevel(state, championIndex,
                                                     CHAMPION_SKILL_WIZARD);
            int impact;
            int actualEnergy;
            int spawned;
            const char* subtypeName;
            if (skillWiz < 0) skillWiz = 0;
            manaCost = 7 - (skillWiz > 6 ? 6 : skillWiz);
            if (manaCost < 1) manaCost = 1;
            roll = F0732_COMBAT_RngRandom_Compat(&state->world.masterRng, 6);
            energyRoll = F0732_COMBAT_RngRandom_Compat(&state->world.masterRng, 128);
            kinetic = energyRoll + 100;
            switch (roll) {
                case 0:
                    subtype = PROJECTILE_SUBTYPE_POISON_BOLT;
                    attackType = COMBAT_ATTACK_NORMAL;
                    subtypeName = "POISON BOLT";
                    break;
                case 1:
                    subtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
                    attackType = COMBAT_ATTACK_NORMAL;
                    subtypeName = "POISON CLOUD";
                    break;
                case 2:
                    subtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
                    attackType = COMBAT_ATTACK_MAGIC;
                    subtypeName = "DISPELL";
                    break;
                default:
                    subtype = PROJECTILE_SUBTYPE_FIREBALL;
                    attackType = COMBAT_ATTACK_FIRE;
                    subtypeName = "FIREBALL";
                    break;
            }
            if ((int)champ->mana.current < manaCost) {
                if (manaCost > 0) {
                    actualEnergy = (int)champ->mana.current * kinetic / manaCost;
                } else {
                    actualEnergy = kinetic;
                }
                if (actualEnergy < 2) actualEnergy = 2;
                manaCost = (int)champ->mana.current;
            } else {
                actualEnergy = kinetic;
            }
            if (manaCost > 0) {
                if ((int)champ->mana.current >= manaCost) {
                    champ->mana.current = (uint16_t)((int)champ->mana.current - manaCost);
                } else {
                    champ->mana.current = 0;
                }
            }
            impact = m11_action_projectile_impact_attack(state, championIndex);
            spawned = m11_spawn_action_projectile(state, championIndex,
                                                  subtype,
                                                  PROJECTILE_CATEGORY_MAGICAL,
                                                  actualEnergy, impact,
                                                  attackType);
            m11_log_event(state, M11_COLOR_MAGENTA,
                          "T%u: %s INVOKES %s",
                          (unsigned int)state->world.gameTick,
                          champName, subtypeName);
            (void)M11_Audio_EmitMarker(&state->audioState,
                                       M11_AUDIO_MARKER_COMBAT);
            return spawned;
        }
        case 42: { /* THROW */
            /* F0407 case C042_ACTION_THROW: removes the action-
             * hand object and launches it as a kinetic projectile
             * via F0328_CHAMPION_IsObjectThrown.  V1 source-backed
             * route: spawn a kinetic projectile at the party cell,
             * facing party direction, owner=champion.  We do NOT
             * remove the object from the slot yet — DM1's
             * F0300_CHAMPION_GetObjectRemovedFromSlot plumbing
             * requires thing-chain mutation we haven't ported
             * into the V1 slice.  The player still sees the
             * thrown-object projectile leave the party cell,
             * which is the recognisable classic-DM effect. */
            unsigned short handThing = m11_get_action_hand_thing(champ);
            int skillFight = M11_GameView_GetSkillLevel(state, championIndex,
                                                        CHAMPION_SKILL_FIGHTER);
            int throwAttack;
            int spawned;
            if (handThing == THING_NONE || handThing == THING_ENDOFLIST) {
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s HAS NOTHING TO THROW",
                              (unsigned int)state->world.gameTick,
                              champName);
                return 0;
            }
            if (skillFight < 0) skillFight = 0;
            /* Throw attack uses the fighter-skill-scaled kinetic
             * path; without WeaponInfo we settle on 15 as the
             * baseline thrown-object attack (mirrors the medium
             * club / dagger attack in G0238). */
            throwAttack = (15 + skillFight) << 1;
            if (throwAttack > 255) throwAttack = 255;
            spawned = m11_spawn_action_projectile(state, championIndex,
                                                  PROJECTILE_SUBTYPE_KINETIC_ARROW,
                                                  PROJECTILE_CATEGORY_KINETIC,
                                                  100, throwAttack,
                                                  COMBAT_ATTACK_NORMAL);
            m11_log_event(state, M11_COLOR_YELLOW,
                          "T%u: %s THROWS",
                          (unsigned int)state->world.gameTick,
                          champName);
            /* Pass 55: ReDMCSB I34E maps party melee/shoot/throw to
             * sound event 13 (M563_SOUND_COMBAT_ATTACK...). */
            m11_audio_emit_source_sound(state, 13, M11_AUDIO_MARKER_COMBAT);
            return spawned;
        }
        default:
            return 0;
    }
}

int M11_GameView_TriggerActionRow(M11_GameViewState* state,
                                  int actionListIndex) {
    int championIndex;
    unsigned char actions[3];
    int gotActions;
    unsigned char chosen;
    const char* actionName;
    const struct ChampionState_Compat* champ;
    char champName[16];
    int performed = 0;

    if (!state || !state->active) {
        return 0;
    }
    if (actionListIndex < 0 || actionListIndex >= 3) {
        /* DM1 F0391 silently returns FALSE for out-of-range
         * indices; preserve that behaviour here. */
        return 0;
    }
    if (state->actingChampionOrdinal == 0) {
        /* F0391 early exit: no acting champion, no action. */
        return 0;
    }

    championIndex = (int)state->actingChampionOrdinal - 1;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }
    if (championIndex >= state->world.party.championCount) {
        return 0;
    }
    champ = &state->world.party.champions[championIndex];
    if (!champ->present || champ->hp.current == 0) {
        return 0;
    }

    gotActions = M11_GameView_GetActingActionIndices(state, actions);
    if (!gotActions) {
        /* Matches F0391's "ActionList empty" early-exit path:
         * the menu is visible but the ActionSet resolved to NONE,
         * so we simply clear the menu without performing anything. */
        M11_GameView_ClearActingChampion(state);
        return 0;
    }
    chosen = actions[actionListIndex];
    if (chosen == 0xFF) {
        /* F0391: when the chosen list entry is ACTION_NONE the
         * function returns FALSE WITHOUT clearing the acting
         * champion — the menu stays open so the player can pick a
         * real action. */
        return 0;
    }

    actionName = M11_GameView_GetActionName(chosen);
    if (!actionName) actionName = "";

    /* DM1-style log line.  F0391 itself does not emit a message but
     * F0407_MENUS_IsActionPerformed and its downstream damage
     * resolution do; this is the visible cue the player gets that
     * the action actually fired. */
    m11_format_champion_name(champ->name, champName, sizeof(champName));
    if (actionName[0] != '\0') {
        m11_log_event(state, M11_COLOR_LIGHT_CYAN,
                      "T%u: %s: %s",
                      (unsigned int)state->world.gameTick,
                      champName,
                      actionName);
    }

    /* Melee-contact actions advance one CMD_ATTACK tick with the
     * chosen champion as the attacker.  This reuses the existing
     * orchestrator strike path so damage, creature hit overlay,
     * and message-log entries all route through the usual M10
     * machinery; M10 itself is untouched.
     *
     * Non-melee actions run through m11_perform_non_melee_action
     * for the bounded V1 slice (FLIP / HEAL / LIGHT / FREEZE
     * LIFE / SPELLSHIELD / FIRESHIELD / BLOCK / PARRY / WAR CRY /
     * BLOW HORN / CALM / BRANDISH / CONFUSE / SHOOT), which
     * applies real source-backed effects against the GameWorld
     * (MagicState, freezeLifeTicks, champion HP/mana) before we
     * advance a time-passes tick.  Actions outside that slice
     * still emit the log line and advance time so the menu
     * closure feels consistent with the rest of the UI.
     *
     * Either way the acting champion is selected as leader for
     * the closing frame — DM1 updates the leader portrait to the
     * acting champion while the action animation plays. */
    state->world.party.activeChampionIndex = championIndex;
    if (m11_action_is_melee_contact(chosen)) {
        /* Advance one tick with CMD_ATTACK via HandleInput so the
         * full M10 strike resolution runs (damage emission,
         * creature hit overlay, creature-hit log), identical to
         * pressing the A key. */
        (void)M11_GameView_HandleInput(state, M12_MENU_INPUT_ACTION);
        performed = 1;
    } else {
        /* Non-melee: apply bounded effect (if any), then advance
         * a CMD_NONE tick so "time passes" semantics hold —
         * stamina drains, action-disabled ticks roll forward,
         * creature AI resolves, freeze-life decrements are
         * observed.  This keeps the action-menu loop visibly
         * identical to DM1: the player picks an action, the
         * world advances one tick, the menu closes. */
        performed = m11_perform_non_melee_action(state, championIndex,
                                                 chosen, champName);
        (void)m11_apply_tick(state, CMD_NONE, "ACTION");
    }

    /* F0391 ALWAYS clears the acting champion before returning,
     * regardless of whether the action itself succeeded.  This is
     * the behaviour players recognise — picking an action closes
     * the menu and returns to idle icon-cell presentation. */
    M11_GameView_ClearActingChampion(state);

    return performed;
}

int M11_GameView_GetProjectileCount(const M11_GameViewState* state) {
    if (!state) return 0;
    return state->world.projectiles.count;
}

int M11_GameView_TriggerNonMeleeActionByIndex(M11_GameViewState* state,
                                              int championIndex,
                                              int actionIndex) {
    const struct ChampionState_Compat* champ;
    const char* actionName;
    char champName[16];
    int performed;
    if (!state || !state->active) return 0;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    if (championIndex >= state->world.party.championCount) return 0;
    champ = &state->world.party.champions[championIndex];
    if (!champ->present || champ->hp.current == 0) return 0;
    if (actionIndex < 0 || actionIndex >= 44) return 0;
    /* Melee-contact actions are handled through the CMD_ATTACK
     * path via M11_GameView_TriggerActionRow; refusing them here
     * keeps this helper scoped to the bounded non-melee slice. */
    if (m11_action_is_melee_contact((unsigned char)actionIndex)) return 0;

    actionName = M11_GameView_GetActionName((unsigned char)actionIndex);
    if (!actionName) actionName = "";
    m11_format_champion_name(champ->name, champName, sizeof(champName));
    if (actionName[0] != '\0') {
        m11_log_event(state, M11_COLOR_LIGHT_CYAN,
                      "T%u: %s: %s",
                      (unsigned int)state->world.gameTick,
                      champName, actionName);
    }
    state->actingChampionOrdinal = (unsigned int)(championIndex + 1);
    state->world.party.activeChampionIndex = championIndex;
    performed = m11_perform_non_melee_action(state, championIndex,
                                             (unsigned char)actionIndex,
                                             champName);
    (void)m11_apply_tick(state, CMD_NONE, "ACTION");
    M11_GameView_ClearActingChampion(state);
    return performed;
}

/* ---------------------------------------------------------------
 * DM1 action-menu mode (F0387_MENUS_DrawActionArea, menu branch).
 *
 * When G0506_ui_ActingChampionOrdinal is non-zero the action area
 * switches out of idle icon-cell mode and into a classic action
 * menu: the action-area graphic (graphic 10) is blitted fresh,
 * then the acting champion's name is printed into the header zone
 * in BLACK-ON-CYAN, and up to three action names from the
 * champion's action-hand ActionSet are printed into three action
 * rows in CYAN-ON-BLACK.  DM1 selects one of three zones for the
 * graphic blit depending on how many action rows are present:
 *
 *   all three actions      -> C011_ZONE_ACTION_AREA    (87×45)
 *   only two actions       -> C077_ZONE_ACTION_AREA_TWO_ACTIONS_MENU
 *   only one action        -> C079_ZONE_ACTION_AREA_ONE_ACTION_MENU
 *
 * We do not currently remap the graphic crop for the 1/2-action
 * sub-zones — graphic 10's natural layout already reads correctly
 * with the last one or two rows rendered as blank (NONE) cyan-on-
 * black strips, and mirroring DM1's zone-based crop would require
 * the ZONES graphic 555 table which we have not yet extracted.
 * This stays within the bounded slice while still rendering the
 * authentic header + action-row presentation.
 *
 * Geometry within the 87×45 action-area graphic (derived from live
 * DM1 screenshots and the zone-80/85/86/87 print calls in F0387):
 *
 *   Header band (champion name, black-on-cyan): y = 47..55
 *   Action row 0 (action name, cyan-on-black):  y = 58..66
 *   Action row 1:                                y = 69..77
 *   Action row 2:                                y = 80..88
 *
 * Text x-anchor is the action-area inner column at x=226 (2 px in
 * from the left frame edge at x=224), printed left-aligned so the
 * DM1 5px glyph fits within the 87px band.
 *
 * Ref: ReDMCSB ACTIDRAW.C F0387_MENUS_DrawActionArea menu-mode
 *      branch (lines 110..141), MENU.C G0489_as_Graphic560_
 *      ActionSets and G0490_ac_Graphic560_ActionNames. */
#define M11_DM_ACTION_MENU_HEADER_Y   47
#define M11_DM_ACTION_MENU_HEADER_H    9
#define M11_DM_ACTION_MENU_ROW_Y0     58
#define M11_DM_ACTION_MENU_ROW_STEP   11
#define M11_DM_ACTION_MENU_ROW_H       9
#define M11_DM_ACTION_MENU_TEXT_X    226

int M11_GameView_GetV1LeaderHandObjectNameZoneId(void) {
    /* Source layout-696 C017_ZONE_LEADER_HAND_OBJECT_NAME. */
    return 17;
}

int M11_GameView_GetV1LeaderHandObjectNameZone(int* outX,
                                               int* outY,
                                               int* outW,
                                               int* outH) {
    if (!M11_GameView_GetV1LeaderHandObjectNameZoneId()) return 0;
    if (outX) *outX = 233;
    if (outY) *outY = 33;
    if (outW) *outW = 87;
    if (outH) *outH = 6;
    return 1;
}

int M11_GameView_GetV1ActionAreaZoneId(void) {
    return 11;
}

int M11_GameView_GetV1ActionAreaZone(int* outX,
                                        int* outY,
                                        int* outW,
                                        int* outH) {
    if (!M11_GameView_GetV1ActionAreaZoneId()) return 0;
    if (outX) *outX = M11_DM_ACTION_AREA_X;
    if (outY) *outY = M11_DM_ACTION_AREA_Y;
    if (outW) *outW = M11_DM_ACTION_AREA_W;
    if (outH) *outH = M11_DM_ACTION_AREA_H;
    return 1;
}

int M11_GameView_GetV1SpellAreaZoneId(void) {
    return 13;
}

int M11_GameView_GetV1SpellAreaZone(int* outX,
                                       int* outY,
                                       int* outW,
                                       int* outH) {
    if (!M11_GameView_GetV1SpellAreaZoneId()) return 0;
    if (outX) *outX = M11_DM_SPELL_AREA_X;
    if (outY) *outY = M11_DM_SPELL_AREA_Y;
    if (outW) *outW = M11_DM_SPELL_AREA_W;
    if (outH) *outH = M11_DM_SPELL_AREA_H;
    return 1;
}

int M11_GameView_GetV1ActionAreaGraphicId(void) {
    return M11_GFX_ACTION_AREA;
}

int M11_GameView_GetV1ActionMenuGraphicZoneId(int actionRowCount) {
    if (actionRowCount <= 1) return 79;
    if (actionRowCount == 2) return 77;
    return 11;
}

int M11_GameView_GetV1ActionMenuGraphicZone(int actionRowCount,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH) {
    int zoneId = M11_GameView_GetV1ActionMenuGraphicZoneId(actionRowCount);
    int actionX, actionY;
    if (!zoneId || !M11_GameView_GetV1ActionAreaZone(&actionX, &actionY, NULL, NULL)) {
        return 0;
    }
    if (outX) *outX = actionX;
    if (outY) *outY = actionY;
    if (outW) *outW = 87;
    if (outH) *outH = (zoneId == 79) ? 21 : ((zoneId == 77) ? 33 : 45);
    return 1;
}

int M11_GameView_GetV1ActionAreaClearColor(void) {
    /* F0387 clears the action area to black before drawing menu/icon state. */
    return M11_COLOR_BLACK;
}

int M11_GameView_GetV1ActionResultZoneId(void) {
    /* Source layout-696 C075_ZONE_ACTION_RESULT covers the action result area. */
    return 75;
}

int M11_GameView_GetV1ActionResultZone(int* outX,
                                       int* outY,
                                       int* outW,
                                       int* outH) {
    if (!M11_GameView_GetV1ActionResultZoneId()) return 0;
    return M11_GameView_GetV1ActionAreaZone(outX, outY, outW, outH);
}

int M11_GameView_GetV1ActionPassZoneId(void) {
    /* Source layout-696 C098_ZONE_ACTION_AREA_PASS, under the C097 35x7 text area. */
    return 98;
}

int M11_GameView_GetV1ActionPassZone(int* outX,
                                     int* outY,
                                     int* outW,
                                     int* outH) {
    int actionX, actionY;
    if (!M11_GameView_GetV1ActionPassZoneId() ||
        !M11_GameView_GetV1ActionAreaZone(&actionX, &actionY, NULL, NULL)) {
        return 0;
    }
    if (outX) *outX = actionX + 51;
    if (outY) *outY = actionY;
    if (outW) *outW = 35;
    if (outH) *outH = 7;
    return 1;
}

int M11_GameView_GetV1SpellCasterPanelZoneId(void) {
    /* Source layout-696 C221_ZONE_SPELL_AREA_SET_MAGIC_CASTER. */
    return 221;
}

int M11_GameView_GetV1SpellCasterPanelZone(int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH) {
    int spellX, spellY;
    if (!M11_GameView_GetV1SpellCasterPanelZoneId() ||
        !M11_GameView_GetV1SpellAreaZone(&spellX, &spellY, NULL, NULL)) {
        return 0;
    }
    if (outX) *outX = spellX;
    if (outY) *outY = spellY;
    if (outW) *outW = 87;
    if (outH) *outH = 8;
    return 1;
}

int M11_GameView_GetV1SpellCasterTabZoneId(void) {
    if (!M11_GameView_GetV1SpellCasterPanelZoneId()) return 0;
    /* Source layout-696 C224_ZONE_SPELL_AREA_MAGIC_CASTER_TAB. */
    return 224;
}

int M11_GameView_GetV1SpellCasterTabZone(int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH) {
    int panelX, panelY;
    if (!M11_GameView_GetV1SpellCasterTabZoneId() ||
        !M11_GameView_GetV1SpellCasterPanelZone(&panelX, &panelY, NULL, NULL)) {
        return 0;
    }
    if (outX) *outX = panelX;
    if (outY) *outY = panelY;
    if (outW) *outW = 45;
    if (outH) *outH = 8;
    return 1;
}

int M11_GameView_GetV1ActionSpellStripZone(int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH) {
    int actionX, actionY, actionW, actionH;
    int spellX, spellY, spellW, spellH;
    (void)M11_GameView_GetV1ActionAreaZone(&actionX, &actionY, &actionW, &actionH);
    (void)M11_GameView_GetV1SpellAreaZone(&spellX, &spellY, &spellW, &spellH);
    if (outX) *outX = actionX < spellX ? actionX : spellX;
    if (outY) *outY = actionY < spellY ? actionY : spellY;
    if (outW) *outW = (actionX + actionW > spellX + spellW ?
                       actionX + actionW : spellX + spellW) -
                      (actionX < spellX ? actionX : spellX);
    if (outH) *outH = (actionY + actionH > spellY + spellH ?
                       actionY + actionH : spellY + spellH) -
                      (actionY < spellY ? actionY : spellY);
    return 1;
}

int M11_GameView_GetV1SpellAreaBackgroundGraphicId(void) {
    return M11_GFX_SPELL_AREA_BG;
}

int M11_GameView_GetV1ChampionPortraitGraphicId(void) {
    return M11_GFX_CHAMPION_PORTRAITS;
}

int M11_GameView_GetV1ChampionIconGraphicId(void) {
    return M11_GFX_CHAMPION_ICONS;
}

int M11_GameView_GetV1InventoryPanelGraphicId(void) {
    return M11_GFX_PANEL_EMPTY;
}

int M11_GameView_GetV1InventoryPanelZoneId(void) {
    /* Source layout-696 C101_ZONE_PANEL, centered at (152,89). */
    return 101;
}

int M11_GameView_GetV1InventoryPanelZone(int* outX,
                                          int* outY,
                                          int* outW,
                                          int* outH) {
    if (!M11_GameView_GetV1InventoryPanelZoneId()) return 0;
    if (outX) *outX = 80;
    if (outY) *outY = 52;
    if (outW) *outW = 144;
    if (outH) *outH = 73;
    return 1;
}

int M11_GameView_GetV1EndgameTheEndGraphicId(void) {
    return 6;
}

int M11_GameView_GetV1EndgameChampionMirrorGraphicId(void) {
    return 346;
}

int M11_GameView_GetV1DialogBackdropGraphicId(void) {
    return M11_GFX_DIALOG_BOX;
}

int M11_GameView_GetV1DialogVersionTextOrigin(int* outX, int* outY) {
    if (outX) *outX = M11_VIEWPORT_X + 192;
    if (outY) *outY = M11_VIEWPORT_Y + 7;
    return 1;
}

int M11_GameView_GetV1DialogChoicePatchZone(int choiceCount,
                                             int* outSrcX,
                                             int* outSrcY,
                                             int* outW,
                                             int* outH,
                                             int* outDstX,
                                             int* outDstY) {
    int sx, sy, w, h, dx, dy;
    if (choiceCount == 3) return 0;
    if (choiceCount <= 1) {
        /* M621_NEGGRAPHIC_DIALOG_PATCH_1_CHOICE -> C451. */
        sx = 0; sy = 14; w = 224; h = 75; dx = 0; dy = 51;
    } else if (choiceCount == 2) {
        /* M622_NEGGRAPHIC_DIALOG_PATCH_2_CHOICES -> C452. */
        sx = 102; sy = 52; w = 21; h = 37; dx = 102; dy = 89;
    } else {
        /* M623_NEGGRAPHIC_DIALOG_PATCH_4_CHOICES -> C453. */
        sx = 102; sy = 99; w = 21; h = 36; dx = 102; dy = 62;
    }
    if (outSrcX) *outSrcX = sx;
    if (outSrcY) *outSrcY = sy;
    if (outW) *outW = w;
    if (outH) *outH = h;
    if (outDstX) *outDstX = dx;
    if (outDstY) *outDstY = dy;
    return 1;
}

int M11_GameView_GetV1FoodLabelGraphicId(void) {
    return M11_GFX_FOOD_LABEL;
}

int M11_GameView_GetV1WaterLabelGraphicId(void) {
    return M11_GFX_WATER_LABEL;
}

int M11_GameView_GetV1FoodBarZoneId(void) {
    /* Source layout-696 C103_ZONE_FOOD_BAR. */
    return 103;
}

int M11_GameView_GetV1FoodBarZone(int* outX,
                                  int* outY,
                                  int* outW,
                                  int* outH,
                                  int* outSrcY) {
    if (!M11_GameView_GetV1FoodBarZoneId()) return 0;
    if (outX) *outX = 113;
    if (outY) *outY = 69;
    if (outW) *outW = 34;
    if (outH) *outH = 6;
    if (outSrcY) *outSrcY = 2;
    return 1;
}

int M11_GameView_GetV1FoodWaterPanelZoneId(void) {
    /* Source layout-696 C104_ZONE_FOOD_WATER. */
    return 104;
}

int M11_GameView_GetV1FoodWaterPanelZone(int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH,
                                         int* outSrcY) {
    if (!M11_GameView_GetV1FoodWaterPanelZoneId()) return 0;
    if (outX) *outX = 113;
    if (outY) *outY = 92;
    if (outW) *outW = 46;
    if (outH) *outH = 6;
    if (outSrcY) *outSrcY = 2;
    return 1;
}

int M11_GameView_GetV1ActionMenuHeaderZoneId(void) {
    /* F0387 prints the acting champion name through zone 80. */
    return 80;
}

int M11_GameView_GetV1ActionMenuHeaderZone(int* outX,
                                               int* outY,
                                               int* outW,
                                               int* outH) {
    (void)M11_GameView_GetV1ActionMenuHeaderZoneId();
    if (outX) *outX = M11_DM_ACTION_AREA_X;
    if (outY) *outY = M11_DM_ACTION_MENU_HEADER_Y;
    if (outW) *outW = M11_DM_ACTION_AREA_W;
    if (outH) *outH = M11_DM_ACTION_MENU_HEADER_H;
    return 1;
}

int M11_GameView_GetV1ActionMenuRowCount(void) {
    /* F0387 prints exactly three action rows for the selected ActionSet. */
    return 3;
}

int M11_GameView_GetV1ActionMenuRowBaseZoneId(int rowIndex) {
    if (rowIndex < 0 || rowIndex >= M11_GameView_GetV1ActionMenuRowCount()) return 0;
    /* Source action-row parent zones are C082, C083, and C084. */
    return 82 + rowIndex;
}

int M11_GameView_GetV1ActionMenuRowZoneId(int rowIndex) {
    if (!M11_GameView_GetV1ActionMenuRowBaseZoneId(rowIndex)) return 0;
    /* F0387 prints action names through zones 85, 86, and 87. */
    return 85 + rowIndex;
}

int M11_GameView_GetV1ActionMenuRowZone(int rowIndex,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH) {
    int zoneId = M11_GameView_GetV1ActionMenuRowZoneId(rowIndex);
    if (zoneId == 0) return 0;
    if (outX) *outX = M11_DM_ACTION_AREA_X;
    if (outY) *outY = M11_DM_ACTION_MENU_ROW_Y0 +
                      (zoneId - 85) * M11_DM_ACTION_MENU_ROW_STEP;
    if (outW) *outW = M11_DM_ACTION_AREA_W;
    if (outH) *outH = M11_DM_ACTION_MENU_ROW_H;
    return 1;
}

int M11_GameView_GetV1ActionMenuTextInset(int* outX,
                                           int* outY) {
    if (outX) *outX = 2;
    if (outY) *outY = 1;
    return 1;
}

int M11_GameView_GetV1ActionMenuTextOrigin(int rowIndex,
                                               int* outX,
                                               int* outY) {
    int zoneX, zoneY;
    int insetX, insetY;
    (void)M11_GameView_GetV1ActionMenuTextInset(&insetX, &insetY);
    if (rowIndex < 0) {
        if (!M11_GameView_GetV1ActionMenuHeaderZone(&zoneX, &zoneY,
                                                    NULL, NULL)) return 0;
        if (outX) *outX = zoneX + insetX;
        if (outY) *outY = zoneY + insetY;
        return 1;
    }
    if (!M11_GameView_GetV1ActionMenuRowZone(rowIndex, &zoneX, &zoneY,
                                             NULL, NULL)) return 0;
    if (outX) *outX = zoneX + insetX;
    if (outY) *outY = zoneY + insetY;
    return 1;
}

int M11_GameView_GetV1ActionMenuHeaderFillColor(void) {
    return M11_COLOR_CYAN;
}

int M11_GameView_GetV1ActionMenuHeaderTextColor(void) {
    return M11_COLOR_BLACK;
}

int M11_GameView_GetV1ActionMenuRowFillColor(void) {
    return M11_COLOR_BLACK;
}

int M11_GameView_GetV1ActionMenuRowTextColor(void) {
    return M11_COLOR_CYAN;
}

static int m11_draw_dm_action_menu(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight) {
    int actingIndex;
    unsigned char actions[3];
    int gotActions;
    int row;
    char nameBuf[16];
    M11_TextStyle styleBlackOnCyan;
    M11_TextStyle styleCyanOnBlack;
    const struct ChampionState_Compat* champ;
    if (!state) return 0;
    if (state->actingChampionOrdinal == 0) return 0;
    actingIndex = (int)state->actingChampionOrdinal - 1;
    if (actingIndex < 0 || actingIndex >= CHAMPION_MAX_PARTY) return 0;
    if (actingIndex >= state->world.party.championCount) return 0;
    champ = &state->world.party.champions[actingIndex];
    if (!champ->present) return 0;

    /* F0387 always fills the full action area with black before
     * blitting the menu graphic.  We've already blitted graphic 10
     * once at the top of m11_draw_utility_panel (as the right-
     * column chrome); here we re-fill+re-blit to ensure we start
     * from a clean action-mode surface regardless of any prior
     * icon-cell overpaint from an earlier frame. */
    {
        int actionX, actionY, actionW, actionH;
        (void)M11_GameView_GetV1ActionAreaZone(
            &actionX, &actionY, &actionW, &actionH);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      actionX, actionY, actionW, actionH,
                      (unsigned char)M11_GameView_GetV1ActionAreaClearColor());
        (void)m11_blit_panel_asset_native(state,
            framebuffer, framebufferWidth, framebufferHeight,
            M11_GameView_GetV1ActionAreaGraphicId(),
            actionW, actionH, actionX, actionY);
    }

    /* Header band: fill cyan and print the champion name in black.
     * Matches F0387's zone-80 print (black text, cyan background). */
    {
        int headerX, headerY, headerW, headerH;
        (void)M11_GameView_GetV1ActionMenuHeaderZone(
            &headerX, &headerY, &headerW, &headerH);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      headerX, headerY, headerW, headerH,
                      (unsigned char)M11_GameView_GetV1ActionMenuHeaderFillColor());
    }

    m11_format_champion_name(champ->name, nameBuf, sizeof(nameBuf));
    styleBlackOnCyan = g_text_small;
    styleBlackOnCyan.color = (unsigned char)M11_GameView_GetV1ActionMenuHeaderTextColor();
    styleBlackOnCyan.shadowDx = 0;
    styleBlackOnCyan.shadowDy = 0;
    styleBlackOnCyan.shadowColor = (unsigned char)M11_GameView_GetV1ActionMenuHeaderFillColor();
    {
        int textX, textY;
        (void)M11_GameView_GetV1ActionMenuTextOrigin(-1, &textX, &textY);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      textX, textY, nameBuf, &styleBlackOnCyan);
    }

    /* Action rows: each row is a black strip with cyan text.  Pull
     * the 3-tuple from the champion's action-hand ActionSet; any
     * C0xFF_ACTION_NONE entry is drawn as an empty strip (DM1's
     * F0384_MENUS_GetActionName returns "" for 0xFF). */
    styleCyanOnBlack = g_text_small;
    styleCyanOnBlack.color = (unsigned char)M11_GameView_GetV1ActionMenuRowTextColor();
    styleCyanOnBlack.shadowDx = 0;
    styleCyanOnBlack.shadowDy = 0;
    styleCyanOnBlack.shadowColor = (unsigned char)M11_GameView_GetV1ActionMenuRowFillColor();

    gotActions = M11_GameView_GetActingActionIndices(state, actions);
    for (row = 0; row < M11_GameView_GetV1ActionMenuRowCount(); ++row) {
        int rowX, rowY, rowW, rowH;
        const char* name;
        (void)M11_GameView_GetV1ActionMenuRowZone(
            row, &rowX, &rowY, &rowW, &rowH);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      rowX, rowY, rowW, rowH,
                      (unsigned char)M11_GameView_GetV1ActionMenuRowFillColor());
        if (!gotActions) continue;
        name = M11_GameView_GetActionName(actions[row]);
        if (!name || name[0] == '\0') continue;
        {
            int textX, textY;
            (void)M11_GameView_GetV1ActionMenuTextOrigin(row, &textX, &textY);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          textX, textY, name, &styleCyanOnBlack);
        }
    }
    return 1;
}

/* Draw the four DM1 action-hand icon cells across the right column,
 * matching F0386_MENUS_DrawActionIcon for every present champion.
 * Returns the number of cells drawn.
 *
 * Dead champions get a plain black cell (DM1: FillBox BLACK, return).
 * Living champions get a cyan-filled cell; the inner 16×16 icon box
 * is filled cyan as the icon backdrop.  Empty hands blit the source
 * C201_ICON_ACTION_ICON_EMPTY_HAND icon from object-icon graphics 42..48;
 * items with ActionSetIndex > 0 blit their source object icon from the
 * same atlases; ActionSetIndex == 0 items intentionally remain plain cyan.
 */
static int m11_draw_dm_action_icon_cells(const M11_GameViewState* state,
                                         unsigned char* framebuffer,
                                         int framebufferWidth,
                                         int framebufferHeight) {
    int drawn = 0;
    int slot;
    if (!state) return 0;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        int cellX, cellY, cellW, cellH;
        int isDead;
        int innerX, innerY, innerW, innerH;
        int drewSprite = 0;
        const struct ChampionState_Compat* champ;
        unsigned short handThing;

        if (slot >= state->world.party.championCount) break;
        champ = &state->world.party.champions[slot];
        if (!champ->present) continue;
        if (!M11_GameView_GetV1ActionIconCellZone(
                slot, &cellX, &cellY, &cellW, &cellH) ||
            !M11_GameView_GetV1ActionIconInnerZone(
                slot, &innerX, &innerY, &innerW, &innerH)) {
            continue;
        }
        isDead = (champ->hp.current == 0);

        if (isDead) {
            /* DM1: FillBox BLACK then return — no icon for dead. */
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cellX, cellY, cellW, cellH,
                          (unsigned char)M11_GameView_GetV1ActionIconCellBackdropColor(state, slot));
            ++drawn;
            continue;
        }

        /* Living champion: cyan cell backdrop. */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      cellX, cellY, cellW, cellH,
                      (unsigned char)M11_GameView_GetV1ActionIconCellBackdropColor(state, slot));

        /* Inner icon backdrop: DM1 fills the 16×16 bitmap with
         * C04_COLOR_CYAN when the hand has an object without an
         * action set (e.g. food) and then blits the icon on top.
         * We fill the inner box cyan too, so empty hands and
         * non-weapon items both read as the authentic cyan cell. */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      innerX, innerY, innerW, innerH,
                      M11_COLOR_CYAN);

        handThing = m11_get_action_hand_thing(champ);
        if (handThing == THING_NONE || handThing == THING_ENDOFLIST) {
            drewSprite = m11_draw_dm_object_icon_index(
                state, framebuffer, framebufferWidth, framebufferHeight,
                M11_DM_OBJECT_ICON_EMPTY_HAND,
                innerX, innerY, 1);
        } else if (state->assetsAvailable && state->world.things) {
            /* F0386 branch: only objects with a non-zero
             * ActionSetIndex get an icon blitted; everything else
             * (food, plain potions, armour, keys, chests, scrolls,
             * most junk) stays on the plain cyan icon backdrop.
             * This matches ReDMCSB MENUS.C F0386_MENUS_DrawActionIcon
             * exactly: when ActionSetIndex==0 the code fills the
             * icon bitmap with C04_COLOR_CYAN and jumps past
             * F0036_OBJECT_ExtractIconFromBitmap (no icon blit). */
            unsigned int actionSet = m11_action_set_index_for_thing(
                state->world.things, handThing);
            if (actionSet != 0) {
                int iconIndex = m11_object_icon_index_for_thing(
                    state, state->world.things, handThing);
                drewSprite = m11_draw_dm_object_icon_index(
                    state, framebuffer, framebufferWidth, framebufferHeight,
                    iconIndex, innerX, innerY, 1);
            }
        }
        (void)drewSprite;

        /* F0386 finishes by hatching the champion action cell when
         * actions are globally disabled by champion-candidate selection
         * or party resting (G0299_ui_CandidateChampionOrdinal /
         * G0300_B_PartyIsResting).  The original VGA hatch is a black
         * checker over the already-drawn cyan/icon cell.  M11 does not
         * yet carry the source MASK0x0008_DISABLE_ACTION bitfield for
         * per-champion cooldown, but the two global gates are present
         * in GameView state and should visibly match DM1. */
        if (M11_GameView_ShouldHatchV1ActionIconCells(state)) {
            m11_hatch_rect(framebuffer, framebufferWidth, framebufferHeight,
                           cellX, cellY, cellW, cellH);
        }
        ++drawn;
    }
    return drawn;
}

static unsigned short m11_get_status_hand_thing(const struct ChampionState_Compat* champ,
                                                int handIndex) {
    unsigned short t;
    if (!champ) return THING_NONE;
    if (handIndex == 0) {
        return champ->inventory[CHAMPION_SLOT_HAND_LEFT];
    }
    t = champ->inventory[CHAMPION_SLOT_ACTION_HAND];
    if (t != THING_NONE && t != THING_ENDOFLIST) return t;
    return champ->inventory[CHAMPION_SLOT_HAND_RIGHT];
}

int M11_GameView_GetV1StatusBoxZoneId(int championSlot) {
    if (championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY) return 0;
    return 151 + championSlot;
}

int M11_GameView_GetV1StatusBoxZone(int championSlot,
                                    int* outX,
                                    int* outY,
                                    int* outW,
                                    int* outH) {
    int zoneId = M11_GameView_GetV1StatusBoxZoneId(championSlot);
    if (!zoneId) {
        return 0;
    }
    if (outX) *outX = M11_PARTY_PANEL_X + (zoneId - 151) * m11_party_slot_step();
    if (outY) *outY = M11_PARTY_PANEL_Y;
    if (outW) *outW = M11_V1_PARTY_SLOT_W;
    if (outH) *outH = 29;
    return 1;
}

int M11_GameView_GetV1StatusBarGraphZoneId(int championSlot) {
    if (championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY) return 0;
    return 187 + championSlot;
}

int M11_GameView_GetV1StatusBarZoneId(int statIndex) {
    if (statIndex < 0 || statIndex > 2) return 0;
    return 195 + statIndex * 4;
}

int M11_GameView_GetV1StatusBarValueZoneId(int championSlot,
                                           int statIndex) {
    if (!M11_GameView_GetV1StatusBarGraphZoneId(championSlot) ||
        statIndex < 0 || statIndex > 2) {
        return 0;
    }
    return M11_GameView_GetV1StatusBarZoneId(statIndex) + championSlot;
}

int M11_GameView_GetV1StatusBarZone(int championSlot,
                                    int statIndex,
                                    int* outX,
                                    int* outY,
                                    int* outW,
                                    int* outH) {
    int relX;
    int zoneId = M11_GameView_GetV1StatusBarValueZoneId(championSlot, statIndex);
    if (!zoneId) {
        return 0;
    }
    championSlot = (zoneId - 195) % CHAMPION_MAX_PARTY;
    relX = m11_v1_bar_graph_x(statIndex);
    if (relX < 0) return 0;
    if (outX) *outX = M11_PARTY_PANEL_X + championSlot * m11_party_slot_step() + relX;
    if (outY) *outY = M11_PARTY_PANEL_Y + m11_v1_bar_graph_y_top();
    if (outW) *outW = M11_V1_BAR_CONTAINER_W;
    if (outH) *outH = M11_V1_BAR_CONTAINER_H;
    return 1;
}

int M11_GameView_GetV1StatusHandParentZoneId(int championSlot) {
    if (championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY) return 0;
    return 207 + championSlot;
}

int M11_GameView_GetV1StatusHandZoneId(int championSlot,
                                       int handIndex) {
    if (!M11_GameView_GetV1StatusHandParentZoneId(championSlot) ||
        handIndex < 0 || handIndex > 1) {
        return 0;
    }
    return 211 + championSlot * 2 + handIndex;
}

int M11_GameView_GetV1StatusHandZone(int championSlot,
                                     int handIndex,
                                     int* outX,
                                     int* outY,
                                     int* outW,
                                     int* outH) {
    int zoneId = M11_GameView_GetV1StatusHandZoneId(championSlot, handIndex);
    int sourceSlot;
    int sourceHand;
    if (zoneId == 0) return 0;
    sourceSlot = (zoneId - 211) / 2;
    sourceHand = (zoneId - 211) % 2;
    if (outX) *outX = M11_PARTY_PANEL_X + sourceSlot * m11_party_slot_step() +
                      m11_v1_status_hand_x(sourceHand);
    if (outY) *outY = M11_PARTY_PANEL_Y + M11_V1_STATUS_HAND_Y;
    if (outW) *outW = M11_V1_STATUS_HAND_ZONE_W;
    if (outH) *outH = M11_V1_STATUS_HAND_ZONE_H;
    return 1;
}

int M11_GameView_GetV1StatusHandIconZone(int championSlot,
                                         int handIndex,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH) {
    int handX, handY;
    if (!M11_GameView_GetV1StatusHandZone(championSlot, handIndex,
                                          &handX, &handY,
                                          NULL, NULL)) {
        return 0;
    }
    if (outX) *outX = handX + 1;
    if (outY) *outY = handY + 1;
    if (outW) *outW = 16;
    if (outH) *outH = 16;
    return 1;
}

int M11_GameView_GetV1StatusHandSlotBoxZone(int championSlot,
                                            int handIndex,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH) {
    int handX, handY;
    if (!M11_GameView_GetV1StatusHandZone(championSlot, handIndex,
                                          &handX, &handY,
                                          NULL, NULL)) {
        return 0;
    }
    /* CHAMDRAW.C F0291 draws the C033/C034/C035 hand-slot box
     * bitmap at the hand-zone origin.  Those source bitmaps are
     * 18x18, deliberately overhanging the 16x16 layout-696 parent
     * zone by one pixel on the right/bottom; the 16x16 object icon
     * is then drawn at +1,+1 inside the box. */
    if (outX) *outX = handX;
    if (outY) *outY = handY;
    if (outW) *outW = 18;
    if (outH) *outH = 18;
    return 1;
}

int M11_GameView_GetV1SlotBoxNormalGraphicId(void) {
    return M11_GFX_SLOT_BOX_NORMAL;
}

int M11_GameView_GetV1SlotBoxWoundedGraphicId(void) {
    return M11_GFX_SLOT_BOX_WOUNDED;
}

int M11_GameView_GetV1SlotBoxActingHandGraphicId(void) {
    return M11_GFX_SLOT_BOX_ACTING_HAND;
}

int M11_GameView_GetV1StatusHandSlotGraphic(const M11_GameViewState* state,
                                               int championSlot,
                                               int handIndex) {
    const struct ChampionState_Compat* champ;
    if (!state || championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY ||
        handIndex < 0 || handIndex > 1 ||
        championSlot >= state->world.party.championCount) {
        return 0;
    }
    champ = &state->world.party.champions[championSlot];
    if (!champ->present || champ->hp.current == 0) return 0;
    if (handIndex == 1 &&
        state->actingChampionOrdinal == (unsigned int)(championSlot + 1)) {
        /* Source F0291 assigns wounded/normal first, then acting hand
         * overrides it for C01_SLOT_ACTION_HAND. */
        return M11_GameView_GetV1SlotBoxActingHandGraphicId();
    }
    if (champ->wounds & (handIndex == 0 ? 0x0001u : 0x0002u)) {
        return M11_GameView_GetV1SlotBoxWoundedGraphicId();
    }
    return M11_GameView_GetV1SlotBoxNormalGraphicId();
}

int M11_GameView_GetV1StatusNameColor(const M11_GameViewState* state,
                                      int championSlot) {
    if (!state || championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY ||
        championSlot >= state->world.party.championCount ||
        !state->world.party.champions[championSlot].present) {
        return -1;
    }
    /* CHAMDRAW.C F0292: dead status boxes print the champion name in
     * C13 lightest gray.  Living leader name is C11 yellow; other
     * living champions are C09 gold. */
    if (state->world.party.champions[championSlot].hp.current == 0) {
        return M11_COLOR_SILVER;
    }
    return (championSlot == state->world.party.activeChampionIndex)
        ? M11_COLOR_YELLOW
        : M11_COLOR_ORANGE;
}

int M11_GameView_GetV1StatusNameClearColor(void) {
    /* CHAMDRAW.C F0292 clears C159+n with C01_COLOR_DARK_GRAY before
     * drawing the centered name into child zone C163+n.  In the DM PC
     * VGA palette used by M11, C01 is the gray slot. */
    return M11_COLOR_GRAY;
}

int M11_GameView_GetV1StatusBoxFillColor(void) {
    /* CHAMDRAW.C F0292 clears the live champion status box with
     * C12_COLOR_DARKEST_GRAY before drawing name, bars, and hands. */
    return M11_COLOR_DARK_GRAY;
}

int M11_GameView_GetV1StatusNameClearZoneId(int championSlot) {
    if (championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY) return 0;
    return 159 + championSlot;
}

int M11_GameView_GetV1StatusNameTextZoneId(int championSlot) {
    if (championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY) return 0;
    return 163 + championSlot;
}

int M11_GameView_GetV1StatusNameZone(int championSlot,
                                     int* outX,
                                     int* outY,
                                     int* outW,
                                     int* outH) {
    if (!M11_GameView_GetV1StatusNameClearZoneId(championSlot)) return 0;
    if (outX) *outX = M11_PARTY_PANEL_X + championSlot * m11_party_slot_step() +
                      M11_V1_STATUS_NAME_CLEAR_X;
    if (outY) *outY = M11_PARTY_PANEL_Y + M11_V1_STATUS_NAME_CLEAR_Y;
    if (outW) *outW = M11_V1_STATUS_NAME_CLEAR_W;
    if (outH) *outH = M11_V1_STATUS_NAME_CLEAR_H;
    return 1;
}

int M11_GameView_GetV1StatusNameTextZone(int championSlot,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH) {
    if (!M11_GameView_GetV1StatusNameTextZoneId(championSlot)) return 0;
    if (outX) *outX = M11_PARTY_PANEL_X + championSlot * m11_party_slot_step() +
                      M11_V1_STATUS_NAME_TEXT_X;
    if (outY) *outY = M11_PARTY_PANEL_Y + M11_V1_STATUS_NAME_TEXT_Y;
    if (outW) *outW = M11_V1_STATUS_NAME_TEXT_W;
    if (outH) *outH = M11_V1_STATUS_NAME_TEXT_H;
    return 1;
}

int M11_GameView_GetV1ActionIconParentZoneId(void) {
    /* Source action-hand icon template/root zone is C088 under C011. */
    return 88;
}

int M11_GameView_GetV1ActionIconCellZoneId(int championSlot) {
    if (!M11_GameView_GetV1ActionIconParentZoneId() ||
        championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY) return 0;
    /* Source action-hand icon cell parent zones are C089..C092. */
    return 89 + championSlot;
}

int M11_GameView_GetV1ActionIconCellZone(int championSlot,
                                             int* outX,
                                             int* outY,
                                             int* outW,
                                             int* outH) {
    int zoneId = M11_GameView_GetV1ActionIconCellZoneId(championSlot);
    if (zoneId == 0) return 0;
    if (outX) *outX = M11_DM_ACTION_ICON_CELL_X0 +
                      (zoneId - 89) * M11_DM_ACTION_ICON_CELL_STEP;
    if (outY) *outY = M11_DM_ACTION_ICON_CELL_Y;
    if (outW) *outW = M11_DM_ACTION_ICON_CELL_W;
    if (outH) *outH = M11_DM_ACTION_ICON_CELL_H;
    return 1;
}

int M11_GameView_GetV1ActionIconInnerZoneId(int championSlot) {
    if (championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY) return 0;
    /* Source inner icon zones are C093..C096. */
    return 93 + championSlot;
}

int M11_GameView_GetV1ActionIconInnerZone(int championSlot,
                                              int* outX,
                                              int* outY,
                                              int* outW,
                                              int* outH) {
    int cellX;
    if (!M11_GameView_GetV1ActionIconInnerZoneId(championSlot) ||
        !M11_GameView_GetV1ActionIconCellZone(championSlot,
                                              &cellX, NULL, NULL, NULL)) {
        return 0;
    }
    if (outX) *outX = cellX + M11_DM_ACTION_ICON_INNER_X_OFF;
    if (outY) *outY = M11_DM_ACTION_ICON_INNER_Y;
    if (outW) *outW = M11_DM_ACTION_ICON_INNER_W;
    if (outH) *outH = M11_DM_ACTION_ICON_INNER_H;
    return 1;
}

int M11_GameView_GetV1StatusBoxGraphicId(void) {
    return M11_GFX_STATUS_BOX;
}

int M11_GameView_GetV1DeadStatusBoxGraphicId(void) {
    return M11_GFX_STATUS_BOX_DEAD;
}

int M11_GameView_GetV1StatusBoxBaseGraphic(const M11_GameViewState* state,
                                           int championSlot) {
    const struct ChampionState_Compat* champ;
    if (!state || championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY ||
        championSlot >= state->world.party.championCount) {
        return 0;
    }
    champ = &state->world.party.champions[championSlot];
    if (!champ->present) return 0;
    return champ->hp.current == 0 ? M11_GameView_GetV1DeadStatusBoxGraphicId() : 0;
}

int M11_GameView_GetV1PartyShieldBorderGraphicId(void) {
    return M11_GFX_BORDER_PARTY_SHIELD;
}

int M11_GameView_GetV1FireShieldBorderGraphicId(void) {
    return M11_GFX_BORDER_PARTY_FIRESHIELD;
}

int M11_GameView_GetV1SpellShieldBorderGraphicId(void) {
    return M11_GFX_BORDER_PARTY_SPELLSHIELD;
}

int M11_GameView_GetV1StatusShieldBorderGraphic(const M11_GameViewState* state) {
    if (!state) return 0;
    if (state->world.magic.spellShieldDefense > 0) {
        return M11_GameView_GetV1SpellShieldBorderGraphicId();
    }
    if (state->world.magic.fireShieldDefense > 0) {
        return M11_GameView_GetV1FireShieldBorderGraphicId();
    }
    if (state->world.magic.partyShieldDefense > 0) {
        return M11_GameView_GetV1PartyShieldBorderGraphicId();
    }
    return 0;
}

int M11_GameView_GetV1PoisonLabelGraphicId(void) {
    return M11_GFX_POISONED_LABEL;
}

int M11_GameView_GetV1ChampionSmallDamageGraphicId(void) {
    return M11_GFX_DAMAGE_TO_CHAMPION_SMALL;
}

int M11_GameView_GetV1ChampionBigDamageGraphicId(void) {
    return M11_GFX_DAMAGE_TO_CHAMPION_BIG;
}

int M11_GameView_GetV1CreatureDamageGraphicId(void) {
    return M11_GFX_DAMAGE_TO_CREATURE;
}

int M11_GameView_GetV1StatusShieldBorderZone(int championSlot,
                                             int* outX,
                                             int* outY,
                                             int* outW,
                                             int* outH) {
    return M11_GameView_GetV1StatusBoxZone(championSlot,
                                           outX, outY, outW, outH);
}

int M11_GameView_GetV1PoisonLabelZone(int championSlot,
                                      int labelW,
                                      int labelH,
                                      int* outX,
                                      int* outY,
                                      int* outW,
                                      int* outH) {
    int boxX, boxY, boxW, boxH;
    if (labelW <= 0 || labelH <= 0) return 0;
    if (!M11_GameView_GetV1StatusBoxZone(championSlot,
                                         &boxX, &boxY, &boxW, &boxH)) {
        return 0;
    }
    if (outX) *outX = boxX + (boxW - labelW) / 2;
    if (outY) *outY = boxY + boxH;
    if (outW) *outW = labelW;
    if (outH) *outH = labelH;
    return 1;
}

int M11_GameView_GetV1DamageIndicatorZone(int championSlot,
                                          int indicatorW,
                                          int indicatorH,
                                          int* outX,
                                          int* outY,
                                          int* outW,
                                          int* outH) {
    int boxX, boxY, boxW, boxH;
    if (indicatorW <= 0 || indicatorH <= 0) return 0;
    if (!M11_GameView_GetV1StatusBoxZone(championSlot,
                                         &boxX, &boxY, &boxW, &boxH)) {
        return 0;
    }
    if (outX) *outX = boxX + (boxW - indicatorW) / 2;
    if (outY) *outY = boxY + (boxH - indicatorH) / 2;
    if (outW) *outW = indicatorW;
    if (outH) *outH = indicatorH;
    return 1;
}

int M11_GameView_GetV1DamageNumberOrigin(int championSlot,
                                         int* outX,
                                         int* outY) {
    int boxX, boxY, boxW, boxH;
    int dmgX, dmgY, dmgW, dmgH;
    if (!M11_GameView_GetV1StatusBoxZone(championSlot,
                                         &boxX, &boxY, &boxW, &boxH)) {
        return 0;
    }
    if (!M11_GameView_GetV1DamageIndicatorZone(championSlot,
                                               45, 7,
                                               &dmgX, &dmgY,
                                               &dmgW, &dmgH)) {
        return 0;
    }
    (void)boxY;
    (void)boxH;
    (void)dmgX;
    (void)dmgW;
    (void)dmgH;
    if (outX) *outX = boxX + boxW / 2 - 4;
    if (outY) *outY = dmgY;
    return 1;
}

int M11_GameView_GetV1StatusHandIconIndex(const M11_GameViewState* state,
                                          int championSlot,
                                          int handIndex) {
    const struct ChampionState_Compat* champ;
    unsigned short thing;
    int sourceSlotIndex;
    if (!state || championSlot < 0 || championSlot >= CHAMPION_MAX_PARTY ||
        handIndex < 0 || handIndex > 1 ||
        championSlot >= state->world.party.championCount) {
        return -1;
    }
    champ = &state->world.party.champions[championSlot];
    if (!champ->present || champ->hp.current == 0) return -1;

    thing = m11_get_status_hand_thing(champ, handIndex);
    if (thing != THING_NONE && thing != THING_ENDOFLIST) {
        return m11_object_icon_index_for_thing(state, state->world.things, thing);
    }

    /* ReDMCSB CHAMDRAW.C F0291_CHAMPION_DrawSlot:
     *   if empty and slot <= C05_SLOT_FEET:
     *     IconIndex = C212_ICON_READY_HAND + (SlotIndex << 1);
     *     if Wounds has (1 << SlotIndex), IconIndex++ and box=C034.
     * For status boxes only C00_SLOT_READY_HAND and
     * C01_SLOT_ACTION_HAND are drawn, so the exact empty-hand icons are:
     *   ready normal/wounded  = 212/213
     *   action normal/wounded = 214/215
     * The later acting-champion override changes only the slot-box graphic
     * (C035), not the selected empty-hand icon. */
    sourceSlotIndex = handIndex; /* C00_SLOT_READY_HAND / C01_SLOT_ACTION_HAND */
    return 212 + (sourceSlotIndex << 1) +
           ((champ->wounds & (1u << sourceSlotIndex)) ? 1 : 0);
}

static int m11_draw_v1_status_hand_slot(const M11_GameViewState* state,
                                        const struct ChampionState_Compat* champ,
                                        unsigned char* framebuffer,
                                        int framebufferWidth,
                                        int framebufferHeight,
                                        int championSlot,
                                        int handIndex,
                                        int dstX,
                                        int dstY) {
    int boxGfx;
    int drewBox = 0;
    int iconIndex;

    if (!state || !champ || !framebuffer) return 0;
    boxGfx = M11_GameView_GetV1StatusHandSlotGraphic(
        state, championSlot, handIndex);
    if (boxGfx <= 0) return 0;

    if (state->assetsAvailable) {
        const M11_AssetSlot* box = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, boxGfx);
        if (box && box->width == 18 && box->height == 18) {
            M11_AssetLoader_Blit(box, framebuffer, framebufferWidth,
                                 framebufferHeight, dstX, dstY, 0);
            drewBox = 1;
        }
    }
    if (!drewBox) {
        int boxX = dstX;
        int boxY = dstY;
        int boxW = 18;
        int boxH = 18;
        (void)M11_GameView_GetV1StatusHandSlotBoxZone(
            championSlot, handIndex, &boxX, &boxY, &boxW, &boxH);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      boxX, boxY, boxW, boxH, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      boxX, boxY, boxW, boxH, M11_COLOR_DARK_GRAY);
    }

    iconIndex = M11_GameView_GetV1StatusHandIconIndex(
        state, championSlot, handIndex);
    if (iconIndex < 0) return drewBox;

    {
        int iconX = dstX + 1;
        int iconY = dstY + 1;
        int iconW, iconH;
        (void)M11_GameView_GetV1StatusHandIconZone(
            championSlot, handIndex, &iconX, &iconY, &iconW, &iconH);
        (void)iconW;
        (void)iconH;
        (void)m11_draw_dm_object_icon_index(
            state, framebuffer, framebufferWidth, framebufferHeight,
            iconIndex, iconX, iconY, 0);
    }
    return 1;
}

static void m11_draw_utility_panel(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const struct DungeonMapDesc_Compat* mapDesc) {
    char line[32];
    char champion[16];
    const struct ChampionState_Compat* activeChampion = m11_get_active_champion(state);
    unsigned char accent = activeChampion ? M11_COLOR_LIGHT_GREEN : M11_COLOR_LIGHT_CYAN;
    int drewAuthenticFrames = 0;

    if (!state) {
        return;
    }

    if (m11_v2_vertical_slice_enabled()) {
        m11_blit_v2_slice_asset(&m11_v2_action_area_base,
                                framebuffer, framebufferWidth, framebufferHeight,
                                224, 45, 1);
        m11_blit_v2_slice_asset(&m11_v2_spell_area_base,
                                framebuffer, framebufferWidth, framebufferHeight,
                                M11_DM_SPELL_AREA_X, M11_DM_SPELL_AREA_Y, 1);
        m11_blit_v2_slice_asset(&m11_v2_spell_area_rune_bed,
                                framebuffer, framebufferWidth, framebufferHeight,
                                M11_DM_SPELL_AREA_X, M11_DM_SPELL_AREA_Y, 1);
        m11_blit_v2_slice_asset(state->spellPanelOpen
                                    ? &m11_v2_spell_area_active
                                    : &m11_v2_spell_area_highlight,
                                framebuffer, framebufferWidth, framebufferHeight,
                                M11_DM_SPELL_AREA_X, M11_DM_SPELL_AREA_Y, 1);
        drewAuthenticFrames = 1;
    }

    /* V1 mode: replace the procedural utility-panel backdrop with the
     * original GRAPHICS.DAT action area (graphic 10, 87x45) and the
     * spell area backdrop (graphic 9, 87x25) blitted at their
     * classic-DM screen coordinates.  This gives the right column the
     * authentic carved frame and avoids the flat-fill + hairline rect
     * that read as procedural.  Debug HUD keeps the procedural panel
     * so the ad-hoc I/S/L buttons still have a dark backdrop.
     *
     * The combined action+spell strip covers y=45..114, which lines up
     * with the utility panel's current vertical band (y=28..70) plus
     * the previously empty space above the party HUD (y=70..146).
     * Reference: ReDMCSB ACTIDRAW.C / CASTER.C and C011_ZONE_ACTION_AREA /
     * C013_ZONE_SPELL_AREA in ZONES.H. */
    if (!state->showDebugHUD && !m11_v2_vertical_slice_enabled()) {
        int actionX, actionY, actionW, actionH;
        int spellX, spellY, spellW, spellH;
        int drewAction;
        int drewSpell;
        (void)M11_GameView_GetV1ActionAreaZone(
            &actionX, &actionY, &actionW, &actionH);
        (void)M11_GameView_GetV1SpellAreaZone(
            &spellX, &spellY, &spellW, &spellH);
        drewAction = m11_blit_panel_asset_native(state,
            framebuffer, framebufferWidth, framebufferHeight,
            M11_GameView_GetV1ActionAreaGraphicId(),
            actionW, actionH, actionX, actionY);
        drewSpell = m11_blit_panel_asset_native(state,
            framebuffer, framebufferWidth, framebufferHeight,
            M11_GameView_GetV1SpellAreaBackgroundGraphicId(),
            spellW, spellH, spellX, spellY);
        if (drewAction && drewSpell) {
            drewAuthenticFrames = 1;
        } else {
            /* If only part loaded, clear and fall back to procedural so
             * we don't leave a half-original frame. */
            int stripX, stripY, stripW, stripH;
            (void)M11_GameView_GetV1ActionSpellStripZone(
                &stripX, &stripY, &stripW, &stripH);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          stripX, stripY, stripW, stripH,
                          (unsigned char)M11_GameView_GetV1ActionAreaClearColor());
        }
    }

    if (!drewAuthenticFrames) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      M11_UTILITY_PANEL_X, M11_UTILITY_PANEL_Y,
                      M11_UTILITY_PANEL_W, M11_UTILITY_PANEL_H, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      M11_UTILITY_PANEL_X, M11_UTILITY_PANEL_Y,
                      M11_UTILITY_PANEL_W, M11_UTILITY_PANEL_H, M11_COLOR_LIGHT_CYAN);
    }

    m11_get_active_champion_label(state, champion, sizeof(champion));

    if (state->showDebugHUD) {
        /* Debug mode: show MENU label, source kind, full metadata */
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      222, 34, "MENU", &g_text_small);
        snprintf(line, sizeof(line), "%s %s", m11_source_name(state->sourceKind), champion);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      250, 34, line, &g_text_small);
    } else if (activeChampion && !drewAuthenticFrames) {
        /* Legacy/procedural fallback only.  In normal V1 with the
         * original action-area graphic active, the idle/icon branch is
         * owned by F0386-style action-hand icon cells; champion names
         * belong to F0387 menu mode and are drawn there after a fresh
         * graphic-10 blit. */
        int nameY = 34;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      227, nameY, champion, &g_text_small);
    }

    line[0] = '\0';
    if (activeChampion && !(drewAuthenticFrames && !state->showDebugHUD)) {
        if (state->showDebugHUD) {
            snprintf(line, sizeof(line), "L%d HP%u ST%u",
                     mapDesc ? (int)mapDesc->level : 0,
                     (unsigned int)activeChampion->hp.current,
                     (unsigned int)activeChampion->stamina.current);
        } else {
            snprintf(line, sizeof(line), "HP%u ST%u",
                     (unsigned int)activeChampion->hp.current,
                     (unsigned int)activeChampion->stamina.current);
        }
    } else if (state->showDebugHUD) {
        snprintf(line, sizeof(line), "%s",
                 state->lastOutcome[0] != '\0' ? state->lastOutcome : "READY");
    }
    if (line[0] != '\0') {
        int statY = 44;
        int statX = 222;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      statX, statY, line, &g_text_small);
    }

    /* DM1 action-area mode selection — mirror
     * F0387_MENUS_DrawActionArea's branch on
     * G0509_B_ActionAreaContainsIcons / G0506_ui_ActingChampionOrdinal.
     *
     *  actingChampionOrdinal == 0  -> icon-mode (four action-hand
     *                                 cells, one per champion)
     *  actingChampionOrdinal >  0  -> menu-mode (graphic 10 header
     *                                 with champion name + up to
     *                                 three action names)
     *
     * Both modes only engage once the authentic frame blits have
     * succeeded and the V1 presentation is active.  Debug HUD keeps
     * the legacy utility-panel rendering untouched. */
    if (drewAuthenticFrames && !state->showDebugHUD) {
        if (state->actingChampionOrdinal != 0 &&
            m11_draw_dm_action_menu(state, framebuffer,
                                    framebufferWidth,
                                    framebufferHeight)) {
            /* Menu mode rendered; icon cells are suppressed, matching
             * DM1's F0387 menu-mode branch which fills the whole
             * action area before drawing the menu. */
        } else {
            /* F0387 icon-mode branch fills C011_ZONE_ACTION_AREA black
             * before drawing the four action-hand cells.  Keep the
             * spell-area frame below intact; the tall cyan cells then
             * overdraw y=86..120 exactly like F0386. */
            {
                int actionX, actionY, actionW, actionH;
                (void)M11_GameView_GetV1ActionAreaZone(
                    &actionX, &actionY, &actionW, &actionH);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              actionX, actionY, actionW, actionH,
                              (unsigned char)M11_GameView_GetV1ActionAreaClearColor());
            }
            (void)m11_draw_dm_action_icon_cells(state, framebuffer,
                                                framebufferWidth,
                                                framebufferHeight);
        }
    }

    if (state->showDebugHUD) {
        /* Debug mode: show I/S/L button labels */
        m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                                M11_UTILITY_INSPECT_X, M11_UTILITY_BUTTON_Y,
                                M11_UTILITY_INSPECT_W, M11_UTILITY_BUTTON_H,
                                "I", -1,
                                accent, M11_COLOR_BLACK);
        m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                                M11_UTILITY_SAVE_X, M11_UTILITY_BUTTON_Y,
                                M11_UTILITY_SAVE_W, M11_UTILITY_BUTTON_H,
                                "S", -1,
                                M11_COLOR_YELLOW, M11_COLOR_BLACK);
        m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                                M11_UTILITY_LOAD_X, M11_UTILITY_BUTTON_Y,
                                M11_UTILITY_LOAD_W, M11_UTILITY_BUTTON_H,
                                "L", -1,
                                M11_COLOR_LIGHT_BLUE, M11_COLOR_BLACK);
    }

    /* Light level indicator: debug/procedural fallback only.  The
     * normal V1 action/spell strip is source-owned; invented light
     * bars make the DM1 action area look like a debug utility panel. */
    if (!drewAuthenticFrames || state->showDebugHUD) {
        int lightLevel = m11_compute_light_level(state);
        unsigned char lightColor;
        const char* lightLabel;
        int barW;
        if (lightLevel >= 200) {
            lightColor = M11_COLOR_YELLOW;
            lightLabel = "BRIGHT";
        } else if (lightLevel >= 120) {
            lightColor = M11_COLOR_BROWN;
            lightLabel = "LIT";
        } else if (lightLevel >= 50) {
            lightColor = M11_COLOR_DARK_GRAY;
            lightLabel = "DIM";
        } else {
            lightColor = M11_COLOR_BLACK;
            lightLabel = "DARK";
        }
        int barY = 67;
        int barFillY = barY + 1;
        barW = (lightLevel * 80) / M11_LIGHT_MAX;
        if (barW < 1 && lightLevel > 0) barW = 1;
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      222, barY, 80, 5, M11_COLOR_DARK_GRAY);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      222, barFillY, 80, 3, M11_COLOR_DARK_GRAY);
        if (barW > 0) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          222, barFillY, barW, 3, lightColor);
        }
        if (state->showDebugHUD) {
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          222, 73, lightLabel, &g_text_small);
        }
    }
}

static void m11_draw_viewport(const M11_GameViewState* state,
                              unsigned char* framebuffer,
                              int framebufferWidth,
                              int framebufferHeight) {
    static const M11_ViewRect viewport = {M11_VIEWPORT_X, M11_VIEWPORT_Y, M11_VIEWPORT_W, M11_VIEWPORT_H};
    static const M11_ViewRect frames[4] = {
        {8, 41, 208, 120},
        {34, 54, 156, 88},
        {58, 68, 108, 58},
        {78, 79, 68, 36}
    };
    M11_ViewportCell cells[3][3];
    int depth;
    int occluded = 0;

    memset(cells, 0, sizeof(cells));

    for (depth = 0; depth < 3; ++depth) {
        int side;
        for (side = 0; side < 3; ++side) {
            (void)m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side]);
        }
    }

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  viewport.x, viewport.y, viewport.w, viewport.h, M11_COLOR_BLACK);
    /* Real viewport background from GRAPHICS.DAT, or solid fallback */
    m11_draw_viewport_background(state, framebuffer, framebufferWidth, framebufferHeight,
                                 viewport.x, viewport.y, viewport.w, viewport.h);
    if (state->showDebugHUD) {
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      viewport.x - 2, viewport.y - 2, viewport.w + 4, viewport.h + 4, M11_COLOR_YELLOW);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      viewport.x, viewport.y, viewport.w, viewport.h, M11_COLOR_LIGHT_CYAN);
    }

    /* First source-bound wall passes: draw blocked side/front square
     * panels using original wall-set bitmaps and original layout-696
     * zones.  This is still narrower than full DUNVIEW.C: ornaments,
     * doors, pits, stairs, fields, and exact object order remain next. */
    m11_draw_dm1_floor_pits(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_floor_ornaments(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_side_walls(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_front_walls(state, framebuffer, framebufferWidth, framebufferHeight, cells);
    m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_stairs(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_teleporter_fields(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_side_doors(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_side_door_ornaments(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_side_destroyed_door_masks(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_dm1_center_doors(state, framebuffer, framebufferWidth, framebufferHeight, cells);
    m11_draw_dm1_center_door_ornaments(state, framebuffer, framebufferWidth, framebufferHeight, cells);
    m11_draw_dm1_center_destroyed_door_masks(state, framebuffer, framebufferWidth, framebufferHeight, cells);
    m11_draw_dm1_center_door_buttons(state, framebuffer, framebufferWidth, framebufferHeight, cells);
    m11_draw_dm1_d3r_door_button(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Until the full C2500/C3200 object+creature zone pass lands, keep
     * visible center-lane objects and creatures alive in normal V1 by
     * drawing the existing source-asset-backed contents layer over open
     * center cells.  The old procedural wall geometry remains debug-only;
     * this call only draws floor ornaments/items/creatures/projectiles for
     * open cells and gives M612/M618 changes a visual gate. */
    occluded = 0;
    for (depth = 0; depth < 3; ++depth) {
        if (!occluded) {
            m11_draw_wall_contents(framebuffer, framebufferWidth, framebufferHeight,
                                   &frames[depth + 1], &cells[depth][1], depth);
            if (!m11_viewport_cell_is_open(&cells[depth][1])) {
                occluded = 1;
            }
        }
    }

    /* The Firestaff procedural corridor/trapezoid renderer is not DM1
     * DRAWVIEW output.  It stays available in debug HUD mode, but normal
     * V1 should not draw these invented wall panels over the source floor
     * and ceiling base.  Next source-bound pass should replace this with
     * DUNVIEW.C wall zones C702..C717 and wall-set entries 93..107. */
    if (state->showDebugHUD) {
        for (depth = 0; depth < 3; ++depth) {
            m11_draw_corridor_frame(framebuffer, framebufferWidth, framebufferHeight,
                                    &frames[depth], &frames[depth + 1], depth);
        }

        for (depth = 0; depth < 3; ++depth) {
            m11_draw_side_feature(framebuffer, framebufferWidth, framebufferHeight,
                                  &frames[depth], &frames[depth + 1], &cells[depth][0], -1, depth);
            m11_draw_side_feature(framebuffer, framebufferWidth, framebufferHeight,
                                  &frames[depth], &frames[depth + 1], &cells[depth][2], 1, depth);
            if (!occluded) {
                m11_draw_wall_face(framebuffer, framebufferWidth, framebufferHeight,
                                   &frames[depth + 1], &cells[depth][1], depth);
                if (!m11_viewport_cell_is_open(&cells[depth][1])) {
                    occluded = 1;
                }
            }
        }

        occluded = 0;
        for (depth = 0; depth < 3; ++depth) {
            if (!occluded) {
                m11_draw_wall_contents(framebuffer, framebufferWidth, framebufferHeight,
                                       &frames[depth + 1], &cells[depth][1], depth);
                if (!m11_viewport_cell_is_open(&cells[depth][1])) {
                    occluded = 1;
                }
            }
        }
    }

    if (state->showDebugHUD) {
        m11_draw_lane_chip(framebuffer, framebufferWidth, framebufferHeight,
                           viewport.x + 26, viewport.y + 6, 42, 11,
                           &cells[0][0], "L");
        m11_draw_lane_chip(framebuffer, framebufferWidth, framebufferHeight,
                           viewport.x + 77, viewport.y + 6, 42, 11,
                           &cells[0][1], "F");
        m11_draw_lane_chip(framebuffer, framebufferWidth, framebufferHeight,
                           viewport.x + 128, viewport.y + 6, 42, 11,
                           &cells[0][2], "R");

        m11_draw_depth_chip(framebuffer, framebufferWidth, framebufferHeight,
                            viewport.x + 40, viewport.y + viewport.h - 24, 34, 10,
                            &cells[0][1], 0);
        m11_draw_depth_chip(framebuffer, framebufferWidth, framebufferHeight,
                            viewport.x + 88, viewport.y + viewport.h - 24, 34, 10,
                            &cells[1][1], 1);
        m11_draw_depth_chip(framebuffer, framebufferWidth, framebufferHeight,
                            viewport.x + 136, viewport.y + viewport.h - 24, 34, 10,
                            &cells[2][1], 2);

        m11_draw_focus_brackets(framebuffer, framebufferWidth, framebufferHeight,
                                &frames[1], &cells[0][1]);

        m11_draw_viewport_feedback_frame(framebuffer, framebufferWidth, framebufferHeight,
                                         state, &cells[0][1]);
        m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                       viewport.x + 6, viewport.x + viewport.w - 7,
                       viewport.y + viewport.h / 2 + 8, M11_COLOR_LIGHT_GRAY);
    }

    /* Debug/prototype only: the old Firestaff path tiled GRAPHICS.DAT
     * wall/floor strips across procedural trapezoids.  That creates the
     * noisy, speckled "not DM1" viewport Daniel called out.  Normal V1
     * must wait for source-bound DRAWVIEW-style wall/floor placement
     * instead of this placeholder tiling. */
    if (state->assetsAvailable && state->showDebugHUD) {
        int d;
        int mapWallSet = m11_current_map_wall_set(state);
        int mapFloorSet = m11_current_map_floor_set(state);
        for (d = 2; d >= 0; --d) {
            const M11_AssetSlot* wallSlot;
            unsigned int wallIdx = (unsigned int)(M11_GFX_WALL_SET_0 + (mapWallSet & 3));
            wallSlot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, wallIdx);
            if (wallSlot && wallSlot->width > 0 && wallSlot->height > 0) {
                int ceilH = frames[d + 1].y - frames[d].y;
                int floorY = frames[d + 1].y + frames[d + 1].h;
                int floorH = (frames[d].y + frames[d].h) - floorY;
                int leftW = frames[d + 1].x - frames[d].x;
                int rightX = frames[d + 1].x + frames[d + 1].w;
                int rightW = (frames[d].x + frames[d].w) - rightX;
                /* Ceiling strip */
                if (ceilH > 2) {
                    M11_AssetLoader_BlitScaled(wallSlot, framebuffer,
                        framebufferWidth, framebufferHeight,
                        frames[d].x + 1, frames[d].y + 1,
                        frames[d].w - 2, ceilH - 1, -1);
                }
                /* Left wall strip */
                if (leftW > 2) {
                    M11_AssetLoader_BlitScaled(wallSlot, framebuffer,
                        framebufferWidth, framebufferHeight,
                        frames[d].x + 1, frames[d + 1].y,
                        leftW - 1, frames[d + 1].h, -1);
                }
                /* Right wall strip */
                if (rightW > 2) {
                    M11_AssetLoader_BlitScaled(wallSlot, framebuffer,
                        framebufferWidth, framebufferHeight,
                        rightX + 1, frames[d + 1].y,
                        rightW - 1, frames[d + 1].h, -1);
                }
                /* Floor strip — use the current map's floor set */
                if (floorH > 2) {
                    const M11_AssetSlot* floorSlot = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader,
                        (unsigned int)(M11_GFX_FLOOR_SET_0 + (mapFloorSet & 1)));
                    if (floorSlot && floorSlot->width > 0 && floorSlot->height > 0) {
                        M11_AssetLoader_BlitScaled(floorSlot, framebuffer,
                            framebufferWidth, framebufferHeight,
                            frames[d].x + 1, floorY + 1,
                            frames[d].w - 2, floorH - 1, -1);
                    }
                }
            }
        }
    }

    /* Apply dynamic depth dimming based on the party's current light
     * level.  The original DM1 uses torches, FUL spell, ILLUMULET, and
     * FLAMITT to illuminate the dungeon.  We map the computed 0..255
     * light level to dimming passes per depth band:
     *
     *   light >= 200  (bright):  no dimming at any depth
     *   light >= 120  (normal):  depth 2 dimmed once
     *   light >= 50   (dim):     depth 1 once, depth 2 twice
     *   light <  50   (dark):    depth 0 once, depth 1 twice, depth 2 three times
     */
    {
        int lightLevel = m11_compute_light_level(state);
        if (lightLevel < 50) {
            /* Very dark: heavy dimming everywhere */
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[0].x, frames[0].y,
                                    frames[0].w, frames[0].h, 1);
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[1].x, frames[1].y,
                                    frames[1].w, frames[1].h, 2);
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[2].x, frames[2].y,
                                    frames[2].w, frames[2].h, 3);
        } else if (lightLevel < 120) {
            /* Dim: moderate dimming */
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[1].x, frames[1].y,
                                    frames[1].w, frames[1].h, 1);
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[2].x, frames[2].y,
                                    frames[2].w, frames[2].h, 2);
        } else if (lightLevel < 200) {
            /* Normal: only the far band dimmed once */
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[2].x, frames[2].y,
                                    frames[2].w, frames[2].h, 1);
        }
        /* lightLevel >= 200: bright, no dimming */
    }
}

static void m11_format_champion_name(const unsigned char* raw,
                                     char* out,
                                     size_t outSize) {
    size_t i;
    size_t end = 0;
    if (!out || outSize == 0U) {
        return;
    }
    if (!raw) {
        snprintf(out, outSize, "EMPTY");
        return;
    }
    for (i = 0; i + 1 < outSize && i < CHAMPION_NAME_LENGTH; ++i) {
        unsigned char ch = raw[i];
        if (ch == 0U) {
            break;
        }
        out[i] = isprint(ch) ? (char)ch : ' ';
        if (out[i] != ' ') {
            end = i + 1;
        }
    }
    out[i < outSize ? i : outSize - 1] = '\0';
    out[end] = '\0';
    if (out[0] == '\0') {
        snprintf(out, outSize, "EMPTY");
    }
}

static void m11_format_champion_title(const unsigned char* raw,
                                      char* out,
                                      size_t outSize) {
    size_t i;
    size_t end = 0;
    if (!out || outSize == 0U) return;
    out[0] = '\0';
    if (!raw) return;
    for (i = 0; i + 1 < outSize && i < CHAMPION_TITLE_LENGTH; ++i) {
        unsigned char ch = raw[i];
        if (ch == 0U) break;
        out[i] = isprint(ch) ? (char)ch : ' ';
        if (out[i] != ' ') end = i + 1;
    }
    out[i < outSize ? i : outSize - 1] = '\0';
    out[end] = '\0';
}

int M11_GameView_EndgameTitleXForSourceText(const char* name, const char* title) {
    int titleX;
    char firstTitleChar;
    if (!name) name = "";
    if (!title || !title[0]) return 87 + ((int)strlen(name) * 6);
    titleX = 87 + ((int)strlen(name) * 6);
    firstTitleChar = title[0];
    if (firstTitleChar != ',' && firstTitleChar != ';' && firstTitleChar != '-') {
        titleX += 6;
    }
    return titleX;
}

static int m11_endgame_source_skill_level(const M11_GameViewState* state,
                                           int championIndex,
                                           int baseSkillIndex) {
    int lifecycleLevel = 1;
    int storedLevel = 1;
    if (!state || championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY ||
        baseSkillIndex < 0 || baseSkillIndex >= CHAMPION_SKILL_COUNT) {
        return 1;
    }
    if (!state->world.party.champions[championIndex].present) {
        return 1;
    }
    storedLevel = (int)state->world.party.champions[championIndex].skillLevels[baseSkillIndex];
    lifecycleLevel = F0848_LIFECYCLE_ComputeSkillLevel_Compat(
        &state->world.lifecycle.champions[championIndex],
        baseSkillIndex,
        1); /* ENDGAME.C uses IGNORE_TEMPORARY_EXPERIENCE. */
    if (lifecycleLevel < storedLevel) lifecycleLevel = storedLevel;
    if (lifecycleLevel > 16) lifecycleLevel = 16;
    if (lifecycleLevel < 1) lifecycleLevel = 1;
    return lifecycleLevel;
}

static void m11_get_active_champion_label(const M11_GameViewState* state,
                                          char* out,
                                          size_t outSize) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!state || !state->active ||
        state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY ||
        !state->world.party.champions[state->world.party.activeChampionIndex].present) {
        snprintf(out, outSize, "LEADER");
        return;
    }
    m11_format_champion_name(state->world.party.champions[state->world.party.activeChampionIndex].name,
                             out,
                             outSize);
}

static int m11_cycle_active_champion(M11_GameViewState* state) {
    int start;
    int step;
    char champion[16];

    if (!state || !state->active || state->world.party.championCount <= 0) {
        return 0;
    }

    start = state->world.party.activeChampionIndex;
    if (start < 0 || start >= CHAMPION_MAX_PARTY ||
        !state->world.party.champions[start].present) {
        start = 0;
    }

    for (step = 1; step <= CHAMPION_MAX_PARTY; ++step) {
        int candidate = (start + step) % CHAMPION_MAX_PARTY;
        if (candidate < state->world.party.championCount &&
            state->world.party.champions[candidate].present) {
            state->world.party.activeChampionIndex = candidate;
            m11_get_active_champion_label(state, champion, sizeof(champion));
            m11_set_status(state, "CHAMP", "ACTIVE CHAMPION READY");
            snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s READY", champion);
            snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                     "SPACE ACTS, ENTER INSPECTS, CLICK CENTER TO COMMIT, TAB CYCLES THE FRONT CHAMPION");
            return 1;
        }
    }

    return 0;
}

static int m11_set_active_champion(M11_GameViewState* state, int championIndex) {
    char champion[16];

    if (!state || !state->active || championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }
    if (championIndex >= state->world.party.championCount ||
        !state->world.party.champions[championIndex].present) {
        return 0;
    }
    if (state->world.party.activeChampionIndex == championIndex) {
        return 1;
    }

    state->world.party.activeChampionIndex = championIndex;
    m11_get_active_champion_label(state, champion, sizeof(champion));
    m11_set_status(state, "CHAMP", "ACTIVE CHAMPION READY");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s READY", champion);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "CLICK OR TAB TO SWAP, CLICK CENTER TO COMMIT, SPACE ACTS, ENTER INSPECTS");
    return 1;
}

static void m11_draw_party_panel(const M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight) {
    int slot;
    int activeIndex = -1;
    int useV2PartyHud = 0;
    int slotStep;
    int slotW;
    if (state) {
        activeIndex = state->world.party.activeChampionIndex;
        useV2PartyHud = m11_v2_vertical_slice_enabled();
    }
    slotStep = m11_party_slot_step();
    slotW    = m11_party_slot_w();
    if (useV2PartyHud) {
        m11_blit_v2_slice_asset(&m11_v2_party_hud_four_slot_base,
                                framebuffer, framebufferWidth, framebufferHeight,
                                M11_PARTY_PANEL_X, M11_PARTY_PANEL_Y, 1);
    }
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        int x = M11_PARTY_PANEL_X + slot * slotStep;
        int y = M11_PARTY_PANEL_Y;
        int slotH = M11_PARTY_SLOT_H;
        char line[48];
        int drewStatusBox = 0;
        if (!useV2PartyHud) {
            (void)M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &slotW, &slotH);
        }

        if (slot < state->world.party.championCount && state->world.party.champions[slot].present) {
            char name[16];
            const struct ChampionState_Compat* champ = &state->world.party.champions[slot];
            int isDead = (champ->hp.current == 0);
            m11_format_champion_name(champ->name, name, sizeof(name));

            if (useV2PartyHud) {
                int cellBaseX = x + 47;
                int cellY = y + 6;
                int cell;
                if (slot == activeIndex) {
                    m11_blit_v2_slice_asset(&m11_v2_party_hud_four_slot_active_overlay,
                                            framebuffer, framebufferWidth, framebufferHeight,
                                            x, y, 1);
                }
                for (cell = 0; cell < 3; ++cell) {
                    m11_blit_v2_slice_asset(&m11_v2_party_hud_cell_base,
                                            framebuffer, framebufferWidth, framebufferHeight,
                                            cellBaseX + cell * 6,
                                            cellY + (cell == 1 ? 1 : 0), 1);
                }
                if (slot == activeIndex) {
                    m11_blit_v2_slice_asset(&m11_v2_party_hud_cell_highlight,
                                            framebuffer, framebufferWidth, framebufferHeight,
                                            cellBaseX,
                                            cellY, 1);
                }
                drewStatusBox = 1;
            }

            /* V1 source status-box background.  ReDMCSB marks
             * C007_GRAPHIC_STATUS_BOX as "never used"; alive champion
             * status boxes are cleared to C12 darkest-gray, then name,
             * bars, hands, and optional shield borders are drawn on top
             * (CHAMPION.C F0292).  Dead champions still use graphic 8. */
            if (!useV2PartyHud) {
                int baseGfx = M11_GameView_GetV1StatusBoxBaseGraphic(state, slot);
                if (baseGfx && state->assetsAvailable) {
                    const M11_AssetSlot* boxAsset = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader,
                        baseGfx);
                    if (boxAsset && boxAsset->width == slotW && boxAsset->height == slotH) {
                        M11_AssetLoader_BlitRegion(boxAsset,
                            0, 0, slotW, slotH,
                            framebuffer, framebufferWidth, framebufferHeight,
                            x, y, 0);
                        drewStatusBox = 1;
                    }
                }
                if (!isDead) {
                    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  x, y, slotW, slotH,
                                  (unsigned char)M11_GameView_GetV1StatusBoxFillColor());
                    drewStatusBox = 1;
                }
            }
            if (!drewStatusBox) {
                /* Procedural fallback.  V1 uses the same source status-box
                 * rectangle as C007/C008 (67x29); V2 keeps the legacy
                 * 71x28 shell baked into its vertical-slice HUD assets. */
                int fallbackH = useV2PartyHud ? M11_PARTY_SLOT_H : slotH;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x, y, slotW, fallbackH, M11_COLOR_BLACK);
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x, y, slotW, fallbackH, M11_COLOR_LIGHT_CYAN);
            }

            /* No invented active-champion frame in V1.  The source
             * indicates leader/active state through the champion-name
             * text color (yellow for leader, gold otherwise), not a
             * double rectangular border around the status box. */

            /* V1 champion name/title status text.  Source F0292 does
             * not draw the 19x14 champion icon inside the compact
             * status box.  It clears C159+n (43x7) to C01 dark gray
             * and prints the champion name centered in C163+n:
             * leader C11 yellow, other champions C09 gold. */
            if (!useV2PartyHud) {
                M11_TextStyle nameStyle = g_text_small;
                nameStyle.color = (unsigned char)M11_GameView_GetV1StatusNameColor(state, slot);
                {
                    int nameClearX, nameClearY, nameClearW, nameClearH;
                    (void)M11_GameView_GetV1StatusNameZone(
                        slot, &nameClearX, &nameClearY, &nameClearW, &nameClearH);
                    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  nameClearX, nameClearY, nameClearW, nameClearH,
                                  (unsigned char)M11_GameView_GetV1StatusNameClearColor());
                }
                {
                    int nameTextX, nameTextY, nameTextW;
                    (void)M11_GameView_GetV1StatusNameTextZone(
                        slot, &nameTextX, &nameTextY, &nameTextW, NULL);
                    m11_draw_text_centered_in_rect(
                        framebuffer, framebufferWidth, framebufferHeight,
                        nameTextX, nameTextY, nameTextW,
                        name, &nameStyle);
                }
            } else {
                int nameOffX = 4;
                if (slot == activeIndex) {
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  x + nameOffX, y + 3, name, &g_text_shadow);
                } else {
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  x + nameOffX, y + 3, name, &g_text_small);
                }
            }
            if (isDead && useV2PartyHud) {
                M11_TextStyle ds = g_text_small;
                ds.color = M11_COLOR_RED;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 12, "DEAD", &ds);
            }
            /* Pass 43: champion HP/stamina/mana bar graphs.
             *
             * V1 mode (default): source-faithful vertical bars per
             * CHAMDRAW.C F0287_CHAMPION_DrawBarGraphs.  Each of the
             * three bars is a 4x25 rectangle bottom-anchored inside
             * the 24x29 bar-graph region at the right side of the
             * 67x29 status-box frame.  Blank portion is darkest-gray
             * (C12), filled portion is the per-champion color.  Bar
             * drains from the top: when current < maximum, the
             * (max-current)/max fraction of the bar at the top is
             * filled with darkest-gray first, and only the remaining
             * bottom portion shows the champion color.  Dead
             * champion (current==0) gets all darkest-gray and skips
             * the colored fill entirely (matches F0287's
             * L2254_i_Bars[...][0] != 0 guard).
             *
             * V2 vertical-slice / opt-out: legacy horizontal 59x{2,1,1}
             * strip preserved so the pre-baked HUD sprite stays
             * pixel-aligned.  Opt-out via FIRESTAFF_V1_BAR_GRAPHS=0.
             *
             * Ref: firestaff_pc34_core_amalgam.c l.11229..11260 for
             * F0287; zones_h_reconstruction.json records 187..206;
             * parity-evidence/pass43_bar_graphs.md. */
            if (m11_v1_bar_graphs_enabled()) {
                int statIdx;
                unsigned char champColor =
                    isDead
                        ? (unsigned char)M11_GameView_GetV1StatusBarBlankColor()
                        : (unsigned char)M11_GameView_GetV1ChampionBarColor(slot);
                long curs[3];
                long maxs[3];
                curs[0] = (long)champ->hp.current;
                maxs[0] = (long)champ->hp.maximum;
                curs[1] = (long)champ->stamina.current;
                maxs[1] = (long)champ->stamina.maximum;
                curs[2] = (long)champ->mana.current;
                maxs[2] = (long)champ->mana.maximum;
                for (statIdx = 0; statIdx < 3; ++statIdx) {
                    int barX, barTopY, barW, barFullHeight;
                    int blankHeight;
                    int fillHeight;
                    (void)M11_GameView_GetV1StatusBarZone(
                        slot, statIdx, &barX, &barTopY, &barW, &barFullHeight);
                    /* Blank the whole container to darkest-gray
                     * first (matches F0287's F0732_FillScreenArea
                     * (L2004_ai_XYZBlankBar, C12_COLOR_DARKEST_GRAY)
                     * combined with the colored overdraw). */
                    m11_fill_rect(framebuffer, framebufferWidth,
                                  framebufferHeight,
                                  barX, barTopY, barW, barFullHeight,
                                  (unsigned char)M11_GameView_GetV1StatusBarBlankColor());
                    if (maxs[statIdx] > 0 && curs[statIdx] > 0) {
                        /* F0287 scales fill height by cur/max,
                         * min 1 px if any cur > 0.  Blank height is
                         * reduced by this amount; the colored bar's
                         * top moves down by blank-height. */
                        long scaled = (long)barFullHeight *
                                       curs[statIdx] / maxs[statIdx];
                        if (scaled < 1) scaled = 1;
                        if (scaled > barFullHeight) scaled = barFullHeight;
                        fillHeight = (int)scaled;
                        blankHeight = barFullHeight - fillHeight;
                        if (fillHeight > 0) {
                            m11_fill_rect(framebuffer, framebufferWidth,
                                          framebufferHeight,
                                          barX, barTopY + blankHeight,
                                          barW, fillHeight, champColor);
                        }
                    }
                    /* current == 0 leaves the container all-blank,
                     * matching F0287's L2254_i_Bars[...][0] != 0
                     * final-color guard. */
                }
            } else {
                int hpWidth = champ->hp.maximum > 0 ? (int)(champ->hp.current * 59) / (int)champ->hp.maximum : 0;
                int staminaWidth = champ->stamina.maximum > 0 ? (int)(champ->stamina.current * 59) / (int)champ->stamina.maximum : 0;
                int manaWidth = champ->mana.maximum > 0 ? (int)(champ->mana.current * 59) / (int)champ->mana.maximum : 0;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 20, 59, 2, M11_COLOR_DARK_GRAY);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 20, hpWidth, 2, isDead ? M11_COLOR_DARK_GRAY : M11_COLOR_LIGHT_RED);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 23, 59, 1, M11_COLOR_DARK_GRAY);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 23, staminaWidth, 1, M11_COLOR_LIGHT_GREEN);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 25, 59, 1, M11_COLOR_DARK_GRAY);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 25, manaWidth, 1, M11_COLOR_LIGHT_BLUE);
            }

            /* V1 status-box hand slots.  Source F0292/F0291 draws
             * ready/action hand slot boxes at zones C211..C218 for
             * present living champions; these are separate from the
             * right-column action icon cells. */
            if (!useV2PartyHud && !isDead) {
                int readyX, readyY, actionX, actionY;
                (void)M11_GameView_GetV1StatusHandSlotBoxZone(
                    slot, 0, &readyX, &readyY, NULL, NULL);
                (void)M11_GameView_GetV1StatusHandSlotBoxZone(
                    slot, 1, &actionX, &actionY, NULL, NULL);
                (void)m11_draw_v1_status_hand_slot(
                    state, champ, framebuffer, framebufferWidth, framebufferHeight,
                    slot, 0, readyX, readyY);
                (void)m11_draw_v1_status_hand_slot(
                    state, champ, framebuffer, framebufferWidth, framebufferHeight,
                    slot, 1, actionX, actionY);
            }

            /* GRAPHICS.DAT-backed shield border overlays (67×29).
             * Drawn with transparency on top of the status box when the
             * party has an active shield spell.
             * Ref: ReDMCSB INVNTORY.C — G0310_i_ShieldDefenseType selects
             *   C037 (party shield), C038 (fire shield), or C039 (spell shield).
             * Priority: spell > fire > party (highest active wins). */
            if (state->assetsAvailable && !isDead) {
                unsigned int borderGfx =
                    (unsigned int)M11_GameView_GetV1StatusShieldBorderGraphic(state);
                if (borderGfx) {
                    const M11_AssetSlot* borderAsset = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader, borderGfx);
                    if (borderAsset && borderAsset->width == slotW &&
                        borderAsset->height == slotH) {
                        int borderX = x;
                        int borderY = y;
                        int borderW = slotW;
                        int borderH = slotH;
                        if (!useV2PartyHud) {
                            (void)M11_GameView_GetV1StatusShieldBorderZone(
                                slot, &borderX, &borderY, &borderW, &borderH);
                        }
                        M11_AssetLoader_BlitRegion(borderAsset,
                            0, 0, borderW, borderH,
                            framebuffer, framebufferWidth, framebufferHeight,
                            borderX, borderY, 0); /* transparentColor=0 (black) */
                    }
                }
            }

            /* GRAPHICS.DAT-backed POISONED label (96×15, graphic 32).
             * Drawn below the status box when champion is poisoned.
             * Ref: ReDMCSB INVNTORY.C — drawn when poisonDose > 0. */
            if (state->assetsAvailable && champ->poisonDose > 0) {
                const M11_AssetSlot* poisonLbl = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader,
                    (unsigned int)M11_GameView_GetV1PoisonLabelGraphicId());
                if (poisonLbl && poisonLbl->width > 0 &&
                    poisonLbl->height > 0) {
                    /* Center the 96-wide label within the source status-box
                     * zone; in DM1 this spills across adjacent boxes, which
                     * is the correct original behaviour.  Preserve the old
                     * V2 positioning while V1 routes through C007 geometry. */
                    int lblX, lblY, lblW, lblH;
                    if (!useV2PartyHud) {
                        (void)M11_GameView_GetV1PoisonLabelZone(
                            slot, (int)poisonLbl->width, (int)poisonLbl->height,
                            &lblX, &lblY, &lblW, &lblH);
                    } else {
                        int poisonBaseW = 67;
                        int poisonBaseH = 29;
                        lblX = x + (poisonBaseW - (int)poisonLbl->width) / 2;
                        lblY = y + poisonBaseH;
                        lblW = (int)poisonLbl->width;
                        lblH = (int)poisonLbl->height;
                    }
                    M11_AssetLoader_BlitRegion(poisonLbl,
                        0, 0, lblW, lblH,
                        framebuffer, framebufferWidth, framebufferHeight,
                        lblX, lblY, 0);
                }
            }

            /* GRAPHICS.DAT-backed damage indicator (45×7, graphic 15).
             * Overlaid on the status box when the champion just took
             * damage (timer > 0).  The damage number is drawn on top.
             * Ref: ReDMCSB CHAMPION.C F0291. */
            if (state->championDamageTimer[slot] > 0) {
                if (state->assetsAvailable) {
                    const M11_AssetSlot* dmgAsset = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader,
                        (unsigned int)M11_GameView_GetV1ChampionSmallDamageGraphicId());
                    if (dmgAsset && dmgAsset->width == 45 &&
                        dmgAsset->height == 7) {
                        int dmgX, dmgY, dmgW, dmgH;
                        if (!useV2PartyHud) {
                            (void)M11_GameView_GetV1DamageIndicatorZone(
                                slot, (int)dmgAsset->width, (int)dmgAsset->height,
                                &dmgX, &dmgY, &dmgW, &dmgH);
                        } else {
                            int dmgBaseW = 67;
                            int dmgBaseH = 29;
                            dmgX = x + (dmgBaseW - (int)dmgAsset->width) / 2;
                            dmgY = y + (dmgBaseH - (int)dmgAsset->height) / 2;
                            dmgW = (int)dmgAsset->width;
                            dmgH = (int)dmgAsset->height;
                        }
                        M11_AssetLoader_BlitRegion(dmgAsset,
                            0, 0, dmgW, dmgH,
                            framebuffer, framebufferWidth, framebufferHeight,
                            dmgX, dmgY, 0);
                    }
                }
                /* Always draw the damage number (even without assets) */
                {
                    char dmgNum[8];
                    M11_TextStyle dmgStyle = g_text_small;
                    dmgStyle.color = M11_COLOR_WHITE;
                    snprintf(dmgNum, sizeof(dmgNum), "%d",
                             state->championDamageAmount[slot]);
                    int dmgNumX;
                    int dmgNumY;
                    if (!useV2PartyHud) {
                        (void)M11_GameView_GetV1DamageNumberOrigin(
                            slot, &dmgNumX, &dmgNumY);
                    } else {
                        int dmgBaseW = 67;
                        int dmgBaseH = 29;
                        dmgNumX = x + dmgBaseW / 2 - 4;
                        dmgNumY = y + dmgBaseH / 2 - 3;
                    }
                    m11_draw_text(framebuffer, framebufferWidth,
                                  framebufferHeight,
                                  dmgNumX,
                                  dmgNumY,
                                  dmgNum, &dmgStyle);
                }
            }
        } else {
            /* Empty V1 party slots are not drawn by CHAMPION.C; only
             * present champions have status boxes.  Keep the old
             * structural empty cells for debug/V2 surfaces only. */
            if (!useV2PartyHud && !state->showDebugHUD) {
                continue;
            }
            m11_fill_rect(framebuffer, framebufferWidth,
                          framebufferHeight, x, y, 71, 28,
                          M11_COLOR_BLACK);
            m11_draw_rect(framebuffer, framebufferWidth,
                          framebufferHeight, x, y, 71, 28,
                          M11_COLOR_DARK_GRAY);
            m11_draw_rect(framebuffer, framebufferWidth,
                          framebufferHeight, x + 1, y + 1, 69, 26,
                          M11_COLOR_DARK_GRAY);
            if (state->assetsAvailable) {
                const M11_AssetSlot* boxAsset = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader,
                    (unsigned int)M11_GameView_GetV1StatusBoxGraphicId());
                if (boxAsset && boxAsset->width == 67 && boxAsset->height == 29) {
                    M11_AssetLoader_BlitRegion(boxAsset,
                        0, 0, 67, 29,
                        framebuffer, framebufferWidth, framebufferHeight,
                        x, y, 0);
                }
            }
            if (state->showDebugHUD) {
                snprintf(line, sizeof(line), "SLOT %d", slot + 1);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 6, line, &g_text_small);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 16, "EMPTY", &g_text_small);
            }
        }
    }
}

static void m11_format_front_cell_prompt(const M11_GameViewState* state,
                                         const M11_ViewportCell* cell,
                                         char* outAction,
                                         size_t outActionSize,
                                         char* outHint,
                                         size_t outHintSize) {
    char champion[16];
    if (outAction && outActionSize > 0U) {
        outAction[0] = '\0';
    }
    if (outHint && outHintSize > 0U) {
        outHint[0] = '\0';
    }
    if (!cell || !cell->valid) {
        snprintf(outAction, outActionSize, "FOCUS VOID");
        snprintf(outHint, outHintSize, "TURN TO RE-ENTER THE MAP");
        return;
    }
    if (cell->summary.groups > 0) {
        m11_get_active_champion_label(state, champion, sizeof(champion));
        snprintf(outAction, outActionSize, "ATTACK WITH %s", champion);
        snprintf(outHint, outHintSize, "SPACE OR CLICK CENTER STRIKES, A/D OR LOWER CORNERS STRAFE, TAB SWAPS CHAMPION");
        return;
    }
    if (cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        snprintf(outAction, outActionSize, "FOCUS ACTIVE EFFECT");
        snprintf(outHint, outHintSize, "SPACE WAITS, ENTER INSPECTS, TURN IF THE CELL STAYS HOT");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        if (m11_viewport_cell_is_open(cell)) {
            snprintf(outAction, outActionSize, "FOCUS OPEN DOOR");
            snprintf(outHint, outHintSize, "UP OR CLICK CENTER CROSSES, A/D STRAFES, ENTER INSPECTS, SPACE CLOSES THE DOOR");
        } else {
            snprintf(outAction, outActionSize, "FOCUS CLOSED DOOR");
            snprintf(outHint, outHintSize, "SPACE OPENS THE DOOR, ENTER INSPECTS, A/D STRAFES, TURN TO SEARCH");
        }
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT) {
        snprintf(outAction, outActionSize, "FOCUS PIT");
        snprintf(outHint, outHintSize, "UP RISKS A DROP, ENTER INSPECTS BEFORE COMMITTING");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        snprintf(outAction, outActionSize, "FOCUS STAIRS");
        snprintf(outHint, outHintSize, "UP CLIMBS, ENTER INSPECTS, SPACE WAITS");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_TELEPORTER) {
        snprintf(outAction, outActionSize, "FOCUS TELEPORTER");
        snprintf(outHint, outHintSize, "UP TESTS THE RIFT, ENTER INSPECTS FIRST");
        return;
    }
    if (cell->summary.items > 0 || cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        snprintf(outAction, outActionSize, "FOCUS INTERACTABLE");
        snprintf(outHint, outHintSize, "ENTER INSPECTS, G GRABS FLOOR ITEMS, A/D STRAFES, SPACE WAITS");
        return;
    }
    if (m11_viewport_cell_is_open(cell)) {
        snprintf(outAction, outActionSize, "FOCUS CLEAR PASSAGE");
        snprintf(outHint, outHintSize, "UP ADVANCES, A/D OR LOWER CORNERS STRAFE, ENTER INSPECTS, TAB SWAPS CHAMPION");
        return;
    }
    snprintf(outAction, outActionSize, "FOCUS BLOCKED");
    snprintf(outHint, outHintSize, "ENTER INSPECTS, SPACE WAITS, TURN TO SEARCH");
}

/* ── Full-screen map overlay (M key) ──
 *
 * Period-faithful presentation modeled after DM1 / ReDMCSB automap style:
 * - Full-screen black fill to guarantee no HUD bleed-through
 * - Stone-gray single border (not bright yellow/brown double frame)
 * - Compact title without debug dimensions
 * - Muted tile palette: walls dark, corridors mid-gray, specials subdued
 * - Minimal footer — just the dismiss key, not a keybinding reference card
 */

/* Map-specific muted tile colors — period-faithful restraint.
 * Walls blend into the background; corridors are the primary readable
 * feature.  Special tiles (doors, stairs, pits) use subdued accents
 * rather than the saturated utility-palette colors. */
static unsigned char m11_map_tile_color(int elementType) {
    switch (elementType) {
        case DUNGEON_ELEMENT_WALL:       return M11_COLOR_BLACK;      /* walls vanish into bg */
        case DUNGEON_ELEMENT_CORRIDOR:   return M11_COLOR_DARK_GRAY;  /* explored floor */
        case DUNGEON_ELEMENT_PIT:        return M11_COLOR_BROWN;      /* subtle hazard */
        case DUNGEON_ELEMENT_STAIRS:     return M11_COLOR_LIGHT_GRAY; /* notable but muted */
        case DUNGEON_ELEMENT_DOOR:       return M11_COLOR_BROWN;      /* structural feature */
        case DUNGEON_ELEMENT_TELEPORTER: return M11_COLOR_NAVY;       /* arcane, not neon */
        case DUNGEON_ELEMENT_FAKEWALL:   return M11_COLOR_DARK_GRAY;  /* blends with corridor */
        default: return M11_COLOR_BLACK;
    }
}

static void m11_draw_fullscreen_map(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight) {
    int panelX, panelY, panelW, panelH;
    const struct DungeonMapDesc_Compat* mapDesc = NULL;
    int mapW = 0, mapH = 0;
    int cellSize;
    int offsetX, offsetY;
    int gx, gy;

    if (!state || !state->active || !state->world.dungeon) return;
    if (state->world.party.mapIndex < 0 ||
        state->world.party.mapIndex >= (int)state->world.dungeon->header.mapCount) return;
    mapDesc = &state->world.dungeon->maps[state->world.party.mapIndex];
    mapW = (int)mapDesc->width;
    mapH = (int)mapDesc->height;
    if (mapW <= 0 || mapH <= 0) return;

    /* Full-screen black fill — guarantees no HUD content leaks through
     * at any edge, regardless of panel inset.  This is the primary
     * clipping-cleanliness measure (S2.3). */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  0, 0, framebufferWidth, framebufferHeight, M11_COLOR_BLACK);

    /* Panel inset — generous margins for a deliberate, composed feel
     * rather than an edge-to-edge utility window. */
    panelX = 8;
    panelY = 6;
    panelW = framebufferWidth - 16;
    panelH = framebufferHeight - 12;

    /* Single stone-gray border — period-appropriate, not flashy */
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX, panelY, panelW, panelH, M11_COLOR_DARK_GRAY);

    /* Title — compact, no debug dimensions */
    {
        char mapTitle[32];
        M11_TextStyle titleStyle = g_text_shadow;
        titleStyle.color = M11_COLOR_LIGHT_GRAY;
        snprintf(mapTitle, sizeof(mapTitle), "LEVEL %d",
                 state->world.party.mapIndex + 1);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      panelX + 6, panelY + 4, mapTitle, &titleStyle);
    }

    /* Compute cell size to fit map in available area */
    {
        int availW = panelW - 12;
        int availH = panelH - 28; /* title + footer headroom */
        int cw = availW / mapW;
        int ch = availH / mapH;
        cellSize = cw < ch ? cw : ch;
        if (cellSize < 2) cellSize = 2;
        if (cellSize > 12) cellSize = 12;
        offsetX = panelX + 6 + (availW - mapW * cellSize) / 2;
        offsetY = panelY + 18 + (availH - mapH * cellSize) / 2;
    }

    /* Draw each tile */
    for (gy = 0; gy < mapH; ++gy) {
        for (gx = 0; gx < mapW; ++gx) {
            unsigned char square = 0;
            unsigned char fill = M11_COLOR_BLACK;
            int drawX = offsetX + gx * cellSize;
            int drawY = offsetY + gy * cellSize;
            int ok = m11_get_square_byte(&state->world,
                                         state->world.party.mapIndex,
                                         gx, gy, &square);
            if (ok) {
                if (m11_is_explored(state, gx, gy)) {
                    fill = m11_map_tile_color(square >> 5);
                } else {
                    /* Unexplored: pure black — indistinguishable from void */
                    fill = M11_COLOR_BLACK;
                }
            }
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          drawX, drawY, cellSize - 1, cellSize - 1, fill);

            /* Party position marker — white dot, restrained */
            if (gx == state->world.party.mapX && gy == state->world.party.mapY) {
                int cx = drawX + cellSize / 2 - 1;
                int cy = drawY + cellSize / 2 - 1;
                int ms = cellSize > 4 ? cellSize - 3 : 2;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx, cy, ms, ms, M11_COLOR_WHITE);
                if (cellSize >= 6) {
                    m11_draw_party_arrow(framebuffer, framebufferWidth, framebufferHeight,
                                         cx - 1, cy - 1, ms + 2,
                                         state->world.party.direction, M11_COLOR_LIGHT_GRAY);
                }
            }
        }
    }

    /* Footer — minimal dismiss hint, not a keybinding cheat sheet */
    {
        M11_TextStyle footerStyle = g_text_small;
        footerStyle.color = M11_COLOR_DARK_GRAY;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      panelX + 6, panelY + panelH - 12, "PRESS M", &footerStyle);
    }
}

/* ── Full inventory panel (I key) ──
 * ReDMCSB-faithful composition: portrait+name at top, hands row,
 * body column, containers, backpack grid.  All equipment slots are
 * uniform 16×16 icon boxes matching the original SLOT_BOX structure
 * from Fontanel’s INVNTORY.C / OBJECT.C DrawIconInSlotBox.
 *
 * Original DM1 layout reference (viewport-relative):
 *   [Portrait 32×29]  [Name]       Hands: [Ready][Action]
 *   [Stats bars]       Body:  HEAD  NECK  TORSO  LEGS  FEET
 *                      Containers: POUCH×2  QUIVER×4
 *                      Backpack: 4×2 grid
 *                      Panel: Food/Water or object details
 */

/* Helper: draw one equipment slot box using original GRAPHICS.DAT
 * slot box bitmaps (18×18).  Falls back to procedural 16×16 box
 * when the asset is unavailable.
 *
 * Original DM1 uses three slot box variants:
 *   graphic 33 = normal slot (C033_GRAPHIC_SLOT_BOX_NORMAL)
 *   graphic 34 = wounded champion slot (C034_GRAPHIC_SLOT_BOX_WOUNDED)
 *   graphic 35 = acting-hand slot (C035_GRAPHIC_SLOT_BOX_ACTING_HAND)
 *
 * Ref: DEFS.H lines 2193-2195, ReDMCSB INVNTORY.C DrawIconInSlotBox. */
static void m11_draw_inv_slot(const M11_GameViewState* state,
                              const struct ChampionState_Compat* champ,
                              unsigned char* fb, int fbW, int fbH,
                              int sx, int sy,
                              int slotIdx, const char* shortLabel) {
    /* Original slot boxes are 18×18 in GRAPHICS.DAT */
    static const int SZ_ORIG = 18;
    static const int SZ = 16; /* fallback procedural size */
    unsigned short thingId = champ->inventory[slotIdx];
    int selected = (slotIdx == state->inventorySelectedSlot);
    int isDead = (champ->hp.current == 0);
    int isActingHand = (slotIdx == CHAMPION_SLOT_HAND_LEFT ||
                        slotIdx == CHAMPION_SLOT_HAND_RIGHT);
    int drewSlotBox = 0;
    int slotBoxW = SZ, slotBoxH = SZ; /* effective size for item overlay */

    /* Try original GRAPHICS.DAT slot box graphic */
    if (state->assetsAvailable) {
        unsigned int gfxIdx;
        if (isActingHand)
            gfxIdx = (unsigned int)M11_GameView_GetV1SlotBoxActingHandGraphicId();
        else if (isDead)
            gfxIdx = (unsigned int)M11_GameView_GetV1SlotBoxWoundedGraphicId();
        else
            gfxIdx = (unsigned int)M11_GameView_GetV1SlotBoxNormalGraphicId();

        const M11_AssetSlot* boxSlot = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, gfxIdx);
        if (boxSlot && boxSlot->width == SZ_ORIG && boxSlot->height == SZ_ORIG) {
            M11_AssetLoader_Blit(boxSlot, fb, fbW, fbH, sx, sy, 0);
            drewSlotBox = 1;
            slotBoxW = SZ_ORIG;
            slotBoxH = SZ_ORIG;
        }
    }

    if (!drewSlotBox) {
        /* Procedural fallback */
        unsigned char borderColor = selected ? M11_COLOR_LIGHT_GREEN : M11_COLOR_DARK_GRAY;
        m11_fill_rect(fb, fbW, fbH, sx, sy, SZ, SZ, M11_COLOR_BLACK);
        m11_draw_rect(fb, fbW, fbH, sx, sy, SZ, SZ, borderColor);
    }

    /* Selection highlight: bright border around the slot box */
    if (selected) {
        m11_draw_rect(fb, fbW, fbH, sx, sy, slotBoxW, slotBoxH,
                      M11_COLOR_LIGHT_GREEN);
    }

    if (thingId != THING_NONE && thingId != THING_ENDOFLIST) {
        /* Occupied — DM1 draws the source 16×16 object icon in the
         * slot box (F0038_OBJECT_DrawIconInSlotBox), not the larger
         * viewport object sprite.  Fall back to the legacy sprite/tag
         * path only when the icon atlas is unavailable. */
        int drewSprite = 0;
        if (state->assetsAvailable && state->world.things) {
            int iconIndex = m11_object_icon_index_for_thing(
                state, state->world.things, thingId);
            drewSprite = m11_draw_dm_object_icon_index(
                state, fb, fbW, fbH, iconIndex, sx + 1, sy + 1, 0);
        }
        if (!drewSprite && state->assetsAvailable && state->world.things) {
            unsigned int gfxIdx = m11_inventory_thing_sprite_index(
                state->world.things, thingId);
            if (gfxIdx > 0 && gfxIdx < M11_GFX_ITEM_SPRITE_END) {
                const M11_AssetSlot* slot = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader, gfxIdx);
                if (slot && slot->width > 0 && slot->height > 0) {
                    /* Compatibility fallback: scale item sprite into the slot box. */
                    M11_AssetLoader_BlitScaled(slot, fb, fbW, fbH,
                                              sx + 1, sy + 1,
                                              slotBoxW - 2, slotBoxH - 2, 0);
                    drewSprite = 1;
                }
            }
        }
        if (!drewSprite) {
            /* Fallback: 2-char type tag */
            int thingType = THING_GET_TYPE(thingId);
            const char* tag = "?";
            M11_TextStyle s = g_text_small;
            s.color = M11_COLOR_WHITE;
            switch (thingType) {
                case THING_TYPE_WEAPON:    tag = "Wp"; break;
                case THING_TYPE_ARMOUR:    tag = "Ar"; break;
                case THING_TYPE_SCROLL:    tag = "Sc"; break;
                case THING_TYPE_POTION:    tag = "Po"; break;
                case THING_TYPE_CONTAINER: tag = "Ch"; break;
                case THING_TYPE_JUNK:      tag = "Jk"; break;
                default: break;
            }
            m11_draw_text(fb, fbW, fbH, sx + 3, sy + 4, tag, &s);
        }
    } else if (!drewSlotBox) {
        /* Empty + no original graphic — faint position hint */
        M11_TextStyle dim = g_text_small;
        dim.color = M11_COLOR_DARK_GRAY;
        m11_draw_text(fb, fbW, fbH, sx + 2, sy + 4, shortLabel, &dim);
    }
}

static void m11_draw_inventory_panel(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight) {
    /* Overlay positioned like the DM1 viewport-replacement inventory,
     * centered with proportional margins. */
    int panelX = 8, panelY = 8;
    int panelW = framebufferWidth - 16;
    int panelH = framebufferHeight - 16;
    const struct ChampionState_Compat* champ;
    char champion[32];
    int isDead;
    int bpI;

    /* Portrait proportions from ReDMCSB: 32×29 pixels */
    static const int PORT_W = 32;
    static const int PORT_H = 29;
    /* Slot box size: 18×18 when using original GRAPHICS.DAT assets,
     * 16×16 as procedural fallback. */
    int SZ = 16;
    static const int GAP = 2; /* spacing between adjacent slot boxes */

    /* Layout anchors */
    int portX, portY;   /* portrait position */
    int nameX, nameY;   /* champion name */
    int handY;          /* hands row Y */
    int handLX, handRX; /* hand slot X positions */
    int bodyX, bodyY;   /* body column anchor (Head at top) */
    int contX, contY;   /* container section anchor */
    int bpX, bpY;       /* backpack grid anchor */
    int panelBottom;    /* bottom info panel Y */

    if (!state || !state->active) return;
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) return;
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    isDead = (champ->hp.current == 0);

    /* Use 18×18 slot boxes when GRAPHICS.DAT assets are available */
    if (state->assetsAvailable) {
        const M11_AssetSlot* testBox = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader,
            (unsigned int)M11_GameView_GetV1SlotBoxNormalGraphicId());
        if (testBox && testBox->width == 18 && testBox->height == 18)
            SZ = 18;
    }

    /* ── Panel background — original graphic 20 (144×73), or
     * procedural double-border as fallback ── */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX, panelY, panelW, panelH, M11_COLOR_BLACK);
    if (state->assetsAvailable) {
        const M11_AssetSlot* panelBg = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader,
            (unsigned int)M11_GameView_GetV1InventoryPanelGraphicId());
        if (panelBg && panelBg->width > 0 && panelBg->height > 0) {
            /* Blit the original panel background scaled to fit */
            M11_AssetLoader_BlitScaled(panelBg,
                framebuffer, framebufferWidth, framebufferHeight,
                panelX, panelY, panelW, panelH, 0);
        }
    }
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX, panelY, panelW, panelH, M11_COLOR_BROWN);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX + 1, panelY + 1, panelW - 2, panelH - 2, M11_COLOR_DARK_GRAY);

    /* ── Champion identity ── portrait + name + stat bars */
    portX = panelX + 5;
    portY = panelY + 4;
    m11_get_active_champion_label(state, champion, sizeof(champion));

    /* Portrait box — GRAPHICS.DAT champion portrait (graphic 26,
     * C026_GRAPHIC_CHAMPION_PORTRAITS, 256×87 strip of 32×29 portraits).
     * M027_PORTRAIT_X(index) = (index & 7) * 32
     * M028_PORTRAIT_Y(index) = (index >> 3) * 29
     * Falls back to small icon (graphic 28) or silhouette. */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  portX, portY, PORT_W, PORT_H, M11_COLOR_DARK_GRAY);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  portX, portY, PORT_W, PORT_H,
                  isDead ? M11_COLOR_RED : M11_COLOR_LIGHT_CYAN);
    {
        int drewPortrait = 0;
        if (state->assetsAvailable) {
            /* Try full 32×29 portrait from graphic 26 first */
            const M11_AssetSlot* portraits = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader,
                (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
            if (portraits && portraits->width >= 256 && portraits->height >= 29) {
                int pIdx = champ->portraitIndex & 0x1F; /* 0–23 range */
                int srcPX = (pIdx & 7) * M11_PORTRAIT_W;
                int srcPY = (pIdx >> 3) * M11_PORTRAIT_H;
                if (srcPX + M11_PORTRAIT_W <= (int)portraits->width &&
                    srcPY + M11_PORTRAIT_H <= (int)portraits->height) {
                    M11_AssetLoader_BlitRegion(portraits,
                        srcPX, srcPY, M11_PORTRAIT_W, M11_PORTRAIT_H,
                        framebuffer, framebufferWidth, framebufferHeight,
                        portX, portY, 0);
                    drewPortrait = 1;
                }
            }
            /* Fallback: small icon from graphic 28 */
            if (!drewPortrait) {
                const M11_AssetSlot* iconStrip = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader,
                    (unsigned int)M11_GameView_GetV1ChampionIconGraphicId());
                if (iconStrip && iconStrip->width > 0 && iconStrip->height > 0) {
                    int iconCol = champ->portraitIndex & 3;
                    int srcX = iconCol * M11_CHAMPION_ICON_W;
                    if (srcX + M11_CHAMPION_ICON_W <= (int)iconStrip->width) {
                        M11_AssetLoader_BlitRegion(iconStrip,
                            srcX, 0, M11_CHAMPION_ICON_W, M11_CHAMPION_ICON_H,
                            framebuffer, framebufferWidth, framebufferHeight,
                            portX + 1, portY + 1, 0);
                        drewPortrait = 1;
                    }
                }
            }
        }
        if (!drewPortrait) {
            /* Fallback: minimal face silhouette */
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          portX + 9,  portY + 8, 3, 3, M11_COLOR_WHITE);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          portX + 20, portY + 8, 3, 3, M11_COLOR_WHITE);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          portX + 10, portY + 19, 12, 2,
                          isDead ? M11_COLOR_RED : M11_COLOR_LIGHT_GRAY);
        }
    }

    /* Champion name — prominent, right of portrait */
    nameX = portX + PORT_W + 6;
    nameY = portY + 2;
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  nameX, nameY, champion, &g_text_title);

    if (isDead) {
        M11_TextStyle deadStyle = g_text_small;
        deadStyle.color = M11_COLOR_RED;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      nameX, nameY + 14, "DEAD", &deadStyle);
    }

    /* Stat bars — compact DM-style HP/STA/MANA right of portrait */
    {
        int barX = nameX;
        int barY = nameY + (isDead ? 22 : 14);
        int barMaxW = 80;
        int barH = 3;
        int hpW = champ->hp.maximum > 0
            ? (int)(champ->hp.current * barMaxW) / (int)champ->hp.maximum : 0;
        int staW = champ->stamina.maximum > 0
            ? (int)(champ->stamina.current * barMaxW) / (int)champ->stamina.maximum : 0;
        int manaW = champ->mana.maximum > 0
            ? (int)(champ->mana.current * barMaxW) / (int)champ->mana.maximum : 0;

        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY, barMaxW, barH, M11_COLOR_DARK_GRAY);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY, hpW, barH,
                      isDead ? M11_COLOR_DARK_GRAY : M11_COLOR_LIGHT_RED);

        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + barH + 1, barMaxW, barH, M11_COLOR_DARK_GRAY);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + barH + 1, staW, barH, M11_COLOR_LIGHT_GREEN);

        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + 2 * (barH + 1), barMaxW, barH, M11_COLOR_DARK_GRAY);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + 2 * (barH + 1), manaW, barH, M11_COLOR_LIGHT_BLUE);
    }

    /* ── Hands row — ReDMCSB places Ready Hand + Action Hand as the
     * first two slots, prominently above the body equipment column.
     * In the original, these are the primary interaction slots. ── */
    handY = portY + PORT_H + 4;
    handLX = panelX + 5;
    handRX = handLX + SZ + GAP;

    /* Thin section label */
    {
        M11_TextStyle hlbl = g_text_small;
        hlbl.color = M11_COLOR_BROWN;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      handLX, handY - 1, "HANDS", &hlbl);
    }
    handY += 7;
    m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                      handLX, handY, CHAMPION_SLOT_HAND_LEFT, "L");
    m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                      handRX, handY, CHAMPION_SLOT_HAND_RIGHT, "R");

    /* ── Body column — Head → Neck → Torso → Legs → Feet ──
     * Vertical column to the right of hands, matching ReDMCSB’s
     * equipment slot arrangement.  Each slot is a 16×16 icon box. */
    bodyX = handRX + SZ + 10;
    bodyY = handY;
    {
        static const int bodySlots[5] = {
            CHAMPION_SLOT_HEAD, CHAMPION_SLOT_NECK, CHAMPION_SLOT_TORSO,
            CHAMPION_SLOT_LEGS, CHAMPION_SLOT_FEET
        };
        static const char* bodyLabels[5] = { "Hd", "Nk", "To", "Lg", "Ft" };
        int bi;
        M11_TextStyle blbl = g_text_small;
        blbl.color = M11_COLOR_BROWN;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      bodyX, handY - 8, "BODY", &blbl);
        for (bi = 0; bi < 5; ++bi) {
            m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                              bodyX, bodyY + bi * (SZ + GAP),
                              bodySlots[bi], bodyLabels[bi]);
        }
    }

    /* ── Container region: Pouches + Quiver ──
     * Positioned to the right of the body column, matching the
     * original’s right-side container layout. */
    contX = bodyX + SZ + 10;
    contY = handY;
    {
        M11_TextStyle clbl = g_text_small;
        clbl.color = M11_COLOR_BROWN;

        /* Pouches (2 slots side by side) */
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      contX, contY - 1, "POUCH", &clbl);
        contY += 7;
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX, contY, CHAMPION_SLOT_POUCH_1, "1");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + SZ + GAP, contY, CHAMPION_SLOT_POUCH_2, "2");

        /* Quiver (4 slots in a row) */
        contY += SZ + GAP + 4;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      contX, contY, "QUIVER", &clbl);
        contY += 8;
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX, contY, CHAMPION_SLOT_QUIVER_1, "1");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + (SZ + GAP), contY, CHAMPION_SLOT_QUIVER_2, "2");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + 2 * (SZ + GAP), contY, CHAMPION_SLOT_QUIVER_3, "3");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + 3 * (SZ + GAP), contY, CHAMPION_SLOT_QUIVER_4, "4");
    }

    /* ── Backpack grid — 4×2, the largest container section ──
     * ReDMCSB: BACKPACK slots 13–20, drawn as 4-wide × 2-tall grid
     * at the bottom of the equipment area. */
    bpX = panelX + 5;
    bpY = handY + 5 * (SZ + GAP) + 6;
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   panelX + 3, bpY - 3, panelW - 6, M11_COLOR_DARK_GRAY);
    {
        M11_TextStyle plbl = g_text_small;
        plbl.color = M11_COLOR_BROWN;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      bpX, bpY, "BACKPACK", &plbl);
    }
    bpY += 8;
    for (bpI = 0; bpI < 8; ++bpI) {
        int col = bpI % 4;
        int row = bpI / 4;
        char bpLabel[4];
        snprintf(bpLabel, sizeof(bpLabel), "%d", bpI + 1);
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          bpX + col * (SZ + GAP),
                          bpY + row * (SZ + GAP),
                          CHAMPION_SLOT_BACKPACK_1 + bpI, bpLabel);
    }

    /* ── Bottom panel area — food/water status (ReDMCSB: G424_i_PanelContent) ── */
    panelBottom = bpY + 2 * (SZ + GAP) + 4;
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   panelX + 3, panelBottom - 2, panelW - 6, M11_COLOR_DARK_GRAY);
    {
        int avgFood = (int)champ->food;
        int avgWater = (int)champ->water;
        int labelDrawn = 0;
        int fwX = panelX + 5;
        int fwY = panelBottom;

        /* GRAPHICS.DAT-backed food/water labels (graphics 30, 31).
         * Food label: 34×9, Water label: 46×9.
         * Falls back to text rendering when assets unavailable. */
        if (state->assetsAvailable) {
            const M11_AssetSlot* foodLbl = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader,
                (unsigned int)M11_GameView_GetV1FoodLabelGraphicId());
            const M11_AssetSlot* waterLbl = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader,
                (unsigned int)M11_GameView_GetV1WaterLabelGraphicId());
            if (foodLbl && foodLbl->width > 0 && foodLbl->height > 0 &&
                waterLbl && waterLbl->width > 0 && waterLbl->height > 0) {
                char numBuf[8];
                M11_TextStyle fwNumStyle = g_text_small;
                fwNumStyle.color = M11_COLOR_LIGHT_GRAY;
                M11_AssetLoader_BlitRegion(foodLbl,
                    0, 0, (int)foodLbl->width, (int)foodLbl->height,
                    framebuffer, framebufferWidth, framebufferHeight,
                    fwX, fwY, 0);
                snprintf(numBuf, sizeof(numBuf), "%d", avgFood > 999 ? 999 : avgFood);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              fwX + (int)foodLbl->width + 2, fwY, numBuf, &fwNumStyle);
                fwX += (int)foodLbl->width + 30;
                M11_AssetLoader_BlitRegion(waterLbl,
                    0, 0, (int)waterLbl->width, (int)waterLbl->height,
                    framebuffer, framebufferWidth, framebufferHeight,
                    fwX, fwY, 0);
                snprintf(numBuf, sizeof(numBuf), "%d", avgWater > 999 ? 999 : avgWater);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              fwX + (int)waterLbl->width + 2, fwY, numBuf, &fwNumStyle);
                labelDrawn = 1;
            }
        }
        if (!labelDrawn) {
            char foodWater[48];
            M11_TextStyle fwStyle = g_text_small;
            fwStyle.color = M11_COLOR_LIGHT_GRAY;
            snprintf(foodWater, sizeof(foodWater), "FOOD %d  WATER %d",
                     avgFood > 999 ? 999 : avgFood,
                     avgWater > 999 ? 999 : avgWater);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          fwX, fwY, foodWater, &fwStyle);
        }
    }

    /* ── Inventory damage overlay: GRAPHICS.DAT graphic 16 (32×29) ── */
    /* In DM1, when the inventory view is open and the active champion
     * takes damage, graphic 16 ("damage to champion big", 32×29) is
     * drawn on the portrait area instead of graphic 15 (small).
     * Ref: ReDMCSB CHAMPION.C F0291 — G0423_i_InventoryChampionOrdinal. */
    {
        int activeSlot = state->world.party.activeChampionIndex;
        if (activeSlot >= 0 && activeSlot < 4 &&
            state->championDamageTimer[activeSlot] > 0) {
            int dmgDrawn = 0;
            if (state->assetsAvailable) {
                const M11_AssetSlot* dmg16 = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader,
                    (unsigned int)M11_GameView_GetV1ChampionBigDamageGraphicId());
                if (dmg16 && dmg16->width == 32 && dmg16->height == 29) {
                    M11_AssetLoader_BlitRegion(dmg16,
                        0, 0, 32, 29,
                        framebuffer, framebufferWidth, framebufferHeight,
                        portX, portY, 0);
                    dmgDrawn = 1;
                }
            }
            if (!dmgDrawn) {
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              portX, portY, PORT_W, PORT_H, M11_COLOR_LIGHT_RED);
            }
            /* Damage number centered on portrait */
            {
                char dmgNum[8];
                M11_TextStyle dmgStyle = g_text_small;
                dmgStyle.color = M11_COLOR_WHITE;
                snprintf(dmgNum, sizeof(dmgNum), "%d",
                         state->championDamageAmount[activeSlot]);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              portX + PORT_W / 2 - 4,
                              portY + PORT_H / 2 - 3,
                              dmgNum, &dmgStyle);
            }
        }
    }

    /* Footer — key hints are debug-only. The original inventory panel
     * does not put helper text at the bottom edge; it relies on the
     * player knowing the controls. In V1 we honour that. */
    if (state->showDebugHUD) {
        M11_TextStyle footStyle = g_text_small;
        footStyle.color = M11_COLOR_DARK_GRAY;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      panelX + 5, panelY + panelH - 10,
                      "I CLOSE  TAB CHAMPION", &footStyle);
    }
}

static void m11_draw_map_panel(const M11_GameViewState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight) {
    static const int radius = 2;
    static const int cell = 11;
    int cx;
    int cy;
    int gx;
    int gy;
    int baseX = 228;
    int baseY = 78;
    if (!state || !state->active) {
        return;
    }
    for (gy = -radius; gy <= radius; ++gy) {
        for (gx = -radius; gx <= radius; ++gx) {
            unsigned char square = 0;
            int drawX = baseX + (gx + radius) * cell;
            int drawY = baseY + (gy + radius) * cell;
            unsigned char fill = M11_COLOR_BLACK;
            int ok = m11_get_square_byte(&state->world,
                                         state->world.party.mapIndex,
                                         state->world.party.mapX + gx,
                                         state->world.party.mapY + gy,
                                         &square);
            if (ok) {
                fill = m11_tile_color(square >> 5);
                /* Dim unexplored cells */
                if (!m11_is_explored(state,
                                     state->world.party.mapX + gx,
                                     state->world.party.mapY + gy)) {
                    fill = M11_COLOR_NAVY;
                }
            }
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          drawX, drawY, cell - 1, cell - 1, fill);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          drawX, drawY, cell - 1, cell - 1, M11_COLOR_BLACK);
            if (gx == 0 && gy == 0) {
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              drawX + 2, drawY + 2, cell - 5, cell - 5, M11_COLOR_LIGHT_GREEN);
                m11_draw_party_arrow(framebuffer, framebufferWidth, framebufferHeight,
                                     drawX + 2, drawY + 2, cell - 5,
                                     state->world.party.direction, M11_COLOR_BLACK);
            }
        }
    }
    cx = baseX + radius * cell;
    cy = baseY + radius * cell;
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  cx - cell, cy - cell, cell * 3 - 1, cell * 3 - 1, M11_COLOR_YELLOW);
}

void M11_GameView_Draw(const M11_GameViewState* state,
                       unsigned char* framebuffer,
                       int framebufferWidth,
                       int framebufferHeight) {
    char line[96];
    char line2[96];
    char line3[96];
    char focusAction[96];
    char focusHint[96];
    unsigned short firstThing = THING_ENDOFLIST;
    int squareThingCount = 0;
    M11_ViewportCell currentCell;
    M11_ViewportCell aheadCell;
    const struct DungeonMapDesc_Compat* mapDesc = NULL;
    int avgFood = 0;
    int avgWater = 0;
    char champion[24];
    if (!framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0) {
        return;
    }
    /* Set file-scope draw state for asset-backed rendering helpers */
    g_drawState = state;
    /* Activate original DM1 font for this render pass */
    g_activeOriginalFont = state->originalFontAvailable
        ? &state->originalFont : NULL;
    /* Base background is BLACK (DM PC VGA slot 0).  The pre-correction
     * base was NAVY (slot 14, pure blue), which produced the bright-blue
     * outermost frame strip the player saw around the whole game view.
     * Classic DM1 renders the area outside the HUD chrome as solid
     * black; only the HUD frame itself carries stone/bronze shading. */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  0, 0, framebufferWidth, framebufferHeight, M11_COLOR_BLACK);
    if (!state || !state->active) {
        g_drawState = NULL;
        g_activeOriginalFont = NULL;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      18, 18, "NO GAME VIEW", &g_text_title);
        return;
    }
    memset(&currentCell, 0, sizeof(currentCell));
    memset(&aheadCell, 0, sizeof(aheadCell));
    if (state->world.dungeon && state->world.party.mapIndex >= 0 &&
        state->world.party.mapIndex < (int)state->world.dungeon->header.mapCount) {
        mapDesc = &state->world.dungeon->maps[state->world.party.mapIndex];
    }
    if (state->showDebugHUD) {
        firstThing = m11_get_first_square_thing(&state->world,
                                                state->world.party.mapIndex,
                                                state->world.party.mapX,
                                                state->world.party.mapY);
        squareThingCount = m11_count_square_things(&state->world,
                                                   state->world.party.mapIndex,
                                                   state->world.party.mapX,
                                                   state->world.party.mapY);
        (void)m11_sample_viewport_cell(state, 0, 0, &currentCell);
        (void)m11_sample_viewport_cell(state, 1, 0, &aheadCell);
        m11_get_food_water_average(&state->world.party, &avgFood, &avgWater);
        m11_get_active_champion_label(state, champion, sizeof(champion));
        m11_format_front_cell_prompt(state,
                                     &aheadCell,
                                     focusAction,
                                     sizeof(focusAction),
                                     focusHint,
                                     sizeof(focusHint));
    }

    /* ── V1 screen layout ──
     * Three major zones (DM-like composition):
     *   1. Top bar:     thin title strip (dungeon name only)
     *   2. Middle:      viewport (left) + right status column
     *   3. Bottom:      party panel + single message line
     *
     * Debug/helper elements are only drawn when showDebugHUD is set. */

    /* HUD chrome background: DARK_GRAY (slot 12 = 73,73,73) is the
     * darkest stone tone in the DM PC VGA palette and matches the
     * muted slate shade classic DM1 uses for the outer HUD frame.
     * The perimeter outline is drawn in the same DARK_GRAY so it
     * blends cleanly with the fill — the pre-correction version drew
     * a harsh YELLOW (slot 11) outline around the entire screen,
     * which produced the bright-gold arcade-y rectangle seen in the
     * April 23 capture.  Classic DM1 has no yellow outer frame; only
     * the dungeon name title and occasional status text use yellow. */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_DARK_GRAY);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_DARK_GRAY);

    if (state->showDebugHUD) {
        /* Top title/debug bar is diagnostic-only.  Normal V1 should not
         * advertise itself with Firestaff text chrome above the viewport;
         * DM1's in-game screen is asset panels + viewport, not a captioned
         * debug screenshot. */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      12, 12, framebufferWidth - 24, 12, M11_COLOR_BLACK);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      18, 13, state->title[0] != '\0' ? state->title : "DUNGEON MASTER", &g_text_shadow);
        snprintf(line, sizeof(line), "L%d %s %s%s",
                 state->world.party.mapIndex + 1,
                 m11_direction_name(state->world.party.direction),
                 state->lastAction,
                 state->resting ? " ZZZ" : "");
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      140, 13, line, &g_text_small);
        snprintf(line, sizeof(line), "G GRAB P DROP R REST 1-6 RUNE C CAST");
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      240, 13, line, &g_text_small);
    }

    /* Viewport zone — source-faithful DM1 rectangle. */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_VIEWPORT_X, M11_VIEWPORT_Y,
                  M11_VIEWPORT_W, M11_VIEWPORT_H, M11_COLOR_BLACK);
    m11_draw_viewport(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Old Firestaff frame-strip assets are debug-only now; in normal
     * V1 they overdraw the source viewport/floor/ceiling composition. */
    if (state->showDebugHUD) {
        m11_draw_ui_frame_assets(state, framebuffer, framebufferWidth, framebufferHeight);
    }

    /* Right status column — align normal V1 to the original viewport
     * edge (x=224) so it does not erase the rightmost 10 px of the
     * 224-wide dungeon view. */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  state->showDebugHUD ? 214 : 224,
                  state->showDebugHUD ? 24 : 33,
                  state->showDebugHUD ? 94 : 87,
                  state->showDebugHUD ? 120 : 136,
                  M11_COLOR_BLACK);
    m11_draw_utility_panel(state, framebuffer, framebufferWidth, framebufferHeight, mapDesc);

    if (state->showDebugHUD) {
        /* Map mini-panel, focus card, message log — debug only */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      218, 74, 86, 68, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      218, 74, 86, 68, M11_COLOR_LIGHT_BLUE);
        m11_draw_map_panel(state, framebuffer, framebufferWidth, framebufferHeight);
        m11_draw_focus_card(framebuffer, framebufferWidth, framebufferHeight, state, &aheadCell);
        {
            int logI;
            int logY = 108;
            int logCount = state->messageLog.count;
            int logMax = 4;
            if (logCount > logMax) {
                logCount = logMax;
            }
            for (logI = 0; logI < logCount; ++logI) {
                const M11_LogEntry* entry = m11_log_entry_at(&state->messageLog, logI);
                if (entry && entry->text[0] != '\0') {
                    M11_TextStyle logStyle = g_text_small;
                    logStyle.color = entry->color;
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  222, logY + logI * 8, entry->text, &logStyle);
                }
            }
        }
    }

    /* Bottom zone — in normal V1 keep it below the source viewport
     * (viewport bottom is y=168). */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, state->showDebugHUD ? 146 : 169,
                  296, state->showDebugHUD ? 46 : 23,
                  M11_COLOR_BLACK);

    if (state->showDebugHUD) {
        /* Diagnostic square summary lines — debug only */
        m11_format_square_summary(&currentCell, line, sizeof(line));
        m11_format_square_things(firstThing, squareThingCount, line3, sizeof(line3));
        snprintf(line2, sizeof(line2), "HERE  %s", line);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 16, 149, line2, &g_text_small);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 122, 149, line3, &g_text_small);
        snprintf(line, sizeof(line), "TICK %u", (unsigned int)state->world.gameTick);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 240, 149, line, &g_text_small);

        m11_format_square_summary(&aheadCell, line, sizeof(line));
        snprintf(line2, sizeof(line2), "AHEAD %s", line);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 16, 157, line2, &g_text_small);
        snprintf(line, sizeof(line), "%s F%03d W%03d",
                 champion,
                 avgFood,
                 avgWater);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 154, 157, line, &g_text_small);

        /* Prompt strip and focus helpers — debug only */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      M11_PROMPT_STRIP_X, M11_PROMPT_STRIP_Y,
                      M11_PROMPT_STRIP_W, M11_PROMPT_STRIP_H, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      M11_PROMPT_STRIP_X, M11_PROMPT_STRIP_Y,
                      M11_PROMPT_STRIP_W, M11_PROMPT_STRIP_H, m11_focus_color(&aheadCell));
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      108, 168, focusAction, &g_text_small);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      197, 168, focusHint, &g_text_small);

        m11_draw_feedback_strip(framebuffer, framebufferWidth, framebufferHeight,
                                state, &aheadCell);
    }

    /* Normal V1 presentation intentionally renders no rolling event/debug
     * text in the party panel.  DM1 does show a few modal/story messages,
     * but Firestaff's messageLog is mostly synthetic telemetry (movement,
     * projectiles, tick status, etc.), so drawing it here makes the game
     * look like a debug build.  Keep all of that behind showDebugHUD until
     * each real DM1 message surface is source-bound. */
    if (state->showDebugHUD && !m11_v1_chrome_mode_enabled()) {
        m11_draw_control_strip(framebuffer, framebufferWidth, framebufferHeight, &aheadCell);
    }
    m11_draw_party_panel(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Spell panel overlay */
    if (state->spellPanelOpen) {
        /* ── P4+P6 V1 Presentation: DM1-style rune-dominant spell panel
         * with GRAPHICS.DAT-backed spell area grid ── */
        int spI;
        int pnlX = 24, pnlY = 36, pnlW = 180, pnlH = 110;
        int row = state->spellRuneRow < 4 ? state->spellRuneRow : 3;
        const char* rowNames[4] = { "POWER", "ELEMENT", "FORM", "CLASS" };

        /* Panel background — DM1-style double-bordered rectangle.
         * The original spell panel is drawn on top of the action area, not
         * using graphic 20 (C020_GRAPHIC_PANEL_EMPTY) which is the inventory
         * panel background and has a different layout.  The spell casting UI
         * in the original uses the game's default panel region with the
         * spell area background (graphic 9) as a sub-region for the rune
         * buttons.  We preserve the procedural panel frame here. */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX, pnlY, pnlW, pnlH, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX, pnlY, pnlW, pnlH, M11_COLOR_BROWN);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX + 2, pnlY + 2, pnlW - 4, pnlH - 4, M11_COLOR_BROWN);

        /* ── Selected rune sequence (pass 44: C011 rune-label cells) ── */
        {
            int bI;
            int seqCellY = pnlY + 7;
            int seqCellGap = 2;
            int seqCellX = pnlX + ((pnlW - ((4 * M11_SPELL_LABEL_CELL_W) +
                                            (3 * seqCellGap))) / 2);
            int drewCells = 0;
            for (bI = 0; bI < 4; ++bI) {
                int cellX = seqCellX + bI * (M11_SPELL_LABEL_CELL_W + seqCellGap);
                if (bI < state->spellBuffer.runeCount) {
                    int rv = state->spellBuffer.runes[bI];
                    int rr = (rv - 0x60) / 6;
                    int rc = (rv - 0x60) % 6;
                    char abbrev[3];
                    M11_TextStyle seqStyle = g_text_small;
                    if (!m11_blit_spell_label_cell(state,
                                                   framebuffer,
                                                   framebufferWidth,
                                                   framebufferHeight,
                                                   cellX,
                                                   seqCellY,
                                                   1)) {
                        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      cellX, seqCellY,
                                      M11_SPELL_LABEL_CELL_W,
                                      M11_SPELL_LABEL_CELL_H,
                                      M11_COLOR_NAVY);
                        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      cellX, seqCellY,
                                      M11_SPELL_LABEL_CELL_W,
                                      M11_SPELL_LABEL_CELL_H,
                                      M11_COLOR_LIGHT_BLUE);
                    } else {
                        drewCells = 1;
                    }
                    m11_get_rune_abbrev(rr, rc, abbrev);
                    seqStyle.color = M11_COLOR_LIGHT_GREEN;
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  cellX + 1, seqCellY + 3, abbrev, &seqStyle);
                } else {
                    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  cellX, seqCellY,
                                  M11_SPELL_LABEL_CELL_W,
                                  M11_SPELL_LABEL_CELL_H,
                                  M11_COLOR_DARK_GRAY);
                }
            }
            if (!drewCells && state->spellBuffer.runeCount == 0) {
                M11_TextStyle dimStyle = g_text_small;
                dimStyle.color = M11_COLOR_DARK_GRAY;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              pnlX + 8, seqCellY + 2, "- - - -", &dimStyle);
            }
        }

        /* ── Separator line below rune sequence ── */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX + 6, pnlY + 22, pnlW - 12, 1, M11_COLOR_BROWN);

        /* ── Active rune row: category label + pass-44 C011 cell blits ── */
        if (state->spellBuffer.runeCount < 4) {
            int runeGap = 2;
            int rowWidth = 6 * M11_SPELL_LABEL_CELL_W + 5 * runeGap;
            int rowStartX = pnlX + ((pnlW - rowWidth) / 2);
            int runeY = pnlY + 38;
            M11_TextStyle catStyle = g_text_small;
            catStyle.color = M11_COLOR_YELLOW;
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          pnlX + 8, pnlY + 26, rowNames[row], &catStyle);

            for (spI = 0; spI < 6; ++spI) {
                int bx = rowStartX + spI * (M11_SPELL_LABEL_CELL_W + runeGap);
                char abbrev[3];
                M11_TextStyle runeStyle = g_text_small;
                if (!m11_blit_spell_label_cell(state,
                                               framebuffer,
                                               framebufferWidth,
                                               framebufferHeight,
                                               bx,
                                               runeY,
                                               0)) {
                    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  bx, runeY,
                                  M11_SPELL_LABEL_CELL_W,
                                  M11_SPELL_LABEL_CELL_H,
                                  M11_COLOR_NAVY);
                    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  bx, runeY,
                                  M11_SPELL_LABEL_CELL_W,
                                  M11_SPELL_LABEL_CELL_H,
                                  M11_COLOR_LIGHT_BLUE);
                }
                m11_get_rune_abbrev(row, spI, abbrev);
                runeStyle.color = M11_COLOR_WHITE;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              bx + 1, runeY + 3, abbrev, &runeStyle);
            }
        } else {
            /* All 4 runes selected — show cast-ready state */
            M11_TextStyle readyStyle = g_text_title;
            readyStyle.color = M11_COLOR_LIGHT_GREEN;
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          pnlX + 40, pnlY + 42, "CAST", &readyStyle);
        }

        /* ── Mana indicator (compact bar + value) ── */
        if (state->world.party.activeChampionIndex >= 0 &&
            state->world.party.activeChampionIndex < CHAMPION_MAX_PARTY) {
            const struct ChampionState_Compat* sc =
                &state->world.party.champions[state->world.party.activeChampionIndex];
            int barX = pnlX + 8, barY = pnlY + 64;
            int barW = pnlW - 16, barH = 6;
            int fillW;
            char manaStr[24];
            M11_TextStyle manaStyle = g_text_small;
            manaStyle.color = M11_COLOR_LIGHT_CYAN;
            /* Mana bar outline */
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          barX, barY, barW, barH, M11_COLOR_DARK_GRAY);
            /* Mana bar fill */
            fillW = (sc->mana.maximum > 0)
                ? (int)((long)sc->mana.current * (barW - 2) / sc->mana.maximum)
                : 0;
            if (fillW > barW - 2) fillW = barW - 2;
            if (fillW > 0) {
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              barX + 1, barY + 1, fillW, barH - 2,
                              M11_COLOR_LIGHT_BLUE);
            }
            snprintf(manaStr, sizeof(manaStr), "%d/%d",
                     (int)sc->mana.current, (int)sc->mana.maximum);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          barX, barY + barH + 2, manaStr, &manaStyle);
        }

        /* ── Remaining rune rows shown dimmed below active row ── */
        if (state->spellBuffer.runeCount < 4) {
            int futRow;
            int dimY = pnlY + 76;
            M11_TextStyle dimStyle = g_text_small;
            dimStyle.color = M11_COLOR_DARK_GRAY;
            for (futRow = row + 1; futRow < 4; ++futRow) {
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              pnlX + 8, dimY, rowNames[futRow], &dimStyle);
                dimY += 10;
            }
        }
    }

    /* Endgame victory overlay */
    if (state->gameWon) {
        if (m11_v1_chrome_mode_enabled() && state->assetsAvailable) {
            const M11_AssetSlot* theEnd = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader,
                (unsigned int)M11_GameView_GetV1EndgameTheEndGraphicId());
            const M11_AssetSlot* mirror = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader,
                (unsigned int)M11_GameView_GetV1EndgameChampionMirrorGraphicId());
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          0, 0, framebufferWidth, framebufferHeight,
                          M11_COLOR_DARK_GRAY);
            if (mirror && mirror->loaded && mirror->pixels) {
                int i;
                for (i = 0; i < 4; ++i) {
                    M11_AssetLoader_Blit(mirror, framebuffer, framebufferWidth,
                                         framebufferHeight, 19, 7 + (i * 48), 10);
                    if (i < state->world.party.championCount &&
                        state->world.party.champions[i].present) {
                        const M11_AssetSlot* portraits = M11_AssetLoader_Load(
                            (M11_AssetLoader*)&state->assetLoader,
                            (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
                        if (portraits && portraits->loaded && portraits->pixels &&
                            portraits->width >= 256 && portraits->height >= 87) {
                            int pIdx = state->world.party.champions[i].portraitIndex & 0x1F;
                            int srcPX = (pIdx & 7) * M11_PORTRAIT_W;
                            int srcPY = (pIdx >> 3) * M11_PORTRAIT_H;
                            if (srcPX + M11_PORTRAIT_W <= (int)portraits->width &&
                                srcPY + M11_PORTRAIT_H <= (int)portraits->height) {
                                M11_AssetLoader_BlitRegion(portraits,
                                    srcPX, srcPY,
                                    M11_PORTRAIT_W, M11_PORTRAIT_H,
                                    framebuffer, framebufferWidth, framebufferHeight,
                                    27, 13 + (i * 48), M11_COLOR_DARK_GRAY);
                            }
                        }
                        char champName[16];
                        M11_TextStyle nameStyle = g_text_small;
                        nameStyle.color = M11_COLOR_LIGHT_RED;
                        nameStyle.shadowColor = M11_COLOR_DARK_GRAY;
                        m11_format_champion_name(state->world.party.champions[i].name,
                                                 champName, sizeof(champName));
                        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                      87, 14 + (i * 48), champName, &nameStyle);
                        {
                            char rawTitle[CHAMPION_TITLE_LENGTH + 1];
                            const char* champTitle;
                            m11_format_champion_title(state->world.party.champions[i].title,
                                                      rawTitle, sizeof(rawTitle));
                            champTitle = rawTitle;
                            if (champTitle[0]) {
                                int titleX = M11_GameView_EndgameTitleXForSourceText(champName,
                                                                                         champTitle);
                                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                              titleX, 14 + (i * 48), champTitle, &nameStyle);
                            }
                        }
                        {
                            static const char* const kEndgameSkillLevelNames[15] = {
                                "NEOPHYTE", "NOVICE", "APPRENTICE", "JOURNEYMAN",
                                "CRAFTSMAN", "ARTISAN", "ADEPT", "EXPERT",
                                "` MASTER", "a MASTER", "b MASTER", "c MASTER",
                                "d MASTER", "e MASTER", "ARCHMASTER"
                            };
                            static const char* const kEndgameBaseSkillNames[CHAMPION_SKILL_COUNT] = {
                                "FIGHTER", "NINJA", "PRIEST", "WIZARD"
                            };
                            int skillIndex;
                            int skillY = 15 + (i * 48);
                            M11_TextStyle skillStyle = g_text_small;
                            skillStyle.color = M11_COLOR_SILVER;
                            skillStyle.shadowColor = M11_COLOR_DARK_GRAY;
                            for (skillIndex = 0; skillIndex < CHAMPION_SKILL_COUNT; ++skillIndex) {
                                int level = m11_endgame_source_skill_level(state, i, skillIndex);
                                char skillLine[32];
                                if (level <= 1) {
                                    continue;
                                }
                                if (level > 16) level = 16;
                                skillY += 8;
                                snprintf(skillLine, sizeof(skillLine), "%s %s",
                                         kEndgameSkillLevelNames[level - 2],
                                         kEndgameBaseSkillNames[skillIndex]);
                                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                              105, skillY, skillLine, &skillStyle);
                            }
                        }
                    }
                }
            }
            if (theEnd && theEnd->loaded && theEnd->pixels) {
                M11_AssetLoader_Blit(theEnd, framebuffer, framebufferWidth,
                                     framebufferHeight,
                                     (framebufferWidth - (int)theEnd->width) / 2,
                                     122,
                                     -1);
            }
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          103, 140, 115, 15, M11_COLOR_DARK_GRAY);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          105, 142, 111, 11, M11_COLOR_BLACK);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          110, 149, "RESTART THIS GAME", &g_text_small);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          127, 165, 67, 15, M11_COLOR_DARK_GRAY);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          129, 167, 63, 11, M11_COLOR_BLACK);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          134, 174, "QUIT", &g_text_small);
        } else {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          40, 40, 240, 120, M11_COLOR_BLACK);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          40, 40, 240, 120, M11_COLOR_LIGHT_GREEN);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          42, 42, 236, 116, M11_COLOR_LIGHT_GREEN);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          90, 52, "QUEST COMPLETE", &g_text_title);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          56, 80, "LORD CHAOS IS DEFEATED.", &g_text_shadow);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          52, 92, "THE FIRESTAFF IS RESTORED.", &g_text_shadow);
            if (state->showDebugHUD || !m11_v1_chrome_mode_enabled()) {
                char wonLine[48];
                snprintf(wonLine, sizeof(wonLine), "VICTORY AT TICK %u",
                         (unsigned int)state->gameWonTick);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              80, 112, wonLine, &g_text_small);
            }
            if (state->showDebugHUD || !m11_v1_chrome_mode_enabled()) {
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              80, 132, "ESC TO RETURN TO MENU", &g_text_small);
            }
        }
    }

    /* Dialog box overlay for text plaque inspection */
    if (state->dialogOverlayActive && state->dialogOverlayText[0] != '\0') {
        int dlgX = 30, dlgY = 50, dlgW = 260, dlgH = 80;
        int textY;
        int drewSourceBackdrop = 0;
        if (m11_v1_chrome_mode_enabled()) {
            drewSourceBackdrop = m11_draw_dm_dialog_backdrop(
                state, framebuffer, framebufferWidth, framebufferHeight);
        }
        if (!drewSourceBackdrop) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX, dlgY, dlgW, dlgH, M11_COLOR_BLACK);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX, dlgY, dlgW, dlgH, M11_COLOR_YELLOW);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX + 2, dlgY + 2, dlgW - 4, dlgH - 4, M11_COLOR_BROWN);
        }
        if (state->showDebugHUD || !m11_v1_chrome_mode_enabled()) {
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX + 8, dlgY + 8, "TEXT PLAQUE", &g_text_title);
        }
        if (drewSourceBackdrop) {
            m11_apply_dm_dialog_choice_patch(state, framebuffer,
                                             framebufferWidth, framebufferHeight);
            M11_TextStyle versionStyle = g_text_small;
            versionStyle.color = M11_COLOR_LIGHT_GRAY;
            versionStyle.shadowColor = M11_COLOR_DARK_GRAY;
            /* ReDMCSB DIALOG.C:F0427 prints "V3.4" into
             * C450_ZONE_DIALOG_VERSION after expanding the source
             * C000 dialog-box graphic.  ZONES.H reconstruction gives
             * type 4 / parent 4 / d1=192 / d2=7; parent zone 4 is the
             * 224×136 viewport, so screen origin is viewport + d1/d2. */
            int versionX, versionY;
            (void)M11_GameView_GetV1DialogVersionTextOrigin(&versionX, &versionY);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          versionX,
                          versionY,
                          "V3.4", &versionStyle);
        }
        /* Word-wrap the dialog text into the box.  Source-dialog mode uses the
         * C469 zone width decision instead of the old fixed 40-character cut. */
        if (drewSourceBackdrop) {
            char line1[80], line2[128];
            int lineCount = m11_dialog_source_split_two_lines(
                state->dialogOverlayText, line1, sizeof(line1), line2, sizeof(line2),
                m11_dialog_source_message_width_for_choices(state->dialogChoiceCount));
            textY = (state->dialogChoiceCount > 1)
                        ? M11_GameView_GetV1DialogMultiChoiceMessageTextY(lineCount)
                        : M11_GameView_GetV1DialogSingleChoiceMessageTextY(lineCount);
            m11_draw_text_centered_in_rect(framebuffer,
                                           framebufferWidth,
                                           framebufferHeight,
                                           M11_VIEWPORT_X,
                                           textY,
                                           M11_VIEWPORT_W,
                                           line1,
                                           &g_text_shadow);
            if (lineCount > 1 && line2[0] != '\0') {
                m11_draw_text_centered_in_rect(framebuffer,
                                               framebufferWidth,
                                               framebufferHeight,
                                               M11_VIEWPORT_X,
                                               textY + 8,
                                               M11_VIEWPORT_W,
                                               line2,
                                               &g_text_shadow);
            }
        } else {
            textY = dlgY + ((state->showDebugHUD || !m11_v1_chrome_mode_enabled()) ? 28 : 18);
            if (strlen(state->dialogOverlayText) <= 40) {
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              dlgX + 12, textY, state->dialogOverlayText,
                              &g_text_shadow);
            } else {
                /* Split at nearest space before character 40 */
                char line1[48], line2[80];
                int splitPos = 40;
                while (splitPos > 0 && state->dialogOverlayText[splitPos] != ' ') {
                    --splitPos;
                }
                if (splitPos == 0) splitPos = 40;
                memcpy(line1, state->dialogOverlayText, (size_t)splitPos);
                line1[splitPos] = '\0';
                snprintf(line2, sizeof(line2), "%s",
                         state->dialogOverlayText + splitPos + (state->dialogOverlayText[splitPos] == ' ' ? 1 : 0));
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              dlgX + 12, textY, line1, &g_text_shadow);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              dlgX + 12, textY + 14, line2, &g_text_shadow);
            }
        }
        if (state->showDebugHUD || !m11_v1_chrome_mode_enabled()) {
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX + 50, dlgY + dlgH - 16,
                          "PRESS ANY KEY TO DISMISS", &g_text_small);
        }
        if (drewSourceBackdrop) {
            m11_draw_dialog_choices_source(state, framebuffer,
                                           framebufferWidth, framebufferHeight);
        }
    }

    /* Rest / death overlay */
    if (state->partyDead) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      60, 70, 200, 60, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      60, 70, 200, 60, M11_COLOR_LIGHT_RED);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      80, 80, "GAME OVER", &g_text_title);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      72, 108, "LOAD SAVE OR ESC TO MENU", &g_text_shadow);
    } else if (state->resting) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      100, 70, 120, 30, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      100, 70, 120, 30, M11_COLOR_LIGHT_BLUE);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      112, 78, "RESTING...", &g_text_shadow);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      112, 88, "R TO WAKE", &g_text_small);
    }

    /* ── Damage flash: red border overlay when party takes melee hit ── */
    if (state->damageFlashTimer > 0) {
        int vx = 12, vy = 24, vw = 196, vh = 118;
        int thickness = 2;
        /* Top border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx, vy, vw, thickness, M11_COLOR_LIGHT_RED);
        /* Bottom border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx, vy + vh - thickness, vw, thickness, M11_COLOR_LIGHT_RED);
        /* Left border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx, vy, thickness, vh, M11_COLOR_LIGHT_RED);
        /* Right border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx + vw - thickness, vy, thickness, vh, M11_COLOR_LIGHT_RED);
    }

    /* ── Attack cue: diagonal slash marks when creature attacks ── */
    if (state->attackCueTimer > 0) {
        int cx = 110, cy = 78; /* centre of viewport face */
        int len = 14;
        int i;
        /* Draw two crossing diagonal lines (X slash) */
        for (i = 0; i < len; ++i) {
            int px1 = cx - len / 2 + i;
            int py1 = cy - len / 2 + i;
            int px2 = cx + len / 2 - i;
            int py2 = cy - len / 2 + i;
            if (px1 >= 0 && px1 < framebufferWidth &&
                py1 >= 0 && py1 < framebufferHeight)
                framebuffer[py1 * framebufferWidth + px1] = M11_COLOR_YELLOW;
            if (px2 >= 0 && px2 < framebufferWidth &&
                py2 >= 0 && py2 < framebufferHeight)
                framebuffer[py2 * framebufferWidth + px2] = M11_COLOR_YELLOW;
        }
    }

    /* ── Creature-hit overlay: GRAPHICS.DAT graphic 14 on viewport ── */
    /* In the original DM1, when the party deals melee damage to a
     * creature, the "damage to creature" graphic (C014, 88×45) is
     * drawn centered in the viewport's action-result zone with the
     * damage number overlaid.  Size scales with damage magnitude:
     * >40 = full (88×45), 15-40 = medium (64×37), <15 = small (42×37).
     * Ref: ReDMCSB MELEE.C, G2093-G2096. */
    if (state->creatureHitOverlayTimer > 0) {
        int vCX = 110, vCY = 78; /* viewport center */
        int drawn = 0;
        if (state->assetsAvailable) {
            const M11_AssetSlot* dmg14 = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader,
                (unsigned int)M11_GameView_GetV1CreatureDamageGraphicId());
            if (dmg14 && dmg14->width > 0 && dmg14->height > 0) {
                int blitW, blitH;
                if (state->creatureHitDamageAmount > 40) {
                    blitW = 88; blitH = 45;
                } else if (state->creatureHitDamageAmount > 15) {
                    blitW = 64; blitH = 37;
                } else {
                    blitW = 42; blitH = 37;
                }
                M11_AssetLoader_BlitScaled(dmg14, framebuffer,
                    framebufferWidth, framebufferHeight,
                    vCX - blitW / 2, vCY - blitH / 2,
                    blitW, blitH, 0);
                drawn = 1;
            }
        }
        if (!drawn) {
            /* Fallback: tinted rectangle */
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          vCX - 32, vCY - 16, 64, 32, M11_COLOR_LIGHT_RED);
        }
        /* Damage number centered on overlay */
        {
            char hitNum[8];
            M11_TextStyle hitStyle = g_text_small;
            hitStyle.color = M11_COLOR_WHITE;
            snprintf(hitNum, sizeof(hitNum), "%d",
                     state->creatureHitDamageAmount);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          vCX - 6, vCY - 3, hitNum, &hitStyle);
        }
    }

    /* ── Full-screen overlay panels (drawn last, on top of everything) ── */
    if (state->mapOverlayActive) {
        m11_draw_fullscreen_map(state, framebuffer, framebufferWidth, framebufferHeight);
    }
    if (state->inventoryPanelActive) {
        m11_draw_inventory_panel(state, framebuffer, framebufferWidth, framebufferHeight);
    }

    g_drawState = NULL;
    g_activeOriginalFont = NULL;
}

/* ── Creature animation implementation ── */

void M11_GameView_TickAnimation(M11_GameViewState* state) {
    if (!state) return;
    state->animTick++;
    if (state->damageFlashTimer > 0) state->damageFlashTimer--;
    if (state->attackCueTimer > 0)   state->attackCueTimer--;
    if (state->creatureHitOverlayTimer > 0) state->creatureHitOverlayTimer--;
    { int ci; for (ci = 0; ci < 4; ++ci) {
        if (state->championDamageTimer[ci] > 0) state->championDamageTimer[ci]--;
    }}
}

void M11_GameView_NotifyDamageFlash(M11_GameViewState* state,
                                    int creatureType) {
    if (!state) return;
    state->damageFlashTimer = M11_DAMAGE_FLASH_DURATION;
    state->attackCueTimer   = M11_ATTACK_CUE_DURATION;
    state->attackCueCreatureType = creatureType;
}

int M11_GameView_GetDamageFlashTimer(const M11_GameViewState* state) {
    return state ? state->damageFlashTimer : 0;
}

int M11_GameView_GetAttackCueTimer(const M11_GameViewState* state) {
    return state ? state->attackCueTimer : 0;
}

void M11_GameView_NotifyChampionDamage(M11_GameViewState* state,
                                       int championSlot,
                                       int damageAmount) {
    if (!state || championSlot < 0 || championSlot >= 4) return;
    state->championDamageTimer[championSlot] = M11_DAMAGE_FLASH_DURATION;
    state->championDamageAmount[championSlot] = damageAmount;
}

uint32_t M11_GameView_GetAnimTick(const M11_GameViewState* state) {
    return state ? state->animTick : 0;
}

void M11_GameView_NotifyCreatureHit(M11_GameViewState* state,
                                    int damageAmount) {
    if (!state) return;
    state->creatureHitOverlayTimer = M11_CREATURE_HIT_OVERLAY_DURATION;
    state->creatureHitDamageAmount = damageAmount;
}

int M11_GameView_GetCreatureHitOverlayTimer(const M11_GameViewState* state) {
    return state ? state->creatureHitOverlayTimer : 0;
}

int M11_GameView_CreatureAnimFrame(const M11_GameViewState* state,
                                   int creatureType) {
    /* Stagger animation per creature type so groups don't all
     * move in lockstep.  Returns 0 or 1. */
    uint32_t phase;
    if (!state) return 0;
    phase = state->animTick + (uint32_t)creatureType * 3;
    return (int)((phase / M11_CREATURE_ANIM_PERIOD) & 1);
}

int M11_GameView_GetAttackCueCreatureType(const M11_GameViewState* state) {
    return state ? state->attackCueCreatureType : -1;
}

/* ── Full-screen map overlay API ── */

int M11_GameView_ToggleMapOverlay(M11_GameViewState* state) {
    if (!state) return 0;
    state->mapOverlayActive = !state->mapOverlayActive;
    return state->mapOverlayActive;
}

int M11_GameView_IsMapOverlayActive(const M11_GameViewState* state) {
    return state ? state->mapOverlayActive : 0;
}

/* ── Full inventory panel API ── */

int M11_GameView_ToggleInventoryPanel(M11_GameViewState* state) {
    if (!state) return 0;
    state->inventoryPanelActive = !state->inventoryPanelActive;
    if (state->inventoryPanelActive) {
        state->inventorySelectedSlot = 0;
    }
    return state->inventoryPanelActive;
}

int M11_GameView_IsInventoryPanelActive(const M11_GameViewState* state) {
    return state ? state->inventoryPanelActive : 0;
}

int M11_GameView_GetInventorySelectedSlot(const M11_GameViewState* state) {
    return state ? state->inventorySelectedSlot : -1;
}

const char* M11_GameView_SlotName(int slotIndex) {
    switch (slotIndex) {
        case CHAMPION_SLOT_HEAD:       return "HEAD";
        case CHAMPION_SLOT_NECK:       return "NECK";
        case CHAMPION_SLOT_TORSO:      return "TORSO";
        case CHAMPION_SLOT_LEGS:       return "LEGS";
        case CHAMPION_SLOT_FEET:       return "FEET";
        case CHAMPION_SLOT_POUCH_1:    return "POUCH 1";
        case CHAMPION_SLOT_POUCH_2:    return "POUCH 2";
        case CHAMPION_SLOT_QUIVER_1:   return "QUIVER 1";
        case CHAMPION_SLOT_QUIVER_2:   return "QUIVER 2";
        case CHAMPION_SLOT_QUIVER_3:   return "QUIVER 3";
        case CHAMPION_SLOT_QUIVER_4:   return "QUIVER 4";
        case CHAMPION_SLOT_BACKPACK_1: return "BACK 1";
        case CHAMPION_SLOT_BACKPACK_2: return "BACK 2";
        case CHAMPION_SLOT_BACKPACK_3: return "BACK 3";
        case CHAMPION_SLOT_BACKPACK_4: return "BACK 4";
        case CHAMPION_SLOT_BACKPACK_5: return "BACK 5";
        case CHAMPION_SLOT_BACKPACK_6: return "BACK 6";
        case CHAMPION_SLOT_BACKPACK_7: return "BACK 7";
        case CHAMPION_SLOT_BACKPACK_8: return "BACK 8";
        case CHAMPION_SLOT_HAND_LEFT:  return "L HAND";
        case CHAMPION_SLOT_HAND_RIGHT: return "R HAND";
        default: return "SLOT";
    }
}

/* ── Dialog / endgame query API ── */

int M11_GameView_IsGameWon(const M11_GameViewState* state) {
    return state ? state->gameWon : 0;
}

uint32_t M11_GameView_GetGameWonTick(const M11_GameViewState* state) {
    return state ? state->gameWonTick : 0;
}

int M11_GameView_IsDialogOverlayActive(const M11_GameViewState* state) {
    return state ? state->dialogOverlayActive : 0;
}

int M11_GameView_GetDialogSelectedChoice(const M11_GameViewState* state) {
    return state ? state->dialogSelectedChoice : 0;
}

int M11_GameView_DismissDialogOverlay(M11_GameViewState* state) {
    if (!state || !state->dialogOverlayActive) return 0;
    state->dialogOverlayActive = 0;
    state->dialogOverlayText[0] = '\0';
    state->dialogChoiceCount = 0;
    memset(state->dialogChoices, 0, sizeof(state->dialogChoices));
    return 1;
}

int M11_GameView_ShowDialogOverlay(M11_GameViewState* state,
                                   const char* text) {
    return M11_GameView_ShowDialogOverlayChoices(state, text, "OK", NULL, NULL, NULL);
}

int M11_GameView_ShowDialogOverlayChoices(M11_GameViewState* state,
                                          const char* text,
                                          const char* choice1,
                                          const char* choice2,
                                          const char* choice3,
                                          const char* choice4) {
    const char* choices[4];
    int i;
    if (!state || !text) return 0;
    state->dialogOverlayActive = 1;
    state->dialogSelectedChoice = 0;
    snprintf(state->dialogOverlayText, sizeof(state->dialogOverlayText),
             "%s", text);
    state->dialogChoiceCount = 0;
    memset(state->dialogChoices, 0, sizeof(state->dialogChoices));
    choices[0] = choice1;
    choices[1] = choice2;
    choices[2] = choice3;
    choices[3] = choice4;
    for (i = 0; i < 4; ++i) {
        if (choices[i] && choices[i][0] != '\0') {
            snprintf(state->dialogChoices[state->dialogChoiceCount],
                     sizeof(state->dialogChoices[state->dialogChoiceCount]),
                     "%s", choices[i]);
            ++state->dialogChoiceCount;
        }
    }
    return 1;
}

/* ── Creature aspect query API ── */

int M11_GameView_GetCreatureCoordinateSet(int creatureType) {
    return m11_creature_coordinate_set(creatureType);
}

int M11_GameView_GetCreatureTransparentColor(int creatureType) {
    return m11_creature_transparent_color(creatureType);
}

unsigned int M11_GameView_GetCreatureSpriteForDepth(int creatureType, int depthIndex) {
    return m11_creature_sprite_for_pose(creatureType, depthIndex,
                                        M11_CREATURE_POSE_FRONT);
}

unsigned int M11_GameView_GetCreatureSpriteForView(int creatureType,
                                                   int depthIndex,
                                                   int creatureDir,
                                                   int partyDir,
                                                   int attacking,
                                                   int* outMirror) {
    int relFacing = m11_creature_relative_facing(creatureDir, partyDir);
    int pose = m11_creature_pose_for_view(relFacing, attacking);
    if (outMirror) {
        *outMirror = m11_creature_pose_mirror_with_info(creatureType,
                                                        relFacing, pose,
                                                        attacking);
    }
    return m11_creature_sprite_for_pose(creatureType, depthIndex, pose);
}

unsigned int M11_GameView_GetCreatureGraphicInfo(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0u;
    return (unsigned int)s_creatureAspects[creatureType].graphicInfo;
}

int M11_GameView_GetCreatureAdditional(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (int)M11_CREATURE_GI_ADDITIONAL(
        (unsigned int)s_creatureAspects[creatureType].graphicInfo);
}

int M11_GameView_CreatureHasSpecialD2Front(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_SPECIAL_D2_FRONT) ? 1 : 0;
}

int M11_GameView_CreatureHasD2FrontIsFlippedFront(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_D2_FRONT_IS_FLIPPED) ? 1 : 0;
}

int M11_GameView_CreatureHasFlipDuringAttack(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_FLIP_DURING_ATTACK) ? 1 : 0;
}

int M11_GameView_GetCreatureNativeBitmapCount(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return m11_creature_native_bitmap_count_from_gi(
        (unsigned int)s_creatureAspects[creatureType].graphicInfo);
}

int M11_GameView_GetCreatureDerivedBitmapCount(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return m11_creature_derived_bitmap_count_from_gi(
        (unsigned int)s_creatureAspects[creatureType].graphicInfo);
}

int M11_GameView_GetCreatureMaxHorizontalOffset(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (int)M11_CREATURE_GI_MAX_HORIZONTAL_OFFSET(
        (unsigned int)s_creatureAspects[creatureType].graphicInfo);
}

int M11_GameView_GetCreatureMaxVerticalOffset(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (int)M11_CREATURE_GI_MAX_VERTICAL_OFFSET(
        (unsigned int)s_creatureAspects[creatureType].graphicInfo);
}

int M11_GameView_CreatureHasSideBitmap(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_SIDE) ? 1 : 0;
}

int M11_GameView_CreatureHasBackBitmap(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_BACK) ? 1 : 0;
}

int M11_GameView_CreatureHasAttackBitmap(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_ATTACK) ? 1 : 0;
}

int M11_GameView_CreatureHasFlipNonAttack(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_FLIP_NON_ATTACK) ? 1 : 0;
}

int M11_GameView_CreatureHasFlipAttack(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return (s_creatureAspects[creatureType].graphicInfo
            & M11_CREATURE_GI_MASK_FLIP_ATTACK) ? 1 : 0;
}

int M11_GameView_GetCreatureReplacementColors(int creatureType,
                                               int* outReplDst9,
                                               int* outReplDst10) {
    return m11_creature_replacement_colors(creatureType, outReplDst9, outReplDst10);
}

int M11_GameView_GetFloorOrnamentOrdinal(const M11_GameViewState* state,
                                         int relForward, int relSide) {
    M11_ViewportCell cell;
    if (!state || !state->active) return 0;
    if (!m11_sample_viewport_cell(state, relForward, relSide, &cell)) return 0;
    return cell.floorOrnamentOrdinal;
}
