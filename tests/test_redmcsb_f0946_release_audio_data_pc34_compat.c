#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0946_release_audio_data_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

int main(void)
{
    const char *evidence =
        redmcsb_f0946_release_audio_data_source_evidence_pc34();
    (void)evidence;

    assert(!redmcsb_f0946_release_audio_data_pc34_compat());
    assert(strstr(evidence, "SOUND.C:496-557") != NULL);
    assert(strstr(evidence, "F0946_ReleaseAudioData") != NULL);
    assert(strstr(evidence, "CloseDevice") != NULL);
    assert(strstr(evidence, "six struct IOAudio requests") != NULL);
    assert(strstr(evidence, "six reply ports") != NULL);
    assert(strstr(evidence, "No PC 3.4 branch") != NULL);
    puts("ok: ReDMCSB F0946 native audio host boundary");
    return 0;
}
