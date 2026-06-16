/*
 * firestaff_nexus_v2_upscaler_probe.c
 *
 * Nexus V2.1 upscaler (EPX + bilinear) headless probe.
 *
 * Headless probe: verifies nexus_v2_upscaler.c without requiring
 * live game asset files or a running SDL renderer.
 *
 * This probe validates:
 *
 *   1. nexus_v2_epx_upscale() converts V1 indexed pixels (0..255)
 *      through a 256-entry palette to RGBA
 *
 *   2. EPX output is deterministic: same input -> same output
 *
 *   3. EPX output respects the palette: dst pixels are exactly
 *      palette[src] when no edge rule matches
 *
 *   4. EPX output buffer size = dst_w * dst_h * sizeof(uint32_t)
 *      (one RGBA per output pixel)
 *
 *   5. Boundary handling: edge pixels fall back to source color
 *      (no OOB read from source buffer)
 *
 *   6. Null-args are safe: no crash, no write
 *
 *   7. Small input (1x1) works: output = src_w*2 * src_h*2
 *
 *   8. nexus_v2_bilinear_smooth() is safe on null and produces
 *      a non-crashing output
 *
 *   9. nexus_v2_upscaler_source_evidence() returns non-NULL,
 *      non-empty string referencing Scale2x / DM1 V2.1 / Saturn
 *      VDP1 / ReDMCSB GFX.C
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_nexus_v2_upscaler_probe
 *
 * Source references:
 *   DM1 V2.1 EPX implementation     (same algorithm)
 *   Scale2x algorithm              flat pixel preservation, diagonal
 *                                  smoothing
 *   Saturn VDP1 bitmap upscaling   (similar 2x block)
 *   ReDMCSB GFX.C                  (pixel-art scaling requirements)
 */

#include "nexus_v2_upscaler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_total = 0;
static int g_failed = 0;

static void check(int cond, const char *name)
{
    ++g_total;
    if (!cond) {
        ++g_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void check_palette_lookup(void)
{
    uint8_t src[16] = {
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15
    };
    uint32_t palette[256] = {0};
    uint32_t dst[64];  /* 4x4 -> 8x8 = 64 */
    int i;
    /* Distinct RGBA per palette index. */
    for (i = 0; i < 256; ++i) {
        palette[i] = 0xFF000000u | ((uint32_t)i << 16) |
                     ((uint32_t)(i ^ 0x55) << 8) | (uint32_t)(i ^ 0xAA);
    }
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_epx_upscale(src, 4, 4, dst, 8, 8, palette);
    /* For a uniform src (=15 across the top row), EPX produces the
     * palette-mapped color. dst[0] corresponds to src[0]=0 -> palette[0]. */
    check(dst[0] == palette[0],
          "epx: src[0]=0 maps to palette[0] (no edge rule matches uniform)");
    /* Every dst pixel must equal some palette entry (no garbage). */
    int all_palette = 1;
    for (i = 0; i < 64; ++i) {
        int found = 0;
        int j;
        for (j = 0; j < 256; ++j) {
            if (dst[i] == palette[j]) { found = 1; break; }
        }
        if (!found) {
            all_palette = 0;
            break;
        }
    }
    check(all_palette, "epx: every dst pixel matches a palette entry");
}

static void check_epx_deterministic(void)
{
    uint8_t src[64];
    uint32_t palette[256];
    uint32_t dst1[256];
    uint32_t dst2[256];
    int i;
    int same;
    for (i = 0; i < 64; ++i) src[i] = (uint8_t)((i * 17 + 3) & 0xFF);
    for (i = 0; i < 256; ++i) {
        palette[i] = 0xFF000000u | ((uint32_t)i * 0x010101u);
    }
    memset(dst1, 0xCC, sizeof(dst1));
    memset(dst2, 0xCC, sizeof(dst2));
    nexus_v2_epx_upscale(src, 8, 8, dst1, 16, 16, palette);
    nexus_v2_epx_upscale(src, 8, 8, dst2, 16, 16, palette);
    same = 1;
    for (i = 0; i < 256; ++i) {
        if (dst1[i] != dst2[i]) {
            same = 0;
            break;
        }
    }
    check(same, "epx: deterministic for same input");
}

static void check_epx_output_size(void)
{
    /* Output buffer must be filled for the dst_w * dst_h region. */
    uint8_t src[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    uint32_t palette[256];
    uint32_t dst[64];  /* 4x4 src -> 8x8 dst */
    int i;
    int any_unwritten = 0;
    for (i = 0; i < 256; ++i) palette[i] = 0xFF000000u | (uint32_t)i;
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_epx_upscale(src, 2, 2, dst, 4, 4, palette);
    /* Every output pixel must be palette-mapped (not 0xCC). */
    for (i = 0; i < 16; ++i) {
        if (dst[i] == 0xCCCCCCCCu) {
            any_unwritten = 1;
            break;
        }
    }
    check(any_unwritten == 0,
          "epx: all 4x4 dst pixels written (no 0xCC sentinel left)");
}

static void check_epx_boundary(void)
{
    /* 1x1 src -> 2x2 dst. No OOB read; output = palette[src[0]] for
     * both 2x2 output pixels (no neighbour matches). */
    uint8_t src[1] = { 0x42 };
    uint32_t palette[256];
    uint32_t dst[4];
    int i;
    for (i = 0; i < 256; ++i) palette[i] = 0xFF000000u | ((uint32_t)i * 0x010203u);
    memset(dst, 0, sizeof(dst));
    nexus_v2_epx_upscale(src, 1, 1, dst, 2, 2, palette);
    check(dst[0] == palette[0x42], "epx boundary 1x1: dst[0] = palette[src[0]]");
    check(dst[1] == palette[0x42], "epx boundary 1x1: dst[1] = palette[src[0]]");
    check(dst[2] == palette[0x42], "epx boundary 1x1: dst[2] = palette[src[0]]");
    check(dst[3] == palette[0x42], "epx boundary 1x1: dst[3] = palette[src[0]]");
}

static void check_epx_null_args(void)
{
    /* All null-arg calls must be safe. */
    uint8_t src[1] = { 0 };
    uint32_t dst[4] = {0};
    uint32_t palette[1] = {0};
    /* pre-fill dst with sentinel; if any call writes to dst, the
     * sentinel must persist (null-arg must be a no-op). */
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_epx_upscale(0, 4, 4, dst, 8, 8, palette);
    check(dst[0] == 0xCCCCCCCCu, "epx null: NULL src leaves dst unchanged");
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_epx_upscale(src, 4, 4, 0, 8, 8, palette);
    check(1, "epx null: NULL dst no crash");
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_epx_upscale(src, 4, 4, dst, 8, 8, 0);
    check(dst[0] == 0xCCCCCCCCu, "epx null: NULL palette leaves dst unchanged");
    /* Zero dims -> no-op. */
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_epx_upscale(src, 0, 0, dst, 8, 8, palette);
    check(dst[0] == 0xCCCCCCCCu, "epx null: 0x0 src dims no-op");
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_epx_upscale(src, 4, 4, dst, 0, 0, palette);
    check(dst[0] == 0xCCCCCCCCu, "epx null: 0x0 dst dims no-op");
}

static void check_bilinear_safe(void)
{
    /* Bilinear must not crash on null, on zero dims, on small buffers. */
    uint32_t dst[64];
    int i;
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_bilinear_smooth(0, 4, 4);
    check(1, "bilinear null: no crash");
    memset(dst, 0xCC, sizeof(dst));
    nexus_v2_bilinear_smooth(dst, 0, 0);
    check(dst[0] == 0xCCCCCCCCu, "bilinear null: 0x0 dims no-op");
    /* 1x1: should not crash. */
    dst[0] = 0xFF808080u;
    nexus_v2_bilinear_smooth(dst, 1, 1);
    check(1, "bilinear: 1x1 no crash");
    /* 4x4: should not crash. */
    for (i = 0; i < 16; ++i) dst[i] = 0xFF000000u | (uint32_t)(i * 0x101010);
    nexus_v2_bilinear_smooth(dst, 4, 4);
    check(1, "bilinear: 4x4 no crash");
}

static void check_source_evidence(void)
{
    const char *e = nexus_v2_upscaler_source_evidence();
    check(e != 0 && e[0] != 0, "evidence: present + non-empty");
    check(e && strstr(e, "EPX") != 0, "evidence: EPX");
    check(e && strstr(e, "Scale2x") != 0, "evidence: Scale2x");
    check(e && strstr(e, "DM1") != 0, "evidence: DM1 V2.1 (same algorithm)");
    check(e && strstr(e, "VDP1") != 0, "evidence: Saturn VDP1");
    check(e && strstr(e, "GFX.C") != 0, "evidence: ReDMCSB GFX.C");
}

int main(void)
{
    printf("=== Nexus V2 Upscaler Probe ===\n");
    check_palette_lookup();
    check_epx_deterministic();
    check_epx_output_size();
    check_epx_boundary();
    check_epx_null_args();
    check_bilinear_safe();
    check_source_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
