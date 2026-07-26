#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1057_pre_f0433_startend_process_command140_save_game_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f1057_pre_f0433_startend_process_command140_save_game_source_evidence_pc34();
    (void)evidence;

    redmcsb_f1057_pre_f0433_startend_process_command140_save_game_pc34_compat();

    assert(strstr(evidence, "LOADSAVE.C:532-547") != NULL);
    assert(strstr(evidence,
                  "F1057_Pre_F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF") !=
           NULL);
    assert(strstr(evidence, "MEDIA626_A31E_A31M_A33M_A35E_A35M") != NULL);
    assert(strstr(evidence, "LOADSAVE.C:537-545") != NULL);
    assert(strstr(evidence, "FAKE1.C") != NULL);
    assert(strstr(evidence, "MEDIA636_A31E_A31M_A35E") != NULL);
    assert(strstr(evidence, "FAKE3.C") != NULL);
    assert(strstr(evidence, "MEDIA665_A33M") != NULL);
    assert(strstr(evidence, "FAKE2.C") != NULL);
    assert(strstr(evidence, "MEDIA742_A35M") != NULL);
    assert(strstr(evidence, "FAKE1.C:1-17") != NULL);
    assert(strstr(evidence, "FAKE2.C:1-17") != NULL);
    assert(strstr(evidence, "FAKE3.C:1-19") != NULL);
    assert(strstr(evidence, "32 byte long") != NULL);
    assert(strstr(evidence, "never executed") != NULL);
    assert(strstr(evidence, "AMIGA.H:402") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);

    puts("ok: ReDMCSB F1057 pre-save-game fake-code boundary");
    return 0;
}
