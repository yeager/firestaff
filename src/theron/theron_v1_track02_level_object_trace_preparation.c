#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_level_object_trace_preparation.h"

static int theron_v1_track02_window_within_payload(size_t offset,
                                                     size_t bytes,
                                                     size_t payload_bytes) {
    return offset < payload_bytes && bytes != 0u && bytes <= payload_bytes - offset;
}

int theron_v1_track02_prepare_level_object_trace_runtime(
    const Theron_V1Track02ProvenanceRuntimeConsumerReceipt *provenance,
    const Theron_V1Track02ObjectDungeonConsumerGrammarReceipt *grammar,
    Theron_V1Track02LevelObjectTracePreparationReceipt *out) {
    Theron_V1Track02LevelObjectTracePreparationReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!provenance || !grammar || !out || !provenance->valid ||
        !provenance->bitmap_capture_runtime_consumed ||
        !provenance->loader_record_runtime_consumed ||
        !provenance->same_track02_source_verified ||
        !provenance->original_level_object_consumer_trace_required ||
        provenance->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        theron_v1_track02_variant_for_md5(provenance->track02_md5) !=
            provenance->track02_variant ||
        provenance->loader_record == 0u ||
        provenance->loader_destination == 0u ||
        provenance->loader_payload_bytes == 0u ||
        provenance->loader_payload_checksum == 0u ||
        provenance->level_envelope_checksum == 0u ||
        provenance->post_envelope_checksum == 0u ||
        provenance->level_admission_allowed ||
        provenance->object_admission_allowed ||
        provenance->pixel_decode_allowed ||
        provenance->dungeon_draw_allowed ||
        provenance->fallback_visuals_allowed ||
        !grammar->valid || !grammar->no_fallback ||
        !grammar->original_consumer_trace_bound ||
        !grammar->same_capture_as_loader_payload ||
        grammar->track02_variant != provenance->track02_variant ||
        grammar->record != provenance->loader_record ||
        grammar->loader_record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        grammar->loader_destination != provenance->loader_destination ||
        grammar->loader_payload_bytes != provenance->loader_payload_bytes ||
        grammar->payload_checksum != provenance->loader_payload_checksum ||
        grammar->level_envelope_checksum != provenance->level_envelope_checksum ||
        grammar->post_envelope_checksum != provenance->post_envelope_checksum ||
        grammar->consumer_trace_checksum == 0u ||
        grammar->dungeon_record_consumer_pc == 0u ||
        grammar->object_table_consumer_pc == 0u ||
        !theron_v1_track02_window_within_payload(
            grammar->dungeon_record_payload_offset,
            grammar->dungeon_record_byte_count,
            provenance->loader_payload_bytes) ||
        grammar->dungeon_record_window_checksum == 0u ||
        !theron_v1_track02_window_within_payload(
            grammar->object_table_payload_offset,
            grammar->object_table_byte_count,
            provenance->loader_payload_bytes) ||
        grammar->object_table_window_checksum == 0u ||
        !grammar->dungeon_record_grammar_proven ||
        !grammar->object_table_grammar_proven ||
        !grammar->dungeon_record_fields_blocked ||
        !grammar->object_table_fields_blocked ||
        grammar->bitmap_route_bound || grammar->palette_binding_verified ||
        grammar->rgba_output_allowed || grammar->runtime_handoff_allowed ||
        grammar->fallback_visuals_allowed) {
        return 0;
    }

    receipt.valid = 1;
    receipt.provenance_runtime_consumer_consumed = 1;
    receipt.original_consumer_trace_consumed = 1;
    receipt.exact_record_windows_verified = 1;
    receipt.track02_variant = provenance->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             provenance->track02_md5);
    receipt.loader_record = provenance->loader_record;
    receipt.consumer_trace_checksum = grammar->consumer_trace_checksum;
    receipt.dungeon_record_consumer_pc = grammar->dungeon_record_consumer_pc;
    receipt.dungeon_record_payload_offset = grammar->dungeon_record_payload_offset;
    receipt.dungeon_record_byte_count = grammar->dungeon_record_byte_count;
    receipt.dungeon_record_window_checksum = grammar->dungeon_record_window_checksum;
    receipt.object_table_consumer_pc = grammar->object_table_consumer_pc;
    receipt.object_table_payload_offset = grammar->object_table_payload_offset;
    receipt.object_table_byte_count = grammar->object_table_byte_count;
    receipt.object_table_window_checksum = grammar->object_table_window_checksum;
    receipt.level_field_decoder_required = 1;
    receipt.object_field_decoder_required = 1;
    receipt.level_admission_allowed = 0;
    receipt.object_admission_allowed = 0;
    receipt.bitmap_palette_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}
