#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D2C F0108 floor+ceiling+ornament source lock.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011: M558 floor ornament ordinal, footprint
 *   recursion, M592_VIEW_FLOOR_D2C center-footprint flip, C10 transparency,
 *   and PC34 C1500 + CoordinateSet * 11 + ViewFloor zone math.
 * - DUNVIEW.C F0121:7244-7388: D2C body.  Door-front calls F0108 at
 *   7314 before the F0115 pass-1 call at 7315; corridor/pit/teleporter/
 *   stairs-front route calls F0108 at 7357, F0112 at 7359-7365, F0115 at
 *   7368, and teleporter F0113 at 7377-7386 after F0115.
 * - DUNVIEW.C F0128:8511-8521: D2L, D2R, then D2C dispatch neighborhood.
 * - DUNVIEW.C F0107:3502-3938 and F0121:7308-7312: D2C wall-ornament branch
 *   is separate from this F0108 floor route.
 * - DUNVIEW.C F0115:4547-4581 and 4795-4800: ordered-cell nibble walk and
 *   door-front drawing pass, with pass 1 before F0111 and pass 2 after.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, F0172:2466-2523 and
 *   2666-2721: thing-list append/walk boundaries and M558 sensor ordinal.
 * - DEFS.H:2088, 2547-2559, 2669-2676, 2678-2705, 2739-2760, 4045-4049,
 *   4188/4212, 4223: C10, M550..M558, cell orders, M575..M583, floor views,
 *   C705/C706/C709, D2C ceiling zones, and C1500.
 *
 * The probe writes a synthetic 320x200 framebuffer constrained to the
 * original 224x136 viewport.  It does not read GRAPHICS.DAT and makes no
 * original DOS pixel-parity claim.
 */

#define DM1_V1_D2C_F0108_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D2C_F0108_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D2C_F0108_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2C_F0108_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2C_F0108_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2C_F0108_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D2C_F0108_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D2C_F0108_CONTEXT_COUNT_PC34 6
#define DM1_V1_D2C_F0108_EVENT_COUNT_PC34 11
#define DM1_V1_D2C_F0108_CELL_ORDER_COUNT_PC34 3
#define DM1_V1_D2C_F0108_SAMPLE_COUNT_PC34 8

typedef enum {
    DM1_V1_D2C_F0108_CONTEXT_WALL_PC34 = 0,
    DM1_V1_D2C_F0108_CONTEXT_CORRIDOR_PC34 = 1,
    DM1_V1_D2C_F0108_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_V1_D2C_F0108_CONTEXT_TELEPORTER_PC34 = 5,
    DM1_V1_D2C_F0108_CONTEXT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D2C_F0108_CONTEXT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D2CF0108ContextPc34;

typedef enum {
    DM1_V1_D2C_F0108_EVENT_F0128_DISPATCH_PC34 = 0,
    DM1_V1_D2C_F0108_EVENT_F0121_BODY_PC34,
    DM1_V1_D2C_F0108_EVENT_F0107_WALL_KEEP_OUT_PC34,
    DM1_V1_D2C_F0108_EVENT_F0108_FLOOR_PC34,
    DM1_V1_D2C_F0108_EVENT_F0112_CEILING_PC34,
    DM1_V1_D2C_F0108_EVENT_F0115_DOOR_PASS1_PC34,
    DM1_V1_D2C_F0108_EVENT_F0111_DOOR_PC34,
    DM1_V1_D2C_F0108_EVENT_F0115_DOOR_PASS2_PC34,
    DM1_V1_D2C_F0108_EVENT_F0115_OPEN_PASS_PC34,
    DM1_V1_D2C_F0108_EVENT_F0113_FIELD_PC34,
    DM1_V1_D2C_F0108_EVENT_FRAMEBUFFER_PROBE_PC34
} DM1_V1_D2CF0108EventKindPc34;

typedef struct {
    DM1_V1_D2CF0108EventKindPc34 kind;
    int order_index;
    int redmcsb_line;
    int expected_for_wall;
    int expected_for_door_front;
    int expected_for_open_route;
    int expected_for_teleporter;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D2CF0108EventPc34;

typedef struct {
    unsigned int input_ordinal;
    int has_input_ordinal;
    int footprint_flag_set;
    unsigned int cleared_ordinal;
    int primary_draws;
    int primary_index;
    int recursive_footprints_draw;
    int recursive_footprints_index;
    int flips_on_d2c_when_floor_is_flipped;
    int metadata_blit_count;
} DM1_V1_D2CF0108OrdinalPc34;

typedef struct {
    int order_value;
    int decoded_cells[4];
    int decoded_count;
    int door_front_pass;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D2CF0108CellOrderPc34;

typedef struct {
    int coordinate_set;
    int view_floor;
    int f0108_zone;
    int f0107_view_wall;
    int f0107_zone;
    int source_locked_distinct;
} DM1_V1_D2CF0108ZoneMathPc34;

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
    const char *name;
} DM1_V1_D2CF0108RectPc34;

typedef struct {
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
} DM1_V1_D2CF0108PixelSamplePc34;

typedef struct {
    DM1_V1_D2CF0108ContextPc34 context;
    unsigned int floor_ornament_ordinal;
    int floor_flipped;
    uint8_t ceiling_pixel;
    uint8_t floor_pixel;
    uint8_t ornament_pixel;
    uint8_t thing_pixel;
    uint8_t field_pixel;
    bool mutate_thing_list;
    bool allow_outside_viewport;
    bool allow_f0107_wall_duplicate;
    bool allow_f0111_only_route;
} DM1_V1_D2CF0108StatePc34;

typedef struct {
    int ok;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int f0128_d2l_d2r_before_d2c;
    int f0128_d2c_before_d1_d0;
    int f0108_calls;
    int f0112_calls;
    int f0115_calls;
    int f0111_calls;
    int f0113_calls;
    int wall_f0107_calls;
    int rejected_non_contract_state;
    int open_pit_still_draws_floor_ornament;
    int door_f0108_before_f0115_pass1;
    int door_pass1_before_f0111;
    int door_pass2_after_f0111;
    int open_route_f0112_before_f0115;
    int teleporter_f0113_after_f0115;
    int terminal_depth_side_pair_correction;
    int thing_list_mutation_guard_ok;
    int non_overlap_ok;
    int floor_zone;
    int f0107_contrast_zone;
    int touched_pixels;
    int transparent_skips;
    uint8_t ceiling_sample;
    uint8_t floor_sample;
    uint8_t ornament_sample;
    uint8_t thing_sample;
    uint8_t field_sample;
    uint32_t framebuffer_hash;
    uint32_t deterministic_hash;
} DM1_V1_D2CF0108ResultPc34;

typedef struct {
    int ok;
    int view_square_d2c;
    int view_floor_d2c;
    int first_thing_slot;
    int floor_ornament_slot;
    int wall_zone_d2c;
    int sibling_wall_zone_d3l;
    int sibling_wall_zone_d3r;
    int ceiling_zone_d2c_pc34;
    int color_flesh;
    int floor_zone_base;
    int f0108_start_line;
    int f0108_end_line;
    int f0121_start_line;
    int f0121_end_line;
    int f0128_d2c_update_line;
    int f0128_d2c_draw_line;
    int f0172_sensor_line;
    int f0172_first_thing_line;
    DM1_V1_D2CF0108EventPc34 events[DM1_V1_D2C_F0108_EVENT_COUNT_PC34];
    DM1_V1_D2CF0108CellOrderPc34 cell_orders[DM1_V1_D2C_F0108_CELL_ORDER_COUNT_PC34];
    DM1_V1_D2CF0108ZoneMathPc34 zone_math;
    DM1_V1_D2CF0108PixelSamplePc34 samples[DM1_V1_D2C_F0108_SAMPLE_COUNT_PC34];
    DM1_V1_D2CF0108RectPc34 viewport;
    DM1_V1_D2CF0108RectPc34 ceiling_rect;
    DM1_V1_D2CF0108RectPc34 floor_rect;
    DM1_V1_D2CF0108RectPc34 ornament_rect;
    DM1_V1_D2CF0108RectPc34 thing_rect;
    DM1_V1_D2CF0108RectPc34 field_rect;
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D2CF0108ModelPc34;

bool dm1_v1_viewport_d2c_f0108_model_build_pc34(
    DM1_V1_D2CF0108ModelPc34 *out_model);

const DM1_V1_D2CF0108ModelPc34 *
dm1_v1_viewport_d2c_f0108_model_pc34(void);

const DM1_V1_D2CF0108EventPc34 *
dm1_v1_viewport_d2c_f0108_event_at_pc34(size_t index);

const DM1_V1_D2CF0108CellOrderPc34 *
dm1_v1_viewport_d2c_f0108_cell_order_at_pc34(size_t index);

bool dm1_v1_viewport_d2c_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    int floor_flipped,
    DM1_V1_D2CF0108OrdinalPc34 *out);

uint8_t dm1_v1_viewport_d2c_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d2c_f0108_floor_zone_pc34(
    int coordinate_set,
    int view_floor);

int dm1_v1_viewport_d2c_f0108_f0107_wall_zone_pc34(
    int coordinate_set,
    int view_wall);

bool dm1_v1_viewport_d2c_f0108_initial_state_pc34(
    DM1_V1_D2CF0108ContextPc34 context,
    DM1_V1_D2CF0108StatePc34 *out);

bool dm1_v1_viewport_d2c_f0108_compose_pc34(
    const DM1_V1_D2CF0108StatePc34 *state,
    DM1_V1_D2CF0108ResultPc34 *out);

uint32_t dm1_v1_viewport_d2c_f0108_hash_model_pc34(
    const DM1_V1_D2CF0108ModelPc34 *model);

uint32_t dm1_v1_viewport_d2c_f0108_deterministic_hash_pc34(void);

const char *dm1_v1_viewport_d2c_f0108_source_evidence_pc34(void);

const char *dm1_v1_viewport_d2c_f0108_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
