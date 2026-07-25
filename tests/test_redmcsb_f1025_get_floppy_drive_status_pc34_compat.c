#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1025_get_floppy_drive_status_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1025_get_floppy_drive_status_source_evidence_pc34();

    assert(!redmcsb_f1025_get_floppy_drive_status_pc34_compat(0x90));
    assert(!redmcsb_f1025_get_floppy_drive_status_pc34_compat(0x91));
    assert(strstr(evidence, "FILE.C:754") != NULL);
    assert(strstr(evidence, "FILE.C:1128-1151") != NULL);
    assert(strstr(evidence, "MEDIA607_X30J_X31J") != NULL);
    assert(strstr(evidence, "B_DRVCHK (D0=0x4E)") != NULL);
    assert(strstr(evidence, "TRAP 15") != NULL);
    assert(strstr(evidence, "PDA 0x90 or 0x91") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1025 X68000 floppy-status host boundary");
    return 0;
}
