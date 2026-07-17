#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02_level_object_trace_preparation.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

int main(void) {
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt provenance = {0};
    Theron_V1Track02LoaderSemanticGateReceipt loader_gate = {0};
    Theron_V1Track02Post3800ConsumerTraceFacts trace_facts = {0};
    Theron_V1Track02ObjectDungeonConsumerGrammarReceipt grammar = {0};
    Theron_V1Track02LevelObjectTracePreparationReceipt preparation;

    provenance.valid = 1;
    provenance.bitmap_capture_runtime_consumed = 1;
    provenance.loader_record_runtime_consumed = 1;
    provenance.same_track02_source_verified = 1;
    provenance.original_level_object_consumer_trace_required = 1;
    provenance.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(provenance.track02_md5, sizeof(provenance.track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    provenance.loader_record = 0x00000b52u;
    provenance.loader_destination = 0x3800u;
    provenance.loader_raw_user_data_offset = 0x0029f000u;
    provenance.loader_payload_bytes = 2048u;
    provenance.loader_payload_checksum = 0x7b0f13c9u;
    provenance.level_envelope_checksum = 0x3a5d7811u;
    provenance.post_envelope_checksum = 0x55aa7744u;

    loader_gate.valid = 1;
    loader_gate.no_fallback = 1;
    loader_gate.real_payload_available = 1;
    loader_gate.level_envelope_available = 1;
    loader_gate.post_envelope_available = 1;
    loader_gate.track02_variant = provenance.track02_variant;
    loader_gate.record = provenance.loader_record;
    loader_gate.payload_checksum = provenance.loader_payload_checksum;
    loader_gate.level_envelope_checksum = provenance.level_envelope_checksum;
    loader_gate.post_envelope_checksum = provenance.post_envelope_checksum;

    trace_facts.authenticated_original_trace = 1;
    trace_facts.post_3800_execution_observed = 1;
    trace_facts.same_capture_as_loader_payload = 1;
    trace_facts.track02_variant = provenance.track02_variant;
    trace_facts.record = provenance.loader_record;
    trace_facts.loader_record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    trace_facts.loader_destination = provenance.loader_destination;
    trace_facts.loader_payload_bytes = provenance.loader_payload_bytes;
    trace_facts.payload_checksum = provenance.loader_payload_checksum;
    trace_facts.level_envelope_checksum = provenance.level_envelope_checksum;
    trace_facts.post_envelope_checksum = provenance.post_envelope_checksum;
    trace_facts.consumer_trace_checksum = 0x2468ace0u;
    trace_facts.dungeon_record_consumer_pc = 0x4120u;
    trace_facts.dungeon_record_payload_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    trace_facts.dungeon_record_byte_count =
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    trace_facts.dungeon_record_window_checksum = provenance.level_envelope_checksum;
    trace_facts.object_table_consumer_pc = 0x4180u;
    trace_facts.object_table_payload_offset =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    trace_facts.object_table_byte_count =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    trace_facts.object_table_window_checksum = provenance.post_envelope_checksum;
    trace_facts.dungeon_record_consumer_observed = 1;
    trace_facts.object_table_consumer_observed = 1;
    CHECK(theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
        &loader_gate, &trace_facts, &grammar));

    CHECK(theron_v1_track02_prepare_level_object_trace_runtime(
        &provenance, &grammar, &preparation));
    CHECK(preparation.valid && preparation.exact_record_windows_verified);
    CHECK(preparation.original_consumer_trace_consumed);
    CHECK(preparation.level_field_decoder_required);
    CHECK(preparation.object_field_decoder_required);
    CHECK(!preparation.level_admission_allowed);
    CHECK(!preparation.object_admission_allowed);
    CHECK(!preparation.bitmap_palette_admission_allowed);
    CHECK(!preparation.dungeon_draw_allowed);
    CHECK(!preparation.fallback_visuals_allowed);

    grammar.object_table_payload_offset = provenance.loader_payload_bytes;
    CHECK(!theron_v1_track02_prepare_level_object_trace_runtime(
        &provenance, &grammar, &preparation));
    grammar.object_table_payload_offset =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    grammar.runtime_handoff_allowed = 1;
    CHECK(!theron_v1_track02_prepare_level_object_trace_runtime(
        &provenance, &grammar, &preparation));
    grammar.runtime_handoff_allowed = 0;
    grammar.dungeon_record_fields_blocked = 0;
    CHECK(!theron_v1_track02_prepare_level_object_trace_runtime(
        &provenance, &grammar, &preparation));

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
