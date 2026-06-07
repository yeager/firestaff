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

const char* dm1_v2_side_by_side_seed_source_evidence(void) {
    return kSourceEvidence;
}
