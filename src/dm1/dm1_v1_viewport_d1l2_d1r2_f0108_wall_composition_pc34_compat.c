#include "dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_pc34_compat.h"

#include <string.h>

enum {
    DM1_D1L_VIEW_SQUARE = 4,
    DM1_D1R_VIEW_SQUARE = 5,
    DM1_D1L_VIEW_WALL_RIGHT = 12,
    DM1_D1R_VIEW_WALL_LEFT = 13,
    DM1_D1L_VIEW_FLOOR = 8,
    DM1_D1R_VIEW_FLOOR = 10,
    DM1_WALL_ZONE_D1L = 713,
    DM1_WALL_ZONE_D1R = 714,
    DM1_WALL_ORNAMENT_ZONE_BASE = 1004,
    DM1_WALL_ORNAMENT_ZONE_STRIDE_PC34 = 15,
    DM1_FLOOR_KEEPOUT_ZONE_BASE = 1500,
    DM1_FLOOR_KEEPOUT_ZONE_STRIDE_PC34 = 11,
    DM1_WALL_ELEMENT = 0,
    DM1_D1L_WALL_NATIVE = 3,
    DM1_D1R_WALL_NATIVE = 2,
    DM1_CELL_ORDER_WALL_RETURN = 0x0021,
    DM1_CELL_ORDER_D1L_OPEN = 0x0032,
    DM1_CELL_ORDER_D1R_OPEN = 0x0041,
    DM1_CELL_ORDER_D1L_DOOR_PASS1 = 0x0028,
    DM1_CELL_ORDER_D1R_DOOR_PASS1 = 0x0018,
    DM1_CELL_ORDER_D1L_DOOR_PASS2 = 0x0039,
    DM1_CELL_ORDER_D1R_DOOR_PASS2 = 0x0049
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. ReDMCSB anchors: "
    "DUNVIEW.C F0107:3502-3938 wall ornament ordinal gate, C10 blit, "
    "D1R horizontal flip, and F0107 zone math C1004 + CoordinateSet * 15 "
    "+ ViewWall; "
    "DUNVIEW.C F0104:3113-3156 and DUNVIEW.C F0105:3185-3247 native/flipped "
    "C10 blit contracts; DUNVIEW.C F0108:3940-4011 related floor-ornament "
    "ordinal gate, MASK0x8000_FOOTPRINTS recursion, C10 blit, and keepout "
    "zone math C1500 + CoordinateSet * 11 + ViewFloor; DUNVIEW.C "
    "F0122:7436-7460 D1L wall calls F0107 and returns before F0108; "
    "DUNVIEW.C F0123:7604-7628 D1R wall calls F0107 and returns before "
    "F0108; DUNVIEW.C F0128:8318-8486 mirror D1R horizontal flip setup plus "
    "F0128:8524-8529 D1L/D1R dispatch order; DUNGEON.C F0163:1769-1838 "
    "and F0164:1840-1905 thing-list mutation anchors; DUNGEON.C "
    "F0172:2466-2523 square-aspect source; DEFS.H:2088 C10_COLOR_FLESH; "
    "DEFS.H:2596-2611 view squares 4/5 for D1L2/D1R2; DEFS.H:4139-4153 "
    "cell-order zones; DEFS.H:4205-4207 floor ornament keepout; "
    "DEFS.H:2662/2668-2677 cell orders; DEFS.H:2708-2709 view walls; "
    "DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT.";

static const DM1_V1_D1L2D1R2F0108WallSpecPc34 s_specs[] = {
    {
        DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1L2_PC34,
        DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_NATIVE_PC34,
        "D1L2 wall ornament composition native wall band",
        0, 1, -1,
        DM1_D1L_VIEW_SQUARE,
        DM1_D1L_VIEW_WALL_RIGHT,
        DM1_D1L_VIEW_FLOOR,
        DM1_WALL_ZONE_D1L,
        DM1_WALL_ORNAMENT_ZONE_BASE,
        DM1_WALL_ORNAMENT_ZONE_STRIDE_PC34,
        DM1_FLOOR_KEEPOUT_ZONE_BASE,
        DM1_FLOOR_KEEPOUT_ZONE_STRIDE_PC34,
        DM1_WALL_ELEMENT,
        DM1_D1L_WALL_NATIVE,
        DM1_D1R_WALL_NATIVE,
        1, 0, 0, 0,
        DM1_CELL_ORDER_WALL_RETURN,
        DM1_CELL_ORDER_D1L_OPEN,
        DM1_CELL_ORDER_D1L_DOOR_PASS1,
        DM1_CELL_ORDER_D1L_DOOR_PASS2,
        1, 1, 1,
        "DUNVIEW.C F0122:7436-7460",
        "DUNVIEW.C F0107:3502-3938",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DEFS.H:2088/2596-2611/2662/2668-2677/2708-2709/4205-4207"
    },
    {
        DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1L2_PC34,
        DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_F0128_FLIPPED_PC34,
        "D1L2 wall ornament composition F0128 flipped wall band",
        1, 1, -1,
        DM1_D1L_VIEW_SQUARE,
        DM1_D1L_VIEW_WALL_RIGHT,
        DM1_D1L_VIEW_FLOOR,
        DM1_WALL_ZONE_D1L,
        DM1_WALL_ORNAMENT_ZONE_BASE,
        DM1_WALL_ORNAMENT_ZONE_STRIDE_PC34,
        DM1_FLOOR_KEEPOUT_ZONE_BASE,
        DM1_FLOOR_KEEPOUT_ZONE_STRIDE_PC34,
        DM1_WALL_ELEMENT,
        DM1_D1L_WALL_NATIVE,
        DM1_D1R_WALL_NATIVE,
        0, 1, 0, 1,
        DM1_CELL_ORDER_WALL_RETURN,
        DM1_CELL_ORDER_D1L_OPEN,
        DM1_CELL_ORDER_D1L_DOOR_PASS1,
        DM1_CELL_ORDER_D1L_DOOR_PASS2,
        1, 1, 1,
        "DUNVIEW.C F0128:8318-8486 mirrored D1L wall",
        "DUNVIEW.C F0107:3502-3938",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DEFS.H:2088/2596-2611/2662/2668-2677/2708-2709/4205-4207"
    },
    {
        DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34,
        DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_NATIVE_PC34,
        "D1R2 wall ornament composition native D1R flip contract",
        2, 1, 1,
        DM1_D1R_VIEW_SQUARE,
        DM1_D1R_VIEW_WALL_LEFT,
        DM1_D1R_VIEW_FLOOR,
        DM1_WALL_ZONE_D1R,
        DM1_WALL_ORNAMENT_ZONE_BASE,
        DM1_WALL_ORNAMENT_ZONE_STRIDE_PC34,
        DM1_FLOOR_KEEPOUT_ZONE_BASE,
        DM1_FLOOR_KEEPOUT_ZONE_STRIDE_PC34,
        DM1_WALL_ELEMENT,
        DM1_D1R_WALL_NATIVE,
        DM1_D1L_WALL_NATIVE,
        1, 0, 1, 0,
        DM1_CELL_ORDER_WALL_RETURN,
        DM1_CELL_ORDER_D1R_OPEN,
        DM1_CELL_ORDER_D1R_DOOR_PASS1,
        DM1_CELL_ORDER_D1R_DOOR_PASS2,
        1, 1, 1,
        "DUNVIEW.C F0123:7604-7628",
        "DUNVIEW.C F0107:3502-3938",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DEFS.H:2088/2596-2611/2662/2668-2677/2708-2709/4205-4207"
    },
    {
        DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34,
        DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_F0128_FLIPPED_PC34,
        "D1R2 wall ornament composition F0128 flipped D1R contract",
        3, 1, 1,
        DM1_D1R_VIEW_SQUARE,
        DM1_D1R_VIEW_WALL_LEFT,
        DM1_D1R_VIEW_FLOOR,
        DM1_WALL_ZONE_D1R,
        DM1_WALL_ORNAMENT_ZONE_BASE,
        DM1_WALL_ORNAMENT_ZONE_STRIDE_PC34,
        DM1_FLOOR_KEEPOUT_ZONE_BASE,
        DM1_FLOOR_KEEPOUT_ZONE_STRIDE_PC34,
        DM1_WALL_ELEMENT,
        DM1_D1R_WALL_NATIVE,
        DM1_D1L_WALL_NATIVE,
        0, 1, 1, 1,
        DM1_CELL_ORDER_WALL_RETURN,
        DM1_CELL_ORDER_D1R_OPEN,
        DM1_CELL_ORDER_D1R_DOOR_PASS1,
        DM1_CELL_ORDER_D1R_DOOR_PASS2,
        1, 1, 1,
        "DUNVIEW.C F0128:8318-8486 mirrored D1R wall",
        "DUNVIEW.C F0107:3502-3938",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DEFS.H:2088/2596-2611/2662/2668-2677/2708-2709/4205-4207"
    }
};

static DM1_V1_D1L2D1R2F0108WallSelfTestResultPc34 s_last_self_test;

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

size_t dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D1L2D1R2F0108WallSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D1L2D1R2F0108WallSpecPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_for_pc34(
    DM1_V1_D1L2D1R2F0108WallSidePc34 side,
    DM1_V1_D1L2D1R2F0108WallRoutePc34 route)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_count_pc34(); ++i) {
        if (s_specs[i].side == side && s_specs[i].route == route) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_initial_state_pc34(
    DM1_V1_D1L2D1R2F0108WallSidePc34 side,
    DM1_V1_D1L2D1R2F0108WallRoutePc34 route,
    DM1_V1_D1L2D1R2F0108WallStatePc34 *out)
{
    const DM1_V1_D1L2D1R2F0108WallSpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_for_pc34(side, route);
    if (!spec) return false;

    out->side = side;
    out->route = route;
    out->wall_ornament_ordinal =
        side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 0x8006u : 0x8004u;
    out->floor_ornament_ordinal =
        side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 6u : 4u;
    out->wall_ornament_coordinate_set =
        side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 2 : 1;
    out->floor_ornament_coordinate_set = out->wall_ornament_coordinate_set;
    out->view_cell = side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 0 : 1;
    out->view_depth = 1;
    out->destination_pixel =
        side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 0x22u : 0x21u;
    out->wall_pixel =
        side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 0x52u : 0x51u;
    out->footprint_pixel =
        route == DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_F0128_FLIPPED_PC34 ? 0x64u : 0x63u;
    out->seed = 0x7170108u + (uint32_t)spec->draw_order_index;
    out->contract_only = true;
    out->no_real_asset_bitmap_parity = true;
    out->no_game_data_load = true;
    return true;
}

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_decode_ordinal_pc34(
    unsigned int wall_ornament_ordinal,
    DM1_V1_D1L2D1R2F0108WallOrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = wall_ornament_ordinal;
    out->has_input_ordinal = wall_ornament_ordinal != 0u;
    out->primary_index = -1;
    out->recursive_footprints_index = -1;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set =
        (wall_ornament_ordinal & DM1_V1_D1L2_D1R2_F0108_WALL_FOOTPRINT_MASK_PC34) != 0u;
    cleared = wall_ornament_ordinal &
        ~DM1_V1_D1L2_D1R2_F0108_WALL_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_ordinal = out->footprint_flag_set ? cleared : wall_ornament_ordinal;
        out->primary_index = (int)out->primary_ordinal - 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D1L2_D1R2_F0108_WALL_FOOTPRINT_INDEX_PC34;
        out->recursive_footprints_ordinal =
            (unsigned int)DM1_V1_D1L2_D1R2_F0108_WALL_FOOTPRINT_INDEX_PC34 + 1u;
    }
    return true;
}

int dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_wall_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    if (coordinate_set < 0 || view_wall < 0) return -1;
    return DM1_WALL_ORNAMENT_ZONE_BASE +
        coordinate_set * DM1_WALL_ORNAMENT_ZONE_STRIDE_PC34 + view_wall;
}

int dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_floor_keepout_zone_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0 || view_floor < 0) return -1;
    return DM1_FLOOR_KEEPOUT_ZONE_BASE +
        coordinate_set * DM1_FLOOR_KEEPOUT_ZONE_STRIDE_PC34 + view_floor;
}

uint8_t dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D1L2_D1R2_F0108_WALL_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_flip_row_pc34(
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

int dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
    int view_square,
    int view_cell,
    int view_depth)
{
    if ((view_square != DM1_D1L_VIEW_SQUARE && view_square != DM1_D1R_VIEW_SQUARE) ||
        view_depth != 1) {
        return 0;
    }
    if (view_square == DM1_D1L_VIEW_SQUARE && view_cell != 1) return 0;
    if (view_square == DM1_D1R_VIEW_SQUARE && view_cell != 0) return 0;
    return 1;
}

bool dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
    const DM1_V1_D1L2D1R2F0108WallStatePc34 *state,
    uint8_t *caller_surface,
    size_t caller_surface_size,
    DM1_V1_D1L2D1R2F0108WallResultPc34 *out)
{
    const DM1_V1_D1L2D1R2F0108WallSpecPc34 *spec;
    DM1_V1_D1L2D1R2F0108WallOrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state || !caller_surface ||
        caller_surface_size < DM1_V1_D1L2_D1R2_F0108_WALL_SURFACE_BYTES_PC34) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    spec = dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_for_pc34(
        state->side, state->route);
    out->spec = spec;
    if (!spec ||
        !state->contract_only ||
        !state->no_real_asset_bitmap_parity ||
        !state->no_game_data_load ||
        state->attempts_f0108_floor_band ||
        state->mutate_thing_list ||
        state->wall_ornament_coordinate_set < 0 ||
        state->floor_ornament_coordinate_set < 0) {
        out->rejected_non_contract_state = 1;
        out->mutation_rejections = state->mutate_thing_list ? 1 : 0;
        return false;
    }

    if (!dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
            spec->view_square, state->view_cell, state->view_depth)) {
        out->rejected_non_contract_state = 1;
        out->row_guard_rejections = 1;
        return false;
    }

    if (!dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_decode_ordinal_pc34(
            state->wall_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->ok = 1;
    out->source_locked_contract_only = 1;
    out->no_real_asset_bitmap_parity = 1;
    out->no_game_data_load = 1;
    out->row_guard_accepts = 1;
    out->f0108_floor_calls = 0;
    out->f0108_floor_keepout_ok = state->floor_ornament_ordinal != 0u;
    out->wall_zone =
        dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_wall_zone_pc34(
            state->wall_ornament_coordinate_set, spec->view_wall);
    out->floor_keepout_zone =
        dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_floor_keepout_zone_pc34(
            state->floor_ornament_coordinate_set, spec->view_floor_keepout);
    out->primary_index = ordinal.primary_index;
    out->recursive_index = ordinal.recursive_footprints_index;
    out->wall_ordinal_zero_skip = ordinal.has_input_ordinal ? 0 : 1;
    out->d1r_horizontal_flip_observed = spec->d1r_horizontal_flip_contract;
    out->f0128_global_flip_observed = spec->f0128_global_flip_contract;
    out->cell_order_transition_ok =
        spec->cell_order_wall_return == DM1_CELL_ORDER_WALL_RETURN &&
        (spec->cell_order_open == DM1_CELL_ORDER_D1L_OPEN ||
         spec->cell_order_open == DM1_CELL_ORDER_D1R_OPEN) &&
        (spec->cell_order_door_pass1 == DM1_CELL_ORDER_D1L_DOOR_PASS1 ||
         spec->cell_order_door_pass1 == DM1_CELL_ORDER_D1R_DOOR_PASS1) &&
        (spec->cell_order_door_pass2 == DM1_CELL_ORDER_D1L_DOOR_PASS2 ||
         spec->cell_order_door_pass2 == DM1_CELL_ORDER_D1R_DOOR_PASS2);

    pixel = state->destination_pixel;
    if (ordinal.has_input_ordinal) {
        out->f0107_wall_calls = 1;
        out->f0107_primary_blits = ordinal.primary_draws ? 1 : 0;
        out->footprint_recursions = ordinal.recursive_footprints_draw ? 1 : 0;
        if (ordinal.primary_draws) {
            pixel = dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_blend_c10_pc34(
                pixel, state->wall_pixel);
        }
    }
    out->after_wall = pixel;
    if (ordinal.recursive_footprints_draw) {
        pixel = dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_blend_c10_pc34(
            pixel, state->footprint_pixel);
    }
    out->after_footprint = pixel;

    caller_surface[0] = out->after_wall;
    caller_surface[1] = out->after_footprint;
    caller_surface[2] = (uint8_t)out->wall_zone;
    caller_surface[3] = (uint8_t)out->floor_keepout_zone;
    out->caller_surface_mutations = 4;

    out->deterministic_hash = mix_hash(state->seed, (uint32_t)out->wall_zone);
    out->deterministic_hash = mix_hash(out->deterministic_hash, (uint32_t)out->floor_keepout_zone);
    out->deterministic_hash = mix_hash(out->deterministic_hash, out->after_wall);
    out->deterministic_hash = mix_hash(out->deterministic_hash, out->after_footprint);
    out->deterministic_hash = mix_hash(out->deterministic_hash, (uint32_t)out->primary_index);
    out->deterministic_hash = mix_hash(out->deterministic_hash, (uint32_t)out->recursive_index);
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

static void self_check_surface_unchanged(SelfTestCounters *c,
                                         const uint8_t *got,
                                         const uint8_t *want,
                                         size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        self_check_eq(c, got[i], want[i]);
    }
}

int run_dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_self_test(void)
{
    SelfTestCounters c = { 0, 0 };
    uint32_t deterministic_hash = 0x717u;
    uint32_t seed = 0x7170108u;
    int i;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));

    self_check_contains(&c, s_source_evidence, "source_locked_contract_only=1");
    self_check_contains(&c, s_source_evidence, "no_real_asset_bitmap_parity=1");
    self_check_contains(&c, s_source_evidence, "no_game_data_load=1");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0107:3502-3938");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0104:3113-3156");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0105:3185-3247");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0108:3940-4011");
    self_check_contains(&c, s_source_evidence, "DUNVIEW.C F0128:8318-8486");
    self_check_contains(&c, s_source_evidence, "DUNGEON.C F0163:1769-1838");
    self_check_contains(&c, s_source_evidence, "F0164:1840-1905");
    self_check_contains(&c, s_source_evidence, "F0172:2466-2523");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2088");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2596-2611");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4139-4153");
    self_check_contains(&c, s_source_evidence, "DEFS.H:4205-4207");
    self_check_contains(&c, s_source_evidence, "DEFS.H:2662/2668-2677");
    self_check_contains(&c, s_source_evidence, "C1500 + CoordinateSet * 11 + ViewFloor");

    self_check_eq(&c, (int)dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_count_pc34(), 4);
    self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_at_pc34(4) == NULL);
    self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_for_pc34(
        (DM1_V1_D1L2D1R2F0108WallSidePc34)9,
        DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_NATIVE_PC34) == NULL);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_wall_zone_pc34(1, 12), 1031);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_wall_zone_pc34(2, 13), 1047);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_floor_keepout_zone_pc34(1, 8), 1519);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_floor_keepout_zone_pc34(2, 10), 1532);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_wall_zone_pc34(-1, 12), -1);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_floor_keepout_zone_pc34(1, -1), -1);

    for (i = 0; i < 4; ++i) {
        const DM1_V1_D1L2D1R2F0108WallSpecPc34 *spec =
            dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_at_pc34((size_t)i);
        DM1_V1_D1L2D1R2F0108WallStatePc34 state;
        DM1_V1_D1L2D1R2F0108WallStatePc34 reject_state;
        DM1_V1_D1L2D1R2F0108WallResultPc34 result;
        DM1_V1_D1L2D1R2F0108WallOrdinalPc34 ordinal;
        uint8_t surface[DM1_V1_D1L2_D1R2_F0108_WALL_SURFACE_BYTES_PC34] =
            { 0xa1u, 0xa2u, 0xa3u, 0xa4u, 0xa5u, 0xa6u, 0xa7u, 0xa8u };
        uint8_t before[DM1_V1_D1L2_D1R2_F0108_WALL_SURFACE_BYTES_PC34];
        uint8_t source[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        uint8_t flipped[8] = { 0 };

        self_check(&c, spec != NULL);
        if (!spec) continue;
        self_check_eq(&c, spec->draw_order_index, i);
        self_check_eq(&c, spec->relative_depth, 1);
        self_check_eq(&c, spec->wall_ornament_zone_base, DM1_WALL_ORNAMENT_ZONE_BASE);
        self_check_eq(&c, spec->wall_ornament_zone_stride_pc34, DM1_WALL_ORNAMENT_ZONE_STRIDE_PC34);
        self_check_eq(&c, spec->floor_keepout_zone_base, DM1_FLOOR_KEEPOUT_ZONE_BASE);
        self_check_eq(&c, spec->floor_keepout_zone_stride_pc34, DM1_FLOOR_KEEPOUT_ZONE_STRIDE_PC34);
        self_check_eq(&c, spec->wall_element, DM1_WALL_ELEMENT);
        self_check_eq(&c, spec->source_locked_contract_only, 1);
        self_check_eq(&c, spec->no_real_asset_bitmap_parity, 1);
        self_check_eq(&c, spec->no_game_data_load, 1);
        self_check(&c, strstr(spec->redmcsb_wall_anchor, "DUNVIEW.C") != NULL);
        self_check(&c, strstr(spec->redmcsb_f0107_anchor, "F0107") != NULL);
        self_check(&c, strstr(spec->redmcsb_f0108_keepout_anchor, "F0108") != NULL);
        self_check(&c, strstr(spec->redmcsb_defs_anchor, "DEFS.H") != NULL);

        if (spec->side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1L2_PC34) {
            self_check_eq(&c, spec->relative_lateral, -1);
            self_check_eq(&c, spec->view_square, DM1_D1L_VIEW_SQUARE);
            self_check_eq(&c, spec->view_wall, DM1_D1L_VIEW_WALL_RIGHT);
            self_check_eq(&c, spec->view_floor_keepout, DM1_D1L_VIEW_FLOOR);
            self_check_eq(&c, spec->wall_zone, DM1_WALL_ZONE_D1L);
            self_check_eq(&c, spec->cell_order_open, DM1_CELL_ORDER_D1L_OPEN);
            self_check_eq(&c, spec->cell_order_door_pass1, DM1_CELL_ORDER_D1L_DOOR_PASS1);
            self_check_eq(&c, spec->cell_order_door_pass2, DM1_CELL_ORDER_D1L_DOOR_PASS2);
        } else {
            self_check_eq(&c, spec->relative_lateral, 1);
            self_check_eq(&c, spec->view_square, DM1_D1R_VIEW_SQUARE);
            self_check_eq(&c, spec->view_wall, DM1_D1R_VIEW_WALL_LEFT);
            self_check_eq(&c, spec->view_floor_keepout, DM1_D1R_VIEW_FLOOR);
            self_check_eq(&c, spec->wall_zone, DM1_WALL_ZONE_D1R);
            self_check_eq(&c, spec->cell_order_open, DM1_CELL_ORDER_D1R_OPEN);
            self_check_eq(&c, spec->cell_order_door_pass1, DM1_CELL_ORDER_D1R_DOOR_PASS1);
            self_check_eq(&c, spec->cell_order_door_pass2, DM1_CELL_ORDER_D1R_DOOR_PASS2);
        }

        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_initial_state_pc34(
            spec->side, spec->route, &state));
        self_check_eq(&c, (int)state.side, (int)spec->side);
        self_check_eq(&c, (int)state.route, (int)spec->route);
        self_check_eq(&c, state.view_depth, 1);
        self_check(&c, state.contract_only);
        self_check(&c, state.no_real_asset_bitmap_parity);
        self_check(&c, state.no_game_data_load);

        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_decode_ordinal_pc34(
            state.wall_ornament_ordinal, &ordinal));
        self_check(&c, ordinal.has_input_ordinal);
        self_check(&c, ordinal.footprint_flag_set);
        self_check(&c, ordinal.primary_draws);
        self_check_eq(&c, ordinal.primary_index,
                      spec->side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 5 : 3);
        self_check_eq(&c, ordinal.recursive_footprints_index, 15);
        self_check_eq(&c, (int)ordinal.recursive_footprints_ordinal, 16);

        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
            &state, surface, sizeof(surface), &result));
        self_check_eq(&c, result.ok, 1);
        self_check_eq(&c, result.source_locked_contract_only, 1);
        self_check_eq(&c, result.no_real_asset_bitmap_parity, 1);
        self_check_eq(&c, result.no_game_data_load, 1);
        self_check_eq(&c, result.f0107_wall_calls, 1);
        self_check_eq(&c, result.f0107_primary_blits, 1);
        self_check_eq(&c, result.footprint_recursions, 1);
        self_check_eq(&c, result.f0108_floor_calls, 0);
        self_check_eq(&c, result.f0108_floor_keepout_ok, 1);
        self_check_eq(&c, result.row_guard_accepts, 1);
        self_check_eq(&c, result.cell_order_transition_ok, 1);
        self_check_eq(&c, result.wall_zone,
                      dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_wall_zone_pc34(
                          state.wall_ornament_coordinate_set, spec->view_wall));
        self_check_eq(&c, result.floor_keepout_zone,
                      dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_floor_keepout_zone_pc34(
                          state.floor_ornament_coordinate_set, spec->view_floor_keepout));
        self_check_eq(&c, result.after_wall, state.wall_pixel);
        self_check_eq(&c, result.after_footprint, state.footprint_pixel);
        self_check_eq(&c, result.caller_surface_mutations, 4);
        self_check_eq(&c, surface[0], state.wall_pixel);
        self_check_eq(&c, surface[1], state.footprint_pixel);
        self_check(&c, result.deterministic_hash != 0u);
        deterministic_hash = mix_hash(deterministic_hash, result.deterministic_hash);
        s_last_self_test.wall_draws += result.f0107_primary_blits;
        s_last_self_test.footprint_recursions += result.footprint_recursions;

        state.wall_ornament_ordinal = 0;
        state.wall_pixel = 0x7bu;
        memset(surface, 0xd4, sizeof(surface));
        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
            &state, surface, sizeof(surface), &result));
        self_check_eq(&c, result.wall_ordinal_zero_skip, 1);
        self_check_eq(&c, result.f0107_wall_calls, 0);
        self_check_eq(&c, result.after_wall, state.destination_pixel);
        self_check_eq(&c, result.f0108_floor_calls, 0);

        state.wall_ornament_ordinal =
            spec->side == DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 ? 6u : 4u;
        state.wall_pixel = DM1_V1_D1L2_D1R2_F0108_WALL_C10_COLOR_FLESH_PC34;
        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
            &state, surface, sizeof(surface), &result));
        self_check_eq(&c, result.f0107_primary_blits, 1);
        self_check_eq(&c, result.after_wall, state.destination_pixel);
        self_check_eq(&c, result.footprint_recursions, 0);

        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_flip_row_pc34(
            source, flipped, 4, 2));
        self_check_eq(&c, flipped[0], 4);
        self_check_eq(&c, flipped[3], 1);
        self_check_eq(&c, flipped[4], 8);
        self_check_eq(&c, flipped[7], 5);
        self_check(&c, !dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_flip_row_pc34(
            NULL, flipped, 4, 2));

        memcpy(before, surface, sizeof(before));
        reject_state = state;
        reject_state.mutate_thing_list = true;
        self_check(&c, !dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
            &reject_state, surface, sizeof(surface), &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
        self_check_eq(&c, result.mutation_rejections, 1);
        self_check_surface_unchanged(&c, surface, before, sizeof(surface));
        ++s_last_self_test.mutation_rejections;

        reject_state = state;
        reject_state.view_cell = 7;
        self_check(&c, !dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
            &reject_state, surface, sizeof(surface), &result));
        self_check_eq(&c, result.rejected_non_contract_state, 1);
        self_check_eq(&c, result.row_guard_rejections, 1);
        self_check_surface_unchanged(&c, surface, before, sizeof(surface));
        ++s_last_self_test.row_guard_rejections;

        reject_state = state;
        reject_state.attempts_f0108_floor_band = true;
        self_check(&c, !dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
            &reject_state, surface, sizeof(surface), &result));
        self_check_eq(&c, result.f0108_floor_calls, 0);
        self_check_surface_unchanged(&c, surface, before, sizeof(surface));
    }

    self_check(&c, !dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_decode_ordinal_pc34(
        1u, NULL));
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_blend_c10_pc34(
        0x44u, DM1_V1_D1L2_D1R2_F0108_WALL_C10_COLOR_FLESH_PC34), 0x44);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_blend_c10_pc34(
        0x44u, 0x55u), 0x55);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
        DM1_D1L_VIEW_SQUARE, 1, 1), 1);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
        DM1_D1R_VIEW_SQUARE, 0, 1), 1);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
        DM1_D1L_VIEW_SQUARE, 0, 1), 0);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
        DM1_D1R_VIEW_SQUARE, 1, 1), 0);
    self_check_eq(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_row_guard_accepts_pc34(
        DM1_D1R_VIEW_SQUARE, 0, 2), 0);

    for (i = 0; i < 48; ++i) {
        DM1_V1_D1L2D1R2F0108WallStatePc34 state;
        DM1_V1_D1L2D1R2F0108WallResultPc34 result;
        uint8_t surface[DM1_V1_D1L2_D1R2_F0108_WALL_SURFACE_BYTES_PC34] = { 0 };
        const DM1_V1_D1L2D1R2F0108WallSidePc34 side =
            (next_lcg(&seed) & 1u) ?
            DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1R2_PC34 :
            DM1_V1_D1L2_D1R2_F0108_WALL_SIDE_D1L2_PC34;
        const DM1_V1_D1L2D1R2F0108WallRoutePc34 route =
            (next_lcg(&seed) & 1u) ?
            DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_F0128_FLIPPED_PC34 :
            DM1_V1_D1L2_D1R2_F0108_WALL_ROUTE_NATIVE_PC34;

        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_initial_state_pc34(
            side, route, &state));
        state.seed = seed;
        state.wall_ornament_ordinal = ((next_lcg(&seed) & 7u) + 1u) |
            ((i & 1) ? DM1_V1_D1L2_D1R2_F0108_WALL_FOOTPRINT_MASK_PC34 : 0u);
        state.wall_pixel = (uint8_t)((next_lcg(&seed) & 0x7fu) + 0x20u);
        if ((i % 9) == 0) state.wall_pixel = DM1_V1_D1L2_D1R2_F0108_WALL_C10_COLOR_FLESH_PC34;
        self_check(&c, dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_compose_pc34(
            &state, surface, sizeof(surface), &result));
        self_check_eq(&c, result.f0108_floor_calls, 0);
        self_check_eq(&c, result.f0108_floor_keepout_ok, 1);
        self_check_eq(&c, result.source_locked_contract_only, 1);
        self_check_eq(&c, result.no_real_asset_bitmap_parity, 1);
        self_check_eq(&c, result.no_game_data_load, 1);
        deterministic_hash = mix_hash(deterministic_hash, result.deterministic_hash);
    }

    s_last_self_test.assertions = c.assertions;
    s_last_self_test.failures = c.failures;
    s_last_self_test.ok = c.failures == 0 && c.assertions >= 150;
    s_last_self_test.deterministic_hash = deterministic_hash;
    return s_last_self_test.ok;
}

const DM1_V1_D1L2D1R2F0108WallSelfTestResultPc34 *
dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_source_evidence_pc34(void)
{
    return s_source_evidence;
}
