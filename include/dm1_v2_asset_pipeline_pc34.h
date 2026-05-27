#ifndef FIRESTAFF_DM1_V2_ASSET_PIPELINE_PC34_H
#define FIRESTAFF_DM1_V2_ASSET_PIPELINE_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════════
 * DM1 V2.1 Graphics Pipeline — Source-Asset-Preserving Upscale Path
 *
 * Phase 2 of DM1 V2: source-preserving EPX upscale for all indexed
 * V1 asset surfaces.
 *
 * Source-lock anchors:
 * - ReDMCSB DUNVIEW.C F0115:4547-4602 draws objects/creatures/projectiles
 *   after walls/doors/floors; F0107:3502-3939 draws wall ornaments;
 *   F0108:3940-4007 draws floor ornaments.
 * - ReDMCSB FONT.C / DEFS.H:2218,2324,2376: font/inscription graphics
 *   use GRAPHICS.DAT M648/M653 bitmap indices at 6-wide char cells.
 * - ReDMCSB PANEL.C:418-428 selects G0304_i_DungeonViewPaletteIndex
 *   (6 levels: 0=brightest, 5=darkest) from torch charges + magic.
 * - ReDMCSB ENTRANCE.C:1-900 owns title/entrance animation sequencing
 *   and GRAPHICS.DAT surface composition.
 * - ReDMCSB DRAWVIEW.C:1-200: creature sprite framing and animation
 *   state machine from G0011_i_CreaturePosture.
 * - ReDMCSB PROJEXPL.C:43-165 owns projectile/explosion sprite routing.
 *
 * Pipeline contract:
 *   V1 indexed pixel (4-bit level | 4-bit palette index)
 *     → per-category surface blit (walls, creatures, objects, projectiles,
 *       fonts, title/entrance) — all at original 320×200 viewport coords
 *     → EPX 2x (indexed, edge-preserving on pixel-art)
 *     → palette expand to RGBA (G9010_auc_VgaPaletteAll_Compat per level)
 *     → optional bilinear to target resolution
 *     → presentation surface
 *
 * All presentation work is V2-only (behind DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION).
 * No V1 gameplay state, timing, or collision is affected.
 * ══════════════════════════════════════════════════════════════════════ */

#include <stdint.h>

/* ── Asset surface categories ─────────────────────────────────────── */

/* Per-category surface IDs for V2.1 upscaled presentation.
 * These are surface labels, not GRAPHICS.DAT indices — the renderer
 * uses them to select the correct source surface and upscale path. */
typedef enum {
    DM1_V2_SURFACE_UNKNOWN = 0,
    DM1_V2_SURFACE_WALL_BACK,        /* D3L2/D3R2 back wall (F0107 frame routing) */
    DM1_V2_SURFACE_WALL_NEAR,         /* D2L2/D2R2/D2C near wall (F0107/F0678/F0679) */
    DM1_V2_SURFACE_WALL_CENTER,       /* D3C center wall */
    DM1_V2_SURFACE_DOOR,              /* Door panel + frame (F0111) */
    DM1_V2_SURFACE_FLOOR_ORNAMENT,   /* Floor ornament (F0108) */
    DM1_V2_SURFACE_WALL_ORNAMENT,     /* Wall ornament / alcove (F0107) */
    DM1_V2_SURFACE_CREATURE,          /* Creature sprite (DRAWVIEW.C framing) */
    DM1_V2_SURFACE_OBJECT,            /* Dungeon object (F0115 object pass) */
    DM1_V2_SURFACE_PROJECTILE,       /* Projectile sprite (PROJEXPL.C) */
    DM1_V2_SURFACE_EXPLOSION,         /* Explosion sprite (PROJEXPL.C:95-165) */
    DM1_V2_SURFACE_FLUXCAGE,          /* Fluxcage field (PROJEXPL.C:987-994) */
    DM1_V2_SURFACE_FONT,              /* Font / inscription (M648/M653) */
    DM1_V2_SURFACE_PANEL_CHAMPION,    /* Champion portrait/panel area */
    DM1_V2_SURFACE_PANEL_MESSAGE,     /* Message log area */
    DM1_V2_SURFACE_TITLE,             /* Title screen surface (ENTRANCE.C) */
    DM1_V2_SURFACE_ENTRANCE,          /* Entrance animation surface */
    DM1_V2_SURFACE_COUNT
} DM1_V2_SurfaceCategory;

/* ── Palette / light-level constants ──────────────────────────────── */

/* Source: ReDMCSB DATA.C:359-360, consumed by PANEL.C:418-428.
 * 6 dungeon light levels; index 0 = brightest, index 5 = darkest.
 * V2.1 uses the source-selected palette index verbatim for gameplay
 * lighting, then applies optional V2-only presentation enhancement. */
#define DM1_V2_PALETTE_LEVELS 6

/* ── Upscale configuration ────────────────────────────────────────── */

typedef enum {
    DM1_V2_SCALE_MODE_NATIVE,   /* V1: 320×200, no upscale */
    DM1_V2_SCALE_MODE_EPX_2X,   /* V2.1: EPX 2x → 640×400 (default) */
    DM1_V2_SCALE_MODE_EPX_4X,   /* V2.1: EPX 2x → bilinear → target (e.g. 1280×800) */
} DM1_V2_ScaleMode;

typedef enum {
    DM1_V2_PALETTE_MODE_ORIGINAL,   /* G9010_auc_VgaPaletteAll_Compat, no correction */
    DM1_V2_PALETTE_MODE_ENHANCED,   /* Gamma/brightness/contrast corrected LUT */
} DM1_V2_PaletteMode;

typedef struct {
    DM1_V2_ScaleMode scale_mode;
    DM1_V2_PaletteMode palette_mode;
    int epx_enabled;           /* 0=disabled, 1=EPX 2x (default) */
    int bilinear_enabled;       /* 0=disabled, 1=bilinear upscale from EPX result */
    int palette_enhanced;      /* 0=original, 1=corrected LUT */
    int sharpen_enabled;       /* 0=disabled, 1=post-upscale unsharp mask */
    int source_light_floor;    /* 0..99: brightness floor from source palette level */
} DM1_V2_AssetPipelineConfig;

/* ── Pipeline state ────────────────────────────────────────────────── */

typedef struct {
    /* Indexed V1 source surface (320×200, or per-surface crop) */
    const uint8_t* src_indexed;
    int src_w;
    int src_h;

    /* EPX 2x indexed output (640×400 at 2x, 1280×800 at 4x) */
    uint8_t epx_buffer[320 * 2 * 200 * 2]; /* 256000 bytes — enough for 640×400 */
    int epx_w;
    int epx_h;

    /* Final RGBA presentation surface */
    uint32_t rgba_output[320 * 4 * 200 * 4]; /* 1024000 bytes — 1280×800 RGBA */
    int out_w;
    int out_h;

    /* Palette LUT: [6-levels][16-palette-entries][3-RGB] */
    uint32_t palette_lut[DM1_V2_PALETTE_LEVELS][16];
    int palette_lut_valid;

    /* Category metadata */
    DM1_V2_SurfaceCategory category;
    int source_palette_level;   /* 0..5 from source selection */
    const char* source_anchor;  /* e.g. "DUNVIEW.C:4547-4602" */
} DM1_V2_AssetPipelineState;

/* ── API ──────────────────────────────────────────────────────────── */

void dm1_v2_asset_pipeline_init(void);
void dm1_v2_asset_pipeline_configure(const DM1_V2_AssetPipelineConfig* config);
const DM1_V2_AssetPipelineConfig* dm1_v2_asset_pipeline_get_config(void);

/* Per-category surface upscale.
 * V2.1: all surfaces go through the same EPX 2x → palette → RGBA pipeline.
 * The category argument selects the source surface and provides metadata
 * for the viewport compositor. */
int dm1_v2_asset_upscale_surface(DM1_V2_SurfaceCategory category,
    const uint8_t* src_indexed, int src_w, int src_h,
    int source_palette_level,
    uint32_t* out_rgba, int* out_w, int* out_h);

/* EPX 2x for indexed pixel art.
 * Core of the V2.1 upscaler: preserves sharp edges in indexed sprites. */
void dm1_v2_asset_epx_upscale(const uint8_t* src, int sw, int sh,
    uint8_t* dst, int dw, int dh);

/* Palette expand indexed → RGBA using per-level LUT.
 * V2.1 uses G9010_auc_VgaPaletteAll_Compat as the base palette,
 * with optional gamma/brightness correction when palette_enhanced is set. */
void dm1_v2_asset_palette_expand(const uint8_t* indexed,
    int w, int h, int palette_level,
    const uint32_t* lut, uint32_t* rgba_out);

/* Full V2.1 pipeline for a surface: EPX → palette → RGBA */
int dm1_v2_asset_pipeline_process(DM1_V2_SurfaceCategory category,
    const uint8_t* src_indexed, int src_w, int src_h,
    int source_palette_level,
    uint32_t* rgba_out, int* out_w, int* out_h);

/* Palette LUT (re)build from source G9010_auc_VgaPaletteAll_Compat */
void dm1_v2_asset_rebuild_palette_lut(int gamma100, int brightness, int contrast);

/* ── Source evidence ──────────────────────────────────────────────── */

const char* dm1_v2_asset_pipeline_source_evidence(void);
const char* dm1_v2_asset_surface_category_name(DM1_V2_SurfaceCategory cat);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_ASSET_PIPELINE_PC34_H */