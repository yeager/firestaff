#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_timeline.h"

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
    static const uint32_t expected_displays[] = { 217u, 600u };
    static const uint64_t expected_bytes[] = { 797426u, 2204900u };
    const char *root = getenv("FIRESTAFF_DM2_DOS_ROOT");
    size_t movie_index;
    if (!root) {
        puts("SKIP: no DM2 DOS root");
        return 0;
    }
    for (movie_index = 0u; movie_index < 2u; ++movie_index) {
        const dm2_v1_dos_file_fp_t *fp =
            dm2_v1_dos_file_fp_lookup_pc34(names[movie_index]);
        DM2_V1_MveTimelineReceipt receipt;
        DM2_V1_MveDisplayAudioBoundary *boundaries;
        uint8_t *bytes;
        char path[1024];
        size_t size;
        uint32_t index;
        snprintf(path, sizeof(path), "%s/%s", root, names[movie_index]);
        bytes = read_original(path, &size);
        assert(fp && bytes && size == fp->size_bytes);
        assert(dm2_v1_mve_display_audio_timeline(bytes, size, NULL, 0u,
                                                  &receipt) == 1);
        assert(receipt.valid && receipt.presentation_count == expected_displays[movie_index] &&
               receipt.audio_packet_count == expected_displays[movie_index] &&
               receipt.timer_rate_us == 10416u && receipt.timer_subdivision == 8u &&
               receipt.prebuffer_audio_packet_count == 12u &&
               receipt.terminal_silent_presentation_count == 11u &&
               receipt.unpresented_audio_packet_count == 0u &&
               receipt.audio_sample_bytes == expected_bytes[movie_index] &&
               receipt.audio_sample_frames == expected_bytes[movie_index] / 2u);
        boundaries = calloc(receipt.presentation_count, sizeof(*boundaries));
        assert(boundaries && dm2_v1_mve_display_audio_timeline(
            bytes, size, boundaries, receipt.presentation_count, &receipt) == 1);
        assert(boundaries[0].valid && boundaries[0].presentation_time_us == 0u &&
               boundaries[0].first_audio_source_sequence == 0u &&
               boundaries[0].audio_packet_count == 12u);
        for (index = 1u; index + 11u < receipt.presentation_count; ++index) {
            assert(boundaries[index].valid &&
                   boundaries[index].presentation_time_us == (uint64_t)index * 83328u &&
                   boundaries[index].first_audio_source_sequence == index + 11u &&
                   boundaries[index].audio_packet_count == 1u);
        }
        for (; index < receipt.presentation_count; ++index)
            assert(boundaries[index].audio_packet_count == 0u);
        free(boundaries);
        free(bytes);
    }
    puts("PASS: DM2 MVE timeline retains retail PCM/display byte-order boundaries");
    return 0;
}
