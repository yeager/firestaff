#ifndef NEXUS_V1_PALETTE_H
#define NEXUS_V1_PALETTE_H
#include <stdint.h>

/* Nexus V1 Palette and Texture System
 * ==================================
 * Saturn VDP1 color palette (16-bit BGR555 per entry, 256 entries).
 * Palette is loaded from STONE.BIN (wall/stone base palette) and
 * extracted from TITLE.CG / ITEM.IBS surface files.
 *
 * Source-lock reference:
 *   ReDMCSB PALETTE.C — DM1 palette load/apply
 *   Saturn SDK: VDP1 color RAM (32 KB, 16-bit BGR555)
 *   docs/NEXUS_FILE_CLASSIFICATION.md — STONE.BIN 4 KB, TITLE.CG 164 KB
 *
 * DGN geometry blob encodes per-square palette indices, allowing
 * texture lookup without loading full texture bitmaps.
 *
 * Nexus uses the same 256-color indexed palette as DM1/CSB but with
 * a different master palette optimized for Saturn's color space.
 *
 * Palette slots:
 *   0       = transparency / black
 *   1-15    = dungeon base (stone/wall tones)
 *   16-31   = creature/character colors
 *   32-63   = object/pickup colors
 *   64-127  = UI / HUD colors
 *   128-255 = extended dungeon (ornaments, effects, lava, water) */

/* Number of palette entries and texture slots */
#define NEXUS_PALETTE_SIZE     256
#define NEXUS_MAX_TEXTURES     32

/* Texture atlas entry — a (w × h) indexed bitmap */
typedef struct {
    uint8_t *data;           /* indexed color buffer         */
    int      w, h;           /* dimensions in pixels        */
    int      owns_data;      /* 1=calloc'd, 0=borrowed ref  */
    uint8_t  pal_start;      /* first palette entry used    */
    uint8_t  pal_count;      /* entries assigned to this tx */
    const char *source_file; /* e.g. "STONE.BIN"            */
    const char *label;       /* e.g. "wall_normal"          */
} Nexus_Texture;

/* Master palette — mirrors Saturn VDP1 Color RAM (BGR555 × 256) */
typedef struct {
    /* 16-bit BGR555 entries (Saturn native format)
     * Bit layout: 1R RRRG GGGGB BBBBB (16 bits per entry) */
    uint16_t entries[NEXUS_PALETTE_SIZE];
    /* Pre-expanded RGBA (0xAARRGGBB) for fast framebuffer blit */
    uint32_t rgba[NEXUS_PALETTE_SIZE];

    /* Texture atlas */
    Nexus_Texture textures[NEXUS_MAX_TEXTURES];
    int texture_count;
} Nexus_PaletteState;

/* ── Default dungeon palette (BGR555) ─────────────────────────────── */

/* Source-lock: DM Nexus Saturn master palette inferred from STONE.BIN
 * (4 KB = 256 entries × 16-bit BGR555). If STONE.BIN is absent, these
 * defaults produce perceptually close dungeon colors. */
extern const uint16_t g_nexus_default_palette_bgr555[NEXUS_PALETTE_SIZE];

/* Flat color used for any texture slot with no loaded bitmap */
#define NEXUS_TEXTURE_FLAT_COLOR  7  /* palette entry 7 = mid-gray  */

/* ── Palette API ────────────────────────────────────────────────────── */

/* Initialize palette from STONE.BIN (16-bit BGR555, 256 entries).
 * If STONE.BIN is absent or shorter than expected, fills with
 * g_nexus_default_palette_bgr555 and logs a diagnostic.
 * Returns number of entries loaded, or 0 on error. */
int nexus_palette_load_stone(Nexus_PaletteState *pal, const uint8_t *data, int size);

/* Load extended palette from a surface file (TITLE.CG, ITEM.IBS, etc).
 * Reads BGR555 or RGB888 entries at a specific offset.
 * Returns.entries written or 0 on error. */
int nexus_palette_load_surface(Nexus_PaletteState *pal, const uint8_t *data,
    int size, int offset, int count, uint8_t pal_start);

/* Convert BGR555 → 0xAARRGGBB and expand into rgba[].
 * Call after any mutation to entries[]. */
void nexus_palette_expand_rgba(Nexus_PaletteState *pal);

/* Look up RGBA from an indexed color index.
 * This is the fastest path for framebuffer → RGBA conversion. */
static inline uint32_t nexus_palette_lookup(const Nexus_PaletteState *pal, uint8_t idx) {
    return pal->rgba[idx & 0xFF];
}

/* Load a texture from a bitmap region (x, y, w, h) in a surface file.
 * Texture ID is returned (>= 0), or -1 on failure.
 * Source label is used for deterministic fallback diagnostics. */
int nexus_texture_load_from_surface(Nexus_PaletteState *pal,
    const uint8_t *surface_data, int surface_size,
    int x, int y, int w, int h,
    uint8_t pal_start, uint8_t pal_count,
    const char *source_file, const char *label);

/* Free all texture data owned by the palette state. */
void nexus_palette_free_textures(Nexus_PaletteState *pal);

/* Initialize palette with defaults (g_nexus_default_palette_bgr555). */
void nexus_palette_init_defaults(Nexus_PaletteState *pal);

#endif /* NEXUS_V1_PALETTE_H */
