
#include "nexus_v1_sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int tests_run = 0;

static void test_decode_empty(void) {
    Nexus_SoundEngine eng;
    nexus_sound_init(&eng);
    assert(nexus_sound_decode_sal(&eng) == 0);
    assert(eng.sal_decode_ready == 0);
    nexus_sound_shutdown(&eng);
    tests_run++;
}

static void test_decode_null(void) {
    assert(nexus_sound_decode_sal(NULL) == 0);
    tests_run++;
}

static void test_mix_empty(void) {
    Nexus_SoundEngine eng;
    int16_t buf[64];
    nexus_sound_init(&eng);
    memset(buf, 0x7f, sizeof(buf));
    assert(nexus_sound_mix(&eng, buf, 64) == 64);
    assert(buf[0] == 0);
    assert(buf[63] == 0);
    nexus_sound_shutdown(&eng);
    tests_run++;
}

static void test_mix_null(void) {
    int16_t buf[16];
    assert(nexus_sound_mix(NULL, buf, 16) == 0);
    tests_run++;
}

static void test_voice_limit(void) {
    Nexus_SoundEngine eng;
    int i;
    nexus_sound_init(&eng);
    for (i = 0; i < NEXUS_SOUND_MAX_VOICES + 2; i++) {
        eng.voices[i % NEXUS_SOUND_MAX_VOICES].active = 1;
    }
    nexus_sound_shutdown(&eng);
    tests_run++;
}

static void test_cd_callbacks(void) {
    Nexus_SoundEngine eng;
    nexus_sound_init(&eng);
    assert(eng.cd_play_callback == NULL);
    assert(eng.cd_stop_callback == NULL);
    nexus_sound_set_cd_callbacks(&eng, NULL, NULL, NULL);
    assert(eng.cd_play_callback == NULL);
    nexus_sound_shutdown(&eng);
    tests_run++;
}

static void test_event_name(void) {
    assert(strcmp(nexus_sound_event_name(NEXUS_SFX_FOOTSTEP), "FOOTSTEP") == 0);
    assert(strcmp(nexus_sound_event_name(NEXUS_SFX_DOOR_OPEN), "DOOR_OPEN") == 0);
    assert(strcmp(nexus_sound_event_name(NEXUS_SFX_NONE), "UNKNOWN") == 0);
    tests_run++;
}

int main(void) {
    test_decode_empty();
    test_decode_null();
    test_mix_empty();
    test_mix_null();
    test_voice_limit();
    test_cd_callbacks();
    test_event_name();

    printf("ok: Nexus SAL decode tests passed (%d tests)\n", tests_run);
    return 0;
}
