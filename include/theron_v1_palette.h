#ifndef THERON_V1_PALETTE_H
#define THERON_V1_PALETTE_H

#include <stdint.h>

/* Platform macro for cross-compiler compatibility — must be defined
 * before use in function declarations below, so lives outside the
 * extern "C" block. */
#if defined(__GNUC__)
#define TQR_RESTRICT __restrict__
#elif defined(_MSC_VER)
#define TQR_RESTRICT __restrict
#else
#define TQR_RESTRICT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════════
 * Theron's Quest V1 — Palette and PC Engine Tile System
 *
 * PC Engine (TurboGrafx-16) graphics architecture:
 *   HuC6260 VDC — Video Display Controller
 *   HuC6270 VCE — Video Color Encoder (512-color palette)
 *   8x8 pixel tiles, stored as planar bitmaps in VRAM
 *   16 colors per tile palette (4-bit index per pixel)
 *   512 total palette entries: 16 palette groups x 16 colors each
 *
 * Tile format (2bpp = walls/floors, 4bpp = sprites/creatures):
 *   Each row of 8 pixels stored as consecutive bit-plane bytes.
 *   2bpp:  16 bytes per tile  (8 rows x 2 bit-planes)
 *   4bpp:  32 bytes per tile  (8 rows x 4 bit-planes)
 *   Bit order: LSB = leftmost pixel (HuC6260 native)
 *
 * Palette BGR444 format (4 bits per channel, 12-bit packed):
 *   bits [11:8] = R (4 bits, values 0-15)
 *   bits [ 7:4] = G (4 bits, values 0-15)
 *   bits [ 3:0] = B (4 bits, values 0-15)
 *   Expanded to RGBA8: r=(R<<4)|R, g=(G<<4)|G, b=(B<<4)|B, a=0xFF
 *
 * Deterministic fallback (Phase 4 mandate):
 *   Any tile/texture that cannot be loaded from Track 02 data is
 *   rendered as a solid color using palette entry 7 (mid-gray),
 *   never as a crash or uninitialized buffer.
 *
 * Source references:
 *   THQUEST.ASM T400   — tile bank loading
 *   THQUEST.ASM T520   — dungeon viewport tile selection
 *   HuC6260/HuC6270 datasheet — VDC/VCE color/palette format
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §7
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Palette constants ─────────────────────────────────────────────── */
#define TQR_PALETTE_SIZE        512  /* 16 palette groups × 16 colors */
#define TQR_PALETTE_GROUP_SIZE   16  /* colors per palette group     */
#define TQR_PALETTE_GROUPS       16  /* palette groups 0-15          */
#define TQR_MAX_TILES          1024  /* max tile indices in atlas    */
#define TQR_TILE_SIZE_2BPP       16  /* bytes per 8×8 tile, 2bpp     */
#define TQR_TILE_SIZE_4BPP       32  /* bytes per 8×8 tile, 4bpp     */
#define TQR_TILE_DIM              8  /* tile is 8×8 pixels           */
#define TQR_VIEWPORT_W          256  /* PC Engine NTSC viewport width  */
#define TQR_VIEWPORT_H          224  /* PC Engine NTSC viewport height */
#define TQR_SCREEN_W            320  /* Extended mode width (x1.25)   */
#define TQR_SCREEN_H            240  /* Extended mode height          */

/* Palette group assignments */
#define TQR_PAL_GROUP_DUNGEON     0  /* walls, floors, ceiling        */
#define TQR_PAL_GROUP_CREATURES   1  /* monster/creature sprites      */
#define TQR_PAL_GROUP_OBJECTS     2  /* items, doors, interactive     */
#define TQR_PAL_GROUP_UI         3  /* HUD, menus, text               */
#define TQR_PAL_GROUP_TITLE      14  /* title screen graphics          */
#define TQR_PAL_GROUP_FONT        4  /* 8×8 font tile palette          */
#define TQR_PAL_FLAT_COLOR        7  /* deterministic fallback color   */

/* ── Tile descriptor ───────────────────────────────────────────────── */
typedef struct {
    uint8_t  *data;           /* planar tile bitmap (NULL=not loaded) */
    int       bpp;             /* 2 or 4 — bits per pixel              */
    int       pal_group;       /* 0-15 — which palette group           */
    uint16_t  vram_index;      /* VRAM tile index (if uploaded)        */
    int       uploaded;        /* 1=tile in VRAM, 0=system RAM only   */
    const char *source_label;  /* e.g. "wall_tiles" for diagnostics    */
} TQR_Tile;

/* ── Palette entry ─────────────────────────────────────────────────── */
typedef struct {
    uint16_t  bgr444;          /* 12-bit packed BGR (HuC6270 native)  */
    uint32_t  rgba;            /* 0xAARRGGBB, expanded 4→8 bits      */
} TQR_PaletteEntry;

/* ── Master palette state ─────────────────────────────────────────── */
typedef struct {
    /* 512 palette entries (16 groups × 16 colors) */
    TQR_PaletteEntry entries[TQR_PALETTE_SIZE];

    /* Tile atlas — system-RAM tile pool */
    TQR_Tile  tiles[TQR_MAX_TILES];
    int       tile_count;

    /* Tile bank layout — offsets into tile[] for each category    */
    /* These are set by theron_v1_palette_load_banks() based on  */
    /* the data format discovered in Track 02.                   */
    int wall_tile_first;
    int wall_tile_count;
    int floor_tile_first;
    int floor_tile_count;
    int object_tile_first;
    int object_tile_count;
    int creature_tile_first;
    int creature_tile_count;
    int font_tile_first;
    int font_tile_count;
} TQR_PaletteState;

/* ── PC Engine planar tile decode ─────────────────────────────────── */

/* Decode one 8×8 planar tile row into an 8-pixel indexed bitmap.
 * bpp=2: src has 2 bytes per row (bitplane 0 then bitplane 1)
 * bpp=4: src has 4 bytes per row (bitplane 0..3)
 * Out row must have 8 bytes (one palette index per pixel).
 * Bit layout: LSB = leftmost pixel (HuC6260 native, NOT flipped). */
void tqr_decode_tile_row(uint8_t *TQR_RESTRICT out_row,
                          const uint8_t *TQR_RESTRICT src_row,
                          int bpp);

/* Decode a full 8×8 tile into a linear 64-byte indexed bitmap.
 * Calls tqr_decode_tile_row() for each of the 8 rows. */
void tqr_decode_tile(uint8_t *TQR_RESTRICT out64,
                     const uint8_t *TQR_RESTRICT src,
                     int bpp);

/* ── Palette API ───────────────────────────────────────────────────── */

/* BGR444 → RGBA8 expansion (4-bit → 8-bit per channel).
 * HuC6270 format: 4 bits per channel, left-shift 4 to expand. */
static inline uint32_t tqr_bgr444_to_rgba(uint16_t bgr444) {
    unsigned r = (bgr444 >> 8) & 0xF;
    unsigned g = (bgr444 >> 4) & 0xF;
    unsigned b =  bgr444        & 0xF;
    r = (r << 4) | r;
    g = (g << 4) | g;
    b = (b << 4) | b;
    return 0xFF000000U | (r << 16) | (g << 8) | b;
}

/* Initialize palette with PC Engine dungeon default tones.
 * Fills all 512 entries with deterministic dungeon stone palette. */
void tqr_palette_init_defaults(TQR_PaletteState *pal);

/* Load palette data from a packed BGR444 buffer.
 * count entries from data (2 bytes each), starting at entry index start.
 * Returns entries loaded, or 0 on error. */
int tqr_palette_load_group(TQR_PaletteState *pal,
                           const uint8_t *data,
                           int start, int count);

/* Build RGBA lookup from raw BGR444 entries (call after any entry edit). */
void tqr_palette_expand_rgba(TQR_PaletteState *pal);

/* Look up RGBA by palette entry index (fast inline path). */
static inline uint32_t tqr_palette_lookup(const TQR_PaletteState *pal,
                                           uint8_t idx) {
    return pal->entries[idx & (TQR_PALETTE_SIZE - 1)].rgba;
}

/* ── Tile loading API ─────────────────────────────────────────────── */

/* Load a single 8×8 tile from a planar byte buffer into the tile pool.
 * Returns tile index (>=0), or -1 on error/fallback.
 * bpp: 2 or 4. pal_group: 0-15.
 * label: diagnostic string (e.g. "wall_tile_17"). */
int tqr_tile_load_from_data(TQR_PaletteState *pal,
                            const uint8_t *data,
                            int bpp,
                            int pal_group,
                            const char *label);

/* Load a strip of N tiles (contiguous in data) into the tile pool.
 * Each tile is tile_size bytes. Returns first tile index or -1 on error. */
int tqr_tile_strip_load(TQR_PaletteState *pal,
                        const uint8_t *data,
                        int tile_count,
                        int tile_size_bytes,
                        int bpp,
                        int pal_group,
                        const char *label_prefix);

/* Free all tile data owned by the palette state. */
void tqr_palette_free_tiles(TQR_PaletteState *pal);

/* Get tile data pointer and row stride for rasterization.
 * Returns NULL if tile_index is out of range or tile not loaded. */
const uint8_t *tqr_tile_get_data(const TQR_PaletteState *pal,
                                 int tile_index);

/* Get tile metadata. Returns 0 on success, -1 if tile not present. */
int tqr_tile_get_info(const TQR_PaletteState *pal,
                      int tile_index,
                      int *out_bpp,
                      int *out_pal_group);

/* Default dungeon master palette (512 x BGR444, PC Engine native).
 * Covers dungeon stone (groups 0-3), creatures (group 1), objects
 * (group 2), UI (group 3), title (group 14), font (group 4).
 * Used as fallback when Track 02 palette data is unavailable. */
extern const uint16_t g_tqr_default_palette_bgr444[TQR_PALETTE_SIZE];

/* ── Deterministic fallback rules ─────────────────────────────────── */

/* Flat-color tile used when a tile slot cannot be loaded.
 * Uses palette entry TQR_PAL_FLAT_COLOR (index 7), which is mid-gray
 * in the default palette. This ensures identical output on all
 * platforms even when Track 02 data is missing or malformed. */
#define TQR_TILE_FALLBACK_COLOR_INDEX  TQR_PAL_FLAT_COLOR  /* = 7 */

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_PALETTE_H */