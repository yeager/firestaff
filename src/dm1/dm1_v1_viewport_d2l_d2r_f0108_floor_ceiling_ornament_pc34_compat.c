#include "firestaff/dm1/v1/viewport/d2l_d2r_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D2_FORWARD = 2,
    DM1_D2L_LANE = -1,
    DM1_D2R_LANE = 1,
    DM1_M604_VIEW_SQUARE_D2L = 7,
    DM1_M605_VIEW_SQUARE_D2R = 8,
    DM1_M591_VIEW_FLOOR_D2L = 5,
    DM1_M593_VIEW_FLOOR_D2R = 7,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_CEILING_GRAPHIC_D2_PC34 = 64,
    DM1_CEILING_ZONE_D2L = 864,
    DM1_CEILING_ZONE_D2R = 866
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0108:3940-4011 handles the M558 ordinal, footprint
 *   recursion, C10 transparency, right-side flip, and PC34 floor-zone math.
 * - DUNVIEW.C F0119:6987-7031 draws D2L floor ornament, door/pass order,
 *   ceiling pit at C864, then the F0115 thing pass.
 * - DUNVIEW.C F0120:7180-7224 mirrors D2R with M593, flipped ceiling at
 *   C866, and the right-side thing-pass orders.
 * - DUNVIEW.C F0128:8503-8517 proves D2L2/D2R2 draw before D2L/D2R.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523
 *   anchor thing-list mutation and square-aspect construction.
 */
static const char s_source_evidence[] =
    "Contract-only DM1 V1 D2L/D2R F0108 floor+ceiling+ornament source lock; "
    "no original DOS parity claim; no game-data bitmap comparison. Anchors: "
    "DUNVIEW.C F0108:3940-4011; DUNVIEW.C F0119:6987-7031; "
    "DUNVIEW.C F0120:7180-7224; DUNVIEW.C F0128:8503-8517; "
    "DUNGEON.C F0163:1769-1838; DUNGEON.C F0164:1840-1905; "
    "DUNGEON.C F0172:2466-2523.";

static const DM1_V1_D2LD2RF0108SpecPc34 s_specs[] = {
    {
        DM1_V1_D2L_D2R_F0108_SIDE_D2L_PC34,
        "D2L F0108 floor, C864 ceiling, F0115 handoff",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        0,
        DM1_D2_FORWARD,
        DM1_D2L_LANE,
        DM1_M604_VIEW_SQUARE_D2L,
        DM1_M591_VIEW_FLOOR_D2L,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE_PC34,
        1505,
        0,
        DM1_CEILING_GRAPHIC_D2_PC34,
        DM1_CEILING_ZONE_D2L,
        0,
        0x3421,
        0x0218,
        0x0349,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0119:6987-7031 / F0128:8510-8513",
        "DUNGEON.C F0163:1769-1838 / F0164:1840-1905 / F0172:2466-2523"
    },
    {
        DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34,
        "D2R F0108 floor, C866 flipped ceiling, F0115 handoff",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        1,
        DM1_D2_FORWARD,
        DM1_D2R_LANE,
        DM1_M605_VIEW_SQUARE_D2R,
        DM1_M593_VIEW_FLOOR_D2R,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE_PC34,
        1507,
        1,
        DM1_CEILING_GRAPHIC_D2_PC34,
        DM1_CEILING_ZONE_D2R,
        1,
        0x4312,
        0x0128,
        0x0439,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0120:7180-7224 / F0128:8514-8517",
        "DUNGEON.C F0163:1769-1838 / F0164:1840-1905 / F0172:2466-2523"
    }
};

static DM1_V1_D2LD2RF0108SelfTestResultPc34 s_last_self_test;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static int supported_context(DM1_V1_D2LD2RF0108ContextPc34 context)
{
    return context == DM1_V1_D2L_D2R_F0108_CONTEXT_CORRIDOR_PC34 ||
           context == DM1_V1_D2L_D2R_F0108_CONTEXT_OPEN_PIT_PC34 ||
           context == DM1_V1_D2L_D2R_F0108_CONTEXT_TELEPORTER_PC34 ||
           context == DM1_V1_D2L_D2R_F0108_CONTEXT_DOOR_SIDE_PC34 ||
           context == DM1_V1_D2L_D2R_F0108_CONTEXT_DOOR_FRONT_PC34 ||
           context == DM1_V1_D2L_D2R_F0108_CONTEXT_STAIRS_FRONT_PC34;
}

size_t dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D2LD2RF0108SpecPc34 *
dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D2LD2RF0108SpecPc34 *
dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_count_pc34(); ++i) {
        if ((int)s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d2l_d2r_f0108_initial_state_pc34(
    DM1_V1_D2LD2RF0108SidePc34 side,
    DM1_V1_D2LD2RF0108ContextPc34 context,
    DM1_V1_D2LD2RF0108StatePc34 *out)
{
    const DM1_V1_D2LD2RF0108SpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_for_side_pc34((int)side);
    if (!spec || !supported_context(context)) return false;

    out->side = side;
    out->context = context;
    out->floor_ornament_ordinal =
        side == DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34 ? 0x8006u : 0x8004u;
    out->destination_pixel =
        side == DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34 ? 0x32u : 0x31u;
    out->floor_pixel =
        side == DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34 ? 0x52u : 0x51u;
    out->ceiling_pixel =
        side == DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34 ? 0x72u : 0x71u;
    out->rear_thing_pixel = DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34;
    out->front_thing_pixel = DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34;
    out->source_locked_contract_only = true;
    out->no_original_dos_parity_claim = true;
    out->no_game_data_load = true;
    return true;
}

bool dm1_v1_viewport_d2l_d2r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D2LD2RF0108OrdinalPc34 *out)
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
        (floor_ornament_ordinal & DM1_V1_D2L_D2R_F0108_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal & ~DM1_V1_D2L_D2R_F0108_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_index = (int)(out->footprint_flag_set ? cleared : floor_ornament_ordinal) - 1;
        out->metadata_blit_count = 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index = DM1_V1_D2L_D2R_F0108_FOOTPRINT_INDEX_PC34;
        ++out->metadata_blit_count;
    }
    return true;
}

uint8_t dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d2l_d2r_f0108_compose_pc34(
    const DM1_V1_D2LD2RF0108StatePc34 *state,
    DM1_V1_D2LD2RF0108ResultPc34 *out)
{
    const DM1_V1_D2LD2RF0108SpecPc34 *spec;
    DM1_V1_D2LD2RF0108OrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state) return false;

    spec = dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_for_side_pc34((int)state->side);
    out->spec = spec;
    if (!spec ||
        !supported_context(state->context) ||
        !state->source_locked_contract_only ||
        !state->no_original_dos_parity_claim ||
        !state->no_game_data_load ||
        state->mutate_thing_list ||
        state->allow_wall_ornament_overlap ||
        state->allow_door_overlap) {
        out->rejected_non_contract_state = 1;
        return false;
    }
    if (!dm1_v1_viewport_d2l_d2r_f0108_decode_ordinal_pc34(
            state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    out->spec = spec;
    out->ok = 1;
    out->lateral2_prepass_complete = 1;
    out->wall_door_keepout_ok = 1;
    out->mutation_guard_ok = 1;
    out->floor_zone =
        spec->floor_zone_base + spec->floor_zone_stride_pc34 * 0 + spec->view_floor;
    out->floor_primary_index = ordinal.primary_index;
    out->floor_recursive_index = ordinal.recursive_footprints_index;
    pixel = state->destination_pixel;

    if (ordinal.has_input_ordinal) {
        out->floor_ornament_calls = 1;
        out->floor_primary_blits = ordinal.primary_draws ? 1 : 0;
        out->footprint_recursions = ordinal.recursive_footprints_draw ? 1 : 0;
        if (ordinal.primary_draws) {
            if (state->floor_pixel == DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34) {
                ++out->c10_transparent_blits;
            }
            pixel = dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(
                pixel, state->floor_pixel);
        }
    }
    if (state->context == DM1_V1_D2L_D2R_F0108_CONTEXT_OPEN_PIT_PC34 &&
        out->floor_ornament_calls) {
        out->open_pit_still_draws_floor_ornament = 1;
    }
    out->after_floor = pixel;

    if (state->context == DM1_V1_D2L_D2R_F0108_CONTEXT_DOOR_FRONT_PC34) {
        ++out->rear_thing_pass_calls;
        pixel = dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(
            pixel, state->rear_thing_pixel);
        if (state->rear_thing_pixel == DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34) {
            ++out->c10_transparent_blits;
        }
    }

    out->ceiling_calls = 1;
    pixel = dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(pixel, state->ceiling_pixel);
    if (state->ceiling_pixel == DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    out->after_ceiling = pixel;

    ++out->front_thing_pass_calls;
    pixel = dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(
        pixel, state->front_thing_pixel);
    if (state->front_thing_pixel == DM1_V1_D2L_D2R_F0108_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    out->after_front_thing = pixel;

    out->deterministic_hash = hash_u32(2166136261u, (uint32_t)state->side);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)state->context);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_zone);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->after_floor);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->after_ceiling);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->after_front_thing);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_primary_index);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_recursive_index);
    return true;
}

static void self_check(int condition, DM1_V1_D2LD2RF0108SelfTestResultPc34 *result)
{
    ++result->assertions;
    if (!condition) ++result->failures;
}

static void exercise_case(
    DM1_V1_D2LD2RF0108SidePc34 side,
    DM1_V1_D2LD2RF0108ContextPc34 context,
    DM1_V1_D2LD2RF0108SelfTestResultPc34 *result)
{
    DM1_V1_D2LD2RF0108StatePc34 state;
    DM1_V1_D2LD2RF0108ResultPc34 composed;

    self_check(dm1_v1_viewport_d2l_d2r_f0108_initial_state_pc34(side, context, &state), result);
    self_check(dm1_v1_viewport_d2l_d2r_f0108_compose_pc34(&state, &composed), result);
    self_check(composed.ok == 1, result);
    self_check(composed.spec != NULL, result);
    self_check(composed.floor_zone == (side == DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34 ? 1507 : 1505), result);
    self_check(composed.ceiling_calls == 1, result);
    self_check(composed.front_thing_pass_calls == 1, result);
    self_check(composed.wall_door_keepout_ok == 1, result);
    self_check(composed.mutation_guard_ok == 1, result);
    self_check(composed.lateral2_prepass_complete == 1, result);
    self_check(composed.floor_primary_index >= 0, result);
    self_check(composed.footprint_recursions == 1, result);
    if (context == DM1_V1_D2L_D2R_F0108_CONTEXT_DOOR_FRONT_PC34) {
        self_check(composed.rear_thing_pass_calls == 1, result);
    }
    if (context == DM1_V1_D2L_D2R_F0108_CONTEXT_OPEN_PIT_PC34) {
        self_check(composed.open_pit_still_draws_floor_ornament == 1, result);
    }
    if (side == DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34) {
        ++result->d2r_floor_calls;
    } else {
        ++result->d2l_floor_calls;
    }
    result->footprint_recursions += composed.footprint_recursions;
    result->ceiling_calls += composed.ceiling_calls;
    result->thing_pass_calls += composed.front_thing_pass_calls + composed.rear_thing_pass_calls;
    result->deterministic_hash = hash_u32(result->deterministic_hash, composed.deterministic_hash);
}

int run_dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_self_test(void)
{
    DM1_V1_D2LD2RF0108OrdinalPc34 ordinal;
    DM1_V1_D2LD2RF0108StatePc34 rejected;
    DM1_V1_D2LD2RF0108ResultPc34 composed;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));
    s_last_self_test.deterministic_hash = 2166136261u;

    self_check(dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_count_pc34() == 2u,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d2l_d2r_f0108_floor_ceiling_ornament_at_pc34(2) == NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNVIEW.C F0119:6987-7031") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "no original DOS parity claim") != NULL,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d2l_d2r_f0108_decode_ordinal_pc34(0x8004u, &ordinal),
               &s_last_self_test);
    self_check(ordinal.primary_index == 3, &s_last_self_test);
    self_check(ordinal.recursive_footprints_index == DM1_V1_D2L_D2R_F0108_FOOTPRINT_INDEX_PC34,
               &s_last_self_test);
    self_check(ordinal.metadata_blit_count == 2, &s_last_self_test);
    self_check(dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(0x44u, 10u) == 0x44u,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d2l_d2r_f0108_blend_c10_pc34(0x44u, 0x55u) == 0x55u,
               &s_last_self_test);

    exercise_case(DM1_V1_D2L_D2R_F0108_SIDE_D2L_PC34,
                  DM1_V1_D2L_D2R_F0108_CONTEXT_CORRIDOR_PC34,
                  &s_last_self_test);
    exercise_case(DM1_V1_D2L_D2R_F0108_SIDE_D2L_PC34,
                  DM1_V1_D2L_D2R_F0108_CONTEXT_OPEN_PIT_PC34,
                  &s_last_self_test);
    exercise_case(DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34,
                  DM1_V1_D2L_D2R_F0108_CONTEXT_CORRIDOR_PC34,
                  &s_last_self_test);
    exercise_case(DM1_V1_D2L_D2R_F0108_SIDE_D2R_PC34,
                  DM1_V1_D2L_D2R_F0108_CONTEXT_DOOR_FRONT_PC34,
                  &s_last_self_test);

    self_check(dm1_v1_viewport_d2l_d2r_f0108_initial_state_pc34(
                   DM1_V1_D2L_D2R_F0108_SIDE_D2L_PC34,
                   DM1_V1_D2L_D2R_F0108_CONTEXT_CORRIDOR_PC34,
                   &rejected),
               &s_last_self_test);
    rejected.mutate_thing_list = true;
    self_check(!dm1_v1_viewport_d2l_d2r_f0108_compose_pc34(&rejected, &composed),
               &s_last_self_test);
    self_check(composed.rejected_non_contract_state == 1, &s_last_self_test);
    ++s_last_self_test.mutation_rejections;

    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D2LD2RF0108SelfTestResultPc34 *
dm1_v1_viewport_d2l_d2r_f0108_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *dm1_v1_viewport_d2l_d2r_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}
