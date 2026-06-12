#include "dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat.h"

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

static void test_spec(void)
{
    const DM1_V1_D3CF0111DoorFrontSpecPc34 *s =
        dm1_v1_viewport_d3c_f0111_door_front_pair_center_pc34();

    CHECK_EQ("count", dm1_v1_viewport_d3c_f0111_door_front_pair_count_pc34(),
             1, "DUNVIEW.C:8498-8499 single D3C center route");
    CHECK_EQ("at0", dm1_v1_viewport_d3c_f0111_door_front_pair_at_pc34(0) == s,
             1, "D3C is the only member of this source-lock pair");
    CHECK_EQ("at1.null", dm1_v1_viewport_d3c_f0111_door_front_pair_at_pc34(1) == NULL,
             1, "bounds guard");
    CHECK_EQ("spec.present", s != NULL, 1, "DUNVIEW.C:6721-6747 D3C route");
    if (!s) return;

    CHECK_EQ("spec.side.center", s->side, 0, "D3C center cell");
    CHECK_EQ("spec.order", s->draw_order_index, 0, "DUNVIEW.C:8498-8499 D3C dispatch");
    CHECK_EQ("spec.view_square", s->view_square_index, 11, "DEFS.H:2607 M600_VIEW_SQUARE_D3C");
    CHECK_EQ("spec.depth", s->relative_depth, 3, "DUNVIEW.C:8498 depth 3");
    CHECK_EQ("spec.lateral", s->relative_lateral, 0, "DUNVIEW.C:8498 lane 0");
    CHECK_EQ("spec.wall_zone", s->wall_zone, 704, "DEFS.H:4044 C704_ZONE_WALL_D3C");
    CHECK_EQ("spec.wall_view", s->wall_ornament_view, 5, "DEFS.H:2701 M578_VIEW_WALL_D3C_FRONT");
    CHECK_EQ("spec.floor_view", s->floor_ornament_view, 3, "DEFS.H:2753 M589_VIEW_FLOOR_D3C");
    CHECK_EQ("spec.floor_slot", s->floor_ornament_slot, 558, "DUNVIEW.C:6722 M558");
    CHECK_EQ("spec.front_wall_slot", s->front_wall_ornament_slot, 552, "DUNVIEW.C:6716 M552");
    CHECK_EQ("spec.door_zone", s->door_zone, 3730, "DEFS.H:4253 M625_ZONE_DOOR_D3C");
    CHECK_EQ("spec.d3l_neighbor_zone", s->door_zone_d3l_neighbor, 3720,
             "DEFS.H:4252 M624_ZONE_DOOR_D3L");
    CHECK_EQ("spec.d3r_neighbor_zone", s->door_zone_d3r_neighbor, 3740,
             "DEFS.H:4254 M626_ZONE_DOOR_D3R");
    CHECK_EQ("spec.d2c_disjoint_zone", s->door_zone_d2c_disjoint, 3760,
             "DEFS.H:4256 M628_ZONE_DOOR_D2C disjoint from D3C");
    CHECK_EQ("spec.d2r_disjoint_zone", s->door_zone_d2r_disjoint, 3770,
             "DEFS.H:4257 M629_ZONE_DOOR_D2R disjoint from D3C");
    CHECK_EQ("spec.door_not_m628", s->door_zone != s->door_zone_d2c_disjoint, 1,
             "DEFS.H:4253/4256 D3C is not M628");
    CHECK_EQ("spec.door_not_m629", s->door_zone != s->door_zone_d2r_disjoint, 1,
             "DEFS.H:4253/4257 D3C is not M629");
    CHECK_EQ("spec.frame_left_zone", s->door_frame_left_zone, 722,
             "DEFS.H:4080 C722_ZONE_DOOR_FRAME_LEFT_D3C");
    CHECK_EQ("spec.frame_right_zone", s->door_frame_right_zone, 723,
             "DEFS.H:4081 C723_ZONE_DOOR_FRAME_RIGHT_D3C");
    CHECK_EQ("spec.frame_bitmap", s->door_frame_bitmap_id, 2119,
             "DUNVIEW.C:148/6734 G2119_DoorFrameLeftD3C");
    CHECK_EQ("spec.bitmap", s->front_bitmap_id, 693,
             "DUNVIEW.C:6744 G0693 front D3LCR bitmap");
    CHECK_EQ("spec.bitmap_symbol",
             strcmp(s->front_bitmap_symbol,
                    "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR"),
             0, "DUNVIEW.C:6744 bitmap symbol");
    CHECK_EQ("spec.ornament_view", s->door_ornament_view, 0,
             "DEFS.H:2789 C0_VIEW_DOOR_ORNAMENT_D3LCR");
    CHECK_EQ("spec.pass1", (int)s->pass1_cell_order, 0x0218,
             "DEFS.H:2669 C0x0218 door pass1");
    CHECK_EQ("spec.pass2", (int)s->pass2_cell_order, 0x0349,
             "DEFS.H:2672 C0x0349 door pass2");
    CHECK_EQ("spec.frame_x_first", s->door_frame_x_first, 88,
             "DUNVIEW.C:624 G0180 closed frame");
    CHECK_EQ("spec.frame_x_last", s->door_frame_x_last, 135,
             "DUNVIEW.C:624 G0180 closed frame");
    CHECK_EQ("spec.frame_y_first", s->door_frame_y_first, 28,
             "DUNVIEW.C:624 G0180 closed frame");
    CHECK_EQ("spec.frame_y_last", s->door_frame_y_last, 67,
             "DUNVIEW.C:624 G0180 closed frame");
    CHECK_EQ("spec.frame_byte_width", s->door_frame_byte_width, 24,
             "DUNVIEW.C:624 G0180 closed frame byte width");
    CHECK_EQ("spec.frame_height", s->door_frame_height, 41,
             "DUNVIEW.C:624 G0180 closed frame height");
    CHECK_EQ("spec.source_x", s->door_source_x_first, 0,
             "DUNVIEW.C:624 G0180 closed source X");
    CHECK_EQ("spec.source_y", s->door_source_y_first, 0,
             "DUNVIEW.C:624 G0180 closed source Y");
    CHECK_EQ("spec.c10", s->transparent_color, 10,
             "DEFS.H:2088 C10_COLOR_FLESH");
    CHECK_EQ("spec.c45", s->c45_preserved_pixel, 45,
             "C45 synthetic preservation sentinel");
    CHECK_EQ("spec.contract", s->source_locked_contract_only ? 1 : 0, 1,
             "contract-only source lock");
    CHECK_EQ("spec.no_assets", s->no_real_asset_bitmap_parity ? 1 : 0, 1,
             "asset-free test");
    CHECK_EQ("spec.no_data", s->no_game_data_load ? 1 : 0, 1,
             "no game data load");
    CHECK_EQ("spec.f0104_anchor", strstr(s->redmcsb_f0104_anchor, "F0104") != NULL,
             1, "source carries F0104 anchor");
    CHECK_EQ("spec.f0105_anchor", strstr(s->redmcsb_f0105_anchor, "F0105") != NULL,
             1, "source carries F0105 anchor");
    CHECK_EQ("spec.f0107_anchor", strstr(s->redmcsb_f0107_anchor, "F0107") != NULL,
             1, "source carries F0107 anchor");
    CHECK_EQ("spec.f0111_anchor", strstr(s->redmcsb_f0111_anchor, "F0111") != NULL,
             1, "source carries F0111 anchor");
    CHECK_EQ("spec.f0115_anchor", strstr(s->redmcsb_f0115_anchor, "F0115") != NULL,
             1, "source carries F0115 anchor");
    CHECK_EQ("spec.f0128_anchor", strstr(s->redmcsb_f0128_anchor, "F0128") != NULL,
             1, "source carries F0128 anchor");
    CHECK_EQ("spec.defs_anchor", strstr(s->redmcsb_defs_anchor, "DEFS.H:") != NULL,
             1, "source carries DEFS.H anchor");
}

static void test_steps_and_orders(void)
{
    DM1_V1_D3CF0111DoorFrontStepInfoPc34 steps[6];
    size_t count = dm1_v1_viewport_d3c_f0111_door_front_pair_steps_pc34(
        steps, sizeof(steps) / sizeof(steps[0]));

    CHECK_EQ("steps.count", count, 6, "DUNVIEW.C:6722-6747 composition");
    CHECK_EQ("steps.0", steps[0].step, DM1_V1_D3C_F0111_STEP_F0108_FLOOR_ORNAMENT_PC34,
             "DUNVIEW.C:6722");
    CHECK_EQ("steps.1", steps[1].step, DM1_V1_D3C_F0111_STEP_F0115_PASS1_PC34,
             "DUNVIEW.C:6723");
    CHECK_EQ("steps.2", steps[2].step, DM1_V1_D3C_F0111_STEP_F0104_LEFT_FRAME_PC34,
             "DUNVIEW.C:6734");
    CHECK_EQ("steps.3", steps[3].step, DM1_V1_D3C_F0111_STEP_F0105_RIGHT_FRAME_FLIPPED_PC34,
             "DUNVIEW.C:6735");
    CHECK_EQ("steps.4", steps[4].step, DM1_V1_D3C_F0111_STEP_F0111_DOOR_FRONT_PC34,
             "DUNVIEW.C:6744");
    CHECK_EQ("steps.5", steps[5].step, DM1_V1_D3C_F0111_STEP_F0115_PASS2_PC34,
             "DUNVIEW.C:6746-6747");
    check_contains("steps.2.anchor", steps[2].redmcsb_anchor, "F0104", "DUNVIEW.C:6734");
    check_contains("steps.3.anchor", steps[3].redmcsb_anchor, "F0105", "DUNVIEW.C:6735");
    check_contains("steps.4.anchor", steps[4].redmcsb_anchor, "M625_ZONE_DOOR_D3C", "DUNVIEW.C:6744");
    CHECK_EQ("decode.pass1.0", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0218, 0),
             0, "DEFS.H:2669 strips door-front marker 8");
    CHECK_EQ("decode.pass1.1", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0218, 1),
             1, "DEFS.H:2669 back-left/back-right");
    CHECK_EQ("decode.pass1.end", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0218, 2),
             -1, "F0115 nibble termination");
    CHECK_EQ("decode.pass2.0", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0349, 0),
             3, "DEFS.H:2672 front-left/front-right");
    CHECK_EQ("decode.pass2.1", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0349, 1),
             2, "DEFS.H:2672 front-left/front-right");
    CHECK_EQ("decode.pass2.end", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0349, 2),
             -1, "F0115 nibble termination");
    CHECK_EQ("decode.d3r.pass1.0", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0128, 0),
             1, "DEFS.H:2668 neighbor order remains disjoint");
    CHECK_EQ("decode.d3r.pass2.0", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0439, 0),
             2, "DEFS.H:2675 neighbor order remains disjoint");
    CHECK_EQ("decode.zero", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0, 0),
             -1, "zero terminates cell order");
    CHECK_EQ("decode.bad_hi", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0218, 4),
             -1, "decode guard");
    CHECK_EQ("decode.bad_lo", dm1_v1_viewport_d3c_f0111_decode_cell_pc34(0x0218, -1),
             -1, "decode guard");
}

static void test_wall_ornament_keepout(void)
{
    const DM1_V1_D3CF0111WallOrnamentPc34 *w =
        dm1_v1_viewport_d3c_f0111_wall_ornament_keepout_pc34();
    DM1_V1_D3CF0111DoorFrontRectPc34 left_neighbor = { 0, 87, 28, 67 };

    CHECK_EQ("wall.present", w != NULL, 1, "DUNVIEW.C F0107:3502-3938");
    if (!w) return;
    CHECK_EQ("wall.ordinal", w->wall_ornament_ordinal, 1, "DUNVIEW.C:3568 non-zero ordinal");
    CHECK_EQ("wall.view", w->wall_ornament_view, 5, "DEFS.H:2701 M578_VIEW_WALL_D3C_FRONT");
    CHECK_EQ("wall.zone", w->zone_index, 1009, "DUNVIEW.C:3586-3587 C1004 + view");
    CHECK_EQ("wall.palette", w->palette_replacement_index_c10, 10,
             "DUNVIEW.C:3502-3938 palette replacement metadata");
    CHECK_EQ("wall.transparent", w->transparent_color, 10,
             "DUNVIEW.C:3922 F0791 C10 wall ornament");
    CHECK_EQ("wall.draw.left", w->draw_rect.left, 74, "DUNVIEW.C:583 G0163 D3C wall frame");
    CHECK_EQ("wall.draw.right", w->draw_rect.right, 149, "DUNVIEW.C:583 G0163 D3C wall frame");
    CHECK_EQ("wall.draw.top", w->draw_rect.top, 25, "DUNVIEW.C:583 G0163 D3C wall frame");
    CHECK_EQ("wall.draw.bottom", w->draw_rect.bottom, 75, "DUNVIEW.C:583 G0163 D3C wall frame");
    CHECK_EQ("wall.keepout.left", w->keepout_rect.left, 88, "DUNVIEW.C:624 G0180 D3C door frame");
    CHECK_EQ("wall.keepout.right", w->keepout_rect.right, 135, "DUNVIEW.C:624 G0180 D3C door frame");
    CHECK_EQ("wall.keepout.top", w->keepout_rect.top, 28, "DUNVIEW.C:624 G0180 D3C door frame");
    CHECK_EQ("wall.keepout.bottom", w->keepout_rect.bottom, 67, "DUNVIEW.C:624 G0180 D3C door frame");
    CHECK_EQ("wall.overlaps_keepout",
             dm1_v1_viewport_d3c_f0111_rects_overlap_pc34(w->draw_rect, w->keepout_rect),
             1, "F0107 wall ornament must stay out of F0111 door-front span");
    CHECK_EQ("wall.left_neighbor_no_overlap",
             dm1_v1_viewport_d3c_f0111_rects_overlap_pc34(left_neighbor, w->keepout_rect),
             0, "keepout boundary is exact");
    CHECK_EQ("wall.allowed_wall_case", w->draw_allowed_in_wall_case ? 1 : 0,
             1, "DUNVIEW.C:6716 wall case calls F0107");
    CHECK_EQ("wall.rejected_door_case", w->rejected_in_door_front_case ? 1 : 0,
             1, "DUNVIEW.C:6721 door case does not call F0107");
    CHECK_EQ("f0107.zero", dm1_v1_viewport_d3c_f0111_f0107_returns_alcove_pc34(0, true),
             0, "DUNVIEW.C:3568/3936 zero ordinal false");
    CHECK_EQ("f0107.not_alcove", dm1_v1_viewport_d3c_f0111_f0107_returns_alcove_pc34(3, false),
             0, "DUNVIEW.C:3589 classifier false");
    CHECK_EQ("f0107.alcove", dm1_v1_viewport_d3c_f0111_f0107_returns_alcove_pc34(3, true),
             1, "DUNVIEW.C:3933 returns alcove flag");
    check_contains("wall.anchor.f0107", w->redmcsb_anchor, "F0107:3502-3938",
                   "wall ornament anchor");
    check_contains("wall.anchor.m578", w->redmcsb_anchor, "M578",
                   "DEFS.H:2701 M578");
}

static void test_pixel_composition(void)
{
    const DM1_V1_D3CF0111DoorFrontSpecPc34 *s =
        dm1_v1_viewport_d3c_f0111_door_front_pair_center_pc34();
    DM1_V1_D3CF0111DoorFrontPixelPc34 p;

    CHECK_EQ("blend.transparent",
             dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(0xaa, 10, 10),
             0xaa, "DEFS.H:2088 C10_COLOR_FLESH preserves destination");
    CHECK_EQ("blend.opaque",
             dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(0xaa, 0xbb, 10),
             0xbb, "F0791 opaque source writes");
    CHECK_EQ("compose.null.out",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 88, 28, 0, 0, 0, 0, 0, 0, 0, NULL),
             0, "null output guard");
    CHECK_EQ("compose.null.spec",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 NULL, 88, 28, 0, 0, 0, 0, 0, 0, 0, &p),
             0, "null spec guard");
    CHECK_EQ("compose.apply",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 88, 28, 0x11, 0x21, 0x31, 0x41, 0x42, 0x51, 0x61, &p),
             1, "DUNVIEW.C:6722-6747 composition applies");
    CHECK_EQ("compose.in_frame", p.in_frame ? 1 : 0, 1, "DUNVIEW.C:624 frame");
    CHECK_EQ("compose.after_floor", p.after_floor, 0x21, "DUNVIEW.C:6722 F0108");
    CHECK_EQ("compose.after_pass1", p.after_pass1, 0x31, "DUNVIEW.C:6723 F0115 pass1");
    CHECK_EQ("compose.after_left", p.after_left_frame, 0x41, "DUNVIEW.C:6734 F0104");
    CHECK_EQ("compose.after_right", p.after_right_frame, 0x42, "DUNVIEW.C:6735 F0105");
    CHECK_EQ("compose.after_door", p.after_door, 0x51, "DUNVIEW.C:6744 F0111");
    CHECK_EQ("compose.after_pass2", p.after_pass2, 0x61, "DUNVIEW.C:6746-6747 F0115 pass2");
    CHECK_EQ("compose.c10.apply",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 88, 28, 0x66, 10, 10, 10, 10, 10, 10, &p),
             1, "F0791 C10 transparent route");
    CHECK_EQ("compose.c10.floor", p.floor_transparent ? 1 : 0, 1, "F0108 C10");
    CHECK_EQ("compose.c10.pass1", p.pass1_transparent ? 1 : 0, 1, "F0115 pass1 C10");
    CHECK_EQ("compose.c10.left", p.left_frame_transparent ? 1 : 0, 1, "F0104 C10");
    CHECK_EQ("compose.c10.right", p.right_frame_transparent ? 1 : 0, 1, "F0105 C10");
    CHECK_EQ("compose.c10.door", p.door_transparent ? 1 : 0, 1, "F0111 C10");
    CHECK_EQ("compose.c10.pass2", p.pass2_transparent ? 1 : 0, 1, "F0115 pass2 C10");
    CHECK_EQ("compose.c10.final", p.after_pass2, 0x66, "transparent layers preserve destination");
    CHECK_EQ("compose.c45.apply",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 88, 28, 0x66, 10, 10, 10, 10, 45, 10, &p),
             1, "C45 preservation sentinel");
    CHECK_EQ("compose.c45.not_transparent", p.door_transparent ? 1 : 0, 0,
             "C45 is not C10 transparent");
    CHECK_EQ("compose.c45.final", p.after_pass2, 45,
             "C45 opaque door pixel survives transparent pass2");
    CHECK_EQ("compose.outside.left",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 87, 28, 0x77, 0x21, 0x31, 0x41, 0x42, 0x51, 0x61, &p),
             1, "outside frame left");
    CHECK_EQ("compose.outside.left.flag", p.outside_untouched ? 1 : 0,
             1, "outside frame has no write metadata");
    CHECK_EQ("compose.outside.left.final", p.after_pass2, 0x77,
             "outside left preserves destination");
    CHECK_EQ("compose.outside.right",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 136, 28, 0x78, 0x21, 0x31, 0x41, 0x42, 0x51, 0x61, &p),
             1, "outside frame right");
    CHECK_EQ("compose.outside.right.final", p.after_pass2, 0x78,
             "outside right preserves destination");
    CHECK_EQ("compose.outside.top",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 88, 27, 0x79, 0x21, 0x31, 0x41, 0x42, 0x51, 0x61, &p),
             1, "outside frame top");
    CHECK_EQ("compose.outside.top.final", p.after_pass2, 0x79,
             "outside top preserves destination");
    CHECK_EQ("compose.outside.bottom",
             dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
                 s, 88, 68, 0x7a, 0x21, 0x31, 0x41, 0x42, 0x51, 0x61, &p),
             1, "outside frame bottom");
    CHECK_EQ("compose.outside.bottom.final", p.after_pass2, 0x7a,
             "outside bottom preserves destination");
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_d3c_f0111_door_front_pair_source_evidence_pc34();

    check_contains("evidence.f0118", e, "DUNVIEW.C F0118:6721-6747",
                   "D3C composition route");
    check_contains("evidence.f0111", e, "DUNVIEW.C F0111:4218-4337",
                   "mandatory F0111 anchor");
    check_contains("evidence.f0111.path", e, "4243-4266",
                   "F0111 copy/ornament path");
    check_contains("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                   "mandatory F0107 anchor");
    check_contains("evidence.f0104", e, "DUNVIEW.C F0104:3113-3156",
                   "mandatory F0104 anchor");
    check_contains("evidence.f0105", e, "F0105:3185-3247",
                   "mandatory F0105 anchor");
    check_contains("evidence.f0115", e, "DUNVIEW.C F0115:4547-4581",
                   "mandatory F0115 anchor");
    check_contains("evidence.f0128", e, "F0128:8478-8508",
                   "mandatory F0128 anchor");
    check_contains("evidence.f0128.tail", e, "8534-8542",
                   "mandatory F0128 tail anchor");
    check_contains("evidence.c10", e, "DEFS.H:2088",
                   "C10 transparency anchor");
    check_contains("evidence.cell_orders", e, "DEFS.H:2668-2677",
                   "cell order anchors");
    check_contains("evidence.m575", e, "DEFS.H:2698-2702",
                   "M575..M579 anchors");
    check_contains("evidence.c705", e, "C705/C706",
                   "adjacent wall-zone anchors");
    check_contains("evidence.m624", e, "M624_ZONE_DOOR_D3L",
                   "door-zone family anchor");
    check_contains("evidence.m625", e, "M625_ZONE_DOOR_D3C=3730",
                   "source-correct D3C door-zone anchor");
    check_contains("evidence.m628", e, "M628_ZONE_DOOR_D2C",
                   "M628 disjoint-zone guard");
    check_contains("evidence.m629", e, "M629_ZONE_DOOR_D2R",
                   "M629 disjoint-zone guard");
}

static void test_hash(void)
{
    CHECK_EQ("hash", (int)dm1_v1_viewport_d3c_f0111_door_front_pair_hash_pc34(),
             (int)0xae1f9a60U, "deterministic D3C F0111/F0107 source-lock hash");
}

int main(void)
{
    test_spec();
    test_steps_and_orders();
    test_wall_ornament_keepout();
    test_pixel_composition();
    test_source_evidence();
    test_hash();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat assertions=%d failures=%d hash=0x%08x\n",
               g_assertions, g_failures,
               dm1_v1_viewport_d3c_f0111_door_front_pair_hash_pc34());
        return 1;
    }
    printf("PASS dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat assertions=%d failures=0 hash=0x%08x\n",
           g_assertions,
           dm1_v1_viewport_d3c_f0111_door_front_pair_hash_pc34());
    return 0;
}
