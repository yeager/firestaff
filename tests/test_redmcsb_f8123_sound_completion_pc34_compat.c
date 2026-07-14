#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f8123_sound_completion_pc34_compat.h"

static unsigned int tandy_calls;

static int16_t tandy_timer_status(void *context)
{
    const int16_t *status = (const int16_t *)context;

    ++tandy_calls;
    return *status;
}

static int check_int(const char *label, int actual, int expected)
{
    if (actual == expected) {
        return 1;
    }
    fprintf(stderr, "%s: got %d, expected %d\n", label, actual, expected);
    return 0;
}

int main(void)
{
    int16_t tandy_status = 1;
    redmcsb_f8123_sound_state_pc34_compat sound_state = {
        REDMCSB_F8123_SOUND_PC_SPEAKER_PC34, 12, tandy_timer_status,
        &tandy_status
    };
    int ok = 1;

    redmcsb_f8123_play_audio_cd_track_pc34_compat(&sound_state);
    ok &= check_int("F8123 preserves device", sound_state.sound_device,
                    REDMCSB_F8123_SOUND_PC_SPEAKER_PC34);
    ok &= check_int("F8123 preserves remaining units",
                    sound_state.remaining_sound_units, 12);

    ok &= check_int("speaker remaining units",
                    redmcsb_f8124_is_sound_play_complete_pc34_compat(
                        &sound_state), 12);
    sound_state.sound_device = REDMCSB_F8123_SOUND_BLASTER_PC34;
    sound_state.remaining_sound_units = 0;
    ok &= check_int("blaster zero remains zero",
                    redmcsb_f8124_is_sound_play_complete_pc34_compat(
                        &sound_state), 0);

    sound_state.sound_device = REDMCSB_F8123_SOUND_TANDY_PC34;
    ok &= check_int("Tandy timer result",
                    redmcsb_f8124_is_sound_play_complete_pc34_compat(
                        &sound_state), 1);
    ok &= check_int("Tandy timer call count", (int)tandy_calls, 1);

    sound_state.sound_device = REDMCSB_F8123_SOUND_NONE_PC34;
    ok &= check_int("no sound default", redmcsb_f8124_is_sound_play_complete_pc34_compat(
                    &sound_state), 0);
    ok &= check_int("source anchors",
                    strstr(redmcsb_f8123_sound_completion_source_evidence_pc34(),
                           "IBMIO.C:2080-2107") != 0, 1);

    return ok ? 0 : 1;
}
