#ifndef FIRESTAFF_DM1_V1_VIEWPORT_F0098_FLOOR_CEILING_FALLBACK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_F0098_FLOOR_CEILING_FALLBACK_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F0098_FALLBACK_STEP_F0128_DIRTY_GUARD_PC34 = 0,
    DM1_V1_F0098_FALLBACK_STEP_F0098_CLEAR_BLACK_AREA_PC34 = 1,
    DM1_V1_F0098_FALLBACK_STEP_F0098_COPY_CEILING_PC34 = 2,
    DM1_V1_F0098_FALLBACK_STEP_F0098_COPY_FLOOR_PC34 = 3,
    DM1_V1_F0098_FALLBACK_STEP_F0098_SET_VIEWPORT_SIZE_PC34 = 4,
    DM1_V1_F0098_FALLBACK_STEP_F0098_CLEAR_DIRTY_FLAG_PC34 = 5,
    DM1_V1_F0098_FALLBACK_STEP_F0128_ENUMERATE_D0L_PC34 = 6,
    DM1_V1_F0098_FALLBACK_STEP_F0128_ENUMERATE_D0R_PC34 = 7,
    DM1_V1_F0098_FALLBACK_STEP_F0128_PRESENT_AND_PREFILL_PC34 = 8
} DM1_V1_F0098FloorCeilingFallbackStepPc34;

typedef struct {
    DM1_V1_F0098FloorCeilingFallbackStepPc34 step;
    int order_index;
    const char *function_name;
    const char *source_anchor;
    const char *contract;
} DM1_V1_F0098FloorCeilingFallbackDispatchPc34;

typedef struct {
    bool contract_only;
    bool function_level_contract;
    bool direction_specific_contract;
    bool real_asset_required;
    int viewport_width;
    int viewport_height;
    int viewport_byte_width;
    int black_area_height;
    int ceiling_height;
    int floor_y;
    int floor_height;
    int dirty_guard_order;
    int f0098_entry_order;
    int f0098_exit_order;
    int f0128_viewport_enumeration_order;
    int present_order;
    int c10_transparent_color;
    int no_transparency_color;
    int floor_native_bitmap_index;
    int ceiling_native_bitmap_index;
    int d0l_view_square;
    int d0r_view_square;
    int d0l_wall_zone;
    int d0r_wall_zone;
    int d0l_wall_bitmap_index;
    int d0r_wall_bitmap_index;
    const char *floor_symbol;
    const char *ceiling_symbol;
    const char *d0l_view_square_symbol;
    const char *d0r_view_square_symbol;
    const char *d0l_wall_zone_symbol;
    const char *d0r_wall_zone_symbol;
    const char *source_lines;
} DM1_V1_F0098FloorCeilingFallbackSpecPc34;

const DM1_V1_F0098FloorCeilingFallbackSpecPc34 *
dm1_v1_viewport_f0098_floor_ceiling_fallback_spec_pc34(void);

const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *
dm1_v1_viewport_f0098_floor_ceiling_fallback_dispatch_pc34(size_t *count);

bool dm1_v1_viewport_f0098_floor_ceiling_should_enter_pc34(
    bool draw_floor_and_ceiling_requested);

bool dm1_v1_viewport_f0098_floor_ceiling_dirty_after_exit_pc34(void);

bool dm1_v1_viewport_f0098_floor_ceiling_zero_ordinal_draws_pc34(
    unsigned int ordinal);

int dm1_v1_viewport_f0098_floor_ceiling_blit_pixel_pc34(
    int destination_color,
    int source_color,
    int transparent_color);

const char *dm1_v1_viewport_f0098_floor_ceiling_fallback_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_F0098_FLOOR_CEILING_FALLBACK_PC34_COMPAT_H */
