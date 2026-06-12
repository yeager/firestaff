#include "csb_v1_viewport_d2c_f0111_door_front_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
}

static const char *A_DUNVIEW =
    "ReDMCSB DUNVIEW.C:7244-7389 F0121, 4218-4339 F0111, 4547-4581 F0115";
static const char *A_DUNGEON =
    "ReDMCSB DUNGEON.C:1769-1838 F0163, 1840-1905 F0164, 2466-2523 F0172";
static const char *A_DRAWVIEW =
    "ReDMCSB DRAWVIEW.C:709-722 F0097_DUNGEONVIEW_DrawViewport";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2533-2559,2602,2657-2677,2756,2790,4049,4256";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1865-1879,1903-1915,1192-1209";

static int test_identity(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *s =
        csb_v1_viewport_d2c_f0111_door_front_spec_pc34();

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d2c_f0111_door_front_spec_count_pc34(),
                     1, A_DUNVIEW);
    ok &= expect_int("spec.non_null", s != NULL, 1, A_DUNVIEW);
    ok &= expect_int("spec.at0",
                     csb_v1_viewport_d2c_f0111_door_front_spec_at_pc34(0) == s,
                     1, A_DUNVIEW);
    ok &= expect_int("spec.at1.null",
                     csb_v1_viewport_d2c_f0111_door_front_spec_at_pc34(1) == NULL,
                     1, A_DUNVIEW);
    ok &= expect_contains("spec.identifier", s ? s->identifier : NULL,
                          "CSB_V1_D2C_F0111", A_DUNVIEW);
    ok &= expect_int("spec.contract.only",
                     s ? s->source_locked_contract_only : 0, 1, A_DUNVIEW);
    ok &= expect_int("spec.no.real.assets",
                     s ? s->no_real_asset_bitmap_parity : 0, 1, A_DUNVIEW);
    ok &= expect_int("spec.no.game.data",
                     s ? s->no_game_data_load : 0, 1, A_DUNVIEW);
    ok &= expect_int("spec.single.lane.d2c", s ? s->single_lane_d2c : 0,
                     1, A_DUNVIEW);
    ok &= expect_int("spec.distinct.pass665", s ? s->distinct_from_pass665_d0c : 0,
                     1, "pass665 D0C is not this D2C lane");
    ok &= expect_int("spec.distinct.pass703",
                     s ? s->distinct_from_pass703_d0l2_d0r2 : 0, 1,
                     "pass703 D0L2/D0R2 is not this D2C lane");
    ok &= expect_int("spec.distinct.partly",
                     s ? s->distinct_from_d2c_partly_open_gate : 0, 1,
                     A_DUNVIEW);
    ok &= expect_int("spec.distinct.wall.ornament",
                     s ? s->distinct_from_d2c_wall_ornament_gate : 0, 1,
                     A_DUNVIEW);

    return ok;
}

static int test_route_spec(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *s =
        csb_v1_viewport_d2c_f0111_door_front_spec_pc34();

    ok &= expect_int("view.square.d2c", s ? s->view_square_index : -1, 6, A_DEFS);
    ok &= expect_int("relative.depth", s ? s->relative_depth : -1, 2, A_DUNVIEW);
    ok &= expect_int("relative.lateral", s ? s->relative_lateral : -9, 0,
                     A_DUNVIEW);
    ok &= expect_int("f0128.dispatch.line", s ? s->f0128_dispatch_line : -1,
                     8521, A_DUNVIEW);
    ok &= expect_int("f0121.function", s ? s->f0121_function_id : -1, 121,
                     A_DUNVIEW);
    ok &= expect_int("element.door.front", s ? s->element_door_front : -1, 17,
                     A_DUNVIEW);
    ok &= expect_int("floor.view.d2c", s ? s->floor_view : -1, 6, A_DEFS);
    ok &= expect_int("rear.order", s ? (int)s->f0115_rear_cell_order : -1,
                     0x0218, A_DEFS);
    ok &= expect_int("front.order", s ? (int)s->f0115_front_cell_order : -1,
                     0x0349, A_DEFS);
    ok &= expect_int("bitmap.d2lcr", s ? s->f0111_front_bitmap_id : -1, 694,
                     A_DUNVIEW);
    ok &= expect_int("ornament.d2lcr", s ? s->f0111_door_ornament_view : -1,
                     1, A_DEFS);
    ok &= expect_int("wall.zone.d2c", s ? s->wall_zone_d2c : -1, 709, A_DEFS);
    ok &= expect_int("door.zone.d2c", s ? s->door_zone_d2c : -1, 3760, A_DEFS);
    ok &= expect_int("door.width", s ? s->door_width : -1, 64, A_DUNVIEW);
    ok &= expect_int("door.height", s ? s->door_height : -1, 61, A_DUNVIEW);
    ok &= expect_int("transparent.color", s ? s->transparent_color : -1, 10,
                     A_DEFS);
    ok &= expect_int("transparent.macro",
                     CSB_V1_D2C_F0111_DOOR_FRONT_TRANSPARENT_COLOR_PC34, 10,
                     A_DEFS);

    return ok;
}

static int test_order_contract(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *s =
        csb_v1_viewport_d2c_f0111_door_front_spec_pc34();

    ok &= expect_int("order.f0108.before.rear",
                     s ? s->f0108_before_rear_f0115 : 0, 1, A_DUNVIEW);
    ok &= expect_int("order.rear.before.f0111",
                     s ? s->rear_f0115_before_f0111 : 0, 1, A_DUNVIEW);
    ok &= expect_int("order.f0111.before.front",
                     s ? s->f0111_before_front_f0115 : 0, 1, A_DUNVIEW);
    ok &= expect_int("marker.rear", s ? s->f0115_door_marker_nibble : -1, 8,
                     A_DUNVIEW);
    ok &= expect_int("marker.front", s ? s->f0115_front_marker_nibble : -1, 9,
                     A_DUNVIEW);
    ok &= expect_int("f0111.open.skips",
                     s ? s->f0111_open_state_skips_blit : 0, 1, A_DUNVIEW);
    ok &= expect_int("f0111.closed.draws",
                     s ? s->f0111_closed_state_draws_bitmap : 0, 1, A_DUNVIEW);
    ok &= expect_int("f0111.destroyed.mask",
                     s ? s->f0111_destroyed_state_applies_mask : 0, 1,
                     A_DUNVIEW);
    ok &= expect_int("f0111.partly.decrements",
                     s ? s->f0111_partly_open_state_decrements : 0, 1,
                     A_DUNVIEW);
    ok &= expect_int("f0172.source",
                     s ? s->f0172_square_aspect_source : 0, 1, A_DUNGEON);
    ok &= expect_int("f0163.not.called",
                     s ? s->f0163_not_called_by_draw : 0, 1, A_DUNGEON);
    ok &= expect_int("f0164.not.called",
                     s ? s->f0164_not_called_by_draw : 0, 1, A_DUNGEON);
    ok &= expect_int("drawview.handoff",
                     s ? s->drawview_f0097_viewport_handoff : 0, 1, A_DRAWVIEW);
    ok &= expect_int("lineage.f2", s ? s->csb_lineage_f2_door_facing_reference : 0,
                     1, A_LINEAGE);
    ok &= expect_int("lineage.f1", s ? s->csb_lineage_f1_door_facing_reference : 0,
                     1, A_LINEAGE);
    ok &= expect_int("lineage.open",
                     s ? s->csb_lineage_open_room_reference : 0, 1, A_LINEAGE);

    return ok;
}

static int test_cell_order_decode(void)
{
    int ok = 1;

    ok &= expect_int("decode.rear.marker",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0218u, 0),
                     -1, A_DEFS);
    ok &= expect_int("decode.rear.backleft",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0218u, 1),
                     0, A_DEFS);
    ok &= expect_int("decode.rear.backright",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0218u, 2),
                     1, A_DEFS);
    ok &= expect_int("decode.rear.terminator",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0218u, 3),
                     -1, A_DEFS);
    ok &= expect_int("decode.front.marker",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0349u, 0),
                     -1, A_DEFS);
    ok &= expect_int("decode.front.frontleft",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0349u, 1),
                     3, A_DEFS);
    ok &= expect_int("decode.front.frontright",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0349u, 2),
                     2, A_DEFS);
    ok &= expect_int("decode.front.terminator",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0349u, 3),
                     -1, A_DEFS);
    ok &= expect_int("decode.bad.ordinal.high",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0349u, 4),
                     -1, A_DEFS);
    ok &= expect_int("decode.bad.ordinal.negative",
                     csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
                         0x0349u, -1),
                     -1, A_DEFS);

    return ok;
}

static int test_composition(void)
{
    int ok = 1;
    CSB_V1_D2CF0111DoorFrontTracePc34 trace;
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *s =
        csb_v1_viewport_d2c_f0111_door_front_spec_pc34();

    ok &= expect_int("blend.transparent",
                     csb_v1_viewport_d2c_f0111_door_front_blend_pc34(77, 10),
                     77, A_DEFS);
    ok &= expect_int("blend.opaque",
                     csb_v1_viewport_d2c_f0111_door_front_blend_pc34(77, 42),
                     42, A_DEFS);
    ok &= expect_int("compose.result",
                     csb_v1_viewport_d2c_f0111_door_front_compose_pixel_pc34(
                         s, 0x10, 0x20, 0x30, 0x40, 0x50, &trace),
                     0, A_DUNVIEW);
    ok &= expect_int("compose.ok", trace.ok, 1, A_DUNVIEW);
    ok &= expect_int("compose.f0108.calls", trace.f0108_calls, 1, A_DUNVIEW);
    ok &= expect_int("compose.f0115.calls", trace.f0115_calls, 2, A_DUNVIEW);
    ok &= expect_int("compose.f0111.calls", trace.f0111_calls, 1, A_DUNVIEW);
    ok &= expect_int("compose.after.floor", trace.after_floor, 0x20, A_DUNVIEW);
    ok &= expect_int("compose.after.rear", trace.after_rear_pass, 0x30,
                     A_DUNVIEW);
    ok &= expect_int("compose.after.door", trace.after_door, 0x40, A_DUNVIEW);
    ok &= expect_int("compose.after.front", trace.after_front_pass, 0x50,
                     A_DUNVIEW);

    ok &= expect_int("compose.c10.result",
                     csb_v1_viewport_d2c_f0111_door_front_compose_pixel_pc34(
                         s, 0x66, 10, 10, 10, 0x55, &trace),
                     0, A_DEFS);
    ok &= expect_int("compose.c10.floor.flag", trace.floor_transparent, 1,
                     A_DEFS);
    ok &= expect_int("compose.c10.rear.flag", trace.rear_transparent, 1, A_DEFS);
    ok &= expect_int("compose.c10.door.flag", trace.door_transparent, 1, A_DEFS);
    ok &= expect_int("compose.c10.front.flag", trace.front_transparent, 0,
                     A_DEFS);
    ok &= expect_int("compose.c10.after.floor", trace.after_floor, 0x66,
                     A_DEFS);
    ok &= expect_int("compose.c10.after.rear", trace.after_rear_pass, 0x66,
                     A_DEFS);
    ok &= expect_int("compose.c10.after.door", trace.after_door, 0x66,
                     A_DEFS);
    ok &= expect_int("compose.c10.after.front", trace.after_front_pass, 0x55,
                     A_DEFS);
    ok &= expect_int("compose.null.spec",
                     csb_v1_viewport_d2c_f0111_door_front_compose_pixel_pc34(
                         NULL, 0, 0, 0, 0, 0, &trace),
                     -1, A_DUNVIEW);
    ok &= expect_int("compose.null.trace",
                     csb_v1_viewport_d2c_f0111_door_front_compose_pixel_pc34(
                         s, 0, 0, 0, 0, 0, NULL),
                     -1, A_DUNVIEW);

    return ok;
}

static int test_mutation_and_evidence(void)
{
    int ok = 1;
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *s =
        csb_v1_viewport_d2c_f0111_door_front_spec_pc34();
    const char *e = csb_v1_viewport_d2c_f0111_door_front_source_evidence_pc34();

    ok &= expect_int("mutation.false",
                     csb_v1_viewport_d2c_f0111_door_front_is_draw_mutating_pc34(s),
                     0, A_DUNGEON);
    ok &= expect_int("mutation.null",
                     csb_v1_viewport_d2c_f0111_door_front_is_draw_mutating_pc34(NULL),
                     -1, A_DUNGEON);
    ok &= expect_contains("anchor.dunview", s ? s->redmcsb_dunview_anchor : NULL,
                          "F0121", A_DUNVIEW);
    ok &= expect_contains("anchor.dungeon", s ? s->redmcsb_dungeon_anchor : NULL,
                          "F0172", A_DUNGEON);
    ok &= expect_contains("anchor.drawview", s ? s->redmcsb_drawview_anchor : NULL,
                          "F0097", A_DRAWVIEW);
    ok &= expect_contains("anchor.defs", s ? s->redmcsb_defs_anchor : NULL,
                          "DEFS.H", A_DEFS);
    ok &= expect_contains("anchor.lineage", s ? s->csb_lineage_anchor : NULL,
                          "Viewport.cpp", A_LINEAGE);
    ok &= expect_contains("evidence.pass", e, "pass705", A_DUNVIEW);
    ok &= expect_contains("evidence.d2c", e, "D2C F0111 door-front", A_DUNVIEW);
    ok &= expect_contains("evidence.f0108", e, "line 7314 calls F0108",
                          A_DUNVIEW);
    ok &= expect_contains("evidence.rear", e, "C0x0218", A_DEFS);
    ok &= expect_contains("evidence.f0111", e, "F0111_DUNGEONVIEW_DrawDoor",
                          A_DUNVIEW);
    ok &= expect_contains("evidence.front", e, "C0x0349", A_DEFS);
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH", A_DEFS);
    ok &= expect_contains("evidence.f0128", e, "line 8521", A_DUNVIEW);
    ok &= expect_contains("evidence.dungeon", e, "DUNGEON.C:1769-1838",
                          A_DUNGEON);
    ok &= expect_contains("evidence.drawview", e, "DRAWVIEW.C:709-722",
                          A_DRAWVIEW);
    ok &= expect_contains("evidence.lineage.f2", e, "Viewport.cpp:1865-1879",
                          A_LINEAGE);
    ok &= expect_contains("evidence.lineage.f1", e, "Viewport.cpp:1903-1915",
                          A_LINEAGE);
    ok &= expect_contains("evidence.nodup.pass665", e, "pass665 D0C",
                          "non-duplication evidence");
    ok &= expect_contains("evidence.nodup.pass703", e, "pass703 D0L2/D0R2",
                          "non-duplication evidence");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2c_f0111_door_front_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2c_f0111_door_front_source_evidence_pc34());

    ok &= test_identity();
    ok &= test_route_spec();
    ok &= test_order_contract();
    ok &= test_cell_order_decode();
    ok &= test_composition();
    ok &= test_mutation_and_evidence();

    printf("assertions=%d\n", g_assertions);
    printf("failures=%d\n", g_failures);
    if (g_assertions < 30) {
        printf("FAIL assertion floor got=%d want>=30\n", g_assertions);
        return 1;
    }
    if (!ok) return 1;
    return g_failures == 0 ? 0 : 1;
}
