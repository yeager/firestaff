#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0948_release_disk_data_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f0948_release_disk_data_source_evidence_pc34();

    assert(!redmcsb_f0948_release_disk_data_pc34_compat());
    assert(strstr(evidence, "EXEC.C:4-16") != NULL);
    assert(strstr(evidence, "EXEC.C:568-579") != NULL);
    assert(strstr(evidence, "MEDIA442_A20E_A21E") != NULL);
    assert(strstr(evidence, "FreeMem for IOExtTD") != NULL);
    assert(strstr(evidence, "DeletePort for IOExtTDReplyPort") != NULL);
    assert(strstr(evidence, "TD_SECTOR * NUMSECS") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F0948 Amiga disk-data host boundary");
    return 0;
}
