#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f1053_pre_f0380_command_process_queue_source_evidence_pc34();

    redmcsb_f1053_pre_f0380_command_process_queue_pc34_compat();

    assert(strstr(evidence, "COMMAND.C:2028-2043") != NULL);
    assert(strstr(evidence,
                  "F1053_Pre_F0380_COMMAND_ProcessQueue_CPSC") != NULL);
    assert(strstr(evidence, "MEDIA626_A31E_A31M_A33M_A35E_A35M") != NULL);
    assert(strstr(evidence, "COMMAND.C:2033-2040") != NULL);
    assert(strstr(evidence, "FAKE3.C") != NULL);
    assert(strstr(evidence, "MEDIA641_A31E_A33M_A35E") != NULL);
    assert(strstr(evidence, "FAKE1.C") != NULL);
    assert(strstr(evidence, "MEDIA655_A31M") != NULL);
    assert(strstr(evidence, "FAKE2.C") != NULL);
    assert(strstr(evidence, "MEDIA742_A35M") != NULL);
    assert(strstr(evidence, "FAKE1.C:1-17") != NULL);
    assert(strstr(evidence, "FAKE2.C:1-17") != NULL);
    assert(strstr(evidence, "FAKE3.C:1-19") != NULL);
    assert(strstr(evidence, "32 byte long") != NULL);
    assert(strstr(evidence, "never executed") != NULL);
    assert(strstr(evidence, "AMIGA.H:400") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1053 pre-command-queue compatibility boundary");
    return 0;
}
