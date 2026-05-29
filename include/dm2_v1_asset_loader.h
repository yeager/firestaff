#ifndef FIRESTAFF_DM2_V1_ASSET_LOADER_H
#define FIRESTAFF_DM2_V1_ASSET_LOADER_H
/*
 * dm2_v1_asset_loader.h — DM2 V1 Graphics Asset Loader
 *
 * DM2 Phase 2: DM2-specific GRAPHICS.DAT loading.
 *
 * DM2 extends the GRAPHICS.DAT format from DM1:
 *   - 240 GDAT categories (vs DM1's 29)
 *   - ~8.6 MB (vs DM1's ~363 KB) — 24x larger
 *   - New outdoor environment art (sky, ground, trees, buildings)
 *   - Extended creature AI (64 vs 42 types) with new sprites
 *   - DM2-specific UI (champion sheets, shops, compass, depth counter)
 *   - Day/night palette variants with runtime palette switching
 *   - 640x400 mode for outdoor levels (not in DM1)
 *
 * DM2 still uses:
 *   - IMG3 (4-bit nibble encoding) for simple textures
 *   - IMG3 overlay for doors/panels
 *   - IMG9 (9-bit per-pixel) for complex walls
 *   - 8-bit palette-indexed core (c_pixel256) with 16-bit overlay passes
 *
 * Source: docs/dm2_v1_phase2_data_formats_H2254.md §3 — GRAPHICS.DAT structure
 * Source: docs/dm2_graphics.md — GDAT categories, image formats, palette system
 * Source: docs/dm2_platform_data.md — DM2 GRAPHICS.DAT size (~8.6 MB)
 * Source: SKULL.ASM T560 — dungeon viewport rendering (indoor)
 * Source: SKULL.ASM T600 — outdoor viewport rendering (sky/terrain)
 * Source: SKULL.ASM — GDAT image decoding (decode_img3_underlay/overlay, decode_img9)
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── GDAT Category Constants (DM2-extended) ─────────────────────── */
/*
 * DM2 extends categories from 29 (0x1D) to 240 (0xF0).
 * Source: SkGlobal.h:636 — GDAT_CATEGORY_LIMIT (DM2) vs (DM1)
 * Source: docs/dm2_v1_phase2_data_formats_H2254.md §1
 */
#define DM2_GDAT_CATEGORY_LIMIT   0xF0   /* 240 categories */
#define DM1_GDAT_CATEGORY_LIMIT   0x1D   /* 29 categories */

typedef enum {
    DM2_GDAT_CATEGORY_SPELL_DEF          = 0x02, /* Custom spells (up to 255 vs DM1's 34) */
    DM2_GDAT_CATEGORY_CREATURES          = 0x0D, /* 64 AI creature types */
    DM2_GDAT_CATEGORY_DOORS              = 0x0E, /* Door properties (strength, color keys) */
    DM2_GDAT_CATEGORY_WEAPONS            = 0x10, /* Extended weapons (projectile flags) */
    DM2_GDAT_CATEGORY_CHAMPIONS          = 0x16, /* Champion NPC data (sounds) */
    DM2_GDAT_CATEGORY_ENVIRONMENT        = 0x17, /* Outdoor assets (sky, ground, trees) */
    DM2_GDAT_CATEGORY_TELEPORTERS        = 0x18, /* Teleporter square type */
    DM2_GDAT_CATEGORY_CREATURE_AI        = 0x19, /* Per-creature AI behaviors */
    DM2_GDAT_CATEGORY_DIALOG_BOXES       = 0x1A, /* Dialog box graphics */
    DM2_GDAT_CATEGORY_WALL_ORNAMENTS     = 0x0C, /* Wall decorations */
    DM2_GDAT_CATEGORY_FLOOR_ORNAMENTS    = 0x0D, /* Floor decorations */
    DM2_GDAT_CATEGORY_ITEMS              = 0x0F, /* Item sprites */
    DM2_GDAT_CATEGORY_ARMOUR             = 0x11, /* Clothing/armor sprites */
    DM2_GDAT_CATEGORY_INTERFACE          = 0x12, /* Champion sheets, HUD */
    DM2_GDAT_CATEGORY_TEXT               = 0x00, /* Text fonts */
} DM2_GDAT_Category;

/* ── Image Compression Types ─────────────────────────────────────── */
typedef enum {
    DM2_IMG_FMT_UNKNOWN     = 0,
    DM2_IMG_FMT_IMG3        = 3,   /* 4-bit nibble encoding (simple textures) */
    DM2_IMG_FMT_IMG9        = 9,   /* 9-bit per-pixel (complex walls) */
} DM2_ImageFormat;

/* ── Asset Loader Context ─────────────────────────────────────────── */

typedef struct {
    const uint8_t *data;     /* GRAPHICS.DAT data (owned or ref) */
    size_t         data_size; /* GRAPHICS.DAT size */
    int            category_count;
    int            loaded;    /* 1 if successfully loaded */
    uint32_t       md5_hash;  /* Low 32 bits of MD5 (for verification) */
} DM2_V1_AssetLoader;

/* ── Public API ─────────────────────────────────────────────────── */

/* Initialize asset loader with GRAPHICS.DAT data.
 * Returns 0 on success, -1 on failure.
 * Source: docs/dm2_graphics.md — GDAT file structure */
int dm2_v1_asset_loader_init(DM2_V1_AssetLoader *loader,
                              const uint8_t *data, size_t size);

/* Load asset by (category, index, field) triple.
 * Returns raw asset data pointer (NULL on failure).
 * Source: SkGlobal.h — c_gdatfile class, GDAT2 field codes */
const uint8_t *dm2_v1_asset_load(const DM2_V1_AssetLoader *loader,
                                   int category, int index, int field);

/* Load image asset and decode to pixel buffer.
 * out_width, out_height set dimensions, out_format sets format.
 * Caller owns returned buffer (must free with dm2_v1_asset_free_pixels).
 * Source: SKULL.ASM T560 — viewport rendering
 * Source: ReDMCSB DUNGEON.C:1371-1421 — wall frame table */
uint8_t *dm2_v1_asset_load_image(const DM2_V1_AssetLoader *loader,
                                  int category, int index,
                                  int *out_width, int *out_height,
                                  DM2_ImageFormat *out_format);

/* Get GDAT category count and index range for a category.
 * Returns number of entries in category.
 * Source: SkGlobal.h:636 — GDAT_CATEGORY_LIMIT */
int dm2_v1_asset_category_entry_count(const DM2_V1_AssetLoader *loader,
                                       int category);

/* Free pixel buffer allocated by dm2_v1_asset_load_image. */
void dm2_v1_asset_free_pixels(uint8_t *pixels);

/* Get GDAT2 field code description.
 * Source: SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt */
const char *dm2_v1_asset_gdat2_field_name(int field_code);

/* Check if DM2 GRAPHICS.DAT is verified (MD5 matches known good hash).
 * DM2 PC English: 25247ede4dabb6a71e5dabdfbcd5907d (~8.6 MB)
 * Source: docs/dm2_platform_data.md */
int dm2_v1_asset_loader_verify(const DM2_V1_AssetLoader *loader);

/* Free asset loader and all owned resources. */
void dm2_v1_asset_loader_free(DM2_V1_AssetLoader *loader);

/* Source-lock citation string. */
const char *dm2_v1_asset_loader_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ASSET_LOADER_H */