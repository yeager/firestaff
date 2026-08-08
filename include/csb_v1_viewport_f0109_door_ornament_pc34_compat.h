#ifndef FIRESTAFF_CSB_V1_VIEWPORT_F0109_DOOR_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_F0109_DOOR_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB viewport bridge for F0109 door ornament rendering.
 *
 * ReDMCSB DUNVIEW.C:4013-4117 F0109_DUNGEONVIEW_DrawDoorOrnament.
 *
 * Parameters:
 *   door_ornament_ordinal  1-based (0 = no ornament)
 *   view_door_ornament_index  C0=D3LCR, C1=D2LCR, C2=D1LCR
 *
 * F0109 decrements ordinal to index G0103_as_CurrentMapDoorOrnamentsInfo[]
 * which yields NativeBitmapIndex and CoordinateSet.
 *   D1: native bitmap directly (48x88).
 *   D2/D3: scales via F0129 with palette changes (G0200 for D3, G0201 for D2).
 *   Transparency: C09_COLOR_GOLD (9).
 */

typedef struct CSB_V1_ViewportDoorOrnamentPlanPc34 {
    bool accepted;
    int ordinal;
    int view_index;
    int depth;
    int coord_set;
    int native_bitmap_index;
    bool needs_scaling;
    int scale_width;
    int scale_height;
    int palette_remap_index;
    int transparent_color;
    const char *source_evidence;
} CSB_V1_ViewportDoorOrnamentPlanPc34;

/*
 * Compute a door ornament render plan from ordinal and view index.
 *
 * view_door_ornament_index: 0=D3LCR, 1=D2LCR, 2=D1LCR.
 * ordinal: 1-based door ornament ordinal (0 = no ornament, returns false).
 * cache_loaded: whether G0103 door ornament info cache is loaded.
 * local_to_global: map-local to global ornament index table (16 entries).
 *
 * Returns true if the plan was accepted, false if the ornament should not
 * be rendered (ordinal 0, out of range, or cache not loaded).
 */
bool csb_v1_viewport_f0109_door_ornament_plan_pc34(
    int ordinal,
    int view_door_ornament_index,
    int cache_loaded,
    const int local_to_global[16],
    CSB_V1_ViewportDoorOrnamentPlanPc34 *out_plan);

/*
 * Look up the G0207 coordinate set for a door ornament at a given depth.
 *
 * coord_set: coordinate set index from G0103 (0..3).
 * view_index: 0=D3, 1=D2, 2=D1.
 * out_x, out_y, out_w, out_h: bounding rectangle in ornament space.
 *
 * Returns true on success.
 */
bool csb_v1_viewport_f0109_door_ornament_coords_pc34(
    int coord_set,
    int view_index,
    int *out_x,
    int *out_y,
    int *out_w,
    int *out_h);

const char *csb_v1_viewport_f0109_door_ornament_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_F0109_DOOR_ORNAMENT_PC34_COMPAT_H */
