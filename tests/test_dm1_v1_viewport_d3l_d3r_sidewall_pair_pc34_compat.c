#include "dm1_v1_viewport_d3l_d3r_sidewall_pair_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

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

static void check_spec_one(const DM1_V1_D3LD3RSidewallSpecPc34 *s,
                           int side,
                           int order,
                           int view_square,
                           int lateral,
                           int wall_zone,
                           int wall_bitmap,
                           int flipped_bitmap,
                           int side_view,
                           int front_view,
                           int door_zone,
                           int pass1,
                           int pass2,
                           int item_row,
                           int explosion_row,
                           int x_first,
                           int x_last,
                           int source_x)
{
    CHECK_EQ("spec.present", s != NULL, 1,
             "DUNVIEW.C:6361-6480/6500-6622 D3L/D3R helpers");
    if (!s) return;

    CHECK_EQ("spec.side", s->side, side,
             "DUNVIEW.C F0116/F0117 side identity");
    CHECK_EQ("spec.draw_order", s->draw_order_index, order,
             "DUNVIEW.C:6432-6600 D3L before D3R within pair");
    CHECK_EQ("spec.view_square", s->view_square_index, view_square,
             "DEFS.H:2608-2609 M601/M602");
    CHECK_EQ("spec.depth", s->relative_depth, 3,
             "DUNVIEW.C D3 side squares are depth 3");
    CHECK_EQ("spec.lateral", s->relative_lateral, lateral,
             "DUNVIEW.C D3L/D3R lateral offsets");
    CHECK_EQ("spec.wall_zone", s->wall_zone, wall_zone,
             "DEFS.H:4045-4046 C705/C706");
    CHECK_EQ("spec.wall_bitmap", s->wall_bitmap, wall_bitmap,
             "DEFS.H:3435-3436 C12/C13 are D3R/D3L");
    CHECK_EQ("spec.flipped_bitmap", s->flipped_wall_bitmap, flipped_bitmap,
             "DUNVIEW.C:6423/6555 flipped wall route");
    CHECK_EQ("spec.side_view", s->side_wall_ornament_view, side_view,
             "DEFS.H:2698-2699 M575/M576");
    CHECK_EQ("spec.front_view", s->front_wall_ornament_view, front_view,
             "DEFS.H:2700/2702 M577/M579");
    CHECK_EQ("spec.center_front_view", s->center_front_view, 5,
             "DEFS.H:2701 M578_VIEW_WALL_D3C_FRONT");
    CHECK_EQ("spec.door_zone", s->door_zone, door_zone,
             "DEFS.H:4252-4254 M624/M626 PC34 zones");
    CHECK_EQ("spec.front_bitmap_id", s->door_front_bitmap_id, 693,
             "DUNVIEW.C:6457/6599 G0693 front door bitmap array");
    CHECK_EQ("spec.front_bitmap_symbol",
             strcmp(s->door_front_bitmap_symbol,
                    "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR"),
             0, "DUNVIEW.C:6457/6599 front bitmap symbol");
    CHECK_EQ("spec.door_ornament_view", s->door_ornament_view, 0,
             "DUNVIEW.C:6457/6599 C0_VIEW_DOOR_ORNAMENT_D3LCR");
    CHECK_EQ("spec.pass1_order", s->f0115_door_pass1_cell_order, pass1,
             "DUNVIEW.C:6448/6584 F0115 pass1");
    CHECK_EQ("spec.pass2_order", s->f0115_door_pass2_cell_order, pass2,
             "DUNVIEW.C:6458/6600 F0115 pass2");
    CHECK_EQ("spec.alcove_order", s->f0115_alcove_cell_order, 0,
             "DUNVIEW.C:6433-6435/6569-6571 alcove order");
    CHECK_EQ("spec.f0115_square", s->f0115_view_square, view_square,
             "DUNVIEW.C:6448/6480/6584/6622 F0115 view square");
    CHECK_EQ("spec.item_row", s->thing_item_row, item_row,
             "DUNVIEW.C:373 G2028[M601/M602]");
    CHECK_EQ("spec.creature_row", s->creature_row, item_row,
             "DUNVIEW.C:375 G2033[M601/M602]");
    CHECK_EQ("spec.explosion_row", s->explosion_row, explosion_row,
             "DUNVIEW.C:376 G2034[M601/M602]");
    CHECK_EQ("spec.frame_x_first", s->frame_x_first, x_first,
             "DUNVIEW.C:581-585 G0163 frame");
    CHECK_EQ("spec.frame_x_last", s->frame_x_last, x_last,
             "DUNVIEW.C:581-585 clipped visible frame");
    CHECK_EQ("spec.frame_y_first", s->frame_y_first, 25,
             "DUNVIEW.C:581-585 G0163 Y1");
    CHECK_EQ("spec.frame_y_last", s->frame_y_last, 75,
             "DUNVIEW.C:581-585 G0163 Y2");
    CHECK_EQ("spec.source_x", s->source_x_first, source_x,
             "DUNVIEW.C:581-585 G0163 source X");
    CHECK_EQ("spec.source_y", s->source_y_first, 0,
             "DUNVIEW.C:581-585 G0163 source Y");
    CHECK_EQ("spec.c10", s->transparent_color, 10,
             "DEFS.H:2088 C10_COLOR_FLESH");
    CHECK_EQ("spec.contract_only", s->source_locked_contract_only ? 1 : 0, 1,
             "contract-only source lock");
    CHECK_EQ("spec.no_asset_parity", s->no_real_asset_bitmap_parity ? 1 : 0, 1,
             "no real asset bitmap parity");
    CHECK_EQ("spec.no_game_data", s->no_game_data_load ? 1 : 0, 1,
             "no game data load");
    CHECK_EQ("spec.wall_anchor", strstr(s->redmcsb_wall_anchor, "DUNVIEW.C:") != NULL,
             1, "fixture carries ReDMCSB wall anchor");
    CHECK_EQ("spec.f0107_anchor", strstr(s->redmcsb_f0107_anchor, "F0107") != NULL,
             1, "fixture carries ReDMCSB F0107 anchor");
    CHECK_EQ("spec.f0111_anchor", strstr(s->redmcsb_f0111_anchor, "F0111") != NULL,
             1, "fixture carries ReDMCSB F0111 anchor");
    CHECK_EQ("spec.f0115_anchor", strstr(s->redmcsb_f0115_anchor, "F0115") != NULL,
             1, "fixture carries ReDMCSB F0115 anchor");
}

static void test_specs(void)
{
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3l =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(1);
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3r =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(2);

    CHECK_EQ("count", dm1_v1_viewport_d3l_d3r_sidewall_pair_count_pc34(), 2,
             "only D3L/D3R pair is covered");
    CHECK_EQ("at0", dm1_v1_viewport_d3l_d3r_sidewall_pair_at_pc34(0) == d3l,
             1, "D3L fixture first");
    CHECK_EQ("at1", dm1_v1_viewport_d3l_d3r_sidewall_pair_at_pc34(1) == d3r,
             1, "D3R fixture second");
    CHECK_EQ("at2.null", dm1_v1_viewport_d3l_d3r_sidewall_pair_at_pc34(2) == NULL,
             1, "fixture bounds");
    CHECK_EQ("bad_side.null",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(3) == NULL,
             1, "D3L2/D3R2 are not in this gate");

    check_spec_one(d3l, 1, 0, 12, -1, 705, 13, 12, 2, 4, 3720,
                   0x0218, 0x0349, 1, 4, 0, 31, 32);
    check_spec_one(d3r, 2, 1, 13, 1, 706, 12, 13, 3, 6, 3740,
                   0x0128, 0x0439, 2, 5, 139, 202, 0);
}

static void test_f0107_wall_ornament_alcove_returns(void)
{
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3l =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(1);
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3r =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(2);

    CHECK_EQ("f0107.zero.false",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(0, true),
             0, "DUNVIEW.C:3584/3937 ordinal zero returns C0_FALSE");
    CHECK_EQ("f0107.nonalcove.false",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(7, false),
             0, "DUNVIEW.C:3636 F0149 alcove classifier");
    CHECK_EQ("f0107.alcove.true",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(7, true),
             1, "DUNVIEW.C:3933 returns L0096_B_IsAlcove");
    CHECK_EQ("d3l.side_ornament_view", d3l ? d3l->side_wall_ornament_view : -1, 2,
             "DUNVIEW.C:6432 M575_VIEW_WALL_D3L_RIGHT");
    CHECK_EQ("d3l.front_ornament_view", d3l ? d3l->front_wall_ornament_view : -1, 4,
             "DUNVIEW.C:6433 M577_VIEW_WALL_D3L_FRONT");
    CHECK_EQ("d3r.side_ornament_view", d3r ? d3r->side_wall_ornament_view : -1, 3,
             "DUNVIEW.C:6568 M576_VIEW_WALL_D3R_LEFT");
    CHECK_EQ("d3r.front_ornament_view", d3r ? d3r->front_wall_ornament_view : -1, 6,
             "DUNVIEW.C:6569 M579_VIEW_WALL_D3R_FRONT");
    CHECK_EQ("d3l.front_alcove_enters_f0115",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(11, true),
             1, "DUNVIEW.C:6433-6435 front alcove goto T0116017");
    CHECK_EQ("d3r.front_alcove_enters_f0115",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(12, true),
             1, "DUNVIEW.C:6569-6571 front alcove goto T0117018");
    CHECK_EQ("d3l.side_return_ignored_but_true",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(13, true),
             1, "DUNVIEW.C:6432 side ornament return ignored");
    CHECK_EQ("d3r.side_return_ignored_but_true",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(14, true),
             1, "DUNVIEW.C:6568 side ornament return ignored");
}

static void test_f0111_f0115_routes(void)
{
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3l =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(1);
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3r =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(2);

    CHECK_EQ("f0111.d3l.zone",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_door_zone_pc34(d3l),
             3720, "DEFS.H:4252 M624_ZONE_DOOR_D3L PC34");
    CHECK_EQ("f0111.d3r.zone",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_door_zone_pc34(d3r),
             3740, "DEFS.H:4254 M626_ZONE_DOOR_D3R PC34");
    CHECK_EQ("f0111.null.zone",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_door_zone_pc34(NULL),
             -1, "invalid door route guard");
    CHECK_EQ("f0111.d3l.front_bitmap",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_front_bitmap_pc34(d3l),
             693, "DUNVIEW.C:6457 G0693 front bitmap");
    CHECK_EQ("f0111.d3r.front_bitmap",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_front_bitmap_pc34(d3r),
             693, "DUNVIEW.C:6599 G0693 front bitmap");
    CHECK_EQ("f0115.d3l.square",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0115_view_square_pc34(d3l),
             12, "DUNVIEW.C:6448/6480 M601_VIEW_SQUARE_D3L");
    CHECK_EQ("f0115.d3r.square",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0115_view_square_pc34(d3r),
             13, "DUNVIEW.C:6584/6622 M602_VIEW_SQUARE_D3R");
    CHECK_EQ("f0115.null.square",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_f0115_view_square_pc34(NULL),
             -1, "invalid F0115 route guard");

    CHECK_EQ("decode.d3l.pass1.first",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0218, 0),
             0, "DEFS.H:2669 C0x0218 door pass1");
    CHECK_EQ("decode.d3l.pass1.second",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0218, 1),
             1, "DEFS.H:2669 C0x0218 door pass1");
    CHECK_EQ("decode.d3l.pass1.third",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0218, 2),
             -1, "DUNVIEW.C:4561-4564 zero terminates order");
    CHECK_EQ("decode.d3r.pass1.first",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0128, 0),
             1, "DEFS.H:2668 C0x0128 door pass1");
    CHECK_EQ("decode.d3r.pass1.second",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0128, 1),
             0, "DEFS.H:2668 C0x0128 door pass1");
    CHECK_EQ("decode.d3l.pass2.first",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0349, 0),
             3, "DEFS.H:2672 C0x0349 door pass2");
    CHECK_EQ("decode.d3l.pass2.second",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0349, 1),
             2, "DEFS.H:2672 C0x0349 door pass2");
    CHECK_EQ("decode.d3r.pass2.first",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0439, 0),
             2, "DEFS.H:2675 C0x0439 door pass2");
    CHECK_EQ("decode.d3r.pass2.second",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0439, 1),
             3, "DEFS.H:2675 C0x0439 door pass2");
    CHECK_EQ("decode.bad.index",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0218, 4),
             -1, "decode guard");
    CHECK_EQ("decode.bad.negative",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(0x0218, -1),
             -1, "decode guard");
}

static void check_composition_one(const DM1_V1_D3LD3RSidewallSpecPc34 *s,
                                  int x,
                                  int outside_x)
{
    DM1_V1_D3LD3RSidewallPixelPc34 p;

    CHECK_EQ("compose.order.apply",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
                 s, x, 25, 0x11, 0x21, 0x31, 0x41, 0x51, &p),
             1, "wall -> F0107 -> F0111 -> F0115 pixel order");
    CHECK_EQ("compose.order.in_clip", p.in_clip ? 1 : 0, 1,
             "DUNVIEW.C:581-585 frame clip");
    CHECK_EQ("compose.order.after_wall", p.after_wall, 0x21,
             "first wall layer writes");
    CHECK_EQ("compose.order.after_ornament", p.after_ornament, 0x31,
             "F0107 ornament follows wall");
    CHECK_EQ("compose.order.after_door", p.after_door, 0x41,
             "F0111 door-front follows ornament");
    CHECK_EQ("compose.order.after_thing", p.after_thing, 0x51,
             "F0115 thing pass follows door-front");

    CHECK_EQ("compose.c10.first.apply",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
                 s, x, 25, 0x66, 10, 10, 10, 10, &p),
             1, "DEFS.H:2088 C10 transparent first pixel");
    CHECK_EQ("compose.c10.wall_transparent", p.wall_transparent ? 1 : 0, 1,
             "F0100/F0104/F0105 C10 transparency");
    CHECK_EQ("compose.c10.final", p.after_thing, 0x66,
             "all C10 layers preserve destination");

    CHECK_EQ("compose.opaque.after_c10.apply",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
                 s, x, 25, 0x66, 10, 0x32, 10, 0x52, &p),
             1, "transparent layers do not block later opaque layers");
    CHECK_EQ("compose.opaque.after_wall", p.after_wall, 0x66,
             "C10 wall preserves before ornament");
    CHECK_EQ("compose.opaque.after_ornament", p.after_ornament, 0x32,
             "F0107 opaque ornament writes");
    CHECK_EQ("compose.opaque.after_door", p.after_door, 0x32,
             "C10 door-front preserves ornament");
    CHECK_EQ("compose.opaque.after_thing", p.after_thing, 0x52,
             "F0115 opaque thing writes last");

    CHECK_EQ("compose.nonoverlap.apply",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
                 s, outside_x, 25, 0x77, 0x21, 0x31, 0x41, 0x51, &p),
             1, "opaque non-overlap with neighbor wall pixels");
    CHECK_EQ("compose.nonoverlap.no_write", p.no_write_metadata ? 1 : 0, 1,
             "outside D3L/D3R side-wall clip");
    CHECK_EQ("compose.nonoverlap.final", p.after_thing, 0x77,
             "neighbor wall pixel stays unchanged");

    CHECK_EQ("compose.yclip.apply",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
                 s, x, 76, 0x78, 0x21, 0x31, 0x41, 0x51, &p),
             1, "bottom outside D3L/D3R side-wall clip");
    CHECK_EQ("compose.yclip.final", p.after_thing, 0x78,
             "outside y clip stays unchanged");
}

static void test_pixel_composition(void)
{
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3l =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(1);
    const DM1_V1_D3LD3RSidewallSpecPc34 *d3r =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(2);
    DM1_V1_D3LD3RSidewallPixelPc34 p;

    CHECK_EQ("blend.transparent",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(0xaa, 10, 10),
             0xaa, "DEFS.H:2088 C10_COLOR_FLESH");
    CHECK_EQ("blend.opaque",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(0xaa, 0xbb, 10),
             0xbb, "opaque source pixel writes");
    CHECK_EQ("compose.null.out",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
                 d3l, 0, 25, 0, 0, 0, 0, 0, NULL),
             0, "null output guard");
    CHECK_EQ("compose.null.spec",
             dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
                 NULL, 0, 25, 0, 0, 0, 0, 0, &p),
             0, "null spec guard");
    check_composition_one(d3l, 0, 32);
    check_composition_one(d3r, 139, 138);
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_source_evidence_pc34();
    const char *c =
        dm1_v1_viewport_d3l_d3r_sidewall_pair_csb_lineage_viewport_cpp_evidence_pc34;

    check_contains("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                   "mandatory ReDMCSB F0107 anchor");
    check_contains("evidence.f0111", e, "DUNVIEW.C F0111:4218-4337",
                   "mandatory ReDMCSB F0111 anchor");
    check_contains("evidence.f0115", e, "DUNVIEW.C F0115:4547-4581,5668-5671",
                   "mandatory ReDMCSB F0115 anchor");
    check_contains("evidence.f0116", e, "DUNVIEW.C F0116:6361-6480",
                   "mandatory ReDMCSB F0116 anchor");
    check_contains("evidence.f0117", e, "F0117:6500-6622",
                   "mandatory ReDMCSB F0117 anchor");
    check_contains("evidence.route", e, "DUNVIEW.C:6432-6600",
                   "mandatory D3L/D3R route anchor");
    check_contains("evidence.c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                   "mandatory C10 anchor");
    check_contains("evidence.d3l_d3r", e, "DEFS.H:2608-2609",
                   "mandatory M601/M602 anchor");
    check_contains("evidence.orders", e, "DEFS.H:2668-2677 and 2698-2702",
                   "mandatory order and wall-view anchors");
    check_contains("evidence.zones", e, "DEFS.H:4045-4046 C705/C706",
                   "mandatory wall zone anchors");
    check_contains("evidence.doors", e, "M624_ZONE_DOOR_D3L and M626_ZONE_DOOR_D3R",
                   "F0111 door zone anchors");
    check_contains("csb.1192", c, "Viewport.cpp:1192-1209 quotes:",
                   "CSB-lineage FOV cross-reference");
    check_contains("csb.1903", c, "Viewport.cpp:1903-1915 quotes:",
                   "CSB-lineage door-facing cross-reference");
    check_contains("csb.draworder218", c, "DrawOrder218",
                   "CSB-lineage F0115 pass1 order");
    check_contains("csb.draworder349", c, "DrawOrder349",
                   "CSB-lineage F0115 pass2 order");
    CHECK_EQ("evidence.csb.not_empty", c[0] != '\0', 1,
             "CSB evidence constant is linked");
}

int main(void)
{
    test_specs();
    test_f0107_wall_ornament_alcove_returns();
    test_f0111_f0115_routes();
    test_pixel_composition();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d3l_d3r_sidewall_pair_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d3l_d3r_sidewall_pair_pc34_compat assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
