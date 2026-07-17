#include <stdio.h>
#include <stdlib.h>

#include "theron_v1_track02_external_capture_launcher.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); ++failures; \
} } while (0)

int main(void) {
    Theron_V1Track02ExternalCaptureRequest request = {0};
    Theron_V1Track02ExternalCaptureReceipt receipt;
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt admission;
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt provenance = {0};
    Theron_V1Track02LevelObjectTracePreparationReceipt preparation = {0};
    Theron_V1Track02MednafenTraceConvertRequest trace_request = {0};

    CHECK(theron_v1_track02_external_capture_write_skeleton(NULL, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED);
    CHECK(!receipt.external_launch_performed && !receipt.media_copied);
    CHECK(!receipt.synthetic_trace_created && !receipt.decoder_invoked);
    request.emulator_path = "/tmp/firestaff-no-such-emulator";
    request.media_path = "/tmp/firestaff-no-such-track02.cue";
    request.expected_track02_md5 = THERON_TRACK02_MD5_US_BIN;
    request.manifest_path = "/tmp/firestaff-theron-capture-skeleton.manifest";
    CHECK(theron_v1_track02_external_capture_write_skeleton(&request, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED);
    CHECK(!receipt.manifest_skeleton_written);
    CHECK(theron_v1_track02_external_capture_validate(
        &request, &provenance, &preparation, &admission, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED);
    CHECK(!admission.valid);
    CHECK(theron_v1_track02_external_capture_validate_mednafen_trace(
        &request, &trace_request, &provenance, &preparation, &admission,
        &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED);
    CHECK(!admission.valid && !receipt.external_launch_performed &&
          !receipt.media_copied && !receipt.synthetic_trace_created &&
          !receipt.decoder_invoked && !receipt.mednafen_trace_source_verified &&
          !receipt.mednafen_event_log_verified &&
          !receipt.mednafen_trace_source_path[0] &&
          !receipt.mednafen_trace_source_md5[0] &&
          !receipt.mednafen_event_log_md5[0]);
    CHECK(theron_v1_track02_external_capture_validate_huc6280_log(
        &request, "/tmp/firestaff-no-such-huc6280-capture.log", &provenance,
        &preparation, &admission, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_EXTERNAL_CAPTURE_REJECTED);
    CHECK(!admission.valid);
    printf("test_theron_v1_track02_external_capture_launcher: SKIP (no local Track 02 media)\n");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
