#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1073_open_intuition_library_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1073_open_intuition_library_source_evidence_pc34();

    redmcsb_f1073_open_intuition_library_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:88-130") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:110-120") != NULL);
    assert(strstr(evidence, "F1073_OpenIntuitionLibrary") != NULL);
    assert(strstr(evidence, "intuition.library") != NULL);
    assert(strstr(evidence, "version 31") != NULL);
    assert(strstr(evidence, "IntuitionBase") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF0004)") != NULL);
    assert(strstr(evidence, "F9073_DisplayError(0x80F10004)") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:333-361") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1073 intuition-library boundary");
    return 0;
}
