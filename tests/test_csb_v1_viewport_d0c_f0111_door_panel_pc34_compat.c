/*
 * ReDMCSB evidence (CSB V1 D0C F0111 door panel):
 * - DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor; line 4262 base
 *   ornament; lines 4291-4294 Thieves-Eye branch gated by M631_ZONE_DOOR_D1C.
 * - DUNVIEW.C:1983-1989 and 8164-8311 are the local
 *   F0127_DUNGEONVIEW_DrawSquareD0C dispatch anchors. The requested
 *   F0118_DUNGEONVIEW_DrawSquareD0C_CPSF name is not present in this
 *   Common/Source snapshot.
 * - DUNVIEW.C:4547-4581 F0115 thing-pass loop; D0C calls it at 8294, outside
 *   the F0111 door-panel path.
 * - DUNVIEW.C:8478-8508 and 8534-8542 F0128 post-dispatch wall followup.
 * - DUNVIEW.C:3113-3156 F0104, 3185-3247 F0105, 3502-3938 F0107, and
 *   8164-8311 F0127 contrast non-door wall/frame/field routes.
 * - DEFS.H:2088 and 4040-4057 / 4249-4261 zone ids.
 * - CSB-lineage Viewport.cpp:1903-1906 thing-pass anchor.
 */
#include "csb_v1_viewport_d0c_f0111_door_panel_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_BASE_ORNAMENT =
    "ReDMCSB DUNVIEW.C:4262 F0109 base door ornament";
static const char *A_THIEVES_EYE =
    "ReDMCSB DUNVIEW.C:4291-4294 M631_ZONE_DOOR_D1C Thieves-Eye branch";
static const char *A_F0127 =
    "ReDMCSB DUNVIEW.C:1983-1989 and 8164-8311 F0127 D0C dispatch";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 thing pass; D0C call at 8294";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:8478-8508 and 8534-8542 F0128 post-dispatch";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,4040-4057,4249-4261";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1903-1906 thing pass";

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", id, got, anchor);
    return 1;
}

static int expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    return expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static int expect_contains(
    const char *id,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=\"%s\" anchor=%s\n",
               id, needle ? needle : "(null)", anchor);
        return 0;
    }
    printf("PASS %s contains=\"%s\" anchor=%s\n", id, needle, anchor);
    return 1;
}

static CSB_V1_D0CF0111DoorPanelInputPc34 door_input(
    int door_state,
    int ornament_ordinal,
    int thieves_eye)
{
    CSB_V1_D0CF0111DoorPanelInputPc34 input = {
        CSB_V1_D0C_F0111_ELEMENT_DOOR_FRONT_PC34,
        1,
        door_state,
        ornament_ordinal,
        thieves_eye,
        1
    };
    return input;
}

static int test_contract_identity_and_line_anchors(void)
{
    int ok = 1;
    const CSB_V1_D0CF0111DoorPanelContractPc34 *c =
        csb_v1_viewport_d0c_f0111_door_panel_contract_pc34();

    ok &= expect_int("contract.non_null", c != NULL, 1, A_F0111);
    ok &= expect_bool("contract.only", c ? c->source_locked_contract_only : false,
                      true, A_F0111);
    ok &= expect_bool("no.real.asset.bitmap.parity",
                      c ? c->no_real_asset_bitmap_parity : false, true, A_F0111);
    ok &= expect_bool("no.game.data.load", c ? c->no_game_data_load : false,
                      true, A_F0111);
    ok &= expect_bool("base.ornament.dispatches",
                      c ? c->f0111_base_ornament_dispatches : false, true,
                      A_BASE_ORNAMENT);
    ok &= expect_bool("d0c.thieves.eye.zone.rejected",
                      c ? c->d0c_thieves_eye_mask_rejected_by_zone : false, true,
                      A_THIEVES_EYE);
    ok &= expect_bool("f0115.anchor.stable",
                      c ? c->f0115_thing_pass_anchor_stable : false, true,
                      A_F0115);
    ok &= expect_bool("lineage.anchor.stable",
                      c ? c->csb_lineage_viewport_1903_1906_stable : false,
                      true, A_LINEAGE);
    ok &= expect_bool("partly.open.stays.f0111",
                      c ? c->partly_open_stays_on_f0111_not_wall_path : false,
                      true, A_F0111);

    ok &= expect_int("f0111.line.start", c ? c->f0111_line_start : -1, 4218,
                     A_F0111);
    ok &= expect_int("f0111.line.end", c ? c->f0111_line_end : -1, 4339,
                     A_F0111);
    ok &= expect_int("f0111.line.span", c ? c->f0111_line_end - c->f0111_line_start : -1,
                     121, A_F0111);
    ok &= expect_int("f0127.line.start", c ? c->f0127_line_start : -1, 8164,
                     A_F0127);
    ok &= expect_int("f0127.line.end", c ? c->f0127_line_end : -1, 8311,
                     A_F0127);
    ok &= expect_int("f0115.line.start", c ? c->f0115_line_start : -1, 4547,
                     A_F0115);
    ok &= expect_int("f0115.line.end", c ? c->f0115_line_end : -1, 4581,
                     A_F0115);
    ok &= expect_int("f0128.wall.followup.start",
                     c ? c->f0128_wall_followup_start : -1, 8478, A_F0128);
    ok &= expect_int("f0128.wall.followup.end",
                     c ? c->f0128_wall_followup_end : -1, 8508, A_F0128);
    ok &= expect_int("f0128.post.dispatch.start",
                     c ? c->f0128_post_dispatch_start : -1, 8534, A_F0128);
    ok &= expect_int("f0128.post.dispatch.end",
                     c ? c->f0128_post_dispatch_end : -1, 8542, A_F0128);

    ok &= expect_contains("anchor.f0111", c ? c->redmcsb_f0111_anchor : NULL,
                          "4218-4339", A_F0111);
    ok &= expect_contains("anchor.f0127", c ? c->redmcsb_f0127_anchor : NULL,
                          "8164-8311", A_F0127);
    ok &= expect_contains("anchor.f0127.alias.absent",
                          c ? c->redmcsb_f0127_anchor : NULL, "F0118", A_F0127);
    ok &= expect_contains("anchor.f0115", c ? c->redmcsb_f0115_anchor : NULL,
                          "4547-4581", A_F0115);
    ok &= expect_contains("anchor.f0128", c ? c->redmcsb_f0128_anchor : NULL,
                          "8534-8542", A_F0128);
    ok &= expect_contains("anchor.lineage", c ? c->csb_lineage_anchor : NULL,
                          "1903-1906", A_LINEAGE);

    return ok;
}

static int test_defs_and_zone_contract(void)
{
    int ok = 1;
    const CSB_V1_D0CF0111DoorPanelContractPc34 *c =
        csb_v1_viewport_d0c_f0111_door_panel_contract_pc34();

    ok &= expect_int("defs.c10", c ? c->transparent_color : -1, 10, A_DEFS);
    ok &= expect_int("defs.c6", c ? c->c6_unknown : -1, 6, A_DEFS);
    ok &= expect_int("defs.mask4000", c ? c->mask0x4000_shift : -1, 0x4000,
                     A_DEFS);
    ok &= expect_int("defs.destroyed.mask.c15",
                     c ? c->destroyed_mask_ordinal : -1, 15, A_DEFS);
    ok &= expect_int("defs.thieves.eye.mask.c16",
                     c ? c->thieves_eye_mask_ordinal : -1, 16, A_DEFS);
    ok &= expect_int("d0c.door.zone.m608", c ? c->door_zone_d0c : -1, 608,
                     "assigned D0C door zone M608");
    ok &= expect_int("d1c.thieves.eye.zone.m631",
                     c ? c->thieves_eye_zone_d1c : -1, 3790, A_THIEVES_EYE);
    ok &= expect_int("d0c.zone.not.d1c",
                     c ? (c->door_zone_d0c == c->thieves_eye_zone_d1c) : -1,
                     0, A_THIEVES_EYE);
    ok &= expect_int("d0c.wall.zone.c715", c ? c->wall_zone_d0c : -1, 715,
                     A_DEFS);
    ok &= expect_int("d0c.view.square.m609.pc34",
                     c ? c->view_square_d0c : -1, 0, A_F0127);
    ok &= expect_int("d0c.f0115.cell.order",
                     c ? c->f0115_cell_order_d0c : -1, 0x0021, A_F0115);
    ok &= expect_contains("defs.anchor.c10", c ? c->redmcsb_defs_anchor : NULL,
                          "2088", A_DEFS);
    ok &= expect_contains("defs.anchor.zones", c ? c->redmcsb_defs_anchor : NULL,
                          "4040-4057", A_DEFS);
    ok &= expect_contains("defs.anchor.door.zones",
                          c ? c->redmcsb_defs_anchor : NULL, "4249-4261",
                          A_DEFS);

    return ok;
}

static int test_closed_d0c_door_and_ornament_ordinals(void)
{
    int ok = 1;
    CSB_V1_D0CF0111DoorPanelStatePc34 csbDoorState =
        csb_v1_viewport_d0c_f0111_door_panel_dispatch_pc34(
            door_input(4, 0, 1));

    ok &= expect_int("closed.branch",
                     csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(4),
                     CSB_V1_D0C_F0111_BRANCH_CLOSED_PC34, A_F0111);
    ok &= expect_int("closed.capturedF0111", csbDoorState.capturedF0111, 1,
                     A_F0111);
    ok &= expect_int("closed.base.ornament",
                     csbDoorState.capturedBaseOrnament, 1, A_BASE_ORNAMENT);
    ok &= expect_int("closed.not.wall.path",
                     csbDoorState.capturedF0118WallPath, 0, A_F0128);
    ok &= expect_int("closed.thieves.eye.not.taken",
                     csbDoorState.capturedThievesEyeBranch, 0, A_THIEVES_EYE);
    ok &= expect_int("closed.no.f0115.inside.f0111",
                     csbDoorState.capturedF0115InsideF0111, 0, A_F0115);
    ok &= expect_int("closed.final.zone", csbDoorState.final_zone, 608,
                     "closed D0C door at M608");
    ok &= expect_int("closed.transparent.color",
                     csbDoorState.transparent_color, 10, A_DEFS);
    ok &= expect_int("closed.blend.transparent",
                     csb_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
                         0x44, 10, 10),
                     0x44, "ReDMCSB DUNVIEW.C:4334 C10 transparent blit");
    ok &= expect_int("closed.blend.opaque",
                     csb_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
                         0x44, 0x55, 10),
                     0x55, "ReDMCSB DUNVIEW.C:4334 opaque blit");

    for (int ordinal = 0; ordinal <= 5; ++ordinal) {
        CSB_V1_D0CF0111DoorPanelStatePc34 ordinal_state =
            csb_v1_viewport_d0c_f0111_door_panel_dispatch_pc34(
                door_input(4, ordinal, 0));
        ok &= expect_int("closed.ornament.ordinal.c0_to_c5",
                         csb_v1_viewport_d0c_f0111_door_panel_closed_ornament_pc34(
                             4, ordinal),
                         ordinal, A_BASE_ORNAMENT);
        ok &= expect_int("closed.ordinal.dispatch.f0111",
                         ordinal_state.capturedF0111, 1, A_F0111);
        ok &= expect_int("closed.ordinal.value",
                         ordinal_state.ornament_ordinal, ordinal,
                         "closed-door path includes C0..C5 ornament ordinals");
    }

    ok &= expect_int("closed.ornament.before.range.rejected",
                     csb_v1_viewport_d0c_f0111_door_panel_closed_ornament_pc34(
                         4, -1),
                     -1, A_BASE_ORNAMENT);
    ok &= expect_int("closed.ornament.after.range.rejected",
                     csb_v1_viewport_d0c_f0111_door_panel_closed_ornament_pc34(
                         4, 6),
                     -1, A_BASE_ORNAMENT);
    ok &= expect_int("open.ornament.rejected",
                     csb_v1_viewport_d0c_f0111_door_panel_closed_ornament_pc34(
                         0, 3),
                     -1, A_F0111);

    return ok;
}

static int test_partly_open_d0c_stays_on_f0111(void)
{
    int ok = 1;
    static const int expected_first_half[] = { -1, 615, 616, 617 };
    static const int expected_second_half[] = { -1, 16996, 16997, 16998 };

    for (int state = 1; state <= 3; ++state) {
        CSB_V1_D0CF0111DoorPanelStatePc34 partly =
            csb_v1_viewport_d0c_f0111_door_panel_dispatch_pc34(
                door_input(state, state, 1));
        ok &= expect_int("partly.branch",
                         csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(state),
                         CSB_V1_D0C_F0111_BRANCH_PARTLY_OPEN_PC34, A_F0111);
        ok &= expect_int("partly.capturedF0111", partly.capturedF0111, 1,
                         A_F0111);
        ok &= expect_int("partly.not.f0118.wall.path",
                         partly.capturedF0118WallPath, 0, A_F0128);
        ok &= expect_int("partly.base.ornament",
                         partly.capturedBaseOrnament, 1, A_BASE_ORNAMENT);
        ok &= expect_int("partly.thieves.eye.not.taken",
                         partly.capturedThievesEyeBranch, 0, A_THIEVES_EYE);
        ok &= expect_int("partly.first.half.zone",
                         partly.first_half_zone, expected_first_half[state],
                         "ReDMCSB DUNVIEW.C:4317-4324 P2084+state+C6");
        ok &= expect_int("partly.second.half.zone",
                         partly.second_half_zone, expected_second_half[state],
                         "ReDMCSB DUNVIEW.C:4325-4326 state+3|MASK0x4000");
        ok &= expect_int("partly.no.f0115.inside.f0111",
                         partly.capturedF0115InsideF0111, 0, A_F0115);
    }

    ok &= expect_int("open.branch.skip",
                     csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(0),
                     CSB_V1_D0C_F0111_BRANCH_OPEN_SKIP_PC34, A_F0111);
    ok &= expect_int("destroyed.branch",
                     csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(5),
                     CSB_V1_D0C_F0111_BRANCH_DESTROYED_PC34, A_F0111);
    ok &= expect_int("invalid.branch",
                     csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(6),
                     CSB_V1_D0C_F0111_BRANCH_INVALID_PC34, A_F0111);

    return ok;
}

static int test_no_door_center_field_sanity_and_anchors(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d0c_f0111_door_panel_source_evidence_pc34();
    CSB_V1_D0CF0111DoorPanelInputPc34 input = {
        CSB_V1_D0C_F0111_ELEMENT_EMPTY_CENTER_FIELD_PC34,
        0,
        0,
        0,
        0,
        0
    };
    CSB_V1_D0CF0111DoorPanelStatePc34 no_door =
        csb_v1_viewport_d0c_f0111_door_panel_dispatch_pc34(input);

    ok &= expect_int("no.door.branch", no_door.branch,
                     CSB_V1_D0C_F0111_BRANCH_NO_DOOR_PC34, A_F0127);
    ok &= expect_int("no.door.no.f0111", no_door.capturedF0111, 0, A_F0127);
    ok &= expect_int("no.door.center.field.sane",
                     no_door.no_door_center_field_sane, 1, A_F0127);
    ok &= expect_int("no.door.wall.zone", no_door.wall_zone_d0c, 715, A_DEFS);
    ok &= expect_int("no.door.f0115.anchor.stable",
                     no_door.f0115ThingPassAnchorStable, 1, A_F0115);
    ok &= expect_int("no.door.lineage.anchor.stable",
                     no_door.csbLineageViewport1903ThingPassStable, 1,
                     A_LINEAGE);
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4339", A_F0111);
    ok &= expect_contains("evidence.base.ornament", e, "line 4262",
                          A_BASE_ORNAMENT);
    ok &= expect_contains("evidence.thieves.eye", e, "4291-4294",
                          A_THIEVES_EYE);
    ok &= expect_contains("evidence.f0127", e, "F0127_DUNGEONVIEW_DrawSquareD0C",
                          A_F0127);
    ok &= expect_contains("evidence.f0118.absent", e, "F0118", A_F0127);
    ok &= expect_contains("evidence.f0115", e, "4547-4581", A_F0115);
    ok &= expect_contains("evidence.f0128", e, "8478-8508", A_F0128);
    ok &= expect_contains("evidence.f0104", e, "3113-3156 F0104", A_F0127);
    ok &= expect_contains("evidence.f0105", e, "3185-3247 F0105", A_F0127);
    ok &= expect_contains("evidence.f0107", e, "3502-3938 F0107", A_F0127);
    ok &= expect_contains("evidence.defs", e, "DEFS.H:2088", A_DEFS);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1903-1906",
                          A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_contract_identity_and_line_anchors();
    ok &= test_defs_and_zone_contract();
    ok &= test_closed_d0c_door_and_ornament_ordinals();
    ok &= test_partly_open_d0c_stays_on_f0111();
    ok &= test_no_door_center_field_sanity_and_anchors();

    ok &= expect_int("assertions.at.least.80", g_assertions >= 80, 1,
                     "assigned CSB V1 D0C F0111 door-panel gate");
    ok &= expect_int("assertions.at.most.120", g_assertions <= 120, 1,
                     "assigned CSB V1 D0C F0111 door-panel gate");

    printf("probe=csb_v1_viewport_d0c_f0111_door_panel_pc34_compat\n");
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d0c_f0111_door_panel_pc34_compat assertions=%d\n",
               g_assertions);
        return 0;
    }
    return 1;
}
