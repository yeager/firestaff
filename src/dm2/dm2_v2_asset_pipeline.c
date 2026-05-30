/*
 * dm2_v2_asset_pipeline.c — DM2 V2.1 Graphics Pipeline
 *
 * Phase 2: Enhanced asset pipeline for Dungeon Master II: Skullkeep.
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
 * Source-lock anchors (SKULL.ASM / ReDMCSB):
 *   SKULL.ASM T560  — dungeon viewport rendering
 *   SKULL.ASM T600  — outdoor viewport rendering
 *   SKULL.ASM T520  — party/movement tick
 *   SKULL.ASM T580  — dungeon load and asset verification
 *   ReDMCSB DUNVIEW.C:575-586  — G0163 wall frame table
 *   ReDMCSB DUNVIEW.C:148-157  — wall set indices (G3060 variant)
 *   ReDMCSB DUNVIEW.C:2962-3047 — F0098 DrawFloorAndCeiling
 *   ReDMCSB DUNVIEW.C:3048-3070 — F0100 DrawWallSetBitmap
 *   ReDMCSB DUNVIEW.C:3082-3095 — F0102 DrawDoorBitmap
 *   ReDMCSB DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament
 *   ReDMCSB DUNVIEW.C:4016-4050 — F0109 DrawDoorOrnament
 *   ReDMCSB DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor
 *   ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *   ReDMCSB PANEL.C:418-428     — G0304_i_DungeonViewPaletteIndex (6 levels)
 *   ReDMCSB DATA.C:359-360      — k_source_palette_light_amount_floor[]
 *
 * Pipeline contract: V1 framebuffer content preserved identically.
 * Only presentation resolution and palette expansion change.
 */

#include "dm2_v2_asset_pipeline.h"
#include "dm2_v2_phase_gate.h"
#include "vga_palette_pc34_compat.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Module config ────────────────────────────────────────────────── */

static DM2_V2_AssetPipelineConfig g_config = {
    DM2_V2_SCALE_MODE_NATIVE,      /* scale_mode */
    DM2_V2_PALETTE_MODE_ORIGINAL,  /* palette_mode */
    0,                             /* epx_enabled — V1 pass-through by default */
    0,                             /* bilinear_enabled */
    0,                             /* palette_enhanced */
    0,                             /* scanlines_enabled */
    0                              /* sharpen_enabled */
};

/* ── Per-level light floor (ReDMCSB DATA.C:359-360) ───────────────── */

const uint8_t DM2_V2_k_SourcePaletteLightFloor[DM2_V2_PALETTE_LEVELS] = {
    99, 75, 50, 25, 1, 0
};

/* ── Module state ──────────────────────────────────────────────────── */

static int g_pipeline_initialized = 0;
static DM2_V2_GfxMode g_gfx_mode = DM2_V2_GFX_MODE_V1_ORIGINAL;
static char g_modern_asset_root[512] = {0};

/* ── Corrected palette cache ──────────────────────────────────────── */

static uint8_t s_corrected_palette[DM2_V2_PALETTE_LEVELS][16][3];
static int s_corrected_lut_valid = 0;

/* ── Surface category metadata ────────────────────────────────────── */

static const char* k_category_names[DM2_V2_SURFACE_COUNT] = {
    [DM2_V2_SURFACE_UNKNOWN]         = "unknown",

    /* Dungeon viewport surfaces */
    [DM2_V2_SURFACE_WALL_BACK]       = "wall-back",
    [DM2_V2_SURFACE_WALL_NEAR]       = "wall-near",
    [DM2_V2_SURFACE_WALL_CENTER]     = "wall-center",
    [DM2_V2_SURFACE_DOOR]            = "door",
    [DM2_V2_SURFACE_FLOOR_ORNAMENT]  = "floor-ornament",
    [DM2_V2_SURFACE_WALL_ORNAMENT]   = "wall-ornament",
    [DM2_V2_SURFACE_CREATURE]         = "creature",
    [DM2_V2_SURFACE_OBJECT]           = "object",
    [DM2_V2_SURFACE_PROJECTILE]       = "projectile",
    [DM2_V2_SURFACE_EXPLOSION]        = "explosion",
    [DM2_V2_SURFACE_FLUXCAGE]         = "fluxcage",

    /* UI chrome surfaces */
    [DM2_V2_SURFACE_FONT]             = "font",
    [DM2_V2_SURFACE_PANEL_CHAMPION]   = "panel-champion",
    [DM2_V2_SURFACE_PANEL_MESSAGE]    = "panel-message",
    [DM2_V2_SURFACE_PANEL_STATS]      = "panel-stats",
    [DM2_V2_SURFACE_PANEL_SPELL]      = "panel-spell",
    [DM2_V2_SURFACE_ACTION_STRIP]      = "action-strip",
    [DM2_V2_SURFACE_GOLD_COUNTER]     = "gold-counter",
    [DM2_V2_SURFACE_TOP_STATUS_BAR]   = "top-status-bar",

    /* Title / entrance surfaces */
    [DM2_V2_SURFACE_TITLE]             = "title",
    [DM2_V2_SURFACE_ENTRANCE]          = "entrance",
    [DM2_V2_SURFACE_CREDITS]           = "credits",
    [DM2_V2_SURFACE_IMPORT]            = "import",

    /* Outdoor surfaces */
    [DM2_V2_SURFACE_SKY_GRADIENT]      = "sky-gradient",
    [DM2_V2_SURFACE_TERRAIN]           = "terrain",
    [DM2_V2_SURFACE_BUILDING]          = "building",
    [DM2_V2_SURFACE_WEATHER_OVERLAY]   = "weather-overlay",

    /* DM2-specific surfaces */
    [DM2_V2_SURFACE_ROOM_WALL]          = "room-wall",
    [DM2_V2_SURFACE_ROOM_FLOOR]        = "room-floor",
    [DM2_V2_SURFACE_ROOM_CEILING]      = "room-ceiling",
    [DM2_V2_SURFACE_DUNGEON_FLOOR]     = "dungeon-floor",
    [DM2_V2_SURFACE_DUNGEON_CEILING]  = "dungeon-ceiling",
};

static const char* k_category_evidence[DM2_V2_SURFACE_COUNT] = {
    [DM2_V2_SURFACE_WALL_BACK]      = "DUNVIEW.C:575-586 G0163 wall frame table; DUNVIEW.C:6816-6831 D3L2/D3R2",
    [DM2_V2_SURFACE_WALL_NEAR]      = "DUNVIEW.C:148-157 G3060 variant wall sets; DUNVIEW.C:6361-6499 D2L2/D2R2/D2C",
    [DM2_V2_SURFACE_WALL_CENTER]    = "DUNVIEW.C:3048-3070 F0100 DrawWallSetBitmap; G3060_i_WallSet_Wall_D3C",
    [DM2_V2_SURFACE_DOOR]           = "DUNVIEW.C:4119-4270 F0110 DrawDoorButton, F0111 DrawDoor; G2116-G2119 door frames",
    [DM2_V2_SURFACE_FLOOR_ORNAMENT] = "DUNVIEW.C:3940-4015 F0108_DrawFloorOrnament; floor ornament seed DUNGEON.C:1371",
    [DM2_V2_SURFACE_WALL_ORNAMENT]  = "DUNVIEW.C:4016-4050 F0109 DrawDoorOrnament; alcove detection",
    [DM2_V2_SURFACE_CREATURE]       = "SKULL.ASM creature render; SKULLWIN creature sprite blit",
    [DM2_V2_SURFACE_OBJECT]         = "DUNVIEW.C F0115 object pass",
    [DM2_V2_SURFACE_PROJECTILE]    = "SKULL.ASM projectile render",
    [DM2_V2_SURFACE_EXPLOSION]      = "SKULL.ASM explosion render",
    [DM2_V2_SURFACE_FLUXCAGE]       = "SKULL.ASM fluxcage field",
    [DM2_V2_SURFACE_FONT]           = "DEFS.H M648_INSCRIPTION_FONT / M653_GRAPHIC_FONT",
    [DM2_V2_SURFACE_PANEL_CHAMPION] = "SKULL.ASM champion portrait panel; PANEL.C G0304",
    [DM2_V2_SURFACE_PANEL_STATS]    = "SKULL.ASM champion stats (HP/MP/condition)",
    [DM2_V2_SURFACE_ACTION_STRIP]    = "SKULL.ASM T560 bottom action strip (Attack/Cast/Use/Drop/Move)",
    [DM2_V2_SURFACE_GOLD_COUNTER]   = "SKULL.ASM gold counter (DM2-specific)",
    [DM2_V2_SURFACE_TOP_STATUS_BAR] = "SKULL.ASM T560 top status bar (DM2-specific, no portraits)",
    [DM2_V2_SURFACE_TITLE]          = "SKULL.ASM F0806 title screen",
    [DM2_V2_SURFACE_ENTRANCE]       = "SKULL.ASM entrance animation",
    [DM2_V2_SURFACE_SKY_GRADIENT]   = "SKULL.ASM T600 outdoor sky; dm2_v1_outdoor_renderer.c",
    [DM2_V2_SURFACE_TERRAIN]        = "SKULL.ASM T600 outdoor terrain",
    [DM2_V2_SURFACE_BUILDING]       = "SKULL.ASM T600 outdoor building exterior",
    [DM2_V2_SURFACE_WEATHER_OVERLAY]= "dm2_v1_weather.c; SKULL.ASM T600 weather",
    [DM2_V2_SURFACE_ROOM_WALL]      = "DM2 room structure (rooms vs corridors); DUNGEON.C room walls",
    [DM2_V2_SURFACE_ROOM_FLOOR]    = "DM2 room floor; DUNGEON.C room rendering",
    [DM2_V2_SURFACE_ROOM_CEILING]  = "DM2 room ceiling; DUNGEON.C room rendering",
    [DM2_V2_SURFACE_DUNGEON_FLOOR] = "DUNVIEW.C:2962-3047 F0098 DrawFloorAndCeiling; G2108_Floor=-1",
    [DM2_V2_SURFACE_DUNGEON_CEILING]="DUNVIEW.C:2962-3047 F0098 DrawFloorAndCeiling; G2109_Ceiling=-2",
};

/* ── Public API ────────────────────────────────────────────────────── */

void dm2_v2_asset_pipeline_init(void) {
    if (g_pipeline_initialized) return;
    g_pipeline_initialized = 1;
    dm2_v2_asset_rebuild_palette_lut(100, 0, 0);
}

void dm2_v2_asset_pipeline_configure(const DM2_V2_AssetPipelineConfig* config) {
    if (!config) return;
    g_config = *config;
}

const DM2_V2_AssetPipelineConfig* dm2_v2_asset_pipeline_get_config(void) {
    return &g_config;
}

/* ── Graphics mode ─────────────────────────────────────────────────── */

DM2_V2_GfxMode dm2_v2_asset_get_gfx_mode(void) {
    return g_gfx_mode;
}

void dm2_v2_asset_set_gfx_mode(DM2_V2_GfxMode mode) {
    if (mode < DM2_V2_GFX_MODE_V1_ORIGINAL || mode > DM2_V2_GFX_MODE_V2_MODERN) return;
    g_gfx_mode = mode;
}

int dm2_v2_asset_is_v2_mode(void) {
    return g_gfx_mode >= DM2_V2_GFX_MODE_V2_FILTERED ? 1 : 0;
}

int dm2_v2_asset_is_v2_filtered(void) {
    return g_gfx_mode >= DM2_V2_GFX_MODE_V2_FILTERED ? 1 : 0;
}

int dm2_v2_asset_is_v2_upscaled(void) {
    return g_gfx_mode >= DM2_V2_GFX_MODE_V2_UPSCALED ? 1 : 0;
}

int dm2_v2_asset_is_v2_modern(void) {
    return g_gfx_mode == DM2_V2_GFX_MODE_V2_MODERN ? 1 : 0;
}

/* ── Bilinear RGBA upscale ────────────────────────────────────────── */

static void dm2_v2_asset_bilinear_rgba(const uint32_t* src, int sw, int sh,
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

/* ── EPX 2x — edge-preserving indexed pixel upscale ───────────────── */

void dm2_v2_asset_epx_upscale(const uint8_t* src, int sw, int sh,
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
            dst[oy * ow + ox + 1]     = (A == B && A != C && B != D) ? B : P;
            dst[(oy + 1) * ow + ox]   = (D == C && D != B && C != A) ? C : P;
            dst[(oy + 1) * ow + ox + 1] = (B == D && B != A && D != C) ? D : P;
        }
    }
}

/* ── Palette expand: indexed -> RGBA8888 ──────────────────────────── */

void dm2_v2_asset_palette_expand(const uint8_t* indexed,
    int w, int h,
    int palette_level,
    const uint32_t* lut,
    uint32_t* rgba_out)
{
    int i, total;
    if (!indexed || !rgba_out) return;

    if (palette_level < 0) palette_level = 0;
    if (palette_level >= DM2_V2_PALETTE_LEVELS) palette_level = DM2_V2_PALETTE_LEVELS - 1;

    total = w * h;
    for (i = 0; i < total; i++) {
        uint8_t byte = indexed[i];
        uint8_t level = (byte >> 4) & 0x0F;
        uint8_t idx   = byte & 0x0F;
        if (level >= DM2_V2_PALETTE_LEVELS) level = (uint8_t)palette_level;

        /* LUT stored as BGRA; flip to RGBA for SDL surface */
        uint32_t bgra = lut[level * 16 + idx];
        uint8_t b = (uint8_t)(bgra >> 16);
        uint8_t g = (uint8_t)(bgra >> 8);
        uint8_t r = (uint8_t)bgra;
        rgba_out[i] = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
}

/* ── Per-category surface upscale — V2.1 EPX pipeline ─────────────── */

int dm2_v2_asset_pipeline_process(DM2_V2_SurfaceCategory category,
    const uint8_t*  src_indexed,
    int             src_w,
    int             src_h,
    int             source_palette_level,
    uint32_t*       rgba_out,
    int*            out_w,
    int*            out_h)
{
    if (!src_indexed || !rgba_out) return -1;
    if (category <= DM2_V2_SURFACE_UNKNOWN || category >= DM2_V2_SURFACE_COUNT) return -1;

    int epx_w = g_config.epx_enabled ? src_w * 2 : src_w;
    int epx_h = g_config.epx_enabled ? src_h * 2 : src_h;
    int final_w = g_config.bilinear_enabled ? epx_w * 2 : epx_w;
    int final_h = g_config.bilinear_enabled ? epx_h * 2 : epx_h;

    /* EPX stage */
    static uint8_t s_epx_buf[320 * 2 * 200 * 2]; /* 256 KiB — 640x400 indexed */
    if (g_config.epx_enabled) {
        dm2_v2_asset_epx_upscale(src_indexed, src_w, src_h, s_epx_buf, epx_w, epx_h);
    }
    const uint8_t* palette_src = g_config.epx_enabled ? s_epx_buf : src_indexed;

    /* Build per-level RGBA LUT */
    static uint32_t s_lut[DM2_V2_PALETTE_LEVELS * 16];
    int lv;
    for (lv = 0; lv < DM2_V2_PALETTE_LEVELS; lv++) {
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
            /* Store as BGRA (matching G9010 layout); palette_expand flips to RGBA */
            s_lut[lv * 16 + pi] = 0xFF000000u | ((uint32_t)bb << 16) | ((uint32_t)gg << 8) | rr;
        }
    }

    /* Palette expand to RGBA in s_rgba */
    static uint32_t s_rgba[320 * 4 * 200 * 4]; /* 1024 KiB — 1280x800 RGBA */
    dm2_v2_asset_palette_expand(palette_src, epx_w, epx_h,
        source_palette_level, s_lut, s_rgba);

    /* Bilinear upscale if enabled */
    if (g_config.bilinear_enabled) {
        dm2_v2_asset_bilinear_rgba(s_rgba, epx_w, epx_h, s_rgba, final_w, final_h);
    }

    /* Copy to output */
    int out_total = final_w * final_h;
    memcpy(rgba_out, s_rgba, (size_t)out_total * sizeof(uint32_t));

    if (out_w) *out_w = final_w;
    if (out_h) *out_h = final_h;

    (void)category; /* reserved for future category-specific processing */
    return 0;
}

/* ── Palette LUT (stub — gamma/brightness/contrast correction) ────── */

int dm2_v2_asset_rebuild_palette_lut(int gamma100, int brightness, int contrast) {
    /* Stub: palette correction not yet implemented for DM2.
     * When the V2.0 FILTERED mode is activated, this function should
     * apply gamma/brightness/contrast to G9010 source palette entries
     * and store results in s_corrected_palette[].
     * Until then, s_corrected_lut_valid stays 0 and the pipeline
     * uses G9010_auc_VgaPaletteAll_Compat directly. */
    (void)gamma100; (void)brightness; (void)contrast;
    /* Mark as valid when implemented */
    /* s_corrected_lut_valid = 1; */
    (void)s_corrected_palette; /* suppress unused-var warning until implemented */
    return 0;
}

void dm2_v2_asset_invalidate_cached_palette(void) {
    s_corrected_lut_valid = 0;
}

/* ── V2.2 Modern Asset Mode ───────────────────────────────────────── */

int dm2_v2_asset_load_modern_asset_manifest(void) {
    /* Scan for modern asset manifest at:
     *   ~/.firestaff/assets/dm2/modern/modern_asset_manifest.json
     *
     * If present, record root for renderer path building.
     * If absent, silently fall back to V2.1 EPX pipeline. */
    char home[512] = {0};
    char manifest_path[768] = {0};
    FILE* fp = NULL;

#if defined(_WIN32) || defined(_WIN64)
    {
        const char* home_env = getenv("USERPROFILE");
        if (home_env && home_env[0]) snprintf(home, sizeof(home), "%s", home_env);
    }
#else
    {
        const char* home_env = getenv("HOME");
        if (home_env && home_env[0]) snprintf(home, sizeof(home), "%s", home_env);
    }
#endif

    if (!home[0]) return 0;

    snprintf(manifest_path, sizeof(manifest_path),
             "%s/.firestaff/assets/dm2/modern/modern_asset_manifest.json", home);

    fp = fopen(manifest_path, "r");
    if (!fp) {
        g_modern_asset_root[0] = '\0';
        return 0;
    }
    fclose(fp);

    snprintf(g_modern_asset_root, sizeof(g_modern_asset_root),
             "%s/.firestaff/assets/dm2/modern", home);

    return 1;
}

void dm2_v2_asset_get_modern_asset_root(char* out, size_t out_size) {
    if (!out || out_size == 0) return;
    snprintf(out, out_size, "%s", g_modern_asset_root);
}

/* ── Shape source selection ───────────────────────────────────────── */

DM2_V22_ShapeSource dm2_v2_best_available_shape_source(int presentation_mode_index) {
    /* Fallback chain: MODERN -> UPSCALED -> FILTERED -> ORIGINAL */
    switch (presentation_mode_index) {
        case 3: /* V2.2 MODERN */
            if (g_modern_asset_root[0]) return DM2_V22_SHAPE_SOURCE_V2_MODERN;
            /* fall through */
        case 2: /* V2.1 UPSCALED */
            if (g_config.epx_enabled) return DM2_V22_SHAPE_SOURCE_V2_UPSCALED;
            /* fall through */
        case 1: /* V2.0 FILTERED */
            if (g_config.scanlines_enabled || g_config.palette_enhanced)
                return DM2_V22_SHAPE_SOURCE_V2_FILTERED;
            /* fall through */
        default:
            return DM2_V22_SHAPE_SOURCE_V1_ORIGINAL;
    }
}

const char* dm2_v2_shape_source_name(DM2_V22_ShapeSource src) {
    switch (src) {
        case DM2_V22_SHAPE_SOURCE_V1_ORIGINAL: return "V1_ORIGINAL";
        case DM2_V22_SHAPE_SOURCE_V2_FILTERED: return "V2_FILTERED";
        case DM2_V22_SHAPE_SOURCE_V2_UPSCALED: return "V2_UPSCALED";
        case DM2_V22_SHAPE_SOURCE_V2_MODERN:   return "V2_MODERN";
        default: return "UNKNOWN";
    }
}

const char* dm2_v2_asset_surface_category_name(DM2_V2_SurfaceCategory cat) {
    if (cat <= DM2_V2_SURFACE_UNKNOWN || cat >= DM2_V2_SURFACE_COUNT) return "unknown";
    return k_category_names[cat];
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char* dm2_v2_asset_pipeline_source_evidence(void) {
    return
        "DM2 V2.1 Graphics Pipeline — source-asset-preserving EPX upscale\n"
        "\n"
        "Source anchors (SKULL.ASM / ReDMCSB):\n"
        "  SKULL.ASM T520  — party/movement tick (V1 invariant)\n"
        "  SKULL.ASM T560  — dungeon viewport rendering\n"
        "  SKULL.ASM T580  — dungeon load and asset verification\n"
        "  SKULL.ASM T600  — outdoor viewport rendering\n"
        "  DUNVIEW.C:575-586  — G0163 wall frame table\n"
        "  DUNVIEW.C:148-157  — wall set indices (G3060 variant)\n"
        "  DUNVIEW.C:2962-3047 — F0098 DrawFloorAndCeiling\n"
        "  DUNVIEW.C:3048-3070 — F0100 DrawWallSetBitmap\n"
        "  DUNVIEW.C:3082-3095 — F0102 DrawDoorBitmap\n"
        "  DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament\n"
        "  DUNVIEW.C:4016-4050 — F0109 DrawDoorOrnament\n"
        "  DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor\n"
        "  DUNGEON.C:1371-1421 — map coordinate resolution\n"
        "  PANEL.C:418-428      — G0304_i_DungeonViewPaletteIndex (6 levels)\n"
        "  DATA.C:359-360       — k_source_palette_light_amount_floor[]\n"
        "\n"
        "Pipeline: V1 indexed → EPX 2x (indexed, edge-preserving)\n"
        "       → per-level palette expand (G9010_auc_VgaPaletteAll_Compat)\n"
        "       → RGBA8888 → optional bilinear upscale\n"
        "\n"
        "V2.0 (scanlines/palette correction) applies AFTER this pipeline.\n"
        "Phase 2 contract: V1 framebuffer content preserved identically;\n"
        "only the presentation resolution and palette expansion change.\n"
        "\n"
        "DM2-specific surfaces: outdoor sky/terrain, room walls/floors/ceilings,\n"
        "gold counter, top status bar, action strip (vs DM1 champion portraits).\n";
}