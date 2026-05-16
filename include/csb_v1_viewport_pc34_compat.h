
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
} CSB_V1_ViewportConfig;

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg);
void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set);
void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id);
const char *csb_v1_viewport_source_evidence(void);

#endif

