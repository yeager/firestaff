#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1070_close_dos_library_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1070_close_dos_library_source_evidence_pc34();

    redmcsb_f1070_close_dos_library_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:79-86") != NULL);
    assert(strstr(evidence, "F1070_CloseDosLibrary") != NULL);
    assert(strstr(evidence, "CloseLibrary(DOSBase)") != NULL);
    assert(strstr(evidence, "clears DOSBase") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:363-373") != NULL);
    assert(strstr(evidence, "F1089_CloseAmigaStuff") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1070 DOS-library teardown boundary");
    return 0;
}
