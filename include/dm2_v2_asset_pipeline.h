/*
 * dm2_v2_asset_pipeline.h — DM2 V2.1 Graphics Pipeline
 *
 * Phase 2: Enhanced asset pipeline for Dungeon Master II: Skullkeep.
 *
 * DM2 differs from DM1/CSB in its asset structure: rooms are not
 * corridor dungeons, wall set indices are DM2-specific (G3060 variant
 * rather than G0700), and outdoor mode introduces sky/terrain surfaces.
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
 *   SKULLWIN CSB.cpp CSBData::Initialize — boot/asset pattern
 *
 * DM2 V2.2 modern assets are optional drop-in replacements installed to:
 *   ~/.firestaff/assets/dm2/modern/
 *
 * Each pack contains:
 *   modern_asset_manifest.json  -- catalog (category, id, w, h)
 *   shapes/                      -- wall, floor, creature, object shapes
 *   ui_chrome/                   -- panel chrome and HUD textures
 *   outdoor/                     -- sky, terrain, building textures
 *
 * Asset-not-found guard: if a modern asset cannot be opened, the
 * renderer silently falls back to the V2.1 EPX pipeline.
 *
 * Pipeline contract: V1 framebuffer content preserved identically.
 * Only presentation resolution and palette expansion change.
 */

#ifndef FIRESTAFF_DM2_V2_ASSET_PIPELINE_H
#define FIRESTAFF_DM2_V2_ASSET_PIPELINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =================================================================
 * DM2 V2 Surface Categories
 *
 * Covers all DM2 surface types: dungeon walls/doors/floors/ornaments,
 * creatures, objects, projectiles, UI chrome, outdoor surfaces.
 * Categories mirror the DM1/CSB V2 pipeline categories with the
 * addition of DM2-specific outdoor and UI surfaces.
 * ================================================================= */

typedef enum {
    DM2_V2_SURFACE_UNKNOWN = 0,

    /* -- Dungeon viewport surfaces (320x200 indexed) -------------- */

    DM2_V2_SURFACE_WALL_BACK,          /* D3L2/D3R2 back wall -- F0107 frame routing */
    DM2_V2_SURFACE_WALL_NEAR,          /* D2L2/D2R2/D2C near wall -- F0107/G3060 variant */
    DM2_V2_SURFACE_WALL_CENTER,        /* D3C center wall -- G3060_i_WallSet_Wall_D3C */
    DM2_V2_SURFACE_DOOR,               /* Door panel + frame -- F0110/F0111 DrawDoor */
    DM2_V2_SURFACE_FLOOR_ORNAMENT,     /* Floor ornament -- F0108_DrawFloorOrnament */
    DM2_V2_SURFACE_WALL_ORNAMENT,      /* Wall ornament / alcove -- F0109 DrawDoorOrnament */
    DM2_V2_SURFACE_CREATURE,           /* Creature sprite -- SKULL.ASM creature render */
    DM2_V2_SURFACE_OBJECT,             /* Dungeon object -- F0115 object pass */
    DM2_V2_SURFACE_PROJECTILE,         /* Projectile sprite */
    DM2_V2_SURFACE_EXPLOSION,          /* Explosion sprite */
    DM2_V2_SURFACE_FLUXCAGE,           /* Fluxcage field */

    /* -- UI chrome surfaces ---------------------------------------- */

    DM2_V2_SURFACE_FONT,               /* Font / inscription -- M648/M653 */
    DM2_V2_SURFACE_PANEL_CHAMPION,     /* Champion portrait/panel area */
    DM2_V2_SURFACE_PANEL_MESSAGE,      /* Message log area */
    DM2_V2_SURFACE_PANEL_STATS,        /* Champion stats panel (HP/MP/condition) */
    DM2_V2_SURFACE_PANEL_SPELL,        /* Spell casting panel */
    DM2_V2_SURFACE_ACTION_STRIP,       /* Action strip (Attack/Cast/Use/Drop/Move) */
    DM2_V2_SURFACE_GOLD_COUNTER,       /* Gold counter (DM2-specific) */
    DM2_V2_SURFACE_TOP_STATUS_BAR,     /* Top status bar (DM2-specific, no portraits) */

    /* -- Title / entrance surfaces -------------------------------- */

    DM2_V2_SURFACE_TITLE,              /* Title screen -- SKULL.ASM F0806 */
    DM2_V2_SURFACE_ENTRANCE,           /* Entrance animation */
    DM2_V2_SURFACE_CREDITS,            /* Credits screen */
    DM2_V2_SURFACE_IMPORT,             /* Champion import screen (DM2 floppy) */

    /* -- Outdoor surfaces ----------------------------------------- */

    DM2_V2_SURFACE_SKY_GRADIENT,       /* Sky gradient -- T600 outdoor */
    DM2_V2_SURFACE_TERRAIN,            /* Terrain/floor outdoor */
    DM2_V2_SURFACE_BUILDING,           /* Building exterior (outdoor) */
    DM2_V2_SURFACE_WEATHER_OVERLAY,    /* Rain/snow overlay */

    /* -- DM2-specific surfaces ------------------------------------ */

    DM2_V2_SURFACE_ROOM_WALL,          /* Room wall (DM2 rooms vs corridors) */
    DM2_V2_SURFACE_ROOM_FLOOR,        /* Room floor */
    DM2_V2_SURFACE_ROOM_CEILING,       /* Room ceiling */
    DM2_V2_SURFACE_DUNGEON_FLOOR,     /* Corridor dungeon floor */
    DM2_V2_SURFACE_DUNGEON_CEILING,   /* Corridor dungeon ceiling */

    DM2_V2_SURFACE_COUNT
} DM2_V2_SurfaceCategory;

/* Human-readable name */
const char* dm2_v2_asset_surface_category_name(DM2_V2_SurfaceCategory cat);

/* =================================================================
 * DM2 V2 Graphics Mode
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
    DM2_V2_GFX_MODE_V1_ORIGINAL = 0, /* 320x200 native, pixel-perfect */
    DM2_V2_GFX_MODE_V2_FILTERED  = 1, /* V1 + scanlines + palette correction */
    DM2_V2_GFX_MODE_V2_UPSCALED  = 2, /* EPX 2x (640x400 RGBA) */
    DM2_V2_GFX_MODE_V2_MODERN    = 3  /* Modern 1920x1080 drop-in assets */
} DM2_V2_GfxMode;

#define DM2_V2_GFX_MODE_DEFAULT  DM2_V2_GFX_MODE_V1_ORIGINAL

/* Get / set the current graphics mode */
DM2_V2_GfxMode dm2_v2_asset_get_gfx_mode(void);
void           dm2_v2_asset_set_gfx_mode(DM2_V2_GfxMode mode);

/* Convenience predicates */
int dm2_v2_asset_is_v2_mode(void);         /* V2.0, V2.1, or V2.2 active */
int dm2_v2_asset_is_v2_filtered(void);     /* V2.0 FILTERED or higher */
int dm2_v2_asset_is_v2_upscaled(void);     /* V2.1 UPSCALED or higher */
int dm2_v2_asset_is_v2_modern(void);        /* V2.2 MODERN specifically */

/* =================================================================
 * Palette / light-level constants
 *
 * Source: ReDMCSB DATA.C:359-360, consumed by PANEL.C:418-428.
 * 6 dungeon light levels (0=brightest, 5=darkest), matching DM1/CSB.
 * ================================================================= */

#define DM2_V2_PALETTE_LEVELS  6

/* Per-level source light amount floor (ReDMCSB DATA.C:359-360) */
extern const uint8_t DM2_V2_k_SourcePaletteLightFloor[DM2_V2_PALETTE_LEVELS];

/* =================================================================
 * Per-category surface upscale
 *
 * Entry point for the V2.1 EPX pipeline.
 *
 * Input:  V1 indexed pixels at src_w x src_h (typically 320x200)
 *         Each byte encodes (level << 4) | palette_index.
 *
 * Output: RGBA at (epx_enabled ? src_w*2 : src_w) x (epx_enabled ? src_h*2 : src_h).
 *
 * Returns: 0 on success, -1 on null input or invalid category.
 * ================================================================= */

int dm2_v2_asset_pipeline_process(
    DM2_V2_SurfaceCategory category,
    const uint8_t*  src_indexed,
    int             src_w,
    int             src_h,
    int             source_palette_level,  /* 0..5 from source selection */
    uint32_t*       rgba_out,
    int*            out_w,
    int*            out_h);

/* EPX 2x for indexed pixel art */
void dm2_v2_asset_epx_upscale(
    const uint8_t* src, int sw, int sh,
    uint8_t* dst, int dw, int dh);

/* Palette expand: indexed -> RGBA8888 */
void dm2_v2_asset_palette_expand(
    const uint8_t* indexed, int w, int h,
    int palette_level,
    const uint32_t* lut,
    uint32_t* rgba_out);

/* =================================================================
 * Module configuration
 * ================================================================= */

typedef enum {
    DM2_V2_PALETTE_MODE_ORIGINAL = 0,  /* G9010_auc_VgaPaletteAll_Compat, untouched */
    DM2_V2_PALETTE_MODE_ENHANCED        /* Gamma/brightness/contrast corrected */
} DM2_V2_PaletteMode;

typedef enum {
    DM2_V2_SCALE_MODE_NATIVE = 0,  /* V1: 320x200, no upscale */
    DM2_V2_SCALE_MODE_EPX_2X,       /* V2.1: EPX 2x -> 640x400 */
    DM2_V2_SCALE_MODE_BILINEAR_2X    /* V2.1: EPX -> bilinear -> 1280x800 */
} DM2_V2_ScaleMode;

typedef struct {
    DM2_V2_ScaleMode   scale_mode;
    DM2_V2_PaletteMode palette_mode;
    int                epx_enabled;        /* 0=off, 1=EPX 2x */
    int                bilinear_enabled;   /* 0=off, 1=bilinear upscale */
    int                palette_enhanced;   /* 0=original, 1=gamma-corrected LUT */
    int                scanlines_enabled;  /* 0=off, 1=CRT scanline overlay */
    int                sharpen_enabled;    /* 0=off, 1=unsharp mask post */
    int                source_light_floor; /* 0..99 brightness floor */
} DM2_V2_AssetPipelineConfig;

/* Get / set pipeline config */
void                               dm2_v2_asset_pipeline_configure(const DM2_V2_AssetPipelineConfig* config);
const DM2_V2_AssetPipelineConfig*   dm2_v2_asset_pipeline_get_config(void);

/* (Re)build corrected palette LUT */
int  dm2_v2_asset_rebuild_palette_lut(int gamma100, int brightness, int contrast);
void dm2_v2_asset_invalidate_cached_palette(void);

/* Module init */
void dm2_v2_asset_pipeline_init(void);

/* =================================================================
 * V2.2 Modern Asset Mode
 *
 * Modern assets installed to: ~/.firestaff/assets/dm2/modern/
 * ================================================================= */

/* Scan for modern asset manifest.
 * Returns 1 if manifest found, 0 if absent (falls back to EPX pipeline). */
int dm2_v2_asset_load_modern_asset_manifest(void);

/* Returns modern asset root, or "". */
void dm2_v2_asset_get_modern_asset_root(char* out, size_t out_size);

/* =================================================================
 * Shape source selection
 * ================================================================= */

typedef enum {
    DM2_V22_SHAPE_SOURCE_V1_ORIGINAL = 0,
    DM2_V22_SHAPE_SOURCE_V2_FILTERED,
    DM2_V22_SHAPE_SOURCE_V2_UPSCALED,
    DM2_V22_SHAPE_SOURCE_V2_MODERN,
    DM2_V22_SHAPE_SOURCE_COUNT
} DM2_V22_ShapeSource;

DM2_V22_ShapeSource dm2_v2_best_available_shape_source(int presentation_mode_index);
const char*         dm2_v2_shape_source_name(DM2_V22_ShapeSource src);

/* Source evidence string for documentation */
const char* dm2_v2_asset_pipeline_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_ASSET_PIPELINE_H */