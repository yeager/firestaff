#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0945_init_audio_data_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
    (void)evidence;
        redmcsb_f0945_init_audio_data_source_evidence_pc34();

    assert(!redmcsb_f0945_init_audio_data_pc34_compat());
    assert(strstr(evidence, "SOUND.C:425-494") != NULL);
    assert(strstr(evidence, "MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M") != NULL);
    assert(strstr(evidence, "I34E/I34M") != NULL);
    assert(strstr(evidence, "OpenDevice(\"audio.device\")") != NULL);
    assert(strstr(evidence, "CreatePort") != NULL);
    assert(strstr(evidence, "DoIO") != NULL);
    assert(strstr(evidence, "EXEC.C:433-501") != NULL);
    assert(strstr(evidence, "no portable host adapter") != NULL);
    puts("ok: ReDMCSB F0945 Amiga audio host boundary");
    return 0;
}
