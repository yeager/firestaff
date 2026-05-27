#ifndef NEXUS_V1_UI_SURFACES_H
#define NEXUS_V1_UI_SURFACES_H
#include <stdint.h>

/* Nexus V1 UI / Title Surface Renderer
 * ===================================
 * Parses and renders DM Nexus Saturn UI screens and surface assets:
 *   TITLE.CG   — title screen color graphics (164 KB)
 *   WARNING.BIN — warning/disclaimer screen  (99 KB)
 *   GAMEOVER.BIN — game over screen         (101 KB)
 *   FACE.BIN    — champion portrait sprites (44 KB, 24 entries)
 *   STABG.BIN   — status-area background  (52 KB)
 *   FONT256.S2D — Saturn SCR font           (already nexus_v1_saturn_font.c)
 *
 * Rendering entry points:
 *   nexus_ui_render_title()    — blit TITLE.CG to framebuffer
 *   nexus_ui_render_warning()  — blit WARNING.BIN
 *   nexus_ui_render_gameover() — blit GAMEOVER.BIN
 *   nexus_ui_render_portrait() — blit FACE.BIN entry
 *   nexus_ui_render_stabg()    — blit STABG.BIN as status area bg
 *
 * Source-lock references:
 *   ReDMCSB BLIT.C      — F0132 blit rect (F0132)
 *   ReDMCSB PANEL.C    — panel draw (F0120-F0125)
 *   ReDMCSB CEDTINCK.C — CEDT font/text render
 *   Saturn SDK         — VDP1 bitmap surfaces, VDP2 background layers
 *   docs/NEXUS_FILE_CLASSIFICATION.md  — file sizes / formats
 *
 * Deterministic fallback:
 *   Any surface that fails to load produces a deterministic solid-color
 *   placeholder from the corresponding palette range, no crash. */

/* ── Surface descriptor ───────────────────────────────────────── */
#define NEXUS_UI_MAX_SURFACES 8

typedef struct {
    uint8_t  *data;       /* indexed pixels (320×200) or texture bitmap */
    int       w, h;        /* dimensions */
    uint8_t   pal_start;  /* first palette slot for this surface */
    uint8_t   pal_count;  /* number of palette entries */
    int       owns_data;  /* 1=calloc'd, 0=borrowed ref */
    const char *source;   /* e.g. "TITLE.CG" */
    uint64_t  hash;       /* SHA-256 hash of source file (if known) */
} Nexus_UI_Surface;

/* Named surfaces */
typedef enum {
    NEXUS_SURFACE_TITLE = 0,
    NEXUS_SURFACE_WARNING,
    NEXUS_SURFACE_GAMEOVER,
    NEXUS_SURFACE_FACE0,    /* portrait 0-23 */
    NEXUS_SURFACE_FACE23 = NEXUS_SURFACE_FACE0 + 23,
    NEXUS_SURFACE_STABG,    /* status area background */
    NEXUS_SURFACE_COUNT
} Nexus_UISurfaceType;

typedef struct {
    Nexus_UI_Surface surfaces[NEXUS_SURFACE_COUNT];
} Nexus_UI_Manager;

/* ── Manager lifecycle ─────────────────────────────────────────── */
void nexus_ui_manager_init(Nexus_UI_Manager *mgr);
void nexus_ui_manager_free(Nexus_UI_Manager *mgr);

/* ── Surface loaders ───────────────────────────────────────────── */

/* Load a generic indexed surface (320×200 CBUF or any w×h bitmap).
 * data may be NULL → fills with deterministic dark color + logs diag. */
int nexus_ui_surface_load(Nexus_UI_Manager *mgr,
    Nexus_UISurfaceType which,
    const uint8_t *data, int data_size,
    int w, int h,
    uint8_t pal_start, uint8_t pal_count,
    const char *source);

/* Load TITLE.CG (164 KB) as the title screen.
 * Expects data[0..163839] = indexed image (320×200 or larger).
 * Attempts to detect format: if first bytes == "SEGA" header,
 * skips to pixel data offset. Falls back to full data as image.    */
int nexus_ui_load_title(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load WARNING.BIN (99 KB) as disclaimer screen */
int nexus_ui_load_warning(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load GAMEOVER.BIN (101 KB) as game over screen */
int nexus_ui_load_gameover(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load STABG.BIN (52 KB) as status-area background (200×52 or 320×200) */
int nexus_ui_load_stabg(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_size,
    const uint32_t *palette);

/* Load FACE.BIN (44 KB) as champion portraits.
 * Layout: 24 portraits laid out in a horizontal strip.
 * Each portrait: 48×48 pixels (or closest power-of-2).
 * face_index: 0..23 → portrait number. */
int nexus_ui_load_faces(Nexus_UI_Manager *mgr,
    const uint8_t *data, int data_of_face,
    int data_size, int face_index,
    int portrait_w, int portrait_h,
    const uint32_t *palette);

/* Free a specific surface */
void nexus_ui_surface_free(Nexus_UI_Manager *mgr, Nexus_UISurfaceType which);

/* ── Rendering entry points ────────────────────────────────────── */

/* Simple 1:1 blit from surface → indexed framebuffer (320×200).
 * dx, dy = destination top-left in framebuffer.
 * Clips to framebuffer bounds.                               */
void nexus_ui_blit_surface(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h, int dx, int dy);

/* Blit with optional horizontal flip (for mirror-portrait champions) */
void nexus_ui_blit_surface_flip(const Nexus_UI_Surface *surf,
    uint8_t *fb, int fb_w, int fb_h, int dx, int dy, int flip_h);

/* Convenience wrappers using the global manager */
void nexus_ui_render_title(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h);
void nexus_ui_render_warning(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h);
void nexus_ui_render_gameover(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h);
void nexus_ui_render_stabg(const Nexus_UI_Manager *mgr,
    uint8_t *fb, int fb_w, int fb_h,
    int dest_x, int dest_y);
/* portrait_index 0..23 → blit FACE.BIN entry at that index */
void nexus_ui_render_portrait(const Nexus_UI_Manager *mgr,
    int portrait_index,
    uint8_t *fb, int fb_w, int fb_h,
    int dest_x, int dest_y, int flip_h);

/* ── Blit helpers (palette index remap) ─────────────────────────── */

/* Remap surface palette indices to a different base.
 * e.g. portrait at pal_start=64 → remap to fb palette offset 192.   */
void nexus_ui_surface_remap_pal(Nexus_UI_Surface *surf,
    uint8_t new_pal_start);

/* Darken a surface in-place for focus/blur states (e.g. paused overlay) */
void nexus_ui_surface_darken(Nexus_UI_Surface *surf, float factor);

#endif /* NEXUS_V1_UI_SURFACES_H */
