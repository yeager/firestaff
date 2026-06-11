/*
 * test_dm1_v2_side_by_side_seed_pc34.c
 *
 * CTest companion to src/dm1v2/dm1_v2_side_by_side_seed_pc34.c.
 *
 * The seed source exposes a small, deterministic V1/V2 side-by-side
 * verification helper. This ctest entry locks its contracts:
 *
 *   TC-1. The canonical scaffold dimensions are the public
 *         DM1_V2_SIDE_BY_SIDE_W/H/GAP_W macros and the dimension
 *         query function returns them.
 *
 *   TC-2. dm1_v2_side_by_side_seed_build_entry() returns 1, the
 *         V1/V2 lanes are byte-equal across the full 224x136
 *         viewport, and the side-by-side hash is deterministic.
 *
 *   TC-3. dm1_v2_side_by_side_seed_hash_layout() reproduces the
 *         same hash for the same V1/V2 pair, and produces a
 *         different hash for a different pair (sanity check that
 *         the hash actually depends on the buffers).
 *
 *   TC-4. The V1 movement command adapter preserves the V1
 *         source command ids C001..C006 (ReDMCSB DEFS.H:238-243)
 *         with v2PresentationEnabled=0: every row reports
 *         routeKind=V1_SOURCE, sourceCommand == runtimeCommand,
 *         and the route echoes the requested v2PresentationEnabled
 *         flag. The seed is presentation-disabled, so no row can
 *         be silently re-routed by a future V2 presentation change.
 *
 *   TC-5. The source evidence string is non-NULL and references
 *         the ReDMCSB source anchors.
 *
 *   TC-6. dm1_v2_side_by_side_seed_write_rgba8888() materializes
 *         the same canonical V1-gap-V2 composite into a row-major
 *         RGBA8888 buffer without touching row padding.
 *
 *   TC-7. The RGBA8888 export keeps every row's V1/gap/V2 lane
 *         boundaries aligned: the full 8-pixel gap band is the
 *         canonical label colour and the first/last lane pixels
 *         match the composite accessor on every row.
 *
 *   TC-8. dm1_v2_side_by_side_seed_region() exposes named
 *         composite-space rectangles for the V1 lane, gap, V2 lane,
 *         and both D1C wall/portrait rectangles in both lanes.
 *
 *   TC-9. The manifest rectangles are bound to actual pixels: full
 *         V1/V2 lanes, D1C wall rectangles, and D1C portrait
 *         rectangles compare byte-identically and produce matching
 *         region hashes.
 *
 * The test is headless: it depends only on the firestaff_v2 static
 * library and does not require any game data files.
 *
 * Source locks (ReDMCSB):
 *   DEFS.H:238-243          C001..C006 V1 source command ids.
 *   COMMAND.C:2045-2155     F0359 command queue dispatch.
 *   DUNVIEW.C:2999-3000     224x136 V1/V2 viewport bitmap.
 *   COORD.C:1721-1722       224x136 viewport.
 *
 * Exit codes: 0 = pass, 1 = at least one assertion failed.
 */

#include "dm1_v2_side_by_side_seed_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"
#include "dm1_v2_movement_command_adapter_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

/* V1 source command table (ReDMCSB DEFS.H:238-243). */
typedef struct {
    DM1_V2_MovementCommand v2Command;
    int v1SourceCommand;
    const char* label;
} V1SourceCommandRow;

static const V1SourceCommandRow g_v1_command_table[6] = {
    { DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,    1, "C001 TURN_LEFT"    },
    { DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT,   2, "C002 TURN_RIGHT"   },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, "C003 MOVE_FORWARD" },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT,   4, "C004 MOVE_RIGHT"   },
    { DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD,5, "C005 MOVE_BACKWARD"},
    { DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT,    6, "C006 MOVE_LEFT"    },
};
#define N_V1_COMMAND_ROWS \
    ((int)(sizeof(g_v1_command_table) / sizeof(g_v1_command_table[0])))

static int colors_equal(const DM1_V2_Color* a, const DM1_V2_Color* b) {
    return a && b &&
           a->r == b->r &&
           a->g == b->g &&
           a->b == b->b &&
           a->a == b->a;
}

/* ── TC-1: scaffold dimensions ──────────────────────────────────── */

static int test_scaffold_dimensions(void) {
    int w = 0;
    int h = 0;
    int gap = 0;
    dm1_v2_side_by_side_seed_scaffold_dimensions(&w, &h, &gap);
    CHECK(w == DM1_V2_SIDE_BY_SIDE_W);
    CHECK(w == DM1_V2_VIEWPORT_W * 2 + DM1_V2_SIDE_BY_SIDE_GAP_W);
    CHECK(h == DM1_V2_SIDE_BY_SIDE_H);
    CHECK(h == DM1_V2_VIEWPORT_H);
    CHECK(gap == DM1_V2_SIDE_BY_SIDE_GAP_W);

    /* NULL out-args are safe. */
    dm1_v2_side_by_side_seed_scaffold_dimensions(NULL, NULL, NULL);
    return 0;
}

/* ── TC-2: build the entry seed, lock hash + lane parity ────────── */

static int test_build_entry_seed(void) {
    DM1_V2_SideBySideSeed seed;
    memset(&seed, 0, sizeof(seed));
    CHECK(dm1_v2_side_by_side_seed_build_entry(&seed) == 1);
    CHECK(seed.scaffoldW == DM1_V2_SIDE_BY_SIDE_W);
    CHECK(seed.scaffoldH == DM1_V2_SIDE_BY_SIDE_H);
    CHECK(seed.gapW == DM1_V2_SIDE_BY_SIDE_GAP_W);
    CHECK(seed.lanesByteEqual == 1);
    CHECK(seed.mismatchedPixels == 0);
    CHECK(seed.firstMismatchX == -1);
    CHECK(seed.firstMismatchY == -1);
    /* The hash must be non-zero (the FNV-1a basis) and must not
     * equal the FNV-1a basis (a real framebuffer produces a
     * different fold). */
    CHECK(seed.sideBySideHash != 0);
    CHECK(seed.sideBySideHash != DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS);
    /* Lock the canonical seed hash so any future regression in the
     * FNV-1a fold, gap colour, or layout traversal surfaces as a
     * known-good baseline mismatch. The value matches the existing
     * probes/dm1/firestaff_dm1_v2_side_by_side_presentation_seed_probe.c
     * output (sideBySideHash=cf0cbcce6f491525) and confirms the
     * new source-side helper is byte-for-byte compatible with the
     * existing probe infrastructure. */
    CHECK(seed.sideBySideHash == 0xcf0cbcce6f491525ULL);
    return 0;
}

/* ── TC-3: layout hash reproduces and depends on the buffers ────── */

static int test_layout_hash_reproduces(void) {
    DM1_V2_SideBySideSeed a;
    DM1_V2_SideBySideSeed b;
    DM1_V2_SideBySideSeed c;
    CHECK(dm1_v2_side_by_side_seed_build_entry(&a) == 1);
    CHECK(dm1_v2_side_by_side_seed_build_entry(&b) == 1);
    CHECK(a.sideBySideHash == b.sideBySideHash);

    /* hash_layout() must reproduce the same hash for the same pair. */
    CHECK(dm1_v2_side_by_side_seed_hash_layout(&a.v1, &a.v2) ==
          dm1_v2_side_by_side_seed_hash_layout(&b.v1, &b.v2));

    /* Mutate one byte of c.v1 to confirm the hash depends on
     * the buffer content (it must change). */
    c = a;
    c.v1.framebuffer[0][0].r ^= 0x01U;
    CHECK(dm1_v2_side_by_side_seed_hash_layout(&c.v1, &c.v2) !=
          a.sideBySideHash);
    return 0;
}

/* ── TC-4: composite pixel accessor matches canonical layout ────── */

static int test_composite_pixel_accessor(void) {
    DM1_V2_SideBySideSeed seed;
    DM1_V2_Color c;
    uint64_t hash;
    int x, y;
    CHECK(dm1_v2_side_by_side_seed_build_entry(&seed) == 1);

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(&seed, 0, 0, &c) == 1);
    CHECK(colors_equal(&c, &seed.v1.framebuffer[0][0]));

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed, DM1_V2_VIEWPORT_W, 0, &c) == 1);
    CHECK(c.r == DM1_V2_SIDE_BY_SIDE_GAP_R);
    CHECK(c.g == DM1_V2_SIDE_BY_SIDE_GAP_G);
    CHECK(c.b == DM1_V2_SIDE_BY_SIDE_GAP_B);
    CHECK(c.a == DM1_V2_SIDE_BY_SIDE_GAP_A);

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed,
              DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W,
              0,
              &c) == 1);
    CHECK(colors_equal(&c, &seed.v2.framebuffer[0][0]));

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed,
              DM1_V2_SIDE_BY_SIDE_W - 1,
              DM1_V2_SIDE_BY_SIDE_H - 1,
              &c) == 1);
    CHECK(colors_equal(
        &c,
        &seed.v2.framebuffer[DM1_V2_VIEWPORT_H - 1][DM1_V2_VIEWPORT_W - 1]));

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(NULL, 0, 0, &c) == 0);
    CHECK(dm1_v2_side_by_side_seed_composite_pixel(&seed, 0, 0, NULL) == 0);
    CHECK(dm1_v2_side_by_side_seed_composite_pixel(&seed, -1, 0, &c) == 0);
    CHECK(dm1_v2_side_by_side_seed_composite_pixel(&seed, 0, -1, &c) == 0);
    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed, DM1_V2_SIDE_BY_SIDE_W, 0, &c) == 0);
    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed, 0, DM1_V2_SIDE_BY_SIDE_H, &c) == 0);

    hash = DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS;
    for (y = 0; y < DM1_V2_SIDE_BY_SIDE_H; ++y) {
        for (x = 0; x < DM1_V2_SIDE_BY_SIDE_W; ++x) {
            CHECK(dm1_v2_side_by_side_seed_composite_pixel(&seed, x, y, &c) == 1);
            hash = dm1_v2_side_by_side_seed_hash_color(hash, &c);
        }
    }
    CHECK(hash == seed.sideBySideHash);
    return 0;
}

/* ── TC-5: RGBA8888 export materializes the same composite ─────── */

static int test_rgba8888_export(void) {
    enum {
        kPadBytes = 8,
        kTightStride = DM1_V2_SIDE_BY_SIDE_W * 4,
        kPaddedStride = kTightStride + kPadBytes,
        kBufferBytes = kPaddedStride * DM1_V2_SIDE_BY_SIDE_H
    };
    static unsigned char rgba[kBufferBytes];
    DM1_V2_SideBySideSeed seed;
    DM1_V2_Color c;
    uint64_t hash = DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS;
    const size_t requiredBytes =
        (size_t)(DM1_V2_SIDE_BY_SIDE_H - 1) * (size_t)kPaddedStride +
        (size_t)kTightStride;
    int y, x, p;

    memset(rgba, 0xa5, sizeof(rgba));
    CHECK(dm1_v2_side_by_side_seed_build_entry(&seed) == 1);
    CHECK(dm1_v2_side_by_side_seed_write_rgba8888(
              &seed, rgba, sizeof(rgba), kPaddedStride) == 1);

    CHECK(rgba[0] == seed.v1.framebuffer[0][0].r);
    CHECK(rgba[1] == seed.v1.framebuffer[0][0].g);
    CHECK(rgba[2] == seed.v1.framebuffer[0][0].b);
    CHECK(rgba[3] == seed.v1.framebuffer[0][0].a);

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed, DM1_V2_VIEWPORT_W, 0, &c) == 1);
    CHECK(rgba[(size_t)DM1_V2_VIEWPORT_W * 4u + 0u] == c.r);
    CHECK(rgba[(size_t)DM1_V2_VIEWPORT_W * 4u + 1u] == c.g);
    CHECK(rgba[(size_t)DM1_V2_VIEWPORT_W * 4u + 2u] == c.b);
    CHECK(rgba[(size_t)DM1_V2_VIEWPORT_W * 4u + 3u] == c.a);

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed,
              DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W,
              0,
              &c) == 1);
    CHECK(rgba[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 0u] == c.r);
    CHECK(rgba[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 1u] == c.g);
    CHECK(rgba[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 2u] == c.b);
    CHECK(rgba[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 3u] == c.a);

    for (y = 0; y < DM1_V2_SIDE_BY_SIDE_H; ++y) {
        const unsigned char* row = rgba + (size_t)y * (size_t)kPaddedStride;
        for (x = 0; x < DM1_V2_SIDE_BY_SIDE_W; ++x) {
            c.r = row[(size_t)x * 4u + 0u];
            c.g = row[(size_t)x * 4u + 1u];
            c.b = row[(size_t)x * 4u + 2u];
            c.a = row[(size_t)x * 4u + 3u];
            hash = dm1_v2_side_by_side_seed_hash_color(hash, &c);
        }
        for (p = 0; p < kPadBytes; ++p) {
            CHECK(row[kTightStride + p] == 0xa5);
        }
    }
    CHECK(hash == seed.sideBySideHash);

    CHECK(dm1_v2_side_by_side_seed_write_rgba8888(
              NULL, rgba, sizeof(rgba), kPaddedStride) == 0);
    CHECK(dm1_v2_side_by_side_seed_write_rgba8888(
              &seed, NULL, sizeof(rgba), kPaddedStride) == 0);
    CHECK(dm1_v2_side_by_side_seed_write_rgba8888(
              &seed, rgba, sizeof(rgba), kTightStride - 1) == 0);
    CHECK(dm1_v2_side_by_side_seed_write_rgba8888(
              &seed, rgba, requiredBytes - 1u, kPaddedStride) == 0);
    return 0;
}

/* ── TC-6: RGBA8888 lane boundaries stay aligned on every row ───── */

static int test_rgba8888_lane_boundaries(void) {
    enum {
        kStride = DM1_V2_SIDE_BY_SIDE_W * 4
    };
    static unsigned char rgba[(size_t)kStride * DM1_V2_SIDE_BY_SIDE_H];
    DM1_V2_SideBySideSeed seed;
    DM1_V2_Color expected;
    int y, gapX;

    memset(rgba, 0, sizeof(rgba));
    CHECK(dm1_v2_side_by_side_seed_build_entry(&seed) == 1);
    CHECK(dm1_v2_side_by_side_seed_write_rgba8888(
              &seed, rgba, sizeof(rgba), kStride) == 1);

    for (y = 0; y < DM1_V2_SIDE_BY_SIDE_H; ++y) {
        const unsigned char* row = rgba + (size_t)y * (size_t)kStride;

        CHECK(dm1_v2_side_by_side_seed_composite_pixel(&seed, 0, y, &expected) == 1);
        CHECK(row[0] == expected.r);
        CHECK(row[1] == expected.g);
        CHECK(row[2] == expected.b);
        CHECK(row[3] == expected.a);

        CHECK(dm1_v2_side_by_side_seed_composite_pixel(
                  &seed, DM1_V2_VIEWPORT_W - 1, y, &expected) == 1);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W - 1) * 4u + 0u] == expected.r);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W - 1) * 4u + 1u] == expected.g);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W - 1) * 4u + 2u] == expected.b);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W - 1) * 4u + 3u] == expected.a);

        for (gapX = 0; gapX < DM1_V2_SIDE_BY_SIDE_GAP_W; ++gapX) {
            const size_t x = (size_t)(DM1_V2_VIEWPORT_W + gapX);
            CHECK(row[x * 4u + 0u] == DM1_V2_SIDE_BY_SIDE_GAP_R);
            CHECK(row[x * 4u + 1u] == DM1_V2_SIDE_BY_SIDE_GAP_G);
            CHECK(row[x * 4u + 2u] == DM1_V2_SIDE_BY_SIDE_GAP_B);
            CHECK(row[x * 4u + 3u] == DM1_V2_SIDE_BY_SIDE_GAP_A);
        }

        CHECK(dm1_v2_side_by_side_seed_composite_pixel(
                  &seed,
                  DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W,
                  y,
                  &expected) == 1);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 0u] == expected.r);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 1u] == expected.g);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 2u] == expected.b);
        CHECK(row[(size_t)(DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) * 4u + 3u] == expected.a);

        CHECK(dm1_v2_side_by_side_seed_composite_pixel(
                  &seed, DM1_V2_SIDE_BY_SIDE_W - 1, y, &expected) == 1);
        CHECK(row[(size_t)(DM1_V2_SIDE_BY_SIDE_W - 1) * 4u + 0u] == expected.r);
        CHECK(row[(size_t)(DM1_V2_SIDE_BY_SIDE_W - 1) * 4u + 1u] == expected.g);
        CHECK(row[(size_t)(DM1_V2_SIDE_BY_SIDE_W - 1) * 4u + 2u] == expected.b);
        CHECK(row[(size_t)(DM1_V2_SIDE_BY_SIDE_W - 1) * 4u + 3u] == expected.a);
    }
    return 0;
}

/* ── TC-7: C001..C006 source command ids preserved under V1 source ─ */

static int test_v1_source_commands_preserved(void) {
    int i;
    for (i = 0; i < N_V1_COMMAND_ROWS; ++i) {
        DM1_V2_MovementCommandRoute route;
        char id[64];
        snprintf(id, sizeof(id), "v1_command_table[%s]",
                 g_v1_command_table[i].label);
        route = dm1_v2_movement_command_route_for_presentation(
            0, g_v1_command_table[i].v2Command);
        CHECK(route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(route.v2PresentationEnabled == 0);
        CHECK(route.sourceCommand == g_v1_command_table[i].v1SourceCommand);
        CHECK(route.runtimeCommand == g_v1_command_table[i].v1SourceCommand);
        (void)id;
    }
    return 0;
}

/* ── TC-8: source evidence string references ReDMCSB ────────────── */

static int test_source_evidence_anchors(void) {
    const char* ev = dm1_v2_side_by_side_seed_source_evidence();
    CHECK(ev != NULL);
    CHECK(strlen(ev) > 32);
    CHECK(strstr(ev, "ReDMCSB") != NULL);
    CHECK(strstr(ev, "DUNVIEW.C:2999-3000") != NULL);
    CHECK(strstr(ev, "DEFS.H:238-243") != NULL);
    return 0;
}

/* ── TC-9: V1 viewport geometry scaffold accessor ───────────────
 *
 * Locks the source-locked V1 viewport geometry constants (portrait
 * + wall panel) used by future screenshot-diff and pixel-scaffolding
 * gates. The macros are the source of truth; the accessor must
 * report the same values. Both anchor strings must be non-empty and
 * reference the ReDMCSB DUNVIEW.C lines that fix the constants.
 *
 * ReDMCSB DUNVIEW.C:3913-3928 champion portrait blit at {96,35}.
 * ReDMCSB DUNVIEW.C:525  G0109 portrait box {96,127,35,63} (W=32, H=29).
 * ReDMCSB DUNVIEW.C:587  G0163 Frame_Walls[12][8] D1C row {32,191,9,119,128,111,48,0}
 *                       (W=160, H=111, viewport-local X=32, Y=9).
 */

static int test_v1_geometry_scaffold(void) {
    DM1_V2_SideBySideV1Geometry geom;
    memset(&geom, 0, sizeof(geom));
    CHECK(dm1_v2_side_by_side_seed_v1_geometry(&geom) == 1);

    /* Viewport dimensions: COORD.C:1721-1722 / DUNVIEW.C:2999-3000. */
    CHECK(geom.viewportW == 224);
    CHECK(geom.viewportH == 136);
    CHECK(geom.viewportW == DM1_V2_VIEWPORT_W);
    CHECK(geom.viewportH == DM1_V2_VIEWPORT_H);

    /* D1C champion-portrait square: DUNVIEW.C:3913-3928. */
    CHECK(geom.d1cPortraitW == 32);
    CHECK(geom.d1cPortraitH == 29);
    CHECK(geom.d1cPortraitX == 96);
    CHECK(geom.d1cPortraitY == 35);
    CHECK(geom.d1cPortraitW == DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W);
    CHECK(geom.d1cPortraitH == DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H);
    CHECK(geom.d1cPortraitX == DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X);
    CHECK(geom.d1cPortraitY == DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y);
    CHECK(geom.portraitAnchor != NULL);
    CHECK(strlen(geom.portraitAnchor) > 4);
    CHECK(strstr(geom.portraitAnchor, "ReDMCSB") != NULL);
    CHECK(strstr(geom.portraitAnchor, "DUNVIEW.C:3913-3928") != NULL);

    /* D1C wall panel: DUNVIEW.C:581-593 (G0163_aauc_Graphic558_Frame_Walls[12][8]
     * D1C row indexed by M606_VIEW_SQUARE_D1C). */
    CHECK(geom.d1cWallX == 32);
    CHECK(geom.d1cWallY == 9);
    CHECK(geom.d1cWallW == 160);
    CHECK(geom.d1cWallH == 111);
    CHECK(geom.d1cWallX == DM1_V2_SIDE_BY_SIDE_D1C_WALL_X);
    CHECK(geom.d1cWallY == DM1_V2_SIDE_BY_SIDE_D1C_WALL_Y);
    CHECK(geom.d1cWallW == DM1_V2_SIDE_BY_SIDE_D1C_WALL_W);
    CHECK(geom.d1cWallH == DM1_V2_SIDE_BY_SIDE_D1C_WALL_H);
    CHECK(geom.wallAnchor != NULL);
    CHECK(strlen(geom.wallAnchor) > 4);
    CHECK(strstr(geom.wallAnchor, "ReDMCSB") != NULL);
    CHECK(strstr(geom.wallAnchor, "DUNVIEW.C:581-593") != NULL);

    /* Consistency: the D1C portrait must be entirely inside the
     * D1C wall panel. The wall panel is the front-aspect blit and
     * the portrait is blit on top of it. If the portrait ever
     * escapes the wall panel, the V1 source-truth chain breaks. */
    CHECK(geom.d1cPortraitX >= geom.d1cWallX);
    CHECK(geom.d1cPortraitY >= geom.d1cWallY);
    CHECK(geom.d1cPortraitX + geom.d1cPortraitW <= geom.d1cWallX + geom.d1cWallW);
    CHECK(geom.d1cPortraitY + geom.d1cPortraitH <= geom.d1cWallY + geom.d1cWallH);

    /* Consistency: both rectangles must fit inside the 224x136
     * viewport. */
    CHECK(geom.d1cWallX + geom.d1cWallW <= geom.viewportW);
    CHECK(geom.d1cWallY + geom.d1cWallH <= geom.viewportH);
    CHECK(geom.d1cPortraitX + geom.d1cPortraitW <= geom.viewportW);
    CHECK(geom.d1cPortraitY + geom.d1cPortraitH <= geom.viewportH);
    return 0;
}

/* ── TC-10: v1_geometry(NULL) is null-safe ─────────────────────── */

static int test_v1_geometry_null_safe(void) {
    CHECK(dm1_v2_side_by_side_seed_v1_geometry(NULL) == 0);
    return 0;
}

/* ── TC-11: side-by-side region manifest ───────────────────────── */

static int test_region_manifest(void) {
    DM1_V2_SideBySideV1Geometry geom;
    DM1_V2_SideBySideRegion r;
    const int v2LaneX = DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W;
    int i;
    CHECK(dm1_v2_side_by_side_seed_v1_geometry(&geom) == 1);

    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE, &r) == 1);
    CHECK(r.id == DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE);
    CHECK(r.x == 0);
    CHECK(r.y == 0);
    CHECK(r.w == DM1_V2_VIEWPORT_W);
    CHECK(r.h == DM1_V2_VIEWPORT_H);
    CHECK(strcmp(r.label, "v1_lane") == 0);
    CHECK(strstr(r.sourceAnchor, "DUNVIEW.C:2999-3000") != NULL);

    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_GAP, &r) == 1);
    CHECK(r.x == DM1_V2_VIEWPORT_W);
    CHECK(r.y == 0);
    CHECK(r.w == DM1_V2_SIDE_BY_SIDE_GAP_W);
    CHECK(r.h == DM1_V2_VIEWPORT_H);
    CHECK(strcmp(r.label, "gap") == 0);

    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V2_LANE, &r) == 1);
    CHECK(r.x == v2LaneX);
    CHECK(r.y == 0);
    CHECK(r.w == DM1_V2_VIEWPORT_W);
    CHECK(r.h == DM1_V2_VIEWPORT_H);
    CHECK(strcmp(r.label, "v2_lane") == 0);
    CHECK(r.x + r.w == DM1_V2_SIDE_BY_SIDE_W);

    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_WALL, &r) == 1);
    CHECK(r.x == geom.d1cWallX);
    CHECK(r.y == geom.d1cWallY);
    CHECK(r.w == geom.d1cWallW);
    CHECK(r.h == geom.d1cWallH);
    CHECK(strcmp(r.label, "v1_d1c_wall") == 0);
    CHECK(strstr(r.sourceAnchor, "DUNVIEW.C:581-593") != NULL);

    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_PORTRAIT, &r) == 1);
    CHECK(r.x == geom.d1cPortraitX);
    CHECK(r.y == geom.d1cPortraitY);
    CHECK(r.w == geom.d1cPortraitW);
    CHECK(r.h == geom.d1cPortraitH);
    CHECK(strcmp(r.label, "v1_d1c_portrait") == 0);
    CHECK(strstr(r.sourceAnchor, "DUNVIEW.C:3913-3928") != NULL);

    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_WALL, &r) == 1);
    CHECK(r.x == v2LaneX + geom.d1cWallX);
    CHECK(r.y == geom.d1cWallY);
    CHECK(r.w == geom.d1cWallW);
    CHECK(r.h == geom.d1cWallH);
    CHECK(strcmp(r.label, "v2_d1c_wall") == 0);

    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_PORTRAIT, &r) == 1);
    CHECK(r.x == v2LaneX + geom.d1cPortraitX);
    CHECK(r.y == geom.d1cPortraitY);
    CHECK(r.w == geom.d1cPortraitW);
    CHECK(r.h == geom.d1cPortraitH);
    CHECK(strcmp(r.label, "v2_d1c_portrait") == 0);

    for (i = 0; i < DM1_V2_SIDE_BY_SIDE_REGION_COUNT; ++i) {
        CHECK(dm1_v2_side_by_side_seed_region((DM1_V2_SideBySideRegionId)i,
                                              &r) == 1);
        CHECK(r.label != NULL);
        CHECK(r.sourceAnchor != NULL);
        CHECK(r.w > 0);
        CHECK(r.h > 0);
        CHECK(r.x >= 0);
        CHECK(r.y >= 0);
        CHECK(r.x + r.w <= DM1_V2_SIDE_BY_SIDE_W);
        CHECK(r.y + r.h <= DM1_V2_SIDE_BY_SIDE_H);
    }

    CHECK(dm1_v2_side_by_side_seed_region(
              (DM1_V2_SideBySideRegionId)-1, &r) == 0);
    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_COUNT, &r) == 0);
    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE, NULL) == 0);
    return 0;
}

/* ── TC-12: region pixels translate between V1 and V2 lanes ────── */

static int test_region_manifest_pixel_translation(void) {
    DM1_V2_SideBySideSeed seed;
    DM1_V2_SideBySideRegion v1Wall;
    DM1_V2_SideBySideRegion v2Wall;
    DM1_V2_SideBySideRegion v1Portrait;
    DM1_V2_SideBySideRegion v2Portrait;
    DM1_V2_SideBySideRegion gap;
    DM1_V2_Color a;
    DM1_V2_Color b;
    int dx, dy;

    CHECK(dm1_v2_side_by_side_seed_build_entry(&seed) == 1);
    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_WALL, &v1Wall) == 1);
    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_WALL, &v2Wall) == 1);
    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_PORTRAIT, &v1Portrait) == 1);
    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_PORTRAIT, &v2Portrait) == 1);
    CHECK(dm1_v2_side_by_side_seed_region(
              DM1_V2_SIDE_BY_SIDE_REGION_GAP, &gap) == 1);

    CHECK(v2Wall.x - v1Wall.x ==
          DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W);
    CHECK(v2Portrait.x - v1Portrait.x ==
          DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W);

    for (dy = 0; dy <= 1; ++dy) {
        for (dx = 0; dx <= 1; ++dx) {
            const int wallX = dx ? v1Wall.x + v1Wall.w - 1 : v1Wall.x;
            const int wallY = dy ? v1Wall.y + v1Wall.h - 1 : v1Wall.y;
            const int portraitX =
                dx ? v1Portrait.x + v1Portrait.w - 1 : v1Portrait.x;
            const int portraitY =
                dy ? v1Portrait.y + v1Portrait.h - 1 : v1Portrait.y;

            CHECK(dm1_v2_side_by_side_seed_composite_pixel(
                      &seed, wallX, wallY, &a) == 1);
            CHECK(dm1_v2_side_by_side_seed_composite_pixel(
                      &seed, wallX + v2Wall.x - v1Wall.x, wallY, &b) == 1);
            CHECK(colors_equal(&a, &b));

            CHECK(dm1_v2_side_by_side_seed_composite_pixel(
                      &seed, portraitX, portraitY, &a) == 1);
            CHECK(dm1_v2_side_by_side_seed_composite_pixel(
                      &seed,
                      portraitX + v2Portrait.x - v1Portrait.x,
                      portraitY,
                      &b) == 1);
            CHECK(colors_equal(&a, &b));
        }
    }

    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed, gap.x, gap.y, &a) == 1);
    CHECK(a.r == DM1_V2_SIDE_BY_SIDE_GAP_R);
    CHECK(a.g == DM1_V2_SIDE_BY_SIDE_GAP_G);
    CHECK(a.b == DM1_V2_SIDE_BY_SIDE_GAP_B);
    CHECK(a.a == DM1_V2_SIDE_BY_SIDE_GAP_A);
    CHECK(dm1_v2_side_by_side_seed_composite_pixel(
              &seed, gap.x + gap.w - 1, gap.y + gap.h - 1, &a) == 1);
    CHECK(a.r == DM1_V2_SIDE_BY_SIDE_GAP_R);
    CHECK(a.g == DM1_V2_SIDE_BY_SIDE_GAP_G);
    CHECK(a.b == DM1_V2_SIDE_BY_SIDE_GAP_B);
    CHECK(a.a == DM1_V2_SIDE_BY_SIDE_GAP_A);
    return 0;
}

/* ── TC-13: region manifest full-pixel gates ─────────────────────
 *
 * This is the enhanced-presentation screenshot/pixel gate binding for
 * the seed manifest: compare the full named rectangles, not only their
 * translated corners. With presentation disabled the V1 and V2 lanes
 * must be byte-identical across the full 224x136 viewport, and the
 * source-locked D1C wall/portrait sub-rectangles must match pixel-for-
 * pixel in both lanes.
 */

static int test_region_manifest_full_pixel_gates(void) {
    DM1_V2_SideBySideSeed seed;
    DM1_V2_SideBySideRegionCompareResult cmp;
    int pixelsA = 0;
    int pixelsB = 0;
    uint64_t hashA;
    uint64_t hashB;

    CHECK(dm1_v2_side_by_side_seed_build_entry(&seed) == 1);

    CHECK(dm1_v2_side_by_side_seed_compare_regions(
              &seed,
              DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE,
              DM1_V2_SIDE_BY_SIDE_REGION_V2_LANE,
              &cmp) == 1);
    CHECK(cmp.comparedPixels == DM1_V2_VIEWPORT_W * DM1_V2_VIEWPORT_H);
    CHECK(cmp.mismatchedPixels == 0);
    CHECK(cmp.firstMismatchAX == -1);
    CHECK(cmp.firstMismatchBX == -1);
    hashA = dm1_v2_side_by_side_seed_hash_region(
        &seed, DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE, &pixelsA);
    hashB = dm1_v2_side_by_side_seed_hash_region(
        &seed, DM1_V2_SIDE_BY_SIDE_REGION_V2_LANE, &pixelsB);
    CHECK(pixelsA == DM1_V2_VIEWPORT_W * DM1_V2_VIEWPORT_H);
    CHECK(pixelsB == pixelsA);
    CHECK(hashA == hashB);
    CHECK(hashA != DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS);

    CHECK(dm1_v2_side_by_side_seed_compare_regions(
              &seed,
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_WALL,
              DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_WALL,
              &cmp) == 1);
    CHECK(cmp.comparedPixels ==
          DM1_V2_SIDE_BY_SIDE_D1C_WALL_W * DM1_V2_SIDE_BY_SIDE_D1C_WALL_H);
    CHECK(cmp.mismatchedPixels == 0);
    pixelsA = pixelsB = 0;
    hashA = dm1_v2_side_by_side_seed_hash_region(
        &seed, DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_WALL, &pixelsA);
    hashB = dm1_v2_side_by_side_seed_hash_region(
        &seed, DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_WALL, &pixelsB);
    CHECK(pixelsA ==
          DM1_V2_SIDE_BY_SIDE_D1C_WALL_W * DM1_V2_SIDE_BY_SIDE_D1C_WALL_H);
    CHECK(pixelsB == pixelsA);
    CHECK(hashA == hashB);
    CHECK(hashA != DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS);

    CHECK(dm1_v2_side_by_side_seed_compare_regions(
              &seed,
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_PORTRAIT,
              DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_PORTRAIT,
              &cmp) == 1);
    CHECK(cmp.comparedPixels ==
          DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W *
          DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H);
    CHECK(cmp.mismatchedPixels == 0);
    pixelsA = pixelsB = 0;
    hashA = dm1_v2_side_by_side_seed_hash_region(
        &seed, DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_PORTRAIT, &pixelsA);
    hashB = dm1_v2_side_by_side_seed_hash_region(
        &seed, DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_PORTRAIT, &pixelsB);
    CHECK(pixelsA ==
          DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W *
          DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H);
    CHECK(pixelsB == pixelsA);
    CHECK(hashA == hashB);
    CHECK(hashA != DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS);

    CHECK(dm1_v2_side_by_side_seed_compare_regions(
              &seed,
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_WALL,
              DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_PORTRAIT,
              &cmp) == 0);
    CHECK(cmp.comparedPixels == 0);
    CHECK(cmp.mismatchedPixels == 0);

    pixelsA = 1234;
    CHECK(dm1_v2_side_by_side_seed_hash_region(
              NULL, DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE, &pixelsA) ==
          DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS);
    CHECK(pixelsA == 0);
    pixelsA = 1234;
    CHECK(dm1_v2_side_by_side_seed_hash_region(
              &seed, DM1_V2_SIDE_BY_SIDE_REGION_COUNT, &pixelsA) ==
          DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS);
    CHECK(pixelsA == 0);
    CHECK(dm1_v2_side_by_side_seed_compare_regions(
              NULL,
              DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE,
              DM1_V2_SIDE_BY_SIDE_REGION_V2_LANE,
              &cmp) == 0);
    CHECK(dm1_v2_side_by_side_seed_compare_regions(
              &seed,
              DM1_V2_SIDE_BY_SIDE_REGION_COUNT,
              DM1_V2_SIDE_BY_SIDE_REGION_V2_LANE,
              &cmp) == 0);
    return 0;
}

/* ── Main ───────────────────────────────────────────────────────── */

int main(void) {
    DM1_V2_SideBySideSeed seed;
    memset(&seed, 0, sizeof(seed));
    if (test_scaffold_dimensions()) return 1;
    if (test_build_entry_seed()) return 1;
    if (test_layout_hash_reproduces()) return 1;
    if (test_composite_pixel_accessor()) return 1;
    if (test_rgba8888_export()) return 1;
    if (test_rgba8888_lane_boundaries()) return 1;
    if (test_v1_source_commands_preserved()) return 1;
    if (test_source_evidence_anchors()) return 1;
    if (test_v1_geometry_scaffold()) return 1;
    if (test_v1_geometry_null_safe()) return 1;
    if (test_region_manifest()) return 1;
    if (test_region_manifest_pixel_translation()) return 1;
    if (test_region_manifest_full_pixel_gates()) return 1;
    /* Print the canonical seed fingerprint so downstream visual-diff
     * gates can lock a known-good baseline against it. */
    if (dm1_v2_side_by_side_seed_build_entry(&seed) == 1) {
        printf("dm1_v2_side_by_side_seed_pc34: sideBySideHash=%016llx "
               "lanesByteEqual=%d mismatchedPixels=%d\n",
               (unsigned long long)seed.sideBySideHash,
               seed.lanesByteEqual,
               seed.mismatchedPixels);
    }
    puts("dm1_v2_side_by_side_seed_pc34: ok");
    return 0;
}
