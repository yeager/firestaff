/*
 * firestaff_dm1_v2_side_by_side_presentation_seed_probe.c
 *
 * DM1 V2 side-by-side verification seed.
 *
 * Purpose
 *   Provide a small, deterministic verification seed that exercises the
 *   V1/V2 presentation-disabled parity boundary at the DM1 PC 3.4 entry
 *   state fixture and emits a side-by-side pixel-scaffolding hash that
 *   later probes / visual gates can reuse as a stable seed.
 *
 * What it verifies
 *   1. The DM1 PC34 entry state fixture (real-data map 0 x=1 y=3 dir=2)
 *      builds a stable composition with the front wall champion-portrait
 *      square at D1C and the D0C current corridor square.
 *   2. Rendering that composition through dm1_v2_vp_render_composition_flat
 *      twice (V1 route and V2 route, both with V2 presentation disabled)
 *      produces two byte-identical 224x136 RGBA framebuffers. This is the
 *      "presentation-disabled parity" gate: the V2 shell must never alter
 *      pixel truth when the gameplay route is V1-source-locked.
 *   3. A side-by-side pixel-scaffolding composite is built: V1 framebuffer
 *      || 8-pixel column gap (V2 label color) || V2 framebuffer, then
 *      FNV-1a hashed into a stable 64-bit seed that is reproducible across
 *      machines and builds.
 *   4. The route is checked across every C001..C006 V1 source command
 *      (DEFS.H:238-243). With v2PresentationEnabled=0 every row resolves
 *      to DM1_V2_MOVEMENT_ROUTE_V1_SOURCE, sourceCommand == runtimeCommand
 *      == C-id, and route.v2PresentationEnabled echoes the requested flag,
 *      so no row can be silently re-routed by a future V2 presentation
 *      change.
 *
 * Source locks
 *   ReDMCSB DUNVIEW.C:2999-3000  viewport bitmap 224x136 dimensions.
 *   ReDMCSB DUNVIEW.C:8337-8338  draw floor/ceiling before walking squares.
 *   ReDMCSB DUNVIEW.C:8490-8542  D3 -> D0 left/center/right draw order.
 *   ReDMCSB DEFS.H:235           "Commands" section header.
 *   ReDMCSB DEFS.H:238-243       C001..C006 movement command ids.
 *   ReDMCSB COMMAND.C:2045-2155  F0359 command queue dispatch keeps V1
 *                                gameplay outside V2 presentation.
 *   ReDMCSB GAMELOOP.C:90        F0128_DUNGEONVIEW_Draw_CPSF snapshot draw.
 *   dm1_v2_presentation_profile_pc34.c  Phase 1 V1/off defaults.
 *   dm1_v2_movement_command_adapter_pc34.c  presentation-disabled route.
 *
 * Headless: no game assets, no SDL window, no DUNGEON.DAT needed.
 *
 * Schema: firestaff.dm1_v2.side_by_side_presentation_seed_probe.v1
 *
 * Exit codes: 0 = PASS, 1 = FAIL.
 */

#include "dm1_v2_viewport_renderer_pc34.h"
#include "dm1_v2_presentation_profile_pc34.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
#include "dm1_v2_side_by_side_seed_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Side-by-side scaffolding dimensions. The gap is a thin column drawn in
 * the V2 label color so any future visual diff tool can detect alignment
 * drift between the V1 and V2 lanes.
 *
 * The composite width is DM1_V2_VIEWPORT_W * 2 + SIDE_BY_SIDE_GAP_W. The
 * composite height matches the V1/V2 viewport height. */
#define SIDE_BY_SIDE_GAP_W 8
#define SIDE_BY_SIDE_GAP_R 16
#define SIDE_BY_SIDE_GAP_G 16
#define SIDE_BY_SIDE_GAP_B 16
#define SIDE_BY_SIDE_W (DM1_V2_VIEWPORT_W * 2 + SIDE_BY_SIDE_GAP_W)
#define SIDE_BY_SIDE_H DM1_V2_VIEWPORT_H

static int g_failures = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        g_failures++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
    } \
} while (0)

/* FNV-1a 64-bit byte hash. */
static uint64_t fnv1a_u32(uint64_t hash, uint32_t value) {
    int byteIndex;
    for (byteIndex = 0; byteIndex < 4; byteIndex++) {
        hash ^= (uint64_t)((value >> (byteIndex * 8)) & 0xFFU);
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t fnv1a_seed_begin(void) {
    return 14695981039346656037ULL; /* FNV-1a 64-bit offset basis */
}

/* Hash a 224x136 RGBA framebuffer into a 64-bit seed. */
static uint64_t hash_framebuffer(uint64_t hash, const DM1_V2_ViewportState* vp) {
    int y;
    int x;
    if (!vp) return hash;
    /* Source-lock: DUNVIEW.C:2999-3000 fixes the original viewport bitmap at
     * 224x136. We hash rows in order so the seed is stable across machines. */
    for (y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            const DM1_V2_Color* c = &vp->framebuffer[y][x];
            uint32_t packed = ((uint32_t)c->r)
                            | ((uint32_t)c->g << 8)
                            | ((uint32_t)c->b << 16)
                            | ((uint32_t)c->a << 24);
            hash = fnv1a_u32(hash, packed);
        }
    }
    return hash;
}

/* Compare two 224x136 RGBA framebuffers pixel-by-pixel. */
static int framebuffers_equal(const DM1_V2_ViewportState* a,
                              const DM1_V2_ViewportState* b,
                              int* firstMismatchX,
                              int* firstMismatchY) {
    int y, x;
    if (firstMismatchX) *firstMismatchX = -1;
    if (firstMismatchY) *firstMismatchY = -1;
    if (!a || !b) return 0;
    for (y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            const DM1_V2_Color* ca = &a->framebuffer[y][x];
            const DM1_V2_Color* cb = &b->framebuffer[y][x];
            if (ca->r != cb->r || ca->g != cb->g ||
                ca->b != cb->b || ca->a != cb->a) {
                if (firstMismatchX) *firstMismatchX = x;
                if (firstMismatchY) *firstMismatchY = y;
                return 0;
            }
        }
    }
    return 1;
}

/* Hash a side-by-side scaffolding layout:
 *   [ V1 framebuffer | 8-pixel gap in V2 label color | V2 framebuffer ]
 * Width is SIDE_BY_SIDE_W, height SIDE_BY_SIDE_H.
 *
 * The hash sweeps the composite row-by-row; the gap column is hashed using
 * its own RGB triple (16,16,16) so future tooling can also confirm the gap
 * is present and aligned. */
static uint64_t hash_side_by_side(uint64_t hash,
                                  const DM1_V2_ViewportState* v1,
                                  const DM1_V2_ViewportState* v2) {
    int y, x;
    if (!v1 || !v2) return hash;
    for (y = 0; y < SIDE_BY_SIDE_H; y++) {
        for (x = 0; x < SIDE_BY_SIDE_W; x++) {
            uint32_t packed;
            if (x < DM1_V2_VIEWPORT_W) {
                const DM1_V2_Color* c = &v1->framebuffer[y][x];
                packed = ((uint32_t)c->r)
                       | ((uint32_t)c->g << 8)
                       | ((uint32_t)c->b << 16)
                       | ((uint32_t)c->a << 24);
            } else if (x < DM1_V2_VIEWPORT_W + SIDE_BY_SIDE_GAP_W) {
                packed = ((uint32_t)SIDE_BY_SIDE_GAP_R)
                       | ((uint32_t)SIDE_BY_SIDE_GAP_G << 8)
                       | ((uint32_t)SIDE_BY_SIDE_GAP_B << 16)
                       | ((uint32_t)0xFFu << 24);
            } else {
                const DM1_V2_Color* c =
                    &v2->framebuffer[y][x - DM1_V2_VIEWPORT_W - SIDE_BY_SIDE_GAP_W];
                packed = ((uint32_t)c->r)
                       | ((uint32_t)c->g << 8)
                       | ((uint32_t)c->b << 16)
                       | ((uint32_t)c->a << 24);
            }
            hash = fnv1a_u32(hash, packed);
        }
    }
    return hash;
}

/* Build the DM1 PC 3.4 entry composition from the canonical fixture and
 * render it through the V2 viewport renderer with V2 presentation disabled
 * (the only legitimate V1-parity route). The two viewports are produced by
 * independent init/render passes so any state carried over between calls
 * would surface as a framebuffer mismatch. */
static void render_entry_v1_and_v2(DM1_V2_ViewportState* v1,
                                   DM1_V2_ViewportState* v2) {
    const DM1_V2_DungeonStateFixture* fixture = dm1_v2_vp_dm1_pc34_entry_state_fixture();
    DM1_V2_ViewportCompositionInput input;
    if (!v1 || !v2 || !fixture) return;
    PROBE_ASSERT(fixture->startMapX == 1,
                 "fixture.startMapX=1 source=DUNGEON.DAT offset 8 + pass173");
    PROBE_ASSERT(fixture->startMapY == 3,
                 "fixture.startMapY=3 source=DUNGEON.DAT offset 8 + pass173");
    PROBE_ASSERT(fixture->startDirection == 2,
                 "fixture.startDirection=2 (S) source=DUNGEON.DAT offset 8 + pass173");

    PROBE_ASSERT(dm1_v2_vp_build_composition_from_fixture(fixture,
                                                          fixture->startMapX,
                                                          fixture->startMapY,
                                                          fixture->startDirection,
                                                          &input) == 1,
                 "build_composition_from_fixture ok");
    PROBE_ASSERT(input.squares[1][1].element == DM1_V2_ELEMENT_WALL,
                 "input D1C element is WALL (front champion-portrait sensor square)");
    PROBE_ASSERT(input.squares[0][1].element == DM1_V2_ELEMENT_CORRIDOR,
                 "input D0C element is CORRIDOR (party current square)");

    /* V1 route: presentation disabled, identical material colors and draw
     * order as V2 route. The V2 viewport renderer is the only RGBA path
     * that exposes the V1 parity truth to a probe, so the V1 and V2 lanes
     * both go through it with v2PresentationEnabled=0. */
    dm1_v2_vp_init(v1);
    dm1_v2_vp_init(v2);
    PROBE_ASSERT(dm1_v2_vp_render_composition_flat(v1, &input) == 1,
                 "render_composition_flat (V1 lane) ok");
    PROBE_ASSERT(dm1_v2_vp_render_composition_flat(v2, &input) == 1,
                 "render_composition_flat (V2 lane) ok");
}

/* Verify the DM1_V2_PresentationProfile default route stays V1-source-locked
 * and that the movement command adapter keeps V1 gameplay dispatch outside
 * V2 presentation when V2 is disabled. */
static void check_presentation_profile_defaults(void) {
    DM1_V2_PresentationProfile profile;
    dm1_v2_presentation_profile_defaults(&profile);
    PROBE_ASSERT(dm1_v2_presentation_profile_uses_v1_gameplay(&profile) == 1,
                 "defaults: gameplay route pinned to V1 source (GAMELOOP.C:90 / COMMAND.C:2045-2155)");
    PROBE_ASSERT(profile.presentationMode == DM1_V2_PRESENTATION_MODE_V1_ORIGINAL,
                 "defaults: presentation mode V1_ORIGINAL (V2 off by default)");
    PROBE_ASSERT(profile.v2PresentationEnabled == 0,
                 "defaults: v2PresentationEnabled=0 (Phase 1 boot path)");
}

/* V1 source command truth table (ReDMCSB DEFS.H:238-243).
 *
 * The adapter must report routeKind=V1_SOURCE, sourceCommand == runtimeCommand
 * == C-id, and v2PresentationEnabled echoes the requested flag for every
 * C001..C006 row when v2PresentationEnabled=0. Spreading the assertion
 * across all six commands makes any future V2 presentation change that
 * silently re-routes a V1 source command break at least one row.
 *
 * The DM1_V2_MovementCommand enum shares its integer value with the
 * ReDMCSB C-id (TURN_LEFT=1, TURN_RIGHT=2, MOVE_FORWARD=3, MOVE_RIGHT=4,
 * MOVE_BACKWARD=5, MOVE_LEFT=6), so the same integer compares true to
 * both names. This is the V1 source truth and must never change. */
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

static void check_movement_command_route_disabled_v1(void) {
    int i;
    /* V1 source lane: every C001..C006 source command must resolve to a
     * V1_SOURCE route, preserve its source command, and echo the
     * v2PresentationEnabled=0 flag. The movement command adapter must
     * never silently promote a V2 runtime command into a different
     * source command when presentation is disabled. */
    for (i = 0; i < N_V1_COMMAND_ROWS; ++i) {
        DM1_V2_MovementCommandRoute route;
        char id[64];
        snprintf(id, sizeof(id), "v1_command_table[%s]",
                 g_v1_command_table[i].label);
        route = dm1_v2_movement_command_route_for_presentation(
            0, g_v1_command_table[i].v2Command);
        PROBE_ASSERT(route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE,
                     "%s -> V1_SOURCE route (DEFS.H:238-243 / COMMAND.C:2045-2155)",
                     id);
        PROBE_ASSERT(route.v2PresentationEnabled == 0,
                     "%s -> route.v2PresentationEnabled echoes requested flag",
                     id);
        PROBE_ASSERT(route.sourceCommand == g_v1_command_table[i].v1SourceCommand,
                     "%s -> sourceCommand matches ReDMCSB C-id",
                     id);
        PROBE_ASSERT(route.runtimeCommand == route.sourceCommand,
                     "%s -> runtimeCommand == sourceCommand (V1 truth preserved)",
                     id);
    }
}

static void check_source_side_by_side_seed_helper(
    const DM1_V2_ViewportState* v1,
    const DM1_V2_ViewportState* v2,
    uint64_t probeSideBySideHash) {
    DM1_V2_SideBySideSeed seed;
    DM1_V2_Color pixel;
    uint64_t helperHash;
    int scaffoldW = 0;
    int scaffoldH = 0;
    int gapW = 0;

    dm1_v2_side_by_side_seed_scaffold_dimensions(&scaffoldW, &scaffoldH, &gapW);
    PROBE_ASSERT(scaffoldW == SIDE_BY_SIDE_W,
                 "source helper scaffold width matches probe (%d)", SIDE_BY_SIDE_W);
    PROBE_ASSERT(scaffoldH == SIDE_BY_SIDE_H,
                 "source helper scaffold height matches probe (%d)", SIDE_BY_SIDE_H);
    PROBE_ASSERT(gapW == SIDE_BY_SIDE_GAP_W,
                 "source helper gap width matches probe (%d)", SIDE_BY_SIDE_GAP_W);

    helperHash = dm1_v2_side_by_side_seed_hash_layout(v1, v2);
    PROBE_ASSERT(helperHash == probeSideBySideHash,
                 "source helper hash matches probe side-by-side hash: %016llx",
                 (unsigned long long)helperHash);

    PROBE_ASSERT(dm1_v2_side_by_side_seed_build_entry(&seed) == 1,
                 "source helper build_entry succeeds");
    PROBE_ASSERT(seed.lanesByteEqual == 1,
                 "source helper build_entry lanes byte-equal");
    PROBE_ASSERT(seed.mismatchedPixels == 0,
                 "source helper build_entry mismatchedPixels=0");
    PROBE_ASSERT(seed.sideBySideHash == probeSideBySideHash,
                 "source helper build_entry hash matches probe: %016llx",
                 (unsigned long long)seed.sideBySideHash);

    PROBE_ASSERT(dm1_v2_side_by_side_seed_composite_pixel(
                     &seed, DM1_V2_VIEWPORT_W, 0, &pixel) == 1,
                 "source helper composite gap pixel is addressable");
    PROBE_ASSERT(pixel.r == SIDE_BY_SIDE_GAP_R &&
                 pixel.g == SIDE_BY_SIDE_GAP_G &&
                 pixel.b == SIDE_BY_SIDE_GAP_B &&
                 pixel.a == 255,
                 "source helper composite gap pixel matches V2 label colour");
    PROBE_ASSERT(dm1_v2_side_by_side_seed_composite_pixel(
                     &seed,
                     DM1_V2_VIEWPORT_W + SIDE_BY_SIDE_GAP_W,
                     0,
                     &pixel) == 1,
                 "source helper composite V2 lane first pixel is addressable");
    PROBE_ASSERT(pixel.r == seed.v2.framebuffer[0][0].r &&
                 pixel.g == seed.v2.framebuffer[0][0].g &&
                 pixel.b == seed.v2.framebuffer[0][0].b &&
                 pixel.a == seed.v2.framebuffer[0][0].a,
                 "source helper composite V2 lane first pixel matches framebuffer");
}

int main(void) {
    DM1_V2_ViewportState v1;
    DM1_V2_ViewportState v2;
    uint64_t v1Hash;
    uint64_t v2Hash;
    uint64_t sideBySideHash;
    int firstMismatchX = -1;
    int firstMismatchY = -1;
    int equal;

    printf("--- DM1 V2 side-by-side presentation seed ---\n");
    printf("viewport=%dx%d source=DUNVIEW.C:2999-3000\n",
           DM1_V2_VIEWPORT_W, DM1_V2_VIEWPORT_H);
    printf("sideBySide=%dx%d (V1=%d + gap=%d + V2=%d)\n",
           SIDE_BY_SIDE_W, SIDE_BY_SIDE_H,
           DM1_V2_VIEWPORT_W, SIDE_BY_SIDE_GAP_W, DM1_V2_VIEWPORT_W);

    check_presentation_profile_defaults();
    check_movement_command_route_disabled_v1();

    render_entry_v1_and_v2(&v1, &v2);

    equal = framebuffers_equal(&v1, &v2, &firstMismatchX, &firstMismatchY);
    PROBE_ASSERT(equal == 1,
                 "V1/V2 presentation-disabled framebuffers byte-equal (first_mismatch=(%d,%d))",
                 firstMismatchX, firstMismatchY);

    /* Region comparator contract: with the same region the V1 and V2
     * framebuffers must report zero mismatched pixels. The region covers
     * the whole viewport so the contract is symmetric and the V1/V2
     * pipeline cannot silently re-route a single pixel under V2. */
    {
        DM1_V2_ViewportRegion region = {0, 0, DM1_V2_VIEWPORT_W, DM1_V2_VIEWPORT_H, "side-by-side-full"};
        DM1_V2_RegionCompareResult result;
        int ok = dm1_v2_vp_compare_viewport_region(&v1.framebuffer[0][0],
                                                   &v2.framebuffer[0][0],
                                                   DM1_V2_VIEWPORT_W,
                                                   region,
                                                   &result);
        PROBE_ASSERT(ok == 1, "dm1_v2_vp_compare_viewport_region -> match");
        PROBE_ASSERT(result.comparedPixels == DM1_V2_VIEWPORT_W * DM1_V2_VIEWPORT_H,
                     "comparedPixels=%d (full viewport)",
                     DM1_V2_VIEWPORT_W * DM1_V2_VIEWPORT_H);
        PROBE_ASSERT(result.mismatchedPixels == 0,
                     "mismatchedPixels=0 (V1 == V2 presentation-disabled)");
    }

    v1Hash = hash_framebuffer(fnv1a_seed_begin(), &v1);
    v2Hash = hash_framebuffer(fnv1a_seed_begin(), &v2);
    PROBE_ASSERT(v1Hash == v2Hash,
                 "framebuffer hash parity: v1=%016llx v2=%016llx",
                 (unsigned long long)v1Hash, (unsigned long long)v2Hash);

    sideBySideHash = hash_side_by_side(fnv1a_seed_begin(), &v1, &v2);
    check_source_side_by_side_seed_helper(&v1, &v2, sideBySideHash);
    printf("sideBySideHash=%016llx\n", (unsigned long long)sideBySideHash);
    printf("v1Hash=%016llx v2Hash=%016llx\n",
           (unsigned long long)v1Hash, (unsigned long long)v2Hash);

    if (g_failures) {
        fprintf(stderr, "result=FAIL failures=%d\n", g_failures);
        return 1;
    }
    printf("result=PASS\n");
    return 0;
}
