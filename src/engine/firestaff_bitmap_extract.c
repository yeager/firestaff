
#include "firestaff_bitmap_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fs_gfx_load(FS_GraphicsAtlas *atlas, const char *path) {
    FILE *f;
    long fsize;
    int count, i, offset;
    const uint8_t *hdr;

    if (!atlas || !path) return -1;
    memset(atlas, 0, sizeof(*atlas));

    f = fopen(path, "rb");
    if (!f) { printf("GFX: cannot open %s\n", path); return -1; }
    fseek(f, 0, SEEK_END); fsize = ftell(f); fseek(f, 0, SEEK_SET);
    atlas->raw_data = (uint8_t *)malloc(fsize);
    if (!atlas->raw_data) { fclose(f); return -1; }
    fread(atlas->raw_data, 1, fsize, f);
    fclose(f);
    atlas->raw_size = (int)fsize;

    hdr = atlas->raw_data;

    /* Verify PC34 new format signature */
    if (fsize < 8 || hdr[0] != 0x01 || hdr[1] != 0x80) {
        printf("GFX: not PC34 new format (sig=%02x%02x)\n", hdr[0], hdr[1]);
        /* Try anyway — might be old format */
    }

    count = hdr[2] | (hdr[3] << 8);
    if (count <= 0 || count > GFX_MAX_BITMAPS) {
        printf("GFX: invalid count %d\n", count);
        return -1;
    }
    atlas->bitmap_count = count;

    /* Parse header tables */
    offset = 4;

    /* Compressed sizes */
    for (i = 0; i < count && offset + 2 <= (int)fsize; i++) {
        atlas->bitmaps[i].comp_size = hdr[offset] | (hdr[offset+1] << 8);
        offset += 2;
    }

    /* Decompressed sizes */
    for (i = 0; i < count && offset + 2 <= (int)fsize; i++) {
        atlas->bitmaps[i].decomp_size = hdr[offset] | (hdr[offset+1] << 8);
        offset += 2;
    }

    /* Dimensions (packed: low 16 = width, high 16 = height... or two uint16) */
    for (i = 0; i < count && offset + 4 <= (int)fsize; i++) {
        atlas->bitmaps[i].width = hdr[offset] | (hdr[offset+1] << 8);
        atlas->bitmaps[i].height = hdr[offset+2] | (hdr[offset+3] << 8);
        offset += 4;
    }

    /* Data offsets: sequential after header */
    {
        int data_start = offset;
        int cur = data_start;
        for (i = 0; i < count; i++) {
            atlas->bitmaps[i].data_offset = cur;
            int sz = atlas->bitmaps[i].comp_size;
            if (sz == 0) sz = atlas->bitmaps[i].decomp_size;
            cur += sz;
        }
    }

    /* Try to extract VGA palette from first resource if it looks like one */
    if (count > 0 && atlas->bitmaps[0].comp_size == 768) {
        int po = atlas->bitmaps[0].data_offset;
        if (po + 768 <= (int)fsize) {
            for (i = 0; i < 256; i++) {
                int r = (hdr[po + i*3] & 0x3F) << 2;
                int g = (hdr[po + i*3+1] & 0x3F) << 2;
                int b = (hdr[po + i*3+2] & 0x3F) << 2;
                atlas->palette[i] = 0xFF000000 | (r<<16) | (g<<8) | b;
            }
            atlas->palette_loaded = 1;
            printf("GFX: palette extracted from resource 0\n");
        }
    }

    printf("GFX: loaded %d bitmaps from %s (%ld bytes)\n", count, path, fsize);
    return count;
}

int fs_gfx_extract_bitmap(const FS_GraphicsAtlas *atlas, int index,
    uint8_t *out_pixels, int max_size)
{
    int w, h, src_bytes, i;
    const uint8_t *src;

    if (!atlas || !out_pixels || index < 0 || index >= atlas->bitmap_count)
        return -1;

    w = atlas->bitmaps[index].width;
    h = atlas->bitmaps[index].height;
    if (w * h > max_size) return -1;

    src = atlas->raw_data + atlas->bitmaps[index].data_offset;
    src_bytes = atlas->bitmaps[index].comp_size;
    if (src_bytes == 0) src_bytes = atlas->bitmaps[index].decomp_size;

    if (atlas->bitmaps[index].data_offset + src_bytes > atlas->raw_size)
        return -1;

    /* 4bpp: 2 pixels per byte, high nibble first */
    for (i = 0; i < src_bytes && i * 2 < w * h; i++) {
        out_pixels[i * 2] = (src[i] >> 4) & 0x0F;
        if (i * 2 + 1 < w * h)
            out_pixels[i * 2 + 1] = src[i] & 0x0F;
    }

    return w * h;
}

int fs_gfx_get_size(const FS_GraphicsAtlas *atlas, int index, int *w, int *h) {
    if (!atlas || index < 0 || index >= atlas->bitmap_count) return -1;
    if (w) *w = atlas->bitmaps[index].width;
    if (h) *h = atlas->bitmaps[index].height;
    return 0;
}

void fs_gfx_indexed_to_rgba(const uint8_t *indexed, int w, int h,
    const uint32_t *palette, uint32_t *rgba_out)
{
    int i, total = w * h;
    if (!indexed || !palette || !rgba_out) return;
    for (i = 0; i < total; i++)
        rgba_out[i] = palette[indexed[i]];
}

void fs_gfx_free(FS_GraphicsAtlas *atlas) {
    if (atlas) { free(atlas->raw_data); atlas->raw_data = NULL; }
}

