#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0943_release_disk_resource_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f0943_release_disk_resource_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f0943_release_disk_resource_pc34_compat());
    assert(strstr(evidence, "EXEC.C:4-16") != NULL);
    assert(strstr(evidence, "EXEC.C:397-406") != NULL);
    assert(strstr(evidence, "MEDIA442_A20E_A21E") != NULL);
    assert(strstr(evidence, "DeletePort") != NULL);
    assert(strstr(evidence, "sizeof(struct DiscResourceUnit)") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F0943 Amiga disk.resource host boundary");
    return 0;
}
