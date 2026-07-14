#include "redmcsb_f0719_play_music_track_pc34_compat.h"

#include <stddef.h>

void redmcsb_f0719_play_music_track_pc34_compat(
    const redmcsb_f0719_io_driver_pc34_compat *io_driver,
    int16_t music_track)
{
    /* IO.C F0719, I34E/I34M: G2024_B_PendingMusicOn is the sole guard. */
    if (io_driver != NULL && io_driver->pending_music_on &&
        io_driver->play_audio_cd_track != NULL) {
        io_driver->play_audio_cd_track(io_driver->context, music_track);
    }
}

const char *redmcsb_f0719_play_music_track_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C F0719_PlayMusicTrack, MEDIA463_P20JA_P20JB_I34E_"
           "I34M_P31J: G2024_B_PendingMusicOn gates "
           "G2161_IODriver->IODRV_23_PlayAudioCDTrack(music track).";
}
