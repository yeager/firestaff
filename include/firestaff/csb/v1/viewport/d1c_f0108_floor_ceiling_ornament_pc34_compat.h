#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D1C floor+ceiling+ornament source lock.
 * ReDMCSB anchors: DUNVIEW.C F0108:3940-4011, F0124:7873-7957,
 * F0128:8491-8499 and 8524-8542, F0115:4547-4581, 5458-5570,
 * 4923, 5180-5188, 5211-5214, 5668-5671; DUNGEON.C
 * F0163:1769-1838, F0164:1840-1905, F0172:2466-2523; DEFS.H
 * C10_COLOR_FLESH, M575..M579, M595, M606, C705/C706, C1500.
 * CSB-lineage anchors: Viewport.cpp:6507-6548 and 6924-6927.
 */

#define CSB_V1_D1C_F0108_C10_COLOR_FLESH_PC34 10
#define CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34 0x8000u
#define CSB_V1_D1C_F0108_FRAMEBUFFER_WIDTH_PC34 320
#define CSB_V1_D1C_F0108_FRAMEBUFFER_HEIGHT_PC34 200
#define CSB_V1_D1C_F0108_VIEWPORT_WIDTH_PC34 224
#define CSB_V1_D1C_F0108_VIEWPORT_HEIGHT_PC34 136
#define CSB_V1_D1C_F0108_FLOOR_BAND_ZONE_PC34 1505
#define CSB_V1_D1C_F0108_REDMCSB_PC34_ZONE_PC34 1509
#define CSB_V1_D1C_F0108_CUSTOM_BACKGROUNDS_SLOT_PC34 11
#define CSB_V1_D1C_F0108_CONTRACT_HASH_PC34 0x4f3ba518u

typedef enum {
    CSB_V1_D1C_F0108_CONTEXT_CORRIDOR_PC34 = 1,
    CSB_V1_D1C_F0108_CONTEXT_OPEN_PIT_PC34 = 2,
    CSB_V1_D1C_F0108_CONTEXT_TELEPORTER_PC34 = 5,
    CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34 = 17
} CSB_V1_D1CF0108ContextPc34;

typedef struct {
    CSB_V1_D1CF0108ContextPc34 context;
    const char *name;
    int redmcsb_element;
    int view_square;
    int view_lane;
    int view_depth;
    int field_aspect;
    int floor_view;
    int floor_band_zone;
    int redmcsb_pc34_zone;
    int ceiling_zone;
    unsigned int first_cell_order;
    unsigned int second_cell_order;
    int thing_passes;
    int custom_backgrounds_slot;
    int custom_backgrounds_apply_after_floor_ceiling;
    int custom_background_mask_covers_floor_band;
    int door_front_after_custom_backgrounds;
    int f0108_call_line;
    int f0112_before_f0115;
    int f0113_after_f0115;
    const char *redmcsb_anchor;
    const char *lineage_anchor;
} CSB_V1_D1CF0108SpecPc34;

typedef struct {
    int ok;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int assertions;
    int failures;
    int contexts;
    int floor_ornament_calls;
    int footprint_recursions;
    int c10_transparent_blits;
    int custom_bg_masks;
    int d1c_floor;
    int ceiling_copies;
    int thing_passes;
    int palette_keepouts;
    int mutation_rejections;
    uint32_t deterministic_hash;
} CSB_V1_D1CF0108SelfTestResultPc34;

size_t csb_v1_viewport_d1c_f0108_context_count_pc34(void);

const CSB_V1_D1CF0108SpecPc34 *
csb_v1_viewport_d1c_f0108_spec_at_pc34(size_t index);

const CSB_V1_D1CF0108SpecPc34 *
csb_v1_viewport_d1c_f0108_spec_for_context_pc34(
    CSB_V1_D1CF0108ContextPc34 context);

uint8_t csb_v1_viewport_d1c_f0108_blend_c10_pc34(uint8_t destination_pixel,
                                                 uint8_t source_pixel);

int csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(
    int coordinate_set,
    int view_floor);

int csb_v1_viewport_d1c_f0108_trace_context_pc34(
    const CSB_V1_D1CF0108SpecPc34 *spec,
    unsigned int floor_ornament_ordinal,
    uint32_t seed,
    CSB_V1_D1CF0108SelfTestResultPc34 *out_result);

int run_csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_self_test(void);

const CSB_V1_D1CF0108SelfTestResultPc34 *
csb_v1_viewport_d1c_f0108_last_self_test_result_pc34(void);

const char *csb_v1_viewport_d1c_f0108_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
