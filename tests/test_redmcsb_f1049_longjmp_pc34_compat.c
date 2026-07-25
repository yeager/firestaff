#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1049_longjmp_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1049_longjmp_source_evidence_pc34();

    assert(!redmcsb_f1049_longjmp_pc34_compat());
    assert(strstr(evidence, "DEFS.H:3208-3215") != NULL);
    assert(strstr(evidence, "F1049_longjmp") != NULL);
    assert(strstr(evidence,
                  "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J") != NULL);
    assert(strstr(evidence, "MEDIA764_AU1E_AU2E_AU3E") != NULL);
    assert(strstr(evidence, "DEFS.H:3399-3408") != NULL);
    assert(strstr(evidence, "MEDIA551_F20E_F20J_F31E_F31J") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1049 non-PC longjmp boundary");
    return 0;
}
