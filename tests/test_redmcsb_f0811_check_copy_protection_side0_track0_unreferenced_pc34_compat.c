#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_pc34_compat.h"

int main(void)
{
    assert(!redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_pc34_compat());
    assert(strstr(
               redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_source_evidence_pc34(),
               "IO.C:4122-4151") != NULL);

    puts("ok: ReDMCSB F0811 PC-98-only copy-protection check is source-locked");
    return 0;
}
