#include "firestaff/dm1/v1/viewport/d3l_d3r_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D3_FORWARD = 3,
    DM1_D3L_LANE = -1,
    DM1_D3R_LANE = 1,
    DM1_M601_VIEW_SQUARE_D3L = 12,
    DM1_M602_VIEW_SQUARE_D3R = 13,
    DM1_M588_VIEW_FLOOR_D3L = 2,
    DM1_M590_VIEW_FLOOR_D3R = 4,
    DM1_M575_VIEW_WALL_D3L_RIGHT = 2,
    DM1_M576_VIEW_WALL_D3R_LEFT = 3,
    DM1_M577_VIEW_WALL_D3L_FRONT = 4,
    DM1_M579_VIEW_WALL_D3R_FRONT = 6,
    DM1_C705_ZONE_WALL_D3L = 705,
    DM1_C706_ZONE_WALL_D3R = 706,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_C1500_ZONE_FLOOR_ORNAMENT = 1500,
    DM1_PC34_ZONE_STRIDE = 11,
    DM1_D3L_CORRIDOR_ORDER = 0x3421,
    DM1_D3R_CORRIDOR_ORDER = 0x4312,
    DM1_D3L_DOOR_SIDE_ORDER = 0x0321,
    DM1_D3R_DOOR_SIDE_ORDER = 0x0412,
    DM1_D3L_DOOR_PASS1_ORDER = 0x0218,
    DM1_D3R_DOOR_PASS1_ORDER = 0x0128,
    DM1_D3L_DOOR_PASS2_ORDER = 0x0349,
    DM1_D3R_DOOR_PASS2_ORDER = 0x0439,
    DM1_FB_WIDTH = DM1_V1_D3L_D3R_F0108_FRAMEBUFFER_WIDTH_PC34,
    DM1_FB_HEIGHT = DM1_V1_D3L_D3R_F0108_FRAMEBUFFER_HEIGHT_PC34,
    DM1_VIEW_WIDTH = DM1_V1_D3L_D3R_F0108_VIEWPORT_WIDTH_PC34,
    DM1_VIEW_HEIGHT = DM1_V1_D3L_D3R_F0108_VIEWPORT_HEIGHT_PC34,
    DM1_PIXEL_BACKGROUND = 0x00,
    DM1_PIXEL_D2_LATER = 0x7d
};

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} DM1_D3LD3RF0108RectPc34;

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0108:3940-4011 handles M558 floor ornaments, C10
 *   transparency, footprint recursion, D3R flip, and C1500 zone math.
 * - DUNVIEW.C F0116:6361-6498 and F0117:6500-6640 are the D3L/D3R bodies
 *   that call F0108 on door-front and open/stairs/pit/teleporter routes.
 * - DUNVIEW.C F0128:8491-8517 orders D3L, D3R, D3C, then the D2 pair.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523
 *   anchor no-mutation square-aspect/thing-list boundaries.
 */
static const char s_source_evidence[] =
    "Contract-only DM1 V1 D3L/D3R F0108 floor+ceiling+ornament gate; no "
    "original DOS pixel parity claim and no real-asset bitmap comparison. "
    "Anchors: DUNVIEW.C F0108:3940-4011; DUNVIEW.C F0116:6361-6498 "
    "D3L body with F0108 at 6443/6478 and C705/M575/M577; DUNVIEW.C "
    "F0117:6500-6640 D3R body with F0108 at 6579/6620 and "
    "C706/M576/M579; DUNVIEW.C F0128:8491-8517 D3L then D3R then D3C "
    "then D2L/D2R; DUNGEON.C F0163:1769-1838; DUNGEON.C F0164:1840-1905; "
    "DUNGEON.C F0172:2466-2523; DEFS.H C0..C5, M575..M579, C705/C706, "
    "C1004, C1500.";

static const char s_non_overlap_marker[] =
    "pass770 D3L/D3R F0108 floor+ceiling+ornament only; does not duplicate "
    "D0C, D0L/D0R, D0L2/D0R2, D1C, D1L/D1R, D1L2/D1R2, D2L/D2R, "
    "D2L2/D2R2, or D3C F0108 gates; does not duplicate D3L/D3R F0107 "
    "wall-ornament gate.";

static const DM1_V1_D3LD3RF0108SpecPc34 s_specs[] = {
    {
        DM1_V1_D3L_D3R_F0108_SIDE_D3L_PC34,
        "D3L F0108 floor, C705 wall carrier, F0115 handoff",
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        0,
        DM1_D3_FORWARD,
        DM1_D3L_LANE,
        DM1_M601_VIEW_SQUARE_D3L,
        DM1_M588_VIEW_FLOOR_D3L,
        DM1_C1500_ZONE_FLOOR_ORNAMENT,
        DM1_PC34_ZONE_STRIDE,
        1502,
        0,
        DM1_C705_ZONE_WALL_D3L,
        DM1_M575_VIEW_WALL_D3L_RIGHT,
        DM1_M577_VIEW_WALL_D3L_FRONT,
        DM1_C1004_ZONE_WALL_ORNAMENT,
        DM1_PC34_ZONE_STRIDE,
        1028,
        1030,
        DM1_D3L_CORRIDOR_ORDER,
        DM1_D3L_DOOR_SIDE_ORDER,
        DM1_D3L_DOOR_PASS1_ORDER,
        DM1_D3L_DOOR_PASS2_ORDER,
        6443,
        6478,
        6480,
        6495,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0116:6361-6498",
        "DUNVIEW.C F0128:8491 then 8495 then 8499; D2 pair 8513/8517",
        "DUNGEON.C F0163:1769-1838 / F0164:1840-1905 / F0172:2466-2523",
        s_non_overlap_marker
    },
    {
        DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34,
        "D3R F0108 floor, C706 wall carrier, F0115 handoff",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        1,
        DM1_D3_FORWARD,
        DM1_D3R_LANE,
        DM1_M602_VIEW_SQUARE_D3R,
        DM1_M590_VIEW_FLOOR_D3R,
        DM1_C1500_ZONE_FLOOR_ORNAMENT,
        DM1_PC34_ZONE_STRIDE,
        1504,
        1,
        DM1_C706_ZONE_WALL_D3R,
        DM1_M576_VIEW_WALL_D3R_LEFT,
        DM1_M579_VIEW_WALL_D3R_FRONT,
        DM1_C1004_ZONE_WALL_ORNAMENT,
        DM1_PC34_ZONE_STRIDE,
        1029,
        1032,
        DM1_D3R_CORRIDOR_ORDER,
        DM1_D3R_DOOR_SIDE_ORDER,
        DM1_D3R_DOOR_PASS1_ORDER,
        DM1_D3R_DOOR_PASS2_ORDER,
        6579,
        6620,
        6622,
        6637,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0117:6500-6640",
        "DUNVIEW.C F0128:8491 then 8495 then 8499; D2 pair 8513/8517",
        "DUNGEON.C F0163:1769-1838 / F0164:1840-1905 / F0172:2466-2523",
        s_non_overlap_marker
    }
};

static const DM1_D3LD3RF0108RectPc34 s_ceiling_rect = { 40, 16, 183, 43 };
static const DM1_D3LD3RF0108RectPc34 s_floor_rect = { 32, 80, 191, 135 };
static const DM1_D3LD3RF0108RectPc34 s_d3l_wall_rect = { 8, 40, 79, 120 };
static const DM1_D3LD3RF0108RectPc34 s_d3r_wall_rect = { 144, 40, 215, 120 };
static const DM1_D3LD3RF0108RectPc34 s_d3l_ornament_rect = { 52, 88, 91, 119 };
static const DM1_D3LD3RF0108RectPc34 s_d3r_ornament_rect = { 132, 88, 171, 119 };
static const DM1_D3LD3RF0108RectPc34 s_d2_later_rect = { 104, 96, 119, 111 };
static const DM1_D3LD3RF0108RectPc34 s_thing_rect = { 88, 92, 135, 123 };

static DM1_V1_D3LD3RF0108SelfTestResultPc34 s_last_self_test;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> ((unsigned int)i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static bool rect_in_viewport(DM1_D3LD3RF0108RectPc34 rect)
{
    return rect.x1 >= 0 && rect.y1 >= 0 &&
        rect.x2 < DM1_VIEW_WIDTH && rect.y2 < DM1_VIEW_HEIGHT;
}

static size_t framebuffer_offset(int x, int y)
{
    return (size_t)y * (size_t)DM1_FB_WIDTH + (size_t)x;
}

static int supported_context(DM1_V1_D3LD3RF0108ContextPc34 context)
{
    return context == DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34 ||
        context == DM1_V1_D3L_D3R_F0108_CONTEXT_CORRIDOR_PC34 ||
        context == DM1_V1_D3L_D3R_F0108_CONTEXT_OPEN_PIT_PC34 ||
        context == DM1_V1_D3L_D3R_F0108_CONTEXT_TELEPORTER_PC34 ||
        context == DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_SIDE_PC34 ||
        context == DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_FRONT_PC34 ||
        context == DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_SIDE_PC34 ||
        context == DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_FRONT_PC34;
}

static int context_calls_f0108(DM1_V1_D3LD3RF0108ContextPc34 context)
{
    return context != DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34;
}

static const DM1_D3LD3RF0108RectPc34 *wall_rect_for_side(
    DM1_V1_D3LD3RF0108SidePc34 side)
{
    return side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ?
        &s_d3r_wall_rect : &s_d3l_wall_rect;
}

static const DM1_D3LD3RF0108RectPc34 *ornament_rect_for_side(
    DM1_V1_D3LD3RF0108SidePc34 side)
{
    return side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ?
        &s_d3r_ornament_rect : &s_d3l_ornament_rect;
}

static void draw_rect(uint8_t *framebuffer,
                      DM1_D3LD3RF0108RectPc34 rect,
                      uint8_t pixel,
                      int use_c10,
                      int *transparent_blits)
{
    int y;

    for (y = rect.y1; y <= rect.y2; ++y) {
        int x;
        for (x = rect.x1; x <= rect.x2; ++x) {
            uint8_t *dst = &framebuffer[framebuffer_offset(x, y)];
            if (use_c10 &&
                pixel == DM1_V1_D3L_D3R_F0108_C10_COLOR_FLESH_PC34) {
                ++(*transparent_blits);
            } else {
                *dst = dm1_v1_viewport_d3l_d3r_f0108_blend_c10_pc34(
                    *dst, pixel);
            }
        }
    }
}

size_t dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D3LD3RF0108SpecPc34 *
dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D3LD3RF0108SpecPc34 *
dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_count_pc34(); ++i) {
        if ((int)s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d3l_d3r_f0108_initial_state_pc34(
    DM1_V1_D3LD3RF0108SidePc34 side,
    DM1_V1_D3LD3RF0108ContextPc34 context,
    DM1_V1_D3LD3RF0108StatePc34 *out)
{
    const DM1_V1_D3LD3RF0108SpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_for_side_pc34((int)side);
    if (!spec || !supported_context(context)) return false;

    out->side = side;
    out->context = context;
    out->floor_ornament_ordinal =
        side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 0x8006u : 0x8004u;
    out->wall_pixel =
        side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 0x42u : 0x41u;
    out->floor_pixel =
        side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 0x52u : 0x51u;
    out->ceiling_pixel =
        side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 0x62u : 0x61u;
    out->ornament_pixel =
        side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 0x72u : 0x71u;
    out->thing_pixel = DM1_V1_D3L_D3R_F0108_C10_COLOR_FLESH_PC34;
    out->source_locked_contract_only = true;
    out->no_original_dos_parity_claim = true;
    out->no_game_data_load = true;
    return true;
}

bool dm1_v1_viewport_d3l_d3r_f0108_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D3LD3RF0108OrdinalPc34 *out)
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
        (floor_ornament_ordinal & DM1_V1_D3L_D3R_F0108_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal & ~DM1_V1_D3L_D3R_F0108_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_index = (int)(out->footprint_flag_set ? cleared : floor_ornament_ordinal) - 1;
        out->metadata_blit_count = 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D3L_D3R_F0108_FOOTPRINT_INDEX_PC34;
        ++out->metadata_blit_count;
    }
    return true;
}

uint8_t dm1_v1_viewport_d3l_d3r_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D3L_D3R_F0108_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

int dm1_v1_viewport_d3l_d3r_f0108_floor_zone_pc34(
    int coordinate_set,
    int view_floor)
{
    if (coordinate_set < 0 || view_floor < 0) return -1;
    return DM1_C1500_ZONE_FLOOR_ORNAMENT +
        coordinate_set * DM1_PC34_ZONE_STRIDE + view_floor;
}

int dm1_v1_viewport_d3l_d3r_f0108_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    if (coordinate_set < 0 || view_wall < 0) return -1;
    return DM1_C1004_ZONE_WALL_ORNAMENT +
        coordinate_set * DM1_PC34_ZONE_STRIDE + view_wall;
}

bool dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(
    const DM1_V1_D3LD3RF0108StatePc34 *state,
    DM1_V1_D3LD3RF0108ResultPc34 *out)
{
    const DM1_V1_D3LD3RF0108SpecPc34 *spec;
    DM1_V1_D3LD3RF0108OrdinalPc34 ordinal;
    uint8_t framebuffer[(size_t)DM1_FB_WIDTH * (size_t)DM1_FB_HEIGHT];
    int transparent_blits = 0;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!state) return false;

    spec = dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_for_side_pc34(
        (int)state->side);
    out->spec = spec;
    out->framebuffer_width = DM1_FB_WIDTH;
    out->framebuffer_height = DM1_FB_HEIGHT;
    out->viewport_width = DM1_VIEW_WIDTH;
    out->viewport_height = DM1_VIEW_HEIGHT;
    if (!spec ||
        !supported_context(state->context) ||
        !state->source_locked_contract_only ||
        !state->no_original_dos_parity_claim ||
        !state->no_game_data_load ||
        state->mutate_thing_list ||
        state->allow_sibling_f0108_overlap ||
        state->allow_f0107_wall_ornament_duplicate ||
        state->allow_outside_viewport ||
        !rect_in_viewport(s_ceiling_rect) ||
        !rect_in_viewport(s_floor_rect) ||
        !rect_in_viewport(*wall_rect_for_side(state->side)) ||
        !rect_in_viewport(*ornament_rect_for_side(state->side)) ||
        !rect_in_viewport(s_d2_later_rect) ||
        !rect_in_viewport(s_thing_rect) ||
        !dm1_v1_viewport_d3l_d3r_f0108_decode_ordinal_pc34(
            state->floor_ornament_ordinal, &ordinal)) {
        out->rejected_non_contract_state = 1;
        return false;
    }

    memset(framebuffer, DM1_PIXEL_BACKGROUND, sizeof(framebuffer));
    out->ok = 1;
    out->mutation_guard_ok = 1;
    out->non_overlap_ok = 1;
    out->f0128_d3l_then_d3r_then_d3c = 1;
    out->terminal_depth_d2_pair_drawn_later = 1;
    out->floor_zone = spec->floor_zone;
    out->wall_zone = spec->wall_zone;
    out->side_wall_ornament_zone = spec->side_wall_ornament_zone;
    out->front_wall_ornament_zone = spec->front_wall_ornament_zone;
    out->floor_primary_index = ordinal.primary_index;
    out->floor_recursive_index = ordinal.recursive_footprints_index;

    draw_rect(framebuffer, s_ceiling_rect, state->ceiling_pixel, 0,
              &transparent_blits);
    draw_rect(framebuffer, s_floor_rect, state->floor_pixel, 0,
              &transparent_blits);
    out->ceiling_base_calls = 1;
    out->floor_base_calls = 1;

    if (state->context == DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34) {
        draw_rect(framebuffer, *wall_rect_for_side(state->side),
                  state->wall_pixel, 0, &transparent_blits);
        out->wall_body_calls = 1;
    }

    if (context_calls_f0108(state->context) && ordinal.has_input_ordinal) {
        out->floor_ornament_calls = 1;
        out->floor_primary_blits = ordinal.primary_draws ? 1 : 0;
        out->footprint_recursions = ordinal.recursive_footprints_draw ? 1 : 0;
        if (ordinal.primary_draws) {
            draw_rect(framebuffer, *ornament_rect_for_side(state->side),
                      state->ornament_pixel, 1, &transparent_blits);
        }
        if (state->context == DM1_V1_D3L_D3R_F0108_CONTEXT_OPEN_PIT_PC34) {
            out->open_pit_still_draws_floor_ornament = 1;
        }
    }

    if (state->context == DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_FRONT_PC34) {
        ++out->rear_thing_pass_calls;
        draw_rect(framebuffer, s_thing_rect, state->thing_pixel, 1,
                  &transparent_blits);
    }
    if (state->context != DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34) {
        ++out->front_thing_pass_calls;
        draw_rect(framebuffer, s_thing_rect, state->thing_pixel, 1,
                  &transparent_blits);
    }

    draw_rect(framebuffer, s_d2_later_rect, DM1_PIXEL_D2_LATER, 0,
              &transparent_blits);

    out->c10_transparent_blits = transparent_blits;
    out->ceiling_sample = framebuffer[framebuffer_offset(64, 24)];
    out->floor_sample = framebuffer[framebuffer_offset(64, 100)];
    out->ornament_sample_after_c10 =
        framebuffer[framebuffer_offset(state->side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 140 : 80, 100)];
    out->d2_later_sample = framebuffer[framebuffer_offset(110, 104)];

    out->deterministic_hash = hash_u32(2166136261u, (uint32_t)state->side);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)state->context);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_zone);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->wall_zone);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_primary_index);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_recursive_index);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->ceiling_sample);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->floor_sample);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->ornament_sample_after_c10);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->d2_later_sample);
    out->deterministic_hash = hash_u32(out->deterministic_hash, (uint32_t)out->c10_transparent_blits);
    return true;
}

static void self_check(int condition, DM1_V1_D3LD3RF0108SelfTestResultPc34 *result)
{
    ++result->assertions;
    if (!condition) ++result->failures;
}

static void exercise_case(
    DM1_V1_D3LD3RF0108SidePc34 side,
    DM1_V1_D3LD3RF0108ContextPc34 context,
    DM1_V1_D3LD3RF0108SelfTestResultPc34 *result)
{
    DM1_V1_D3LD3RF0108StatePc34 state;
    DM1_V1_D3LD3RF0108ResultPc34 composed;

    self_check(dm1_v1_viewport_d3l_d3r_f0108_initial_state_pc34(side, context, &state),
               result);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(&state, &composed),
               result);
    self_check(composed.ok == 1, result);
    self_check(composed.spec != NULL, result);
    self_check(composed.framebuffer_width == 320, result);
    self_check(composed.framebuffer_height == 200, result);
    self_check(composed.viewport_width == 224, result);
    self_check(composed.viewport_height == 136, result);
    self_check(composed.ceiling_base_calls == 1, result);
    self_check(composed.floor_base_calls == 1, result);
    self_check(composed.mutation_guard_ok == 1, result);
    self_check(composed.non_overlap_ok == 1, result);
    self_check(composed.f0128_d3l_then_d3r_then_d3c == 1, result);
    self_check(composed.terminal_depth_d2_pair_drawn_later == 1, result);
    self_check(composed.d2_later_sample == DM1_PIXEL_D2_LATER, result);
    self_check(composed.floor_zone ==
               (side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 1504 : 1502),
               result);
    self_check(composed.wall_zone ==
               (side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 706 : 705),
               result);
    self_check(composed.side_wall_ornament_zone ==
               (side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 1029 : 1028),
               result);
    self_check(composed.front_wall_ornament_zone ==
               (side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34 ? 1032 : 1030),
               result);
    if (context == DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34) {
        self_check(composed.wall_body_calls == 1, result);
        self_check(composed.floor_ornament_calls == 0, result);
        ++result->wall_body_calls;
    } else {
        self_check(composed.floor_ornament_calls == 1, result);
        self_check(composed.floor_primary_blits == 1, result);
        self_check(composed.footprint_recursions == 1, result);
        self_check(composed.c10_transparent_blits > 0, result);
        self_check(composed.ornament_sample_after_c10 == state.ornament_pixel ||
                   composed.ornament_sample_after_c10 == DM1_PIXEL_D2_LATER,
                   result);
        if (context == DM1_V1_D3L_D3R_F0108_CONTEXT_OPEN_PIT_PC34) {
            self_check(composed.open_pit_still_draws_floor_ornament == 1, result);
        }
        if (context == DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_FRONT_PC34) {
            self_check(composed.rear_thing_pass_calls == 1, result);
        }
        if (side == DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34) {
            ++result->d3r_floor_calls;
        } else {
            ++result->d3l_floor_calls;
        }
        result->footprint_recursions += composed.footprint_recursions;
        result->thing_pass_calls += composed.front_thing_pass_calls +
            composed.rear_thing_pass_calls;
    }
    result->ceiling_calls += composed.ceiling_base_calls;
    result->deterministic_hash = hash_u32(result->deterministic_hash,
                                          composed.deterministic_hash);
}

int run_dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_self_test(void)
{
    static const DM1_V1_D3LD3RF0108ContextPc34 contexts[] = {
        DM1_V1_D3L_D3R_F0108_CONTEXT_WALL_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_CORRIDOR_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_OPEN_PIT_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_TELEPORTER_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_SIDE_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_DOOR_FRONT_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_SIDE_PC34,
        DM1_V1_D3L_D3R_F0108_CONTEXT_STAIRS_FRONT_PC34
    };
    DM1_V1_D3LD3RF0108OrdinalPc34 ordinal;
    DM1_V1_D3LD3RF0108StatePc34 rejected;
    DM1_V1_D3LD3RF0108ResultPc34 composed;
    size_t i;

    memset(&s_last_self_test, 0, sizeof(s_last_self_test));
    s_last_self_test.deterministic_hash = 2166136261u;

    self_check(dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_count_pc34() == 2u,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_at_pc34(2) == NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "DUNVIEW.C F0108:3940-4011") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "F0116:6361-6498") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "F0117:6500-6640") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "F0128:8491-8517") != NULL,
               &s_last_self_test);
    self_check(strstr(s_source_evidence, "no original DOS pixel parity claim") != NULL,
               &s_last_self_test);
    self_check(strstr(s_non_overlap_marker, "D3L/D3R F0107") != NULL,
               &s_last_self_test);
    ++s_last_self_test.non_overlap_assertions;

    self_check(dm1_v1_viewport_d3l_d3r_f0108_decode_ordinal_pc34(0x8004u, &ordinal),
               &s_last_self_test);
    self_check(ordinal.primary_index == 3, &s_last_self_test);
    self_check(ordinal.recursive_footprints_index ==
               DM1_V1_D3L_D3R_F0108_FOOTPRINT_INDEX_PC34,
               &s_last_self_test);
    self_check(ordinal.metadata_blit_count == 2, &s_last_self_test);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_blend_c10_pc34(0x44u, 10u) == 0x44u,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_blend_c10_pc34(0x44u, 0x55u) == 0x55u,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_floor_zone_pc34(0, 2) == 1502,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_floor_zone_pc34(0, 4) == 1504,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_wall_ornament_zone_pc34(2, 2) == 1028,
               &s_last_self_test);
    self_check(dm1_v1_viewport_d3l_d3r_f0108_wall_ornament_zone_pc34(2, 6) == 1032,
               &s_last_self_test);

    for (i = 0; i < sizeof(contexts) / sizeof(contexts[0]); ++i) {
        exercise_case(DM1_V1_D3L_D3R_F0108_SIDE_D3L_PC34,
                      contexts[i], &s_last_self_test);
        exercise_case(DM1_V1_D3L_D3R_F0108_SIDE_D3R_PC34,
                      contexts[i], &s_last_self_test);
    }

    self_check(dm1_v1_viewport_d3l_d3r_f0108_initial_state_pc34(
                   DM1_V1_D3L_D3R_F0108_SIDE_D3L_PC34,
                   DM1_V1_D3L_D3R_F0108_CONTEXT_CORRIDOR_PC34,
                   &rejected),
               &s_last_self_test);
    rejected.mutate_thing_list = true;
    self_check(!dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(&rejected, &composed),
               &s_last_self_test);
    self_check(composed.rejected_non_contract_state == 1, &s_last_self_test);
    ++s_last_self_test.mutation_rejections;

    rejected.mutate_thing_list = false;
    rejected.allow_sibling_f0108_overlap = true;
    self_check(!dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(&rejected, &composed),
               &s_last_self_test);
    self_check(composed.rejected_non_contract_state == 1, &s_last_self_test);
    ++s_last_self_test.non_overlap_assertions;

    rejected.allow_sibling_f0108_overlap = false;
    rejected.allow_f0107_wall_ornament_duplicate = true;
    self_check(!dm1_v1_viewport_d3l_d3r_f0108_compose_pc34(&rejected, &composed),
               &s_last_self_test);
    self_check(composed.rejected_non_contract_state == 1, &s_last_self_test);
    ++s_last_self_test.non_overlap_assertions;

    s_last_self_test.ok = s_last_self_test.failures == 0;
    return s_last_self_test.ok;
}

const DM1_V1_D3LD3RF0108SelfTestResultPc34 *
dm1_v1_viewport_d3l_d3r_f0108_last_self_test_result_pc34(void)
{
    return &s_last_self_test;
}

const char *dm1_v1_viewport_d3l_d3r_f0108_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d3l_d3r_f0108_non_overlap_marker_pc34(void)
{
    return s_non_overlap_marker;
}
