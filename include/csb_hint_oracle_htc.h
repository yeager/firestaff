/*
 * csb_hint_oracle_htc.h
 *
 * Read-only parser for the Chaos Strikes Back Utility Disk HCSB.HTC
 * Hint Oracle text/layout file.
 *
 * Format sources:
 *   - dmweb Hint Oracle Files format page: HCSB.HTC big-endian layout
 *   - ReDMCSB HINTLOAD.C:11-18 names HCSB.HTC/HCSB.DAT/CSBGAME.DAT
 *   - ReDMCSB HINTHTC.C:177-358 validates format 2, dungeon id 13,
 *     reads location records, hint records, page byte counts, and stores
 *     the compressed content offset
 *   - ReDMCSB HINTLZW.C:122-212 decompresses hint content on demand
 *
 * This is a format-contract module only. It does not draw the Hint Oracle
 * UI or bind HCSB.HTC into CSB runtime launch flow.
 */
#ifndef FIRESTAFF_CSB_HINT_ORACLE_HTC_H
#define FIRESTAFF_CSB_HINT_ORACLE_HTC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_HINT_ORACLE_HTC_FORMAT_WORD 2u
#define CSB_HINT_ORACLE_HTC_DUNGEON_ID 13u
#define CSB_HINT_ORACLE_HTC_LOCATION_RECORD_SIZE 6u
#define CSB_HINT_ORACLE_HTC_HINT_RECORD_SIZE 26u
#define CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES 22u
#define CSB_HINT_ORACLE_HTC_ANY_XY 255u

typedef enum {
    CSB_HINT_ORACLE_HTC_OK = 0,
    CSB_HINT_ORACLE_HTC_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_HTC_ERR_TRUNCATED = -2,
    CSB_HINT_ORACLE_HTC_ERR_BAD_FORMAT = -3,
    CSB_HINT_ORACLE_HTC_ERR_BAD_DUNGEON = -4,
    CSB_HINT_ORACLE_HTC_ERR_BAD_HEADER = -5,
    CSB_HINT_ORACLE_HTC_ERR_BAD_RECORD_SIZE = -6,
    CSB_HINT_ORACLE_HTC_ERR_BAD_HINT_RANGE = -7,
    CSB_HINT_ORACLE_HTC_ERR_BAD_CONTENT_SIZE = -8,
    CSB_HINT_ORACLE_HTC_ERR_OUTPUT_TOO_SMALL = -9,
    CSB_HINT_ORACLE_HTC_ERR_BAD_LZW = -10
} CSB_HintOracleHTC_Result;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t level;
    uint8_t unused;
    uint16_t hint_index;
} CSB_HintOracleHTC_Location;

typedef struct {
    char name[CSB_HINT_ORACLE_HTC_HINT_NAME_BYTES + 1u];
    uint16_t first_page_index;
    uint16_t page_count;
} CSB_HintOracleHTC_Hint;

typedef struct {
    uint16_t format_word;
    uint16_t dungeon_id;
    uint16_t header_word_count;
    uint16_t location_record_size;
    uint16_t hint_record_size;

    size_t location_count;
    size_t hint_count;
    size_t page_count;
    size_t content_offset;
    size_t content_size;

    const uint8_t *data;
    size_t data_size;
    const uint8_t *locations;
    const uint8_t *hints;
    const uint8_t *page_lengths;
    const uint8_t *contents;
} CSB_HintOracleHTC;

int csb_hint_oracle_htc_parse(const uint8_t *data,
                              size_t data_size,
                              CSB_HintOracleHTC *out);

int csb_hint_oracle_htc_get_location(
    const CSB_HintOracleHTC *htc,
    size_t index,
    CSB_HintOracleHTC_Location *out_location);

int csb_hint_oracle_htc_get_hint(const CSB_HintOracleHTC *htc,
                                 size_t index,
                                 CSB_HintOracleHTC_Hint *out_hint);

uint16_t csb_hint_oracle_htc_page_compressed_length(
    const CSB_HintOracleHTC *htc,
    size_t page_index);

int csb_hint_oracle_htc_find_hints_for_location(
    const CSB_HintOracleHTC *htc,
    uint8_t level,
    uint8_t x,
    uint8_t y,
    uint16_t *out_hint_indices,
    size_t out_hint_capacity,
    size_t *out_hint_count);

int csb_hint_oracle_htc_get_hint_content_slice(
    const CSB_HintOracleHTC *htc,
    size_t hint_index,
    const uint8_t **out_compressed,
    size_t *out_compressed_size);

int csb_hint_oracle_htc_lzw_decompress(const uint8_t *compressed,
                                       size_t compressed_size,
                                       uint8_t *out,
                                       size_t out_capacity,
                                       size_t *out_size);

const char *csb_hint_oracle_htc_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_HTC_H */
