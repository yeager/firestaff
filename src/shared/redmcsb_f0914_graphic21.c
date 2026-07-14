#include "redmcsb_f0914_graphic21.h"

static int16_t redmcsb_f0914_graphic21_run(
    const uint16_t *sector_buffer,
    const redmcsb_f0914_graphic21_result *result,
    uint16_t first_word_index,
    uint16_t last_word_index)
{
    uint16_t byte_index;
    uint16_t counter = UINT16_C(0);
    uint16_t fuzzy_bit_count = UINT16_C(0);
    uint16_t fuzzy_bits = UINT16_C(0);
    int16_t differing_word_count = INT16_C(0);

    for (byte_index = first_word_index; byte_index <= last_word_index;
         ++byte_index) {
        fuzzy_bits = (uint16_t)((uint16_t)(fuzzy_bits << 1U) |
                                ((sector_buffer[byte_index] >> 14U) & 1U));
        if ((fuzzy_bit_count++ == UINT16_C(16)) ||
            (byte_index == last_word_index)) {
            fuzzy_bit_count = UINT16_C(0);
            if (fuzzy_bits != result->fuzzy_bits[counter]) {
                ++differing_word_count;
            }
            result->fuzzy_bits[counter++] = fuzzy_bits;
            fuzzy_bits = UINT16_C(0);
        }
    }

    if (differing_word_count == INT16_C(0)) {
        *result->last_event22_time = INT32_C(0);
    }
    *result->check_last_event22_time =
        REDMCSB_F0914_GRAPHIC21_CHECK_TIME_VALUE;
    *result->fuzzy_sector_analyzed = REDMCSB_F0914_GRAPHIC21_ANALYZED_VALUE;

    return differing_word_count;
}

int16_t redmcsb_f0914_graphic21_a20e(
    const uint16_t *sector_buffer,
    const redmcsb_f0914_graphic21_result *result)
{
    return redmcsb_f0914_graphic21_run(
        sector_buffer, result, REDMCSB_F0914_GRAPHIC21_A20E_FIRST_WORD_INDEX,
        REDMCSB_F0914_GRAPHIC21_A20E_LAST_WORD_INDEX);
}

int16_t redmcsb_f0914_graphic21_a31e(
    const uint16_t *sector_buffer,
    const redmcsb_f0914_graphic21_result *result)
{
    return redmcsb_f0914_graphic21_run(
        sector_buffer, result, REDMCSB_F0914_GRAPHIC21_A31E_FIRST_WORD_INDEX,
        REDMCSB_F0914_GRAPHIC21_A31E_LAST_WORD_INDEX);
}

const char *redmcsb_f0914_graphic21_source_evidence(void)
{
    return "ReDMCSB GRAPH21.C:170-204 defines MEDIA432_A20E F0914 "
           "over sector words 20..508; GRAPH21.C:207-241 defines "
           "MEDIA618_A31E over words 635..1123. Both assemble bit 14, "
           "commit when fuzzy_bit_count++ == 16 or at the final word, "
           "clear LastEvent22Time only for zero differences, then write "
           "C00512_FALSE (512) and C00136_TRUE (136).";
}
