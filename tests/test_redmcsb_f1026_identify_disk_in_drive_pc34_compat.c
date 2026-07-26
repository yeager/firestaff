#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1026_identify_disk_in_drive_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1026_identify_disk_in_drive_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1026_identify_disk_in_drive_pc34_compat(0x90));
    assert(!redmcsb_f1026_identify_disk_in_drive_pc34_compat(0x91));
    assert(strstr(evidence, "FLOPPY.C:458-543") != NULL);
    assert(strstr(evidence, "MEDIA607_X30J_X31J") != NULL);
    assert(strstr(evidence, "B_READ sector 9 twice") != NULL);
    assert(strstr(evidence, "TRAP 15") != NULL);
    assert(strstr(evidence, "HPR-0007") != NULL);
    assert(strstr(evidence, "F1031") != NULL);
    assert(strstr(evidence, "0 (unformatted), 1 (game), and 2 (save)") !=
           NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1026 X68000 disk-identification host boundary");
    return 0;
}
