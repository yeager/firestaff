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

static void test_f0061_loud_table_and_index_mask(void)
{
    CsbV1PsgChannelAmplitudes amplitudes;

    amplitudes = csb_v1_audio_runtime_channel_amplitudes(8);
    CHECK(amplitudes.channelA == 14);
    CHECK(amplitudes.channelB == 8);
    CHECK(amplitudes.channelC == 0);

    amplitudes = csb_v1_audio_runtime_channel_amplitudes(15);
    CHECK(amplitudes.channelA == 14);
    CHECK(amplitudes.channelB == 14);
    CHECK(amplitudes.channelC == 14);

    amplitudes = csb_v1_audio_runtime_channel_amplitudes(24);
    CHECK(amplitudes.channelA == 14);
    CHECK(amplitudes.channelB == 8);
    CHECK(amplitudes.channelC == 0);

    amplitudes = csb_v1_audio_runtime_channel_amplitudes(-1);
    CHECK(amplitudes.channelA == 14);
    CHECK(amplitudes.channelB == 14);
    CHECK(amplitudes.channelC == 14);

    amplitudes = csb_v1_audio_runtime_channel_amplitudes_soft(8);
    CHECK(amplitudes.channelA == 11);
    CHECK(amplitudes.channelB == 3);
    CHECK(amplitudes.channelC == 1);

    amplitudes = csb_v1_audio_runtime_channel_amplitudes_soft(15);
    CHECK(amplitudes.channelA == 12);
    CHECK(amplitudes.channelB == 9);
    CHECK(amplitudes.channelC == 0);
}

static CsbV1AudioRequest make_request(int16_t soundIndex,
                                      int16_t mode,
                                      int16_t volume,
                                      uint8_t priority)
{
    CsbV1AudioRequest request = {0};

    request.soundIndex = soundIndex;
    request.mode = mode;
    request.volume = volume;
    request.priority = priority;
    return request;
}

static void test_f0064_f0065_pending_sound_runtime(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioSaveSnapshot snapshot;
    CsbV1AudioRequest quiet = make_request(
        CSB_V1_SOUND_SWITCH, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 32, 3);
    CsbV1AudioRequest louder = make_request(
        CSB_V1_SOUND_COMBAT, CSB_V1_MODE_PLAY_ONE_TICK_LATER, 48, 1);
    CsbV1AudioRequest equalHigherPriority = make_request(
        CSB_V1_SOUND_SPELL, CSB_V1_MODE_PLAY_IF_PRIORITIZED, 48, 6);
    CsbV1AudioRequest immediate = make_request(
        CSB_V1_SOUND_PARTY_DAMAGED, CSB_V1_MODE_PLAY_IMMEDIATELY, 64, 0);
    CsbV1AudioRequest invalidMode = make_request(
        CSB_V1_SOUND_BUZZ, 3, 64, 7);

    csb_v1_audio_runtime_init(&runtime);
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE);
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_NONE);
    CHECK(runtime.lastCreatureAttackTime == -200);

    CHECK(csb_v1_audio_runtime_request(&runtime, &quiet) == 1);
    CHECK(csb_v1_audio_runtime_request(&runtime, &louder) == 1);
    CHECK(csb_v1_audio_runtime_request(&runtime, &equalHigherPriority) == 1);
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_SPELL);
    CHECK(runtime.pendingVolume == 48);
    CHECK(runtime.pendingPriority == 6);

    CHECK(csb_v1_audio_runtime_request(&runtime, &immediate) == 1);
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_PARTY_DAMAGED);
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE);
    CHECK(runtime.totalImmediatePlays == 1);
    CHECK(csb_v1_audio_runtime_flush_pending(&runtime) == 0);

    CHECK(csb_v1_audio_runtime_request(&runtime, &equalHigherPriority) == 1);
    CHECK(csb_v1_audio_runtime_flush_pending(&runtime) == 1);
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_SPELL);
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE);
    CHECK(runtime.totalPendingFlushes == 1);

    CHECK(csb_v1_audio_runtime_request(&runtime, &invalidMode) == 0);
    CHECK(runtime.totalRejectedRequests == 1);

    csb_v1_audio_runtime_record_creature_attack(&runtime, 733);
    csb_v1_audio_runtime_save_snapshot(&runtime, &snapshot);
    CHECK(snapshot.lastCreatureAttackTime == 733);
    CHECK(csb_v1_audio_runtime_request(&runtime, &quiet) == 1);
    csb_v1_audio_runtime_load_snapshot(&runtime, &snapshot);
    CHECK(runtime.lastCreatureAttackTime == 733);
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE);
}

static void test_load_snapshot_restarts_transient_audio_state(void)
{
    CsbV1AudioRuntime runtime;
    CsbV1AudioSaveSnapshot snapshot;
    CsbV1AudioRequest request = make_request(
        CSB_V1_SOUND_COMBAT, CSB_V1_MODE_PLAY_ONE_TICK_LATER, 64, 2);

    csb_v1_audio_runtime_init(&runtime);
    csb_v1_audio_runtime_record_creature_attack(&runtime, 1842);
    csb_v1_audio_runtime_save_snapshot(&runtime, &snapshot);
    CHECK(csb_v1_audio_runtime_request(&runtime, &request) == 1);
    CHECK(csb_v1_audio_runtime_flush_pending(&runtime) == 1);
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_COMBAT);

    csb_v1_audio_runtime_init(&runtime);
    CHECK(runtime.lastCreatureAttackTime == -200);
    csb_v1_audio_runtime_load_snapshot(&runtime, &snapshot);
    CHECK(runtime.lastCreatureAttackTime == 1842);
    CHECK(runtime.pendingSoundIndex == CSB_V1_SOUND_NONE);
    CHECK(runtime.lastPlayedSoundIndex == CSB_V1_SOUND_NONE);
}

static void test_atari_st_source_table_stops_before_absent_sounds(void)
{
    const CsbV1AtariStSoundSpec *first;
    const CsbV1AtariStSoundSpec *last;

    first = csb_v1_audio_runtime_atari_st_sound_spec(0);
    last = csb_v1_audio_runtime_atari_st_sound_spec(
        CSB_V1_ATARI_ST_SOUND_COUNT - 1);
    CHECK(first != NULL);
    CHECK(first && first->graphicIndex == 533u && first->period == 112u &&
          first->priority == 11u && first->loudDistance == 3u &&
          first->softDistance == 6u);
    CHECK(last != NULL);
    CHECK(last && last->graphicIndex == 555u && last->priority == 96u);
    CHECK(csb_v1_audio_runtime_atari_st_sound_spec(-1) == NULL);
    CHECK(csb_v1_audio_runtime_atari_st_sound_spec(
              CSB_V1_ATARI_ST_SOUND_COUNT) == NULL);
    /* ReDMCSB's next cross-platform row names graphic 563 explicitly as
     * absent on Atari ST. It is also the first index after the 563-item file. */
    CHECK(csb_v1_audio_runtime_atari_st_sound_spec(22) == NULL);
}

int main(void)
{
    test_high_nibble_first_and_repeat_runs();
    test_leading_repeat_uses_existing_psg_level();
    test_malformed_and_short_output_are_rejected();
    test_f0061_loud_table_and_index_mask();
    test_f0064_f0065_pending_sound_runtime();
    test_load_snapshot_restarts_transient_audio_state();
    test_atari_st_source_table_stops_before_absent_sounds();
    printf("test_csb_v1_f0060_sound_play_pc34_compat: %d failures\n", failures);
    return failures != 0;
}
