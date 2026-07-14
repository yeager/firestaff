#ifndef FIRESTAFF_REDMCSB_F0719_PLAY_MUSIC_TRACK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0719_PLAY_MUSIC_TRACK_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0719_play_audio_cd_track_pc34_compat)(
    void *context, int16_t music_track);

typedef struct {
    bool pending_music_on;
    redmcsb_f0719_play_audio_cd_track_pc34_compat play_audio_cd_track;
    void *context;
} redmcsb_f0719_io_driver_pc34_compat;

/* ReDMCSB IO.C F0719, PC 3.4 I34E/I34M: G2024 gates IODRV_23 only. */
void redmcsb_f0719_play_music_track_pc34_compat(
    const redmcsb_f0719_io_driver_pc34_compat *io_driver,
    int16_t music_track);

const char *redmcsb_f0719_play_music_track_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
