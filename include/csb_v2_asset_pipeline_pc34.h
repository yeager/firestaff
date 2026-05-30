/*
 * csb_v2_asset_pipeline_pc34.h — CSB V2.1 Graphics Pipeline
 *
 * Enhanced asset pipeline: loads enhanced graphics (V2.0/V2.1 mode assets)
 * rather than original V1 assets. Asset selection is based on the current
 * graphics mode (FILTERED=2.0, UPSCALED=2.1, MODERN=2.2) with transparent
 * fallback to V1 assets when enhanced assets are unavailable.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   DUNGEON.C:1-900         CSB dungeon surface dispatch and square type routing
 *   DUNVIEW.C:4547-4602     F0115 draw objects/creatures/projectiles
 *   DUNVIEW.C:3502-3939     F0107 wall ornaments / alcove detection (CSB D3L2/D3R2)
 *   DUNGEON.C:360-430       CSB wall sets and floor ornament selection
 *   PROJEXPL.C:43-165       projectile/explosion sprite creation and routing
 *   PROJEXPL.C:817-864       explosion damage/state
 *   DRAWVIEW.C:1-200        creature sprite framing from G0011_i_CreaturePosture
 *   PANEL.C:418-428         G0304_i_DungeonViewPaletteIndex (6 levels)
 *   FONT.C / DEFS.H:2218    CSB font/inscription graphics (M648/M653)
 *   ENTRANCE.C:1-900        CSB entrance/title animation sequencing
 *
 * Pipeline invariant: every surface goes through
 *   V1 indexed (level|index) -> EPX 2x -> per-level palette -> RGBA -> optional post
 * No gameplay state, collision, or timing is modified.
 *
 * Runtime dependency:
 *   Requires csb_v2_phase_gate_pc34.o to be linked.
 *   All functions are gated behind CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION.
 *
 * Fallback chain:
 *   V2.2 MODERN -> V2.1 UPSCALED -> V2.0 FILTERED -> V1 ORIGINAL
 */

#ifndef FIRESTAFF_CSB_V2_ASSET_PIPELINE_PC34_H
#define FIRESTAFF_CSB_V2_ASSET_PIPELINE_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =================================================================
 * CSB V2 Surface Categories
 * ================================================================= */

typedef enum {
    CSB_V2_SURFACE_UNKNOWN = 0,

    /* -- Dungeon viewport surfaces (320x200 indexed) -------------- */

    CSB_V2_SURFACE_WALL_BACK,          /* D3L2/D3R2 back wall -- F0107 frame routing */
    CSB_V2_SURFACE_WALL_NEAR,          /* D2L2/D2R2/D2C near wall -- F0107/F0678/F0679 */
    CSB_V2_SURFACE_WALL_CENTER,        /* D3C center wall -- G0700_puc_Bitmap_WallSet */
    CSB_V2_SURFACE_DOOR,               /* Door panel + frame -- G0074 F0132 blit */
    CSB_V2_SURFACE_FLOOR_ORNAMENT,     /* Floor ornament -- F0108_DrawFloorOrnament */
    CSB_V2_SURFACE_WALL_ORNAMENT,       /* Wall ornament / alcove -- F0107 */
    CSB_V2_SURFACE_CREATURE,            /* Creature sprite -- DRAWVIEW.C framing */
    CSB_V2_SURFACE_OBJECT,              /* Dungeon object -- F0115 object pass */
    CSB_V2_SURFACE_PROJECTILE,          /* Projectile sprite -- PROJEXPL.C:43-92 */
    CSB_V2_SURFACE_EXPLOSION,          /* Explosion sprite -- PROJEXPL.C:95-165 */
    CSB_V2_SURFACE_FLUXCAGE,           /* Fluxcage field -- PROJEXPL.C:987-994 */

    /* -- UI chrome surfaces ---------------------------------------- */

    CSB_V2_SURFACE_FONT,               /* Font / inscription -- DEFS.H:2218 M648/M653 */
    CSB_V2_SURFACE_PANEL_CHAMPION,     /* Champion portrait/panel area */
    CSB_V2_SURFACE_PANEL_MESSAGE,      /* Message log area -- DRAWMSGA.C */
    CSB_V2_SURFACE_PANEL_STATS,        /* Champion stats panel (CSB-specific) */
    CSB_V2_SURFACE_PANEL_SPELL,        /* Spell casting panel (CSB-specific) */

    /* -- Title / entrance surfaces -------------------------------- */

    CSB_V2_SURFACE_TITLE,             /* Title screen -- ENTRANCE.C F0806 */
    CSB_V2_SURFACE_ENTRANCE,          /* Entrance animation -- ENTRANCE.C:900-1800 */
    CSB_V2_SURFACE_CREDITS,           /* CSB credits screen */

    /* -- CSB-specific surfaces ------------------------------------- */

    CSB_V2_SURFACE_CHAOS_OVERLAY,      /* Chaos magic overlay effects */
    CSB_V2_SURFACE_PARTY_VIEW,          /* Party overview panel (CSB roster) */
    CSB_V2_SURFACE_DUNGEON_FLOOR,     /* Dungeon floor bitmap */
    CSB_V2_SURFACE_DUNGEON_CEILING,   /* Dungeon ceiling bitmap */
    CSB_V2_SURFACE_CHAMPION_IMPORT,    /* Champion import wizard (CSB floppy) */
    CSB_V2_SURFACE_COUNT
} CSB_V2_SurfaceCategory;

/* Human-readable names for surface categories */
const char* csb_v2_asset_surface_category_name(CSB_V2_SurfaceCategory cat);

/* =================================================================
 * CSB V2 Graphics Mode
 *
 * Graphics mode determines which art pipeline feeds the renderer:
 *   V1 ORIGINAL  -- raw V1 indexed surfaces at 320x200 (no upscaling)
 *   V2.0 FILTERED -- V1 + CRT scanlines + palette correction post-process
 *   V2.1 UPSCALED -- EPX 2x from V1 indexed -> RGBA 640x400
 *   V2.2 MODERN   -- hand-crafted modern assets at target resolution
 *
 * Asset selection order (per surface category):
 *   1. MODERN (V2.2) if gfx_mode == V2.2 MODERN and modern asset found
 *   2. UPSCALED (V2.1) if gfx_mode >= V2.1 UPSCALED -- EPX pipeline
 *   3. FILTERED (V2.0) if gfx_mode >= V2.0 FILTERED -- V1 + post-process
 *   4. ORIGINAL (V1)   -- always available as final fallback
 * ================================================================= */

typedef enum {
    CSB_V2_GFX_MODE_V1_ORIGINAL = 0, /* 320x200 native, pixel-perfect */
    CSB_V2_GFX_MODE_V2_FILTERED  = 1, /* V1 + scanlines + palette correction */
    CSB_V2_GFX_MODE_V2_UPSCALED  = 2, /* EPX 2x (640x400 RGBA) */
    CSB_V2_GFX_MODE_V2_MODERN    = 3  /* Modern 1920x1080 drop-in assets */
} CSB_V2_GfxMode;

#define CSB_V2_GFX_MODE_DEFAULT  CSB_V2_GFX_MODE_V1_ORIGINAL

/* Get / set the current graphics mode */
CSB_V2_GfxMode csb_v2_asset_get_gfx_mode(void);
void           csb_v2_asset_set_gfx_mode(CSB_V2_GfxMode mode);

/* Convenience predicates */
int csb_v2_asset_is_v2_mode(void);         /* V2.0, V2.1, or V2.2 active */
int csb_v2_asset_is_v2_filtered(void);     /* V2.0 FILTERED or higher */
int csb_v2_asset_is_v2_upscaled(void);     /* V2.1 UPSCALED or higher */
int csb_v2_asset_is_v2_modern(void);       /* V2.2 MODERN specifically */

/* =================================================================
 * Palette / light-level constants
 *
 * Source: ReDMCSB DATA.C:359-360, consumed by PANEL.C:418-428.
 * 6 dungeon light levels (0=brightest, 5=darkest), matching DM1.
 * ================================================================= */

#define CSB_V2_PALETTE_LEVELS  6

/* Per-level source light amount floor (ReDMCSB DATA.C:359-360).
 * V2 presentation uses this to derive the darkness percentage. */
extern const uint8_t CSB_V2_k_SourcePaletteLightFloor[CSB_V2_PALETTE_LEVELS];

/* =================================================================
 * Per-category surface upscale
 *
 * Entry point for the V2.1 EPX pipeline. All surfaces follow the same
 * pipeline; the category argument enables renderer-side surface
 * routing and metadata logging.
 *
 * Input:  V1 indexed pixels at src_w x src_h (typically 320x200)
 *         Each byte encodes (level << 4) | palette_index.
 *
 * Output: RGBA at (epx_enabled ? src_w*2 : src_w) x (epx_enabled ? src_h*2 : src_h).
 *         When bilinear is enabled, bilateral upscale is applied as well.
 *
 * Returns: 0 on success, -1 on null input or invalid category.
 * ================================================================= */

/* Full V2.1 EPX pipeline for a surface: EPX 2x -> palette -> RGBA */
int csb_v2_asset_pipeline_process(
    CSB_V2_SurfaceCategory category,
    const uint8_t*  src_indexed,
    int             src_w,
    int             src_h,
    int             source_palette_level,  /* 0..5 from source selection */
    uint32_t*       rgba_out,
    int*            out_w,
    int*            out_h);

/* EPX 2x for indexed pixel art.
 * Preserves sharp pixel-art edges without palette interpolation.
 * Input:  sw x sh indexed. Output: (sw x 2) x (sh x 2) indexed. */
void csb_v2_asset_epx_upscale(
    const uint8_t* src,
    int sw, int sh,
    uint8_t* dst,
    int dw, int dh);

/* Palette expand: indexed -> RGBA8888 using per-level G9010 LUT.
 * indexed bytes encode (level << 4) | palette_index.
 * lut must have at least CSB_V2_PALETTE_LEVELS * 16 entries. */
void csb_v2_asset_palette_expand(
    const uint8_t* indexed,
    int w, int h,
    int palette_level,
    const uint32_t* lut,
    uint32_t* rgba_out);

/* =================================================================
 * Module configuration
 *
 * Defaults (V1 compatibility):
 *   epx_enabled   = 0  (no upscale, pass-through)
 *   palette_mode = ORIGINAL (G9010 with no correction)
 *   scanlines_enabled = 0
 *   sharpen_enabled   = 0
 *
 * V2.0 FILTERED: scanlines_enabled=1, palette_mode=ENHANCED
 * V2.1 UPSCALED: epx_enabled=1, scanlines_enabled=0
 * V2.2 MODERN: uses modern asset manifest to resolve assets
 *              before falling back to V2.1 pipeline
 * ================================================================= */

typedef enum {
    CSB_V2_PALETTE_MODE_ORIGINAL = 0,  /* G9010_auc_VgaPaletteAll_Compat, untouched */
    CSB_V2_PALETTE_MODE_ENHANCED        /* Gamma/brightness/contrast corrected */
} CSB_V2_PaletteMode;

typedef enum {
    CSB_V2_SCALE_MODE_NATIVE = 0,  /* V1: 320x200, no upscale */
    CSB_V2_SCALE_MODE_EPX_2X,       /* V2.1: EPX 2x -> 640x400 */
    CSB_V2_SCALE_MODE_BILINEAR_2X   /* V2.1: EPX -> bilinear -> 1280x800 */
} CSB_V2_ScaleMode;

typedef struct {
    CSB_V2_ScaleMode   scale_mode;
    CSB_V2_PaletteMode palette_mode;
    int                epx_enabled;        /* 0=off, 1=EPX 2x */
    int                bilinear_enabled;   /* 0=off, 1=bilinear upscale */
    int                palette_enhanced;   /* 0=original, 1=gamma-corrected LUT */
    int                scanlines_enabled;  /* 0=off, 1=CRT scanline overlay */
    int                sharpen_enabled;    /* 0=off, 1=unsharp mask post */
    int                source_light_floor; /* 0..99 brightness floor */
} CSB_V2_AssetPipelineConfig;

/* Get / set pipeline config */
void                              csb_v2_asset_pipeline_configure(const CSB_V2_AssetPipelineConfig* config);
const CSB_V2_AssetPipelineConfig* csb_v2_asset_pipeline_get_config(void);

/* (Re)build the corrected palette LUT from G9010 source.
 * gamma100: 80..260 (100 = no change, 220 ~ 2.2 CRT approximation)
 * brightness: -50..+50
 * contrast: -50..+50
 * Returns 0 on success, -1 if parameters out of range. */
int csb_v2_asset_rebuild_palette_lut(int gamma100, int brightness, int contrast);

/* Invalidate the corrected palette cache (e.g., on V1 fallback) */
void csb_v2_asset_invalidate_cached_palette(void);

/* Module init -- call once before any pipeline use */
void csb_v2_asset_pipeline_init(void);

/* =================================================================
 * V2.2 Modern Asset Mode
 *
 * Modern assets are optional drop-in replacement packages installed to:
 *   ~/.firestaff/assets/csb/modern/
 *
 * Each pack contains:
 *   modern_asset_manifest.json  -- catalog (category, id, source_file, w, h)
 *   shapes/                      -- wall, floor, creature, object shapes as PNG/TGA
 *   ui_chrome/                   -- panel chrome and HUD textures
 *   champion_portraits/           -- CSB champion portrait textures
 *
 * Asset-not-found guard: if a modern asset cannot be opened,
 * the renderer silently falls back to V2.1 (EPX) pipeline.
 * ================================================================= */

/* Scan for modern asset manifest at:
 *   ~/.firestaff/assets/csb/modern/modern_asset_manifest.json
 *
 * Returns 1 if manifest found and valid, 0 if absent (caller uses
 * the EPX pipeline fallback). Sets the modern asset root internally. */
int csb_v2_asset_load_modern_asset_manifest(void);

/* Returns the modern asset root path, or "" if not available.
 * Copies into caller's buffer -- this is not a persistent string. */
void csb_v2_asset_get_modern_asset_root(char* out, size_t out_size);

/* Shape source enum -- which art pipeline is active for rendering.
 * Used by csb_v2_best_available_shape_source() to select the best
 * available pipeline given the current config and asset state. */
typedef enum {
    CSB_V22_SHAPE_SOURCE_V1_ORIGINAL = 0,  /* 320x200 native V1 */
    CSB_V22_SHAPE_SOURCE_V2_FILTERED,       /* V1 + scanline/palette post */
    CSB_V22_SHAPE_SOURCE_V2_UPSCALED,       /* EPX 2x + palette -> RGBA */
    CSB_V22_SHAPE_SOURCE_V2_MODERN,         /* Modern 1920x1080 assets */
    CSB_V22_SHAPE_SOURCE_COUNT
} CSB_V22_ShapeSource;

/* Returns the best available shape source given the presentation mode
 * index (0=V1, 1=V2.0, 2=V2.1, 3=V2.2) and current asset state.
 * Applies fallback chain: MOD/V2.2->UPSCALED/V2.1->FILTERED/V2.0->ORIGINAL/V1. */
CSB_V22_ShapeSource csb_v2_best_available_shape_source(int presentation_mode_index);

/* Human-readable shape source name */
const char* csb_v2_shape_source_name(CSB_V22_ShapeSource src);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_ASSET_PIPELINE_PC34_H */
