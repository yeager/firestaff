#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_CENTER_FIELD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_CENTER_FIELD_PC34_COMPAT_H

/*
 * ReDMCSB source anchors for the DM1 V1 D3C center-field contract:
 * - DUNVIEW.C F0128:8488-8499 dispatches depth 3, lane 0 to
 *   F0118_DUNGEONVIEW_DrawSquareD3C_CPSF.
 * - DUNVIEW.C F0118:6811-6831 is the D3C no-wall center-field route;
 *   DUNVIEW.C F0124:7922-7955 is the adjacent D1C no-wall reference.
 * - DUNVIEW.C F0113:6213-6219 and F0118:6825-6831 draw field overlays.
 * - DEFS.H:2595-2611 defines I34E view-square constants, including
 *   M600_VIEW_SQUARE_D3C and C14/C15 side-square indices.
 * - DEFS.H:3432-3437 defines the I34E wall ordinal family through C14_D3C.
 * - DEFS.H:4030-4049 and DEFS.H:4044 define the I34E wall-zone family
 *   and C704_ZONE_WALL_D3C.
 * - DEFS.H:2088 defines C10_COLOR_FLESH transparency.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_D3C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX_D3C = 11,
    DM1_V1_D3C_CENTER_FIELD_PC34_LANE = 0,
    DM1_V1_D3C_CENTER_FIELD_PC34_DEPTH = 3,
    DM1_V1_D3C_CENTER_FIELD_PC34_FIELD_ASPECT = 2,
    DM1_V1_D3C_CENTER_FIELD_PC34_C704_ZONE_WALL_D3C = 704
};

typedef enum {
    DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0098_FLOOR_CEILING = 0,
    DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0128_DISPATCH,
    DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0118_NO_WALL,
    DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0115_THINGS,
    DM1_V1_D3C_CENTER_FIELD_STEP_PC34_F0113_FIELD
} Dm1V1ViewportD3cCenterFieldStepIdPc34Compat;

typedef struct {
    Dm1V1ViewportD3cCenterFieldStepIdPc34Compat id;
    const char *name;
    const char *redmcsb_anchor;
} Dm1V1ViewportD3cCenterFieldStepPc34Compat;

typedef struct {
    int contract_only;
    int view_square_index_d3c;
    int lane;
    int depth;
    int field_aspect;
    int wall_case_returns;
    int routes_through_f0100;
    int routes_through_f0105;
    int routes_through_f0107;
    int routes_through_f0111;
    int routes_through_f0113;
    int preserves_c10_color_flesh_transparency;
    int g0163_frame_clip_applies;
    int floor_ceiling_ownership_f0098;
    int things_pass_includes_f0115_only_after_f0113;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0124_anchor;
    const char *redmcsb_f0113_anchor;
    const char *redmcsb_defs_view_square_anchor;
    const char *redmcsb_defs_zone_anchor;
    const char *redmcsb_defs_c10_anchor;
    const char *non_overlap_note;
    const char *source_summary;
} Dm1V1ViewportD3cCenterFieldContractPc34Compat;

const Dm1V1ViewportD3cCenterFieldContractPc34Compat *
dm1_v1_viewport_d3c_center_field_contract_pc34_compat(void);

size_t dm1_v1_viewport_d3c_center_field_steps_pc34_compat(
    Dm1V1ViewportD3cCenterFieldStepPc34Compat *out,
    size_t cap);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3C_CENTER_FIELD_PC34_COMPAT_H */
