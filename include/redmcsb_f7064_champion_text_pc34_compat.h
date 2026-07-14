#ifndef REDMCSB_F7064_CHAMPION_TEXT_PC34_COMPAT_H
#define REDMCSB_F7064_CHAMPION_TEXT_PC34_COMPAT_H

#include <stdint.h>

#define REDMCSB_F7064_CHAMPION_NAME_BYTES 8U
#define REDMCSB_F7064_CHAMPION_TITLE_BYTES 20U

/* ReDMCSB CEDTINCQ.C F7064, PC34 fixed champion text fields. */
void redmcsb_f7064_pad_champion_name_and_title_pc34(
    uint8_t (*names)[REDMCSB_F7064_CHAMPION_NAME_BYTES],
    uint8_t (*titles)[REDMCSB_F7064_CHAMPION_TITLE_BYTES],
    uint16_t champion_count);

const char *redmcsb_f7064_champion_text_pc34_source_evidence(void);

#endif
