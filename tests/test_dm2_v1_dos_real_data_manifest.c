#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_dos_startup_media.h"
#include "dm2_v1_mve_stream.h"
#include "dm2_v1_mve_video.h"
#include "dm2_v1_mve_pcm.h"
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

int main(void) {
    /* Sanity lookups. */
    const dm2_v1_dos_file_fp_t *e = dm2_v1_dos_file_fp_lookup_pc34("skull.exe");
    assert(e != NULL);
    assert(e->size_bytes == 522637u);
    assert(dm2_v1_dos_file_fp_lookup_pc34("does-not-exist") == NULL);
    assert(dm2_v1_dos_file_fp_lookup_pc34(NULL) == NULL);

    /* Match/mismatch. */
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes, e->sha256) == 1);
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes + 1u, e->sha256) == 0);
    uint8_t bogus[32] = {0};
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes, bogus) == 0);
    assert(dm2_v1_dos_file_fp_matches_pc34("skull.exe",
        e->size_bytes, NULL) == 0);

    /* Uniqueness: no duplicate names or digests. */
    for (int i = 0; i < DM2_V1_DOS_FILE_COUNT; ++i) {
        for (int j = i + 1; j < DM2_V1_DOS_FILE_COUNT; ++j) {
            assert(strcmp(dm2_v1_dos_files[i].name,
                          dm2_v1_dos_files[j].name) != 0);
            assert(memcmp(dm2_v1_dos_files[i].sha256,
                          dm2_v1_dos_files[j].sha256, 32) != 0);
        }
    }

    /* Real-data size check when a root is provided. */
    const char *root = getenv("FIRESTAFF_DM2_DOS_ROOT");
    if (!root) { puts("SKIP: no DM2 DOS root"); goto done; }
    int ok = 0;
    for (int i = 0; i < DM2_V1_DOS_FILE_COUNT; ++i) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", root,
                 dm2_v1_dos_files[i].name);
        FILE *fp = fopen(path, "rb");
        if (!fp) { printf("MISS: %s\n", dm2_v1_dos_files[i].name); continue; }
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fclose(fp);
        if ((size_t)sz == dm2_v1_dos_files[i].size_bytes) ++ok;
        else printf("SIZE-MISMATCH: %s (expected %zu, actual %ld)\n",
            dm2_v1_dos_files[i].name, dm2_v1_dos_files[i].size_bytes, sz);
    }
    printf("PASS: %d/%d DM2 DOS files match manifest size\n",
        ok, DM2_V1_DOS_FILE_COUNT);
    assert(ok == DM2_V1_DOS_FILE_COUNT);
    {
        DM2_V1_DosStartupMediaReceipt startup;
        assert(dm2_v1_dos_startup_media_probe(root, &startup) == 1);
        assert(startup.valid && startup.complete &&
               startup.batch_dispatches_ibmiop && startup.ibmiop_verified &&
               startup.splash_verified && startup.ftl_verified &&
               startup.intro_verified && startup.end_verified &&
               startup.intrplay_pcx_verified &&
               startup.intro_has_interplay_mve && startup.end_has_interplay_mve &&
               startup.intro_mve_header_offset > 0u &&
               startup.end_mve_header_offset > 0u && startup.receipt_hash != 0u);
        puts("PASS: DM2 DOS IBMIOP/MVE startup route matches retail media");
    }
    {
        static const char *const movies[] = { "intro", "end" };
        static const uint32_t expected_presentations[] = { 217u, 600u };
        static const uint32_t expected_pcm_bytes[] = { 797426u, 2204900u };
        static const uint32_t expected_pcm_fnv1a[] = { 0x2cb44b6au, 0x107b1929u };
        static const char *const expected_frame_sha256[][3] = {
            { "f4f285b4f4b97058bb408c7747f0354761415ea029078320df0e932231e0746c",
              "550c99c3cec2885c21871a24f552b93a421f6788b93f7115029ffe55e0eb0a5c",
              "6bdfc70fae47fb3cb8b18a7b574e62b9e6af0b63cb25f2ebc8033fb436d2de17" },
            { "4f7988030a00d082fe445e00a2ac5dab502300ff1b80e8592dd569867b60ef74",
              "efc94f0c32ef2dac2594398d934de13086ecb1dbe0d198df1daee38bbe274522",
              "ff833bf7b168df74a9ec24a45e9d0e0132dbd4a8a4bc79fafbcfd89acba8a26c" }
        };
        for (size_t i = 0u; i < sizeof(movies) / sizeof(movies[0]); ++i) {
            char path[1024];
            DM2_V1_MveStreamReceipt mve;
            DM2_V1_MvePresentationIterator iterator;
            DM2_V1_MvePresentation presentation;
            DM2_V1_MveVideo video;
            uint32_t presentation_count = 0u;
            uint64_t previous_time = 0u;
            size_t size = 0u;
            uint8_t *bytes;
            snprintf(path, sizeof(path), "%s/%s", root, movies[i]);
            bytes = read_original(path, &size);
            assert(bytes != NULL);
            assert(dm2_v1_mve_stream_parse(bytes, size, &mve) == 1);
            assert(mve.valid && mve.mve_offset == 100206u &&
                   mve.width == 320u && mve.height == 200u &&
                   mve.timer_rate_us != 0u && mve.timer_subdivision != 0u &&
                   mve.video_frame_count != 0u && mve.display_count != 0u &&
                   mve.receipt_hash != 0u);
            assert(dm2_v1_mve_presentation_iterator_init(&iterator, bytes,
                                                           size) == 1);
            dm2_v1_mve_video_init(&video);
            for (;;) {
                const int next = dm2_v1_mve_presentation_iterator_next(
                    &iterator, &presentation);
                if (next == 0) break;
                assert(next == 1);
                assert(presentation.valid &&
                       presentation.presentation_index == presentation_count &&
                       presentation.code_map_size == 500u &&
                       presentation.video_version == 3u &&
                       presentation.video_data_size > 0u &&
                       presentation.audio_sample_rate == 22050u &&
                       presentation.audio_channels == 2u &&
                       presentation.audio_bits == 8u &&
                       presentation.audio_compressed == 0u &&
                       presentation.transport13_size == 132u);
                assert(dm2_v1_mve_video_decode_presentation(&video,
                                                              &presentation,
                                                              bytes, size) == 1);
                if (presentation.presentation_index == 0u ||
                    presentation.presentation_index ==
                        (expected_presentations[i] - 1u) / 2u ||
                    presentation.presentation_index + 1u == expected_presentations[i]) {
                    char actual[65];
                    unsigned int sample = presentation.presentation_index == 0u ? 0u :
                        (presentation.presentation_index + 1u == expected_presentations[i] ? 2u : 1u);
                    assert(firestaff_x68k_media_receipt_sha256_hex(
                               dm2_v1_mve_video_pixels(&video),
                               DM2_V1_MVE_VIDEO_PIXELS,
                               actual, sizeof(actual)) == 0);
                    assert(strcmp(actual, expected_frame_sha256[i][sample]) == 0);
                }
                if (presentation_count != 0u)
                    assert(presentation.presentation_time_us - previous_time ==
                           10416u * 8u);
                previous_time = presentation.presentation_time_us;
                ++presentation_count;
            }
            assert(presentation_count == expected_presentations[i] &&
                   presentation_count == mve.display_count &&
                   iterator.width == 320u && iterator.height == 200u &&
                   iterator.timer_rate_us == 10416u &&
                   iterator.timer_subdivision == 8u);
            assert(video.decoded_presentations == presentation_count &&
                   dm2_v1_mve_video_pixels(&video) != NULL &&
                   dm2_v1_mve_video_palette_rgb(&video) != NULL);
            {
                DM2_V1_MveAudioIterator audio_iterator;
                DM2_V1_MvePcm pcm;
                DM2_V1_MvePcmSourceFrame source_frame;
                DM2_V1_MvePcmFrame pcm_frame;
                uint32_t pcm_count = 0u;
                uint32_t pcm_bytes = 0u;
                uint32_t pcm_fnv1a = 2166136261u;
                assert(dm2_v1_mve_audio_iterator_init(&audio_iterator, bytes,
                                                       size) == 1);
                dm2_v1_mve_pcm_init(&pcm);
                for (;;) {
                    const int next = dm2_v1_mve_audio_iterator_next(&audio_iterator,
                                                                       &source_frame);
                    if (next == 0) break;
                    assert(next == 1 && source_frame.valid &&
                           source_frame.source_sequence == pcm_count &&
                           source_frame.source_stream_mask == 1u);
                    assert(dm2_v1_mve_pcm_decode_source_frame(&pcm, &source_frame,
                                                               bytes, size,
                                                               &pcm_frame) == 1);
                    assert(pcm_frame.valid && pcm_frame.sample_bytes != 0u &&
                           pcm_frame.sample_frames * 2u == pcm_frame.sample_bytes);
                    pcm_bytes += pcm_frame.sample_bytes;
                    pcm_fnv1a = fnv1a_bytes(pcm_fnv1a, pcm_frame.samples,
                                             pcm_frame.sample_bytes);
                    ++pcm_count;
                }
                assert(pcm.initialized && pcm.sample_rate == 22050u &&
                       pcm.channels == 2u && pcm.bits == 8u &&
                       pcm.decoded_frame_count == pcm_count &&
                       pcm_count == expected_presentations[i] &&
                       pcm_bytes == expected_pcm_bytes[i] &&
                       pcm_fnv1a == expected_pcm_fnv1a[i]);
            }
            free(bytes);
        }
        puts("PASS: DM2 DOS MVE streams expose every original PAL8 and PCM presentation in RAM");
    }

done:
    puts("All dm2_v1_dos_real_data_manifest tests passed.");
    return 0;
}
