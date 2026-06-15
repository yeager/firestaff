/*
 * dm1_v2_side_by_side_seed_pc34.c
 *
 * DM1 V2 — V1/V2 side-by-side verification seed (PC 3.4 compatibility).
 *
 * Implementation of the small, deterministic V1/V2 side-by-side
 * verification seed. The seed renders the DM1 PC 3.4 entry
 * composition once through the V2 viewport renderer with V2
 * presentation disabled, captures the V1 lane and the V2 lane as
 * independent 224x136 RGBA framebuffers, and emits a stable 64-bit
 * FNV-1a side-by-side hash.
 *
 * With v2PresentationEnabled=0 the V1 source truth is preserved
 * (DEFS.H:238-243 C001..C006, COMMAND.C:2045-2155 F0359) and the
 * V2 runtime shell is reduced to the V1 framebuffer. The V1 and V2
 * lanes are byte-equal pixel-by-pixel, so the seed hash captures a
 * canonical "V1 || gap || V2" composite in a single value that
 * downstream visual-diff gates can lock to a known-good baseline.
 *
 * The seed is small: a 4x3 viewport composition (D0..D3, L/C/R) at
 * the canonical DM1 entry position, plus an FNV-1a 64-bit fold of
 * the resulting layout. No game data is required and no SDL
 * rendering is initialised.
 *
 * Source locks (ReDMCSB):
 *   DEFS.H:238-243          C001..C006 V1 source command ids.
 *   COMMAND.C:2045-2155     F0359 command queue dispatch.
 *   DUNVIEW.C:2999-3000     224x136 V1/V2 viewport bitmap.
 *   DUNVIEW.C:3913-3928     D1C champion portrait 32x29 at {96,35}.
 *   DUNVIEW.C:8337-8338     draw floor/ceiling before walking squares.
 *   DUNVIEW.C:8490-8542     D3 -> D0 left/center/right draw order.
 *   COORD.C:1721-1722       224x136 viewport.
 *   DUNGEON.C:2573,2610-12  C127 sensor view-direction mapping.
 *   GAMELOOP.C:90           F0128_DUNGEONVIEW_Draw_CPSF snapshot draw.
 *
 * This file is headless and depends on the firestaff_v2 library.
 */

#include "dm1_v2_side_by_side_seed_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"

#include <stddef.h>
#include <string.h>

/* Source evidence string, returned by
 * dm1_v2_side_by_side_seed_source_evidence(). */
static const char* kSourceEvidence =
    "ReDMCSB DEFS.H:238-243 C001..C006 V1 source command ids; "
    "COMMAND.C:2045-2155 F0359 command queue dispatch; "
    "DUNVIEW.C:2999-3000 224x136 V1/V2 viewport bitmap; "
    "DUNVIEW.C:3913-3928 D1C champion portrait 32x29 at {96,35}; "
    "DUNVIEW.C:8337-8338 draw floor/ceiling before walking squares; "
    "DUNVIEW.C:8490-8542 D3 -> D0 left/center/right draw order; "
    "COORD.C:1721-1722 224x136 viewport; "
    "DUNGEON.C:2573/2610-2612 C127 sensor view-direction mapping; "
    "GAMELOOP.C:90 F0128_DUNGEONVIEW_Draw_CPSF snapshot draw.";

void dm1_v2_side_by_side_seed_scaffold_dimensions(int* outW,
                                                  int* outH,
                                                  int* outGapW) {
    if (outW) *outW = DM1_V2_SIDE_BY_SIDE_W;
    if (outH) *outH = DM1_V2_SIDE_BY_SIDE_H;
    if (outGapW) *outGapW = DM1_V2_SIDE_BY_SIDE_GAP_W;
}

uint64_t dm1_v2_side_by_side_seed_hash_color(uint64_t hash,
                                             const DM1_V2_Color* color) {
    if (!color) return hash;
    /* Fold the four RGBA bytes one at a time, LSB-first, so the
     * hash is stable across endianness. */
    hash ^= (uint64_t)color->r;
    hash *= DM1_V2_SIDE_BY_SIDE_FNV1A_PRIME;
    hash ^= (uint64_t)color->g;
    hash *= DM1_V2_SIDE_BY_SIDE_FNV1A_PRIME;
    hash ^= (uint64_t)color->b;
    hash *= DM1_V2_SIDE_BY_SIDE_FNV1A_PRIME;
    hash ^= (uint64_t)color->a;
    hash *= DM1_V2_SIDE_BY_SIDE_FNV1A_PRIME;
    return hash;
}

uint64_t dm1_v2_side_by_side_seed_hash_layout(const DM1_V2_ViewportState* v1,
                                              const DM1_V2_ViewportState* v2) {
    uint64_t hash = DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS;
    int y, x;
    if (!v1 || !v2) return hash;

    /* Sweep the canonical "V1 || gap || V2" layout row-by-row.
     * DUNVIEW.C:2999-3000 fixes both lanes at 224x136; the gap is
     * DM1_V2_SIDE_BY_SIDE_GAP_W pixels of (16,16,16,255) V2-label
     * colour, indexed by the same y. */
    for (y = 0; y < DM1_V2_SIDE_BY_SIDE_H; ++y) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
            hash = dm1_v2_side_by_side_seed_hash_color(
                hash, &v1->framebuffer[y][x]);
        }
        /* Hash the gap column with the canonical V2 label colour
         * so the seed captures alignment as well as content. */
        for (x = 0; x < DM1_V2_SIDE_BY_SIDE_GAP_W; ++x) {
            const DM1_V2_Color gapColor = {
                DM1_V2_SIDE_BY_SIDE_GAP_R,
                DM1_V2_SIDE_BY_SIDE_GAP_G,
                DM1_V2_SIDE_BY_SIDE_GAP_B,
                DM1_V2_SIDE_BY_SIDE_GAP_A
            };
            hash = dm1_v2_side_by_side_seed_hash_color(hash, &gapColor);
        }
        for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
            hash = dm1_v2_side_by_side_seed_hash_color(
                hash, &v2->framebuffer[y][x]);
        }
    }
    return hash;
}

int dm1_v2_side_by_side_seed_composite_pixel(
    const DM1_V2_SideBySideSeed* seed,
    int x,
    int y,
    DM1_V2_Color* outColor) {
    if (!seed || !outColor) return 0;
    if (x < 0 || y < 0 ||
        x >= DM1_V2_SIDE_BY_SIDE_W ||
        y >= DM1_V2_SIDE_BY_SIDE_H) {
        return 0;
    }

    /* Pixel scaffold for the canonical row-major side-by-side
     * composite. DUNVIEW.C:2999-3000 fixes both lanes at 224x136;
     * the middle gap is a deterministic V2-label colour so screenshot
     * gates can detect lane alignment drift independently of viewport
     * content. */
    if (x < DM1_V2_VIEWPORT_W) {
        *outColor = seed->v1.framebuffer[y][x];
    } else if (x < DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W) {
        outColor->r = DM1_V2_SIDE_BY_SIDE_GAP_R;
        outColor->g = DM1_V2_SIDE_BY_SIDE_GAP_G;
        outColor->b = DM1_V2_SIDE_BY_SIDE_GAP_B;
        outColor->a = DM1_V2_SIDE_BY_SIDE_GAP_A;
    } else {
        *outColor =
            seed->v2.framebuffer[y][x - DM1_V2_VIEWPORT_W - DM1_V2_SIDE_BY_SIDE_GAP_W];
    }
    return 1;
}

int dm1_v2_side_by_side_seed_write_rgba8888(
    const DM1_V2_SideBySideSeed* seed,
    unsigned char* out,
    size_t outByteCount,
    int outStrideBytes) {
    const size_t rowBytes = (size_t)DM1_V2_SIDE_BY_SIDE_W * 4u;
    size_t requiredBytes;
    int y, x;

    if (!seed || !out) return 0;
    if (outStrideBytes < (int)rowBytes) return 0;
    requiredBytes = (size_t)(DM1_V2_SIDE_BY_SIDE_H - 1) *
                    (size_t)outStrideBytes + rowBytes;
    if (outByteCount < requiredBytes) return 0;

    /* Screenshot scaffold export for the canonical row-major composite.
     * DUNVIEW.C:2999-3000 fixes both source lanes at 224x136; this writer
     * keeps the same V1/gap/V2 coordinate split as composite_pixel() so
     * future screenshot probes do not duplicate lane math. */
    for (y = 0; y < DM1_V2_SIDE_BY_SIDE_H; ++y) {
        unsigned char* row = out + (size_t)y * (size_t)outStrideBytes;
        for (x = 0; x < DM1_V2_SIDE_BY_SIDE_W; ++x) {
            DM1_V2_Color c;
            if (!dm1_v2_side_by_side_seed_composite_pixel(seed, x, y, &c)) {
                return 0;
            }
            row[(size_t)x * 4u + 0u] = c.r;
            row[(size_t)x * 4u + 1u] = c.g;
            row[(size_t)x * 4u + 2u] = c.b;
            row[(size_t)x * 4u + 3u] = c.a;
        }
    }
    return 1;
}

int dm1_v2_side_by_side_seed_build_entry(DM1_V2_SideBySideSeed* out) {
    const DM1_V2_DungeonStateFixture* fixture;
    DM1_V2_ViewportCompositionInput input;
    int y, x;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->scaffoldW = DM1_V2_SIDE_BY_SIDE_W;
    out->scaffoldH = DM1_V2_SIDE_BY_SIDE_H;
    out->gapW = DM1_V2_SIDE_BY_SIDE_GAP_W;
    out->firstMismatchX = -1;
    out->firstMismatchY = -1;
    out->sideBySideHash = DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS;

    fixture = dm1_v2_vp_dm1_pc34_entry_state_fixture();
    if (!fixture) return 0;
    if (!dm1_v2_vp_build_composition_from_fixture(fixture,
                                                  fixture->startMapX,
                                                  fixture->startMapY,
                                                  fixture->startDirection,
                                                  &input)) {
        return 0;
    }

    /* V1 lane: independent init + render pass.
     * V2 lane: independent init + render pass with the same input.
     * The V2 viewport renderer is the only RGBA path that exposes
     * the V1 parity truth to a probe, so the V1 and V2 lanes both
     * go through it with v2PresentationEnabled=0. */
    dm1_v2_vp_init(&out->v1);
    dm1_v2_vp_init(&out->v2);
    if (!dm1_v2_vp_render_composition_flat(&out->v1, &input)) return 0;
    if (!dm1_v2_vp_render_composition_flat(&out->v2, &input)) return 0;

    /* Byte-equal pixel-by-pixel check across the full 224x136
     * viewport (DUNVIEW.C:2999-3000). Record the first mismatch so
     * downstream gates can localise the drift. */
    out->mismatchedPixels = 0;
    out->lanesByteEqual = 1;
    for (y = 0; y < DM1_V2_VIEWPORT_H; ++y) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
            const DM1_V2_Color* a = &out->v1.framebuffer[y][x];
            const DM1_V2_Color* b = &out->v2.framebuffer[y][x];
            if (a->r != b->r || a->g != b->g ||
                a->b != b->b || a->a != b->a) {
                if (out->mismatchedPixels == 0) {
                    out->firstMismatchX = x;
                    out->firstMismatchY = y;
                }
                ++out->mismatchedPixels;
                out->lanesByteEqual = 0;
            }
        }
    }

    /* Side-by-side hash: V1 || gap || V2 composite. */
    out->sideBySideHash = dm1_v2_side_by_side_seed_hash_layout(&out->v1, &out->v2);
    return 1;
}

int dm1_v2_side_by_side_seed_v1_geometry(DM1_V2_SideBySideV1Geometry* out) {
    if (!out) return 0;
    /* Viewport dimensions: ReDMCSB COORD.C:1721-1722 + DUNVIEW.C:2999-3000. */
    out->viewportW = DM1_V2_VIEWPORT_W;
    out->viewportH = DM1_V2_VIEWPORT_H;
    /* D1C champion portrait: ReDMCSB DUNVIEW.C:3913-3928 (32x29 blit at
     * viewport-local {96,35}). These are the V1 source truth for the
     * front-wall champion-portrait sensor square (sensor C127 view-
     * direction mapping DUNGEON.C:2573,2610-12). */
    out->d1cPortraitW = DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_W;
    out->d1cPortraitH = DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_H;
    out->d1cPortraitX = DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_X;
    out->d1cPortraitY = DM1_V2_SIDE_BY_SIDE_D1C_PORTRAIT_Y;
    out->portraitAnchor = "ReDMCSB DUNVIEW.C:3913-3928";
    /* D1C wall panel: ReDMCSB DUNVIEW.C:581-593
     * G0163_aauc_Graphic558_Frame_Walls[12][8] D1C row indexed by
     * M606_VIEW_SQUARE_D1C (160x111 panel at viewport-local {32,9}).
     * The wall panel is the D1C front-aspect blit for direction=2
     * (south-facing party) when the front square is a WALL element,
     * which is the DM1 PC 3.4 entry state. */
    out->d1cWallX = DM1_V2_SIDE_BY_SIDE_D1C_WALL_X;
    out->d1cWallY = DM1_V2_SIDE_BY_SIDE_D1C_WALL_Y;
    out->d1cWallW = DM1_V2_SIDE_BY_SIDE_D1C_WALL_W;
    out->d1cWallH = DM1_V2_SIDE_BY_SIDE_D1C_WALL_H;
    out->wallAnchor = "ReDMCSB DUNVIEW.C:581-593 (G0163_aauc_Graphic558_Frame_Walls[12][8] D1C row, M606_VIEW_SQUARE_D1C)";
    return 1;
}

int dm1_v2_side_by_side_seed_region(DM1_V2_SideBySideRegionId id,
                                    DM1_V2_SideBySideRegion* out) {
    DM1_V2_SideBySideV1Geometry geom;
    const int v2LaneX = DM1_V2_VIEWPORT_W + DM1_V2_SIDE_BY_SIDE_GAP_W;
    if (!out) return 0;
    if (id < 0 || id >= DM1_V2_SIDE_BY_SIDE_REGION_COUNT) return 0;
    if (!dm1_v2_side_by_side_seed_v1_geometry(&geom)) return 0;

    memset(out, 0, sizeof(*out));
    out->id = id;

    switch (id) {
    case DM1_V2_SIDE_BY_SIDE_REGION_V1_LANE:
        out->x = 0;
        out->y = 0;
        out->w = DM1_V2_VIEWPORT_W;
        out->h = DM1_V2_VIEWPORT_H;
        out->label = "v1_lane";
        out->sourceAnchor = "ReDMCSB COORD.C:1721-1722; DUNVIEW.C:2999-3000";
        return 1;
    case DM1_V2_SIDE_BY_SIDE_REGION_GAP:
        out->x = DM1_V2_VIEWPORT_W;
        out->y = 0;
        out->w = DM1_V2_SIDE_BY_SIDE_GAP_W;
        out->h = DM1_V2_VIEWPORT_H;
        out->label = "gap";
        out->sourceAnchor = "Firestaff canonical V1/V2 side-by-side gap";
        return 1;
    case DM1_V2_SIDE_BY_SIDE_REGION_V2_LANE:
        out->x = v2LaneX;
        out->y = 0;
        out->w = DM1_V2_VIEWPORT_W;
        out->h = DM1_V2_VIEWPORT_H;
        out->label = "v2_lane";
        out->sourceAnchor = "ReDMCSB COORD.C:1721-1722; DUNVIEW.C:2999-3000";
        return 1;
    case DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_WALL:
        out->x = geom.d1cWallX;
        out->y = geom.d1cWallY;
        out->w = geom.d1cWallW;
        out->h = geom.d1cWallH;
        out->label = "v1_d1c_wall";
        out->sourceAnchor = geom.wallAnchor;
        return 1;
    case DM1_V2_SIDE_BY_SIDE_REGION_V1_D1C_PORTRAIT:
        out->x = geom.d1cPortraitX;
        out->y = geom.d1cPortraitY;
        out->w = geom.d1cPortraitW;
        out->h = geom.d1cPortraitH;
        out->label = "v1_d1c_portrait";
        out->sourceAnchor = geom.portraitAnchor;
        return 1;
    case DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_WALL:
        out->x = v2LaneX + geom.d1cWallX;
        out->y = geom.d1cWallY;
        out->w = geom.d1cWallW;
        out->h = geom.d1cWallH;
        out->label = "v2_d1c_wall";
        out->sourceAnchor = geom.wallAnchor;
        return 1;
    case DM1_V2_SIDE_BY_SIDE_REGION_V2_D1C_PORTRAIT:
        out->x = v2LaneX + geom.d1cPortraitX;
        out->y = geom.d1cPortraitY;
        out->w = geom.d1cPortraitW;
        out->h = geom.d1cPortraitH;
        out->label = "v2_d1c_portrait";
        out->sourceAnchor = geom.portraitAnchor;
        return 1;
    default:
        return 0;
    }
}

uint64_t dm1_v2_side_by_side_seed_hash_region(
    const DM1_V2_SideBySideSeed* seed,
    DM1_V2_SideBySideRegionId id,
    int* outPixelCount) {
    DM1_V2_SideBySideRegion region;
    uint64_t hash = DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS;
    int dx, dy;

    if (outPixelCount) *outPixelCount = 0;
    if (!seed) return hash;
    if (!dm1_v2_side_by_side_seed_region(id, &region)) return hash;

    /* Full screenshot-region hash. The region manifest carries
     * ReDMCSB viewport/D1C anchors; this fold binds those rectangles
     * to real seed pixels instead of leaving them as coordinates only. */
    for (dy = 0; dy < region.h; ++dy) {
        for (dx = 0; dx < region.w; ++dx) {
            DM1_V2_Color c;
            if (!dm1_v2_side_by_side_seed_composite_pixel(
                    seed, region.x + dx, region.y + dy, &c)) {
                return DM1_V2_SIDE_BY_SIDE_FNV1A_BASIS;
            }
            hash = dm1_v2_side_by_side_seed_hash_color(hash, &c);
            if (outPixelCount) (*outPixelCount)++;
        }
    }
    return hash;
}

int dm1_v2_side_by_side_seed_compare_regions(
    const DM1_V2_SideBySideSeed* seed,
    DM1_V2_SideBySideRegionId a,
    DM1_V2_SideBySideRegionId b,
    DM1_V2_SideBySideRegionCompareResult* result) {
    DM1_V2_SideBySideRegion ra;
    DM1_V2_SideBySideRegion rb;
    DM1_V2_SideBySideRegionCompareResult local;
    int dx, dy;

    if (result) {
        result->comparedPixels = 0;
        result->mismatchedPixels = 0;
        result->firstMismatchAX = -1;
        result->firstMismatchAY = -1;
        result->firstMismatchBX = -1;
        result->firstMismatchBY = -1;
    }
    if (!seed) return 0;
    if (!dm1_v2_side_by_side_seed_region(a, &ra)) return 0;
    if (!dm1_v2_side_by_side_seed_region(b, &rb)) return 0;
    if (ra.w != rb.w || ra.h != rb.h) return 0;

    local.comparedPixels = 0;
    local.mismatchedPixels = 0;
    local.firstMismatchAX = -1;
    local.firstMismatchAY = -1;
    local.firstMismatchBX = -1;
    local.firstMismatchBY = -1;

    /* Pixel gate for named manifest rectangles. DUNVIEW.C:2999-3000
     * fixes lane dimensions, while DUNVIEW.C:581-593 and 3913-3928
     * fix the D1C wall/portrait rectangles this helper compares. */
    for (dy = 0; dy < ra.h; ++dy) {
        for (dx = 0; dx < ra.w; ++dx) {
            DM1_V2_Color ca;
            DM1_V2_Color cb;
            const int ax = ra.x + dx;
            const int ay = ra.y + dy;
            const int bx = rb.x + dx;
            const int by = rb.y + dy;
            if (!dm1_v2_side_by_side_seed_composite_pixel(seed, ax, ay, &ca)) {
                return 0;
            }
            if (!dm1_v2_side_by_side_seed_composite_pixel(seed, bx, by, &cb)) {
                return 0;
            }
            ++local.comparedPixels;
            if (ca.r != cb.r || ca.g != cb.g ||
                ca.b != cb.b || ca.a != cb.a) {
                if (local.mismatchedPixels == 0) {
                    local.firstMismatchAX = ax;
                    local.firstMismatchAY = ay;
                    local.firstMismatchBX = bx;
                    local.firstMismatchBY = by;
                }
                ++local.mismatchedPixels;
            }
        }
    }
    if (result) *result = local;
    return local.mismatchedPixels == 0;
}

const char* dm1_v2_side_by_side_seed_source_evidence(void) {
    return kSourceEvidence;
}
