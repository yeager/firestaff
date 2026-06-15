/*
 * DM1 V1 D0L/D0R wall row-composition source-lock gate.
 *
 * ReDMCSB anchors:
 * - DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport lines 709-722 and 821-857
 *   keep palette/blit state deterministic around the composed viewport.
 * - VIEWPORT.C F0564/F0565/F0566 lines 15-63 bind the 224x136 viewport
 *   bitplanes, palette switch, and full viewport blit invariants.
 * - DUNVIEW.C F0128 lines 8534-8541 dispatch D0L before D0R, then D0C.
 * - DUNVIEW.C F0125/F0126 lines 8007-8038 and 8117-8144 bind WALL cases
 *   to C716/C717 and return before F0115/F0111/F0108.
 * - DEFS.H lines 2597-2598 and 4056-4057 bind M610/M611 to C716/C717.
 */
#include "dm1_v1_viewport_d0l_d0r_wall_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    DM1_V1_D0_ROW_FIXTURE_Y_PC34 = 67,
    DM1_V1_D0_ROW_PALETTE_INDEX_PC34 = 0,
    DM1_V1_D0_ROW_LIGHT_LEVEL_PC34 = 0,
    DM1_V1_D0_ROW_TILE_WALL_PC34 = 0,
    DM1_V1_D0_ROW_TILE_CORRIDOR_PC34 = 1
};

typedef struct {
    int d0l_tile;
    int d0r_tile;
    int d0c_tile;
    int row_y;
    int palette_index;
    int light_level;
    uint8_t transparent_color;
} DM1V1D0RowWallFixturePc34;

static int g_assertions = 0;
static int g_failures = 0;

static const char *k_row_source_lock =
    "DRAWVIEW.C:709-722,821-857 F0097_DUNGEONVIEW_DrawViewport "
    "preserves deterministic viewport/palette handoff; VIEWPORT.C:15-63 "
    "F0564/F0565/F0566 bind 224x136 bitplanes, palette switch, and blit; "
    "DUNVIEW.C:8534-8541 F0128 dispatches D0L then D0R; "
    "DUNVIEW.C:8007-8038 F0125 D0L wall draws C716 and returns; "
    "DUNVIEW.C:8117-8144 F0126 D0R wall draws C717 and returns; "
    "DEFS.H:2597-2598 M610/M611 and DEFS.H:4056-4057 C716/C717.";

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

static void expect_contains(
    const char *id,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static size_t viewport_offset(int y, int x)
{
    return (size_t)y * DM1_V1_D0L_D0R_WALL_VIEWPORT_WIDTH_PC34 + (size_t)x;
}

static size_t source_offset(int y, int x)
{
    return (size_t)y * DM1_V1_D0L_D0R_WALL_SOURCE_WIDTH_PC34 + (size_t)x;
}

static void compose_d0_wall_row(
    const DM1V1D0RowWallFixturePc34 *fixture,
    const uint8_t *wall_source,
    size_t wall_source_len,
    uint8_t *viewport,
    size_t viewport_len)
{
    DM1_V1_D0LD0RWallInputPc34 d0l;
    DM1_V1_D0LD0RWallInputPc34 d0r;
    DM1_V1_D0LD0RWallPixelPc34 pixel;
    int x;

    d0l.route = DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34;
    d0l.row = fixture->row_y;
    d0l.transparent_color = fixture->transparent_color;
    d0r.route = DM1_V1_D0L_D0R_WALL_ROUTE_D0R_PARITY_PC34;
    d0r.row = fixture->row_y;
    d0r.transparent_color = fixture->transparent_color;

    /*
     * ReDMCSB: DUNVIEW.C:8534-8541 F0128 updates the relative D0L square
     * and draws F0125 before updating/drawing D0R through F0126.
     */
    if (fixture->d0l_tile == DM1_V1_D0_ROW_TILE_WALL_PC34) {
        for (x = 0; x < 64; ++x) {
            (void)M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                &d0l, wall_source, wall_source_len, viewport, viewport_len, x, &pixel);
        }
    }
    if (fixture->d0r_tile == DM1_V1_D0_ROW_TILE_WALL_PC34) {
        for (x = 0; x < 64; ++x) {
            (void)M11_GameView_D0LD0RWallApplyPixelSlicePc34(
                &d0r, wall_source, wall_source_len, viewport, viewport_len, x, &pixel);
        }
    }
}

static void test_row_fixture_metadata_and_routes(void)
{
    const DM1V1D0RowWallFixturePc34 fixture = {
        DM1_V1_D0_ROW_TILE_WALL_PC34,
        DM1_V1_D0_ROW_TILE_WALL_PC34,
        DM1_V1_D0_ROW_TILE_CORRIDOR_PC34,
        DM1_V1_D0_ROW_FIXTURE_Y_PC34,
        DM1_V1_D0_ROW_PALETTE_INDEX_PC34,
        DM1_V1_D0_ROW_LIGHT_LEVEL_PC34,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallInputPc34 d0l = {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0L_NATIVE_PC34,
        DM1_V1_D0_ROW_FIXTURE_Y_PC34,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallInputPc34 d0r = {
        DM1_V1_D0L_D0R_WALL_ROUTE_D0R_PARITY_PC34,
        DM1_V1_D0_ROW_FIXTURE_Y_PC34,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    DM1_V1_D0LD0RWallSpecPc34 d0l_spec;
    DM1_V1_D0LD0RWallSpecPc34 d0r_spec;

    expect_int("fixture.d0l_wall", fixture.d0l_tile, DM1_V1_D0_ROW_TILE_WALL_PC34,
               "DUNVIEW.C:8007-8038 D0L WALL case");
    expect_int("fixture.d0r_wall", fixture.d0r_tile, DM1_V1_D0_ROW_TILE_WALL_PC34,
               "DUNVIEW.C:8117-8144 D0R WALL case");
    expect_int("fixture.palette", fixture.palette_index, 0,
               "DRAWVIEW.C:821-839 deterministic palette branch");
    expect_int("fixture.light", fixture.light_level, 0,
               "F0097 viewport handoff does not alter row pixels");
    expect_int("fixture.transparent", fixture.transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("d0l.resolve",
               M11_GameView_D0LD0RWallResolvePc34(&d0l, &d0l_spec) ? 1 : 0,
               1, "DUNVIEW.C:8033 C716 F0104");
    expect_int("d0r.resolve",
               M11_GameView_D0LD0RWallResolvePc34(&d0r, &d0r_spec) ? 1 : 0,
               1, "DUNVIEW.C:8127/8139 C717 parity/native wall path");
    expect_int("d0l.view_square", d0l_spec.view_square_index, 1,
               "DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("d0r.view_square", d0r_spec.view_square_index, 2,
               "DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("d0l.zone", d0l_spec.pc34_wall_zone_index, 716,
               "DEFS.H:4056 C716_ZONE_WALL_D0L");
    expect_int("d0r.zone", d0r_spec.pc34_wall_zone_index, 717,
               "DEFS.H:4057 C717_ZONE_WALL_D0R");
    expect_int("d0l.wall_returns", d0l_spec.wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:8038 returns before thing pass");
    expect_int("d0r.wall_returns", d0r_spec.wall_case_returns ? 1 : 0, 1,
               "DUNVIEW.C:8144 returns before thing pass");
    expect_int("d0l.no_f0115", d0l_spec.calls_f0115_thing_pass ? 1 : 0, 0,
               "DUNVIEW.C:8007-8038 wall return excludes F0115");
    expect_int("d0r.no_f0115", d0r_spec.calls_f0115_thing_pass ? 1 : 0, 0,
               "DUNVIEW.C:8117-8144 wall return excludes F0115");
}

static void test_d0l_then_d0r_row_composition(void)
{
    const DM1V1D0RowWallFixturePc34 fixture = {
        DM1_V1_D0_ROW_TILE_WALL_PC34,
        DM1_V1_D0_ROW_TILE_WALL_PC34,
        DM1_V1_D0_ROW_TILE_CORRIDOR_PC34,
        DM1_V1_D0_ROW_FIXTURE_Y_PC34,
        DM1_V1_D0_ROW_PALETTE_INDEX_PC34,
        DM1_V1_D0_ROW_LIGHT_LEVEL_PC34,
        DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34
    };
    uint8_t source[DM1_V1_D0L_D0R_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D0L_D0R_WALL_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D0L_D0R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D0L_D0R_WALL_VIEWPORT_HEIGHT_PC34];

    memset(source, DM1_V1_D0L_D0R_WALL_C10_COLOR_FLESH_PC34, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));

    source[source_offset(fixture.row_y, 0)] = 0x33;
    source[source_offset(fixture.row_y, 16)] = 0x21;
    source[source_offset(fixture.row_y, 32)] = 0x22;
    source[source_offset(fixture.row_y, 33)] = 0x23;
    source[source_offset(fixture.row_y, 46)] = 10;
    source[source_offset(fixture.row_y, 47)] = 0x31;
    source[source_offset(fixture.row_y, 63)] = 0x24;

    compose_d0_wall_row(&fixture, source, sizeof(source), viewport, sizeof(viewport));

    expect_int("row.x0.d0l_native",
               viewport[viewport_offset(fixture.row_y, 0)], 0x21,
               "DUNVIEW.C:8033 D0L source x16 lands at viewport x0");
    expect_int("row.x15.transparent_preserve",
               viewport[viewport_offset(fixture.row_y, 15)], 0xee,
               "DUNVIEW.C:3113-3129/F0104 C10 transparent preserve");
    expect_int("row.x16.d0r_overwrites_overlap",
               viewport[viewport_offset(fixture.row_y, 16)], 0x31,
               "DUNVIEW.C:8537-8541 F0128 draws D0R after D0L");
    expect_int("row.x17.d0r_transparent_preserves_d0l",
               viewport[viewport_offset(fixture.row_y, 17)], 0x23,
               "DUNVIEW.C:3185-3204 F0105 C10 transparent flip preserves");
    expect_int("row.x47.d0r_second_on_overlap",
               viewport[viewport_offset(fixture.row_y, 47)], 0x21,
               "DUNVIEW.C:8127 D0R parity maps source x16 over D0L x63");
    expect_int("row.x48.d0r_transparent",
               viewport[viewport_offset(fixture.row_y, 48)], 0xee,
               "DUNVIEW.C:3185-3204 F0105 transparent scratch blit");
    expect_int("row.x63.d0r_right_edge",
               viewport[viewport_offset(fixture.row_y, 63)], 0x33,
               "DUNVIEW.C:8127 D0R parity maps source x0 to viewport x63");
    expect_int("row.x64.untouched_by_d0_sides",
               viewport[viewport_offset(fixture.row_y, 64)], 0xee,
               "DUNVIEW.C:8534-8542 leaves D0C to the next draw call");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *wall_evidence = M11_GameView_D0LD0RWallSourceLockPc34();

    expect_contains("row_evidence.drawview", k_row_source_lock, "DRAWVIEW.C:709-722",
                    "F0097 viewport draw handoff");
    expect_contains("row_evidence.viewport", k_row_source_lock, "VIEWPORT.C:15-63",
                    "F0564/F0565/F0566 viewport blit invariants");
    expect_contains("row_evidence.f0128", k_row_source_lock, "DUNVIEW.C:8534-8541",
                    "F0128 D0L then D0R row order");
    expect_contains("wall_evidence.d0l", wall_evidence, "DUNVIEW.C:8007-8038",
                    "F0125 D0L wall return");
    expect_contains("wall_evidence.d0r", wall_evidence, "8117-8144",
                    "F0126 D0R wall return");
    expect_contains("wall_evidence.c716", wall_evidence, "C716_ZONE_WALL_D0L",
                    "DEFS.H:4056 C716 zone");
    expect_contains("wall_evidence.c717", wall_evidence, "C717_ZONE_WALL_D0R",
                    "DEFS.H:4057 C717 zone");
    expect_contains("wall_evidence.no_f0115", wall_evidence, "no F0115 thing pass",
                    "wall cases return before F0115");
}

int main(void)
{
    test_row_fixture_metadata_and_routes();
    test_d0l_then_d0r_row_composition();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_d0l_d0r_wall_source_lock_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_d0l_d0r_wall_source_lock_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
