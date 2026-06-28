/*
 * Nexus V1 SAL/MAP/CD-DA audio receipt gate.
 *
 * Data-free and skip-safe: verifies the receipt/classification surface for
 * SNDLEV00-15.SAL, SNDLEV00-15.MAP, and the documented CD-DA layout
 * (Track 1 data + audio tracks 2-9). It does not parse SAL, decode MAP,
 * inspect real disc sectors, or bind playback.
 */

#include "nexus_v1_audio_receipt.h"
#include "nexus_v1_game.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

#define CHECK(cond, msg) do {                                      \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }          \
    else { printf("  FAIL: %s\n", msg); ++g_fail; }               \
} while (0)

static void check_expected_tables(void) {
    int level;

    printf("\n[Expected SAL/MAP receipt table]\n");
    for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level) {
        Nexus_V1_AudioReceipt sal;
        Nexus_V1_AudioReceipt map;
        int sal_rc = nexus_v1_audio_expected_asset(
            NEXUS_V1_AUDIO_KIND_SAL_BANK, level, &sal);
        int map_rc = nexus_v1_audio_expected_asset(
            NEXUS_V1_AUDIO_KIND_MAP_TABLE, level, &map);
        char msg[96];
        char sal_name[16];
        char map_name[16];

        snprintf(sal_name, sizeof(sal_name), "SNDLEV%02d.SAL", level);
        snprintf(map_name, sizeof(map_name), "SNDLEV%02d.MAP", level);

        snprintf(msg, sizeof(msg), "level %02d SAL expected row exists", level);
        CHECK(sal_rc == NEXUS_V1_AUDIO_OK &&
              sal.kind == NEXUS_V1_AUDIO_KIND_SAL_BANK &&
              sal.level_index == level &&
              sal.expected_size >= 290000u &&
              sal.expected_size <= 470000u &&
              sal.expected_sha256 != NULL &&
              strlen(sal.expected_sha256) == 64u,
              msg);

        snprintf(msg, sizeof(msg), "level %02d MAP expected row exists", level);
        CHECK(map_rc == NEXUS_V1_AUDIO_OK &&
              map.kind == NEXUS_V1_AUDIO_KIND_MAP_TABLE &&
              map.level_index == level &&
              map.expected_size >= 66u &&
              map.expected_size <= 90u &&
              map.expected_sha256 != NULL &&
              strlen(map.expected_sha256) == 64u,
              msg);

        snprintf(msg, sizeof(msg), "level %02d expected names match", level);
        CHECK(strcmp(sal.expected_name, sal_name) == 0 &&
              strcmp(map.expected_name, map_name) == 0,
              msg);
    }
}

static void check_marker_classification(void) {
    Nexus_V1_AudioReceipt expected;
    Nexus_V1_AudioReceipt receipt;
    int rc;

    printf("\n[SAL/MAP marker classification]\n");

    rc = nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_SAL_BANK, 8,
                                       &expected);
    CHECK(rc == NEXUS_V1_AUDIO_OK, "expected SAL level 08 row available");
    rc = nexus_v1_audio_classify_file("nexus/SNDLEV08.SAL",
                                      expected.expected_size,
                                      expected.expected_sha256,
                                      &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          receipt.receipt_class == NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH &&
          receipt.level_index == 8 &&
          receipt.cd_track == 6,
          "SNDLEV08.SAL exact size/hash verifies");

    rc = nexus_v1_audio_classify_file("NEXUS\\sndlev08.sal",
                                      expected.expected_size,
                                      NULL,
                                      &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          receipt.receipt_class == NEXUS_V1_AUDIO_RECEIPT_SIZE_MATCH,
          "case-insensitive SAL basename with no hash returns size-match");

    rc = nexus_v1_audio_classify_file("nexus/SNDLEV08.SAL",
                                      expected.expected_size + 1u,
                                      expected.expected_sha256,
                                      &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          receipt.receipt_class == NEXUS_V1_AUDIO_RECEIPT_SIZE_MISMATCH,
          "SNDLEV08.SAL wrong size is not promoted");

    rc = nexus_v1_audio_classify_file(
        "nexus/SNDLEV08.SAL",
        expected.expected_size,
        "0000000000000000000000000000000000000000000000000000000000000000",
        &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          receipt.receipt_class == NEXUS_V1_AUDIO_RECEIPT_HASH_MISMATCH,
          "SNDLEV08.SAL wrong hash is not promoted");

    rc = nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_MAP_TABLE, 15,
                                       &expected);
    CHECK(rc == NEXUS_V1_AUDIO_OK, "expected MAP level 15 row available");
    rc = nexus_v1_audio_classify_file("nexus/SNDLEV15.MAP",
                                      expected.expected_size,
                                      expected.expected_sha256,
                                      &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          receipt.kind == NEXUS_V1_AUDIO_KIND_MAP_TABLE &&
          receipt.receipt_class == NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH &&
          receipt.level_index == 15 &&
          receipt.cd_track == 9,
          "SNDLEV15.MAP exact size/hash verifies");

    rc = nexus_v1_audio_classify_file("nexus/SNDLEV16.SAL",
                                      1u, NULL, &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_ERR_BAD_LEVEL,
          "SNDLEV16.SAL is rejected outside 0..15");

    rc = nexus_v1_audio_classify_file("nexus/SOUND08.SAL",
                                      1u, NULL, &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_ERR_BAD_NAME,
          "non-SNDLEV audio name is not claimed");

    rc = nexus_v1_audio_classify_file(NULL, 1u, NULL, &receipt);
    CHECK(rc == NEXUS_V1_AUDIO_ERR_NULL,
          "NULL path rejected");
    rc = nexus_v1_audio_classify_file("nexus/SNDLEV00.SAL", 1u, NULL, NULL);
    CHECK(rc == NEXUS_V1_AUDIO_ERR_NULL,
          "NULL output rejected");
}

static void check_all_verified_rows(void) {
    int level;
    int verified = 0;

    printf("\n[All 32 known SAL/MAP rows]\n");
    for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level) {
        Nexus_V1_AudioReceipt expected;
        Nexus_V1_AudioReceipt receipt;
        char path[32];
        int rc;

        rc = nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_SAL_BANK,
                                           level, &expected);
        snprintf(path, sizeof(path), "nexus/%s", expected.expected_name);
        rc = (rc == NEXUS_V1_AUDIO_OK)
            ? nexus_v1_audio_classify_file(path,
                                           expected.expected_size,
                                           expected.expected_sha256,
                                           &receipt)
            : rc;
        if (rc == NEXUS_V1_AUDIO_OK &&
            receipt.receipt_class == NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH) {
            ++verified;
        }

        rc = nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_MAP_TABLE,
                                           level, &expected);
        snprintf(path, sizeof(path), "nexus/%s", expected.expected_name);
        rc = (rc == NEXUS_V1_AUDIO_OK)
            ? nexus_v1_audio_classify_file(path,
                                           expected.expected_size,
                                           expected.expected_sha256,
                                           &receipt)
            : rc;
        if (rc == NEXUS_V1_AUDIO_OK &&
            receipt.receipt_class == NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH) {
            ++verified;
        }
    }

    CHECK(verified == 32, "all 16 SAL + 16 MAP verified rows classify");
}

static void check_cdda_layout(void) {
    Nexus_V1_CddaLayoutReceipt layout;
    int level;
    int rc;

    printf("\n[CD-DA layout receipt]\n");

    rc = nexus_v1_audio_classify_cdda_layout(1, 8, 2, 9, &layout);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          layout.receipt_class == NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_MATCH,
          "1 data track + audio tracks 2-9 is a Nexus CD-DA layout match");

    rc = nexus_v1_audio_classify_cdda_layout(1, 4, 2, 5, &layout);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          layout.receipt_class == NEXUS_V1_AUDIO_RECEIPT_CDDA_LAYOUT_PARTIAL,
          "partial audio range stays partial, not verified");

    rc = nexus_v1_audio_classify_cdda_layout(0, 8, 2, 9, &layout);
    CHECK(rc == NEXUS_V1_AUDIO_OK &&
          layout.receipt_class == NEXUS_V1_AUDIO_RECEIPT_UNKNOWN,
          "missing data track is not a Nexus CD-DA layout match");

    rc = nexus_v1_audio_classify_cdda_layout(1, 8, 9, 2, &layout);
    CHECK(rc == NEXUS_V1_AUDIO_ERR_BOUNDS,
          "inverted CD-DA track range rejected");

    for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level) {
        int receipt_track = nexus_v1_audio_cd_track_for_level_receipt(level);
        int runtime_track = nexus_v1_cd_track_for_level(level);
        char msg[96];
        snprintf(msg, sizeof(msg),
                 "level %02d receipt track matches runtime mapping", level);
        CHECK(receipt_track == runtime_track &&
              receipt_track >= NEXUS_V1_AUDIO_CDDA_TRACK_FIRST &&
              receipt_track <= NEXUS_V1_AUDIO_CDDA_TRACK_LAST,
              msg);
    }

    CHECK(nexus_v1_audio_cd_track_for_level_receipt(-1) == -1,
          "negative level rejected by receipt helper");
    CHECK(nexus_v1_audio_cd_track_for_level_receipt(16) == -1,
          "level 16 rejected by receipt helper");
}

static void check_boundary_names(void) {
    printf("\n[Boundary/status names]\n");
    CHECK(nexus_v1_audio_decode_supported(NEXUS_V1_AUDIO_KIND_SAL_BANK) == 0,
          "SAL decode remains unsupported");
    CHECK(nexus_v1_audio_decode_supported(NEXUS_V1_AUDIO_KIND_MAP_TABLE) == 0,
          "MAP decode remains unsupported");
    CHECK(nexus_v1_audio_decode_supported(NEXUS_V1_AUDIO_KIND_CDDA_LAYOUT) == 0,
          "CD-DA playback binding remains unsupported");
    CHECK(strcmp(nexus_v1_audio_kind_name(NEXUS_V1_AUDIO_KIND_SAL_BANK),
                 "sal-bank") == 0,
          "kind name for SAL is stable");
    CHECK(strcmp(nexus_v1_audio_receipt_class_name(
                     NEXUS_V1_AUDIO_RECEIPT_VERIFIED_HASH),
                 "verified-hash") == 0,
          "receipt class name for verified hash is stable");
    CHECK(strcmp(nexus_v1_audio_status_string(NEXUS_V1_AUDIO_OK),
                 "ok") == 0,
          "status OK string is stable");
    CHECK(strstr(nexus_v1_audio_source_evidence(), "no SAL decode") != NULL,
          "source evidence states non-decode boundary");
}

int main(void) {
    printf("=== Nexus V1 SAL/MAP/CD-DA audio receipt probe ===\n");

    check_expected_tables();
    check_marker_classification();
    check_all_verified_rows();
    check_cdda_layout();
    check_boundary_names();

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
