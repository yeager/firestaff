#include "redmcsb_f0818_is_japanese_text_pc34_compat.h"

int redmcsb_f0818_is_japanese_text_pc34_compat(const uint8_t *string)
{
    uint8_t character;

    if (*string == 0x1BU) {
        return 1;
    }
    while ((character = *string++) != 0U) {
        if ((character & 0x80U) != 0U) {
            return 1;
        }
    }
    return 0;
}

const char *redmcsb_f0818_is_japanese_text_source_evidence_pc34(void)
{
    return "ReDMCSB CEDT030.C:1207-1220; DEFS.H PC-98 MEDIA689_F31J_X31J_P31J";
}
