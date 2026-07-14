#include "redmcsb_f7067_portrait_info_pc34_compat.h"

static int has_source_format(uint16_t champion_format)
{
    return champion_format == REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED ||
           champion_format == REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED;
}

int redmcsb_f7067_get_champion_portrait_pc34(
    uint8_t *const *portrait_slots, uint16_t champion_count,
    uint16_t champion_format, uint16_t champion_index, uint8_t **portrait)
{
    if (portrait_slots == NULL || portrait == NULL || !has_source_format(champion_format) ||
        champion_index >= champion_count) {
        return 0;
    }
    *portrait = portrait_slots[champion_index];
    return 1;
}

int redmcsb_f7068_set_champion_portrait_pc34(
    uint8_t **portrait_slots, uint16_t champion_count, uint16_t champion_format,
    uint16_t champion_index, uint8_t *portrait)
{
    if (portrait_slots == NULL || !has_source_format(champion_format) ||
        champion_index >= champion_count) {
        return 0;
    }
    portrait_slots[champion_index] = portrait;
    return 1;
}

const char *redmcsb_f7067_portrait_info_pc34_source_evidence(void)
{
    return "ReDMCSB CEDT007.C F7067/F7068 C31_CHAMPION_INFO_PORTRAIT";
}
