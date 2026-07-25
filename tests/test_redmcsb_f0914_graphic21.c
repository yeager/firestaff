#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0914_graphic21.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static redmcsb_f0914_graphic21_result make_result(
    uint16_t fuzzy_bits[REDMCSB_F0914_GRAPHIC21_FUZZY_WORD_COUNT],
    int16_t *analyzed,
    int32_t *last_event22_time,
    int16_t *check_last_event22_time)
{
    redmcsb_f0914_graphic21_result result = {
        fuzzy_bits,
        analyzed,
        last_event22_time,
        check_last_event22_time
    };

    return result;
}

int main(void)
{
    uint16_t a20e_sector[509] = { 0U };
    uint16_t a31e_sector[1124] = { 0U };
    uint16_t fuzzy_bits[REDMCSB_F0914_GRAPHIC21_FUZZY_WORD_COUNT] = { 0U };
    int16_t analyzed = INT16_C(-1);
    int32_t last_event22_time = INT32_C(91);
    int16_t check_last_event22_time = INT16_C(-1);
    redmcsb_f0914_graphic21_result result = make_result(
    (void)result;
        fuzzy_bits, &analyzed, &last_event22_time, &check_last_event22_time);

    assert(redmcsb_f0914_graphic21_a20e(a20e_sector, &result) == 0);
    assert(last_event22_time == 0);
    assert(analyzed == REDMCSB_F0914_GRAPHIC21_ANALYZED_VALUE);
    assert(check_last_event22_time == REDMCSB_F0914_GRAPHIC21_CHECK_TIME_VALUE);

    last_event22_time = INT32_C(91);
    a20e_sector[36] = UINT16_C(0x4000);
    assert(redmcsb_f0914_graphic21_a20e(a20e_sector, &result) == 1);
    assert(fuzzy_bits[0] == 1U);
    assert(last_event22_time == INT32_C(91));

    memset(fuzzy_bits, 0, sizeof(fuzzy_bits));
    a20e_sector[36] = 0U;
    a20e_sector[REDMCSB_F0914_GRAPHIC21_A20E_LAST_WORD_INDEX] =
        UINT16_C(0x4000);
    assert(redmcsb_f0914_graphic21_a20e(a20e_sector, &result) == 1);
    assert(fuzzy_bits[REDMCSB_F0914_GRAPHIC21_FUZZY_WORD_COUNT - 1U] == 1U);

    memset(fuzzy_bits, 0, sizeof(fuzzy_bits));
    last_event22_time = INT32_C(47);
    a31e_sector[651] = UINT16_C(0x4000);
    assert(redmcsb_f0914_graphic21_a31e(a31e_sector, &result) == 1);
    assert(fuzzy_bits[0] == 1U);
    assert(last_event22_time == INT32_C(47));

    assert(strstr(redmcsb_f0914_graphic21_source_evidence(),
                  "GRAPH21.C:170-204") != NULL);
    assert(strstr(redmcsb_f0914_graphic21_source_evidence(),
                  "GRAPH21.C:207-241") != NULL);

    puts("ok: ReDMCSB F0914 GRAPH21 A20E/A31E source branches");
    return 0;
}
