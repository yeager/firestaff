#include "dm1_v1_atari_st_graphics_dat.h"
#include "csb_v1_graphics_lzw_pc34_compat.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"
#include "dm1_v1_legacy_graphics_dat.h"

#include <stdlib.h>
#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

int dm1_v1_atari_st_graphics_open(const uint8_t *data, size_t size,
                                  DM1_V1_AtariStGraphicsDat *out)
{
    const size_t table = 2u + DM1_V1_ATARI_ST_GRAPHICS_COUNT * 4u;
    size_t cursor = table;
    uint32_t total = 0u;
    uint16_t i;

    if (!data || !out || size < table ||
        be16(data) != DM1_V1_ATARI_ST_GRAPHICS_COUNT) return 0;
    memset(out, 0, sizeof(*out));
    for (i = 0u; i < DM1_V1_ATARI_ST_GRAPHICS_COUNT; ++i) {
        out->records[i].compressed_size = be16(data + 2u + (size_t)i * 2u);
        out->records[i].expanded_size = be16(data + 2u +
            DM1_V1_ATARI_ST_GRAPHICS_COUNT * 2u + (size_t)i * 2u);
        out->records[i].offset = cursor;
        if ((size_t)out->records[i].compressed_size > size - cursor) return 0;
        cursor += out->records[i].compressed_size;
        total += out->records[i].compressed_size;
    }
    if (cursor != size || total != size - table) return 0;
    out->data = data;
    out->size = size;
    return 1;
}

int dm1_v1_atari_st_graphics_read(const DM1_V1_AtariStGraphicsDat *dat,
                                  uint16_t index, uint8_t *out,
                                  size_t capacity)
{
    const DM1_V1_AtariStGraphic *record;
    if (!dat || !dat->data || !out || index >= DM1_V1_ATARI_ST_GRAPHICS_COUNT)
        return 0;
    record = &dat->records[index];
    if (capacity < record->expanded_size) return 0;
    if (record->compressed_size == record->expanded_size) {
        memcpy(out, dat->data + record->offset, record->expanded_size);
        return (int)record->expanded_size;
    }
    {
        size_t decoded = 0;
        /* ReDMCSB LZW.C F0497 starts dictionary entries at 257 (not an
         * end code); F0496 also expands the 0x90 repetition stream. */
        int status = csb_v1_graphics_lzw_decode_pc34_compat(
            dat->data + record->offset, record->compressed_size,
            out, record->expanded_size, &decoded);
        return status == 0 && decoded == record->expanded_size ? (int)decoded : 0;
    }
}

int dm1_v1_atari_st_graphics_decode(const DM1_V1_AtariStGraphicsDat *dat,
                                    uint16_t index, uint8_t *indexed_pixels,
                                    size_t pixel_capacity,
                                    uint16_t *out_width,
                                    uint16_t *out_height)
{
    const DM1_V1_AtariStGraphic *record;
    uint8_t *item;
    int decoded;

    if (!dat || !indexed_pixels || index >= DM1_V1_ATARI_ST_GRAPHICS_COUNT)
        return 0;
    record = &dat->records[index];
    if (record->expanded_size < 4u) return 0;
    item = (uint8_t *)malloc(record->expanded_size);
    if (!item) return 0;
    decoded = dm1_v1_atari_st_graphics_read(
        dat, index, item, record->expanded_size);
    if (decoded != (int)record->expanded_size) {
        free(item);
        return 0;
    }
    decoded = dm1_v1_legacy_graphics_decode_item(
        item, record->expanded_size, 1, indexed_pixels, pixel_capacity,
        out_width, out_height);
    free(item);
    return decoded;
}
