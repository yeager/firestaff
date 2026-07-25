#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1067_init_amiga_stuff_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1067_init_amiga_stuff_source_evidence_pc34();

    redmcsb_f1067_init_amiga_stuff_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:539-628") != NULL);
    assert(strstr(evidence, "F1067_InitAmigaStuff") != NULL);
    assert(strstr(evidence, "vertical blank") != NULL);
    assert(strstr(evidence, "copper interrupt") != NULL);
    assert(strstr(evidence, "chip/fast memory") != NULL);
    assert(strstr(evidence, "HINT001.C:4,56-63") != NULL);
    assert(strstr(evidence, "MEDIA763_AU1E_AU2E") != NULL);
    assert(strstr(evidence, "initializes the screen") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1067 Amiga initialization boundary");
    return 0;
}
