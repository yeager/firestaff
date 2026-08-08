#ifndef FIRESTAFF_CSB_V1_VIEWPORT_F0110_DOOR_BUTTON_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_F0110_DOOR_BUTTON_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

/*
 * CSB V1 Viewport F0110 Door Button — pc34 compat layer.
 *
 * Thin wrapper bridging the CSB viewport pipeline to the existing DM1
 * door button functions (dm1_v1_viewport_3d_pc34_compat.h).
 *
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C F0110_DUNGEONVIEW_DrawDoorButton:4119-4216
 * - DUNVIEW.C G0208_aaauc_Graphic558_DoorButtonCoordinateSets:1210-1216
 * - DUNVIEW.C G0198/G0199 palette remap tables
 * - DEFS.H C10_COLOR_FLESH:2088
 * - DEFS.H M634_GRAPHIC_FIRST_DOOR_BUTTON
 *
 * Parameters (ReDMCSB F0110):
 *   door_button_ordinal: 1-based (0 = no button)
 *   view_door_button_index: C0=D3R, C1=D3C, C2=D2C, C3=D1C
 *
 * Coordinate sets from G0208[G0197[ordinal]][viewIndex]:
 *   D3R = {199,204,41,44,8,4}
 *   D3C = {136,141,41,44,8,4}
 *   D2C = {144,155,42,47,8,6}
 *   D1C = {160,175,44,52,8,9}
 *
 * At D1C: native bitmap, copy coord set to clickable box C05.
 * At D2/D3: derived (scaled) bitmap with palette remap
 *   (G0199 for D2, G0198 for D3).
 * Transparency color: C10_COLOR_FLESH (10).
 */

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_F0110_DOOR_BUTTON_PC34_TRANSPARENT_COLOR 10
#define CSB_V1_F0110_DOOR_BUTTON_PC34_SOURCE_LOCK_GATE "parity-csb-f0110-door-button"

/*
 * CSB plan struct for F0110 door button rendering.
 * Captures all decisions needed before pixel work begins.
 */
typedef struct {
    bool valid;                  /* true if ordinal > 0 and view_index in range */
    int ordinal;                 /* 1-based door button ordinal (0 = none) */
    int view_index;              /* DM1_ViewDoorButtonIndex: 0=D3R,1=D3C,2=D2C,3=D1C */
    int depth;                   /* viewport depth: 3 for D3R/D3C, 2 for D2C, 1 for D1C */
    int coord_x1;                /* G0208 left_x */
    int coord_x2;                /* G0208 right_x */
    int coord_y1;                /* G0208 top_y */
    int coord_y2;                /* G0208 bottom_y */
    int byte_width;              /* G0208 blit byte width */
    int height;                  /* G0208 blit height */
    bool needs_scaling;          /* true at D2/D3, false at D1C (native) */
    int native_bitmap_index;     /* ordinal - 1 + M634_GRAPHIC_FIRST_DOOR_BUTTON */
    int transparent_color;       /* C10_COLOR_FLESH = 10 */
    bool is_clickable;           /* true at D1C: copies coords to clickable box C05 */
    const uint8_t *palette_remap; /* G0198 for D3, G0199 for D2, NULL for D1C */
    const char *source_evidence;
} CSB_V1_ViewportDoorButtonPlanPc34;

/*
 * Build a rendering plan for the given door button ordinal and view index.
 * Returns the plan in *out_plan.  If ordinal is 0 or view_index is out of
 * range, out_plan->valid is false.
 *
 * view_index values: 0=D3R, 1=D3C, 2=D2C, 3=D1C
 * (matches DM1_ViewDoorButtonIndex enum in dm1_v1_viewport_3d_pc34_compat.h)
 */
void csb_v1_viewport_f0110_door_button_plan_pc34(
    int door_button_ordinal,
    int view_door_button_index,
    CSB_V1_ViewportDoorButtonPlanPc34 *out_plan);

/*
 * Return the source-locked coordinate set for the given door button
 * ordinal and view index.  Delegates to dm1_v1_viewport_get_door_button_frame_pc34.
 * Returns NULL if ordinal or view_index is invalid.
 *
 * The returned pointer points to static storage with fields:
 *   left_x, right_x, top_y, bottom_y, blit_x, blit_y, byte_width, height
 */
const void *csb_v1_viewport_f0110_door_button_coord_set_pc34(
    int door_button_ordinal,
    int view_door_button_index);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_F0110_DOOR_BUTTON_PC34_COMPAT_H */
