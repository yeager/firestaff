#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3C_CENTER_FIELD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3C_CENTER_FIELD_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6642-6833
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8490-8499
 * - DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4853-4920
 * - DEFS.H M600/C704/C722/C723/C803/C816/C853/M625/C0x0218/C0x0349/C0x3421:2607,4044,4080-4081,4142,4155,4200,4253,2669,2672,2676
 */

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D3C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX 11
#define CSB_V1_D3C_CENTER_FIELD_PC34_LANE 0
#define CSB_V1_D3C_CENTER_FIELD_PC34_DEPTH 3
#define CSB_V1_D3C_CENTER_FIELD_PC34_FIELD_ASPECT 2
#define CSB_V1_D3C_CENTER_FIELD_PC34_TRANSPARENT_COLOR 10
#define CSB_V1_D3C_CENTER_FIELD_PC34_SOURCE_LOCK_GATE "parity-csb-d3c-center-field"

typedef enum {
    CSB_V1_D3C_CENTER_FIELD_ELEMENT_WALL = 0,
    CSB_V1_D3C_CENTER_FIELD_ELEMENT_CORRIDOR = 1,
    CSB_V1_D3C_CENTER_FIELD_ELEMENT_PIT = 2,
    CSB_V1_D3C_CENTER_FIELD_ELEMENT_TELEPORTER = 5,
    CSB_V1_D3C_CENTER_FIELD_ELEMENT_DOOR_FRONT = 17,
    CSB_V1_D3C_CENTER_FIELD_ELEMENT_STAIRS_FRONT = 19
} CSB_V1_D3CCenterFieldElementPc34;

typedef enum {
    CSB_V1_D3C_CENTER_FIELD_ROUTE_INVALID = 0,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_NO_ALCOVE,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_ALCOVE_THING_PASS,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_UP_THING_PASS,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_DOWN_THING_PASS,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_VISIBLE_THING_PASS,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_INVISIBLE_THING_PASS,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD,
    CSB_V1_D3C_CENTER_FIELD_ROUTE_DOOR_FRONT_DOUBLE_PASS
} CSB_V1_D3CCenterFieldRoutePc34;

typedef struct {
    int x1;
    int x2;
    int y1;
    int y2;
    int byte_width;
    int height;
    int blit_x;
    int blit_y;
} CSB_V1_D3CCenterFieldFramePc34;

typedef struct {
    int element;
    bool pit_or_teleporter_visible;
    bool stairs_up;
    bool wall_ornament_is_alcove;
    bool door_has_button;
    int first_thing;
} CSB_V1_D3CCenterFieldInputPc34;

typedef struct {
    bool contract_only;
    int view_square_index;
    int view_lane;
    int view_depth;
    int field_aspect;
    int wall_zone;
    int door_frame_left_zone;
    int door_frame_right_zone;
    int door_zone;
    int stairs_up_front_zone;
    int stairs_down_front_zone;
    int floor_pit_zone;
    int cell_order_alcove;
    int cell_order_door_pass1;
    int cell_order_door_pass2;
    int cell_order_open;
    int transparent_color;
    CSB_V1_D3CCenterFieldFramePc34 wall_frame;
    const char *redmcsb_f0118_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *non_lineage_replacement_marker;
    const char *source_evidence;
} CSB_V1_D3CCenterFieldContractPc34;

typedef struct {
    bool accepted;
    CSB_V1_D3CCenterFieldRoutePc34 route;
    int view_square_index;
    int field_aspect;
    int first_thing;
    int wall_zone;
    int door_frame_left_zone;
    int door_frame_right_zone;
    int door_zone;
    int stairs_zone;
    int floor_pit_zone;
    int field_zone;
    int first_cell_order;
    int second_cell_order;
    int f0115_call_count;
    bool calls_f0100_wall;
    bool calls_f0104;
    bool calls_f0105;
    bool calls_f0107_alcove_probe;
    bool calls_f0108_floor_ornament;
    bool calls_f0110_door_button;
    bool calls_f0111_door;
    bool calls_f0113_field;
    bool wall_returns_before_floor_ornament;
    bool pit_uses_floor_pit_bitmap;
    bool teleporter_field_after_thing_pass;
    const char *source_evidence;
} CSB_V1_D3CCenterFieldPlanPc34;

const CSB_V1_D3CCenterFieldContractPc34 *
csb_v1_viewport_d3c_center_field_contract_pc34(void);

const char *
csb_v1_viewport_d3c_center_field_source_evidence_pc34(void);

CSB_V1_D3CCenterFieldPlanPc34
csb_v1_viewport_d3c_center_field_plan_pc34(
    CSB_V1_D3CCenterFieldInputPc34 input);

int csb_v1_viewport_d3c_center_field_apply_synthetic_c10_field_pc34(
    const CSB_V1_D3CCenterFieldContractPc34 *contract,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *
csb_v1_viewport_d3c_center_field_route_name_pc34(
    CSB_V1_D3CCenterFieldRoutePc34 route);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D3C_CENTER_FIELD_PC34_COMPAT_H */
