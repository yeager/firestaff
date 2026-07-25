#include "redmcsb_f1018_mfree_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *evidence = redmcsb_f1018_mfree_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1018_mfree_pc34_compat());
    assert(strstr(evidence, "CEDT018.C") != NULL);
    assert(strstr(evidence, "X68000") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1018 X68000 Mfree host boundary");
    return 0;
}
