#include "csb_v1_fmtowns_portrait.h"
#include <string.h>

static uint16_t be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
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

int csb_v1_fmtowns_portrait_probe(const uint8_t *data, size_t size) {
    uint16_t id;
    if (!data || size != CSB_FMTOWNS_PORTRAIT_FILE_SIZE) return 0;
    /* ReDMCSB CEDT001.C F7002_ReadCMP (lines 197-210): F31 first swaps
     * each header word, then requires Magic == 0x91A7, cmp_ui_6 == 1 and
     * cmp_ui_8 without bit 0x8000.  The retail bytes are big-endian, so
     * validate their pre-swap representation directly.  Merely accepting
     * the magic used to let the C06 file-picker expose portraits the native
     * Load Champions transaction would subsequently reject. */
    id = be16(data);
    return id == 0x91a7u &&
           be16(data + 6u) == 1u &&
           (be16(data + 8u) & 0x8000u) == 0u;
}

int csb_v1_fmtowns_portrait_decode_planar(const uint8_t *img,
                                           size_t planar_size,
                                           uint8_t *indexed_pixels,
                                           size_t pixel_capacity) {
    unsigned int x;
    unsigned int y;

    if (!img || planar_size != CSB_FMTOWNS_PORTRAIT_DATA_SIZE ||
        !indexed_pixels ||
        pixel_capacity < CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT) return 0;

    /* ReDMCSB PORTRAIT.C F7251 (MEDIA670_F31E_F31J): each 16-pixel
     * group is four big-endian Atari ST plane words. The converter writes
     * its chunky F31 bitmap with pixel 0 in the low nibble and pixel 1 in
     * the high nibble (the final nibble swap in F7251). Decode directly to
     * one byte per source pixel, preserving the F7276 even/odd convention
     * in CEDTINCO.C:180-190. */
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

    return 1;
}

int csb_v1_fmtowns_portrait_encode_planar(const uint8_t *indexed_pixels,
                                           size_t pixel_count,
                                           uint8_t *planar_bytes,
                                           size_t planar_capacity) {
    unsigned int x;
    unsigned int y;

    if (!indexed_pixels || pixel_count != CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT ||
        !planar_bytes || planar_capacity < CSB_FMTOWNS_PORTRAIT_DATA_SIZE)
        return 0;
    memset(planar_bytes, 0, CSB_FMTOWNS_PORTRAIT_DATA_SIZE);
    /* ReDMCSB PORTRAIT.C F7252, inverse of F7251: preserve the big-endian
     * four-plane groups that C06 writes to a real F31 portrait record. */
    for (y = 0u; y < CSB_FMTOWNS_PORTRAIT_HEIGHT; ++y) {
        for (x = 0u; x < CSB_FMTOWNS_PORTRAIT_WIDTH; ++x) {
            const size_t plane_group =
                (size_t)y * 16u + (size_t)(x >> 4) * 8u;
            const uint16_t bit = (uint16_t)(0x8000u >> (x & 15u));
            const uint8_t color = indexed_pixels[
                (size_t)y * CSB_FMTOWNS_PORTRAIT_WIDTH + x];
            unsigned int plane;
            if (color > 15u) return 0;
            for (plane = 0u; plane < 4u; ++plane) {
                if ((color & (uint8_t)(1u << plane)) != 0u) {
                    uint8_t *word = planar_bytes + plane_group + plane * 2u;
                    uint16_t value = (uint16_t)(((uint16_t)word[0] << 8u) |
                                                word[1]);
                    value = (uint16_t)(value | bit);
                    word[0] = (uint8_t)(value >> 8u);
                    word[1] = (uint8_t)value;
                }
            }
        }
    }
    return 1;
}

int csb_v1_fmtowns_portrait_decode(const uint8_t *data, size_t size,
                                    uint8_t *indexed_pixels,
                                    size_t pixel_capacity,
                                    CSB_V1_FmtownsPortraitReceipt *receipt) {
    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (!csb_v1_fmtowns_portrait_probe(data, size) ||
        !csb_v1_fmtowns_portrait_decode_planar(
            data + CSB_FMTOWNS_PORTRAIT_HEADER_SIZE,
            CSB_FMTOWNS_PORTRAIT_DATA_SIZE, indexed_pixels, pixel_capacity)) {
        return 0;
    }
    if (receipt) {
        receipt->valid = 1;
        /* Keep the historic receipt's wire-order identifier convention;
         * C06 admission itself above uses the post-swap 0x91A7 value. */
        receipt->identifier = (uint16_t)(data[0] | ((uint16_t)data[1] << 8u));
        memcpy(receipt->name, data + 16, CSB_FMTOWNS_PORTRAIT_NAME_LEN);
        receipt->name[CSB_FMTOWNS_PORTRAIT_NAME_LEN] = '\0';
        memcpy(receipt->title, data + 24, CSB_FMTOWNS_PORTRAIT_TITLE_LEN);
        receipt->title[CSB_FMTOWNS_PORTRAIT_TITLE_LEN] = '\0';
        receipt->pixel_fnv1a = fnv1a(indexed_pixels,
                                     CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT);
    }
    return 1;
}
