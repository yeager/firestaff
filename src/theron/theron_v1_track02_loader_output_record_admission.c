#include "theron_v1_track02_loader_output_record_admission.h"

#include <stdio.h>
#include <string.h>

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24u) | ((uint32_t)bytes[1] << 16u) |
        ((uint32_t)bytes[2] << 8u) | bytes[3];
}

int theron_v1_track02_loader_output_record_admit(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const Theron_V1Track02HandoffArtifactCorpusReceipt *artifact_corpus,
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *sector_corpus,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *expected_source_trace_md5,
    Theron_V1Track02LoaderOutputRecordAdmissionReceipt *out)
{
    Theron_V1Track02LoaderOutputRecordAdmissionReceipt receipt = {0};
    const Theron_Track02InitialLevelObjectBoundaryReceipt *boundary;

    if (!out) return 0;
    *out = receipt;
    if (!handoff || !artifact_corpus || !sector_corpus || !plan ||
        !expected_source_trace_md5 || !expected_source_trace_md5[0] ||
        !theron_v1_raw_loader_trace_manifest_initial_level_handoff_is_complete(handoff) ||
        !theron_v1_track02_handoff_artifact_corpus_matches_sector_record(
            artifact_corpus, sector_corpus, plan, expected_source_trace_md5) ||
        handoff->initial_level_semantics_proven || handoff->object_tail_semantics_proven ||
        handoff->fallback_visuals_allowed ||
        handoff->observed_track02_record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        handoff->loader_payload.record != handoff->observed_track02_record ||
        handoff->loader_payload.payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        handoff->loader_payload.payload_checksum != handoff->complete_payload_checksum ||
        sector_corpus->sector_record.resolved_track02_record !=
            handoff->observed_track02_record ||
        sector_corpus->sector_record.loader_destination != handoff->loader_payload.destination ||
        sector_corpus->sector_record.loader_destination_payload_bytes !=
            handoff->loader_payload.payload_bytes ||
        sector_corpus->sector_record.loader_destination_payload_checksum !=
            handoff->loader_payload.payload_checksum) return 0;
    boundary = &handoff->initial_level_boundary;
    if (!boundary->valid || boundary->track02_record != handoff->observed_track02_record ||
        boundary->level_user_data_offset_in_record !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        boundary->level_byte_count != THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        !boundary->level_payload_hash || boundary->object_table_parsed ||
        boundary->object_table_semantics_proven || !boundary->promotion_blocked ||
        boundary->object_boundary_user_data_offset_in_record !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        boundary->following_user_data_bytes_in_record !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        !boundary->following_user_data_hash ||
        handoff->loader_level_envelope.record_user_data_offset !=
            boundary->level_user_data_offset_in_record ||
        handoff->loader_level_envelope.envelope_bytes != boundary->level_byte_count ||
        handoff->loader_level_envelope.envelope_checksum != boundary->level_payload_hash ||
        handoff->loader_post_envelope.record_user_data_offset !=
            boundary->object_boundary_user_data_offset_in_record ||
        handoff->loader_post_envelope.byte_count !=
            boundary->following_user_data_bytes_in_record ||
        handoff->loader_post_envelope.checksum != boundary->following_user_data_hash ||
        boundary->level_width == 0u || boundary->level_height == 0u ||
        boundary->level_seed == 0u ||
        read_be16(handoff->loader_level_envelope.envelope + 0u) !=
            boundary->level_width ||
        read_be16(handoff->loader_level_envelope.envelope + 2u) !=
            boundary->level_height ||
        read_be32(handoff->loader_level_envelope.envelope + 4u) !=
            boundary->level_seed ||
        read_be16(handoff->loader_level_envelope.envelope + 8u) !=
            boundary->level_index ||
        read_be16(handoff->loader_level_envelope.envelope + 10u) !=
            boundary->level_header_extension_be)
        return 0;
    receipt.valid = receipt.original_loader_output_consumed = 1;
    receipt.envelope_header_fields_proven = 1;
    receipt.level_boundary_proven = 1;
    receipt.object_continuation_boundary_proven = 1;
    receipt.no_draw_only = 1;
    receipt.track02_variant = handoff->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", handoff->track02_md5);
    receipt.record = handoff->observed_track02_record;
    receipt.destination = handoff->loader_payload.destination;
    receipt.loader_output_bytes = handoff->loader_payload.payload_bytes;
    receipt.loader_output_checksum = handoff->loader_payload.payload_checksum;
    receipt.header_width = boundary->level_width;
    receipt.header_height = boundary->level_height;
    receipt.header_seed = boundary->level_seed;
    receipt.header_level_index = boundary->level_index;
    receipt.header_extension_be = boundary->level_header_extension_be;
    receipt.level_offset = boundary->level_user_data_offset_in_record;
    receipt.level_bytes = boundary->level_byte_count;
    receipt.level_checksum = boundary->level_payload_hash;
    receipt.object_continuation_offset = boundary->object_boundary_user_data_offset_in_record;
    receipt.object_continuation_bytes = boundary->following_user_data_bytes_in_record;
    receipt.object_continuation_checksum = boundary->following_user_data_hash;
    *out = receipt;
    return 1;
}
