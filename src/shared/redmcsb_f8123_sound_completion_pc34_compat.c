#include "redmcsb_f8123_sound_completion_pc34_compat.h"

void redmcsb_f8123_play_audio_cd_track_pc34_compat(
    redmcsb_f8123_sound_state_pc34_compat *sound_state)
{
    (void)sound_state;
}

int16_t redmcsb_f8124_is_sound_play_complete_pc34_compat(
    const redmcsb_f8123_sound_state_pc34_compat *sound_state)
{
    switch (sound_state->sound_device) {
    case REDMCSB_F8123_SOUND_PC_SPEAKER_PC34:
    case REDMCSB_F8123_SOUND_FTL_ADAPTER_PC34:
    case REDMCSB_F8123_SOUND_DISNEY_PC34:
    case REDMCSB_F8123_SOUND_BLASTER_PC34:
    case REDMCSB_F8123_SOUND_ADLIB_PC34:
        return sound_state->remaining_sound_units;
    case REDMCSB_F8123_SOUND_TANDY_PC34:
        if (sound_state->tandy_timer_status != 0) {
            return sound_state->tandy_timer_status(sound_state->context);
        }
        return 0;
    default:
        return 0;
    }
}

const char *redmcsb_f8123_sound_completion_source_evidence_pc34(void)
{
    return "ReDMCSB IBMIO.C:2080-2107; MEDIA701_I34E PC route";
}
