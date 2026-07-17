#ifndef THERON_V1_TRACK02_HUC6280_CAPTURE_EVENT_LOG_H
#define THERON_V1_TRACK02_HUC6280_CAPTURE_EVENT_LOG_H

#include "theron_v1_track02_capture_trace_manifest.h"

typedef enum {
    THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_UNAVAILABLE = 0,
    THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_REJECTED,
    THERON_V1_TRACK02_HUC6280_CAPTURE_LOG_READY
} Theron_V1Track02Huc6280CaptureLogStatus;

/* A closed external HuC6280 observation log. CD-read values are retained as
 * opaque window identity only; this structure never owns or interprets the
 * captured bytes. */
typedef struct {
    Theron_V1Track02Huc6280CaptureLogStatus status;
    uint32_t consumer_trace_checksum;
    uint32_t loader_pc;
    uint32_t loader_record;
    uint32_t loader_destination;
    size_t loader_payload_bytes;
    uint32_t loader_payload_checksum;
    uint32_t dungeon_record_consumer_pc;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    uint32_t object_table_consumer_pc;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
    int opaque_cd_read_window_retained;
    int opaque_dungeon_window_retained;
    int opaque_object_window_retained;
} Theron_V1Track02Huc6280CaptureEventLog;

/* Parses only an exact ordered HuC6280 event log from a non-symlink regular
 * file. Unknown event labels, duplicate/missing rows, malformed numbers, and
 * non-file input reject. Missing input reports UNAVAILABLE. */
int theron_v1_track02_huc6280_capture_event_log_parse(
    const char *path,
    Theron_V1Track02Huc6280CaptureEventLog *out);

/* Binds observed loader and consumer coordinates to existing authenticated
 * receipts and emits the current strict manifest representation. The `$4090`
 * loader PC is source-locked; consumer PCs and windows must match preparation
 * exactly. No level/object fields, payload bytes, or visual semantics enter
 * this bridge. */
int theron_v1_track02_huc6280_capture_event_log_bind_manifest(
    const Theron_V1Track02Huc6280CaptureEventLog *log,
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02LevelObjectTracePreparationReceipt *preparation,
    Theron_V1Track02CaptureTraceManifest *out_manifest);

#endif
