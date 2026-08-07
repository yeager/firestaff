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
    if (w0 == 0x8001u) {
        /* CSB ext_v1: identical to legacy DM1 (two-table format) but
         * with a 2-byte signature prefix. Payload = 2 * count u16
         * sizes (primary + secondary tables, byte-identical), then
         * consecutive asset bytes summing to primary_sum. Verified
         * against real CSB CDATA/GRAPHICS.DAT: primary sum equals
         * secondary sum, header + payload fits file. */
        out->format = FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V1;
        out->record_size_bytes = 2;   /* per size-table entry */
        out->header_size_bytes = 4 + (uint32_t)w1 * 4;
        return 1;
    }
    if (w0 == 0x8005u) {
        /* DM2 DOS ext_v5: byte-verified 2026-08-07 against the
         * shipping DOS DM2 GRAPHICS.DAT (8 639 757 bytes, count
         * 5624). Same 4-byte record stride as v4; per-record
         * decode still open. */
        out->format = FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V5;
        out->record_size_bytes = 4;
        out->header_size_bytes = 4 + (uint32_t)w1 * 4;
        return 1;
    }
    if (w0 == 0x8004u) {
        /* DM2 ext_v4: different per-record layout — {u16 size,
         * u16 flags/aux} per asset. Sum of primary sizes does NOT
         * balance the file directly; per-record decode still open.
         * Format classifier reports the 4-byte record stride so
         * consumers can walk records but MUST NOT assume the
         * sum-check that works for legacy/v1. */
        out->format = FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4;
        out->record_size_bytes = 4;
        out->header_size_bytes = 4 + (uint32_t)w1 * 4;
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
        case FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4: return 3407; /* DM2 FM Towns */
        case FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V5: return 5624; /* DM2 DOS */
        default: return 0;
    }
}
