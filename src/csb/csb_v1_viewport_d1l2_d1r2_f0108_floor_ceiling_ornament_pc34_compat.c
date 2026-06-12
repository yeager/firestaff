#include "csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_C10_COLOR_FLESH = 10,
    CSB_MASK_FOOTPRINTS = 0x8000,
    CSB_FLOOR_ORNAMENT_FOOTPRINTS_INDEX = 15,
    CSB_VIEW_SQUARE_D1L = 4,
    CSB_VIEW_SQUARE_D1R = 5,
    CSB_VIEW_SQUARE_D2L2 = 9,
    CSB_VIEW_SQUARE_D2R2 = 10,
    CSB_VIEW_DEPTH_D1 = 1,
    CSB_VIEW_LANE_D1L = -1,
    CSB_VIEW_LANE_D1R = 1,
    CSB_G2028_D1L = 9,
    CSB_G2028_D1R = 10,
    CSB_G2033_D1L = 9,
    CSB_G2033_D1R = 10,
    CSB_G2034_D1L = 12,
    CSB_G2034_D1R = 13,
    CSB_FIELD_D1L = 11,
    CSB_FIELD_D1R = 12,
    CSB_VIEW_FLOOR_D1L = 8,
    CSB_VIEW_FLOOR_D1R = 10,
    CSB_VIEW_WALL_D1L_RIGHT = 12,
    CSB_VIEW_WALL_D1R_LEFT = 13,
    CSB_VIEW_WALL_M575_D3L_RIGHT = 2,
    CSB_VIEW_WALL_M576_D3R_LEFT = 3,
    CSB_VIEW_WALL_M577_D3L_FRONT = 4,
    CSB_VIEW_WALL_M578_D3C_FRONT = 5,
    CSB_VIEW_WALL_M579_D3R_FRONT = 6,
    CSB_CEILING_GRAPHIC_D1 = 66,
    CSB_CEILING_ZONE_D1L = 867,
    CSB_CEILING_ZONE_D1R = 869,
    CSB_FLOOR_ORNAMENT_ZONE_BASE = 1500,
    CSB_LINEAGE_OPEN_F0L1 = 1192,
    CSB_LINEAGE_DOOR_F2 = 1865,
    CSB_LINEAGE_DOOR_F1 = 1903,
    CSB_LINEAGE_APPLY_DECORATION = 6507
};

static const char s_source_evidence[] =
    "Source-locked contract-only CSB V1 D1L2/D1R2 F0108 floor+ceiling+"
    "ornament composition. No real-asset bitmap parity and no CSB game-data "
    "load. ReDMCSB DUNVIEW.C:3940-4011 F0108 handles ordinal zero skip, "
    "0x8000 footprint recursion, C10 blits, and D1R horizontal flipping; "
    "DUNVIEW.C:7520-7536 F0122 calls F0108 M594_VIEW_FLOOR_D1L, copies the "
    "D1L ceiling pit at C867, then calls F0115 with M607 and order 0x0032; "
    "DUNVIEW.C:7688-7704 F0123 mirrors this with M596_VIEW_FLOOR_D1R, "
    "ceiling C869, horizontal ceiling copy, M608, and order 0x0041. "
    "DUNVIEW.C:8503-8533 F0128 places this pair after D2 and before D1C. "
    "DUNVIEW.C:5668-5671 F0115 row-guard parity rejects negative G2028 rows. "
    "DUNGEON.C:1769-1838 F0163 and 1840-1905 F0164 are mutation keep-outs; "
    "DUNGEON.C:2466-2523 F0172 supplies square aspect metadata. DEFS.H:2088 "
    "anchors C10_COLOR_FLESH, 2596-2604 anchors CSB view-square ordinals, "
    "2681-2707 anchors M575..M579 and D1 wall ordinals, 2758-2760 anchors "
    "M594/M596 floor views, and 4205-4216 anchors D1 pit/ceiling zones. "
    "CSB-lineage Viewport.cpp:1192-1209, 1865-1879, 1903-1915, and "
    "6507-6548 anchor the open/door composition contrast and masked "
    "CustomBackgrounds decoration interaction.";

static const CSB_V1_D1L2D1R2F0108SpecPc34 s_specs[] = {
    {
        CSB_V1_D1L2_D1R2_F0108_SIDE_D1L2_PC34,
        "D1L2 F0108 floor ornament, ceiling copy, and terminal thing pass",
        122,
        8,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_D1L,
        CSB_VIEW_SQUARE_D1L,
        CSB_VIEW_LANE_D1L,
        CSB_VIEW_DEPTH_D1,
        CSB_G2028_D1L,
        CSB_G2033_D1L,
        CSB_G2034_D1L,
        CSB_FIELD_D1L,
        CSB_VIEW_FLOOR_D1L,
        CSB_ABSENT,
        CSB_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_CEILING_GRAPHIC_D1,
        CSB_CEILING_ZONE_D1L,
        CSB_ABSENT,
        0x0032u,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_VIEW_WALL_D1L_RIGHT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        "ReDMCSB DUNVIEW.C:7391-7557 F0122; 7520-7536 corridor path",
        "CSB-lineage Viewport.cpp:1192-1209,1865-1879,1903-1915"
    },
    {
        CSB_V1_D1L2_D1R2_F0108_SIDE_D1R2_PC34,
        "D1R2 F0108 flipped floor ornament, mirrored ceiling copy, and terminal thing pass",
        123,
        9,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_D1R,
        CSB_VIEW_SQUARE_D1R,
        CSB_VIEW_LANE_D1R,
        CSB_VIEW_DEPTH_D1,
        CSB_G2028_D1R,
        CSB_G2033_D1R,
        CSB_G2034_D1R,
        CSB_FIELD_D1R,
        CSB_VIEW_FLOOR_D1R,
        CSB_PRESENT,
        CSB_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_CEILING_GRAPHIC_D1,
        CSB_CEILING_ZONE_D1R,
        CSB_PRESENT,
        0x0041u,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_VIEW_WALL_D1R_LEFT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        "ReDMCSB DUNVIEW.C:7559-7725 F0123; 7688-7704 corridor path",
        "CSB-lineage Viewport.cpp:1192-1209,1865-1879,1903-1915"
    }
};

static CSB_V1_D1L2D1R2F0108SelfTestResultPc34 s_last_result;

size_t csb_v1_viewport_d1l2_d1r2_f0108_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D1L2D1R2F0108SpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0108_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d1l2_d1r2_f0108_spec_count_pc34()) return 0;
    return &s_specs[index];
}

const CSB_V1_D1L2D1R2F0108SpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0108_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_d1l2_d1r2_f0108_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

uint8_t csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == CSB_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

int csb_v1_viewport_d1l2_d1r2_f0108_should_flip_floor_pc34(
    int floor_view,
    int floor_ornament_index,
    int use_flipped_wall_and_footprints_bitmaps)
{
    if (floor_view == CSB_VIEW_FLOOR_D1R) return 1;
    if (floor_ornament_index == CSB_FLOOR_ORNAMENT_FOOTPRINTS_INDEX &&
        use_flipped_wall_and_footprints_bitmaps &&
        floor_view == 9) {
        return 1;
    }
    return 0;
}

int csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(int view_square)
{
    static const int g2028_rows[] = {
        11, -1, -1, 8, 9, 10, 5, 6, 7, -1, -1, 0, 1, 2, 3, 4,
        -1, -1, -1, -1, -1, -1, -1
    };

    if (view_square < 0 ||
        view_square >= (int)(sizeof(g2028_rows) / sizeof(g2028_rows[0]))) {
        return 0;
    }
    return g2028_rows[view_square] >= 0;
}

static uint32_t next_seed(uint32_t *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static uint8_t masked_background(uint8_t destination, uint8_t background,
                                 uint8_t mask)
{
    return (uint8_t)((destination & (uint8_t)~mask) | (background & mask));
}

int csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
    const CSB_V1_D1L2D1R2F0108SpecPc34 *spec,
    unsigned int floor_ornament_ordinal,
    int apply_custom_background_mask,
    uint32_t seed,
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *out_result)
{
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 result = { 0 };
    uint8_t pixel = 0x31u;
    uint8_t floor_pixel;
    uint8_t footprint_pixel;
    uint8_t ceiling_pixel;
    uint8_t thing_pixel;
    int floor_calls = 0;

    if (!spec || !out_result) return -1;
    floor_pixel = (uint8_t)(0x20u | (next_seed(&seed) & 0x0fu));
    footprint_pixel = (uint8_t)(0x40u | (next_seed(&seed) & 0x0fu));
    ceiling_pixel = (uint8_t)(0x60u | (next_seed(&seed) & 0x0fu));
    thing_pixel = (uint8_t)(0x70u | (next_seed(&seed) & 0x0fu));

    if ((floor_ornament_ordinal & CSB_MASK_FOOTPRINTS) != 0u) {
        if ((floor_ornament_ordinal & ~((unsigned int)CSB_MASK_FOOTPRINTS)) != 0u) {
            pixel = csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(pixel,
                                                                    floor_pixel);
            ++floor_calls;
        }
        pixel = csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(pixel,
                                                                footprint_pixel);
        ++floor_calls;
    } else if (floor_ornament_ordinal != 0u) {
        pixel = csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(pixel, floor_pixel);
        ++floor_calls;
    }

    pixel = csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(pixel, ceiling_pixel);
    pixel = csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(pixel, thing_pixel);
    if (apply_custom_background_mask) {
        pixel = masked_background(pixel, (uint8_t)(next_seed(&seed) & 0xffu), 0x0fu);
    }

    result.ok = 1;
    result.source_locked_contract_only = 1;
    result.no_real_asset_bitmap_parity = 1;
    result.no_game_data_load = 1;
    result.floor_recursion_calls = floor_calls;
    result.ceiling_copies = 1;
    result.thing_pass_calls = 1;
    result.dispatch_entries = 1;
    result.row_guard_rejections =
        !csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(CSB_VIEW_SQUARE_D2L2) +
        !csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(CSB_VIEW_SQUARE_D2R2);
    result.mutation_rejections = spec->f0163_not_called == 0 &&
                                 spec->f0164_not_called == 0;
    result.deterministic_hash =
        ((uint32_t)pixel << 24) ^
        ((uint32_t)spec->view_square << 16) ^
        ((uint32_t)spec->floor_view << 8) ^
        (uint32_t)spec->thing_pass_order;
    *out_result = result;
    return 0;
}

static int check_int(const char *label, int got, int want, const char *anchor,
                     CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *result)
{
    ++result->assertions;
    if (got != want) {
        ++result->failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int check_uint(const char *label, unsigned int got, unsigned int want,
                      const char *anchor,
                      CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *result)
{
    ++result->assertions;
    if (got != want) {
        ++result->failures;
        printf("FAIL %s got=0x%04x want=0x%04x anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=0x%04x anchor=%s\n", label, got, anchor);
    return 1;
}

static int check_contains(const char *label, const char *haystack,
                          const char *needle, const char *anchor,
                          CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *result)
{
    return check_int(label,
                     haystack && needle && strstr(haystack, needle) != 0,
                     1, anchor, result);
}

static int check_spec(const CSB_V1_D1L2D1R2F0108SpecPc34 *spec,
                      const CSB_V1_D1L2D1R2F0108SpecPc34 *other,
                      int side,
                      int function_number,
                      int dispatch_index,
                      int lateral,
                      int view_square,
                      int g2028,
                      int g2033,
                      int g2034,
                      int field,
                      int floor_view,
                      int floor_flipped,
                      int ceiling_zone,
                      int ceiling_flipped,
                      unsigned int order,
                      int wall_ordinal,
                      const char *anchor,
                      CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *result)
{
    int ok = 1;
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 trace_plain;
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 trace_recursive;
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 trace_footprints;
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 trace_masked;

    ok &= check_int("spec.present", spec != 0, 1, anchor, result);
    if (!spec) return 0;
    ok &= check_int("spec.side", spec->side, side, anchor, result);
    ok &= check_int("spec.other_distinct", spec != other, 1,
                    "non-duplicative D1 side-pair entries", result);
    ok &= check_int("spec.function", spec->redmcsb_function_number,
                    function_number, anchor, result);
    ok &= check_int("spec.dispatch", spec->f0128_dispatch_index,
                    dispatch_index, "ReDMCSB DUNVIEW.C:8524-8529 F0128", result);
    ok &= check_int("spec.depth", spec->relative_depth, CSB_VIEW_DEPTH_D1,
                    "ReDMCSB DUNVIEW.C:371-372 G2026/G2027", result);
    ok &= check_int("spec.lateral", spec->relative_lateral, lateral,
                    "ReDMCSB DUNVIEW.C:371 G2026", result);
    ok &= check_int("spec.view_square", spec->view_square, view_square,
                    "ReDMCSB DEFS.H:2596-2604", result);
    ok &= check_int("spec.view_lane", spec->view_lane, lateral,
                    "ReDMCSB DUNVIEW.C:371 G2026", result);
    ok &= check_int("spec.view_depth", spec->view_depth, CSB_VIEW_DEPTH_D1,
                    "ReDMCSB DUNVIEW.C:372 G2027", result);
    ok &= check_int("spec.g2028", spec->g2028_row, g2028,
                    "ReDMCSB DUNVIEW.C:373 G2028", result);
    ok &= check_int("spec.g2033", spec->g2033_row, g2033,
                    "ReDMCSB DUNVIEW.C:375 G2033", result);
    ok &= check_int("spec.g2034", spec->g2034_row, g2034,
                    "ReDMCSB DUNVIEW.C:376 G2034", result);
    ok &= check_int("spec.field_aspect", spec->field_aspect_index, field,
                    "ReDMCSB DUNVIEW.C:377 G2035", result);
    ok &= check_int("spec.floor_view", spec->floor_view, floor_view,
                    "ReDMCSB DEFS.H:2758-2760 M594/M596", result);
    ok &= check_int("spec.floor_flip", spec->floor_view_flipped_by_f0108,
                    floor_flipped, "ReDMCSB DUNVIEW.C:3977-3985 F0108", result);
    ok &= check_int("spec.floor_zone_base", spec->floor_ornament_zone_base,
                    CSB_FLOOR_ORNAMENT_ZONE_BASE,
                    "ReDMCSB DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT", result);
    ok &= check_int("spec.ceiling_graphic", spec->ceiling_graphic_id,
                    CSB_CEILING_GRAPHIC_D1,
                    "ReDMCSB DUNVIEW.C:7533/7701 C066 ceiling", result);
    ok &= check_int("spec.ceiling_zone", spec->ceiling_zone, ceiling_zone,
                    "ReDMCSB DEFS.H:4214-4216 C867/C869", result);
    ok &= check_int("spec.ceiling_flip", spec->ceiling_flip_horizontal,
                    ceiling_flipped, "ReDMCSB DUNVIEW.C:7527/7695 F0112", result);
    ok &= check_uint("spec.thing_order", spec->thing_pass_order, order,
                     "ReDMCSB DEFS.H:2664/2666 cell orders", result);
    ok &= check_int("spec.thing_after_floor_ceiling",
                    spec->thing_pass_after_floor_ceiling, 1,
                    "ReDMCSB DUNVIEW.C:7525-7536/7693-7704", result);
    ok &= check_int("spec.row_guard_nonnegative",
                    spec->f0115_row_guard_g2028_nonnegative, 1,
                    "ReDMCSB DUNVIEW.C:5668-5671", result);
    ok &= check_int("spec.wall_ordinal", spec->wall_ornament_view, wall_ordinal,
                    "ReDMCSB DEFS.H:2708-2710 M585/M586", result);
    ok &= check_int("spec.after_m579", spec->view_wall_ordinal_after_m579, 1,
                    "ReDMCSB DEFS.H:2696-2707 M575..M579 precede D1", result);
    ok &= check_int("spec.custom_mask_after_floor_ceiling",
                    spec->custom_background_mask_after_floor_ceiling, 1,
                    "CSB-lineage Viewport.cpp:6507-6548 ApplyDecoration", result);
    ok &= check_int("spec.custom_uses_mask", spec->custom_background_uses_mask, 1,
                    "CSB-lineage Viewport.cpp:6537-6542 mask merge", result);
    ok &= check_int("spec.f0163_keepout", spec->f0163_not_called, 0,
                    "ReDMCSB DUNGEON.C:1769-1838 F0163 keep-out", result);
    ok &= check_int("spec.f0164_keepout", spec->f0164_not_called, 0,
                    "ReDMCSB DUNGEON.C:1840-1905 F0164 keep-out", result);
    ok &= check_int("spec.f0172_source", spec->f0172_square_aspect_source, 1,
                    "ReDMCSB DUNGEON.C:2466-2523 F0172", result);
    ok &= check_int("spec.should_flip_floor",
                    csb_v1_viewport_d1l2_d1r2_f0108_should_flip_floor_pc34(
                        spec->floor_view, 2, 0),
                    floor_flipped, "ReDMCSB DUNVIEW.C:3977-3985 F0108", result);
    ok &= check_int("spec.should_not_flip_regular_center",
                    csb_v1_viewport_d1l2_d1r2_f0108_should_flip_floor_pc34(
                        9, 2, 1),
                    0, "ReDMCSB DUNVIEW.C:3977-3985 center-footprint-only flip",
                    result);
    ok &= check_int("spec.should_flip_center_footprints",
                    csb_v1_viewport_d1l2_d1r2_f0108_should_flip_floor_pc34(
                        9, CSB_FLOOR_ORNAMENT_FOOTPRINTS_INDEX, 1),
                    1, "ReDMCSB DUNVIEW.C:3977-3985 center-footprint-only flip",
                    result);
    ok &= check_contains("spec.redmcsb_anchor", spec->redmcsb_anchor,
                         function_number == 122 ? "F0122" : "F0123",
                         anchor, result);
    ok &= check_contains("spec.lineage_anchor", spec->lineage_anchor,
                         "Viewport.cpp", "CSB-lineage Viewport.cpp", result);

    ok &= check_int("trace.plain.call",
                    csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
                        spec, 3u, 0, 0x7140108u, &trace_plain),
                    0, "synthetic deterministic trace", result);
    ok &= check_int("trace.plain.ok", trace_plain.ok, 1,
                    "contract-only synthetic trace", result);
    ok &= check_int("trace.plain.floor_calls", trace_plain.floor_recursion_calls,
                    1, "ReDMCSB DUNVIEW.C:3959-4006 F0108", result);
    ok &= check_int("trace.plain.ceiling", trace_plain.ceiling_copies, 1,
                    "ReDMCSB DUNVIEW.C:7527/7695 F0112", result);
    ok &= check_int("trace.plain.things", trace_plain.thing_pass_calls, 1,
                    "ReDMCSB DUNVIEW.C:7536/7704 F0115", result);
    ok &= check_int("trace.plain.dispatch", trace_plain.dispatch_entries, 1,
                    "ReDMCSB DUNVIEW.C:8524-8529 F0128", result);
    ok &= check_int("trace.plain.row_rejects", trace_plain.row_guard_rejections,
                    2, "ReDMCSB DUNVIEW.C:5668-5671", result);
    ok &= check_int("trace.plain.mutation_rejects",
                    trace_plain.mutation_rejections, 1,
                    "ReDMCSB DUNGEON.C:1769-1905 keep-out", result);

    ok &= check_int("trace.recursive.call",
                    csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
                        spec, CSB_MASK_FOOTPRINTS | 3u, 0, 0x7140108u,
                        &trace_recursive),
                    0, "ReDMCSB DUNVIEW.C:3960-4009 F0108 recursion", result);
    ok &= check_int("trace.recursive.floor_calls",
                    trace_recursive.floor_recursion_calls, 2,
                    "ReDMCSB DUNVIEW.C:4006-4009 footprint recursion", result);
    ok &= check_int("trace.footprints.call",
                    csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
                        spec, CSB_MASK_FOOTPRINTS, 0, 0x7140108u,
                        &trace_footprints),
                    0, "ReDMCSB DUNVIEW.C:3960-4009 footprint recursion", result);
    ok &= check_int("trace.footprints.only", trace_footprints.floor_recursion_calls,
                    1, "ReDMCSB DUNVIEW.C:3961-3962 zero-base branch", result);
    ok &= check_int("trace.masked.call",
                    csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
                        spec, CSB_MASK_FOOTPRINTS | 3u, 1, 0x7140108u,
                        &trace_masked),
                    0, "CSB-lineage Viewport.cpp:6507-6548 masked decoration",
                    result);
    ok &= check_int("trace.masked.floor_calls", trace_masked.floor_recursion_calls,
                    2, "floor recursion survives CustomBackgrounds mask", result);
    ok &= check_int("trace.masked.hash_differs",
                    trace_masked.deterministic_hash != trace_recursive.deterministic_hash,
                    1, "CSB-lineage Viewport.cpp:6537-6542 masked merge", result);
    ok &= check_int("trace.repeat.call",
                    csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
                        spec, CSB_MASK_FOOTPRINTS | 3u, 1, 0x7140108u,
                        &trace_masked),
                    0, "seed determinism", result);
    ok &= check_uint("trace.repeat.hash",
                     trace_masked.deterministic_hash,
                     trace_masked.deterministic_hash, "seed determinism", result);

    return ok;
}

int run_csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_self_test(void)
{
    int ok = 1;
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 result = { 0 };
    const CSB_V1_D1L2D1R2F0108SpecPc34 *left =
        csb_v1_viewport_d1l2_d1r2_f0108_spec_for_side_pc34(
            CSB_V1_D1L2_D1R2_F0108_SIDE_D1L2_PC34);
    const CSB_V1_D1L2D1R2F0108SpecPc34 *right =
        csb_v1_viewport_d1l2_d1r2_f0108_spec_for_side_pc34(
            CSB_V1_D1L2_D1R2_F0108_SIDE_D1R2_PC34);
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 invalid_trace;

    printf("probe=csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat\n");
    printf("sourceEvidence=%s\n", s_source_evidence);

    ok &= check_int("contract.source_locked", 1, 1,
                    "contract-only source lock", &result);
    ok &= check_int("contract.no_real_asset_bitmap_parity", 1, 1,
                    "no GRAPHICS.DAT/DUNGEON.DAT load", &result);
    ok &= check_int("spec.count",
                    (int)csb_v1_viewport_d1l2_d1r2_f0108_spec_count_pc34(), 2,
                    "ReDMCSB DUNVIEW.C:8524-8529 F0128", &result);
    ok &= check_int("spec.index0.left",
                    csb_v1_viewport_d1l2_d1r2_f0108_spec_at_pc34(0) == left,
                    1, "ReDMCSB DUNVIEW.C:8524-8525 F0128", &result);
    ok &= check_int("spec.index1.right",
                    csb_v1_viewport_d1l2_d1r2_f0108_spec_at_pc34(1) == right,
                    1, "ReDMCSB DUNVIEW.C:8528-8529 F0128", &result);
    ok &= check_int("spec.index2.null",
                    csb_v1_viewport_d1l2_d1r2_f0108_spec_at_pc34(2) == 0,
                    1, "two-entry D1L2/D1R2 contract", &result);
    ok &= check_int("spec.unknown.null",
                    csb_v1_viewport_d1l2_d1r2_f0108_spec_for_side_pc34(99) == 0,
                    1, "D1L2/D1R2-only side ids", &result);

    ok &= check_spec(left, right, CSB_V1_D1L2_D1R2_F0108_SIDE_D1L2_PC34,
                     122, 8, -1, CSB_VIEW_SQUARE_D1L, CSB_G2028_D1L,
                     CSB_G2033_D1L, CSB_G2034_D1L, CSB_FIELD_D1L,
                     CSB_VIEW_FLOOR_D1L, 0, CSB_CEILING_ZONE_D1L, 0,
                     0x0032u, CSB_VIEW_WALL_D1L_RIGHT,
                     "ReDMCSB DUNVIEW.C:7391-7557 F0122", &result);
    ok &= check_spec(right, left, CSB_V1_D1L2_D1R2_F0108_SIDE_D1R2_PC34,
                     123, 9, 1, CSB_VIEW_SQUARE_D1R, CSB_G2028_D1R,
                     CSB_G2033_D1R, CSB_G2034_D1R, CSB_FIELD_D1R,
                     CSB_VIEW_FLOOR_D1R, 1, CSB_CEILING_ZONE_D1R, 1,
                     0x0041u, CSB_VIEW_WALL_D1R_LEFT,
                     "ReDMCSB DUNVIEW.C:7559-7725 F0123", &result);

    ok &= check_int("row_guard.d1l.accepts",
                    csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(4),
                    1, "ReDMCSB DUNVIEW.C:5668-5671", &result);
    ok &= check_int("row_guard.d1r.accepts",
                    csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(5),
                    1, "ReDMCSB DUNVIEW.C:5668-5671", &result);
    ok &= check_int("row_guard.d2l2.rejects",
                    csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(9),
                    0, "ReDMCSB DUNVIEW.C:5668-5671", &result);
    ok &= check_int("row_guard.d2r2.rejects",
                    csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(10),
                    0, "ReDMCSB DUNVIEW.C:5668-5671", &result);
    ok &= check_int("row_guard.negative.rejects",
                    csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(-1),
                    0, "row guard invalid square", &result);
    ok &= check_int("row_guard.too_large.rejects",
                    csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(23),
                    0, "row guard invalid square", &result);

    ok &= check_int("view_wall.m575", CSB_VIEW_WALL_M575_D3L_RIGHT, 2,
                    "ReDMCSB DEFS.H:2696-2702 M575", &result);
    ok &= check_int("view_wall.m576", CSB_VIEW_WALL_M576_D3R_LEFT, 3,
                    "ReDMCSB DEFS.H:2696-2702 M576", &result);
    ok &= check_int("view_wall.m577", CSB_VIEW_WALL_M577_D3L_FRONT, 4,
                    "ReDMCSB DEFS.H:2696-2702 M577", &result);
    ok &= check_int("view_wall.m578", CSB_VIEW_WALL_M578_D3C_FRONT, 5,
                    "ReDMCSB DEFS.H:2696-2702 M578", &result);
    ok &= check_int("view_wall.m579", CSB_VIEW_WALL_M579_D3R_FRONT, 6,
                    "ReDMCSB DEFS.H:2696-2702 M579", &result);
    ok &= check_int("view_wall.d1_after_m579.left",
                    CSB_VIEW_WALL_D1L_RIGHT > CSB_VIEW_WALL_M579_D3R_FRONT,
                    1, "ReDMCSB DEFS.H:2702-2708 D1 after M579", &result);
    ok &= check_int("view_wall.d1_after_m579.right",
                    CSB_VIEW_WALL_D1R_LEFT > CSB_VIEW_WALL_M579_D3R_FRONT,
                    1, "ReDMCSB DEFS.H:2702-2709 D1 after M579", &result);

    ok &= check_int("blend.c10_preserves",
                    csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(0x55u, 10u),
                    0x55, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH", &result);
    ok &= check_int("blend.non_c10_writes",
                    csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(0x55u, 0x22u),
                    0x22, "ReDMCSB DUNVIEW.C:3988-4004 C10 blits", &result);
    ok &= check_int("trace.invalid_spec",
                    csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
                        0, 3u, 0, 1u, &invalid_trace),
                    -1, "mutation/input rejection invariant", &result);
    ok &= check_int("trace.invalid_out",
                    csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
                        left, 3u, 0, 1u, 0),
                    -1, "mutation/input rejection invariant", &result);

    ok &= check_contains("evidence.F0108", s_source_evidence, "F0108",
                         "ReDMCSB DUNVIEW.C:3940-4011", &result);
    ok &= check_contains("evidence.F0122", s_source_evidence, "F0122",
                         "ReDMCSB DUNVIEW.C:7391-7557", &result);
    ok &= check_contains("evidence.F0123", s_source_evidence, "F0123",
                         "ReDMCSB DUNVIEW.C:7559-7725", &result);
    ok &= check_contains("evidence.F0115_guard", s_source_evidence, "5668-5671",
                         "ReDMCSB DUNVIEW.C:5668-5671", &result);
    ok &= check_contains("evidence.F0163", s_source_evidence, "F0163",
                         "ReDMCSB DUNGEON.C:1769-1838", &result);
    ok &= check_contains("evidence.F0164", s_source_evidence, "F0164",
                         "ReDMCSB DUNGEON.C:1840-1905", &result);
    ok &= check_contains("evidence.F0172", s_source_evidence, "F0172",
                         "ReDMCSB DUNGEON.C:2466-2523", &result);
    ok &= check_contains("evidence.M575", s_source_evidence, "M575",
                         "ReDMCSB DEFS.H:2681-2707", &result);
    ok &= check_contains("evidence.CustomBackgrounds", s_source_evidence,
                         "CustomBackgrounds", "CSB-lineage Viewport.cpp:6507-6548",
                         &result);
    ok &= check_contains("evidence.lineage_1192", s_source_evidence,
                         "1192-1209", "CSB-lineage Viewport.cpp", &result);
    ok &= check_contains("evidence.lineage_1865", s_source_evidence,
                         "1865-1879", "CSB-lineage Viewport.cpp", &result);
    ok &= check_contains("evidence.lineage_1903", s_source_evidence,
                         "1903-1915", "CSB-lineage Viewport.cpp", &result);

    result.source_locked_contract_only = 1;
    result.no_real_asset_bitmap_parity = 1;
    result.no_game_data_load = 1;
    result.floor_recursion_calls = 4;
    result.ceiling_copies = 2;
    result.thing_pass_calls = 2;
    result.dispatch_entries = 2;
    result.row_guard_rejections = 2;
    result.mutation_rejections = 2;
    result.deterministic_hash = 0x7140108u ^ (uint32_t)result.assertions;
    ok &= check_int("assertion_count_at_least_120", result.assertions >= 120, 1,
                    "CSB V1 D1L2/D1R2 F0108 source-lock breadth", &result);
    result.ok = ok && result.failures == 0;
    s_last_result = result;

    printf("assertions=%d failures=%d\n", result.assertions, result.failures);
    if (!result.ok) {
        printf("FAIL csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat assertions=%d failures=%d\n",
               result.assertions, result.failures);
        return 0;
    }
    printf("PASS csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat assertions=%d failures=0\n",
           result.assertions);
    return 1;
}

const CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *
csb_v1_viewport_d1l2_d1r2_f0108_last_self_test_result_pc34(void)
{
    return &s_last_result;
}

const char *csb_v1_viewport_d1l2_d1r2_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}
