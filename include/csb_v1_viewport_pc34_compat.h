#ifndef FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Viewport — CSB-specific rendering differences
 *
 * CSB shares the DM1 viewport engine but has:
 * - Different wall sets (CSB dungeon themes)
 * - Custom room backgrounds (per DSA script)
 * - Extended creature graphics
 * - Prison door / intro sequence renderer
 *
 * Source: CSBWin/Viewport.cpp (7290 lines)
 * Source: CSBWin/Graphics.cpp (3186 lines)
 * Base: ReDMCSB DUNVIEW.C (shared viewport core)
 */

typedef struct {
    int wall_set_index;
    int custom_background;
    int prison_door_open;  /* 0-100 open percentage for intro */
    int has_custom_ceiling;
    uint32_t ambient_color;

    /* Viewport pixel buffer (224×136, the dungeon view area).
     * Points into the global g_framebuffer or a screen buffer.
     * Viewport occupies pixel rows [33..168] of a 320×200 screen. */
    uint8_t *viewport_pixels;
    int      viewport_stride;  /* bytes per row (320 for screen) */

    /* Dungeon data for rendering decisions.
     * When viewport_pixels is NULL, render calls are no-ops. */
    const uint8_t *dungeon_grid;
    int dungeon_width;
    int dungeon_height;
} CSB_V1_ViewportConfig;

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg);
void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set);
void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id);

/* Render one dungeon view frame using the DM1 viewport engine.
 * party_dir: facing direction (0=N, 1=E, 2=S, 3=W)
 * party_x, party_y: position on dungeon grid
 *
 * Uses dm1_viewport_3d_draw_frame internally; CSB config
 * selects CSB-specific wall set and custom backgrounds.
 *
 * No-op when cfg->viewport_pixels is NULL.
 */
void csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg,
                                   int party_dir,
                                   int party_x,
                                   int party_y);

const char *csb_v1_viewport_source_evidence(void);

#endif