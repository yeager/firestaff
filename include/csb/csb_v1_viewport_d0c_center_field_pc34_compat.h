#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0C_CENTER_FIELD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0C_CENTER_FIELD_PC34_COMPAT_H

/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8537-8542
 * - DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4853-4920
 * - DEFS.H M609/C728/C811/C812/C824/C825/C862/C871/C715/C0x0021:2596,4086,4150-4164,4209,4218,4055,2662
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_D0C_CENTER_FIELD_ELEMENT_CORRIDOR = 1,
    CSB_V1_D0C_CENTER_FIELD_ELEMENT_PIT = 2,
    CSB_V1_D0C_CENTER_FIELD_ELEMENT_TELEPORTER = 5,
    CSB_V1_D0C_CENTER_FIELD_ELEMENT_DOOR_SIDE = 16,
    CSB_V1_D0C_CENTER_FIELD_ELEMENT_STAIRS_FRONT = 19
} CSB_V1_ViewportD0CCenterFieldElementPc34;

typedef enum {
    CSB_V1_D0C_CENTER_FIELD_ROUTE_INVALID = 0,
    CSB_V1_D0C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS,
    CSB_V1_D0C_CENTER_FIELD_ROUTE_DOOR_FRAME,
    CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_UP_PAIR,
    CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_DOWN_PAIR,
    CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_VISIBLE,
    CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_INVISIBLE,
    CSB_V1_D0C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD
} CSB_V1_ViewportD0CCenterFieldRoutePc34;

typedef struct {
    int element;
    int pit_or_teleporter_visible;
    int stairs_up;
    int first_thing;
} CSB_V1_ViewportD0CCenterFieldInputPc34;

typedef struct {
    int contract_only;
    int view_square;
    int view_lane;
    int view_depth;
    int field_aspect;
    int transparent_color;
    int f0115_cell_order;
    int door_frame_zone;
    int stairs_up_left_zone;
    int stairs_up_right_zone;
    int stairs_down_left_zone;
    int stairs_down_right_zone;
    int floor_pit_zone;
    int ceiling_pit_zone;
    int field_zone;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *source_evidence;
} CSB_V1_ViewportD0CCenterFieldContractPc34;

typedef struct {
    int accepted;
    CSB_V1_ViewportD0CCenterFieldRoutePc34 route;
    int view_square;
    int field_aspect;
    int first_thing;
    int calls_f0104;
    int calls_f0105;
    int calls_f0112;
    int calls_f0113;
    int calls_f0115;
    int calls_f0107;
    int calls_f0111;
    int door_frame_zone;
    int left_stairs_zone;
    int right_stairs_zone;
    int floor_pit_zone;
    int ceiling_pit_zone;
    int field_zone;
    int f0115_cell_order;
    int uses_invisible_pit_graphic;
    const char *source_evidence;
} CSB_V1_ViewportD0CCenterFieldPlanPc34;

const CSB_V1_ViewportD0CCenterFieldContractPc34 *
csb_v1_viewport_d0c_center_field_contract_pc34(void);

const char *
csb_v1_viewport_d0c_center_field_source_evidence_pc34(void);

CSB_V1_ViewportD0CCenterFieldPlanPc34
csb_v1_viewport_d0c_center_field_plan_pc34(
    CSB_V1_ViewportD0CCenterFieldInputPc34 input);

const char *
csb_v1_viewport_d0c_center_field_route_name_pc34(
    CSB_V1_ViewportD0CCenterFieldRoutePc34 route);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D0C_CENTER_FIELD_PC34_COMPAT_H */
