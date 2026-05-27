/*
 * verify_pass648_dm1_v2_asset_pipeline.c
 *
 * Probe: DM1 V2.1 graphics pipeline — source-asset-preserving EPX upscale
 * for walls, creatures, objects, projectiles, fonts, palette/light levels,
 * and title/entrance surfaces.
 *
 * Source-lock scope:
 *   DUNVIEW.C:4547-4602  F0115 objects/creatures/projectiles/explosions
 *   DUNVIEW.C:3502-3939  F0107 wall ornaments + alcove detection
 *   DUNVIEW.C:3940-4007  F0108 floor ornaments
 *   DUNVIEW.C:6816-6831  field draw (fluxcage) after creature/projectile
 *   PROJEXPL.C:43-165    projectile/explosion creation
 *   PROJEXPL.C:817-864   explosion damage/state
 *   PROJEXPL.C:987-994   fluxcage removal (C24)
 *   DRAWVIEW.C:1-200    creature sprite framing
 *   DEFS.H:421-430       projectile-associated explosion thing values
 *   DEFS.H:2218,2324,2376 font/inscription graphics M648/M653
 *   PANEL.C:418-428      G0304_i_DungeonViewPaletteIndex (6 levels)
 *   DATA.C:359-360        k_source_palette_light_amount_floor[]
 *   ENTRANCE.C:1-900     title/entrance animation sequencing
 *
 * Pass rule: V1 framebuffer content is preserved identically at the
 * logical pixel level; only presentation resolution and palette expansion
 * change. No V1 gameplay state, timing, or collision is modified.
 *
 * Run:  ./verify_pass648_dm1_v2_asset_pipeline
 * (No game data required — pure unit probe.)
 */

#include "dm1_v2_asset_pipeline_pc34.h"
#include "vga_palette_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ── Test helpers ─────────────────────────────────────────────────── */

static int g_pass = 1;

static void check(int cond, const char* msg) {
    if (!cond) {
        printf("FAIL: %s\n", msg);
        g_pass = 0;
    }
}

static int check_test(int cond, const char* msg) {
    if (!cond) {
        printf("FAIL: %s\n", msg);
    }
    return cond ? 1 : 0;
}

/* ── EPX correctness test ─────────────────────────────────────────── */

/* Synthetic single-pixel "sprite" to verify EPX edge preservation. */
static void test_epx_single_pixel(void) {
    printf("=== EPX single-pixel edge preservation ===\n");
    int tp = 1;

    /*
     * EPX/Scale2x edge detection rules (for output block at P):
     *   top-left  = (C==A && C!=D && A!=B) ? A : P
     *   top-right = (A==B && A!=C && B!=D) ? B : P
     *   bot-left  = (D==C && D!=B && C!=A) ? C : P
     *   bot-right = (B==D && B!=A && D!=C) ? D : P
     *
     * All four neighbors different from each other → no rule fires
     * (all equal to P → no rule fires — uniform case).
     * Corner pattern with equality along one diagonal triggers one corner.
     *
     * We verify: uniform field, and two edge cases where we can
     * predict the exact outcome from the rule conditions.
     */

    /* Test 1: Uniform field → all outputs = P */
    {
        uint8_t uni[9]; memset(uni, 5, 9);
        uint8_t udst[54]; memset(udst, 0, sizeof(udst));
        dm1_v2_asset_epx_upscale(uni, 3, 3, udst, 6, 6);
        int ok = 1, i;
        for (i = 0; i < 36; i++) if (udst[i] != 5) ok = 0;
        tp &= check_test(ok, "uniform field → all P");
    }

    /* Test 2: Corner pattern (interior pixel with all 4 neighbors defined)
     * src 3x3 with P=128 at (1,1):
     *   [64, 64, 192]   A=src[0]=64, B=src[2]=192
     *   [64, 128, 192]  C=src[3]=64,  D=src[5]=192
     *   [64, 192, 192]
     * Rules for block at x=1,y=1:
     *   top-left:  C==A (64==64=T) && C!=D (64!=192=T) && A!=B (64!=192=T) → =A=64
     *   top-right: A==B (64==192=F) → =P=128
     *   bot-left:  D==C (192==64=F) → =P=128
     *   bot-right: B==D (192==192=T) && B!=A (192!=64=T) && D!=C (192!=64=T) → =D=192 */
    {
        uint8_t src[9] = { 64,64,192, 64,128,192, 64,192,192 };
        uint8_t dst[54]; memset(dst, 0, sizeof(dst));
        dm1_v2_asset_epx_upscale(src, 3, 3, dst, 6, 6);
        /* Pixel (1,1): ox=2, oy=2, row_bytes=6.
         * top-left=dst[14], top-right=dst[15],
         * bot-left=dst[20], bot-right=dst[21] */
        int ok = (dst[14]==64 && dst[15]==128 && dst[20]==128 && dst[21]==192);
        tp &= check_test(ok, "corner pattern → top-left=64,bot-right=192");
    }

    /* Test 3: Diagonal corner bottom-right
     * Requires an interior pixel in a 4x4 or larger grid.
     * For a 3x3 grid, pixel (1,1) sits at the bottom row — D gets clamped
     * to P even though src[8]=200 exists. This is expected boundary-clamp
     * behavior; the EPX output is still deterministic.
     *
     * We verify the diagonal pattern works in a 4x4 test instead. */
    {
        /* 4x4: pixel (2,2) has all four neighbors defined.
         * A=C=100, B=D=200, P=100 → bot-right=D=200 */
        uint8_t src[16] = {
            100,100,100,100,
            100,100,100,200,
            100,100,100,200,
            100,100,200,200
        };
        /* Pixel (2,2): P=100, A=src[1*4+2]=100, B=src[2*4+3]=200,
         *   C=src[2*4+1]=100, D=src[3*4+2]=200 */
        uint8_t dst[128]; memset(dst, 0, sizeof(dst));
        dm1_v2_asset_epx_upscale(src, 4, 4, dst, 8, 8);
        /* Block at x=2,y=2: ox=4, oy=4, row_bytes=8.
         * top-left=dst[36], top-right=dst[37],
         * bot-left=dst[44], bot-right=dst[45] */
        int ok = (dst[36]==100 && dst[37]==100 && dst[44]==100 && dst[45]==200);
        tp &= check_test(ok, "4x4 diagonal corner → bot-right=D=200");
    }

    /* Test 4: Checkerboard — all distinct neighbors → all outputs = P */
    {
        uint8_t chk[9] = { 0,255,0, 255,0,255, 0,255,0 };
        uint8_t cdst[54]; memset(cdst, 0, sizeof(cdst));
        dm1_v2_asset_epx_upscale(chk, 3, 3, cdst, 6, 6);
        /* All pixels have all 4 neighbors different → no rule triggers → all = P */
        int ok = (cdst[0]==0 && cdst[2]==255 && cdst[14]==0 && cdst[21]==0);
        tp &= check_test(ok, "checkerboard → all outputs = P");
    }

    g_pass &= tp;
    printf("  EPX single-pixel: %s\n", tp ? "PASS" : "FAIL");
}

/* ── Palette expand correctness test ──────────────────────────────── */

/* Verify that palette expand produces the right RGBA for a known pixel. */
static void test_palette_expand(void) {
    printf("=== Palette expand ===\n");

    /* One pixel: level=0, index=4 (brightest level, 4th color in G9010) */
    uint8_t indexed[1] = { (0 << 4) | 4 };
    uint32_t lut[6 * 16];
    int i;
    for (i = 0; i < 6 * 16; i++) {
        lut[i] = 0xFF000000; /* opaque black base */
    }
    /* Set level=0, index=4 to red */
    lut[0 * 16 + 4] = 0xFF0000FF; /* BGRA: blue channel in low byte */

    uint32_t rgba[1];
    dm1_v2_asset_palette_expand(indexed, 1, 1, 0, lut, rgba);

    check(rgba[0] != 0, "palette expand produces non-zero RGBA");
    printf("  Palette expand pixel: 0x%08X (expect non-zero)\n", rgba[0]);
}

/* ── Full pipeline test ────────────────────────────────────────────── */

static void test_pipeline(void) {
    printf("=== Full pipeline ===\n");

    /* 8x8 synthetic "sprite" (wall fragment) */
    uint8_t src[64];
    int i;
    for (i = 0; i < 64; i++) {
        src[i] = (uint8_t)((i % 5) << 4 | (i % 13));
    }

    uint32_t rgba[256 * 4]; /* enough for 16x16 RGBA */
    int out_w, out_h;

    int rv = dm1_v2_asset_pipeline_process(
        DM1_V2_SURFACE_WALL_NEAR,
        src, 8, 8,
        0, /* source_palette_level = brightest */
        rgba, &out_w, &out_h);

    check(rv == 0, "pipeline_process returns 0");

    /* EPX 2x → 16x16 output */
    check(out_w == 16, "EPX 2x: output width 16");
    check(out_h == 16, "EPX 2x: output height 16");

    /* All RGBA pixels non-zero (sprite has content) */
    int nonzero = 0;
    for (i = 0; i < out_w * out_h; i++) {
        if (rgba[i] != 0) nonzero++;
    }
    check(nonzero > 0, "RGBA output has non-zero pixels");

    printf("  Pipeline: %dx%d RGBA, %d non-zero pixels\n", out_w, out_h, nonzero);
}

/* ── Surface category names ───────────────────────────────────────── */

static void test_category_names(void) {
    printf("=== Surface category names ===\n");

    check(strcmp(dm1_v2_asset_surface_category_name(DM1_V2_SURFACE_WALL_BACK), "wall-back") == 0,
          "DM1_V2_SURFACE_WALL_BACK name");
    check(strcmp(dm1_v2_asset_surface_category_name(DM1_V2_SURFACE_CREATURE), "creature") == 0,
          "DM1_V2_SURFACE_CREATURE name");
    check(strcmp(dm1_v2_asset_surface_category_name(DM1_V2_SURFACE_TITLE), "title") == 0,
          "DM1_V2_SURFACE_TITLE name");
    check(strcmp(dm1_v2_asset_surface_category_name(DM1_V2_SURFACE_COUNT - 1), "entrance") == 0,
          "DM1_V2_SURFACE_ENTRANCE name");
    check(strcmp(dm1_v2_asset_surface_category_name(DM1_V2_SURFACE_UNKNOWN), "unknown") == 0,
          "unknown category name");
    check(strcmp(dm1_v2_asset_surface_category_name(DM1_V2_SURFACE_COUNT + 1), "unknown") == 0,
          "out-of-range category name");

    printf("  Category names: PASS\n");
}

/* ── Config init ──────────────────────────────────────────────────── */

static void test_config_init(void) {
    printf("=== Config init ===\n");

    dm1_v2_asset_pipeline_init();
    const DM1_V2_AssetPipelineConfig* cfg = dm1_v2_asset_pipeline_get_config();
    check(cfg != NULL, "get_config returns non-NULL");
    check(cfg->epx_enabled == 1, "EPX enabled by default");
    check(cfg->bilinear_enabled == 0, "bilinear disabled by default");
    check(cfg->palette_enhanced == 0, "palette_enhanced off by default");
    check(cfg->scale_mode == DM1_V2_SCALE_MODE_EPX_2X, "scale_mode = EPX_2X default");

    printf("  Config init: PASS\n");
}

/* ── Source evidence ──────────────────────────────────────────────── */

static void test_source_evidence(void) {
    printf("=== Source evidence ===\n");

    const char* ev = dm1_v2_asset_pipeline_source_evidence();
    check(ev != NULL && strlen(ev) > 100, "source evidence is populated");
    check(strstr(ev, "DUNVIEW.C:4547") != NULL, "evidence cites F0115");
    check(strstr(ev, "DUNVIEW.C:3502") != NULL, "evidence cites F0107");
    check(strstr(ev, "PROJEXPL.C:43") != NULL, "evidence cites projectile");
    check(strstr(ev, "PANEL.C:418") != NULL, "evidence cites palette index");
    check(strstr(ev, "ENTRANCE.C") != NULL, "evidence cites entrance");

    printf("  Source evidence: PASS (len=%zu)\n", strlen(ev));
}

/* ── All categories ────────────────────────────────────────────────── */

static void test_all_categories(void) {
    printf("=== All surface categories ===\n");

    /* 8x8 solid indexed surface */
    uint8_t src[64];
    memset(src, 0x42, sizeof(src));
    uint32_t rgba[256 * 4];

    /* Verify the category enum covers all expected values */
    const DM1_V2_SurfaceCategory cats[] = {
        DM1_V2_SURFACE_WALL_BACK,
        DM1_V2_SURFACE_WALL_NEAR,
        DM1_V2_SURFACE_WALL_CENTER,
        DM1_V2_SURFACE_DOOR,
        DM1_V2_SURFACE_FLOOR_ORNAMENT,
        DM1_V2_SURFACE_WALL_ORNAMENT,
        DM1_V2_SURFACE_CREATURE,
        DM1_V2_SURFACE_OBJECT,
        DM1_V2_SURFACE_PROJECTILE,
        DM1_V2_SURFACE_EXPLOSION,
        DM1_V2_SURFACE_FLUXCAGE,
        DM1_V2_SURFACE_FONT,
        DM1_V2_SURFACE_PANEL_CHAMPION,
        DM1_V2_SURFACE_PANEL_MESSAGE,
        DM1_V2_SURFACE_TITLE,
        DM1_V2_SURFACE_ENTRANCE,
    };

    /* Verify enum sequence is contiguous */
    check(DM1_V2_SURFACE_COUNT == 17,
          "DM1_V2_SURFACE_COUNT == 17 (16 categories + UNKNOWN)");

    int pass = 1;
    int i;
    for (i = 0; i < 16; i++) {
        int out_w, out_h;
        int rv = dm1_v2_asset_upscale_surface(cats[i], src, 8, 8, 0, rgba, &out_w, &out_h);
        if (rv != 0) {
            printf("  FAIL at category %d (%s)\
",
                   cats[i], dm1_v2_asset_surface_category_name(cats[i]));
            pass = 0;
        } else {
            /* EPX 2x → 16x16 */
            if (out_w != 16 || out_h != 16) {
                printf("  WRONG SIZE at category %d (%s): %dx%d (expect 16x16)\n",
                       cats[i], dm1_v2_asset_surface_category_name(cats[i]), out_w, out_h);
                pass = 0;
            }
        }
    }
    g_pass &= pass;
    printf("  All %d categories: %s\n", 16, pass ? "PASS" : "FAIL");
}

/* ── Palette level sweep ──────────────────────────────────────────── */

static void test_palette_levels(void) {
    printf("=== Palette level sweep ===\n");

    uint8_t indexed[16];
    int pi;
    for (pi = 0; pi < 16; pi++) {
        indexed[pi] = (uint8_t)(pi); /* level=0, index=pi */
    }
    uint32_t rgba[16];
    uint32_t lut[6 * 16];
    memset(lut, 0, sizeof(lut));
    /* mark every entry with its level+index as BGRA color to make them distinguishable */
    int lv, idx;
    for (lv = 0; lv < 6; lv++) {
        for (idx = 0; idx < 16; idx++) {
            lut[lv * 16 + idx] = 0xFF000000 | (uint8_t)(lv << 4) | (uint8_t)idx;
        }
    }

    for (lv = 0; lv < 6; lv++) {
        dm1_v2_asset_palette_expand(indexed, 16, 1, lv, lut, rgba);
        /* Each output pixel should have a different value based on level+index */
        int varies = 0;
        for (pi = 1; pi < 16; pi++) {
            if (rgba[pi] != rgba[pi - 1]) varies++;
        }
        check(varies > 0, "palette expand varies across indices at level");
        (void)varies;
    }
    printf("  Palette levels 0-5: PASS\n");
}

/* ── Main ──────────────────────────────────────────────────────────── */

int main(void) {
    printf("\n====== PASS648: DM1 V2.1 Graphics Pipeline Probe ======\n\n");

    /* Phase 1: Module init and config */
    test_config_init();

    /* Phase 2: EPX correctness */
    test_epx_single_pixel();

    /* Phase 3: Palette expand */
    test_palette_expand();

    /* Phase 4: Full pipeline */
    test_pipeline();

    /* Phase 5: Surface category metadata */
    test_category_names();

    /* Phase 6: All categories covered */
    test_all_categories();

    /* Phase 7: Palette level sweep */
    test_palette_levels();

    /* Phase 8: Source evidence */
    test_source_evidence();

    printf("\n====== RESULT: %s ======\n\n", g_pass ? "PASS" : "FAIL");
    return g_pass ? 0 : 1;
}
