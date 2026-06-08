#ifndef FIRESTAFF_DM1_V1_VIEWPORT_F0095_FLOOR_ORNAMENT_AGGREGATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_F0095_FLOOR_ORNAMENT_AGGREGATE_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F0095_CENTER_D3C_PC34 = 0,
    DM1_V1_F0095_CENTER_D2C_PC34 = 1,
    DM1_V1_F0095_CENTER_D1C_PC34 = 2,
    DM1_V1_F0095_CENTER_D0C_PC34 = 3
} DM1_V1_F0095CenterSquarePc34;

typedef enum {
    DM1_V1_F0095_CENTER_ELEMENT_WALL_PC34 = 0,
    DM1_V1_F0095_CENTER_ELEMENT_DOOR_FRONT_PC34 = 1,
    DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34 = 2,
    DM1_V1_F0095_CENTER_ELEMENT_PIT_PC34 = 3,
    DM1_V1_F0095_CENTER_ELEMENT_STAIRS_PC34 = 4
} DM1_V1_F0095CenterElementPc34;

typedef struct {
    DM1_V1_F0095CenterSquarePc34 square;
    const char *name;
    const char *draw_function;
    int f0128_dispatch_line;
    int depth;
    int lane;
    int view_square_index;
    int view_floor_index;
    int door_front_f0108_line;
    int open_f0108_line;
    int door_front_first_f0115_line;
    int open_first_f0115_line;
    bool center_square_can_call_f0108;
    bool d0c_ceiling_keepout;
} DM1_V1_F0095CenterSquareSpecPc34;

typedef struct {
    int c10_transparent_color;
    int m558_floor_slot_pc34;
    int m558_floor_slot_i34;
    int floor_zone_base;
    int floor_zone_stride;
    int wall_zone_d3l2_c702;
    int wall_zone_d3r2_c703;
    bool f0095_wallset_loads_g0095_native_wall_binding;
    bool g0109_champion_portrait_box_is_not_floor_ornament;
    bool contract_only;
    bool real_asset_runtime_parity;
} DM1_V1_F0095FloorOrnamentBindingPc34;

typedef struct {
    bool accepted;
    bool calls_f0108;
    bool calls_f0098_inside_center_square;
    bool f0098_precedes_f0128_center_row;
    bool f0108_precedes_f0115_when_present;
    bool c10_transparency;
    bool decrements_ordinal;
    bool bug64_pit_overlay_contract;
    int floor_ornament_index;
    int floor_ornament_zone;
    int f0108_source_line;
    int f0115_source_line;
    const char *source_anchor;
} DM1_V1_F0095FloorOrnamentAggregateResultPc34;

int dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_count_pc34(void);

const DM1_V1_F0095CenterSquareSpecPc34 *
dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_at_pc34(int index);

const DM1_V1_F0095FloorOrnamentBindingPc34 *
dm1_v1_viewport_f0095_floor_ornament_aggregate_binding_pc34(void);

bool dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
    DM1_V1_F0095CenterSquarePc34 square,
    DM1_V1_F0095CenterElementPc34 element,
    int floor_ornament_ordinal,
    int coordinate_set,
    DM1_V1_F0095FloorOrnamentAggregateResultPc34 *out);

const char *
dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(void);

int dm1_v1_viewport_f0095_floor_ornament_aggregate_run_pc34(
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif
