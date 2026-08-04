/*
 * dm2_v1_music_wav_pc34_compat.c — DM2 WAV/OGG music filename generation.
 *
 * Source: skproject/SKWINSPX/src/v4/c_music_wav.cpp
 */

#include "dm2_v1_music_wav_pc34_compat.h"
#include <stdio.h>
#include <string.h>

DM2_V1_MusicWavReceipt dm2_v1_music_wav_prepare(int16_t track_nr)
{
    DM2_V1_MusicWavReceipt r;
    memset(&r, 0, sizeof(r));
    r.track_number = track_nr;
    snprintf(r.filename, sizeof(r.filename), "./DATA/sk%02d.ogg", (int)track_nr);
    r.filename_generated = true;
    return r;
}

DM2_V1_MusicStopReceipt dm2_v1_music_wav_stop(void)
{
    DM2_V1_MusicStopReceipt r;
    r.stopped = true;
    return r;
}
