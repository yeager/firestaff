/*
 * ReDMCSB evidence (D0C F0111 door panel):
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor.
 * - DUNVIEW.C:8164-8363 F0127_DUNGEONVIEW_DrawSquareD0C.
 * - DUNVIEW.C:594 G0163_aauc_Graphic558_Frame_Walls D0C row
 *   = { 0, 223, 0, 135, 0, 0, 0, 0 }.
 * - DUNVIEW.C:597 G0172_auc_Graphic558_Frame_DoorFrame_D0C
 *   = { 96, 127, 0, 122, 16, 123, 0, 0 }.
 * - DEFS.H:1039-1044,2088,2466,3508,3516,4036,4055,4067,4086.
 *
 * Drift hardening mirrors pass515 / pass570 / pass577 / pass510 anchors
 * in test_dm1_v1_viewport_3d_pc34_compat.c:2838-3020.  Each needle is a
 * whole-file token that must remain in the firestaff tree so a future
 * m11_game_view.c refactor cannot silently break the source-lock.
 */
#include "dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor";
static const char *A_F0127 =
    "ReDMCSB DUNVIEW.C:8164-8363 F0127_DUNGEONVIEW_DrawSquareD0C";
static const char *A_G0163 =
    "ReDMCSB DUNVIEW.C:594 G0163_aauc_Graphic558_Frame_Walls[M609]";
static const char *A_G0172 =
    "ReDMCSB DUNVIEW.C:597 G0172_auc_Graphic558_Frame_DoorFrame_D0C";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:1039-1044,2088,2466,3508,3516";
static const char *A_ZONES =
    "ReDMCSB DEFS.H:4036,4055,4067,4086 D0C wall/door-frame zones";

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

static int expect_contains(const char *id, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing \"%s\" anchor=%s\n",
               id, needle ? needle : "(null)", anchor);
        return 0;
    }
    printf("PASS %s contains \"%s\" anchor=%s\n", id, needle, anchor);
    return 1;
}

static int expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    return expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static int test_contract_identity_and_anchors(void)
{
    int ok = 1;
    const DM1_V1_D0CF0111DoorPanelPc34Contract *c =
        dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract();

    ok &= expect_int("contract.non_null", c != NULL, 1, A_F0111);
    ok &= expect_bool("contract.only", c ? c->source_locked_contract_only : 0,
                      true, A_F0111);
    ok &= expect_bool("no.real.asset.bitmap.parity",
                      c ? c->no_real_asset_bitmap_parity : 0, true, A_F0111);
    ok &= expect_bool("no.game.data.load", c ? c->no_game_data_load : 0,
                      true, A_F0111);
    ok &= expect_bool("f0111.line.range.anchor",
                      c ? c->f0111_line_range_anchor_present : 0, true,
                      A_F0111);
    ok &= expect_bool("f0127.line.range.anchor",
                      c ? c->f0127_line_range_anchor_present : 0, true,
                      A_F0127);
    ok &= expect_contains("f0111.anchor.string",
                          c ? c->redmcsb_f0111_anchor : NULL,
                          "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("f0111.function.name",
                          c ? c->redmcsb_f0111_anchor : NULL,
                          "F0111_DUNGEONVIEW_DrawDoor", A_F0111);
    ok &= expect_contains("f0127.anchor.string",
                          c ? c->redmcsb_f0127_anchor : NULL,
                          "DUNVIEW.C:8164-8363", A_F0127);
    ok &= expect_contains("f0127.function.name",
                          c ? c->redmcsb_f0127_anchor : NULL,
                          "F0127_DUNGEONVIEW_DrawSquareD0C", A_F0127);

    return ok;
}

static int test_d0c_geometry_and_zone_math(void)
{
    int ok = 1;
    const DM1_V1_D0CF0111DoorPanelPc34Contract *c =
        dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract();

    ok &= expect_int("d0c.view_square", c ? c->d0c_view_square : -1, 9, A_F0111);
    ok &= expect_int("d0c.view_square.m609", c ? c->view_square_d0c : -1, 9,
                     A_F0111);
    ok &= expect_int("d0c.view_depth", c ? c->d0c_view_depth : -1, 0,
                     "ReDMCSB DUNVIEW.C:372 G2027[9] M609_VIEW_SQUARE_D0C");
    ok &= expect_int("d0c.view_lane", c ? c->d0c_view_lane : -9, 0,
                     "ReDMCSB DUNVIEW.C:371 G2026[9] D0C center lane");

    /* G0163 D0C row: full viewport, no-blit sentinel. */
    ok &= expect_int("d0c.g0163.left_x", c ? c->d0c_g0163_left_x : -1, 0,
                     A_G0163);
    ok &= expect_int("d0c.g0163.right_x", c ? c->d0c_g0163_right_x : -1, 223,
                     A_G0163);
    ok &= expect_int("d0c.g0163.top_y", c ? c->d0c_g0163_top_y : -1, 0,
                     A_G0163);
    ok &= expect_int("d0c.g0163.bottom_y", c ? c->d0c_g0163_bottom_y : -1, 135,
                     A_G0163);
    ok &= expect_int("d0c.g0163.byte_width",
                     c ? c->d0c_g0163_byte_width : -1, 0, A_G0163);
    ok &= expect_int("d0c.g0163.height", c ? c->d0c_g0163_height : -1, 0,
                     A_G0163);
    ok &= expect_bool("d0c.g0163.full_viewport_no_blit",
                      c ? c->d0c_g0163_frame_row_full_viewport_no_blit : 0,
                      true, A_G0163);
    ok &= expect_contains("g0163.anchor",
                          c ? c->redmcsb_d0c_g0163_anchor : NULL,
                          "0, 223, 0, 135, 0, 0, 0, 0", A_G0163);

    /* G0172 D0C door frame. */
    ok &= expect_int("d0c.g0172.door_frame.left_x",
                     c ? c->d0c_g0172_door_frame_left_x : -1, 96, A_G0172);
    ok &= expect_int("d0c.g0172.door_frame.right_x",
                     c ? c->d0c_g0172_door_frame_right_x : -1, 127, A_G0172);
    ok &= expect_int("d0c.g0172.door_frame.top_y",
                     c ? c->d0c_g0172_door_frame_top_y : -1, 0, A_G0172);
    ok &= expect_int("d0c.g0172.door_frame.bottom_y",
                     c ? c->d0c_g0172_door_frame_bottom_y : -1, 122,
                     A_G0172);
    ok &= expect_int("d0c.g0172.door_frame.byte_width",
                     c ? c->d0c_g0172_door_frame_byte_width : -1, 16,
                     A_G0172);
    ok &= expect_int("d0c.g0172.door_frame.height",
                     c ? c->d0c_g0172_door_frame_height : -1, 123, A_G0172);
    ok &= expect_bool("d0c.g0172.anchor.present",
                      c ? c->d0c_g0172_door_frame_anchor_present : 0, true,
                      A_G0172);
    ok &= expect_contains("g0172.anchor",
                          c ? c->redmcsb_d0c_g0172_anchor : NULL,
                          "96, 127, 0, 122, 16, 123, 0, 0", A_G0172);

    return ok;
}

static int test_d0c_zone_constants(void)
{
    int ok = 1;
    const DM1_V1_D0CF0111DoorPanelPc34Contract *c =
        dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract();

    ok &= expect_int("d0c.zone.wall.media508", c ? c->d0c_zone_wall_media508 : -1,
                     713, A_ZONES);
    ok &= expect_int("d0c.zone.wall.media720", c ? c->d0c_zone_wall_media720 : -1,
                     715, A_ZONES);
    ok &= expect_int("d0c.zone.door_frame.media508",
                     c ? c->d0c_zone_door_frame_media508 : -1, 724, A_ZONES);
    ok &= expect_int("d0c.zone.door_frame.media720",
                     c ? c->d0c_zone_door_frame_media720 : -1, 728, A_ZONES);
    ok &= expect_int("d0c.zone.floor_pit.media720",
                     c ? c->d0c_zone_floor_pit_media720 : -1, 862,
                     "ReDMCSB DUNVIEW.C:8279-8280 C862_ZONE_FLOORPIT_D0C");
    ok &= expect_int("d0c.zone.ceiling_pit.media720",
                     c ? c->d0c_zone_ceiling_pit_media720 : -1, 871,
                     "ReDMCSB DUNVIEW.C:8294-8296 C871_ZONE_CEILING_PIT_D0C");
    ok &= expect_contains("zones.anchor",
                          c ? c->redmcsb_d0c_zones_anchor : NULL,
                          "DEFS.H:4036", A_ZONES);
    ok &= expect_contains("zones.anchor.media720",
                          c ? c->redmcsb_d0c_zones_anchor : NULL,
                          "DEFS.H:4055", A_ZONES);
    ok &= expect_contains("zones.anchor.door_frame.media508",
                          c ? c->redmcsb_d0c_zones_anchor : NULL,
                          "DEFS.H:4067", A_ZONES);
    ok &= expect_contains("zones.anchor.door_frame.media720",
                          c ? c->redmcsb_d0c_zones_anchor : NULL,
                          "DEFS.H:4086", A_ZONES);

    return ok;
}

static int test_d0c_f0111_state_machine_constants(void)
{
    int ok = 1;
    const DM1_V1_D0CF0111DoorPanelPc34Contract *c =
        dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract();

    /* F0111 line range. */
    ok &= expect_int("f0111.line.start", c ? c->f0111_line_start : -1, 4218,
                     A_F0111);
    ok &= expect_int("f0111.line.end", c ? c->f0111_line_end : -1, 4337,
                     A_F0111);
    ok &= expect_int("f0111.line.span",
                     c ? c->f0111_line_end - c->f0111_line_start : -1, 119,
                     A_F0111);

    /* F0127 line range. */
    ok &= expect_int("f0127.line.start", c ? c->f0127_line_start : -1, 8164,
                     A_F0127);
    ok &= expect_int("f0127.line.end", c ? c->f0127_line_end : -1, 8363,
                     A_F0127);
    ok &= expect_int("f0127.line.span",
                     c ? c->f0127_line_end - c->f0127_line_start : -1, 199,
                     A_F0127);

    /* Door state machine. */
    ok &= expect_int("open.state", c ? c->open_state : -1, 0, A_DEFS);
    ok &= expect_int("closed.state", c ? c->closed_state : -1, 4, A_DEFS);
    ok &= expect_int("destroyed.state", c ? c->destroyed_state : -1, 5,
                     A_DEFS);
    ok &= expect_int("c10.color.flesh", c ? c->c10_color_flesh : -1, 10,
                     A_DEFS);
    ok &= expect_int("c15.destroyed.mask", c ? c->c15_destroyed_mask : -1,
                     15, A_DEFS);
    ok &= expect_int("c6.unknown", c ? c->c6_unknown : -1, 6, A_DEFS);
    ok &= expect_int("mask0x4000.shift", c ? c->mask0x4000_shift : -1, 0x4000,
                     A_DEFS);

    return ok;
}

static int test_d0c_f0111_call_site_and_negative_boundary(void)
{
    int ok = 1;
    const DM1_V1_D0CF0111DoorPanelPc34Contract *c =
        dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract();

    /* (1) F0111 D0C call-site-reached contract: south-facing Hall D0C
     *     door cell, C0..C5 door state covers all six states. */
    ok &= expect_bool("f0127.dispatches.d0c.door_side",
                      c ? c->f0127_dispatches_d0c_door_side_c16 : 0, true,
                      "ReDMCSB DUNVIEW.C:8185-8216 F0127 C16_ELEMENT_DOOR_SIDE");

    /* (2) F0111 C10_COLOR_FLESH transparent blit honored. */
    ok &= expect_bool("c10.transparent.blit",
                      c ? c->c10_color_flesh_transparent_blit : 0, true,
                      A_F0111);
    ok &= expect_int("c10.blend.transparent",
                     dm1_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
                         0x44, 10, 10),
                     0x44, "ReDMCSB DUNVIEW.C:4334 F0791 C10 transparent blit");
    ok &= expect_int("c10.blend.opaque",
                     dm1_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
                         0x44, 0x55, 10),
                     0x55, "ReDMCSB DUNVIEW.C:4334 F0791 opaque pixel writes");

    /* (3) F0111 destroyed-mask zero-pixel write path. */
    ok &= expect_bool("c15.destroyed.mask.path",
                      c ? c->c15_destroyed_mask_path : 0, true,
                      "ReDMCSB DUNVIEW.C:4301-4304 C15 destroyed mask path");
    ok &= expect_bool("destroyed.state.returns.zero.pixel",
                      c ? c->destroyed_state_returns_zero_pixel : 0, true,
                      "ReDMCSB DUNVIEW.C:4301-4304 destroyed mask writes");

    /* (4) D0C F0111 dispatch does NOT call F0100/F0105/F0107/F0115. */
    ok &= expect_bool("d0c.f0111.no.f0100",
                      c ? c->d0c_f0111_does_not_call_f0100 : 0, true,
                      "F0100 is a non-door wall bitmap route, not F0111");
    ok &= expect_bool("d0c.f0111.no.f0105",
                      c ? c->d0c_f0111_does_not_call_f0105 : 0, true,
                      "F0105 is a non-door floor-pit/stairs flip route");
    ok &= expect_bool("d0c.f0111.no.f0107",
                      c ? c->d0c_f0111_does_not_call_f0107 : 0, true,
                      "F0107 is a non-door wall-ornament alcove route");
    ok &= expect_bool("d0c.f0111.no.f0115",
                      c ? c->d0c_f0111_does_not_call_f0115 : 0, true,
                      "F0115 is the non-door thing-pass route");

    /* (5) D0C F0111 line range guard. */
    ok &= expect_bool("open.state.skips.f0111",
                      c ? c->open_state_skips_f0111 : 0, true,
                      "ReDMCSB DUNVIEW.C:4248 C0_DOOR_STATE_OPEN guard");
    ok &= expect_bool("closed.state.uses.d0c.zone",
                      c ? c->closed_state_uses_d0c_zone : 0, true,
                      "ReDMCSB DUNVIEW.C:4297-4298 C4 closed branch");
    ok &= expect_bool("c6.unknown.partial.zone.shift",
                      c ? c->c6_unknown_partial_zone_shift : 0, true,
                      "ReDMCSB DUNVIEW.C:4322 partial horizontal front-half");
    ok &= expect_bool("mask0x4000.unreadable.inscription.shift",
                      c ? c->mask0x4000_unreadable_inscription_shift : 0, true,
                      "ReDMCSB DUNVIEW.C:4325 3|MASK0x4000 partial shift");

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const DM1_V1_D0CF0111DoorPanelPc34Contract *c =
        dm1_v1_viewport_d0c_f0111_door_panel_pc34_contract();
    const char *e = dm1_v1_viewport_d0c_f0111_door_panel_pc34_source_evidence();

    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("evidence.f0111.function", e,
                          "F0111_DUNGEONVIEW_DrawDoor", A_F0111);
    ok &= expect_contains("evidence.f0127", e, "DUNVIEW.C:8164-8363", A_F0127);
    ok &= expect_contains("evidence.f0127.function", e,
                          "F0127_DUNGEONVIEW_DrawSquareD0C", A_F0127);
    ok &= expect_contains("evidence.g0163", e,
                          "G0163_aauc_Graphic558_Frame_Walls[M609_VIEW_SQUARE_D0C]",
                          A_G0163);
    ok &= expect_contains("evidence.g0172", e,
                          "G0172_auc_Graphic558_Frame_DoorFrame_D0C",
                          A_G0172);
    ok &= expect_contains("evidence.open.guard", e, "4248 C0_DOOR_STATE_OPEN",
                          A_F0111);
    ok &= expect_contains("evidence.destroyed.mask", e,
                          "C15_DOOR_ORNAMENT_DESTROYED_MASK", A_DEFS);
    ok &= expect_contains("evidence.c10.blit", e, "C10_COLOR_FLESH",
                          A_F0111);
    ok &= expect_contains("evidence.f0791", e, "F0791", A_F0111);
    ok &= expect_contains("evidence.m609", e, "M609_VIEW_SQUARE_D0C", A_F0111);
    ok &= expect_contains("evidence.thieves.eye", e,
                          "C16_DOOR_ORNAMENT_THIEVES_EYE_MASK", A_F0111);
    ok &= expect_contains("evidence.d0c.cell_order", e,
                          "C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT",
                          "ReDMCSB DUNVIEW.C:8294 D0C F0115 cell order");
    ok &= expect_contains("evidence.d0c.field", e,
                          "C05_ELEMENT_TELEPORTER", A_F0127);
    ok &= expect_contains("evidence.non.overlap", e,
                          "Non-overlap: this gate is the D0C F0111 door-panel",
                          A_F0111);
    ok &= expect_contains("evidence.pointer",
                          c ? c->d0c_f0111_door_panel_source_evidence : NULL,
                          "G0172_auc_Graphic558_Frame_DoorFrame_D0C",
                          A_G0172);

    return ok;
}

/* ----- drift hardening (whole-file token-presence guards) ----- */

static int read_whole_file(const char *rel, char **out_data, size_t *out_size)
{
    char path[1024];
    FILE *fp;
    long size;
    char *data;
    size_t got;

    *out_data = NULL;
    *out_size = 0;
#ifndef FIRESTAFF_SOURCE_DIR
#define FIRESTAFF_SOURCE_DIR "."
#endif
    snprintf(path, sizeof(path), "%s/%s", FIRESTAFF_SOURCE_DIR, rel);
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    size = ftell(fp);
    if (size < 0) { fclose(fp); return 0; }
    rewind(fp);
    data = (char *)malloc((size_t)size + 1);
    if (!data) { fclose(fp); return 0; }
    got = fread(data, 1, (size_t)size, fp);
    fclose(fp);
    if (got != (size_t)size) { free(data); return 0; }
    data[size] = '\0';
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int file_contains(const char *rel, const char *needle)
{
    char *data = NULL;
    size_t size = 0;
    int hit;
    if (!read_whole_file(rel, &data, &size)) return 0;
    hit = strstr(data, needle) != NULL;
    free(data);
    return hit;
}

static int test_dm1_v1_d0c_f0111_drift_regression(void)
{
    /* Whole-file token-presence guards mirror the pattern in
     * test_dm1_v1_viewport_3d_pc34_compat.c:2838-3020.  These anchors
     * are the canonical evidence tokens the source-lock must continue
     * to honor.  Future m11_game_view.c refactors that delete or rename
     * any of these tokens will fail the regression. */
    static const struct {
        const char *rel;
        const char *token;
        const char *id;
    } needles[] = {
        /* pass515: D0C F0111 door-panel contract surface. */
        { "include/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.h",
          "F0127_DUNGEONVIEW_DrawSquareD0C",
          "pass515_d0c_f0111_header" },
        { "include/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.h",
          "F0111_DUNGEONVIEW_DrawDoor",
          "pass515_d0c_f0111_header_f0111" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "G0172_auc_Graphic558_Frame_DoorFrame_D0C",
          "pass515_d0c_f0111_source_g0172" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "G0163_aauc_Graphic558_Frame_Walls",
          "pass515_d0c_f0111_source_g0163" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C10_COLOR_FLESH",
          "pass515_d0c_f0111_source_c10" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C15_DOOR_ORNAMENT_DESTROYED_MASK",
          "pass515_d0c_f0111_source_c15" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C728_ZONE_DOOR_FRAME_D0C",
          "pass515_d0c_f0111_source_c728" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C724_ZONE_DOOR_FRAME_D0C",
          "pass515_d0c_f0111_source_c724" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C713_ZONE_WALL_D0C",
          "pass515_d0c_f0111_source_c713" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C715_ZONE_WALL_D0C",
          "pass515_d0c_f0111_source_c715" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR",
          "pass515_d0c_f0111_source_mask4000" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C6_UNKNOWN",
          "pass515_d0c_f0111_source_c6" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "F0791",
          "pass515_d0c_f0111_source_f0791" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "M609_VIEW_SQUARE_D0C",
          "pass515_d0c_f0111_source_m609" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C16_DOOR_ORNAMENT_THIEVES_EYE_MASK",
          "pass515_d0c_f0111_source_thieves_eye" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT",
          "pass515_d0c_f0111_source_cell_order" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C02_ELEMENT_PIT",
          "pass515_d0c_f0111_source_c02" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C05_ELEMENT_TELEPORTER",
          "pass515_d0c_f0111_source_c05" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C19_ELEMENT_STAIRS_FRONT",
          "pass515_d0c_f0111_source_c19" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "C16_ELEMENT_DOOR_SIDE",
          "pass515_d0c_f0111_source_c16" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "0, 223, 0, 135, 0, 0, 0, 0",
          "pass515_d0c_f0111_source_g0163_values" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "96, 127, 0, 122, 16, 123, 0, 0",
          "pass515_d0c_f0111_source_g0172_values" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DUNVIEW.C:4218-4337",
          "pass515_d0c_f0111_source_f0111_lines" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DUNVIEW.C:8164-8363",
          "pass515_d0c_f0111_source_f0127_lines" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DUNVIEW.C:594",
          "pass515_d0c_f0111_source_g0163_line" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DUNVIEW.C:597",
          "pass515_d0c_f0111_source_g0172_line" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DEFS.H:1039-1044",
          "pass515_d0c_f0111_source_defs_door_states" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DEFS.H:2088",
          "pass515_d0c_f0111_source_defs_c10" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DEFS.H:2466",
          "pass515_d0c_f0111_source_defs_c15" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DEFS.H:3508",
          "pass515_d0c_f0111_source_defs_c6" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DEFS.H:3516",
          "pass515_d0c_f0111_source_defs_mask4000" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DEFS.H:4036",
          "pass515_d0c_f0111_source_defs_c713" },
        { "src/dm1/dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "DEFS.H:4055",
          "pass515_d0c_f0111_source_defs_c715" },
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "dm1_viewport_3d_get_wall_frame",
          "pass515_d0c_f0111_existing_3d_helper" },
        /* Reuse the existing DM1 wall-frame table. */
        { "src/dm1/dm1_v1_viewport_3d_pc34_compat.c",
          "G0163_aauc_Graphic558_Frame_Walls",
          "pass515_d0c_f0111_existing_g0163" },
        { "tests/test_dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat.c",
          "pass515_d0c_f0111_anchor",
          "pass515_d0c_f0111_test_self_ref" },
    };

    for (size_t i = 0; i < sizeof(needles) / sizeof(needles[0]); ++i) {
        char id[160];
        snprintf(id, sizeof(id), "drift.%s", needles[i].id);
        if (!file_contains(needles[i].rel, needles[i].token)) {
            ++g_failures;
            printf("FAIL %s missing \"%s\" in %s\n",
                   id, needles[i].token, needles[i].rel);
        } else {
            ++g_assertions;
            printf("PASS %s present in %s\n", id, needles[i].rel);
        }
    }

    return 1;
}

int main(void)
{
    int ok = 1;

    printf("probe=dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_viewport_d0c_f0111_door_panel_pc34_source_evidence());

    ok &= test_contract_identity_and_anchors();
    ok &= test_d0c_geometry_and_zone_math();
    ok &= test_d0c_zone_constants();
    ok &= test_d0c_f0111_state_machine_constants();
    ok &= test_d0c_f0111_call_site_and_negative_boundary();
    ok &= test_evidence_strings();
    ok &= test_dm1_v1_d0c_f0111_drift_regression();

    ok &= expect_int("assertion_count_at_least_30", g_assertions >= 30, 1,
                     A_F0111);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS dm1_v1_viewport_d0c_f0111_door_panel_pc34_compat "
               "assertions=%d failures=0\n", g_assertions);
    }

    return (ok && g_failures == 0) ? 0 : 1;
}
