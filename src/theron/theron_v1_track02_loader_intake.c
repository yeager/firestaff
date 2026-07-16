#include "theron_v1_track02_loader_intake.h"

#include <string.h>

static uint32_t theron_v1_track02_loader_intake_fnv1a32(
    const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes && byte_count != 0u) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int theron_v1_track02_loader_intake_consumer_windows_match(
    const Theron_V1Track02LoaderSemanticGateReceipt *loader_gate,
    const Theron_V1Track02Post3800ConsumerTraceFacts *facts) {
    return facts->loader_record_user_data_offset ==
               THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET &&
           facts->loader_destination ==
               THERON_V1_INITIAL_ENVELOPE_DESTINATION &&
           facts->loader_payload_bytes ==
               THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES &&
           facts->dungeon_record_consumer_pc != 0u &&
           facts->object_table_consumer_pc != 0u &&
           facts->dungeon_record_payload_offset ==
               THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET &&
           facts->dungeon_record_byte_count ==
               THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES &&
           facts->dungeon_record_window_checksum ==
               loader_gate->level_envelope_checksum &&
           facts->object_table_payload_offset ==
               THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET &&
           facts->object_table_byte_count ==
               THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES &&
           facts->object_table_window_checksum ==
               loader_gate->post_envelope_checksum;
}

int theron_v1_track02_loader_intake_observe(
    const Theron_V1Track02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt) {
    Theron_V1Track02LoaderIntakeReceipt receipt = {0};

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!facts || !out_receipt || !facts->authenticated_original_trace ||
        !facts->later_than_stage2_transfer ||
        (facts->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         facts->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        facts->track02_record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        facts->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        facts->destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        facts->byte_count != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        !facts->complete_payload_witness_verified ||
        facts->complete_payload_checksum == 0u) {
        return 0;
    }

    receipt.observed = 1;
    receipt.track02_variant = facts->track02_variant;
    receipt.record = facts->track02_record;
    receipt.record_user_data_offset = facts->record_user_data_offset;
    receipt.observed_destination = facts->destination;
    receipt.observed_byte_count = facts->byte_count;
    receipt.observed_payload_checksum = facts->complete_payload_checksum;
    receipt.status =
        "initial_envelope_loader_read_observed_media_bound_payload_blocked";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_complete_payload(
    const Theron_V1Track02LoaderIntakeReceipt *intake,
    const uint8_t *payload,
    size_t payload_bytes,
    Theron_V1Track02LoaderPayloadReceipt *out_receipt) {
    Theron_V1Track02LoaderPayloadReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!intake || !payload || !out_receipt || !intake->observed ||
        intake->payload_intake_admitted ||
        (intake->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         intake->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        intake->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        intake->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        intake->observed_destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        intake->observed_byte_count != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        intake->observed_payload_checksum == 0u ||
        payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        theron_v1_track02_loader_intake_fnv1a32(payload, payload_bytes) !=
            intake->observed_payload_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.track02_variant = intake->track02_variant;
    receipt.record = intake->record;
    receipt.record_user_data_offset = intake->record_user_data_offset;
    receipt.destination = intake->observed_destination;
    receipt.payload_bytes = (uint32_t)payload_bytes;
    receipt.payload_checksum = intake->observed_payload_checksum;
    memcpy(receipt.payload, payload, payload_bytes);
    receipt.status = "initial_envelope_payload_handoff_no_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_level_envelope(
    const Theron_V1Track02LoaderPayloadReceipt *payload,
    uint32_t record_user_data_offset,
    uint32_t envelope_bytes,
    uint32_t envelope_checksum,
    Theron_V1Track02LoaderLevelEnvelopeReceipt *out_receipt) {
    Theron_V1Track02LoaderLevelEnvelopeReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!payload || !out_receipt || !payload->handed_off ||
        !payload->no_fallback ||
        (payload->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        payload->payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        record_user_data_offset != THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        envelope_bytes != THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        envelope_checksum == 0u ||
        record_user_data_offset > payload->payload_bytes ||
        envelope_bytes > payload->payload_bytes - record_user_data_offset ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload->payload + record_user_data_offset, envelope_bytes) !=
            envelope_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.track02_variant = payload->track02_variant;
    receipt.record = payload->record;
    receipt.record_user_data_offset = record_user_data_offset;
    receipt.envelope_bytes = envelope_bytes;
    receipt.envelope_checksum = envelope_checksum;
    memcpy(receipt.envelope, payload->payload + record_user_data_offset,
           envelope_bytes);
    receipt.status = "initial_level_record_slice_handoff_no_object_or_visual_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
    const Theron_V1Track02LoaderPayloadReceipt *payload,
    uint32_t post_envelope_checksum,
    Theron_V1Track02LoaderPostEnvelopeReceipt *out_receipt) {
    Theron_V1Track02LoaderPostEnvelopeReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!payload || !out_receipt || !payload->handed_off ||
        !payload->no_fallback ||
        (payload->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        payload->payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        post_envelope_checksum == 0u ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload->payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) !=
            post_envelope_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.track02_variant = payload->track02_variant;
    receipt.record = payload->record;
    receipt.record_user_data_offset =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    receipt.byte_count = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    receipt.checksum = post_envelope_checksum;
    memcpy(receipt.bytes,
           payload->payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
           sizeof(receipt.bytes));
    receipt.status =
        "initial_level_post_envelope_source_bytes_no_object_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_semantic_gate(
    const Theron_V1Track02LoaderPayloadReceipt *payload,
    const Theron_V1Track02LoaderLevelEnvelopeReceipt *level_envelope,
    const Theron_V1Track02LoaderPostEnvelopeReceipt *post_envelope,
    Theron_V1Track02LoaderSemanticGateReceipt *out_receipt) {
    Theron_V1Track02LoaderSemanticGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!payload || !level_envelope || !post_envelope || !out_receipt ||
        !payload->handed_off || !payload->no_fallback ||
        !level_envelope->handed_off || !level_envelope->no_fallback ||
        !post_envelope->handed_off || !post_envelope->no_fallback ||
        (payload->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        level_envelope->track02_variant != payload->track02_variant ||
        post_envelope->track02_variant != payload->track02_variant ||
        payload->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        level_envelope->record != payload->record ||
        post_envelope->record != payload->record ||
        payload->payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        payload->payload_checksum == 0u ||
        level_envelope->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        level_envelope->envelope_bytes !=
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        level_envelope->envelope_checksum == 0u ||
        post_envelope->record_user_data_offset !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        post_envelope->byte_count !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        post_envelope->checksum == 0u ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload->payload, payload->payload_bytes) !=
            payload->payload_checksum ||
        theron_v1_track02_loader_intake_fnv1a32(
            level_envelope->envelope, level_envelope->envelope_bytes) !=
            level_envelope->envelope_checksum ||
        theron_v1_track02_loader_intake_fnv1a32(
            post_envelope->bytes, post_envelope->byte_count) !=
            post_envelope->checksum ||
        memcmp(payload->payload + level_envelope->record_user_data_offset,
               level_envelope->envelope,
               level_envelope->envelope_bytes) != 0 ||
        memcmp(payload->payload + post_envelope->record_user_data_offset,
               post_envelope->bytes,
               post_envelope->byte_count) != 0) {
        return 0;
    }

    receipt.valid = 1;
    receipt.no_fallback = 1;
    receipt.real_payload_available = 1;
    receipt.level_envelope_available = 1;
    receipt.post_envelope_available = 1;
    receipt.track02_variant = payload->track02_variant;
    receipt.record = payload->record;
    receipt.payload_checksum = payload->payload_checksum;
    receipt.level_envelope_checksum = level_envelope->envelope_checksum;
    receipt.post_envelope_checksum = post_envelope->checksum;
    receipt.dungeon_record_semantics_proven = 0;
    receipt.object_table_semantics_proven = 0;
    receipt.bitmap_route_bound = 0;
    receipt.palette_binding_verified = 0;
    receipt.rgba_output_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    receipt.status =
        "real_loader_bytes_available_semantic_routes_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_post3800_consumer_semantic_gate(
    const Theron_V1Track02LoaderSemanticGateReceipt *loader_gate,
    const Theron_V1Track02Post3800ConsumerTraceFacts *facts,
    Theron_V1Track02Post3800ConsumerSemanticReceipt *out_receipt) {
    Theron_V1Track02Post3800ConsumerSemanticReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader_gate || !facts || !out_receipt || !loader_gate->valid ||
        !loader_gate->no_fallback ||
        !loader_gate->real_payload_available ||
        !loader_gate->level_envelope_available ||
        !loader_gate->post_envelope_available ||
        loader_gate->dungeon_record_semantics_proven ||
        loader_gate->object_table_semantics_proven ||
        loader_gate->bitmap_route_bound ||
        loader_gate->palette_binding_verified ||
        loader_gate->rgba_output_allowed ||
        loader_gate->fallback_visuals_allowed ||
        !facts->authenticated_original_trace ||
        !facts->post_3800_execution_observed ||
        !facts->same_capture_as_loader_payload ||
        (facts->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         facts->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        facts->track02_variant != loader_gate->track02_variant ||
        facts->record != loader_gate->record ||
        facts->payload_checksum == 0u ||
        facts->level_envelope_checksum == 0u ||
        facts->post_envelope_checksum == 0u ||
        facts->payload_checksum != loader_gate->payload_checksum ||
        facts->level_envelope_checksum !=
            loader_gate->level_envelope_checksum ||
        facts->post_envelope_checksum !=
            loader_gate->post_envelope_checksum ||
        facts->consumer_trace_checksum == 0u ||
        !theron_v1_track02_loader_intake_consumer_windows_match(
            loader_gate, facts) ||
        !facts->dungeon_record_consumer_observed ||
        !facts->object_table_consumer_observed ||
        !facts->bitmap_consumer_observed ||
        !facts->palette_consumer_observed ||
        facts->synthetic_dungeon_promoted ||
        facts->synthetic_object_table_promoted ||
        facts->synthetic_bitmap_promoted ||
        facts->synthetic_palette_promoted ||
        facts->fallback_visuals_observed ||
        facts->fallback_visuals_allowed) {
        return 0;
    }

    receipt.valid = 1;
    receipt.no_fallback = 1;
    receipt.original_consumer_trace_bound = 1;
    receipt.track02_variant = facts->track02_variant;
    receipt.record = facts->record;
    receipt.loader_record_user_data_offset =
        facts->loader_record_user_data_offset;
    receipt.loader_destination = facts->loader_destination;
    receipt.loader_payload_bytes = facts->loader_payload_bytes;
    receipt.payload_checksum = facts->payload_checksum;
    receipt.level_envelope_checksum = facts->level_envelope_checksum;
    receipt.post_envelope_checksum = facts->post_envelope_checksum;
    receipt.consumer_trace_checksum = facts->consumer_trace_checksum;
    receipt.dungeon_record_semantics_proven = 1;
    receipt.object_table_semantics_proven = 1;
    receipt.bitmap_route_bound = 1;
    receipt.palette_binding_verified = 1;
    receipt.rgba_output_allowed = 1;
    receipt.fallback_visuals_allowed = 0;
    receipt.status =
        "post_3800_original_consumer_semantics_bound_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
    const Theron_V1Track02LoaderSemanticGateReceipt *loader_gate,
    const Theron_V1Track02Post3800ConsumerTraceFacts *facts,
    Theron_V1Track02ObjectDungeonConsumerGrammarReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonConsumerGrammarReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader_gate || !facts || !out_receipt || !loader_gate->valid ||
        !loader_gate->no_fallback ||
        !loader_gate->real_payload_available ||
        !loader_gate->level_envelope_available ||
        !loader_gate->post_envelope_available ||
        loader_gate->dungeon_record_semantics_proven ||
        loader_gate->object_table_semantics_proven ||
        loader_gate->bitmap_route_bound ||
        loader_gate->palette_binding_verified ||
        loader_gate->rgba_output_allowed ||
        loader_gate->fallback_visuals_allowed ||
        !facts->authenticated_original_trace ||
        !facts->post_3800_execution_observed ||
        !facts->same_capture_as_loader_payload ||
        (facts->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         facts->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        facts->track02_variant != loader_gate->track02_variant ||
        facts->record != loader_gate->record ||
        facts->payload_checksum == 0u ||
        facts->level_envelope_checksum == 0u ||
        facts->post_envelope_checksum == 0u ||
        facts->payload_checksum != loader_gate->payload_checksum ||
        facts->level_envelope_checksum !=
            loader_gate->level_envelope_checksum ||
        facts->post_envelope_checksum !=
            loader_gate->post_envelope_checksum ||
        facts->consumer_trace_checksum == 0u ||
        !theron_v1_track02_loader_intake_consumer_windows_match(
            loader_gate, facts) ||
        !facts->dungeon_record_consumer_observed ||
        !facts->object_table_consumer_observed ||
        facts->bitmap_consumer_observed ||
        facts->palette_consumer_observed ||
        facts->synthetic_dungeon_promoted ||
        facts->synthetic_object_table_promoted ||
        facts->synthetic_bitmap_promoted ||
        facts->synthetic_palette_promoted ||
        facts->fallback_visuals_observed ||
        facts->fallback_visuals_allowed) {
        return 0;
    }

    receipt.valid = 1;
    receipt.no_fallback = 1;
    receipt.original_consumer_trace_bound = 1;
    receipt.same_capture_as_loader_payload = 1;
    receipt.track02_variant = facts->track02_variant;
    receipt.record = facts->record;
    receipt.loader_record_user_data_offset =
        facts->loader_record_user_data_offset;
    receipt.loader_destination = facts->loader_destination;
    receipt.loader_payload_bytes = facts->loader_payload_bytes;
    receipt.payload_checksum = facts->payload_checksum;
    receipt.level_envelope_checksum = facts->level_envelope_checksum;
    receipt.post_envelope_checksum = facts->post_envelope_checksum;
    receipt.consumer_trace_checksum = facts->consumer_trace_checksum;
    receipt.dungeon_record_consumer_pc = facts->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = facts->object_table_consumer_pc;
    receipt.dungeon_record_payload_offset =
        facts->dungeon_record_payload_offset;
    receipt.dungeon_record_byte_count = facts->dungeon_record_byte_count;
    receipt.dungeon_record_window_checksum =
        facts->dungeon_record_window_checksum;
    receipt.object_table_payload_offset = facts->object_table_payload_offset;
    receipt.object_table_byte_count = facts->object_table_byte_count;
    receipt.object_table_window_checksum =
        facts->object_table_window_checksum;
    receipt.dungeon_record_grammar_proven = 1;
    receipt.object_table_grammar_proven = 1;
    receipt.dungeon_record_fields_blocked = 1;
    receipt.object_table_fields_blocked = 1;
    receipt.bitmap_route_bound = 0;
    receipt.palette_binding_verified = 0;
    receipt.rgba_output_allowed = 0;
    receipt.runtime_handoff_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    receipt.status =
        "post_3800_object_dungeon_grammar_bound_visuals_runtime_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_iso_level_object_record(
    const Theron_V1Track02IsoLevelObjectReadFacts *facts,
    const uint8_t *payload,
    size_t payload_bytes,
    Theron_V1Track02IsoLevelObjectReceipt *out_receipt) {
    Theron_V1Track02IsoLevelObjectReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!facts || !payload || !out_receipt ||
        !facts->authenticated_original_iso_capture ||
        !facts->cue_declares_mode1_2048 ||
        facts->raw_bin_trace_borrowed ||
        facts->sector_conversion_applied ||
        facts->synthetic_dungeon_promoted ||
        facts->track02_variant != THERON_TRACK02_VARIANT_US_ISO ||
        facts->track02_record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        facts->destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        facts->byte_count != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        !facts->complete_payload_witness_verified ||
        !facts->level_envelope_witness_verified ||
        !facts->post_envelope_witness_verified ||
        facts->complete_payload_checksum == 0u ||
        facts->level_envelope_checksum == 0u ||
        facts->post_envelope_checksum == 0u ||
        payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        theron_v1_track02_loader_intake_fnv1a32(payload, payload_bytes) !=
            facts->complete_payload_checksum ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES) !=
            facts->level_envelope_checksum ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) !=
            facts->post_envelope_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.original_iso_capture = 1;
    receipt.cue_mode1_2048 = 1;
    receipt.no_raw_bin_trace_borrowing = 1;
    receipt.no_sector_conversion = 1;
    receipt.no_synthetic_dungeon = 1;
    receipt.track02_variant = facts->track02_variant;
    receipt.record = facts->track02_record;
    receipt.destination = facts->destination;
    receipt.payload_bytes = facts->byte_count;
    receipt.payload_checksum = facts->complete_payload_checksum;
    receipt.level_envelope_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    receipt.level_envelope_bytes = THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    receipt.level_envelope_checksum = facts->level_envelope_checksum;
    memcpy(receipt.level_envelope,
           payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
           sizeof(receipt.level_envelope));
    receipt.post_envelope_offset = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    receipt.post_envelope_bytes = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    receipt.post_envelope_checksum = facts->post_envelope_checksum;
    memcpy(receipt.post_envelope,
           payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
           sizeof(receipt.post_envelope));
    receipt.status =
        "iso_mode1_2048_initial_level_and_post_envelope_source_bytes_no_semantics";
    *out_receipt = receipt;
    return 1;
}
