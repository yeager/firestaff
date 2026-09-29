#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_audio_sdl_owner.h"
#include "dm2_v1_mve_presentation_owner.h"
#include "firestaff_zip_extract.h"

/* Original PCM queue receipts must remain active in Release. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

static uint8_t *read_original_member(const char *archive, const char *name,
                                     size_t *out_size)
{
    uint8_t *bytes = NULL;
    if (!archive || !archive[0] || !name || !out_size ||
        firestaff_zip_extract_by_suffix(archive, name, &bytes, out_size) != 0)
        return NULL;
    return bytes;
}

int main(void)
{
    static const char *const names[] = { "intro", "end" };
    static const uint32_t expected_packets[] = { 217u, 600u };
    static const uint64_t expected_bytes[] = { 797426u, 2204900u };
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    size_t movie_index;

    if (!archive || !archive[0]) {
        puts("SKIP: no DM2 DOS archive");
        return 77;
    }
    for (movie_index = 0u; movie_index < sizeof(names) / sizeof(names[0]);
         ++movie_index) {
        const dm2_v1_dos_file_fp_t *fingerprint =
            dm2_v1_dos_file_fp_lookup_pc34(names[movie_index]);
        DM2_V1_MvePresentationOwner presentation;
        DM2_V1_MveAudioSdlOwner audio;
        DM2_V1_MvePcmFrame frame;
        uint8_t *bytes;
        size_t byte_count;
        uint32_t packet_count = 0u;

        assert(fingerprint != NULL);
        bytes = read_original_member(archive, names[movie_index], &byte_count);
        assert(bytes != NULL && byte_count == fingerprint->size_bytes);
        assert(dm2_v1_mve_presentation_owner_init(&presentation, bytes,
                                                   byte_count) == 1);
        assert(dm2_v1_mve_audio_sdl_owner_open(&audio) == 1);
        assert(audio.master_volume == 128);
        assert(dm2_v1_mve_audio_sdl_owner_set_host_paused(&audio, 1));
        for (;;) {
            const int next = dm2_v1_mve_presentation_owner_next_source_pcm(
                &presentation, &frame);
            if (next == 0) break;
            assert(next == 1 && frame.valid &&
                   frame.source_sequence == packet_count &&
                   frame.source_stream_mask == 1u);
            assert(dm2_v1_mve_audio_sdl_owner_queue(&audio, &frame) == 1);
            ++packet_count;
        }
        assert(packet_count == expected_packets[movie_index]);
        assert(audio.queued_source_packets == packet_count &&
               audio.queued_source_bytes == expected_bytes[movie_index] &&
               audio.queued_sample_frames == expected_bytes[movie_index] / 2u);
        {
            static const int volumes[] = {64, 0, -1, 256, 128};
            static const int expected[] = {64, 0, 0, 128, 128};
            unsigned check;
            SDL_AudioStream* stream = (SDL_AudioStream*)audio.sdl_stream;
            int queued = stream ? SDL_GetAudioStreamQueued(stream) : 0;
            for (check = 0; check < sizeof(volumes) / sizeof(volumes[0]); ++check) {
                assert(dm2_v1_mve_audio_sdl_owner_set_master_volume(&audio, volumes[check]));
                assert(audio.master_volume == expected[check] && audio.host_paused);
                assert(audio.queued_source_packets == packet_count &&
                       audio.queued_source_bytes == expected_bytes[movie_index] &&
                       audio.queued_sample_frames == expected_bytes[movie_index] / 2U &&
                       audio.next_source_sequence == packet_count);
                if (stream) {
                    assert(SDL_GetAudioStreamGain(stream) == (float)expected[check] / 128.0f);
                    assert(SDL_AudioStreamDevicePaused(stream));
                    assert(SDL_GetAudioStreamQueued(stream) == queued);
                } else {
                    assert(audio.output_unavailable);
                }
            }
            assert(dm2_v1_mve_audio_sdl_owner_set_host_paused(&audio, 0));
            assert(!audio.host_paused);
            if (stream) assert(!SDL_AudioStreamDevicePaused(stream));
        }
        dm2_v1_mve_audio_sdl_owner_close(&audio);
        assert(!audio.initialized && !audio.sdl_stream && audio.master_volume == 0);
        free(bytes);
    }
    puts("PASS: DM2 MVE SDL owner preserves original PCM and pause under live master gain");
    return 0;
}
