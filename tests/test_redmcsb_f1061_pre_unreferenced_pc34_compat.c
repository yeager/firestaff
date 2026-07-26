#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1061_pre_unreferenced_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1061_pre_unreferenced_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1061_pre_unreferenced_pc34_compat();

    assert(strstr(evidence, "READWRIT.C:73-88") != NULL);
    assert(strstr(evidence, "F1061_Pre_Unreferenced") != NULL);
    assert(strstr(evidence, "MEDIA626_A31E_A31M_A33M_A35M") != NULL);
    assert(strstr(evidence, "NOCOPYPROTECTION disabled") != NULL);
    assert(strstr(evidence, "READWRIT.C:79-86") != NULL);
    assert(strstr(evidence, "FAKE3.C") != NULL);
    assert(strstr(evidence, "MEDIA618_A31E") != NULL);
    assert(strstr(evidence, "FAKE2.C") != NULL);
    assert(strstr(evidence, "MEDIA657_A31M_A33M_A35E") != NULL);
    assert(strstr(evidence, "FAKE4.C") != NULL);
    assert(strstr(evidence, "MEDIA742_A35M") != NULL);
    assert(strstr(evidence, "FAKE2.C:1-17") != NULL);
    assert(strstr(evidence, "FAKE3.C:1-19") != NULL);
    assert(strstr(evidence, "FAKE4.C:1-17") != NULL);
    assert(strstr(evidence, "32 byte long") != NULL);
    assert(strstr(evidence, "never executed") != NULL);
    assert(strstr(evidence, "no portable behavior") != NULL);
    assert(strstr(evidence, "PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1061 pre-unreferenced fake-code boundary");
    return 0;
}
