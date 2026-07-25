#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0915_graphic538_pc34_compat.h"

int main(void)
{
    const char *evidence = redmcsb_f0915_graphic538_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f0915_graphic538_pc34_compat());
    assert(strstr(evidence, "GRAPH538.C:1-56") != NULL);
    assert(strstr(evidence, "GRAPH538.C:58-76") != NULL);
    assert(strstr(evidence, "GRAPH538.C:78-110") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F0915 is explicitly unavailable on the PC 3.4 host");
    return 0;
}
