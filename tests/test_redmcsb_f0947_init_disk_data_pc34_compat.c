#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0947_init_disk_data_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f0947_init_disk_data_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f0947_init_disk_data_pc34_compat());
    assert(strstr(evidence, "EXEC.C:4-16") != NULL);
    assert(strstr(evidence, "EXEC.C:547-566") != NULL);
    assert(strstr(evidence, "MEDIA442_A20E_A21E") != NULL);
    assert(strstr(evidence, "TrackBuffer") != NULL);
    assert(strstr(evidence, "IOExtTD") != NULL);
    assert(strstr(evidence, "CreatePort") != NULL);
    assert(strstr(evidence, "mn_ReplyPort") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F0947 Amiga disk-data host boundary");
    return 0;
}
