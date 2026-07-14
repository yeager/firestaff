#include <stdint.h>
#include <string.h>

#include "redmcsb_f0818_is_japanese_text_pc34_compat.h"

int main(void)
{
    static const uint8_t empty[] = {0U};
    static const uint8_t ascii[] = {'D', 'M', '1', 0U};
    static const uint8_t escape[] = {0x1BU, 'D', 'M', 0U};
    static const uint8_t high_bit[] = {'D', 0x82U, 'M', 0U};
    static const uint8_t escape_later[] = {'D', 0x1BU, 'M', 0U};

    if (redmcsb_f0818_is_japanese_text_pc34_compat(empty) != 0 ||
        redmcsb_f0818_is_japanese_text_pc34_compat(ascii) != 0 ||
        redmcsb_f0818_is_japanese_text_pc34_compat(escape) != 1 ||
        redmcsb_f0818_is_japanese_text_pc34_compat(high_bit) != 1 ||
        redmcsb_f0818_is_japanese_text_pc34_compat(escape_later) != 0) {
        return 1;
    }

    return strcmp(
               redmcsb_f0818_is_japanese_text_source_evidence_pc34(),
               "ReDMCSB CEDT030.C:1207-1220; DEFS.H PC-98 MEDIA689_F31J_X31J_P31J") != 0;
}
