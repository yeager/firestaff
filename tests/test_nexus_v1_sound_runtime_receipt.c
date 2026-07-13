#include "nexus_v1_sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

#define NEXUS_SAL_EXB_SHARED_PREFIX_BYTES 0x45bb5u
#define NEXUS_DM_BIN_IMAGE_BASE 0x06010000u
#define NEXUS_DM_BIN_SNDLEV_POINTER_TABLE_OFFSET 0x3bd94u
#define NEXUS_DM_BIN_SNDLEV_FIRST_STRING_ADDRESS 0x06048f34u

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        ++g_failures; \
    } \
} while (0)

static unsigned int read_u32_be(const unsigned char *p) {
    return ((unsigned int)p[0] << 24) |
           ((unsigned int)p[1] << 16) |
           ((unsigned int)p[2] << 8) |
           (unsigned int)p[3];
}

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
    memset(map_data, 0, sizeof(map_data));
    map_data[NEXUS_SFX_DOOR_OPEN] = 7;
    map_data[NEXUS_SFX_MENU_CONFIRM] = 11;
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
          "size-matched SAL blocks on unsupported sample decode");
    CHECK(receipt.sal_receipt.receipt_class ==
              NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH &&
          receipt.map_receipt.receipt_class ==
              NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH,
          "runtime receipt preserves SAL/MAP size-match classes");
    CHECK(receipt.sound_driver_receipt.kind ==
              NEXUS_V1_AUDIO_KIND_SOUND_DRIVER &&
          receipt.sound_driver_receipt.level_index == -1 &&
          receipt.sound_driver_receipt.expected_size == 26610u &&
          strcmp(receipt.sound_driver_receipt.expected_name, "SDDRVS.TSK") == 0,
          "runtime receipt keeps the source-locked SDDRVS identity separate from SAL/MAP");
    CHECK(receipt.cd_track == 2 &&
          receipt.level_index == 0 &&
          receipt.playback_enabled == 0,
          "runtime receipt exposes level 0 CD track without enabling playback");
    CHECK(receipt.sal_decode_supported == 0 &&
          receipt.sal_canonical_source_verified == 0 &&
          receipt.map_canonical_source_verified == 0 &&
          receipt.map_decode_supported == 1 &&
          receipt.map_event_count == 0 &&
          receipt.map_mapped_event_count == 0 &&
          receipt.map_first_sample_index == -1 &&
          receipt.map_last_sample_index == -1,
          "runtime receipt refuses raw MAP byte-to-event promotion");
    CHECK(receipt.sal_package_profile_supported == 0 &&
          receipt.sal_word_count == 0 &&
          receipt.sal_nonzero_byte_count == 0,
          "zero SAL package is profiled but not promoted");
    CHECK(receipt.blocks_real_sfx_playback == 1 &&
          receipt.fallback_visuals_permitted == 0,
          "unsupported decode forbids fallback playback");
    nexus_sound_play(&eng, NEXUS_SFX_MENU_CONFIRM);
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
          receipt.last_event == NEXUS_SFX_MENU_CONFIRM &&
          receipt.last_sample_index == -1 &&
          receipt.last_event_record_found == 0,
          "blocked playback does not promote a raw MAP event route");
    CHECK(strcmp(nexus_sound_sfx_runtime_status_name(receipt.status),
                 "blocked-unsupported-decode") == 0,
          "unsupported decode status name is stable");
    nexus_sound_shutdown(&eng);
}

static void test_canonical_source_handoff_stays_decode_blocked(void) {
    Nexus_SoundEngine eng;
    Nexus_SfxRuntimeReceipt receipt;
    static unsigned char sal_data[297082];
    static unsigned char map_data[66];

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    sal_data[0] = 1;
    CHECK(nexus_sound_init(&eng) == 0, "canonical handoff engine initializes");
    CHECK(nexus_sound_load_canonical_level(&eng, 0, sal_data,
                                           (int)sizeof(sal_data), map_data,
                                           (int)sizeof(map_data), 1, 1) == 0,
          "canonical source handoff loads");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
          receipt.sal_canonical_source_verified == 1 &&
          receipt.map_canonical_source_verified == 1 &&
          receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE &&
          receipt.blocks_real_sfx_playback == 1 &&
          receipt.playback_enabled == 0,
          "verified source does not promote unproven SAL playback");
    CHECK(receipt.sound_driver_canonical_source_verified == 0,
          "canonical SAL/MAP do not fabricate a Saturn sound-driver source");
    nexus_sound_set_driver_canonical_source_verified(&eng, 1);
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
          receipt.sound_driver_canonical_source_verified == 1 &&
          receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE &&
          receipt.blocks_real_sfx_playback == 1,
          "verified driver identity still cannot promote an unproven decoder");
    nexus_sound_shutdown(&eng);
}

static void test_sal_package_profile_blocks_playback(void) {
    Nexus_SoundEngine eng;
    Nexus_SfxRuntimeReceipt receipt;
    static unsigned char sal_data[297082];
    static unsigned char map_data[66];

    memset(sal_data, 0, sizeof(sal_data));
    memset(map_data, 0, sizeof(map_data));
    sal_data[0] = 0x12;
    sal_data[1] = 0x34;
    sal_data[10] = 0x80;
    sal_data[11] = 0x01;
    sal_data[sizeof(sal_data) - 2] = 0x56;
    sal_data[sizeof(sal_data) - 1] = 0x78;
    map_data[NEXUS_SFX_FOOTSTEP] = 3;

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_sound_init(&eng) == 0, "sound engine initializes");
    CHECK(nexus_sound_load_level(&eng,
                                 0,
                                 sal_data,
                                 (int)sizeof(sal_data),
                                 map_data,
                                 (int)sizeof(map_data)) == 0,
          "profiled SAL/MAP load succeeds");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
          "profiled SAL runtime receipt emits");
    CHECK(receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE &&
          receipt.blocks_real_sfx_playback == 1 &&
          receipt.playback_enabled == 0,
          "profiled SAL still blocks unsafe playback");
    CHECK(receipt.sal_package_profile_supported == 1 &&
          receipt.sal_word_count == (int)(sizeof(sal_data) / 2) &&
          receipt.sal_nonzero_byte_count == 6 &&
          receipt.sal_high_bit_byte_count == 1 &&
          receipt.sal_zero_run_count == 2 &&
          receipt.sal_max_zero_run == (int)sizeof(sal_data) - 14 &&
          receipt.sal_first_nonzero_offset == 0 &&
          receipt.sal_last_nonzero_offset == (int)sizeof(sal_data) - 1 &&
          receipt.sal_checksum16 == ((0x1234 + 0x8001 + 0x5678) & 0xffff),
          "SAL package receipt records bounded byte/word metadata");
    nexus_sound_shutdown(&eng);
}

static void test_sal_container_preamble_stays_opaque(void) {
    Nexus_SoundEngine eng;
    Nexus_SfxRuntimeReceipt receipt;
    static unsigned char sal_data[297082];
    static unsigned char map_data[66];

    memset(sal_data, 0, sizeof(sal_data));
    memset(map_data, 0, sizeof(map_data));
    memcpy(sal_data, "dsp01.EXB", 8);

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_sound_init(&eng) == 0, "sound engine initializes");
    CHECK(nexus_sound_load_level(&eng, 0, sal_data, (int)sizeof(sal_data),
                                 map_data, (int)sizeof(map_data)) == 0,
          "known SAL preamble loads for bounded inspection");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
          "known SAL preamble receipt emits");
    CHECK(receipt.sal_container_preamble_supported == 1 &&
          receipt.sal_payload_offset == 8 &&
          receipt.sal_opaque_payload_size == (int)sizeof(sal_data) - 8 &&
          receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE &&
          receipt.blocks_real_sfx_playback == 1,
          "known preamble exposes only an opaque payload boundary");
    nexus_sound_shutdown(&eng);

    memset(sal_data, 0, sizeof(sal_data));
    memcpy(sal_data, "dsp02.EXB", 8);
    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_sound_init(&eng) == 0, "sound engine reinitializes");
    CHECK(nexus_sound_load_level(&eng, 0, sal_data, (int)sizeof(sal_data),
                                 map_data, (int)sizeof(map_data)) == 0,
          "unknown SAL preamble loads only for rejection");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
          "unknown SAL preamble receipt emits");
    CHECK(receipt.sal_container_preamble_supported == 0,
          "unknown preamble is not accepted");
    CHECK(receipt.sal_payload_offset == -1 &&
          receipt.sal_opaque_payload_size == 0,
          "unknown preamble has no payload boundary");
    CHECK(receipt.blocks_real_sfx_playback == 1 &&
          receipt.playback_enabled == 0,
          "unknown preamble cannot enable playback");
    nexus_sound_shutdown(&eng);
}

static void test_sfx_map_record_table_receipt(void) {
    Nexus_SoundEngine eng;
    Nexus_SfxRuntimeReceipt receipt;
    static unsigned char sal_data[297082];
    static unsigned char map_data[66];

    memset(sal_data, 0, sizeof(sal_data));
    memset(map_data, 0, sizeof(map_data));
    sal_data[0x1000] = 0x10;
    sal_data[0x1001] = 0x20;
    sal_data[0x101f] = 0x80;
    sal_data[0x1400] = 0x30;
    sal_data[0x1401] = 0x40;
    sal_data[0x142f] = 0x90;
    map_data[0] = 0x53;
    map_data[1] = 0x46;
    map_data[23] = 0x80;
    map_data[24] = 0x10;
    map_data[25] = 0x04;
    map_data[26] = 0x00;
    map_data[27] = 0x20;
    map_data[28] = 0x00;
    map_data[29] = 0x00;
    map_data[30] = 0x10;
    map_data[31] = 0x00;
    map_data[32] = 0x11;
    map_data[33] = 0x05;
    map_data[34] = 0x00;
    map_data[35] = 0x30;
    map_data[36] = 0x00;
    map_data[37] = 0x00;
    map_data[38] = 0x14;
    map_data[39] = 0x00;
    map_data[40] = 0xff;
    map_data[41] = 0xff;

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_sound_init(&eng) == 0, "sound engine initializes");
    CHECK(nexus_sound_load_level(&eng,
                                 0,
                                 sal_data,
                                 (int)sizeof(sal_data),
                                 map_data,
                                 (int)sizeof(map_data)) == 0,
          "SFX MAP record fixture loads");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
          "SFX MAP record receipt emits");
    CHECK(receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE &&
          receipt.blocks_real_sfx_playback == 1,
          "SFX MAP record receipt still blocks playback");
    CHECK(receipt.map_header_checksum16 == (0x53 + 0x46 + 0x80) &&
          receipt.map_header_nonzero_byte_count == 3 &&
          receipt.map_header_distinct_byte_count == 4 &&
          receipt.map_header_transition_count == 3,
          "SFX MAP header receipt exposes bounded byte diagnostics");
    CHECK(receipt.map_record_table_supported == 1 &&
          receipt.map_record_count == 2 &&
          receipt.map_record_terminator_offset == 40 &&
          receipt.map_first_record_event == 0x10 &&
          receipt.map_min_record_event == 0x10 &&
          receipt.map_max_record_event == 0x11 &&
          receipt.map_record_event_span == 2 &&
          receipt.map_unique_record_event_count == 2 &&
          receipt.map_duplicate_record_event_count == 0 &&
          receipt.map_has_duplicate_record_events == 0 &&
          receipt.map_first_record_sal_offset == 0x1000 &&
          receipt.map_first_record_size == 0x20 &&
          receipt.map_last_record_sal_offset == 0x1400 &&
          receipt.map_max_record_end == 0x1430 &&
          receipt.map_total_record_bytes == 0x50 &&
          receipt.map_out_of_bounds_record_count == 0,
          "SFX MAP record table exposes bounded SAL windows");
    CHECK(receipt.map_first_window_checksum16 == 0xb0 &&
          receipt.map_first_window_nonzero_byte_count == 3 &&
          receipt.map_first_window_high_bit_byte_count == 1 &&
          receipt.map_last_window_checksum16 == 0x100 &&
          receipt.map_last_window_nonzero_byte_count == 3 &&
          receipt.map_last_window_high_bit_byte_count == 1,
          "SFX MAP record receipt profiles first and last SAL windows");
    CHECK(receipt.map_first_window_first_nonzero_relative_offset == 0 &&
          receipt.map_first_window_last_nonzero_relative_offset == 0x1f &&
          receipt.map_first_window_distinct_byte_count == 4 &&
          receipt.map_first_window_transition_count == 3 &&
          receipt.map_last_window_first_nonzero_relative_offset == 0 &&
          receipt.map_last_window_last_nonzero_relative_offset == 0x2f &&
          receipt.map_last_window_distinct_byte_count == 4 &&
          receipt.map_last_window_transition_count == 3,
          "SFX MAP record receipt exposes bounded payload diagnostics");
    nexus_sound_play(&eng, NEXUS_SFX_PIT_FALL);
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
          receipt.last_event == NEXUS_SFX_PIT_FALL &&
          receipt.last_sample_index == -1 &&
          receipt.last_event_record_found == 0 &&
          receipt.last_event_sal_offset == -1 &&
          receipt.last_event_sal_size == 0,
          "blocked SFX event does not infer a MAP record identity");
    nexus_sound_play(&eng, NEXUS_SFX_MENU_CANCEL);
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
          receipt.last_event == NEXUS_SFX_MENU_CANCEL &&
          receipt.last_event_record_found == 0 &&
          receipt.last_event_sal_offset == -1 &&
          receipt.last_event_sal_size == 0,
          "unmapped blocked SFX event keeps SAL window receipt empty");
    nexus_sound_shutdown(&eng);
}

static void test_sfx_map_duplicate_event_receipt(void) {
    Nexus_SoundEngine eng;
    Nexus_SfxRuntimeReceipt receipt;
    static unsigned char sal_data[297082];
    static unsigned char map_data[66];

    memset(sal_data, 0, sizeof(sal_data));
    memset(map_data, 0, sizeof(map_data));
    sal_data[0x1000] = 0x11;
    sal_data[0x1020] = 0x22;
    map_data[24] = 0x10;
    map_data[26] = 0x00;
    map_data[27] = 0x10;
    map_data[30] = 0x10;
    map_data[31] = 0x00;
    map_data[32] = 0x10;
    map_data[34] = 0x00;
    map_data[35] = 0x10;
    map_data[38] = 0x10;
    map_data[39] = 0x20;
    map_data[40] = 0x13;
    map_data[42] = 0x00;
    map_data[43] = 0x08;
    map_data[46] = 0x10;
    map_data[47] = 0x40;
    map_data[48] = 0xff;
    map_data[49] = 0xff;

    memset(&eng, 0, sizeof(eng));
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_sound_init(&eng) == 0, "sound engine initializes");
    CHECK(nexus_sound_load_level(&eng,
                                 0,
                                 sal_data,
                                 (int)sizeof(sal_data),
                                 map_data,
                                 (int)sizeof(map_data)) == 0,
          "duplicate-event SFX MAP fixture loads");
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
          receipt.map_record_table_supported == 1 &&
          receipt.map_record_count == 3 &&
          receipt.map_min_record_event == 0x10 &&
          receipt.map_max_record_event == 0x13 &&
          receipt.map_record_event_span == 4 &&
          receipt.map_unique_record_event_count == 2 &&
          receipt.map_duplicate_record_event_count == 1 &&
          receipt.map_has_duplicate_record_events == 1,
          "SFX MAP record receipt exposes event range and duplicates");
    nexus_sound_play(&eng, NEXUS_SFX_PIT_FALL);
    CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
          receipt.last_event_record_found == 0 &&
          receipt.last_event_sal_offset == -1 &&
          receipt.last_event_sal_size == 0,
          "duplicate MAP records never select a host SFX route");
    nexus_sound_shutdown(&eng);
}

static void test_optional_real_sal_corpus_profile(void) {
    const char *home = getenv("HOME");
    int seen = 0;
    int profiled = 0;
    int level;

    if (!home || home[0] == '\0') return;
    for (level = 0; level < 16; ++level) {
        char sal_path[512];
        char map_path[512];
        FILE *sal_fp;
        FILE *map_fp;
        long sal_size;
        long map_size;
        unsigned char *sal_data;
        unsigned char *map_data;
        Nexus_SoundEngine eng;
        Nexus_SfxRuntimeReceipt receipt;

        snprintf(sal_path, sizeof(sal_path),
                 "%s/.firestaff/data/nexus/SNDLEV%02d.SAL", home, level);
        snprintf(map_path, sizeof(map_path),
                 "%s/.firestaff/data/nexus/SNDLEV%02d.MAP", home, level);
        sal_fp = fopen(sal_path, "rb");
        map_fp = fopen(map_path, "rb");
        if (!sal_fp || !map_fp) {
            if (sal_fp) fclose(sal_fp);
            if (map_fp) fclose(map_fp);
            continue;
        }
        if (fseek(sal_fp, 0, SEEK_END) != 0 ||
            fseek(map_fp, 0, SEEK_END) != 0) {
            fclose(sal_fp);
            fclose(map_fp);
            continue;
        }
        sal_size = ftell(sal_fp);
        map_size = ftell(map_fp);
        if (sal_size <= 0 || sal_size > 1000000L ||
            map_size <= 0 || map_size > 1024L ||
            fseek(sal_fp, 0, SEEK_SET) != 0 ||
            fseek(map_fp, 0, SEEK_SET) != 0) {
            fclose(sal_fp);
            fclose(map_fp);
            continue;
        }
        sal_data = (unsigned char *)malloc((size_t)sal_size);
        map_data = (unsigned char *)malloc((size_t)map_size);
        if (!sal_data || !map_data) {
            free(sal_data);
            free(map_data);
            fclose(sal_fp);
            fclose(map_fp);
            continue;
        }
        if (fread(sal_data, 1, (size_t)sal_size, sal_fp) != (size_t)sal_size ||
            fread(map_data, 1, (size_t)map_size, map_fp) != (size_t)map_size) {
            free(sal_data);
            free(map_data);
            fclose(sal_fp);
            fclose(map_fp);
            continue;
        }
        fclose(sal_fp);
        fclose(map_fp);

        seen++;
        memset(&eng, 0, sizeof(eng));
        memset(&receipt, 0, sizeof(receipt));
        CHECK(nexus_sound_init(&eng) == 0, "real SAL sound engine initializes");
        CHECK(nexus_sound_load_level(&eng,
                                     level,
                                     sal_data,
                                     (int)sal_size,
                                     map_data,
                                     (int)map_size) == 0,
              "real SAL/MAP corpus load succeeds");
        CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0,
              "real SAL/MAP corpus receipt emits");
        if (receipt.sal_package_profile_supported) {
            profiled++;
            CHECK(receipt.status == NEXUS_SFX_RUNTIME_BLOCKED_UNSUPPORTED_DECODE &&
                  receipt.blocks_real_sfx_playback == 1 &&
                  receipt.sal_container_preamble_supported == 1 &&
                  receipt.sal_payload_offset == 8 &&
                  receipt.sal_opaque_payload_size == sal_size - 8 &&
                  receipt.sal_word_count == (int)(sal_size / 2) &&
                  receipt.sal_nonzero_byte_count > 0 &&
                  receipt.sal_last_nonzero_offset >=
                      receipt.sal_first_nonzero_offset &&
                  receipt.map_event_count == 0 &&
                  receipt.map_mapped_event_count == 0 &&
                  receipt.map_first_sample_index == -1 &&
                  receipt.map_last_sample_index == -1 &&
                  receipt.map_record_table_supported == 1 &&
                  receipt.map_record_count > 0 &&
                  receipt.map_record_terminator_offset > 0 &&
                  receipt.map_min_record_event >= 0 &&
                  receipt.map_max_record_event >=
                      receipt.map_min_record_event &&
                  receipt.map_record_event_span ==
                      receipt.map_max_record_event -
                          receipt.map_min_record_event + 1 &&
                  receipt.map_unique_record_event_count > 0 &&
                  receipt.map_duplicate_record_event_count >= 0 &&
                  receipt.map_has_duplicate_record_events ==
                      (receipt.map_duplicate_record_event_count > 0 ? 1 : 0) &&
                  receipt.map_out_of_bounds_record_count == 0 &&
                  receipt.map_max_record_end <= sal_size &&
                  receipt.map_total_record_bytes > 0 &&
                  receipt.map_header_distinct_byte_count > 0 &&
                  receipt.map_first_window_checksum16 >= 0 &&
                  receipt.map_last_window_checksum16 >= 0,
                  "real SAL/SFX record receipt consumes package without playback");
            nexus_sound_play(&eng, NEXUS_SFX_PIT_FALL);
            CHECK(nexus_sound_level_runtime_receipt(&eng, &receipt) == 0 &&
                  receipt.last_event_record_found == 0 &&
                  receipt.last_event_sal_offset == -1 &&
                  receipt.last_event_sal_size == 0,
                  "real MAP records stay unbound from host SFX requests");
        }
        nexus_sound_shutdown(&eng);
        free(sal_data);
        free(map_data);
    }
    if (seen > 0) {
        CHECK(profiled == seen, "all staged real SAL files profile");
    }
}

static void test_optional_real_sal_layout_provenance(void) {
    const char *home = getenv("HOME");
    unsigned char *base = NULL;
    size_t base_size = 0;
    size_t shared_prefix = 0;
    long file_size;
    int level;
    int banks = 0;
    char base_path[512];
    char dm_path[512];
    FILE *fp;

    if (!home || home[0] == '\0') return;
    snprintf(base_path, sizeof(base_path),
             "%s/.firestaff/data/nexus/SNDLEV00.SAL", home);
    fp = fopen(base_path, "rb");
    if (!fp || fseek(fp, 0, SEEK_END) != 0 ||
        (file_size = ftell(fp)) <= 0 || fseek(fp, 0, SEEK_SET) != 0) {
        if (fp) fclose(fp);
        return;
    }
    base_size = (size_t)file_size;
    base = (unsigned char *)malloc(base_size);
    if (!base || fread(base, 1, base_size, fp) != base_size) {
        free(base);
        fclose(fp);
        return;
    }
    fclose(fp);
    shared_prefix = base_size;

    for (level = 0; level < 16; ++level) {
        char sal_path[512];
        unsigned char *data;
        size_t size;
        size_t limit;
        size_t offset;

        snprintf(sal_path, sizeof(sal_path),
                 "%s/.firestaff/data/nexus/SNDLEV%02d.SAL", home, level);
        fp = fopen(sal_path, "rb");
        if (!fp || fseek(fp, 0, SEEK_END) != 0 ||
            (file_size = ftell(fp)) <= 0 || fseek(fp, 0, SEEK_SET) != 0) {
            if (fp) fclose(fp);
            free(base);
            return;
        }
        size = (size_t)file_size;
        data = (unsigned char *)malloc(size);
        if (!data || fread(data, 1, size, fp) != size) {
            free(data);
            fclose(fp);
            free(base);
            return;
        }
        fclose(fp);
        limit = size < base_size ? size : base_size;
        for (offset = 0; offset < limit && data[offset] == base[offset]; ++offset) {
        }
        if (offset < shared_prefix) shared_prefix = offset;
        ++banks;
        free(data);
    }

    CHECK(banks == 16 && shared_prefix == NEXUS_SAL_EXB_SHARED_PREFIX_BYTES,
          "all real SAL banks share the bounded EXB prefix before level variation");
    free(base);

    /* DM.BIN's original SNDLEV pointer table starts at file offset 0x3bd94.
     * Each big-endian pointer names SNDLEV01 through SNDLEV15 in the image's
     * 0x06010000 address space. This proves the level-bank lookup provenance,
     * not any sample frame or codec field. */
    snprintf(dm_path, sizeof(dm_path), "%s/.firestaff/data/nexus/DM.BIN", home);
    fp = fopen(dm_path, "rb");
    if (!fp || fseek(fp, NEXUS_DM_BIN_SNDLEV_POINTER_TABLE_OFFSET, SEEK_SET) != 0) {
        if (fp) fclose(fp);
        return;
    }
    for (level = 1; level <= 15; ++level) {
        unsigned char pointer_bytes[4];
        unsigned int address;
        unsigned int file_offset;
        char expected[16];
        char actual[10];

        if (fread(pointer_bytes, 1, sizeof(pointer_bytes), fp) !=
            sizeof(pointer_bytes)) {
            fclose(fp);
            return;
        }
        address = read_u32_be(pointer_bytes);
        file_offset = address - NEXUS_DM_BIN_IMAGE_BASE;
        snprintf(expected, sizeof(expected), "SNDLEV%02d", level);
        CHECK(address == NEXUS_DM_BIN_SNDLEV_FIRST_STRING_ADDRESS +
                         (unsigned int)(level - 1) * 12u &&
              fseek(fp, (long)file_offset, SEEK_SET) == 0 &&
              fread(actual, 1, 9, fp) == 9 &&
              memcmp(actual, expected, 9) == 0,
              "DM.BIN SNDLEV table resolves the original level-bank name");
        if (fseek(fp, NEXUS_DM_BIN_SNDLEV_POINTER_TABLE_OFFSET +
                      (long)level * 4L, SEEK_SET) != 0) {
            fclose(fp);
            return;
        }
    }
    fclose(fp);
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
    test_sal_package_profile_blocks_playback();
    test_sal_container_preamble_stays_opaque();
    test_sfx_map_record_table_receipt();
    test_sfx_map_duplicate_event_receipt();
    test_mismatched_assets_block_playback();
    test_canonical_source_handoff_stays_decode_blocked();
    test_optional_real_sal_corpus_profile();
    test_optional_real_sal_layout_provenance();
    if (g_failures) {
        printf("test_nexus_v1_sound_runtime_receipt: %d failure(s)\n",
               g_failures);
        return 1;
    }
    puts("test_nexus_v1_sound_runtime_receipt: PASS");
    return 0;
}
