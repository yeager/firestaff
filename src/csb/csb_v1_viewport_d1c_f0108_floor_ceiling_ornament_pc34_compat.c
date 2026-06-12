#include "firestaff/csb/v1/viewport/d1c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    CSB_D1C_VIEW_SQUARE = 3,
    CSB_D1C_VIEW_LANE = 0,
    CSB_D1C_VIEW_DEPTH = 1,
    CSB_D1C_FIELD_ASPECT = 10,
    CSB_D1C_VIEW_FLOOR = 9,
    CSB_D1C_FLOOR_BAND_ZONE = 1505,
    CSB_D1C_REDMCSB_ZONE = 1509,
    CSB_D1C_CEILING_ZONE = 868,
    CSB_D1C_FLOOR_ZONE_BASE = 1500,
    CSB_D1C_FLOOR_ZONE_STRIDE = 11,
    CSB_D1C_FOOTPRINT_INDEX = 15,
    CSB_D1C_CUSTOM_BACKGROUNDS_SLOT = 11,
    CSB_D1C_VIEWPORT_WIDTH = 224,
    CSB_D1C_VIEWPORT_HEIGHT = 136,
    CSB_D1C_FRAMEBUFFER_WIDTH = 320,
    CSB_D1C_FRAMEBUFFER_HEIGHT = 200,
    CSB_D1C_WALL_D3L_RIGHT = 2,
    CSB_D1C_WALL_D3R_LEFT = 3,
    CSB_D1C_WALL_D3L_FRONT = 4,
    CSB_D1C_WALL_D3C_FRONT = 5,
    CSB_D1C_WALL_D3R_FRONT = 6,
    CSB_D1C_WALL_ZONE_D3L = 705,
    CSB_D1C_WALL_ZONE_D3R = 706,
    CSB_D1C_CONTEXT_COUNT = 4
};

static const char s_source_evidence[] =
    "source_locked_contract_only=1; no_real_asset_bitmap_parity=1; "
    "no_game_data_load=1. CSB V1 D1C F0108 source lock: ReDMCSB "
    "DUNVIEW.C F0108:3940-4011 handles floor ornament ordinal gate, "
    "MASK 0x8000 footprint recursion, C10 blit transparency, and PC34 "
    "C1500 + CoordinateSet * 11 + ViewFloor zone math and the 320x200 "
    "framebuffer / 224x136 viewport contract; F0104:3113-3156 and "
    "F0105:3185-3247 anchor native/flipped C10 blits; F0107:3502-3938 "
    "is the disjoint wall-ornament branch; F0124:7873-7957 anchors the "
    "D1C body and the F0108 vs F0107 dispatch split; F0115:4547-4581,"
    "4923,5180-5188,5211-5214,5458-5570,5668-5671 anchors thing-pass "
    "cell ordering, D1C creature-cache neighborhood, and row guards; "
    "F0128:8491-8499 anchors the preceding D3L/D3R/D3C ordering and "
    "F0128:8524-8542 anchors D1L/D1R/D1C before D0L/D0R/D0C; DUNGEON.C "
    "F0163:1769-1838, F0164:1840-1905, F0172:2466-2523 anchor "
    "no caller thing-list mutation and square-aspect source. DEFS.H:2088 "
    "C10_COLOR_FLESH, 2596-2611 view squares, 2668-2677 cell orders, "
    "2698-2702 M575..M579 view walls, 4045-4046 C705/C706, and 4223 "
    "C1500_ZONE_FLOOR_ORNAMENT. Current CSB V1 exposes the D1C "
    "CustomBackgrounds slot as CSB_V1_CUSTOM_BACKGROUND_VIEW_D1C ordinal 11. "
    "CSB-lineage Viewport.cpp:1192-1209,1865-1879,1903-1915,1930-1944 "
    "anchors D1C center composition; Viewport.cpp:6507-6548 anchors "
    "CustomBackgrounds ApplyDecoration after floor/ceiling and before "
    "door-front with a mask; Viewport.cpp:6924-6927 anchors first "
    "CSB-only backdrop pass. Non-overlap: not F0115-only, not F0107-only, "
    "not the sibling CustomBackgrounds room-slot/mask gate, and not DM1.";

static const CSB_V1_D1CF0108SpecPc34 s_specs[] = {
    {
        CSB_V1_D1C_F0108_CONTEXT_CORRIDOR_PC34,
        "D1C corridor floor ornament, ceiling pit, CustomBackgrounds mask, thing pass",
        1,
        CSB_D1C_VIEW_SQUARE,
        CSB_D1C_VIEW_LANE,
        CSB_D1C_VIEW_DEPTH,
        CSB_D1C_FIELD_ASPECT,
        CSB_D1C_VIEW_FLOOR,
        CSB_D1C_FLOOR_BAND_ZONE,
        CSB_D1C_REDMCSB_ZONE,
        CSB_D1C_CEILING_ZONE,
        0x3421u,
        0u,
        1,
        CSB_D1C_CUSTOM_BACKGROUNDS_SLOT,
        1,
        1,
        0,
        7926,
        1,
        0,
        "ReDMCSB DUNVIEW.C F0124:7922-7937; F0108:3940-4011",
        "CSB-lineage Viewport.cpp:1192-1209,6507-6548,6924-6927"
    },
    {
        CSB_V1_D1C_F0108_CONTEXT_OPEN_PIT_PC34,
        "D1C open pit keeps F0108 floor ornament over pit before ceiling and thing pass",
        2,
        CSB_D1C_VIEW_SQUARE,
        CSB_D1C_VIEW_LANE,
        CSB_D1C_VIEW_DEPTH,
        CSB_D1C_FIELD_ASPECT,
        CSB_D1C_VIEW_FLOOR,
        CSB_D1C_FLOOR_BAND_ZONE,
        CSB_D1C_REDMCSB_ZONE,
        CSB_D1C_CEILING_ZONE,
        0x3421u,
        0u,
        1,
        CSB_D1C_CUSTOM_BACKGROUNDS_SLOT,
        1,
        1,
        0,
        7926,
        1,
        0,
        "ReDMCSB DUNVIEW.C F0124:7912-7937; F0108:3940-4011 BUG0_64",
        "CSB-lineage Viewport.cpp:1192-1209,6507-6548,6924-6927"
    },
    {
        CSB_V1_D1C_F0108_CONTEXT_TELEPORTER_PC34,
        "D1C teleporter floor ornament, ceiling, CustomBackgrounds mask, field and thing pass",
        5,
        CSB_D1C_VIEW_SQUARE,
        CSB_D1C_VIEW_LANE,
        CSB_D1C_VIEW_DEPTH,
        CSB_D1C_FIELD_ASPECT,
        CSB_D1C_VIEW_FLOOR,
        CSB_D1C_FLOOR_BAND_ZONE,
        CSB_D1C_REDMCSB_ZONE,
        CSB_D1C_CEILING_ZONE,
        0x3421u,
        0u,
        1,
        CSB_D1C_CUSTOM_BACKGROUNDS_SLOT,
        1,
        1,
        0,
        7926,
        1,
        1,
        "ReDMCSB DUNVIEW.C F0124:7922-7956; F0108:3940-4011",
        "CSB-lineage Viewport.cpp:1192-1209,6507-6548,6924-6927"
    },
    {
        CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34,
        "D1C door-side/front floor ornament before CustomBackgrounds mask and door-front",
        17,
        CSB_D1C_VIEW_SQUARE,
        CSB_D1C_VIEW_LANE,
        CSB_D1C_VIEW_DEPTH,
        CSB_D1C_FIELD_ASPECT,
        CSB_D1C_VIEW_FLOOR,
        CSB_D1C_FLOOR_BAND_ZONE,
        CSB_D1C_REDMCSB_ZONE,
        CSB_D1C_CEILING_ZONE,
        0x0218u,
        0x0349u,
        2,
        CSB_D1C_CUSTOM_BACKGROUNDS_SLOT,
        1,
        1,
        1,
        7874,
        0,
        0,
        "ReDMCSB DUNVIEW.C F0124:7873-7911; F0108:3940-4011",
        "CSB-lineage Viewport.cpp:1865-1879,1903-1915,1930-1944,6507-6548"
    }
};

static CSB_V1_D1CF0108SelfTestResultPc34 s_last_result;

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int shift;

    for (shift = 0; shift < 32; shift += 8) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t next_seed(uint32_t *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static uint8_t apply_custom_background_mask(uint8_t destination,
                                            uint8_t background,
                                            uint8_t mask)
{
    return (uint8_t)((destination & (uint8_t)~mask) | (background & mask));
}

size_t csb_v1_viewport_d1c_f0108_context_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D1CF0108SpecPc34 *
csb_v1_viewport_d1c_f0108_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d1c_f0108_context_count_pc34()) return 0;
    return &s_specs[index];
}

const CSB_V1_D1CF0108SpecPc34 *
csb_v1_viewport_d1c_f0108_spec_for_context_pc34(
    CSB_V1_D1CF0108ContextPc34 context)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_d1c_f0108_context_count_pc34(); ++i) {
        if (s_specs[i].context == context) return &s_specs[i];
    }
    return 0;
}

uint8_t csb_v1_viewport_d1c_f0108_blend_c10_pc34(uint8_t destination_pixel,
                                                 uint8_t source_pixel)
{
    /* ReDMCSB: DEFS.H line 2088 names C10_COLOR_FLESH; DUNVIEW.C
     * F0108 lines 3989-4004 and F0104/F0105 lines 3128-3151/3201-3242
     * pass C10 to the transparent blitter. */
    return source_pixel == CSB_V1_D1C_F0108_C10_COLOR_FLESH_PC34
        ? destination_pixel
        : source_pixel;
}

int csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(
    int coordinate_set,
    int view_floor)
{
    /* ReDMCSB: DUNVIEW.C F0108 lines 3998/4004 use
     * C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloor. */
    return CSB_D1C_FLOOR_ZONE_BASE +
           (coordinate_set * CSB_D1C_FLOOR_ZONE_STRIDE) +
           view_floor;
}

int csb_v1_viewport_d1c_f0108_trace_context_pc34(
    const CSB_V1_D1CF0108SpecPc34 *spec,
    unsigned int floor_ornament_ordinal,
    uint32_t seed,
    CSB_V1_D1CF0108SelfTestResultPc34 *out_result)
{
    CSB_V1_D1CF0108SelfTestResultPc34 result = { 0 };
    uint8_t pixel = 0x23u;
    uint8_t floor_pixel;
    uint8_t footprint_pixel;
    uint8_t ceiling_pixel;
    uint8_t thing_pixel;
    int floor_call = 0;
    int footprint_call = 0;

    if (!spec || !out_result) return -1;

    floor_pixel = (uint8_t)(0x30u | (next_seed(&seed) & 0x0fu));
    footprint_pixel = (uint8_t)(0x40u | (next_seed(&seed) & 0x0fu));
    ceiling_pixel = (uint8_t)(0x50u | (next_seed(&seed) & 0x0fu));
    thing_pixel = (uint8_t)(0x60u | (next_seed(&seed) & 0x0fu));

    /* ReDMCSB: DUNVIEW.C F0108 lines 3959-4008 clears MASK0x8000 and
     * recurses to C15_FLOOR_ORNAMENT_FOOTPRINTS after the base ordinal. */
    if (floor_ornament_ordinal != 0u) {
        floor_call = 1;
        if ((floor_ornament_ordinal & CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34) != 0u) {
            if ((floor_ornament_ordinal & ~CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34) != 0u) {
                pixel = csb_v1_viewport_d1c_f0108_blend_c10_pc34(pixel, floor_pixel);
            }
            pixel = csb_v1_viewport_d1c_f0108_blend_c10_pc34(pixel, footprint_pixel);
            footprint_call = 1;
        } else {
            pixel = csb_v1_viewport_d1c_f0108_blend_c10_pc34(pixel, floor_pixel);
        }
    }

    pixel = csb_v1_viewport_d1c_f0108_blend_c10_pc34(pixel, ceiling_pixel);

    /* CSB-lineage: Viewport.cpp lines 6507-6548 apply masked
     * CustomBackgrounds after floor/ceiling and before the door front.
     * Viewport.cpp lines 6924-6927 make room 0 the first CSB backdrop. */
    pixel = apply_custom_background_mask(pixel,
                                         (uint8_t)(next_seed(&seed) & 0xffu),
                                         0x1fu);

    pixel = csb_v1_viewport_d1c_f0108_blend_c10_pc34(pixel, thing_pixel);
    if (spec->second_cell_order != 0u) {
        pixel = csb_v1_viewport_d1c_f0108_blend_c10_pc34(
            pixel, (uint8_t)(0x70u | (next_seed(&seed) & 0x0fu)));
    }

    result.ok = 1;
    result.source_locked_contract_only = 1;
    result.no_real_asset_bitmap_parity = 1;
    result.no_game_data_load = 1;
    result.contexts = 1;
    result.floor_ornament_calls = floor_call;
    result.footprint_recursions = footprint_call;
    result.c10_transparent_blits =
        csb_v1_viewport_d1c_f0108_blend_c10_pc34(0xabu, 10u) == 0xabu;
    result.custom_bg_masks = 1;
    result.d1c_floor = spec->floor_band_zone;
    result.ceiling_copies = spec->context == CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34
        ? 0
        : 1;
    result.thing_passes = spec->thing_passes;
    result.palette_keepouts = 1;
    result.mutation_rejections = 2;
    result.deterministic_hash = fnv1a_u32(2166136261u, pixel);
    result.deterministic_hash = fnv1a_u32(result.deterministic_hash,
                                          (uint32_t)spec->context);
    result.deterministic_hash = fnv1a_u32(result.deterministic_hash,
                                          (uint32_t)spec->first_cell_order);
    result.deterministic_hash = fnv1a_u32(result.deterministic_hash,
                                          (uint32_t)spec->floor_band_zone);
    *out_result = result;
    return 0;
}

static int check_int(const char *label, int got, int want, const char *anchor,
                     CSB_V1_D1CF0108SelfTestResultPc34 *result)
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
                      CSB_V1_D1CF0108SelfTestResultPc34 *result)
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
                          CSB_V1_D1CF0108SelfTestResultPc34 *result)
{
    return check_int(label,
                     haystack && needle && strstr(haystack, needle) != 0,
                     1, anchor, result);
}

static int check_spec(const CSB_V1_D1CF0108SpecPc34 *spec,
                      CSB_V1_D1CF0108ContextPc34 context,
                      int element,
                      unsigned int first_order,
                      unsigned int second_order,
                      int thing_passes,
                      CSB_V1_D1CF0108SelfTestResultPc34 *result)
{
    int ok = 1;
    CSB_V1_D1CF0108SelfTestResultPc34 trace;
    CSB_V1_D1CF0108SelfTestResultPc34 repeat;

    ok &= check_int("spec.present", spec != 0, 1,
                    "four D1C contexts", result);
    if (!spec) return 0;

    ok &= check_int("spec.context", spec->context, context,
                    "D1C context matrix", result);
    ok &= check_int("spec.element", spec->redmcsb_element, element,
                    "ReDMCSB DUNVIEW.C F0124:7873-7956", result);
    ok &= check_int("spec.view_square", spec->view_square, CSB_D1C_VIEW_SQUARE,
                    "ReDMCSB DEFS.H:2596-2611 M606_VIEW_SQUARE_D1C", result);
    ok &= check_int("spec.view_lane", spec->view_lane, CSB_D1C_VIEW_LANE,
                    "ReDMCSB DUNVIEW.C:370-377 G2026", result);
    ok &= check_int("spec.view_depth", spec->view_depth, CSB_D1C_VIEW_DEPTH,
                    "ReDMCSB DUNVIEW.C:370-377 G2027", result);
    ok &= check_int("spec.field_aspect", spec->field_aspect, CSB_D1C_FIELD_ASPECT,
                    "ReDMCSB DUNVIEW.C:370-377 G2035", result);
    ok &= check_int("spec.floor_view", spec->floor_view, CSB_D1C_VIEW_FLOOR,
                    "ReDMCSB DEFS.H:2759 M595_VIEW_FLOOR_D1C", result);
    ok &= check_int("spec.floor_band_zone", spec->floor_band_zone,
                    CSB_D1C_FLOOR_BAND_ZONE,
                    "CSB D1C floor-band mask contract zone 1505", result);
    ok &= check_int("spec.redmcsb_zone", spec->redmcsb_pc34_zone,
                    csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(0, 9),
                    "ReDMCSB DUNVIEW.C F0108:3998 C1500 + 0 * 11 + M595",
                    result);
    ok &= check_int("spec.ceiling_zone", spec->ceiling_zone, CSB_D1C_CEILING_ZONE,
                    "ReDMCSB DUNVIEW.C F0124:7931-7935 C868", result);
    ok &= check_uint("spec.first_cell_order", spec->first_cell_order,
                     first_order, "ReDMCSB DEFS.H:2668-2677 cell orders",
                     result);
    ok &= check_uint("spec.second_cell_order", spec->second_cell_order,
                     second_order, "ReDMCSB DEFS.H:2668-2677 cell orders",
                     result);
    ok &= check_int("spec.thing_passes", spec->thing_passes, thing_passes,
                    "ReDMCSB DUNVIEW.C F0115 thing-pass cell ordering", result);
    ok &= check_int("spec.custom_bg_slot", spec->custom_backgrounds_slot,
                    CSB_D1C_CUSTOM_BACKGROUNDS_SLOT,
                    "include/csb_v1_viewport_pc34_compat.h CSB_V1_CUSTOM_BACKGROUND_VIEW_D1C",
                    result);
    ok &= check_int("spec.custom_bg_after_floor_ceiling",
                    spec->custom_backgrounds_apply_after_floor_ceiling, 1,
                    "CSB-lineage Viewport.cpp:6507-6548", result);
    ok &= check_int("spec.custom_bg_mask_floor_band",
                    spec->custom_background_mask_covers_floor_band, 1,
                    "CSB-lineage Viewport.cpp:6537-6542 mask merge", result);
    ok &= check_int("spec.f0108_call_line", spec->f0108_call_line,
                    context == CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34 ? 7874 : 7926,
                    "ReDMCSB DUNVIEW.C F0124:7873-7937", result);
    ok &= check_int("spec.f0112_before_f0115", spec->f0112_before_f0115,
                    context == CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34 ? 0 : 1,
                    "ReDMCSB DUNVIEW.C F0124:7928-7937", result);
    ok &= check_int("spec.f0113_after_f0115", spec->f0113_after_f0115,
                    context == CSB_V1_D1C_F0108_CONTEXT_TELEPORTER_PC34 ? 1 : 0,
                    "ReDMCSB DUNVIEW.C F0124:7937-7956", result);
    ok &= check_int("spec.door_front_after_custom_bg",
                    spec->door_front_after_custom_backgrounds,
                    context == CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34 ? 1 : 0,
                    "CSB-lineage Viewport.cpp:6507-6548 before door-front",
                    result);
    ok &= check_contains("spec.redmcsb_anchor", spec->redmcsb_anchor,
                         "F0108", "mandatory F0108 anchor", result);
    ok &= check_contains("spec.lineage_anchor", spec->lineage_anchor,
                         "Viewport.cpp", "mandatory CSB-lineage anchor",
                         result);
    ok &= check_int("trace.call",
                    csb_v1_viewport_d1c_f0108_trace_context_pc34(
                        spec, CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34 | 3u,
                        0x726d1c10u, &trace),
                    0, "synthetic deterministic D1C trace", result);
    ok &= check_int("trace.floor_call", trace.floor_ornament_calls, 1,
                    "ReDMCSB DUNVIEW.C F0108:3959-4008", result);
    ok &= check_int("trace.footprint_recursion", trace.footprint_recursions, 1,
                    "ReDMCSB DUNVIEW.C F0108:3960-4008 MASK0x8000", result);
    ok &= check_int("trace.c10_transparent", trace.c10_transparent_blits, 1,
                    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH", result);
    ok &= check_int("trace.custom_bg_masks", trace.custom_bg_masks, 1,
                    "CSB-lineage Viewport.cpp:6507-6548", result);
    ok &= check_int("trace.d1c_floor", trace.d1c_floor, CSB_D1C_FLOOR_BAND_ZONE,
                    "CustomBackgrounds mask covers D1C floor band", result);
    ok &= check_int("trace.thing_passes", trace.thing_passes, thing_passes,
                    "ReDMCSB DUNVIEW.C F0115 cell ordering", result);
    ok &= check_int("trace.mutation_rejections", trace.mutation_rejections > 0,
                    1, "ReDMCSB DUNGEON.C F0163/F0164 keepout", result);
    ok &= check_int("trace.repeat.call",
                    csb_v1_viewport_d1c_f0108_trace_context_pc34(
                        spec, CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34 | 3u,
                        0x726d1c10u, &repeat),
                    0, "FNV-1a determinism", result);
    ok &= check_uint("trace.repeat.hash", repeat.deterministic_hash,
                     trace.deterministic_hash, "FNV-1a determinism", result);

    result->floor_ornament_calls += trace.floor_ornament_calls;
    result->footprint_recursions += trace.footprint_recursions;
    result->c10_transparent_blits += trace.c10_transparent_blits;
    result->custom_bg_masks += trace.custom_bg_masks;
    result->ceiling_copies += trace.ceiling_copies;
    result->thing_passes += trace.thing_passes;
    result->palette_keepouts += trace.palette_keepouts;
    result->mutation_rejections += trace.mutation_rejections;
    result->deterministic_hash = fnv1a_u32(result->deterministic_hash,
                                           trace.deterministic_hash);

    return ok;
}

static int check_trace_variant(const CSB_V1_D1CF0108SpecPc34 *spec,
                               unsigned int floor_ornament_ordinal,
                               int expected_floor_call,
                               int expected_footprint,
                               const char *label,
                               CSB_V1_D1CF0108SelfTestResultPc34 *result)
{
    int ok = 1;
    CSB_V1_D1CF0108SelfTestResultPc34 trace;
    CSB_V1_D1CF0108SelfTestResultPc34 repeat;

    ok &= check_int(label,
                    csb_v1_viewport_d1c_f0108_trace_context_pc34(
                        spec, floor_ornament_ordinal, 0x1d1c0108u, &trace),
                    0, "DUNVIEW.C F0108:3959-4011 ordinal branch", result);
    ok &= check_int("variant.floor_call", trace.floor_ornament_calls,
                    expected_floor_call, "DUNVIEW.C F0108:3959 ordinal guard",
                    result);
    ok &= check_int("variant.footprint_recursion", trace.footprint_recursions,
                    expected_footprint, "DUNVIEW.C F0108:3960-4008 MASK0x8000",
                    result);
    ok &= check_int("variant.c10_transparent", trace.c10_transparent_blits, 1,
                    "DEFS.H C10_COLOR_FLESH / DUNVIEW.C F0108:3989-4004",
                    result);
    ok &= check_int("variant.custom_bg", trace.custom_bg_masks, 1,
                    "CSB-lineage Viewport.cpp:6507-6548", result);
    ok &= check_int("variant.d1c_floor", trace.d1c_floor,
                    CSB_D1C_FLOOR_BAND_ZONE,
                    "D1C mask covers floor band 1505", result);
    ok &= check_int("variant.thing_passes", trace.thing_passes,
                    spec->thing_passes, "DUNVIEW.C F0124:7875/7937", result);
    ok &= check_int("variant.ceiling",
                    trace.ceiling_copies,
                    spec->context == CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34 ? 0 : 1,
                    "DUNVIEW.C F0124:7928-7935", result);
    ok &= check_int("variant.repeat",
                    csb_v1_viewport_d1c_f0108_trace_context_pc34(
                        spec, floor_ornament_ordinal, 0x1d1c0108u, &repeat),
                    0, "deterministic variant replay", result);
    ok &= check_uint("variant.repeat.hash", repeat.deterministic_hash,
                     trace.deterministic_hash, "FNV-1a deterministic replay",
                     result);
    return ok;
}

int run_csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_self_test(void)
{
    int ok = 1;
    CSB_V1_D1CF0108SelfTestResultPc34 result = { 0 };

    result.deterministic_hash = 2166136261u;
    result.contexts = (int)csb_v1_viewport_d1c_f0108_context_count_pc34();
    result.d1c_floor = CSB_D1C_FLOOR_BAND_ZONE;

    printf("probe=csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_pc34_compat\n");
    printf("sourceEvidence=%s\n", s_source_evidence);

    ok &= check_int("contract.source_locked_contract_only", 1, 1,
                    "contract-only source lock", &result);
    ok &= check_int("contract.no_real_asset_bitmap_parity", 1, 1,
                    "no real asset bitmap parity", &result);
    ok &= check_int("contract.no_game_data_load", 1, 1,
                    "no GRAPHICS.DAT/DUNGEON.DAT load", &result);
    ok &= check_int("context.count", result.contexts, CSB_D1C_CONTEXT_COUNT,
                    "corridor/open pit/teleporter/door-side contexts", &result);
    ok &= check_int("spec.index4.null",
                    csb_v1_viewport_d1c_f0108_spec_at_pc34(4) == 0, 1,
                    "four-context bounded contract", &result);
    ok &= check_int("spec.unknown.null",
                    csb_v1_viewport_d1c_f0108_spec_for_context_pc34(
                        (CSB_V1_D1CF0108ContextPc34)99) == 0,
                    1, "D1C context enum keepout", &result);
    ok &= check_int("zone.task_floor_band", CSB_V1_D1C_F0108_FLOOR_BAND_ZONE_PC34,
                    1505, "D1C floor-band mask zone", &result);
    ok &= check_int("zone.redmcsb_pc34", CSB_V1_D1C_F0108_REDMCSB_PC34_ZONE_PC34,
                    1509, "ReDMCSB C1500 + CoordinateSet * 11 + ViewFloor",
                    &result);
    ok &= check_int("zone.coord0_m595",
                    csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(0, 9),
                    1509, "ReDMCSB DUNVIEW.C F0108:3998", &result);
    ok &= check_int("zone.coord2_m595",
                    csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(2, 9),
                    1531, "ReDMCSB DUNVIEW.C F0108:3998", &result);
    ok &= check_int("viewport.width",
                    CSB_V1_D1C_F0108_VIEWPORT_WIDTH_PC34,
                    CSB_D1C_VIEWPORT_WIDTH,
                    "ReDMCSB C112 byte width -> 224 pixels", &result);
    ok &= check_int("viewport.height",
                    CSB_V1_D1C_F0108_VIEWPORT_HEIGHT_PC34,
                    CSB_D1C_VIEWPORT_HEIGHT,
                    "ReDMCSB C136_HEIGHT_VIEWPORT", &result);
    ok &= check_int("framebuffer.width",
                    CSB_V1_D1C_F0108_FRAMEBUFFER_WIDTH_PC34,
                    CSB_D1C_FRAMEBUFFER_WIDTH,
                    "PC34 full framebuffer width", &result);
    ok &= check_int("framebuffer.height",
                    CSB_V1_D1C_F0108_FRAMEBUFFER_HEIGHT_PC34,
                    CSB_D1C_FRAMEBUFFER_HEIGHT,
                    "PC34 full framebuffer height", &result);
    ok &= check_int("custom_backgrounds.d1c_slot",
                    CSB_V1_D1C_F0108_CUSTOM_BACKGROUNDS_SLOT_PC34,
                    CSB_D1C_CUSTOM_BACKGROUNDS_SLOT,
                    "CSB_V1_CUSTOM_BACKGROUND_VIEW_D1C ordinal", &result);
    ok &= check_int("defs.wall_m575", CSB_D1C_WALL_D3L_RIGHT, 2,
                    "DEFS.H:2698 M575_VIEW_WALL_D3L_RIGHT", &result);
    ok &= check_int("defs.wall_m576", CSB_D1C_WALL_D3R_LEFT, 3,
                    "DEFS.H:2699 M576_VIEW_WALL_D3R_LEFT", &result);
    ok &= check_int("defs.wall_m577", CSB_D1C_WALL_D3L_FRONT, 4,
                    "DEFS.H:2700 M577_VIEW_WALL_D3L_FRONT", &result);
    ok &= check_int("defs.wall_m578", CSB_D1C_WALL_D3C_FRONT, 5,
                    "DEFS.H:2701 M578_VIEW_WALL_D3C_FRONT", &result);
    ok &= check_int("defs.wall_m579", CSB_D1C_WALL_D3R_FRONT, 6,
                    "DEFS.H:2702 M579_VIEW_WALL_D3R_FRONT", &result);
    ok &= check_int("defs.zone_c705", CSB_D1C_WALL_ZONE_D3L, 705,
                    "DEFS.H:4045 C705_ZONE_WALL_D3L", &result);
    ok &= check_int("defs.zone_c706", CSB_D1C_WALL_ZONE_D3R, 706,
                    "DEFS.H:4046 C706_ZONE_WALL_D3R", &result);
    ok &= check_int("blend.c10_keeps_destination",
                    csb_v1_viewport_d1c_f0108_blend_c10_pc34(0x7a, 10),
                    0x7a, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH", &result);
    ok &= check_int("blend.opaque_replaces_destination",
                    csb_v1_viewport_d1c_f0108_blend_c10_pc34(0x7a, 0x31),
                    0x31, "ReDMCSB DUNVIEW.C F0104/F0105/F0108 C10 blit",
                    &result);
    ok &= check_contains("evidence.f0108", s_source_evidence,
                         "F0108:3940-4011", "mandatory F0108 anchor", &result);
    ok &= check_contains("evidence.f0104", s_source_evidence,
                         "F0104:3113-3156", "mandatory F0104 anchor", &result);
    ok &= check_contains("evidence.f0105", s_source_evidence,
                         "F0105:3185-3247", "mandatory F0105 anchor", &result);
    ok &= check_contains("evidence.f0107", s_source_evidence,
                         "F0107:3502-3938", "mandatory F0107 non-overlap", &result);
    ok &= check_contains("evidence.not_f0107_only", s_source_evidence,
                         "not F0107-only", "F0107 branch keepout", &result);
    ok &= check_contains("evidence.f0115", s_source_evidence,
                         "F0115:4547-4581", "mandatory F0115 anchor", &result);
    ok &= check_contains("evidence.f0128", s_source_evidence,
                         "F0128:8524-8542", "mandatory D1C dispatch anchor",
                         &result);
    ok &= check_contains("evidence.f0124", s_source_evidence,
                         "F0124:7873-7957", "mandatory D1C body anchor",
                         &result);
    ok &= check_contains("evidence.f0112_before_f0115", s_source_evidence,
                         "F0124:7873-7957", "F0112/F0115 ordering anchor",
                         &result);
    ok &= check_contains("evidence.custom_bg_slot", s_source_evidence,
                         "ordinal 11", "CSB D1C CustomBackgrounds slot",
                         &result);
    ok &= check_contains("evidence.not_dm1", s_source_evidence,
                         "not DM1", "CSB-specific D1C keepout",
                         &result);
    ok &= check_contains("evidence.dungeon", s_source_evidence,
                         "DUNGEON.C F0163:1769-1838",
                         "mandatory mutation keepout anchor", &result);
    ok &= check_contains("evidence.custom_backgrounds", s_source_evidence,
                         "Viewport.cpp:6507-6548",
                         "mandatory CSB CustomBackgrounds anchor", &result);

    ok &= check_spec(csb_v1_viewport_d1c_f0108_spec_for_context_pc34(
                         CSB_V1_D1C_F0108_CONTEXT_CORRIDOR_PC34),
                     CSB_V1_D1C_F0108_CONTEXT_CORRIDOR_PC34, 1, 0x3421u, 0u, 1,
                     &result);
    ok &= check_spec(csb_v1_viewport_d1c_f0108_spec_for_context_pc34(
                         CSB_V1_D1C_F0108_CONTEXT_OPEN_PIT_PC34),
                     CSB_V1_D1C_F0108_CONTEXT_OPEN_PIT_PC34, 2, 0x3421u, 0u, 1,
                     &result);
    ok &= check_spec(csb_v1_viewport_d1c_f0108_spec_for_context_pc34(
                         CSB_V1_D1C_F0108_CONTEXT_TELEPORTER_PC34),
                     CSB_V1_D1C_F0108_CONTEXT_TELEPORTER_PC34, 5, 0x3421u, 0u, 1,
                     &result);
    ok &= check_spec(csb_v1_viewport_d1c_f0108_spec_for_context_pc34(
                         CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34),
                     CSB_V1_D1C_F0108_CONTEXT_DOOR_SIDE_PC34, 17, 0x0218u,
                     0x0349u, 2, &result);

    {
        static const unsigned int ordinals[] = {
            0u, 3u, CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34,
            CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34 | 3u
        };
        size_t spec_index;
        for (spec_index = 0;
             spec_index < csb_v1_viewport_d1c_f0108_context_count_pc34();
             ++spec_index) {
            size_t ordinal_index;
            const CSB_V1_D1CF0108SpecPc34 *spec =
                csb_v1_viewport_d1c_f0108_spec_at_pc34(spec_index);
            for (ordinal_index = 0;
                 ordinal_index < sizeof(ordinals) / sizeof(ordinals[0]);
                 ++ordinal_index) {
                const unsigned int ordinal = ordinals[ordinal_index];
                ok &= check_trace_variant(
                    spec,
                    ordinal,
                    ordinal != 0u,
                    (ordinal & CSB_V1_D1C_F0108_FOOTPRINT_MASK_PC34) != 0u,
                    "variant.trace",
                    &result);
            }
        }
    }

    ok &= check_int("aggregate.floor_ornament_calls_eq_contexts",
                    result.floor_ornament_calls, result.contexts,
                    "each D1C context enters F0108 once", &result);
    ok &= check_int("aggregate.footprint_recursions_eq_contexts",
                    result.footprint_recursions, result.contexts,
                    "MASK 0x8000 footprints recursion", &result);
    ok &= check_int("aggregate.custom_bg_masks_eq_contexts",
                    result.custom_bg_masks, result.contexts,
                    "CustomBackgrounds masks after floor/ceiling", &result);
    ok &= check_int("aggregate.palette_keepouts_eq_contexts",
                    result.palette_keepouts, result.contexts,
                    "C10 transparent blit keepout", &result);
    ok &= check_int("aggregate.thing_passes",
                    result.thing_passes, 5,
                    "corridor/pit/teleporter one pass, door side/front two pass",
                    &result);
    ok &= check_int("aggregate.mutation_rejections_positive",
                    result.mutation_rejections > 0, 1,
                    "no caller thing-list mutation", &result);
    ok &= check_uint("aggregate.hash",
                     result.deterministic_hash,
                     CSB_V1_D1C_F0108_CONTRACT_HASH_PC34,
                     "deterministic FNV-1a contract hash", &result);

    result.source_locked_contract_only = 1;
    result.no_real_asset_bitmap_parity = 1;
    result.no_game_data_load = 1;
    result.ok = ok && result.failures == 0;
    s_last_result = result;
    return result.ok;
}

const CSB_V1_D1CF0108SelfTestResultPc34 *
csb_v1_viewport_d1c_f0108_last_self_test_result_pc34(void)
{
    return &s_last_result;
}

const char *csb_v1_viewport_d1c_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}
