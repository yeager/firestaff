#ifndef THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTER_H
#define THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTER_H

#include "theron_v1_track02_huc6280_capture_event_log.h"
#include "theron_v1_track02_capture_target_plan.h"

#define THERON_V1_TRACK02_MEDNAFEN_TRACE_PATH_CAPACITY 512

typedef enum {
    THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE = 0,
    THERON_V1_TRACK02_MEDNAFEN_TRACE_REJECTED,
    THERON_V1_TRACK02_MEDNAFEN_TRACE_CONVERTED,
    THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED
} Theron_V1Track02MednafenTraceStatus;

typedef struct {
    const char *source_trace_path;
    const char *expected_source_trace_md5;
    const char *event_log_path;
} Theron_V1Track02MednafenTraceConvertRequest;

/* Conversion provenance for an external debugger export. No emulator process
 * is launched and no Track 02 media or payload bytes are copied. */
typedef struct {
    Theron_V1Track02MednafenTraceStatus status;
    int source_trace_md5_verified;
    int source_rows_observed;
    int huc6280_event_log_written;
    int huc6280_event_log_md5_verified;
    int emulator_launched;
    int media_copied;
    int synthetic_event_created;
    char source_trace_path[THERON_V1_TRACK02_MEDNAFEN_TRACE_PATH_CAPACITY];
    char source_trace_md5[33];
    char event_log_path[THERON_V1_TRACK02_MEDNAFEN_TRACE_PATH_CAPACITY];
    char event_log_md5[33];
} Theron_V1Track02MednafenTraceConvertReceipt;

/* Reads an existing non-symlink regular-file export only to verify its closed
 * row grammar and stable MD5. It writes no event log and does not admit the
 * trace to runtime. The resulting MD5 is an operator input for conversion. */
int theron_v1_track02_mednafen_trace_inspect_file(
    const char *source_trace_path,
    Theron_V1Track02MednafenTraceConvertReceipt *out);

/* Converts only this closed instrumented Mednafen export grammar:
 * source=mednafen-pce-instrumented
 * huc6280_loader_cd_read pc=<hex> record=<hex> destination=<hex>
 *   byte_count=<decimal> payload_checksum=<hex>
 * huc6280_dungeon_consumer pc=<hex> payload_offset=<hex>
 *   byte_count=<decimal> window_checksum=<hex>
 * huc6280_object_consumer pc=<hex> payload_offset=<hex>
 *   byte_count=<decimal> window_checksum=<hex>
 * consumer_trace_checksum=<hex>
 *
 * The rows must be in that exact order and no other row is accepted. */
int theron_v1_track02_mednafen_trace_convert_file(
    const Theron_V1Track02MednafenTraceConvertRequest *request,
    Theron_V1Track02MednafenTraceConvertReceipt *out);

/* Stamps an importer-compatible opaque artifact envelope from an already
 * converted observed trace and a source-locked plan. It writes identities
 * only, never trace payloads, palette values, or bitmap bytes. */
int theron_v1_track02_mednafen_trace_stamp_handoff_envelope(
    const Theron_V1Track02MednafenTraceConvertReceipt *trace,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *envelope_path,
    char out_envelope_md5[33]);

#endif
