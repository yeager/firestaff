#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1037_mouse2_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence = redmcsb_f1037_mouse2_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1037_mouse2_pc34_compat());
    assert(strstr(evidence, "IO.C:1861-2091") != NULL);
    assert(strstr(evidence, "IO.C:2017-2090") != NULL);
    assert(strstr(evidence, "MEDIA613_X30J_A36M_A31E_A31M_A33M_A35E_A35M_X31J") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F1037 Mouse2 is a non-PC boundary");
    return 0;
}
