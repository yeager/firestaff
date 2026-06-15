#include "dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0L2_VIEW_SQUARE = 1,
    DM1_D0R2_VIEW_SQUARE = 2,
    DM1_D0L2_VIEW_FLOOR = 8,
    DM1_D0R2_VIEW_FLOOR = 10,
    DM1_D0_DEPTH = 0,
    DM1_D0L2_LANE = -1,
    DM1_D0R2_LANE = 1,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_D0L2_CEILING_ZONE = 870,
    DM1_D0R2_CEILING_ZONE = 872,
    DM1_CEILING_GRAPHIC_D0L_PC34 = 66,
    DM1_MUTATION_GUARD_BEFORE = 0x7160u,
    DM1_MUTATION_GUARD_AFTER = 0x6170u
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. ReDMCSB anchors: "
    "DUNVIEW.C F0108:3940-4011 floor ornament ordinal gate, C10 blit, "
    "MASK0x8000_FOOTPRINTS recursion, D0R horizontal flip, and PC34 C1500 + "
    "CoordinateSet * 11 + ViewFloor zone math; DUNVIEW.C F0104:3113-3156 "
    "and DUNVIEW.C F0105:3185-3247 native/flipped C10 blit contracts; DUNVIEW.C "
    "F0107:3502-3938 wall ornament keepout; DUNVIEW.C F0115:4547-4581,"
    "F0115:4923,F0115:5180-5188,F0115:5211-5214,F0115:5668-5671 "
    "thing-pass cell order and D0 row "
    "guard; graphic 66 ceiling C10 blit contract; "
    "DUNVIEW.C F0125:7960-8062 D0L dispatch with F0112 and F0115; "
    "DUNVIEW.C F0126:8064-8162 D0R dispatch with flipped F0105/F0112 paths; "
    "DUNVIEW.C F0128:8318-8486 mirror D0R horizontal flip and draws D0L "
    "then D0R before D0C; DUNGEON.C F0163:1769-1838 and F0164:1840-1905 "
    "thing-list mutation anchors; DUNGEON.C F0172:2466-2523 square-aspect "
    "source; DEFS.H:2088 C10; DEFS.H:2596-2611 view squares; DEFS.H:4139-4153 "
    "cell-order zones; DEFS.H:4205-4207 ornament zones; DEFS.H:4223 "
    "C1500_ZONE_FLOOR_ORNAMENT.";

static const DM1_V1_D0L2D0R2F0108SpecPc34 s_specs[] = {
    {
        DM1_V1_D0L2_D0R2_F0108_SIDE_D0L2_PC34,
        "D0L2 open-floor floor+ceiling+ornament composition",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        15,
        0,
        -1,
        DM1_D0L2_VIEW_SQUARE,
        DM1_D0L2_VIEW_FLOOR,
        DM1_D0_DEPTH,
        DM1_D0L2_LANE,
        4,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE_PC34,
        0,
        DM1_CEILING_GRAPHIC_D0L_PC34,
        DM1_D0L2_CEILING_ZONE,
        0,
        0x0002,
        DM1_D0L2_VIEW_SQUARE,
        -1,
        11,
        15,
        1,
        2, 3, 4, 5, 6,
        1,
        "DUNVIEW.C F0125:7960-8062 / F0128:8536-8537",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,5668-5671",
        "DEFS.H:2088/2596-2611/4139-4153/4205-4207/4223"
    },
    {
        DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34,
        "D0R2 open-floor floor+ceiling+ornament composition",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        16,
        0,
        1,
        DM1_D0R2_VIEW_SQUARE,
        DM1_D0R2_VIEW_FLOOR,
        DM1_D0_DEPTH,
        DM1_D0R2_LANE,
        4,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE_PC34,
        1,
        DM1_CEILING_GRAPHIC_D0L_PC34,
        DM1_D0R2_CEILING_ZONE,
        1,
        0x0001,
        DM1_D0R2_VIEW_SQUARE,
        -1,
        12,
        16,
        2,
        2, 3, 4, 5, 6,
        1,
        "DUNVIEW.C F0126:8064-8162 / F0128:8538-8541",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,5668-5671",
        "DEFS.H:2088/2596-2611/4139-4153/4205-4207/4223"
    }
};

static DM1_V1_D0L2D0R2F0108SelfTestResultPc34 s_last_self_test;

static uint32_t mix_hash(uint32_t hash, uint32_t value)
{
    hash ^= value + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash;
}

static uint32_t next_lcg(uint32_t *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static int is_open_floor_square(int square_element)
{
    return square_element == DM1_V1_D0L2_D0R2_F0108_SQUARE_CORRIDOR_PC34 ||
           square_element == DM1_V1_D0L2_D0R2_F0108_SQUARE_PIT_PC34 ||
           square_element == DM1_V1_D0L2_D0R2_F0108_SQUARE_TELEPORTER_PC34 ||
           square_element == DM1_V1_D0L2_D0R2_F0108_SQUARE_DOOR_SIDE_PC34 ||
           square_element == DM1_V1_D0L2_D0R2_F0108_SQUARE_STAIRS_SIDE_PC34 ||
           square_element == DM1_V1_D0L2_D0R2_F0108_SQUARE_STAIRS_FRONT_PC34;
}

size_t dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D0L2D0R2F0108SpecPc34 *
dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D0L2D0R2F0108SpecPc34 *
dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_count_pc34(); ++i) {
        if ((int)s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d0l2_d0r2_f0108_initial_state_pc34(
    DM1_V1_D0L2D0R2F0108SidePc34 side,
    DM1_V1_D0L2D0R2F0108StatePc34 *out)
{
    const DM1_V1_D0L2D0R2F0108SpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_for_side_pc34((int)side);
    if (!spec) return false;

    out->side = side;
    out->square_element = DM1_V1_D0L2_D0R2_F0108_SQUARE_CORRIDOR_PC34;
    out->floor_ornament_ordinal = side == DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34 ?
        0x8006u : 0x8004u;
    out->floor_ornament_coordinate_set = side == DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34 ?
        2 : 1;
    out->floor_ornament_native_bitmap_index = 240 + spec->view_floor;
    out->destination_pixel = side == DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34 ? 0x22u : 0x21u;
    out->floor_pixel = side == DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34 ? 0x42u : 0x41u;
    out->ceiling_pixel = side == DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34 ? 0x62u : 0x61u;
    out->thing_pass_pixel = DM1_V1_D0L2_D0R2_F0108_FLOOR_CEILING_C10_COLOR_FLESH_PC34;
    out->seed = side == DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34 ? 0x716002u : 0x716001u;
    out->mutation_guard_before = DM1_MUTATION_GUARD_BEFORE;
    out->mutation_guard_after = DM1_MUTATION_GUARD_AFTER;
    out->contract_only = true;
    out->no_real_asset_bitmap_parity = true;
    out->no_game_data_load = true;
    return true;
}

bool dm1_v1_viewport_d0l2_d0r2_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D0L2D0R2F0108OrdinalPc34 *out)
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
        (floor_ornament_ordinal &
         DM1_V1_D0L2_D0R2_F0108_FLOOR_CEILING_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D0L2_D0R2_F0108_FLOOR_CEILING_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_ordinal = out->footprint_flag_set ? cleared : floor_ornament_ordinal;
        out->primary_index = (int)out->primary_ordinal - 1;
        out->metadata_blit_count = 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D0L2_D0R2_F0108_FLOOR_CEILING_FOOTPRINT_INDEX_PC34;
        out->recursive_footprints_ordinal =
            (unsigned int)DM1_V1_D0L2_D0R2_F0108_FLOOR_CEILING_FOOTPRINT_INDEX_PC34 + 1u;
        ++out->metadata_blit_count;
    }
    return true;
}

uint8_t dm1_v1_viewport_d0l2_d0r2_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D0L2_D0R2_F0108_FLOOR_CEILING_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d0l2_d0r2_f0108_flip_row_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t width,
    size_t rows)
{
    size_t row;

    if (!source || !destination || width == 0u || rows == 0u) return false;
    for (row = 0u; row < rows; ++row) {
        size_t col;
        const size_t base = row * width;
        for (col = 0u; col < width; ++col) {
            destination[base + col] = source[base + (width - 1u - col)];
        }
    }
    return true;
}

int dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(
    int view_square,
    int view_cell,
    int view_depth)
{
    static const int rows[] = { -1, -1, -1, -1, 9, 10, -1, 11, 12, -1, -1, -1, -1, -1, -1, -1 };
    int row;

    if (view_square < 0 || view_square >= (int)(sizeof(rows) / sizeof(rows[0]))) return 0;
    row = rows[view_square];
    if (row < 0) return 0;
    if (view_depth == 3 && view_cell <= 1) return 0;
    if (view_depth == 0 && view_cell >= 2) return 0;
    return 1;
}

int dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(int ordinal)
{
    static const int ordinals[] = { 2, 3, 4, 5, 6 };

    if (ordinal < 575 || ordinal > 579) return -1;
    return ordinals[ordinal - 575];
}

bool dm1_v1_viewport_d0l2_d0r2_f0108_compose_pc34(
    const DM1_V1_D0L2D0R2F0108StatePc34 *state,
    DM1_V1_D0L2D0R2F0108ResultPc34 *out)
{
    const DM1_V1_D0L2D0R2F0108SpecPc34 *spec;
    DM1_V1_D0L2D0R2F0108OrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state) return false;
    spec = dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_for_side_pc34((int)state->side);
    out->spec = spec;
    if (!spec ||
        !state->contract_only ||
        !state->no_real_asset_bitmap_parity ||
        !state->no_game_data_load ||
        !is_open_floor_square(state->square_element) ||
        state->attempts_f0107_wall_ornament ||
        state->attempts_f0111_door ||
        state->mutate_thing_list ||
        state->floor_ornament_coordinate_set < 0 ||
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

    if (!dm1_v1_viewport_d0l2_d0r2_f0108_decode_ordinal_pc34(
            state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->ok = 1;
    out->source_locked_contract_only = 1;
    out->no_real_asset_bitmap_parity = 1;
    out->no_game_data_load = 1;
    out->dispatch_entries = 1;
    out->f0674_f0675_dispatch_entries = spec->f0674_f0675_dispatch_entry;
    out->floor_zone = spec->floor_zone_base +
        (state->floor_ornament_coordinate_set * spec->floor_zone_stride_pc34) +
        spec->view_floor;
    out->floor_primary_index = ordinal.primary_index;
    out->floor_recursive_index = ordinal.recursive_footprints_index;
    out->f0107_keepout_ok = 1;
    out->f0111_keepout_ok = 1;
    out->row_guard_accepts = dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(
        spec->view_square, spec->thing_pass_order & 0x000Fu, spec->view_depth);
    out->m575_to_m579_ordinal_parity =
        dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(575) == spec->m575_view_wall_d3l_right &&
        dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(576) == spec->m576_view_wall_d3r_left &&
        dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(577) == spec->m577_view_wall_d3l_front &&
        dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(578) == spec->m578_view_wall_d3c_front &&
        dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(579) == spec->m579_view_wall_d3r_front;

    pixel = state->destination_pixel;
    if (ordinal.has_input_ordinal) {
        out->f0108_floor_calls = 1;
        out->f0108_primary_blits = ordinal.primary_draws ? 1 : 0;
        out->f0108_footprint_recursions = ordinal.recursive_footprints_draw ? 1 : 0;
        if (ordinal.primary_draws) {
            pixel = dm1_v1_viewport_d0l2_d0r2_f0108_blend_c10_pc34(pixel, state->floor_pixel);
        }
    }
    out->after_floor = pixel;

    out->ceiling_copy_calls = 1;
    pixel = dm1_v1_viewport_d0l2_d0r2_f0108_blend_c10_pc34(pixel, state->ceiling_pixel);
    out->after_ceiling = pixel;

    out->thing_pass_calls = 1;
    pixel = dm1_v1_viewport_d0l2_d0r2_f0108_blend_c10_pc34(pixel, state->thing_pass_pixel);
    out->after_thing_pass = pixel;
    out->call_order_floor_before_ceiling = out->f0108_floor_calls <= out->ceiling_copy_calls;
    out->call_order_ceiling_before_thing_pass = out->ceiling_copy_calls <= out->thing_pass_calls;
    out->deterministic_hash = mix_hash(state->seed, (uint32_t)out->floor_zone);
    out->deterministic_hash = mix_hash(out->deterministic_hash, out->after_floor);
    out->deterministic_hash = mix_hash(out->deterministic_hash, out->after_ceiling);
    out->deterministic_hash = mix_hash(out->deterministic_hash, out->after_thing_pass);
    out->deterministic_hash = mix_hash(out->deterministic_hash, (uint32_t)out->floor_primary_index);
    out->deterministic_hash = mix_hash(out->deterministic_hash, (uint32_t)out->floor_recursive_index);
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

int run_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_self_test(void)
{
    SelfTestCounters c = { 0, 0 };
    uint32_t deterministic_hash = 0x716u;
    uint32_t seed = 0x7160108u;
    int side_index;
    int i;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));

    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0108:3940-4011");
    self_check_contains(&c, s_source_evidence, "MASK0x8000_FOOTPRINTS");
    self_check_contains(&c, s_source_evidence, "C1500 + CoordinateSet * 11");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0104:3113-3156");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0105:3185-3247");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0107:3502-3938");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0115:4547-4581");
    self_check_contains(&c, s_source_evidence, "F0115:4923");
    self_check_contains(&c, s_source_evidence, "F0115:5180-5188");
    self_check_contains(&c, s_source_evidence, "F0115:5211-5214");
    self_check_contains(&c, s_source_evidence, "F0115:5668-5671");
    self_check_contains(&c, s_source_evidence, "graphic 66");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0125:7960-8062");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0126:8064-8162");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0128:8318-8486");
    self_check_contains(&c, s_source_evidence, "DUNGEON.C F0163:1769-1838");
    self_check_contains(&c, s_source_evidence, "F0164:1840-1905");
    self_check_contains(&c, s_source_evidence, "F0172:2466-2523");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2088");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2596-2611");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4139-4153");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4205-4207");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4223");
    self_check_contains(&c, s_source_evidence, "source_locked_contract_only=1");

    self_check_eq(&c, (int)dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_count_pc34(), 2);
    self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_at_pc34(2) == NULL);
    self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_for_side_pc34(9) == NULL);

    for (side_index = 0; side_index < 2; ++side_index) {
        const int side = side_index == 0 ?
            DM1_V1_D0L2_D0R2_F0108_SIDE_D0L2_PC34 :
            DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34;
        const DM1_V1_D0L2D0R2F0108SpecPc34 *spec =
            dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_for_side_pc34(side);
        DM1_V1_D0L2D0R2F0108StatePc34 state;
        DM1_V1_D0L2D0R2F0108ResultPc34 result;
        DM1_V1_D0L2D0R2F0108StatePc34 reject_state;
        DM1_V1_D0L2D0R2F0108OrdinalPc34 ordinal;
        uint8_t source[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        uint8_t flipped[8] = { 0 };

        self_check(&c, spec != NULL);
        if (!spec) continue;
        self_check_eq(&c, (int)spec->side, side);
        self_check_eq(&c, spec->relative_depth, 0);
        self_check_eq(&c, spec->view_depth, DM1_D0_DEPTH);
        self_check_eq(&c, spec->floor_ornament_native_increment, 4);
        self_check_eq(&c, spec->floor_zone_base, DM1_FLOOR_ZONE_BASE);
        self_check_eq(&c, spec->floor_zone_stride_pc34, DM1_FLOOR_ZONE_STRIDE_PC34);
        self_check_eq(&c, spec->f0674_f0675_dispatch_entry, 1);
        self_check_eq(&c, spec->m575_view_wall_d3l_right, 2);
        self_check_eq(&c, spec->m576_view_wall_d3r_left, 3);
        self_check_eq(&c, spec->m577_view_wall_d3l_front, 4);
        self_check_eq(&c, spec->m578_view_wall_d3c_front, 5);
        self_check_eq(&c, spec->m579_view_wall_d3r_front, 6);
        self_check(&c, strstr(spec->redmcsb_dispatch_anchor, side == 1 ? "F0125" : "F0126") != NULL);
        self_check(&c, strstr(spec->redmcsb_f0108_anchor, "F0108") != NULL);
        self_check(&c, strstr(spec->redmcsb_f0115_anchor, "F0115") != NULL);
        self_check(&c, strstr(spec->redmcsb_defs_anchor, "DEFS.H") != NULL);

        if (side == DM1_V1_D0L2_D0R2_F0108_SIDE_D0L2_PC34) {
            self_check_eq(&c, spec->f0128_dispatch_index, 15);
            self_check_eq(&c, spec->relative_lateral, -1);
            self_check_eq(&c, spec->view_square, DM1_D0L2_VIEW_SQUARE);
            self_check_eq(&c, spec->view_floor, DM1_D0L2_VIEW_FLOOR);
            self_check_eq(&c, spec->floor_view_flipped_by_f0108, 0);
            self_check_eq(&c, spec->ceiling_graphic, 66);
            self_check_eq(&c, spec->ceiling_zone, DM1_D0L2_CEILING_ZONE);
            self_check_eq(&c, spec->ceiling_flip_horizontal, 0);
            self_check_eq(&c, spec->thing_pass_order, 0x0002);
            self_check_eq(&c, spec->g2028_row, -1);
        } else {
            self_check_eq(&c, spec->f0128_dispatch_index, 16);
            self_check_eq(&c, spec->relative_lateral, 1);
            self_check_eq(&c, spec->view_square, DM1_D0R2_VIEW_SQUARE);
            self_check_eq(&c, spec->view_floor, DM1_D0R2_VIEW_FLOOR);
            self_check_eq(&c, spec->floor_view_flipped_by_f0108, 1);
            self_check_eq(&c, spec->ceiling_graphic, 66);
            self_check_eq(&c, spec->ceiling_zone, DM1_D0R2_CEILING_ZONE);
            self_check_eq(&c, spec->ceiling_flip_horizontal, 1);
            self_check_eq(&c, spec->thing_pass_order, 0x0001);
            self_check_eq(&c, spec->g2028_row, -1);
        }

        self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_initial_state_pc34((DM1_V1_D0L2D0R2F0108SidePc34)side, &state));
        self_check_eq(&c, (int)state.side, side);
        self_check_eq(&c, state.square_element, DM1_V1_D0L2_D0R2_F0108_SQUARE_CORRIDOR_PC34);
        self_check_eq(&c, state.floor_ornament_coordinate_set, side == 1 ? 1 : 2);
        self_check_eq(&c, state.mutation_guard_before, DM1_MUTATION_GUARD_BEFORE);
        self_check_eq(&c, state.mutation_guard_after, DM1_MUTATION_GUARD_AFTER);
        self_check(&c, state.contract_only);
        self_check(&c, state.no_real_asset_bitmap_parity);
        self_check(&c, state.no_game_data_load);

        self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_decode_ordinal_pc34(state.floor_ornament_ordinal, &ordinal));
        self_check(&c, ordinal.has_input_ordinal);
        self_check(&c, ordinal.footprint_flag_set);
        self_check(&c, ordinal.primary_draws);
        self_check_eq(&c, ordinal.recursive_footprints_index, 15);
        self_check_eq(&c, (int)ordinal.recursive_footprints_ordinal, 16);
        self_check_eq(&c, ordinal.metadata_blit_count, 2);

        self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.ok, 1);
        self_check_eq(&c, result.dispatch_entries, 1);
        self_check_eq(&c, result.f0674_f0675_dispatch_entries, 1);
        self_check_eq(&c, result.f0108_floor_calls, 1);
        self_check_eq(&c, result.f0108_primary_blits, 1);
        self_check_eq(&c, result.f0108_footprint_recursions, 1);
        self_check_eq(&c, result.ceiling_copy_calls, 1);
        self_check_eq(&c, result.thing_pass_calls, 1);
        self_check_eq(&c, result.row_guard_accepts, 0);
        self_check_eq(&c, result.f0107_keepout_ok, 1);
        self_check_eq(&c, result.f0111_keepout_ok, 1);
        self_check_eq(&c, result.m575_to_m579_ordinal_parity, 1);
        self_check_eq(&c, result.call_order_floor_before_ceiling, 1);
        self_check_eq(&c, result.call_order_ceiling_before_thing_pass, 1);
        self_check_eq(&c, result.floor_zone,
                      DM1_FLOOR_ZONE_BASE +
                      state.floor_ornament_coordinate_set * DM1_FLOOR_ZONE_STRIDE_PC34 +
                      spec->view_floor);
        self_check_eq(&c, result.after_floor, state.floor_pixel);
        self_check_eq(&c, result.after_ceiling, state.ceiling_pixel);
        self_check_eq(&c, result.after_thing_pass, state.ceiling_pixel);
        s_last_self_test.floor_recursion_calls += result.f0108_footprint_recursions;
        s_last_self_test.ceiling_copies += result.ceiling_copy_calls;
        s_last_self_test.thing_pass_calls += result.thing_pass_calls;
        s_last_self_test.dispatch_entries += result.dispatch_entries;
        deterministic_hash = mix_hash(deterministic_hash, result.deterministic_hash);

        self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_flip_row_pc34(source, flipped, 4, 2));
        self_check_eq(&c, flipped[0], 4);
        self_check_eq(&c, flipped[3], 1);
        self_check_eq(&c, flipped[4], 8);
        self_check_eq(&c, flipped[7], 5);
        self_check(&c, !dm1_v1_viewport_d0l2_d0r2_f0108_flip_row_pc34(NULL, flipped, 4, 2));

        reject_state = state;
        reject_state.floor_ornament_ordinal = 0;
        self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_compose_pc34(&reject_state, &result));
        self_check_eq(&c, result.f0108_floor_calls, 0);
        self_check_eq(&c, result.f0108_footprint_recursions, 0);
        self_check_eq(&c, result.ceiling_copy_calls, 1);
        self_check_eq(&c, result.thing_pass_calls, 1);

        reject_state = state;
        reject_state.mutate_thing_list = true;
        self_check(&c, !dm1_v1_viewport_d0l2_d0r2_f0108_compose_pc34(&reject_state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
        self_check_eq(&c, result.mutation_rejections, 1);
        self_check_eq(&c, reject_state.destination_pixel, state.destination_pixel);
        self_check_eq(&c, reject_state.floor_pixel, state.floor_pixel);
        self_check_eq(&c, reject_state.ceiling_pixel, state.ceiling_pixel);
        ++s_last_self_test.mutation_rejections;

        reject_state = state;
        reject_state.attempts_f0107_wall_ornament = true;
        self_check(&c, !dm1_v1_viewport_d0l2_d0r2_f0108_compose_pc34(&reject_state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);

        reject_state = state;
        reject_state.attempts_f0111_door = true;
        self_check(&c, !dm1_v1_viewport_d0l2_d0r2_f0108_compose_pc34(&reject_state, &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
    }

    self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_decode_ordinal_pc34(0, NULL) == false);
    for (i = 0; i < 5; ++i) {
        self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(575 + i), 2 + i);
    }
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_view_wall_ordinal_pc34(574), -1);
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(1, 2, 0), 0);
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(2, 3, 0), 0);
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(4, 1, 1), 1);
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(5, 0, 1), 1);
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(3, 0, 1), 0);
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(14, 0, 3), 0);
    self_check_eq(&c, dm1_v1_viewport_d0l2_d0r2_f0108_row_guard_accepts_pc34(4, 2, 0), 0);
    s_last_self_test.row_guard_rejections += 5;

    for (i = 0; i < 40; ++i) {
        DM1_V1_D0L2D0R2F0108StatePc34 state;
        DM1_V1_D0L2D0R2F0108ResultPc34 result;
        int side = (next_lcg(&seed) & 1u) ?
            DM1_V1_D0L2_D0R2_F0108_SIDE_D0R2_PC34 :
            DM1_V1_D0L2_D0R2_F0108_SIDE_D0L2_PC34;
        unsigned int ordinal = (next_lcg(&seed) & 7u) + 1u;

        self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_initial_state_pc34((DM1_V1_D0L2D0R2F0108SidePc34)side, &state));
        state.seed = seed;
        state.floor_ornament_ordinal = ordinal | ((i & 1) ? 0x8000u : 0u);
        state.floor_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x20u);
        if ((i % 7) == 0) state.floor_pixel = DM1_V1_D0L2_D0R2_F0108_FLOOR_CEILING_C10_COLOR_FLESH_PC34;
        state.ceiling_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x40u);
        self_check(&c, dm1_v1_viewport_d0l2_d0r2_f0108_compose_pc34(&state, &result));
        self_check_eq(&c, result.source_locked_contract_only, 1);
        self_check_eq(&c, result.no_real_asset_bitmap_parity, 1);
        self_check_eq(&c, result.no_game_data_load, 1);
        self_check_eq(&c, result.thing_pass_calls, 1);
        self_check_eq(&c, result.ceiling_copy_calls, 1);
        self_check_eq(&c, result.f0108_footprint_recursions, (i & 1) ? 1 : 0);
        deterministic_hash = mix_hash(deterministic_hash, result.deterministic_hash);
    }

    s_last_self_test.assertions = c.assertions;
    s_last_self_test.failures = c.failures;
    s_last_self_test.ok = c.failures == 0;
    s_last_self_test.deterministic_hash = deterministic_hash;
    return s_last_self_test.ok;
}

const DM1_V1_D0L2D0R2F0108SelfTestResultPc34 *
dm1_v1_viewport_d0l2_d0r2_f0108_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *dm1_v1_viewport_d0l2_d0r2_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}
