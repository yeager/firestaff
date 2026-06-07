#include "csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * CSB V1 D1L2/D1R2 corridor thing-pass source-lock gate.
 * ReDMCSB: DUNVIEW.C:7391-7557 F0122 and 7559-7725 F0123 route corridor,
 * pit, and teleporter cases through F0115 at 7536/7704, while wall returns
 * through F0107 and door-front uses the separate F0111 path. F0115 anchors
 * are DUNVIEW.C:4547-4581, 4806-4811, 4923, 5075, 5201-5214, 5615-5617,
 * 5668-5683, 5916-5923, and 5998-5999. CSBWin and CSB-lineage
 * Viewport.cpp:1167-1188 confirm the F1L1/F1R1 Open StdDrawRoomObjects
 * bindings; CSB-lineage Viewport.cpp:6503-6551 keeps CustomBackgrounds out.
 */

static const char *A_D1L =
    "ReDMCSB DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L";
static const char *A_D1R =
    "ReDMCSB DUNVIEW.C:7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R";
static const char *A_DISPATCH =
    "ReDMCSB DUNVIEW.C:6773-6793 view-depth-2 F0122/F0123 dispatch";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 draw order";
static const char *A_F0115_ITEM =
    "ReDMCSB DUNVIEW.C:4806-4811,4923,5075 C2500 item binding";
static const char *A_F0115_CREATURE =
    "ReDMCSB DUNVIEW.C:5201-5214,5615-5617 C3200 creature binding";
static const char *A_F0115_PROJECTILE =
    "ReDMCSB DUNVIEW.C:5668-5683 C2900 projectile binding";
static const char *A_F0115_EXPLOSION =
    "ReDMCSB DUNVIEW.C:5916-5923,5998-5999 C3000 explosion binding";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2596-2601,4228-4230,4250-4260";
static const char *A_CSBWIN =
    "ReDMCSB DUNVIEW.C:7520-7536/7688-7704; CSBWin Viewport.cpp:1167-1188";
static const char *A_LINEAGE =
    "ReDMCSB DUNVIEW.C:7520-7536/7688-7704; CSB-lineage Viewport.cpp:1167-1188,6503-6551";

static int g_assertions = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_uint(const char *label, unsigned int got, unsigned int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04x want=0x%04x anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=0x%04x anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label,
                      haystack && needle && strstr(haystack, needle) != 0,
                      1, anchor);
}

static int test_identity_and_accessors(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *d1l2;
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *d1r2;

    ok &= expect_int("init", csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34(),
                     1, A_DISPATCH);
    d1l2 = csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    d1r2 = csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    ok &= expect_int("fixture.count",
                     (int)csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34(),
                     2, A_DEFS);
    ok &= expect_int("d1l2.present", d1l2 != 0, 1, A_D1L);
    ok &= expect_int("d1r2.present", d1r2 != 0, 1, A_D1R);
    ok &= expect_int("unknown.side.null",
                     csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(3) == 0,
                     1, A_DEFS);
    ok &= expect_int("index0.d1l2",
                     csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(0) == d1l2,
                     1, A_D1L);
    ok &= expect_int("index1.d1r2",
                     csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(1) == d1r2,
                     1, A_D1R);
    ok &= expect_int("past.end.null",
                     csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(2) == 0,
                     1, A_DEFS);
    ok &= expect_int("d1l2.side", d1l2 ? d1l2->side : -1, 1, A_D1L);
    ok &= expect_int("d1r2.side", d1r2 ? d1r2->side : -1, 2, A_D1R);

    return ok;
}

static int test_contract_markers(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixtures[2] = {
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1),
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2)
    };

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *f = fixtures[i];
        const char *anchor = i == 0 ? A_D1L : A_D1R;
        char label[64];

        snprintf(label, sizeof(label), "side%d.source_locked_contract_only", i + 1);
        ok &= expect_int(label, f ? f->source_locked_contract_only : 0, 1, anchor);
        snprintf(label, sizeof(label), "side%d.no_real_asset_bitmap_parity", i + 1);
        ok &= expect_int(label, f ? f->no_real_asset_bitmap_parity : 0, 1, anchor);
        snprintf(label, sizeof(label), "side%d.no_game_data_load", i + 1);
        ok &= expect_int(label, f ? f->no_game_data_load : 0, 1, anchor);
        snprintf(label, sizeof(label), "side%d.route_count", i + 1);
        ok &= expect_int(label, f ? f->route_count : -1, 1, anchor);
        snprintf(label, sizeof(label), "side%d.f0115_call_count", i + 1);
        ok &= expect_int(label, f ? f->f0115_call_count : -1, 1, anchor);
        snprintf(label, sizeof(label), "side%d.c10_transparency_flag", i + 1);
        ok &= expect_int(label, f ? f->c10_transparency_flag : 0, 1, A_DEFS);
        snprintf(label, sizeof(label), "side%d.transparent_color", i + 1);
        ok &= expect_int(label, f ? f->transparent_color : -1, 10, A_DEFS);
        snprintf(label, sizeof(label), "side%d.wall_route_absent", i + 1);
        ok &= expect_int(label, f ? f->wall_route : 1, 0, anchor);
        snprintf(label, sizeof(label), "side%d.no_wall_route", i + 1);
        ok &= expect_int(label, f ? f->no_wall_route : 0, 1, anchor);
        snprintf(label, sizeof(label), "side%d.no_f0107_contract", i + 1);
        ok &= expect_int(label, f ? f->no_f0107_contract : 0, 1, anchor);
        snprintf(label, sizeof(label), "side%d.no_f0111_contract", i + 1);
        ok &= expect_int(label, f ? f->no_f0111_contract : 0, 1, anchor);
        snprintf(label, sizeof(label), "side%d.no_custom_backgrounds", i + 1);
        ok &= expect_int(label, f ? f->no_custom_backgrounds_contract : 0, 1,
                         A_LINEAGE);
    }

    return ok;
}

static int test_view_square_and_order_metadata(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    ok &= expect_int("d1l2.view_square_index", d1l2 ? d1l2->view_square_index : -1,
                     4, A_DEFS);
    ok &= expect_int("d1r2.view_square_index", d1r2 ? d1r2->view_square_index : -1,
                     5, A_DEFS);
    ok &= expect_int("d1l2.depth", d1l2 ? d1l2->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:372 G2027[4]");
    ok &= expect_int("d1r2.depth", d1r2 ? d1r2->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:372 G2027[5]");
    ok &= expect_int("d1l2.lane", d1l2 ? d1l2->view_lane : 99, -1,
                     "ReDMCSB DUNVIEW.C:371 G2026[4]");
    ok &= expect_int("d1r2.lane", d1r2 ? d1r2->view_lane : 99, 1,
                     "ReDMCSB DUNVIEW.C:371 G2026[5]");
    ok &= expect_int("d1l2.object_row", d1l2 ? d1l2->object_g2028_row : -1,
                     9, A_F0115_ITEM);
    ok &= expect_int("d1r2.object_row", d1r2 ? d1r2->object_g2028_row : -1,
                     10, A_F0115_ITEM);
    ok &= expect_int("d1l2.creature_row", d1l2 ? d1l2->creature_g2033_row : -1,
                     9, A_F0115_CREATURE);
    ok &= expect_int("d1r2.creature_row", d1r2 ? d1r2->creature_g2033_row : -1,
                     10, A_F0115_CREATURE);
    ok &= expect_int("d1l2.explosion_row", d1l2 ? d1l2->explosion_g2034_row : -1,
                     12, A_F0115_EXPLOSION);
    ok &= expect_int("d1r2.explosion_row", d1r2 ? d1r2->explosion_g2034_row : -1,
                     13, A_F0115_EXPLOSION);
    ok &= expect_int("d1l2.field_aspect", d1l2 ? d1l2->field_aspect_index : -1,
                     11, A_F0115_EXPLOSION);
    ok &= expect_int("d1r2.field_aspect", d1r2 ? d1r2->field_aspect_index : -1,
                     12, A_F0115_EXPLOSION);
    ok &= expect_uint("d1l2.cell_order",
                      d1l2 ? d1l2->f0115_cell_order : 0, 0x0032u, A_D1L);
    ok &= expect_uint("d1r2.cell_order",
                      d1r2 ? d1r2->f0115_cell_order : 0, 0x0041u, A_D1R);

    return ok;
}

static int test_zone_binding_tags(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixtures[2] = {
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1),
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2)
    };

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *f = fixtures[i];
        char label[64];

        snprintf(label, sizeof(label), "side%d.zone_tag_count", i + 1);
        ok &= expect_int(label, f ? f->zone_binding_tag_count : -1, 4, A_F0115);
        snprintf(label, sizeof(label), "side%d.item_zone", i + 1);
        ok &= expect_int(label, f ? f->item_zone_base : -1, 2500, A_F0115_ITEM);
        snprintf(label, sizeof(label), "side%d.projectile_zone", i + 1);
        ok &= expect_int(label, f ? f->projectile_zone_base : -1, 2900,
                         A_F0115_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.creature_zone", i + 1);
        ok &= expect_int(label, f ? f->creature_zone_base : -1, 3200,
                         A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.explosion_zone", i + 1);
        ok &= expect_int(label, f ? f->explosion_zone_base : -1, 3000,
                         A_F0115_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.tag.item", i + 1);
        ok &= expect_contains(label, f ? f->zone_binding_tag : 0, "C2500 item",
                              A_F0115_ITEM);
        snprintf(label, sizeof(label), "side%d.tag.projectile", i + 1);
        ok &= expect_contains(label, f ? f->zone_binding_tag : 0, "C2900 projectile",
                              A_F0115_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.tag.creature", i + 1);
        ok &= expect_contains(label, f ? f->zone_binding_tag : 0, "C3200 creature",
                              A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.tag.explosion", i + 1);
        ok &= expect_contains(label, f ? f->zone_binding_tag : 0, "C3000 explosion",
                              A_F0115_EXPLOSION);
    }

    return ok;
}

static int test_route_exclusions_and_draw_order(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixtures[2] = {
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1),
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2)
    };

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *f = fixtures[i];
        const char *anchor = i == 0 ? A_D1L : A_D1R;
        char label[72];

        snprintf(label, sizeof(label), "side%d.floor_ornament_before_f0115", i + 1);
        ok &= expect_int(label, f ? f->floor_ornament_precedes_f0115 : 0, 1,
                         anchor);
        snprintf(label, sizeof(label), "side%d.ceiling_pit_before_f0115", i + 1);
        ok &= expect_int(label, f ? f->ceiling_pit_precedes_f0115 : 0, 1,
                         anchor);
        snprintf(label, sizeof(label), "side%d.teleporter_field_after_f0115", i + 1);
        ok &= expect_int(label, f ? f->teleporter_field_follows_f0115 : 0, 1,
                         anchor);
        snprintf(label, sizeof(label), "side%d.door_front_excluded", i + 1);
        ok &= expect_int(label, f ? f->door_front_route_excluded : 0, 1,
                         A_DEFS);
        snprintf(label, sizeof(label), "side%d.draw_order.objects", i + 1);
        ok &= expect_int(label, f ? f->f0115_draw_order_objects_first : 0, 1,
                         A_F0115);
        snprintf(label, sizeof(label), "side%d.draw_order.creatures", i + 1);
        ok &= expect_int(label, f ? f->f0115_draw_order_creatures_second : 0, 1,
                         A_F0115_CREATURE);
        snprintf(label, sizeof(label), "side%d.draw_order.projectiles", i + 1);
        ok &= expect_int(label, f ? f->f0115_draw_order_projectiles_third : 0,
                         1, A_F0115_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.draw_order.explosions", i + 1);
        ok &= expect_int(label, f ? f->f0115_draw_order_explosions_last : 0,
                         1, A_F0115_EXPLOSION);
    }

    return ok;
}

static int test_cross_source_bindings(void)
{
    int ok = 1;
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    ok &= expect_int("d1l2.csbwin.relative_cell",
                     d1l2 ? d1l2->csbwin_relative_cell : -1, 15, A_CSBWIN);
    ok &= expect_int("d1r2.csbwin.relative_cell",
                     d1r2 ? d1r2->csbwin_relative_cell : -1, 16, A_CSBWIN);
    ok &= expect_int("d1l2.csbwin.contents",
                     d1l2 ? d1l2->csbwin_contents_opcode : -1, 60125,
                     A_CSBWIN);
    ok &= expect_int("d1r2.csbwin.contents",
                     d1r2 ? d1r2->csbwin_contents_opcode : -1, 60127,
                     A_CSBWIN);
    ok &= expect_int("d1l2.csbwin.draw_order",
                     d1l2 ? d1l2->csbwin_draw_order_opcode : -1, 60272,
                     A_CSBWIN);
    ok &= expect_int("d1r2.csbwin.draw_order",
                     d1r2 ? d1r2->csbwin_draw_order_opcode : -1, 60275,
                     A_CSBWIN);
    ok &= expect_int("d1l2.csbwin.std_room_objects",
                     d1l2 ? d1l2->csbwin_std_draw_room_objects_opcode : -1,
                     60006, A_CSBWIN);
    ok &= expect_int("d1r2.csbwin.std_room_objects",
                     d1r2 ? d1r2->csbwin_std_draw_room_objects_opcode : -1,
                     60006, A_CSBWIN);
    ok &= expect_contains("d1l2.route_name", d1l2 ? d1l2->route_name : 0,
                          "M607 D1L corridor", A_D1L);
    ok &= expect_contains("d1r2.route_name", d1r2 ? d1r2->route_name : 0,
                          "M608 D1R corridor", A_D1R);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const char *e =
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_evidence_pc34();
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassEvidencePc34 *ev =
        csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_evidence_pc34();

    ok &= expect_contains("evidence.contract", e, "contract-only", A_F0115);
    ok &= expect_contains("evidence.no_asset", e, "no real-asset bitmap parity",
                          A_F0115);
    ok &= expect_contains("evidence.no_game_data", e, "no CSB game-data load",
                          A_F0115);
    ok &= expect_contains("evidence.dispatch", e, "DUNVIEW.C:6789-6793",
                          A_DISPATCH);
    ok &= expect_contains("evidence.d1l", e, "DUNVIEW.C:7391-7557", A_D1L);
    ok &= expect_contains("evidence.d1r", e, "DUNVIEW.C:7559-7725", A_D1R);
    ok &= expect_contains("evidence.f0115.order", e, "DUNVIEW.C:4547-4581",
                          A_F0115);
    ok &= expect_contains("evidence.item", e, "C2500 item", A_F0115_ITEM);
    ok &= expect_contains("evidence.creature", e, "C3200 creature",
                          A_F0115_CREATURE);
    ok &= expect_contains("evidence.projectile", e, "C2900 projectile",
                          A_F0115_PROJECTILE);
    ok &= expect_contains("evidence.explosion", e, "C3000 explosion",
                          A_F0115_EXPLOSION);
    ok &= expect_contains("evidence.defs_c10", e, "DEFS.H:2088", A_DEFS);
    ok &= expect_contains("evidence.defs_views", e, "DEFS.H:2596-2601",
                          A_DEFS);
    ok &= expect_contains("evidence.defs_door_out", e, "DEFS.H:4250-4260",
                          A_DEFS);
    ok &= expect_contains("evidence.csbwin", e, "CSBWin Viewport.cpp",
                          A_CSBWIN);
    ok &= expect_contains("evidence.lineage", e, "CSB-lineage Viewport.cpp",
                          A_LINEAGE);
    ok &= expect_contains("evidence.custom_backgrounds", e, "6503-6551",
                          A_LINEAGE);
    ok &= expect_contains("struct.f0122", ev ? ev->f0122_d1l_lines : 0,
                          "F0122_DUNGEONVIEW_DrawSquareD1L", A_D1L);
    ok &= expect_contains("struct.f0123", ev ? ev->f0123_d1r_lines : 0,
                          "F0123_DUNGEONVIEW_DrawSquareD1R", A_D1R);
    ok &= expect_contains("struct.f0115", ev ? ev->f0115_lines : 0,
                          "5615-5617", A_F0115_CREATURE);
    ok &= expect_contains("struct.defs", ev ? ev->defs_lines : 0,
                          "4250-4260", A_DEFS);
    ok &= expect_contains("struct.csbwin", ev ? ev->csbwin_room_object_lines : 0,
                          "1167-1188", A_CSBWIN);
    ok &= expect_contains("struct.lineage", ev ? ev->csb_lineage_room_object_lines : 0,
                          "1167-1188", A_LINEAGE);
    ok &= expect_contains("struct.custom", ev ? ev->csb_lineage_custom_backgrounds_lines : 0,
                          "6503-6551", A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_evidence_pc34());

    ok &= test_identity_and_accessors();
    ok &= test_contract_markers();
    ok &= test_view_square_and_order_metadata();
    ok &= test_zone_binding_tags();
    ok &= test_route_exclusions_and_draw_order();
    ok &= test_cross_source_bindings();
    ok &= test_evidence_strings();

    ok &= expect_int("assertion_count_at_least_60", g_assertions >= 60, 1,
                     A_F0115);
    printf("assertionCount=%d\n", g_assertions);
    if (ok) {
        printf("PASS final_status=1 anchor=%s assertionCount=%d\n",
               A_F0115, g_assertions);
    }
    return ok ? 0 : 1;
}
