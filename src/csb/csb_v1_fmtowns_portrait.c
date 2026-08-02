#include "csb_v1_fmtowns_portrait.h"
#include <string.h>

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

int csb_v1_fmtowns_portrait_probe(const uint8_t *data, size_t size) {
    uint16_t id;
    if (!data || size != CSB_FMTOWNS_PORTRAIT_FILE_SIZE) return 0;
    id = (uint16_t)(data[0] | ((uint16_t)data[1] << 8));
    return id == CSB_FMTOWNS_PORTRAIT_IDENTIFIER;
}

int csb_v1_fmtowns_portrait_decode(const uint8_t *data, size_t size,
                                    uint8_t *indexed_pixels,
                                    size_t pixel_capacity,
                                    CSB_V1_FmtownsPortraitReceipt *receipt) {
    const uint8_t *img;
    size_t i;

    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!csb_v1_fmtowns_portrait_probe(data, size)) return 0;
    if (!indexed_pixels ||
        pixel_capacity < CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT) return 0;

    /* Extract name and title */
    if (receipt) {
        receipt->valid = 1;
        receipt->identifier = (uint16_t)(data[0] | ((uint16_t)data[1] << 8));
        memcpy(receipt->name, data + 16, CSB_FMTOWNS_PORTRAIT_NAME_LEN);
        receipt->name[CSB_FMTOWNS_PORTRAIT_NAME_LEN] = '\0';
        memcpy(receipt->title, data + 24, CSB_FMTOWNS_PORTRAIT_TITLE_LEN);
        receipt->title[CSB_FMTOWNS_PORTRAIT_TITLE_LEN] = '\0';
    }

    /* Unpack 4bpp pixels: high nibble first */
    img = data + CSB_FMTOWNS_PORTRAIT_HEADER_SIZE;
    for (i = 0; i < CSB_FMTOWNS_PORTRAIT_DATA_SIZE; i++) {
        indexed_pixels[i * 2]     = (img[i] >> 4) & 0x0fu;
        indexed_pixels[i * 2 + 1] = img[i] & 0x0fu;
    }

    if (receipt) {
        receipt->pixel_fnv1a = fnv1a(indexed_pixels,
                                     CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT);
    }
    return 1;
}
