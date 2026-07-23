/* test_dm2_v1_sound_playback_sdl.c — DM2-008 audible playback backend.
 *
 * Verifies voice allocation, PCM decode, and real SDL3 playback of samples
 * decoded from verified GRAPHICS.DAT audio raw entries, behind the fail-closed
 * contract: nothing plays unless the sample decodes from a verified GDAT
 * entry and the SDL backend reports ready.  Runs headless with
 * SDL_AUDIODRIVER=dummy (set by the ctest environment); skips cleanly when no
 * local canonical DM2 data is present.
 *
 * Source: skproject/SKWIN/SkwinSDL.cpp (OpenAudio 6000 Hz, MAX_SB = 16,
 * sdlAudMix), SKULLWIN/c_sound.cpp:256-308 (R_928 metric).
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_sound.h"
#include "dm2_v1_sound_sdl_backend.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static int load_graphics_dat(uint8_t **graphics, size_t *graphics_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char default_root[1024];
    char graphics_path[1100];

    if (!root || !root[0]) {
        if (!home || !home[0]) return 0;
        snprintf(default_root, sizeof(default_root),
                 "%s/.firestaff/data/dm2/data", home);
        root = default_root;
    }
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    return read_file(graphics_path, graphics, graphics_size);
}

/* Find the loadable SOUND entry with the smallest payload (fast completion)
 * and the one with the largest payload (slow completion for voice-exhaustion
 * checks). */
static int find_sound_entries(const DM2_V1_AssetLoader *loader,
                              uint8_t *small_cat, uint8_t *small_idx,
                              uint8_t *small_field, uint32_t *small_len,
                              uint8_t *large_cat, uint8_t *large_idx,
                              uint8_t *large_field, uint32_t *large_len)
{
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt entry;
    int found = 0;

    memset(&iterator, 0, sizeof(iterator));
    iterator.category_first = 0;
    iterator.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iterator.index_filter = -1;
    iterator.type_filter = DM2_GDAT_ENTRY_TYPE_SOUND;
    iterator.field_filter = -1;
    while (dm2_v1_query_next_gdat_entry(loader, &iterator, &entry)) {
        if (!entry.loadable_raw || entry.raw_length <= 2u) continue;
        if (!found || entry.raw_length - 2u < *small_len) {
            *small_cat = entry.category;
            *small_idx = entry.index;
            *small_field = entry.field;
            *small_len = entry.raw_length - 2u;
        }
        if (!found || entry.raw_length - 2u > *large_len) {
            *large_cat = entry.category;
            *large_idx = entry.index;
            *large_field = entry.field;
            *large_len = entry.raw_length - 2u;
        }
        found = 1;
    }
    return found;
}

static int wait_for_voice_idle(unsigned voice, int timeout_ms)
{
    int waited = 0;
    while (waited < timeout_ms) {
        if (!dm2_v1_sound_voice_active(voice)) return 1;
        SDL_Delay(10);
        waited += 10;
    }
    return 0;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_SoundPlaybackBackend backend;
    DM2_V1_SoundPlaybackReceipt play;
    uint8_t small_cat = 0, small_idx = 0, small_field = 0;
    uint8_t large_cat = 0, large_idx = 0, large_field = 0;
    uint32_t small_len = 0u, large_len = 0u;
    uint64_t frames_before;
    unsigned i;
    int failures = 0;
    int raw_id;

    if (!load_graphics_dat(&graphics, &graphics_size)) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }

    memset(&loader, 0, sizeof(loader));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0) {
        fputs("FAIL: canonical DM2 GRAPHICS.DAT was not accepted\n", stderr);
        failures = 1;
        goto done;
    }
    if (!find_sound_entries(&loader, &small_cat, &small_idx, &small_field,
                            &small_len, &large_cat, &large_idx, &large_field,
                            &large_len)) {
        puts("SKIP: no loadable SOUND entries in local GRAPHICS.DAT");
        goto done;
    }

    dm2_v1_sound_bind_gdat_loader(&loader, 1);

    /* Fail-closed before any backend binding. */
    assert(dm2_v1_sound_play_gdat_entry(small_cat, small_idx, small_field,
                                        127, &play) == 0);
    assert(play.rejected_no_backend);

    /* Bind the real SDL3 backend (SDL_AUDIODRIVER=dummy from ctest). */
    dm2_v1_sound_sdl_backend_describe(&backend);
    dm2_v1_sound_bind_playback_backend(&backend);

    /* ── Voice 1: full audible playback of the smallest verified entry ── */
    assert(dm2_v1_sound_play_gdat_entry(small_cat, small_idx, small_field,
                                        127, &play) == 1);
    assert(play.valid && play.playback_started);
    assert(play.voice_slot == 0u);
    assert(play.volume == 127u);
    assert(play.attenuation == 127u); /* R_928: dx == dy == 0 keeps volume */
    assert(play.sample_count == small_len);
    assert(dm2_v1_sound_sdl_backend_is_ready());
    assert(dm2_v1_sound_sdl_backend_started_voice_count() == 1u);
    frames_before = dm2_v1_sound_sdl_backend_mixed_frames();
    assert(dm2_v1_sound_voice_active(play.voice_slot));
    /* The dummy driver consumes in real time; the shortest sample is well
     * under one second at 6000 Hz, so the voice must complete. */
    assert(wait_for_voice_idle(play.voice_slot, 5000));
    assert(dm2_v1_sound_sdl_backend_mixed_frames() > frames_before);

    /* ── Legacy sound_id play path resolves the raw binding from GDAT ── */
    {
        DM2_V1_SoundPcmReceipt pcm;
        assert(dm2_v1_sound_decode_gdat_pcm(small_cat, small_idx, small_field,
                                            NULL, 0u, &pcm) == 1);
        raw_id = (int)pcm.raw_index;
    }
    assert(dm2_v1_sound_play(raw_id, 100) == raw_id);
    assert(wait_for_voice_idle(0, 5000));
    /* Unresolvable ids stay fail-closed even with backend + loader. */
    assert(dm2_v1_sound_play(-1, 100) == -1);
    assert(dm2_v1_sound_play(0, 100) == -1);

    /* ── Positional playback uses the source R_928 attenuation ── */
    assert(dm2_v1_sound_play_gdat_entry_positional(small_cat, small_idx,
                                                   small_field, 80, 4, 4,
                                                   &play) == 1);
    assert(play.attenuation == 16u); /* R_928(80, 4, 4) per c_sound.cpp:256 */
    assert(wait_for_voice_idle(play.voice_slot, 5000));
    assert(dm2_v1_sound_play_positional(raw_id, 4, 4, 0, 0) == raw_id);
    assert(wait_for_voice_idle(0, 5000));

    /* ── Voice allocation: MAX_SB = 16 voices, no stealing ── */
    for (i = 0; i < DM2_V1_SOUND_VOICE_MAX; ++i) {
        assert(dm2_v1_sound_play_gdat_entry(large_cat, large_idx, large_field,
                                            127, &play) == 1);
        assert(play.voice_slot == i);
    }
    /* 17th request finds no free voice and fails closed. */
    assert(dm2_v1_sound_play_gdat_entry(large_cat, large_idx, large_field,
                                        127, &play) == 0);
    assert(play.valid && play.rejected_no_free_voice);
    assert(!play.playback_started);
    assert(dm2_v1_sound_sdl_backend_started_voice_count() ==
           1u + 1u + 1u + 1u + DM2_V1_SOUND_VOICE_MAX);

    /* stop_all frees every voice; allocation restarts at slot 0. */
    dm2_v1_sound_stop_all_voices();
    for (i = 0; i < DM2_V1_SOUND_VOICE_MAX; ++i)
        assert(!dm2_v1_sound_voice_active(i));
    assert(dm2_v1_sound_play_gdat_entry(small_cat, small_idx, small_field,
                                        127, &play) == 1);
    assert(play.voice_slot == 0u);
    dm2_v1_sound_stop_all_voices();

    printf("PASS: SDL playback backend verified against real GRAPHICS.DAT "
           "(small entry %u/%u/%u len=%lu, large entry %u/%u/%u len=%lu)\n",
           (unsigned)small_cat, (unsigned)small_idx, (unsigned)small_field,
           (unsigned long)small_len,
           (unsigned)large_cat, (unsigned)large_idx, (unsigned)large_field,
           (unsigned long)large_len);

done:
    dm2_v1_sound_bind_playback_backend(NULL);
    dm2_v1_sound_sdl_backend_close();
    dm2_v1_sound_bind_gdat_loader(NULL, 0);
    free(graphics);
    return failures;
}
