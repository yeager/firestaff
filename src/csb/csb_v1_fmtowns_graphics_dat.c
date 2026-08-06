#include "csb_v1_fmtowns_graphics_dat.h"
#include <string.h>
#include <stdlib.h>

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t fnv1a(const uint8_t *data, size_t len) {
    uint32_t h = 2166136261u;
    size_t i;
    if (!data || len == 0) return 0;
    for (i = 0; i < len; i++) {
        h ^= data[i];
        h *= 16777619u;
    }
    return h ? h : 1u;
}

int csb_v1_fmtowns_graphics_probe(const uint8_t *data, size_t size) {
    uint16_t marker, count;
    if (!data || size < 4) return 0;
    marker = rd16le(data);
    count = rd16le(data + 2);
    return marker == CSB_FMTOWNS_GRAPHICS_CONTAINER_WORD &&
           count == CSB_FMTOWNS_GRAPHICS_ITEM_COUNT &&
           size >= CSB_FMTOWNS_GRAPHICS_MIN_SIZE &&
           size <= CSB_FMTOWNS_GRAPHICS_MAX_SIZE;
}

int csb_v1_fmtowns_graphics_receipt(const uint8_t *data, size_t size,
                                    CSB_V1_FmtownsGraphicsReceipt *out) {
    uint16_t count;
    size_t off, payload_start, payload_off;
    uint32_t images, empty, datarec;
    uint16_t i;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!csb_v1_fmtowns_graphics_probe(data, size)) return -1;

    count = rd16le(data + 2);
    out->is_fmtowns = 1;
    out->item_count = count;
    out->file_size = (uint32_t)size;

    /* Header: 4 bytes marker+count, then 3 tables of count entries each */
    payload_start = 4u + (size_t)count * 2u  /* compressed sizes */
                      + (size_t)count * 2u   /* decompressed sizes */
                      + (size_t)count * 4u;  /* width/height pairs */

    if (payload_start > size) return -1;

    out->header_fnv1a = fnv1a(data, payload_start);

    /* Classify items */
    images = 0;
    empty = 0;
    datarec = 0;
    payload_off = payload_start;
    off = 4;
    for (i = 0; i < count; i++) {
        uint16_t comp = rd16le(data + off);
        uint16_t cw = rd16le(data + 4u + (size_t)count * 4u + (size_t)i * 4u);
        uint16_t ch = rd16le(data + 4u + (size_t)count * 4u + (size_t)i * 4u + 2u);

        if (comp == 0) {
            empty++;
        } else if (comp >= 4 && payload_off + comp <= size) {
            uint16_t iw = rd16le(data + payload_off);
            uint16_t ih = rd16le(data + payload_off + 2);
            if (iw == cw && ih == ch && iw > 0 && ih > 0 &&
                iw <= 640 && ih <= 400) {
                images++;
            } else {
                datarec++;
            }
        } else {
            datarec++;
        }
        payload_off += comp;
        off += 2;
    }

    out->image_item_count = images;
    out->empty_item_count = empty;
    out->data_item_count = datarec;
    out->payload_fnv1a = fnv1a(data + payload_start,
                               size - payload_start);
    return 0;
}

/*
 * IMG2 byte-command decoder for FM Towns CSB items (LE16 headers).
 *
 * Command byte layout (ReDMCSB IMAGE2.C F0689, CSBWin ExpandGraphic):
 *   bit 7 = 0: short RLE — color = cmd & 0x0F, count = ((cmd >> 4) & 7) + 1
 *   bit 7 = 1:
 *     bit 6 = 0: medium count from next byte + 1
 *     bit 6 = 1: long count from next two bytes (BE16) + 1
 *     bits 5-4 (mode):
 *       0x00: solid fill with color
 *       0x10: literal run (packed nibble pairs)
 *       0x30: copy from previous scanline, then store color
 */
int csb_v1_fmtowns_img2_decode(const uint8_t *item, size_t item_size,
                                uint16_t container_width,
                                uint16_t container_height,
                                uint8_t *indexed_pixels,
                                size_t pixel_capacity,
                                CSB_V1_FmtownsItemDecodeReceipt *receipt) {
    uint16_t w, h;
    size_t total, src, pos, count, i;

    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!item || !indexed_pixels || item_size < 4) return 0;

    w = rd16le(item);
    h = rd16le(item + 2);

    /* Validate item header matches container dimensions */
    if (w != container_width || h != container_height ||
        w == 0 || h == 0 || w > 640 || h > 400) {
        /* Not an image item — might be a data record */
        if (receipt) {
            receipt->valid = 1;
            receipt->is_data_record = 1;
            receipt->stream_byte_count = item_size;
            receipt->stream_fnv1a = fnv1a(item, item_size);
        }
        return 0;
    }

    total = (size_t)w * h;
    if (total > pixel_capacity) return 0;
    memset(indexed_pixels, 0, total);

    src = 4;
    pos = 0;

    while (pos < total && src < item_size) {
        uint8_t cmd = item[src++];
        uint8_t color = cmd & 0x0fu;

        if ((cmd & 0x80u) == 0) {
            /* Short RLE: count in bits 6-4 */
            count = (size_t)((cmd >> 4) & 0x07u) + 1u;
            if (count > total - pos) count = total - pos;
            memset(indexed_pixels + pos, color, count);
            pos += count;
        } else {
            uint8_t mode;

            /* Read extended count */
            if ((cmd & 0x40u) == 0) {
                if (src >= item_size) return 0;
                count = (size_t)item[src++] + 1u;
            } else {
                if (src + 1 >= item_size) return 0;
                count = (size_t)(((uint16_t)item[src] << 8) |
                                  item[src + 1]) + 1u;
                src += 2;
            }

            mode = (cmd >> 4) & 0x03u;
            switch (mode) {
            case 0: /* solid fill */
                if (count > total - pos) count = total - pos;
                memset(indexed_pixels + pos, color, count);
                pos += count;
                break;

            case 1: { /* literal nibble pairs */
                size_t remaining = count;
                if (remaining > total - pos) remaining = total - pos;
                if (remaining & 1u) {
                    indexed_pixels[pos++] = color;
                    remaining--;
                }
                if (src + remaining / 2u > item_size) return 0;
                for (i = 0; i < remaining / 2u; i++) {
                    uint8_t packed = item[src++];
                    indexed_pixels[pos++] = packed >> 4;
                    indexed_pixels[pos++] = packed & 0x0fu;
                }
                break;
            }

            case 3: /* copy from previous scanline + final color */
                if (count > total - pos) count = total - pos;
                for (i = 0; i < count; i++) {
                    if (pos >= w) {
                        indexed_pixels[pos] = indexed_pixels[pos - w];
                    }
                    pos++;
                }
                if (pos < total) {
                    indexed_pixels[pos++] = color;
                }
                break;

            default:
                return 0;
            }
        }
    }

    if (receipt) {
        receipt->valid = 1;
        receipt->width = w;
        receipt->height = h;
        receipt->stream_byte_count = item_size;
        receipt->stream_bytes_consumed = src;
        receipt->pixel_count = pos;
        receipt->stream_fnv1a = fnv1a(item, item_size);
        receipt->pixel_fnv1a = fnv1a(indexed_pixels, total);
        receipt->is_image = 1;
        receipt->is_data_record = 0;
    }
    return pos >= total ? 1 : 0;
}

int csb_v1_fmtowns_graphics_decode_item(
    const uint8_t *data, size_t size, uint16_t item_index,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_FmtownsItemDecodeReceipt *receipt) {
    uint16_t count;
    size_t compressed_table;
    size_t dimensions_table;
    size_t payload_start;
    size_t payload_offset;
    uint16_t compressed_size;
    uint16_t width;
    uint16_t height;
    uint16_t i;

    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!data || !indexed_pixels || !csb_v1_fmtowns_graphics_probe(data, size)) {
        return 0;
    }
    count = rd16le(data + 2);
    if (item_index >= count) return 0;
    compressed_table = 4u;
    dimensions_table = compressed_table + (size_t)count * 4u;
    payload_start = dimensions_table + (size_t)count * 4u;
    if (payload_start > size) return 0;
    payload_offset = payload_start;
    for (i = 0u; i < item_index; ++i) {
        uint16_t prior_size = rd16le(data + compressed_table + (size_t)i * 2u);
        if (prior_size > size - payload_offset) return 0;
        payload_offset += prior_size;
    }
    compressed_size = rd16le(data + compressed_table + (size_t)item_index * 2u);
    width = rd16le(data + dimensions_table + (size_t)item_index * 4u);
    height = rd16le(data + dimensions_table + (size_t)item_index * 4u + 2u);
    if (compressed_size < 4u || compressed_size > size - payload_offset ||
        width == 0u || height == 0u || (size_t)width * height > pixel_capacity) {
        return 0;
    }
    if (!csb_v1_fmtowns_img2_decode(data + payload_offset, compressed_size,
                                    width, height, indexed_pixels,
                                    pixel_capacity, receipt)) {
        return 0;
    }
    if (receipt) receipt->container_offset = payload_offset;
    return 1;
}

int csb_v1_fmtowns_graphics_copy_raw_item(
    const uint8_t *data, size_t size, uint16_t item_index,
    uint8_t *raw_bytes, size_t raw_capacity,
    CSB_V1_FmtownsItemDecodeReceipt *receipt)
{
    uint16_t count;
    size_t compressed_table;
    size_t decompressed_table;
    size_t payload_start;
    size_t payload_offset;
    uint16_t compressed_size;
    uint16_t decompressed_size;
    uint16_t index;

    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!data || !raw_bytes || !csb_v1_fmtowns_graphics_probe(data, size)) {
        return 0;
    }
    count = rd16le(data + 2u);
    if (item_index >= count) return 0;
    compressed_table = 4u;
    decompressed_table = compressed_table + (size_t)count * 2u;
    payload_start = decompressed_table + (size_t)count * 2u +
                    (size_t)count * 4u;
    if (payload_start > size) return 0;
    payload_offset = payload_start;
    for (index = 0u; index < item_index; ++index) {
        uint16_t prior_size = rd16le(data + compressed_table +
                                     (size_t)index * 2u);
        if (prior_size > size - payload_offset) return 0;
        payload_offset += prior_size;
    }
    compressed_size = rd16le(data + compressed_table +
                             (size_t)item_index * 2u);
    decompressed_size = rd16le(data + decompressed_table +
                               (size_t)item_index * 2u);
    if (compressed_size == 0u || compressed_size != decompressed_size ||
        compressed_size > raw_capacity ||
        compressed_size > size - payload_offset) return 0;
    memcpy(raw_bytes, data + payload_offset, compressed_size);
    if (receipt) {
        receipt->valid = 1;
        receipt->stream_byte_count = compressed_size;
        receipt->stream_bytes_consumed = compressed_size;
        receipt->container_offset = payload_offset;
        receipt->pixel_count = compressed_size;
        receipt->stream_fnv1a = fnv1a(raw_bytes, compressed_size);
        receipt->pixel_fnv1a = receipt->stream_fnv1a;
        receipt->is_data_record = 1;
    }
    return 1;
}
