#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "asset_status_m12.h"
#include "theron_v1_track02_mednafen_trace_converter.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); ++failures; \
} } while (0)

static void write_fixture(FILE *file, int unknown) {
    fputs("source=mednafen-pce-instrumented\n"
          "huc6280_loader_cd_read pc=4090 record=b52 destination=3800 byte_count=2048 payload_checksum=7b0f13c9\n"
          "huc6280_dungeon_consumer pc=4120 payload_offset=114 byte_count=876 window_checksum=3a5d7811\n", file);
    if (unknown) fputs("unrecognised_debugger_row value=1\n", file);
    else fputs("huc6280_object_consumer pc=4180 payload_offset=480 byte_count=896 window_checksum=55aa7744\n"
               "consumer_trace_checksum=2468ace0\n", file);
}

int main(void) {
    const char *source = "/tmp/firestaff-theron-mednafen-trace.txt";
    const char *source_alias = "/tmp/./firestaff-theron-mednafen-trace.txt";
    const char *source_link = "/tmp/firestaff-theron-mednafen-trace-link.txt";
    const char *output = "/tmp/firestaff-theron-huc6280-event.log";
    Theron_V1Track02MednafenTraceConvertRequest request = {0};
    Theron_V1Track02MednafenTraceConvertReceipt receipt;
    Theron_V1Track02Huc6280CaptureEventLog event_log;
    char md5[33];
    char event_log_md5[33];
    FILE *file;

    CHECK(theron_v1_track02_mednafen_trace_convert_file(NULL, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE);
    CHECK(theron_v1_track02_mednafen_trace_inspect_file(NULL, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE);
    CHECK(theron_v1_track02_mednafen_trace_inspect_file("/dev/null", &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED);
    request.source_trace_path = "/dev/null";
    request.expected_source_trace_md5 = "00000000000000000000000000000000";
    request.event_log_path = output;
    CHECK(theron_v1_track02_mednafen_trace_convert_file(&request, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED);
    remove(output);
    file = fopen(source, "wb");
    CHECK(file != NULL);
    if (file) { write_fixture(file, 0); fclose(file); }
    CHECK(m12_file_md5_hex(source, md5));
    CHECK(theron_v1_track02_mednafen_trace_inspect_file(source, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED);
    CHECK(receipt.source_trace_md5_verified && receipt.source_rows_observed &&
          !receipt.huc6280_event_log_written && !receipt.event_log_path[0]);
    CHECK(!strcmp(receipt.source_trace_md5, md5));
#if !defined(_WIN32)
    remove(source_link);
    CHECK(symlink(source, source_link) == 0);
    CHECK(theron_v1_track02_mednafen_trace_inspect_file(source_link, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED);
    remove(source_link);
#endif
    request.source_trace_path = source;
    request.expected_source_trace_md5 = md5;
    request.event_log_path = output;
    CHECK(theron_v1_track02_mednafen_trace_convert_file(&request, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED);
    CHECK(receipt.source_trace_md5_verified && receipt.source_rows_observed &&
          receipt.huc6280_event_log_md5_verified);
    CHECK(!receipt.emulator_launched && !receipt.media_copied && !receipt.synthetic_event_created);
    CHECK(theron_v1_track02_huc6280_capture_event_log_parse(output, &event_log));
    CHECK(event_log.status == THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY);
    CHECK(m12_file_md5_hex(output, event_log_md5));
    CHECK(!strcmp(event_log_md5, receipt.event_log_md5));
    CHECK(theron_v1_track02_mednafen_trace_convert_file(&request, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED);
    CHECK(m12_file_md5_hex(output, md5));
    CHECK(!strcmp(md5, event_log_md5));
    request.event_log_path = source;
    CHECK(theron_v1_track02_mednafen_trace_convert_file(&request, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED);
    CHECK(m12_file_md5_hex(source, md5));
    CHECK(!strcmp(md5, request.expected_source_trace_md5));
    request.event_log_path = source_alias;
    CHECK(theron_v1_track02_mednafen_trace_convert_file(&request, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED);
    CHECK(m12_file_md5_hex(source, md5));
    CHECK(!strcmp(md5, request.expected_source_trace_md5));
    request.event_log_path = output;
    remove(output);
    file = fopen(source, "wb");
    CHECK(file != NULL);
    if (file) { write_fixture(file, 1); fclose(file); }
    CHECK(m12_file_md5_hex(source, md5));
    request.expected_source_trace_md5 = md5;
    CHECK(theron_v1_track02_mednafen_trace_convert_file(&request, &receipt));
    CHECK(receipt.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED);
    remove(source);
    remove(output);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
