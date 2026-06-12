#include "dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define CHECK_EQ(ID, GOT, WANT, ANCHOR)                                      \
    do {                                                                     \
        const int got_value__ = (int)(GOT);                                  \
        const int want_value__ = (int)(WANT);                                \
        ++g_assertions;                                                      \
        if (got_value__ != want_value__) {                                   \
            printf("FAIL %s got=%d want=%d anchor=%s\n",                    \
                   (ID), got_value__, want_value__, (ANCHOR));              \
            ++g_failures;                                                    \
        }                                                                    \
    } while (0)

static void check_contains(const char *id, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)",
               anchor);
        ++g_failures;
    }
}

static void check_spec(
    const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *s,
    int side,
    int view_square,
    int view_floor,
    int lane,
    int wall_zone,
    unsigned int open_order,
    unsigned int pass1_order,
    unsigned int pass2_order,
    int flips)
{
    CHECK_EQ("spec.present", s != NULL, 1, "DUNVIEW.C F0676/F0677");
    if (!s) return;

    CHECK_EQ("spec.side", s->side, side, "DUNVIEW.C F0676/F0677 side");
    CHECK_EQ("spec.view_square", s->view_square, view_square,
             "DEFS.H:2610-2611 C14/C15");
    CHECK_EQ("spec.view_floor", s->view_floor, view_floor,
             "DEFS.H:2750-2751 C00/C01 view floor");
    CHECK_EQ("spec.depth", s->view_depth, 3, "DUNVIEW.C G2027 depth");
    CHECK_EQ("spec.lane", s->view_lane, lane, "DUNVIEW.C G2026 lane");
    CHECK_EQ("spec.wall_zone", s->wall_zone, wall_zone,
             "DEFS.H:4042-4043 C702/C703");
    CHECK_EQ("spec.zone_base", s->floor_zone_base, 1500,
             "DUNVIEW.C F0108 C1500 zone base");
    CHECK_EQ("spec.zone_stride", s->floor_zone_stride, 11,
             "DUNVIEW.C F0108 PC34 coordinate stride");
    CHECK_EQ("spec.open_order", (int)s->open_cell_order, (int)open_order,
             "DEFS.H:2676-2677 open order");
    CHECK_EQ("spec.pass1_order", (int)s->door_pass1_cell_order, (int)pass1_order,
             "DEFS.H:2668-2669 door pass1 order");
    CHECK_EQ("spec.pass2_order", (int)s->door_pass2_cell_order, (int)pass2_order,
             "DEFS.H:2672/2675 door pass2 order");
    CHECK_EQ("spec.flips", s->f0108_right_side_flips ? 1 : 0, flips,
             "DUNVIEW.C F0108:3977-3983");
    CHECK_EQ("spec.row_local", s->f0099_row_local_flip_preserved ? 1 : 0, 1,
             "DUNVIEW.C F0099 row-local parity");
    CHECK_EQ("spec.post_d3c", s->f0128_dispatch_after_d3c ? 1 : 0, 1,
             "DUNVIEW.C F0128:8318-8486");
    CHECK_EQ("spec.f0115_external", s->f0115_contract_external ? 1 : 0, 1,
             "DUNVIEW.C F0115 separate pass");
    CHECK_EQ("spec.anchor.f067x", strstr(s->redmcsb_f067x_anchor, "F067") != NULL, 1,
             "F0676/F0677 source anchor");
    CHECK_EQ("spec.anchor.f0108", strstr(s->redmcsb_f0108_anchor, "F0108") != NULL, 1,
             "F0108 source anchor");
    CHECK_EQ("spec.anchor.f0098", strstr(s->redmcsb_f0098_anchor, "F0098") != NULL, 1,
             "F0098 source anchor");
    CHECK_EQ("spec.anchor.f0099", strstr(s->redmcsb_f0099_anchor, "F0099") != NULL, 1,
             "F0099 source anchor");
    CHECK_EQ("spec.anchor.f0115", strstr(s->redmcsb_f0115_anchor, "F0115") != NULL, 1,
             "F0115 source anchor");
    CHECK_EQ("spec.anchor.defs", strstr(s->redmcsb_defs_anchor, "DEFS.H") != NULL, 1,
             "DEFS.H source anchor");
    CHECK_EQ("spec.label", strstr(s->label, "D3") != NULL, 1,
             "human-readable spec label");
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_source_evidence_pc34();

    check_contains("evidence.f0108.main", e, "DUNVIEW.C F0108:3940-4011",
                   "mandatory F0108 source anchor");
    check_contains("evidence.f0108.flip", e, "F0108:3977-3983",
                   "F0108 D3R2 flip anchor");
    check_contains("evidence.f0098", e, "DUNVIEW.C F0098:2962-3014",
                   "mandatory F0098 source anchor");
    check_contains("evidence.f0099", e, "DUNVIEW.C F0099:3018-3049",
                   "mandatory F0099 source anchor");
    check_contains("evidence.f0128", e, "DUNVIEW.C F0128:8318-8486",
                   "mandatory F0128 post-D3C anchor");
    check_contains("evidence.f0676", e, "DUNVIEW.C F0676:6226-6290",
                   "mandatory F0676 D3L2 anchor");
    check_contains("evidence.f0677", e, "DUNVIEW.C F0677:6293-6357",
                   "mandatory F0677 D3R2 anchor");
    check_contains("evidence.f0115", e, "DUNVIEW.C F0115:4547-4581",
                   "mandatory F0115 anchor");
    check_contains("evidence.f0115.blit", e, "5180-5188",
                   "F0115 C10 blit anchor");
    check_contains("evidence.f0163_f0164", e, "DUNGEON.C F0163/F0164:1769-1840",
                   "mandatory DUNGEON.C thing-list anchor");
    check_contains("evidence.f0172", e, "DUNGEON.C F0172:2466-2523",
                   "mandatory DUNGEON.C square-aspect anchor");
    check_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                   "mandatory C10 anchor");
    check_contains("evidence.view_squares", e, "DEFS.H:2596-2611",
                   "mandatory view-square anchor");
    check_contains("evidence.cell_order_zones", e, "DEFS.H:4139-4153",
                   "mandatory cell-order zone anchor");
    check_contains("evidence.ornament_metadata", e, "DEFS.H:4205-4207",
                   "mandatory ornament metadata anchor");
    check_contains("evidence.contract_only", e, "source_locked_contract_only",
                   "synthetic contract-only declaration");
    check_contains("evidence.no_pixel_parity", e, "no real-asset pixel parity",
                   "no real-asset parity claim");
}

static void test_specs(void)
{
    const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *d3l2 =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_for_pc34(
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34);
    const DM1_V1_D3L2D3R2F0108FloorCeilingSpecPc34 *d3r2 =
        dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_for_pc34(
            DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34);

    CHECK_EQ("count", dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_count_pc34(),
             2, "D3L2 plus D3R2");
    CHECK_EQ("at0", dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_at_pc34(0) == d3l2,
             1, "stable spec order");
    CHECK_EQ("at1", dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_at_pc34(1) == d3r2,
             1, "stable spec order");
    CHECK_EQ("at2.null", dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_at_pc34(2) == NULL,
             1, "bounds guard");
    CHECK_EQ("bad.side.null",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_for_pc34(
                 (DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34)9) == NULL,
             1, "unknown side rejected");

    check_spec(d3l2, 0, 14, 0, -2, 702, 0x3421u, 0x0218u, 0x0349u, 0);
    check_spec(d3r2, 1, 15, 1, 2, 703, 0x4312u, 0x0128u, 0x0439u, 1);
}

static void test_initial_state_and_ordinal(void)
{
    DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 state;
    DM1_V1_D3L2D3R2F0108FloorCeilingOrdinalPc34 ordinal;

    CHECK_EQ("initial.null",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
                 DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34, NULL),
             0, "initial-state null guard");
    CHECK_EQ("initial.bad_side",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
                 (DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34)7, &state),
             0, "initial-state side guard");
    CHECK_EQ("initial.d3l2",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
                 DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34, &state),
             1, "D3L2 initial-state");
    CHECK_EQ("initial.side", state.side, 0, "D3L2 state side");
    CHECK_EQ("initial.square", state.square_element, 1, "corridor square");
    CHECK_EQ("initial.post_d3c", state.post_d3c_reached ? 1 : 0, 1,
             "F0128 post-D3C reached");
    CHECK_EQ("initial.f0108", state.enable_f0108_floor_ornament ? 1 : 0, 1,
             "F0108 enabled");
    CHECK_EQ("initial.f0098_floor", state.enable_f0098_floor_fallback ? 1 : 0, 1,
             "F0098 floor fallback");
    CHECK_EQ("initial.f0098_ceiling", state.enable_f0098_ceiling_fallback ? 1 : 0, 1,
             "F0098 ceiling fallback");
    CHECK_EQ("initial.f0115_external", state.f0115_thing_pass_already_covered ? 1 : 0, 1,
             "F0115 external no-op");
    CHECK_EQ("initial.no_f0107", state.attempts_f0107_wall_ornament ? 1 : 0, 0,
             "F0107 non-overlap");
    CHECK_EQ("initial.no_f0111", state.attempts_f0111_door ? 1 : 0, 0,
             "F0111 non-overlap");
    CHECK_EQ("initial.ordinal", (int)state.floor_ornament_ordinal, 4,
             "F0108 ordinary ordinal");
    CHECK_EQ("initial.coordinate", state.floor_ornament_coordinate_set, 2,
             "F0108 coordinate set");
    CHECK_EQ("initial.native", state.floor_ornament_native_bitmap_index, 240,
             "F0108 native bitmap");
    CHECK_EQ("initial.dest", state.destination_pixel, 0x11, "synthetic pixel");
    CHECK_EQ("initial.floor", state.f0098_floor_pixel, 0x21, "synthetic F0098 floor");
    CHECK_EQ("initial.ceiling", state.f0098_ceiling_pixel, 0x31, "synthetic F0098 ceiling");
    CHECK_EQ("initial.ornament", state.f0108_ornament_pixel, 0x41, "synthetic F0108");
    CHECK_EQ("initial.f0115_pixel", state.f0115_synthetic_pixel, 10, "C10 no-op");
    CHECK_EQ("initial.guard_before", (int)state.mutation_guard_before, 0x7080,
             "mutation guard before");
    CHECK_EQ("initial.guard_after", (int)state.mutation_guard_after, 0x8070,
             "mutation guard after");

    CHECK_EQ("decode.null",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(1, NULL),
             0, "ordinal null guard");
    CHECK_EQ("decode.zero",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(0, &ordinal),
             1, "F0108 zero ordinal");
    CHECK_EQ("decode.zero.has", ordinal.has_input_ordinal ? 1 : 0, 0,
             "zero ordinal no draw");
    CHECK_EQ("decode.zero.primary", ordinal.primary_draws ? 1 : 0, 0,
             "zero ordinal no primary");
    CHECK_EQ("decode.zero.primary_index", ordinal.primary_index, -1,
             "zero ordinal primary index");
    CHECK_EQ("decode.zero.recursive", ordinal.recursive_footprints_draw ? 1 : 0, 0,
             "zero ordinal no recursion");
    CHECK_EQ("decode.primary",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(4, &ordinal),
             1, "F0108 primary ordinal");
    CHECK_EQ("decode.primary.has", ordinal.has_input_ordinal ? 1 : 0, 1,
             "primary has ordinal");
    CHECK_EQ("decode.primary.flag", ordinal.footprint_flag_set ? 1 : 0, 0,
             "primary no footprint flag");
    CHECK_EQ("decode.primary.cleared", (int)ordinal.cleared_ordinal, 4,
             "primary cleared ordinal");
    CHECK_EQ("decode.primary.draws", ordinal.primary_draws ? 1 : 0, 1,
             "primary draws");
    CHECK_EQ("decode.primary.ordinal", (int)ordinal.primary_ordinal, 4,
             "primary ordinal");
    CHECK_EQ("decode.primary.index", ordinal.primary_index, 3,
             "primary index");
    CHECK_EQ("decode.primary.recursive", ordinal.recursive_footprints_draw ? 1 : 0, 0,
             "primary no recursion");
    CHECK_EQ("decode.footprint",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(0x8004u, &ordinal),
             1, "F0108 footprint ordinal");
    CHECK_EQ("decode.footprint.flag", ordinal.footprint_flag_set ? 1 : 0, 1,
             "footprint flag set");
    CHECK_EQ("decode.footprint.cleared", (int)ordinal.cleared_ordinal, 4,
             "footprint clears mask");
    CHECK_EQ("decode.footprint.primary", ordinal.primary_draws ? 1 : 0, 1,
             "footprint primary draw");
    CHECK_EQ("decode.footprint.index", ordinal.primary_index, 3,
             "footprint primary index");
    CHECK_EQ("decode.footprint.recursive", ordinal.recursive_footprints_draw ? 1 : 0, 1,
             "footprint recursion");
    CHECK_EQ("decode.footprint.recursive_ordinal", (int)ordinal.recursive_footprints_ordinal, 16,
             "C15 index to ordinal");
    CHECK_EQ("decode.footprint.recursive_index", ordinal.recursive_footprints_index, 15,
             "C15 footprints index");
    CHECK_EQ("decode.only_footprint",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_decode_ordinal_pc34(0x8000u, &ordinal),
             1, "F0108 footprint-only ordinal");
    CHECK_EQ("decode.only_footprint.primary", ordinal.primary_draws ? 1 : 0, 0,
             "footprint-only no primary");
    CHECK_EQ("decode.only_footprint.recursive", ordinal.recursive_footprints_draw ? 1 : 0, 1,
             "footprint-only recursion");
    CHECK_EQ("decode.only_footprint.index", ordinal.recursive_footprints_index, 15,
             "footprint-only C15 index");
}

static void check_full_composition(
    DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34 side,
    int want_f0676,
    int want_f0677,
    int want_view_square,
    int want_view_floor,
    int want_order,
    int want_flip_count,
    int want_zone,
    int want_primary_index,
    int want_footprint_index)
{
    DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 state;
    DM1_V1_D3L2D3R2F0108FloorCeilingResultPc34 result;

    CHECK_EQ("compose.initial",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(side, &state),
             1, "initial state for composition");
    CHECK_EQ("compose.run",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             1, "F0098/F0108/F0099 composition");
    CHECK_EQ("compose.spec", result.spec != NULL, 1, "composition spec");
    CHECK_EQ("compose.f0098_floor", result.f0098FloorCount, 1,
             "F0098 floor fallback count");
    CHECK_EQ("compose.f0098_ceiling", result.f0098CeilingCount, 1,
             "F0098 ceiling fallback count");
    CHECK_EQ("compose.f0099_flip", result.f0099FlipCount, want_flip_count,
             "F0099 row-local flip count");
    CHECK_EQ("compose.f0108", result.f0108OrnamentCount,
             want_footprint_index >= 0 ? 2 : 1, "F0108 primary/footprint count");
    CHECK_EQ("compose.f0115_noop", result.f0115ThingPassNoOpCount, 1,
             "F0115 thing-pass non-overlap no-op");
    CHECK_EQ("compose.c10_count", result.c10TransparentBlitCount, 1,
             "C10 F0115 synthetic pixel ignored");
    CHECK_EQ("compose.f0676", result.f0676FrameCount, want_f0676,
             "F0676 D3L2 frame dispatch count");
    CHECK_EQ("compose.f0677", result.f0677FrameCount, want_f0677,
             "F0677 D3R2 frame dispatch count");
    CHECK_EQ("compose.f0128", result.f0128PostD3cCount, 1,
             "F0128 post-D3C count");
    CHECK_EQ("compose.mutation", result.mutationGuardsOk, 1,
             "mutation guards remain intact");
    CHECK_EQ("compose.f0107_nonoverlap", result.f0107NonOverlapOk, 1,
             "F0107 non-overlap");
    CHECK_EQ("compose.f0111_nonoverlap", result.f0111NonOverlapOk, 1,
             "F0111 non-overlap");
    CHECK_EQ("compose.nonoverlap", result.nonOverlapWithF0107F0111, 1,
             "F0107/F0111 non-overlap");
    CHECK_EQ("compose.reject_flag", result.rejectsNonContractState, 0,
             "accepted contract state");
    CHECK_EQ("compose.zone", result.floor_ornament_zone, want_zone,
             "F0108 floor ornament zone");
    CHECK_EQ("compose.primary_index", result.floor_ornament_primary_index, want_primary_index,
             "F0108 ordinal-to-index");
    CHECK_EQ("compose.footprint_index", result.footprint_recursion_index, want_footprint_index,
             "F0108 footprint recursion index");
    CHECK_EQ("compose.view_square", result.view_square, want_view_square,
             "D3L2/D3R2 view square");
    CHECK_EQ("compose.view_floor", result.view_floor, want_view_floor,
             "D3L2/D3R2 view floor");
    CHECK_EQ("compose.open_order", result.open_cell_order, want_order,
             "open-cell order invariant");
    CHECK_EQ("compose.row_local", result.row_local_parity_ok ? 1 : 0, 1,
             "F0099 row-local parity");
    CHECK_EQ("compose.contract", result.source_locked_contract_only ? 1 : 0, 1,
             "contract-only");
    CHECK_EQ("compose.no_pixels", result.no_real_asset_bitmap_parity ? 1 : 0, 1,
             "no real asset parity");
    CHECK_EQ("compose.no_data", result.no_game_data_load ? 1 : 0, 1,
             "no game-data load");
    CHECK_EQ("compose.after_floor", result.after_f0098_floor, 0x21,
             "F0098 floor synthetic blend");
    CHECK_EQ("compose.after_ornament", result.after_f0108_ornament, 0x41,
             "F0108 synthetic blend");
    CHECK_EQ("compose.after_ceiling", result.after_f0098_ceiling, 0x31,
             "F0098 ceiling synthetic blend");
    CHECK_EQ("compose.after_f0115", result.after_f0115_noop, result.after_f0098_ceiling,
             "F0115 thing pass no-op");
}

static void test_full_composition(void)
{
    check_full_composition(
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34,
        1, 0, 14, 0, 0x3421, 0, 1500 + 2 * 11 + 0, 3, -1);
    check_full_composition(
        DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34,
        0, 1, 15, 1, 0x4312, 1, 1500 + 3 * 11 + 1, 3, 15);
}

static void test_c10_transparency_and_f0098_fallback(void)
{
    DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 state;
    DM1_V1_D3L2D3R2F0108FloorCeilingResultPc34 result;

    CHECK_EQ("blend.transparent",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_blend_pixel_pc34(
                 0x44, 10, 10),
             0x44, "DEFS.H:2088 C10 preserves destination");
    CHECK_EQ("blend.opaque",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_blend_pixel_pc34(
                 0x44, 0x55, 10),
             0x55, "non-C10 source writes");
    CHECK_EQ("compose.null.out",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(
                 NULL, NULL),
             0, "null output guard");
    CHECK_EQ("compose.null.state",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(
                 NULL, &result),
             0, "null state guard");

    CHECK_EQ("fallback.initial",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
                 DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34, &state),
             1, "fallback initial state");
    state.enable_f0108_floor_ornament = false;
    state.f0098_floor_pixel = 10;
    state.f0098_ceiling_pixel = 0x62;
    CHECK_EQ("fallback.compose",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             1, "F0098 fallback without F0108");
    CHECK_EQ("fallback.f0108_count", result.f0108OrnamentCount, 0,
             "F0108 not engaged");
    CHECK_EQ("fallback.f0098_floor", result.f0098FloorCount, 1,
             "F0098 floor fallback engaged");
    CHECK_EQ("fallback.f0098_ceiling", result.f0098CeilingCount, 1,
             "F0098 ceiling fallback engaged");
    CHECK_EQ("fallback.c10_count", result.c10TransparentBlitCount, 2,
             "C10 floor plus F0115 no-op");
    CHECK_EQ("fallback.after_floor", result.after_f0098_floor, 0x11,
             "C10 fallback floor preserves destination");
    CHECK_EQ("fallback.after_ornament", result.after_f0108_ornament, 0x11,
             "no F0108 layer changes destination");
    CHECK_EQ("fallback.after_ceiling", result.after_f0098_ceiling, 0x62,
             "F0098 ceiling fallback writes");
    CHECK_EQ("fallback.after_f0115", result.after_f0115_noop, 0x62,
             "F0115 remains no-op");

    CHECK_EQ("transparent.initial",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
                 DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3R2_PC34, &state),
             1, "transparent initial state");
    state.f0098_floor_pixel = 10;
    state.f0108_ornament_pixel = 10;
    state.f0098_ceiling_pixel = 10;
    CHECK_EQ("transparent.compose",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             1, "all visible layers transparent");
    CHECK_EQ("transparent.c10_count", result.c10TransparentBlitCount, 4,
             "F0098 floor/F0108/F0098 ceiling/F0115 C10");
    CHECK_EQ("transparent.final", result.after_f0115_noop, 0x11,
             "all C10 layers preserve destination");
    CHECK_EQ("transparent.f0108_count", result.f0108OrnamentCount, 2,
             "footprint ordinal counts primary plus recursion");
    CHECK_EQ("transparent.flip", result.f0099FlipCount, 1,
             "D3R2 F0099 flip preserved");
}

static void test_row_local_flip(void)
{
    uint8_t source[8] = { 1, 2, 3, 4, 11, 12, 13, 14 };
    uint8_t dest[8] = { 0 };
    uint8_t guard_before = 0xA5;
    uint8_t guard_after = 0x5A;

    CHECK_EQ("flip.null.source",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_flip_row_pc34(
                 NULL, dest, 4, 2),
             0, "F0099 source guard");
    CHECK_EQ("flip.null.dest",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_flip_row_pc34(
                 source, NULL, 4, 2),
             0, "F0099 destination guard");
    CHECK_EQ("flip.zero_width",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_flip_row_pc34(
                 source, dest, 0, 2),
             0, "F0099 width guard");
    CHECK_EQ("flip.zero_rows",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_flip_row_pc34(
                 source, dest, 4, 0),
             0, "F0099 row-count guard");
    CHECK_EQ("flip.run",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_flip_row_pc34(
                 source, dest, 4, 2),
             1, "F0099 row-local flip");
    CHECK_EQ("flip.r0c0", dest[0], 4, "row 0 col 0 from row 0 col 3");
    CHECK_EQ("flip.r0c1", dest[1], 3, "row 0 col 1 from row 0 col 2");
    CHECK_EQ("flip.r0c2", dest[2], 2, "row 0 col 2 from row 0 col 1");
    CHECK_EQ("flip.r0c3", dest[3], 1, "row 0 col 3 from row 0 col 0");
    CHECK_EQ("flip.r1c0", dest[4], 14, "row 1 col 0 from row 1 col 3");
    CHECK_EQ("flip.r1c1", dest[5], 13, "row 1 col 1 from row 1 col 2");
    CHECK_EQ("flip.r1c2", dest[6], 12, "row 1 col 2 from row 1 col 1");
    CHECK_EQ("flip.r1c3", dest[7], 11, "row 1 col 3 from row 1 col 0");
    CHECK_EQ("flip.guard_before", guard_before, 0xA5, "external guard unchanged");
    CHECK_EQ("flip.guard_after", guard_after, 0x5A, "external guard unchanged");
}

static void test_rejects_non_contract_state(void)
{
    DM1_V1_D3L2D3R2F0108FloorCeilingStatePc34 state;
    DM1_V1_D3L2D3R2F0108FloorCeilingResultPc34 result;

    CHECK_EQ("reject.initial",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_initial_state_pc34(
                 DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SIDE_D3L2_PC34, &state),
             1, "reject initial state");

    state.square_element = DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_WALL_PC34;
    CHECK_EQ("reject.wall",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "F0107 wall route is outside this F0108 contract");
    CHECK_EQ("reject.wall.flag", result.rejectsNonContractState, 1,
             "wall rejection flag");

    state.square_element = DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_DOOR_FRONT_PC34;
    CHECK_EQ("reject.door_front",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "F0111 door-front route is outside this contract");
    CHECK_EQ("reject.door_front.flag", result.rejectsNonContractState, 1,
             "door-front rejection flag");

    state.square_element = DM1_V1_D3L2_D3R2_F0108_FLOOR_CEILING_SQUARE_CORRIDOR_PC34;
    state.attempts_f0107_wall_ornament = true;
    CHECK_EQ("reject.f0107_attempt",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "F0107 overlap rejected");
    CHECK_EQ("reject.f0107.flag", result.rejectsNonContractState, 1,
             "F0107 rejection flag");

    state.attempts_f0107_wall_ornament = false;
    state.attempts_f0111_door = true;
    CHECK_EQ("reject.f0111_attempt",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "F0111 overlap rejected");
    CHECK_EQ("reject.f0111.flag", result.rejectsNonContractState, 1,
             "F0111 rejection flag");

    state.attempts_f0111_door = false;
    state.post_d3c_reached = false;
    CHECK_EQ("reject.no_post_d3c",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "F0128 post-D3C guard");
    CHECK_EQ("reject.no_post_d3c.flag", result.rejectsNonContractState, 1,
             "post-D3C rejection flag");

    state.post_d3c_reached = true;
    state.floor_ornament_coordinate_set = -1;
    CHECK_EQ("reject.coordinate",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "negative coordinate set rejected");
    CHECK_EQ("reject.coordinate.flag", result.rejectsNonContractState, 1,
             "coordinate rejection flag");

    state.floor_ornament_coordinate_set = 0;
    state.floor_ornament_native_bitmap_index = -1;
    CHECK_EQ("reject.native",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "negative native bitmap rejected");
    CHECK_EQ("reject.native.flag", result.rejectsNonContractState, 1,
             "native rejection flag");

    state.floor_ornament_native_bitmap_index = 1;
    state.mutation_guard_before = 0;
    CHECK_EQ("reject.guard_before",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "mutation guard before rejected");
    CHECK_EQ("reject.guard_before.flag", result.rejectsNonContractState, 1,
             "guard-before rejection flag");

    state.mutation_guard_before = 0x7080u;
    state.mutation_guard_after = 0;
    CHECK_EQ("reject.guard_after",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "mutation guard after rejected");
    CHECK_EQ("reject.guard_after.flag", result.rejectsNonContractState, 1,
             "guard-after rejection flag");

    state.mutation_guard_after = 0x8070u;
    state.side = (DM1_V1_D3L2D3R2F0108FloorCeilingSidePc34)8;
    CHECK_EQ("reject.side",
             dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_compose_pc34(&state, &result),
             0, "unknown side rejected");
    CHECK_EQ("reject.side.flag", result.rejectsNonContractState, 1,
             "unknown-side rejection flag");
}

int main(void)
{
    test_source_evidence();
    test_specs();
    test_initial_state_and_ordinal();
    test_full_composition();
    test_c10_transparency_and_f0098_fallback();
    test_row_local_flip();
    test_rejects_non_contract_state();

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
