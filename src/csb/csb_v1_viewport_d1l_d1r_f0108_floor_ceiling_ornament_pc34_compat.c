#include "firestaff/csb/v1/viewport/d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    CSB_D1L_VIEW_SQUARE = 4,      /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L */
    CSB_D1R_VIEW_SQUARE = 5,      /* ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R */
    CSB_D1_DEPTH = 1,             /* ReDMCSB DUNVIEW.C:372 G2027 depth row */
    CSB_D1L_LANE = -1,            /* ReDMCSB DUNVIEW.C:371 G2026 D1L lane */
    CSB_D1R_LANE = 1,             /* ReDMCSB DUNVIEW.C:371 G2026 D1R lane */
    CSB_D1L_VIEW_FLOOR = 8,       /* ReDMCSB DEFS.H:2758 M594_VIEW_FLOOR_D1L */
    CSB_D1R_VIEW_FLOOR = 10,      /* ReDMCSB DEFS.H:2760 M596_VIEW_FLOOR_D1R */
    CSB_FLOOR_ZONE_BASE = 1500,   /* ReDMCSB DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT */
    CSB_FLOOR_ZONE_STRIDE_PC34 = 11,
    CSB_D1L_CEILING_ZONE = 867,   /* ReDMCSB DEFS.H:4214 C867_ZONE_CEILING_PIT_D1L */
    CSB_D1R_CEILING_ZONE = 869,   /* ReDMCSB DEFS.H:4216 C869_ZONE_CEILING_PIT_D1R */
    CSB_CEILING_GRAPHIC_D1L_PC34 = 66,
    CSB_WALL_KEEP_OUT_ZONE_FIRST = 705, /* ReDMCSB DEFS.H:4045 C705_ZONE_WALL_D3L */
    CSB_WALL_KEEP_OUT_ZONE_LAST = 706,  /* ReDMCSB DEFS.H:4046 C706_ZONE_WALL_D3R */
    CSB_D1L_WALL_ZONE = 713,      /* ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L */
    CSB_D1R_WALL_ZONE = 714,      /* ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R */
    CSB_D1L_WALL_ORNAMENT_VIEW = 12,
    CSB_D1R_WALL_ORNAMENT_VIEW = 13,
    CSB_MUTATION_GUARD_BEFORE = 0x730108u,
    CSB_MUTATION_GUARD_AFTER = 0x108730u
};

static const char s_source_evidence[] =
    "Source-locked contract-only CSB V1 D1L/D1R F0108 gate: "
    "source_locked_contract_only=1; no_real_asset_bitmap_parity=1; "
    "no_game_data_load=1. ReDMCSB DUNVIEW.C F0108:3940-4011 floor "
    "ornament ordinal gate, MASK0x8000 footprints recursion, C10 blit, "
    "D1R horizontal flip, and PC34 C1500 + CoordinateSet * 11 + ViewFloor "
    "zone math; DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 native/"
    "flipped C10 blit; DUNVIEW.C F0107:3502-3938 wall ornament keepout; "
    "DUNVIEW.C F0115:4547-4581,F0115:4923,F0115:5180-5188,"
    "F0115:5211-5214,F0115:5668-5671 thing-pass ordering and row guard; "
    "DUNVIEW.C F0122/F0123 side-pair calls D1L then D1R at F0128:8524-8529 "
    "with F0108:7525 and F0108:7693, F0112 ceiling C867/C869 at "
    "7533/7701, and F0115:7536/F0115:7704; DUNVIEW.C F0124:7873-7957 "
    "D1C center exclusion; "
    "DUNVIEW.C F0127/F0128:8318-8486 and 8536-8541 compose dispatch, "
    "including D1L/D1R before D1C and D0L/D0R after D1C; DUNGEON.C "
    "F0163:1769-1838 and F0164:1840-1905 mutation keepouts; DUNGEON.C "
    "F0172:2466-2523 square-aspect source; DEFS.H:2088 C10_COLOR_FLESH; "
    "DEFS.H:2596-2611 M607/M608 and sibling view-square keepouts; "
    "DEFS.H:2662 and 2668-2677 cell-order constants; DEFS.H:4045-4046 "
    "C705/C706 wall keepout; DEFS.H:4139-4153 stairs-front zone keepout; "
    "DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT. CSB-lineage Viewport.cpp:"
    "1192-1209 near-side open row, Viewport.cpp:1865-1879/1903-1915/"
    "1930-1944 door-facing side/front contrast, Viewport.cpp:6507-6548 "
    "ApplyDecoration mask, and Viewport.cpp:6924-6927 CustomBackgrounds "
    "pre-decoration ordering.";

static const CSB_V1_D1LD1RF0108SpecPc34 s_specs[] = {
    {
        CSB_V1_D1L_D1R_F0108_SIDE_D1L_PC34,
        "D1L floor+ceiling+ornament source-lock contract",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        CSB_D1_DEPTH,
        CSB_D1L_LANE,
        8524,
        10,
        7050,
        CSB_D1L_VIEW_SQUARE,
        CSB_D1L_VIEW_FLOOR,
        CSB_V1_D1L_D1R_F0108_D1L_FLOOR_ZONE_PC34,
        CSB_FLOOR_ZONE_BASE,
        CSB_FLOOR_ZONE_STRIDE_PC34,
        0,
        0,
        CSB_CEILING_GRAPHIC_D1L_PC34,
        CSB_D1L_CEILING_ZONE,
        0,
        0x0032,
        CSB_D1L_VIEW_SQUARE,
        2,
        3,
        CSB_WALL_KEEP_OUT_ZONE_FIRST,
        CSB_WALL_KEEP_OUT_ZONE_LAST,
        CSB_D1L_WALL_ZONE,
        CSB_D1L_WALL_ORNAMENT_VIEW,
        1,
        1,
        "DUNVIEW.C F0122 side call / F0128:8524-8525; F0124:7873-7957 excluded",
        "DUNVIEW.C F0108:3940-4011 / F0122:7525",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,5668-5671",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223",
        "CSB-lineage Viewport.cpp:1192-1209,6507-6548,6924-6927"
    },
    {
        CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34,
        "D1R floor+ceiling+ornament source-lock contract",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        CSB_D1_DEPTH,
        CSB_D1R_LANE,
        8528,
        11,
        7070,
        CSB_D1R_VIEW_SQUARE,
        CSB_D1R_VIEW_FLOOR,
        CSB_V1_D1L_D1R_F0108_D1R_FLOOR_ZONE_PC34,
        CSB_FLOOR_ZONE_BASE,
        CSB_FLOOR_ZONE_STRIDE_PC34,
        0,
        1,
        CSB_CEILING_GRAPHIC_D1L_PC34,
        CSB_D1R_CEILING_ZONE,
        1,
        0x0041,
        CSB_D1R_VIEW_SQUARE,
        1,
        4,
        CSB_WALL_KEEP_OUT_ZONE_FIRST,
        CSB_WALL_KEEP_OUT_ZONE_LAST,
        CSB_D1R_WALL_ZONE,
        CSB_D1R_WALL_ORNAMENT_VIEW,
        1,
        1,
        "DUNVIEW.C F0123 side call / F0128:8528-8529; F0124:7873-7957 excluded",
        "DUNVIEW.C F0108:3940-4011 / F0123:7693",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,5668-5671",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223",
        "CSB-lineage Viewport.cpp:1192-1209,6507-6548,6924-6927"
    }
};

static CSB_V1_D1LD1RF0108SelfTestResultPc34 s_last_self_test;

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t next_lcg(uint32_t *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static int is_supported_context(CSB_V1_D1LD1RF0108ContextPc34 context)
{
    return context == CSB_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34 ||
           context == CSB_V1_D1L_D1R_F0108_CONTEXT_OPEN_PIT_PC34 ||
           context == CSB_V1_D1L_D1R_F0108_CONTEXT_TELEPORTER_PC34 ||
           context == CSB_V1_D1L_D1R_F0108_CONTEXT_DOOR_SIDE_PC34 ||
           context == CSB_V1_D1L_D1R_F0108_CONTEXT_STAIRS_FRONT_PC34;
}

static int f0115_row_guard_accepts(int view_square)
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

size_t csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D1LD1RF0108SpecPc34 *
csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const CSB_V1_D1LD1RF0108SpecPc34 *
csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(int side)
{
    size_t i;

    for (i = 0u; i < csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(); ++i) {
        if ((int)s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool csb_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
    CSB_V1_D1LD1RF0108SidePc34 side,
    CSB_V1_D1LD1RF0108ContextPc34 context,
    CSB_V1_D1LD1RF0108StatePc34 *out)
{
    const CSB_V1_D1LD1RF0108SpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34((int)side);
    if (!spec || !is_supported_context(context)) return false;

    out->side = side;
    out->context = context;
    out->floor_ornament_ordinal =
        side == CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x8006u : 0x8004u;
    out->floor_ornament_native_bitmap_index = 240 + spec->view_floor;
    out->destination_pixel =
        side == CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x32u : 0x31u;
    out->floor_pixel =
        side == CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x52u : 0x51u;
    out->ceiling_pixel =
        side == CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x72u : 0x71u;
    out->custom_background_pixel =
        side == CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0xa6u : 0xa5u;
    out->custom_background_mask = 0x0fu;
    out->thing_pass_pixel = CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34;
    out->seed = side == CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x730102u : 0x730101u;
    out->mutation_guard_before = CSB_MUTATION_GUARD_BEFORE;
    out->mutation_guard_after = CSB_MUTATION_GUARD_AFTER;
    out->source_locked_contract_only = true;
    out->no_real_asset_bitmap_parity = true;
    out->no_game_data_load = true;
    return true;
}

bool csb_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    CSB_V1_D1LD1RF0108OrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = floor_ornament_ordinal;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    out->primary_index = -1;
    out->recursive_footprints_index = -1;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set =
        (floor_ornament_ordinal & CSB_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal & ~CSB_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_index = (int)(out->footprint_flag_set ? cleared : floor_ornament_ordinal) - 1;
        out->metadata_blit_count = 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index = CSB_V1_D1L_D1R_F0108_FOOTPRINT_INDEX_PC34;
        ++out->metadata_blit_count;
    }
    return true;
}

uint8_t csb_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB: DEFS.H line 2088 names C10_COLOR_FLESH; DUNVIEW.C
     * F0108 lines 3989-4004 and F0104/F0105 lines 3128-3151/3201-3242
     * pass C10 to the transparent blitter. */
    return source_pixel == CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

uint8_t csb_v1_viewport_d1l_d1r_f0108_apply_custom_background_mask_pc34(
    uint8_t destination_pixel,
    uint8_t background_pixel,
    uint8_t mask)
{
    /* CSB-lineage: Viewport.cpp lines 6507-6548 ApplyDecoration merges
     * each destination word through a mask before/around cell drawing. */
    return (uint8_t)((destination_pixel & (uint8_t)~mask) |
                     (background_pixel & mask));
}

int csb_v1_viewport_d1l_d1r_f0108_zone_for_coordinate_set_pc34(
    int coordinate_set,
    int view_floor)
{
    /* ReDMCSB: DUNVIEW.C F0108 line 3998 uses
     * C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloor. */
    return CSB_FLOOR_ZONE_BASE + coordinate_set * CSB_FLOOR_ZONE_STRIDE_PC34 +
           view_floor;
}

bool csb_v1_viewport_d1l_d1r_f0108_compose_pc34(
    const CSB_V1_D1LD1RF0108StatePc34 *state,
    CSB_V1_D1LD1RF0108ResultPc34 *out)
{
    const CSB_V1_D1LD1RF0108SpecPc34 *spec;
    CSB_V1_D1LD1RF0108OrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state) return false;
    spec = csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34((int)state->side);
    out->spec = spec;
    if (!spec ||
        !state->source_locked_contract_only ||
        !state->no_real_asset_bitmap_parity ||
        !state->no_game_data_load ||
        !is_supported_context(state->context) ||
        state->attempts_f0107_wall_ornament ||
        state->attempts_f0111_door ||
        state->mutate_thing_list ||
        state->floor_ornament_native_bitmap_index < 0 ||
        state->mutation_guard_before != CSB_MUTATION_GUARD_BEFORE ||
        state->mutation_guard_after != CSB_MUTATION_GUARD_AFTER) {
        out->rejected_non_contract_state = 1;
        if (state->mutate_thing_list ||
            state->mutation_guard_before != CSB_MUTATION_GUARD_BEFORE ||
            state->mutation_guard_after != CSB_MUTATION_GUARD_AFTER) {
            out->mutation_rejections = 1;
        }
        return false;
    }

    if (!csb_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
            state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->ok = 1;
    out->source_locked_contract_only = 1;
    out->no_real_asset_bitmap_parity = 1;
    out->no_game_data_load = 1;
    out->floor_zone = csb_v1_viewport_d1l_d1r_f0108_zone_for_coordinate_set_pc34(
        spec->floor_coordinate_set, spec->view_floor);
    out->floor_primary_index = ordinal.primary_index;
    out->floor_recursive_index = ordinal.recursive_footprints_index;
    out->f0107_keepout_ok = 1;
    out->f0111_keepout_ok = 1;
    out->f0115_row_guard_ok = f0115_row_guard_accepts(spec->view_square);
    out->cell_order_band_ok =
        (spec->thing_pass_order == 0x0032 || spec->thing_pass_order == 0x0041) &&
        spec->thing_pass_first_cell > 0 && spec->thing_pass_second_cell > 0;
    out->palette_keepout_ok =
        spec->wall_keepout_zone_first == CSB_WALL_KEEP_OUT_ZONE_FIRST &&
        spec->wall_keepout_zone_last == CSB_WALL_KEEP_OUT_ZONE_LAST;

    pixel = state->destination_pixel;
    if (ordinal.has_input_ordinal) {
        out->f0108_floor_calls = 1;
        out->f0108_primary_blits = ordinal.primary_draws ? 1 : 0;
        out->f0108_footprint_recursions = ordinal.recursive_footprints_draw ? 1 : 0;
        if (ordinal.primary_draws) {
            pixel = csb_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(pixel, state->floor_pixel);
            if (state->floor_pixel == CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34) {
                ++out->c10_transparent_blits;
            }
        }
    }
    out->after_floor = pixel;

    out->ceiling_copy_calls = 1;
    pixel = csb_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(pixel, state->ceiling_pixel);
    if (state->ceiling_pixel == CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    out->after_ceiling = pixel;

    out->custom_background_masks = spec->custom_background_uses_mask;
    out->custom_background_after_floor_ceiling =
        spec->custom_background_mask_after_floor_ceiling;
    pixel = csb_v1_viewport_d1l_d1r_f0108_apply_custom_background_mask_pc34(
        pixel, state->custom_background_pixel, state->custom_background_mask);
    out->after_custom_background = pixel;

    out->thing_pass_calls = 1;
    pixel = csb_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(pixel, state->thing_pass_pixel);
    if (state->thing_pass_pixel == CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    out->after_thing_pass = pixel;

    out->deterministic_hash = fnv1a_u32(state->seed, (uint32_t)state->context);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, (uint32_t)out->floor_zone);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, out->after_floor);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, out->after_ceiling);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, out->after_custom_background);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, out->after_thing_pass);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, (uint32_t)out->floor_primary_index);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, (uint32_t)out->floor_recursive_index);
    return true;
}

typedef struct {
    int assertions;
    int failures;
} SelfTestCounters;

static void self_check(SelfTestCounters *c, int condition)
{
    ++c->assertions;
    if (!condition) {
        ++c->failures;
        printf("FAIL assertion[%d]\n", c->assertions);
    }
}

static void self_check_eq(SelfTestCounters *c, int got, int want)
{
    ++c->assertions;
    if (got != want) {
        ++c->failures;
        printf("FAIL assertion[%d] got=%d want=%d\n", c->assertions, got, want);
    }
}

static void self_check_contains(SelfTestCounters *c, const char *haystack,
                                const char *needle)
{
    ++c->assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++c->failures;
        printf("FAIL assertion[%d] missing=%s\n",
               c->assertions, needle ? needle : "(null)");
    }
}

int run_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_self_test(void)
{
    static const CSB_V1_D1LD1RF0108ContextPc34 contexts[] = {
        CSB_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34,
        CSB_V1_D1L_D1R_F0108_CONTEXT_OPEN_PIT_PC34,
        CSB_V1_D1L_D1R_F0108_CONTEXT_TELEPORTER_PC34,
        CSB_V1_D1L_D1R_F0108_CONTEXT_DOOR_SIDE_PC34,
        CSB_V1_D1L_D1R_F0108_CONTEXT_STAIRS_FRONT_PC34
    };
    SelfTestCounters c = { 0, 0 };
    uint32_t deterministic_hash = 2166136261u;
    uint32_t seed = 0x7300108u;
    int side_index;
    int context_index;
    int i;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));

    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0108:3940-4011");
    self_check_contains(&c, s_source_evidence, "MASK0x8000");
    self_check_contains(&c, s_source_evidence, "C1500 + CoordinateSet * 11");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0104:3113-3156");
    self_check_contains(&c, s_source_evidence, "F0105:3185-3247");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0107:3502-3938");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0115:4547-4581");
    self_check_contains(&c, s_source_evidence, "F0115:4923");
    self_check_contains(&c, s_source_evidence, "F0115:5180-5188");
    self_check_contains(&c, s_source_evidence, "F0115:5211-5214");
    self_check_contains(&c, s_source_evidence, "F0115:5668-5671");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0122/F0123");
    self_check_contains(&c, s_source_evidence, "F0108:7525");
    self_check_contains(&c, s_source_evidence, "F0115:7536");
    self_check_contains(&c, s_source_evidence, "F0124:7873-7957");
    self_check_contains(&c, s_source_evidence, "F0108:7693");
    self_check_contains(&c, s_source_evidence, "F0115:7704");
    self_check_contains(&c, s_source_evidence, "F0127/F0128:8318-8486");
    self_check_contains(&c, s_source_evidence, "8536-8541");
    self_check_contains(&c, s_source_evidence, "8524-8529");
    self_check_contains(&c, s_source_evidence, "DUNGEON.C F0163:1769-1838");
    self_check_contains(&c, s_source_evidence, "F0164:1840-1905");
    self_check_contains(&c, s_source_evidence, "F0172:2466-2523");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2088");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2596-2611");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2662");
    self_check_contains(&c, s_source_evidence, "2668-2677");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4045-4046 C705/C706");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4139-4153");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4223");
    self_check_contains(&c, s_source_evidence, "Viewport.cpp:1192-1209");
    self_check_contains(&c, s_source_evidence, "Viewport.cpp:1865-1879");
    self_check_contains(&c, s_source_evidence, "1903-1915");
    self_check_contains(&c, s_source_evidence, "1930-1944");
    self_check_contains(&c, s_source_evidence, "Viewport.cpp:6507-6548");
    self_check_contains(&c, s_source_evidence, "Viewport.cpp:6924-6927");
    self_check_contains(&c, s_source_evidence, "CustomBackgrounds");

    self_check_eq(&c, CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34, 10);
    self_check_eq(&c, CSB_V1_D1L_D1R_F0108_C705_ZONE_WALL_D3L_PC34, 705);
    self_check_eq(&c, CSB_V1_D1L_D1R_F0108_C706_ZONE_WALL_D3R_PC34, 706);
    self_check_eq(&c, CSB_V1_D1L_D1R_F0108_D1L_FLOOR_ZONE_PC34, 1508);
    self_check_eq(&c, CSB_V1_D1L_D1R_F0108_D1R_FLOOR_ZONE_PC34, 1510);
    self_check_eq(&c, csb_v1_viewport_d1l_d1r_f0108_zone_for_coordinate_set_pc34(0, 8), 1508);
    self_check_eq(&c, csb_v1_viewport_d1l_d1r_f0108_zone_for_coordinate_set_pc34(0, 10), 1510);
    self_check_eq(&c, csb_v1_viewport_d1l_d1r_f0108_zone_for_coordinate_set_pc34(2, 8), 1530);

    self_check_eq(&c, (int)csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(), 2);
    self_check(&c, csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_at_pc34(2) == NULL);
    self_check(&c, csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(9) == NULL);

    for (side_index = 0; side_index < 2; ++side_index) {
        const int side = side_index == 0 ?
            CSB_V1_D1L_D1R_F0108_SIDE_D1L_PC34 :
            CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34;
        const CSB_V1_D1LD1RF0108SpecPc34 *spec =
            csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(side);

        self_check(&c, spec != NULL);
        if (!spec) continue;
        self_check_eq(&c, (int)spec->side, side);
        self_check_eq(&c, spec->relative_depth, 1);
        self_check_eq(&c, spec->floor_coordinate_set, 0);
        self_check_eq(&c, spec->floor_zone_base, CSB_FLOOR_ZONE_BASE);
        self_check_eq(&c, spec->floor_zone_stride_pc34, CSB_FLOOR_ZONE_STRIDE_PC34);
        self_check_eq(&c, spec->ceiling_graphic, 66);
        self_check_eq(&c, spec->wall_keepout_zone_first, CSB_WALL_KEEP_OUT_ZONE_FIRST);
        self_check_eq(&c, spec->wall_keepout_zone_last, CSB_WALL_KEEP_OUT_ZONE_LAST);
        self_check_eq(&c, spec->custom_background_mask_after_floor_ceiling, 1);
        self_check_eq(&c, spec->custom_background_uses_mask, 1);
        self_check(&c, strstr(spec->redmcsb_f0108_anchor, "F0108") != NULL);
        self_check(&c, strstr(spec->redmcsb_f0115_anchor, "F0115") != NULL);
        self_check(&c, strstr(spec->redmcsb_defs_anchor, "DEFS.H") != NULL);
        self_check(&c, strstr(spec->lineage_anchor, "Viewport.cpp") != NULL);

        if (side == CSB_V1_D1L_D1R_F0108_SIDE_D1L_PC34) {
            self_check_eq(&c, spec->relative_lateral, -1);
            self_check_eq(&c, spec->f0128_dispatch_line, 8524);
            self_check_eq(&c, spec->lineage_room_slot, 10);
            self_check_eq(&c, spec->lineage_custom_background_line, 7050);
            self_check_eq(&c, spec->view_square, 4);
            self_check_eq(&c, spec->view_floor, 8);
            self_check_eq(&c, spec->floor_zone, 1508);
            self_check_eq(&c, spec->floor_flip_horizontal, 0);
            self_check_eq(&c, spec->ceiling_zone, CSB_D1L_CEILING_ZONE);
            self_check_eq(&c, spec->ceiling_flip_horizontal, 0);
            self_check_eq(&c, spec->thing_pass_order, 0x0032);
            self_check_eq(&c, spec->thing_pass_view_square, CSB_D1L_VIEW_SQUARE);
            self_check_eq(&c, spec->wall_zone, CSB_D1L_WALL_ZONE);
            self_check_eq(&c, spec->wall_ornament_view, CSB_D1L_WALL_ORNAMENT_VIEW);
        } else {
            self_check_eq(&c, spec->relative_lateral, 1);
            self_check_eq(&c, spec->f0128_dispatch_line, 8528);
            self_check_eq(&c, spec->lineage_room_slot, 11);
            self_check_eq(&c, spec->lineage_custom_background_line, 7070);
            self_check_eq(&c, spec->view_square, 5);
            self_check_eq(&c, spec->view_floor, 10);
            self_check_eq(&c, spec->floor_zone, 1510);
            self_check_eq(&c, spec->floor_flip_horizontal, 1);
            self_check_eq(&c, spec->ceiling_zone, CSB_D1R_CEILING_ZONE);
            self_check_eq(&c, spec->ceiling_flip_horizontal, 1);
            self_check_eq(&c, spec->thing_pass_order, 0x0041);
            self_check_eq(&c, spec->thing_pass_view_square, CSB_D1R_VIEW_SQUARE);
            self_check_eq(&c, spec->wall_zone, CSB_D1R_WALL_ZONE);
            self_check_eq(&c, spec->wall_ornament_view, CSB_D1R_WALL_ORNAMENT_VIEW);
        }

        for (context_index = 0;
             context_index < (int)(sizeof(contexts) / sizeof(contexts[0]));
             ++context_index) {
            CSB_V1_D1LD1RF0108StatePc34 state;
            CSB_V1_D1LD1RF0108ResultPc34 result;
            CSB_V1_D1LD1RF0108OrdinalPc34 ordinal;

            self_check(&c, csb_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
                (CSB_V1_D1LD1RF0108SidePc34)side, contexts[context_index], &state));
            self_check_eq(&c, (int)state.side, side);
            self_check_eq(&c, (int)state.context, (int)contexts[context_index]);
            self_check(&c, state.source_locked_contract_only);
            self_check(&c, state.no_real_asset_bitmap_parity);
            self_check(&c, state.no_game_data_load);

            self_check(&c, csb_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
                state.floor_ornament_ordinal, &ordinal));
            self_check(&c, ordinal.has_input_ordinal);
            self_check(&c, ordinal.footprint_flag_set);
            self_check_eq(&c, ordinal.recursive_footprints_index, 15);
            self_check_eq(&c, ordinal.metadata_blit_count, 2);

            self_check(&c, csb_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
            self_check_eq(&c, result.ok, 1);
            self_check_eq(&c, result.source_locked_contract_only, 1);
            self_check_eq(&c, result.no_real_asset_bitmap_parity, 1);
            self_check_eq(&c, result.no_game_data_load, 1);
            self_check_eq(&c, result.f0108_floor_calls, 1);
            self_check_eq(&c, result.f0108_primary_blits, 1);
            self_check_eq(&c, result.f0108_footprint_recursions, 1);
            self_check_eq(&c, result.ceiling_copy_calls, 1);
            self_check_eq(&c, result.custom_background_masks, 1);
            self_check_eq(&c, result.custom_background_after_floor_ceiling, 1);
            self_check_eq(&c, result.thing_pass_calls, 1);
            self_check_eq(&c, result.c10_transparent_blits, 1);
            self_check_eq(&c, result.f0115_row_guard_ok, 1);
            self_check_eq(&c, result.cell_order_band_ok, 1);
            self_check_eq(&c, result.palette_keepout_ok, 1);
            self_check_eq(&c, result.f0107_keepout_ok, 1);
            self_check_eq(&c, result.f0111_keepout_ok, 1);
            self_check_eq(&c, result.floor_zone, side == 1 ? 1508 : 1510);
            self_check_eq(&c, result.floor_primary_index, side == 1 ? 3 : 5);
            self_check_eq(&c, result.floor_recursive_index, 15);
            self_check_eq(&c, result.after_floor, state.floor_pixel);
            self_check_eq(&c, result.after_ceiling, state.ceiling_pixel);
            self_check_eq(&c, result.after_custom_background,
                          csb_v1_viewport_d1l_d1r_f0108_apply_custom_background_mask_pc34(
                              state.ceiling_pixel, state.custom_background_pixel,
                              state.custom_background_mask));
            self_check_eq(&c, result.after_thing_pass, result.after_custom_background);
            if (side == CSB_V1_D1L_D1R_F0108_SIDE_D1L_PC34) {
                ++s_last_self_test.d1l_floor_calls;
            } else {
                ++s_last_self_test.d1r_floor_calls;
            }
            s_last_self_test.footprint_recursions += result.f0108_footprint_recursions;
            s_last_self_test.ceiling_copies += result.ceiling_copy_calls;
            s_last_self_test.custom_background_masks += result.custom_background_masks;
            s_last_self_test.thing_pass_calls += result.thing_pass_calls;
            s_last_self_test.palette_keepouts += result.palette_keepout_ok;
            deterministic_hash = fnv1a_u32(deterministic_hash, result.deterministic_hash);
        }
    }

    self_check_eq(&c, s_last_self_test.d1l_floor_calls,
                  (int)(sizeof(contexts) / sizeof(contexts[0])));
    self_check_eq(&c, s_last_self_test.d1r_floor_calls,
                  (int)(sizeof(contexts) / sizeof(contexts[0])));
    self_check_eq(&c, s_last_self_test.footprint_recursions,
                  (int)(sizeof(contexts) / sizeof(contexts[0])) * 2);
    self_check_eq(&c, s_last_self_test.ceiling_copies,
                  (int)(sizeof(contexts) / sizeof(contexts[0])) * 2);
    self_check_eq(&c, s_last_self_test.custom_background_masks,
                  (int)(sizeof(contexts) / sizeof(contexts[0])) * 2);
    self_check_eq(&c, s_last_self_test.thing_pass_calls,
                  (int)(sizeof(contexts) / sizeof(contexts[0])) * 2);
    self_check_eq(&c, csb_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(77, 10), 77);
    self_check_eq(&c, csb_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(77, 11), 11);
    self_check_eq(&c, csb_v1_viewport_d1l_d1r_f0108_apply_custom_background_mask_pc34(0x70, 0xa5, 0x0f), 0x75);
    self_check(&c, !csb_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(0, NULL));

    for (i = 0; i < 16; ++i) {
        CSB_V1_D1LD1RF0108StatePc34 state;
        CSB_V1_D1LD1RF0108ResultPc34 result;
        int side = (next_lcg(&seed) & 1u) ?
            CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34 :
            CSB_V1_D1L_D1R_F0108_SIDE_D1L_PC34;

        self_check(&c, csb_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
            (CSB_V1_D1LD1RF0108SidePc34)side,
            contexts[i % (int)(sizeof(contexts) / sizeof(contexts[0]))],
            &state));
        state.seed = seed;
        state.floor_ornament_ordinal = ((next_lcg(&seed) & 7u) + 1u) |
            ((i & 1) ? CSB_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34 : 0u);
        state.floor_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x20u);
        state.ceiling_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x40u);
        state.custom_background_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x80u);
        if ((i % 5) == 0) {
            state.floor_pixel = CSB_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34;
        }
        self_check(&c, csb_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.source_locked_contract_only, 1);
        self_check_eq(&c, result.no_game_data_load, 1);
        self_check_eq(&c, result.custom_background_masks, 1);
        self_check_eq(&c, result.f0108_footprint_recursions, (i & 1) ? 1 : 0);
        deterministic_hash = fnv1a_u32(deterministic_hash, result.deterministic_hash);
    }

    for (side_index = 0; side_index < 2; ++side_index) {
        CSB_V1_D1LD1RF0108StatePc34 state;
        CSB_V1_D1LD1RF0108ResultPc34 result;
        int side = side_index == 0 ?
            CSB_V1_D1L_D1R_F0108_SIDE_D1L_PC34 :
            CSB_V1_D1L_D1R_F0108_SIDE_D1R_PC34;

        self_check(&c, csb_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
            (CSB_V1_D1LD1RF0108SidePc34)side,
            CSB_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34,
            &state));
        state.mutate_thing_list = true;
        self_check(&c, !csb_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
        self_check_eq(&c, result.mutation_rejections, 1);
        ++s_last_self_test.mutation_rejections;

        state.mutate_thing_list = false;
        state.mutation_guard_before ^= 0x11u;
        self_check(&c, !csb_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
        self_check_eq(&c, result.mutation_rejections, 1);
        ++s_last_self_test.mutation_rejections;

        state.mutation_guard_before = CSB_MUTATION_GUARD_BEFORE;
        state.mutation_guard_after ^= 0x22u;
        self_check(&c, !csb_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
        self_check_eq(&c, result.mutation_rejections, 1);
        ++s_last_self_test.mutation_rejections;

        state.mutation_guard_after = CSB_MUTATION_GUARD_AFTER;
        state.attempts_f0107_wall_ornament = true;
        self_check(&c, !csb_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);

        state.attempts_f0107_wall_ornament = false;
        state.attempts_f0111_door = true;
        self_check(&c, !csb_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
    }

    s_last_self_test.assertions = c.assertions;
    s_last_self_test.failures = c.failures;
    s_last_self_test.ok = c.failures == 0;
    s_last_self_test.deterministic_hash = deterministic_hash;
    return s_last_self_test.ok;
}

const CSB_V1_D1LD1RF0108SelfTestResultPc34 *
csb_v1_viewport_d1l_d1r_f0108_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *csb_v1_viewport_d1l_d1r_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}
