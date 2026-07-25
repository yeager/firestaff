#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1032_hatch_box_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence = redmcsb_f1032_hatch_box_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f1032_hatch_box_pc34_compat());
    assert(strstr(evidence, "FILLBOX.C:624-718") != NULL);
    assert(strstr(evidence, "MEDIA611_X30J_A36M_A31E_A31M_A33M_A35E_A35M") !=
           NULL);
    assert(strstr(evidence, "FILLBOX.C:642-695") != NULL);
    assert(strstr(evidence, "G3090_X68000VideoMemoryAddress") != NULL);
    assert(strstr(evidence, "FILLBOX.C:697-715") != NULL);
    assert(strstr(evidence, "G3209_") != NULL);
    assert(strstr(evidence, "JAM1 RectFill") != NULL);
    assert(strstr(evidence, "GRF1.C:29") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1032 hatch-box PC 3.4 host boundary");
    return 0;
}
