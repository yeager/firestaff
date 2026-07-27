#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02_raw_media_intake.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

int main(void) {
    Theron_V1Track02RawMediaIntakeReceipt receipt;
    Theron_V1Track02RawTraceMediaInput trace_input;
    Theron_Track02Variant variant;
    const char *media = getenv("FIRESTAFF_THERON_TRACK02_MEDIA");
    const char *wrong_cue = "/tmp/firestaff-theron-track02-wrong-layout.cue";
    const char *missing_payload_cue = "/tmp/firestaff-theron-track02-missing-payload.cue";
    const char *unknown_iso = "/tmp/firestaff-theron-track02-unknown.iso";
    const char *unknown_bin = "/tmp/firestaff-theron-track02-unknown.bin";
    const char *declared_iso_alias = "/tmp/TQUS02.iso";
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
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
