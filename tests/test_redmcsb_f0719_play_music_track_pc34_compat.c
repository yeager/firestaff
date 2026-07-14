#include "redmcsb_f0719_play_music_track_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    unsigned int calls;
    int16_t music_track;
} redmcsb_f0719_capture_pc34_compat;

static void play_audio_cd_track(void *context, int16_t music_track)
{
    redmcsb_f0719_capture_pc34_compat *capture = context;

    capture->calls++;
    capture->music_track = music_track;
}

int main(void)
{
    redmcsb_f0719_capture_pc34_compat capture = { 0U, 0 };
    redmcsb_f0719_io_driver_pc34_compat driver = {
        false, play_audio_cd_track, &capture
    };

    redmcsb_f0719_play_music_track_pc34_compat(&driver, 7);
    assert(capture.calls == 0U);

    driver.pending_music_on = true;
    redmcsb_f0719_play_music_track_pc34_compat(&driver, INT16_MIN);
    assert(capture.calls == 1U);
    assert(capture.music_track == INT16_MIN);

    driver.play_audio_cd_track = NULL;
    redmcsb_f0719_play_music_track_pc34_compat(&driver, 3);
    redmcsb_f0719_play_music_track_pc34_compat(NULL, 3);
    assert(capture.calls == 1U);
    assert(strstr(redmcsb_f0719_play_music_track_source_evidence_pc34(),
                  "G2024_B_PendingMusicOn") != NULL);
    return 0;
}
