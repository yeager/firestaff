#ifndef REDMCSB_F7065_PORTRAIT_SLOTS_PC34_COMPAT_H
#define REDMCSB_F7065_PORTRAIT_SLOTS_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#define REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_INCLUDED 1U
#define REDMCSB_F7065_CHAMPION_FORMAT_PORTRAITS_EXCLUDED 2U

/* ReDMCSB CEDTINCS.C F7065/F7066 portrait-pointer save lifecycle. */

void redmcsb_f7065_clear_portrait_slots_before_save_pc34(
    uint8_t **portrait_slots, uint16_t portrait_count, uint16_t champion_format);

int redmcsb_f7066_bind_portrait_slots_after_load_pc34(
    uint8_t **portrait_slots, uint16_t portrait_count, uint16_t champion_format,
    uint8_t *portrait_bytes, size_t portrait_bytes_size, uint16_t portrait_byte_count);

const char *redmcsb_f7065_portrait_slots_pc34_source_evidence(void);

#endif
