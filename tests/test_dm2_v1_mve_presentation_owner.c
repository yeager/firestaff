#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_presentation_owner.h"
#include "firestaff_x68k_media_receipt.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static uint32_t fnv1a_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{
    size_t i;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    static const char *const names[] = { "intro", "end" };
    static const uint32_t expected_frames[] = { 217u, 600u };
    static const uint32_t expected_pcm_bytes[] = { 797426u, 2204900u };
    static const uint32_t expected_pcm_hash[] = { 0x2cb44b6au, 0x107b1929u };
    static const char *const expected_final_frame_hash[] = {
        "6bdfc70fae47fb3cb8b18a7b574e62b9e6af0b63cb25f2ebc8033fb436d2de17",
        "ff833bf7b168df74a9ec24a45e9d0e0132dbd4a8a4bc79fafbcfd89acba8a26c"
    };
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
        DM2_V1_MvePresentationOwner owner;
        DM2_V1_MvePresentationFrame frame;
        uint8_t *bytes;
        size_t size;
        char path[1024];
        uint32_t frame_count = 0u;
        uint32_t source_pcm_count = 0u;
        uint32_t pcm_hash = 2166136261u;
        char image_hash[65];

        assert(fingerprint != NULL);
        snprintf(path, sizeof(path), "%s/%s", root, names[movie_index]);
        bytes = read_original(path, &size);
        assert(bytes != NULL && size == fingerprint->size_bytes);
        assert(dm2_v1_mve_presentation_owner_init(&owner, bytes, size) == 1);
        assert(owner.presentation_interval_us == 83328u);
        for (;;) {
            const int next = dm2_v1_mve_presentation_owner_next(&owner, &frame);
            if (next == 0) break;
            assert(next == 1 && frame.valid);
            assert(frame.presentation_index == frame_count &&
                   frame.presentation_time_us == (uint64_t)frame_count * 83328u &&
                   frame.presentation_interval_us == 83328u &&
                   frame.indexed_pixels != NULL && frame.palette_rgb != NULL);
            ++frame_count;
        }
        for (;;) {
            DM2_V1_MvePcmFrame source_pcm;
            const int next = dm2_v1_mve_presentation_owner_next_source_pcm(
                &owner, &source_pcm);
            if (next == 0) break;
            assert(next == 1 && source_pcm.valid &&
                   source_pcm.source_sequence == source_pcm_count &&
                   source_pcm.source_stream_mask == 1u);
            pcm_hash = fnv1a_bytes(pcm_hash, source_pcm.samples,
                                   source_pcm.sample_bytes);
            ++source_pcm_count;
        }
        assert(frame_count == expected_frames[movie_index] && owner.ended &&
               !owner.failed && owner.presented_frame_count == frame_count &&
               owner.retained_pcm_bytes == expected_pcm_bytes[movie_index] &&
               pcm_hash == expected_pcm_hash[movie_index] &&
               owner.video.decoded_presentations == frame_count &&
               owner.source_pcm.decoded_frame_count == source_pcm_count &&
               source_pcm_count == expected_frames[movie_index]);
        assert(firestaff_x68k_media_receipt_sha256_hex(
                   dm2_v1_mve_video_pixels(&owner.video),
                   DM2_V1_MVE_VIDEO_PIXELS, image_hash, sizeof(image_hash)) == 0);
        assert(strcmp(image_hash, expected_final_frame_hash[movie_index]) == 0);
        free(bytes);
    }
    puts("PASS: DM2 MVE owner retains source PAL8/PCM order and clock in RAM");
    return 0;
}
