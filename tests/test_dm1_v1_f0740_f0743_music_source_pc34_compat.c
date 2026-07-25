#include "dm1_v1_f0740_f0743_music_source_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_V1_F0740_F0743_MAP_COUNT_PC34 == 14);
    assert(DM1_V1_F0740_F0743_START_DELAY_PC34 == 100);
    assert(DM1_V1_F0741_GAME_WON_TRACK_PC34 == 2);
    assert(DM1_V1_F0740_F0743_TRACK_NONE_PC34 == -1);
}

static void test_state_init(void)
{
    DM1_V1_F0740F0743MusicStatePc34 s;
    dm1_v1_f0740_f0743_music_state_init_pc34(&s);
    assert(s.musicOn == 1);
    assert(s.paused == 0);
    assert(s.currentMapIndex == DM1_V1_F0740_F0743_TRACK_NONE_PC34);
    assert(s.pendingTrack == DM1_V1_F0740_F0743_TRACK_NONE_PC34);
    assert(s.playingTrack == DM1_V1_F0740_F0743_TRACK_NONE_PC34);
}

static void test_source_struct_layout(void)
{
    DM1_V1_F0740F0743MusicSourcePc34 src;
    memset(&src, 0, sizeof(src));
    assert(src.authenticated == 0);
    assert(src.md5[0] == '\0');
}

static void test_receipt_struct_layout(void)
{
    DM1_V1_F0740F0743MusicReceiptPc34 r;
    memset(&r, 0, sizeof(r));
    assert(r.valid == 0);
    assert(r.suppressSyntheticPlayback == 0);
    assert(r.functionId == 0);
    assert(r.sourceTrackId == 0);
}

static void test_driver_struct_layout(void)
{
    DM1_V1_F0740F0743MusicDriverPc34 d;
    memset(&d, 0, sizeof(d));
    assert(d.pause == NULL);
    assert(d.play == NULL);
    assert(d.context == NULL);
}

static void test_bind_nonexistent(void)
{
    DM1_V1_F0740F0743MusicSourcePc34 src;
    memset(&src, 0, sizeof(src));
    int r = dm1_v1_f0740_f0743_bind_song_dat_pc34("/nonexistent/SONG.DAT", &src);
    (void)r;
    assert(r == 0);
    assert(src.authenticated == 0);
}

static void test_pause_unauthenticated(void)
{
    DM1_V1_F0740F0743MusicSourcePc34 src;
    memset(&src, 0, sizeof(src));
    DM1_V1_F0740F0743MusicStatePc34 state;
    dm1_v1_f0740_f0743_music_state_init_pc34(&state);
    DM1_V1_F0740F0743MusicDriverPc34 driver;
    memset(&driver, 0, sizeof(driver));
    DM1_V1_F0740F0743MusicReceiptPc34 receipt;
    int r = dm1_v1_f0740_music_pause_pc34(&src, &state, &driver, &receipt);
    (void)r;
    assert(r == 0);
}

int main(void)
{
    test_constants();
    test_state_init();
    test_source_struct_layout();
    test_receipt_struct_layout();
    test_driver_struct_layout();
    test_bind_nonexistent();
    test_pause_unauthenticated();

    puts("ok: DM1 F0740-F0743 music source (Q-DM1-08) 7 tests passed");
    return 0;
}
