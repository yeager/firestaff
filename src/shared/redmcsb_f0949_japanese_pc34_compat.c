#include "redmcsb_f0949_japanese_pc34_compat.h"

#include <stdint.h>

int16_t redmcsb_f0949_japanese_pc34_compat(int16_t packed_character)
{
    uint16_t character = (uint16_t)packed_character;
    uint8_t lead = (uint8_t)(character >> 8);
    uint8_t trail = (uint8_t)character;

    if (lead >= UINT8_C(0xe0)) {
        lead = (uint8_t)(lead - UINT8_C(0x40));
    }
    lead = (uint8_t)(lead - UINT8_C(0x80));
    lead = (uint8_t)(lead + lead);

    if (trail > UINT8_C(0x9e)) {
        trail = (uint8_t)(trail - UINT8_C(0x7e));
    } else {
        lead = (uint8_t)(lead - UINT8_C(1));
        trail = (uint8_t)(trail -
                          ((trail >= UINT8_C(0x7f)) ? UINT8_C(0x20)
                                                     : UINT8_C(0x1f)));
    }
    lead = (uint8_t)(lead + UINT8_C(0x20));

    return (int16_t)(((uint16_t)lead << 8) | trail);
}

const char *redmcsb_f0949_japanese_source_evidence_pc34(void)
{
    return "ReDMCSB JAPANESE.C:15-34 guards F0949_JAPANESE_ with "
           "MEDIA459_P20JA_P20JB_P31J and transforms AX through AH/AL; "
           "JAPANESE.C:288-298 identifies Shift-JIS two-byte leads and "
           "passes their packed bytes to F0949_JAPANESE_; JAPANESE.C:315 "
           "passes the resulting code to F0950_JAPANESE_.";
}
