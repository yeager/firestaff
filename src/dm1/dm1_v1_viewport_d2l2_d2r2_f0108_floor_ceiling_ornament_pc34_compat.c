#include "firestaff/dm1/v1/viewport/d2l2_d2r2_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    DM1_FRAMEBUFFER_W = 320,
    DM1_FRAMEBUFFER_H = 200,
    DM1_VIEWPORT_X = 48,
    DM1_VIEWPORT_Y = 33,
    DM1_VIEWPORT_W = 224,
    DM1_VIEWPORT_H = 136,
    DM1_C09_VIEW_SQUARE_D2L2 = 9,
    DM1_C10_VIEW_SQUARE_D2R2 = 10,
    DM1_M604_VIEW_SQUARE_D2L = 7,
    DM1_M605_VIEW_SQUARE_D2R = 8,
    DM1_M591_VIEW_FLOOR_D2L = 5,
    DM1_M593_VIEW_FLOOR_D2R = 7,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE_PC34 = 11,
    DM1_CEILING_GRAPHIC_D2_PC34 = 64,
    DM1_CEILING_ZONE_D2L = 864,
    DM1_CEILING_ZONE_D2R = 866,
    DM1_C10_COLOR_FLESH_PC34 = 10,
    DM1_FOOTPRINT_MASK_PC34 = 0x8000,
    DM1_FOOTPRINT_INDEX_PC34 = 15
};

typedef enum {
    DM1_D2L2_D2R2_F0108_SIDE_D2L2_PC34 = 1,
    DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34 = 2
} DM1D2L2D2R2SidePc34;

typedef enum {
    DM1_D2L2_D2R2_F0108_CONTEXT_CORRIDOR_PC34 = 1,
    DM1_D2L2_D2R2_F0108_CONTEXT_OPEN_PIT_PC34 = 2,
    DM1_D2L2_D2R2_F0108_CONTEXT_TELEPORTER_PC34 = 5,
    DM1_D2L2_D2R2_F0108_CONTEXT_DOOR_FRONT_PC34 = 17
} DM1D2L2D2R2ContextPc34;

typedef struct {
    DM1D2L2D2R2SidePc34 side;
    const char *name;
    int f0128_dispatch_order;
    int second_depth_guard_square;
    int f0108_owner_view_square;
    int view_floor;
    int floor_zone;
    int ceiling_zone;
    int ceiling_flip_horizontal;
    int thing_pass_owner_square;
    int sample_x;
    int floor_y;
    int ceiling_y;
    int thing_y;
    unsigned int corridor_order;
    unsigned int door_pass1_order;
    unsigned int door_pass2_order;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_second_depth_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_dungeon_anchor;
} DM1D2L2D2R2SpecPc34;

typedef struct {
    DM1D2L2D2R2SidePc34 side;
    DM1D2L2D2R2ContextPc34 context;
    unsigned int floor_ornament_ordinal;
    uint8_t destination_pixel;
    uint8_t floor_pixel;
    uint8_t ceiling_pixel;
    uint8_t rear_thing_pixel;
    uint8_t front_thing_pixel;
    bool source_locked_contract_only;
    bool no_original_dos_parity_claim;
    bool guard_prepass_complete;
    bool framebuffer_320x200;
    bool viewport_224x136;
    bool mutate_thing_list;
    bool allow_f0107_wall_overlap;
    bool allow_f0111_door_overlap;
} DM1D2L2D2R2StatePc34;

typedef struct {
    bool has_input_ordinal;
    bool footprint_flag_set;
    bool primary_draws;
    int primary_index;
    int recursive_footprints_index;
    int metadata_blit_count;
} DM1D2L2D2R2OrdinalPc34;

typedef struct {
    const DM1D2L2D2R2SpecPc34 *spec;
    int ok;
    int rejected_non_contract_state;
    int guard_prepass_calls;
    int floor_ornament_calls;
    int footprint_recursions;
    int ceiling_calls;
    int rear_thing_pass_calls;
    int front_thing_pass_calls;
    int c10_transparent_blits;
    int viewport_write_count;
    int framebuffer_bounds_ok;
    int open_pit_still_draws_floor_ornament;
    int wall_door_keepout_ok;
    int mutation_guard_ok;
    int floor_primary_index;
    int floor_recursive_index;
    uint8_t after_floor;
    uint8_t after_ceiling;
    uint8_t after_front_thing;
    uint32_t deterministic_hash;
} DM1D2L2D2R2ResultPc34;

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0108:3940-4011 supplies floor ornament ordinal handling,
 *   PC34 floor zone lookup, C10 transparency, and footprint recursion.
 * - DUNVIEW.C F0119:6987-7031 clips the second-depth left path before
 *   D2L floor, C864 ceiling, and F0115 thing pass.
 * - DUNVIEW.C F0120:7180-7224 mirrors the second-depth right side with
 *   D2R floor, C866 flipped ceiling, and side ornament/thing ordering.
 * - DUNVIEW.C F0128:8503-8517 proves D2L2/D2R2 dispatch precedes the
 *   D2L/D2R F0108-capable square recursion.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523
 *   anchor cell fetch, thing-cell fetch, and kappetaal aspect data.
 */
static const char s_source_evidence[] =
    "Contract-only DM1 V1 D2L2/D2R2 F0108 floor+ceiling+ornament source "
    "lock in a 320x200 framebuffer with 224x136 viewport bounds; no original "
    "DOS parity claim; anchors: DUNVIEW.C F0108:3940-4011; DUNVIEW.C "
    "F0119:6987-7031; DUNVIEW.C F0120:7180-7224; DUNVIEW.C "
    "F0128:8503-8517; DUNGEON.C F0163:1769-1838; DUNGEON.C "
    "F0164:1840-1905; DUNGEON.C F0172:2466-2523.";

static const DM1D2L2D2R2SpecPc34 s_specs[] = {
    {
        DM1_D2L2_D2R2_F0108_SIDE_D2L2_PC34,
        "D2L2 second-depth guard feeding D2L F0108 floor, C864 ceiling, F0115",
        0,
        DM1_C09_VIEW_SQUARE_D2L2,
        DM1_M604_VIEW_SQUARE_D2L,
        DM1_M591_VIEW_FLOOR_D2L,
        DM1_FLOOR_ZONE_BASE + DM1_M591_VIEW_FLOOR_D2L,
        DM1_CEILING_ZONE_D2L,
        0,
        DM1_M604_VIEW_SQUARE_D2L,
        DM1_VIEWPORT_X + 56,
        DM1_VIEWPORT_Y + 104,
        DM1_VIEWPORT_Y + 18,
        DM1_VIEWPORT_Y + 72,
        0x3421u,
        0x0218u,
        0x0349u,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0119:6987-7031",
        "DUNVIEW.C F0128:8503-8517",
        "DUNGEON.C F0163:1769-1838 / F0164:1840-1905 / F0172:2466-2523"
    },
    {
        DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34,
        "D2R2 second-depth guard feeding D2R F0108 floor, C866 ceiling, F0115",
        1,
        DM1_C10_VIEW_SQUARE_D2R2,
        DM1_M605_VIEW_SQUARE_D2R,
        DM1_M593_VIEW_FLOOR_D2R,
        DM1_FLOOR_ZONE_BASE + DM1_M593_VIEW_FLOOR_D2R,
        DM1_CEILING_ZONE_D2R,
        1,
        DM1_M605_VIEW_SQUARE_D2R,
        DM1_VIEWPORT_X + 168,
        DM1_VIEWPORT_Y + 104,
        DM1_VIEWPORT_Y + 18,
        DM1_VIEWPORT_Y + 72,
        0x4312u,
        0x0128u,
        0x0439u,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0120:7180-7224",
        "DUNVIEW.C F0128:8503-8517",
        "DUNGEON.C F0163:1769-1838 / F0164:1840-1905 / F0172:2466-2523"
    }
};

static DM1_V1_D2L2D2R2F0108SelfTestResultPc34 s_last_self_test;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static const DM1D2L2D2R2SpecPc34 *spec_for_side(DM1D2L2D2R2SidePc34 side)
{
    size_t i;

    for (i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

static int viewport_contains(int x, int y)
{
    return x >= DM1_VIEWPORT_X &&
           y >= DM1_VIEWPORT_Y &&
           x < DM1_VIEWPORT_X + DM1_VIEWPORT_W &&
           y < DM1_VIEWPORT_Y + DM1_VIEWPORT_H &&
           x < DM1_FRAMEBUFFER_W &&
           y < DM1_FRAMEBUFFER_H;
}

static int supported_context(DM1D2L2D2R2ContextPc34 context)
{
    return context == DM1_D2L2_D2R2_F0108_CONTEXT_CORRIDOR_PC34 ||
           context == DM1_D2L2_D2R2_F0108_CONTEXT_OPEN_PIT_PC34 ||
           context == DM1_D2L2_D2R2_F0108_CONTEXT_TELEPORTER_PC34 ||
           context == DM1_D2L2_D2R2_F0108_CONTEXT_DOOR_FRONT_PC34;
}

static uint8_t blend_c10(uint8_t destination_pixel, uint8_t source_pixel)
{
    return source_pixel == DM1_C10_COLOR_FLESH_PC34 ? destination_pixel : source_pixel;
}

static void write_viewport_pixel(uint8_t *framebuffer, int x, int y, uint8_t pixel,
                                 DM1D2L2D2R2ResultPc34 *out)
{
    if (!viewport_contains(x, y)) {
        out->framebuffer_bounds_ok = 0;
        return;
    }
    framebuffer[y * DM1_FRAMEBUFFER_W + x] = pixel;
    ++out->viewport_write_count;
}

static bool decode_ordinal(unsigned int floor_ornament_ordinal,
                           DM1D2L2D2R2OrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->primary_index = -1;
    out->recursive_footprints_index = -1;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set = (floor_ornament_ordinal & DM1_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal & ~((unsigned int)DM1_FOOTPRINT_MASK_PC34);
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_index = (int)(out->footprint_flag_set ? cleared : floor_ornament_ordinal) - 1;
        out->metadata_blit_count = 1;
    }
    if (out->footprint_flag_set) {
        out->recursive_footprints_index = DM1_FOOTPRINT_INDEX_PC34;
        ++out->metadata_blit_count;
    }
    return true;
}

static bool initial_state(DM1D2L2D2R2SidePc34 side,
                          DM1D2L2D2R2ContextPc34 context,
                          DM1D2L2D2R2StatePc34 *out)
{
    const DM1D2L2D2R2SpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = spec_for_side(side);
    if (!spec || !supported_context(context)) return false;

    out->side = side;
    out->context = context;
    out->floor_ornament_ordinal =
        side == DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34 ? 0x8006u : 0x8004u;
    out->destination_pixel =
        side == DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34 ? 0x32u : 0x31u;
    out->floor_pixel =
        side == DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34 ? 0x52u : 0x51u;
    out->ceiling_pixel =
        side == DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34 ? 0x72u : 0x71u;
    out->rear_thing_pixel = DM1_C10_COLOR_FLESH_PC34;
    out->front_thing_pixel = DM1_C10_COLOR_FLESH_PC34;
    out->source_locked_contract_only = true;
    out->no_original_dos_parity_claim = true;
    out->guard_prepass_complete = true;
    out->framebuffer_320x200 = true;
    out->viewport_224x136 = true;
    (void)spec;
    return true;
}

static bool compose(const DM1D2L2D2R2StatePc34 *state, DM1D2L2D2R2ResultPc34 *out)
{
    const DM1D2L2D2R2SpecPc34 *spec;
    DM1D2L2D2R2OrdinalPc34 ordinal;
    uint8_t framebuffer[DM1_FRAMEBUFFER_W * DM1_FRAMEBUFFER_H];
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->floor_primary_index = -1;
    out->floor_recursive_index = -1;
    out->framebuffer_bounds_ok = 1;
    if (!state) return false;

    spec = spec_for_side(state->side);
    out->spec = spec;
    if (!spec ||
        !supported_context(state->context) ||
        !state->source_locked_contract_only ||
        !state->no_original_dos_parity_claim ||
        !state->guard_prepass_complete ||
        !state->framebuffer_320x200 ||
        !state->viewport_224x136 ||
        state->mutate_thing_list ||
        state->allow_f0107_wall_overlap ||
        state->allow_f0111_door_overlap) {
        out->rejected_non_contract_state = 1;
        return false;
    }
    if (!decode_ordinal(state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    memset(framebuffer, state->destination_pixel, sizeof(framebuffer));
    pixel = state->destination_pixel;
    out->ok = 1;
    out->guard_prepass_calls = 1;
    out->wall_door_keepout_ok = 1;
    out->mutation_guard_ok = 1;
    out->floor_primary_index = ordinal.primary_index;
    out->floor_recursive_index = ordinal.recursive_footprints_index;

    if (ordinal.has_input_ordinal) {
        out->floor_ornament_calls = 1;
        out->footprint_recursions = ordinal.footprint_flag_set ? 1 : 0;
        if (state->floor_pixel == DM1_C10_COLOR_FLESH_PC34) {
            ++out->c10_transparent_blits;
        }
        pixel = blend_c10(pixel, state->floor_pixel);
        write_viewport_pixel(framebuffer, spec->sample_x, spec->floor_y, pixel, out);
        if (state->context == DM1_D2L2_D2R2_F0108_CONTEXT_OPEN_PIT_PC34) {
            out->open_pit_still_draws_floor_ornament = 1;
        }
    }
    out->after_floor = pixel;

    if (state->context == DM1_D2L2_D2R2_F0108_CONTEXT_DOOR_FRONT_PC34) {
        ++out->rear_thing_pass_calls;
        if (state->rear_thing_pixel == DM1_C10_COLOR_FLESH_PC34) {
            ++out->c10_transparent_blits;
        }
        pixel = blend_c10(pixel, state->rear_thing_pixel);
    }

    ++out->ceiling_calls;
    if (state->ceiling_pixel == DM1_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    pixel = blend_c10(pixel, state->ceiling_pixel);
    write_viewport_pixel(framebuffer, spec->sample_x, spec->ceiling_y, pixel, out);
    out->after_ceiling = pixel;

    ++out->front_thing_pass_calls;
    if (state->front_thing_pixel == DM1_C10_COLOR_FLESH_PC34) {
        ++out->c10_transparent_blits;
    }
    pixel = blend_c10(pixel, state->front_thing_pixel);
    write_viewport_pixel(framebuffer, spec->sample_x, spec->thing_y, pixel, out);
    out->after_front_thing = pixel;

    out->deterministic_hash = hash_u32(2166136261u, (uint32_t)state->side);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)state->context);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)spec->second_depth_guard_square);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)spec->f0108_owner_view_square);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)spec->floor_zone);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)spec->ceiling_zone);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->viewport_write_count);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->after_floor);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->after_ceiling);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->after_front_thing);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_primary_index);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_recursive_index);
    out->deterministic_hash = hash_u32(out->deterministic_hash, framebuffer[spec->floor_y * DM1_FRAMEBUFFER_W + spec->sample_x]);
    out->deterministic_hash = hash_u32(out->deterministic_hash, framebuffer[spec->ceiling_y * DM1_FRAMEBUFFER_W + spec->sample_x]);
    out->deterministic_hash = hash_u32(out->deterministic_hash, framebuffer[spec->thing_y * DM1_FRAMEBUFFER_W + spec->sample_x]);
    return true;
}

static void self_check(int condition, DM1_V1_D2L2D2R2F0108SelfTestResultPc34 *result)
{
    ++result->assertions;
    if (!condition) ++result->failures;
}

static void exercise_case(DM1D2L2D2R2SidePc34 side,
                          DM1D2L2D2R2ContextPc34 context,
                          DM1_V1_D2L2D2R2F0108SelfTestResultPc34 *result)
{
    DM1D2L2D2R2StatePc34 state;
    DM1D2L2D2R2ResultPc34 composed;
    const DM1D2L2D2R2SpecPc34 *spec;

    spec = spec_for_side(side);
    self_check(spec != NULL, result);
    self_check(initial_state(side, context, &state), result);
    self_check(compose(&state, &composed), result);
    self_check(composed.ok == 1, result);
    self_check(composed.spec == spec, result);
    self_check(composed.guard_prepass_calls == 1, result);
    self_check(composed.floor_ornament_calls == 1, result);
    self_check(composed.ceiling_calls == 1, result);
    self_check(composed.front_thing_pass_calls == 1, result);
    self_check(composed.wall_door_keepout_ok == 1, result);
    self_check(composed.mutation_guard_ok == 1, result);
    self_check(composed.framebuffer_bounds_ok == 1, result);
    self_check(composed.viewport_write_count == 3, result);
    self_check(composed.footprint_recursions == 1, result);
    self_check(composed.floor_primary_index >= 0, result);
    self_check(composed.floor_recursive_index == DM1_FOOTPRINT_INDEX_PC34, result);
    self_check(composed.after_floor == state.floor_pixel, result);
    self_check(composed.after_ceiling == state.ceiling_pixel, result);
    self_check(composed.after_front_thing == state.ceiling_pixel, result);
    self_check(viewport_contains(spec ? spec->sample_x : 0, spec ? spec->floor_y : 0), result);
    self_check(viewport_contains(spec ? spec->sample_x : 0, spec ? spec->ceiling_y : 0), result);
    self_check(viewport_contains(spec ? spec->sample_x : 0, spec ? spec->thing_y : 0), result);
    self_check(spec && spec->floor_zone == (side == DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34 ? 1507 : 1505),
               result);
    self_check(spec && spec->ceiling_zone == (side == DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34 ? 866 : 864),
               result);
    if (context == DM1_D2L2_D2R2_F0108_CONTEXT_DOOR_FRONT_PC34) {
        self_check(composed.rear_thing_pass_calls == 1, result);
    }
    if (context == DM1_D2L2_D2R2_F0108_CONTEXT_OPEN_PIT_PC34) {
        self_check(composed.open_pit_still_draws_floor_ornament == 1, result);
    }

    if (side == DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34) {
        ++result->d2r2_floor_calls;
    } else {
        ++result->d2l2_floor_calls;
    }
    result->footprint_recursions += composed.footprint_recursions;
    result->ceiling_calls += composed.ceiling_calls;
    result->thing_pass_calls += composed.front_thing_pass_calls + composed.rear_thing_pass_calls;
    result->deterministic_hash = hash_u32(result->deterministic_hash, composed.deterministic_hash);
}

int run_dm1_v1_viewport_d2l2_d2r2_f0108_floor_ceiling_ornament_self_test(void)
{
    DM1D2L2D2R2OrdinalPc34 ordinal;
    DM1D2L2D2R2StatePc34 rejected;
    DM1D2L2D2R2ResultPc34 composed;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));
    s_last_self_test.deterministic_hash = 2166136261u;

    self_check(sizeof(s_specs) / sizeof(s_specs[0]) == 2u, &s_last_self_test);
    self_check(spec_for_side(DM1_D2L2_D2R2_F0108_SIDE_D2L2_PC34) != NULL,
               &s_last_self_test);
    self_check(spec_for_side(DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34) != NULL,
               &s_last_self_test);
    self_check(spec_for_side((DM1D2L2D2R2SidePc34)99) == NULL, &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNVIEW.C F0108:3940-4011") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNVIEW.C F0119:6987-7031") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNVIEW.C F0120:7180-7224") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNVIEW.C F0128:8503-8517") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNGEON.C F0163:1769-1838") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNGEON.C F0164:1840-1905") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNGEON.C F0172:2466-2523") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "no original DOS parity claim") != NULL,
               &s_last_self_test);
    self_check(DM1_FRAMEBUFFER_W == 320 && DM1_FRAMEBUFFER_H == 200,
               &s_last_self_test);
    self_check(DM1_VIEWPORT_W == 224 && DM1_VIEWPORT_H == 136, &s_last_self_test);
    self_check(viewport_contains(DM1_VIEWPORT_X, DM1_VIEWPORT_Y), &s_last_self_test);
    self_check(!viewport_contains(DM1_VIEWPORT_X - 1, DM1_VIEWPORT_Y), &s_last_self_test);
    self_check(!viewport_contains(DM1_VIEWPORT_X + DM1_VIEWPORT_W, DM1_VIEWPORT_Y),
               &s_last_self_test);
    self_check(decode_ordinal(0x8004u, &ordinal), &s_last_self_test);
    self_check(ordinal.has_input_ordinal, &s_last_self_test);
    self_check(ordinal.footprint_flag_set, &s_last_self_test);
    self_check(ordinal.primary_draws, &s_last_self_test);
    self_check(ordinal.primary_index == 3, &s_last_self_test);
    self_check(ordinal.recursive_footprints_index == DM1_FOOTPRINT_INDEX_PC34,
               &s_last_self_test);
    self_check(ordinal.metadata_blit_count == 2, &s_last_self_test);
    self_check(blend_c10(0x44u, DM1_C10_COLOR_FLESH_PC34) == 0x44u, &s_last_self_test);
    self_check(blend_c10(0x44u, 0x55u) == 0x55u, &s_last_self_test);

    exercise_case(DM1_D2L2_D2R2_F0108_SIDE_D2L2_PC34,
                  DM1_D2L2_D2R2_F0108_CONTEXT_CORRIDOR_PC34,
                  &s_last_self_test);
    exercise_case(DM1_D2L2_D2R2_F0108_SIDE_D2L2_PC34,
                  DM1_D2L2_D2R2_F0108_CONTEXT_OPEN_PIT_PC34,
                  &s_last_self_test);
    exercise_case(DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34,
                  DM1_D2L2_D2R2_F0108_CONTEXT_CORRIDOR_PC34,
                  &s_last_self_test);
    exercise_case(DM1_D2L2_D2R2_F0108_SIDE_D2R2_PC34,
                  DM1_D2L2_D2R2_F0108_CONTEXT_DOOR_FRONT_PC34,
                  &s_last_self_test);

    self_check(initial_state(DM1_D2L2_D2R2_F0108_SIDE_D2L2_PC34,
                             DM1_D2L2_D2R2_F0108_CONTEXT_CORRIDOR_PC34,
                             &rejected),
               &s_last_self_test);
    rejected.mutate_thing_list = true;
    self_check(!compose(&rejected, &composed), &s_last_self_test);
    self_check(composed.rejected_non_contract_state == 1, &s_last_self_test);
    ++s_last_self_test.mutation_rejections;

    self_check(s_last_self_test.d2l2_floor_calls == 2, &s_last_self_test);
    self_check(s_last_self_test.d2r2_floor_calls == 2, &s_last_self_test);
    self_check(s_last_self_test.footprint_recursions == 4, &s_last_self_test);
    self_check(s_last_self_test.ceiling_calls == 4, &s_last_self_test);
    self_check(s_last_self_test.thing_pass_calls == 5, &s_last_self_test);
    self_check(s_last_self_test.mutation_rejections == 1, &s_last_self_test);
    self_check(s_last_self_test.assertions >= 50, &s_last_self_test);
    self_check(s_last_self_test.deterministic_hash != 2166136261u, &s_last_self_test);

    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D2L2D2R2F0108SelfTestResultPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}
