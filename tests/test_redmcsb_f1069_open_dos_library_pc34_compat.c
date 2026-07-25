#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1069_open_dos_library_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1069_open_dos_library_source_evidence_pc34();

    redmcsb_f1069_open_dos_library_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:67-77") != NULL);
    assert(strstr(evidence, "F1069_OpenDosLibrary") != NULL);
    assert(strstr(evidence, "dos.library") != NULL);
    assert(strstr(evidence, "version 31") != NULL);
    assert(strstr(evidence, "DOSBase") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF0001)") != NULL);
    assert(strstr(evidence, "F9073_DisplayError(0x80F10001)") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:343") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1069 Amiga DOS-library boundary");
    return 0;
}
