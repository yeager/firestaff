/* Nexus V1 Palette and Texture System — implementation
 * =====================================================
 * Saturn VDP1 color palette (BGR555, 256 entries) + texture atlas.
 *
 * Source-lock references:
 *   ReDMCSB PALETTE.C   — DM1 palette load/expand
 *   Saturn VDP1 SDK     — Color RAM format (BGR555, 32 KB / 256 entries)
 *   docs/NEXUS_FILE_CLASSIFICATION.md  — STONE.BIN 4 KB, TITLE.CG 164 KB,
 *     ITEM.IBS 98 KB
 *
 * Missing or incomplete source palettes remain unavailable. They never
 * promote a generated colour table to a renderable Saturn surface. */

#include "nexus_v1_palette.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* ── BGR555 → RGBA ─────────────────────────────────────────────────── */

/* Saturn VDP1: Color RAM 16-bit BGR555
 *   bits [14:10] = R (5 bits)
 *   bits [ 9: 5] = G (5 bits)
 *   bits [ 4: 0] = B (5 bits)
 * Expand to 8-bit per channel by left-shift 3, then OR top 2 bits
 * (8 = 5<<3 | 5>>2 produces 11111111 for max value 31). */
static inline uint32_t bgr555_to_rgba(uint16_t bgr555) {
    unsigned r = (bgr555 >> 10) & 0x1F;
    unsigned g = (bgr555 >>  5) & 0x1F;
    unsigned b =  bgr555        & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return 0xFF000000U | (r << 16) | (g << 8) | b;
}

/* ── Public API ────────────────────────────────────────────────────── */

void nexus_palette_init_defaults(Nexus_PaletteState *pal) {
    if (!pal) return;
    memset(pal, 0, sizeof(*pal));
}

int nexus_palette_load_stone(Nexus_PaletteState *pal,
    const uint8_t *data, int size)
{
    (void)data;
    (void)size;
    if (!pal) return 0;

    /* DMWeb's retail STONE.BIN is eight independent 550-byte `pp` records:
     * each owns a 16-entry big-endian palette beside its 4bpp texels.  It is
     * not a 256-entry global palette.  Keep this legacy entry point blocked;
     * callers must use nexus_palette_stone_pp_receipt() and
     * nexus_palette_decode_stone_pp_record() so image-local provenance is
     * preserved instead of manufacturing a global colour table. */
    memset(pal->entries, 0, sizeof(pal->entries));
    memset(pal->rgba, 0, sizeof(pal->rgba));
    pal->source_palette_bound = 0;
    printf("Nexus palette: STONE.BIN uses image-local pp palettes; "
           "global palette load blocked\n");
    return 0;
}

int nexus_palette_load_surface(Nexus_PaletteState *pal,
    const uint8_t *data, int size,
    int offset, int count, uint8_t pal_start)
{
    int i;
    if (!pal || !data) return 0;
    /* A surface span is source data, not a palette initializer.  Do not
     * clamp, zero-fill, or perform offset+length arithmetic before the
     * bounds have been proven: any of those would manufacture Saturn
     * palette entries from an incomplete asset. */
    if (size < 0 || offset < 0 || count <= 0 ||
        (int)pal_start >= NEXUS_PALETTE_SIZE ||
        count > NEXUS_PALETTE_SIZE - (int)pal_start ||
        offset > size || count > (size - offset) / 2) {
        printf("Nexus palette: incomplete/invalid surface span "
               "offset=%d count=%d size=%d start=%u — source palette unchanged\n",
               offset, count, size, (unsigned)pal_start);
        return 0;
    }
    for (i = 0; i < count; i++)
        pal->entries[pal_start + i] =
            (uint16_t)data[offset + i*2] | ((uint16_t)data[offset + i*2+1] << 8);
    /* A partial surface span may extend an already authenticated palette,
     * but cannot authenticate one by itself.  Only a complete 256-entry
     * source span may promote an unbound palette. */
    if (pal_start == 0 && count == NEXUS_PALETTE_SIZE)
        pal->source_palette_bound = 1;
    nexus_palette_expand_rgba(pal);
    printf("Nexus palette: loaded %d entries at offset %d [start=%d]\n",
           count, offset, pal_start);
    return count;
}

void nexus_palette_expand_rgba(Nexus_PaletteState *pal) {
    int i;
    if (!pal) return;
    for (i = 0; i < NEXUS_PALETTE_SIZE; i++)
        pal->rgba[i] = bgr555_to_rgba(pal->entries[i]);
}

/* ── Texture loading ───────────────────────────────────────────────── */

int nexus_texture_load_from_surface(Nexus_PaletteState *pal,
    const uint8_t *data, int data_size,
    int x, int y, int w, int h,
    uint8_t pal_start, uint8_t pal_count,
    const char *source_file, const char *label)
{
    Nexus_Texture *tx;
    int row, needed_sz;

    if (!pal) {
        printf("Nexus texture: ERROR null palette for '%s'\n",
               label ? label : "?");
        return -1;
    }
    if (x < 0 || y < 0 || w <= 0 || h <= 0) {
        printf("Nexus texture: ERROR invalid %dx%d for '%s' "
               "[source=%s] — source surface blocked\n",
               w, h, label ? label : "?", source_file ? source_file : "?");
        return -1;
    }
    if (pal->texture_count >= NEXUS_MAX_TEXTURES) {
        printf("Nexus texture: ERROR atlas full (%d) loading '%s' "
               "[source=%s] — source surface blocked\n",
               NEXUS_MAX_TEXTURES, label ? label : "?",
               source_file ? source_file : "?");
        return -1;
    }

    tx = &pal->textures[pal->texture_count];
    tx->w = w;
    tx->h = h;
    tx->pal_start = pal_start;
    tx->pal_count = pal_count;
    tx->source_file = source_file;
    tx->label = label;

    if (y > (INT_MAX - h) || x > (INT_MAX - w) ||
        (y + h) > (INT_MAX / w)) {
        printf("Nexus texture: ERROR source dimensions overflow for '%s' "
               "[source=%s] — source surface blocked\n",
               label ? label : "?", source_file ? source_file : "?");
        return -1;
    }
    needed_sz = (y + h - 1) * w + (x + w);
    tx->data = (uint8_t *)calloc(w * h, 1);
    if (!tx->data) return -1;
    tx->owns_data = 1;

    if (!data || data_size < needed_sz) {
        printf("Nexus texture: ERROR incomplete source %d < %d for '%s' "
               "[%s] — source surface blocked\n",
               data_size, needed_sz, label ? label : "?",
               source_file ? source_file : "?");
        free(tx->data);
        tx->data = NULL;
        tx->owns_data = 0;
        return -1;
    }
    for (row = 0; row < h; row++)
        memcpy(tx->data + row * w, data + (y + row) * w + x, w);

    printf("Nexus texture: loaded %dx%d '%s' from %s "
           "(pal %d-%d) -> id=%d\n",
           w, h, label ? label : "?", source_file ? source_file : "?",
           pal_start, pal_start + pal_count - 1,
           pal->texture_count);

    pal->texture_count++;
    return pal->texture_count - 1;
}

void nexus_palette_free_textures(Nexus_PaletteState *pal) {
    int i;
    if (!pal) return;
    for (i = 0; i < pal->texture_count; i++) {
        if (pal->textures[i].owns_data && pal->textures[i].data) {
            free(pal->textures[i].data);
            pal->textures[i].data = NULL;
            pal->textures[i].owns_data = 0;
        }
    }
    pal->texture_count = 0;
}
