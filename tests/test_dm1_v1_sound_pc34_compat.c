#include "dm1_v1_sound_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_sound_constants(void)
{
    assert(DM1_SND_METALLIC_THUD == 0);
    assert(DM1_SND_COMBAT == 13);
    assert(DM1_SND_MOVE_SKELETON == 34);
    assert(DM1_SND_NONE == -1);
    assert(DM1_SND_COUNT == 35);
    assert(DM1_SND_FIRST_ATTACK == 19);
    assert(DM1_SND_FIRST_MOVEMENT == 28);
}

static void test_play_modes(void)
{
    assert(DM1_MODE_DO_NOT_PLAY == -1);
    assert(DM1_MODE_PLAY_IMMEDIATELY == 0);
    assert(DM1_MODE_PLAY_IF_PRIORITIZED == 1);
    assert(DM1_MODE_PLAY_ONE_TICK_LATER == 2);
}

static void test_music_constants(void)
{
    assert(DM1_MUSIC_MAP_COUNT == 14);
    assert(DM1_MUSIC_TRACK_NONE == -1);
    assert(DM1_PENDING_NONE == -1);
}

static void test_emission_route_enum(void)
{
    assert(DM1_V1_AUDIO_EMISSION_ROUTE_NONE == 0);
    assert(DM1_V1_AUDIO_EMISSION_ROUTE_FOOTSTEP == 1);
    assert(DM1_V1_AUDIO_EMISSION_ROUTE_SOURCE_SOUND == 6);
}

static void test_init(void)
{
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    assert(sys.pending.pendingSoundIndex == DM1_PENDING_NONE);
    assert(sys.muted == 0);
    assert(sys.totalSoundRequests == 0);
}

static void test_set_party_position(void)
{
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 10, 20, 2, 5);
    assert(sys.partyMapX == 10);
    assert(sys.partyMapY == 20);
    assert(sys.partyDirection == 2);
    assert(sys.partyMapIndex == 5);
}

static void test_music_toggle(void)
{
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    int on1 = DM1_Music_IsOn(&sys);
    (void)on1;
    DM1_Music_Toggle(&sys);
    int on2 = DM1_Music_IsOn(&sys);
    (void)on2;
    assert(on1 != on2);
}

static void test_sound_name(void)
{
    const char* name = DM1_Sound_Name(DM1_SND_COMBAT);
    assert(name != NULL);
    assert(strlen(name) > 0);
}

static void test_sound_data(void)
{
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    const DM1_SoundData* sd = DM1_Sound_GetSoundData(&sys, DM1_SND_SWITCH);
    assert(sd != NULL);
}

static void test_request_play(void)
{
    DM1_SoundSystem sys;
    DM1_Sound_Init(&sys);
    DM1_Sound_SetPartyPosition(&sys, 5, 5, 0, 0);
    DM1_Sound_RequestPlay(&sys, DM1_SND_SWITCH, 5, 5, DM1_MODE_PLAY_IMMEDIATELY);
    assert(sys.totalSoundRequests == 1);
}

int main(void)
{
    test_sound_constants();
    test_play_modes();
    test_music_constants();
    test_emission_route_enum();
    test_init();
    test_set_party_position();
    test_music_toggle();
    test_sound_name();
    test_sound_data();
    test_request_play();

    puts("ok: DM1 sound system (Q-DM1-08) 10 tests passed");
    return 0;
}
