#include "redmcsb_f7064_champion_text_pc34_compat.h"

static void pad_after_nul(uint8_t *text, uint16_t byte_count)
{
    uint16_t index;

    for (index = 0; index < byte_count; ++index) {
        if (text[index] == 0U) {
            break;
        }
    }
    while (index < byte_count) {
        text[index++] = 0U;
    }
}

void redmcsb_f7064_pad_champion_name_and_title_pc34(
    uint8_t (*names)[REDMCSB_F7064_CHAMPION_NAME_BYTES],
    uint8_t (*titles)[REDMCSB_F7064_CHAMPION_TITLE_BYTES],
    uint16_t champion_count)
{
    uint16_t champion_index;

    for (champion_index = 0; champion_index < champion_count; ++champion_index) {
        pad_after_nul(names[champion_index], REDMCSB_F7064_CHAMPION_NAME_BYTES);
        pad_after_nul(titles[champion_index], REDMCSB_F7064_CHAMPION_TITLE_BYTES);
    }
}

const char *redmcsb_f7064_champion_text_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINCQ.C F7064 PC34 fixed name/title padding";
}
