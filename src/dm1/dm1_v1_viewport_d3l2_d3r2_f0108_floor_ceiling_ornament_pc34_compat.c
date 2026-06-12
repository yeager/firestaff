#include "dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_D3L2_VIEW_SQUARE = 14,
    DM1_D3R2_VIEW_SQUARE = 15,
    DM1_D3L2_VIEW_FLOOR = 0,
    DM1_D3R2_VIEW_FLOOR = 1,
    DM1_D3_DEPTH = 3,
    DM1_D3L2_LANE = -2,
    DM1_D3R2_LANE = 2,
    DM1_D3L2_WALL_ZONE = 702,
    DM1_D3R2_WALL_ZONE = 703,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_FLOOR_ZONE_STRIDE = 11,
    DM1_MUTATION_GUARD_BEFORE = 0x7080u,
    DM1_MUTATION_GUARD_AFTER = 0x8070u
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0108:3940-4011 draws nonzero floor ornaments, clears
 *   MASK0x8000_FOOTPRINTS, recurses to C15 footprints, and blits through
 *   C10_COLOR_FLESH.  F0108:3977-3983 flips C01_VIEW_FLOOR_D3R2.
 * - DUNVIEW.C F0098:2962-3014 is the floor/ceiling fallback used by
 *   F0128:8338-8443 before the D3L2/D3R2 post-D3C dispatch.
 * - DUNVIEW.C F0099:3018-3049 and F0128:8363/8425 copy rows locally when
 *   flipping floor/ceiling fallback buffers.
 * - DUNVIEW.C F0676:6226-6290 and F0677:6293-6357 dispatch D3L2/D3R2
 *   wall frames, F0108 floor ornaments, and the external F0115 thing pass.
 * - DUNVIEW.C F0128:8318-8486 performs the post-D3C follow-up and calls
 *   F0676/F0677 after D3C and D4 object passes.
 * - DUNVIEW.C F0115:4547-4581, 4794-4800, 4923, 5180-5188, 5211-5214,
 *   5668-5675, and 5920-5923 is a separate thing-pass contract for these
 *   cells and is intentionally a no-op here.
 * - DUNGEON.C F0163/F0164:1769-1840 and F0172:2466-2523 anchor thing-list
 *   mutation/classification inputs without mutating them in this contract.
 * - DEFS.H:2088, 2596-2611, 4139-4153, and 4205-4207 anchor C10, view
 *   squares, cell-order zones, and ornament metadata.
 */

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0108:3940-4011 floor ornament "
    "metadata, MASK0x8000_FOOTPRINTS clear, C10 transparent blit, and "
    "footprint recursion; DUNVIEW.C F0108:3977-3983 C01_VIEW_FLOOR_D3R2 "
    "F0099 flip; DUNVIEW.C F0098:2962-3014 floor/ceiling fallback; "
    "DUNVIEW.C F0099:3018-3049 row-local copy/flip parity; DUNVIEW.C "
    "F0128:8318-8486 post-D3C follow-up calls F0098 then F0676/F0677; "
    "DUNVIEW.C F0676:6226-6290 D3L2 frame/F0108/F0115 dispatch; "
    "DUNVIEW.C F0677:6293-6357 D3R2 frame/F0108/F0115 dispatch; "
    "DUNVIEW.C F0115:4547-4581,4794-4800,4923,5180-5188,5211-5214,"
    "5668-5675,5920-5923 separate thing-pass follow-up no-op here; "
    "DUNGEON.C F0163/F0164:1769-1840 thing-list mutation anchors; "
    "DUNGEON.C F0172:2466-2523 square aspect classification; "
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2596-2611 view squares; "
    "DEFS.H:4139-4153 cell-order zones; DEFS.H:4205-4207 ornament "
    "metadata; source_locked_contract_only no real-asset pixel parity.";

static const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 s_specs[] = {
    {
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34,
        "D3L2 post-D3C F0108 floor ornament plus F0098 ceiling fallback",
        DM1_D3L2_VIEW_SQUARE,
        DM1_D3L2_VIEW_FLOOR,
        DM1_D3_DEPTH,
        DM1_D3L2_LANE,
        DM1_D3L2_WALL_ZONE,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE,
        0x3421u,
        0x0218u,
        0x0349u,
        false,
        true,
        true,
        true,
        "DUNVIEW.C F0676:6226-6290",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0098:2962-3014",
        "DUNVIEW.C F0099:3018-3049",
        "DUNVIEW.C F0115:4547-4581/5180-5188",
        "DEFS.H:2088/2596-2611/4139-4153/4205-4207"
    },
    {
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34,
        "D3R2 post-D3C F0108 floor ornament plus F0098 ceiling fallback",
        DM1_D3R2_VIEW_SQUARE,
        DM1_D3R2_VIEW_FLOOR,
        DM1_D3_DEPTH,
        DM1_D3R2_LANE,
        DM1_D3R2_WALL_ZONE,
        DM1_FLOOR_ZONE_BASE,
        DM1_FLOOR_ZONE_STRIDE,
        0x4312u,
        0x0128u,
        0x0439u,
        true,
        true,
        true,
        true,
        "DUNVIEW.C F0677:6293-6357",
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0098:2962-3014",
        "DUNVIEW.C F0099:3018-3049",
        "DUNVIEW.C F0115:4547-4581/5180-5188",
        "DEFS.H:2088/2596-2611/4139-4153/4205-4207"
    }
};

static bool is_contract_square(int square_element)
{
    return square_element == DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_CORRIDOR_PC34 ||
           square_element == DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_PIT_PC34 ||
           square_element == DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_TELEPORTER_PC34 ||
           square_element == DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_DOOR_SIDE_PC34;
}

size_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_for_pc34(
    DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34 side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
    DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34 side,
    DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 *out)
{
    const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *spec;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    spec = dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_for_pc34(side);
    if (!spec) return false;

    out->side = side;
    out->square_element = DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_CORRIDOR_PC34;
    out->post_d3c_reached = true;
    out->enable_f0108_floor_ornament = true;
    out->enable_f0098_floor_fallback = true;
    out->enable_f0098_ceiling_fallback = true;
    out->f0115_thing_pass_already_covered = true;
    out->floor_ornament_ordinal = spec->side ==
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34 ? 0x8004u : 4u;
    out->floor_ornament_coordinate_set = spec->side ==
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34 ? 3 : 2;
    out->floor_ornament_native_bitmap_index = 240 + spec->view_floor;
    out->destination_pixel = 0x11u;
    out->f0098_floor_pixel = 0x21u;
    out->f0098_ceiling_pixel = 0x31u;
    out->f0108_ornament_pixel = 0x41u;
    out->f0115_synthetic_pixel =
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34;
    out->mutation_guard_before = DM1_MUTATION_GUARD_BEFORE;
    out->mutation_guard_after = DM1_MUTATION_GUARD_AFTER;
    return true;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D3L2D3R2F0108FloorCeilingOrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = floor_ornament_ordinal;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    out->primary_index = -1;
    out->recursive_footprints_index = -1;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set = (floor_ornament_ordinal &
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_ordinal = out->footprint_flag_set ? cleared : floor_ornament_ordinal;
        out->primary_index = (int)out->primary_ordinal - 1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_FOOTPRINT_INDEX_PC34;
        out->recursive_footprints_ordinal =
            (unsigned int)DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_FOOTPRINT_INDEX_PC34 + 1u;
    }
    return true;
}

uint8_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_flip_row_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t width,
    size_t row_count)
{
    size_t row;

    if (!source || !destination || width == 0u || row_count == 0u) return false;
    for (row = 0u; row < row_count; ++row) {
        size_t col;
        const size_t base = row * width;
        for (col = 0u; col < width; ++col) {
            destination[base + col] = source[base + (width - 1u - col)];
        }
    }
    return true;
}

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(
    const DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 *state,
    DM1_V1_D3L2D3R2F0108FloorCeilingResultPc34 *out)
{
    const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *spec;
    DM1_V1_D3L2D3R2F0108FloorCeilingOrdinalPc34 ordinal;
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->floor_ornament_primary_index = -1;
    out->footprint_recursion_index = -1;
    if (!state) return false;

    spec = dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_for_pc34(state->side);
    out->spec = spec;
    if (!spec || !state->post_d3c_reached || !is_contract_square(state->square_element) ||
        state->attempts_f0107_wall_ornament || state->attempts_f0111_door ||
        state->floor_ornament_coordinate_set < 0 || state->floor_ornament_native_bitmap_index < 0 ||
        state->mutation_guard_before != DM1_MUTATION_GUARD_BEFORE ||
        state->mutation_guard_after != DM1_MUTATION_GUARD_AFTER) {
        out->rejectsNonContractState = 1;
        return false;
    }

    dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(
        state->floor_ornament_ordinal, &ordinal);

    out->source_locked_contract_only = true;
    out->no_real_asset_bitmap_parity = true;
    out->no_game_data_load = true;
    out->view_square = spec->view_square;
    out->view_floor = spec->view_floor;
    out->open_cell_order = (int)spec->open_cell_order;
    out->floor_ornament_zone = spec->floor_zone_base +
        state->floor_ornament_coordinate_set * spec->floor_zone_stride + spec->view_floor;
    out->f0676FrameCount = spec->side ==
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34 ? 1 : 0;
    out->f0677FrameCount = spec->side ==
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34 ? 1 : 0;
    out->f0128PostD3cCount = 1;
    out->f0098FloorCount = state->enable_f0098_floor_fallback ? 1 : 0;
    out->f0098CeilingCount = state->enable_f0098_ceiling_fallback ? 1 : 0;
    out->f0099FlipCount = spec->f0108_right_side_flips ? 1 : 0;
    out->f0115ThingPassNoOpCount = state->f0115_thing_pass_already_covered ? 1 : 0;
    out->mutationGuardsOk = 1;
    out->f0107NonOverlapOk = !state->attempts_f0107_wall_ornament;
    out->f0111NonOverlapOk = !state->attempts_f0111_door;
    out->nonOverlapWithF0107F0111 = out->f0107NonOverlapOk && out->f0111NonOverlapOk;
    out->row_local_parity_ok = spec->f0099_row_local_flip_preserved;

    pixel = state->destination_pixel;
    if (state->enable_f0098_floor_fallback) {
        if (state->f0098_floor_pixel ==
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34) {
            ++out->c10TransparentBlitCount;
        }
        pixel = dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_blend_pixel_pc34(
            pixel, state->f0098_floor_pixel,
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34);
    }
    out->after_f0098_floor = pixel;

    if (state->enable_f0108_floor_ornament && ordinal.has_input_ordinal) {
        out->f0108OrnamentCount = ordinal.primary_draws ? 1 : 0;
        if (ordinal.recursive_footprints_draw) ++out->f0108OrnamentCount;
        out->floor_ornament_primary_index = ordinal.primary_index;
        out->footprint_recursion_index = ordinal.recursive_footprints_index;
        if (state->f0108_ornament_pixel ==
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34) {
            ++out->c10TransparentBlitCount;
        }
        pixel = dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_blend_pixel_pc34(
            pixel, state->f0108_ornament_pixel,
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34);
    }
    out->after_f0108_ornament = pixel;

    if (state->enable_f0098_ceiling_fallback) {
        if (state->f0098_ceiling_pixel ==
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34) {
            ++out->c10TransparentBlitCount;
        }
        pixel = dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_blend_pixel_pc34(
            pixel, state->f0098_ceiling_pixel,
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34);
    }
    out->after_f0098_ceiling = pixel;
    if (state->f0115_thing_pass_already_covered &&
        state->f0115_synthetic_pixel ==
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_ORNAMENT_C10_COLOR_FLESH_PC34) {
        ++out->c10TransparentBlitCount;
    }
    out->after_f0115_noop = pixel;
    return true;
}

const char *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}
