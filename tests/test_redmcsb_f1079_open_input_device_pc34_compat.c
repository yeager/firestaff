#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1079_open_input_device_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1079_open_input_device_source_evidence_pc34();

    redmcsb_f1079_open_input_device_pc34_compat();

    assert(strstr(evidence, "AMIGINIT.C:4") != NULL);
    assert(strstr(evidence,
                  "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_"
                  "AU2G_AU3E") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:31-35") != NULL);
    assert(strstr(evidence, "C03_GAME") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:200-215") != NULL);
    assert(strstr(evidence, "F1079_OpenInputDevice") != NULL);
    assert(strstr(evidence, "G3157_B_InputDeviceAllocated") != NULL);
    assert(strstr(evidence, "CreatePort(NULL, 0L)") != NULL);
    assert(strstr(evidence, "CreateStdIO") != NULL);
    assert(strstr(evidence, "input.device") != NULL);
    assert(strstr(evidence, "unit 0L") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF000D)") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF000E)") != NULL);
    assert(strstr(evidence,
                  "F1050_AlertCSBSystemError(0x80FF000F)") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:333-361") != NULL);
    assert(strstr(evidence, "F1088_OpenAmigaStuff") != NULL);
    assert(strstr(evidence, "AMIGINIT.C:363-389") != NULL);
    assert(strstr(evidence, "F1080_CloseInputDevice") != NULL);
    assert(strstr(evidence, "F1089_CloseAmigaStuff") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    assert(strstr(evidence, "portable host behavior") != NULL);

    puts("ok: ReDMCSB F1079 input-device boundary");
    return 0;
}
