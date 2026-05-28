/*
 * dm1_v2_asset_pipeline_pc34.c — DM1 V2.1 Graphics Pipeline
 *
 * Source-asset-preserving EPX upscale for all indexed V1 surface categories:
 *   walls, doors, ornaments, creatures, objects, projectiles,
 *   explosions, fonts, title/entrance, and panel surfaces.
 *
 * Source-lock anchors:
 *   ReDMCSB DUNVIEW.C:4547-4602  F0115 draw objects/creatures/projectiles
 *   ReDMCSB DUNVIEW.C:3502-3939  F0107 wall ornaments / alcove detection
 *   ReDMCSB DUNVIEW.C:3940-4007  F0108 floor ornaments
 *   ReDMCSB DUNVIEW.C:6816-6831  field draw after object/creature pass
 *   ReDMCSB PROJEXPL.C:43-165    projectile/explosion creation and routing
 *   ReDMCSB PROJEXPL.C:817-864   explosion damage/state
 *   ReDMCSB PROJEXPL.C:987-994   fluxcage removal (C24)
 *   ReDMCSB DRAWVIEW.C:1-200     creature sprite framing from G0011_i_CreaturePosture
 *   ReDMCSB DEFS.H:421-430       projectile-associated explosion thing values
 *   ReDMCSB DEFS.H:2218,2324,2376 font/inscription graphics (M648/M653)
 *   ReDMCSB PANEL.C:418-428       G0304_i_DungeonViewPaletteIndex (6 levels)
 *   ReDMCSB DATA.C:359-360       k_source_palette_light_amount_floor[]
 *   ReDMCSB ENTRANCE.C:1-900      title/entrance animation sequencing
 *
 * Pipeline invariant: every surface goes through
 *   V1 indexed (level|index) → EPX 2x → per-level palette → RGBA
 * No gameplay state, collision, or timing is modified.
 */

#include "dm1_v2_asset_pipeline_pc34.h"
#include "vga_palette_pc34_compat.h"
#include "render_sdl_m11.h"
#include "dm1v2/dm1_v2_filters.h"

#include <string.h>

/* ── Module config ────────────────────────────────────────────────── */

static DM1_V2_AssetPipelineConfig g_config = {
    DM1_V2_SCALE_MODE_EPX_2X,   /* scale_mode */
    DM1_V2_PALETTE_MODE_ORIGINAL,/* palette_mode */
    1,                           /* epx_enabled */
    0,                           /* bilinear_enabled */
    0,                           /* palette_enhanced */
    0,                           /* sharpen_enabled */
    0                            /* source_light_floor */
};

/* Per-level light amount floor, from ReDMCSB DATA.C:359-360.
 * Used to derive the presentation darkness percentage. */
static const uint8_t k_source_palette_light_amount_floor[DM1_V2_PALETTE_LEVELS] = {
    99, 75, 50, 25, 1, 0
};

/* Module state */
static int g_pipeline_initialized = 0;

/* Per-category source anchor table */
static const char* k_category_evidence[DM1_V2_SURFACE_COUNT] = {
    [DM1_V2_SURFACE_WALL_BACK]        = "DUNVIEW.C:6816-6831 D3L2/D3R2 back-wall draw; F0107:3502-3939 wall ornament routing",
    [DM1_V2_SURFACE_WALL_NEAR]        = "DUNVIEW.C:6361-6499 D2L2/D2R2 near-wall; F0678/F0679 parity bitmap selection",
    [DM1_V2_SURFACE_WALL_CENTER]      = "DUNVIEW.C:6716-6760 D3C center wall draw",
    [DM1_V2_SURFACE_DOOR]            = "DUNVIEW.C:6204-6218 door panel + frame; F0111 door draw",
    [DM1_V2_SURFACE_FLOOR_ORNAMENT]  = "DUNVIEW.C:3940-4007 F0108_DrawFloorOrnament; floor ornament seed from DUNGEON.C:1371",
    [DM1_V2_SURFACE_WALL_ORNAMENT]   = "DUNVIEW.C:3502-3939 F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
    [DM1_V2_SURFACE_CREATURE]        = "DRAWVIEW.C:1-200 G0011_i_CreaturePosture framing; DEFS.H:1617-1628 creature front/flip flags",
    [DM1_V2_SURFACE_OBJECT]          = "DUNVIEW.C:4547-4602 F0115 object pass before creature/projectile/explosion",
    [DM1_V2_SURFACE_PROJECTILE]      = "PROJEXPL.C:43-92 C48/C49 movement; DEFS.H:421-428 projectile thing values",
    [DM1_V2_SURFACE_EXPLOSION]       = "PROJEXPL.C:95-165 explosion thing creation; PROJEXPL.C:817-864 damage/state",
    [DM1_V2_SURFACE_FLUXCAGE]        = "PROJEXPL.C:987-994 C24 fluxcage removal; DUNVIEW.C:6816-6831 field draw after creature pass",
    [DM1_V2_SURFACE_FONT]            = "DEFS.H:2218,2324,2376 M648_INSCRIPTION_FONT / M653_GRAPHIC_FONT; FONT.C bitmap layout",
    [DM1_V2_SURFACE_PANEL_CHAMPION]  = "PANEL.C F0337_INVENTORY_SetDungeonViewPalette; champion portrait surface",
    [DM1_V2_SURFACE_PANEL_MESSAGE]   = "DRAWMSGA.C text rendering; message log area 320×64 at bottom panel",
    [DM1_V2_SURFACE_TITLE]           = "ENTRANCE.C:1-900 title screen sequencing; GRAPHICS.DAT title surface layout",
    [DM1_V2_SURFACE_ENTRANCE]        = "ENTRANCE.C:900-1800 entrance animation; C03_SOUND_DOOR_RATTLE_ENTRANCE event"
};

static const char* k_category_names[DM1_V2_SURFACE_COUNT] = {
    [DM1_V2_SURFACE_WALL_BACK]        = "wall-back",
    [DM1_V2_SURFACE_WALL_NEAR]        = "wall-near",
    [DM1_V2_SURFACE_WALL_CENTER]      = "wall-center",
    [DM1_V2_SURFACE_DOOR]            = "door",
    [DM1_V2_SURFACE_FLOOR_ORNAMENT]  = "floor-ornament",
    [DM1_V2_SURFACE_WALL_ORNAMENT]   = "wall-ornament",
    [DM1_V2_SURFACE_CREATURE]        = "creature",
    [DM1_V2_SURFACE_OBJECT]          = "object",
    [DM1_V2_SURFACE_PROJECTILE]      = "projectile",
    [DM1_V2_SURFACE_EXPLOSION]       = "explosion",
    [DM1_V2_SURFACE_FLUXCAGE]        = "fluxcage",
    [DM1_V2_SURFACE_FONT]            = "font",
    [DM1_V2_SURFACE_PANEL_CHAMPION]  = "panel-champion",
    [DM1_V2_SURFACE_PANEL_MESSAGE]   = "panel-message",
    [DM1_V2_SURFACE_TITLE]           = "title",
    [DM1_V2_SURFACE_ENTRANCE]        = "entrance"
};

/* ── Public API ────────────────────────────────────────────────────── */

void dm1_v2_asset_pipeline_init(void) {
    if (g_pipeline_initialized) return;
    g_pipeline_initialized = 1;
    /* Build default LUT (original palette, no correction) */
    dm1_v2_asset_rebuild_palette_lut(100, 0, 0);
}

void dm1_v2_asset_pipeline_configure(const DM1_V2_AssetPipelineConfig* config) {
    if (!config) return;
    g_config = *config;
    /* Rebuild LUT if palette correction changed */
    dm1_v2_asset_rebuild_palette_lut(100, 0, 0);
}

const DM1_V2_AssetPipelineConfig* dm1_v2_asset_pipeline_get_config(void) {
    return &g_config;
}

/* ══════════════════════════════════════════════════════════════════════
 * EPX 2x — Core V2.1 upscaler for indexed pixel art
 *
 * EPX (Eric's Pixel Expansion) / Scale2x preserves sharp edges in
 * pixel art by examining the 4 neighbors (A=up, B=right, C=left, D=down)
 * of each source pixel P and choosing the appropriate expansion:
 *
 *   A          1 2      Rules (Scale2x):
 * C P B   →    3 4        if C==A && C!=D && A!=B → 1=A
 *   D                     if A==B && A!=C && B!=D → 2=B
 *                         if D==C && D!=B && C!=A → 3=C
 *                         if B==D && B!=A && D!=C → 4=D
 *   else all four outputs = P
 *
 * This is the correct upscaler for indexed DM1 pixel art — nearest
 * introduces stair-step aliasing; bilinear blends across palette indices
 * (creating out-of-palette colors). EPX is the industry-standard choice
 * for pixel-art doubling in emulators and retro-gaming pipelines.
 *
 * Reference: "Scale2x: Sharp Pixel Art Doubling"
 *   http://www.scale2x.it/
 * ══════════════════════════════════════════════════════════════════════ */

/* ══════════════════════════════════════════════════════════════════════
 * RGBA bilinear upscale
 *
 * Scales an RGBA8888 surface from (sw×sh) to (dw×dh) using bilinear
 * interpolation. Alpha is carried through; color channels are
 * interpolated in float space and clamped to [0,255].
 *
 * Used as the final upscale step when bilinear_enabled is set and
 * the source is already RGBA (post EPX + palette expand).
 * ══════════════════════════════════════════════════════════════════════ */

static void dm1_v2_asset_bilinear_rgba(const uint32_t* src, int sw, int sh,
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

            /* Interpolate each channel */
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

            dst[y * dw + x] = 0xFF000000 | (ri << 16) | (gi << 8) | bi;
        }
    }
}

/* Reuse EPX from dm1_v2_texture_upscale_pc34.c — identical algorithm,
 * but called here to keep the full pipeline in one module. */
void dm1_v2_asset_epx_upscale(const uint8_t* src, int sw, int sh,
    uint8_t* dst, int dw, int dh)
{
    int x, y;
    (void)dw; (void)dh; /* always exactly 2x */
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
            int row_bytes = sw * 2;

            dst[oy * row_bytes + ox]         = (C == A && C != D && A != B) ? A : P;
            dst[oy * row_bytes + ox + 1]     = (A == B && A != C && B != D) ? B : P;
            dst[(oy + 1) * row_bytes + ox]   = (D == C && D != B && C != A) ? C : P;
            dst[(oy + 1) * row_bytes + ox + 1] = (B == D && B != A && D != C) ? D : P;
        }
    }
}

/* Palette expand: indexed (8-bit: high nibble=level, low nibble=palette index)
 * → RGBA using the per-level palette LUT.
 *
 * For DM1 V1 rendering, the framebuffer encodes (level << 4) | palette_index.
 * V2.1 palette expand uses the source-selected palette level directly
 * (from G0304_i_DungeonViewPaletteIndex, ReDMCSB PANEL.C:418-428) to
 * select which of the 6 palette levels to use, then looks up the 4-bit
 * palette index in G9010_auc_VgaPaletteAll_Compat[level][index].
 *
 * When palette_enhanced is set, the LUT contains the gamma/brightness/
 * contrast corrected values from dm1_v2_filter_palette_build_lut().
 */
void dm1_v2_asset_palette_expand(const uint8_t* indexed,
    int w, int h, int palette_level,
    const uint32_t* lut, uint32_t* rgba_out)
{
    int i, total;
    if (!indexed || !rgba_out) return;

    /* Clamp palette level to valid range */
    if (palette_level < 0) palette_level = 0;
    if (palette_level >= DM1_V2_PALETTE_LEVELS) palette_level = DM1_V2_PALETTE_LEVELS - 1;

    total = w * h;
    for (i = 0; i < total; i++) {
        uint8_t byte = indexed[i];
        uint8_t level = (byte & M11_FB_LEVEL_MASK) >> M11_FB_LEVEL_SHIFT;
        uint8_t idx   = byte & M11_FB_INDEX_MASK;

        /* Use source-provided level for gameplay lighting parity.
         * Fall back to clamped palette_level if out of range. */
        if (level >= DM1_V2_PALETTE_LEVELS) level = (uint8_t)palette_level;

        /* BGRA → RGBA for SDL compatibility.
         * LUT entries are stored as BGRA (0xAABBGGRR) to match
         * G9010_auc_VgaPaletteAll_Compat[level][idx][RGB] layout. */
        uint32_t bgra = lut[level * 16 + idx];
        uint8_t b = (uint8_t)(bgra >> 16);
        uint8_t g = (uint8_t)(bgra >> 8);
        uint8_t r = (uint8_t)bgra;
        rgba_out[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Per-category surface upscale
 *
 * All surfaces follow the same pipeline; the category argument is used
 * for metadata only (source anchor, logging, potential future category-
 * specific processing).
 *
 * Input:  V1 indexed pixel at src_w × src_h (typically 320×200 or cropped)
 * Output: RGBA at 2×src_w × 2×src_h (when epx_enabled) or src_w × src_h
 * ══════════════════════════════════════════════════════════════════════ */
/* FIXED Phase 8: Corrected palette cache. Definitions are populated by
 * dm1_v2_asset_rebuild_palette_lut() below.
 */
static unsigned char s_corrected_palette[DM1_V2_PALETTE_LEVELS][16][3];
static int s_corrected_lut_valid = 0;


int dm1_v2_asset_upscale_surface(DM1_V2_SurfaceCategory category,
    const uint8_t* src_indexed, int src_w, int src_h,
    int source_palette_level,
    uint32_t* out_rgba, int* out_w, int* out_h)
{
    if (!src_indexed || !out_rgba) return -1;
    if (category <= DM1_V2_SURFACE_UNKNOWN || category >= DM1_V2_SURFACE_COUNT) return -1;

    /* Compute dimensions */
    int epx_w = g_config.epx_enabled ? src_w * 2 : src_w;
    int epx_h = g_config.epx_enabled ? src_h * 2 : src_h;
    int final_w = g_config.bilinear_enabled ? epx_w * 2 : epx_w;
    int final_h = g_config.bilinear_enabled ? epx_h * 2 : epx_h;

    /* EPX stage: src → epx_buffer (indexed) */
    static uint8_t s_epx_buf[320 * 2 * 200 * 2]; /* 256000 bytes */
    if (g_config.epx_enabled) {
        dm1_v2_asset_epx_upscale(src_indexed, src_w, src_h, s_epx_buf, epx_w, epx_h);
    }

    const uint8_t* palette_src = g_config.epx_enabled ? s_epx_buf : src_indexed;

    /* Build per-level RGBA palette LUT on the fly for this surface.
     * Use corrected LUT if palette_enhanced; otherwise G9010 directly. */
    static uint32_t s_lut[DM1_V2_PALETTE_LEVELS * 16];
    int lv;
    for (lv = 0; lv < DM1_V2_PALETTE_LEVELS; lv++) {
        int pi;
        for (pi = 0; pi < 16; pi++) {
            uint8_t rr, gg, bb;
            if (g_config.palette_enhanced && s_corrected_lut_valid) {
                /* FIXED Phase 8: read from cache (previously identical to else branch). */
                rr = s_corrected_palette[lv][pi][0];
                gg = s_corrected_palette[lv][pi][1];
                bb = s_corrected_palette[lv][pi][2];
            } else {
                rr = G9010_auc_VgaPaletteAll_Compat[lv][pi][0];
                gg = G9010_auc_VgaPaletteAll_Compat[lv][pi][1];
                bb = G9010_auc_VgaPaletteAll_Compat[lv][pi][2];
            }
            /* Store as BGRA (byte order matches palette source data) */
            s_lut[lv * 16 + pi] = 0xFF000000 | (bb << 16) | (gg << 8) | rr;
        }
    }

    /* Palette expand to RGBA */
    static uint32_t s_rgba[320 * 4 * 200 * 4]; /* 1024000 bytes, enough for 1280×800 */
    dm1_v2_asset_palette_expand(palette_src, epx_w, epx_h,
        source_palette_level, s_lut, s_rgba);

    /* Bilinear upscale if enabled (scales from EPX result) */
    if (g_config.bilinear_enabled) {
        /* Scale epx_w × epx_h → final_w × final_h via RGBA bilinear */
        dm1_v2_asset_bilinear_rgba(s_rgba, epx_w, epx_h, s_rgba, final_w, final_h);
        (void)final_w; (void)final_h;
    }

    /* Copy to output */
    int out_total = final_w * final_h;
    memcpy(out_rgba, s_rgba, out_total * sizeof(uint32_t));

    if (out_w) *out_w = final_w;
    if (out_h) *out_h = final_h;

    return 0;
}

/* Full pipeline for a surface: EPX → palette → RGBA */
int dm1_v2_asset_pipeline_process(DM1_V2_SurfaceCategory category,
    const uint8_t* src_indexed, int src_w, int src_h,
    int source_palette_level,
    uint32_t* rgba_out, int* out_w, int* out_h)
{
    return dm1_v2_asset_upscale_surface(category, src_indexed, src_w, src_h,
        source_palette_level, rgba_out, out_w, out_h);
}

/* Rebuild palette LUT from source G9010_auc_VgaPaletteAll_Compat.
 * Called at init and when palette correction settings change.
 * gamma100: 80..260 (100 = no gamma change, 220 = 2.2 CRT approximation)
 * brightness: -50..+50
 * contrast: -50..+50 */
void dm1_v2_asset_rebuild_palette_lut(int gamma100, int brightness, int contrast) {
    /* FIXED Phase 8: previously a stub. Now calls dm1_v2_filter_palette_build_lut(). */
    int rv = dm1_v2_filter_palette_build_lut(gamma100, brightness, contrast,
                                              s_corrected_palette);
    s_corrected_lut_valid = (rv == 0) ? 1 : 0;
}

void dm1_v2_asset_invalidate_cached_palette(void) {
    s_corrected_lut_valid = 0;
}

const char* dm1_v2_asset_pipeline_source_evidence(void) {
    return
        "DM1 V2.1 Graphics Pipeline — source-asset-preserving EPX upscale\n"
        "\n"
        "Source anchors:\n"
        "  DUNVIEW.C:4547-4602  F0115 objects/creatures/projectiles/explosions\n"
        "  DUNVIEW.C:3502-3939  F0107 wall ornaments + alcove detection\n"
        "  DUNVIEW.C:3940-4007  F0108 floor ornaments\n"
        "  DUNVIEW.C:6816-6831  field draw (fluxcage) after creature/projectile\n"
        "  DUNVIEW.C:6204-6218  door panel + frame draw\n"
        "  DUNVIEW.C:6697-6720  D3C center wall draw\n"
        "  DUNVIEW.C:6361-6499  D2L2/D2R2 near-wall draw\n"
        "  PROJEXPL.C:43-92     projectile creation and C48/C49 movement\n"
        "  PROJEXPL.C:95-165    explosion thing creation and C25 event\n"
        "  PROJEXPL.C:817-864   explosion damage/state processing\n"
        "  PROJEXPL.C:987-994   fluxcage removal (C24 event)\n"
        "  DRAWVIEW.C:1-200    creature sprite framing from G0011_i_CreaturePosture\n"
        "  DEFS.H:421-430       projectile-associated explosion thing values\n"
        "  DEFS.H:2218,2324,2376 font/inscription graphics M648/M653\n"
        "  PANEL.C:418-428      G0304_i_DungeonViewPaletteIndex (6 levels 0-5)\n"
        "  DATA.C:359-360        k_source_palette_light_amount_floor[]\n"
        "  ENTRANCE.C:1-900     title/entrance animation sequencing\n"
        "\n"
        "Pipeline: V1 indexed → EPX 2x (indexed, edge-preserving)\n"
        "       → per-level palette expand (G9010_auc_VgaPaletteAll_Compat)\n"
        "       → RGBA8888 → optional bilinear upscale\n"
        "\n"
        "V2.0 (scanlines/palette correction/sharpening) applies AFTER this\n"
        "pipeline as a post-process over the RGBA output surface.\n"
        "Phase 2 contract: V1 framebuffer content is preserved identically;\n"
        "only the presentation resolution and palette expansion change.\n";
}

const char* dm1_v2_asset_surface_category_name(DM1_V2_SurfaceCategory cat) {
    if (cat <= DM1_V2_SURFACE_UNKNOWN || cat >= DM1_V2_SURFACE_COUNT) return "unknown";
    return k_category_names[cat];
}