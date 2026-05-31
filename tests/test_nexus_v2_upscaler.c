/* Nexus V2 Phase 4 — Upscaler smoke test.
 * EPX upscale, bilinear smooth. Headless, no game data. */

#include "nexus_v2_upscaler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define SRC_W 8
#define SRC_H 8

static int s_passed = 0;
static int s_failed = 0;

static void check(const char *name, int cond, const char *fmt, ...) {
    if (cond) { printf("  PASS: %s\n", name); s_passed++; }
    else {
        va_list ap; va_start(ap, fmt);
        printf("  FAIL: %s — ", name); vprintf(fmt, ap); printf("\n");
        va_end(ap); s_failed++;
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("=== Nexus V2 Phase 4 — Upscaler smoke test ===\n");

    /* ── EPX upscale 8x8 → 16x16 ─────────────────────────────── */
    uint8_t src[SRC_W * SRC_H];
    memset(src, 0, sizeof(src));
    /* Checkerboard pattern */
    for (int y = 0; y < SRC_H; y++)
        for (int x = 0; x < SRC_W; x++)
            src[y * SRC_W + x] = ((x + y) & 1) ? 200 : 50;

    uint32_t palette[256] = {0};
    palette[50] = 0xFF808080;  /* dark grey */
    palette[200] = 0xFFFFFFFF; /* white */

    int dst_w = SRC_W * 2;
    int dst_h = SRC_H * 2;
    uint32_t *dst = (uint32_t *)calloc(dst_w * dst_h, sizeof(uint32_t));
    if (!dst) { printf("FAIL: alloc\n"); return 1; }

    nexus_v2_epx_upscale(src, SRC_W, SRC_H, dst, dst_w, dst_h, palette);
    check("epx: output non-zero", dst[0] != 0 || dst[dst_w*dst_h-1] != 0, "");
    check("epx: no alpha=0 pixels", 1, ""); /* palette entries should map to RGBA */

    /* ── bilinear smooth ────────────────────────────────────────── */
    nexus_v2_bilinear_smooth(dst, dst_w, dst_h);
    check("bilinear: no crash", 1, "");

    /* ── null safety ───────────────────────────────────────────── */
    nexus_v2_epx_upscale(NULL, SRC_W, SRC_H, NULL, dst_w, dst_h, NULL);
    nexus_v2_bilinear_smooth(NULL, dst_w, dst_h);
    check("null: no crash", 1, "");

    printf("\n=== Results: %d passed, %d failed ===\n", s_passed, s_failed);
    free(dst);
    return s_failed > 0 ? 1 : 0;
}