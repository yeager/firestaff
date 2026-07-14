#ifndef REDMCSB_F7067_PORTRAIT_INFO_PC34_COMPAT_H
#define REDMCSB_F7067_PORTRAIT_INFO_PC34_COMPAT_H

#include "redmcsb_f7065_portrait_slots_pc34_compat.h"

#include <stdint.h>

/* ReDMCSB CEDT007.C F7067/F7068 C31_CHAMPION_INFO_PORTRAIT access. */

int redmcsb_f7067_get_champion_portrait_pc34(
    uint8_t *const *portrait_slots, uint16_t champion_count,
    uint16_t champion_format, uint16_t champion_index, uint8_t **portrait);

int redmcsb_f7068_set_champion_portrait_pc34(
    uint8_t **portrait_slots, uint16_t champion_count, uint16_t champion_format,
    uint16_t champion_index, uint8_t *portrait);

const char *redmcsb_f7067_portrait_info_pc34_source_evidence(void);

#endif
