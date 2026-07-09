#include "nexus_v1_sound.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        ++g_failures; \
    } \
} while (0)

static void test_missing_assets_block_playback(void) {
    Nexus_SoundEngine eng;
    Nexus_SfxRuntimeReceipt receipt;

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_sound_init(&eng) == 0, "sound engine initializes");
    CHECK(nexus_sound_load_level(&eng, 0, NULL, 0, NULL, 0) == 0,
          "empty level sound load succeeds for receipt");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
          "empty sound runtime receipt emits");
    CHECK(receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_MISSING_ASSET,
          "missing SAL/MAP blocks runtime SFX");
    CHECK(receipt.blocks_real_sfx_playback == 1 &&
          receipt.fallback_visuals_permitted == 0,
          "missing SFX route forbids fallback playback");
    CHECK(strcmp(nexus_sound_sfx_runtime_status_name(receipt.status),
                 "blocked-missing-asset") == 0,
          "missing SFX status name is stable");
    nexus_sound_shutdown(&eng);
}

static void test_size_matched_assets_block_decode(void) {
    Nexus_SoundEngine eng;
    Nexus_V1_AudioReceipt sal_expected;
    Nexus_V1_AudioReceipt map_expected;
    Nexus_SfxRuntimeReceipt receipt;
    static unsigned char sal_data[297082];
    static unsigned char map_data[66];

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_SAL_BANK,
                                        0,
                                        &sal_expected) == 0,
          "level 0 SAL expected row exists");
    CHECK(nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_MAP_TABLE,
                                        0,
                                        &map_expected) == 0,
          "level 0 MAP expected row exists");
    CHECK(sal_expected.expected_size == sizeof(sal_data) &&
          map_expected.expected_size == sizeof(map_data),
          "fixture sizes match level 0 audio receipts");
    CHECK(nexus_sound_init(&eng) == 0, "sound engine initializes");
    CHECK(nexus_sound_load_level(&eng,
                                 0,
                                 sal_data,
                                 (int)sizeof(sal_data),
                                 map_data,
                                 (int)sizeof(map_data)) == 0,
          "size-matched SAL/MAP load succeeds");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
          "size-matched sound runtime receipt emits");
    CHECK(receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE,
          "size-matched SAL/MAP blocks on unsupported decode");
    CHECK(receipt.sal_receipt.receipt_class ==
              NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH &&
          receipt.map_receipt.receipt_class ==
              NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH,
          "runtime receipt preserves SAL/MAP size-match classes");
    CHECK(receipt.cd_track == 2 &&
          receipt.level_index == 0 &&
          receipt.playback_enabled == 0,
          "runtime receipt exposes level 0 CD track without enabling playback");
    CHECK(receipt.blocks_real_sfx_playback == 1 &&
          receipt.fallback_visuals_permitted == 0,
          "unsupported decode forbids fallback playback");
    CHECK(strcmp(nexus_sound_sfx_runtime_status_name(receipt.status),
                 "blocked-unsupported-decode") == 0,
          "unsupported decode status name is stable");
    nexus_sound_shutdown(&eng);
}

static void test_mismatched_assets_block_playback(void) {
    Nexus_SoundEngine eng;
    Nexus_SfxRuntimeReceipt receipt;
    static const unsigned char sal_data[12] = {0};
    static const unsigned char map_data[12] = {0};

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_sound_init(&eng) == 0, "sound engine initializes");
    CHECK(nexus_sound_load_level(&eng,
                                 1,
                                 sal_data,
                                 (int)sizeof(sal_data),
                                 map_data,
                                 (int)sizeof(map_data)) == 0,
          "mismatched SAL/MAP load succeeds for receipt");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
          "mismatched sound runtime receipt emits");
    CHECK(receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_ASSET_MISMATCH,
          "wrong-size SAL/MAP blocks runtime SFX");
    CHECK(receipt.sal_receipt.receipt_class ==
              NEXUS_V1_AUDIO_RECEIPT_SIZE_MISMATCH &&
          receipt.map_receipt.receipt_class ==
              NEXUS_V1_AUDIO_RECEIPT_SIZE_MISMATCH,
          "runtime receipt preserves size mismatch classes");
    nexus_sound_shutdown(&eng);
}

int main(void) {
    test_missing_assets_block_playback();
    test_size_matched_assets_block_decode();
    test_mismatched_assets_block_playback();
    if (g_failures) {
        printf("test_nexus_v1_sound_runtime_receipt: %d failure(s)\n",
               g_failures);
        return 1;
    }
    puts("test_nexus_v1_sound_runtime_receipt: PASS");
    return 0;
}
