/* ReDMCSB DUNVIEW.C F0108:3940-4011 D0C floor-ornament keepout contract. */
#include "dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0C_VIEW_SQUARE = 0,
    DM1_D0C_NO_VIEW_FLOOR = -1,
    DM1_D0C_RELATIVE_DEPTH = 0,
    DM1_D0C_RELATIVE_LATERAL = 0,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_D0C_THING_ORDER = 0x0021,
    DM1_D0C_CELL_ORDER_BAND_START = 800,
    DM1_D0C_CELL_ORDER_BAND_END = 814,
    DM1_WALL_KEEPOUT_ZONE_D3L = 705,
    DM1_WALL_KEEPOUT_ZONE_D3R = 706
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. ReDMCSB anchors: "
    "DUNVIEW.C F0108:3940-4011 floor ornament ordinal gate, "
    "MASK0x8000_FOOTPRINTS recursion, C10 blit, and C1500 + CoordinateSet "
    "* 11 + ViewFloor PC34 zone math; DUNVIEW.C F0107:3502-3938 wall "
    "ornament palette keepout so D0C does not borrow wall-ornament blits; "
    "DUNVIEW.C F0115:4547-4581,F0115:4923,F0115:5180-5188,"
    "F0115:5211-5214 thing-pass cell ordering; DUNVIEW.C F0127:8164-8310 "
    "D0C foreground, ceiling, C0x0021 thing pass, and field route with no "
    "F0108 call; DUNVIEW.C F0128:8318-8486 D0L/D0R then D0C dispatch; "
    "DUNGEON.C F0163:1769-1838 and F0164:1840-1905 thing-list mutation "
    "anchors; DUNGEON.C F0172:2466-2523 square-aspect source; "
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2596-2611 D0C view-square ordinal; "
    "DEFS.H:2662/2668-2677 cell orders; DEFS.H:4045-4046 C705/C706 wall "
    "zones kept out of D0C F0108 composition; DEFS.H:4139-4153 cell-order "
    "zone band; DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT.";

static const DM1_V1_D0CF0108FloorOrnamentSpecPc34 s_specs[] = {
    {
        DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_CORRIDOR_PC34,
        "D0C corridor no-floor-ornament composition",
        1, 0, DM1_D0C_RELATIVE_DEPTH, DM1_D0C_RELATIVE_LATERAL,
        DM1_D0C_VIEW_SQUARE, DM1_D0C_NO_VIEW_FLOOR,
        DM1_FLOOR_ZONE_BASE, DM1_FLOOR_ZONE_STRIDE_PC34, 558,
        0, 0, 0, 1, 0, DM1_D0C_THING_ORDER, DM1_D0C_VIEW_SQUARE,
        1, DM1_WALL_KEEPOUT_ZONE_D3L, DM1_WALL_KEEPOUT_ZONE_D3R,
        DM1_D0C_CELL_ORDER_BAND_START, DM1_D0C_CELL_ORDER_BAND_END,
        1, 1, 1,
        "DUNVIEW.C F0127:8284-8294 no floor-ornament call",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DUNVIEW.C F0107:3502-3938 palette keepout",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223"
    },
    {
        DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_OPEN_PIT_PC34,
        "D0C pit foreground no-floor-ornament composition",
        2, 1, DM1_D0C_RELATIVE_DEPTH, DM1_D0C_RELATIVE_LATERAL,
        DM1_D0C_VIEW_SQUARE, DM1_D0C_NO_VIEW_FLOOR,
        DM1_FLOOR_ZONE_BASE, DM1_FLOOR_ZONE_STRIDE_PC34, 558,
        0, 0, 1, 1, 0, DM1_D0C_THING_ORDER, DM1_D0C_VIEW_SQUARE,
        1, DM1_WALL_KEEPOUT_ZONE_D3L, DM1_WALL_KEEPOUT_ZONE_D3R,
        DM1_D0C_CELL_ORDER_BAND_START, DM1_D0C_CELL_ORDER_BAND_END,
        1, 1, 1,
        "DUNVIEW.C F0127:8274-8294 pit then no floor-ornament call",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DUNVIEW.C F0107:3502-3938 palette keepout",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223"
    },
    {
        DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_TELEPORTER_PC34,
        "D0C teleporter field no-floor-ornament composition",
        5, 2, DM1_D0C_RELATIVE_DEPTH, DM1_D0C_RELATIVE_LATERAL,
        DM1_D0C_VIEW_SQUARE, DM1_D0C_NO_VIEW_FLOOR,
        DM1_FLOOR_ZONE_BASE, DM1_FLOOR_ZONE_STRIDE_PC34, 558,
        0, 0, 0, 1, 1, DM1_D0C_THING_ORDER, DM1_D0C_VIEW_SQUARE,
        1, DM1_WALL_KEEPOUT_ZONE_D3L, DM1_WALL_KEEPOUT_ZONE_D3R,
        DM1_D0C_CELL_ORDER_BAND_START, DM1_D0C_CELL_ORDER_BAND_END,
        1, 1, 1,
        "DUNVIEW.C F0127:8294-8310 thing pass before field, no F0108",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DUNVIEW.C F0107:3502-3938 palette keepout",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223"
    },
    {
        DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_DOOR_SIDE_PC34,
        "D0C door-side foreground no-floor-ornament composition",
        16, 3, DM1_D0C_RELATIVE_DEPTH, DM1_D0C_RELATIVE_LATERAL,
        DM1_D0C_VIEW_SQUARE, DM1_D0C_NO_VIEW_FLOOR,
        DM1_FLOOR_ZONE_BASE, DM1_FLOOR_ZONE_STRIDE_PC34, 558,
        0, 0, 1, 1, 0, DM1_D0C_THING_ORDER, DM1_D0C_VIEW_SQUARE,
        1, DM1_WALL_KEEPOUT_ZONE_D3L, DM1_WALL_KEEPOUT_ZONE_D3R,
        DM1_D0C_CELL_ORDER_BAND_START, DM1_D0C_CELL_ORDER_BAND_END,
        1, 1, 1,
        "DUNVIEW.C F0127:8185-8240 door-side foreground, no F0108",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DUNVIEW.C F0107:3502-3938 palette keepout",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223"
    },
    {
        DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_STAIRS_FRONT_PC34,
        "D0C stairs-front foreground no-floor-ornament composition",
        19, 4, DM1_D0C_RELATIVE_DEPTH, DM1_D0C_RELATIVE_LATERAL,
        DM1_D0C_VIEW_SQUARE, DM1_D0C_NO_VIEW_FLOOR,
        DM1_FLOOR_ZONE_BASE, DM1_FLOOR_ZONE_STRIDE_PC34, 558,
        0, 0, 1, 1, 0, DM1_D0C_THING_ORDER, DM1_D0C_VIEW_SQUARE,
        1, DM1_WALL_KEEPOUT_ZONE_D3L, DM1_WALL_KEEPOUT_ZONE_D3R,
        DM1_D0C_CELL_ORDER_BAND_START, DM1_D0C_CELL_ORDER_BAND_END,
        1, 1, 1,
        "DUNVIEW.C F0127:8241-8273 stairs-front foreground, no F0108",
        "DUNVIEW.C F0108:3940-4011 keepout",
        "DUNVIEW.C F0107:3502-3938 palette keepout",
        "DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214",
        "DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153/4223"
    }
};

static uint32_t fnv1a_u32_pc34_compat(uint32_t hash, uint32_t value)
{
    unsigned int i;

    for (i = 0; i < 4u; ++i) {
        hash ^= (value >> (i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t next_lcg_pc34_compat(uint32_t *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static int context_has_foreground_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentContextPc34 context)
{
    return context == DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_OPEN_PIT_PC34 ||
           context == DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_DOOR_SIDE_PC34 ||
           context == DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_STAIRS_FRONT_PC34;
}

size_t dm1_v1_viewport_d0c_f0108_floor_ornament_count_pc34_compat(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_at_pc34_compat(size_t index)
{
    if (index >= dm1_v1_viewport_d0c_f0108_floor_ornament_count_pc34_compat()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_for_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentContextPc34 context)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d0c_f0108_floor_ornament_count_pc34_compat(); ++i) {
        if (s_specs[i].context == context) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentContextPc34 context,
    DM1_V1_D0CF0108FloorOrnamentStatePc34 *out)
{
    const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d0c_f0108_floor_ornament_for_pc34_compat(context);
    if (!spec) return false;

    out->context = context;
    out->floor_ornament_ordinal = 0x8004u;
    out->floor_ornament_coordinate_set = 1;
    out->destination_pixel = (uint8_t)(0x20u + (uint8_t)spec->draw_order_index);
    out->hypothetical_floor_pixel = (uint8_t)(0x40u + (uint8_t)spec->draw_order_index);
    out->foreground_pixel = (uint8_t)(0x50u + (uint8_t)spec->draw_order_index);
    out->ceiling_pixel = (uint8_t)(0x60u + (uint8_t)spec->draw_order_index);
    out->thing_pass_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
    out->field_pixel = (uint8_t)(0x70u + (uint8_t)spec->draw_order_index);
    out->seed = 0x7200108u + (uint32_t)spec->draw_order_index;
    out->teleporter_visible =
        context == DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_TELEPORTER_PC34;
    out->contract_only = true;
    out->no_real_asset_bitmap_parity = true;
    out->no_game_data_load = true;
    return true;
}

bool dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D0CF0108FloorOrnamentOrdinalPc34 *out)
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
         DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_ordinal = out->footprint_flag_set ? cleared : floor_ornament_ordinal;
        out->primary_index = (int)out->primary_ordinal - 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34;
        out->recursive_footprints_ordinal =
            (unsigned int)DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34 + 1u;
    }
    return true;
}

int dm1_v1_viewport_d0c_f0108_floor_ornament_zone_pc34_compat(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0 || view_floor < 0) return -1;
    return DM1_FLOOR_ZONE_BASE + coordinate_set * DM1_FLOOR_ZONE_STRIDE_PC34 + view_floor;
}

uint8_t dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d0c_f0108_floor_ornament_flip_row_pc34_compat(
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

bool dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
    const DM1_V1_D0CF0108FloorOrnamentStatePc34 *state,
    uint8_t *caller_surface,
    size_t caller_surface_size,
    DM1_V1_D0CF0108FloorOrnamentResultPc34 *out)
{
    const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *spec;
    DM1_V1_D0CF0108FloorOrnamentOrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state || !caller_surface ||
        caller_surface_size < DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    spec = dm1_v1_viewport_d0c_f0108_floor_ornament_for_pc34_compat(state->context);
    out->spec = spec;
    if (!spec ||
        !state->contract_only ||
        !state->no_real_asset_bitmap_parity ||
        !state->no_game_data_load ||
        state->attempts_f0108_floor_ornament ||
        state->attempts_f0107_wall_ornament ||
        state->mutate_thing_list ||
        state->floor_ornament_coordinate_set < 0) {
        out->rejected_non_contract_state = 1;
        out->mutation_rejections = state->mutate_thing_list ? 1 : 0;
        return false;
    }

    if (!dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(
            state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->ok = 1;
    out->source_locked_contract_only = 1;
    out->no_real_asset_bitmap_parity = 1;
    out->no_game_data_load = 1;
    out->f0108_floor_calls = 0;
    out->f0108_primary_blits = 0;
    out->f0108_footprint_recursions = 0;
    out->f0108_d0c_no_view_floor_keepout_ok = spec->view_floor == DM1_D0C_NO_VIEW_FLOOR;
    out->f0107_palette_keepout_ok = spec->wall_ornament_palette_keepout;
    out->thing_pass_calls = 1;
    out->foreground_calls = context_has_foreground_pc34_compat(state->context);
    out->ceiling_calls = 1;
    out->field_calls = state->teleporter_visible ? 1 : 0;
    out->floor_zone =
        dm1_v1_viewport_d0c_f0108_floor_ornament_zone_pc34_compat(
            state->floor_ornament_coordinate_set, spec->view_floor);
    out->floor_primary_index = ordinal.primary_index;
    out->floor_recursive_index = ordinal.recursive_footprints_index;
    out->cell_order_transition_ok =
        spec->thing_pass_order == DM1_D0C_THING_ORDER &&
        spec->cell_order_band_start == DM1_D0C_CELL_ORDER_BAND_START &&
        spec->cell_order_band_end == DM1_D0C_CELL_ORDER_BAND_END;

    pixel = state->destination_pixel;
    if (out->foreground_calls) {
        pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
            pixel, state->foreground_pixel);
    }
    out->after_foreground = pixel;
    pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
        pixel, state->ceiling_pixel);
    out->after_ceiling = pixel;
    pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
        pixel, state->thing_pass_pixel);
    out->after_thing_pass = pixel;
    if (out->field_calls) {
        pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
            pixel, state->field_pixel);
    }
    out->after_field = pixel;

    caller_surface[0] = out->after_foreground;
    caller_surface[1] = out->after_ceiling;
    caller_surface[2] = out->after_thing_pass;
    caller_surface[3] = out->after_field;
    caller_surface[4] = (uint8_t)spec->thing_pass_order;
    caller_surface[5] = (uint8_t)spec->view_square;
    out->caller_surface_mutations = 6;

    out->deterministic_hash = fnv1a_u32_pc34_compat(state->seed, (uint32_t)spec->draw_order_index);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, (uint32_t)out->floor_zone);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, (uint32_t)out->floor_primary_index);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, (uint32_t)out->floor_recursive_index);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, out->after_field);
    return true;
}

typedef struct {
    int assertions;
    int failures;
} SelfTestCountersPc34Compat;

static void self_check_pc34_compat(SelfTestCountersPc34Compat *c, int condition)
{
    ++c->assertions;
    if (!condition) ++c->failures;
}

static void self_check_eq_pc34_compat(SelfTestCountersPc34Compat *c, int got, int want)
{
    ++c->assertions;
    if (got != want) ++c->failures;
}

static void self_check_contains_pc34_compat(
    SelfTestCountersPc34Compat *c,
    const char *haystack,
    const char *needle)
{
    ++c->assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) ++c->failures;
}

int run_dm1_v1_viewport_d0c_f0108_floor_ornament_self_test_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34 *out)
{
    SelfTestCountersPc34Compat c = { 0, 0 };
    DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34 local;
    uint32_t deterministic_hash = 2166136261u;
    uint32_t seed = 0x7200108u;
    size_t i;

    if (!out) return 0;
    memset(&local, 0, sizeof(local));

    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0108:3940-4011");
    self_check_contains_pc34_compat(&c, s_source_evidence, "MASK0x8000_FOOTPRINTS");
    self_check_contains_pc34_compat(&c, s_source_evidence, "C1500 + CoordinateSet * 11 + ViewFloor");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0107:3502-3938");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0115:4547-4581");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0115:4923");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0115:5180-5188");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0115:5211-5214");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0127:8164-8310");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0128:8318-8486");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNGEON.C F0163:1769-1838");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0164:1840-1905");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0172:2466-2523");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:2088");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:2596-2611");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:2662/2668-2677");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:4045-4046");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:4139-4153");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:4223");
    self_check_contains_pc34_compat(&c, s_source_evidence, "source_locked_contract_only=1");

    self_check_eq_pc34_compat(&c,
        (int)dm1_v1_viewport_d0c_f0108_floor_ornament_count_pc34_compat(), 5);
    self_check_pc34_compat(&c, dm1_v1_viewport_d0c_f0108_floor_ornament_at_pc34_compat(5) == NULL);
    self_check_pc34_compat(&c, dm1_v1_viewport_d0c_f0108_floor_ornament_for_pc34_compat(
        (DM1_V1_D0CF0108FloorOrnamentContextPc34)7) == NULL);
    self_check_eq_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_zone_pc34_compat(1, -1), -1);
    self_check_eq_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_zone_pc34_compat(1, 8), 1519);
    self_check_eq_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(0x22u, 10u), 0x22);
    self_check_eq_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(0x22u, 0x33u), 0x33);

    for (i = 0; i < dm1_v1_viewport_d0c_f0108_floor_ornament_count_pc34_compat(); ++i) {
        const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *spec =
            dm1_v1_viewport_d0c_f0108_floor_ornament_at_pc34_compat(i);
        DM1_V1_D0CF0108FloorOrnamentStatePc34 state;
        DM1_V1_D0CF0108FloorOrnamentStatePc34 reject_state;
        DM1_V1_D0CF0108FloorOrnamentResultPc34 result;
        DM1_V1_D0CF0108FloorOrnamentOrdinalPc34 ordinal;
        uint8_t surface[DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34] =
            { 0xa1u, 0xa2u, 0xa3u, 0xa4u, 0xa5u, 0xa6u, 0xa7u, 0xa8u };
        uint8_t source[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        uint8_t flipped[8] = { 0 };

        self_check_pc34_compat(&c, spec != NULL);
        if (!spec) continue;
        self_check_eq_pc34_compat(&c, spec->draw_order_index, (int)i);
        self_check_eq_pc34_compat(&c, spec->relative_depth, 0);
        self_check_eq_pc34_compat(&c, spec->relative_lateral, 0);
        self_check_eq_pc34_compat(&c, spec->view_square, DM1_D0C_VIEW_SQUARE);
        self_check_eq_pc34_compat(&c, spec->view_floor, DM1_D0C_NO_VIEW_FLOOR);
        self_check_eq_pc34_compat(&c, spec->floor_zone_base, DM1_FLOOR_ZONE_BASE);
        self_check_eq_pc34_compat(&c, spec->floor_zone_stride_pc34, DM1_FLOOR_ZONE_STRIDE_PC34);
        self_check_eq_pc34_compat(&c, spec->floor_ornament_aspect_slot, 558);
        self_check_eq_pc34_compat(&c, spec->calls_f0108_floor_ornament, 0);
        self_check_eq_pc34_compat(&c, spec->reads_floor_ornament_slot, 0);
        self_check_eq_pc34_compat(&c, spec->ceiling_before_thing_pass, 1);
        self_check_eq_pc34_compat(&c, spec->thing_pass_order, DM1_D0C_THING_ORDER);
        self_check_eq_pc34_compat(&c, spec->thing_pass_view_square, DM1_D0C_VIEW_SQUARE);
        self_check_eq_pc34_compat(&c, spec->wall_ornament_palette_keepout, 1);
        self_check_eq_pc34_compat(&c, spec->wall_keepout_zone_left, DM1_WALL_KEEPOUT_ZONE_D3L);
        self_check_eq_pc34_compat(&c, spec->wall_keepout_zone_right, DM1_WALL_KEEPOUT_ZONE_D3R);
        self_check_eq_pc34_compat(&c, spec->cell_order_band_start, DM1_D0C_CELL_ORDER_BAND_START);
        self_check_eq_pc34_compat(&c, spec->cell_order_band_end, DM1_D0C_CELL_ORDER_BAND_END);
        self_check_eq_pc34_compat(&c, spec->source_locked_contract_only, 1);
        self_check_eq_pc34_compat(&c, spec->no_real_asset_bitmap_parity, 1);
        self_check_eq_pc34_compat(&c, spec->no_game_data_load, 1);
        self_check_pc34_compat(&c, strstr(spec->redmcsb_f0127_anchor, "F0127") != NULL);
        self_check_pc34_compat(&c, strstr(spec->redmcsb_f0108_anchor, "F0108") != NULL);
        self_check_pc34_compat(&c, strstr(spec->redmcsb_f0107_anchor, "F0107") != NULL);
        self_check_pc34_compat(&c, strstr(spec->redmcsb_f0115_anchor, "F0115") != NULL);
        self_check_pc34_compat(&c, strstr(spec->redmcsb_defs_anchor, "DEFS.H") != NULL);

        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(
                spec->context, &state));
        self_check_eq_pc34_compat(&c, (int)state.context, (int)spec->context);
        self_check_eq_pc34_compat(&c, state.floor_ornament_coordinate_set, 1);
        self_check_pc34_compat(&c, state.contract_only);
        self_check_pc34_compat(&c, state.no_real_asset_bitmap_parity);
        self_check_pc34_compat(&c, state.no_game_data_load);

        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(
                state.floor_ornament_ordinal, &ordinal));
        self_check_pc34_compat(&c, ordinal.has_input_ordinal);
        self_check_pc34_compat(&c, ordinal.footprint_flag_set);
        self_check_pc34_compat(&c, ordinal.primary_draws);
        self_check_eq_pc34_compat(&c, ordinal.primary_index, 3);
        self_check_eq_pc34_compat(&c, ordinal.recursive_footprints_index, 15);
        self_check_eq_pc34_compat(&c, (int)ordinal.recursive_footprints_ordinal, 16);

        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
                &state, surface, sizeof(surface), &result));
        self_check_eq_pc34_compat(&c, result.ok, 1);
        self_check_eq_pc34_compat(&c, result.source_locked_contract_only, 1);
        self_check_eq_pc34_compat(&c, result.no_real_asset_bitmap_parity, 1);
        self_check_eq_pc34_compat(&c, result.no_game_data_load, 1);
        self_check_eq_pc34_compat(&c, result.f0108_floor_calls, 0);
        self_check_eq_pc34_compat(&c, result.f0108_primary_blits, 0);
        self_check_eq_pc34_compat(&c, result.f0108_footprint_recursions, 0);
        self_check_eq_pc34_compat(&c, result.f0108_d0c_no_view_floor_keepout_ok, 1);
        self_check_eq_pc34_compat(&c, result.f0107_palette_keepout_ok, 1);
        self_check_eq_pc34_compat(&c, result.thing_pass_calls, 1);
        self_check_eq_pc34_compat(&c, result.foreground_calls,
            context_has_foreground_pc34_compat(spec->context));
        self_check_eq_pc34_compat(&c, result.ceiling_calls, 1);
        self_check_eq_pc34_compat(&c, result.field_calls,
            spec->context == DM1_V1_D0C_F0108_FLOOR_ORNAMENT_CONTEXT_TELEPORTER_PC34);
        self_check_eq_pc34_compat(&c, result.floor_zone, -1);
        self_check_eq_pc34_compat(&c, result.floor_primary_index, 3);
        self_check_eq_pc34_compat(&c, result.floor_recursive_index, 15);
        self_check_eq_pc34_compat(&c, result.cell_order_transition_ok, 1);
        self_check_eq_pc34_compat(&c, result.caller_surface_mutations, 6);
        self_check_eq_pc34_compat(&c, surface[0], result.after_foreground);
        self_check_eq_pc34_compat(&c, surface[1], result.after_ceiling);
        self_check_eq_pc34_compat(&c, surface[2], result.after_thing_pass);
        self_check_eq_pc34_compat(&c, surface[3], result.after_field);
        self_check_pc34_compat(&c, result.deterministic_hash != 0u);
        deterministic_hash = fnv1a_u32_pc34_compat(deterministic_hash, result.deterministic_hash);
        local.d0c_keepout_compositions += result.f0108_d0c_no_view_floor_keepout_ok;
        local.thing_pass_calls += result.thing_pass_calls;
        local.palette_keepouts += result.f0107_palette_keepout_ok;

        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_flip_row_pc34_compat(
                source, flipped, 4, 2));
        self_check_eq_pc34_compat(&c, flipped[0], 4);
        self_check_eq_pc34_compat(&c, flipped[3], 1);
        self_check_eq_pc34_compat(&c, flipped[4], 8);
        self_check_eq_pc34_compat(&c, flipped[7], 5);
        self_check_pc34_compat(&c,
            !dm1_v1_viewport_d0c_f0108_floor_ornament_flip_row_pc34_compat(
                NULL, flipped, 4, 2));

        reject_state = state;
        reject_state.attempts_f0108_floor_ornament = true;
        self_check_pc34_compat(&c,
            !dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
                &reject_state, surface, sizeof(surface), &result));
        self_check_eq_pc34_compat(&c, result.rejected_non_contract_state, 1);

        reject_state = state;
        reject_state.attempts_f0107_wall_ornament = true;
        self_check_pc34_compat(&c,
            !dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
                &reject_state, surface, sizeof(surface), &result));
        self_check_eq_pc34_compat(&c, result.rejected_non_contract_state, 1);

        reject_state = state;
        reject_state.mutate_thing_list = true;
        self_check_pc34_compat(&c,
            !dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
                &reject_state, surface, sizeof(surface), &result));
        self_check_eq_pc34_compat(&c, result.rejected_non_contract_state, 1);
        self_check_eq_pc34_compat(&c, result.mutation_rejections, 1);
        ++local.mutation_rejections;
    }

    self_check_pc34_compat(&c,
        !dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(0, NULL));
    for (i = 0; i < 32u; ++i) {
        DM1_V1_D0CF0108FloorOrnamentStatePc34 state;
        DM1_V1_D0CF0108FloorOrnamentResultPc34 result;
        uint8_t surface[DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34] = { 0 };
        const size_t spec_index = next_lcg_pc34_compat(&seed) %
            dm1_v1_viewport_d0c_f0108_floor_ornament_count_pc34_compat();
        const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *spec =
            dm1_v1_viewport_d0c_f0108_floor_ornament_at_pc34_compat(spec_index);

        self_check_pc34_compat(&c, spec != NULL);
        if (!spec) continue;
        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(
                spec->context, &state));
        state.seed = seed;
        state.floor_ornament_ordinal = ((next_lcg_pc34_compat(&seed) & 7u) + 1u) |
            ((i & 1u) ? DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34 : 0u);
        state.ceiling_pixel = (uint8_t)((next_lcg_pc34_compat(&seed) & 0x7fu) + 0x30u);
        if ((i % 5u) == 0u) {
            state.ceiling_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
        }
        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
                &state, surface, sizeof(surface), &result));
        self_check_eq_pc34_compat(&c, result.f0108_floor_calls, 0);
        self_check_eq_pc34_compat(&c, result.f0108_d0c_no_view_floor_keepout_ok, 1);
        self_check_eq_pc34_compat(&c, result.thing_pass_calls, 1);
        deterministic_hash = fnv1a_u32_pc34_compat(deterministic_hash, result.deterministic_hash);
    }

    local.assertions = c.assertions;
    local.failures = c.failures;
    local.ok = c.failures == 0;
    local.deterministic_hash = deterministic_hash;
    *out = local;
    return local.ok;
}

const char *dm1_v1_viewport_d0c_f0108_floor_ornament_source_evidence_pc34_compat(void)
{
    return s_source_evidence;
}
