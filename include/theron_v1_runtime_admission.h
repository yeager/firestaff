#ifndef THERON_V1_RUNTIME_ADMISSION_H
#define THERON_V1_RUNTIME_ADMISSION_H

#include "theron_v1_capture_config.h"
#include "theron_v1_raw_loader_trace.h"

typedef struct {
    int attached;
    int admitted;
    int game_owned_fifo_payload_attached;
    int game_owned_fifo_payload_admitted;
    Theron_Track02Variant game_owned_fifo_payload_variant;
    char game_owned_fifo_payload_track02_md5[33];
    uint32_t game_owned_fifo_payload_record;
    unsigned int game_owned_fifo_payload_source_offset;
    uint8_t game_owned_fifo_payload_source_byte;
    int cdb_read6_verified;
    int fifo_to_game_ram_verified;
    int game_ram_consumer_verified;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
} Theron_V1RuntimeAdmissionReceipt;

typedef struct {
    int valid;
    int runtime_admitted;
} Theron_V1TraceSourceProvenanceReceipt;

typedef struct {
    int valid;
    int startup_session_handoff_ready;
    int runtime_capture_required;
    int game_owned_fifo_payload_admitted;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    unsigned int source_offset;
    uint8_t source_byte;
    int cdb_read6_verified;
    int fifo_to_game_ram_verified;
    int game_ram_consumer_verified;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
    int object_table_admission_allowed;
    int level_admission_allowed;
} Theron_V1RuntimeSessionHandoffReceipt;

void theron_v1_runtime_admission_init(
    Theron_V1RuntimeAdmissionReceipt *out);

void theron_v1_runtime_session_handoff_init(
    Theron_V1RuntimeSessionHandoffReceipt *out);

int theron_v1_runtime_admission_attach(
    Theron_V1RuntimeAdmissionReceipt *out,
    const char *trace_identity,
    int placeholder_or_synthetic);

int theron_v1_runtime_admission_attach_game_owned_fifo_payload(
    Theron_V1RuntimeAdmissionReceipt *out,
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payload);

int theron_v1_runtime_trace_identity_valid(
    const char *identity,
    const Theron_V1CaptureConfig *config);

int theron_v1_trace_source_provenance(
    const char *source_id,
    const char *config_identity,
    Theron_V1TraceSourceProvenanceReceipt *out);

int theron_v1_runtime_session_handoff_from_admission(
    const Theron_V1RuntimeAdmissionReceipt *admission,
    Theron_V1RuntimeSessionHandoffReceipt *out);

#endif
