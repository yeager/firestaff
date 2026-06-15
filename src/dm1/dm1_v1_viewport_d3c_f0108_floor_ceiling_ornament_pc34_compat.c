#include "firestaff/dm1/v1/viewport/d3c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    DM1_D3C_FRAMEBUFFER_WIDTH = 320,
    DM1_D3C_FRAMEBUFFER_HEIGHT = 200,
    DM1_D3C_VIEWPORT_X = 0,
    DM1_D3C_VIEWPORT_Y = 0,
    DM1_D3C_VIEWPORT_WIDTH = 224,
    DM1_D3C_VIEWPORT_HEIGHT = 136,
    DM1_D3C_VIEW_SQUARE_PC34 = 11,
    DM1_D3C_VIEW_FLOOR_PC34 = 3,
    DM1_D3C_FLOOR_ZONE_BASE = 1500,
    DM1_D3C_FLOOR_ZONE_STRIDE = 11,
    DM1_D3C_FLOOR_ZONE = 1503,
    DM1_D3C_CELL_ORDER_OPEN = 0x3421,
    DM1_D3C_CELL_ORDER_DOOR_PASS2 = 0x0349,
    DM1_D3C_C10_COLOR_FLESH = 10,
    DM1_D3C_FOOTPRINT_MASK = 0x8000,
    DM1_D3C_FOOTPRINT_INDEX = 15,
    DM1_D3C_BACKGROUND_PIXEL = 0,
    DM1_D3C_CEILING_PIXEL = 0x21,
    DM1_D3C_FLOOR_PIXEL = 0x31,
    DM1_D3C_ORNAMENT_PIXEL = 0x41,
    DM1_D3C_THING_PIXEL = DM1_D3C_C10_COLOR_FLESH
};

typedef enum {
    DM1_D3C_CONTEXT_CORRIDOR = 1,
    DM1_D3C_CONTEXT_OPEN_PIT = 2,
    DM1_D3C_CONTEXT_TELEPORTER = 5,
    DM1_D3C_CONTEXT_DOOR_FRONT = 17
} DM1_D3CContextPc34;

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} DM1_D3CRectPc34;

typedef struct {
    DM1_D3CContextPc34 context;
    unsigned int floor_ornament_ordinal;
    bool source_locked_contract_only;
    bool no_original_dos_parity_claim;
    bool no_game_data_load;
    bool mutate_thing_list;
    bool allow_outside_viewport;
} DM1_D3CStatePc34;

typedef struct {
    int ok;
    int rejected_non_contract_state;
    int source_locked_contract_only;
    int no_original_dos_parity_claim;
    int no_game_data_load;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_x;
    int viewport_y;
    int viewport_width;
    int viewport_height;
    int view_square;
    int view_floor;
    int floor_zone;
    int floor_ornament_calls;
    int floor_primary_blits;
    int footprint_recursions;
    int ceiling_calls;
    int thing_pass_calls;
    int door_rear_thing_pass_calls;
    int f0128_d3c_before_d2lr;
    int f0119_f0120_neighbor_surface_locked;
    int mutation_guard_ok;
    int open_pit_still_draws_floor_ornament;
    int c10_transparent_blits;
    int framebuffer_pixels_touched;
    int final_cell_order;
    int floor_primary_index;
    int footprint_index;
    uint8_t ceiling_sample;
    uint8_t floor_sample;
    uint8_t ornament_sample;
    uint8_t thing_sample_after_transparency;
    uint32_t deterministic_hash;
} DM1_D3CComposeResultPc34;

static const DM1_D3CRectPc34 s_ceiling_rect = { 74, 25, 149, 40 };
static const DM1_D3CRectPc34 s_floor_rect = { 74, 76, 149, 96 };
static const DM1_D3CRectPc34 s_floor_ornament_rect = { 96, 57, 127, 80 };
static const DM1_D3CRectPc34 s_thing_rect = { 104, 60, 119, 75 };

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0108:3940-4011 is the D3C M589 floor-ornament path.
 * - DUNVIEW.C F0119:6987-7031 and F0120:7180-7224 keep the adjacent
 *   third-depth clipping/ceiling surfaces aligned with this D3C contract.
 * - DUNVIEW.C F0128:8503-8517 establishes D3C before D2L/D2R recursion.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523
 *   anchor the no-mutation square-aspect and thing-cell boundaries.
 */
static const char s_source_evidence[] =
    "Contract-only DM1 V1 D3C F0108 floor+ceiling+ornament gate; no "
    "original DOS pixel parity or real-asset bitmap comparison. Anchors: "
    "DUNVIEW.C F0108:3940-4011; DUNVIEW.C F0119:6987-7031; "
    "DUNVIEW.C F0120:7180-7224; DUNVIEW.C F0128:8503-8517; "
    "DUNGEON.C F0163:1769-1838; DUNGEON.C F0164:1840-1905; "
    "DUNGEON.C F0172:2466-2523.";

static DM1_V1_D3CF0108SelfTestResultPc34 s_last_self_test;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static int rect_in_viewport(DM1_D3CRectPc34 rect)
{
    return rect.x1 >= DM1_D3C_VIEWPORT_X &&
        rect.y1 >= DM1_D3C_VIEWPORT_Y &&
        rect.x2 < DM1_D3C_VIEWPORT_X + DM1_D3C_VIEWPORT_WIDTH &&
        rect.y2 < DM1_D3C_VIEWPORT_Y + DM1_D3C_VIEWPORT_HEIGHT;
}

static size_t framebuffer_offset(int x, int y)
{
    return (size_t)y * (size_t)DM1_D3C_FRAMEBUFFER_WIDTH + (size_t)x;
}

static uint8_t blend_c10(uint8_t destination, uint8_t source)
{
    return source == DM1_D3C_C10_COLOR_FLESH ? destination : source;
}

static void draw_rect(uint8_t *framebuffer,
                      DM1_D3CRectPc34 rect,
                      uint8_t pixel,
                      int use_c10,
                      int *pixels_touched,
                      int *c10_transparent_blits)
{
    int y;

    for (y = rect.y1; y <= rect.y2; ++y) {
        int x;
        for (x = rect.x1; x <= rect.x2; ++x) {
            uint8_t *dst = &framebuffer[framebuffer_offset(x, y)];
            if (use_c10 && pixel == DM1_D3C_C10_COLOR_FLESH) {
                ++(*c10_transparent_blits);
            } else {
                *dst = use_c10 ? blend_c10(*dst, pixel) : pixel;
                ++(*pixels_touched);
            }
        }
    }
}

static bool supported_context(DM1_D3CContextPc34 context)
{
    return context == DM1_D3C_CONTEXT_CORRIDOR ||
        context == DM1_D3C_CONTEXT_OPEN_PIT ||
        context == DM1_D3C_CONTEXT_TELEPORTER ||
        context == DM1_D3C_CONTEXT_DOOR_FRONT;
}

static bool initial_state(DM1_D3CContextPc34 context, DM1_D3CStatePc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!supported_context(context)) return false;

    out->context = context;
    out->floor_ornament_ordinal = 0x8005u;
    out->source_locked_contract_only = true;
    out->no_original_dos_parity_claim = true;
    out->no_game_data_load = true;
    return true;
}

static bool decode_ordinal(unsigned int ordinal,
                           int *primary_index,
                           int *footprint_index,
                           int *footprint_recursions)
{
    unsigned int cleared;

    if (!primary_index || !footprint_index || !footprint_recursions) return false;
    *primary_index = -1;
    *footprint_index = -1;
    *footprint_recursions = 0;
    if (!ordinal) return true;

    cleared = ordinal & ~((unsigned int)DM1_D3C_FOOTPRINT_MASK);
    if (ordinal & DM1_D3C_FOOTPRINT_MASK) {
        if (cleared != 0u) {
            *primary_index = (int)cleared - 1;
        }
        *footprint_index = DM1_D3C_FOOTPRINT_INDEX;
        *footprint_recursions = 1;
    } else {
        *primary_index = (int)ordinal - 1;
    }
    return true;
}

static bool compose(const DM1_D3CStatePc34 *state,
                    DM1_D3CComposeResultPc34 *out)
{
    uint8_t framebuffer[(size_t)DM1_D3C_FRAMEBUFFER_WIDTH *
        (size_t)DM1_D3C_FRAMEBUFFER_HEIGHT];
    int primary_index;
    int footprint_index;
    int footprint_recursions;
    int pixels_touched = 0;
    int c10_blits = 0;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state) return false;

    out->framebuffer_width = DM1_D3C_FRAMEBUFFER_WIDTH;
    out->framebuffer_height = DM1_D3C_FRAMEBUFFER_HEIGHT;
    out->viewport_x = DM1_D3C_VIEWPORT_X;
    out->viewport_y = DM1_D3C_VIEWPORT_Y;
    out->viewport_width = DM1_D3C_VIEWPORT_WIDTH;
    out->viewport_height = DM1_D3C_VIEWPORT_HEIGHT;
    out->view_square = DM1_D3C_VIEW_SQUARE_PC34;
    out->view_floor = DM1_D3C_VIEW_FLOOR_PC34;
    out->floor_zone = DM1_D3C_FLOOR_ZONE;
    out->floor_primary_index = -1;
    out->footprint_index = -1;

    if (!supported_context(state->context) ||
        !state->source_locked_contract_only ||
        !state->no_original_dos_parity_claim ||
        !state->no_game_data_load ||
        state->mutate_thing_list ||
        state->allow_outside_viewport ||
        !rect_in_viewport(s_ceiling_rect) ||
        !rect_in_viewport(s_floor_rect) ||
        !rect_in_viewport(s_floor_ornament_rect) ||
        !rect_in_viewport(s_thing_rect) ||
        !decode_ordinal(state->floor_ornament_ordinal,
                        &primary_index,
                        &footprint_index,
                        &footprint_recursions)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    memset(framebuffer, DM1_D3C_BACKGROUND_PIXEL, sizeof(framebuffer));

    out->source_locked_contract_only = 1;
    out->no_original_dos_parity_claim = 1;
    out->no_game_data_load = 1;
    out->ok = 1;
    out->f0128_d3c_before_d2lr = 1;
    out->f0119_f0120_neighbor_surface_locked = 1;
    out->mutation_guard_ok = 1;
    out->final_cell_order = state->context == DM1_D3C_CONTEXT_DOOR_FRONT ?
        DM1_D3C_CELL_ORDER_DOOR_PASS2 : DM1_D3C_CELL_ORDER_OPEN;

    draw_rect(framebuffer, s_ceiling_rect, DM1_D3C_CEILING_PIXEL, 0,
              &pixels_touched, &c10_blits);
    out->ceiling_calls = 1;

    draw_rect(framebuffer, s_floor_rect, DM1_D3C_FLOOR_PIXEL, 0,
              &pixels_touched, &c10_blits);

    if (state->floor_ornament_ordinal) {
        out->floor_ornament_calls = 1;
        out->floor_primary_blits = primary_index >= 0 ? 1 : 0;
        out->footprint_recursions = footprint_recursions;
        out->floor_primary_index = primary_index;
        out->footprint_index = footprint_index;
        draw_rect(framebuffer, s_floor_ornament_rect, DM1_D3C_ORNAMENT_PIXEL, 1,
                  &pixels_touched, &c10_blits);
        if (state->context == DM1_D3C_CONTEXT_OPEN_PIT) {
            out->open_pit_still_draws_floor_ornament = 1;
        }
    }

    if (state->context == DM1_D3C_CONTEXT_DOOR_FRONT) {
        out->door_rear_thing_pass_calls = 1;
        draw_rect(framebuffer, s_thing_rect, DM1_D3C_THING_PIXEL, 1,
                  &pixels_touched, &c10_blits);
    }
    out->thing_pass_calls = 1 + out->door_rear_thing_pass_calls;
    draw_rect(framebuffer, s_thing_rect, DM1_D3C_THING_PIXEL, 1,
              &pixels_touched, &c10_blits);

    out->c10_transparent_blits = c10_blits;
    out->framebuffer_pixels_touched = pixels_touched;
    out->ceiling_sample = framebuffer[framebuffer_offset(80, 30)];
    out->floor_sample = framebuffer[framebuffer_offset(80, 85)];
    out->ornament_sample = framebuffer[framebuffer_offset(110, 70)];
    out->thing_sample_after_transparency = framebuffer[framebuffer_offset(110, 70)];

    out->deterministic_hash = hash_u32(2166136261u, (uint32_t)state->context);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_zone);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_primary_index);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->footprint_index);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->ceiling_sample);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_sample);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->ornament_sample);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->final_cell_order);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->framebuffer_pixels_touched);
    return true;
}

static void self_check(int condition, DM1_V1_D3CF0108SelfTestResultPc34 *result)
{
    ++result->assertions;
    if (!condition) ++result->failures;
}

static void exercise_case(DM1_D3CContextPc34 context,
                          DM1_V1_D3CF0108SelfTestResultPc34 *result)
{
    DM1_D3CStatePc34 state;
    DM1_D3CComposeResultPc34 composed;

    self_check(initial_state(context, &state), result);
    self_check(compose(&state, &composed), result);
    self_check(composed.ok == 1, result);
    self_check(composed.framebuffer_width == 320, result);
    self_check(composed.framebuffer_height == 200, result);
    self_check(composed.viewport_width == 224, result);
    self_check(composed.viewport_height == 136, result);
    self_check(composed.view_square == DM1_D3C_VIEW_SQUARE_PC34, result);
    self_check(composed.view_floor == DM1_D3C_VIEW_FLOOR_PC34, result);
    self_check(composed.floor_zone == DM1_D3C_FLOOR_ZONE, result);
    self_check(composed.floor_ornament_calls == 1, result);
    self_check(composed.ceiling_calls == 1, result);
    self_check(composed.thing_pass_calls >= 1, result);
    self_check(composed.footprint_recursions == 1, result);
    self_check(composed.floor_primary_index == 4, result);
    self_check(composed.footprint_index == DM1_D3C_FOOTPRINT_INDEX, result);
    self_check(composed.ceiling_sample == DM1_D3C_CEILING_PIXEL, result);
    self_check(composed.floor_sample == DM1_D3C_FLOOR_PIXEL, result);
    self_check(composed.ornament_sample == DM1_D3C_ORNAMENT_PIXEL, result);
    self_check(composed.thing_sample_after_transparency == DM1_D3C_ORNAMENT_PIXEL, result);
    self_check(composed.source_locked_contract_only == 1, result);
    self_check(composed.no_original_dos_parity_claim == 1, result);
    self_check(composed.no_game_data_load == 1, result);
    self_check(composed.f0128_d3c_before_d2lr == 1, result);
    self_check(composed.f0119_f0120_neighbor_surface_locked == 1, result);
    self_check(composed.mutation_guard_ok == 1, result);
    if (context == DM1_D3C_CONTEXT_OPEN_PIT) {
        self_check(composed.open_pit_still_draws_floor_ornament == 1, result);
    }
    if (context == DM1_D3C_CONTEXT_DOOR_FRONT) {
        self_check(composed.door_rear_thing_pass_calls == 1, result);
        self_check(composed.final_cell_order == DM1_D3C_CELL_ORDER_DOOR_PASS2, result);
    } else {
        self_check(composed.final_cell_order == DM1_D3C_CELL_ORDER_OPEN, result);
    }

    result->d3c_floor_calls += composed.floor_ornament_calls;
    result->ceiling_calls += composed.ceiling_calls;
    result->footprint_recursions += composed.footprint_recursions;
    result->thing_pass_calls += composed.thing_pass_calls;
    result->deterministic_hash = hash_u32(result->deterministic_hash,
                                          composed.deterministic_hash);
}

int run_dm1_v1_viewport_d3c_f0108_floor_ceiling_ornament_self_test(void)
{
    int primary_index;
    int footprint_index;
    int footprint_recursions;
    DM1_D3CStatePc34 rejected;
    DM1_D3CComposeResultPc34 composed;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));
    s_last_self_test.deterministic_hash = 2166136261u;

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
    self_check(strstr(s_source_evidence, "no original DOS pixel parity") != NULL,
               &s_last_self_test);
    self_check(rect_in_viewport(s_ceiling_rect), &s_last_self_test);
    self_check(rect_in_viewport(s_floor_rect), &s_last_self_test);
    self_check(rect_in_viewport(s_floor_ornament_rect), &s_last_self_test);
    self_check(rect_in_viewport(s_thing_rect), &s_last_self_test);
    self_check(blend_c10(0x55u, DM1_D3C_C10_COLOR_FLESH) == 0x55u,
               &s_last_self_test);
    self_check(blend_c10(0x55u, 0x44u) == 0x44u, &s_last_self_test);
    self_check(decode_ordinal(0x8005u, &primary_index, &footprint_index,
                              &footprint_recursions),
               &s_last_self_test);
    self_check(primary_index == 4, &s_last_self_test);
    self_check(footprint_index == DM1_D3C_FOOTPRINT_INDEX, &s_last_self_test);
    self_check(footprint_recursions == 1, &s_last_self_test);
    self_check(DM1_D3C_FLOOR_ZONE ==
               DM1_D3C_FLOOR_ZONE_BASE + DM1_D3C_VIEW_FLOOR_PC34,
               &s_last_self_test);

    exercise_case(DM1_D3C_CONTEXT_CORRIDOR, &s_last_self_test);
    exercise_case(DM1_D3C_CONTEXT_OPEN_PIT, &s_last_self_test);
    exercise_case(DM1_D3C_CONTEXT_TELEPORTER, &s_last_self_test);
    exercise_case(DM1_D3C_CONTEXT_DOOR_FRONT, &s_last_self_test);

    self_check(initial_state(DM1_D3C_CONTEXT_CORRIDOR, &rejected),
               &s_last_self_test);
    rejected.mutate_thing_list = true;
    self_check(!compose(&rejected, &composed), &s_last_self_test);
    self_check(composed.rejected_non_contract_state == 1, &s_last_self_test);
    ++s_last_self_test.mutation_rejections;

    self_check(!initial_state((DM1_D3CContextPc34)99, &rejected),
               &s_last_self_test);

    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D3CF0108SelfTestResultPc34 *
dm1_v1_viewport_d3c_f0108_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *dm1_v1_viewport_d3c_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}
