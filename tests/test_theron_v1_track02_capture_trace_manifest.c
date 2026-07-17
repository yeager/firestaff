#include <stdio.h>
#include <stdlib.h>

#include "theron_v1_track02_capture_trace_manifest.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); ++failures; \
} } while (0)

int main(void) {
    Theron_V1Track02CaptureTraceManifest manifest;
    Theron_V1Track02CaptureTraceOpaqueEvidenceReceipt receipt;
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt provenance = {0};
    Theron_V1Track02LevelObjectTracePreparationReceipt preparation = {0};
    const char *bad_path = "/tmp/firestaff-theron-track02-bad-capture.manifest";
    const char *good_path = "/tmp/firestaff-theron-track02-capture.manifest";
    FILE *file;

    CHECK(theron_v1_track02_capture_trace_manifest_parse(NULL, &manifest));
    CHECK(manifest.status == THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_UNAVAILABLE);
    file = fopen(bad_path, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("format=theron_track02_capture_trace_v1\nunknown_consumer_pc=0x4120\n", file);
        fclose(file);
        CHECK(theron_v1_track02_capture_trace_manifest_parse(bad_path, &manifest));
        CHECK(manifest.status == THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_REJECTED);
        remove(bad_path);
    }
    file = fopen(good_path, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("format=theron_track02_capture_trace_v1\n"
              "track02_md5=f23601102138f87c33025877767ebf76\n"
              "track02_variant=us_bin\n"
              "loader_record=0xb52\nloader_destination=0x3800\n"
              "loader_payload_bytes=2048\nloader_payload_checksum=0x7b0f13c9\n"
              "consumer_trace_checksum=0x2468ace0\n"
              "dungeon_record_consumer_pc=0x4120\n"
              "dungeon_record_payload_offset=0x114\n"
              "dungeon_record_byte_count=876\n"
              "dungeon_record_window_checksum=0x3a5d7811\n"
              "object_table_consumer_pc=0x4180\n"
              "object_table_payload_offset=0x480\n"
              "object_table_byte_count=896\n"
              "object_table_window_checksum=0x55aa7744\n", file);
        fclose(file);
        CHECK(theron_v1_track02_capture_trace_manifest_parse(good_path, &manifest));
        CHECK(manifest.status == THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_READY);
        CHECK(manifest.dungeon_record_consumer_pc == 0x4120u);
        CHECK(manifest.object_table_payload_offset == 0x480u);
        remove(good_path);
    }
    CHECK(!theron_v1_track02_capture_trace_manifest_discover_and_bind(
        NULL, NULL, &provenance, &preparation, &receipt));
    CHECK(!receipt.valid && !receipt.dungeon_draw_allowed && !receipt.pixel_decode_allowed);
    printf("test_theron_v1_track02_capture_trace_manifest: SKIP (no local Track 02 media/capture manifest)\n");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
