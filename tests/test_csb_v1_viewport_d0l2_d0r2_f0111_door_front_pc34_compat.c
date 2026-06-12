#include "csb_v1_viewport_d0l2_d0r2_f0111_door_front_pc34_compat.h"

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
        } else {                                                             \
            printf("PASS %s == %d anchor=%s\n",                             \
                   (ID), want_value__, (ANCHOR));                           \
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
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void check_spec_one(
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *s,
    int side,
    int lateral,
    int draw_index,
    int view_square,
    int wall_zone,
    int door_zone,
    int floor_view,
    unsigned int rear_order,
    unsigned int front_order)
{
    CHECK_EQ("spec.present", s != NULL, 1,
             "ReDMCSB DUNVIEW.C:6442-6460/6578-6602 door-front callers");
    if (!s) return;

    CHECK_EQ("spec.side", s->side, side, "D0L2/D0R2 fixture identity");
    CHECK_EQ("spec.contract_only", s->source_locked_contract_only, 1,
             "contract-only source lock");
    CHECK_EQ("spec.no_asset_parity", s->no_real_asset_bitmap_parity, 1,
             "asset-free synthetic pixels");
    CHECK_EQ("spec.no_game_data", s->no_game_data_load, 1,
             "no GRAPHICS.DAT/DUNGEON.DAT load");
    CHECK_EQ("spec.distinct_wall", s->distinct_from_d0l2_d0r2_wall_gate, 1,
             "distinct from pass696-style wall surface");
    CHECK_EQ("spec.distinct_f0115", s->distinct_from_f0115_thing_pass_gate, 1,
             "distinct from D0L2/D0R2 F0115 thing-pass gate");
    CHECK_EQ("spec.depth", s->relative_depth, 3,
             "ReDMCSB DUNVIEW.C:8490-8495 relative depth");
    CHECK_EQ("spec.lateral", s->relative_lateral, lateral,
             "ReDMCSB DUNVIEW.C:8490-8495 lateral pair");
    CHECK_EQ("spec.draw_index", s->f0128_draw_index, draw_index,
             "ReDMCSB DUNVIEW.C:8490-8495 D3L before D3R");
    CHECK_EQ("spec.view_square", s->view_square_index, view_square,
             "ReDMCSB DEFS.H:2608-2609 M601/M602");
    CHECK_EQ("spec.wall_zone", s->wall_zone, wall_zone,
             "ReDMCSB DEFS.H:4045-4046 C705/C706");
    CHECK_EQ("spec.door_zone", s->door_zone, door_zone,
             "ReDMCSB DEFS.H:4252-4254 M624/M626");
    CHECK_EQ("spec.floor_view", s->f0108_floor_view, floor_view,
             "ReDMCSB DUNVIEW.C:6443/6579 F0108 floor view");
    CHECK_EQ("spec.rear_order", (int)s->f0115_rear_cell_order, (int)rear_order,
             "ReDMCSB DUNVIEW.C:6444/6580 rear F0115 pass");
    CHECK_EQ("spec.front_order", (int)s->f0115_front_cell_order, (int)front_order,
             "ReDMCSB DUNVIEW.C:6459/6601 front F0115 pass");
    CHECK_EQ("spec.bitmap", s->f0111_front_bitmap_id, 693,
             "ReDMCSB DUNVIEW.C:6457/6599 G0693 front bitmap");
    CHECK_EQ("spec.ornament", s->f0111_door_ornament_view, 0,
             "ReDMCSB DEFS.H:2789 C0_VIEW_DOOR_ORNAMENT_D3LCR");
    CHECK_EQ("spec.open_skips", s->f0111_open_state_skips_blit, 1,
             "ReDMCSB DUNVIEW.C:4248-4253");
    CHECK_EQ("spec.closed_draws", s->f0111_closed_state_draws_closed_or_destroyed, 1,
             "ReDMCSB DUNVIEW.C:4297-4299");
    CHECK_EQ("spec.destroyed_mask", s->f0111_destroyed_state_applies_destroyed_mask, 1,
             "ReDMCSB DUNVIEW.C:4301-4305");
    CHECK_EQ("spec.partly_open", s->f0111_partly_open_state_decrements_state, 1,
             "ReDMCSB DUNVIEW.C:4307-4318");
    CHECK_EQ("spec.final_c10", s->f0111_final_blit_uses_c10, 1,
             "ReDMCSB DUNVIEW.C:4334");
    CHECK_EQ("spec.transparent_color", s->transparent_color, 10,
             "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    CHECK_EQ("spec.rear_before_door", s->rear_pass_before_door, 1,
             "ReDMCSB DUNVIEW.C:6444 before 6457; 6580 before 6599");
    CHECK_EQ("spec.door_before_front", s->door_before_front_pass, 1,
             "ReDMCSB DUNVIEW.C:6457 before 6459; 6599 before 6601");
    CHECK_EQ("spec.f0172_source", s->f0172_square_aspect_source, 1,
             "ReDMCSB DUNGEON.C:2466-2523 F0172");
    CHECK_EQ("spec.f0163_not_called", s->f0163_not_called_by_draw, 1,
             "ReDMCSB DUNGEON.C:1769-1838 F0163");
    CHECK_EQ("spec.f0164_not_called", s->f0164_not_called_by_draw, 1,
             "ReDMCSB DUNGEON.C:1840-1905 F0164");
    CHECK_EQ("spec.lineage_f1", s->csb_lineage_f1_two_pass_reference, 1,
             "CSB-lineage Viewport.cpp:1903-1915");
    CHECK_EQ("spec.lineage_f0_return", s->csb_lineage_f0l1_f0r1_return_only_reference, 1,
             "CSB-lineage Viewport.cpp:1930-1944");
    CHECK_EQ("spec.lineage_open", s->csb_lineage_open_room_objects_reference, 1,
             "CSB-lineage Viewport.cpp:1192-1209");
    CHECK_EQ("spec.lineage_rear_opcode", s->lineage_rear_draw_order_opcode, 60279,
             "CSB-lineage Viewport.cpp:681/1906 DrawOrder218");
    CHECK_EQ("spec.lineage_front_opcode", s->lineage_front_draw_order_opcode, 60280,
             "CSB-lineage Viewport.cpp:682/1915 DrawOrder349");
    CHECK_EQ("spec.lineage_objects", s->lineage_room_objects_opcode, 60006,
             "CSB-lineage Viewport.cpp:379 StdDrawRoomObjects");
    check_contains("spec.f0111_anchor", s->redmcsb_f0111_anchor, "F0111",
                   "fixture carries F0111 anchor");
    check_contains("spec.dungeon_anchor", s->redmcsb_dungeon_anchor, "F0172",
                   "fixture carries F0172 anchor");
    check_contains("spec.defs_anchor", s->redmcsb_defs_anchor, "DEFS.H",
                   "fixture carries DEFS anchor");
    check_contains("spec.lineage_anchor", s->csb_lineage_anchor, "Viewport.cpp",
                   "fixture carries lineage anchor");
}

static void test_specs(void)
{
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *right =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(2);

    CHECK_EQ("count",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_count_pc34(),
             2, "two-lane D0L2/D0R2 compatibility surface");
    CHECK_EQ("at0.left",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_at_pc34(0) == left,
             1, "left fixture first");
    CHECK_EQ("at1.right",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_at_pc34(1) == right,
             1, "right fixture second");
    CHECK_EQ("at2.null",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_at_pc34(2) == NULL,
             1, "fixture bounds");
    CHECK_EQ("bad_side.null",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(3) == NULL,
             1, "fixture side guard");

    check_spec_one(left, 1, -1, 0, 12, 705, 3720, 2, 0x0218u, 0x0349u);
    check_spec_one(right, 2, 1, 1, 13, 706, 3740, 4, 0x0128u, 0x0439u);
}

static void test_cell_order_decode(void)
{
    CHECK_EQ("decode.left.rear.0",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0218u, 0),
             -1, "DEFS.H:2669 marker 8 is not a cell");
    CHECK_EQ("decode.left.rear.1",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0218u, 1),
             0, "DEFS.H:2669 back-left");
    CHECK_EQ("decode.left.rear.2",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0218u, 2),
             1, "DEFS.H:2669 back-right");
    CHECK_EQ("decode.right.rear.1",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0128u, 1),
             1, "DEFS.H:2668 back-right");
    CHECK_EQ("decode.right.rear.2",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0128u, 2),
             0, "DEFS.H:2668 back-left");
    CHECK_EQ("decode.left.front.0",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0349u, 0),
             -1, "DEFS.H:2672 marker 9 is not a cell");
    CHECK_EQ("decode.left.front.1",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0349u, 1),
             3, "DEFS.H:2672 front-left");
    CHECK_EQ("decode.left.front.2",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0349u, 2),
             2, "DEFS.H:2672 front-right");
    CHECK_EQ("decode.right.front.1",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0439u, 1),
             2, "DEFS.H:2675 front-right");
    CHECK_EQ("decode.right.front.2",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0439u, 2),
             3, "DEFS.H:2675 front-left");
    CHECK_EQ("decode.bad.ordinal",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0439u, 4),
             -1, "decode guard");
    CHECK_EQ("decode.negative.ordinal",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(0x0439u, -1),
             -1, "decode guard");
}

static void test_composition_and_mutation_guards(void)
{
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(1);
    CSB_V1_D0L2D0R2F0111DoorFrontTracePc34 trace;

    CHECK_EQ("blend.transparent",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(77, 10),
             77, "ReDMCSB DUNVIEW.C:4334 C10 transparent");
    CHECK_EQ("blend.opaque",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(77, 42),
             42, "ReDMCSB DUNVIEW.C:4334 C10 transparent");

    CHECK_EQ("compose.ok",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_compose_pixel_pc34(
                 left, 0x10, 0x20, 0x30, 0x40, 0x50, &trace),
             0, "F0108 -> F0115 rear -> F0111 -> F0115 front");
    CHECK_EQ("compose.trace_ok", trace.ok, 1,
             "synthetic composition route");
    CHECK_EQ("compose.f0108_calls", trace.f0108_calls, 1,
             "ReDMCSB DUNVIEW.C:6443/6579");
    CHECK_EQ("compose.f0115_calls", trace.f0115_calls, 2,
             "ReDMCSB DUNVIEW.C:6444/6459 and 6580/6601");
    CHECK_EQ("compose.f0111_calls", trace.f0111_calls, 1,
             "ReDMCSB DUNVIEW.C:6457/6599");
    CHECK_EQ("compose.after_floor", trace.after_floor, 0x20,
             "floor writes before rear pass");
    CHECK_EQ("compose.after_rear", trace.after_rear_pass, 0x30,
             "rear pass writes before door");
    CHECK_EQ("compose.after_door", trace.after_door, 0x40,
             "door writes before front pass");
    CHECK_EQ("compose.after_front", trace.after_front_pass, 0x50,
             "front pass writes last");

    CHECK_EQ("compose.c10.ok",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_compose_pixel_pc34(
                 left, 0x66, 10, 10, 10, 0x55, &trace),
             0, "C10 transparency propagation");
    CHECK_EQ("compose.c10.floor_flag", trace.floor_transparent, 1,
             "F0108 C10 transparent");
    CHECK_EQ("compose.c10.rear_flag", trace.rear_transparent, 1,
             "F0115 rear C10 transparent");
    CHECK_EQ("compose.c10.door_flag", trace.door_transparent, 1,
             "F0111 C10 transparent");
    CHECK_EQ("compose.c10.front_flag", trace.front_transparent, 0,
             "front pass opaque");
    CHECK_EQ("compose.c10.after_floor", trace.after_floor, 0x66,
             "floor C10 preserves base");
    CHECK_EQ("compose.c10.after_rear", trace.after_rear_pass, 0x66,
             "rear C10 preserves base");
    CHECK_EQ("compose.c10.after_door", trace.after_door, 0x66,
             "door C10 preserves base");
    CHECK_EQ("compose.c10.after_front", trace.after_front_pass, 0x55,
             "front pass writes over transparent door");

    CHECK_EQ("compose.null_spec",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_compose_pixel_pc34(
                 NULL, 0, 0, 0, 0, 0, &trace),
             -1, "invalid input guard");
    CHECK_EQ("compose.null_trace",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_compose_pixel_pc34(
                 left, 0, 0, 0, 0, 0, NULL),
             -1, "invalid input guard");
    CHECK_EQ("mutation.false",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_is_draw_mutating_pc34(left),
             0, "DUNGEON.C F0163/F0164 not called by F0111 draw");
    CHECK_EQ("mutation.null",
             csb_v1_viewport_d0l2_d0r2_f0111_door_front_is_draw_mutating_pc34(NULL),
             -1, "invalid input guard");
}

static void test_source_evidence(void)
{
    const char *e =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_source_evidence_pc34();

    check_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337",
                   "ReDMCSB F0111 anchor");
    check_contains("evidence.compat_surface", e, "D0L2/D0R2 F0111 door-front",
                   "CSB V1 compatibility surface label");
    check_contains("evidence.open", e, "4248-4253",
                   "ReDMCSB F0111 open skip");
    check_contains("evidence.final_blit", e, "4334",
                   "ReDMCSB F0111 final blit");
    check_contains("evidence.left_caller", e, "6442-6460",
                   "ReDMCSB F0116 caller");
    check_contains("evidence.right_caller", e, "6578-6602",
                   "ReDMCSB F0117 caller");
    check_contains("evidence.f0172", e, "DUNGEON.C:2466-2523",
                   "ReDMCSB F0172 anchor");
    check_contains("evidence.f0163", e, "DUNGEON.C:1769-1838",
                   "ReDMCSB F0163 anchor");
    check_contains("evidence.f0164", e, "1840-1905",
                   "ReDMCSB F0164 anchor");
    check_contains("evidence.defs_zones", e, "DEFS.H:4045-4046",
                   "ReDMCSB C705/C706 anchor");
    check_contains("evidence.lineage_f1", e, "Viewport.cpp:1903-1915",
                   "CSB-lineage F1 two-pass anchor");
    check_contains("evidence.lineage_f0", e, "Viewport.cpp:1930-1944",
                   "CSB-lineage F0 return-only anchor");
    check_contains("evidence.lineage_open", e, "Viewport.cpp:1192-1209",
                   "CSB-lineage open contrast anchor");
}

int main(void)
{
    printf("probe=csb_v1_viewport_d0l2_d0r2_f0111_door_front_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d0l2_d0r2_f0111_door_front_source_evidence_pc34());

    test_specs();
    test_cell_order_decode();
    test_composition_and_mutation_guards();
    test_source_evidence();

    printf("assertions=%d\n", g_assertions);
    if (g_assertions < 30) {
        printf("FAIL assertion floor got=%d want>=30\n", g_assertions);
        return 1;
    }
    return g_failures == 0 ? 0 : 1;
}
