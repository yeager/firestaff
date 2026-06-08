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

/* ── TC-5: C001..C006 source command ids preserved under V1 source ─ */

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

/* ── TC-6: source evidence string references ReDMCSB ────────────── */

static int test_source_evidence_anchors(void) {
    const char* ev = dm1_v2_side_by_side_seed_source_evidence();
    CHECK(ev != NULL);
    CHECK(strlen(ev) > 32);
    CHECK(strstr(ev, "ReDMCSB") != NULL);
    CHECK(strstr(ev, "DUNVIEW.C:2999-3000") != NULL);
    CHECK(strstr(ev, "DEFS.H:238-243") != NULL);
    return 0;
}

/* ── TC-7: V1 viewport geometry scaffold accessor ───────────────
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

/* ── TC-8: v1_geometry(NULL) is null-safe ──────────────────────── */

static int test_v1_geometry_null_safe(void) {
    CHECK(dm1_v2_side_by_side_seed_v1_geometry(NULL) == 0);
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
    if (test_v1_source_commands_preserved()) return 1;
    if (test_source_evidence_anchors()) return 1;
    if (test_v1_geometry_scaffold()) return 1;
    if (test_v1_geometry_null_safe()) return 1;
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
