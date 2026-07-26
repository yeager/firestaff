#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1077_open_console_device_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1077_open_console_device_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1077_open_console_device_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:132-198") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:154-180") != NULL);
    assert(strstr(evidence, "F1077_OpenConsoleDevice") != NULL);
    assert(strstr(evidence, "CreatePort(NULL, 0L)") != NULL);
    assert(strstr(evidence, "CreateStdIO") != NULL);
    assert(strstr(evidence, "console.device") != NULL);
    assert(strstr(evidence, "unit -1L") != NULL);
    assert(strstr(evidence, "io_Device") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF000A)") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF000B)") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF000C)") != NULL);
    assert(strstr(evidence, "F9073_DisplayError(0x80F1000A)") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:333-361") != NULL);
    assert(strstr(evidence, "F1088_OpenAmigaStuff") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1077 console-device boundary");
    return 0;
}
