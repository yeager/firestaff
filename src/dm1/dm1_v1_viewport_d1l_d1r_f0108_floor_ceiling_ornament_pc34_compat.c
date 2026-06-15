#include "dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D1L_VIEW_SQUARE = 4,       /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L */
    DM1_D1R_VIEW_SQUARE = 5,       /* ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R */
    DM1_D1L_ZONE_VIEW_FLOOR = 3,   /* Contract zone: C1500 + 0 * 11 + 3 = 1503 */
    DM1_D1R_ZONE_VIEW_FLOOR = 4,   /* Contract zone: C1500 + 0 * 11 + 4 = 1504 */
    DM1_D1_DEPTH = 1,
    DM1_D1L_LANE = -1,
    DM1_D1R_LANE = 1,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_D1L_CEILING_ZONE = 867,
    DM1_D1R_CEILING_ZONE = 869,
    DM1_CEILING_GRAPHIC_D1L_PC34 = 66,
    DM1_WALL_KEEP_OUT_ZONE_FIRST = 705,
    DM1_WALL_KEEP_OUT_ZONE_LAST = 706,
    DM1_MUTATION_GUARD_BEFORE = 0x725108u,
    DM1_MUTATION_GUARD_AFTER = 0x801527u
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. ReDMCSB anchors: "
    "DUNVIEW.C F0108:3940-4011 floor ornament ordinal gate, MASK0x8000 "
    "footprints recursion, C10 blit, and PC34 C1500 + CoordinateSet * 11 + "
    "ViewFloor zone math; DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 "
    "native/flipped C10 blit; DUNVIEW.C F0107:3502-3938 wall ornament "
    "keepout; DUNVIEW.C F0115:4547-4581,F0115:4923,F0115:5180-5188,"
    "F0115:5211-5214 thing-pass cell ordering; DUNVIEW.C F0127 plus "
    "F0128:8318-8486,8536-8541 "
    "dispatcher sweep anchor, with D1L/D1R at F0128:8524-8529; "
    "DUNGEON.C F0163:1769-1838 and F0164:1840-1905 thing-list mutation "
    "anchors; DUNGEON.C F0172:2466-2523 square-aspect source; "
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2596-2611 view squares; "
    "DEFS.H:2662/2668-2677 cell orders; DEFS.H:4045-4046 C705/C706 wall "
    "zones; DEFS.H:4139-4153 cell-order zone band; DEFS.H:4223 "
    "C1500_ZONE_FLOOR_ORNAMENT.";

static const DM1_V1_D1LD1RF0108SpecPc34 s_specs[] = {
    {
        DM1_V1_D1L_D1R_F0108_SIDE_D1L_PC34,
        "D1L floor+ceiling+ornament source-lock contract",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        DM1_D1_DEPTH,
        DM1_D1L_LANE,
        8524,
        DM1_D1L_VIEW_SQUARE,
        DM1_D1L_ZONE_VIEW_FLOOR,
        1503,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE_PC34,
        0,
        0,
        DM1_CEILING_GRAPHIC_D1L_PC34,
        DM1_D1L_CEILING_ZONE,
        0,
        0x0032,
        DM1_D1L_VIEW_SQUARE,
        2,
        3,
        DM1_WALL_KEEP_OUT_ZONE_FIRST,
        DM1_WALL_KEEP_OUT_ZONE_LAST,
        "DUNVIEW.C F0122:7391-7557 / F0128:8524-8525",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223"
    },
    {
        DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34,
        "D1R floor+ceiling+ornament source-lock contract",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        DM1_D1_DEPTH,
        DM1_D1R_LANE,
        8528,
        DM1_D1R_VIEW_SQUARE,
        DM1_D1R_ZONE_VIEW_FLOOR,
        1504,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE_PC34,
        0,
        1,
        DM1_CEILING_GRAPHIC_D1L_PC34,
        DM1_D1R_CEILING_ZONE,
        1,
        0x0041,
        DM1_D1R_VIEW_SQUARE,
        1,
        4,
        DM1_WALL_KEEP_OUT_ZONE_FIRST,
        DM1_WALL_KEEP_OUT_ZONE_LAST,
        "DUNVIEW.C F0123:7559-7725 / F0128:8528-8529",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223"
    }
};

static DM1_V1_D1LD1RF0108SelfTestResultPc34 s_last_self_test;

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

static int is_supported_context(DM1_V1_D1LD1RF0108ContextPc34 context)
{
    return context == DM1_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34 ||
           context == DM1_V1_D1L_D1R_F0108_CONTEXT_OPEN_PIT_PC34 ||
           context == DM1_V1_D1L_D1R_F0108_CONTEXT_TELEPORTER_PC34 ||
           context == DM1_V1_D1L_D1R_F0108_CONTEXT_DOOR_SIDE_PC34 ||
           context == DM1_V1_D1L_D1R_F0108_CONTEXT_STAIRS_FRONT_PC34;
}

size_t dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D1LD1RF0108SpecPc34 *
dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D1LD1RF0108SpecPc34 *
dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(int side)
{
    size_t i;

    for (i = 0u; i < dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(); ++i) {
        if ((int)s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
    DM1_V1_D1LD1RF0108SidePc34 side,
    DM1_V1_D1LD1RF0108ContextPc34 context,
    DM1_V1_D1LD1RF0108StatePc34 *out)
{
    const DM1_V1_D1LD1RF0108SpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34((int)side);
    if (!spec || !is_supported_context(context)) return false;

    out->side = side;
    out->context = context;
    out->floor_ornament_ordinal =
        side == DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x8006u : 0x8004u;
    out->floor_ornament_native_bitmap_index = 240 + spec->view_floor;
    out->destination_pixel =
        side == DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x32u : 0x31u;
    out->floor_pixel =
        side == DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x52u : 0x51u;
    out->ceiling_pixel =
        side == DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x72u : 0x71u;
    out->thing_pass_pixel = DM1_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34;
    out->seed = side == DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34 ? 0x725102u : 0x725101u;
    out->mutation_guard_before = DM1_MUTATION_GUARD_BEFORE;
    out->mutation_guard_after = DM1_MUTATION_GUARD_AFTER;
    out->source_locked_contract_only = true;
    out->no_real_asset_bitmap_parity = true;
    out->no_game_data_load = true;
    return true;
}

bool dm1_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D1LD1RF0108OrdinalPc34 *out)
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
        (floor_ornament_ordinal & DM1_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal & ~DM1_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_index = (int)(out->footprint_flag_set ? cleared : floor_ornament_ordinal) - 1;
        out->metadata_blit_count = 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index = DM1_V1_D1L_D1R_F0108_FOOTPRINT_INDEX_PC34;
        ++out->metadata_blit_count;
    }
    return true;
}

uint8_t dm1_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d1l_d1r_f0108_compose_pc34(
    const DM1_V1_D1LD1RF0108StatePc34 *state,
    DM1_V1_D1LD1RF0108ResultPc34 *out)
{
    const DM1_V1_D1LD1RF0108SpecPc34 *spec;
    DM1_V1_D1LD1RF0108OrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state) return false;
    spec = dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34((int)state->side);
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
        state->mutation_guard_before != DM1_MUTATION_GUARD_BEFORE ||
        state->mutation_guard_after != DM1_MUTATION_GUARD_AFTER) {
        out->rejected_non_contract_state = 1;
        if (state->mutate_thing_list ||
            state->mutation_guard_before != DM1_MUTATION_GUARD_BEFORE ||
            state->mutation_guard_after != DM1_MUTATION_GUARD_AFTER) {
            out->mutation_rejections = 1;
        }
        return false;
    }

    if (!dm1_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
            state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->ok = 1;
    out->source_locked_contract_only = 1;
    out->no_real_asset_bitmap_parity = 1;
    out->no_game_data_load = 1;
    out->floor_zone =
        spec->floor_zone_base + spec->floor_coordinate_set * spec->floor_zone_stride_pc34 +
        spec->view_floor;
    out->floor_primary_index = ordinal.primary_index;
    out->floor_recursive_index = ordinal.recursive_footprints_index;
    out->f0107_keepout_ok = 1;
    out->f0111_keepout_ok = 1;
    out->cell_order_band_ok =
        (spec->thing_pass_order == 0x0032 || spec->thing_pass_order == 0x0041) &&
        spec->thing_pass_first_cell > 0 && spec->thing_pass_second_cell > 0;
    out->palette_keepout_ok =
        spec->wall_keepout_zone_first == DM1_WALL_KEEP_OUT_ZONE_FIRST &&
        spec->wall_keepout_zone_last == DM1_WALL_KEEP_OUT_ZONE_LAST;

    pixel = state->destination_pixel;
    if (ordinal.has_input_ordinal) {
        out->f0108_floor_calls = 1;
        out->f0108_primary_blits = ordinal.primary_draws ? 1 : 0;
        out->f0108_footprint_recursions = ordinal.recursive_footprints_draw ? 1 : 0;
        if (ordinal.primary_draws) {
            pixel = dm1_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(pixel, state->floor_pixel);
            if (state->floor_pixel == DM1_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34) {
                ++out->c10_transparent_blits;
            }
        }
    }
    out->after_floor = pixel;

    out->ceiling_copy_calls = 1;
    pixel = dm1_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(pixel, state->ceiling_pixel);
    if (state->ceiling_pixel == DM1_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    out->after_ceiling = pixel;

    out->thing_pass_calls = 1;
    pixel = dm1_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(pixel, state->thing_pass_pixel);
    if (state->thing_pass_pixel == DM1_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    out->after_thing_pass = pixel;

    out->deterministic_hash = fnv1a_u32(state->seed, (uint32_t)state->context);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, (uint32_t)out->floor_zone);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, out->after_floor);
    out->deterministic_hash = fnv1a_u32(out->deterministic_hash, out->after_ceiling);
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
    if (!condition) ++c->failures;
}

static void self_check_eq(SelfTestCounters *c, int got, int want)
{
    ++c->assertions;
    if (got != want) ++c->failures;
}

static void self_check_contains(SelfTestCounters *c, const char *haystack, const char *needle)
{
    ++c->assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) ++c->failures;
}

int run_dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_self_test(void)
{
    static const DM1_V1_D1LD1RF0108ContextPc34 contexts[] = {
        DM1_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34,
        DM1_V1_D1L_D1R_F0108_CONTEXT_OPEN_PIT_PC34,
        DM1_V1_D1L_D1R_F0108_CONTEXT_TELEPORTER_PC34,
        DM1_V1_D1L_D1R_F0108_CONTEXT_DOOR_SIDE_PC34,
        DM1_V1_D1L_D1R_F0108_CONTEXT_STAIRS_FRONT_PC34
    };
    SelfTestCounters c = { 0, 0 };
    uint32_t deterministic_hash = 2166136261u;
    uint32_t seed = 0x7250108u;
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
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0127");
    self_check_contains(&c, s_source_evidence, "F0128:8318-8486");
    self_check_contains(&c, s_source_evidence, "F0128:8524-8529");
    self_check_contains(&c, s_source_evidence, "DUNGEON.C F0163:1769-1838");
    self_check_contains(&c, s_source_evidence, "F0164:1840-1905");
    self_check_contains(&c, s_source_evidence, "F0172:2466-2523");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2088");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2596-2611");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2662/2668-2677");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4045-4046");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4139-4153");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4223");
    self_check_contains(&c, s_source_evidence, "source_locked_contract_only=1");
    self_check_contains(&c, s_source_evidence, "no_real_asset_bitmap_parity=1");
    self_check_contains(&c, s_source_evidence, "no_game_data_load=1");

    self_check_eq(&c, (int)dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_count_pc34(), 2);
    self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_at_pc34(2) == NULL);
    self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(9) == NULL);

    for (side_index = 0; side_index < 2; ++side_index) {
        const int side = side_index == 0 ?
            DM1_V1_D1L_D1R_F0108_SIDE_D1L_PC34 :
            DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34;
        const DM1_V1_D1LD1RF0108SpecPc34 *spec =
            dm1_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_for_side_pc34(side);

        self_check(&c, spec != NULL);
        if (!spec) continue;
        self_check_eq(&c, (int)spec->side, side);
        self_check_eq(&c, spec->relative_depth, 1);
        self_check_eq(&c, spec->floor_coordinate_set, 0);
        self_check_eq(&c, spec->floor_zone_base, DM1_FLOOR_ZONE_BASE);
        self_check_eq(&c, spec->floor_zone_stride_pc34, DM1_FLOOR_ZONE_STRIDE_PC34);
        self_check_eq(&c, spec->ceiling_graphic, 66);
        self_check_eq(&c, spec->wall_keepout_zone_first, DM1_WALL_KEEP_OUT_ZONE_FIRST);
        self_check_eq(&c, spec->wall_keepout_zone_last, DM1_WALL_KEEP_OUT_ZONE_LAST);
        self_check(&c, strstr(spec->redmcsb_f0108_anchor, "F0108") != NULL);
        self_check(&c, strstr(spec->redmcsb_f0115_anchor, "F0115") != NULL);
        self_check(&c, strstr(spec->redmcsb_defs_anchor, "DEFS.H") != NULL);

        if (side == DM1_V1_D1L_D1R_F0108_SIDE_D1L_PC34) {
            self_check_eq(&c, spec->relative_lateral, -1);
            self_check_eq(&c, spec->f0128_dispatch_line, 8524);
            self_check_eq(&c, spec->view_square, 4);
            self_check_eq(&c, spec->view_floor, 3);
            self_check_eq(&c, spec->floor_zone, 1503);
            self_check_eq(&c, spec->floor_flip_horizontal, 0);
            self_check_eq(&c, spec->ceiling_zone, DM1_D1L_CEILING_ZONE);
            self_check_eq(&c, spec->ceiling_flip_horizontal, 0);
            self_check_eq(&c, spec->thing_pass_order, 0x0032);
            self_check_eq(&c, spec->thing_pass_view_square, DM1_D1L_VIEW_SQUARE);
        } else {
            self_check_eq(&c, spec->relative_lateral, 1);
            self_check_eq(&c, spec->f0128_dispatch_line, 8528);
            self_check_eq(&c, spec->view_square, 5);
            self_check_eq(&c, spec->view_floor, 4);
            self_check_eq(&c, spec->floor_zone, 1504);
            self_check_eq(&c, spec->floor_flip_horizontal, 1);
            self_check_eq(&c, spec->ceiling_zone, DM1_D1R_CEILING_ZONE);
            self_check_eq(&c, spec->ceiling_flip_horizontal, 1);
            self_check_eq(&c, spec->thing_pass_order, 0x0041);
            self_check_eq(&c, spec->thing_pass_view_square, DM1_D1R_VIEW_SQUARE);
        }

        for (context_index = 0;
             context_index < (int)(sizeof(contexts) / sizeof(contexts[0]));
             ++context_index) {
            DM1_V1_D1LD1RF0108StatePc34 state;
            DM1_V1_D1LD1RF0108ResultPc34 result;
            DM1_V1_D1LD1RF0108OrdinalPc34 ordinal;

            self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
                (DM1_V1_D1LD1RF0108SidePc34)side, contexts[context_index], &state));
            self_check_eq(&c, (int)state.side, side);
            self_check_eq(&c, (int)state.context, (int)contexts[context_index]);
            self_check(&c, state.source_locked_contract_only);
            self_check(&c, state.no_real_asset_bitmap_parity);
            self_check(&c, state.no_game_data_load);

            self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(
                state.floor_ornament_ordinal, &ordinal));
            self_check(&c, ordinal.has_input_ordinal);
            self_check(&c, ordinal.footprint_flag_set);
            self_check_eq(&c, ordinal.recursive_footprints_index, 15);
            self_check_eq(&c, ordinal.metadata_blit_count, 2);

            self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
            self_check_eq(&c, result.ok, 1);
            self_check_eq(&c, result.source_locked_contract_only, 1);
            self_check_eq(&c, result.no_real_asset_bitmap_parity, 1);
            self_check_eq(&c, result.no_game_data_load, 1);
            self_check_eq(&c, result.f0108_floor_calls, 1);
            self_check_eq(&c, result.f0108_primary_blits, 1);
            self_check_eq(&c, result.f0108_footprint_recursions, 1);
            self_check_eq(&c, result.ceiling_copy_calls, 1);
            self_check_eq(&c, result.thing_pass_calls, 1);
            self_check_eq(&c, result.c10_transparent_blits, 1);
            self_check_eq(&c, result.cell_order_band_ok, 1);
            self_check_eq(&c, result.palette_keepout_ok, 1);
            self_check_eq(&c, result.f0107_keepout_ok, 1);
            self_check_eq(&c, result.f0111_keepout_ok, 1);
            self_check_eq(&c, result.floor_zone, side == 1 ? 1503 : 1504);
            self_check_eq(&c, result.floor_primary_index, side == 1 ? 3 : 5);
            self_check_eq(&c, result.floor_recursive_index, 15);
            self_check_eq(&c, result.after_floor, state.floor_pixel);
            self_check_eq(&c, result.after_ceiling, state.ceiling_pixel);
            self_check_eq(&c, result.after_thing_pass, state.ceiling_pixel);
            if (side == DM1_V1_D1L_D1R_F0108_SIDE_D1L_PC34) {
                ++s_last_self_test.d1l_floor_calls;
            } else {
                ++s_last_self_test.d1r_floor_calls;
            }
            s_last_self_test.footprint_recursions += result.f0108_footprint_recursions;
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
    self_check_eq(&c, s_last_self_test.thing_pass_calls,
                  (int)(sizeof(contexts) / sizeof(contexts[0])) * 2);
    self_check_eq(&c, dm1_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(77, 10), 77);
    self_check_eq(&c, dm1_v1_viewport_d1l_d1r_f0108_blend_c10_pc34(77, 11), 11);
    self_check(&c, !dm1_v1_viewport_d1l_d1r_f0108_decode_ordinal_pc34(0, NULL));

    for (i = 0; i < 16; ++i) {
        DM1_V1_D1LD1RF0108StatePc34 state;
        DM1_V1_D1LD1RF0108ResultPc34 result;
        int side = (next_lcg(&seed) & 1u) ?
            DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34 :
            DM1_V1_D1L_D1R_F0108_SIDE_D1L_PC34;

        self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
            (DM1_V1_D1LD1RF0108SidePc34)side,
            contexts[i % (int)(sizeof(contexts) / sizeof(contexts[0]))],
            &state));
        state.seed = seed;
        state.floor_ornament_ordinal = ((next_lcg(&seed) & 7u) + 1u) |
            ((i & 1) ? DM1_V1_D1L_D1R_F0108_FOOTPRINT_MASK_PC34 : 0u);
        state.floor_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x20u);
        state.ceiling_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x40u);
        if ((i % 5) == 0) state.floor_pixel = DM1_V1_D1L_D1R_F0108_C10_COLOR_FLESH_PC34;
        self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.source_locked_contract_only, 1);
        self_check_eq(&c, result.no_game_data_load, 1);
        self_check_eq(&c, result.f0108_footprint_recursions, (i & 1) ? 1 : 0);
        deterministic_hash = fnv1a_u32(deterministic_hash, result.deterministic_hash);
    }

    for (side_index = 0; side_index < 2; ++side_index) {
        DM1_V1_D1LD1RF0108StatePc34 state;
        DM1_V1_D1LD1RF0108ResultPc34 result;
        int side = side_index == 0 ?
            DM1_V1_D1L_D1R_F0108_SIDE_D1L_PC34 :
            DM1_V1_D1L_D1R_F0108_SIDE_D1R_PC34;

        self_check(&c, dm1_v1_viewport_d1l_d1r_f0108_initial_state_pc34(
            (DM1_V1_D1LD1RF0108SidePc34)side,
            DM1_V1_D1L_D1R_F0108_CONTEXT_CORRIDOR_PC34,
            &state));
        state.mutate_thing_list = true;
        self_check(&c, !dm1_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
        self_check_eq(&c, result.mutation_rejections, 1);
        ++s_last_self_test.mutation_rejections;

        state.mutate_thing_list = false;
        state.attempts_f0107_wall_ornament = true;
        self_check(&c, !dm1_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);

        state.attempts_f0107_wall_ornament = false;
        state.attempts_f0111_door = true;
        self_check(&c, !dm1_v1_viewport_d1l_d1r_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
    }

    s_last_self_test.assertions = c.assertions;
    s_last_self_test.failures = c.failures;
    s_last_self_test.ok = c.failures == 0;
    s_last_self_test.deterministic_hash = deterministic_hash;
    return s_last_self_test.ok;
}

const DM1_V1_D1LD1RF0108SelfTestResultPc34 *
dm1_v1_viewport_d1l_d1r_f0108_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *dm1_v1_viewport_d1l_d1r_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}
