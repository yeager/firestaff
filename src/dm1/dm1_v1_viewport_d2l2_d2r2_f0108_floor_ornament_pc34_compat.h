#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0108_FLOOR_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0108_FLOOR_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34 15

typedef enum {
    DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34 = 0,
    DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34 = 1
} DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34;

typedef enum {
    DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34 = 0,
    DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_OPEN_PIT_PC34 = 1,
    DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_DOOR_FRONT_PC34 = 2
} DM1_V1_D2L2D2R2F0108FloorOrnamentContextPc34;

typedef struct {
    DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34 side;
    const char *label;
    int f0128_dispatch_order;
    int f067x_lateral2_view_square;
    int f0108_owner_view_square;
    int f0108_view_floor;
    int floor_ornament_aspect_slot;
    int floor_ornament_zone_base;
    int floor_ornament_zone_stride_pc34;
    int right_side_flip;
    int m575_view_wall_d3l_right;
    int m576_view_wall_d3r_left;
    int m577_view_wall_d3l_front;
    int m578_view_wall_d3c_front;
    int m579_view_wall_d3r_front;
    unsigned int corridor_cell_order;
    unsigned int door_pass1_cell_order;
    unsigned int door_pass2_cell_order;
    int transparent_color;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34;

typedef struct {
    DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34 side;
    DM1_V1_D2L2D2R2F0108FloorOrnamentContextPc34 context;
    unsigned int floor_ornament_ordinal;
    uint16_t first_thing_before;
    uint8_t destination_pixel_before;
    uint8_t floor_ornament_pixel;
    bool contract_enabled;
    bool allow_f0107_wall_overlap;
    bool allow_f0111_door_overlap;
    bool mutate_thing_list;
} DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34;

typedef struct {
    unsigned int input_ordinal;
    bool has_input_ordinal;
    bool footprint_flag_set;
    unsigned int cleared_ordinal;
    bool primary_draws;
    unsigned int primary_ordinal;
    int primary_index;
    bool recursive_footprints_draw;
    unsigned int recursive_footprints_ordinal;
    int recursive_footprints_index;
    int metadata_blit_count;
} DM1_V1_D2L2D2R2F0108FloorOrnamentOrdinalPc34;

typedef struct {
    const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *spec;
    DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 state;
    uint8_t destination_pixel_after;
    int f0108FootprintRecursionCount;
    int f0108OrnamentMetadataCount;
    int f0108OpenPitSkipCount;
    int f0678FrameCount;
    int f0679FrameCount;
    int f0115ThingPassNoOpCount;
    int c10TransparentBlitCount;
    int f0128PostCount;
    int d2l2CellThingUnchanged;
    int d2r2CellThingUnchanged;
    int mutationGuardsOk;
    int nonOverlapWithF0107F0111;
    bool rejected_non_contract_state;
    bool floor_ornament_drawn;
    bool open_pit_preserved;
} DM1_V1_D2L2D2R2F0108FloorOrnamentResultPc34;

size_t dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_count_pc34(void);

const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_for_pc34(
    DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34 side);

DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
    DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34 side,
    DM1_V1_D2L2D2R2F0108FloorOrnamentContextPc34 context);

bool dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D2L2D2R2F0108FloorOrnamentOrdinalPc34 *out);

uint8_t dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_blit_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

bool dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(
    const DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 *state,
    DM1_V1_D2L2D2R2F0108FloorOrnamentResultPc34 *out);

const char *
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
