#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "theron_v1_track02_raw_media_intake.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static const char *find_standard_us_bin(void) {
    const char *home = getenv("HOME");
    static char path[512];
    FILE *file;

    if (!home || !home[0] || snprintf(path, sizeof(path),
            "%s/.firestaff/data/theron/TQUS02.bin", home) >=
            (int)sizeof(path)) return NULL;
    file = fopen(path, "rb");
    if (!file) return NULL;
    fclose(file);
    return path;
}

static const char *find_real_us_cue(void) {
    const char *override = getenv("FIRESTAFF_THERON_CUE");
    const char *home = getenv("HOME");
    static char path[512];
    FILE *file;

    if (override && override[0]) {
        file = fopen(override, "rb");
        if (file) {
            fclose(file);
            return override;
        }
    }
    if (!home || !home[0] || snprintf(path, sizeof(path),
            "%s/.firestaff/data/theron/TQUS.cue", home) >=
            (int)sizeof(path)) return NULL;
    file = fopen(path, "rb");
    if (!file) return NULL;
    fclose(file);
    return path;
}

static const char *find_real_jp_cue(void) {
    const char *override = getenv("FIRESTAFF_THERON_JP_CUE");
    const char *home = getenv("HOME");
    static char path[512];
    FILE *file;

    if (override && override[0]) {
        file = fopen(override, "rb");
        if (file) {
            fclose(file);
            return override;
        }
    }
    if (!home || !home[0] || snprintf(path, sizeof(path),
            "%s/.firestaff/data/theron/TQJP.cue", home) >=
            (int)sizeof(path)) return NULL;
    file = fopen(path, "rb");
    if (!file) return NULL;
    fclose(file);
    return path;
}

static void test_real_split_us_cue_path(void) {
#if defined(_WIN32)
    (void)find_real_us_cue;
#else
    const char *cue = find_real_us_cue();
    Theron_V1Track02RawMediaIntakeReceipt receipt;

    if (!cue) {
        printf("test_theron_v1_track02_raw_media_intake: SKIP split US CUE\n");
        return;
    }
    CHECK(theron_v1_track02_raw_media_intake_discover(cue, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY);
    CHECK(receipt.variant == THERON_TRACK02_VARIANT_US_ISO);
    CHECK(receipt.cue_consumed && receipt.mode1_2048 && !receipt.mode1_2352);
    CHECK(receipt.payload_bytes == 6596608u);
    CHECK(receipt.sector_count == 3221u);
    CHECK(!strcmp(receipt.track02_md5, THERON_TRACK02_MD5_US_ISO));
    CHECK(strstr(receipt.payload_path, "TQUS02-") != NULL);
#endif
}

static void test_real_split_jp_cue_path(void) {
#if defined(_WIN32)
    (void)find_real_jp_cue;
#else
    const char *cue = find_real_jp_cue();
    Theron_V1Track02RawMediaIntakeReceipt receipt;

    if (!cue) {
        printf("test_theron_v1_track02_raw_media_intake: SKIP split JP CUE\n");
        return;
    }
    CHECK(theron_v1_track02_raw_media_intake_discover(cue, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY);
    CHECK(receipt.variant == THERON_TRACK02_VARIANT_JP_REV1_ISO);
    CHECK(receipt.cue_consumed && receipt.mode1_2048 && !receipt.mode1_2352);
    CHECK(receipt.payload_bytes == 305152u);
    CHECK(receipt.sector_count == 149u);
    CHECK(!strcmp(receipt.track02_md5, THERON_TRACK02_MD5_JP_REV1_ISO));
    CHECK(strstr(receipt.payload_path, "TQJP02End.iso") != NULL);
#endif
}

static void test_real_us_cue_path(void) {
#if defined(_WIN32)
    (void)find_standard_us_bin;
#else
    const char *bin = find_standard_us_bin();
    char cue_path[256];
    FILE *cue;
    Theron_V1Track02RawMediaIntakeReceipt receipt;

    if (!bin) {
        printf("test_theron_v1_track02_raw_media_intake: SKIP real US CUE\n");
        return;
    }
    snprintf(cue_path, sizeof(cue_path),
             "/tmp/firestaff_theron_real_us_%ld.cue", (long)getpid());
    cue = fopen(cue_path, "wb");
    CHECK(cue != NULL);
    if (!cue) return;
    fprintf(cue, "file \"%s\" binary\n"
                 "  track 02 mode1/2352\n"
                 "    pregap 00:03:00\n"
                 "    index 01 00:00:00\n", bin);
    CHECK(fclose(cue) == 0);

    CHECK(theron_v1_track02_raw_media_intake_discover(cue_path, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY);
    CHECK(receipt.cue_consumed && receipt.mode1_2352);
    CHECK(receipt.variant == THERON_TRACK02_VARIANT_US_BIN);
    CHECK(receipt.cue_index01_sector == 225u);
    CHECK(receipt.raw_trace_preparation_allowed);
    CHECK(!strcmp(receipt.track02_md5, THERON_TRACK02_MD5_US_BIN));
    remove(cue_path);
#endif
}

int main(void) {
    Theron_V1Track02RawMediaIntakeReceipt receipt;
    Theron_V1Track02RawTraceMediaInput trace_input;
    Theron_Track02Variant variant;
    const char *media = getenv("FIRESTAFF_THERON_TRACK02_MEDIA");
    const char *wrong_cue = "/tmp/firestaff-theron-track02-wrong-layout.cue";
    const char *trailing_cue = "/tmp/firestaff-theron-track02-trailing-token.cue";
    const char *missing_payload_cue = "/tmp/firestaff-theron-track02-missing-payload.cue";
    const char *unknown_iso = "/tmp/firestaff-theron-track02-unknown.iso";
    const char *unknown_bin = "/tmp/firestaff-theron-track02-unknown.bin";
    const char *declared_iso_alias = "/tmp/TQUS02.iso";
    const char *home = getenv("HOME");
    char canonical_iso[512];
    FILE *file;

    CHECK(theron_v1_track02_raw_media_intake_discover(NULL, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE);
    CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE);
    CHECK(!strcmp(theron_v1_track02_media_failure_reason_id(
                      receipt.failure_reason), "path_unavailable"));

    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              THERON_TRACK02_MD5_US_ISO, 2048, 0, 0u, 0u, 2048u,
              &variant) == THERON_V1_TRACK02_MEDIA_REASON_NONE);
    CHECK(variant == THERON_TRACK02_VARIANT_US_ISO);
    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              THERON_TRACK02_MD5_US_BIN, 2352, 0, 0u, 0u, 2352u,
              &variant) == THERON_V1_TRACK02_MEDIA_REASON_NONE);
    CHECK(variant == THERON_TRACK02_VARIANT_US_BIN);
    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              THERON_TRACK02_MD5_US_BIN, 2352, 1, 225u, 225u,
              226u * 2352u, &variant) == THERON_V1_TRACK02_MEDIA_REASON_NONE);
    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              THERON_TRACK02_MD5_US_ISO, 2048, 1, 150u, 0u, 2048u,
              &variant) == THERON_V1_TRACK02_MEDIA_REASON_NONE);
    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              "00000000000000000000000000000000", 2048, 0, 0u, 0u, 2048u,
              &variant) == THERON_V1_TRACK02_MEDIA_REASON_TRACK02_HASH_UNKNOWN);
    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              THERON_TRACK02_MD5_US_ISO, 2352, 0, 0u, 0u, 2352u,
              &variant) == THERON_V1_TRACK02_MEDIA_REASON_LAYOUT_HASH_MISMATCH);
    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              THERON_TRACK02_MD5_US_BIN, 2352, 1, 224u, 224u,
              226u * 2352u, &variant) == THERON_V1_TRACK02_MEDIA_REASON_CUE_INDEX_INVALID);
    CHECK(theron_v1_track02_raw_media_intake_validate_verified_layout(
              THERON_TRACK02_MD5_US_ISO, 2048, 0, 0u, 0u, 2049u,
              &variant) == THERON_V1_TRACK02_MEDIA_REASON_SECTOR_ALIGNMENT_INVALID);

    file = fopen(wrong_cue, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("FILE \"wrong.bin\" BINARY\nTRACK 01 AUDIO\nINDEX 01 00:00:00\n",
              file);
        fclose(file);
        CHECK(theron_v1_track02_raw_media_intake_discover(wrong_cue, &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED);
        CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_CUE_LAYOUT_INVALID);
        remove(wrong_cue);
    }
    file = fopen(trailing_cue, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("FILE \"track.bin\" BINARY extra\n"
              "TRACK 02 MODE1/2352 trailing\n"
              "INDEX 01 00:00:00\n", file);
        fclose(file);
        CHECK(theron_v1_track02_raw_media_intake_discover(trailing_cue,
                                                           &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED);
        CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_CUE_LAYOUT_INVALID);
        remove(trailing_cue);
    }
    file = fopen(missing_payload_cue, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("FILE missing-track02.bin BINARY\n"
              "  TRACK 02 MODE1/2352\n"
              "    PREGAP 00:03:00\n"
              "    INDEX 01 00:00:00\n", file);
        fclose(file);
        CHECK(theron_v1_track02_raw_media_intake_discover(missing_payload_cue,
                                                           &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE);
        CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_PAYLOAD_UNAVAILABLE);
        remove(missing_payload_cue);
    }
    file = fopen(unknown_iso, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("0", file);
        CHECK(fseek(file, 2047L, SEEK_SET) == 0);
        fputc(0, file);
        fclose(file);
        CHECK(theron_v1_track02_raw_media_intake_discover(unknown_iso, &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED);
        CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_TRACK02_HASH_UNKNOWN);
        remove(unknown_iso);
    }
    file = fopen(unknown_bin, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("0", file);
        CHECK(fseek(file, 2351L, SEEK_SET) == 0);
        fputc(0, file);
        fclose(file);
        CHECK(theron_v1_track02_raw_media_intake_discover(unknown_bin, &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED);
        CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_TRACK02_HASH_UNKNOWN);
        remove(unknown_bin);
    }
    remove(declared_iso_alias);
    file = fopen("/tmp/TQUS02End.iso", "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("0", file);
        CHECK(fseek(file, 2047L, SEEK_SET) == 0);
        fputc(0, file);
        fclose(file);
        CHECK(theron_v1_track02_raw_media_intake_discover(declared_iso_alias,
                                                           &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE);
        CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_PAYLOAD_UNAVAILABLE);
        CHECK(!strcmp(receipt.payload_path, declared_iso_alias));
        remove("/tmp/TQUS02End.iso");
    }
    CHECK(!receipt.raw_trace_preparation_allowed);
    CHECK(theron_v1_track02_raw_media_intake_discover(
        "/tmp/firestaff-no-such-track02.cue", &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE);
    CHECK(receipt.failure_reason == THERON_V1_TRACK02_MEDIA_REASON_PATH_UNAVAILABLE);

    /* The assembled US ISO is the real local Track 02 artifact used by the
     * production cache.  Keep this assertion optional for clean CI hosts,
     * but when the supplied game data is present, prove the direct ISO route
     * end-to-end instead of relying only on hash/layout unit inputs. */
    canonical_iso[0] = '\0';
    if (home && home[0] &&
        snprintf(canonical_iso, sizeof(canonical_iso),
                 "%s/.firestaff/cache/theron/"
                 "TQUS02-ceb02343868f80cec899e9b239aff2da.iso", home) <
            (int)sizeof(canonical_iso)) {
        FILE *canonical = fopen(canonical_iso, "rb");
        if (canonical) {
            fclose(canonical);
            CHECK(theron_v1_track02_raw_media_intake_discover(
                      canonical_iso, &receipt));
            CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY);
            CHECK(receipt.variant == THERON_TRACK02_VARIANT_US_ISO);
            CHECK(receipt.mode1_2048 && !receipt.mode1_2352);
            CHECK(!receipt.cue_consumed);
            CHECK(!receipt.raw_trace_preparation_allowed);
            CHECK(receipt.payload_bytes == 6596608u);
            CHECK(receipt.sector_count == 3221u);
            CHECK(receipt.first_user_data_offset == 0u);
            CHECK(receipt.logical_user_data_window_bytes == 6596608u);
            CHECK(!strcmp(receipt.track02_md5,
                          THERON_TRACK02_MD5_US_ISO));
        } else {
            printf("test_theron_v1_track02_raw_media_intake: SKIP canonical US ISO\n");
        }
    }

    if (media && media[0]) {
        CHECK(theron_v1_track02_raw_media_intake_discover(media, &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_READY);
        CHECK(receipt.variant == THERON_TRACK02_VARIANT_US_BIN ||
              receipt.variant == THERON_TRACK02_VARIANT_JP_BIN ||
              receipt.variant == THERON_TRACK02_VARIANT_US_ISO ||
              receipt.variant == THERON_TRACK02_VARIANT_JP_REV1_ISO);
        CHECK(receipt.sector_count > 0u);
        CHECK(receipt.logical_user_data_window_bytes >= 2048u);
        if (receipt.raw_trace_preparation_allowed) {
            CHECK(receipt.mode1_2352 && receipt.cue_consumed);
            CHECK(theron_v1_track02_raw_media_intake_prepare_trace_input(
                &receipt, &trace_input));
            CHECK(trace_input.valid);
        } else {
            CHECK(!theron_v1_track02_raw_media_intake_prepare_trace_input(
                &receipt, &trace_input));
        }
    } else {
        printf("test_theron_v1_track02_raw_media_intake: SKIP (no local Track 02 media)\n");
    }
    test_real_us_cue_path();
    test_real_split_us_cue_path();
    test_real_split_jp_cue_path();
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
