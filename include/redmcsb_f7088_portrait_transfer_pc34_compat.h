#ifndef REDMCSB_F7088_PORTRAIT_TRANSFER_PC34_COMPAT_H
#define REDMCSB_F7088_PORTRAIT_TRANSFER_PC34_COMPAT_H

#include "redmcsb_f7067_portrait_info_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#define REDMCSB_F7088_PC34_PORTRAIT_COUNT 4U
#define REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT 464U

/* ReDMCSB CEDTINCR.C F7088 included-to-excluded portrait transfer. */

int redmcsb_f7088_copy_included_portraits_to_excluded_pc34(
    uint8_t *const *source_portrait_slots, uint16_t source_champion_count,
    uint16_t source_champion_format, uint8_t **destination_portrait_slots,
    uint16_t destination_champion_count, uint16_t destination_champion_format,
    uint8_t *destination_portrait_bytes, size_t destination_portrait_bytes_size,
    uint16_t destination_portrait_count, uint16_t destination_portrait_byte_count);

const char *redmcsb_f7088_portrait_transfer_pc34_source_evidence(void);

#endif
