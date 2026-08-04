#ifndef FIRESTAFF_DM2_V1_MUSIC_WAV_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_MUSIC_WAV_PC34_COMPAT_H

/*
 * dm2_v1_music_wav_pc34_compat.h — DM2 WAV/OGG music playback.
 *
 * Ports do_music_wav and do_music_stop from skproject c_music_wav.cpp.
 * Uses receipt-based returns; actual audio playback handled externally.
 *
 * Source: skproject/SKWINSPX/src/v4/c_music_wav.cpp
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool filename_generated;
    char filename[32];
    int16_t track_number;
} DM2_V1_MusicWavReceipt;

typedef struct {
    bool stopped;
} DM2_V1_MusicStopReceipt;

DM2_V1_MusicWavReceipt dm2_v1_music_wav_prepare(int16_t track_nr);
DM2_V1_MusicStopReceipt dm2_v1_music_wav_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MUSIC_WAV_PC34_COMPAT_H */
