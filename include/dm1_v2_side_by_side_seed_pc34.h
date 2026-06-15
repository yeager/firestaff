/*
 * dm1_v2_side_by_side_seed_pc34.h
 *
 * DM1 V2 — V1/V2 side-by-side verification seed (PC 3.4 compatibility).
 *
 * This header is the source-side anchor for the small, deterministic
 * V1/V2 side-by-side verification seed that downstream screenshot and
 * pixel-diff gates can plug into. It centralises three things that
 * would otherwise drift between probes and tests:
 *
 *   1. The canonical side-by-side layout (gap width, gap RGB, lane
 *      ordering) used when a V1 framebuffer and a V2 framebuffer are
 *      composed into a single side-by-side image.
 *
 *   2. A small builder, dm1_v2_side_by_side_seed_build_entry, that
 *      renders the DM1 PC 3.4 entry composition (DUNGEON.DAT offset
 *      8 + pass173 source audit, startMapX=1, startMapY=3,
 *      startDirection=2) into a V1 lane and a V2 lane and emits a
 *      DM1_V2_SideBySideSeed with both framebuffers, the canonical
 *      scaffold dimensions, and a stable 64-bit FNV-1a side-by-side
 *      hash. With v2PresentationEnabled=0 the V1 and V2 lanes are
 *      byte-equal pixel-by-pixel; the hash captures the canonical
 *      "V1 || gap || V2" composite in a single value.
 *
 *   3. A hash helper, dm1_v2_side_by_side_seed_hash_layout, that
 *      re-hashes any 224x136 V1 + 224x136 V2 pair into the same
 *      canonical scaffold hash, so future visual-diff tooling can
 *      reproduce the seed without re-deriving the layout.
 *
 * Source locks (ReDMCSB):
 *   DEFS.H:238-243          C001..C006 V1 source command ids.
 *   COMMAND.C:2045-2155     F0359 command queue dispatch.
 *   DUNVIEW.C:2999-3000     224x136 V1/V2 viewport bitmap.
 *   DUNVIEW.C:3913-3928     D1C champion portrait 32x29 at {96,35}.
 *   COORD.C:1721-1722       224x136 viewport.
 *   DUNGEON.C:2573,2610-12  C127 sensor view-direction mapping.
 *
 * The seed is presentation-disabled: with v2PresentationEnabled=0 the
 * V1 source truth is preserved and the V2 runtime shell is reduced
 * to the V1 framebuffer. The V1 movement command adapter must still
 * report routeKind=V1_SOURCE for every C001..C006 command.
 *
 * This header is headless: it does not require game data files and
 * does not initialise SDL. It depends on the firestaff_v2 library
 * (which contains dm1_v2_viewport_renderer_pc34 and
 * dm1_v2_movement_command_adapter_pc34).
 */

#ifndef FIRESTAFF_DM1_V2_SIDE_BY_SIDE_SEED_PC34_H
#define FIRESTAFF_DM1_V2_SIDE_BY_SIDE_SEED_PC34_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v2_viewport_renderer_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Canonical V1/V2 side-by-side layout ──────────────────────────
 *
 * The composite is "V1 lane || gap || V2 lane", all rows in lock-step
 * at the same viewport height. The gap is a thin column drawn in the
 * V2 label colour (16,16,16) so any future visual diff tool can
 * confirm alignment between the two lanes.
 *
 *   DM1_V2_SIDE_BY_SIDE_GAP_W     8 px
 *   DM1_V2_SIDE_BY_SIDE_GAP_RGB  (16,16,16) — V2 label colour
 *   DM1_V2_SIDE_BY_SIDE_W        VIEWPORT_W * 2 + GAP_W = 456 px
 *   DM1_V2_SIDE_BY_SIDE_H        VIEWPORT_H = 136 px
 */
#define DM1_V2_SIDE_BY_SIDE_GAP_W   8
#define DM1_V2_SIDE_BY_SIDE_GAP_R   16
#define DM1_V2_SIDE_BY_SIDE_GAP_G   16
#define DM1_V2_SIDE_BY_SIDE_GAP_B   16
#define DM1_V2_SIDE_BY_SIDE_GAP_A   255

#define DM1_V2_SIDE_BY_SIDE_W \
    (DM1_V2_VIEWPORT_W * 2 + DM1_V2_SIDE_BY_SIDE_GAP_W)
#define DM1_V2_SIDE_BY_SIDE_H DM1_V2_VIEWPORT_H

/* Canonical V1 viewport scaffold constants (mirrored from the probe
 * scaffold so the seed is reproducible without depending on the
 * probe source files). See probes/firestaff_dm1_v2_v1_v2_side_by_side_seed_probe.c
 * for the original definitions and the ReDMCSB anchors. */
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W 32
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H 29
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X 96
#define DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y 35
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_X     32
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_Y      9
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_W    160
#define DM1_V2_SIDE_BY_SIDE_D1C_WALL_H    111

/* FNV-1a 64-bit offset basis. Same constant used by the V1/V2 state
 * hash seed (tests/test_dm1_v2_source_route_state_hash_pc34.c) so all
 * Firestaff V1/V2 seeds can be compared in a single basis. */
#define DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS \
    ((uint64_t)0xcbf29ce484222325ULL)
#define DM1_V2_SIDE_BY_SIDE_FNV1A_PRIME \
    ((uint64_t)0x100000001b3ULL)

/* ── Side-by-side seed result ─────────────────────────────────────
 *
 * Carries the V1 and V2 lanes, the canonical scaffold dimensions,
 * the byte-equality flag, and a stable 64-bit hash. The hash is the
 * FNV-1a 64-bit fold of the "V1 || gap || V2" composite layout
 * traversed row-by-row. It is deterministic across machines and
 * builds, so any future visual-diff gate can lock a known-good seed.
 */
typedef struct {
    DM1_V2_ViewportState v1;
    DM1_V2_ViewportState v2;
    int scaffoldW;
    int scaffoldH;
    int gapW;
    int lanesByteEqual;
    int mismatchedPixels;
    int firstMismatchX;
    int firstMismatchY;
    uint64_t sideBySideHash;
} DM1_V2_SideBySideSeed;

/* Return the canonical V1/V2 side-by-side scaffold dimensions.
 * Writes W, H, gapW into the out-args. Safe with any NULL out-arg. */
void dm1_v2_side_by_side_seed_scaffold_dimensions(int* outW,
                                                  int* outH,
                                                  int* outGapW);

/* FNV-1a 64-bit fold of one RGBA pixel (r,g,b,a in DM1_V2_Color). */
uint64_t dm1_v2_side_by_side_seed_hash_color(uint64_t hash,
                                             const DM1_V2_Color* color);

/* FNV-1a 64-bit fold of the canonical "V1 || gap || V2" composite
 * for any pair of 224x136 viewport states. Returns the same value
 * the build_entry seed would produce, so downstream visual-diff
 * tooling can reproduce the seed hash from external buffers. */
uint64_t dm1_v2_side_by_side_seed_hash_layout(const DM1_V2_ViewportState* v1,
                                              const DM1_V2_ViewportState* v2);

/* Sample one pixel from the canonical "V1 || gap || V2" composite
 * described above. This is the small screenshot/pixel-scaffolding
 * accessor for future visual-diff gates: callers can verify lane
 * alignment and content without duplicating the V1/gap/V2 x-coordinate
 * math. Returns 1 on success and writes *outColor. Returns 0 for NULL
 * args or out-of-bounds composite coordinates. */
int dm1_v2_side_by_side_seed_composite_pixel(
    const DM1_V2_SideBySideSeed* seed,
    int x,
    int y,
    DM1_V2_Color* outColor);

/* Write the canonical "V1 || gap || V2" composite into a caller-owned
 * row-major RGBA8888 buffer. outStrideBytes is the destination row
 * stride; it must be at least DM1_V2_SIDE_BY_SIDE_W * 4. outByteCount
 * must cover the last written byte. Padding bytes beyond the composite
 * width are left untouched, so screenshot probes can use aligned rows
 * without the helper hiding padding corruption. Returns 1 on success,
 * 0 for NULL args, invalid stride, or too-small buffers. */
int dm1_v2_side_by_side_seed_write_rgba8888(
    const DM1_V2_SideBySideSeed* seed,
    unsigned char* out,
    size_t outByteCount,
    int outStrideBytes);

/* Render the DM1 PC 3.4 entry composition into a V1 lane and a V2
 * lane (both presentation-disabled), populate *out with the seed
 * fields, and return 1 on success. Returns 0 if out is NULL, the
 * entry fixture is unavailable, or the renderer rejects the input.
 *
 * The renderer is the existing V2 viewport renderer
 * (dm1_v2_vp_render_composition_flat) running with V2 presentation
 * disabled. The V1 and V2 lanes are produced by independent
 * init/render passes so any state carried over between calls would
 * surface as a framebuffer mismatch (lanesByteEqual == 0). */
int dm1_v2_side_by_side_seed_build_entry(DM1_V2_SideBySideSeed* out);

/* ── V1 viewport geometry scaffold (screenshot/pixel scaffolding) ─
 *
 * Centralised accessor for the V1 viewport geometry constants that
 * the probe and ctest lock under the side-by-side seed. Both the
 * D1C champion-portrait square (ReDMCSB DUNVIEW.C:3913-3928, 32x29
 * blit at {96,35} viewport-local) and the D1C wall panel
 * (ReDMCSB DUNVIEW.C:581-593 G0163_aauc_Graphic558_Frame_Walls[12][8]
 * D1C row indexed by M606_VIEW_SQUARE_D1C, 160x111 panel at {32,9}
 * viewport-local) are part of the V1 source truth, and any future
 * screenshot-diff or pixel scaffolding gate needs both. The
 * accessor returns them in a single struct so callers do not have
 * to pick the macros up by hand and risk a typo.
 *
 * The sourceAnchor fields cite the ReDMCSB source line ranges so a
 * future regression that silently changes a constant is anchored
 * back to a specific line in the V1 source. NULL out is safe; the
 * function returns 0 in that case. Returns 1 on success. */
typedef struct {
    int viewportW;
    int viewportH;
    int d1cPortraitW;
    int d1cPortraitH;
    int d1cPortraitX;
    int d1cPortraitY;
    int d1cWallX;
    int d1cWallY;
    int d1cWallW;
    int d1cWallH;
    const char* portraitAnchor;
    const char* wallAnchor;
} DM1_V2_SideBySideV1Geometry;

int dm1_v2_side_by_side_seed_v1_geometry(DM1_V2_SideBySideV1Geometry* out);

/* ── Side-by-side composite region manifest ──────────────────────
 *
 * Named rectangles in composite coordinates for screenshot and pixel
 * probes. The lane rectangles cover the full V1/gap/V2 scaffold.
 * The D1C wall and portrait rectangles are the V1 source-locked
 * geometry from dm1_v2_side_by_side_seed_v1_geometry(), translated
 * into each lane of the side-by-side composite.
 */
typedef enum {
    DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE = 0,
    DM1_V2_SIDE_BY_SIDE_REGION_GAP,
    DM1_V2_SIDE_BY_SIDE_REGION_V2_LANE,
    DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_WALL,
    DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_PORTRAIT,
    DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_WALL,
    DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_PORTRAIT,
    DM1_V2_SIDE_BY_SIDE_REGION_COUNT
} DM1_V2_SideBySideRegionId;

typedef struct {
    DM1_V2_SideBySideRegionId id;
    int x;
    int y;
    int w;
    int h;
    const char* label;
    const char* sourceAnchor;
} DM1_V2_SideBySideRegion;

int dm1_v2_side_by_side_seed_region(DM1_V2_SideBySideRegionId id,
                                    DM1_V2_SideBySideRegion* out);

/* Hash every pixel in one named composite-space region. The hash uses
 * the same RGBA FNV-1a fold as dm1_v2_side_by_side_seed_hash_layout(),
 * but it only sweeps the requested manifest rectangle. Returns the
 * FNV-1a basis for invalid inputs; writes the number of folded pixels
 * when outPixelCount is non-NULL. */
uint64_t dm1_v2_side_by_side_seed_hash_region(
    const DM1_V2_SideBySideSeed* seed,
    DM1_V2_SideBySideRegionId id,
    int* outPixelCount);

/* Full pixel comparison for two same-sized side-by-side manifest
 * rectangles. This is the screenshot/pixel gate helper that binds the
 * manifest rectangles to actual framebuffer content instead of only
 * checking their coordinates. Returns 1 when both regions are valid,
 * same-sized, and byte-identical. Returns 0 on invalid inputs, size
 * mismatch, or at least one pixel mismatch. */
typedef struct {
    int comparedPixels;
    int mismatchedPixels;
    int firstMismatchAX;
    int firstMismatchAY;
    int firstMismatchBX;
    int firstMismatchBY;
} DM1_V2_SideBySideRegionCompareResult;

int dm1_v2_side_by_side_seed_compare_regions(
    const DM1_V2_SideBySideSeed* seed,
    DM1_V2_SideBySideRegionId a,
    DM1_V2_SideBySideRegionId b,
    DM1_V2_SideBySideRegionCompareResult* result);

/* Return the ReDMCSB source evidence string for this seed module. */
const char* dm1_v2_side_by_side_seed_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_SIDE_BY_SIDE_SEED_PC34_H */
