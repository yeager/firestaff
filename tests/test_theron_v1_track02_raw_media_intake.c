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
    const char *media = getenv("FIRESTAFF_THERON_TRACK02_MEDIA");
    const char *wrong_cue = "/tmp/firestaff-theron-track02-wrong-layout.cue";
    FILE *file;

    CHECK(theron_v1_track02_raw_media_intake_discover(NULL, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE);

    file = fopen(wrong_cue, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("FILE \"wrong.bin\" BINARY\nTRACK 01 AUDIO\nINDEX 01 00:00:00\n",
              file);
        fclose(file);
        CHECK(theron_v1_track02_raw_media_intake_discover(wrong_cue, &receipt));
        CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_REJECTED);
        remove(wrong_cue);
    }
    CHECK(!receipt.raw_trace_preparation_allowed);
    CHECK(theron_v1_track02_raw_media_intake_discover(
        "/tmp/firestaff-no-such-track02.cue", &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE);

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
