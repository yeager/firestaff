#include "dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_C09_VIEW_SQUARE_D2L2 = 9,
    DM1_C10_VIEW_SQUARE_D2R2 = 10,
    DM1_M604_VIEW_SQUARE_D2L = 7,
    DM1_M605_VIEW_SQUARE_D2R = 8,
    DM1_M591_VIEW_FLOOR_D2L = 5,
    DM1_M593_VIEW_FLOOR_D2R = 7,
    DM1_M558_FLOOR_ORNAMENT_ORDINAL = 5,
    DM1_C1500_ZONE_FLOOR_ORNAMENT = 1500,
    DM1_PC34_FLOOR_ORNAMENT_ZONE_STRIDE = 11
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0108:3940-4011 reads M558 floor-ornament ordinals, clears
 *   MASK0x8000_FOOTPRINTS, draws metadata-selected floor ornaments through
 *   G0102/G0191 and C1500 + CoordinateSet * 11 + view-floor, uses
 *   C10_COLOR_FLESH transparency, and recurses to C15 footprints.
 * - DUNVIEW.C F0118:6642-6763 proves the immediately prior D3C dispatch
 *   reaches its own F0108 route before F0128 moves to D2L2/D2R2/D2L/D2R.
 * - DUNVIEW.C F0115:4547-4581 and 5668-5671 define the thing-pass loops
 *   and the view-square row guard that this floor-ornament contract does
 *   not mutate.
 * - DUNVIEW.C F0678/F0679:6837-6896 draw only D2L2/D2R2 lateral-2 wall
 *   frames or fields; they do not own F0108, F0107, F0111, or F0115.
 * - DUNVIEW.C F0128:8503-8517 orders F0678, F0679, then the D2L/D2R
 *   F0108-capable near-side square dispatches.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523
 *   anchor thing-list mutation and square-aspect construction.
 * - DEFS.H:2088,2443-2452,2582-2583,2596-2604,2662,2668-2677,
 *   2681-2707,4144-4162,4202-4207,4223 anchor C10, stair/pit zones,
 *   D2 view squares, cell orders, M575..M579 wall-view ordinals, and
 *   floor-ornament metadata zone bases.
 */

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0108:3940-4011 floor ornament "
    "metadata, MASK0x8000_FOOTPRINTS clear, C10_COLOR_FLESH blit, "
    "C1500 + CoordinateSet * 11 + view-floor zone, and C15 footprint "
    "recursion; DUNVIEW.C F0118:6642-6763 D3C dispatch before the D2 row; "
    "DUNVIEW.C F0115:4547-4581 thing-pass loop and DUNVIEW.C "
    "F0115:5668-5671 view-square row guard; DUNVIEW.C F0678/F0679:"
    "6837-6896 D2L2/D2R2 lateral-2 frame dispatch without F0108; "
    "DUNVIEW.C F0128:8503-8517 orders F0678, F0679, F0119, F0120; "
    "DUNVIEW.C F0119:6987-7031 and F0120:7180-7224 route D2L/D2R "
    "floor ornaments to M591/M593 and then F0115; DUNGEON.C "
    "F0163:1769-1838, F0164:1840-1905, F0172:2466-2523 thing-list "
    "and aspect guards; DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2443-2452 "
    "D2 stair bitmap ids; DEFS.H:2582-2583 and 2596-2604 D2 view squares; "
    "DEFS.H:2662 and 2668-2677 cell orders; DEFS.H:2681-2707 "
    "M575..M579 view-wall ordinals; DEFS.H:4144-4162 and 4202-4207 "
    "D2 stair/pit zones; DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT.";

static const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 s_specs[] = {
    {
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34,
        "D2L near-side floor ornament after D2L2 frame",
        0,
        DM1_C09_VIEW_SQUARE_D2L2,
        DM1_M604_VIEW_SQUARE_D2L,
        DM1_M591_VIEW_FLOOR_D2L,
        DM1_M558_FLOOR_ORNAMENT_ORDINAL,
        DM1_C1500_ZONE_FLOOR_ORNAMENT,
        DM1_PC34_FLOOR_ORNAMENT_ZONE_STRIDE,
        0,
        2, 3, 4, 5, 6,
        0x3421u,
        0x0218u,
        0x0349u,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0128:8503-8517 / F0119:6987-7031",
        "DUNVIEW.C F0115:4547-4581 and 5668-5671",
        "DEFS.H:2088,2596-2604,2668-2677,2681-2707,4223"
    },
    {
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2R_PC34,
        "D2R near-side floor ornament after D2R2 frame",
        1,
        DM1_C10_VIEW_SQUARE_D2R2,
        DM1_M605_VIEW_SQUARE_D2R,
        DM1_M593_VIEW_FLOOR_D2R,
        DM1_M558_FLOOR_ORNAMENT_ORDINAL,
        DM1_C1500_ZONE_FLOOR_ORNAMENT,
        DM1_PC34_FLOOR_ORNAMENT_ZONE_STRIDE,
        1,
        2, 3, 4, 5, 6,
        0x4312u,
        0x0128u,
        0x0439u,
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0128:8503-8517 / F0120:7180-7224",
        "DUNVIEW.C F0115:4547-4581 and 5668-5671",
        "DEFS.H:2088,2596-2604,2668-2677,2681-2707,4223"
    }
};

size_t dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_for_pc34(
    DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34 side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_count_pc34(); ++i) {
        if (s_specs[i].side == side) {
            return &s_specs[i];
        }
    }
    return NULL;
}

DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_initial_state_pc34(
    DM1_V1_D2L2D2R2F0108FloorOrnamentSidePc34 side,
    DM1_V1_D2L2D2R2F0108FloorOrnamentContextPc34 context)
{
    DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 state;

    memset(&state, 0, sizeof(state));
    state.side = side;
    state.context = context;
    state.floor_ornament_ordinal = 0x8004u;
    state.first_thing_before = side == DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34 ?
        0x1201u : 0x1202u;
    state.destination_pixel_before = side == DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34 ?
        0x31u : 0x32u;
    state.floor_ornament_pixel = side == DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34 ?
        0x41u : 0x42u;
    state.contract_enabled = true;
    return state;
}

bool dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D2L2D2R2F0108FloorOrnamentOrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = floor_ornament_ordinal;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set =
        (floor_ornament_ordinal &
         DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_ordinal = out->footprint_flag_set ? cleared : floor_ornament_ordinal;
        out->primary_index = (int)out->primary_ordinal - 1;
        out->metadata_blit_count = 1;
    } else {
        out->primary_index = -1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34;
        out->recursive_footprints_ordinal =
            (unsigned int)DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_FOOTPRINT_INDEX_PC34 + 1u;
        ++out->metadata_blit_count;
    }
    return true;
}

uint8_t dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_blit_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel ==
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_compose_pc34(
    const DM1_V1_D2L2D2R2F0108FloorOrnamentStatePc34 *state,
    DM1_V1_D2L2D2R2F0108FloorOrnamentResultPc34 *out)
{
    DM1_V1_D2L2D2R2F0108FloorOrnamentOrdinalPc34 ordinal;
    const DM1_V1_D2L2D2R2F0108FloorOrnamentSpecPc34 *spec;
    bool non_contract;

    if (!state || !out) return false;
    memset(out, 0, sizeof(*out));
    out->state = *state;
    spec = dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_for_pc34(state->side);
    out->spec = spec;
    if (!spec) {
        out->rejected_non_contract_state = true;
        return false;
    }

    non_contract = !state->contract_enabled ||
        state->allow_f0107_wall_overlap ||
        state->allow_f0111_door_overlap ||
        state->mutate_thing_list;
    if (non_contract) {
        out->rejected_non_contract_state = true;
        return false;
    }

    if (state->side == DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_SIDE_D2L_PC34) {
        out->f0678FrameCount = 1;
        out->d2l2CellThingUnchanged = 1;
    } else {
        out->f0679FrameCount = 1;
        out->d2r2CellThingUnchanged = 1;
    }
    out->f0128PostCount = 1;
    out->mutationGuardsOk = 1;
    out->nonOverlapWithF0107F0111 = 1;
    out->f0115ThingPassNoOpCount = 1;
    out->destination_pixel_after = state->destination_pixel_before;

    if (state->context ==
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_CONTEXT_OPEN_PIT_PC34) {
        out->f0108OpenPitSkipCount = 1;
        out->open_pit_preserved = true;
        return true;
    }

    if (!dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_decode_ordinal_pc34(
            state->floor_ornament_ordinal, &ordinal)) {
        return false;
    }
    out->f0108FootprintRecursionCount = ordinal.recursive_footprints_draw ? 1 : 0;
    out->f0108OrnamentMetadataCount = ordinal.metadata_blit_count;
    out->c10TransparentBlitCount =
        state->floor_ornament_pixel ==
        DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34 ? 1 : 0;
    if (ordinal.primary_draws) {
        out->destination_pixel_after =
            dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_blit_pixel_pc34(
                state->destination_pixel_before, state->floor_ornament_pixel);
        out->floor_ornament_drawn =
            state->floor_ornament_pixel !=
            DM1_V1_D2L2_D2R2_F0108_FLOOR_ORNAMENT_C10_COLOR_FLESH_PC34;
    }
    return true;
}

const char *
dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}
