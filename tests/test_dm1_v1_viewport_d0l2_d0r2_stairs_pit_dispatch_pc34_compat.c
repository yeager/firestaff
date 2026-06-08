#include "dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define A_F0116 "DUNVIEW.C:F0116_DUNGEONVIEW_DrawSquareD3L:6361-6480"
#define A_F0117 "DUNVIEW.C:F0117_DUNGEONVIEW_DrawSquareD3R:6500-6622"
#define A_F0104 "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156"
#define A_F0105 "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247"
#define A_F0115 "DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4581,5668-5671"
#define A_F0128 "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8478-8508"
#define A_DUNGEON "DUNGEON.C:F0163/F0164:1769-1840; F0172:2466-2523"
#define A_DEFS_C10 "DEFS.H:2088 C10_COLOR_FLESH"
#define A_DEFS_REQUESTED "DEFS.H:2443/2450/2582-2583/2603-2604/2610-2611/2662/2676-2677/4139-4153/4197-4198"
#define A_DEFS_EXACT "DEFS.H:2441/2448/2608-2609/4141/4143/4154/4156/4199/4201"

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

static void expect_not_contains(const char *id, const char *anchor,
                                const char *text, const char *needle)
{
    ++g_assertions;
    if (!text || !needle || strstr(text, needle)) {
        printf("FAIL %s anchor=%s unexpected=%s\n",
               id, anchor, needle ? needle : "(null)");
        ++g_failures;
    } else {
        printf("PASS %s anchor=%s absent=%s\n", id, anchor, needle);
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

static const char *side_anchor(DM1_V1_D0L2D0R2StairsPitSidePc34 side)
{
    return side == DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34 ? A_F0116 : A_F0117;
}

static const char *draw_anchor(DM1_V1_D0L2D0R2StairsPitSidePc34 side)
{
    return side == DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34 ? A_F0104 : A_F0105;
}

static DM1_V1_D0L2D0R2StairsPitPixelInputPc34 make_pixel_input(
    DM1_V1_D0L2D0R2StairsPitSidePc34 side,
    DM1_V1_D0L2D0R2StairsPitRoutePc34 route,
    const uint8_t *source,
    size_t source_len,
    uint8_t *destination,
    size_t destination_len,
    size_t row_width,
    size_t height,
    size_t destination_stride)
{
    DM1_V1_D0L2D0R2StairsPitPixelInputPc34 input;
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
    expect_int("constants.element.corridor", "DEFS.H C01 element", DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_CORRIDOR, 1);
    expect_int("constants.element.pit", "DEFS.H C02 element", DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_PIT, 2);
    expect_int("constants.element.teleporter", "DEFS.H C05 element", DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_TELEPORTER, 5);
    expect_int("constants.element.stairs_side", "DEFS.H C18 element", DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_SIDE, 18);
    expect_int("constants.element.stairs_front", "DEFS.H C19 element", DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ELEMENT_STAIRS_FRONT, 19);
    expect_int("constants.first_stairs", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_FIRST_STAIRS_GRAPHIC, 108);
    expect_int("constants.up_slot", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_STAIRS_UP_FRONT_SLOT_D3L, 0);
    expect_int("constants.down_slot", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_STAIRS_DOWN_FRONT_SLOT_D3L, 7);
    expect_int("constants.floor_pit_graphic", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_FLOOR_PIT_GRAPHIC_D3L, 50);
    expect_int("constants.zone.up.left", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D3L, 802);
    expect_int("constants.zone.up.right", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_UP_FRONT_D3R, 804);
    expect_int("constants.zone.down.left", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D3L, 815);
    expect_int("constants.zone.down.right", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_FRONT_D3R, 817);
    expect_int("constants.zone.pit.left", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3L, 852);
    expect_int("constants.zone.pit.right", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D3R, 854);
    expect_int("constants.legacy.left", A_F0116, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_LEGACY_VIEW_SQUARE_D3L, 1);
    expect_int("constants.legacy.right", A_F0117, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_LEGACY_VIEW_SQUARE_D3R, 2);
    expect_int("constants.pc34.left", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_PC34_VIEW_SQUARE_D3L, 12);
    expect_int("constants.pc34.right", A_DEFS_EXACT, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_PC34_VIEW_SQUARE_D3R, 13);
    expect_int("constants.order.left", A_DEFS_REQUESTED, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_CELL_ORDER_D3L_OPEN, 0x3421);
    expect_int("constants.order.right", A_DEFS_REQUESTED, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_CELL_ORDER_D3R_OPEN, 0x4312);
    expect_int("constants.c10", A_DEFS_C10, DM1_V1_D0L2_D0R2_STAIRS_PIT_PC34_TRANSPARENT_COLOR, 10);
}

static void test_anchor_citation_table(void)
{
    size_t count = 0;
    const DM1_V1_D0L2D0R2StairsPitAnchorPc34 *anchors =
        dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_anchor_citations_pc34(&count);
    size_t i;

    expect_size("anchors.count", A_F0116, count, 12);
    expect_int("anchors.nonnull", A_F0116, anchors != NULL, 1);
    for (i = 0; i < count; ++i) {
        char id[96];
        snprintf(id, sizeof(id), "anchors.%lu.label", (unsigned long)i);
        expect_int(id, "anchor table", anchors[i].label != NULL, 1);
        snprintf(id, sizeof(id), "anchors.%lu.file", (unsigned long)i);
        expect_int(id, "anchor table", anchors[i].file != NULL, 1);
        snprintf(id, sizeof(id), "anchors.%lu.function", (unsigned long)i);
        expect_int(id, "anchor table", anchors[i].function_name != NULL, 1);
        snprintf(id, sizeof(id), "anchors.%lu.lines", (unsigned long)i);
        expect_int(id, "anchor table", anchors[i].first_line <= anchors[i].last_line, 1);
        snprintf(id, sizeof(id), "anchors.%lu.note", (unsigned long)i);
        expect_int(id, "anchor table", anchors[i].contract_note != NULL, 1);
    }
    expect_contains("anchors.0.f0116", A_F0116, anchors[0].function_name, "F0116");
    expect_contains("anchors.1.f0117", A_F0117, anchors[1].function_name, "F0117");
    expect_int("anchors.0.first", A_F0116, anchors[0].first_line, 6361);
    expect_int("anchors.1.last", A_F0117, anchors[1].last_line, 6622);
    expect_contains("anchors.6.f0128", A_F0128, anchors[6].function_name, "F0128");
}

static void test_spec_table(void)
{
    static const struct {
        DM1_V1_D0L2D0R2StairsPitSidePc34 side;
        DM1_V1_D0L2D0R2StairsPitRoutePc34 route;
        int element;
        int slot_or_graphic;
        int native_index;
        int zone;
        int legacy_view_square;
        int pc34_view_square;
        int order;
        int lateral;
        int draw_order;
        int f0104;
        int f0105;
        int has_bitmap;
    } cases[] = {
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 19, 0, 108, 802, 1, 12, 0x3421, -1, 30, 1, 0, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 19, 7, 115, 815, 1, 12, 0x3421, -1, 30, 1, 0, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 2, 50, 50, 852, 1, 12, 0x3421, -1, 30, 1, 0, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34, 2, -1, -1, 852, 1, 12, 0x3421, -1, 30, 0, 0, 0 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 19, 0, 108, 804, 2, 13, 0x4312, 1, 40, 0, 1, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 19, 7, 115, 817, 2, 13, 0x4312, 1, 40, 0, 1, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 2, 50, 50, 854, 2, 13, 0x4312, 1, 40, 0, 1, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34, 2, -1, -1, 854, 2, 13, 0x4312, 1, 40, 0, 0, 0 }
    };
    size_t count = dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_count_pc34();
    size_t i;

    expect_size("spec.count", A_F0116, count, 8);
    expect_int("spec.at0.nonnull", A_F0116,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_at_pc34(0) != NULL, 1);
    expect_int("spec.past.null", A_F0117,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_at_pc34(count) == NULL, 1);
    expect_int("spec.unknown.null", A_F0116,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_pc34(
                   (DM1_V1_D0L2D0R2StairsPitSidePc34)9,
                   DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34) == NULL,
               1);

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const DM1_V1_D0L2D0R2StairsPitSpecPc34 *spec =
            dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_spec_pc34(
                cases[i].side, cases[i].route);
        const int left = cases[i].side == DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34;
        char id[96];

        snprintf(id, sizeof(id), "spec.%lu.exists", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec != NULL, 1);
        if (!spec) continue;
        snprintf(id, sizeof(id), "spec.%lu.side", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->side, cases[i].side);
        snprintf(id, sizeof(id), "spec.%lu.route", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->route, cases[i].route);
        snprintf(id, sizeof(id), "spec.%lu.role", (unsigned long)i);
        expect_contains(id, side_anchor(cases[i].side), spec->role_name, left ? "D0L2" : "D0R2");
        snprintf(id, sizeof(id), "spec.%lu.function", (unsigned long)i);
        expect_contains(id, side_anchor(cases[i].side), spec->draw_square_function, left ? "F0116" : "F0117");
        snprintf(id, sizeof(id), "spec.%lu.dispatch_anchor", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->dispatch_anchor != NULL, 1);
        snprintf(id, sizeof(id), "spec.%lu.draw_anchor", (unsigned long)i);
        if (cases[i].has_bitmap) {
            expect_contains(id, draw_anchor(cases[i].side), spec->draw_anchor, left ? "F0104" : "F0105");
        } else {
            expect_contains(id, side_anchor(cases[i].side), spec->draw_anchor, "no visible-pit bitmap");
        }
        snprintf(id, sizeof(id), "spec.%lu.defs_anchor", (unsigned long)i);
        expect_int(id, A_DEFS_REQUESTED, spec->defs_anchor != NULL, 1);
        snprintf(id, sizeof(id), "spec.%lu.dungeon_anchor", (unsigned long)i);
        expect_contains(id, A_DUNGEON, spec->dungeon_anchor, "F0172");
        snprintf(id, sizeof(id), "spec.%lu.depth", (unsigned long)i);
        expect_int(id, A_F0128, spec->perspective_depth, 3);
        snprintf(id, sizeof(id), "spec.%lu.forward", (unsigned long)i);
        expect_int(id, A_F0128, spec->relative_forward, 3);
        snprintf(id, sizeof(id), "spec.%lu.lateral", (unsigned long)i);
        expect_int(id, A_F0128, spec->relative_lateral, cases[i].lateral);
        snprintf(id, sizeof(id), "spec.%lu.draw_order", (unsigned long)i);
        expect_int(id, A_F0128, spec->f0128_dispatch_order, cases[i].draw_order);
        snprintf(id, sizeof(id), "spec.%lu.element", (unsigned long)i);
        expect_int(id, "DEFS.H element constants", spec->element_class, cases[i].element);
        snprintf(id, sizeof(id), "spec.%lu.slot_graphic", (unsigned long)i);
        expect_int(id, A_DEFS_EXACT, spec->native_bitmap_slot_or_graphic, cases[i].slot_or_graphic);
        snprintf(id, sizeof(id), "spec.%lu.native_index", (unsigned long)i);
        expect_int(id, draw_anchor(cases[i].side), spec->native_bitmap_index, cases[i].native_index);
        snprintf(id, sizeof(id), "spec.%lu.first_stairs", (unsigned long)i);
        expect_int(id, A_DEFS_EXACT, spec->first_stairs_graphic_index, 108);
        snprintf(id, sizeof(id), "spec.%lu.zone", (unsigned long)i);
        expect_int(id, A_DEFS_EXACT, spec->zone_index, cases[i].zone);
        snprintf(id, sizeof(id), "spec.%lu.legacy_view", (unsigned long)i);
        expect_int(id, A_DEFS_EXACT, spec->legacy_view_square_index, cases[i].legacy_view_square);
        snprintf(id, sizeof(id), "spec.%lu.pc34_view", (unsigned long)i);
        expect_int(id, A_DEFS_EXACT, spec->pc34_view_square_index, cases[i].pc34_view_square);
        snprintf(id, sizeof(id), "spec.%lu.order_0021", (unsigned long)i);
        expect_int(id, A_DEFS_REQUESTED, spec->cell_order_backleft_backright, 0x0021);
        snprintf(id, sizeof(id), "spec.%lu.order_open", (unsigned long)i);
        expect_int(id, A_DEFS_REQUESTED, spec->cell_order_open_followup, cases[i].order);
        snprintf(id, sizeof(id), "spec.%lu.c10", (unsigned long)i);
        expect_int(id, A_DEFS_C10, spec->transparent_color, 10);
        snprintf(id, sizeof(id), "spec.%lu.f0104", (unsigned long)i);
        expect_int(id, A_F0104, spec->uses_f0104_native, cases[i].f0104);
        snprintf(id, sizeof(id), "spec.%lu.f0105", (unsigned long)i);
        expect_int(id, A_F0105, spec->uses_f0105_flipped, cases[i].f0105);
        snprintf(id, sizeof(id), "spec.%lu.f0108", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->uses_f0108_floor_followup, 1);
        snprintf(id, sizeof(id), "spec.%lu.f0115", (unsigned long)i);
        expect_int(id, A_F0115, spec->uses_f0115_thing_pass_followup, 1);
        snprintf(id, sizeof(id), "spec.%lu.tail", (unsigned long)i);
        expect_int(id, A_F0115, spec->d0l2_d0r2_tail_dispatch, 1);
        snprintf(id, sizeof(id), "spec.%lu.no_front_helper", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), spec->no_door_front_helper_anchor, 1);
        snprintf(id, sizeof(id), "spec.%lu.no_mixed_route", (unsigned long)i);
        expect_int(id, A_F0128, spec->no_d2l2_d0r2_mixed_route, 1);
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
    DM1_V1_D0L2D0R2StairsPitPixelInputPc34 input;
    DM1_V1_D0L2D0R2StairsPitPixelResultPc34 result;

    memset(destination, 0xee, sizeof(destination));
    input = make_pixel_input(DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
                             DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
                             left_source, sizeof(left_source), destination,
                             sizeof(destination), 3, 2, 4);
    expect_int("pixel.d0l2.ok", A_F0104,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.d0l2.bytes", A_F0104, destination, left_want, sizeof(left_want));
    expect_int("pixel.d0l2.ok_flag", A_F0104, result.ok, 1);
    expect_int("pixel.d0l2.flip", A_F0104, result.flipped_horizontally, 0);
    expect_size("pixel.d0l2.writes", A_F0104, result.writes, 5);
    expect_size("pixel.d0l2.skips", A_DEFS_C10, result.transparent_skips, 1);
    expect_int("pixel.d0l2.skip_seen", A_DEFS_C10, result.transparent_skip_seen, 1);
    expect_int("pixel.d0l2.first_source", A_F0104, result.first_source_byte, 1);
    expect_int("pixel.d0l2.last_source", A_F0104, result.last_source_byte, 6);
    expect_int("pixel.d0l2.first_destination", A_F0104, result.first_destination_byte, 1);
    expect_int("pixel.d0l2.last_destination", A_F0104, result.last_destination_byte, 6);
    expect_int("pixel.d0l2.spec_f0104", A_F0104,
               result.spec != NULL ? result.spec->uses_f0104_native : 0, 1);

    memset(destination, 0xdd, sizeof(destination));
    input = make_pixel_input(DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34,
                             DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
                             right_source, sizeof(right_source), destination,
                             sizeof(destination), 3, 2, 4);
    expect_int("pixel.d0r2.ok", A_F0105,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.d0r2.bytes", A_F0105, destination, right_want, sizeof(right_want));
    expect_int("pixel.d0r2.flip", A_F0105, result.flipped_horizontally, 1);
    expect_size("pixel.d0r2.writes", A_F0105, result.writes, 4);
    expect_size("pixel.d0r2.skips", A_DEFS_C10, result.transparent_skips, 2);
    expect_int("pixel.d0r2.first_destination", A_F0105, result.first_destination_byte, 3);
    expect_int("pixel.d0r2.last_destination", A_F0105, result.last_destination_byte, 4);
    expect_int("pixel.d0r2.spec_f0105", A_F0105,
               result.spec != NULL ? result.spec->uses_f0105_flipped : 0, 1);

    memcpy(destination, transparent_want, sizeof(transparent_want));
    input = make_pixel_input(DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
                             DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
                             transparent_source, sizeof(transparent_source),
                             destination, sizeof(transparent_want), 2, 2, 2);
    expect_int("pixel.transparent.ok", A_DEFS_C10,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.transparent.unchanged", A_DEFS_C10,
                 destination, transparent_want, sizeof(transparent_want));
    expect_size("pixel.transparent.writes", A_DEFS_C10, result.writes, 0);
    expect_size("pixel.transparent.skips", A_DEFS_C10, result.transparent_skips, 4);
    expect_int("pixel.transparent.wrote_any", A_DEFS_C10, result.wrote_any, 0);

    input = make_pixel_input(DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34,
                             DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_VISIBLE_PIT_FOLLOWUP_PC34,
                             left_source, sizeof(left_source), destination,
                             sizeof(destination), 3, 2, 4);
    expect_int("pixel.visible_followup.rejects_no_bitmap", A_F0116,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
    expect_int("pixel.reject.null_input", A_F0104,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   NULL, &result), 0);
    expect_int("pixel.reject.null_output", A_F0104,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, NULL), 0);
    input.contract_only = false;
    expect_int("pixel.reject.non_contract", A_F0104,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
    input.contract_only = true;
    input.real_asset_claim = true;
    expect_int("pixel.reject.real_asset", A_F0104,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
    input.real_asset_claim = false;
    input.route = DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34;
    input.destination_stride = 1;
    expect_int("pixel.reject.short_stride", A_F0104,
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
}

static void test_source_evidence(void)
{
    const char *summary =
        dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_source_evidence_pc34();

    expect_contains("summary.contract", A_F0116, summary, "contract_only=1");
    expect_contains("summary.no_real_asset", A_F0104, summary, "no real-asset");
    expect_contains("summary.no_game_data", "contract boundary", summary, "no game-data load");
    expect_contains("summary.f0116", A_F0116, summary, "F0116_DUNGEONVIEW_DrawSquareD3L:6361-6480");
    expect_contains("summary.f0117", A_F0117, summary, "F0117_DUNGEONVIEW_DrawSquareD3R:6500-6622");
    expect_contains("summary.f0104", A_F0104, summary, "F0104:3113-3156");
    expect_contains("summary.f0105", A_F0105, summary, "F0105:3185-3247");
    expect_contains("summary.f0115", A_F0115, summary, "F0115:4547-4581");
    expect_contains("summary.f0115_projectiles", A_F0115, summary, "5668-5671");
    expect_contains("summary.f0128", A_F0128, summary, "F0128:8478-8508");
    expect_contains("summary.d0l2_tail", A_F0116, summary, "F0115 from this body with M601");
    expect_contains("summary.d0r2_tail", A_F0117, summary, "F0115 from this body with M602");
    expect_contains("summary.no_mixed_route", A_F0128, summary, "no D2L2/D0R2 mixed route");
    expect_contains("summary.f0163", A_DUNGEON, summary, "F0163/F0164");
    expect_contains("summary.f0172", A_DUNGEON, summary, "F0172:2466-2523");
    expect_contains("summary.defs_requested", A_DEFS_REQUESTED, summary, "DEFS.H:2443/2450");
    expect_contains("summary.defs_c14_c15", A_DEFS_REQUESTED, summary, "2610-2611");
    expect_contains("summary.defs_exact_slots", A_DEFS_EXACT, summary, "DEFS.H:2441/2448");
    expect_contains("summary.defs_exact_views", A_DEFS_EXACT, summary, "DEFS.H:2608-2609");
    expect_contains("summary.defs_exact_zones", A_DEFS_EXACT, summary, "DEFS.H:4141/4143/4154/4156");
    expect_contains("summary.defs_pit_zones", A_DEFS_EXACT, summary, "DEFS.H:4199/4201");
    expect_contains("summary.c10", A_DEFS_C10, summary, "C10");
    expect_not_contains("summary.no_front_door_anchor", "front door helper excluded", summary, "F0111");
}

int main(void)
{
    const char *evidence =
        dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_source_evidence_pc34();

    test_public_constants();
    test_anchor_citation_table();
    test_spec_table();
    test_pixel_run_contract();
    test_source_evidence();

    printf("sourceEvidence=%s\n", evidence ? evidence : "(null)");
    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
