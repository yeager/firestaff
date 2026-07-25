#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0949_japanese_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static __attribute__((unused)) uint16_t expected_pc98_code(uint16_t packed_character)
{
    uint16_t lead = packed_character >> 8;
    uint16_t trail = packed_character & UINT16_C(0x00ff);

    if (lead >= UINT16_C(0x00e0)) {
        lead -= UINT16_C(0x0040);
    }
    lead = ((lead - UINT16_C(0x0080)) * UINT16_C(2)) & UINT16_C(0x00ff);

    if (trail > UINT16_C(0x009e)) {
        trail -= UINT16_C(0x007e);
    } else {
        lead = (lead - UINT16_C(1)) & UINT16_C(0x00ff);
        trail -= trail >= UINT16_C(0x007f) ? UINT16_C(0x0020)
                                            : UINT16_C(0x001f);
    }

    return (((lead + UINT16_C(0x0020)) & UINT16_C(0x00ff)) << 8) |
           (trail & UINT16_C(0x00ff));
}

int main(void)
{
    static const struct {
        uint16_t packed_character;
        uint16_t expected_code;
    } cases[] = {
        { UINT16_C(0x8140), UINT16_C(0x2121) },
        { UINT16_C(0x817e), UINT16_C(0x215f) },
        { UINT16_C(0x817f), UINT16_C(0x215f) },
        { UINT16_C(0x819e), UINT16_C(0x217e) },
        { UINT16_C(0x819f), UINT16_C(0x2221) },
        { UINT16_C(0xe040), UINT16_C(0x5f21) },
        { UINT16_C(0xfc4b), UINT16_C(0x972c) }
    };
    const char *evidence = redmcsb_f0949_japanese_source_evidence_pc34();
    (void)evidence;
    uint32_t packed_character;
    size_t index;

    for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        assert((uint16_t)redmcsb_f0949_japanese_pc34_compat(
                   (int16_t)cases[index].packed_character) ==
               cases[index].expected_code);
    }
    for (packed_character = 0; packed_character <= UINT16_MAX;
         ++packed_character) {
        assert((uint16_t)redmcsb_f0949_japanese_pc34_compat(
                   (int16_t)packed_character) ==
               expected_pc98_code((uint16_t)packed_character));
    }

    assert(strstr(evidence, "JAPANESE.C:15-34") != NULL);
    assert(strstr(evidence, "MEDIA459_P20JA_P20JB_P31J") != NULL);
    assert(strstr(evidence, "JAPANESE.C:288-298") != NULL);
    assert(strstr(evidence, "JAPANESE.C:315") != NULL);
    puts("ok: ReDMCSB F0949 Japanese PC-98 character conversion");
    return 0;
}
