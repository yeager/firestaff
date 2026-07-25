#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0747_emm_get_version_pc34_compat.h"

typedef struct {
    uint8_t al;
    unsigned int calls;
} redmcsb_f0747_ems_fixture_pc34_compat;

static __attribute__((unused)) uint8_t redmcsb_f0747_get_version_al(
    void *context)
{
    redmcsb_f0747_ems_fixture_pc34_compat *const fixture = context;

    fixture->calls++;
    return fixture->al;
}

static void redmcsb_f0747_assert_version(
    uint8_t al,
    uint8_t expected_version)
{
    (void)expected_version;
    redmcsb_f0747_ems_fixture_pc34_compat fixture = {al, 0U};
    (void)fixture;

    assert(redmcsb_f0747_emm_get_version_pc34_compat(
               redmcsb_f0747_get_version_al, &fixture) == expected_version);
    assert(fixture.calls == 1U);
}

int main(void)
{
    redmcsb_f0747_assert_version(UINT8_C(0x00), UINT8_C(0));
    redmcsb_f0747_assert_version(UINT8_C(0x1f), UINT8_C(1));
    redmcsb_f0747_assert_version(UINT8_C(0x34), UINT8_C(3));
    redmcsb_f0747_assert_version(UINT8_C(0x40), UINT8_C(4));
    redmcsb_f0747_assert_version(UINT8_C(0xff), UINT8_C(15));

    assert(strstr(redmcsb_f0747_emm_get_version_source_evidence_pc34(),
                  "STARTUP2.C:148-158") != NULL);

    puts("ok: ReDMCSB F0747 PC 3.4 EMS version high nibble");
    return 0;
}
