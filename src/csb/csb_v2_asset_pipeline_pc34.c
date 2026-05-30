/*
 * csb_v2_asset_pipeline_pc34.c — CSB V2.1 Graphics Pipeline
 *
 * Enhanced asset pipeline: loads enhanced graphics (V2.0/V2.1 mode assets)
 * rather than original V1 assets for Chaos Strikes Back.
 *
 * Pipeline architecture:
 *   V1 indexed (level << 4 | palette_index)
 *     -> EPX 2x (indexed, edge-preserving for pixel art)
 *     -> palette expand to RGBA (per-level G9010_auc_VgaPaletteAll_Compat)
 *     -> optional post: scanlines + palette correction + sharpening
 *
 * Fallback chain:
 *   MODERN (V2.2) -> UPSCALED (V2.1) -> FILTERED (V2.0) -> ORIGINAL (V1)
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   DUNVIEW.C:4547-4602  F0115 draw objects/creatures/projectiles
 *   DUNVIEW.C:3502-3939  F0107 wall ornaments / alcove detection
 *   DUNVIEW.C:3940-4007  F0108 floor ornaments
 *   DUNVIEW.C:6816-6831  field draw after creature pass (fluxcage)
 *   DUNVIEW.C:6204-6218  door panel + frame draw
 *   DUNVIEW.C:6361-6499  D2L2/D2R2 near-wall draw
 *   DUNVIEW.C:6697-6720  D3C center wall draw
 *   DUNGEON.C:1-900      CSB dungeon surface dispatch
 *   DUNGEON.C:360-430    CSB wall/floor sets and ornament selection
 *   PROJEXPL.C:43-92     projectile creation and C48/C49 movement
 *   PROJEXPL.C:95-165    explosion thing creation
 *   PROJEXPL.C:817-864   explosion damage/state processing
 *   PROJEXPL.C:987-994   fluxcage removal
 *   DRAWVIEW.C:1-200    creature sprite framing from G0011_i_CreaturePosture
 *   PANEL.C:418-428     G0304_i_DungeonViewPaletteIndex (6 levels 0-5)
 *   DATA.C:359-360       k_source_palette_light_amount_floor[]
 *   FONT.C / DEFS.H:2218 M648_INSCRIPTION_FONT / M653_GRAPHIC_FONT
 *   ENTRANCE.C:1-900   CSB title/entrance animation
 *   ENTRANCE.C:409-441  C28_ENTRANCE_CSB palette (CSB-specific)
 *
 * Pipeline contract: V1 framebuffer content preserved identically.
 * Only presentation resolution and palette expansion change.
 */

#include "csb_v2_asset_pipeline_pc34.h"
#include "csb_v2_phase_gate_pc34.h"
#include "vga_palette_pc34_compat.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Module config ────────────────────────────────────────────────── */

static CSB_V2_AssetPipelineConfig g_config = {
    CSB_V2_SCALE_MODE_NATIVE,     /* scale_mode */
    CSB_V2_PALETTE_MODE_ORIGINAL, /* palette_mode */
    0,                             /* epx_enabled — V1 pass-through by default */
    0,                             /* bilinear_enabled */
    0,                             /* palette_enhanced */
    0,                             /* scanlines_enabled */
    0                              /* sharpen_enabled */
};

/* ── Per-level light floor (ReDMCSB DATA.C:359-360) ───────────────── */

const uint8_t CSB_V2_k_SourcePaletteLightFloor[CSB_V2_PALETTE_LEVELS] = {
    99, 75, 50, 25, 1, 0
};

/* ── Module state ──────────────────────────────────────────────────── */

static int g_pipeline_initialized = 0;
static CSB_V2_GfxMode g_gfx_mode = CSB_V2_GFX_MODE_V1_ORIGINAL;
static char g_modern_asset_root[512] = {0};

/* ── Corrected palette cache ──────────────────────────────────────── */

static uint8_t s_corrected_palette[CSB_V2_PALETTE_LEVELS][16][3];
static int s_corrected_lut_valid = 0;

/* ── Surface category metadata ────────────────────────────────────── */

static const char* k_category_names[CSB_V2_SURFACE_COUNT] = {
    [CSB_V2_SURFACE_WALL_BACK]      = "wall-back",
    [CSB_V2_SURFACE_WALL_NEAR]      = "wall-near",
    [CSB_V2_SURFACE_WALL_CENTER]    = "wall-center",
    [CSB_V2_SURFACE_DOOR]           = "door",
    [CSB_V2_SURFACE_FLOOR_ORNAMENT]           = "floor-ornament",
    [CSB_V2_SURFACE_WALL_ORNAMENT]  = "wall-ornament",
    [CSB_V2_SURFACE_CREATURE]       = "creature",
    [CSB_V2_SURFACE_OBJECT]         = "object",
    [CSB_V2_SURFACE_PROJECTILE]    = "projectile",
    [CSB_V2_SURFACE_EXPLOSION]      = "explosion",
    [CSB_V2_SURFACE_FLUXCAGE]       = "fluxcage",
    [CSB_V2_SURFACE_FONT]           = "font",
    [CSB_V2_SURFACE_PANEL_CHAMPION] = "panel-champion",
    [CSB_V2_SURFACE_PANEL_MESSAGE]  = "panel-message",
    [CSB_V2_SURFACE_PANEL_STATS]    = "panel-stats",
    [CSB_V2_SURFACE_PANEL_SPELL]    = "panel-spell",
    [CSB_V2_SURFACE_TITLE]          = "title",
    [CSB_V2_SURFACE_ENTRANCE]       = "entrance",
    [CSB_V2_SURFACE_CREDITS]        = "credits",
    [CSB_V2_SURFACE_CHAOS_OVERLAY]  = "chaos-overlay",
    [CSB_V2_SURFACE_PARTY_VIEW]     = "party-view",
    [CSB_V2_SURFACE_DUNGEON_FLOOR]  = "dungeon-floor",
    [CSB_V2_SURFACE_DUNGEON_CEILING]= "dungeon-ceiling",
    [CSB_V2_SURFACE_CHAMPION_IMPORT]= "champion-import",
};

static const char* k_category_evidence[CSB_V2_SURFACE_COUNT] = {
    [CSB_V2_SURFACE_WALL_BACK]       = "DUNVIEW.C:6816-6831 D3L2/D3R2; F0107:3502-3939",
    [CSB_V2_SURFACE_WALL_NEAR]       = "DUNVIEW.C:6361-6499 D2L2/D2R2",
    [CSB_V2_SURFACE_WALL_CENTER]     = "DUNVIEW.C:6697-6720 D3C; G0700_puc_Bitmap_WallSet",
    [CSB_V2_SURFACE_DOOR]           = "DUNVIEW.C:6204-6218 door panel; G0074 F0132",
    [CSB_V2_SURFACE_FLOOR_ORNAMENT] = "DUNVIEW.C:3940-4007 F0108",
    [CSB_V2_SURFACE_WALL_ORNAMENT]  = "DUNVIEW.C:3502-3939 F0107",
    [CSB_V2_SURFACE_CREATURE]        = "DRAWVIEW.C:1-200 G0011_i_CreaturePosture",
    [CSB_V2_SURFACE_OBJECT]          = "DUNVIEW.C:4547-4602 F0115",
    [CSB_V2_SURFACE_PROJECTILE]     = "PROJEXPL.C:43-92 C48/C49",
    [CSB_V2_SURFACE_EXPLOSION]      = "PROJEXPL.C:95-165 C25; PROJEXPL.C:817-864",
    [CSB_V2_SURFACE_FLUXCAGE]       = "PROJEXPL.C987-994 C24; DUNVIEW.C:6816-6831",
    [CSB_V2_SURFACE_FONT]           = "DEFS.H:2218 M648/M653",
    [CSB_V2_SURFACE_PANEL_CHAMPION] = "PANEL.C:418-428 G0304",
    [CSB_V2_SURFACE_TITLE]           = "ENTRANCE.C:1-900 F0806 C28_ENTRANCE_CSB",
    [CSB_V2_SURFACE_PANEL_STATS]    = "PANEL.C CSB champion stats",
    [CSB_V2_SURFACE_PANEL_SPELL]    = "CASTER.C chaos spell panel",
    [CSB_V2_SURFACE_CHAOS_OVERLAY]  = "csb_v2_chaos_enhanced.c",
};

/* ── Public API ────────────────────────────────────────────────────── */

void csb_v2_asset_pipeline_init(void) {
    if (g_pipeline_initialized) return;
    g_pipeline_initialized = 1;
    csb_v2_asset_rebuild_palette_lut(100, 0, 0);
}

void csb_v2_asset_pipeline_configure(const CSB_V2_AssetPipelineConfig* config) {
    if (!config) return;
    g_config = *config;
}

const CSB_V2_AssetPipelineConfig* csb_v2_asset_pipeline_get_config(void) {
    return &g_config;
}

/* ══════════════════════════════════════════════════════════════════════
 * Bilinear RGBA upscale
 * ══════════════════════════════════════════════════════════════════════ */

static void csb_v2_asset_bilinear_rgba(const uint32_t* src, int sw, int sh,
    uint32_t* dst, int dw, int dh)
{
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;
    float fx = (float)sw / (float)dw;
    float fy = (float)sh / (float)dh;

    for (int y = 0; y < dh; y++) {
        for (int x = 0; x < dw; x++) {
            float sx = x * fx;
            float sy = y * fy;
            int x0 = (int)sx;
            int y0 = (int)sy;
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            if (x1 >= sw) x1 = sw - 1;
            if (y1 >= sh) y1 = sh - 1;

            float dx = sx - (float)x0;
            float dy = sy - (float)y0;

            uint32_t tl = src[y0 * sw + x0];
            uint32_t tr = src[y0 * sw + x1];
            uint32_t bl = src[y1 * sw + x0];
            uint32_t br = src[y1 * sw + x1];

            float tx0_r = (float)((tl >> 16) & 0xFF) + ((float)((tr >> 16) & 0xFF) - (float)((tl >> 16) & 0xFF)) * dx;
            float tx0_g = (float)((tl >> 8) & 0xFF) + ((float)((tr >> 8) & 0xFF) - (float)((tl >> 8) & 0xFF)) * dx;
            float tx0_b = (float)(tl & 0xFF) + ((float)(tr & 0xFF) - (float)(tl & 0xFF)) * dx;
            float tx1_r = (float)((bl >> 16) & 0xFF) + ((float)((br >> 16) & 0xFF) - (float)((bl >> 16) & 0xFF)) * dx;
            float tx1_g = (float)((bl >> 8) & 0xFF) + ((float)((br >> 8) & 0xFF) - (float)((bl >> 8) & 0xFF)) * dx;
            float tx1_b = (float)(bl & 0xFF) + ((float)(br & 0xFF) - (float)(bl & 0xFF)) * dx;

            float val_r = tx0_r + (tx1_r - tx0_r) * dy;
            float val_g = tx0_g + (tx1_g - tx0_g) * dy;
            float val_b = tx0_b + (tx1_b - tx0_b) * dy;

            int ri = (int)(val_r < 0.0f ? 0.0f : (val_r > 255.0f ? 255.0f : val_r));
            int gi = (int)(val_g < 0.0f ? 0.0f : (val_g > 255.0f ? 255.0f : val_g));
            int bi = (int)(val_b < 0.0f ? 0.0f : (val_b > 255.0f ? 255.0f : val_b));
            dst[y * dw + x] = 0xFF000000u | ((uint32_t)ri << 16) | ((uint32_t)gi << 8) | (uint32_t)bi;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * EPX 2x — edge-preserving index pixel upscale
 * ══════════════════════════════════════════════════════════════════════ */

void csb_v2_asset_epx_upscale(const uint8_t* src, int sw, int sh,
    uint8_t* dst, int dw, int dh)
{
    int x, y;
    (void)dw; (void)dh; /* always 2x — output must be sw*2 x sh*2 */
    if (!src || !dst || sw <= 0 || sh <= 0) return;

    for (y = 0; y < sh; y++) {
        for (x = 0; x < sw; x++) {
            uint8_t P = src[y * sw + x];
            uint8_t A = (y > 0)      ? src[(y - 1) * sw + x] : P;
            uint8_t B = (x < sw - 1) ? src[y * sw + (x + 1)] : P;
            uint8_t C = (x > 0)      ? src[y * sw + (x - 1)] : P;
            uint8_t D = (y < sh - 1) ? src[(y + 1) * sw + x] : P;

            int ox = x * 2;
            int oy = y * 2;
            int ow = sw * 2;

            dst[oy * ow + ox]         = (C == A && C != D && A != B) ? A : P;
            dst[oy * ow + ox + 1]    = (A == B && A != C && B != D) ? B : P;
            dst[(oy + 1) * ow + ox]  = (D == C && D != B && C != A) ? C : P;
            dst[(oy + 1) * ow + ox + 1] = (B == D && B != A && D != C) ? D : P;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Palette expand: indexed -> RGBA8888
 * ══════════════════════════════════════════════════════════════════════ */

void csb_v2_asset_palette_expand(const uint8_t* indexed,
    int w, int h,
    int palette_level,
    const uint32_t* lut,
    uint32_t* rgba_out)
{
    int i, total;
    if (!indexed || !rgba_out) return;

    if (palette_level < 0) palette_level = 0;
    if (palette_level >= CSB_V2_PALETTE_LEVELS) palette_level = CSB_V2_PALETTE_LEVELS - 1;

    total = w * h;
    for (i = 0; i < total; i++) {
        uint8_t byte = indexed[i];
        uint8_t level = (byte >> 4) & 0x0F;
        uint8_t idx   = byte & 0x0F;
        if (level >= CSB_V2_PALETTE_LEVELS) level = (uint8_t)palette_level;

        /* LUT stored as BGRA; flip to RGBA for SDL surface */
        uint32_t bgra = lut[level * 16 + idx];
        uint8_t b = (uint8_t)(bgra >> 16);
        uint8_t g = (uint8_t)(bgra >> 8);
        uint8_t r = (uint8_t)bgra;
        rgba_out[i] = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Per-category surface upscale — V2.1 EPX pipeline
 * ══════════════════════════════════════════════════════════════════════ */

int csb_v2_asset_pipeline_process(CSB_V2_SurfaceCategory category,
    const uint8_t*  src_indexed,
    int             src_w,
    int             src_h,
    int             source_palette_level,
    uint32_t*       rgba_out,
    int*            out_w,
    int*            out_h)
{
    if (!src_indexed || !rgba_out) return -1;
    if (category <= CSB_V2_SURFACE_UNKNOWN || category >= CSB_V2_SURFACE_COUNT) return -1;

    int epx_w = g_config.epx_enabled ? src_w * 2 : src_w;
    int epx_h = g_config.epx_enabled ? src_h * 2 : src_h;
    int final_w = g_config.bilinear_enabled ? epx_w * 2 : epx_w;
    int final_h = g_config.bilinear_enabled ? epx_h * 2 : epx_h;

    /* EPX stage */
    static uint8_t s_epx_buf[320 * 2 * 200 * 2]; /* 256 KiB — 640x400 indexed */
    if (g_config.epx_enabled) {
        csb_v2_asset_epx_upscale(src_indexed, src_w, src_h, s_epx_buf, epx_w, epx_h);
    }
    const uint8_t* palette_src = g_config.epx_enabled ? s_epx_buf : src_indexed;

    /* Build per-level RGBA LUT */
    static uint32_t s_lut[CSB_V2_PALETTE_LEVELS * 16];
    int lv;
    for (lv = 0; lv < CSB_V2_PALETTE_LEVELS; lv++) {
        int pi;
        for (pi = 0; pi < 16; pi++) {
            uint8_t rr, gg, bb;
            if (g_config.palette_enhanced && s_corrected_lut_valid) {
                rr = s_corrected_palette[lv][pi][0];
                gg = s_corrected_palette[lv][pi][1];
                bb = s_corrected_palette[lv][pi][2];
            } else {
                rr = G9010_auc_VgaPaletteAll_Compat[lv][pi][0];
                gg = G9010_auc_VgaPaletteAll_Compat[lv][pi][1];
                bb = G9010_auc_VgaPaletteAll_Compat[lv][pi][2];
            }
            /* Store as BGRA (matching G9010 layout), palette_expand flips to RGBA */
            s_lut[lv * 16 + pi] = 0xFF000000u | ((uint32_t)bb << 16) | ((uint32_t)gg << 8) | rr;
        }
    }

    /* Palette expand to RGBA in s_rgba */
    static uint32_t s_rgba[320 * 4 * 200 * 4]; /* 1024 KiB — 1280x800 RGBA */
    csb_v2_asset_palette_expand(palette_src, epx_w, epx_h,
        source_palette_level, s_lut, s_rgba);

    /* Bilinear upscale if enabled */
    if (g_config.bilinear_enabled) {
        csb_v2_asset_bilinear_rgba(s_rgba, epx_w, epx_h, s_rgba, final_w, final_h);
    }

    /* Copy to output */
    int out_total = final_w * final_h;
    memcpy(rgba_out, s_rgba, (size_t)out_total * sizeof(uint32_t));

    if (out_w) *out_w = final_w;
    if (out_h) *out_h = final_h;

    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Palette LUT rebuild — gamma/brightness/contrast correction
 * ══════════════════════════════════════════════════════════════════════ */

int csb_v2_asset_rebuild_palette_lut(int gamma100, int brightness, int contrast) {
    if (gamma100 < 80 || gamma100 > 260) return -1;
    if (brightness < -50 || brightness > 50) return -1;
    if (contrast < -50 || contrast > 50) return -1;

    int lv, pi;
    for (lv = 0; lv < CSB_V2_PALETTE_LEVELS; lv++) {
        for (pi = 0; pi < 16; pi++) {
            float rv = (float)G9010_auc_VgaPaletteAll_Compat[lv][pi][0];
            float gv = (float)G9010_auc_VgaPaletteAll_Compat[lv][pi][1];
            float bv = (float)G9010_auc_VgaPaletteAll_Compat[lv][pi][2];

            /* brightness */
            rv += (float)brightness;
            gv += (float)brightness;
            bv += (float)brightness;

            /* contrast around midpoint 128 */
            float cf = 1.0f + (float)contrast / 50.0f;
            rv = 128.0f + cf * (rv - 128.0f);
            gv = 128.0f + cf * (gv - 128.0f);
            bv = 128.0f + cf * (bv - 128.0f);

            /* gamma */
            if (gamma100 != 100) {
                float inv_gamma = 100.0f / (float)gamma100;
                float rf = rv / 255.0f;
                float gf = gv / 255.0f;
                float bf = bv / 255.0f;
                rf = powf(rf > 0.0f ? rf : 1e-6f, inv_gamma);
                gf = powf(gf > 0.0f ? gf : 1e-6f, inv_gamma);
                bf = powf(bf > 0.0f ? bf : 1e-6f, inv_gamma);
                rv = rf * 255.0f;
                gv = gf * 255.0f;
                bv = bf * 255.0f;
            }

            int ri = (int)(rv < 0.0f ? 0.0f : (rv > 255.0f ? 255.0f : rv));
            int gi = (int)(gv < 0.0f ? 0.0f : (gv > 255.0f ? 255.0f : gv));
            int bi = (int)(bv < 0.0f ? 0.0f : (bv > 255.0f ? 255.0f : bv));

            uint8_t floor = CSB_V2_k_SourcePaletteLightFloor[lv];
            if (ri < floor) ri = floor;
            if (gi < floor) gi = floor;
            if (bi < floor) bi = floor;

            s_corrected_palette[lv][pi][0] = (uint8_t)ri;
            s_corrected_palette[lv][pi][1] = (uint8_t)gi;
            s_corrected_palette[lv][pi][2] = (uint8_t)bi;
        }
    }

    s_corrected_lut_valid = 1;
    return 0;
}

void csb_v2_asset_invalidate_cached_palette(void) {
    s_corrected_lut_valid = 0;
}

/* ── Graphics mode ─────────────────────────────────────────────────── */

CSB_V2_GfxMode csb_v2_asset_get_gfx_mode(void) {
    return g_gfx_mode;
}

void csb_v2_asset_set_gfx_mode(CSB_V2_GfxMode mode) {
    if (mode < CSB_V2_GFX_MODE_V1_ORIGINAL || mode > CSB_V2_GFX_MODE_V2_MODERN) return;
    g_gfx_mode = mode;

    switch (mode) {
        case CSB_V2_GFX_MODE_V1_ORIGINAL:
            g_config.epx_enabled = 0;
            g_config.bilinear_enabled = 0;
            g_config.palette_enhanced = 0;
            g_config.scanlines_enabled = 0;
            g_config.sharpen_enabled = 0;
            g_config.scale_mode = CSB_V2_SCALE_MODE_NATIVE;
            break;
        case CSB_V2_GFX_MODE_V2_FILTERED:
            g_config.epx_enabled = 0;
            g_config.bilinear_enabled = 0;
            g_config.palette_enhanced = 1;
            g_config.scanlines_enabled = 1;
            g_config.sharpen_enabled = 0;
            g_config.scale_mode = CSB_V2_SCALE_MODE_NATIVE;
            csb_v2_asset_rebuild_palette_lut(220, 0, 0);
            break;
        case CSB_V2_GFX_MODE_V2_UPSCALED:
            g_config.epx_enabled = 1;
            g_config.bilinear_enabled = 0;
            g_config.palette_enhanced = 0;
            g_config.scanlines_enabled = 0;
            g_config.sharpen_enabled = 0;
            g_config.scale_mode = CSB_V2_SCALE_MODE_EPX_2X;
            break;
        case CSB_V2_GFX_MODE_V2_MODERN:
            g_config.epx_enabled = 1;
            g_config.bilinear_enabled = 0;
            g_config.palette_enhanced = 0;
            g_config.scanlines_enabled = 0;
            g_config.sharpen_enabled = 0;
            g_config.scale_mode = CSB_V2_SCALE_MODE_EPX_2X;
            break;
    }
}

int csb_v2_asset_is_v2_mode(void) {
    return g_gfx_mode != CSB_V2_GFX_MODE_V1_ORIGINAL ? 1 : 0;
}

int csb_v2_asset_is_v2_filtered(void) {
    return g_gfx_mode >= CSB_V2_GFX_MODE_V2_FILTERED ? 1 : 0;
}

int csb_v2_asset_is_v2_upscaled(void) {
    return g_gfx_mode >= CSB_V2_GFX_MODE_V2_UPSCALED ? 1 : 0;
}

int csb_v2_asset_is_v2_modern(void) {
    return g_gfx_mode == CSB_V2_GFX_MODE_V2_MODERN ? 1 : 0;
}

/* ── Surface category names ────────────────────────────────────────── */

const char* csb_v2_asset_surface_category_name(CSB_V2_SurfaceCategory cat) {
    if (cat <= CSB_V2_SURFACE_UNKNOWN || cat >= CSB_V2_SURFACE_COUNT) return "unknown";
    return k_category_names[cat];
}

/* ── V2.2 Modern Asset Mode ────────────────────────────────────────── */

static void csb_v2_get_home_dir(char* out, size_t out_size) {
    if (!out || out_size == 0) return;
    out[0] = '\0';
#if defined(_WIN32) || defined(_WIN64)
    {
        const char* home = getenv("USERPROFILE");
        if (home && home[0]) snprintf(out, out_size, "%s", home);
    }
#else
    {
        const char* home = getenv("HOME");
        if (home && home[0]) snprintf(out, out_size, "%s", home);
    }
#endif
}

int csb_v2_asset_load_modern_asset_manifest(void) {
    char home[512] = {0};
    char manifest_path[768] = {0};
    FILE* fp = NULL;

    csb_v2_get_home_dir(home, sizeof(home));
    if (!home[0]) return 0;

    snprintf(manifest_path, sizeof(manifest_path),
             "%s/.firestaff/assets/csb/modern/modern_asset_manifest.json", home);

    fp = fopen(manifest_path, "r");
    if (!fp) {
        g_modern_asset_root[0] = '\0';
        return 0;
    }
    fclose(fp);

    snprintf(g_modern_asset_root, sizeof(g_modern_asset_root),
             "%s/.firestaff/assets/csb/modern", home);
    return 1;
}

void csb_v2_asset_get_modern_asset_root(char* out, size_t out_size) {
    if (!out || out_size == 0) return;
    if (g_modern_asset_root[0]) {
        snprintf(out, out_size, "%s", g_modern_asset_root);
    } else {
        out[0] = '\0';
    }
}

/* ── Shape source selection ────────────────────────────────────────── */

static const char* k_shape_source_names[CSB_V22_SHAPE_SOURCE_COUNT] = {
    [CSB_V22_SHAPE_SOURCE_V1_ORIGINAL] = "V1_ORIGINAL",
    [CSB_V22_SHAPE_SOURCE_V2_FILTERED]  = "V2_FILTERED",
    [CSB_V22_SHAPE_SOURCE_V2_UPSCALED]  = "V2_UPSCALED",
    [CSB_V22_SHAPE_SOURCE_V2_MODERN]    = "V2_MODERN",
};

const char* csb_v2_shape_source_name(CSB_V22_ShapeSource src) {
    if (src < 0 || src >= CSB_V22_SHAPE_SOURCE_COUNT) return "UNKNOWN";
    return k_shape_source_names[src];
}

CSB_V22_ShapeSource csb_v2_best_available_shape_source(int presentation_mode_index) {
    /* presentation_mode_index: 0=V1, 1=V2.0, 2=V2.1, 3=V2.2 */
    switch (presentation_mode_index) {
        case 3: /* V2.2 MODERN */
            if (g_gfx_mode == CSB_V2_GFX_MODE_V2_MODERN
                && g_modern_asset_root[0] != '\0') {
                return CSB_V22_SHAPE_SOURCE_V2_MODERN;
            }
            /* fall through */
        case 2: /* V2.1 UPSCALED */
            if (g_gfx_mode >= CSB_V2_GFX_MODE_V2_UPSCALED) {
                return CSB_V22_SHAPE_SOURCE_V2_UPSCALED;
            }
            /* fall through */
        case 1: /* V2.0 FILTERED */
            if (g_gfx_mode >= CSB_V2_GFX_MODE_V2_FILTERED) {
                return CSB_V22_SHAPE_SOURCE_V2_FILTERED;
            }
            /* fall through */
        default:
            return CSB_V22_SHAPE_SOURCE_V1_ORIGINAL;
    }
}

/* ── Source evidence ────────────────────────────────────────────────── */

const char* csb_v2_asset_pipeline_source_evidence(void) {
    return
        "CSB V2.1 Graphics Pipeline\n"
        "Source: ReDMCSB WIP20210206 "
        "(DUNVIEW.C, DUNGEON.C, PROJEXPL.C, DRAWVIEW.C, PANEL.C, "
        "DATA.C, FONT.C, ENTRANCE.C) + CSBWin/Viewport.cpp\n"
        "\n"
        "Pipeline: V1 indexed -> EPX 2x -> palette expand -> RGBA\n"
        "Fallback: MODERN -> UPSCALED -> FILTERED -> V1 ORIGINAL\n";
}
