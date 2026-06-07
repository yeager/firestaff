#include "dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define A_F0119 "DUNVIEW.C:F0119_DUNGEONVIEW_DrawSquareD2L:6900-7049"
#define A_F0120 "DUNVIEW.C:F0120_DUNGEONVIEW_DrawSquareD2R_CPSF:7051-7220"
#define A_F0104 "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156"
#define A_F0105 "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247"
#define A_F0115 "DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4581"
#define A_F0128 "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8503-8517"
#define A_DEFS_SLOTS "DEFS.H:2443/2450 C02/C09"
#define A_DEFS_VIEW "DEFS.H:2582-2583 and 2603-2604 M604/M605"
#define A_DEFS_ORDER "DEFS.H:2662 C0x0021 and 2676 C0x3421"
#define A_DEFS_ZONES "DEFS.H:4144/4146/4157/4159/4202/4204"
#define A_DEFS_C10 "DEFS.H:2088 C10_COLOR_FLESH"
#define A_DUNGEON "DUNGEON.C:F0163/F0164/F0172"

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, const char *anchor, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s anchor=%s got=%d want=%d\n", id, anchor, got, want);
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s value=%d\n", id, anchor, want);
    }
}

static void expect_size(const char *id, const char *anchor, size_t got, size_t want)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s anchor=%s got=%lu want=%lu\n",
               id, anchor, (unsigned long)got, (unsigned long)want);
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s value=%lu\n", id, anchor, (unsigned long)want);
    }
}

static void expect_contains(const char *id, const char *anchor,
                            const char *text, const char *needle)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        printf("FAIL %s anchor=%s missing=%s\n", id, anchor, needle ? needle : "(null)");
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s contains=%s\n", id, anchor, needle);
    }
}

static void expect_bytes(const char *id, const char *anchor,
                         const uint8_t *got, const uint8_t *want, size_t count)
{
    size_t i;

    ++g_assertions;
    for (i = 0; i < count; ++i) {
        if (got[i] != want[i]) {
            printf("FAIL %s anchor=%s index=%lu got=0x%02x want=0x%02x\n",
                   id, anchor, (unsigned long)i, got[i], want[i]);
            ++g_failures;
            return;
        }
    }
    printf("PASS %s anchor=%s bytes=%lu\n", id, anchor, (unsigned long)count);
}

static const char *side_anchor(DM1_V1_D2L2D2R2StairsPitSidePc34 side)
{
    return side == DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34 ?
        A_F0119 : A_F0120;
}

static const char *draw_anchor(DM1_V1_D2L2D2R2StairsPitSidePc34 side)
{
    return side == DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34 ?
        A_F0104 : A_F0105;
}

static DM1_V1_D2L2D2R2StairsPitPixelInputPc34 make_pixel_input(
    DM1_V1_D2L2D2R2StairsPitSidePc34 side,
    DM1_V1_D2L2D2R2StairsPitRoutePc34 route,
    const uint8_t *source,
    size_t source_len,
    uint8_t *destination,
    size_t destination_len,
    size_t row_width,
    size_t height,
    size_t destination_stride)
{
    DM1_V1_D2L2D2R2StairsPitPixelInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.side = side;
    input.route = route;
    input.source = source;
    input.source_len = source_len;
    input.destination = destination;
    input.destination_len = destination_len;
    input.row_width = row_width;
    input.height = height;
    input.destination_stride = destination_stride;
    input.contract_only = true;
    input.real_asset_claim = false;
    return input;
}

static void test_public_constants(void)
{
    expect_int("const.element.corridor", "DEFS.H element contract",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_CORRIDOR, 1);
    expect_int("const.element.pit", "DEFS.H C02_ELEMENT_PIT",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_PIT, 2);
    expect_int("const.element.teleporter", "DEFS.H C05_ELEMENT_TELEPORTER",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_TELEPORTER, 5);
    expect_int("const.element.stairs_side", "DEFS.H C18_ELEMENT_STAIRS_SIDE",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_SIDE, 18);
    expect_int("const.element.stairs_front", "DEFS.H C19_ELEMENT_STAIRS_FRONT",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT, 19);
    expect_int("const.first_stairs_graphic", A_DEFS_SLOTS,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC, 108);
    expect_int("const.slot.up_d2l", A_DEFS_SLOTS,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D2L, 2);
    expect_int("const.slot.down_d2l", A_DEFS_SLOTS,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D2L, 9);
    expect_int("const.pit.open_d2l", "DEFS.H:2335 M756_GRAPHIC_FLOOR_PIT_D2L",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D2L, 52);
    expect_int("const.pit.invisible_d2l", "DEFS.H:2341 M762_GRAPHIC_FLOOR_PIT_INVISIBLE_D2L",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_FLOOR_PIT_INVISIBLE_GRAPHIC_D2L, 58);
    expect_int("const.zone.up_d2l", A_DEFS_ZONES,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D2L, 805);
    expect_int("const.zone.up_d2r", A_DEFS_ZONES,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D2R, 807);
    expect_int("const.zone.down_d2l", A_DEFS_ZONES,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D2L, 818);
    expect_int("const.zone.down_d2r", A_DEFS_ZONES,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D2R, 820);
    expect_int("const.zone.pit_d2l", A_DEFS_ZONES,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D2L, 855);
    expect_int("const.zone.pit_d2r", A_DEFS_ZONES,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D2R, 857);
    expect_int("const.view.d2l.legacy", "DEFS.H:2582 M604_VIEW_SQUARE_D2L",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2L, 4);
    expect_int("const.view.d2r.legacy", "DEFS.H:2583 M605_VIEW_SQUARE_D2R",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2R, 5);
    expect_int("const.view.d2l.pc34", "DEFS.H:2603 M604_VIEW_SQUARE_D2L",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2L_PC34, 7);
    expect_int("const.view.d2r.pc34", "DEFS.H:2604 M605_VIEW_SQUARE_D2R",
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_VIEW_SQUARE_D2R_PC34, 8);
    expect_int("const.order.c0x0021", A_DEFS_ORDER,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_CELL_ORDER_BACKLEFT_BACKRIGHT,
               0x0021);
    expect_int("const.order.open", A_DEFS_ORDER,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_CELL_ORDER_OPEN, 0x3421);
    expect_int("const.transparent.c10", A_DEFS_C10,
               DM1_V1_D2L2_D2R2_STAIRS_PIT_PC34_TRANSPARENT_COLOR, 10);
}

static void test_anchor_citation_table(void)
{
    size_t count = 0;
    const DM1_V1_D2L2D2R2StairsPitAnchorPc34 *anchors =
        dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_anchor_citations_pc34(&count);
    size_t i;

    expect_int("anchors.nonnull", "anchor-citation table", anchors != NULL, 1);
    expect_size("anchors.count", "anchor-citation table", count, 11);
    for (i = 0; i < count; ++i) {
        char id[96];
        snprintf(id, sizeof(id), "anchor.%lu.label", (unsigned long)i);
        expect_int(id, "anchor-citation table", anchors[i].label != NULL, 1);
        snprintf(id, sizeof(id), "anchor.%lu.file", (unsigned long)i);
        expect_int(id, "anchor-citation table", anchors[i].file != NULL, 1);
        snprintf(id, sizeof(id), "anchor.%lu.function", (unsigned long)i);
        expect_int(id, "anchor-citation table", anchors[i].function_name != NULL, 1);
        snprintf(id, sizeof(id), "anchor.%lu.lines", (unsigned long)i);
        expect_int(id, "anchor-citation table", anchors[i].last_line >= anchors[i].first_line, 1);
        snprintf(id, sizeof(id), "anchor.%lu.note", (unsigned long)i);
        expect_int(id, "anchor-citation table", anchors[i].contract_note != NULL, 1);
    }
    expect_contains("anchors.f0119", A_F0119, anchors[0].function_name, "F0119");
    expect_contains("anchors.f0120_cpsf", A_F0120, anchors[1].function_name, "CPSF");
    expect_int("anchors.f0104.first", A_F0104, anchors[2].first_line, 3113);
    expect_int("anchors.f0105.first", A_F0105, anchors[3].first_line, 3185);
    expect_int("anchors.f0128.first", A_F0128, anchors[6].first_line, 8503);
    expect_int("anchors.f0172.last", A_DUNGEON, anchors[9].last_line, 2523);
}

static void test_spec_table(void)
{
    static const struct {
        DM1_V1_D2L2D2R2StairsPitSidePc34 side;
        DM1_V1_D2L2D2R2StairsPitRoutePc34 route;
        int element;
        int slot_or_graphic;
        int native_index;
        int zone;
        int legacy_view_square;
        int pc34_view_square;
        int lateral;
        int wall_lateral;
        int wall_order;
        int draw_order;
        int f0104;
        int f0105;
        int cpsf;
    } cases[] = {
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
          19, 2, 110, 805, 4, 7, -1, -2, 8, 10, 1, 0, 0 },
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
          19, 9, 117, 818, 4, 7, -1, -2, 8, 10, 1, 0, 0 },
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
          2, 52, 52, 855, 4, 7, -1, -2, 8, 10, 1, 0, 0 },
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
          2, 58, 58, 855, 4, 7, -1, -2, 8, 10, 1, 0, 0 },
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
          19, 2, 110, 807, 5, 8, 1, 2, 9, 11, 0, 1, 1 },
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
          19, 9, 117, 820, 5, 8, 1, 2, 9, 11, 0, 1, 1 },
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
          2, 52, 52, 857, 5, 8, 1, 2, 9, 11, 0, 1, 1 },
        { DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
          DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34,
          2, 58, 58, 857, 5, 8, 1, 2, 9, 11, 0, 1, 1 }
    };
    size_t i;

    expect_size("spec.count", A_F0119,
                dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_count_pc34(), 8);
    expect_int("spec.at0", A_F0119,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_at_pc34(0) != NULL, 1);
    expect_int("spec.at8.null", A_F0119,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_at_pc34(8) == NULL, 1);
    expect_int("spec.unknown.null", A_F0119,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_pc34(
                   (DM1_V1_D2L2D2R2StairsPitSidePc34)99,
                   DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34) == NULL, 1);

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const DM1_V1_D2L2D2R2StairsPitSpecPc34 *spec =
            dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_spec_pc34(
                cases[i].side, cases[i].route);
        char id[96];

        snprintf(id, sizeof(id), "spec.%lu.exists", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec != NULL, 1);
        if (!spec) continue;

        snprintf(id, sizeof(id), "spec.%lu.side", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->side, cases[i].side);
        snprintf(id, sizeof(id), "spec.%lu.route", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->route, cases[i].route);
        snprintf(id, sizeof(id), "spec.%lu.role", (unsigned long)i);
        expect_int(id, "public role string", spec->role_name != NULL, 1);
        snprintf(id, sizeof(id), "spec.%lu.function", (unsigned long)i);
        expect_contains(id, side_anchor(cases[i].side), spec->draw_square_function,
                        cases[i].side == DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34 ?
                        "F0119" : "F0120");
        snprintf(id, sizeof(id), "spec.%lu.cpsf_name", (unsigned long)i);
        expect_int(id, A_F0120, strstr(spec->draw_square_function, "CPSF") != NULL,
                   cases[i].cpsf);
        snprintf(id, sizeof(id), "spec.%lu.dispatch_anchor", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->dispatch_anchor != NULL, 1);
        snprintf(id, sizeof(id), "spec.%lu.draw_anchor", (unsigned long)i);
        expect_contains(id, draw_anchor(cases[i].side), spec->draw_anchor,
                        cases[i].f0104 ? "F0104" : "F0105");
        snprintf(id, sizeof(id), "spec.%lu.defs_anchor", (unsigned long)i);
        expect_int(id, A_DEFS_ZONES, spec->defs_anchor != NULL, 1);
        snprintf(id, sizeof(id), "spec.%lu.dungeon_anchor", (unsigned long)i);
        expect_contains(id, A_DUNGEON, spec->dungeon_anchor, "F0172");
        snprintf(id, sizeof(id), "spec.%lu.depth", (unsigned long)i);
        expect_int(id, A_F0128, spec->perspective_depth, 2);
        snprintf(id, sizeof(id), "spec.%lu.forward", (unsigned long)i);
        expect_int(id, A_F0128, spec->relative_forward, 2);
        snprintf(id, sizeof(id), "spec.%lu.lateral", (unsigned long)i);
        expect_int(id, A_F0128, spec->relative_lateral, cases[i].lateral);
        snprintf(id, sizeof(id), "spec.%lu.wall_lateral", (unsigned long)i);
        expect_int(id, A_F0128, spec->f0128_preceding_wall_lateral,
                   cases[i].wall_lateral);
        snprintf(id, sizeof(id), "spec.%lu.wall_order", (unsigned long)i);
        expect_int(id, A_F0128, spec->f0128_preceding_wall_order, cases[i].wall_order);
        snprintf(id, sizeof(id), "spec.%lu.draw_order", (unsigned long)i);
        expect_int(id, A_F0128, spec->f0128_draw_order, cases[i].draw_order);
        snprintf(id, sizeof(id), "spec.%lu.wall_before_dispatch", (unsigned long)i);
        expect_int(id, A_F0128, spec->f0128_preceding_wall_order < spec->f0128_draw_order, 1);
        snprintf(id, sizeof(id), "spec.%lu.element", (unsigned long)i);
        expect_int(id, "DEFS.H element constants", spec->element_class, cases[i].element);
        snprintf(id, sizeof(id), "spec.%lu.slot_graphic", (unsigned long)i);
        expect_int(id, A_DEFS_SLOTS, spec->native_bitmap_slot_or_graphic,
                   cases[i].slot_or_graphic);
        snprintf(id, sizeof(id), "spec.%lu.native_index", (unsigned long)i);
        expect_int(id, draw_anchor(cases[i].side), spec->native_bitmap_index,
                   cases[i].native_index);
        snprintf(id, sizeof(id), "spec.%lu.first_stairs", (unsigned long)i);
        expect_int(id, A_DEFS_SLOTS, spec->first_stairs_graphic_index, 108);
        snprintf(id, sizeof(id), "spec.%lu.zone", (unsigned long)i);
        expect_int(id, A_DEFS_ZONES, spec->zone_index, cases[i].zone);
        snprintf(id, sizeof(id), "spec.%lu.legacy_view", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, spec->legacy_view_square_index,
                   cases[i].legacy_view_square);
        snprintf(id, sizeof(id), "spec.%lu.pc34_view", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, spec->pc34_view_square_index,
                   cases[i].pc34_view_square);
        snprintf(id, sizeof(id), "spec.%lu.order_0021", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, spec->cell_order_backleft_backright, 0x0021);
        snprintf(id, sizeof(id), "spec.%lu.order_open", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, spec->cell_order_open_followup, 0x3421);
        snprintf(id, sizeof(id), "spec.%lu.c10", (unsigned long)i);
        expect_int(id, A_DEFS_C10, spec->transparent_color, 10);
        snprintf(id, sizeof(id), "spec.%lu.f0104", (unsigned long)i);
        expect_int(id, A_F0104, spec->uses_f0104_native, cases[i].f0104);
        snprintf(id, sizeof(id), "spec.%lu.f0105", (unsigned long)i);
        expect_int(id, A_F0105, spec->uses_f0105_flipped, cases[i].f0105);
        snprintf(id, sizeof(id), "spec.%lu.f0108", (unsigned long)i);
        expect_int(id, "DUNVIEW.C:F0119/F0120 floor follow-up", spec->uses_f0108_floor_followup, 1);
        snprintf(id, sizeof(id), "spec.%lu.f0112", (unsigned long)i);
        expect_int(id, "DUNVIEW.C:F0119/F0120 ceiling follow-up", spec->uses_f0112_ceiling_followup, 1);
        snprintf(id, sizeof(id), "spec.%lu.f0115", (unsigned long)i);
        expect_int(id, A_F0115, spec->uses_f0115_thing_pass_followup, 1);
        snprintf(id, sizeof(id), "spec.%lu.f0128", (unsigned long)i);
        expect_int(id, A_F0128, spec->uses_f0128_post_d2l2_d2r2_wall_followup, 1);
        snprintf(id, sizeof(id), "spec.%lu.no_f0111", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->no_f0111_door_dispatch, 1);
        snprintf(id, sizeof(id), "spec.%lu.cpsf_flag", (unsigned long)i);
        expect_int(id, A_F0120, spec->f0120_cpsf_callout, cases[i].cpsf);
        snprintf(id, sizeof(id), "spec.%lu.contract", (unsigned long)i);
        expect_int(id, "contract-only", spec->contract_only, 1);
        snprintf(id, sizeof(id), "spec.%lu.no_real_asset", (unsigned long)i);
        expect_int(id, "no real asset claim", spec->real_asset_claim, 0);
    }
}

static void test_pixel_run_contract(void)
{
    const uint8_t left_source[] = { 1, 10, 3, 4, 5, 6 };
    const uint8_t left_want[] = { 1, 0xee, 3, 0xee, 4, 5, 6, 0xee };
    const uint8_t right_source[] = { 1, 10, 3, 4, 5, 10 };
    const uint8_t right_want[] = { 3, 0xdd, 1, 0xdd, 0xdd, 5, 4, 0xdd };
    const uint8_t transparent_source[] = { 10, 10, 10, 10 };
    const uint8_t transparent_want[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    uint8_t destination[8];
    DM1_V1_D2L2D2R2StairsPitPixelInputPc34 input;
    DM1_V1_D2L2D2R2StairsPitPixelResultPc34 result;

    memset(destination, 0xee, sizeof(destination));
    input = make_pixel_input(DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
                             DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
                             left_source, sizeof(left_source), destination,
                             sizeof(destination), 3, 2, 4);
    expect_int("pixel.d2l.ok", A_F0104,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.d2l.bytes", A_F0104, destination, left_want, sizeof(left_want));
    expect_int("pixel.d2l.ok_flag", A_F0104, result.ok, 1);
    expect_int("pixel.d2l.flip", A_F0104, result.flipped_horizontally, 0);
    expect_size("pixel.d2l.writes", A_F0104, result.writes, 5);
    expect_size("pixel.d2l.skips", A_DEFS_C10, result.transparent_skips, 1);
    expect_int("pixel.d2l.skip_seen", A_DEFS_C10, result.transparent_skip_seen, 1);
    expect_int("pixel.d2l.first_source", A_F0104, result.first_source_byte, 1);
    expect_int("pixel.d2l.last_source", A_F0104, result.last_source_byte, 6);
    expect_int("pixel.d2l.first_destination", A_F0104, result.first_destination_byte, 1);
    expect_int("pixel.d2l.last_destination", A_F0104, result.last_destination_byte, 6);
    expect_int("pixel.d2l.spec_f0104", A_F0104,
               result.spec != NULL ? result.spec->uses_f0104_native : 0, 1);

    memset(destination, 0xdd, sizeof(destination));
    input = make_pixel_input(DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2R2_PC34,
                             DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
                             right_source, sizeof(right_source), destination,
                             sizeof(destination), 3, 2, 4);
    expect_int("pixel.d2r.ok", A_F0105,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.d2r.bytes", A_F0105, destination, right_want, sizeof(right_want));
    expect_int("pixel.d2r.flip", A_F0105, result.flipped_horizontally, 1);
    expect_size("pixel.d2r.writes", A_F0105, result.writes, 4);
    expect_size("pixel.d2r.skips", A_DEFS_C10, result.transparent_skips, 2);
    expect_int("pixel.d2r.first_destination", A_F0105, result.first_destination_byte, 3);
    expect_int("pixel.d2r.last_destination", A_F0105, result.last_destination_byte, 4);
    expect_int("pixel.d2r.spec_f0105", A_F0105,
               result.spec != NULL ? result.spec->uses_f0105_flipped : 0, 1);

    memcpy(destination, transparent_want, sizeof(transparent_want));
    input = make_pixel_input(DM1_V1_D2L2_D2R2_STAIRS_PIT_SIDE_D2L2_PC34,
                             DM1_V1_D2L2_D2R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
                             transparent_source, sizeof(transparent_source),
                             destination, sizeof(transparent_want), 2, 2, 2);
    expect_int("pixel.transparent.ok", A_DEFS_C10,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.transparent.unchanged", A_DEFS_C10,
                 destination, transparent_want, sizeof(transparent_want));
    expect_size("pixel.transparent.writes", A_DEFS_C10, result.writes, 0);
    expect_size("pixel.transparent.skips", A_DEFS_C10, result.transparent_skips, 4);
    expect_int("pixel.transparent.wrote_any", A_DEFS_C10, result.wrote_any, 0);

    expect_int("pixel.reject.null_input", A_F0104,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   NULL, &result), 0);
    expect_int("pixel.reject.null_output", A_F0104,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, NULL), 0);
    input.contract_only = false;
    expect_int("pixel.reject.non_contract", A_F0104,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
    input.contract_only = true;
    input.real_asset_claim = true;
    expect_int("pixel.reject.real_asset", A_F0104,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
    input.real_asset_claim = false;
    input.destination_stride = 1;
    expect_int("pixel.reject.short_stride", A_F0104,
               dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
}

static void test_source_evidence(void)
{
    const char *summary =
        dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_source_evidence_pc34();

    expect_contains("summary.contract", A_F0119, summary, "contract_only=1");
    expect_contains("summary.no_real_asset", A_F0104, summary, "no real-asset");
    expect_contains("summary.f0119", A_F0119, summary, "6900-7049");
    expect_contains("summary.f0120", A_F0120, summary, "7051-7220");
    expect_contains("summary.f0120_cpsf", A_F0120, summary, "CPSF");
    expect_contains("summary.f0104", A_F0104, summary, "3113-3156");
    expect_contains("summary.f0105", A_F0105, summary, "3185-3247");
    expect_contains("summary.f0115", A_F0115, summary, "F0115");
    expect_contains("summary.f0128", A_F0128, summary, "8503-8517");
    expect_contains("summary.c02", A_DEFS_SLOTS, summary, "C02");
    expect_contains("summary.c09", A_DEFS_SLOTS, summary, "C09");
    expect_contains("summary.m756", "DEFS.H:2335 M756", summary, "M756");
    expect_contains("summary.m762", "DEFS.H:2341 M762", summary, "M762");
    expect_contains("summary.c805", A_DEFS_ZONES, summary, "C805");
    expect_contains("summary.c807", A_DEFS_ZONES, summary, "C807");
    expect_contains("summary.c818", A_DEFS_ZONES, summary, "C818");
    expect_contains("summary.c820", A_DEFS_ZONES, summary, "C820");
    expect_contains("summary.c855", A_DEFS_ZONES, summary, "C855");
    expect_contains("summary.c857", A_DEFS_ZONES, summary, "C857");
    expect_contains("summary.m604", A_DEFS_VIEW, summary, "M604");
    expect_contains("summary.m605", A_DEFS_VIEW, summary, "M605");
    expect_contains("summary.c0x0021", A_DEFS_ORDER, summary, "C0x0021");
    expect_contains("summary.c0x3421", A_DEFS_ORDER, summary, "C0x3421");
    expect_contains("summary.c10", A_DEFS_C10, summary, "C10");
    expect_contains("summary.f0163", A_DUNGEON, summary, "F0163");
    expect_contains("summary.f0164", A_DUNGEON, summary, "F0164");
    expect_contains("summary.f0172", A_DUNGEON, summary, "F0172");
    expect_contains("summary.no_dosbox", "contract boundary", summary, "no live DOSBox");
    expect_contains("summary.no_main_loop", "contract boundary", summary, "main-loop");
    expect_contains("summary.no_game_data", "contract boundary", summary, "game-data");
}

int main(void)
{
    test_public_constants();
    test_anchor_citation_table();
    test_spec_table();
    test_pixel_run_contract();
    test_source_evidence();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
