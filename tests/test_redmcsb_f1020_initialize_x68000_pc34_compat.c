#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1020_initialize_x68000_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1020_initialize_x68000_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1020_initialize_x68000_pc34_compat());
    assert(strstr(evidence, "STARTUP2.C:1361-1374") != NULL);
    assert(strstr(evidence, "MEDIA607_X30J_X31J") != NULL);
    assert(strstr(evidence, "F1024_SetTrap14VectorErrorProcessing") != NULL);
    assert(strstr(evidence, "STARTUP2.C:1625-1671") != NULL);
    assert(strstr(evidence, "IOCS TIMEGET (trap #15)") != NULL);
    assert(strstr(evidence, "MEDIA577_X30J") != NULL);
    assert(strstr(evidence, "MEDIA692_X31J") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1020 X68000 initialization is host-bound");
    return 0;
}
