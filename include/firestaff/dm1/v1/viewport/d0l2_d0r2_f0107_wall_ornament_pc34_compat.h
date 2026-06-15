#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0L2_D0R2_F0107_SIDE_COUNT_PC34 2
#define DM1_V1_D0L2_D0R2_F0107_ELEMENT_COUNT_PC34 7
#define DM1_V1_D0L2_D0R2_F0107_CALL_COUNT_PC34 8
#define DM1_V1_D0L2_D0R2_F0107_ORDINAL_COUNT_PC34 6
#define DM1_V1_D0L2_D0R2_F0107_PIXEL_COUNT_PC34 8
#define DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D0L2_D0R2_F0107_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0L2_D0R2_F0107_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0L2_D0R2_F0107_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D0L2_D0R2_F0107_SIDE_D0L2_PC34 = 0,
    DM1_V1_D0L2_D0R2_F0107_SIDE_D0R2_PC34 = 1
} DM1_V1_D0L2D0R2F0107SidePc34;

typedef enum {
    DM1_V1_D0L2_D0R2_F0107_ELEMENT_WALL_PC34 = 0,
    DM1_V1_D0L2_D0R2_F0107_ELEMENT_CORRIDOR_PC34 = 1,
    DM1_V1_D0L2_D0R2_F0107_ELEMENT_PIT_PC34 = 2,
    DM1_V1_D0L2_D0R2_F0107_ELEMENT_TELEPORTER_PC34 = 5,
    DM1_V1_D0L2_D0R2_F0107_ELEMENT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D0L2_D0R2_F0107_ELEMENT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D0L2_D0R2_F0107_ELEMENT_STAIRS_SIDE_PC34 = 18
} DM1_V1_D0L2D0R2F0107ElementPc34;

typedef struct {
    int side;
    const char *side_name;
    const char *function_name;
    int dispatcher_update_line;
    int dispatcher_draw_line;
    int function_start_line;
    int function_end_line;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int wall_zone;
    int ceiling_pit_zone;
    int first_thing_slot;
    int right_wall_ornament_slot;
    int front_wall_ornament_slot;
    int left_wall_ornament_slot;
    int thing_pass_line;
    int thing_pass_cell_order;
    int terminal_side_pair_pass;
    int pair_dispatch_order;
    const char *redmcsb_anchor;
} DM1_V1_D0L2D0R2F0107LanePc34;

typedef struct {
    int element;
    const char *element_name;
    int supported_by_f0125_f0126;
    int returns_before_tail;
    int has_ceiling_tail;
    int has_thing_pass_tail;
    int has_teleporter_field_tail;
    int f0107_short_circuit_ordinal_zero;
    int d0l_line_start;
    int d0l_line_end;
    int d0r_line_start;
    int d0r_line_end;
    const char *redmcsb_anchor;
} DM1_V1_D0L2D0R2F0107ElementRoutePc34;

typedef struct {
    int call_index;
    int side;
    int element;
    int aspect_slot;
    const char *aspect_slot_name;
    int view_wall;
    const char *view_wall_name;
    int coordinate_set;
    int zone;
    int ordinal_short_circuits;
    int alcove_returns_true;
    int uses_c10_transparency;
    const char *redmcsb_anchor;
} DM1_V1_D0L2D0R2F0107CallPc34;

typedef struct {
    int ordinal_index_c0_to_c5;
    int sensor_ordinal;
    int accepted_at_d0l2_front;
    int accepted_at_d0r2_front;
    int decremented_index;
    int source_slot_m552;
    const char *redmcsb_anchor;
} DM1_V1_D0L2D0R2F0107OrdinalPc34;

typedef struct {
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    int side;
    int element;
    const char *redmcsb_anchor;
} DM1_V1_D0L2D0R2F0107PixelPc34;

typedef struct {
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int c10_transparent_color;
    int wall_ornament_zone_base;
    int wall_ornament_zone_stride;
    int wall_ornament_coordinate_set;
    int d0l2_front_wall_zone;
    int d0r2_front_wall_zone;
    int m550_first_thing_slot;
    int m551_right_wall_ornament_slot;
    int m552_front_wall_ornament_slot;
    int m553_left_wall_ornament_slot;
    int f0128_d0l2_before_d0r2;
    int f0128_after_f0116_f0117_wall_composition;
    int terminal_depth_side_pair_correction;
    int direct_f0107_call_count;
    int f0107_candidate_call_count;
    int zero_ordinal_returns_false;
    int non_alcove_returns_false;
    int alcove_returns_true;
    int c0_to_c5_ordinals_pinned;
    int c10_transparent_preserves_destination;
    int field_level_byte_stability;
    int deterministic_seed;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    int no_graphics_dat_reads;
    DM1_V1_D0L2D0R2F0107LanePc34 lanes[DM1_V1_D0L2_D0R2_F0107_SIDE_COUNT_PC34];
    DM1_V1_D0L2D0R2F0107ElementRoutePc34
        routes[DM1_V1_D0L2_D0R2_F0107_ELEMENT_COUNT_PC34];
    DM1_V1_D0L2D0R2F0107CallPc34 calls[DM1_V1_D0L2_D0R2_F0107_CALL_COUNT_PC34];
    DM1_V1_D0L2D0R2F0107OrdinalPc34 ordinals[DM1_V1_D0L2_D0R2_F0107_ORDINAL_COUNT_PC34];
    DM1_V1_D0L2D0R2F0107PixelPc34 pixels[DM1_V1_D0L2_D0R2_F0107_PIXEL_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34;

bool dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *out_model);

const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34(void);

uint32_t dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D0L2D0R2F0107LanePc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_lane_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0107ElementRoutePc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_route_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0107CallPc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_call_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0107OrdinalPc34 *
dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_ordinal_at_pc34(size_t index);

bool dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

int dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall);

uint8_t dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

int dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size);

const char *dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
