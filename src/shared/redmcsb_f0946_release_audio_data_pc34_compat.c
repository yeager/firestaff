#include "redmcsb_f0946_release_audio_data_pc34_compat.h"

bool redmcsb_f0946_release_audio_data_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0946_release_audio_data_source_evidence_pc34(void)
{
    return "ReDMCSB SOUND.C:496-557 F0946_ReleaseAudioData calls "
           "CloseDevice for audio.device, conditionally FreeMem for six "
           "struct IOAudio requests, and conditionally DeletePort for six "
           "reply ports; it clears each native handle after release. No PC "
           "3.4 branch or portable host adapter is supplied.";
}
