#include "dm2_v1_dos_real_data_manifest.h"
#include "dm2_v1_mve_timeline.h"
#include "firestaff_zip_extract.h"

/* Original MVE display/audio boundary receipts must remain active in Release. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
    static const uint32_t expected_displays[] = { 217u, 600u };
    static const uint64_t expected_bytes[] = { 797426u, 2204900u };
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    size_t movie_index;
    if (!archive || !archive[0]) {
        puts("SKIP: no DM2 DOS archive");
        return 0;
    }
    for (movie_index = 0u; movie_index < 2u; ++movie_index) {
        const dm2_v1_dos_file_fp_t *fp =
            dm2_v1_dos_file_fp_lookup_pc34(names[movie_index]);
        DM2_V1_MveTimelineReceipt receipt;
        DM2_V1_MveDisplayAudioBoundary *boundaries;
        uint8_t *bytes;
        size_t size;
        uint32_t index;
        bytes = read_original_member(archive, names[movie_index], &size);
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
