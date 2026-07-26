#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1068_free_amiga_stuff_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1068_free_amiga_stuff_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1068_free_amiga_stuff_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:630-671") != NULL);
    assert(strstr(evidence, "F1068_FreeAmigaStuff") != NULL);
    assert(strstr(evidence, "blitter") != NULL);
    assert(strstr(evidence, "chip/fast memory") != NULL);
    assert(strstr(evidence, "fuzzy-sector buffer") != NULL);
    assert(strstr(evidence, "copper-interrupt") != NULL);
    assert(strstr(evidence, "HINT001.C:4,65-72") != NULL);
    assert(strstr(evidence, "MEDIA763_AU1E_AU2E") != NULL);
    assert(strstr(evidence, "uninitializes the screen") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1068 Amiga teardown boundary");
    return 0;
}
