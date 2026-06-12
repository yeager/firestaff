#include "dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat.h"

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
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *s,
    int side,
    int order,
    int requested_square,
    int f0111_square,
    int lateral,
    int side_view,
    int front_view,
    int floor_view,
    int door_zone,
    int frame_zone,
    unsigned int pass1,
    unsigned int pass2,
    int x_first,
    int x_last)
{
    CHECK_EQ("spec.present", s != NULL, 1,
             "DUNVIEW.C:6987-7004/7180-7197 F0111 route");
    if (!s) return;

    CHECK_EQ("spec.side", s->side, side, "pair side identity");
    CHECK_EQ("spec.order", s->draw_order_index, order,
             "DUNVIEW.C:8512-8517 D2L before D2R after D2L2/D2R2 guards");
    CHECK_EQ("spec.requested_square", s->requested_view_square_index,
             requested_square, "DEFS.H:2605-2606 C09/C10 D2L2/D2R2");
    CHECK_EQ("spec.f0111_square", s->f0111_view_square_index, f0111_square,
             "DEFS.H:2603-2604 M604/M605 D2L/D2R own F0111");
    CHECK_EQ("spec.requested_not_route", s->requested_square_has_f0111_route, 0,
             "DUNVIEW.C:6837-6896 C09/C10 have no F0111 case");
    CHECK_EQ("spec.depth", s->relative_depth, 2, "D2 row depth");
    CHECK_EQ("spec.lateral", s->relative_lateral, lateral, "D2 lateral pair");
    CHECK_EQ("spec.f0107_side", s->f0107_side_wall_view, side_view,
             "DUNVIEW.C:6968/7119 side ornament call");
    CHECK_EQ("spec.f0107_front", s->f0107_front_wall_view, front_view,
             "DUNVIEW.C:6969/7120 front alcove gate");
    CHECK_EQ("spec.f0108_floor", s->f0108_floor_view, floor_view,
             "DUNVIEW.C:6988/7181 F0108 before F0115 pass1");
    CHECK_EQ("spec.f0108_slot", s->f0108_ornament_aspect_slot, 558,
             "DUNVIEW.C:6988/7181 M558 floor ornament ordinal");
    CHECK_EQ("spec.door_zone", s->f0111_door_zone, door_zone,
             "DEFS.H:4255/4257 M627/M629");
    CHECK_EQ("spec.frame_zone", s->f0111_door_frame_top_zone, frame_zone,
             "DEFS.H:4087-4089/4239-4257 PC34 door-frame zone");
    CHECK_EQ("spec.bitmap", s->f0111_front_bitmap_id, 694,
             "DUNVIEW.C:6992/7185 G0694 front D2LCR bitmap");
    CHECK_EQ("spec.bitmap_symbol",
             strcmp(s->f0111_front_bitmap_symbol,
                    "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR"),
             0, "DUNVIEW.C:6992/7185 bitmap symbol");
    CHECK_EQ("spec.ornament_view", s->f0111_door_ornament_view, 1,
             "DEFS.H:2790 C1_VIEW_DOOR_ORNAMENT_D2LCR");
    CHECK_EQ("spec.pass1", (int)s->f0115_pass1_cell_order, (int)pass1,
             "DEFS.H:2668-2669 door pass1 orders");
    CHECK_EQ("spec.pass2", (int)s->f0115_pass2_cell_order, (int)pass2,
             "DEFS.H:2672/2675 door pass2 orders");
    CHECK_EQ("spec.frame_x_first", s->door_frame_x_first, x_first,
             "DUNVIEW.C:646-681 G0182/G0184 closed frames");
    CHECK_EQ("spec.frame_x_last", s->door_frame_x_last, x_last,
             "DUNVIEW.C:646-681 G0182/G0184 closed frames");
    CHECK_EQ("spec.frame_y_first", s->door_frame_y_first, 24,
             "DUNVIEW.C:646-681 closed frame Y1");
    CHECK_EQ("spec.frame_y_last", s->door_frame_y_last, 82,
             "DUNVIEW.C:646-681 closed frame Y2");
    CHECK_EQ("spec.frame_byte_width", s->door_frame_byte_width, 32,
             "DUNVIEW.C:646-681 closed frame byte width");
    CHECK_EQ("spec.frame_height", s->door_frame_height, 61,
             "DUNVIEW.C:646-681 closed frame height");
    CHECK_EQ("spec.source_x", s->door_source_x_first, 0,
             "DUNVIEW.C:646-681 closed frame source X");
    CHECK_EQ("spec.source_y", s->door_source_y_first, 0,
             "DUNVIEW.C:646-681 closed frame source Y");
    CHECK_EQ("spec.c10", s->transparent_color, 10,
             "DUNVIEW.C:4218-4337 F0111 C10 blit transparency");
    CHECK_EQ("spec.d2l2_no_route", s->d2l2_no_f0107_f0108_f0111_route, 1,
             "DUNVIEW.C:6837-6865 D2L2 route excludes F0107/F0108/F0111");
    CHECK_EQ("spec.d2r2_no_route", s->d2r2_no_f0107_f0108_f0111_route, 1,
             "DUNVIEW.C:6868-6896 D2R2 route excludes F0107/F0108/F0111");
    CHECK_EQ("spec.d2l2_guard_square", s->d2l2_no_f0111_route_square_index, 9,
             "DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    CHECK_EQ("spec.d2r2_guard_square", s->d2r2_no_f0111_route_square_index, 10,
             "DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    CHECK_EQ("spec.contract", s->source_locked_contract_only ? 1 : 0, 1,
             "contract-only source lock");
    CHECK_EQ("spec.no_assets", s->no_real_asset_bitmap_parity ? 1 : 0, 1,
             "asset-free test");
    CHECK_EQ("spec.no_data", s->no_game_data_load ? 1 : 0, 1,
             "no game data load");
    CHECK_EQ("spec.f0107_anchor", strstr(s->redmcsb_f0107_anchor, "F0107") != NULL,
             1, "source comment carries F0107 anchor");
    CHECK_EQ("spec.f0108_anchor", strstr(s->redmcsb_f0108_anchor, "F0108") != NULL,
             1, "source comment carries F0108 anchor");
    CHECK_EQ("spec.f0111_anchor", strstr(s->redmcsb_f0111_anchor, "F0111") != NULL,
             1, "source comment carries F0111 anchor");
    CHECK_EQ("spec.defs_anchor", strstr(s->redmcsb_defs_anchor, "DEFS.H:") != NULL,
             1, "source comment carries DEFS.H anchor");
    CHECK_EQ("spec.dispatch_anchor", strstr(s->redmcsb_dispatch_anchor, "DUNVIEW.C:") != NULL,
             1, "source comment carries dispatch anchor");
}

static void test_specs(void)
{
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *d2l =
        dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_for_side_pc34(1);
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *d2r =
        dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_for_side_pc34(2);

    CHECK_EQ("count", dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_count_pc34(),
             2, "single D2 front-door left/right pair");
    CHECK_EQ("at0", dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_at_pc34(0) == d2l,
             1, "left first");
    CHECK_EQ("at1", dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_at_pc34(1) == d2r,
             1, "right second");
    CHECK_EQ("at2.null", dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_at_pc34(2) == NULL,
             1, "bounds");
    CHECK_EQ("bad_side.null",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_for_side_pc34(3) == NULL,
             1, "no D3L/D3R side-wall duplicate");

    check_spec_one(d2l, 1, 0, 9, 7, -1, 7, 9, 5, 3750, 729,
                   0x0218, 0x0349, 0, 63);
    check_spec_one(d2r, 2, 1, 10, 8, 1, 8, 11, 7, 3770, 731,
                   0x0128, 0x0439, 160, 223);
}

static void test_f0107_and_orders(void)
{
    CHECK_EQ("f0107.zero",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_f0107_returns_alcove_pc34(0, true),
             0, "DUNVIEW.C:3568/3936 zero ordinal false");
    CHECK_EQ("f0107.not_alcove",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_f0107_returns_alcove_pc34(4, false),
             0, "DUNVIEW.C:3589 classifier false");
    CHECK_EQ("f0107.alcove",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_f0107_returns_alcove_pc34(4, true),
             1, "DUNVIEW.C:3933 returns alcove flag");

    CHECK_EQ("decode.d2l.pass1.0",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0218, 0),
             0, "DEFS.H:2669 strips door-front marker 8");
    CHECK_EQ("decode.d2l.pass1.1",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0218, 1),
             1, "DEFS.H:2669 back-left/back-right");
    CHECK_EQ("decode.d2r.pass1.0",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0128, 0),
             1, "DEFS.H:2668 back-right/back-left");
    CHECK_EQ("decode.d2r.pass1.1",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0128, 1),
             0, "DEFS.H:2668 back-right/back-left");
    CHECK_EQ("decode.d2l.pass2.0",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0349, 0),
             3, "DEFS.H:2672 front-left/front-right");
    CHECK_EQ("decode.d2l.pass2.1",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0349, 1),
             2, "DEFS.H:2672 front-left/front-right");
    CHECK_EQ("decode.d2r.pass2.0",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0439, 0),
             2, "DEFS.H:2675 front-right/front-left");
    CHECK_EQ("decode.d2r.pass2.1",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0439, 1),
             3, "DEFS.H:2675 front-right/front-left");
    CHECK_EQ("decode.zero", dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0, 0),
             -1, "zero terminates cell order");
    CHECK_EQ("decode.bad_hi", dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0218, 4),
             -1, "decode guard");
    CHECK_EQ("decode.bad_lo", dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(0x0218, -1),
             -1, "decode guard");
}

static void check_composition_one(
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *s,
    int x,
    int outside_x)
{
    DM1_V1_D2L2D2R2F0111DoorFrontPixelPc34 p;

    CHECK_EQ("compose.apply",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
                 s, x, 24, 0x11, 0x21, 0x31, 0x41, 0x51, &p),
             1, "F0108 -> F0115 pass1 -> F0111 -> F0115 pass2");
    CHECK_EQ("compose.in_clip", p.in_clip ? 1 : 0, 1,
             "DUNVIEW.C:646-681 door frame clip");
    CHECK_EQ("compose.after_floor", p.after_floor, 0x21,
             "F0108 floor ornament writes first");
    CHECK_EQ("compose.after_pass1", p.after_pass1, 0x31,
             "F0115 pass1 writes before door");
    CHECK_EQ("compose.after_door", p.after_door, 0x41,
             "F0111 door survives pass1");
    CHECK_EQ("compose.after_pass2", p.after_pass2, 0x51,
             "F0115 pass2 writes last");

    CHECK_EQ("compose.c10.apply",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
                 s, x, 24, 0x66, 10, 10, 10, 10, &p),
             1, "C10 transparent route");
    CHECK_EQ("compose.c10.floor", p.floor_transparent ? 1 : 0, 1,
             "F0108 C10 transparent");
    CHECK_EQ("compose.c10.pass1", p.pass1_transparent ? 1 : 0, 1,
             "F0115 C10 transparent");
    CHECK_EQ("compose.c10.door", p.door_transparent ? 1 : 0, 1,
             "F0111 C10 transparent");
    CHECK_EQ("compose.c10.final", p.after_pass2, 0x66,
             "all transparent layers preserve destination");

    CHECK_EQ("compose.door_survives.apply",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
                 s, x, 24, 0x66, 10, 10, 0x44, 10, &p),
             1, "door-front survives transparent pass2");
    CHECK_EQ("compose.door_survives", p.after_pass2, 0x44,
             "F0111 door-front pixel remains visible");

    CHECK_EQ("compose.outside.apply",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
                 s, outside_x, 24, 0x77, 0x21, 0x31, 0x41, 0x51, &p),
             1, "outside door rectangle");
    CHECK_EQ("compose.outside.no_write", p.no_write_metadata ? 1 : 0, 1,
             "outside frame has no write metadata");
    CHECK_EQ("compose.outside.final", p.after_pass2, 0x77,
             "outside frame preserves destination");
}

static void test_pixel_composition(void)
{
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *d2l =
        dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_for_side_pc34(1);
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *d2r =
        dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_for_side_pc34(2);
    DM1_V1_D2L2D2R2F0111DoorFrontPixelPc34 p;

    CHECK_EQ("blend.transparent",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(0xaa, 10, 10),
             0xaa, "C10_COLOR_FLESH preserves destination");
    CHECK_EQ("blend.opaque",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(0xaa, 0xbb, 10),
             0xbb, "opaque source writes");
    CHECK_EQ("compose.null.out",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
                 d2l, 0, 24, 0, 0, 0, 0, 0, NULL),
             0, "null output guard");
    CHECK_EQ("compose.null.spec",
             dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
                 NULL, 0, 24, 0, 0, 0, 0, 0, &p),
             0, "null spec guard");
    check_composition_one(d2l, 0, 64);
    check_composition_one(d2r, 160, 159);
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_source_evidence_pc34();
    const char *c =
        dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_csb_lineage_evidence_pc34;

    check_contains("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                   "mandatory F0107 anchor");
    check_contains("evidence.f0108", e, "DUNVIEW.C F0108:3940-4011",
                   "mandatory F0108 anchor");
    check_contains("evidence.f0111", e, "DUNVIEW.C F0111:4218-4337",
                   "mandatory F0111 anchor");
    check_contains("evidence.d2l", e, "DUNVIEW.C:6987-7004",
                   "D2L front-door route");
    check_contains("evidence.d2r", e, "DUNVIEW.C:7180-7197",
                   "D2R front-door route");
    check_contains("evidence.d2l2_guard", e, "DUNVIEW.C:6837-6896",
                   "D2L2/D2R2 no-route guard");
    check_contains("evidence.m604", e, "DEFS.H:2603-2604 M604/M605",
                   "M604/M605 source lock");
    check_contains("evidence.m575", e, "DEFS.H:2698-2707",
                   "wall ornament ordinal family");
    check_contains("evidence.zone_family", e, "M624/M626",
                   "door-zone family cross-link");
    check_contains("csb.1192", c, "Viewport.cpp:1192-1209",
                   "CSB-lineage open-row evidence");
    check_contains("csb.1853", c, "Viewport.cpp:1853-1889",
                   "CSB-lineage D2 door-facing evidence");
    check_contains("csb.1903", c, "Viewport.cpp:1903-1915",
                   "CSB-lineage center door-facing evidence");
    CHECK_EQ("csb.not_empty", c[0] != '\0', 1, "CSB evidence linked");
}

int main(void)
{
    test_specs();
    test_f0107_and_orders();
    test_pixel_composition();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
