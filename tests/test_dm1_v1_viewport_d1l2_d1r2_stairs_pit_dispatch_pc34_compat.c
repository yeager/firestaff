#include "dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define A_F0122 "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7391-7557"
#define A_F0123 "DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7559-7725"
#define A_F0104 "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156"
#define A_F0105 "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3218"
#define A_F0115 "DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4581,5668-5671"
#define A_F0128 "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8524-8542"
#define A_F0127 "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294"
#define A_DEFS_C10 "DEFS.H:2088 C10_COLOR_FLESH"
#define A_DEFS_BITMAPS "DEFS.H:2445-2452 C04/C11; DEFS.H:2337/2343 M758/M764"
#define A_DEFS_VIEW "DEFS.H:2596-2601 M609/M607/M608"
#define A_DEFS_ORDER "DEFS.H:2659-2666 C0x0021/C0x0032/C0x0041"
#define A_DEFS_ZONES "DEFS.H:4147-4162/4205-4207"
#define A_DUNGEON "DUNGEON.C:F0163/F0164:1769-1840; F0172:2466-2523"

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

static const char *side_anchor(DM1_V1_D1L2D1R2StairsPitSidePc34 side)
{
    return side == DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34 ? A_F0122 : A_F0123;
}

static const char *draw_anchor(DM1_V1_D1L2D1R2StairsPitSidePc34 side)
{
    return side == DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34 ? A_F0104 : A_F0105;
}

static DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 context_for(
    DM1_V1_D1L2D1R2StairsPitSidePc34 side,
    int element,
    int stairs_up,
    int visible)
{
    DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 context;
    dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_init_context_pc34(
        &context, side);
    context.direction = 2;
    context.map_x = side == DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34 ? 31 : 33;
    context.map_y = side == DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34 ? 47 : 49;
    context.element_class = element;
    context.stairs_up = stairs_up != 0;
    context.pit_or_teleporter_visible = visible != 0;
    return context;
}

static DM1_V1_D1L2D1R2StairsPitPixelRunInputPc34 pixel_input(
    DM1_V1_D1L2D1R2StairsPitSidePc34 side,
    DM1_V1_D1L2D1R2StairsPitRoutePc34 route,
    const uint8_t *source,
    size_t source_len,
    uint8_t *destination,
    size_t destination_len,
    size_t row_width,
    size_t height,
    size_t destination_stride)
{
    DM1_V1_D1L2D1R2StairsPitPixelRunInputPc34 input;
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

static void test_anchor_table_and_source_summary(void)
{
    size_t count = 0;
    const char *const *anchors =
        dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_anchor_table_pc34(&count);
    const char *summary =
        dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_source_evidence_pc34();
    size_t i;

    expect_size("anchors.count", A_DUNGEON, count, 9);
    expect_int("anchors.nonnull", A_DUNGEON, anchors != NULL, 1);
    for (i = 0; i < count; ++i) {
        char id[80];
        snprintf(id, sizeof(id), "anchors.%lu.nonnull", (unsigned long)i);
        expect_int(id, A_DUNGEON, anchors[i] != NULL, 1);
    }

    expect_contains("summary.contract", A_F0122, summary, "contract_only=1");
    expect_contains("summary.no_asset", A_F0104, summary, "no real-asset");
    expect_contains("summary.no_game_data", A_DUNGEON, summary, "no game-data load");
    expect_contains("summary.f0122", A_F0122, summary, "F0122:7391-7557");
    expect_contains("summary.f0123", A_F0123, summary, "F0123:7559-7725");
    expect_contains("summary.f0104", A_F0104, summary, "F0104:3113-3156");
    expect_contains("summary.f0105", A_F0105, summary, "F0105:3185-3218");
    expect_contains("summary.f0115", A_F0115, summary, "F0115");
    expect_contains("summary.f0128", A_F0128, summary, "F0128:8524-8542");
    expect_contains("summary.f0127", A_F0127, summary, "F0127:8294");
    expect_contains("summary.m608", A_DEFS_VIEW, summary, "M608");
    expect_contains("summary.m609", A_DEFS_VIEW, summary, "M609");
    expect_contains("summary.c0021", A_DEFS_ORDER, summary, "C0x0021");
    expect_contains("summary.c10", A_DEFS_C10, summary, "C10");
    expect_contains("summary.f0163", A_DUNGEON, summary, "F0163");
    expect_contains("summary.f0164", A_DUNGEON, summary, "F0164");
    expect_contains("summary.f0172", A_DUNGEON, summary, "F0172");
}

static void test_spec_table(void)
{
    size_t count = 0;
    const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *all =
        dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_pc34(&count);
    size_t i;

    expect_size("spec.count", A_F0122, count, 8);
    expect_int("spec.nonnull", A_F0122, all != NULL, 1);
    expect_int("spec.unknown.side",
               A_F0122,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_for_pc34(
                   (DM1_V1_D1L2D1R2StairsPitSidePc34)9,
                   DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34) == NULL,
               1);

    for (i = 0; i < count; ++i) {
        const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *s = &all[i];
        const int is_left = s->side == DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34;
        char id[96];

        snprintf(id, sizeof(id), "spec.%lu.contract", (unsigned long)i);
        expect_int(id, s->dispatch_anchor, s->contract_only, 1);
        snprintf(id, sizeof(id), "spec.%lu.no_asset", (unsigned long)i);
        expect_int(id, s->dispatch_anchor, s->real_asset_claim, 0);
        snprintf(id, sizeof(id), "spec.%lu.l2_perspective", (unsigned long)i);
        expect_int(id, s->dispatch_anchor, s->l2_perspective_index, 2);
        snprintf(id, sizeof(id), "spec.%lu.view_depth", (unsigned long)i);
        expect_int(id, s->dispatch_anchor, s->redmcsb_view_depth, 1);
        snprintf(id, sizeof(id), "spec.%lu.forward", (unsigned long)i);
        expect_int(id, A_F0128, s->relative_forward_step, 1);
        snprintf(id, sizeof(id), "spec.%lu.lateral", (unsigned long)i);
        expect_int(id, A_F0128, s->relative_lateral_step, is_left ? -1 : 1);
        snprintf(id, sizeof(id), "spec.%lu.dispatch_order", (unsigned long)i);
        expect_int(id, A_F0128, s->f0128_dispatch_order, is_left ? 10 : 20);
        snprintf(id, sizeof(id), "spec.%lu.view_square", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, s->view_square_index, is_left ? 4 : 5);
        snprintf(id, sizeof(id), "spec.%lu.cell_order", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, s->cell_order, is_left ? 0x0032 : 0x0041);
        snprintf(id, sizeof(id), "spec.%lu.f0104", (unsigned long)i);
        expect_int(id, A_F0104, s->uses_f0104_native, is_left);
        snprintf(id, sizeof(id), "spec.%lu.f0105", (unsigned long)i);
        expect_int(id, A_F0105, s->uses_f0105_flipped, !is_left);
        snprintf(id, sizeof(id), "spec.%lu.f0115", (unsigned long)i);
        expect_int(id, A_F0115, s->uses_f0115_thing_pass_followup, 1);
        snprintf(id, sizeof(id), "spec.%lu.post_follow", (unsigned long)i);
        expect_int(id, A_F0128, s->follows_with_d1c_d0l_d0r_d0c, 1);
        snprintf(id, sizeof(id), "spec.%lu.m609", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, s->followup_d0c_view_square_index, 0);
        snprintf(id, sizeof(id), "spec.%lu.c0021", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, s->followup_d0c_cell_order, 0x0021);
        snprintf(id, sizeof(id), "spec.%lu.d0c_f0115", (unsigned long)i);
        expect_int(id, A_F0127, s->followup_d0c_uses_f0115, 1);
        snprintf(id, sizeof(id), "spec.%lu.c10", (unsigned long)i);
        expect_int(id, A_DEFS_C10, s->transparent_color, 10);
        snprintf(id, sizeof(id), "spec.%lu.stairs_base", (unsigned long)i);
        expect_int(id, A_DEFS_BITMAPS, s->first_stairs_graphic_index, 108);
        snprintf(id, sizeof(id), "spec.%lu.draw_function", (unsigned long)i);
        expect_contains(id, side_anchor(s->side), s->draw_square_function,
                        is_left ? "F0122" : "F0123");
        snprintf(id, sizeof(id), "spec.%lu.draw_anchor", (unsigned long)i);
        expect_contains(id, draw_anchor(s->side), s->draw_anchor,
                        is_left ? "F0104" : "F0105");
        snprintf(id, sizeof(id), "spec.%lu.dungeon_anchor", (unsigned long)i);
        expect_contains(id, A_DUNGEON, s->dungeon_anchor, "F0172");
    }
}

static void test_route_constants_and_distinction(void)
{
    static const struct {
        DM1_V1_D1L2D1R2StairsPitSidePc34 side;
        DM1_V1_D1L2D1R2StairsPitRoutePc34 route;
        int element;
        int slot_or_graphic;
        int native_index;
        int zone;
        int view_square;
        int order;
        int f0104;
        int f0105;
    } cases[] = {
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 19, 4, 112, 808, 4, 0x0032, 1, 0 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 19, 11, 119, 821, 4, 0x0032, 1, 0 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 2, 54, 54, 858, 4, 0x0032, 1, 0 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 2, 60, 60, 858, 4, 0x0032, 1, 0 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 19, 4, 112, 810, 5, 0x0041, 0, 1 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 19, 11, 119, 823, 5, 0x0041, 0, 1 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 2, 54, 54, 860, 5, 0x0041, 0, 1 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 2, 60, 60, 860, 5, 0x0041, 0, 1 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const DM1_V1_D1L2D1R2StairsPitDispatchSpecPc34 *s =
            dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_spec_for_pc34(
                cases[i].side, cases[i].route);
        char id[96];

        snprintf(id, sizeof(id), "route.%lu.exists", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), s != NULL, 1);
        if (!s) continue;
        snprintf(id, sizeof(id), "route.%lu.element", (unsigned long)i);
        expect_int(id, A_DEFS_BITMAPS, s->element_class, cases[i].element);
        snprintf(id, sizeof(id), "route.%lu.slot", (unsigned long)i);
        expect_int(id, A_DEFS_BITMAPS, s->native_bitmap_slot_or_graphic,
                   cases[i].slot_or_graphic);
        snprintf(id, sizeof(id), "route.%lu.native", (unsigned long)i);
        expect_int(id, draw_anchor(cases[i].side), s->native_bitmap_index,
                   cases[i].native_index);
        snprintf(id, sizeof(id), "route.%lu.zone", (unsigned long)i);
        expect_int(id, A_DEFS_ZONES, s->zone_index, cases[i].zone);
        snprintf(id, sizeof(id), "route.%lu.view_square", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, s->view_square_index, cases[i].view_square);
        snprintf(id, sizeof(id), "route.%lu.order", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, s->cell_order, cases[i].order);
        snprintf(id, sizeof(id), "route.%lu.f0104", (unsigned long)i);
        expect_int(id, A_F0104, s->uses_f0104_native, cases[i].f0104);
        snprintf(id, sizeof(id), "route.%lu.f0105", (unsigned long)i);
        expect_int(id, A_F0105, s->uses_f0105_flipped, cases[i].f0105);
    }
}

static void test_resolve_paths(void)
{
    static const struct {
        DM1_V1_D1L2D1R2StairsPitSidePc34 side;
        int element;
        int stairs_up;
        int visible;
        DM1_V1_D1L2D1R2StairsPitRoutePc34 route;
        int native_index;
        int zone;
        int view_square;
        int order;
    } cases[] = {
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34, 19, 1, 0,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 112, 808, 4, 0x0032 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34, 19, 0, 0,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 119, 821, 4, 0x0032 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34, 2, 0, 0,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 54, 858, 4, 0x0032 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34, 2, 0, 1,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 60, 858, 4, 0x0032 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34, 19, 1, 0,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 112, 810, 5, 0x0041 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34, 19, 0, 0,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 119, 823, 5, 0x0041 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34, 2, 0, 0,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 54, 860, 5, 0x0041 },
        { DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34, 2, 0, 1,
          DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 60, 860, 5, 0x0041 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 context =
            context_for(cases[i].side, cases[i].element,
                        cases[i].stairs_up, cases[i].visible);
        DM1_V1_D1L2D1R2StairsPitDispatchResultPc34 out;
        char id[96];

        snprintf(id, sizeof(id), "resolve.%lu.call", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side),
                   dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
                       &context, &out),
                   1);
        snprintf(id, sizeof(id), "resolve.%lu.ok", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.ok, 1);
        snprintf(id, sizeof(id), "resolve.%lu.route", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.route, cases[i].route);
        snprintf(id, sizeof(id), "resolve.%lu.native", (unsigned long)i);
        expect_int(id, draw_anchor(cases[i].side), out.native_bitmap_index,
                   cases[i].native_index);
        snprintf(id, sizeof(id), "resolve.%lu.zone", (unsigned long)i);
        expect_int(id, A_DEFS_ZONES, out.zone_index, cases[i].zone);
        snprintf(id, sizeof(id), "resolve.%lu.view_square", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, out.view_square_index, cases[i].view_square);
        snprintf(id, sizeof(id), "resolve.%lu.order", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, out.cell_order, cases[i].order);
        snprintf(id, sizeof(id), "resolve.%lu.map_x", (unsigned long)i);
        expect_int(id, A_DUNGEON, out.map_x, context.map_x);
        snprintf(id, sizeof(id), "resolve.%lu.map_y", (unsigned long)i);
        expect_int(id, A_DUNGEON, out.map_y, context.map_y);
        snprintf(id, sizeof(id), "resolve.%lu.direction", (unsigned long)i);
        expect_int(id, A_DUNGEON, out.direction, 2);
        snprintf(id, sizeof(id), "resolve.%lu.l2", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.l2_perspective_index, 2);
        snprintf(id, sizeof(id), "resolve.%lu.depth", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.redmcsb_view_depth, 1);
        snprintf(id, sizeof(id), "resolve.%lu.f0115", (unsigned long)i);
        expect_int(id, A_F0115, out.used_f0115_thing_pass_followup, 1);
        snprintf(id, sizeof(id), "resolve.%lu.post", (unsigned long)i);
        expect_int(id, A_F0128, out.follows_with_d1c_d0l_d0r_d0c, 1);
        snprintf(id, sizeof(id), "resolve.%lu.m609", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, out.followup_d0c_view_square_index, 0);
        snprintf(id, sizeof(id), "resolve.%lu.c0021", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, out.followup_d0c_cell_order, 0x0021);
    }
}

static void test_rejections_and_unsupported(void)
{
    DM1_V1_D1L2D1R2StairsPitDispatchContextPc34 context =
        context_for(DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34, 19, 1, 0);
    DM1_V1_D1L2D1R2StairsPitDispatchResultPc34 result;

    expect_int("resolve.reject.null_input", A_F0122,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
                   NULL, &result), 0);
    expect_int("resolve.reject.null_output", A_F0122,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
                   &context, NULL), 0);
    context.contract_only = false;
    expect_int("resolve.reject.non_contract", A_F0122,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
                   &context, &result), 0);
    context.contract_only = true;
    context.real_asset_claim = true;
    expect_int("resolve.reject.real_asset", A_F0122,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
                   &context, &result), 0);

    context = context_for(DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34, 18, 1, 0);
    expect_int("resolve.unsupported.call", A_F0122,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_resolve_pc34(
                   &context, &result), 1);
    expect_int("resolve.unsupported.ok", A_F0122, result.ok, 1);
    expect_int("resolve.unsupported.flag", A_F0122, result.unsupported_element, 1);
    expect_int("resolve.unsupported.no_f0104", A_F0104, result.used_f0104_native, 0);
    expect_int("resolve.unsupported.no_f0105", A_F0105, result.used_f0105_flipped, 0);
}

static void test_pixel_runs(void)
{
    const uint8_t left_source[] = { 1, 10, 3, 4, 5, 6 };
    const uint8_t left_want[] = { 1, 0xee, 3, 0xee, 4, 5, 6, 0xee };
    const uint8_t right_source[] = { 1, 10, 3, 4, 5, 10 };
    const uint8_t right_want[] = { 3, 0xdd, 1, 0xdd, 0xdd, 5, 4, 0xdd };
    const uint8_t transparent_source[] = { 10, 10, 10, 10 };
    const uint8_t transparent_want[] = { 0xa0, 0xa1, 0xa2, 0xa3 };
    uint8_t destination[8];
    DM1_V1_D1L2D1R2StairsPitPixelRunInputPc34 input;
    DM1_V1_D1L2D1R2StairsPitPixelRunResultPc34 result;

    memset(destination, 0xee, sizeof(destination));
    input = pixel_input(DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
                        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
                        left_source, sizeof(left_source), destination,
                        sizeof(destination), 3, 2, 4);
    expect_int("pixel.left.call", A_F0104,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.left.bytes", A_F0104, destination, left_want,
                 sizeof(left_want));
    expect_int("pixel.left.f0104", A_F0104, result.used_f0104_native, 1);
    expect_int("pixel.left.f0105", A_F0105, result.used_f0105_flipped, 0);
    expect_size("pixel.left.writes", A_F0104, result.writes, 5);
    expect_size("pixel.left.skips", A_DEFS_C10, result.transparent_skips, 1);
    expect_int("pixel.left.first_dest", A_F0104, result.first_destination_byte, 1);
    expect_int("pixel.left.last_dest", A_F0104, result.last_destination_byte, 6);

    memset(destination, 0xdd, sizeof(destination));
    input = pixel_input(DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1R2_PC34,
                        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
                        right_source, sizeof(right_source), destination,
                        sizeof(destination), 3, 2, 4);
    expect_int("pixel.right.call", A_F0105,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.right.bytes", A_F0105, destination, right_want,
                 sizeof(right_want));
    expect_int("pixel.right.f0104", A_F0104, result.used_f0104_native, 0);
    expect_int("pixel.right.f0105", A_F0105, result.used_f0105_flipped, 1);
    expect_size("pixel.right.writes", A_F0105, result.writes, 4);
    expect_size("pixel.right.skips", A_DEFS_C10, result.transparent_skips, 2);
    expect_int("pixel.right.first_dest", A_F0105, result.first_destination_byte, 3);
    expect_int("pixel.right.last_dest", A_F0105, result.last_destination_byte, 4);

    memcpy(destination, transparent_want, sizeof(transparent_want));
    input = pixel_input(DM1_V1_D1L2_D1R2_STAIRS_PIT_SIDE_D1L2_PC34,
                        DM1_V1_D1L2_D1R2_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
                        transparent_source, sizeof(transparent_source),
                        destination, sizeof(transparent_want), 2, 2, 2);
    expect_int("pixel.transparent.call", A_DEFS_C10,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 1);
    expect_bytes("pixel.transparent.bytes", A_DEFS_C10, destination,
                 transparent_want, sizeof(transparent_want));
    expect_size("pixel.transparent.writes", A_DEFS_C10, result.writes, 0);
    expect_size("pixel.transparent.skips", A_DEFS_C10, result.transparent_skips, 4);
    expect_int("pixel.transparent.wrote_any", A_DEFS_C10, result.wrote_any, 0);

    expect_int("pixel.reject.null_input", A_F0104,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   NULL, &result), 0);
    expect_int("pixel.reject.null_output", A_F0104,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, NULL), 0);
    input.contract_only = false;
    expect_int("pixel.reject.non_contract", A_F0104,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
    input.contract_only = true;
    input.real_asset_claim = true;
    expect_int("pixel.reject.real_asset", A_F0104,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
    input.real_asset_claim = false;
    input.destination_stride = 1;
    expect_int("pixel.reject.stride", A_F0104,
               dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &result), 0);
}

int main(void)
{
    test_anchor_table_and_source_summary();
    test_spec_table();
    test_route_constants_and_distinction();
    test_resolve_paths();
    test_rejections_and_unsupported();
    test_pixel_runs();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1l2_d1r2_stairs_pit_dispatch_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
