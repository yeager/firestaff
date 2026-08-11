#include "csb_v1_x68k_graphics_handoff.h"

#include "csb_v1_amiga_graphics_dat.h"

#include <stdlib.h>
#include <string.h>

static int extract_graphics(const uint8_t *hdm, size_t hdm_size,
                            uint8_t **out_bytes, size_t *out_size,
                            CSB_V1_X68kHdmReceipt *out_media) {
    uint8_t *bytes;
    size_t size = 0u;

    if (!out_bytes || !out_size ||
        !csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size, "GRAPHICS.DAT",
                                            NULL, 0u, &size, out_media) ||
        size == 0u) return 0;
    bytes = malloc(size);
    if (!bytes || !csb_v1_x68k_hdm_extract_root_file(hdm, hdm_size,
                                                      "GRAPHICS.DAT", bytes,
                                                      size, &size, out_media)) {
        free(bytes);
        return 0;
    }
    *out_bytes = bytes;
    *out_size = size;
    return 1;
}

int csb_v1_x68k_hdm_graphics_receipt(const uint8_t *hdm, size_t hdm_size,
                                     CSB_V1_X68kGraphicsReceipt *out) {
    uint8_t *bytes = NULL;
    size_t size = 0u;
    CSB_V1_X68kGraphicsReceipt receipt;
    uint16_t index;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    memset(&receipt, 0, sizeof(receipt));
    if (!extract_graphics(hdm, hdm_size, &bytes, &size, &receipt.media) ||
        !csb_v1_amiga_graphics_probe(bytes, size)) {
        free(bytes);
        return 0;
    }
    receipt.item_count = (uint16_t)(((uint16_t)bytes[2] << 8) | bytes[3]);
    receipt.graphics_byte_count = (uint32_t)size;
    for (index = 0u; index < receipt.item_count; ++index) {
        CSB_V1_AmigaGraphicsItem item;
        if (!csb_v1_amiga_graphics_item(bytes, size, index, &item)) {
            free(bytes);
            return 0;
        }
        if (item.compressedByteCount == item.decompressedByteCount)
            ++receipt.direct_item_count;
    }
    free(bytes);
    *out = receipt;
    return 1;
}

int csb_v1_x68k_hdm_graphics_item(const uint8_t *hdm, size_t hdm_size,
                                  uint16_t item_index,
                                  CSB_V1_X68kGraphicsItem *out) {
    uint8_t *bytes = NULL;
    size_t size = 0u;
    CSB_V1_AmigaGraphicsItem source;
    int result = 0;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!extract_graphics(hdm, hdm_size, &bytes, &size, NULL) ||
        !csb_v1_amiga_graphics_item(bytes, size, item_index, &source)) {
        free(bytes);
        return 0;
    }
    out->stored_byte_count = source.compressedByteCount;
    out->decoded_byte_count = source.decompressedByteCount;
    out->data_offset = source.dataOffset;
    result = 1;
    free(bytes);
    return result;
}

int csb_v1_x68k_hdm_graphics_decode_item(const uint8_t *hdm, size_t hdm_size,
                                         uint16_t item_index,
                                         uint8_t *indexed_pixels,
                                         size_t indexed_pixel_capacity,
                                         uint16_t *out_width,
                                         uint16_t *out_height) {
    uint8_t *bytes = NULL;
    size_t size = 0u;
    int result;

    if (out_width) *out_width = 0u;
    if (out_height) *out_height = 0u;
    if (!indexed_pixels ||
        !extract_graphics(hdm, hdm_size, &bytes, &size, NULL)) return 0;
    result = csb_v1_amiga_graphics_decode_item(bytes, size, item_index,
                                               indexed_pixels,
                                               indexed_pixel_capacity,
                                               out_width, out_height);
    free(bytes);
    return result;
}
