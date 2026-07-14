#include "redmcsb_f7065_portrait_slots_pc34_compat.h"

void redmcsb_f7065_clear_portrait_slots_before_save_pc34(
    uint8_t **portrait_slots, uint16_t portrait_count, uint16_t champion_format)
{
    uint16_t portrait_index;

    if (champion_format != REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED ||
        portrait_count == 0U) {
        return;
    }

    for (portrait_index = 0; portrait_index < portrait_count; ++portrait_index) {
        portrait_slots[portrait_index] = NULL;
    }
}

int redmcsb_f7066_bind_portrait_slots_after_load_pc34(
    uint8_t **portrait_slots, uint16_t portrait_count, uint16_t champion_format,
    uint8_t *portrait_bytes, size_t portrait_bytes_size, uint16_t portrait_byte_count)
{
    uint16_t portrait_index;
    size_t required_bytes;

    if (champion_format != REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED ||
        portrait_count == 0U) {
        return 1;
    }
    if (portrait_slots == NULL || portrait_bytes == NULL || portrait_byte_count == 0U) {
        return 0;
    }
    required_bytes = (size_t)portrait_count * portrait_byte_count;
    if (required_bytes > portrait_bytes_size) {
        return 0;
    }

    for (portrait_index = 0; portrait_index < portrait_count; ++portrait_index) {
        portrait_slots[portrait_index] =
            portrait_bytes + (size_t)portrait_index * portrait_byte_count;
    }
    return 1;
}

const char *redmcsb_f7065_portrait_slots_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINCS.C F7065/F7066";
}
