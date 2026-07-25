#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1024_set_trap14_vector_error_processing_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1024_set_trap14_vector_error_processing_source_evidence_pc34();

    assert(!redmcsb_f1024_set_trap14_vector_error_processing_pc34_compat());
    assert(strstr(evidence, "FILE.C:806-836") != NULL);
    assert(strstr(evidence, "FILE.C:1091-1126") != NULL);
    assert(strstr(evidence, "F1024_SetTrap14VectorErrorProcessing") != NULL);
    assert(strstr(evidence, "vector 46") != NULL);
    assert(strstr(evidence, "G3091_i_ErrorCount") != NULL);
    assert(strstr(evidence, "STARTUP2.C:1361-1364") != NULL);
    assert(strstr(evidence, "MEDIA692_X31J") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1024 X68000 TRAP 14 host boundary");
    return 0;
}
