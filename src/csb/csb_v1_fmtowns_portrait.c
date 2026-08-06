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
    unsigned int x;
    unsigned int y;

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

    /* ReDMCSB PORTRAIT.C F7251 (MEDIA670_F31E_F31J): each 16-pixel
     * group is four big-endian Atari ST plane words. The converter writes
     * its chunky F31 bitmap with pixel 0 in the low nibble and pixel 1 in
     * the high nibble (the final nibble swap in F7251). Decode directly to
     * one byte per source pixel, preserving the F7276 even/odd convention
     * in CEDTINCO.C:180-190. */
    img = data + CSB_FMTOWNS_PORTRAIT_HEADER_SIZE;
    for (y = 0u; y < CSB_FMTOWNS_PORTRAIT_HEIGHT; ++y) {
        for (x = 0u; x < CSB_FMTOWNS_PORTRAIT_WIDTH; ++x) {
            const size_t plane_group =
                (size_t)y * 16u + (size_t)(x >> 4) * 8u;
            const uint16_t bit = (uint16_t)(0x8000u >> (x & 15u));
            uint8_t color = 0u;
            unsigned int plane;

            for (plane = 0u; plane < 4u; ++plane) {
                const uint8_t *word = img + plane_group + plane * 2u;
                const uint16_t plane_word =
                    (uint16_t)(((uint16_t)word[0] << 8u) | word[1]);
                if ((plane_word & bit) != 0u) {
                    color |= (uint8_t)(1u << plane);
                }
            }
            indexed_pixels[(size_t)y * CSB_FMTOWNS_PORTRAIT_WIDTH + x] =
                color;
        }
    }

    if (receipt) {
        receipt->pixel_fnv1a = fnv1a(indexed_pixels,
                                     CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT);
    }
    return 1;
}
