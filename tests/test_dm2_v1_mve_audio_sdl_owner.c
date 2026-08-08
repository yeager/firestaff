#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_audio_sdl_owner.h"
#include "dm2_v1_mve_presentation_owner.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t *read_original(const char *path, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes;
    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (uint8_t *)malloc((size_t)length)) ||
        fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)length;
    return bytes;
}

int main(void)
{
    static const char *const names[] = { "intro", "end" };
    static const uint32_t expected_packets[] = { 217u, 600u };
    static const uint64_t expected_bytes[] = { 797426u, 2204900u };
    const char *root = getenv("FIRESTAFF_DM2_DOS_ROOT");
    size_t movie_index;

    if (!root) {
        puts("SKIP: no DM2 DOS root");
        return 0;
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
        char path[1024];
        uint32_t packet_count = 0u;

        assert(fingerprint != NULL);
        snprintf(path, sizeof(path), "%s/%s", root, names[movie_index]);
        bytes = read_original(path, &byte_count);
        assert(bytes != NULL && byte_count == fingerprint->size_bytes);
        assert(dm2_v1_mve_presentation_owner_init(&presentation, bytes,
                                                   byte_count) == 1);
        assert(dm2_v1_mve_audio_sdl_owner_open(&audio) == 1);
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
        dm2_v1_mve_audio_sdl_owner_close(&audio);
        free(bytes);
    }
    puts("PASS: DM2 MVE SDL owner queues only original U8 stereo PCM");
    return 0;
}
