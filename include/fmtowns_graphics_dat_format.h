#ifndef FMTOWNS_GRAPHICS_DAT_FORMAT_H
#define FMTOWNS_GRAPHICS_DAT_FORMAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns GRAPHICS.DAT header format classifier.
 *
 * Byte-verified from all three FM Towns Dungeon Master discs:
 *
 *   DM1 GRAPHICS.DAT (396,970 bytes, sha d08049d0...)
 *     word0 = 0x023f (575 = asset count, legacy DM1 format)
 *     Layout: {u16 count, u16 sizes[count], payload}
 *
 *   CSB GRAPHICS.DAT (402,274 bytes, sha 08cceb0c...)
 *     word0 = 0x8001 (extended format v1)
 *     word1 = 0x02d8 (728 asset count)
 *     Layout: {u16 sig=0x8001, u16 count, u32 records[count], payload}
 *
 *   DM2 GRAPHICS.DAT (2,783,791 bytes, sha 634e7004...)
 *     word0 = 0x8004 (extended format v4)
 *     word1 = 0x0d4f (3407 asset count)
 *     Layout: {u16 sig=0x8004, u16 count, u32 records[count], payload}
 *
 * The high bit of word0 is a format flag:
 *   0x0xxx  -> legacy DM1 format, word0 is asset count
 *   0x8xxx  -> extended format, low 15 bits = version, word1 = count
 *
 * DM1's legacy format is fully decoded by
 * `dm1_v1_fmtowns_pic_library`. The extended CSB and DM2 formats
 * have their per-record layout still to be reverse-engineered
 * (a 4-byte record per asset instead of DM1's 2-byte size — bytes
 * 1..2 and 3..4 hold different fields whose semantics still need
 * per-asset round-trip verification).
 */

typedef enum {
    FMTOWNS_GRAPHICS_DAT_FORMAT_UNKNOWN  = 0,
    FMTOWNS_GRAPHICS_DAT_FORMAT_LEGACY   = 1,   /* DM1 */
    FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V1   = 2,   /* CSB */
    FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4   = 3    /* DM2 */
} fmtowns_graphics_dat_format_t;

typedef struct {
    fmtowns_graphics_dat_format_t format;
    uint16_t                       signature;
    uint16_t                       asset_count;
    uint32_t                       header_size_bytes;
    uint32_t                       record_size_bytes;
} fmtowns_graphics_dat_identity_t;

/* Classify a GRAPHICS.DAT blob by inspecting the first 4 bytes and
 * cross-checking against the recovered per-game asset counts.
 * Returns 1 on success, 0 on invalid input or too-small blob.
 * Unknown formats populate `format = UNKNOWN` and record the raw
 * signature so callers can log what they saw. */
int fmtowns_graphics_dat_identify_pc34(
    const uint8_t                    *blob,
    size_t                            blob_size,
    fmtowns_graphics_dat_identity_t  *out);

/* Return the byte-verified expected asset count for a given game's
 * FM Towns GRAPHICS.DAT. 0 for unknown game or invalid enum. */
uint16_t fmtowns_graphics_dat_expected_asset_count_pc34(
    fmtowns_graphics_dat_format_t format);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_GRAPHICS_DAT_FORMAT_H */
