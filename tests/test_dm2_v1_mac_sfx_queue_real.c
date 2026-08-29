/* Authentic Mac GDAT SFX queue proof.
 *
 * The queue rows and PCM bytes come from the original Mac ZIP.  The test only
 * supplies a capture backend, so it never needs an audio device and never
 * fabricates a sound row or sample payload.
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned starts;
    uint16_t raw_index;
    uint32_t sample_count;
    uint8_t volume;
} Capture;

static int capture_ready(void *ctx) { (void)ctx; return 1; }
static int capture_start(void *ctx, unsigned slot, const uint8_t *pcm,
                         uint32_t count, uint8_t volume)
{
    Capture *capture = (Capture *)ctx;
    (void)slot;
    if (!capture || !pcm || count == 0u) return 0;
    capture->starts++;
    capture->sample_count = count;
    capture->volume = volume;
    return 1;
}
static int capture_active(void *ctx, unsigned slot)
{
    (void)ctx; (void)slot; return 0;
}
static void capture_stop(void *ctx) { (void)ctx; }

static int exercise_zip(const char *zip)
{
    DM2_V1_BootProfile profile;
    DM2_V1_SoundQueueState state;
    DM2_V1_SoundQueueEnv env;
    DM2_V1_SoundQueueReceipt queued;
    DM2_V1_SoundPlayReceipt played;
    DM2_V1_SoundPlaybackBackend backend;
    Capture capture;
    const DM2_V1_GameLoadWorldOwner *owner;
    const DM2_V1_AssetLoader *loader;
    const DM2_V1_SoundSsoundEntry *source;
    int result = 0;

    memset(&profile, 0, sizeof(profile));
    memset(&capture, 0, sizeof(capture));
    memset(&backend, 0, sizeof(backend));
    backend.ctx = &capture;
    backend.is_ready = capture_ready;
    backend.start_voice = capture_start;
    backend.voice_active = capture_active;
    backend.stop_all = capture_stop;

    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, zip) != 0 ||
        !profile.assets_verified ||
        dm2_v1_boot_enter_game(&profile) != 0 ||
        !dm2_v1_boot_prepare_new_game_world(&profile)) {
        fprintf(stderr, "Mac ZIP did not reach authentic New Game: %s\n", zip);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    owner = (const DM2_V1_GameLoadWorldOwner *)
        dm2_v1_boot_prepared_new_game_world_readonly(&profile);
    loader = dm2_v1_boot_asset_loader(&profile);
    if (!owner || !loader || !owner->sound_owner.valid ||
        !owner->sound_owner.queue_entries ||
        owner->sound_owner.queue_entry_count == 0u) {
        fprintf(stderr, "Mac ZIP has no authenticated GAME_LOAD sound owner: %s\n",
                zip);
        result = 1;
        goto done;
    }

    source = &owner->sound_owner.queue_entries[0];
    if (source->w_00 < 0 || source->w_05 < 0) {
        fprintf(stderr, "Mac ZIP first authenticated sound row is unresolved: %s\n",
                zip);
        result = 1;
        goto done;
    }
    dm2_v1_sound_bind_gdat_loader(loader, 1);
    dm2_v1_sound_bind_playback_backend(&backend);
    dm2_v1_sound_queue_state_init(&state, owner->sound_owner.queue_entry_count);
    if (!dm2_v1_sound_queue_bind_entries(
            &state, owner->sound_owner.queue_entries,
            owner->sound_owner.queue_entry_count,
            owner->sound_owner.queue_capacity)) {
        result = 1;
        goto done;
    }
    state.sound_enabled = 1;
    state.master_sfx_volume = 1;
    memset(&env, 0, sizeof(env));
    env.facing = 0;
    env.current_map = 0;
    env.gate_map_a = 0;
    env.gate_map_b = 0;

    /* delay_mode 0 is the source's immediate scratch path. */
    if (!dm2_v1_sound_queue_noise_gen1(
            &state, source->b_02, source->b_03, source->b_04,
            1, 255, 0, 0, 0, &env, &queued) ||
        !queued.play_sound_requested || state.immediate_count != 0u) {
        fprintf(stderr, "Mac ZIP did not queue authenticated immediate SFX: %s\n",
                zip);
        result = 1;
        goto done;
    }
    memset(&played, 0, sizeof(played));
    if (!dm2_v1_sound_queue_play_sound(&state, &state.immediate[0], 1,
                                       &played) ||
        played.played_count != 1u || played.playback_unavailable ||
        capture.starts != 1u || capture.sample_count == 0u ||
        state.sample_slots[0] != source->w_00) {
        fprintf(stderr, "Mac ZIP authenticated SFX did not reach capture backend: %s\n",
                zip);
        result = 1;
        goto done;
    }
    /* The original slot-state table must remain recyclable after a voice
     * finishes.  A raw GDAT offset such as 1269 is outside that table and
     * would incorrectly consume a new slot on every source SFX. */
    capture.starts = 0u;
    memset(&played, 0, sizeof(played));
    if (!dm2_v1_sound_queue_play_sound(&state, &state.immediate[0], 1,
                                       &played) ||
        played.played_count != 1u || capture.starts != 1u ||
        state.sample_slots[0] != source->w_00) {
        fprintf(stderr, "Mac ZIP SFX slot was not recyclable: %s\n", zip);
        result = 1;
        goto done;
    }
    printf("PASS: authenticated Mac SFX raw=%d pcm=%u from %s\n",
           source->w_05, capture.sample_count, profile.version_id);

done:
    dm2_v1_sound_stop_all_voices();
    dm2_v1_sound_bind_playback_backend(NULL);
    dm2_v1_boot_cleanup(&profile);
    return result;
}

int main(void)
{
    const char *retail = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    int ran = 0;
    int failed = 0;
    if (retail && retail[0]) { ran = 1; failed |= exercise_zip(retail); }
    if (!ran) {
        puts("SKIP: DM2 Mac retail ZIP environment is not set");
        return 77;
    }
    return failed ? 1 : 0;
}
