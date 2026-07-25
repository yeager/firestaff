#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1043_noop_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence = redmcsb_f1043_noop_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1043_noop_pc34_compat();

    assert(strstr(evidence, "IO.C:1087-1092") != NULL);
    assert(strstr(evidence, "F1043_") != NULL);
    assert(strstr(evidence,
                  "MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M") != NULL);
    assert(strstr(evidence, "body is empty") != NULL);
    assert(strstr(evidence, "INPUT.C:500-525") != NULL);
    assert(strstr(evidence, "Amiga+N") != NULL);
    assert(strstr(evidence, "Amiga+M") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1043 empty Amiga boundary");
    return 0;
}
