#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1010_load_x68000_border_graphics_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1010_load_x68000_border_graphics_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1010_load_x68000_border_graphics_pc34_compat());
    assert(strstr(evidence, "IMAGE.C:58-138") != NULL);
    assert(strstr(evidence, "F1010_LoadX68000BorderGraphics") != NULL);
    assert(strstr(evidence, "MEDIA607_X30J_X31J") != NULL);
    assert(strstr(evidence, "four graphics") != NULL);
    assert(strstr(evidence, "zones 420-423") != NULL);
    assert(strstr(evidence, "G3076_B_ is false") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1010 X68000 border graphics are host-bound");
    return 0;
}
