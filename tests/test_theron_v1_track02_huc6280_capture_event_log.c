#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "theron_v1_track02_huc6280_capture_event_log.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); ++failures; \
} } while (0)

static void write_log(FILE *file, unsigned int loader_pc) {
    fprintf(file,
            "THERON_HUC6280_CAPTURE_EVENT_LOG_V1\n"
            "consumer_trace_checksum=0x2468ace0\n"
            "event=loader_cd_read\npc=0x%x\nrecord=0xb52\n"
            "destination=0x3800\nbyte_count=2048\npayload_checksum=0x7b0f13c9\n"
            "event=dungeon_consumer\npc=0x4120\npayload_offset=0x114\n"
            "byte_count=876\nwindow_checksum=0x3a5d7811\n"
            "event=object_consumer\npc=0x4180\npayload_offset=0x480\n"
            "byte_count=896\nwindow_checksum=0x55aa7744\n", loader_pc);
}

int main(void) {
    const char *tmpdir = getenv("TMPDIR");
    char path[1024];
    char link_path[1024];
    Theron_V1Track02Huc6280CaptureEventLog log;
    Theron_V1Track02CaptureTraceManifest manifest;
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt provenance = {0};
    Theron_V1Track02LevelObjectTracePreparationReceipt preparation = {0};
    FILE *file;

    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    snprintf(path, sizeof(path), "%s/firestaff-theron-huc6280-capture.log", tmpdir);
    snprintf(link_path, sizeof(link_path), "%s/firestaff-theron-huc6280-capture-link.log", tmpdir);

    CHECK(theron_v1_track02_huc6280_capture_event_log_parse(NULL, &log));
    CHECK(log.status == THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_UNAVAILABLE);
    CHECK(theron_v1_track02_huc6280_capture_event_log_parse("/dev/null", &log));
    CHECK(log.status == THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_REJECTED);
    file = fopen(path, "wb");
    CHECK(file != NULL);
    if (file) {
        write_log(file, THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS);
        fclose(file);
    }
    CHECK(theron_v1_track02_huc6280_capture_event_log_parse(path, &log));
    CHECK(log.status == THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY);
    CHECK(log.opaque_cd_read_window_retained && log.opaque_dungeon_window_retained);
#if !defined(_WIN32)
    remove(link_path);
    CHECK(symlink(path, link_path) == 0);
    CHECK(theron_v1_track02_huc6280_capture_event_log_parse(
        link_path, &log));
    CHECK(log.status == THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_REJECTED);
    remove(link_path);
    CHECK(theron_v1_track02_huc6280_capture_event_log_parse(path, &log));
    CHECK(log.status == THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY);
#endif
    provenance.valid = 1;
    provenance.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(provenance.track02_md5, sizeof(provenance.track02_md5), "%s", THERON_TRACK02_MD5_US_BIN);
    provenance.loader_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    provenance.loader_destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    provenance.loader_payload_bytes = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    provenance.loader_payload_checksum = 0x7b0f13c9u;
    preparation.valid = 1;
    preparation.track02_variant = provenance.track02_variant;
    snprintf(preparation.track02_md5, sizeof(preparation.track02_md5), "%s", provenance.track02_md5);
    preparation.loader_record = provenance.loader_record;
    preparation.consumer_trace_checksum = 0x2468ace0u;
    preparation.dungeon_record_consumer_pc = 0x4120u;
    preparation.dungeon_record_payload_offset = THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    preparation.dungeon_record_byte_count = THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    preparation.dungeon_record_window_checksum = 0x3a5d7811u;
    preparation.object_table_consumer_pc = 0x4180u;
    preparation.object_table_payload_offset = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    preparation.object_table_byte_count = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    preparation.object_table_window_checksum = 0x55aa7744u;
    CHECK(theron_v1_track02_huc6280_capture_event_log_bind_manifest(
        &log, &provenance, &preparation, &manifest));
    CHECK(manifest.status == THERON_V1_TRACK02_CAPTURE_TRACE_MANIFEST_READY);
    CHECK(manifest.dungeon_record_consumer_pc == preparation.dungeon_record_consumer_pc);
    file = fopen(path, "wb");
    CHECK(file != NULL);
    if (file) {
        write_log(file, 0x4091u);
        fclose(file);
        CHECK(theron_v1_track02_huc6280_capture_event_log_parse(path, &log));
        CHECK(!theron_v1_track02_huc6280_capture_event_log_bind_manifest(
            &log, &provenance, &preparation, &manifest));
    }
    file = fopen(path, "wb");
    CHECK(file != NULL);
    if (file) {
        fputs("THERON_HUC6280_CAPTURE_EVENT_LOG_V1\n"
              "consumer_trace_checksum=0x2468ace0\n"
              "event=unknown_consumer\n", file);
        fclose(file);
        CHECK(theron_v1_track02_huc6280_capture_event_log_parse(path, &log));
        CHECK(log.status == THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_REJECTED);
    }
    remove(path);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
