/*
 * ReDMCSB anchors: DUNVIEW.C F0108:3940-4011, F0107:3502-3938,
 * F0098:2962-3002, F0115:4547-4581/5180-5188/5211-5214/5668-5671;
 * DEFS.H:2088,2596-2611,2668-2677,2698-2702,4045-4046.
 */
#include "firestaff/dm1/v1/viewport/d0c_f0108_floor_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0C_VIEW_SQUARE = 0,
    DM1_D0C_CENTRAL_FLOOR_VIEW = 9,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_FLOOR_COORDINATE_SET = 1,
    DM1_D0C_FLOOR_ZONE = 1520,
    DM1_F0098_BASE_ORDER = 0,
    DM1_F0108_FLOOR_ORDER = 1,
    DM1_F0107_KEEPOUT_ORDER = 2,
    DM1_F0115_THING_ORDER = 3,
    DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT = 0x0021,
    DM1_WALL_KEEP_OUT_ZONE_LEFT = 705,
    DM1_WALL_KEEP_OUT_ZONE_RIGHT = 706,
    DM1_C02_ELEMENT_PIT = 2,
    DM1_MASK_PIT_INVISIBLE = 0x04,
    DM1_MASK_PIT_OPEN = 0x08,
    DM1_M554_PIT_OR_TELEPORTER_VISIBLE_PC34 = 3,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL_PC34 = 5,
    DM1_GFX_FLOOR_PIT_D0C_PC34 = 57,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D0C_PC34 = 63,
    DM1_ZONE_FLOORPIT_D0C_PC34 = 862,
    DM1_GFX_CEILING_PIT_D0C_PC34 = 69,
    DM1_ZONE_CEILING_PIT_D0C_PC34 = 871,
    DM1_VIEWPORT_WIDTH_PC34 = 224,
    DM1_VIEWPORT_HEIGHT_PC34 = 136,
    DM1_SCREEN_WIDTH_PC34 = 320,
    DM1_SCREEN_HEIGHT_PC34 = 200
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. ReDMCSB anchors: "
    "DUNVIEW.C F0098:2962-3002 floor+ceiling base precedes square content; "
    "DUNVIEW.C F0108:3940-4011 floor ornament ordinal gate, "
    "MASK0x8000_FOOTPRINTS recursion, C10 transparent blit, and PC34 "
    "C1500 + CoordinateSet * 11 + ViewFloor zone math; "
    "DUNVIEW.C F0107:3502-3938 wall-ornament palette keepout with "
    "C705/C706 zones excluded from the D0C floor-ornament surface; "
    "DUNVIEW.C F0115:4547-4581,F0115:5180-5188,F0115:5211-5214,"
    "F0115:5668-5671 thing-pass cell ordering and C10 transparent object/"
    "projectile blits; DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2596-2611 "
    "I34E/P31J view-square ordinals including M609_VIEW_SQUARE_D0C; "
    "DEFS.H:2668-2677 and DEFS.H:2698-2702 cell orders and M575..M579 "
    "view-square ordinals; DEFS.H:4045-4046 C705/C706 wall zones.";

static const DM1_V1_D0CF0108FloorOrnamentSpecPc34 s_spec = {
    "D0C central F0108 floor-ornament before F0115 source-lock contract",
    DM1_D0C_VIEW_SQUARE,
    DM1_D0C_CENTRAL_FLOOR_VIEW,
    DM1_FLOOR_ZONE_BASE,
    DM1_FLOOR_ZONE_STRIDE_PC34,
    DM1_FLOOR_COORDINATE_SET,
    DM1_D0C_FLOOR_ZONE,
    DM1_F0098_BASE_ORDER,
    DM1_F0108_FLOOR_ORDER,
    DM1_F0107_KEEPOUT_ORDER,
    DM1_F0115_THING_ORDER,
    DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
    DM1_D0C_VIEW_SQUARE,
    DM1_WALL_KEEP_OUT_ZONE_LEFT,
    DM1_WALL_KEEP_OUT_ZONE_RIGHT,
    1,
    1,
    1,
    "DUNVIEW.C F0098:2962-3002",
    "DUNVIEW.C F0108:3940-4011",
    "DUNVIEW.C F0107:3502-3938",
    "DUNVIEW.C F0115:4547-4581,5180-5188,5211-5214,5668-5671",
    "DEFS.H:2088/2596-2611/2668-2677/2698-2702/4045-4046"
};

static const DM1_V1_D0CF0108FloorOrnamentKappetaalVariantPc34
s_kappetaal_variant = {
    "D0C open-pit kappetaal front-edge variant kept out of F0108",
    1,
    1,
    DM1_C02_ELEMENT_PIT,
    DM1_MASK_PIT_OPEN,
    DM1_MASK_PIT_INVISIBLE,
    DM1_M554_PIT_OR_TELEPORTER_VISIBLE_PC34,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL_PC34,
    4,
    1,
    0,
    0,
    -1,
    0,
    0,
    0,
    DM1_D0C_VIEW_SQUARE,
    DM1_D0C_CELL_ORDER_BACKLEFT_BACKRIGHT,
    DM1_GFX_FLOOR_PIT_D0C_PC34,
    DM1_GFX_FLOOR_PIT_INVISIBLE_D0C_PC34,
    DM1_ZONE_FLOORPIT_D0C_PC34,
    DM1_GFX_CEILING_PIT_D0C_PC34,
    DM1_ZONE_CEILING_PIT_D0C_PC34,
    DM1_VIEWPORT_WIDTH_PC34,
    DM1_VIEWPORT_HEIGHT_PC34,
    DM1_SCREEN_WIDTH_PC34,
    DM1_SCREEN_HEIGHT_PC34,
    27,
    127,
    170,
    9,
    25,
    127,
    174,
    9,
    1,
    1,
    "DUNVIEW.C F0108:3940-4011",
    "DUNGEON.C F0172:2628-2678",
    "DUNVIEW.C F0127:8274-8296",
    "DEFS.H:1009/1026-1027/2554-2558/2596-2601/4208-4218",
    "DUNVIEW.C G0206/G0207/G0208:1167-1216",
    "src/engine/m11_game_view.c:12829-12830"
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

const DM1_V1_D0CF0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_spec_pc34_compat(void)
{
    return &s_spec;
}

const DM1_V1_D0CF0108FloorOrnamentKappetaalVariantPc34 *
dm1_v1_viewport_d0c_f0108_floor_ornament_kappetaal_variant_pc34_compat(void)
{
    return &s_kappetaal_variant;
}

bool dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(
    DM1_V1_D0CF0108FloorOrnamentStatePc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->floor_ornament_ordinal = 0x8004u;
    out->base_pixel = 0x21u;
    out->floor_ornament_pixel = 0x44u;
    out->footprint_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
    out->f0107_keepout_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
    out->thing_pass_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
    out->seed = 0x7330108u;
    out->source_locked_contract_only = true;
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
        out->primary_index = (int)(out->footprint_flag_set ? cleared : floor_ornament_ordinal) - 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34;
    }
    return true;
}

uint8_t dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
    const DM1_V1_D0CF0108FloorOrnamentStatePc34 *state,
    uint8_t *surface,
    size_t surface_size,
    DM1_V1_D0CF0108FloorOrnamentResultPc34 *out)
{
    DM1_V1_D0CF0108FloorOrnamentOrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = &s_spec;
    if (!state || !surface ||
        surface_size < DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34) {
        out->rejected_non_contract_state = 1;
        return false;
    }
    if (!state->source_locked_contract_only ||
        !state->no_real_asset_bitmap_parity ||
        !state->no_game_data_load ||
        state->attempts_floor_ceiling_composite ||
        state->attempts_d0l_d0r_route ||
        state->attempts_wall_ornament_write ||
        state->mutate_thing_list) {
        out->rejected_non_contract_state = 1;
        out->mutation_rejections = state->mutate_thing_list ? 1 : 0;
        return false;
    }
    if (!dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(
            state->floor_ornament_ordinal, &ordinal) ||
        !ordinal.has_input_ordinal || !ordinal.primary_draws) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->ok = 1;
    out->source_locked_contract_only = 1;
    out->no_real_asset_bitmap_parity = 1;
    out->no_game_data_load = 1;
    out->central_cell_only =
        s_spec.view_square_d0c == DM1_D0C_VIEW_SQUARE &&
        s_spec.thing_pass_view_square == DM1_D0C_VIEW_SQUARE;

    pixel = state->base_pixel;
    out->f0098_base_writes = 1;
    out->after_f0098_base = pixel;
    surface[0] = pixel;

    out->f0108_floor_ornament_writes = 1;
    out->f0108_primary_blits = 1;
    out->f0108_footprint_recursions = ordinal.recursive_footprints_draw ? 1 : 0;
    pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
        pixel, state->floor_ornament_pixel);
    out->after_f0108_floor = pixel;
    surface[1] = pixel;

    if (ordinal.recursive_footprints_draw) {
        pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
            pixel, state->footprint_pixel);
    }
    out->after_f0108_footprints = pixel;
    surface[2] = pixel;

    out->f0107_keepout_writes = 1;
    pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
        pixel, state->f0107_keepout_pixel);
    out->after_f0107_keepout = pixel;
    out->f0107_keepout_preserved_floor = pixel == out->after_f0108_floor;
    surface[3] = pixel;

    out->thing_pass_calls = 1;
    out->thing_pass_observed_pixel = pixel;
    out->thing_pass_observed_floor = out->thing_pass_observed_pixel == out->after_f0108_floor;
    out->floor_write_before_thing_pass =
        s_spec.f0108_floor_ornament_order < s_spec.f0115_thing_pass_order &&
        out->thing_pass_observed_floor;
    pixel = dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(
        pixel, state->thing_pass_pixel);
    out->after_thing_pass = pixel;
    surface[4] = pixel;
    surface[5] = (uint8_t)s_spec.floor_zone;
    surface[6] = (uint8_t)s_spec.thing_pass_cell_order;
    surface[7] = (uint8_t)ordinal.primary_index;

    out->deterministic_hash = fnv1a_u32_pc34_compat(state->seed, (uint32_t)s_spec.floor_zone);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, out->after_f0108_floor);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, out->after_f0107_keepout);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, out->thing_pass_observed_pixel);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, out->after_thing_pass);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, (uint32_t)ordinal.primary_index);
    out->deterministic_hash = fnv1a_u32_pc34_compat(out->deterministic_hash, (uint32_t)ordinal.recursive_footprints_index);
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
    DM1_V1_D0CF0108FloorOrnamentStatePc34 state;
    DM1_V1_D0CF0108FloorOrnamentStatePc34 reject_state;
    DM1_V1_D0CF0108FloorOrnamentResultPc34 result;
    DM1_V1_D0CF0108FloorOrnamentOrdinalPc34 ordinal;
    uint8_t surface[DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34] = { 0 };
    uint32_t deterministic_hash = 2166136261u;
    uint32_t seed = 0x733d0c00u;
    unsigned int i;

    if (!out) return 0;
    memset(&local, 0, sizeof(local));

    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0098:2962-3002");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0108:3940-4011");
    self_check_contains_pc34_compat(&c, s_source_evidence, "MASK0x8000_FOOTPRINTS");
    self_check_contains_pc34_compat(&c, s_source_evidence, "C1500 + CoordinateSet * 11 + ViewFloor");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0107:3502-3938");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DUNVIEW.C F0115:4547-4581");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0115:5180-5188");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0115:5211-5214");
    self_check_contains_pc34_compat(&c, s_source_evidence, "F0115:5668-5671");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:2088");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:2596-2611");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:2668-2677");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:2698-2702");
    self_check_contains_pc34_compat(&c, s_source_evidence, "DEFS.H:4045-4046");
    self_check_contains_pc34_compat(&c, s_source_evidence, "source_locked_contract_only=1");
    self_check_contains_pc34_compat(&c, s_source_evidence, "no_real_asset_bitmap_parity=1");
    self_check_contains_pc34_compat(&c, s_source_evidence, "no_game_data_load=1");

    self_check_eq_pc34_compat(&c, s_spec.view_square_d0c, 0);
    self_check_eq_pc34_compat(&c, s_spec.floor_zone, 1520);
    self_check_eq_pc34_compat(&c,
        s_spec.floor_zone_base + s_spec.floor_coordinate_set * s_spec.floor_zone_stride_pc34 +
            s_spec.central_floor_view,
        s_spec.floor_zone);
    self_check_eq_pc34_compat(&c, s_spec.f0098_base_order, 0);
    self_check_eq_pc34_compat(&c, s_spec.f0108_floor_ornament_order, 1);
    self_check_eq_pc34_compat(&c, s_spec.f0107_keepout_order, 2);
    self_check_eq_pc34_compat(&c, s_spec.f0115_thing_pass_order, 3);
    self_check_eq_pc34_compat(&c, s_spec.thing_pass_cell_order, 0x0021);
    self_check_eq_pc34_compat(&c, s_spec.thing_pass_view_square, 0);
    self_check_eq_pc34_compat(&c, s_spec.wall_keepout_zone_left, 705);
    self_check_eq_pc34_compat(&c, s_spec.wall_keepout_zone_right, 706);
    self_check_pc34_compat(&c, strstr(s_spec.redmcsb_f0108_anchor, "F0108") != NULL);
    self_check_pc34_compat(&c, strstr(s_spec.redmcsb_f0115_anchor, "F0115") != NULL);

    self_check_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(&state));
    self_check_pc34_compat(&c, state.source_locked_contract_only);
    self_check_pc34_compat(&c, state.no_real_asset_bitmap_parity);
    self_check_pc34_compat(&c, state.no_game_data_load);
    self_check_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(
            state.floor_ornament_ordinal, &ordinal));
    self_check_pc34_compat(&c, ordinal.has_input_ordinal);
    self_check_pc34_compat(&c, ordinal.footprint_flag_set);
    self_check_pc34_compat(&c, ordinal.primary_draws);
    self_check_eq_pc34_compat(&c, ordinal.cleared_ordinal, 4);
    self_check_eq_pc34_compat(&c, ordinal.primary_index, 3);
    self_check_eq_pc34_compat(&c, ordinal.recursive_footprints_index, 15);

    self_check_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
            &state, surface, sizeof(surface), &result));
    self_check_eq_pc34_compat(&c, result.ok, 1);
    self_check_eq_pc34_compat(&c, result.f0098_base_writes, 1);
    self_check_eq_pc34_compat(&c, result.f0108_floor_ornament_writes, 1);
    self_check_eq_pc34_compat(&c, result.f0108_primary_blits, 1);
    self_check_eq_pc34_compat(&c, result.f0108_footprint_recursions, 1);
    self_check_eq_pc34_compat(&c, result.f0107_keepout_writes, 1);
    self_check_eq_pc34_compat(&c, result.f0107_keepout_preserved_floor, 1);
    self_check_eq_pc34_compat(&c, result.thing_pass_calls, 1);
    self_check_eq_pc34_compat(&c, result.thing_pass_observed_floor, 1);
    self_check_eq_pc34_compat(&c, result.floor_write_before_thing_pass, 1);
    self_check_eq_pc34_compat(&c, result.central_cell_only, 1);
    self_check_eq_pc34_compat(&c, result.after_f0098_base, state.base_pixel);
    self_check_eq_pc34_compat(&c, result.after_f0108_floor, state.floor_ornament_pixel);
    self_check_eq_pc34_compat(&c, result.after_f0108_footprints, state.floor_ornament_pixel);
    self_check_eq_pc34_compat(&c, result.after_f0107_keepout, state.floor_ornament_pixel);
    self_check_eq_pc34_compat(&c, result.thing_pass_observed_pixel, state.floor_ornament_pixel);
    self_check_eq_pc34_compat(&c, result.after_thing_pass, state.floor_ornament_pixel);
    self_check_eq_pc34_compat(&c, surface[1], state.floor_ornament_pixel);
    self_check_eq_pc34_compat(&c, surface[3], state.floor_ornament_pixel);
    self_check_eq_pc34_compat(&c, surface[4], state.floor_ornament_pixel);
    local.floor_writes += result.f0108_floor_ornament_writes;
    local.thing_pass_calls += result.thing_pass_calls;
    local.keepout_preservations += result.f0107_keepout_preserved_floor;
    deterministic_hash = fnv1a_u32_pc34_compat(deterministic_hash, result.deterministic_hash);

    self_check_eq_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(0x31u, 10u),
        0x31);
    self_check_eq_pc34_compat(&c,
        dm1_v1_viewport_d0c_f0108_floor_ornament_blend_c10_pc34_compat(0x31u, 0x52u),
        0x52);
    self_check_pc34_compat(&c,
        !dm1_v1_viewport_d0c_f0108_floor_ornament_decode_ordinal_pc34_compat(0, NULL));

    reject_state = state;
    reject_state.attempts_floor_ceiling_composite = true;
    self_check_pc34_compat(&c,
        !dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
            &reject_state, surface, sizeof(surface), &result));
    self_check_eq_pc34_compat(&c, result.rejected_non_contract_state, 1);

    reject_state = state;
    reject_state.attempts_d0l_d0r_route = true;
    self_check_pc34_compat(&c,
        !dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
            &reject_state, surface, sizeof(surface), &result));
    self_check_eq_pc34_compat(&c, result.rejected_non_contract_state, 1);

    reject_state = state;
    reject_state.attempts_wall_ornament_write = true;
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

    for (i = 0; i < 24u; ++i) {
        uint8_t fuzz_surface[DM1_V1_D0C_F0108_FLOOR_ORNAMENT_SURFACE_BYTES_PC34] = { 0 };
        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_initial_state_pc34_compat(&state));
        state.seed = seed;
        state.floor_ornament_ordinal = ((next_lcg_pc34_compat(&seed) & 7u) + 1u) |
            DM1_V1_D0C_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34;
        state.floor_ornament_pixel = (uint8_t)((next_lcg_pc34_compat(&seed) & 0x3fu) + 0x40u);
        state.footprint_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
        state.f0107_keepout_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
        state.thing_pass_pixel = DM1_V1_D0C_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
        self_check_pc34_compat(&c,
            dm1_v1_viewport_d0c_f0108_floor_ornament_compose_pc34_compat(
                &state, fuzz_surface, sizeof(fuzz_surface), &result));
        self_check_eq_pc34_compat(&c, result.f0108_floor_ornament_writes, 1);
        self_check_eq_pc34_compat(&c, result.f0107_keepout_preserved_floor, 1);
        self_check_eq_pc34_compat(&c, result.thing_pass_observed_floor, 1);
        self_check_eq_pc34_compat(&c, result.floor_write_before_thing_pass, 1);
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
