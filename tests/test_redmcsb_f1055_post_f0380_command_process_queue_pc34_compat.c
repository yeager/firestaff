#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1055_post_f0380_command_process_queue_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1055_post_f0380_command_process_queue_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1055_post_f0380_command_process_queue_pc34_compat();

    assert(strstr(evidence, "COMMAND.C:2480-2498") != NULL);
    assert(strstr(evidence, "F1055_Post_F0380_COMMAND_ProcessQueue_CPSC") !=
           NULL);
    assert(strstr(evidence, "MEDIA626_A31E_A31M_A33M_A35E_A35M") != NULL);
    assert(strstr(evidence, "FAKE4.C") != NULL);
    assert(strstr(evidence, "FAKE3.C") != NULL);
    assert(strstr(evidence, "FAKE2.C") != NULL);
    assert(strstr(evidence, "FAKE1.C") != NULL);
    assert(strstr(evidence, "no portable") != NULL);
    assert(strstr(evidence, "PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1055 post-command queue fake-code boundary");
    return 0;
}
