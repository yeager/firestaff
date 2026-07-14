#ifndef REDMCSB_CEDTINC8_SAVE_PARTS_PC34_COMPAT_H
#define REDMCSB_CEDTINC8_SAVE_PARTS_PC34_COMPAT_H

#include <stdint.h>

#define REDMCSB_CEDTINC8_SAVE_PART_COUNT 5U
#define REDMCSB_CEDTINC8_SAVE_HEADER_KEY_COUNT 16U

typedef struct RedmcsbCedtinc8SavePart {
    uint8_t *plaintext;
    uint16_t byte_count;
    uint8_t *written_bytes;
} RedmcsbCedtinc8SavePart;

/*
 * CEDTINC8.C's five-part save sequence after its header-key preparation.
 * keys are source-owned header words; written_bytes receives the exact
 * obfuscated part bytes while plaintext is restored before return.
 */
int redmcsb_cedtinc8_prepare_save_parts_pc34(
    RedmcsbCedtinc8SavePart parts[REDMCSB_CEDTINC8_SAVE_PART_COUNT],
    const uint16_t keys[REDMCSB_CEDTINC8_SAVE_HEADER_KEY_COUNT],
    uint16_t checksums[REDMCSB_CEDTINC8_SAVE_PART_COUNT]);

const char *redmcsb_cedtinc8_save_parts_pc34_source_evidence(void);

#endif
