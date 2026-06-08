#include "dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_size(const char *id, size_t got, size_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%lu want=%lu anchor=%s\n",
               id, (unsigned long)got, (unsigned long)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %lu anchor=%s\n", id, (unsigned long)want, anchor);
    }
}

static void expect_contains(const char *id, const char *text, const char *needle,
                            const char *anchor)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void expect_bytes(const char *id, const uint8_t *got, const uint8_t *want,
                         size_t count, const char *anchor)
{
    size_t i;

    ++g_assertions;
    for (i = 0; i < count; ++i) {
        if (got[i] != want[i]) {
            printf("FAIL %s index=%lu got=0x%02x want=0x%02x anchor=%s\n",
                   id, (unsigned long)i, got[i], want[i], anchor);
            ++g_failures;
            return;
        }
    }
    printf("PASS %s bytes=%lu anchor=%s\n", id, (unsigned long)count, anchor);
}

static void test_source_summary(void)
{
    const char *e =
        dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_source_evidence_pc34();

    expect_contains("e.contract", e, "contract_only=1", "source evidence");
    expect_contains("e.no_asset", e, "no real-asset", "source evidence");
    expect_contains("e.f0125", e, "F0125:7960-8062", "DUNVIEW.C F0125");
    expect_contains("e.f0126", e, "F0126:8064-8162", "DUNVIEW.C F0126");
    expect_contains("e.f0104", e, "F0104:3113-3156", "DUNVIEW.C F0104");
    expect_contains("e.f0105", e, "F0105:3185-3247", "DUNVIEW.C F0105");
    expect_contains("e.f0128", e, "F0128:8534-8542", "DUNVIEW.C F0128");
    expect_contains("e.f0172", e, "F0172:2466-2523", "DUNGEON.C F0172");
    expect_contains("e.c832", e, "C832", "DEFS.H stairs-side zones");
    expect_contains("e.c863", e, "C863", "DEFS.H pit zones");
    expect_contains("e.c870", e, "C870", "DEFS.H ceiling-pit zones");
    expect_contains("e.wall_nondup", e, "excludes C00 wall-return", "non-overlap");
    expect_contains("e.stairs_return", e, "stairs-side early return", "non-overlap");
}

static void test_specs(void)
{
    size_t count = 0;
    const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *specs =
        dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_specs_pc34(&count);
    size_t i;

    expect_size("spec.count", count, 6, "D0L2/D0R2 x stairs/open/invisible");
    expect_int("spec.pointer", specs != NULL, 1, "spec table");

    for (i = 0; i < count; ++i) {
        const DM1_V1_D0L2D0R2StairsPitDispatchSpecPc34 *s = &specs[i];
        const int is_left = s->side == DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34;
        const int is_stairs =
            s->route == DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_STAIRS_SIDE_PC34;
        char id[96];

        snprintf(id, sizeof(id), "spec.%lu.contract", (unsigned long)i);
        expect_int(id, s->contract_only, 1, s->dispatch_anchor);
        snprintf(id, sizeof(id), "spec.%lu.no_asset", (unsigned long)i);
        expect_int(id, s->real_asset_claim, 0, s->dispatch_anchor);
        snprintf(id, sizeof(id), "spec.%lu.view_square", (unsigned long)i);
        expect_int(id, s->view_square_index, is_left ? 1 : 2, "DEFS.H:2597-2598");
        snprintf(id, sizeof(id), "spec.%lu.cell_order", (unsigned long)i);
        expect_int(id, s->cell_order, is_left ? 0x0002 : 0x0001, "DEFS.H:2658-2663");
        snprintf(id, sizeof(id), "spec.%lu.zone", (unsigned long)i);
        expect_int(id, s->zone_index,
                   is_stairs ? (is_left ? 832 : 833) : (is_left ? 861 : 863),
                   "DEFS.H:4169-4170/4210-4212");
        snprintf(id, sizeof(id), "spec.%lu.f0104", (unsigned long)i);
        expect_int(id, s->uses_f0104_native, is_left, "DUNVIEW.C F0104");
        snprintf(id, sizeof(id), "spec.%lu.f0105", (unsigned long)i);
        expect_int(id, s->uses_f0105_flipped, !is_left, "DUNVIEW.C F0105");
        snprintf(id, sizeof(id), "spec.%lu.ceiling_tail", (unsigned long)i);
        expect_int(id, s->uses_ceiling_pit_tail, !is_stairs, "DUNVIEW.C F0125/F0126 tail");
        snprintf(id, sizeof(id), "spec.%lu.f0115_tail", (unsigned long)i);
        expect_int(id, s->uses_f0115_thing_pass_tail, !is_stairs, "DUNVIEW.C F0115 tail");
        snprintf(id, sizeof(id), "spec.%lu.early_return", (unsigned long)i);
        expect_int(id, s->returns_before_ceiling_pit, is_stairs, "C18 early return");
        snprintf(id, sizeof(id), "spec.%lu.no_wall", (unsigned long)i);
        expect_int(id, s->excludes_wall_return, 1, "C00 wall-return excluded");
    }
}

static void test_resolve(void)
{
    static const struct {
        DM1_V1_D0L2D0R2StairsPitSidePc34 side;
        int element;
        int visible;
        DM1_V1_D0L2D0R2StairsPitRoutePc34 route;
        int graphic;
        int zone;
        int view_square;
        int f0115;
    } cases[] = {
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34, 18, 0,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_STAIRS_SIDE_PC34, 17, 832, 1, 0 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34, 2, 0,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 56, 861, 1, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34, 2, 1,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 62, 861, 1, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34, 18, 0,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_STAIRS_SIDE_PC34, 17, 833, 2, 0 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34, 2, 0,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34, 56, 863, 2, 1 },
        { DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34, 2, 1,
          DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_INVISIBLE_PIT_PC34, 62, 863, 2, 1 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM1_V1_D0L2D0R2StairsPitDispatchContextPc34 context;
        DM1_V1_D0L2D0R2StairsPitDispatchResultPc34 out;
        char id[96];

        dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_init_context_pc34(
            &context, cases[i].side);
        context.element_class = cases[i].element;
        context.pit_or_teleporter_visible = cases[i].visible != 0;

        snprintf(id, sizeof(id), "resolve.%lu.call", (unsigned long)i);
        expect_int(id,
                   dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_resolve_pc34(
                       &context, &out),
                   1, "resolve call");
        snprintf(id, sizeof(id), "resolve.%lu.ok", (unsigned long)i);
        expect_int(id, out.ok, 1, "resolve ok");
        snprintf(id, sizeof(id), "resolve.%lu.route", (unsigned long)i);
        expect_int(id, out.route, cases[i].route, "route selection");
        snprintf(id, sizeof(id), "resolve.%lu.graphic", (unsigned long)i);
        expect_int(id, out.native_bitmap_slot_or_graphic, cases[i].graphic,
                   "DEFS.H bitmap slot/graphic");
        snprintf(id, sizeof(id), "resolve.%lu.zone", (unsigned long)i);
        expect_int(id, out.zone_index, cases[i].zone, "DEFS.H zone");
        snprintf(id, sizeof(id), "resolve.%lu.view", (unsigned long)i);
        expect_int(id, out.view_square_index, cases[i].view_square, "DEFS.H view square");
        snprintf(id, sizeof(id), "resolve.%lu.f0115", (unsigned long)i);
        expect_int(id, out.used_f0115_thing_pass_tail, cases[i].f0115,
                   "DUNVIEW.C F0115 tail");
    }
}

static void test_rejections_and_pixels(void)
{
    const uint8_t left_source[] = { 1, 10, 3, 4, 5, 6 };
    const uint8_t left_want[] = { 1, 0xee, 3, 0xee, 4, 5, 6, 0xee };
    const uint8_t right_source[] = { 1, 10, 3, 4, 5, 10 };
    const uint8_t right_want[] = { 3, 0xdd, 1, 0xdd, 0xdd, 5, 4, 0xdd };
    uint8_t destination[8];
    DM1_V1_D0L2D0R2StairsPitPixelRunInputPc34 input;
    DM1_V1_D0L2D0R2StairsPitPixelRunResultPc34 out;

    expect_int("resolve.reject.null_input",
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_resolve_pc34(
                   NULL, (DM1_V1_D0L2D0R2StairsPitDispatchResultPc34 *)&out),
               0, "null input rejected");

    memset(destination, 0xee, sizeof(destination));
    memset(&input, 0, sizeof(input));
    input.side = DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0L2_PC34;
    input.route = DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34;
    input.source = left_source;
    input.source_len = sizeof(left_source);
    input.destination = destination;
    input.destination_len = sizeof(destination);
    input.row_width = 3;
    input.height = 2;
    input.destination_stride = 4;
    input.contract_only = true;

    expect_int("pixel.left.call",
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &out),
               1, "DUNVIEW.C F0104 C10");
    expect_bytes("pixel.left.bytes", destination, left_want, sizeof(left_want),
                 "native F0104 preserves C10");
    expect_size("pixel.left.writes", out.writes, 5, "F0104 writes");
    expect_size("pixel.left.skips", out.transparent_skips, 1, "C10 skips");
    expect_int("pixel.left.f0104", out.used_f0104_native, 1, "F0104 native");
    expect_int("pixel.left.f0105", out.used_f0105_flipped, 0, "not flipped");

    memset(destination, 0xdd, sizeof(destination));
    input.side = DM1_V1_D0L2_D0R2_STAIRS_PIT_SIDE_D0R2_PC34;
    input.route = DM1_V1_D0L2_D0R2_STAIRS_PIT_ROUTE_OPEN_PIT_PC34;
    input.source = right_source;
    input.source_len = sizeof(right_source);
    expect_int("pixel.right.call",
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &out),
               1, "DUNVIEW.C F0105 C10");
    expect_bytes("pixel.right.bytes", destination, right_want, sizeof(right_want),
                 "flipped F0105 preserves C10");
    expect_size("pixel.right.writes", out.writes, 4, "F0105 writes");
    expect_size("pixel.right.skips", out.transparent_skips, 2, "C10 skips");
    expect_int("pixel.right.f0104", out.used_f0104_native, 0, "not native");
    expect_int("pixel.right.f0105", out.used_f0105_flipped, 1, "F0105 flipped");

    input.contract_only = false;
    expect_int("pixel.reject.non_contract",
               dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pixel_run_pc34(
                   &input, &out),
               0, "contract-only guard");
}

int main(void)
{
    test_source_summary();
    test_specs();
    test_resolve();
    test_rejections_and_pixels();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0l2_d0r2_stairs_pit_dispatch_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
