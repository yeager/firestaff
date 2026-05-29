#ifndef THERON_V1_VIEWPORT_H
#define THERON_V1_VIEWPORT_H

#include "theron_v1_palette.h"
#include "theron_v1_world.h"
#include "theron_v1_mechanics.h"
#include <stdint.h>

/*
 * theron_v1_viewport.h — Theron's Quest V1 Phase 4: Rendering Pipeline
 *
 * Theron viewport renderer, UI chrome, and asset-wired presentation.
 *
 * Architecture mirrors nexus_v1_viewport.c: the viewport owns a local
 * framebuffer (PlanarFramebuffer) and renders the dungeon view from the
 * world state using the TQR tile/palette system.
 *
 * PC Engine viewport:
 *   Resolution:  256×224 (NTSC native, Theron standard)
 *   Tile size:  8×8 pixels (2bpp walls/floors, 4bpp sprites)
 *   Framebuffer: planar indexed bitmap, one byte per pixel (palette index)
 *
 * UI chrome zones (320×240 layout, Theron-specific):
 *   Top bar:      y=0..23   — dungeon name, quest item count
 *   Viewport:     x=32, y=24, 192×160 — dungeon view (letterboxed)
 *   Right panel:  x=224, y=24, 96×160  — Theron stats + compass
 *   Bottom panel: y=184, 320×56       — party panel (4 slots, 80px each)
 *   Message bar:  y=184..199         — single-line message area
 *
 * Asset wiring:
 *   TQR_PaletteState carries the loaded tile atlas from Track 02.
 *   Viewport queries tiles via tile index + palette group.
 *   Deterministic fallback: any unloaded tile → palette entry 7 (mid-gray).
 *
 * Source references:
 *   THQUEST.ASM T400   — tile bank loading
 *   THQUEST.ASM T520   — dungeon viewport tile selection
 *   THQUEST.ASM T600   — UI overlay zones
 *   HuC6260/HuC6270 datasheet — VDC/VCE rendering
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §7
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Viewport config ─────────────────────────────────────────────── */
#define TQR_VP_X       32
#define TQR_VP_Y       24
#define TQR_VP_W       192
#define TQR_VP_H       160
#define TQR_VP_DEPTH     4  /* view cone depth 4 squares */
#define TQR_FB_W       256  /* PC Engine native width  */
#define TQR_FB_H       224  /* PC Engine native height */
#define TQR_SCREEN_W   320  /* extended mode width     */
#define TQR_SCREEN_H   240  /* extended mode height   */

/* ── UI chrome zones ────────────────────────────────────────────── */
#define TQR_TOPBAR_H     24
#define TQR_RIGHT_W      96
#define TQR_BOTTOM_H     56
#define TQR_MSG_H        16

/* ── Champion slot layout (4 slots, bottom panel) ───────────────── */
#define TQR_CHAMP_SLOT_W     80
#define TQR_CHAMP_SLOT_H     56
#define TQR_CHAMP_SLOT_Y    184

/* ── Planar framebuffer ─────────────────────────────────────────── */
/*
 * Planar bitmap matching PC Engine HuC6260 VDC memory layout.
 * Each row: width bytes, one byte per pixel (palette index 0-255).
 * Tile decoding: planar bitplanes → indexed bitmap → palette lookup → RGBA.
 *
 * For rendering into the M11 320×200 framebuffer:
 *   theron_vp_present() converts the indexed framebuffer to RGBA
 *   and blits to the M11 framebuffer at the appropriate viewport origin.
 */
typedef struct {
    uint8_t *data;     /* indexed bitmap: TQR_FB_W × TQR_FB_H bytes */
    int      w;         /* width  (= TQR_FB_W = 256) */
    int      h;         /* height (= TQR_FB_H = 224) */
    int      stride;    /* row stride (= w) */
} TQR_PlanarFramebuffer;

/* ── Viewport state ─────────────────────────────────────────────── */
typedef struct {
    TQR_PlanarFramebuffer fb;       /* indexed framebuffer           */
    TQR_PaletteState      palette;  /* tile + palette atlas          */
    int                   viewport_x;   /* render offset X (letterbox) */
    int                   viewport_y;   /* render offset Y (letterbox) */
    int                   initialized;  /* 1=palette+fb ready           */
} Theron_V1_Viewport;

/* ── Camera / party view state ──────────────────────────────────── */
/*
 * Theron camera mirrors DM1 viewport cone sampling:
 *   D0 (depth 0) = square immediately ahead of party
 *   D1, D2, D3   = progressively farther
 *   left, center, right columns at each depth
 *
 * Facing direction: 0=N 1=E 2=S 3=W
 */
typedef struct {
    int party_x;
    int party_y;
    int party_dir;   /* 0=N 1=E 2=S 3=W */
} Theron_VP_Camera;

/* ── UI chrome render flags ─────────────────────────────────────── */
#define TQR_UI_TOPBAR       (1U << 0)
#define TQR_UI_RIGHT_PANEL  (1U << 1)
#define TQR_UI_BOTTOM_PANEL (1U << 2)
#define TQR_UI_MESSAGE      (1U << 3)
#define TQR_UI_ALL         (TQR_UI_TOPBAR|TQR_UI_RIGHT_PANEL|TQR_UI_BOTTOM_PANEL|TQR_UI_MESSAGE)

/* ══════════════════════════════════════════════════════════════════════
 * Viewport lifecycle
 * ══════════════════════════════════════════════════════════════════════ */

/* Initialize viewport: alloc planar framebuffer + init palette.
 * Must be called before any render.  Returns 0 on error. */
int theron_vp_init(Theron_V1_Viewport *vp);

/* Free viewport resources (palette tiles + framebuffer). */
void theron_vp_free(Theron_V1_Viewport *vp);

/* Wire a pre-loaded TQR_PaletteState into the viewport.
 * Used when the palette is already populated by asset loading
 * (e.g., from Track 02 tile extraction in theron_v1_boot). */
void theron_vp_set_palette(Theron_V1_Viewport *vp, const TQR_PaletteState *palette);

/* ══════════════════════════════════════════════════════════════════════
 * Dungeon rendering
 * ══════════════════════════════════════════════════════════════════════ */

/* Render the dungeon viewport from the given world state into vp->fb.
 * Uses the current party position and direction (world.party.champions[0]
 * slot 0 position/direction) to determine the view cone.
 *
 * Renders D0..D3 depth passes in order, compositing:
 *   - floor tile at each visible square
 *   - wall tile at each solid square or side wall
 *   - object/creature overlay tiles at D0/D1
 *
 * Deterministic fallback: any missing tile → palette entry 7 (mid-gray).
 *
 * Source: THQUEST.ASM T520 (tile selection), T400 (tile bank loading).
 */
void theron_vp_render_dungeon(Theron_V1_Viewport *vp,
                              const Theron_V1_World *world);

/* ══════════════════════════════════════════════════════════════════════
 * UI chrome rendering
 * ══════════════════════════════════════════════════════════════════════ */

/* Render Theron-specific UI chrome over the viewport fb.
 * Flags controls which zones are drawn (OR of TQR_UI_* bits).
 *
 * Top bar:     dungeon name + quest item count
 * Right panel: Theron stats (HP bar, level, compass direction)
 * Bottom:      4 champion slots (HP/stamina bars, name, class icon)
 * Message bar: last game message (truncated to 38 chars)
 *
 * Source: THQUEST.ASM T600 (UI overlay zones).
 */
void theron_vp_render_ui(Theron_V1_Viewport *vp,
                        const Theron_V1_World *world,
                        uint32_t ui_flags);

/* Render a single HP/stamina/mana bar into the planar fb.
 * x, y: top-left corner of the bar region (within planar fb coords).
 * w, h: bar dimensions in pixels.
 * current, max: bar values (0..max).
 * pal_index: palette entry to use for the filled portion.
 * bg_index: palette entry for the empty portion background.
 *
 * Bar style: Theron's Quest uses the PC Engine dungeon stone palette,
 * with a filled portion in the active color and a dark background.
 */
void theron_vp_draw_bar(TQR_PlanarFramebuffer *fb,
                        int x, int y, int w, int h,
                        int current, int max,
                        uint8_t pal_index,
                        uint8_t bg_index);

/* Render a champion slot (bottom panel).
 * slot_idx: 0..3 (Theron + 3 companions).
 * x, y: top-left of the 80×56 slot region.
 * champion: champion data (NULL = empty slot).
 *
 * Layout:
 *   icon (24×24) | name (20 chars) + class | HP bar | stamina bar | mana bar
 * Source: THQUEST.ASM T800 (champion panel rendering).
 */
void theron_vp_draw_champion_slot(TQR_PlanarFramebuffer *fb,
                                   int slot_idx,
                                   int x, int y,
                                   const Theron_V1_Champion *champion);

/* ══════════════════════════════════════════════════════════════════════
 * Presentation (planar fb → M11 framebuffer)
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * theron_vp_present — composite viewport into M11 framebuffer.
 *
 * Converts the indexed planar fb (palette indices 0-255) to the
 * M11 VGA palette indices via the TQR_PaletteState lookup, then
 * blits into the M11 framebuffer at the Theron viewport origin.
 *
 * Theron viewport is letterboxed within the 320×200 M11 framebuffer:
 *   M11 y=0..23:    black (title bar)
 *   M11 y=24..183:  Theron viewport (192×160 from 224-tall fb)
 *   M11 y=184..199: bottom chrome (champion panel)
 *   M11 y=200..:    black
 *
 * The 256-wide planar fb is centered in the 320-wide M11 fb:
 *   margin_x = (320 - 256) / 2 = 32
 *
 * Source: THQUEST.ASM T520 (viewport offset), HuC6260 datasheet.
 */
void theron_vp_present(const Theron_V1_Viewport *vp,
                       const TQR_PaletteState *palette,
                       unsigned char *m11_fb,
                       int m11_fb_w,
                       int m11_fb_h);

/* ══════════════════════════════════════════════════════════════════════
 * Utility
 * ══════════════════════════════════════════════════════════════════════ */

/* Get tile index for a given square type and depth.
 * Returns tile index into the TQR_PaletteState tile atlas, or -1.
 * depth: 0 (closest) to 3 (farthest).
 * square_type: THERON_SQUARE_* value.
 * is_wall: 1 if this square is a solid wall.
 *
 * Tile selection is deterministic based on square type + depth
 * (THQUEST.ASM T520).  Fallback tile = 0 (first tile in atlas).
 *
 * Returns: tile index (>=0), or -1 for fallback-to-flat.
 */
int theron_vp_tile_for_square(int square_type, int depth, int is_wall);

/* Clear the planar framebuffer with a solid color (palette index). */
void theron_vp_clear(Theron_V1_Viewport *vp, uint8_t color_index);

/* ── Source citation ─────────────────────────────────────────────── */
const char *theron_v1_viewport_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_VIEWPORT_H */