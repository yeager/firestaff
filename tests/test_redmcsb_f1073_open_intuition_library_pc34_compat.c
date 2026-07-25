#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1073_open_intuition_library_pc34_compat.h"

int main(void)
{
    const char *evidence = redmcsb_f1073_open_intuition_library_source_evidence_pc34();
    (void)evidence;
    redmcsb_f1073_open_intuition_library_pc34_compat();
    assert(strstr(evidence, "AMIGINIT.C:110-120") != NULL);
    assert(strstr(evidence, "no PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F1073 intuition-library boundary");
    return 0;
}
