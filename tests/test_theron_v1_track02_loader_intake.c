#include <stdio.h>
#include <string.h>

#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_runtime_admission.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_loader_intake.h"

static int failures;

static uint32_t fnv1a32(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t mix_hash32(uint32_t hash, uint32_t value) {
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t loader_route_pair_hash_for_test(
    const Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt
        *binding) {
    uint32_t hash = 2166136261u;

    hash = mix_hash32(hash, binding->record);
    hash = mix_hash32(hash, binding->consumer_trace_checksum);
    hash = mix_hash32(hash, binding->selected_dungeon_index);
    hash = mix_hash32(hash, binding->level_route_hash);
    hash = mix_hash32(hash, binding->object_table_route_hash);
    hash = mix_hash32(hash, (uint32_t)binding->nonstartup_level_byte_count);
    hash = mix_hash32(hash, binding->nonstartup_level_raw_hash);
    hash = mix_hash32(
        hash, (uint32_t)binding->object_table_user_data_byte_count);
    hash = mix_hash32(hash, binding->object_table_user_data_hash);
    hash = mix_hash32(hash, binding->dungeon_record_consumer_pc);
    hash = mix_hash32(hash, binding->object_table_consumer_pc);
    return hash ? hash : 2166136261u;
}

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static Theron_V1Track02LoaderReadFacts valid_facts(void) {
    Theron_V1Track02LoaderReadFacts facts = {
        1, 1, THERON_TRACK02_VARIANT_US_BIN,
        THERON_V1_INITIAL_ENVELOPE_RECORD,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        THERON_V1_INITIAL_ENVELOPE_DESTINATION,
        THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES,
        1, 0x6e6d4d21u
    };
    return facts;
}

static Theron_V1Track02IsoLevelObjectReadFacts valid_iso_facts(
    const uint8_t *payload, size_t payload_bytes) {
    Theron_V1Track02IsoLevelObjectReadFacts facts;

    memset(&facts, 0, sizeof(facts));
    facts.authenticated_original_iso_capture = 1;
    facts.cue_declares_mode1_2048 = 1;
    facts.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    facts.track02_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    facts.destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    facts.byte_count = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    facts.complete_payload_witness_verified = 1;
    facts.complete_payload_checksum = fnv1a32(payload, payload_bytes);
    facts.level_envelope_witness_verified = 1;
    facts.level_envelope_checksum =
        fnv1a32(payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    facts.post_envelope_witness_verified = 1;
    facts.post_envelope_checksum =
        fnv1a32(payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    return facts;
}

static Theron_V1Track02Post3800ConsumerTraceFacts valid_consumer_facts(
    const Theron_V1Track02LoaderSemanticGateReceipt *gate) {
    Theron_V1Track02Post3800ConsumerTraceFacts facts;

    memset(&facts, 0, sizeof(facts));
    facts.authenticated_original_trace = 1;
    facts.post_3800_execution_observed = 1;
    facts.same_capture_as_loader_payload = 1;
    facts.track02_variant = gate->track02_variant;
    facts.record = gate->record;
    facts.loader_record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.loader_destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    facts.loader_payload_bytes = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    facts.payload_checksum = gate->payload_checksum;
    facts.level_envelope_checksum = gate->level_envelope_checksum;
    facts.post_envelope_checksum = gate->post_envelope_checksum;
    facts.consumer_trace_checksum = 0x3a17b502u;
    facts.dungeon_record_consumer_pc = 0x1f42a0u;
    facts.object_table_consumer_pc = 0x1f45c2u;
    facts.dungeon_record_payload_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.dungeon_record_byte_count = THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    facts.dungeon_record_window_checksum = gate->level_envelope_checksum;
    facts.object_table_payload_offset =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    facts.object_table_byte_count =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    facts.object_table_window_checksum = gate->post_envelope_checksum;
    facts.dungeon_record_consumer_observed = 1;
    facts.object_table_consumer_observed = 1;
    facts.bitmap_consumer_observed = 1;
    facts.palette_consumer_observed = 1;
    return facts;
}

static Theron_V1Track02Post3800ConsumerTraceFacts
valid_object_dungeon_consumer_facts(
    const Theron_V1Track02LoaderSemanticGateReceipt *gate) {
    Theron_V1Track02Post3800ConsumerTraceFacts facts;

    memset(&facts, 0, sizeof(facts));
    facts.authenticated_original_trace = 1;
    facts.post_3800_execution_observed = 1;
    facts.same_capture_as_loader_payload = 1;
    facts.track02_variant = gate->track02_variant;
    facts.record = gate->record;
    facts.loader_record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.loader_destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    facts.loader_payload_bytes = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    facts.payload_checksum = gate->payload_checksum;
    facts.level_envelope_checksum = gate->level_envelope_checksum;
    facts.post_envelope_checksum = gate->post_envelope_checksum;
    facts.consumer_trace_checksum = 0x9f234a71u;
    facts.dungeon_record_consumer_pc = 0x1f42a0u;
    facts.object_table_consumer_pc = 0x1f45c2u;
    facts.dungeon_record_payload_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.dungeon_record_byte_count = THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    facts.dungeon_record_window_checksum = gate->level_envelope_checksum;
    facts.object_table_payload_offset =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    facts.object_table_byte_count =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    facts.object_table_window_checksum = gate->post_envelope_checksum;
    facts.dungeon_record_consumer_observed = 1;
    facts.object_table_consumer_observed = 1;
    return facts;
}

static Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt
valid_original_data_gap(void) {
    Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt gap;
    size_t raw_offset =
        THERON_TRACK02_RAW_SECTOR_BYTES * 10u +
        THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x120u;
    size_t object_raw_offset =
        THERON_TRACK02_RAW_SECTOR_BYTES * 11u +
        THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x80u;

    memset(&gap, 0, sizeof(gap));
    gap.valid = 1;
    gap.verified_track02_capture_consumed = 1;
    gap.fail_closed = 1;
    gap.variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(gap.track02_md5, sizeof(gap.track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    gap.level_route_hash = 0x51627384u;
    gap.object_table_route_hash = 0xa1627384u;
    gap.nonstartup_sector_receipt_hash = 0x11112222u;
    gap.nonstartup_container_index_hash = 0x33334444u;
    gap.nonstartup_anchor_count = 1u;
    gap.nonstartup_window_count = 1u;
    gap.first_nonstartup_entry_index = 2u;
    gap.first_nonstartup_raw_offset = raw_offset;
    gap.first_nonstartup_user_data_offset =
        10u * THERON_TRACK02_RAW_USER_DATA_BYTES + 0x120u;
    gap.first_nonstartup_byte_count = 0x100u;
    gap.first_nonstartup_raw_hash = 0x6123abcdu;
    gap.indexed_container_count = 1u;
    gap.first_container_entry_index = 3u;
    gap.first_container_raw_offset = object_raw_offset;
    gap.first_container_user_data_offset =
        11u * THERON_TRACK02_RAW_USER_DATA_BYTES + 0x80u;
    gap.first_container_user_data_byte_count = 0x180u;
    gap.first_container_user_data_hash = 0x7123abcdu;
    return gap;
}

static Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt
valid_original_consumer_binding(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap) {
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt binding;

    memset(&binding, 0, sizeof(binding));
    binding.valid = 1;
    binding.original_data_gap_consumed = 1;
    binding.original_consumer_trace_consumed = 1;
    binding.same_original_capture_as_gap = 1;
    binding.variant = gap->variant;
    snprintf(binding.track02_md5, sizeof(binding.track02_md5), "%s",
             gap->track02_md5);
    binding.record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    binding.consumer_trace_checksum = 0x9876abcd;
    binding.payload_checksum = 0x12345678u;
    binding.level_envelope_checksum = 0x23456789u;
    binding.post_envelope_checksum = 0x3456789au;
    binding.loader_record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    binding.loader_destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    binding.loader_payload_bytes = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    binding.dungeon_record_consumer_pc = 0x1f42a0u;
    binding.object_table_consumer_pc = 0x1f45c2u;
    binding.dungeon_record_payload_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    binding.dungeon_record_byte_count =
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    binding.dungeon_record_window_checksum =
        binding.level_envelope_checksum;
    binding.object_table_payload_offset =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    binding.object_table_byte_count =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    binding.object_table_window_checksum = binding.post_envelope_checksum;
    binding.nonstartup_level_raw_offset = gap->first_nonstartup_raw_offset;
    binding.nonstartup_level_user_data_offset =
        gap->first_nonstartup_user_data_offset;
    binding.nonstartup_level_raw_hash = gap->first_nonstartup_raw_hash;
    binding.object_table_raw_offset = gap->first_container_raw_offset;
    binding.object_table_user_data_offset =
        gap->first_container_user_data_offset;
    binding.object_table_user_data_hash = gap->first_container_user_data_hash;
    binding.level_route_hash = gap->level_route_hash;
    binding.object_table_route_hash = gap->object_table_route_hash;
    binding.palette_consumer_bound = 1;
    binding.nonstartup_level_consumer_bound = 1;
    binding.object_table_consumer_bound = 1;
    binding.bitmap_consumer_bound = 1;
    binding.runtime_consumer_binding_ready = 1;
    return binding;
}

int main(void) {
    Theron_V1Track02LoaderReadFacts facts = valid_facts();
    Theron_V1Track02IsoLevelObjectReadFacts iso_facts;
    Theron_V1Track02Post3800ConsumerTraceFacts consumer_facts;
    Theron_V1Track02Post3800ConsumerTraceFacts captured_consumer_facts;
    Theron_V1Track02LoaderIntakeReceipt receipt;
    Theron_V1Track02LoaderPayloadReceipt payload_receipt;
    Theron_V1Track02LoaderLevelEnvelopeReceipt level_envelope_receipt;
    Theron_V1Track02LoaderPostEnvelopeReceipt post_envelope_receipt;
    Theron_V1Track02LoaderSemanticGateReceipt semantic_gate;
    Theron_V1Track02Post3800ConsumerSemanticReceipt consumer_semantic;
    Theron_V1Track02ObjectDungeonConsumerGrammarReceipt grammar_receipt;
    Theron_V1Track02IsoLevelObjectReceipt iso_receipt;
    Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt data_gap;
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt consumer_binding;
    Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt
        raw_dungeon_handoff;
    Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt object_level_admission;
    Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt
        nonstartup_level_record;
    Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt object_table_route;
    Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt
        level_object_handoff;
    Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt
        level_object_field_boundary;
    Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt
        reviewed_field_decoder_boundary;
    Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt
        dungeon_route_boundary;
    Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt
        level_object_facts_handoff;
    Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt
        dungeon_selection_boundary;
    Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt
        dungeon_object_level_table_binding;
    Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt
        level_object_loader_route;
    char object_dungeon_trace[2048];
    char nonstartup_level_trace[1536];
    char object_table_trace[1536];
    char field_boundary_trace[1536];
    char reviewed_decoder_trace[1536];
    char dungeon_route_trace[1536];
    char dungeon_selection_trace[1536];
    char dungeon_table_binding_trace[1536];
    char level_object_loader_route_trace[1536];
    uint8_t payload[THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES];
    uint8_t raw_track02[THERON_TRACK02_RAW_SECTOR_BYTES * 2u];
    uint8_t iso_track02[THERON_TRACK02_RAW_USER_DATA_BYTES * 2u];
    uint32_t source_record = 0u;
    uint8_t source_byte = 0u;
    uint32_t loader_route_pair_hash = 0u;
    size_t i;

    for (i = 0u; i < sizeof(payload); ++i) {
        payload[i] = (uint8_t)(i * 37u + 11u);
    }
    facts.complete_payload_checksum = fnv1a32(payload, sizeof(payload));

    memset(raw_track02, 0, sizeof(raw_track02));
    memset(iso_track02, 0, sizeof(iso_track02));
    raw_track02[THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET + 7u] = 0x5au;
    raw_track02[THERON_TRACK02_RAW_SECTOR_BYTES + 3u] = 0xa5u;
    iso_track02[THERON_TRACK02_RAW_USER_DATA_BYTES + 7u] = 0x6bu;

    CHECK(theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        raw_track02, sizeof(raw_track02), THERON_TRACK02_MD5_US_BIN, 3010u,
        THERON_TRACK02_RAW_USER_DATA_OFFSET + 7u, &source_record,
        &source_byte));
    CHECK(source_record == 1u && source_byte == 0x5au);
    CHECK(!theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        raw_track02, sizeof(raw_track02), THERON_TRACK02_MD5_US_BIN, 3010u,
        3u, &source_record, &source_byte));
    CHECK(theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        iso_track02, sizeof(iso_track02), THERON_TRACK02_MD5_US_ISO, 3010u,
        7u, &source_record, &source_byte));
    CHECK(source_record == 1u && source_byte == 0x6bu);
    CHECK(!theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        iso_track02, sizeof(iso_track02), THERON_TRACK02_MD5_US_ISO, 3010u,
        THERON_TRACK02_RAW_USER_DATA_BYTES, &source_record, &source_byte));
    CHECK(!theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        raw_track02, sizeof(raw_track02), "00000000000000000000000000000000",
        3010u, THERON_TRACK02_RAW_USER_DATA_OFFSET + 7u, &source_record,
        &source_byte));

    CHECK(theron_v1_track02_loader_intake_observe(&facts, &receipt));
    CHECK(receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.track02_variant == THERON_TRACK02_VARIANT_US_BIN);
    CHECK(receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(receipt.record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(receipt.observed_destination == facts.destination);
    CHECK(receipt.observed_byte_count == facts.byte_count);
    CHECK(receipt.observed_payload_checksum == facts.complete_payload_checksum);
    CHECK(strcmp(receipt.status,
                 "initial_envelope_loader_read_observed_media_bound_payload_blocked") == 0);
    CHECK(theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    CHECK(payload_receipt.handed_off && payload_receipt.no_fallback);
    CHECK(payload_receipt.track02_variant == receipt.track02_variant);
    CHECK(payload_receipt.record == receipt.record);
    CHECK(payload_receipt.destination == receipt.observed_destination);
    CHECK(payload_receipt.payload_bytes == sizeof(payload));
    CHECK(payload_receipt.payload_checksum == facts.complete_payload_checksum);
    CHECK(memcmp(payload_receipt.payload, payload, sizeof(payload)) == 0);
    CHECK(strcmp(payload_receipt.status,
                 "initial_envelope_payload_handoff_no_semantics") == 0);
    CHECK(theron_v1_track02_loader_intake_handoff_level_envelope(
        &payload_receipt,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
        fnv1a32(payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES),
        &level_envelope_receipt));
    CHECK(level_envelope_receipt.handed_off &&
          level_envelope_receipt.no_fallback);
    CHECK(level_envelope_receipt.track02_variant == receipt.track02_variant);
    CHECK(level_envelope_receipt.record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(level_envelope_receipt.envelope_bytes ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
        &payload_receipt,
        fnv1a32(payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES),
        &post_envelope_receipt));
    CHECK(post_envelope_receipt.handed_off && post_envelope_receipt.no_fallback);
    CHECK(post_envelope_receipt.track02_variant == receipt.track02_variant);
    CHECK(post_envelope_receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(post_envelope_receipt.record_user_data_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(post_envelope_receipt.byte_count ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(memcmp(post_envelope_receipt.bytes,
                 payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                 THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) == 0);
    CHECK(strcmp(post_envelope_receipt.status,
                 "initial_level_post_envelope_source_bytes_no_object_semantics") == 0);
    CHECK(theron_v1_track02_loader_intake_semantic_gate(
        &payload_receipt, &level_envelope_receipt, &post_envelope_receipt,
        &semantic_gate));
    CHECK(semantic_gate.valid && semantic_gate.no_fallback);
    CHECK(semantic_gate.real_payload_available);
    CHECK(semantic_gate.level_envelope_available);
    CHECK(semantic_gate.post_envelope_available);
    CHECK(semantic_gate.track02_variant == receipt.track02_variant);
    CHECK(semantic_gate.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(semantic_gate.payload_checksum == facts.complete_payload_checksum);
    CHECK(semantic_gate.level_envelope_checksum ==
          level_envelope_receipt.envelope_checksum);
    CHECK(semantic_gate.post_envelope_checksum == post_envelope_receipt.checksum);
    CHECK(!semantic_gate.dungeon_record_semantics_proven);
    CHECK(!semantic_gate.object_table_semantics_proven);
    CHECK(!semantic_gate.bitmap_route_bound);
    CHECK(!semantic_gate.palette_binding_verified);
    CHECK(!semantic_gate.rgba_output_allowed);
    CHECK(!semantic_gate.fallback_visuals_allowed);
    CHECK(strcmp(semantic_gate.status,
                 "real_loader_bytes_available_semantic_routes_blocked_no_fallback") == 0);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    CHECK(theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(grammar_receipt.valid && grammar_receipt.no_fallback);
    CHECK(grammar_receipt.original_consumer_trace_bound);
    CHECK(grammar_receipt.same_capture_as_loader_payload);
    CHECK(grammar_receipt.track02_variant == semantic_gate.track02_variant);
    CHECK(grammar_receipt.record == semantic_gate.record);
    CHECK(grammar_receipt.loader_record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(grammar_receipt.loader_destination ==
          THERON_V1_INITIAL_ENVELOPE_DESTINATION);
    CHECK(grammar_receipt.loader_payload_bytes ==
          THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES);
    CHECK(grammar_receipt.payload_checksum == semantic_gate.payload_checksum);
    CHECK(grammar_receipt.level_envelope_checksum ==
          semantic_gate.level_envelope_checksum);
    CHECK(grammar_receipt.post_envelope_checksum ==
          semantic_gate.post_envelope_checksum);
    CHECK(grammar_receipt.consumer_trace_checksum ==
          consumer_facts.consumer_trace_checksum);
    CHECK(grammar_receipt.dungeon_record_consumer_pc ==
          consumer_facts.dungeon_record_consumer_pc);
    CHECK(grammar_receipt.object_table_consumer_pc ==
          consumer_facts.object_table_consumer_pc);
    CHECK(grammar_receipt.dungeon_record_payload_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(grammar_receipt.dungeon_record_byte_count ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(grammar_receipt.dungeon_record_window_checksum ==
          semantic_gate.level_envelope_checksum);
    CHECK(grammar_receipt.object_table_payload_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(grammar_receipt.object_table_byte_count ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(grammar_receipt.object_table_window_checksum ==
          semantic_gate.post_envelope_checksum);
    CHECK(grammar_receipt.dungeon_record_grammar_proven);
    CHECK(grammar_receipt.object_table_grammar_proven);
    CHECK(grammar_receipt.dungeon_record_fields_blocked);
    CHECK(grammar_receipt.object_table_fields_blocked);
    CHECK(!grammar_receipt.bitmap_route_bound);
    CHECK(!grammar_receipt.palette_binding_verified);
    CHECK(!grammar_receipt.rgba_output_allowed);
    CHECK(!grammar_receipt.runtime_handoff_allowed);
    CHECK(!grammar_receipt.fallback_visuals_allowed);
    CHECK(strcmp(grammar_receipt.status,
                 "post_3800_object_dungeon_grammar_bound_visuals_runtime_blocked_no_fallback") == 0);
    consumer_facts.bitmap_consumer_observed = 1;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.palette_consumer_observed = 1;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.fallback_visuals_observed = 1;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.object_table_consumer_observed = 0;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.payload_checksum ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.loader_destination ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.loader_payload_bytes -= 1u;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.object_table_payload_offset ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.dungeon_record_window_checksum ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_object_dungeon_consumer_facts(&semantic_gate);
    consumer_facts.object_table_consumer_pc = 0u;
    CHECK(!theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(!grammar_receipt.valid && grammar_receipt.status == NULL);
    consumer_facts = valid_consumer_facts(&semantic_gate);
    CHECK(theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    CHECK(consumer_semantic.valid && consumer_semantic.no_fallback);
    CHECK(consumer_semantic.original_consumer_trace_bound);
    CHECK(consumer_semantic.track02_variant == semantic_gate.track02_variant);
    CHECK(consumer_semantic.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(consumer_semantic.loader_record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(consumer_semantic.loader_destination ==
          THERON_V1_INITIAL_ENVELOPE_DESTINATION);
    CHECK(consumer_semantic.loader_payload_bytes ==
          THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES);
    CHECK(consumer_semantic.payload_checksum == semantic_gate.payload_checksum);
    CHECK(consumer_semantic.level_envelope_checksum ==
          semantic_gate.level_envelope_checksum);
    CHECK(consumer_semantic.post_envelope_checksum ==
          semantic_gate.post_envelope_checksum);
    CHECK(consumer_semantic.consumer_trace_checksum ==
          consumer_facts.consumer_trace_checksum);
    CHECK(consumer_semantic.dungeon_record_semantics_proven);
    CHECK(consumer_semantic.object_table_semantics_proven);
    CHECK(consumer_semantic.bitmap_route_bound);
    CHECK(consumer_semantic.palette_binding_verified);
    CHECK(consumer_semantic.rgba_output_allowed);
    CHECK(!consumer_semantic.fallback_visuals_allowed);
    CHECK(strcmp(consumer_semantic.status,
                 "post_3800_original_consumer_semantics_bound_no_fallback") == 0);
    consumer_facts.payload_checksum ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    CHECK(!consumer_semantic.valid && consumer_semantic.status == NULL);
    consumer_facts = valid_consumer_facts(&semantic_gate);
    consumer_facts.object_table_consumer_observed = 0;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    consumer_facts = valid_consumer_facts(&semantic_gate);
    consumer_facts.object_table_window_checksum ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    consumer_facts = valid_consumer_facts(&semantic_gate);
    consumer_facts.loader_record_user_data_offset = 0u;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    consumer_facts = valid_consumer_facts(&semantic_gate);
    consumer_facts.palette_consumer_observed = 0;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    consumer_facts = valid_consumer_facts(&semantic_gate);
    consumer_facts.synthetic_bitmap_promoted = 1;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    consumer_facts = valid_consumer_facts(&semantic_gate);
    consumer_facts.fallback_visuals_observed = 1;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    consumer_facts = valid_consumer_facts(&semantic_gate);
    semantic_gate.bitmap_route_bound = 1;
    CHECK(!theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
        &semantic_gate, &consumer_facts, &consumer_semantic));
    CHECK(!consumer_semantic.valid && consumer_semantic.status == NULL);
    semantic_gate.bitmap_route_bound = 0;
    ++level_envelope_receipt.envelope[0];
    CHECK(!theron_v1_track02_loader_intake_semantic_gate(
        &payload_receipt, &level_envelope_receipt, &post_envelope_receipt,
        &semantic_gate));
    CHECK(!semantic_gate.valid && semantic_gate.status == NULL);
    --level_envelope_receipt.envelope[0];
    CHECK(theron_v1_track02_loader_intake_semantic_gate(
        &payload_receipt, &level_envelope_receipt, &post_envelope_receipt,
        &semantic_gate));
    post_envelope_receipt.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    CHECK(!theron_v1_track02_loader_intake_semantic_gate(
        &payload_receipt, &level_envelope_receipt, &post_envelope_receipt,
        &semantic_gate));
    CHECK(!semantic_gate.valid && semantic_gate.status == NULL);
    post_envelope_receipt.track02_variant = payload_receipt.track02_variant;
    CHECK(!theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
        &payload_receipt, post_envelope_receipt.checksum ^ 1u,
        &post_envelope_receipt));
    CHECK(!post_envelope_receipt.handed_off && post_envelope_receipt.status == NULL);
    ++payload[0];
    CHECK(!theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    CHECK(!payload_receipt.handed_off && payload_receipt.status == NULL);
    --payload[0];
    CHECK(!theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload) - 1u, &payload_receipt));
    CHECK(!payload_receipt.handed_off && payload_receipt.status == NULL);
    receipt.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    CHECK(!theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    CHECK(!payload_receipt.handed_off && payload_receipt.status == NULL);
    receipt.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    CHECK(theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    payload_receipt.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    CHECK(!theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
        &payload_receipt,
        fnv1a32(payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES),
        &post_envelope_receipt));
    CHECK(!post_envelope_receipt.handed_off &&
          post_envelope_receipt.status == NULL);
    CHECK(!theron_v1_track02_loader_intake_handoff_level_envelope(
        &payload_receipt,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
        fnv1a32(payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES),
        &level_envelope_receipt));
    CHECK(!level_envelope_receipt.handed_off &&
          level_envelope_receipt.status == NULL);
    payload_receipt.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    CHECK(theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    CHECK(theron_v1_track02_loader_intake_handoff_level_envelope(
        &payload_receipt,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
        fnv1a32(payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES),
        &level_envelope_receipt));
    CHECK(theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
        &payload_receipt,
        fnv1a32(payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES),
        &post_envelope_receipt));
    CHECK(theron_v1_track02_loader_intake_semantic_gate(
        &payload_receipt, &level_envelope_receipt, &post_envelope_receipt,
        &semantic_gate));

    data_gap = valid_original_data_gap();
    snprintf(object_dungeon_trace, sizeof(object_dungeon_trace),
             "theron_track02_object_dungeon_consumer_trace "
             "authenticated_original_trace=1 "
             "post_3800_execution_observed=1 "
             "same_capture_as_loader_payload=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "loader_record_user_data_offset=0x%08x "
             "loader_destination=0x%08x "
             "loader_payload_bytes=%u "
             "payload_checksum=0x%08x "
             "level_envelope_checksum=0x%08x "
             "post_envelope_checksum=0x%08x "
             "nonstartup_level_raw_offset=%zu "
             "object_table_raw_offset=%zu "
             "dungeon_record_consumer_pc=0x001f42a0 "
             "object_table_consumer_pc=0x001f45c2 "
             "dungeon_record_payload_offset=0x%zx "
             "dungeon_record_byte_count=%u "
             "dungeon_record_window_checksum=0x%08x "
             "object_table_payload_offset=0x%zx "
             "object_table_byte_count=%u "
             "object_table_window_checksum=0x%08x "
             "dungeon_record_consumer_observed=1 "
             "object_table_consumer_observed=1 "
             "bitmap_consumer_observed=0 "
             "palette_consumer_observed=0 "
             "synthetic_dungeon_promoted=0 "
             "synthetic_object_table_promoted=0 "
             "synthetic_bitmap_promoted=0 "
             "synthetic_palette_promoted=0 "
             "fallback_visuals_observed=0 "
             "fallback_visuals_allowed=0",
             THERON_V1_INITIAL_ENVELOPE_RECORD,
             THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
             THERON_V1_INITIAL_ENVELOPE_DESTINATION,
             THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES,
             semantic_gate.payload_checksum,
             semantic_gate.level_envelope_checksum,
             semantic_gate.post_envelope_checksum,
             data_gap.first_nonstartup_raw_offset,
             data_gap.first_container_raw_offset,
             (size_t)THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
             THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
             semantic_gate.level_envelope_checksum,
             (size_t)THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
             THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES,
             semantic_gate.post_envelope_checksum);
    CHECK(theron_v1_runtime_track02_object_dungeon_trace_facts_from_capture(
        object_dungeon_trace, &data_gap, THERON_V1_INITIAL_ENVELOPE_RECORD,
        semantic_gate.payload_checksum, semantic_gate.level_envelope_checksum,
        semantic_gate.post_envelope_checksum, &captured_consumer_facts));
    CHECK(captured_consumer_facts.dungeon_record_consumer_pc == 0x1f42a0u);
    CHECK(captured_consumer_facts.object_table_consumer_pc == 0x1f45c2u);
    CHECK(captured_consumer_facts.dungeon_record_payload_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(captured_consumer_facts.dungeon_record_byte_count ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(captured_consumer_facts.object_table_payload_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(captured_consumer_facts.object_table_byte_count ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(!captured_consumer_facts.bitmap_consumer_observed);
    CHECK(!captured_consumer_facts.palette_consumer_observed);
    snprintf(object_dungeon_trace, sizeof(object_dungeon_trace),
             "theron_track02_object_dungeon_consumer_trace "
             "authenticated_original_trace=1 "
             "post_3800_execution_observed=1 "
             "same_capture_as_loader_payload=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "loader_record_user_data_offset=0x%08x "
             "loader_destination=0x%08x "
             "loader_payload_bytes=%u "
             "payload_checksum=0x%08x "
             "level_envelope_checksum=0x%08x "
             "post_envelope_checksum=0x%08x "
             "nonstartup_level_raw_offset=%zu "
             "object_table_raw_offset=%zu "
             "dungeon_record_consumer_pc=0x001f42a0 "
             "object_table_consumer_pc=0x001f45c2 "
             "dungeon_record_payload_offset=0x%zx "
             "dungeon_record_byte_count=%u "
             "dungeon_record_window_checksum=0x%08x "
             "object_table_payload_offset=0x%zx "
             "object_table_byte_count=%u "
             "object_table_window_checksum=0x%08x "
             "dungeon_record_consumer_observed=1 "
             "object_table_consumer_observed=1 "
             "bitmap_consumer_observed=1 "
             "palette_consumer_observed=0 "
             "synthetic_dungeon_promoted=0 "
             "synthetic_object_table_promoted=0 "
             "synthetic_bitmap_promoted=0 "
             "synthetic_palette_promoted=0 "
             "fallback_visuals_observed=0 "
             "fallback_visuals_allowed=0",
             THERON_V1_INITIAL_ENVELOPE_RECORD,
             THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
             THERON_V1_INITIAL_ENVELOPE_DESTINATION,
             THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES,
             semantic_gate.payload_checksum,
             semantic_gate.level_envelope_checksum,
             semantic_gate.post_envelope_checksum,
             data_gap.first_nonstartup_raw_offset,
             data_gap.first_container_raw_offset,
             (size_t)THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
             THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
             semantic_gate.level_envelope_checksum,
             (size_t)THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
             THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES,
             semantic_gate.post_envelope_checksum);
    CHECK(!theron_v1_runtime_track02_object_dungeon_trace_facts_from_capture(
        object_dungeon_trace, &data_gap, THERON_V1_INITIAL_ENVELOPE_RECORD,
        semantic_gate.payload_checksum, semantic_gate.level_envelope_checksum,
        semantic_gate.post_envelope_checksum, &consumer_facts));
    consumer_facts = captured_consumer_facts;
    CHECK(theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &semantic_gate, &consumer_facts, &grammar_receipt));
    CHECK(theron_v1_runtime_bind_track02_original_object_dungeon_consumer_trace(
        &data_gap, &grammar_receipt, &consumer_binding));
    CHECK(consumer_binding.valid);
    CHECK(consumer_binding.original_data_gap_consumed);
    CHECK(consumer_binding.original_consumer_trace_consumed);
    CHECK(consumer_binding.same_original_capture_as_gap);
    CHECK(!consumer_binding.fail_closed_until_consumer_proven);
    CHECK(consumer_binding.consumer_trace_checksum ==
          grammar_receipt.consumer_trace_checksum);
    CHECK(consumer_binding.payload_checksum ==
          grammar_receipt.payload_checksum);
    CHECK(consumer_binding.level_envelope_checksum ==
          grammar_receipt.level_envelope_checksum);
    CHECK(consumer_binding.post_envelope_checksum ==
          grammar_receipt.post_envelope_checksum);
    CHECK(consumer_binding.loader_record_user_data_offset ==
          grammar_receipt.loader_record_user_data_offset);
    CHECK(consumer_binding.loader_destination ==
          grammar_receipt.loader_destination);
    CHECK(consumer_binding.loader_payload_bytes ==
          grammar_receipt.loader_payload_bytes);
    CHECK(consumer_binding.dungeon_record_consumer_pc ==
          grammar_receipt.dungeon_record_consumer_pc);
    CHECK(consumer_binding.object_table_consumer_pc ==
          grammar_receipt.object_table_consumer_pc);
    CHECK(consumer_binding.dungeon_record_payload_offset ==
          grammar_receipt.dungeon_record_payload_offset);
    CHECK(consumer_binding.dungeon_record_byte_count ==
          grammar_receipt.dungeon_record_byte_count);
    CHECK(consumer_binding.dungeon_record_window_checksum ==
          grammar_receipt.dungeon_record_window_checksum);
    CHECK(consumer_binding.object_table_payload_offset ==
          grammar_receipt.object_table_payload_offset);
    CHECK(consumer_binding.object_table_byte_count ==
          grammar_receipt.object_table_byte_count);
    CHECK(consumer_binding.object_table_window_checksum ==
          grammar_receipt.object_table_window_checksum);
    CHECK(consumer_binding.nonstartup_level_raw_offset ==
          data_gap.first_nonstartup_raw_offset);
    CHECK(consumer_binding.object_table_raw_offset ==
          data_gap.first_container_raw_offset);
    CHECK(!consumer_binding.palette_consumer_bound);
    CHECK(consumer_binding.nonstartup_level_consumer_bound);
    CHECK(consumer_binding.object_table_consumer_bound);
    CHECK(!consumer_binding.bitmap_consumer_bound);
    CHECK(consumer_binding.runtime_consumer_binding_ready);
    grammar_receipt.bitmap_route_bound = 1;
    CHECK(!theron_v1_runtime_bind_track02_original_object_dungeon_consumer_trace(
        &data_gap, &grammar_receipt, &consumer_binding));
    CHECK(!consumer_binding.valid);
    grammar_receipt.bitmap_route_bound = 0;
    CHECK(theron_v1_runtime_bind_track02_original_object_dungeon_consumer_trace(
        &data_gap, &grammar_receipt, &consumer_binding));
    CHECK(theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
        &data_gap, &consumer_binding, &raw_dungeon_handoff));
    CHECK(raw_dungeon_handoff.valid);
    CHECK(raw_dungeon_handoff.original_data_gap_consumed);
    CHECK(raw_dungeon_handoff.original_consumer_binding_consumed);
    CHECK(raw_dungeon_handoff.same_original_capture_as_gap);
    CHECK(raw_dungeon_handoff.variant == THERON_TRACK02_VARIANT_US_BIN);
    CHECK(strcmp(raw_dungeon_handoff.track02_md5,
                 THERON_TRACK02_MD5_US_BIN) == 0);
    CHECK(raw_dungeon_handoff.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(raw_dungeon_handoff.loader_record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(raw_dungeon_handoff.loader_destination ==
          THERON_V1_INITIAL_ENVELOPE_DESTINATION);
    CHECK(raw_dungeon_handoff.loader_payload_bytes ==
          THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES);
    CHECK(raw_dungeon_handoff.dungeon_record_consumer_pc ==
          grammar_receipt.dungeon_record_consumer_pc);
    CHECK(raw_dungeon_handoff.object_table_consumer_pc ==
          grammar_receipt.object_table_consumer_pc);
    CHECK(raw_dungeon_handoff.dungeon_record_payload_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(raw_dungeon_handoff.dungeon_record_byte_count ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(raw_dungeon_handoff.dungeon_record_window_checksum ==
          grammar_receipt.dungeon_record_window_checksum);
    CHECK(raw_dungeon_handoff.object_table_payload_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(raw_dungeon_handoff.object_table_byte_count ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(raw_dungeon_handoff.object_table_window_checksum ==
          grammar_receipt.object_table_window_checksum);
    CHECK(raw_dungeon_handoff.nonstartup_level_raw_sector == 10u);
    CHECK(raw_dungeon_handoff.nonstartup_level_raw_sector_user_data_offset ==
          THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x120u);
    CHECK(raw_dungeon_handoff.nonstartup_level_user_data_offset ==
          data_gap.first_nonstartup_user_data_offset);
    CHECK(raw_dungeon_handoff.nonstartup_level_byte_count ==
          data_gap.first_nonstartup_byte_count);
    CHECK(raw_dungeon_handoff.nonstartup_level_raw_hash ==
          data_gap.first_nonstartup_raw_hash);
    CHECK(raw_dungeon_handoff.object_table_raw_sector == 11u);
    CHECK(raw_dungeon_handoff.object_table_raw_sector_user_data_offset ==
          THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x80u);
    CHECK(raw_dungeon_handoff.object_table_user_data_offset ==
          data_gap.first_container_user_data_offset);
    CHECK(raw_dungeon_handoff.object_table_user_data_byte_count ==
          data_gap.first_container_user_data_byte_count);
    CHECK(raw_dungeon_handoff.object_table_user_data_hash ==
          data_gap.first_container_user_data_hash);
    CHECK(raw_dungeon_handoff.raw_sector_user_data_bound);
    CHECK(raw_dungeon_handoff.nonstartup_dungeon_path_ready);
    CHECK(raw_dungeon_handoff.exact_level_fields_blocked);
    CHECK(raw_dungeon_handoff.exact_object_fields_blocked);
    CHECK(!raw_dungeon_handoff.bitmap_route_bound);
    CHECK(!raw_dungeon_handoff.palette_binding_verified);
    CHECK(!raw_dungeon_handoff.rgba_output_allowed);
    CHECK(!raw_dungeon_handoff.dungeon_draw_allowed);
    CHECK(!raw_dungeon_handoff.fallback_visuals_allowed);
    CHECK(theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    CHECK(object_level_admission.valid);
    CHECK(object_level_admission.raw_nonstartup_dungeon_handoff_consumed);
    CHECK(object_level_admission.object_dungeon_grammar_consumed);
    CHECK(object_level_admission.variant == THERON_TRACK02_VARIANT_US_BIN);
    CHECK(strcmp(object_level_admission.track02_md5,
                 THERON_TRACK02_MD5_US_BIN) == 0);
    CHECK(object_level_admission.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(object_level_admission.consumer_trace_checksum ==
          grammar_receipt.consumer_trace_checksum);
    CHECK(object_level_admission.payload_checksum ==
          grammar_receipt.payload_checksum);
    CHECK(object_level_admission.level_envelope_checksum ==
          grammar_receipt.level_envelope_checksum);
    CHECK(object_level_admission.post_envelope_checksum ==
          grammar_receipt.post_envelope_checksum);
    CHECK(object_level_admission.loader_record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(object_level_admission.loader_destination ==
          THERON_V1_INITIAL_ENVELOPE_DESTINATION);
    CHECK(object_level_admission.loader_payload_bytes ==
          THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES);
    CHECK(object_level_admission.dungeon_record_payload_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(object_level_admission.dungeon_record_byte_count ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(object_level_admission.dungeon_record_window_checksum ==
          grammar_receipt.dungeon_record_window_checksum);
    CHECK(object_level_admission.object_table_payload_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(object_level_admission.object_table_byte_count ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(object_level_admission.object_table_window_checksum ==
          grammar_receipt.object_table_window_checksum);
    CHECK(object_level_admission.nonstartup_level_raw_offset ==
          raw_dungeon_handoff.nonstartup_level_raw_offset);
    CHECK(object_level_admission.nonstartup_level_raw_sector == 10u);
    CHECK(object_level_admission.nonstartup_level_raw_sector_user_data_offset ==
          THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x120u);
    CHECK(object_level_admission.nonstartup_level_user_data_offset ==
          data_gap.first_nonstartup_user_data_offset);
    CHECK(object_level_admission.nonstartup_level_byte_count ==
          data_gap.first_nonstartup_byte_count);
    CHECK(object_level_admission.nonstartup_level_raw_hash ==
          data_gap.first_nonstartup_raw_hash);
    CHECK(object_level_admission.object_table_raw_offset ==
          raw_dungeon_handoff.object_table_raw_offset);
    CHECK(object_level_admission.object_table_raw_sector == 11u);
    CHECK(object_level_admission.object_table_raw_sector_user_data_offset ==
          THERON_TRACK02_RAW_USER_DATA_OFFSET + 0x80u);
    CHECK(object_level_admission.object_table_user_data_offset ==
          data_gap.first_container_user_data_offset);
    CHECK(object_level_admission.object_table_user_data_byte_count ==
          data_gap.first_container_user_data_byte_count);
    CHECK(object_level_admission.object_table_user_data_hash ==
          data_gap.first_container_user_data_hash);
    CHECK(object_level_admission.raw_sector_user_data_bound);
    CHECK(object_level_admission.dungeon_record_grammar_proven);
    CHECK(object_level_admission.object_table_grammar_proven);
    CHECK(object_level_admission.nonstartup_level_admission_allowed);
    CHECK(object_level_admission.object_table_admission_allowed);
    CHECK(object_level_admission.exact_level_fields_blocked);
    CHECK(object_level_admission.exact_object_fields_blocked);
    CHECK(!object_level_admission.bitmap_route_bound);
    CHECK(!object_level_admission.palette_binding_verified);
    CHECK(!object_level_admission.rgba_output_allowed);
    CHECK(!object_level_admission.dungeon_draw_allowed);
    CHECK(!object_level_admission.fallback_visuals_allowed);
    snprintf(nonstartup_level_trace, sizeof(nonstartup_level_trace),
             "theron_track02_nonstartup_level_record_trace "
             "same_capture_as_object_level_admission=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "nonstartup_level_raw_offset=%zu "
             "nonstartup_level_user_data_offset=%zu "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "dungeon_record_payload_offset=0x%zx "
             "dungeon_record_byte_count=%zu "
             "dungeon_record_window_checksum=0x%08x "
             "source_nonstartup_level_bytes_bound=1 "
             "nonstartup_level_record_route_observed=1 "
             "exact_level_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "bitmap_route_bound=0 "
             "palette_binding_verified=0 "
             "rgba_output_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             object_level_admission.record,
             object_level_admission.consumer_trace_checksum,
             object_level_admission.level_route_hash,
             object_level_admission.nonstartup_level_raw_offset,
             object_level_admission.nonstartup_level_user_data_offset,
             object_level_admission.nonstartup_level_byte_count,
             object_level_admission.nonstartup_level_raw_hash,
             object_level_admission.dungeon_record_consumer_pc,
             object_level_admission.dungeon_record_payload_offset,
             object_level_admission.dungeon_record_byte_count,
             object_level_admission.dungeon_record_window_checksum);
    CHECK(theron_v1_runtime_bind_track02_nonstartup_level_record_evidence(
        &object_level_admission, nonstartup_level_trace,
        &nonstartup_level_record));
    CHECK(nonstartup_level_record.valid);
    CHECK(nonstartup_level_record.object_level_admission_consumed);
    CHECK(nonstartup_level_record.same_capture_as_object_level_admission);
    CHECK(nonstartup_level_record.nonstartup_level_raw_offset ==
          object_level_admission.nonstartup_level_raw_offset);
    CHECK(nonstartup_level_record.nonstartup_level_raw_sector ==
          object_level_admission.nonstartup_level_raw_sector);
    CHECK(nonstartup_level_record.nonstartup_level_user_data_offset ==
          object_level_admission.nonstartup_level_user_data_offset);
    CHECK(nonstartup_level_record.nonstartup_level_byte_count ==
          object_level_admission.nonstartup_level_byte_count);
    CHECK(nonstartup_level_record.nonstartup_level_raw_hash ==
          object_level_admission.nonstartup_level_raw_hash);
    CHECK(nonstartup_level_record.dungeon_record_consumer_pc ==
          object_level_admission.dungeon_record_consumer_pc);
    CHECK(nonstartup_level_record.source_nonstartup_level_bytes_bound);
    CHECK(nonstartup_level_record.nonstartup_level_record_route_observed);
    CHECK(nonstartup_level_record.exact_level_fields_blocked);
    CHECK(nonstartup_level_record.object_table_layout_blocked);
    CHECK(!nonstartup_level_record.dungeon_draw_allowed);
    CHECK(!nonstartup_level_record.fallback_visuals_allowed);
    snprintf(nonstartup_level_trace, sizeof(nonstartup_level_trace),
             "theron_track02_nonstartup_level_record_trace "
             "same_capture_as_object_level_admission=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "nonstartup_level_raw_offset=%zu "
             "nonstartup_level_user_data_offset=%zu "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "dungeon_record_payload_offset=0x%zx "
             "dungeon_record_byte_count=%zu "
             "dungeon_record_window_checksum=0x%08x "
             "source_nonstartup_level_bytes_bound=1 "
             "nonstartup_level_record_route_observed=1 "
             "exact_level_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "bitmap_route_bound=0 "
             "palette_binding_verified=0 "
             "rgba_output_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=1",
             object_level_admission.record,
             object_level_admission.consumer_trace_checksum,
             object_level_admission.level_route_hash,
             object_level_admission.nonstartup_level_raw_offset,
             object_level_admission.nonstartup_level_user_data_offset,
             object_level_admission.nonstartup_level_byte_count,
             object_level_admission.nonstartup_level_raw_hash,
             object_level_admission.dungeon_record_consumer_pc,
             object_level_admission.dungeon_record_payload_offset,
             object_level_admission.dungeon_record_byte_count,
             object_level_admission.dungeon_record_window_checksum);
    CHECK(!theron_v1_runtime_bind_track02_nonstartup_level_record_evidence(
        &object_level_admission, nonstartup_level_trace,
        &nonstartup_level_record));
    CHECK(!nonstartup_level_record.valid);
    object_level_admission.nonstartup_level_raw_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_nonstartup_level_record_evidence(
        &object_level_admission, nonstartup_level_trace,
        &nonstartup_level_record));
    object_level_admission.nonstartup_level_raw_hash ^= 1u;
    snprintf(object_table_trace, sizeof(object_table_trace),
             "theron_track02_object_table_route_trace "
             "same_capture_as_object_level_admission=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "object_table_route_hash=0x%08x "
             "object_table_raw_offset=%zu "
             "object_table_user_data_offset=%zu "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "object_table_payload_offset=0x%zx "
             "object_table_byte_count=%zu "
             "object_table_window_checksum=0x%08x "
             "source_object_table_bytes_bound=1 "
             "object_table_route_observed=1 "
             "object_table_layout_blocked=1 "
             "exact_object_fields_blocked=1 "
             "bitmap_route_bound=0 "
             "palette_binding_verified=0 "
             "rgba_output_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             object_level_admission.record,
             object_level_admission.consumer_trace_checksum,
             object_level_admission.object_table_route_hash,
             object_level_admission.object_table_raw_offset,
             object_level_admission.object_table_user_data_offset,
             object_level_admission.object_table_user_data_byte_count,
             object_level_admission.object_table_user_data_hash,
             object_level_admission.object_table_consumer_pc,
             object_level_admission.object_table_payload_offset,
             object_level_admission.object_table_byte_count,
             object_level_admission.object_table_window_checksum);
    CHECK(theron_v1_runtime_bind_track02_object_table_route_evidence(
        &object_level_admission, object_table_trace, &object_table_route));
    CHECK(object_table_route.valid);
    CHECK(object_table_route.object_level_admission_consumed);
    CHECK(object_table_route.same_capture_as_object_level_admission);
    CHECK(object_table_route.object_table_raw_offset ==
          object_level_admission.object_table_raw_offset);
    CHECK(object_table_route.object_table_raw_sector ==
          object_level_admission.object_table_raw_sector);
    CHECK(object_table_route.object_table_user_data_offset ==
          object_level_admission.object_table_user_data_offset);
    CHECK(object_table_route.object_table_user_data_byte_count ==
          object_level_admission.object_table_user_data_byte_count);
    CHECK(object_table_route.object_table_user_data_hash ==
          object_level_admission.object_table_user_data_hash);
    CHECK(object_table_route.object_table_consumer_pc ==
          object_level_admission.object_table_consumer_pc);
    CHECK(object_table_route.object_table_payload_offset ==
          object_level_admission.object_table_payload_offset);
    CHECK(object_table_route.object_table_window_checksum ==
          object_level_admission.object_table_window_checksum);
    CHECK(object_table_route.source_object_table_bytes_bound);
    CHECK(object_table_route.object_table_route_observed);
    CHECK(object_table_route.object_table_layout_blocked);
    CHECK(object_table_route.exact_object_fields_blocked);
    CHECK(!object_table_route.dungeon_draw_allowed);
    CHECK(!object_table_route.fallback_visuals_allowed);
    snprintf(object_table_trace, sizeof(object_table_trace),
             "theron_track02_object_table_route_trace "
             "same_capture_as_object_level_admission=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "object_table_route_hash=0x%08x "
             "object_table_raw_offset=%zu "
             "object_table_user_data_offset=%zu "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "object_table_payload_offset=0x%zx "
             "object_table_byte_count=%zu "
             "object_table_window_checksum=0x%08x "
             "source_object_table_bytes_bound=1 "
             "object_table_route_observed=1 "
             "object_table_layout_blocked=1 "
             "exact_object_fields_blocked=1 "
             "bitmap_route_bound=0 "
             "palette_binding_verified=0 "
             "rgba_output_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=1",
             object_level_admission.record,
             object_level_admission.consumer_trace_checksum,
             object_level_admission.object_table_route_hash,
             object_level_admission.object_table_raw_offset,
             object_level_admission.object_table_user_data_offset,
             object_level_admission.object_table_user_data_byte_count,
             object_level_admission.object_table_user_data_hash,
             object_level_admission.object_table_consumer_pc,
             object_level_admission.object_table_payload_offset,
             object_level_admission.object_table_byte_count,
             object_level_admission.object_table_window_checksum);
    CHECK(!theron_v1_runtime_bind_track02_object_table_route_evidence(
        &object_level_admission, object_table_trace, &object_table_route));
    CHECK(!object_table_route.valid);
    object_level_admission.object_table_user_data_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_object_table_route_evidence(
        &object_level_admission, object_table_trace, &object_table_route));
    object_level_admission.object_table_user_data_hash ^= 1u;
    snprintf(nonstartup_level_trace, sizeof(nonstartup_level_trace),
             "theron_track02_nonstartup_level_record_trace "
             "same_capture_as_object_level_admission=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "nonstartup_level_raw_offset=%zu "
             "nonstartup_level_user_data_offset=%zu "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "dungeon_record_payload_offset=0x%zx "
             "dungeon_record_byte_count=%zu "
             "dungeon_record_window_checksum=0x%08x "
             "source_nonstartup_level_bytes_bound=1 "
             "nonstartup_level_record_route_observed=1 "
             "exact_level_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "bitmap_route_bound=0 "
             "palette_binding_verified=0 "
             "rgba_output_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             object_level_admission.record,
             object_level_admission.consumer_trace_checksum,
             object_level_admission.level_route_hash,
             object_level_admission.nonstartup_level_raw_offset,
             object_level_admission.nonstartup_level_user_data_offset,
             object_level_admission.nonstartup_level_byte_count,
             object_level_admission.nonstartup_level_raw_hash,
             object_level_admission.dungeon_record_consumer_pc,
             object_level_admission.dungeon_record_payload_offset,
             object_level_admission.dungeon_record_byte_count,
             object_level_admission.dungeon_record_window_checksum);
    CHECK(theron_v1_runtime_bind_track02_nonstartup_level_record_evidence(
        &object_level_admission, nonstartup_level_trace,
        &nonstartup_level_record));
    snprintf(object_table_trace, sizeof(object_table_trace),
             "theron_track02_object_table_route_trace "
             "same_capture_as_object_level_admission=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "object_table_route_hash=0x%08x "
             "object_table_raw_offset=%zu "
             "object_table_user_data_offset=%zu "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "object_table_payload_offset=0x%zx "
             "object_table_byte_count=%zu "
             "object_table_window_checksum=0x%08x "
             "source_object_table_bytes_bound=1 "
             "object_table_route_observed=1 "
             "object_table_layout_blocked=1 "
             "exact_object_fields_blocked=1 "
             "bitmap_route_bound=0 "
             "palette_binding_verified=0 "
             "rgba_output_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             object_level_admission.record,
             object_level_admission.consumer_trace_checksum,
             object_level_admission.object_table_route_hash,
             object_level_admission.object_table_raw_offset,
             object_level_admission.object_table_user_data_offset,
             object_level_admission.object_table_user_data_byte_count,
             object_level_admission.object_table_user_data_hash,
             object_level_admission.object_table_consumer_pc,
             object_level_admission.object_table_payload_offset,
             object_level_admission.object_table_byte_count,
             object_level_admission.object_table_window_checksum);
    CHECK(theron_v1_runtime_bind_track02_object_table_route_evidence(
        &object_level_admission, object_table_trace, &object_table_route));
    CHECK(theron_v1_runtime_bind_track02_level_object_handoff_evidence(
        &nonstartup_level_record, &object_table_route, &level_object_handoff));
    CHECK(level_object_handoff.valid);
    CHECK(level_object_handoff.nonstartup_level_record_evidence_consumed);
    CHECK(level_object_handoff.object_table_route_evidence_consumed);
    CHECK(level_object_handoff.same_capture_as_object_level_admission);
    CHECK(level_object_handoff.level_route_hash ==
          nonstartup_level_record.level_route_hash);
    CHECK(level_object_handoff.object_table_route_hash ==
          object_table_route.object_table_route_hash);
    CHECK(level_object_handoff.nonstartup_level_raw_offset ==
          nonstartup_level_record.nonstartup_level_raw_offset);
    CHECK(level_object_handoff.object_table_raw_offset ==
          object_table_route.object_table_raw_offset);
    CHECK(level_object_handoff.source_nonstartup_level_bytes_bound);
    CHECK(level_object_handoff.source_object_table_bytes_bound);
    CHECK(level_object_handoff.level_object_pair_route_observed);
    CHECK(level_object_handoff.exact_level_fields_blocked);
    CHECK(level_object_handoff.exact_object_fields_blocked);
    CHECK(level_object_handoff.object_table_layout_blocked);
    CHECK(!level_object_handoff.dungeon_draw_allowed);
    CHECK(!level_object_handoff.fallback_visuals_allowed);
    object_table_route.consumer_trace_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_level_object_handoff_evidence(
        &nonstartup_level_record, &object_table_route, &level_object_handoff));
    object_table_route.consumer_trace_checksum ^= 1u;
    object_table_route.fallback_visuals_allowed = 1;
    CHECK(!theron_v1_runtime_bind_track02_level_object_handoff_evidence(
        &nonstartup_level_record, &object_table_route, &level_object_handoff));
    object_table_route.fallback_visuals_allowed = 0;
    CHECK(theron_v1_runtime_bind_track02_level_object_handoff_evidence(
        &nonstartup_level_record, &object_table_route, &level_object_handoff));
    snprintf(field_boundary_trace, sizeof(field_boundary_trace),
             "theron_track02_level_object_field_boundary "
             "same_capture_as_level_object_handoff=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "dungeon_record_window_checksum=0x%08x "
             "object_table_window_checksum=0x%08x "
             "source_nonstartup_level_bytes_bound=1 "
             "source_object_table_bytes_bound=1 "
             "field_decoder_required=1 "
             "exact_level_fields_blocked=1 "
             "exact_object_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_handoff.record,
             level_object_handoff.consumer_trace_checksum,
             level_object_handoff.level_route_hash,
             level_object_handoff.object_table_route_hash,
             level_object_handoff.nonstartup_level_byte_count,
             level_object_handoff.nonstartup_level_raw_hash,
             level_object_handoff.object_table_user_data_byte_count,
             level_object_handoff.object_table_user_data_hash,
             level_object_handoff.dungeon_record_consumer_pc,
             level_object_handoff.object_table_consumer_pc,
             level_object_handoff.dungeon_record_window_checksum,
             level_object_handoff.object_table_window_checksum);
    CHECK(theron_v1_runtime_bind_track02_level_object_field_boundary(
        &level_object_handoff, field_boundary_trace,
        &level_object_field_boundary));
    CHECK(level_object_field_boundary.valid);
    CHECK(level_object_field_boundary.level_object_handoff_evidence_consumed);
    CHECK(level_object_field_boundary.same_capture_as_level_object_handoff);
    CHECK(level_object_field_boundary.level_route_hash ==
          level_object_handoff.level_route_hash);
    CHECK(level_object_field_boundary.object_table_route_hash ==
          level_object_handoff.object_table_route_hash);
    CHECK(level_object_field_boundary.field_decoder_required);
    CHECK(level_object_field_boundary.exact_level_fields_blocked);
    CHECK(level_object_field_boundary.exact_object_fields_blocked);
    CHECK(level_object_field_boundary.object_table_layout_blocked);
    CHECK(!level_object_field_boundary.dungeon_route_handoff_allowed);
    CHECK(!level_object_field_boundary.dungeon_draw_allowed);
    CHECK(!level_object_field_boundary.fallback_visuals_allowed);
    snprintf(field_boundary_trace, sizeof(field_boundary_trace),
             "theron_track02_level_object_field_boundary "
             "same_capture_as_level_object_handoff=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "dungeon_record_window_checksum=0x%08x "
             "object_table_window_checksum=0x%08x "
             "source_nonstartup_level_bytes_bound=1 "
             "source_object_table_bytes_bound=1 "
             "field_decoder_required=1 "
             "exact_level_fields_blocked=1 "
             "exact_object_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "dungeon_route_handoff_allowed=1 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_handoff.record,
             level_object_handoff.consumer_trace_checksum,
             level_object_handoff.level_route_hash,
             level_object_handoff.object_table_route_hash,
             level_object_handoff.nonstartup_level_byte_count,
             level_object_handoff.nonstartup_level_raw_hash,
             level_object_handoff.object_table_user_data_byte_count,
             level_object_handoff.object_table_user_data_hash,
             level_object_handoff.dungeon_record_consumer_pc,
             level_object_handoff.object_table_consumer_pc,
             level_object_handoff.dungeon_record_window_checksum,
             level_object_handoff.object_table_window_checksum);
    CHECK(!theron_v1_runtime_bind_track02_level_object_field_boundary(
        &level_object_handoff, field_boundary_trace,
        &level_object_field_boundary));
    CHECK(!level_object_field_boundary.valid);
    level_object_handoff.object_table_user_data_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_level_object_field_boundary(
        &level_object_handoff, field_boundary_trace,
        &level_object_field_boundary));
    level_object_handoff.object_table_user_data_hash ^= 1u;
    snprintf(field_boundary_trace, sizeof(field_boundary_trace),
             "theron_track02_level_object_field_boundary "
             "same_capture_as_level_object_handoff=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "dungeon_record_window_checksum=0x%08x "
             "object_table_window_checksum=0x%08x "
             "source_nonstartup_level_bytes_bound=1 "
             "source_object_table_bytes_bound=1 "
             "field_decoder_required=1 "
             "exact_level_fields_blocked=1 "
             "exact_object_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_handoff.record,
             level_object_handoff.consumer_trace_checksum,
             level_object_handoff.level_route_hash,
             level_object_handoff.object_table_route_hash,
             level_object_handoff.nonstartup_level_byte_count,
             level_object_handoff.nonstartup_level_raw_hash,
             level_object_handoff.object_table_user_data_byte_count,
             level_object_handoff.object_table_user_data_hash,
             level_object_handoff.dungeon_record_consumer_pc,
             level_object_handoff.object_table_consumer_pc,
             level_object_handoff.dungeon_record_window_checksum,
             level_object_handoff.object_table_window_checksum);
    CHECK(theron_v1_runtime_bind_track02_level_object_field_boundary(
        &level_object_handoff, field_boundary_trace,
        &level_object_field_boundary));
    snprintf(reviewed_decoder_trace, sizeof(reviewed_decoder_trace),
             "theron_track02_reviewed_field_decoder_boundary "
             "same_capture_as_field_boundary=1 "
             "reviewed_decoder_identity=theron_track02_level_object_fields_v1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "reviewed_decoder_source_bound=1 "
             "field_decoder_required=1 "
             "field_decoder_execution_allowed=0 "
             "exact_level_fields_blocked=1 "
             "exact_object_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_field_boundary.record,
             level_object_field_boundary.consumer_trace_checksum,
             level_object_field_boundary.level_route_hash,
             level_object_field_boundary.object_table_route_hash);
    CHECK(theron_v1_runtime_bind_track02_reviewed_field_decoder_boundary(
        &level_object_field_boundary,
        "theron_track02_level_object_fields_v1", reviewed_decoder_trace,
        &reviewed_field_decoder_boundary));
    CHECK(reviewed_field_decoder_boundary.valid);
    CHECK(reviewed_field_decoder_boundary.level_object_field_boundary_consumed);
    CHECK(reviewed_field_decoder_boundary.same_capture_as_field_boundary);
    CHECK(strcmp(reviewed_field_decoder_boundary.reviewed_decoder_identity,
                 "theron_track02_level_object_fields_v1") == 0);
    CHECK(reviewed_field_decoder_boundary.reviewed_decoder_source_bound);
    CHECK(reviewed_field_decoder_boundary.field_decoder_required);
    CHECK(!reviewed_field_decoder_boundary.field_decoder_execution_allowed);
    CHECK(reviewed_field_decoder_boundary.exact_level_fields_blocked);
    CHECK(reviewed_field_decoder_boundary.exact_object_fields_blocked);
    CHECK(reviewed_field_decoder_boundary.object_table_layout_blocked);
    CHECK(!reviewed_field_decoder_boundary.dungeon_route_handoff_allowed);
    CHECK(!reviewed_field_decoder_boundary.dungeon_draw_allowed);
    CHECK(!reviewed_field_decoder_boundary.fallback_visuals_allowed);
    snprintf(dungeon_route_trace, sizeof(dungeon_route_trace),
             "theron_track02_dungeon_route_admission_boundary "
             "same_capture_as_reviewed_decoder_boundary=1 "
             "reviewed_decoder_identity=theron_track02_level_object_fields_v1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "reviewed_decoder_source_bound=1 "
             "field_decoder_required=1 "
             "field_decoder_execution_allowed=0 "
             "real_track02_level_object_boundary_bound=1 "
             "dungeon_route_review_required=1 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             reviewed_field_decoder_boundary.record,
             reviewed_field_decoder_boundary.consumer_trace_checksum,
             reviewed_field_decoder_boundary.level_route_hash,
             reviewed_field_decoder_boundary.object_table_route_hash);
    CHECK(theron_v1_runtime_bind_track02_dungeon_route_admission_boundary(
        &reviewed_field_decoder_boundary, dungeon_route_trace,
        &dungeon_route_boundary));
    CHECK(dungeon_route_boundary.valid);
    CHECK(dungeon_route_boundary.reviewed_field_decoder_boundary_consumed);
    CHECK(dungeon_route_boundary.same_capture_as_reviewed_decoder_boundary);
    CHECK(dungeon_route_boundary.real_track02_level_object_boundary_bound);
    CHECK(dungeon_route_boundary.dungeon_route_review_required);
    CHECK(!dungeon_route_boundary.field_decoder_execution_allowed);
    CHECK(!dungeon_route_boundary.dungeon_route_handoff_allowed);
    CHECK(!dungeon_route_boundary.dungeon_runtime_admission_allowed);
    CHECK(!dungeon_route_boundary.dungeon_draw_allowed);
    CHECK(!dungeon_route_boundary.fallback_visuals_allowed);
    CHECK(theron_v1_runtime_bind_track02_level_object_facts_handoff(
        &dungeon_route_boundary, &level_object_field_boundary,
        &level_object_facts_handoff));
    CHECK(level_object_facts_handoff.valid);
    CHECK(level_object_facts_handoff.dungeon_route_boundary_consumed);
    CHECK(level_object_facts_handoff.level_object_field_boundary_consumed);
    CHECK(level_object_facts_handoff.same_capture_as_dungeon_route_boundary);
    CHECK(level_object_facts_handoff.level_route_hash ==
          dungeon_route_boundary.level_route_hash);
    CHECK(level_object_facts_handoff.object_table_route_hash ==
          dungeon_route_boundary.object_table_route_hash);
    CHECK(level_object_facts_handoff.nonstartup_level_byte_count ==
          level_object_field_boundary.nonstartup_level_byte_count);
    CHECK(level_object_facts_handoff.nonstartup_level_raw_hash ==
          level_object_field_boundary.nonstartup_level_raw_hash);
    CHECK(level_object_facts_handoff.object_table_user_data_byte_count ==
          level_object_field_boundary.object_table_user_data_byte_count);
    CHECK(level_object_facts_handoff.object_table_user_data_hash ==
          level_object_field_boundary.object_table_user_data_hash);
    CHECK(level_object_facts_handoff.dungeon_record_consumer_pc ==
          level_object_field_boundary.dungeon_record_consumer_pc);
    CHECK(level_object_facts_handoff.object_table_consumer_pc ==
          level_object_field_boundary.object_table_consumer_pc);
    CHECK(level_object_facts_handoff.real_track02_level_object_boundary_bound);
    CHECK(level_object_facts_handoff.field_decoder_required);
    CHECK(!level_object_facts_handoff.field_decoder_execution_allowed);
    CHECK(level_object_facts_handoff.dungeon_route_review_required);
    CHECK(!level_object_facts_handoff.dungeon_route_handoff_allowed);
    CHECK(!level_object_facts_handoff.dungeon_runtime_admission_allowed);
    CHECK(!level_object_facts_handoff.dungeon_draw_allowed);
    CHECK(!level_object_facts_handoff.fallback_visuals_allowed);
    snprintf(dungeon_selection_trace, sizeof(dungeon_selection_trace),
             "theron_track02_dungeon_selection_level_record_boundary "
             "same_capture_as_facts_handoff=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "selected_dungeon_index=1 "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "dungeon_selection_route_observed=1 "
             "level_record_route_bound=1 "
             "level_record_review_required=1 "
             "object_table_layout_blocked=1 "
             "field_decoder_execution_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_facts_handoff.record,
             level_object_facts_handoff.consumer_trace_checksum,
             level_object_facts_handoff.level_route_hash,
             level_object_facts_handoff.object_table_route_hash,
             level_object_facts_handoff.nonstartup_level_byte_count,
             level_object_facts_handoff.nonstartup_level_raw_hash,
             level_object_facts_handoff.object_table_user_data_byte_count,
             level_object_facts_handoff.object_table_user_data_hash,
             level_object_facts_handoff.dungeon_record_consumer_pc,
             level_object_facts_handoff.object_table_consumer_pc);
    CHECK(theron_v1_runtime_bind_track02_dungeon_selection_level_record_boundary(
        &level_object_facts_handoff, dungeon_selection_trace,
        &dungeon_selection_boundary));
    CHECK(dungeon_selection_boundary.valid);
    CHECK(dungeon_selection_boundary.level_object_facts_handoff_consumed);
    CHECK(dungeon_selection_boundary.same_capture_as_facts_handoff);
    CHECK(dungeon_selection_boundary.selected_dungeon_index == 1u);
    CHECK(dungeon_selection_boundary.nonstartup_level_raw_hash ==
          level_object_facts_handoff.nonstartup_level_raw_hash);
    CHECK(dungeon_selection_boundary.object_table_user_data_hash ==
          level_object_facts_handoff.object_table_user_data_hash);
    CHECK(dungeon_selection_boundary.dungeon_selection_route_observed);
    CHECK(dungeon_selection_boundary.level_record_route_bound);
    CHECK(dungeon_selection_boundary.level_record_review_required);
    CHECK(dungeon_selection_boundary.object_table_layout_blocked);
    CHECK(!dungeon_selection_boundary.field_decoder_execution_allowed);
    CHECK(!dungeon_selection_boundary.dungeon_runtime_admission_allowed);
    CHECK(!dungeon_selection_boundary.dungeon_draw_allowed);
    CHECK(!dungeon_selection_boundary.fallback_visuals_allowed);
    snprintf(dungeon_table_binding_trace, sizeof(dungeon_table_binding_trace),
             "theron_track02_dungeon_object_level_table_binding "
             "same_capture_as_dungeon_selection_boundary=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "selected_dungeon_index=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "level_record_table_route_bound=1 "
             "object_table_route_bound=1 "
             "level_object_table_pair_bound=1 "
             "level_record_review_required=1 "
             "object_table_layout_review_required=1 "
             "field_decoder_execution_allowed=0 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             dungeon_selection_boundary.record,
             dungeon_selection_boundary.consumer_trace_checksum,
             dungeon_selection_boundary.level_route_hash,
             dungeon_selection_boundary.object_table_route_hash,
             dungeon_selection_boundary.selected_dungeon_index,
             dungeon_selection_boundary.nonstartup_level_byte_count,
             dungeon_selection_boundary.nonstartup_level_raw_hash,
             dungeon_selection_boundary.object_table_user_data_byte_count,
             dungeon_selection_boundary.object_table_user_data_hash,
             dungeon_selection_boundary.dungeon_record_consumer_pc,
             dungeon_selection_boundary.object_table_consumer_pc);
    CHECK(theron_v1_runtime_bind_track02_dungeon_object_level_table_binding(
        &dungeon_selection_boundary, dungeon_table_binding_trace,
        &dungeon_object_level_table_binding));
    CHECK(dungeon_object_level_table_binding.valid);
    CHECK(dungeon_object_level_table_binding
              .dungeon_selection_level_record_boundary_consumed);
    CHECK(dungeon_object_level_table_binding
              .same_capture_as_dungeon_selection_boundary);
    CHECK(dungeon_object_level_table_binding.selected_dungeon_index ==
          dungeon_selection_boundary.selected_dungeon_index);
    CHECK(dungeon_object_level_table_binding.nonstartup_level_raw_hash ==
          dungeon_selection_boundary.nonstartup_level_raw_hash);
    CHECK(dungeon_object_level_table_binding.object_table_user_data_hash ==
          dungeon_selection_boundary.object_table_user_data_hash);
    CHECK(dungeon_object_level_table_binding.level_record_table_route_bound);
    CHECK(dungeon_object_level_table_binding.object_table_route_bound);
    CHECK(dungeon_object_level_table_binding.level_object_table_pair_bound);
    CHECK(dungeon_object_level_table_binding.level_record_review_required);
    CHECK(dungeon_object_level_table_binding
              .object_table_layout_review_required);
    CHECK(!dungeon_object_level_table_binding.field_decoder_execution_allowed);
    CHECK(!dungeon_object_level_table_binding.dungeon_route_handoff_allowed);
    CHECK(!dungeon_object_level_table_binding.dungeon_runtime_admission_allowed);
    CHECK(!dungeon_object_level_table_binding.dungeon_draw_allowed);
    CHECK(!dungeon_object_level_table_binding.fallback_visuals_allowed);
    loader_route_pair_hash =
        loader_route_pair_hash_for_test(&dungeon_object_level_table_binding);
    snprintf(level_object_loader_route_trace,
             sizeof(level_object_loader_route_trace),
             "theron_track02_level_object_loader_route "
             "same_capture_as_table_binding=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "loader_route_record_bound=1 "
             "loader_route_source_windows_bound=1 "
             "level_object_table_pair_bound=1 "
             "loader_route_review_required=1 "
             "field_decoder_execution_allowed=0 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             dungeon_object_level_table_binding.record,
             dungeon_object_level_table_binding.consumer_trace_checksum,
             dungeon_object_level_table_binding.selected_dungeon_index,
             dungeon_object_level_table_binding.level_route_hash,
             dungeon_object_level_table_binding.object_table_route_hash,
             loader_route_pair_hash,
             dungeon_object_level_table_binding.nonstartup_level_byte_count,
             dungeon_object_level_table_binding.nonstartup_level_raw_hash,
             dungeon_object_level_table_binding.object_table_user_data_byte_count,
             dungeon_object_level_table_binding.object_table_user_data_hash,
             dungeon_object_level_table_binding.dungeon_record_consumer_pc,
             dungeon_object_level_table_binding.object_table_consumer_pc);
    CHECK(theron_v1_runtime_bind_track02_level_object_loader_route(
        &dungeon_object_level_table_binding, level_object_loader_route_trace,
        &level_object_loader_route));
    CHECK(level_object_loader_route.valid);
    CHECK(level_object_loader_route
              .dungeon_object_level_table_binding_consumed);
    CHECK(level_object_loader_route.same_capture_as_table_binding);
    CHECK(level_object_loader_route.selected_dungeon_index ==
          dungeon_object_level_table_binding.selected_dungeon_index);
    CHECK(level_object_loader_route.loader_route_pair_hash ==
          loader_route_pair_hash);
    CHECK(level_object_loader_route.nonstartup_level_raw_hash ==
          dungeon_object_level_table_binding.nonstartup_level_raw_hash);
    CHECK(level_object_loader_route.object_table_user_data_hash ==
          dungeon_object_level_table_binding.object_table_user_data_hash);
    CHECK(level_object_loader_route.loader_route_record_bound);
    CHECK(level_object_loader_route.loader_route_source_windows_bound);
    CHECK(level_object_loader_route.level_object_table_pair_bound);
    CHECK(level_object_loader_route.loader_route_review_required);
    CHECK(!level_object_loader_route.field_decoder_execution_allowed);
    CHECK(!level_object_loader_route.dungeon_route_handoff_allowed);
    CHECK(!level_object_loader_route.dungeon_runtime_admission_allowed);
    CHECK(!level_object_loader_route.dungeon_draw_allowed);
    CHECK(!level_object_loader_route.fallback_visuals_allowed);
    snprintf(level_object_loader_route_trace,
             sizeof(level_object_loader_route_trace),
             "theron_track02_level_object_loader_route "
             "same_capture_as_table_binding=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "loader_route_record_bound=1 "
             "loader_route_source_windows_bound=1 "
             "level_object_table_pair_bound=1 "
             "loader_route_review_required=1 "
             "field_decoder_execution_allowed=0 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             dungeon_object_level_table_binding.record,
             dungeon_object_level_table_binding.consumer_trace_checksum,
             dungeon_object_level_table_binding.selected_dungeon_index,
             dungeon_object_level_table_binding.level_route_hash,
             dungeon_object_level_table_binding.object_table_route_hash,
             loader_route_pair_hash ^ 1u,
             dungeon_object_level_table_binding.nonstartup_level_byte_count,
             dungeon_object_level_table_binding.nonstartup_level_raw_hash,
             dungeon_object_level_table_binding.object_table_user_data_byte_count,
             dungeon_object_level_table_binding.object_table_user_data_hash,
             dungeon_object_level_table_binding.dungeon_record_consumer_pc,
             dungeon_object_level_table_binding.object_table_consumer_pc);
    CHECK(!theron_v1_runtime_bind_track02_level_object_loader_route(
        &dungeon_object_level_table_binding, level_object_loader_route_trace,
        &level_object_loader_route));
    CHECK(!level_object_loader_route.valid);
    snprintf(level_object_loader_route_trace,
             sizeof(level_object_loader_route_trace),
             "theron_track02_level_object_loader_route "
             "same_capture_as_table_binding=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "loader_route_record_bound=1 "
             "loader_route_source_windows_bound=1 "
             "level_object_table_pair_bound=1 "
             "loader_route_review_required=1 "
             "field_decoder_execution_allowed=0 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_runtime_admission_allowed=1 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             dungeon_object_level_table_binding.record,
             dungeon_object_level_table_binding.consumer_trace_checksum,
             dungeon_object_level_table_binding.selected_dungeon_index,
             dungeon_object_level_table_binding.level_route_hash,
             dungeon_object_level_table_binding.object_table_route_hash,
             loader_route_pair_hash,
             dungeon_object_level_table_binding.nonstartup_level_byte_count,
             dungeon_object_level_table_binding.nonstartup_level_raw_hash,
             dungeon_object_level_table_binding.object_table_user_data_byte_count,
             dungeon_object_level_table_binding.object_table_user_data_hash,
             dungeon_object_level_table_binding.dungeon_record_consumer_pc,
             dungeon_object_level_table_binding.object_table_consumer_pc);
    CHECK(!theron_v1_runtime_bind_track02_level_object_loader_route(
        &dungeon_object_level_table_binding, level_object_loader_route_trace,
        &level_object_loader_route));
    CHECK(!level_object_loader_route.valid);
    dungeon_object_level_table_binding.level_route_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_level_object_loader_route(
        &dungeon_object_level_table_binding, level_object_loader_route_trace,
        &level_object_loader_route));
    dungeon_object_level_table_binding.level_route_hash ^= 1u;
    snprintf(dungeon_table_binding_trace, sizeof(dungeon_table_binding_trace),
             "theron_track02_dungeon_object_level_table_binding "
             "same_capture_as_dungeon_selection_boundary=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "selected_dungeon_index=0x%08x "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "level_record_table_route_bound=1 "
             "object_table_route_bound=1 "
             "level_object_table_pair_bound=1 "
             "level_record_review_required=1 "
             "object_table_layout_review_required=1 "
             "field_decoder_execution_allowed=0 "
             "dungeon_route_handoff_allowed=1 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             dungeon_selection_boundary.record,
             dungeon_selection_boundary.consumer_trace_checksum,
             dungeon_selection_boundary.level_route_hash,
             dungeon_selection_boundary.object_table_route_hash,
             dungeon_selection_boundary.selected_dungeon_index,
             dungeon_selection_boundary.nonstartup_level_byte_count,
             dungeon_selection_boundary.nonstartup_level_raw_hash,
             dungeon_selection_boundary.object_table_user_data_byte_count,
             dungeon_selection_boundary.object_table_user_data_hash,
             dungeon_selection_boundary.dungeon_record_consumer_pc,
             dungeon_selection_boundary.object_table_consumer_pc);
    CHECK(!theron_v1_runtime_bind_track02_dungeon_object_level_table_binding(
        &dungeon_selection_boundary, dungeon_table_binding_trace,
        &dungeon_object_level_table_binding));
    CHECK(!dungeon_object_level_table_binding.valid);
    dungeon_selection_boundary.object_table_user_data_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_dungeon_object_level_table_binding(
        &dungeon_selection_boundary, dungeon_table_binding_trace,
        &dungeon_object_level_table_binding));
    dungeon_selection_boundary.object_table_user_data_hash ^= 1u;
    snprintf(dungeon_selection_trace, sizeof(dungeon_selection_trace),
             "theron_track02_dungeon_selection_level_record_boundary "
             "same_capture_as_facts_handoff=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "selected_dungeon_index=1 "
             "nonstartup_level_byte_count=%zu "
             "nonstartup_level_raw_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "dungeon_record_consumer_pc=0x%08x "
             "object_table_consumer_pc=0x%08x "
             "dungeon_selection_route_observed=1 "
             "level_record_route_bound=1 "
             "level_record_review_required=1 "
             "object_table_layout_blocked=1 "
             "field_decoder_execution_allowed=0 "
             "dungeon_runtime_admission_allowed=1 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_facts_handoff.record,
             level_object_facts_handoff.consumer_trace_checksum,
             level_object_facts_handoff.level_route_hash,
             level_object_facts_handoff.object_table_route_hash,
             level_object_facts_handoff.nonstartup_level_byte_count,
             level_object_facts_handoff.nonstartup_level_raw_hash,
             level_object_facts_handoff.object_table_user_data_byte_count,
             level_object_facts_handoff.object_table_user_data_hash,
             level_object_facts_handoff.dungeon_record_consumer_pc,
             level_object_facts_handoff.object_table_consumer_pc);
    CHECK(!theron_v1_runtime_bind_track02_dungeon_selection_level_record_boundary(
        &level_object_facts_handoff, dungeon_selection_trace,
        &dungeon_selection_boundary));
    CHECK(!dungeon_selection_boundary.valid);
    level_object_facts_handoff.nonstartup_level_raw_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_dungeon_selection_level_record_boundary(
        &level_object_facts_handoff, dungeon_selection_trace,
        &dungeon_selection_boundary));
    level_object_facts_handoff.nonstartup_level_raw_hash ^= 1u;
    dungeon_route_boundary.level_route_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_level_object_facts_handoff(
        &dungeon_route_boundary, &level_object_field_boundary,
        &level_object_facts_handoff));
    dungeon_route_boundary.level_route_hash ^= 1u;
    level_object_field_boundary.dungeon_draw_allowed = 1;
    CHECK(!theron_v1_runtime_bind_track02_level_object_facts_handoff(
        &dungeon_route_boundary, &level_object_field_boundary,
        &level_object_facts_handoff));
    level_object_field_boundary.dungeon_draw_allowed = 0;
    snprintf(dungeon_route_trace, sizeof(dungeon_route_trace),
             "theron_track02_dungeon_route_admission_boundary "
             "same_capture_as_reviewed_decoder_boundary=1 "
             "reviewed_decoder_identity=theron_track02_level_object_fields_v1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "reviewed_decoder_source_bound=1 "
             "field_decoder_required=1 "
             "field_decoder_execution_allowed=0 "
             "real_track02_level_object_boundary_bound=1 "
             "dungeon_route_review_required=1 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_runtime_admission_allowed=1 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             reviewed_field_decoder_boundary.record,
             reviewed_field_decoder_boundary.consumer_trace_checksum,
             reviewed_field_decoder_boundary.level_route_hash,
             reviewed_field_decoder_boundary.object_table_route_hash);
    CHECK(!theron_v1_runtime_bind_track02_dungeon_route_admission_boundary(
        &reviewed_field_decoder_boundary, dungeon_route_trace,
        &dungeon_route_boundary));
    CHECK(!dungeon_route_boundary.valid);
    reviewed_field_decoder_boundary.fallback_visuals_allowed = 1;
    CHECK(!theron_v1_runtime_bind_track02_dungeon_route_admission_boundary(
        &reviewed_field_decoder_boundary, dungeon_route_trace,
        &dungeon_route_boundary));
    reviewed_field_decoder_boundary.fallback_visuals_allowed = 0;
    snprintf(reviewed_decoder_trace, sizeof(reviewed_decoder_trace),
             "theron_track02_reviewed_field_decoder_boundary "
             "same_capture_as_field_boundary=1 "
             "reviewed_decoder_identity=theron_track02_level_object_fields_v1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "reviewed_decoder_source_bound=1 "
             "field_decoder_required=1 "
             "field_decoder_execution_allowed=1 "
             "exact_level_fields_blocked=1 "
             "exact_object_fields_blocked=1 "
             "object_table_layout_blocked=1 "
             "dungeon_route_handoff_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_field_boundary.record,
             level_object_field_boundary.consumer_trace_checksum,
             level_object_field_boundary.level_route_hash,
             level_object_field_boundary.object_table_route_hash);
    CHECK(!theron_v1_runtime_bind_track02_reviewed_field_decoder_boundary(
        &level_object_field_boundary,
        "theron_track02_level_object_fields_v1", reviewed_decoder_trace,
        &reviewed_field_decoder_boundary));
    CHECK(!reviewed_field_decoder_boundary.valid);
    CHECK(!theron_v1_runtime_bind_track02_reviewed_field_decoder_boundary(
        &level_object_field_boundary,
        "synthetic_track02_level_object_fields", reviewed_decoder_trace,
        &reviewed_field_decoder_boundary));
    grammar_receipt.payload_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    CHECK(!object_level_admission.valid);
    grammar_receipt.payload_checksum ^= 1u;
    grammar_receipt.bitmap_route_bound = 1;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    grammar_receipt.bitmap_route_bound = 0;
    grammar_receipt.loader_destination ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    grammar_receipt.loader_destination ^= 1u;
    grammar_receipt.dungeon_record_window_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    grammar_receipt.dungeon_record_window_checksum ^= 1u;
    grammar_receipt.object_table_payload_offset ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    grammar_receipt.object_table_payload_offset ^= 1u;
    raw_dungeon_handoff.object_table_window_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    raw_dungeon_handoff.object_table_window_checksum ^= 1u;
    raw_dungeon_handoff.dungeon_draw_allowed = 1;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    raw_dungeon_handoff.dungeon_draw_allowed = 0;
    raw_dungeon_handoff.fallback_visuals_allowed = 1;
    CHECK(!theron_v1_runtime_bind_track02_object_level_admission(
        &raw_dungeon_handoff, &grammar_receipt, &object_level_admission));
    raw_dungeon_handoff.fallback_visuals_allowed = 0;
    grammar_receipt = (Theron_V1Track02ObjectDungeonConsumerGrammarReceipt){0};
    data_gap.first_nonstartup_user_data_offset ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
        &data_gap, &consumer_binding, &raw_dungeon_handoff));
    CHECK(!raw_dungeon_handoff.valid);
    data_gap = valid_original_data_gap();
    consumer_binding = valid_original_consumer_binding(&data_gap);
    consumer_binding.nonstartup_level_raw_hash ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
        &data_gap, &consumer_binding, &raw_dungeon_handoff));
    data_gap = valid_original_data_gap();
    consumer_binding = valid_original_consumer_binding(&data_gap);
    consumer_binding.fallback_visuals_allowed = 1;
    CHECK(!theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
        &data_gap, &consumer_binding, &raw_dungeon_handoff));
    data_gap = valid_original_data_gap();
    data_gap.first_container_raw_offset =
        THERON_TRACK02_RAW_SECTOR_BYTES * 11u + 3u;
    consumer_binding = valid_original_consumer_binding(&data_gap);
    CHECK(!theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
        &data_gap, &consumer_binding, &raw_dungeon_handoff));

    iso_facts = valid_iso_facts(payload, sizeof(payload));
    CHECK(theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    CHECK(iso_receipt.handed_off && iso_receipt.no_fallback);
    CHECK(iso_receipt.original_iso_capture && iso_receipt.cue_mode1_2048);
    CHECK(iso_receipt.no_raw_bin_trace_borrowing &&
          iso_receipt.no_sector_conversion &&
          iso_receipt.no_synthetic_dungeon);
    CHECK(iso_receipt.track02_variant == THERON_TRACK02_VARIANT_US_ISO);
    CHECK(iso_receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(iso_receipt.destination == THERON_V1_INITIAL_ENVELOPE_DESTINATION);
    CHECK(iso_receipt.payload_bytes == sizeof(payload));
    CHECK(iso_receipt.payload_checksum == iso_facts.complete_payload_checksum);
    CHECK(iso_receipt.level_envelope_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(iso_receipt.level_envelope_bytes ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(memcmp(iso_receipt.level_envelope,
                 payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                 THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES) == 0);
    CHECK(iso_receipt.post_envelope_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(iso_receipt.post_envelope_bytes ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(memcmp(iso_receipt.post_envelope,
                 payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                 THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) == 0);
    CHECK(strcmp(iso_receipt.status,
                 "iso_mode1_2048_initial_level_and_post_envelope_source_bytes_no_semantics") == 0);
    iso_facts.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    CHECK(!iso_receipt.handed_off && iso_receipt.status == NULL);
    iso_facts.track02_variant = THERON_TRACK02_VARIANT_JP_REV1_ISO;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.raw_bin_trace_borrowed = 1;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.sector_conversion_applied = 1;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.synthetic_dungeon_promoted = 1;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.level_envelope_checksum ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload) - 1u, &iso_receipt));

    facts.authenticated_original_trace = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.authenticated_original_trace = 1;
    facts.later_than_stage2_transfer = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.later_than_stage2_transfer = 1;
    facts.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_variant = THERON_TRACK02_VARIANT_UNKNOWN;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    facts.track02_record = 0x04e0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = 0x04dfu;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    facts.record_user_data_offset = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.destination = 0x3900u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    facts.byte_count = 1024u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.byte_count = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    facts.complete_payload_witness_verified = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.complete_payload_witness_verified = 1;
    facts.complete_payload_checksum = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.complete_payload_checksum = fnv1a32(payload, sizeof(payload));
    facts.byte_count = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));

    return failures != 0;
}
