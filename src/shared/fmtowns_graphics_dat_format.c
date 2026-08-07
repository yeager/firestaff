#include "fmtowns_graphics_dat_format.h"
#include <string.h>

int fmtowns_graphics_dat_identify_pc34(
        const uint8_t *blob, size_t blob_size,
        fmtowns_graphics_dat_identity_t *out) {
    if (!blob || !out) return 0;
    if (blob_size < 4) return 0;
    memset(out, 0, sizeof(*out));
    uint16_t w0 = (uint16_t)(blob[0] | (blob[1] << 8));
    uint16_t w1 = (uint16_t)(blob[2] | (blob[3] << 8));
    out->signature = w0;
    if ((w0 & 0x8000u) == 0u) {
        /* Legacy DM1 format: word0 = asset count. */
        out->format = FMTOWNS_GRAPHICS_DAT_FORMAT_LEGACY;
        out->asset_count = w0;
        out->record_size_bytes = 2;   /* u16 size per asset */
        out->header_size_bytes = 2 + (uint32_t)w0 * 2;
        return 1;
    }
    /* Extended format: word0 = 0x80xx sig, word1 = count. */
    out->asset_count = w1;
    out->record_size_bytes = 4;   /* 4-byte record per asset */
    out->header_size_bytes = 4 + (uint32_t)w1 * 4;
    if (w0 == 0x8001u) {
        out->format = FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V1;
        return 1;
    }
    if (w0 == 0x8004u) {
        out->format = FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4;
        return 1;
    }
    out->format = FMTOWNS_GRAPHICS_DAT_FORMAT_UNKNOWN;
    return 0;
}

uint16_t fmtowns_graphics_dat_expected_asset_count_pc34(
        fmtowns_graphics_dat_format_t format) {
    switch (format) {
        case FMTOWNS_GRAPHICS_DAT_FORMAT_LEGACY: return 575;  /* DM1 */
        case FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V1: return 728;  /* CSB */
        case FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4: return 3407; /* DM2 */
        default: return 0;
    }
}
