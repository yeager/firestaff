/* test_dm2_v1_sound_gdat_real_data.c — DM2-008 real GDAT sound backend.
 *
 * Verifies that DM2_PLAY_MUSIC, DM2_PLAY_SOUND, DM2_QUERY_SND_ENTRY_INDEX,
 * and the dm2sound.xsndptr2 seven-byte runtime queue are source-locked against
 * verified GRAPHICS.DAT audio raw entries.  When no local DM2 data is present
 * the test skips; when present it requires the exact fail-closed / real-data
 * behaviour documented in dm2_v1_sound.c.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_sound.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Find any loadable SOUND entry so the test does not hard-code a specific
 * (category, index, field) triple that might differ across variants. */
static int find_first_sound_entry(const DM2_V1_AssetLoader *loader,
                                  uint8_t *out_cat,
                                  uint8_t *out_idx,
                                  uint8_t *out_field)
{
    DM2_V1_GdatEntryIterator iterator;
    DM2_V1_GdatEntryQueryReceipt entry;

    if (!loader || !out_cat || !out_idx || !out_field) return 0;
    memset(&iterator, 0, sizeof(iterator));
    iterator.category_first = 0;
    iterator.category_last = DM2_GDAT_CATEGORY_LIMIT;
    iterator.index_filter = -1;
    iterator.type_filter = DM2_GDAT_ENTRY_TYPE_SOUND;
    iterator.field_filter = -1;
    while (dm2_v1_query_next_gdat_entry(loader, &iterator, &entry)) {
        if (entry.loadable_raw && entry.raw_length > 2u) {
            *out_cat = entry.category;
            *out_idx = entry.index;
            *out_field = entry.field;
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_SoundQueueState state;
    uint8_t sound_cat = 0;
    uint8_t sound_idx = 0;
    uint8_t sound_field = 0;
    uint16_t index;
    (void)index;
    int failures = 0;

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

    /* ── Fail-closed before any binding ── */
    assert(dm2_v1_sound_query_entry(0x0E, 0, 0x81) == -1);
    assert(dm2_v1_sound_play(0, 127) == -1);
    assert(dm2_v1_sound_play_positional(0, 1, 2, 3, 4) == -1);
    assert(dm2_v1_sound_play_music(0) == -1);

    /* DM2_SOUND9 without a binding queues an explicitly unbound entry. */
    dm2_v1_sound_queue_state_init(&state, 8);
    assert(dm2_v1_sound9(&state, 9, 9, 9, -1, &index) == 1);
    assert(state.ssound[index - 1u].w_00 == -1); /* unavailable, not faked */

    /* ── Bind verified GDAT loader ── */
    dm2_v1_sound_bind_gdat_loader(&loader, 1);

    /* The PC corpus owns 29 HMP records: index 00 through 1c inclusive.
     * Keep the final source track admitted; an old 28-entry limit silently
     * discarded index 1c despite the real GDAT entry being present. */
    {
        int track;
        for (track = 0; track < DM2_MUSIC_TRACK_COUNT; ++track) {
            DM2_V1_GdatEntryQueryReceipt entry;
            DM2_V1_MusicQueueReceipt music;
            char expected_path[sizeof(music.asset_path)];
            assert(dm2_v1_query_gdat_entry(
                &loader, DM2_GDAT_CATEGORY_MUSICS, track,
                DM2_GDAT_ENTRY_TYPE_HMP, 0, &entry) == 1);
            assert(entry.loadable_raw && entry.raw_length > 0u);
            /* Every original HMP payload must remain source-resolved but
             * fail-closed. A valid-looking header in any non-title track
             * cannot reopen the unproven direct-HMP scheduler path. */
            snprintf(expected_path, sizeof(expected_path),
                     "GRAPHICS.DAT::GDAT(04,%02x,03,00)", track);
            assert(dm2_v1_sound_queue_music(track, 1, &music) ==
                   DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE);
            assert(music.asset_resolved && !music.decoder_proven);
            assert(!music.schedule_handoff_ready);
            assert(music.schedule_event_count == 0u);
            assert(strcmp(music.asset_path, expected_path) == 0);
        }
    }

    /* SKWIN c_sound.cpp::DM2_PLAY_MUSIC(0, true) probes the title cue at
     * MUSICS/0/dtHMP/0.  It must come from the admitted PC GRAPHICS.DAT,
     * never a loose 00.hmp.mid sidecar.  A platform without a native MIDI
     * backend remains honestly silent after proving this source payload. */
    /* Track 00 is covered as part of the complete source corpus above. */

    /* ── DM2_SOUND9 populates the seven-byte xsndptr2 runtime queue ── */
    dm2_v1_sound_queue_state_init(&state, 8);
    assert(dm2_v1_sound9(&state, 1, 2, 3, 7, &index) == 1);
    assert(index == 1);
    assert(state.ssound[0].w_00 == 7);
    assert(state.ssound[0].b_02 == 1);
    assert(state.ssound[0].b_03 == 2);
    assert(state.ssound[0].b_04 == 3);
    assert(state.ssound[0].w_05 == -1);

    /* ── DM2_QUERY_SND_ENTRY_INDEX preserves original 1-based scan order ── */
    assert(dm2_v1_query_snd_entry_index(&state, 1, 2, 3) == 1);
    assert(dm2_v1_query_snd_entry_index(&state, 1, 2, 4) == 0);

    /* Duplicate DM2_SOUND9 is rejected (source order: query first). */
    assert(dm2_v1_sound9(&state, 1, 2, 3, 8, &index) == 0);

    /* ── DM2_QUERY_SND_ENTRY_INDEX with GDAT fallback ── */
    if (find_first_sound_entry(&loader, &sound_cat, &sound_idx, &sound_field)) {
        int q;
        (void)q;
        DM2_V1_GdatSoundEntryReceipt receipt;

        /* The same GDAT triple resolves to a real raw-entry sample binding. */
        assert(dm2_v1_gdat_sound_entry_receipt(&loader, sound_cat, sound_idx,
                                               sound_field, 0, 0,
                                               &receipt) == 1);
        assert(receipt.accepted);
        dm2_v1_sound_queue_state_init(&state, 8);
        assert(dm2_v1_sound9(&state, (int8_t)sound_cat, (int8_t)sound_idx,
                             (int8_t)sound_field, -1, &index) == 1);
        assert(state.ssound[index - 1u].w_00 == (int16_t)receipt.raw_index);

        /* First query call should add the entry to the fallback queue. */
        q = dm2_v1_sound_query_entry(sound_cat, sound_idx, sound_field);
        assert(q == 1);
        /* Second call should find it already queued and return the same index. */
        assert(dm2_v1_sound_query_entry(sound_cat, sound_idx, sound_field) == q);

        /* ── PCM decode from the verified GDAT entry (cycle 16) ── */
        {
            DM2_V1_SoundPcmReceipt pcm;
            DM2_V1_SoundPlaybackReceipt play;
            (void)play;
            uint32_t count = dm2_v1_sound_gdat_pcm_sample_count(
                sound_cat, sound_idx, sound_field);
            uint8_t *decoded;
            size_t raw_size = 0u;
            const uint8_t *raw;
            (void)raw;
            uint32_t i;

            assert(count == receipt.payload_length);
            assert(count > 0u);

            /* Query-only decode receipts the converted payload hash. */
            assert(dm2_v1_sound_decode_gdat_pcm(sound_cat, sound_idx,
                                                sound_field, NULL, 0u,
                                                &pcm) == 1);
            assert(pcm.accepted);
            assert(pcm.raw_index == receipt.raw_index);
            assert(pcm.sample_count == count);
            assert(pcm.sample_rate_hz == DM2_V1_SOUND_PCM_SAMPLE_RATE_HZ);
            assert(pcm.pcm_hash != 0u);

            /* Undersized destination is rejected without decoding. */
            memset(&pcm, 0, sizeof(pcm));
            assert(dm2_v1_sound_decode_gdat_pcm(sound_cat, sound_idx,
                                                sound_field,
                                                (uint8_t *)&raw_size,
                                                count - 1u, &pcm) == 0);
            assert(pcm.rejected_capacity);

            /* Full decode matches payload ^ 0x80 byte for byte. */
            decoded = (uint8_t *)malloc(count);
            assert(decoded != NULL);
            assert(dm2_v1_sound_decode_gdat_pcm(sound_cat, sound_idx,
                                                sound_field, decoded, count,
                                                &pcm) == 1);
            raw = dm2_v1_load_gdat_raw_data(&loader, receipt.raw_index,
                                            &raw_size);
            assert(raw != NULL);
            assert(raw_size >= receipt.raw_length);
            for (i = 0; i < count; ++i)
                assert(decoded[i] ==
                       (uint8_t)(raw[receipt.header_skip_bytes + i] ^ 0x80u));
            free(decoded);

            /* Unknown entry is rejected explicitly. */
            memset(&pcm, 0, sizeof(pcm));
            assert(dm2_v1_sound_decode_gdat_pcm(0xEE, 0xEE, 0xEE, NULL, 0u,
                                                &pcm) == 0);
            assert(pcm.rejected_entry_missing);

            /* Audible playback stays fail-closed without a playback backend:
             * the sample decodes, but no voice is allocated and nothing is
             * synthesized. */
            assert(dm2_v1_sound_play_gdat_entry(sound_cat, sound_idx,
                                                sound_field, 127,
                                                &play) == 0);
            assert(play.valid && play.rejected_no_backend);
            assert(!play.playback_started);
            assert(dm2_v1_sound_play_gdat_entry_positional(
                       sound_cat, sound_idx, sound_field, 127, 1, 2,
                       &play) == 0);
            assert(play.rejected_no_backend);
        }
    } else {
        puts("SKIP: no loadable SOUND entries in local GRAPHICS.DAT");
        goto done;
    }

    /* ── Playback remains fail-closed: no playback backend is bound ── */
    assert(dm2_v1_sound_play(0, 127) == -1);
    assert(dm2_v1_sound_play_positional(0, 1, 2, 3, 4) == -1);

    /* ── Music playback remains fail-closed without verified assets/backend ── */
    assert(dm2_v1_sound_play_music(0) == -1);

    printf("PASS: GDAT sound backend verified against real GRAPHICS.DAT "
           "(sound entry %u/%u/%u)\n",
           (unsigned)sound_cat, (unsigned)sound_idx, (unsigned)sound_field);

done:
    dm2_v1_sound_bind_gdat_loader(NULL, 0);
    free(graphics);
    return failures;
}
