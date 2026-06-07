#include "dm1_v1_viewport_d1l_d1r_stairs_pit_dispatch_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define A_F0122 "DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L:7391-7557"
#define A_F0123 "DUNVIEW.C:F0123_DUNGEONVIEW_DrawSquareD1R:7559-7725"
#define A_F0104 "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156"
#define A_F0105 "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3158-3188"
#define A_DEFS_ELEMENTS "DEFS.H:1009/1016/1017"
#define A_DEFS_SLOTS "DEFS.H:2445/2452"
#define A_DEFS_VIEW "DEFS.H:2600-2601"
#define A_DEFS_ORDER "DEFS.H:2664/2666"
#define A_DEFS_ZONES "DEFS.H:4147/4149/4160/4162/4205/4207"
#define A_DEFS_C10 "DEFS.H:2088"
#define A_DUNGEON_ASPECT "DUNGEON.C:F0172_DUNGEON_SetSquareAspect:2466-2523"
#define A_DUNGEON_THINGS "DUNGEON.C:F0163/F0164"

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

static const char *side_anchor(DM1_V1_D1LD1RStairsPitSidePc34 side)
{
    return side == DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34 ? A_F0122 : A_F0123;
}

static const char *draw_anchor(DM1_V1_D1LD1RStairsPitSidePc34 side)
{
    return side == DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34 ? A_F0104 : A_F0105;
}

static DM1_V1_D1LD1RStairsPitDispatchResultPc34 render(
    DM1_V1_D1LD1RStairsPitDispatchContextPc34 context,
    const char *id)
{
    DM1_V1_D1LD1RStairsPitDispatchResultPc34 result;
    expect_int(id, side_anchor(context.side),
               M11_GameView_ViewportD1LD1RStairsPitDispatch_RenderPc34(
                   &context, &result),
               1);
    return result;
}

static DM1_V1_D1LD1RStairsPitDispatchContextPc34 context_for(
    DM1_V1_D1LD1RStairsPitSidePc34 side,
    int element,
    int stairs_up,
    int visible)
{
    DM1_V1_D1LD1RStairsPitDispatchContextPc34 context;
    M11_GameView_ViewportD1LD1RStairsPitDispatch_InitContextPc34(&context, side);
    context.direction = 2;
    context.map_x = side == DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34 ? 17 : 19;
    context.map_y = side == DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34 ? 23 : 29;
    context.element_class = element;
    context.stairs_up = stairs_up != 0;
    context.pit_or_teleporter_visible = visible != 0;
    return context;
}

static DM1_V1_D1LD1RStairsPitBlitInputPc34 blit_input(
    DM1_V1_D1LD1RStairsPitSidePc34 side,
    DM1_V1_D1LD1RStairsPitRoutePc34 route,
    const uint8_t *source,
    size_t source_len,
    uint8_t *destination,
    size_t destination_len,
    size_t row_width,
    size_t height,
    size_t destination_stride)
{
    DM1_V1_D1LD1RStairsPitBlitInputPc34 input;
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

static void test_evidence_table(void)
{
    size_t count = 0;
    const DM1_V1_D1LD1RStairsPitEvidencePc34 *all =
        M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidencePc34(&count);
    size_t i;

    expect_size("evidence.count", A_F0122, count, 8);
    expect_int("evidence.nonnull", A_F0122, all != NULL, 1);
    expect_int("evidence.d1l.up.exists", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
                   DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
                   DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34) != NULL, 1);
    expect_int("evidence.d1r.pit.exists", A_F0123,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
                   DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
                   DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34) != NULL, 1);
    expect_int("evidence.unknown.missing", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
                   (DM1_V1_D1LD1RStairsPitSidePc34)99,
                   DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34) == NULL, 1);

    for (i = 0; i < count; ++i) {
        const DM1_V1_D1LD1RStairsPitEvidencePc34 *e = &all[i];
        const int is_d1l = e->side == DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34;
        char id[80];

        snprintf(id, sizeof(id), "evidence.%lu.contract", (unsigned long)i);
        expect_int(id, e->dispatch_anchor, e->contract_only, 1);
        snprintf(id, sizeof(id), "evidence.%lu.no_real_asset", (unsigned long)i);
        expect_int(id, e->dispatch_anchor, e->real_asset_claim, 0);
        snprintf(id, sizeof(id), "evidence.%lu.no_f0111", (unsigned long)i);
        expect_int(id, e->dispatch_anchor, e->uses_f0111, 0);
        snprintf(id, sizeof(id), "evidence.%lu.no_f0115", (unsigned long)i);
        expect_int(id, e->dispatch_anchor, e->uses_f0115_thing_pass, 0);
        snprintf(id, sizeof(id), "evidence.%lu.d1l_f0104", (unsigned long)i);
        expect_int(id, draw_anchor(e->side), e->uses_f0104, is_d1l);
        snprintf(id, sizeof(id), "evidence.%lu.d1r_f0105", (unsigned long)i);
        expect_int(id, draw_anchor(e->side), e->uses_f0105_flipped, !is_d1l);
        snprintf(id, sizeof(id), "evidence.%lu.transparent", (unsigned long)i);
        expect_int(id, A_DEFS_C10, e->transparent_color, 10);
        snprintf(id, sizeof(id), "evidence.%lu.anchor_side", (unsigned long)i);
        expect_contains(id, side_anchor(e->side), e->dispatch_anchor,
                        is_d1l ? "F0122" : "F0123");
    }
}

static void test_route_constants(void)
{
    static const struct {
        DM1_V1_D1LD1RStairsPitSidePc34 side;
        DM1_V1_D1LD1RStairsPitRoutePc34 route;
        int slot_or_graphic;
        int native_index;
        int zone;
        int view_square;
        int cell_order;
    } cases[] = {
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 4, 112, 808, 4, 0x0032 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 11, 119, 821, 4, 0x0032 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 54, 54, 858, 4, 0x0032 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 60, 60, 858, 4, 0x0032 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 4, 112, 810, 5, 0x0041 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 11, 119, 823, 5, 0x0041 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 54, 54, 860, 5, 0x0041 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 60, 60, 860, 5, 0x0041 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const DM1_V1_D1LD1RStairsPitEvidencePc34 *e =
            M11_GameView_ViewportD1LD1RStairsPitDispatch_EvidenceForPc34(
                cases[i].side, cases[i].route);
        char id[80];

        snprintf(id, sizeof(id), "route.%lu.exists", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), e != NULL, 1);
        if (!e) continue;
        snprintf(id, sizeof(id), "route.%lu.slot_or_graphic", (unsigned long)i);
        expect_int(id, A_DEFS_SLOTS, e->native_bitmap_slot_or_graphic,
                   cases[i].slot_or_graphic);
        snprintf(id, sizeof(id), "route.%lu.native", (unsigned long)i);
        expect_int(id, e->draw_anchor, e->native_bitmap_index, cases[i].native_index);
        snprintf(id, sizeof(id), "route.%lu.zone", (unsigned long)i);
        expect_int(id, A_DEFS_ZONES, e->zone_index, cases[i].zone);
        snprintf(id, sizeof(id), "route.%lu.view_square", (unsigned long)i);
        expect_int(id, A_DEFS_VIEW, e->view_square_index, cases[i].view_square);
        snprintf(id, sizeof(id), "route.%lu.cell_order", (unsigned long)i);
        expect_int(id, A_DEFS_ORDER, e->cell_order, cases[i].cell_order);
        snprintf(id, sizeof(id), "route.%lu.element", (unsigned long)i);
        expect_int(id, A_DEFS_ELEMENTS, e->element_class,
                   cases[i].route == DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34 ||
                   cases[i].route == DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34 ?
                   19 : 2);
    }
}

static void test_render_routes(void)
{
    static const struct {
        DM1_V1_D1LD1RStairsPitSidePc34 side;
        int element;
        int stairs_up;
        int visible;
        DM1_V1_D1LD1RStairsPitRoutePc34 route;
        int native_index;
        int zone;
        int f0104;
        int f0105;
    } cases[] = {
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34, 19, 1, 0,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 112, 808, 1, 0 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34, 19, 0, 0,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 119, 821, 1, 0 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34, 2, 0, 0,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 54, 858, 1, 0 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34, 2, 0, 1,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 60, 858, 1, 0 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34, 19, 1, 0,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34, 112, 810, 0, 1 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34, 19, 0, 0,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34, 119, 823, 0, 1 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34, 2, 0, 0,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 54, 860, 0, 1 },
        { DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34, 2, 0, 1,
          DM1_V1_D1LR_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 60, 860, 0, 1 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM1_V1_D1LD1RStairsPitDispatchContextPc34 context =
            context_for(cases[i].side, cases[i].element,
                        cases[i].stairs_up, cases[i].visible);
        DM1_V1_D1LD1RStairsPitDispatchResultPc34 out =
            render(context, "render.route.ok");
        char id[80];

        snprintf(id, sizeof(id), "render.%lu.ok", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.ok, 1);
        snprintf(id, sizeof(id), "render.%lu.route", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.route, cases[i].route);
        snprintf(id, sizeof(id), "render.%lu.native", (unsigned long)i);
        expect_int(id, draw_anchor(cases[i].side), out.native_bitmap_index,
                   cases[i].native_index);
        snprintf(id, sizeof(id), "render.%lu.zone", (unsigned long)i);
        expect_int(id, A_DEFS_ZONES, out.zone_index, cases[i].zone);
        snprintf(id, sizeof(id), "render.%lu.map_x", (unsigned long)i);
        expect_int(id, A_DUNGEON_ASPECT, out.map_x, context.map_x);
        snprintf(id, sizeof(id), "render.%lu.map_y", (unsigned long)i);
        expect_int(id, A_DUNGEON_ASPECT, out.map_y, context.map_y);
        snprintf(id, sizeof(id), "render.%lu.direction", (unsigned long)i);
        expect_int(id, A_DUNGEON_ASPECT, out.direction, 2);
        snprintf(id, sizeof(id), "render.%lu.f0104", (unsigned long)i);
        expect_int(id, A_F0104, out.used_f0104, cases[i].f0104);
        snprintf(id, sizeof(id), "render.%lu.f0105", (unsigned long)i);
        expect_int(id, A_F0105, out.used_f0105_flipped, cases[i].f0105);
        snprintf(id, sizeof(id), "render.%lu.no_f0111", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.used_f0111, 0);
        snprintf(id, sizeof(id), "render.%lu.no_f0115", (unsigned long)i);
        expect_int(id, side_anchor(cases[i].side), out.used_f0115_thing_pass, 0);
    }
}

static void test_rejections_and_unsupported(void)
{
    DM1_V1_D1LD1RStairsPitDispatchContextPc34 context =
        context_for(DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34, 19, 1, 0);
    DM1_V1_D1LD1RStairsPitDispatchResultPc34 result;

    expect_int("render.reject.null_input", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_RenderPc34(NULL, &result), 0);
    expect_int("render.reject.null_output", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_RenderPc34(&context, NULL), 0);
    context.contract_only = false;
    expect_int("render.reject.non_contract", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_RenderPc34(&context, &result), 0);
    context.contract_only = true;
    context.real_asset_claim = true;
    expect_int("render.reject.real_asset", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_RenderPc34(&context, &result), 0);

    result = render(context_for(DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34, 18, 1, 0),
                    "render.unsupported.stairs_side");
    expect_int("render.unsupported.stairs_side.flag", A_DEFS_ELEMENTS,
               result.unsupported_element, 1);
    expect_int("render.unsupported.stairs_side.no_f0104", A_F0104, result.used_f0104, 0);

    result = render(context_for(DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34, 5, 0, 1),
                    "render.unsupported.teleporter");
    expect_int("render.unsupported.teleporter.flag", A_DEFS_ELEMENTS,
               result.unsupported_element, 1);
    expect_int("render.unsupported.teleporter.no_f0105", A_F0105,
               result.used_f0105_flipped, 0);
}

static void test_blit_contracts(void)
{
    const uint8_t left_source[] = { 1, 10, 3, 4, 5, 6 };
    const uint8_t left_want[] = { 1, 0xee, 3, 0xee, 4, 5, 6, 0xee };
    const uint8_t right_source[] = { 1, 10, 3, 4, 5, 10 };
    const uint8_t right_want[] = { 3, 0xdd, 1, 0xdd, 0xdd, 5, 4, 0xdd };
    uint8_t destination[8];
    DM1_V1_D1LD1RStairsPitBlitInputPc34 input;
    DM1_V1_D1LD1RStairsPitBlitResultPc34 result;

    memset(destination, 0xee, sizeof(destination));
    input = blit_input(DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
                       DM1_V1_D1LR_STAIRS_PIT_ROUTE_UP_FRONT_PC34,
                       left_source, sizeof(left_source), destination,
                       sizeof(destination), 3, 2, 4);
    expect_int("blit.d1l.ok", A_F0104,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(&input, &result), 1);
    expect_bytes("blit.d1l.bytes", A_F0104, destination, left_want, sizeof(left_want));
    expect_int("blit.d1l.not_flipped", A_F0104, result.flipped_horizontally, 0);
    expect_size("blit.d1l.writes", A_F0104, result.writes, 5);
    expect_size("blit.d1l.skips", A_DEFS_C10, result.transparent_skips, 1);
    expect_int("blit.d1l.first_destination", A_F0104, result.first_destination_byte, 1);
    expect_int("blit.d1l.last_destination", A_F0104, result.last_destination_byte, 6);

    memset(destination, 0xdd, sizeof(destination));
    input = blit_input(DM1_V1_D1LR_STAIRS_PIT_SIDE_D1R_PC34,
                       DM1_V1_D1LR_STAIRS_PIT_ROUTE_OPEN_PIT_PC34,
                       right_source, sizeof(right_source), destination,
                       sizeof(destination), 3, 2, 4);
    expect_int("blit.d1r.ok", A_F0105,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(&input, &result), 1);
    expect_bytes("blit.d1r.bytes", A_F0105, destination, right_want, sizeof(right_want));
    expect_int("blit.d1r.flipped", A_F0105, result.flipped_horizontally, 1);
    expect_size("blit.d1r.writes", A_F0105, result.writes, 4);
    expect_size("blit.d1r.skips", A_DEFS_C10, result.transparent_skips, 2);
    expect_int("blit.d1r.first_destination", A_F0105, result.first_destination_byte, 3);
    expect_int("blit.d1r.last_destination", A_F0105, result.last_destination_byte, 4);
}

static void test_blit_edges_and_assert_helper(void)
{
    const uint8_t transparent_source[] = { 10, 10, 10, 10 };
    const uint8_t original_destination[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    uint8_t destination[] = { 0xaa, 0xbb, 0xcc, 0xdd };
    DM1_V1_D1LD1RStairsPitBlitInputPc34 input =
        blit_input(DM1_V1_D1LR_STAIRS_PIT_SIDE_D1L_PC34,
                   DM1_V1_D1LR_STAIRS_PIT_ROUTE_DOWN_FRONT_PC34,
                   transparent_source, sizeof(transparent_source),
                   destination, sizeof(destination), 2, 2, 2);
    DM1_V1_D1LD1RStairsPitBlitResultPc34 result;
    DM1_V1_D1LD1RStairsPitAssertResultPc34 assert_result;

    expect_int("blit.transparent.ok", A_DEFS_C10,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(&input, &result), 1);
    expect_bytes("blit.transparent.unchanged", A_DEFS_C10,
                 destination, original_destination, sizeof(destination));
    expect_size("blit.transparent.writes", A_DEFS_C10, result.writes, 0);
    expect_size("blit.transparent.skips", A_DEFS_C10, result.transparent_skips, 4);
    expect_int("blit.transparent.wrote_any", A_DEFS_C10, result.wrote_any, 0);

    expect_int("blit.reject.null_input", A_F0104,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(NULL, &result), 0);
    expect_int("blit.reject.null_output", A_F0104,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(&input, NULL), 0);
    input.contract_only = false;
    expect_int("blit.reject.non_contract", A_F0104,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(&input, &result), 0);
    input.contract_only = true;
    input.real_asset_claim = true;
    expect_int("blit.reject.real_asset", A_F0104,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(&input, &result), 0);
    input.real_asset_claim = false;
    input.destination_stride = 1;
    expect_int("blit.reject.short_stride", A_F0104,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_BlitPc34(&input, &result), 0);

    expect_int("assert_helper.ok", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_AssertPc34(
                   &assert_result), 1);
    expect_int("assert_helper.failures", A_F0122, assert_result.failures, 0);
    expect_int("assert_helper.count", A_F0122, assert_result.expected_assertions, 8);
    expect_int("assert_helper.table", A_F0122, assert_result.evidence_table_complete, 1);
    expect_int("assert_helper.d1l_f0104", A_F0104, assert_result.d1l_uses_f0104, 1);
    expect_int("assert_helper.d1r_f0105", A_F0105, assert_result.d1r_uses_f0105, 1);
    expect_int("assert_helper.no_f0111", A_F0122, assert_result.no_f0111, 1);
    expect_int("assert_helper.no_f0115", A_F0123, assert_result.no_f0115_thing_pass, 1);
    expect_int("assert_helper.null", A_F0122,
               M11_GameView_ViewportD1LD1RStairsPitDispatch_AssertPc34(NULL), 0);
}

static void test_source_evidence(void)
{
    const char *summary =
        M11_GameView_ViewportD1LD1RStairsPitDispatch_SourceEvidencePc34();

    expect_contains("summary.contract", A_F0122, summary, "contract_only=1");
    expect_contains("summary.no_real_asset", A_F0104, summary, "no real-asset");
    expect_contains("summary.f0122", A_F0122, summary, "7391-7557");
    expect_contains("summary.f0123", A_F0123, summary, "7559-7725");
    expect_contains("summary.f0104", A_F0104, summary, "F0104");
    expect_contains("summary.f0105", A_F0105, summary, "F0105");
    expect_contains("summary.c04", A_DEFS_SLOTS, summary, "C04");
    expect_contains("summary.c11", A_DEFS_SLOTS, summary, "C11");
    expect_contains("summary.c808", A_DEFS_ZONES, summary, "C808");
    expect_contains("summary.c810", A_DEFS_ZONES, summary, "C810");
    expect_contains("summary.c821", A_DEFS_ZONES, summary, "C821");
    expect_contains("summary.c823", A_DEFS_ZONES, summary, "C823");
    expect_contains("summary.c858", A_DEFS_ZONES, summary, "C858");
    expect_contains("summary.c860", A_DEFS_ZONES, summary, "C860");
    expect_contains("summary.c02", A_DEFS_ELEMENTS, summary, "C02");
    expect_contains("summary.c18", A_DEFS_ELEMENTS, summary, "C18");
    expect_contains("summary.c19", A_DEFS_ELEMENTS, summary, "C19");
    expect_contains("summary.m607", A_DEFS_VIEW, summary, "M607");
    expect_contains("summary.m608", A_DEFS_VIEW, summary, "M608");
    expect_contains("summary.f0163", A_DUNGEON_THINGS, summary, "F0163");
    expect_contains("summary.f0164", A_DUNGEON_THINGS, summary, "F0164");
    expect_contains("summary.f0172", A_DUNGEON_ASPECT, summary, "F0172");
    expect_contains("summary.no_f0111", A_F0122, summary, "excludes F0111");
    expect_contains("summary.no_f0115", A_F0123, summary, "F0115 thing-pass");
}

int main(void)
{
    test_evidence_table();
    test_route_constants();
    test_render_routes();
    test_rejections_and_unsupported();
    test_blit_contracts();
    test_blit_edges_and_assert_helper();
    test_source_evidence();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1l_d1r_stairs_pit_dispatch_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
