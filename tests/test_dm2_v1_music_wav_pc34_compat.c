/*
 * test_dm2_v1_music_wav_pc34_compat.c — unit tests for DM2 music WAV.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_music_wav_pc34_compat.h"

static void test_prepare_track_0(void)
{
    DM2_V1_MusicWavReceipt r = dm2_v1_music_wav_prepare(0);
    assert(r.filename_generated);
    assert(r.track_number == 0);
    assert(strcmp(r.filename, "./DATA/sk00.ogg") == 0);
    printf("  PASS test_prepare_track_0\n");
}

static void test_prepare_track_15(void)
{
    DM2_V1_MusicWavReceipt r = dm2_v1_music_wav_prepare(15);
    assert(r.filename_generated);
    assert(r.track_number == 15);
    assert(strcmp(r.filename, "./DATA/sk15.ogg") == 0);
    printf("  PASS test_prepare_track_15\n");
}

static void test_prepare_negative(void)
{
    DM2_V1_MusicWavReceipt r = dm2_v1_music_wav_prepare(-1);
    assert(r.filename_generated);
    assert(r.track_number == -1);
    /* sprintf with %02d and -1 produces "sk-1" */
    assert(strcmp(r.filename, "./DATA/sk-1.ogg") == 0);
    printf("  PASS test_prepare_negative\n");
}

static void test_stop(void)
{
    DM2_V1_MusicStopReceipt r = dm2_v1_music_wav_stop();
    assert(r.stopped);
    printf("  PASS test_stop\n");
}

int main(void)
{
    printf("test_dm2_v1_music_wav_pc34_compat\n");
    test_prepare_track_0();
    test_prepare_track_15();
    test_prepare_negative();
    test_stop();
    printf("All tests passed.\n");
    return 0;
}
