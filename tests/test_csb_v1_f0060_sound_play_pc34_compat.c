#include "csb_v1_audio_runtime_pc34_compat.h"

#include <stdio.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

static void test_high_nibble_first_and_repeat_runs(void)
{
    /* Count=7: 5, then the zero command holds 5 and its (1 + 2) repeat
     * ticks keep holding it; 9 then begins the next amplitude. */
    const uint8_t encoded[] = {0x00, 0x07, 0x50, 0x19, 0x00};
    const uint8_t expected[] = {5, 5, 5, 5, 5, 9, 9};
    uint8_t levels[7];
    CsbV1StSoundDecodeResult result;
    size_t i;

    CHECK(csb_v1_audio_runtime_decode_st_sound(encoded, sizeof(encoded), 3,
                                                levels, sizeof(levels),
                                                &result) == 0);
    CHECK(result.sampleCount == sizeof(expected));
    CHECK(result.encodedBytesConsumed == sizeof(encoded));
    for (i = 0; i < sizeof(expected); ++i) {
        CHECK(levels[i] == expected[i]);
    }
}

static void test_leading_repeat_uses_existing_psg_level(void)
{
    /* F0060 leaves the PSG amplitude table active; its first zero command
     * therefore repeats the level supplied by the preceding audio state. */
    const uint8_t encoded[] = {0x00, 0x03, 0x00};
    uint8_t levels[3];
    CsbV1StSoundDecodeResult result;

    CHECK(csb_v1_audio_runtime_decode_st_sound(encoded, sizeof(encoded), 12,
                                                levels, sizeof(levels),
                                                &result) == 0);
    CHECK(levels[0] == 12 && levels[1] == 12 && levels[2] == 12);
    CHECK(result.encodedBytesConsumed == sizeof(encoded));
}

static void test_malformed_and_short_output_are_rejected(void)
{
    const uint8_t truncatedRun[] = {0x00, 0x01, 0x08};
    const uint8_t oneSample[] = {0x00, 0x01, 0x10};
    uint8_t level;
    CsbV1StSoundDecodeResult result;

    CHECK(csb_v1_audio_runtime_decode_st_sound(truncatedRun,
                                                sizeof(truncatedRun), 0,
                                                &level, 1, &result) == -2);
    CHECK(csb_v1_audio_runtime_decode_st_sound(oneSample, sizeof(oneSample),
                                                0, &level, 0, &result) == -3);
    CHECK(csb_v1_audio_runtime_decode_st_sound(oneSample, sizeof(oneSample),
                                                16, &level, 1, &result) == -1);
}

int main(void)
{
    test_high_nibble_first_and_repeat_runs();
    test_leading_repeat_uses_existing_psg_level();
    test_malformed_and_short_output_are_rejected();
    printf("test_csb_v1_f0060_sound_play_pc34_compat: %d failures\n", failures);
    return failures != 0;
}
