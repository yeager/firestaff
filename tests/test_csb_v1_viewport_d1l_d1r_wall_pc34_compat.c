#include "csb/csb_v1_viewport_d1l_d1r_wall_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    VIEWPORT_WIDTH = 224,
    VIEWPORT_HEIGHT = 136,
    SOURCE_WIDTH = 256,
    SOURCE_HEIGHT = 111,
    TRANSPARENT = 10
};

static const char *A_D1L =
    "ReDMCSB DUNVIEW.C:7391-7560 F0122_DUNGEONVIEW_DrawSquareD1L";
static const char *A_D1R =
    "ReDMCSB DUNVIEW.C:7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R";
static const char *A_BLIT =
    "ReDMCSB DUNVIEW.C:3113-3156 F0104; 3185-3247 F0105";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:4547-4581,5668-5671 F0115 follow-up keep-out";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:8318-8542 F0128; DUNVIEW.C:8294 F0127";
static const char *A_DUNGEON =
    "ReDMCSB DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH; 4040-4057 C716/C717";
static const char *A_CLIP =
    "ReDMCSB COORD.C:1713-1722; COMMAND.C:1126-1127";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1192-1209,1903-1915";

static int g_assertions = 0;
static int g_failures = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        ++g_failures;
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_u16(const char *label, uint16_t got, uint16_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04x want=0x%04x anchor=%s\n",
               label, (unsigned int)got, (unsigned int)want, anchor);
        ++g_failures;
        return 0;
    }
    printf("PASS %s=0x%04x anchor=%s\n", label, (unsigned int)got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != 0;
    return expect_int(label, got, 1, anchor);
}

static size_t viewport_offset(int y, int x)
{
    return (size_t)y * VIEWPORT_WIDTH + (size_t)x;
}

static size_t source_offset(int y, int x)
{
    return (size_t)y * SOURCE_WIDTH + (size_t)x;
}

static int test_run_entry_point(void)
{
    int ok = 1;
    CSB_V1_D1LD1RWallRunResultPc34 result;

    ok &= expect_int("run.return",
                     csb_v1_viewport_d1l_d1r_wall_pc34_compat_run(&result),
                     0, A_F0128);
    ok &= expect_int("run.ok", result.ok, 1,
                     "contract-only D1L/D1R wall pair");
    ok &= expect_int("run.route_count", result.route_count, 2, A_F0128);
    ok &= expect_int("run.identities_ok", result.identities_ok, 1, A_D1L);
    ok &= expect_int("run.coordinates_ok", result.coordinates_ok, 1, A_DEFS);
    ok &= expect_int("run.c10_transparency_ok", result.c10_transparency_ok, 1,
                     A_BLIT);
    ok &= expect_int("run.row_local_flip_ok", result.row_local_flip_ok, 1,
                     A_BLIT);
    ok &= expect_int("run.edge_clip_ok", result.edge_clip_ok, 1, A_CLIP);
    ok &= expect_int("run.f0128_followup_ok", result.f0128_followup_ok, 1,
                     A_F0128);
    ok &= expect_int("run.dungeon_identity_ok", result.dungeon_identity_ok, 1,
                     A_DUNGEON);
    ok &= expect_int("run.scope_keepout_ok", result.scope_keepout_ok, 1,
                     "contract-only no door/no real assets/no DM1");
    ok &= expect_int("run.d1l_copied_pixels", result.d1l_copied_pixels, 3,
                     "synthetic D1L non-C10 sentinel count");
    ok &= expect_int("run.d1r_copied_pixels", result.d1r_copied_pixels, 3,
                     "synthetic D1R non-C10 sentinel count");
    ok &= expect_u16("run.first_thing_preserved",
                     result.first_thing_after, result.first_thing_before,
                     A_DUNGEON);
    ok &= expect_int("run.map_x_preserved", result.map_x_after,
                     result.map_x_before, A_DUNGEON);
    ok &= expect_int("run.map_y_preserved", result.map_y_after,
                     result.map_y_before, A_DUNGEON);

    return ok;
}

static int test_identity_coordinates_scope(void)
{
    int ok = 1;
    const CSB_V1_D1LD1RWallSpecPc34 *d1l =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(1);
    const CSB_V1_D1LD1RWallSpecPc34 *d1r =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(2);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d1l_d1r_wall_spec_count_pc34(), 2,
                     A_F0128);
    ok &= expect_int("spec.index0.d1l",
                     csb_v1_viewport_d1l_d1r_wall_spec_at_pc34(0) == d1l, 1,
                     A_D1L);
    ok &= expect_int("spec.index1.d1r",
                     csb_v1_viewport_d1l_d1r_wall_spec_at_pc34(1) == d1r, 1,
                     A_D1R);
    ok &= expect_int("spec.index2.null",
                     csb_v1_viewport_d1l_d1r_wall_spec_at_pc34(2) == 0, 1,
                     "D1L/D1R pair has exactly two wall routes");
    ok &= expect_int("spec.unknown.null",
                     csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(99) == 0,
                     1, "D1L/D1R wall-only side ids");
    ok &= expect_int("d1l.function", d1l ? d1l->redmcsb_function_number : 0,
                     122, A_D1L);
    ok &= expect_int("d1r.function", d1r ? d1r->redmcsb_function_number : 0,
                     123, A_D1R);
    ok &= expect_int("d1l.view_square", d1l ? d1l->view_square : -1, 4,
                     "ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L");
    ok &= expect_int("d1r.view_square", d1r ? d1r->view_square : -1, 5,
                     "ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R");
    ok &= expect_int("d1l.depth", d1l ? d1l->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:8524 F0128 depth");
    ok &= expect_int("d1r.depth", d1r ? d1r->view_depth : -1, 1,
                     "ReDMCSB DUNVIEW.C:8528 F0128 depth");
    ok &= expect_int("d1l.lateral", d1l ? d1l->view_lateral : 0, -1,
                     "ReDMCSB DUNVIEW.C:8524 F0128 lateral");
    ok &= expect_int("d1r.lateral", d1r ? d1r->view_lateral : 0, 1,
                     "ReDMCSB DUNVIEW.C:8528 F0128 lateral");
    ok &= expect_int("d1l.wall_zone", d1l ? d1l->wall_zone : -1, 713,
                     "ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L");
    ok &= expect_int("d1r.wall_zone", d1r ? d1r->wall_zone : -1, 714,
                     "ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R");
    ok &= expect_int("zone.d1l",
                     csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(4),
                     713, "709 + M607");
    ok &= expect_int("zone.d1r",
                     csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(5),
                     714, "709 + M608");
    ok &= expect_int("zone.d1c.rejected",
                     csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(3),
                     -1, "D1L/D1R pair excludes D1C gate");
    ok &= expect_int("d1l.no_c17", d1l ? d1l->no_c17_door_ornament : 0, 1,
                     "wall gate has no C17 door ornament");
    ok &= expect_int("d1l.no_door_state", d1l ? d1l->no_door_state_byte : 0, 1,
                     "wall gate has no door-state byte");
    ok &= expect_int("d1l.no_f0111", d1l ? d1l->no_f0111_dispatch : 0, 1,
                     "wall body returns before F0111");
    ok &= expect_int("d1l.no_f0108", d1l ? d1l->no_f0108_floor_ornament_coupling : 0,
                     1, "wall body has no F0108 floor ornament coupling");
    ok &= expect_int("d1l.no_c15_mask", d1l ? d1l->no_c15_destroyed_mask : 0,
                     1, "wall gate has no destroyed-door mask");
    ok &= expect_int("d1l.not_d1l2_d1r2",
                     d1l ? d1l->not_csb_d1l2_d1r2_wall_gate : 0, 1,
                     "non-duplicative of CSB D1L2/D1R2 wall gate");
    ok &= expect_int("d1r.not_dm1",
                     d1r ? d1r->not_dm1_wall_or_stairs_pit_dispatch : 0, 1,
                     "CSB-only lane, no DM1 dispatch");

    return ok;
}

static int test_frames_blits_and_clips(void)
{
    int ok = 1;
    int source_x = -1;
    const CSB_V1_D1LD1RWallSpecPc34 *d1l =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(1);
    const CSB_V1_D1LD1RWallSpecPc34 *d1r =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(2);
    uint8_t source[SOURCE_WIDTH * SOURCE_HEIGHT];
    uint8_t viewport[VIEWPORT_WIDTH * VIEWPORT_HEIGHT];
    CSB_V1_D1LD1RWallBlitStatsPc34 stats;

    ok &= expect_int("d1l.frame_row", d1l ? d1l->frame_row : -1, 7,
                     "ReDMCSB DUNVIEW.C:590 G0163 D1L row");
    ok &= expect_int("d1r.frame_row", d1r ? d1r->frame_row : -1, 8,
                     "ReDMCSB DUNVIEW.C:591 G0163 D1R row");
    ok &= expect_int("d1l.frame.x1", d1l ? d1l->frame_x1 : -1, 0,
                     "ReDMCSB DUNVIEW.C:590 x1");
    ok &= expect_int("d1l.frame.x2", d1l ? d1l->frame_x2 : -1, 63,
                     "ReDMCSB DUNVIEW.C:590 x2");
    ok &= expect_int("d1r.frame.x1", d1r ? d1r->frame_x1 : -1, 160,
                     "ReDMCSB DUNVIEW.C:591 x1");
    ok &= expect_int("d1r.frame.x2", d1r ? d1r->frame_x2 : -1, 223,
                     "ReDMCSB DUNVIEW.C:591 x2");
    ok &= expect_int("d1l.frame.y1", d1l ? d1l->frame_y1 : -1, 9,
                     "ReDMCSB DUNVIEW.C:590 y1");
    ok &= expect_int("d1r.frame.y2", d1r ? d1r->frame_y2 : -1, 119,
                     "ReDMCSB DUNVIEW.C:591 y2");
    ok &= expect_int("d1l.byte_width", d1l ? d1l->frame_byte_width : -1, 128,
                     "ReDMCSB DUNVIEW.C:590 byte width");
    ok &= expect_int("d1r.height", d1r ? d1r->frame_height : -1, 111,
                     "ReDMCSB DUNVIEW.C:591 height");
    ok &= expect_int("screen.width", d1l ? d1l->screen_width : -1, 320,
                     A_CLIP);
    ok &= expect_int("screen.height", d1l ? d1l->screen_height : -1, 200,
                     A_CLIP);
    ok &= expect_int("viewport.width", d1r ? d1r->viewport_width : -1, 224,
                     A_CLIP);
    ok &= expect_int("viewport.height", d1r ? d1r->viewport_height : -1, 136,
                     A_CLIP);
    ok &= expect_int("map.d1l.native",
                     csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
                         d1l, 0, 0, &source_x),
                     0, A_BLIT);
    ok &= expect_int("map.d1l.native.source", source_x, 192, A_BLIT);
    ok &= expect_int("map.d1r.flip",
                     csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
                         d1r, 160, 1, &source_x),
                     0, A_BLIT);
    ok &= expect_int("map.d1r.flip.source", source_x, 63, A_BLIT);
    ok &= expect_int("map.d1r.outside",
                     csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
                         d1r, 159, 0, &source_x),
                     1, "D1R clip does not write x159");

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(0, 192)] = TRANSPARENT;
    source[source_offset(0, 193)] = 0x31u;
    source[source_offset(0, 255)] = 0x7au;
    source[source_offset(110, 192)] = 0x55u;
    ok &= expect_int("d1l.native.copied",
                     csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
                         d1l, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     3, A_BLIT);
    ok &= expect_int("d1l.native.transparent", stats.transparent_pixels,
                     (64 * 111) - 3, A_DEFS);
    ok &= expect_int("d1l.left_edge_transparent_no_write",
                     viewport[viewport_offset(9, 0)], 0xee, A_DEFS);
    ok &= expect_int("d1l.next_pixel", viewport[viewport_offset(9, 1)], 0x31,
                     "synthetic non-C10 D1L source[193]");
    ok &= expect_int("d1l.right_edge", viewport[viewport_offset(9, 63)], 0x7a,
                     A_CLIP);
    ok &= expect_int("d1l.right_neighbor_no_write",
                     viewport[viewport_offset(9, 64)], 0xee, A_CLIP);
    ok &= expect_int("d1l.bottom_edge", viewport[viewport_offset(119, 0)], 0x55,
                     A_CLIP);

    memset(viewport, 0xee, sizeof(viewport));
    ok &= expect_int("d1l.flip.copied",
                     csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
                         d1l, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1, &stats),
                     3, A_BLIT);
    ok &= expect_int("d1l.flip.left_edge", viewport[viewport_offset(9, 0)], 0x7a,
                     "F0105 row-local scratch flip");

    memset(source, TRANSPARENT, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[source_offset(0, 0)] = TRANSPARENT;
    source[source_offset(0, 1)] = 0x61u;
    source[source_offset(0, 63)] = 0x62u;
    source[source_offset(110, 62)] = 0x63u;
    ok &= expect_int("d1r.native.copied",
                     csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
                         d1r, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     3, A_BLIT);
    ok &= expect_int("d1r.left_neighbor_no_write",
                     viewport[viewport_offset(9, 159)], 0xee, A_CLIP);
    ok &= expect_int("d1r.left_edge_transparent_no_write",
                     viewport[viewport_offset(9, 160)], 0xee, A_DEFS);
    ok &= expect_int("d1r.next_pixel", viewport[viewport_offset(9, 161)], 0x61,
                     "synthetic non-C10 D1R source[1]");
    ok &= expect_int("d1r.right_edge", viewport[viewport_offset(9, 223)], 0x62,
                     A_CLIP);
    ok &= expect_int("d1r.flip.copied",
                     csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
                         d1r, source, SOURCE_WIDTH, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 1, &stats),
                     3, A_BLIT);
    ok &= expect_int("d1r.flip.left_edge", viewport[viewport_offset(9, 160)], 0x62,
                     "F0105 row-local scratch flip");
    ok &= expect_int("reject.short_source",
                     csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
                         d1r, source, 63, SOURCE_HEIGHT, viewport,
                         VIEWPORT_WIDTH, VIEWPORT_HEIGHT, 0, &stats),
                     -1, "helper rejects unresolved source clip");
    ok &= expect_int("reject.short_source_flag", stats.rejected, 1,
                     "rejected source leaves no write");

    return ok;
}

static int test_followups_dungeon_identity_and_evidence(void)
{
    int ok = 1;
    int map_x = -1;
    int map_y = -1;
    const char *e = csb_v1_viewport_d1l_d1r_wall_source_evidence_pc34();
    const CSB_V1_D1LD1RWallSpecPc34 *d1l =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(1);
    const CSB_V1_D1LD1RWallSpecPc34 *d1r =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(2);

    ok &= expect_int("d1l.f0107_wall_ornament",
                     d1l ? d1l->f0107_wall_ornament_after_body : -1, 1,
                     "ReDMCSB DUNVIEW.C:7459 F0107 wall ornament after body");
    ok &= expect_int("d1r.f0107_wall_ornament",
                     d1r ? d1r->f0107_wall_ornament_after_body : -1, 1,
                     "ReDMCSB DUNVIEW.C:7627 F0107 wall ornament after body");
    ok &= expect_int("d1l.f0115_keepout",
                     d1l ? d1l->f0115_thing_pass_keepout : -1, 1, A_F0115);
    ok &= expect_int("d1r.f0115_keepout",
                     d1r ? d1r->f0115_thing_pass_keepout : -1, 1, A_F0115);
    ok &= expect_int("d1l.f0128.reset",
                     d1l ? d1l->f0128_resets_map_coordinates_after_side : -1,
                     1, "ReDMCSB DUNVIEW.C:8526-8527");
    ok &= expect_int("d1r.f0128.reset",
                     d1r ? d1r->f0128_resets_map_coordinates_after_side : -1,
                     1, "ReDMCSB DUNVIEW.C:8530-8531");
    ok &= expect_int("d1l.f0128.draws_d1c",
                     d1l ? d1l->f0128_draws_d1c_after_pair : -1, 1,
                     "ReDMCSB DUNVIEW.C:8532-8533");
    ok &= expect_int("d1r.f0127.boundary",
                     d1r ? d1r->f0127_d1c_followup_boundary : -1, 1,
                     "ReDMCSB DUNVIEW.C:8294");
    ok &= expect_int("d1l.f0172.read",
                     d1l ? d1l->f0172_square_aspect_read : -1, 1, A_DUNGEON);
    ok &= expect_int("d1r.f0172.read",
                     d1r ? d1r->f0172_square_aspect_read : -1, 1, A_DUNGEON);
    ok &= expect_int("d1l.no_f0163",
                     d1l ? d1l->f0163_link_thing_keepout : -1, 0, A_DUNGEON);
    ok &= expect_int("d1r.no_f0164",
                     d1r ? d1r->f0164_unlink_thing_keepout : -1, 0, A_DUNGEON);
    ok &= expect_u16("preserve.first_thing",
                     csb_v1_viewport_d1l_d1r_wall_preserve_first_thing_pc34(
                         d1r, 0x4567u),
                     0x4567u, A_DUNGEON);
    ok &= expect_u16("preserve.null_first_thing",
                     csb_v1_viewport_d1l_d1r_wall_preserve_first_thing_pc34(
                         0, 0x4567u),
                     0xffffu, "missing spec rejected");
    ok &= expect_int("preserve.map_identity",
                     csb_v1_viewport_d1l_d1r_wall_preserve_map_identity_pc34(
                         d1l, 12, 34, &map_x, &map_y),
                     0, A_DUNGEON);
    ok &= expect_int("preserve.map_x", map_x, 12, A_DUNGEON);
    ok &= expect_int("preserve.map_y", map_y, 34, A_DUNGEON);
    ok &= expect_int("lineage.d1l.open",
                     d1l ? d1l->lineage_open_row_composition : 0, 1, A_LINEAGE);
    ok &= expect_int("lineage.d1r.door_cross_check",
                     d1r ? d1r->lineage_door_row_cross_check : 0, 1, A_LINEAGE);
    ok &= expect_contains("symbol.d1l.frame", d1l ? d1l->frame_symbol : 0,
                          "{0,63,9,119,128,111,192,0}",
                          "ReDMCSB DUNVIEW.C:590");
    ok &= expect_contains("symbol.d1r.frame", d1r ? d1r->frame_symbol : 0,
                          "{160,223,9,119,128,111,0,0}",
                          "ReDMCSB DUNVIEW.C:591");
    ok &= expect_contains("symbol.d1l.bitmap", d1l ? d1l->bitmap_symbol : 0,
                          "C03_WALL_D1L", A_D1L);
    ok &= expect_contains("symbol.d1r.bitmap", d1r ? d1r->bitmap_symbol : 0,
                          "C02_WALL_D1R", A_D1R);
    ok &= expect_contains("source.d1l", d1l ? d1l->source_lines : 0,
                          "7391-7560", A_D1L);
    ok &= expect_contains("source.d1r", d1r ? d1r->source_lines : 0,
                          "7559-7725", A_D1R);
    ok &= expect_contains("evidence.contract", e, "no real-asset bitmap parity",
                          "contract-only source lock");
    ok &= expect_contains("evidence.no_game_data", e, "no CSB game-data load",
                          "contract-only source lock");
    ok &= expect_contains("evidence.f0122", e, "DUNVIEW.C:7391-7560", A_D1L);
    ok &= expect_contains("evidence.f0123", e, "DUNVIEW.C:7559-7725", A_D1R);
    ok &= expect_contains("evidence.f0104", e, "DUNVIEW.C:3113-3156", A_BLIT);
    ok &= expect_contains("evidence.f0105", e, "DUNVIEW.C:3185-3247", A_BLIT);
    ok &= expect_contains("evidence.f0115", e, "4547-4581", A_F0115);
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8318-8542", A_F0128);
    ok &= expect_contains("evidence.f0127", e, "DUNVIEW.C:8294", A_F0128);
    ok &= expect_contains("evidence.f0163", e, "DUNGEON.C:1769-1838", A_DUNGEON);
    ok &= expect_contains("evidence.f0164", e, "1840-1905", A_DUNGEON);
    ok &= expect_contains("evidence.f0172", e, "2466-2523", A_DUNGEON);
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH", A_DEFS);
    ok &= expect_contains("evidence.c716", e, "C716/C717", A_DEFS);
    ok &= expect_contains("evidence.clip", e, "COORD.C:1713-1722", A_CLIP);
    ok &= expect_contains("evidence.command", e, "COMMAND.C:1126-1127", A_CLIP);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1192-1209",
                          A_LINEAGE);
    ok &= expect_contains("evidence.nonduplicate", e, "D1L2/D1R2", A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1l_d1r_wall_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1l_d1r_wall_source_evidence_pc34());

    ok &= test_run_entry_point();
    ok &= test_identity_coordinates_scope();
    ok &= test_frames_blits_and_clips();
    ok &= test_followups_dungeon_identity_and_evidence();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_30", g_assertions >= 30, 1,
                     "assigned D1L/D1R wall source-lock gate");

    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d1l_d1r_wall_pc34_compat assertions=%d\n",
               g_assertions);
        return 0;
    }
    printf("FAIL csb_v1_viewport_d1l_d1r_wall_pc34_compat assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return 1;
}
