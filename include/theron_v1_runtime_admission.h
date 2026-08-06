#ifndef THERON_V1_RUNTIME_ADMISSION_H
#define THERON_V1_RUNTIME_ADMISSION_H

#include "theron_v1_capture_config.h"
#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v1_track02_capture_campaign.h"
#include "theron_v1_track02_capture_trace_runtime_admission.h"

/* The only host-facing route currently backed by the authenticated US Track
 * 02 dungeon handoff. Keep this closed rather than accepting arbitrary labels
 * as provenance. */
#define THERON_V1_TRACK02_ORIGINAL_HOST_DUNGEON_ROUTE \
    "theron-v1-original-host-route-track02-dungeon"

/* Capture readiness is deliberately distinct from the existing startup and
 * drawing paths. A positive receipt exposes only that all three external
 * observations remain source-consistent. */
typedef struct {
    int valid;
    int startup_capture_ready;
    int soul_room_capture_ready;
    int dungeon_capture_ready;
    int campaign_consumed;
    int startup_media_consumed;
    int dungeon_window_consumed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    int level_object_semantics_allowed;
    int pixel_decode_allowed;
    int render_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02CaptureCampaignAdmissionReceipt;

int theron_v1_runtime_bind_track02_capture_campaign_admission(
    const Theron_StartupMediaStateReceipt *startup_media,
    const Theron_V1Track02CaptureCampaignReceipt *campaign,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *dungeon_window,
    Theron_V1RuntimeTrack02CaptureCampaignAdmissionReceipt *out);

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

typedef struct {
    int valid;
    int startup_level_anchor_consumed;
    int nonstartup_level_evidence_consumed;
    int object_table_evidence_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    unsigned int source_offset;
    uint8_t source_byte;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    unsigned int nonstartup_level_candidate_anchor_mask;
    int nonstartup_level_candidate_count;
    uint32_t first_nonstartup_level_candidate_hash;
    unsigned int object_table_candidate_anchor_mask;
    int object_table_candidate_count;
    uint32_t first_object_table_candidate_hash;
    int capture_consumer_route_ready;
    int object_table_decode_ready;
    int nonstartup_level_decode_ready;
    int object_table_admission_allowed;
    int nonstartup_level_admission_allowed;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02CaptureConsumerGapReceipt;

typedef struct {
    int valid;
    int capture_consumer_gap_consumed;
    int original_consumer_trace_bound;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    unsigned int source_offset;
    uint8_t source_byte;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t consumer_trace_checksum;
    int capture_consumer_route_ready;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    int payload_semantics_proven;
    int visual_semantics_proven;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02ConsumerSemanticReceipt;

typedef struct {
    int valid;
    int same_capture_as_consumer_semantics;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t consumer_trace_checksum;
    uint32_t decoded_level_hash;
    uint32_t decoded_object_table_hash;
    uint32_t decoded_bitmap_hash;
    uint32_t decoded_palette_hash;
    int level_consumer_proven;
    int object_table_consumer_proven;
    int bitmap_consumer_proven;
    int palette_consumer_proven;
    int decoded_bitmap_pixels_proven;
    int decoded_palette_words_proven;
    int synthetic_level_promoted;
    int synthetic_object_table_promoted;
    int synthetic_bitmap_promoted;
    int synthetic_palette_promoted;
    int fallback_visuals_observed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02RenderAssetProof;

typedef struct {
    int valid;
    int consumer_semantics_consumed;
    int real_render_asset_proof_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t consumer_trace_checksum;
    uint32_t decoded_level_hash;
    uint32_t decoded_object_table_hash;
    uint32_t decoded_bitmap_hash;
    uint32_t decoded_palette_hash;
    int object_table_admission_allowed;
    int level_admission_allowed;
    int bitmap_admission_allowed;
    int palette_admission_allowed;
    int real_render_assets_admitted;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt;

typedef struct {
    int valid;
    int verified_track02_capture_consumed;
    int fail_closed;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t nonstartup_sector_receipt_hash;
    uint32_t nonstartup_container_index_hash;
    size_t palette_raw_offset;
    size_t palette_user_data_offset;
    uint32_t palette_payload_checksum;
    uint32_t palette_decoded_checksum;
    int palette_format_valid;
    int palette_semantic_binding_verified;
    int palette_promotion_allowed;
    size_t nonstartup_anchor_count;
    size_t nonstartup_window_count;
    size_t first_nonstartup_entry_index;
    size_t first_nonstartup_raw_offset;
    size_t first_nonstartup_user_data_offset;
    size_t first_nonstartup_byte_count;
    uint32_t first_nonstartup_raw_hash;
    size_t indexed_container_count;
    size_t first_container_entry_index;
    size_t first_container_raw_offset;
    size_t first_container_user_data_offset;
    size_t first_container_user_data_byte_count;
    uint32_t first_container_user_data_hash;
    int nonstartup_level_decode_ready;
    int object_table_decode_ready;
    int render_asset_admission_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt;

typedef struct {
    int valid;
    int original_data_gap_consumed;
    int original_consumer_trace_consumed;
    int same_original_capture_as_gap;
    int fail_closed_until_consumer_proven;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t loader_record_user_data_offset;
    uint32_t loader_destination;
    uint32_t loader_payload_bytes;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
    size_t palette_raw_offset;
    size_t palette_user_data_offset;
    uint32_t palette_payload_checksum;
    uint32_t palette_decoded_checksum;
    size_t nonstartup_level_raw_offset;
    size_t nonstartup_level_user_data_offset;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_raw_offset;
    size_t object_table_user_data_offset;
    uint32_t object_table_user_data_hash;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    int palette_consumer_bound;
    int nonstartup_level_consumer_bound;
    int object_table_consumer_bound;
    int bitmap_consumer_bound;
    int runtime_consumer_binding_ready;
    int render_asset_admission_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt;

typedef struct {
    int valid;
    int original_data_gap_consumed;
    int original_consumer_binding_consumed;
    int same_original_capture_as_gap;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t loader_record_user_data_offset;
    uint32_t loader_destination;
    uint32_t loader_payload_bytes;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
    size_t nonstartup_level_raw_offset;
    size_t nonstartup_level_raw_sector;
    size_t nonstartup_level_raw_sector_user_data_offset;
    size_t nonstartup_level_user_data_offset;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_raw_offset;
    size_t object_table_raw_sector;
    size_t object_table_raw_sector_user_data_offset;
    size_t object_table_user_data_offset;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    int raw_sector_user_data_bound;
    int nonstartup_dungeon_path_ready;
    int exact_level_fields_blocked;
    int exact_object_fields_blocked;
    int bitmap_route_bound;
    int palette_binding_verified;
    int rgba_output_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt;

typedef struct {
    int valid;
    int raw_nonstartup_dungeon_handoff_consumed;
    int object_dungeon_grammar_consumed;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t loader_record_user_data_offset;
    uint32_t loader_destination;
    uint32_t loader_payload_bytes;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
    size_t nonstartup_level_raw_offset;
    size_t nonstartup_level_raw_sector;
    size_t nonstartup_level_raw_sector_user_data_offset;
    size_t nonstartup_level_user_data_offset;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_raw_offset;
    size_t object_table_raw_sector;
    size_t object_table_raw_sector_user_data_offset;
    size_t object_table_user_data_offset;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    int raw_sector_user_data_bound;
    int dungeon_record_grammar_proven;
    int object_table_grammar_proven;
    int nonstartup_level_admission_allowed;
    int object_table_admission_allowed;
    int exact_level_fields_blocked;
    int exact_object_fields_blocked;
    int bitmap_route_bound;
    int palette_binding_verified;
    int rgba_output_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt;

typedef struct {
    int valid;
    int object_level_admission_consumed;
    int same_capture_as_object_level_admission;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    size_t nonstartup_level_raw_offset;
    size_t nonstartup_level_raw_sector;
    size_t nonstartup_level_raw_sector_user_data_offset;
    size_t nonstartup_level_user_data_offset;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    uint32_t dungeon_record_consumer_pc;
    size_t dungeon_record_payload_offset;
    size_t dungeon_record_byte_count;
    uint32_t dungeon_record_window_checksum;
    int source_nonstartup_level_bytes_bound;
    int nonstartup_level_record_route_observed;
    int exact_level_fields_blocked;
    int object_table_layout_blocked;
    int bitmap_route_bound;
    int palette_binding_verified;
    int rgba_output_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt;

typedef struct {
    int valid;
    int object_level_admission_consumed;
    int same_capture_as_object_level_admission;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t object_table_route_hash;
    size_t object_table_raw_offset;
    size_t object_table_raw_sector;
    size_t object_table_raw_sector_user_data_offset;
    size_t object_table_user_data_offset;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t object_table_consumer_pc;
    size_t object_table_payload_offset;
    size_t object_table_byte_count;
    uint32_t object_table_window_checksum;
    int source_object_table_bytes_bound;
    int object_table_route_observed;
    int object_table_layout_blocked;
    int exact_object_fields_blocked;
    int bitmap_route_bound;
    int palette_binding_verified;
    int rgba_output_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt;

typedef struct {
    int valid;
    int nonstartup_level_record_evidence_consumed;
    int object_table_route_evidence_consumed;
    int same_capture_as_object_level_admission;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    size_t nonstartup_level_raw_offset;
    size_t nonstartup_level_user_data_offset;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_raw_offset;
    size_t object_table_user_data_offset;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    uint32_t dungeon_record_window_checksum;
    uint32_t object_table_window_checksum;
    int source_nonstartup_level_bytes_bound;
    int source_object_table_bytes_bound;
    int level_object_pair_route_observed;
    int exact_level_fields_blocked;
    int exact_object_fields_blocked;
    int object_table_layout_blocked;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt;

typedef struct {
    int valid;
    int level_object_handoff_evidence_consumed;
    int same_capture_as_level_object_handoff;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    uint32_t dungeon_record_window_checksum;
    uint32_t object_table_window_checksum;
    int source_nonstartup_level_bytes_bound;
    int source_object_table_bytes_bound;
    int field_decoder_required;
    int exact_level_fields_blocked;
    int exact_object_fields_blocked;
    int object_table_layout_blocked;
    int dungeon_route_handoff_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt;

typedef struct {
    int valid;
    int level_object_field_boundary_consumed;
    int same_capture_as_field_boundary;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    char reviewed_decoder_identity[64];
    int reviewed_decoder_source_bound;
    int field_decoder_required;
    int field_decoder_execution_allowed;
    int exact_level_fields_blocked;
    int exact_object_fields_blocked;
    int object_table_layout_blocked;
    int dungeon_route_handoff_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt;

typedef struct {
    int valid;
    int reviewed_field_decoder_boundary_consumed;
    int same_capture_as_reviewed_decoder_boundary;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    char reviewed_decoder_identity[64];
    int reviewed_decoder_source_bound;
    int field_decoder_required;
    int field_decoder_execution_allowed;
    int real_track02_level_object_boundary_bound;
    int dungeon_route_review_required;
    int dungeon_route_handoff_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt;

typedef struct {
    int valid;
    int dungeon_route_boundary_consumed;
    int level_object_field_boundary_consumed;
    int same_capture_as_dungeon_route_boundary;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    char reviewed_decoder_identity[64];
    int real_track02_level_object_boundary_bound;
    int field_decoder_required;
    int field_decoder_execution_allowed;
    int dungeon_route_review_required;
    int dungeon_route_handoff_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt;

typedef struct {
    int valid;
    int level_object_facts_handoff_consumed;
    int same_capture_as_facts_handoff;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t selected_dungeon_index;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    int dungeon_selection_route_observed;
    int level_record_route_bound;
    int level_record_review_required;
    int object_table_layout_blocked;
    int field_decoder_execution_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt;

typedef struct {
    int valid;
    int dungeon_selection_level_record_boundary_consumed;
    int same_capture_as_dungeon_selection_boundary;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t selected_dungeon_index;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    int level_record_table_route_bound;
    int object_table_route_bound;
    int level_object_table_pair_bound;
    int level_record_review_required;
    int object_table_layout_review_required;
    int field_decoder_execution_allowed;
    int dungeon_route_handoff_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt;

typedef struct {
    int valid;
    int dungeon_object_level_table_binding_consumed;
    int same_capture_as_table_binding;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t selected_dungeon_index;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t loader_route_pair_hash;
    size_t nonstartup_level_byte_count;
    uint32_t nonstartup_level_raw_hash;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    uint32_t dungeon_record_consumer_pc;
    uint32_t object_table_consumer_pc;
    int loader_route_record_bound;
    int loader_route_source_windows_bound;
    int level_object_table_pair_bound;
    int loader_route_review_required;
    int field_decoder_execution_allowed;
    int dungeon_route_handoff_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt;

typedef struct {
    int valid;
    int level_object_loader_route_consumed;
    int object_table_shape_consumed;
    int same_capture_as_loader_route;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t selected_dungeon_index;
    uint32_t selected_level_index;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t loader_route_pair_hash;
    size_t object_table_user_data_byte_count;
    uint32_t object_table_user_data_hash;
    size_t object_record_count;
    uint32_t object_table_checksum;
    unsigned int object_level_mask;
    size_t selected_level_record_count;
    uint32_t selected_level_record_hash;
    uint32_t selected_level_position_hash;
    uint32_t object_placement_state_hash;
    uint8_t first_object_id;
    uint8_t first_object_kind;
    uint8_t first_object_x;
    uint8_t first_object_y;
    uint8_t first_object_level_index;
    uint8_t first_object_state_low_bits;
    uint8_t first_object_flags;
    uint16_t first_object_argument;
    int object_placement_bytes_bound;
    int object_state_low_bits_bound;
    int object_kind_semantics_review_required;
    int world_object_publish_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02ObjectPlacementStateReceipt;

typedef struct {
    int valid;
    int object_placement_state_consumed;
    int object_table_shape_consumed;
    int same_capture_as_placement_state;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t selected_dungeon_index;
    uint32_t selected_level_index;
    uint32_t object_table_route_hash;
    uint32_t loader_route_pair_hash;
    uint32_t object_placement_state_hash;
    size_t selected_level_record_count;
    uint32_t selected_level_record_hash;
    uint32_t selected_level_position_hash;
    unsigned int runtime_kind_low_mask;
    int runtime_kind_quest_item_seen;
    uint32_t object_runtime_state_hash;
    uint8_t first_runtime_type;
    uint8_t first_runtime_state;
    uint32_t first_runtime_flags;
    int first_runtime_quantity;
    int object_kind_semantics_proven;
    int flags_low_bits_state_bound;
    int argument_quantity_bound;
    int object_flags_preserved;
    int all_selected_records_runtime_mappable;
    int world_object_publish_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt;

typedef struct {
    int valid;
    int object_gameplay_semantics_consumed;
    int object_table_shape_consumed;
    int world_mutated;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t selected_level_index;
    size_t selected_level_record_count;
    int before_object_count;
    int removed_selected_level_object_count;
    int placed_object_count;
    int after_object_count;
    int level_loaded_required;
    int level_loaded;
    int current_level_after;
    int thing_count_after;
    uint64_t before_world_hash;
    uint64_t after_world_hash;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt;

typedef struct {
    int valid;
    int source_object_world_handoff_consumed;
    int target_object_gameplay_semantics_consumed;
    int same_capture_as_target_loader_route;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t consumer_trace_checksum;
    uint32_t selected_dungeon_index;
    uint32_t source_level_index;
    uint32_t target_level_index;
    uint32_t target_level_route_hash;
    uint32_t target_object_table_route_hash;
    uint32_t target_loader_route_pair_hash;
    uint32_t target_object_runtime_state_hash;
    size_t target_level_byte_count;
    uint32_t target_level_raw_hash;
    size_t target_object_record_count;
    uint32_t target_object_level_record_hash;
    int loader_level_selector_bound;
    int transition_source_level_bound;
    int transition_target_level_bound;
    int party_placement_bound;
    int object_pool_state_bound;
    int level_runtime_load_allowed;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02LevelTransitionHandoffReceipt;

typedef struct {
    int valid;
    int level_transition_handoff_consumed;
    int target_object_world_handoff_consumed;
    int world_mutated;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t source_level_index;
    uint32_t target_level_index;
    int transition_pending_before;
    int transition_pending_after;
    int level_loaded;
    int current_level_after;
    int party_x;
    int party_y;
    int party_dir;
    int target_object_count;
    int target_thing_count;
    uint64_t before_world_hash;
    uint64_t after_world_hash;
    int dungeon_runtime_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt;

typedef struct {
    int valid;
    int level_transition_runtime_consumed;
    int same_capture_as_level_transition;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t source_level_index;
    uint32_t target_level_index;
    size_t palette_raw_offset;
    size_t palette_user_data_offset;
    uint32_t palette_payload_checksum;
    uint32_t palette_decoded_checksum;
    uint32_t bitmap_route_mask;
    uint32_t bitmap_atlas_checksum;
    uint32_t bitmap_atlas_route_count;
    uint32_t bitmap_atlas_nonzero_pixel_count;
    uint32_t bitmap_palette_source_hash;
    int palette_window_source_bound;
    int bitmap_route_source_bound;
    int palette_decode_verified;
    int bitmap_decode_verified;
    int pixel_output_verified;
    int m11_render_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt;

typedef struct {
    int valid;
    int bitmap_palette_source_consumed;
    int real_track02_bytes_consumed;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t source_level_index;
    uint32_t target_level_index;
    size_t palette_raw_offset;
    size_t palette_user_data_offset;
    uint32_t palette_payload_checksum;
    uint32_t palette_decoded_checksum;
    size_t palette_nonblack_entry_count;
    uint16_t first_palette_word;
    uint8_t first_palette_red;
    uint8_t first_palette_green;
    uint8_t first_palette_blue;
    uint32_t bitmap_route_mask;
    uint32_t bitmap_atlas_checksum;
    size_t bitmap_atlas_route_count;
    size_t bitmap_atlas_tile_count;
    size_t bitmap_atlas_nonzero_pixel_count;
    unsigned int first_bitmap_route_bit;
    size_t first_bitmap_route_width;
    size_t first_bitmap_route_height;
    size_t first_bitmap_route_tile_count;
    size_t first_bitmap_route_nonzero_pixel_count;
    uint32_t first_bitmap_route_checksum;
    size_t first_bitmap_raw_offset;
    size_t first_bitmap_user_data_offset;
    unsigned int stage_bitmap_route_bit;
    size_t stage_bitmap_route_width;
    size_t stage_bitmap_route_height;
    size_t stage_bitmap_route_tile_count;
    size_t stage_bitmap_route_nonzero_pixel_count;
    uint32_t stage_bitmap_route_checksum;
    size_t stage_bitmap_raw_offset;
    size_t stage_bitmap_user_data_offset;
    uint8_t first_pixel_indices[8];
    uint32_t first_pixel_row_hash;
    int palette_decode_verified;
    int bitmap_decode_verified;
    int pixel_output_verified;
    int m11_runtime_consumption_allowed;
    int m11_render_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt;

typedef struct {
    int valid;
    int decode_vector_consumed;
    int world_runtime_media_consumed;
    int soul_room_level0_selected;
    int exact_indexed_atlas_consumed;
    int huc6260_palette_consumed;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t level_index;
    unsigned int route_bit;
    uint16_t source_width;
    uint16_t source_height;
    size_t source_tile_count;
    size_t source_nonzero_pixel_count;
    uint32_t source_checksum;
    uint32_t palette_decoded_checksum;
    size_t palette_nonblack_entry_count;
    size_t first_raw_offset;
    size_t last_raw_offset;
    size_t first_user_data_offset;
    int host_surface_width;
    int host_surface_height;
    int placement_x;
    int placement_y;
    int scale_x;
    int scale_y;
    int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
    uint32_t placement_hash;
    int clip_verified;
    int scale_verified;
    int host_presentation_allowed;
    int m11_runtime_consumption_allowed;
    int m11_render_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02M11SoulRoomConsumptionReceipt;

typedef struct {
    int valid;
    int decode_vector_consumed;
    int level_transition_runtime_consumed;
    int world_runtime_media_consumed;
    int target_level_selected;
    int exact_indexed_atlas_consumed;
    int huc6260_palette_consumed;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t source_level_index;
    uint32_t target_level_index;
    unsigned int route_bit;
    uint16_t source_width;
    uint16_t source_height;
    size_t source_tile_count;
    size_t source_nonzero_pixel_count;
    uint32_t source_checksum;
    uint32_t palette_decoded_checksum;
    size_t palette_nonblack_entry_count;
    size_t first_raw_offset;
    size_t last_raw_offset;
    size_t first_user_data_offset;
    int host_surface_width;
    int host_surface_height;
    int placement_x;
    int placement_y;
    int scale_x;
    int scale_y;
    int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
    uint32_t placement_hash;
    int clip_verified;
    int scale_verified;
    int host_presentation_allowed;
    int m11_runtime_consumption_allowed;
    int m11_render_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02M11LevelConsumptionReceipt;

typedef struct {
    int valid;
    int m11_level_consumption_consumed;
    int level_transition_runtime_consumed;
    int world_runtime_geometry_consumed;
    int object_placement_consumed;
    int viewport_composition_route_bound;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t target_level_index;
    unsigned int route_bit;
    uint16_t media_width;
    uint16_t media_height;
    uint32_t media_checksum;
    uint32_t palette_decoded_checksum;
    int level_width;
    int level_height;
    int party_x;
    int party_y;
    int party_dir;
    int current_square;
    int forward_square;
    size_t sampled_cell_count;
    size_t sampled_wall_count;
    size_t sampled_floor_count;
    size_t sampled_special_count;
    size_t sampled_object_count;
    uint32_t level_geometry_hash;
    uint32_t object_placement_hash;
    uint32_t viewport_route_hash;
    int host_surface_width;
    int host_surface_height;
    int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
    int m11_host_presentation_allowed;
    int dungeon_draw_route_allowed;
    int dungeon_pixel_blit_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt;

typedef struct {
    int valid;
    int m11_level_consumption_consumed;
    int level_transition_runtime_consumed;
    int original_data_binding_gap_consumed;
    int world_runtime_state_inspected;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t selected_dungeon_index;
    uint32_t target_level_index;
    unsigned int route_bit;
    uint32_t media_checksum;
    uint32_t palette_decoded_checksum;
    size_t nonstartup_window_count;
    size_t first_nonstartup_raw_offset;
    size_t first_nonstartup_user_data_offset;
    size_t first_nonstartup_byte_count;
    uint32_t first_nonstartup_raw_hash;
    size_t object_table_raw_offset;
    size_t object_table_user_data_offset;
    size_t object_table_byte_count;
    uint32_t object_table_raw_hash;
    int level1_world_geometry_loaded;
    int level1_object_placement_loaded;
    int transition_level_loaded;
    int transition_target_object_count;
    int transition_target_thing_count;
    int world_object_count;
    int real_track02_level1_media_bound;
    int nonstartup_geometry_source_blocked;
    int object_placement_source_blocked;
    int loadertrace_geometry_window_missing;
    int loadertrace_object_window_missing;
    int dungeon_draw_route_allowed;
    int dungeon_pixel_blit_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02Level1DrawBlockerReceipt;

typedef struct {
    int valid;
    int same_capture_as_render_admission;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t consumer_trace_checksum;
    uint32_t decoded_level_hash;
    uint32_t decoded_object_table_hash;
    uint32_t decoded_bitmap_hash;
    uint32_t decoded_palette_hash;
    int dungeon_runtime_consumer_bound;
    int object_table_layout_proven;
    int bitmap_palette_decode_proven;
    int source_level_bytes_bound;
    int source_object_table_bytes_bound;
    int source_bitmap_bytes_bound;
    int source_palette_bytes_bound;
    int synthetic_dungeon_state_promoted;
    int synthetic_object_layout_promoted;
    int synthetic_bitmap_decode_promoted;
    int synthetic_palette_decode_promoted;
    int fallback_visuals_observed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02DungeonHandoffProof;

typedef struct {
    int valid;
    int render_asset_admission_consumed;
    int dungeon_handoff_proof_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t consumer_trace_checksum;
    uint32_t decoded_level_hash;
    uint32_t decoded_object_table_hash;
    uint32_t decoded_bitmap_hash;
    uint32_t decoded_palette_hash;
    int real_data_handoff_to_dungeon;
    int dungeon_state_admission_allowed;
    int object_table_layout_admission_allowed;
    int bitmap_palette_decode_admission_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02DungeonHandoffReceipt;

typedef struct {
    int valid;
    int same_capture_as_dungeon_handoff;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t consumer_trace_checksum;
    uint32_t decoded_level_hash;
    uint32_t decoded_object_table_hash;
    uint32_t decoded_bitmap_hash;
    uint32_t decoded_palette_hash;
    uint32_t original_host_route_identity_checksum;
    int original_host_route_bound;
    int level_grid_runtime_consumer_bound;
    int object_table_runtime_consumer_bound;
    int bitmap_palette_runtime_consumer_bound;
    int host_surface_upload_proven;
    int host_capture_frame_proven;
    int synthetic_host_frame_promoted;
    int synthetic_level_grid_promoted;
    int synthetic_object_table_promoted;
    int synthetic_bitmap_palette_promoted;
    int fallback_visuals_observed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02HostDungeonConsumerProof;

typedef struct {
    int valid;
    int dungeon_handoff_consumed;
    int host_consumer_proof_consumed;
    int runtime_capture_required;
    Theron_Track02Variant variant;
    char track02_md5[33];
    uint32_t record;
    uint32_t level_route_hash;
    uint32_t object_table_route_hash;
    uint32_t all_dungeon_route_hash;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    uint32_t consumer_trace_checksum;
    uint32_t decoded_level_hash;
    uint32_t decoded_object_table_hash;
    uint32_t decoded_bitmap_hash;
    uint32_t decoded_palette_hash;
    uint32_t original_host_route_identity_checksum;
    int real_track02_dungeon_consumer_ready;
    int host_surface_upload_allowed;
    int host_capture_frame_required;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1RuntimeTrack02HostDungeonConsumerReceipt;

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

void theron_v1_runtime_track02_capture_consumer_gap_init(
    Theron_V1RuntimeTrack02CaptureConsumerGapReceipt *out);

void theron_v1_runtime_track02_consumer_semantic_init(
    Theron_V1RuntimeTrack02ConsumerSemanticReceipt *out);

void theron_v1_runtime_track02_render_asset_admission_init(
    Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt *out);

void theron_v1_runtime_track02_original_data_binding_gap_init(
    Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *out);

void theron_v1_runtime_track02_render_asset_proof_init(
    Theron_V1RuntimeTrack02RenderAssetProof *out);

void theron_v1_runtime_track02_dungeon_handoff_init(
    Theron_V1RuntimeTrack02DungeonHandoffReceipt *out);

void theron_v1_runtime_track02_host_dungeon_consumer_init(
    Theron_V1RuntimeTrack02HostDungeonConsumerReceipt *out);

void theron_v1_runtime_track02_host_dungeon_consumer_proof_init(
    Theron_V1RuntimeTrack02HostDungeonConsumerProof *out);

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

int theron_v1_runtime_bind_track02_capture_consumer_gap(
    const Theron_V1RuntimeStartupLevelAnchorReceipt *startup_anchor,
    const Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt *level_evidence,
    const Theron_V1RuntimeObjectTableRouteEvidenceReceipt *object_evidence,
    Theron_V1RuntimeTrack02CaptureConsumerGapReceipt *out);

int theron_v1_runtime_bind_track02_consumer_semantics(
    const Theron_V1RuntimeTrack02CaptureConsumerGapReceipt *gap,
    const Theron_V1Track02Post3800ConsumerSemanticReceipt *consumer,
    Theron_V1RuntimeTrack02ConsumerSemanticReceipt *out);

int theron_v1_runtime_track02_render_asset_proof_from_decoded_routes(
    const Theron_V1RuntimeTrack02ConsumerSemanticReceipt *consumer,
    const Theron_Track02LevelRouteReceipt *level_route,
    const Theron_Track02ObjectTableRouteReceipt *object_route,
    const Theron_Track02StartupBitmapAtlas *bitmap_atlas,
    const Theron_Track02PaletteWindowEvidence *palette_window,
    Theron_V1RuntimeTrack02RenderAssetProof *out);

int theron_v1_runtime_track02_render_asset_proof_from_track02_capture(
    const Theron_V1RuntimeTrack02ConsumerSemanticReceipt *consumer,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    size_t palette_raw_offset,
    /* Compatibility input only. Non-zero is rejected: a boolean cannot
     * replace an authenticated original consumer/capture receipt. */
    int palette_semantic_binding_verified,
    Theron_V1RuntimeTrack02RenderAssetProof *out);

int theron_v1_runtime_track02_capture_original_data_binding_gap(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    size_t palette_raw_offset,
    Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *out);

int theron_v1_runtime_bind_track02_original_consumer_trace(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1Track02Post3800ConsumerSemanticReceipt *consumer,
    size_t palette_raw_offset,
    size_t nonstartup_level_raw_offset,
    size_t object_table_raw_offset,
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt *out);

int theron_v1_runtime_bind_track02_original_object_dungeon_consumer_trace(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1Track02ObjectDungeonConsumerGrammarReceipt *grammar,
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt *out);

int theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt *binding,
    Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt *out);

int theron_v1_runtime_bind_track02_object_level_admission(
    const Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt *handoff,
    const Theron_V1Track02ObjectDungeonConsumerGrammarReceipt *grammar,
    Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt *out);

int theron_v1_runtime_bind_track02_nonstartup_level_record_evidence(
    const Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt *admission,
    const char *capture_trace,
    Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt *out);

int theron_v1_runtime_bind_track02_object_table_route_evidence(
    const Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt *admission,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt *out);

int theron_v1_runtime_bind_track02_level_object_handoff_evidence(
    const Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt *level,
    const Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt *object,
    Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt *out);

int theron_v1_runtime_bind_track02_level_object_field_boundary(
    const Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt *handoff,
    const char *capture_trace,
    Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt *out);

int theron_v1_runtime_bind_track02_reviewed_field_decoder_boundary(
    const Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt *boundary,
    const char *reviewed_decoder_identity,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt *out);

int theron_v1_runtime_bind_track02_dungeon_route_admission_boundary(
    const Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt *boundary,
    const char *capture_trace,
    Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt *out);

int theron_v1_runtime_bind_track02_level_object_facts_handoff(
    const Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt *route,
    const Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt *boundary,
    Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt *out);

int theron_v1_runtime_bind_track02_dungeon_selection_level_record_boundary(
    const Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt *handoff,
    const char *capture_trace,
    Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt *out);

int theron_v1_runtime_bind_track02_dungeon_object_level_table_binding(
    const Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt
        *boundary,
    const char *capture_trace,
    Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt *out);

int theron_v1_runtime_bind_track02_level_object_loader_route(
    const Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt
        *binding,
    const char *capture_trace,
    Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt *out);

int theron_v1_runtime_bind_track02_object_placement_state(
    const Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt *loader,
    const Theron_Track02ObjectTable *objects,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ObjectPlacementStateReceipt *out);

int theron_v1_runtime_bind_track02_object_gameplay_semantics(
    const Theron_V1RuntimeTrack02ObjectPlacementStateReceipt *placement,
    const Theron_Track02ObjectTable *objects,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt *out);

int theron_v1_runtime_publish_track02_object_gameplay_state(
    Theron_V1_World *world,
    Theron_DungeonID dungeon_id,
    const Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt *semantics,
    const Theron_Track02ObjectTable *objects,
    Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt *out);

int theron_v1_runtime_bind_track02_level_transition_handoff(
    const Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt *source,
    const Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt *target,
    const char *capture_trace,
    Theron_V1RuntimeTrack02LevelTransitionHandoffReceipt *out);

int theron_v1_runtime_publish_track02_level_transition(
    Theron_V1_World *world,
    const Theron_V1RuntimeTrack02LevelTransitionHandoffReceipt *handoff,
    const Theron_V1_Level *target_level,
    const Theron_Track02ObjectTable *target_objects,
    const Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt
        *target_semantics,
    Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *out);

int theron_v1_runtime_bind_track02_bitmap_palette_source(
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *runtime,
    const char *capture_trace,
    Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt *out);

int theron_v1_runtime_decode_track02_bitmap_palette_vector(
    const Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt *source,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt *out);

int theron_v1_runtime_bind_track02_m11_soul_room_consumption(
    const Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt *decode,
    Theron_V1_World *world,
    int host_surface_width,
    int host_surface_height,
    int placement_x,
    int placement_y,
    int scale_x,
    int scale_y,
    Theron_V1RuntimeTrack02M11SoulRoomConsumptionReceipt *out);

int theron_v1_runtime_bind_track02_m11_level_consumption(
    const Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt *decode,
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *transition,
    Theron_V1_World *world,
    int host_surface_width,
    int host_surface_height,
    int placement_x,
    int placement_y,
    int scale_x,
    int scale_y,
    Theron_V1RuntimeTrack02M11LevelConsumptionReceipt *out);

int theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
    const Theron_V1RuntimeTrack02M11LevelConsumptionReceipt *level,
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *transition,
    const Theron_V1_World *world,
    Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt *out);

int theron_v1_runtime_bind_track02_level1_draw_blocker(
    const Theron_V1RuntimeTrack02M11LevelConsumptionReceipt *level,
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *transition,
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1_World *world,
    const Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt *draw_route,
    Theron_V1RuntimeTrack02Level1DrawBlockerReceipt *out);

int theron_v1_runtime_track02_original_consumer_trace_facts_from_capture(
    const char *capture_trace,
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    uint32_t record,
    uint32_t payload_checksum,
    uint32_t level_envelope_checksum,
    uint32_t post_envelope_checksum,
    Theron_V1Track02Post3800ConsumerTraceFacts *out);

int theron_v1_runtime_track02_object_dungeon_trace_facts_from_capture(
    const char *capture_trace,
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    uint32_t record,
    uint32_t payload_checksum,
    uint32_t level_envelope_checksum,
    uint32_t post_envelope_checksum,
    Theron_V1Track02Post3800ConsumerTraceFacts *out);

int theron_v1_runtime_bind_track02_render_asset_admission(
    const Theron_V1RuntimeTrack02ConsumerSemanticReceipt *consumer,
    const Theron_V1RuntimeTrack02RenderAssetProof *proof,
    Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt *out);

int theron_v1_runtime_bind_track02_dungeon_handoff(
    const Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt *admission,
    const Theron_V1RuntimeTrack02DungeonHandoffProof *proof,
    Theron_V1RuntimeTrack02DungeonHandoffReceipt *out);

int theron_v1_runtime_track02_host_dungeon_consumer_proof_from_handoff(
    const Theron_V1RuntimeTrack02DungeonHandoffReceipt *handoff,
    const char *original_host_route_identity,
    int level_grid_runtime_consumer_bound,
    int object_table_runtime_consumer_bound,
    int bitmap_palette_runtime_consumer_bound,
    int host_surface_upload_proven,
    int host_capture_frame_proven,
    Theron_V1RuntimeTrack02HostDungeonConsumerProof *out);

int theron_v1_runtime_bind_track02_host_dungeon_consumer(
    const Theron_V1RuntimeTrack02DungeonHandoffReceipt *handoff,
    const Theron_V1RuntimeTrack02HostDungeonConsumerProof *proof,
    Theron_V1RuntimeTrack02HostDungeonConsumerReceipt *out);

#endif
