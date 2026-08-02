#ifndef DM1_V1_FMTOWNS_GRAPHICS_DAT_H
#define DM1_V1_FMTOWNS_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FM Towns DM1 GRAPHICS.DAT classifier and admission gate.
 *
 * The FM Towns release (Fujitsu, 1989-11, HMA-240) ships a GRAPHICS.DAT
 * that uses the legacy format (no 0x8001 new-format marker). The file
 * has 575 graphics, all uncompressed (comp == decomp), with width/height
 * embedded at the start of each graphic's data rather than in a header
 * array. The file is ~397 KB vs PC 3.4's ~363 KB with 713 graphics.
 *
 * Both English (DATA/) and Japanese (JDATA/) variants exist on the disc.
 * The English GRAPHICS.DAT is 396970 bytes; the Japanese is 396407.
 *
 * Source: extracted from redump BIN/CUE "Dungeon Master (1987)(FTL)(Jp-En)"
 * ISO 9660 data track. System ID "HMA-240", volume ID "DUNGEON". */

#define DM1_FMTOWNS_GRAPHICS_EN_EXPECTED_SIZE   396970U
#define DM1_FMTOWNS_GRAPHICS_JP_EXPECTED_SIZE   396407U
#define DM1_FMTOWNS_GRAPHICS_MIN_SIZE           390000U
#define DM1_FMTOWNS_GRAPHICS_MAX_SIZE           400000U
#define DM1_FMTOWNS_GRAPHICS_EXPECTED_COUNT     575U

typedef enum {
    DM1_FMTOWNS_LANG_UNKNOWN = 0,
    DM1_FMTOWNS_LANG_EN      = 1,
    DM1_FMTOWNS_LANG_JP      = 2
} DM1_V1_FmtownsLang;

typedef struct {
    int              is_fmtowns;
    uint16_t         graphic_count;
    uint32_t         file_size;
    DM1_V1_FmtownsLang lang;
    uint8_t          md5[16];
} DM1_V1_FmtownsGraphicsReceipt;

/* Probe whether a GRAPHICS.DAT buffer is the FM Towns DM1 version.
 * Returns 1 if the format is legacy (no 0x8001 marker), uncompressed,
 * count is 575, and size is in the expected range. */
int dm1_v1_fmtowns_graphics_probe(const uint8_t *data, size_t size);

/* Build a receipt describing the FM Towns DM1 GRAPHICS.DAT.
 * Returns 0 on success, -1 if the data is not a valid FM Towns DM1 GRAPHICS.DAT. */
int dm1_v1_fmtowns_graphics_receipt(const uint8_t *data, size_t size,
                                    DM1_V1_FmtownsGraphicsReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_GRAPHICS_DAT_H */
