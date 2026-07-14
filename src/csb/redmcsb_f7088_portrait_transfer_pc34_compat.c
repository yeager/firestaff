#include "redmcsb_f7088_portrait_transfer_pc34_compat.h"

#include <string.h>

int redmcsb_f7088_copy_included_portraits_to_excluded_pc34(
    uint8_t *const *source_portrait_slots, uint16_t source_champion_count,
    uint16_t source_champion_format, uint8_t **destination_portrait_slots,
    uint16_t destination_champion_count, uint16_t destination_champion_format,
    uint8_t *destination_portrait_bytes, size_t destination_portrait_bytes_size,
    uint16_t destination_portrait_count, uint16_t destination_portrait_byte_count)
{
    uint16_t portrait_index;
    size_t required_bytes;

    required_bytes = (size_t)REDMCSB_F7088_PC34_PORTRAIT_COUNT *
                     REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT;
    if (source_portrait_slots == NULL || destination_portrait_slots == NULL ||
        destination_portrait_bytes == NULL ||
        source_champion_format != REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED ||
        destination_champion_format != REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED ||
        source_champion_count < REDMCSB_F7088_PC34_PORTRAIT_COUNT ||
        destination_champion_count < REDMCSB_F7088_PC34_PORTRAIT_COUNT ||
        destination_portrait_count != REDMCSB_F7088_PC34_PORTRAIT_COUNT ||
        destination_portrait_byte_count != REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT ||
        destination_portrait_bytes_size < required_bytes) {
        return 0;
    }

    for (portrait_index = 0U;
         portrait_index < REDMCSB_F7088_PC34_PORTRAIT_COUNT;
         ++portrait_index) {
        if (source_portrait_slots[portrait_index] == NULL) {
            return 0;
        }
    }

    for (portrait_index = 0U;
         portrait_index < REDMCSB_F7088_PC34_PORTRAIT_COUNT;
         ++portrait_index) {
        memcpy(destination_portrait_bytes +
                   (size_t)portrait_index * REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT,
               source_portrait_slots[portrait_index],
               REDMCSB_F7088_PC34_PORTRAIT_BYTE_COUNT);
    }
    return redmcsb_f7066_bind_portrait_slots_after_load_pc34(
        destination_portrait_slots, destination_portrait_count,
        destination_champion_format, destination_portrait_bytes,
        destination_portrait_bytes_size, destination_portrait_byte_count);
}

const char *redmcsb_f7088_portrait_transfer_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINCR.C F7088 C1-to-C2 portrait transfer";
}
