#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0926_floppy_force_media_change_detection_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f0926_floppy_force_media_change_detection_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f0926_floppy_force_media_change_detection_pc34_compat(0));
    assert(!redmcsb_f0926_floppy_force_media_change_detection_pc34_compat(1));
    assert(strstr(evidence, "PRIM1.C:297-395") != NULL);
    assert(strstr(evidence, "MEDIA772_SU1E") != NULL);
    assert(strstr(evidence, "A:\\F") != NULL);
    assert(strstr(evidence, "Super(0L)") != NULL);
    assert(strstr(evidence, "0x0472, 0x047E, and 0x0476") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F0926 Atari ST media-change handler is host-bound");
    return 0;
}
