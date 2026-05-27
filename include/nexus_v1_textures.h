#ifndef NEXUS_V1_TEXTURES_H
#define NEXUS_V1_TEXTURES_H

#include <stdint.h>

/* Nexus texture manager — handles VDP1 BITMAP texture pages.
 * Each page is 256×256 pixels at 4 bits per pixel = 32 KB.
 * Textures are stored in DGN geometry blobs and DM.BIN. */

/* Maximum texture pages in cache */
#define NEXUS_MAX_TEXTURE_PAGES 256
#define NEXUS_TEX_PAGE_SIZE (256 * 256 / 2)  /* 32 KB (4-bit per pixel) */

typedef struct {
    /* Texture page data (packed 4-bit pixels) */
    uint8_t pages[NEXUS_MAX_TEXTURE_PAGES][NEXUS_TEX_PAGE_SIZE];

    /* Per-page state */
    int page_loaded[NEXUS_MAX_TEXTURE_PAGES];
    int page_refcount[NEXUS_MAX_TEXTURE_PAGES];

    /* Engine reference for loading */
    void *engine;  /* Nexus_V1_Engine* */
    int page_count;
} Nexus_TextureManager;

/* Initialize texture manager */
int nexus_tex_init(Nexus_TextureManager *tex, void *engine);

/* Load a texture page from VDP1 BITMAP data.
 * Returns page index (0..255) or -1 on failure. */
int nexus_tex_load_page(Nexus_TextureManager *tex,
    int page_index,
    const uint8_t *vdp1_data, int data_size);

/* Get pointer to texture page data (or NULL if not loaded) */
const uint8_t *nexus_tex_get_page(const Nexus_TextureManager *tex,
    int page_index);

/* Acquire/release reference (for LRU cache eviction) */
void nexus_tex_acquire(Nexus_TextureManager *tex, int page_index);
void nexus_tex_release(Nexus_TextureManager *tex, int page_index);

/* Look up a single texel from a texture page.
 * Returns 4-bit color index (0..15). */
static inline uint8_t nexus_tex_get_pixel(const Nexus_TextureManager *tex,
    int page_index, int u, int v)
{
    const uint8_t *page = nexus_tex_get_page(tex, page_index);
    if (!page || u < 0 || u >= 256 || v < 0 || v >= 256)
        return 0;
    int byte_idx = (v * 256 + u) >> 1;  /* 2 pixels per byte */
    int shift = (u & 1) ? 0 : 4;         /* low nibble = even, high nibble = odd */
    return (page[byte_idx] >> shift) & 0x0F;
}

/* Shutdown texture manager (frees all pages) */
void nexus_tex_shutdown(Nexus_TextureManager *tex);

/* Procedural fallback texture generators (deterministic) */
void nexus_tex_generate_brick(uint8_t *page,
    int brick_w, int brick_h, int mortar_thick,
    uint8_t brick_color, uint8_t mortar_color);

void nexus_tex_generate_checker(uint8_t *page,
    int tile_size, uint8_t color0, uint8_t color1);

#endif
