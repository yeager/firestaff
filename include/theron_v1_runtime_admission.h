#ifndef THERON_V1_RUNTIME_ADMISSION_H
#define THERON_V1_RUNTIME_ADMISSION_H

#include "theron_v1_capture_config.h"
#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_startup_runtime_entry.h"

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

typedef struct {
    int valid;
    int session_handoff_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    unsigned int source_offset;
    uint8_t source_byte;
    unsigned int all_dungeon_capture_mask;
    int all_dungeon_capture_count;
    unsigned int no_fallback_semantic_role_mask;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    int startup_level_anchor_status;
    uint64_t startup_level_anchor_raw_offset;
    uint64_t startup_level_anchor_user_data_offset;
    int startup_level_anchor_user_data_valid;
    uint16_t startup_level_anchor_width;
    uint16_t startup_level_anchor_height;
    uint32_t startup_level_anchor_seed;
    uint16_t startup_level_anchor_level_index;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    uint32_t all_dungeon_route_hash;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    int object_table_admission_allowed;
    int level_admission_allowed;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
} Theron_V1RuntimeBoundedTrack02RouteReceipt;

typedef struct {
    int valid;
    int bounded_route_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    unsigned int source_offset;
    uint8_t source_byte;
    uint64_t startup_level_raw_offset;
    uint64_t startup_level_user_data_offset;
    int startup_level_user_data_valid;
    uint16_t startup_level_width;
    uint16_t startup_level_height;
    uint32_t startup_level_seed;
    uint16_t startup_level_index;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    int startup_level_anchor_admitted;
    int object_table_admission_allowed;
    int nonstartup_level_admission_allowed;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
} Theron_V1RuntimeStartupLevelAnchorReceipt;

typedef struct {
    int valid;
    int startup_level_anchor_consumed;
    int level_route_receipt_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    unsigned int source_offset;
    uint8_t source_byte;
    unsigned int descriptor_anchor_mask;
    int descriptor_anchor_count;
    unsigned int nonstartup_level_candidate_anchor_mask;
    int nonstartup_level_candidate_count;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    uint64_t first_candidate_raw_offset;
    uint64_t first_candidate_user_data_offset;
    int first_candidate_user_data_valid;
    uint32_t first_candidate_byte_count;
    uint32_t first_candidate_hash;
    uint16_t first_candidate_header_width;
    uint16_t first_candidate_header_height;
    uint32_t first_candidate_header_seed;
    uint16_t first_candidate_header_level_index;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    int nonstartup_level_decode_ready;
    int nonstartup_level_admission_allowed;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
} Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt;

typedef struct {
    int valid;
    int startup_level_anchor_consumed;
    int object_table_route_receipt_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    unsigned int source_offset;
    uint8_t source_byte;
    unsigned int descriptor_anchor_mask;
    int descriptor_anchor_count;
    unsigned int object_table_candidate_anchor_mask;
    int object_table_candidate_count;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    size_t first_candidate_entry_index;
    uint64_t first_candidate_raw_offset;
    uint64_t first_candidate_user_data_offset;
    int first_candidate_user_data_valid;
    uint32_t first_candidate_byte_count;
    uint32_t first_candidate_nonzero_byte_count;
    uint32_t first_candidate_hash;
    uint32_t first_candidate_descriptor_delta;
    int first_candidate_after_descriptor;
    int first_candidate_binding_status;
    int first_candidate_reject_reason;
    uint32_t first_candidate_declared_record_count;
    uint32_t first_candidate_record_count;
    uint32_t first_candidate_required_byte_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    uint32_t all_dungeon_route_hash;
    int object_table_decode_ready;
    int object_table_admission_allowed;
    int exact_object_semantics_ready;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
} Theron_V1RuntimeObjectTableRouteEvidenceReceipt;

void theron_v1_runtime_admission_init(
    Theron_V1RuntimeAdmissionReceipt *out);

void theron_v1_runtime_session_handoff_init(
    Theron_V1RuntimeSessionHandoffReceipt *out);

void theron_v1_runtime_bounded_track02_route_init(
    Theron_V1RuntimeBoundedTrack02RouteReceipt *out);

void theron_v1_runtime_startup_level_anchor_init(
    Theron_V1RuntimeStartupLevelAnchorReceipt *out);

void theron_v1_runtime_nonstartup_level_route_evidence_init(
    Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt *out);

void theron_v1_runtime_object_table_route_evidence_init(
    Theron_V1RuntimeObjectTableRouteEvidenceReceipt *out);

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

int theron_v1_runtime_session_handoff_bind_bounded_track02_route(
    const Theron_V1RuntimeSessionHandoffReceipt *session,
    const Theron_V1StartupAllDungeonRouteReceipt *route,
    Theron_V1RuntimeBoundedTrack02RouteReceipt *out);

int theron_v1_runtime_bounded_track02_route_bind_startup_level_anchor(
    const Theron_V1RuntimeBoundedTrack02RouteReceipt *route,
    Theron_V1RuntimeStartupLevelAnchorReceipt *out);

int theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence(
    const Theron_V1RuntimeStartupLevelAnchorReceipt *startup_anchor,
    const Theron_Track02LevelRouteReceipt *level_route,
    Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt *out);

int theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence(
    const Theron_V1RuntimeStartupLevelAnchorReceipt *startup_anchor,
    const Theron_Track02ObjectTableRouteReceipt *object_route,
    Theron_V1RuntimeObjectTableRouteEvidenceReceipt *out);

#endif
