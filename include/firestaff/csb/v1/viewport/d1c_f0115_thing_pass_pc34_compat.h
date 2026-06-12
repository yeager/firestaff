#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0115_THING_PASS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0115_THING_PASS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_D1C_F0115_PASS_BACK_PC34 = 1,
    CSB_V1_D1C_F0115_PASS_FRONT_PC34 = 2
} CSB_V1_D1CF0115DoorPassPc34;

typedef enum {
    CSB_V1_D1C_F0115_CELL_BACK_LEFT_PC34 = 1,
    CSB_V1_D1C_F0115_CELL_BACK_RIGHT_PC34 = 2,
    CSB_V1_D1C_F0115_CELL_FRONT_RIGHT_PC34 = 3,
    CSB_V1_D1C_F0115_CELL_FRONT_LEFT_PC34 = 4
} CSB_V1_D1CF0115CellPc34;

typedef struct {
    uint16_t input_order;
    int has_door_marker;
    int door_pass;
    uint16_t stripped_order;
    int cell_count;
    int cells[4];
    int back_cell_count;
    int front_cell_count;
    int stops_at_zero_nibble;
    int invalid_cell_seen;
} CSB_V1_D1CF0115DecodedOrderPc34;

typedef struct {
    int contract_only;
    int no_game_data_load;
    int no_graphics_dat_load;
    int no_dungeon_dat_load;
    int no_real_asset_pixels;
    int disjoint_from_custom_backgrounds;
    int disjoint_from_f0108_floor_ceiling_ornament;
    int disjoint_from_f0111_partly_open_door;
    int d1c_view_square;
    int f0115_call_count;
    uint16_t first_back_pass_order;
    uint16_t second_front_pass_order;
    uint16_t door_marker_mask;
    uint16_t door_marker_nibble_pass1;
    uint16_t door_marker_nibble_pass2;
    const char *redmcsb_f0124_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
    const char *source_summary;
} CSB_V1_D1CF0115ThingPassContractPc34;

typedef struct {
    int door_pass;
    const char *pass_name;
    int d1c_view_square;
    uint16_t order_word;
    uint16_t order_after_marker;
    uint16_t door_marker_nibble;
    int expected_door_pass;
    int cell_count;
    int cells[4];
    int back_cell_count;
    int front_cell_count;
    int draws_before_door_frame;
    int draws_after_door_frame;
    int f0115_call_ordinal;
    int object_pass_first;
    int creature_pass_second;
    int projectile_pass_third;
    int explosion_pass_last;
    int item_row_guard;
    int creature_row_guard;
    int projectile_row_guard;
    int c10_transparency;
    const char *redmcsb_call_anchor;
    const char *redmcsb_order_anchor;
    const char *source_lines;
} CSB_V1_D1CF0115ThingPassPc34;

const CSB_V1_D1CF0115ThingPassContractPc34 *
csb_v1_viewport_d1c_f0115_thing_pass_contract_pc34(void);

size_t csb_v1_viewport_d1c_f0115_thing_pass_count_pc34(void);

const CSB_V1_D1CF0115ThingPassPc34 *
csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(size_t index);

const CSB_V1_D1CF0115ThingPassPc34 *
csb_v1_viewport_d1c_f0115_thing_pass_for_pass_pc34(int door_pass);

int csb_v1_viewport_d1c_f0115_decode_order_pc34(
    uint16_t ordered_view_cell_ordinals,
    CSB_V1_D1CF0115DecodedOrderPc34 *out_decoded);

const char *
csb_v1_viewport_d1c_f0115_thing_pass_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
