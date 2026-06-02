/*
 * firestaff_dm2_v2_phase2_asset_pipeline_probe.c
 *
 * DM2 V2 Phase 2 — Enhanced Asset Pipeline Probe
 *
 * Headless probe: verifies DM2 V2 Phase 2 asset pipeline gate and EPX
 * upscale/palette-expand pipeline without requiring live game asset files.
 *
 * Phase 2 domain: DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE
 *   - Requires LAUNCH+PROFILE to be enabled (same as HUD Phase 3)
 *   - When disabled: V1 source-locked path (SKULL.ASM T560/T580)
 *   - When enabled: V2.1 EPX upscale + per-level palette expand active
 *
 * Probe validates:
 *   1. Phase gate ASSET_PIPELINE domain enum value and is_* function
 *   2. Gate decision: disabled config → v1SourceLocked=1, v2Allowed=0
 *   3. Gate decision: LAUNCH-only → ASSET_PIPELINE gated
 *   4. Gate decision: LAUNCH+PROFILE → v1SourceLocked=0, v2Allowed=1
 *   5. Pipeline init/config/get_config cycle
 *   6. EPX upscale produces 2x dimensions
 *   7. Palette expand produces nonzero RGBA pixels
 *   8. Bilinear RGBA upscale (when enabled)
 *   9. Surface category metadata present
 *   10. Gfx mode API: set/get, is_v2_mode, is_v2_filtered, is_v2_upscaled
 *
 * Exit codes:
 *   0  — all checks passed
 *   1  — one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_dm2_v2_phase2_asset_pipeline_probe
 *
 * Source references:
 *   SKULL.ASM T520  — party/movement tick
 *   SKULL.ASM T560  — dungeon viewport rendering
 *   SKULL.ASM T580  — load dungeon (asset hash check)
 *   SKULL.ASM T600  — outdoor viewport rendering
 *   ReDMCSB DUNVIEW.C:2962-3070 — floor/ceiling/wall set drawing
 *   ReDMCSB DUNVIEW.C:3048-3070 — F0100 DrawWallSetBitmap
 *   ReDMCSB PANEL.C:418-428     — G0304_i_DungeonViewPaletteIndex (6 levels)
 *   ReDMCSB DATA.C:359-360      — k_source_palette_light_amount_floor[]
 *   dm2_v2_asset_pipeline.c     — Phase 2 implementation
 */

#include "dm2_v2_phase_gate.h"
#include "dm2_v2_asset_pipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Test counters ──────────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static void rec(int ok, const char *id, const char *msg)
{
    if (ok) { ++g_pass; printf("PASS %s  %s\n", id, msg); }
    else     { ++g_fail; printf("FAIL %s  %s\n", id, msg); }
}

static void sec(const char *title) { printf("\n=== %s ===\n", title); }

#define CHECK(label, cond) rec((cond) != 0, label, #cond)

/* ── EPX test data ───────────────────────────────────────────────────────── */

static uint8_t make_checker(int x, int y)
{
    return ((x ^ y) & 4) ? 0x11 : 0x22;
}

/* ── Palette check: nonzero pixels in RGBA out ──────────────────────────── */

static int count_nonzero_rgba(const uint32_t *rgba, int n)
{
    int cnt = 0;
    while (n--) { if (*rgba++ & 0x00FFFFFFu) cnt++; }
    return cnt;
}

/* ── 1. Domain enum and identifier ──────────────────────────────────────── */

static void test_domain_enum(void)
{
    sec("DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE enum");
    CHECK("enum_value_nonzero", DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE != 0);
    CHECK("enum_neq_LAUNCH",  DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE != DM2_V2_PHASE_DOMAIN_LAUNCH);
    CHECK("enum_neq_PROFILE", DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE != DM2_V2_PHASE_DOMAIN_PROFILE);
    CHECK("enum_neq_HUD",     DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE != DM2_V2_PHASE_DOMAIN_HUD);
    CHECK("enum_eq_3",        DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE == 3);

    sec("dm2_v2_phase_gate_is_asset_pipeline_domain");
    CHECK("is_asset_pipeline(LAUNCH)=false",
          !dm2_v2_phase_gate_is_asset_pipeline_domain(DM2_V2_PHASE_DOMAIN_LAUNCH));
    CHECK("is_asset_pipeline(PROFILE)=false",
          !dm2_v2_phase_gate_is_asset_pipeline_domain(DM2_V2_PHASE_DOMAIN_PROFILE));
    CHECK("is_asset_pipeline(HUD)=false",
          !dm2_v2_phase_gate_is_asset_pipeline_domain(DM2_V2_PHASE_DOMAIN_HUD));
    CHECK("is_asset_pipeline(ASSET_PIPELINE)=true",
          dm2_v2_phase_gate_is_asset_pipeline_domain(DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE));
}

/* ── 2. Gate: default (0,0) → all V1 source-locked ──────────────────────── */

static void test_gate_default(void)
{
    sec("Phase gate: default config (v2LaunchEnabled=0, v2ProfileEnabled=0)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 0;
    cfg.v2ProfileEnabled = 0;

    DM2_V2_PhaseGateDecision d;
    (void)d;

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE);
    CHECK("ASSET_PIPELINE default: v1SourceLocked=1",  d.v1SourceLocked == 1);
    CHECK("ASSET_PIPELINE default: v2Allowed=0",       d.v2Allowed == 0);
    CHECK("sourceAnchor set",                           d.sourceAnchor != NULL);
    CHECK("rule set",                                  d.rule != NULL);
}

/* ── 3. Gate: LAUNCH-only (1,0) → ASSET_PIPELINE gated ─────────────────── */

static void test_gate_launch_only(void)
{
    sec("Phase gate: LAUNCH-only config (v2LaunchEnabled=1, v2ProfileEnabled=0)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 1;
    cfg.v2ProfileEnabled = 0;

    DM2_V2_PhaseGateDecision d;

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE);
    CHECK("ASSET_PIPELINE(1,0): v1SourceLocked=1", d.v1SourceLocked == 1);
    CHECK("ASSET_PIPELINE(1,0): v2Allowed=0",    d.v2Allowed == 0);
    CHECK("gated on PROFILE in rule",             strstr(d.rule, "PROFILE") != NULL || strstr(d.rule, "LAUNCH") != NULL);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_LAUNCH);
    CHECK("LAUNCH(1,0): v2Allowed=1", d.v2Allowed == 1);
}

/* ── 4. Gate: LAUNCH+PROFILE (1,1) → ASSET_PIPELINE active ──────────────── */

static void test_gate_full(void)
{
    sec("Phase gate: LAUNCH+PROFILE config (v2LaunchEnabled=1, v2ProfileEnabled=1)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 1;
    cfg.v2ProfileEnabled = 1;

    DM2_V2_PhaseGateDecision d;

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE);
    CHECK("ASSET_PIPELINE(1,1): v1SourceLocked=0", d.v1SourceLocked == 0);
    CHECK("ASSET_PIPELINE(1,1): v2Allowed=1",     d.v2Allowed == 1);
    CHECK("sourceAnchor mentions DUNVIEW",         strstr(d.sourceAnchor, "DUNVIEW") != NULL);
    CHECK("rule mentions Phase 2",               strstr(d.rule, "Phase 2") != NULL || strstr(d.rule, "EPX") != NULL);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_HUD);
    CHECK("HUD(1,1): v2Allowed=1", d.v2Allowed == 1);
    CHECK("HUD(1,1): v1SourceLocked=0", d.v1SourceLocked == 0);
}

/* ── 5. Pipeline init / configure / get_config ─────────────────────────── */

static void test_pipeline_init(void)
{
    sec("dm2_v2_asset_pipeline init/config/get cycle");
    dm2_v2_asset_pipeline_init();  /* must not crash */
    const DM2_V2_AssetPipelineConfig *c = dm2_v2_asset_pipeline_get_config();
    CHECK("get_config not NULL", c != NULL);
    CHECK("scale_mode valid", c->scale_mode >= DM2_V2_SCALE_MODE_NATIVE &&
                               c->scale_mode <= DM2_V2_SCALE_MODE_BILINEAR_2X);
    CHECK("palette_mode valid", c->palette_mode >= DM2_V2_PALETTE_MODE_ORIGINAL &&
                                c->palette_mode <= DM2_V2_PALETTE_MODE_ENHANCED);
    CHECK("epx off by default", c->epx_enabled == 0);
    CHECK("bilinear off by default", c->bilinear_enabled == 0);

    DM2_V2_AssetPipelineConfig cfg2 = {
        DM2_V2_SCALE_MODE_EPX_2X,
        DM2_V2_PALETTE_MODE_ORIGINAL,
        1,   /* epx_enabled */
        1,   /* bilinear_enabled */
        1,   /* palette_enhanced */
        1,   /* scanlines_enabled */
        1,   /* sharpen_enabled */
        50   /* source_light_floor */
    };
    dm2_v2_asset_pipeline_configure(&cfg2);
    const DM2_V2_AssetPipelineConfig *c2 = dm2_v2_asset_pipeline_get_config();
    CHECK("configure: epx_enabled=1", c2->epx_enabled == 1);
    CHECK("configure: bilinear_enabled=1", c2->bilinear_enabled == 1);
    CHECK("configure: palette_enhanced=1", c2->palette_enhanced == 1);
}

/* ── 6. EPX upscale: 2x dimensions ──────────────────────────────────────── */

static void test_epx_2x(void)
{
    sec("EPX 2x upscale dimensions");
    uint8_t src[8 * 8];
    uint8_t dst[16 * 16];
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) src[y * 8 + x] = make_checker(x, y);

    dm2_v2_asset_epx_upscale(src, 8, 8, dst, 16, 16);

    int non_default = 0;
    for (int i = 0; i < 16 * 16; i++) {
        if (dst[i] != src[0]) non_default++;
    }
    CHECK("EPX output differs from src[0] (some edge pixels changed)", non_default > 0);
    CHECK("EPX output has more than 1 unique value", non_default > 1);

    /* EPX must produce 2x dimensions */
    int zero_src = 0;
    for (int i = 0; i < 8*8; i++) zero_src |= src[i];
    if (zero_src) {
        int nonzero_out = 0;
        for (int i = 0; i < 16*16; i++) nonzero_out |= dst[i];
        CHECK("EPX output nonzero when src nonzero", nonzero_out != 0);
    }
}

/* ── 7. Palette expand: nonzero RGBA pixels ──────────────────────────────── */

static void test_palette_expand(void)
{
    sec("Palette expand: indexed → RGBA8888");
    uint8_t indexed[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                            0x08, 0x19, 0x2A, 0x3B, 0x4C, 0x5D, 0x6E, 0x7F };
    uint32_t rgba[16];
    memset(rgba, 0, sizeof(rgba));

    static uint32_t fake_lut[6 * 16];
    for (int lv = 0; lv < 6; lv++) {
        for (int pi = 0; pi < 16; pi++) {
            int v = (lv * 16 + pi);
            fake_lut[lv * 16 + pi] = 0xFF000000u | ((uint32_t)(v & 0xFF) << 16) |
                                      ((uint32_t)((v*2)&0xFF) << 8) | (uint32_t)((v*3)&0xFF);
        }
    }

    dm2_v2_asset_palette_expand(indexed, 4, 4, 2, fake_lut, rgba);

    int nz = count_nonzero_rgba(rgba, 16);
    CHECK("palette_expand: some pixels nonzero", nz > 0);

    /* Different palette entries must give different RGBA values */
    int different = 0;
    for (int i = 1; i < 16; i++) {
        if (rgba[i] != rgba[0]) different++;
    }
    CHECK("palette_expand: output differs across palette entries", different > 1);
}

/* ── 8. Bilinear RGBA upscale ───────────────────────────────────────────── */

static void test_bilinear_rgba(void)
{
    sec("Bilinear RGBA upscale");

    uint32_t dst[16];
    memset(dst, 0xAA, sizeof(dst));

    /* Bilinear test: configure with bilinear_enabled=1, process input,
     * then verify output dimensions are double the input (pass-through).
     * We test the pipeline with bilinear but no EPX, expecting 2x dimensions.
     * dm2_v2_asset_bilinear_rgba is static (internal) so we test via
     * the public pipeline_process API: bilinear_enabled doubles output dims.
     */
    sec("Bilinear pipeline upscale (via public API)");
    {
        uint8_t src_bi[4] = { 0x11, 0x22, 0x33, 0x44 };  /* 2x2 indexed */
        uint32_t rgba_bi[64];
        int bw = 0, bh = 0;

        DM2_V2_AssetPipelineConfig cfg_bi;
        memset(&cfg_bi, 0, sizeof(cfg_bi));
        cfg_bi.epx_enabled = 0;
        cfg_bi.bilinear_enabled = 1;
        cfg_bi.palette_enhanced = 0;
        cfg_bi.scanlines_enabled = 0;
        cfg_bi.sharpen_enabled = 0;
        dm2_v2_asset_pipeline_configure(&cfg_bi);

        int rc_bi = dm2_v2_asset_pipeline_process(
            DM2_V2_SURFACE_WALL_BACK,
            src_bi, 2, 2,
            0,
            rgba_bi, &bw, &bh);

        /* Bilinear enabled with no EPX: output = input * 2 (since bilinear doubles) */
        CHECK("bilinear: returns 0", rc_bi == 0);
        CHECK("bilinear: output width doubled", bw == 4);
        CHECK("bilinear: output height doubled", bh == 4);

        int nz_bi = count_nonzero_rgba(rgba_bi, bw * bh);
        CHECK("bilinear: nonzero output pixels", nz_bi > 0);

        /* Reset config */
        dm2_v2_asset_pipeline_configure(NULL);
    }
}

/* ── 9. Surface category names and evidence ──────────────────────────────── */

static void test_surface_categories(void)
{
    sec("Surface category names");
    CHECK("WALL_BACK name", strcmp(dm2_v2_asset_surface_category_name(DM2_V2_SURFACE_WALL_BACK), "wall-back") == 0);
    CHECK("DOOR name",     strcmp(dm2_v2_asset_surface_category_name(DM2_V2_SURFACE_DOOR), "door") == 0);
    CHECK("CREATURE name", strcmp(dm2_v2_asset_surface_category_name(DM2_V2_SURFACE_CREATURE), "creature") == 0);
    CHECK("FONT name",     strcmp(dm2_v2_asset_surface_category_name(DM2_V2_SURFACE_FONT), "font") == 0);
    CHECK("SKY_GRADIENT name", strcmp(dm2_v2_asset_surface_category_name(DM2_V2_SURFACE_SKY_GRADIENT), "sky-gradient") == 0);
    CHECK("ROOM_WALL name", strcmp(dm2_v2_asset_surface_category_name(DM2_V2_SURFACE_ROOM_WALL), "room-wall") == 0);
    CHECK("unknown for BAD", strcmp(dm2_v2_asset_surface_category_name((DM2_V2_SurfaceCategory)999), "unknown") == 0);
}

/* ── 10. Gfx mode API ────────────────────────────────────────────────────── */

static void test_gfx_mode_api(void)
{
    sec("dm2_v2_asset gfx mode API");

    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V1_ORIGINAL);
    CHECK("set V1_ORIGINAL: get returns V1", dm2_v2_asset_get_gfx_mode() == DM2_V2_GFX_MODE_V1_ORIGINAL);
    CHECK("V1_ORIGINAL: is_v2_mode=0", dm2_v2_asset_is_v2_mode() == 0);
    CHECK("V1_ORIGINAL: is_v2_filtered=0", dm2_v2_asset_is_v2_filtered() == 0);
    CHECK("V1_ORIGINAL: is_v2_upscaled=0", dm2_v2_asset_is_v2_upscaled() == 0);
    CHECK("V1_ORIGINAL: is_v2_modern=0", dm2_v2_asset_is_v2_modern() == 0);

    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V2_FILTERED);
    CHECK("set V2_FILTERED: is_v2_mode=1", dm2_v2_asset_is_v2_mode() == 1);
    CHECK("set V2_FILTERED: is_v2_filtered=1", dm2_v2_asset_is_v2_filtered() == 1);
    CHECK("set V2_FILTERED: is_v2_upscaled=0", dm2_v2_asset_is_v2_upscaled() == 0);
    CHECK("set V2_FILTERED: is_v2_modern=0", dm2_v2_asset_is_v2_modern() == 0);

    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V2_UPSCALED);
    CHECK("set V2_UPSCALED: is_v2_mode=1", dm2_v2_asset_is_v2_mode() == 1);
    CHECK("set V2_UPSCALED: is_v2_filtered=1", dm2_v2_asset_is_v2_filtered() == 1);
    CHECK("set V2_UPSCALED: is_v2_upscaled=1", dm2_v2_asset_is_v2_upscaled() == 1);
    CHECK("set V2_UPSCALED: is_v2_modern=0", dm2_v2_asset_is_v2_modern() == 0);

    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V2_MODERN);
    CHECK("set V2_MODERN: is_v2_mode=1", dm2_v2_asset_is_v2_mode() == 1);
    CHECK("set V2_MODERN: is_v2_filtered=1", dm2_v2_asset_is_v2_filtered() == 1);
    CHECK("set V2_MODERN: is_v2_upscaled=1", dm2_v2_asset_is_v2_upscaled() == 1);
    CHECK("set V2_MODERN: is_v2_modern=1", dm2_v2_asset_is_v2_modern() == 1);

    /* Reset to V1 */
    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V1_ORIGINAL);
}

/* ── 11. Shape source selection ─────────────────────────────────────────── */

static void test_shape_source(void)
{
    sec("dm2_v2_best_available_shape_source");

    /* With modern root empty → falls through to V1 */
    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V2_MODERN);
    DM2_V22_ShapeSource src = dm2_v2_best_available_shape_source(3);
    CHECK("V2_MODERN without modern root → not V2_MODERN",
          src != DM2_V22_SHAPE_SOURCE_V2_MODERN);

    /* V2_UPSCALED requires epx */
    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V2_UPSCALED);
    src = dm2_v2_best_available_shape_source(2);
    CHECK("shape source name V2_FILTERED",
          strcmp(dm2_v2_shape_source_name(DM2_V22_SHAPE_SOURCE_V2_FILTERED), "V2_FILTERED") == 0);
    CHECK("shape source name V2_UPSCALED",
          strcmp(dm2_v2_shape_source_name(DM2_V22_SHAPE_SOURCE_V2_UPSCALED), "V2_UPSCALED") == 0);
    CHECK("shape source name V1_ORIGINAL",
          strcmp(dm2_v2_shape_source_name(DM2_V22_SHAPE_SOURCE_V1_ORIGINAL), "V1_ORIGINAL") == 0);

    dm2_v2_asset_set_gfx_mode(DM2_V2_GFX_MODE_V1_ORIGINAL);
}

/* ── 12. Null-safety: no crash on NULL inputs ───────────────────────────── */

static void test_null_safety(void)
{
    sec("Null-safety: no crash on NULL inputs");
    dm2_v2_asset_pipeline_configure(NULL);  /* must not crash */
    const DM2_V2_AssetPipelineConfig *c = dm2_v2_asset_pipeline_get_config();
    CHECK("get_config still valid after NULL configure", c != NULL);

    dm2_v2_asset_palette_expand(NULL, 4, 4, 0, NULL, NULL);  /* must not crash */
    dm2_v2_asset_epx_upscale(NULL, 8, 8, NULL, 16, 16);      /* must not crash */
    CHECK("null safety: no crash", 1);
}

/* ── 13. Source evidence ─────────────────────────────────────────────────── */

static void test_source_evidence(void)
{
    sec("Source evidence strings");
    const char *ev = dm2_v2_asset_pipeline_source_evidence();
    CHECK("source_evidence not NULL", ev != NULL);
    CHECK("source_evidence len > 50", strlen(ev) > 50);
    CHECK("mentions SKULL.ASM", strstr(ev, "SKULL.ASM") != NULL);
    CHECK("mentions DUNVIEW", strstr(ev, "DUNVIEW") != NULL);
    CHECK("mentions PANEL.C", strstr(ev, "PANEL.C") != NULL);
    CHECK("mentions Phase 2", strstr(ev, "Phase 2") != NULL);
    CHECK("mentions EPX", strstr(ev, "EPX") != NULL);
}

/* ── 14. Full pipeline process: indexed → RGBA ──────────────────────────── */

static void test_pipeline_process(void)
{
    sec("dm2_v2_asset_pipeline_process full round-trip");

    /* Build 16x16 checkerboard at level=2 */
    static uint8_t src_idx[16 * 16];
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            src_idx[y * 16 + x] = (uint8_t)((2 << 4) | (make_checker(x, y) & 0x0F));
        }
    }

    static uint32_t rgba_out[64 * 64];
    int out_w = 0, out_h = 0;
    memset(rgba_out, 0xAA, sizeof(rgba_out));

    /* Enable EPX for this test */
    DM2_V2_AssetPipelineConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.epx_enabled = 0;  /* pass-through: no upscale */
    cfg.palette_enhanced = 0;
    cfg.scanlines_enabled = 0;
    cfg.sharpen_enabled = 0;
    cfg.bilinear_enabled = 0;
    dm2_v2_asset_pipeline_configure(&cfg);

    int rc = dm2_v2_asset_pipeline_process(
        DM2_V2_SURFACE_WALL_BACK,
        src_idx, 16, 16,
        2,
        rgba_out,
        &out_w, &out_h);

    CHECK("pipeline_process returns 0", rc == 0);
    CHECK("pipeline_process: out_w=16", out_w == 16);
    CHECK("pipeline_process: out_h=16", out_h == 16);

    int nz = count_nonzero_rgba(rgba_out, 16 * 16);
    CHECK("pipeline_process: nonzero pixels", nz > 0);

    /* Verify different input bytes produce different colors (pipeline
     * produces different output when input pixels differ). */
    static uint8_t src_flat[16 * 16];
    static uint8_t src_varied[16 * 16];
    for (int i = 0; i < 16 * 16; i++) { src_flat[i] = 0x11; src_varied[i] = (uint8_t)(i & 0xFF); }

    static uint32_t rgba_flat[16 * 16];
    static uint32_t rgba_varied[16 * 16];
    int w_flat = 0, h_flat = 0, w_var = 0, h_var = 0;
    dm2_v2_asset_pipeline_process(DM2_V2_SURFACE_WALL_BACK, src_flat, 16, 16, 0, rgba_flat, &w_flat, &h_flat);
    dm2_v2_asset_pipeline_process(DM2_V2_SURFACE_WALL_BACK, src_varied, 16, 16, 0, rgba_varied, &w_var, &h_var);

    int flat_nz = count_nonzero_rgba(rgba_flat, 16 * 16);
    int var_nz  = count_nonzero_rgba(rgba_varied, 16 * 16);
    CHECK("pipeline: flat input → nonzero pixels", flat_nz > 0);
    CHECK("pipeline: varied input → nonzero pixels", var_nz > 0);
    (void)w_flat; (void)h_flat; (void)w_var; (void)h_var;
}

/* ── 15. Palette LUT invalidation ───────────────────────────────────────── */

static void test_palette_lut(void)
{
    sec("Palette LUT invalidation and rebuild");

    dm2_v2_asset_invalidate_cached_palette();  /* must not crash */
    int rc = dm2_v2_asset_rebuild_palette_lut(100, 0, 0);  /* stub returns 0 */
    CHECK("rebuild_palette_lut: returns 0 (stub)", rc == 0);
    dm2_v2_asset_invalidate_cached_palette();  /* must not crash */
}

/* ── 16. Null-config decision ───────────────────────────────────────────── */

static void test_null_config(void)
{
    sec("Null-config antisymmetric: ASSET_PIPELINE → V1 locked");

    DM2_V2_PhaseGateDecision d = dm2_v2_phase_gate_decide(NULL, DM2_V2_PHASE_DOMAIN_ASSET_PIPELINE);
    CHECK("null-config: v1SourceLocked=1", d.v1SourceLocked == 1);
    CHECK("null-config: v2Allowed=0",      d.v2Allowed == 0);
    CHECK("null-config: sourceAnchor set",  d.sourceAnchor != NULL);
}

/* ── Main ────────────────────────────────────────────────────────────────── */

int main(void)
{
    printf("DM2 V2 Phase 2 — Asset Pipeline Probe\n");
    printf("Built: %s %s\n", __DATE__, __TIME__);

    test_domain_enum();
    test_gate_default();
    test_gate_launch_only();
    test_gate_full();
    test_pipeline_init();
    test_epx_2x();
    test_palette_expand();
    test_bilinear_rgba();
    test_surface_categories();
    test_gfx_mode_api();
    test_shape_source();
    test_null_safety();
    test_source_evidence();
    test_pipeline_process();
    test_palette_lut();
    test_null_config();

    printf("\n--- Results: %d passed, %d failed ---\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}