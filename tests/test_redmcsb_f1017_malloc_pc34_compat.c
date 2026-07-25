#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1017_malloc_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence = redmcsb_f1017_malloc_source_evidence_pc34();
    (void)evidence;

    assert(redmcsb_f1017_malloc_pc34_compat(0U) == NULL);
    assert(redmcsb_f1017_malloc_pc34_compat(SIZE_MAX) == NULL);
    assert(strstr(evidence, "CEDT018.C") != NULL);
    assert(strstr(evidence, "F1017_Malloc") != NULL);
    assert(strstr(evidence, "non-PC media route") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F1017 Malloc is a non-PC boundary");
    return 0;
}
