/*
 * CTest gate: DM1 V1 Sound & Music System source-lock verification.
 *
 * Verifies:
 *  1. Sound system init with correct defaults
 *  2. All 35 ReDMCSB sound event constants mapped
 *  3. Directional volume table (F0505_SOUND_GetVolume) parity
 *  4. Pending sound priority logic (F0064/F0065)
 *  5. Music track mapping (G2039_ai_MapIndexToMusicTrack)
 *  6. Music update countdown (F0743_MUSIC_Update)
 *  7. Sound name coverage
 */
#include "dm1_v1_sound_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); } \
} while(0)

static void test_init(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    CHECK(sys.masterVolume == 255, "master volume init 255");
    CHECK(sys.sfxVolume == 255, "sfx volume init 255");
    CHECK(sys.musicVolume == 128, "music volume init 128");
    CHECK(sys.pending.pendingSoundIndex == DM1_PENDING_NONE, "no pending sound");
    CHECK(sys.lastPlayedSoundIndex == DM1_SND_NONE, "no last played");
    CHECK(sys.music.currentMapIndex == DM1_MUSIC_TRACK_NONE, "music map -1");
    CHECK(sys.music.musicOn == 1, "music on by default");
}

static void test_sound_constants(void) {
    CHECK(DM1_SND_COUNT == 35, "35 sound events");
    CHECK(DM1_SND_FIRST_ATTACK == 19, "first attack at 19");
    CHECK(DM1_SND_FIRST_MOVEMENT == 28, "first movement at 28");
    CHECK(DM1_SND_METALLIC_THUD == 0, "metallic thud = 0");
    CHECK(DM1_SND_MOVE_SKELETON == 34, "skeleton move = 34");
}

static void test_sound_data(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    for (int i = 0; i < DM1_SND_COUNT; i++) {
        const DM1_SoundData* d = DM1_Sound_GetSoundData(&sys, (int16_t)i);
        CHECK(d != NULL, "sound data not null");
        if (d) {
            CHECK(d->graphicIndex >= 671 && d->graphicIndex <= 712,
                  "graphic index in SND3 range");
            CHECK(d->priority > 0, "priority > 0");
            CHECK(d->loudDistance <= d->softDistance, "loud <= soft distance");
        }
    }
    CHECK(DM1_Sound_GetSoundData(&sys, -1) == NULL, "invalid index returns NULL");
    CHECK(DM1_Sound_GetSoundData(&sys, 35) == NULL, "out of range returns NULL");
}

static void test_volume_at_party(void) {
    /* Sound at party position => center of table => max volume */
    DM1_SoundSystem sys;
    DM1_SoundVolume vol;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0);
    int ok = DM1_Sound_GetVolume(&sys, 10, 10, &vol);
    CHECK(ok == 1, "volume at party is audible");
    CHECK(vol.left == 64, "left volume at party = 64 (table center)");
    CHECK(vol.right == 64, "right volume at party = 64 (table center)");
}

static void test_volume_out_of_range(void) {
    DM1_SoundSystem sys;
    DM1_SoundVolume vol;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0);
    int ok = DM1_Sound_GetVolume(&sys, 10, 30, &vol);
    CHECK(ok == 0, "20 squares away is out of range");
}

static void test_volume_directional(void) {
    /* Sound to the right (east) of a north-facing party =>
     * rightCol = +2, line = 0 => table[12][14] for right, table[12][10] for left */
    DM1_SoundSystem sys;
    DM1_SoundVolume vol;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0); /* facing north */
    int ok = DM1_Sound_GetVolume(&sys, 12, 10, &vol);
    CHECK(ok == 1, "2 squares east is audible");
    CHECK(vol.right >= vol.left, "right louder when source is to the right");
}

static void test_pending_priority(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0);

    /* Request a low-priority sound */
    DM1_Sound_RequestPlay(&sys, DM1_SND_MOVE_SKELETON, 10, 10, DM1_MODE_PLAY_IF_PRIORITIZED);
    CHECK(sys.pending.pendingSoundIndex == DM1_SND_MOVE_SKELETON, "skeleton pending");

    /* Request a higher-priority sound at same volume => replaces */
    DM1_Sound_RequestPlay(&sys, DM1_SND_COMBAT, 10, 10, DM1_MODE_PLAY_IF_PRIORITIZED);
    CHECK(sys.pending.pendingSoundIndex == DM1_SND_COMBAT, "combat replaces skeleton");

    /* Flush pending */
    DM1_Sound_PlayPending(&sys);
    CHECK(sys.lastPlayedSoundIndex == DM1_SND_COMBAT, "combat was played");
    CHECK(sys.pending.pendingSoundIndex == DM1_PENDING_NONE, "pending cleared");
    CHECK(sys.totalPendingFlushes == 1, "one flush");
}

static void test_immediate_play(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0);

    DM1_Sound_RequestPlay(&sys, DM1_SND_SCREAM, 10, 10, DM1_MODE_PLAY_IMMEDIATELY);
    CHECK(sys.lastPlayedSoundIndex == DM1_SND_SCREAM, "scream played immediately");
    CHECK(sys.totalSoundsPlayed == 1, "one sound played");
}

static void test_out_of_range_not_played(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 10, 0, 0);

    /* Sound 20 squares away => beyond softDistance for all sounds */
    DM1_Sound_RequestPlay(&sys, DM1_SND_COMBAT, 30, 10, DM1_MODE_PLAY_IMMEDIATELY);
    CHECK(sys.lastPlayedSoundIndex == DM1_SND_NONE, "distant sound not played");
}

static void test_music_set_track(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);

    DM1_Music_SetTrack(&sys, 0);
    CHECK(sys.music.pendingTrack == 2, "map 0 -> track 2 (I34E)");
    CHECK(sys.music.countdownBeforeStart == 100, "countdown = 100");

    DM1_Music_SetTrack(&sys, 5);
    CHECK(sys.music.pendingTrack == 6, "map 5 -> track 6 (I34E)");
}

static void test_music_update(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);

    DM1_Music_SetTrack(&sys, 0);
    /* Tick down 100 times */
    for (int i = 0; i < 100; i++) {
        DM1_Music_Update(&sys);
    }
    CHECK(sys.music.countdownBeforeStart == 0, "countdown reached 0");
    DM1_Music_Update(&sys); /* This tick triggers the track switch */
    CHECK(sys.music.playingTrack == 2, "track 2 now playing");
    CHECK(sys.music.countdownBeforeStart == -1, "countdown reset to -1");
}

static void test_music_toggle_runtime_state(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);

    CHECK(DM1_Music_IsOn(&sys) == 1, "music starts enabled in Firestaff runtime");
    DM1_Music_SetTrack(&sys, 0);
    for (int i = 0; i <= 100; i++) DM1_Music_Update(&sys);
    CHECK(sys.music.playingTrack == 2, "music playing before toggle off");

    CHECK(DM1_Music_Toggle(&sys) == 0, "toggle returns off");
    DM1_Music_Update(&sys);
    CHECK(sys.music.playingTrack == DM1_MUSIC_TRACK_NONE, "toggle off stops playing track");
    CHECK(sys.music.currentMapIndex == DM1_MUSIC_TRACK_NONE, "toggle off invalidates map index");

    DM1_Music_SetTrack(&sys, 1);
    for (int i = 0; i <= 100; i++) DM1_Music_Update(&sys);
    CHECK(sys.music.playingTrack == DM1_MUSIC_TRACK_NONE, "track requests are ignored while off");

    CHECK(DM1_Music_SetOn(&sys, 1) == 1, "music set on");
    DM1_Music_Update(&sys);
    DM1_Music_SetTrack(&sys, 1);
    for (int i = 0; i <= 100; i++) DM1_Music_Update(&sys);
    CHECK(sys.music.playingTrack == 14, "music resumes map scheduling after on");
}

static void test_music_stop(void) {
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Music_SetTrack(&sys, 3);
    for (int i = 0; i <= 100; i++) DM1_Music_Update(&sys);
    CHECK(sys.music.playingTrack != DM1_MUSIC_TRACK_NONE, "music playing");
    DM1_Music_Stop(&sys);
    CHECK(sys.music.playingTrack == DM1_MUSIC_TRACK_NONE, "music stopped");
}

static void test_sound_names(void) {
    for (int i = 0; i < DM1_SND_COUNT; i++) {
        const char* name = DM1_Sound_Name((int16_t)i);
        CHECK(name != NULL, "name not null");
        CHECK(strcmp(name, "Unknown") != 0, "name is not Unknown");
    }
    CHECK(strcmp(DM1_Sound_Name(99), "Unknown") == 0, "invalid -> Unknown");
}

static void test_music_map_parity(void) {
    /* Verify against ReDMCSB I34E G2039_ai_MapIndexToMusicTrack:
     * {2, 14, 9, 6, 11, 6, 5, 12, 15, 16, 17, 10, 19, 3} */
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    const int16_t expected[] = {2, 14, 9, 6, 11, 6, 5, 12, 15, 16, 17, 10, 19, 3};
    for (int i = 0; i < DM1_MUSIC_MAP_COUNT; i++) {
        CHECK(sys.music.mapToTrack[i] == expected[i], "map-to-track parity");
    }
}

int main(void) {
    test_init();
    test_sound_constants();
    test_sound_data();
    test_volume_at_party();
    test_volume_out_of_range();
    test_volume_directional();
    test_pending_priority();
    test_immediate_play();
    test_out_of_range_not_played();
    test_music_set_track();
    test_music_update();
    test_music_toggle_runtime_state();
    test_music_stop();
    test_sound_names();
    test_music_map_parity();

    printf("dm1_v1_sound_pc34_compat: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
