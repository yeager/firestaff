#include "firestaff_x68k_graphics_dat.h"
#include "dm1_v1_legacy_graphics_dat.h"

#include <string.h>

/*
 * ReDMCSB MEMORY.C, X30J path: marked-format signature and count, then
 * compressed/decompressed uint16 tables, then GRATTR {Width, Height} records.
 * Native X68000 words are big-endian. F0467_MEMORY_GetGraphicOffset uses
 * 4 + count * 8 as the format-1 payload start and sums compressed sizes.
 */
enum {
    X68K_GDAT_HEADER_BYTES = 4u,
    X68K_GDAT_SIZE_WORDS_PER_ITEM = 2u,
    X68K_GDAT_GRATTR_BYTES = 4u,
    X68K_GDAT_BYTES_PER_ITEM = 8u,
    X68K_GDAT_MARKER = 0x8001u
};

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

int FirestaffX68kGraphicsDat_Open(const uint8_t *bytes,
                                  size_t size,
                                  FirestaffX68kGraphicsDat *out) {
    uint16_t count;
    size_t table_bytes;
    size_t payload_offset;
    size_t i;
    size_t payload_size = 0u;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!bytes || size < X68K_GDAT_HEADER_BYTES ||
        read_be16(bytes) != X68K_GDAT_MARKER) {
        return -1;
    }
    count = read_be16(bytes + 2u);
    if (count == 0u || count > FIRESTAFF_X68K_GDAT_FORMAT1_MAX_ITEMS) {
        return -1;
    }
    if ((size_t)count > (SIZE_MAX - X68K_GDAT_HEADER_BYTES) /
                            X68K_GDAT_BYTES_PER_ITEM) {
        return -1;
    }
    table_bytes = (size_t)count * X68K_GDAT_BYTES_PER_ITEM;
    payload_offset = X68K_GDAT_HEADER_BYTES + table_bytes;
    if (payload_offset > size) return -1;

    /* The first table is compressed byte counts; validate the full payload. */
    for (i = 0u; i < (size_t)count; ++i) {
        const size_t table_offset = X68K_GDAT_HEADER_BYTES + i * 2u;
        const size_t compressed_size = read_be16(bytes + table_offset);
        if (compressed_size > size - payload_offset - payload_size) return -1;
        payload_size += compressed_size;
    }
    if (payload_offset + payload_size != size) return -1;

    out->bytes = bytes;
    out->size = size;
    out->count = count;
    out->payload_offset = payload_offset;
    return 0;
}

int FirestaffX68kGraphicsDat_GetItem(const FirestaffX68kGraphicsDat *dat,
                                     uint16_t index,
                                     FirestaffX68kGraphicsDatItem *out) {
    size_t compressed_table;
    size_t decompressed_table;
    size_t grattr_table;
    size_t i;
    size_t payload_offset;

    if (!dat || !dat->bytes || !out || dat->count == 0u ||
        dat->count > FIRESTAFF_X68K_GDAT_FORMAT1_MAX_ITEMS ||
        index >= dat->count ||
        dat->payload_offset != X68K_GDAT_HEADER_BYTES +
                               (size_t)dat->count * X68K_GDAT_BYTES_PER_ITEM ||
        dat->payload_offset > dat->size || dat->size < X68K_GDAT_HEADER_BYTES ||
        read_be16(dat->bytes) != X68K_GDAT_MARKER ||
        read_be16(dat->bytes + 2u) != dat->count) {
        return -1;
    }
    compressed_table = X68K_GDAT_HEADER_BYTES;
    decompressed_table = compressed_table + (size_t)dat->count * 2u;
    grattr_table = decompressed_table + (size_t)dat->count * 2u;
    if (grattr_table + (size_t)dat->count * X68K_GDAT_GRATTR_BYTES !=
        dat->payload_offset) {
        return -1;
    }

    payload_offset = dat->payload_offset;
    for (i = 0u; i < (size_t)index; ++i) {
        const size_t compressed_size =
            read_be16(dat->bytes + compressed_table + i * 2u);
        if (compressed_size > dat->size - payload_offset) return -1;
        payload_offset += compressed_size;
    }
    out->compressed_size = read_be16(dat->bytes + compressed_table +
                                     (size_t)index * 2u);
    out->decompressed_size = read_be16(dat->bytes + decompressed_table +
                                       (size_t)index * 2u);
    out->width = read_be16(dat->bytes + grattr_table +
                           (size_t)index * X68K_GDAT_GRATTR_BYTES);
    out->height = read_be16(dat->bytes + grattr_table +
                            (size_t)index * X68K_GDAT_GRATTR_BYTES + 2u);
    if (out->compressed_size > dat->size - payload_offset) return -1;
    out->payload_offset = payload_offset;
    return 0;
}

/* Greatstone SCK 1.5.1 db/map/dm_x68k_gd.map classifies these IDs as IMG1:
 * 0..532, 538, 548, and 558..561. Keep this X30J classification separate
 * from the different FM Towns/Amiga legacy bitmap index map. */
static int x68k_x30j_item_is_img1(uint16_t index) {
    return index <= 532u || index == 538u || index == 548u ||
           (index >= 558u && index <= 561u);
}

int FirestaffX68kGraphicsDat_DecodeImageItem(
    const FirestaffX68kGraphicsDat *dat,
    uint16_t index,
    uint8_t *indexed_pixels,
    size_t pixel_capacity,
    uint16_t *out_width,
    uint16_t *out_height) {
    FirestaffX68kGraphicsDatItem item;
    uint16_t width;
    uint16_t height;
    size_t pixel_count;

    if (!dat || !indexed_pixels || !x68k_x30j_item_is_img1(index) ||
        FirestaffX68kGraphicsDat_GetItem(dat, index, &item) != 0 ||
        item.compressed_size != item.decompressed_size ||
        item.width == 0u || item.height == 0u || item.width > 640u ||
        item.height > 400u) {
        return -1;
    }
    pixel_count = (size_t)item.width * (size_t)item.height;
    if (pixel_count > 1024u * 1024u || pixel_count > pixel_capacity ||
        dm1_v1_legacy_graphics_decode_item(
            dat->bytes + item.payload_offset, item.compressed_size, 1,
            indexed_pixels, pixel_capacity, &width, &height) != 1 ||
        width != item.width || height != item.height) {
        return -1;
    }
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;
    return 0;
}
