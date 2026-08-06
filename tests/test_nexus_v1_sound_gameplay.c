#include "nexus_v1_sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_fail = 0;
static int g_count = 0;

static void expect(int cond, const char *msg) {
    g_count++;
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); g_fail++; }
}

/* Dummy CD callbacks for registration test */
static int dummy_cd_play_called = 0;
static int dummy_cd_play(const char *path, void *userdata) {
    (void)path; (void)userdata;
    dummy_cd_play_called = 1;
    return 0;
}
static int dummy_cd_stop_called = 0;
static void dummy_cd_stop(void *userdata) {
    (void)userdata;
    dummy_cd_stop_called = 1;
}

static void test_init_shutdown(void) {
    Nexus_SoundEngine eng;
    memset(&eng, 0, sizeof(eng));
    int rc = nexus_sound_init(&eng);
    expect(rc == 0, "nexus_sound_init returns 0");
    expect(eng.initialized == 1, "engine marked initialized");
    expect(eng.sfx_enabled == 1, "sfx enabled by default");
    expect(eng.music_enabled == 1, "music enabled by default");

    nexus_sound_shutdown(&eng);
    expect(eng.initialized == 0, "engine marked not initialized after shutdown");
}

static void test_mix_silence(void) {
    Nexus_SoundEngine eng;
    memset(&eng, 0, sizeof(eng));
    nexus_sound_init(&eng);

    /* Even a manually populated diagnostic voice must not reach a host
     * mixer before Saturn SCSP/SDDRVS capture admits the PCM contract. */
    eng.sal_decoded_samples[0] = (int16_t *)malloc(2 * sizeof(int16_t));
    eng.sal_decoded_sample_count[0] = 2;
    eng.sal_decoded_tone_count = 1;
    eng.sal_decode_ready = 1;
    eng.voices[0].active = 1;
    eng.voices[0].tone_index = 0;
    eng.voices[0].position = 0;

    int16_t buf[256];
    memset(buf, 0xAB, sizeof(buf));
    int n = nexus_sound_mix(&eng, buf, 256);
    /* With no active voices, mix should produce silence (zero samples or zeroed buffer) */
    expect(n >= 0, "mix returns non-negative sample count");
    if (n > 0) {
        int all_zero = 1;
        for (int i = 0; i < n; i++) {
            if (buf[i] != 0) { all_zero = 0; break; }
        }
        expect(all_zero, "mix with no voices produces silence");
    }

    nexus_sound_shutdown(&eng);
}

static void test_cd_callbacks(void) {
    Nexus_SoundEngine eng;
    memset(&eng, 0, sizeof(eng));
    nexus_sound_init(&eng);

    int userdata = 42;
    nexus_sound_set_cd_callbacks(&eng, dummy_cd_play, dummy_cd_stop, &userdata);
    expect(eng.cd_play_callback == dummy_cd_play, "cd play callback registered");
    expect(eng.cd_stop_callback == dummy_cd_stop, "cd stop callback registered");
    expect(eng.cd_callback_userdata == &userdata, "cd callback userdata registered");

    nexus_sound_shutdown(&eng);
}

static void test_cd_track_does_not_submit_empty_path(void) {
    Nexus_SoundEngine eng;
    memset(&eng, 0, sizeof(eng));
    nexus_sound_init(&eng);
    nexus_sound_set_data_root(&eng, "/tmp/firestaff-nexus-no-cd");
    expect(strcmp(eng.data_root, "/tmp/firestaff-nexus-no-cd") == 0,
           "CD audio uses the configured Nexus data root");
    nexus_sound_set_cd_callbacks(&eng, dummy_cd_play, dummy_cd_stop, NULL);

    dummy_cd_play_called = 0;
    expect(nexus_sound_cd_track(&eng, 2) == 0,
           "missing CD track remains a successful selection");
    expect(dummy_cd_play_called == 0,
           "missing CD track is not submitted with an empty path");
    expect(eng.cd_playing == 0,
           "missing CD track does not enter playing state");
    nexus_sound_shutdown(&eng);
}

static void test_cd_track_rejects_host_audio_substitute(void) {
    Nexus_SoundEngine eng;
    const char *root = "/tmp/firestaff-nexus-cdda-substitute";
    char path[128];
    FILE *file;

    memset(&eng, 0, sizeof(eng));
    nexus_sound_init(&eng);
    snprintf(path, sizeof(path), "%s/track02.wav", root);
    (void)mkdir(root, 0700);
    file = fopen(path, "wb");
    if (file) {
        fputs("synthetic host audio", file);
        fclose(file);
    }
    nexus_sound_set_data_root(&eng, root);
    nexus_sound_set_cd_callbacks(&eng, dummy_cd_play, dummy_cd_stop, NULL);
    dummy_cd_play_called = 0;
    expect(nexus_sound_cd_track(&eng, 2) == 0 &&
               eng.current_cd_track == 2 && eng.cd_track_path[0] == '\0' &&
               eng.cd_playing == 0 && !dummy_cd_play_called,
           "a host track file cannot substitute for authenticated Saturn CDDA");
    remove(path);
    rmdir(root);
    nexus_sound_shutdown(&eng);
}

static void test_event_selector_is_instance_local(void) {
    Nexus_SoundEngine first;
    Nexus_SoundEngine second;
    memset(&first, 0, sizeof(first));
    memset(&second, 0, sizeof(second));

    nexus_sound_init(&first);
    nexus_sound_set_event_selector(&first, NEXUS_SFX_DOOR_OPEN, 2);
    expect(first.event_selector[NEXUS_SFX_DOOR_OPEN] == -1,
           "unproven host selector cannot enter the SLEV dispatch table");
    nexus_sound_shutdown(&first);

    nexus_sound_init(&second);
    expect(second.event_selector[NEXUS_SFX_DOOR_OPEN] == -1,
           "diagnostic selector does not leak into a new engine");
    nexus_sound_shutdown(&second);
}

static void test_event_names(void) {
    /* Verify all major event names resolve to non-NULL strings */
    const Nexus_SoundEvent events[] = {
        NEXUS_SFX_NONE, NEXUS_SFX_FOOTSTEP, NEXUS_SFX_DOOR_OPEN,
        NEXUS_SFX_DOOR_CLOSE, NEXUS_SFX_ATTACK_HIT, NEXUS_SFX_ATTACK_MISS,
        NEXUS_SFX_CHAMPION_HURT, NEXUS_SFX_CREATURE_DEATH,
        NEXUS_SFX_CREATURE_ATTACK, NEXUS_SFX_SPELL_CAST, NEXUS_SFX_SPELL_IMPACT,
        NEXUS_SFX_PICKUP_ITEM, NEXUS_SFX_DROP_ITEM, NEXUS_SFX_STAIRS,
        NEXUS_SFX_TELEPORT, NEXUS_SFX_ALARM, NEXUS_SFX_PIT_FALL,
        NEXUS_SFX_MENU_SELECT, NEXUS_SFX_MENU_CONFIRM, NEXUS_SFX_MENU_CANCEL,
        NEXUS_SFX_GOLD_PICKUP, NEXUS_SFX_EXIT_REACHED, NEXUS_SFX_PARTY_HURT,
        NEXUS_SFX_LEVEL_UP, NEXUS_SFX_MAGIC_SHIELD, NEXUS_SFX_MAGIC_HEAL,
        NEXUS_SFX_MAGIC_DAMAGE
    };
    int n = (int)(sizeof(events) / sizeof(events[0]));
    for (int i = 0; i < n; i++) {
        const char *name = nexus_sound_event_name(events[i]);
        expect(name != NULL, "event name is not NULL");
        if (name) {
            expect(strlen(name) > 0, "event name is non-empty");
        }
    }

    /* Individual host labels are intentionally not source-promoted. */
    const char *foot = nexus_sound_event_name(NEXUS_SFX_FOOTSTEP);
    expect(foot != NULL && strcmp(foot, "HOST_EVENT") == 0,
           "host event name remains a generic diagnostic token");
}

static void test_sfx_music_toggle(void) {
    Nexus_SoundEngine eng;
    memset(&eng, 0, sizeof(eng));
    nexus_sound_init(&eng);

    nexus_sound_set_sfx(&eng, 0);
    expect(eng.sfx_enabled == 0, "sfx disabled after set_sfx(0)");
    nexus_sound_set_sfx(&eng, 1);
    expect(eng.sfx_enabled == 1, "sfx enabled after set_sfx(1)");

    nexus_sound_set_music(&eng, 0);
    expect(eng.music_enabled == 0, "music disabled after set_music(0)");
    nexus_sound_set_music(&eng, 1);
    expect(eng.music_enabled == 1, "music enabled after set_music(1)");

    nexus_sound_shutdown(&eng);
}

int main(void) {
    test_init_shutdown();
    test_mix_silence();
    test_cd_callbacks();
    test_cd_track_does_not_submit_empty_path();
    test_cd_track_rejects_host_audio_substitute();
    test_event_selector_is_instance_local();
    test_event_names();
    test_sfx_music_toggle();

    if (g_fail) {
        fprintf(stderr, "test_nexus_v1_sound_gameplay: %d failure(s)\n", g_fail);
        return 1;
    }
    printf("ok: nexus_v1_sound_gameplay (%d tests)\n", g_count);
    return 0;
}
