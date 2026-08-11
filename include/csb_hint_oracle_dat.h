/*
 * csb_hint_oracle_dat.h
 *
 * Read-only index for the original CSB Hint Oracle HCSB.DAT graphic
 * archive.  It deliberately stops at segment boundaries: decoding the
 * platform graphic stream is a separate, source-owned parity task.
 *
 * Format evidence:
 *   - ReDMCSB HINTGRAP.C F1872_LoadGraphics() reads the big-endian graphic
 *     count, converts segment sizes to offsets and starts data at
 *     (count * 4) + 2.
 *   - ReDMCSB HINTSCR.C loads graphic 0 (integer glyphs), graphic 1
 *     (Oracle bitmap) and graphic 2 (font) from HCSB.DAT.
 *
 * The on-disk header therefore has two identical big-endian 16-bit size
 * tables.  This module verifies both copies before exposing a segment.
 */
#ifndef FIRESTAFF_CSB_HINT_ORACLE_DAT_H
#define FIRESTAFF_CSB_HINT_ORACLE_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_HINT_ORACLE_DAT_MAX_SEGMENTS 64u

typedef enum {
    CSB_HINT_ORACLE_DAT_OK = 0,
    CSB_HINT_ORACLE_DAT_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_DAT_ERR_TRUNCATED = -2,
    CSB_HINT_ORACLE_DAT_ERR_BAD_COUNT = -3,
    CSB_HINT_ORACLE_DAT_ERR_BAD_TABLE = -4,
    CSB_HINT_ORACLE_DAT_ERR_BAD_SIZE = -5
} CSB_HintOracleDAT_Result;

typedef struct {
    const uint8_t *data;
    size_t data_size;
    uint16_t segment_count;
    size_t payload_offset;
    size_t segment_offsets[CSB_HINT_ORACLE_DAT_MAX_SEGMENTS];
    uint16_t segment_sizes[CSB_HINT_ORACLE_DAT_MAX_SEGMENTS];
} CSB_HintOracleDAT;

int csb_hint_oracle_dat_parse(const uint8_t *data, size_t data_size,
                              CSB_HintOracleDAT *out);

int csb_hint_oracle_dat_get_segment(const CSB_HintOracleDAT *archive,
                                    size_t index,
                                    const uint8_t **out_bytes,
                                    size_t *out_size);

/* Decode an Atari/Amiga HCSB.DAT IMG2 segment into 4-bit indexed pixels.
 * The segment starts with BE16 width/height and uses ReDMCSB EXPAND.C
 * F0466 command bytes. No palette or display surface is implied. */
int csb_hint_oracle_dat_img2_decode(const uint8_t *segment,
                                    size_t segment_size,
                                    uint16_t *out_width,
                                    uint16_t *out_height,
                                    uint8_t *out_pixels,
                                    size_t out_capacity,
                                    size_t *out_bytes_consumed);

const char *csb_hint_oracle_dat_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_DAT_H */
