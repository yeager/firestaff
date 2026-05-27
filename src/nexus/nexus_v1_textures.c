#include "nexus_v1_textures.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int nexus_tex_init(Nexus_TextureManager *tex, void *engine) {
    int i;
    if (!tex) return -1;
    memset(tex, 0, sizeof(*tex));
    tex->engine = engine;
    for (i = 0; i < NEXUS_MAX_TEXTURE_PAGES; i++)
        tex->page_loaded[i] = 0;
    printf("Nexus texture manager: %d pages, %d bytes each\n",
        NEXUS_MAX_TEXTURE_PAGES, NEXUS_TEX_PAGE_SIZE);
    return 0;
}

int nexus_tex_load_page(Nexus_TextureManager *tex,
    int page_index,
    const uint8_t *vdp1_data, int data_size)
{
    if (!tex || page_index < 0 || page_index >= NEXUS_MAX_TEXTURE_PAGES)
        return -1;
    if (!vdp1_data || data_size < NEXUS_TEX_PAGE_SIZE)
        return -1;

    /* Copy VDP1 BITMAP data (packed 4-bit pixels) */
    memcpy(tex->pages[page_index], vdp1_data, NEXUS_TEX_PAGE_SIZE);
    tex->page_loaded[page_index] = 1;
    tex->page_refcount[page_index] = 1;
    tex->page_count++;
    return 0;
}

const uint8_t *nexus_tex_get_page(const Nexus_TextureManager *tex,
    int page_index)
{
    if (!tex || page_index < 0 || page_index >= NEXUS_MAX_TEXTURE_PAGES)
        return NULL;
    if (!tex->page_loaded[page_index])
        return NULL;
    return tex->pages[page_index];
}

void nexus_tex_acquire(Nexus_TextureManager *tex, int page_index) {
    if (!tex || page_index < 0 || page_index >= NEXUS_MAX_TEXTURE_PAGES)
        return;
    if (tex->page_loaded[page_index])
        tex->page_refcount[page_index]++;
}

void nexus_tex_release(Nexus_TextureManager *tex, int page_index) {
    if (!tex || page_index < 0 || page_index >= NEXUS_MAX_TEXTURE_PAGES)
        return;
    if (!tex->page_loaded[page_index]) return;
    tex->page_refcount[page_index]--;
    if (tex->page_refcount[page_index] <= 0) {
        tex->page_loaded[page_index] = 0;
        tex->page_count--;
    }
}

void nexus_tex_shutdown(Nexus_TextureManager *tex) {
    if (!tex) return;
    memset(tex, 0, sizeof(*tex));
}

/* ── Procedural fallback textures ──────────────────────────── */

void nexus_tex_generate_brick(uint8_t *page,
    int brick_w, int brick_h, int mortar_thick,
    uint8_t brick_color, uint8_t mortar_color)
{
    int x, y, bx, by, ox, oy;
    if (!page) return;
    memset(page, mortar_color, NEXUS_TEX_PAGE_SIZE);

    for (by = 0; by < 256; by += brick_h) {
        int row_offset = (by / brick_h) & 1 ? brick_w / 2 : 0;
        for (bx = 0; bx < 256; bx += brick_w) {
            int ox = (bx + row_offset) & 255;
            for (y = by; y < by + brick_h - mortar_thick && y < 256; y++) {
                for (x = ox; x < ox + brick_w - mortar_thick && x < 256; x++) {
                    int idx = (y * 256 + x) >> 1;
                    int shift = (x & 1) ? 0 : 4;
                    page[idx] = (uint8_t)((page[idx] & ~(0x0F << shift)) | (brick_color << shift));
                }
            }
        }
    }
}

void nexus_tex_generate_checker(uint8_t *page,
    int tile_size, uint8_t color0, uint8_t color1)
{
    int x, y, c;
    if (!page) return;
    for (y = 0; y < 256; y++) {
        for (x = 0; x < 256; x++) {
            c = ((y / tile_size) + (x / tile_size)) & 1 ? color1 : color0;
            int idx = (y * 256 + x) >> 1;
            int shift = (x & 1) ? 0 : 4;
            page[idx] = (uint8_t)((page[idx] & ~(0x0F << shift)) | (c << shift));
        }
    }
}
