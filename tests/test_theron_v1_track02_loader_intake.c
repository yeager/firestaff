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

static uint32_t object_placement_state_hash_for_test(
    const Theron_Track02ObjectTable *objects,
    uint32_t selected_level_index) {
    uint32_t hash = 2166136261u;
    size_t i;

    if (!objects || !objects->shape_ok || objects->record_count == 0u ||
        selected_level_index >= THERON_TRACK02_DUNGEON_COUNT) {
        return 0u;
    }
    hash = mix_hash32(hash, objects->checksum);
    hash = mix_hash32(hash, (uint32_t)objects->record_count);
    hash = mix_hash32(hash, selected_level_index);
    for (i = 0u; i < objects->record_count; ++i) {
        const Theron_Track02ObjectTableRecord *record = &objects->records[i];
        if (record->level_index != selected_level_index) {
            continue;
        }
        hash = mix_hash32(hash, (uint32_t)i);
        hash = mix_hash32(hash, record->object_id);
        hash = mix_hash32(hash, record->kind);
        hash = mix_hash32(hash, record->x);
        hash = mix_hash32(hash, record->y);
        hash = mix_hash32(hash, record->level_index);
        hash = mix_hash32(hash, record->flags);
        hash = mix_hash32(hash, record->argument);
    }
    return hash ? hash : 2166136261u;
}

static uint32_t object_runtime_state_hash_for_test(
    const Theron_Track02ObjectTable *objects,
    uint32_t selected_level_index,
    unsigned int *out_low_kind_mask) {
    uint32_t hash = 2166136261u;
    unsigned int low_kind_mask = 0u;
    size_t i;

    if (out_low_kind_mask) {
        *out_low_kind_mask = 0u;
    }
    if (!objects || !objects->shape_ok ||
        selected_level_index >= THERON_TRACK02_DUNGEON_COUNT) {
        return 0u;
    }
    for (i = 0u; i < objects->record_count; ++i) {
        const Theron_Track02ObjectTableRecord *record = &objects->records[i];
        uint32_t quantity;

        if (record->level_index != selected_level_index) {
            continue;
        }
        if (record->kind < THERON_OBJTYPE_CHEST ||
            (record->kind > THERON_OBJTYPE_TRIGGER &&
             record->kind != THERON_OBJTYPE_QUEST_ITEM)) {
            return 0u;
        }
        if (record->kind != THERON_OBJTYPE_QUEST_ITEM) {
            low_kind_mask |= 1u << record->kind;
        }
        quantity = record->argument ? record->argument : 1u;
        hash = mix_hash32(hash, (uint32_t)i);
        hash = mix_hash32(hash, record->object_id);
        hash = mix_hash32(hash, record->kind);
        hash = mix_hash32(hash, record->flags & 0x03u);
        hash = mix_hash32(hash, record->x);
        hash = mix_hash32(hash, record->y);
        hash = mix_hash32(hash, record->level_index);
        hash = mix_hash32(hash, record->flags);
        hash = mix_hash32(hash, quantity);
    }
    if (out_low_kind_mask) {
        *out_low_kind_mask = low_kind_mask;
    }
    return hash ? hash : 2166136261u;
}

static uint32_t bitmap_palette_source_hash_for_test(
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *runtime,
    size_t palette_raw_offset,
    size_t palette_user_data_offset,
    uint32_t palette_payload_checksum,
    uint32_t palette_decoded_checksum,
    uint32_t bitmap_route_mask,
    uint32_t bitmap_atlas_checksum,
    uint32_t bitmap_atlas_route_count,
    uint32_t bitmap_atlas_nonzero_pixel_count) {
    uint32_t hash = 2166136261u;

    hash = mix_hash32(hash, runtime->record);
    hash = mix_hash32(hash, runtime->selected_dungeon_index);
    hash = mix_hash32(hash, runtime->source_level_index);
    hash = mix_hash32(hash, runtime->target_level_index);
    hash = mix_hash32(hash, (uint32_t)palette_raw_offset);
    hash = mix_hash32(hash, (uint32_t)palette_user_data_offset);
    hash = mix_hash32(hash, palette_payload_checksum);
    hash = mix_hash32(hash, palette_decoded_checksum);
    hash = mix_hash32(hash, bitmap_route_mask);
    hash = mix_hash32(hash, bitmap_atlas_checksum);
    hash = mix_hash32(hash, bitmap_atlas_route_count);
    hash = mix_hash32(hash, bitmap_atlas_nonzero_pixel_count);
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
    Theron_V1RuntimeTrack02ObjectPlacementStateReceipt
        object_placement_state;
    Theron_V1RuntimeTrack02ObjectPlacementStateReceipt
        object_placement_state_ready;
    Theron_V1RuntimeTrack02ObjectPlacementStateReceipt
        target_object_placement_state;
    Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt
        object_gameplay_semantics;
    Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt
        target_object_gameplay_semantics;
    Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt object_world_handoff;
    Theron_V1RuntimeTrack02LevelTransitionHandoffReceipt
        level_transition_handoff;
    Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt
        level_transition_runtime;
    Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt
        bitmap_palette_source;
    Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt decode_vector;
    Theron_V1RuntimeTrack02M11SoulRoomConsumptionReceipt m11_consumption;
    Theron_V1RuntimeTrack02M11LevelConsumptionReceipt m11_level_consumption;
    Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt m11_draw_route;
    Theron_V1RuntimeTrack02Level1DrawBlockerReceipt level1_draw_blocker;
    Theron_Track02ObjectTable object_table;
    Theron_V1_World world;
    Theron_V1_World blocked_world;
    Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt blocked_transition;
    Theron_V1_Object seeded_object;
    Theron_V1_Level target_level;
    char object_dungeon_trace[2048];
    char nonstartup_level_trace[1536];
    char object_table_trace[1536];
    char field_boundary_trace[1536];
    char reviewed_decoder_trace[1536];
    char dungeon_route_trace[1536];
    char dungeon_selection_trace[1536];
    char dungeon_table_binding_trace[1536];
    char level_object_loader_route_trace[1536];
    char object_placement_state_trace[2048];
    char object_gameplay_semantics_trace[2048];
    char level_transition_handoff_trace[2048];
    char bitmap_palette_source_trace[2048];
    uint8_t payload[THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES];
    uint8_t raw_track02[THERON_TRACK02_RAW_SECTOR_BYTES * 2u];
    uint8_t iso_track02[THERON_TRACK02_RAW_USER_DATA_BYTES * 2u];
    uint8_t media_pixels[THERON_RUNTIME_MEDIA_PIXELS];
    uint8_t object_table_bytes[2u +
        2u * THERON_TRACK02_OBJECT_TABLE_RECORD_BYTES];
    uint32_t source_record = 0u;
    uint8_t source_byte = 0u;
    uint32_t loader_route_pair_hash = 0u;
    uint32_t object_placement_state_hash = 0u;
    uint32_t target_object_placement_state_hash = 0u;
    uint32_t object_runtime_state_hash = 0u;
    uint32_t target_object_runtime_state_hash = 0u;
    uint32_t bitmap_palette_source_hash = 0u;
    unsigned int runtime_kind_low_mask = 0u;
    unsigned int target_runtime_kind_low_mask = 0u;
    uint32_t selected_level_index = 0u;
    size_t i;

    for (i = 0u; i < sizeof(payload); ++i) {
        payload[i] = (uint8_t)(i * 37u + 11u);
    }
    facts.complete_payload_checksum = fnv1a32(payload, sizeof(payload));

    memset(raw_track02, 0, sizeof(raw_track02));
    memset(iso_track02, 0, sizeof(iso_track02));
    memset(media_pixels, 1, sizeof(media_pixels));
    memset(object_table_bytes, 0, sizeof(object_table_bytes));
    object_table_bytes[0] = 2u;
    object_table_bytes[2] = 0x11u;
    object_table_bytes[3] = THERON_OBJTYPE_DOOR;
    object_table_bytes[4] = 4u;
    object_table_bytes[5] = 5u;
    object_table_bytes[6] = 0u;
    object_table_bytes[7] = 0x03u;
    object_table_bytes[8] = 0x34u;
    object_table_bytes[9] = 0x12u;
    object_table_bytes[10] = 0x12u;
    object_table_bytes[11] = THERON_OBJTYPE_KEY;
    object_table_bytes[12] = 7u;
    object_table_bytes[13] = 9u;
    object_table_bytes[14] = 1u;
    object_table_bytes[15] = 0x40u;
    object_table_bytes[16] = 0x78u;
    object_table_bytes[17] = 0x56u;
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
    CHECK(theron_v1_track02_read_object_table(
        object_table_bytes, sizeof(object_table_bytes), &object_table) ==
        THERON_TRACK02_SEMANTIC_BINDING_OK);
    selected_level_index = level_object_loader_route.selected_dungeon_index - 1u;
    object_placement_state_hash =
        object_placement_state_hash_for_test(&object_table, selected_level_index);
    CHECK(object_placement_state_hash != 0u);
    snprintf(object_placement_state_trace,
             sizeof(object_placement_state_trace),
             "theron_track02_object_placement_state "
             "same_capture_as_loader_route=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "object_record_count=%zu "
             "object_table_checksum=0x%08x "
             "object_level_mask=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "object_placement_bytes_bound=1 "
             "object_state_low_bits_bound=1 "
             "object_kind_semantics_review_required=1 "
             "world_object_publish_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_loader_route.record,
             level_object_loader_route.consumer_trace_checksum,
             level_object_loader_route.selected_dungeon_index,
             selected_level_index,
             level_object_loader_route.level_route_hash,
             level_object_loader_route.object_table_route_hash,
             level_object_loader_route.loader_route_pair_hash,
             level_object_loader_route.object_table_user_data_byte_count,
             level_object_loader_route.object_table_user_data_hash,
             object_table.record_count,
             object_table.checksum,
             object_table.level_mask,
             object_table.level_record_counts[selected_level_index],
             object_table.level_record_hashes[selected_level_index],
             object_table.level_position_hashes[selected_level_index],
             object_placement_state_hash);
    CHECK(theron_v1_runtime_bind_track02_object_placement_state(
        &level_object_loader_route, &object_table, object_placement_state_trace,
        &object_placement_state));
    CHECK(object_placement_state.valid);
    CHECK(object_placement_state.level_object_loader_route_consumed);
    CHECK(object_placement_state.object_table_shape_consumed);
    CHECK(object_placement_state.same_capture_as_loader_route);
    CHECK(object_placement_state.selected_level_index == selected_level_index);
    CHECK(object_placement_state.object_record_count == object_table.record_count);
    CHECK(object_placement_state.object_table_checksum == object_table.checksum);
    CHECK(object_placement_state.object_level_mask == object_table.level_mask);
    CHECK(object_placement_state.selected_level_record_count ==
          object_table.level_record_counts[selected_level_index]);
    CHECK(object_placement_state.object_placement_state_hash ==
          object_placement_state_hash);
    CHECK(object_placement_state.first_object_id == 0x11u);
    CHECK(object_placement_state.first_object_kind == THERON_OBJTYPE_DOOR);
    CHECK(object_placement_state.first_object_x == 4u);
    CHECK(object_placement_state.first_object_y == 5u);
    CHECK(object_placement_state.first_object_level_index == 0u);
    CHECK(object_placement_state.first_object_state_low_bits == 3u);
    CHECK(object_placement_state.first_object_flags == 0x03u);
    CHECK(object_placement_state.first_object_argument == 0x1234u);
    CHECK(object_placement_state.object_placement_bytes_bound);
    CHECK(object_placement_state.object_state_low_bits_bound);
    CHECK(object_placement_state.object_kind_semantics_review_required);
    CHECK(!object_placement_state.world_object_publish_allowed);
    CHECK(!object_placement_state.dungeon_runtime_admission_allowed);
    CHECK(!object_placement_state.dungeon_draw_allowed);
    CHECK(!object_placement_state.fallback_visuals_allowed);
    object_placement_state_ready = object_placement_state;
    snprintf(object_placement_state_trace,
             sizeof(object_placement_state_trace),
             "theron_track02_object_placement_state "
             "same_capture_as_loader_route=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "object_record_count=%zu "
             "object_table_checksum=0x%08x "
             "object_level_mask=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "object_placement_bytes_bound=1 "
             "object_state_low_bits_bound=1 "
             "object_kind_semantics_review_required=1 "
             "world_object_publish_allowed=1 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_loader_route.record,
             level_object_loader_route.consumer_trace_checksum,
             level_object_loader_route.selected_dungeon_index,
             selected_level_index,
             level_object_loader_route.level_route_hash,
             level_object_loader_route.object_table_route_hash,
             level_object_loader_route.loader_route_pair_hash,
             level_object_loader_route.object_table_user_data_byte_count,
             level_object_loader_route.object_table_user_data_hash,
             object_table.record_count,
             object_table.checksum,
             object_table.level_mask,
             object_table.level_record_counts[selected_level_index],
             object_table.level_record_hashes[selected_level_index],
             object_table.level_position_hashes[selected_level_index],
             object_placement_state_hash);
    CHECK(!theron_v1_runtime_bind_track02_object_placement_state(
        &level_object_loader_route, &object_table, object_placement_state_trace,
        &object_placement_state));
    CHECK(!object_placement_state.valid);
    snprintf(object_placement_state_trace,
             sizeof(object_placement_state_trace),
             "theron_track02_object_placement_state "
             "same_capture_as_loader_route=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "object_record_count=%zu "
             "object_table_checksum=0x%08x "
             "object_level_mask=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "object_placement_bytes_bound=1 "
             "object_state_low_bits_bound=1 "
             "object_kind_semantics_review_required=1 "
             "world_object_publish_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_loader_route.record,
             level_object_loader_route.consumer_trace_checksum,
             level_object_loader_route.selected_dungeon_index,
             selected_level_index,
             level_object_loader_route.level_route_hash,
             level_object_loader_route.object_table_route_hash,
             level_object_loader_route.loader_route_pair_hash,
             level_object_loader_route.object_table_user_data_byte_count,
             level_object_loader_route.object_table_user_data_hash,
             object_table.record_count,
             object_table.checksum,
             object_table.level_mask,
             object_table.level_record_counts[selected_level_index],
             object_table.level_record_hashes[selected_level_index],
             object_table.level_position_hashes[selected_level_index],
             object_placement_state_hash ^ 1u);
    CHECK(!theron_v1_runtime_bind_track02_object_placement_state(
        &level_object_loader_route, &object_table, object_placement_state_trace,
        &object_placement_state));
    CHECK(!object_placement_state.valid);
    object_placement_state = object_placement_state_ready;
    object_runtime_state_hash = object_runtime_state_hash_for_test(
        &object_table, selected_level_index, &runtime_kind_low_mask);
    CHECK(object_runtime_state_hash != 0u);
    CHECK(runtime_kind_low_mask == (1u << THERON_OBJTYPE_DOOR));
    snprintf(object_gameplay_semantics_trace,
             sizeof(object_gameplay_semantics_trace),
             "theron_track02_object_gameplay_semantics "
             "same_capture_as_placement_state=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "runtime_kind_low_mask=0x%08x "
             "runtime_kind_quest_item_seen=0 "
             "object_runtime_state_hash=0x%08x "
             "object_kind_semantics_proven=1 "
             "flags_low_bits_state_bound=1 "
             "argument_quantity_bound=1 "
             "object_flags_preserved=1 "
             "all_selected_records_runtime_mappable=1 "
             "world_object_publish_allowed=1 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             object_placement_state.record,
             object_placement_state.consumer_trace_checksum,
             object_placement_state.selected_dungeon_index,
             object_placement_state.selected_level_index,
             object_placement_state.object_table_route_hash,
             object_placement_state.loader_route_pair_hash,
             object_placement_state.object_placement_state_hash,
             object_placement_state.selected_level_record_count,
             object_placement_state.selected_level_record_hash,
             object_placement_state.selected_level_position_hash,
             runtime_kind_low_mask,
             object_runtime_state_hash);
    CHECK(theron_v1_runtime_bind_track02_object_gameplay_semantics(
        &object_placement_state, &object_table, object_gameplay_semantics_trace,
        &object_gameplay_semantics));
    CHECK(object_gameplay_semantics.valid);
    CHECK(object_gameplay_semantics.object_placement_state_consumed);
    CHECK(object_gameplay_semantics.object_kind_semantics_proven);
    CHECK(object_gameplay_semantics.flags_low_bits_state_bound);
    CHECK(object_gameplay_semantics.argument_quantity_bound);
    CHECK(object_gameplay_semantics.object_flags_preserved);
    CHECK(object_gameplay_semantics.all_selected_records_runtime_mappable);
    CHECK(object_gameplay_semantics.runtime_kind_low_mask ==
          runtime_kind_low_mask);
    CHECK(object_gameplay_semantics.object_runtime_state_hash ==
          object_runtime_state_hash);
    CHECK(object_gameplay_semantics.first_runtime_type == THERON_OBJTYPE_DOOR);
    CHECK(object_gameplay_semantics.first_runtime_state == 3u);
    CHECK(object_gameplay_semantics.first_runtime_flags == 0x03u);
    CHECK(object_gameplay_semantics.first_runtime_quantity == 0x1234);
    CHECK(object_gameplay_semantics.world_object_publish_allowed);
    CHECK(!object_gameplay_semantics.dungeon_runtime_admission_allowed);
    CHECK(!object_gameplay_semantics.dungeon_draw_allowed);
    CHECK(!object_gameplay_semantics.fallback_visuals_allowed);

    theron_v1_world_init(&world);
    world.current_dungeon = THERON_DUNGEON_1_AKUTUBA;
    world.current_level = 0;
    world.level_loaded[0][0] = 1;
    world.levels[0][0].width = 32;
    world.levels[0][0].height = 27;
    memset(&seeded_object, 0, sizeof(seeded_object));
    seeded_object.type = THERON_OBJTYPE_BUTTON;
    seeded_object.x = 1;
    seeded_object.y = 1;
    seeded_object.level = 0;
    seeded_object.dungeon_id = THERON_DUNGEON_1_AKUTUBA;
    CHECK(theron_v1_object_place(&world, &seeded_object) == 0);
    seeded_object.level = 1;
    CHECK(theron_v1_object_place(&world, &seeded_object) == 0);
    CHECK(theron_v1_runtime_publish_track02_object_gameplay_state(
        &world, THERON_DUNGEON_1_AKUTUBA, &object_gameplay_semantics,
        &object_table, &object_world_handoff));
    CHECK(object_world_handoff.valid);
    CHECK(object_world_handoff.world_mutated);
    CHECK(object_world_handoff.before_object_count == 2);
    CHECK(object_world_handoff.removed_selected_level_object_count == 1);
    CHECK(object_world_handoff.placed_object_count == 1);
    CHECK(object_world_handoff.after_object_count == 2);
    CHECK(object_world_handoff.level_loaded);
    CHECK(object_world_handoff.current_level_after == 0);
    CHECK(object_world_handoff.thing_count_after == 1);
    CHECK(object_world_handoff.before_world_hash !=
          object_world_handoff.after_world_hash);
    CHECK(world.objects[0].level == 1);
    CHECK(world.objects[1].type == THERON_OBJTYPE_DOOR);
    CHECK(world.objects[1].state == 3u);
    CHECK(world.objects[1].x == 4);
    CHECK(world.objects[1].y == 5);
    CHECK(world.objects[1].quantity == 0x1234);
    CHECK(world.objects[1].flags == 0x03u);
    CHECK(!object_world_handoff.dungeon_runtime_admission_allowed);
    CHECK(!object_world_handoff.dungeon_draw_allowed);
    CHECK(!object_world_handoff.fallback_visuals_allowed);

    selected_level_index = 1u;
    target_object_placement_state_hash =
        object_placement_state_hash_for_test(&object_table, selected_level_index);
    CHECK(target_object_placement_state_hash != 0u);
    snprintf(object_placement_state_trace,
             sizeof(object_placement_state_trace),
             "theron_track02_object_placement_state "
             "same_capture_as_loader_route=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "level_route_hash=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_table_user_data_byte_count=%zu "
             "object_table_user_data_hash=0x%08x "
             "object_record_count=%zu "
             "object_table_checksum=0x%08x "
             "object_level_mask=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "object_placement_bytes_bound=1 "
             "object_state_low_bits_bound=1 "
             "object_kind_semantics_review_required=1 "
             "world_object_publish_allowed=0 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_object_loader_route.record,
             level_object_loader_route.consumer_trace_checksum,
             level_object_loader_route.selected_dungeon_index,
             selected_level_index,
             level_object_loader_route.level_route_hash,
             level_object_loader_route.object_table_route_hash,
             level_object_loader_route.loader_route_pair_hash,
             level_object_loader_route.object_table_user_data_byte_count,
             level_object_loader_route.object_table_user_data_hash,
             object_table.record_count,
             object_table.checksum,
             object_table.level_mask,
             object_table.level_record_counts[selected_level_index],
             object_table.level_record_hashes[selected_level_index],
             object_table.level_position_hashes[selected_level_index],
             target_object_placement_state_hash);
    CHECK(theron_v1_runtime_bind_track02_object_placement_state(
        &level_object_loader_route, &object_table, object_placement_state_trace,
        &target_object_placement_state));
    target_object_runtime_state_hash = object_runtime_state_hash_for_test(
        &object_table, selected_level_index, &target_runtime_kind_low_mask);
    CHECK(target_object_runtime_state_hash != 0u);
    CHECK(target_runtime_kind_low_mask == (1u << THERON_OBJTYPE_KEY));
    snprintf(object_gameplay_semantics_trace,
             sizeof(object_gameplay_semantics_trace),
             "theron_track02_object_gameplay_semantics "
             "same_capture_as_placement_state=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "runtime_kind_low_mask=0x%08x "
             "runtime_kind_quest_item_seen=0 "
             "object_runtime_state_hash=0x%08x "
             "object_kind_semantics_proven=1 "
             "flags_low_bits_state_bound=1 "
             "argument_quantity_bound=1 "
             "object_flags_preserved=1 "
             "all_selected_records_runtime_mappable=1 "
             "world_object_publish_allowed=1 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             target_object_placement_state.record,
             target_object_placement_state.consumer_trace_checksum,
             target_object_placement_state.selected_dungeon_index,
             target_object_placement_state.selected_level_index,
             target_object_placement_state.object_table_route_hash,
             target_object_placement_state.loader_route_pair_hash,
             target_object_placement_state.object_placement_state_hash,
             target_object_placement_state.selected_level_record_count,
             target_object_placement_state.selected_level_record_hash,
             target_object_placement_state.selected_level_position_hash,
             target_runtime_kind_low_mask,
             target_object_runtime_state_hash);
    CHECK(theron_v1_runtime_bind_track02_object_gameplay_semantics(
        &target_object_placement_state, &object_table,
        object_gameplay_semantics_trace, &target_object_gameplay_semantics));
    snprintf(level_transition_handoff_trace,
             sizeof(level_transition_handoff_trace),
             "theron_track02_level_transition_handoff "
             "same_capture_as_target_loader_route=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "source_level_index=0 "
             "target_level_index=1 "
             "target_object_table_route_hash=0x%08x "
             "target_loader_route_pair_hash=0x%08x "
             "target_object_runtime_state_hash=0x%08x "
             "target_level_byte_count=0x00000370 "
             "target_level_raw_hash=0x51525354 "
             "target_object_record_count=%zu "
             "target_object_level_record_hash=0x%08x "
             "loader_level_selector_bound=1 "
             "transition_source_level_bound=1 "
             "transition_target_level_bound=1 "
             "party_placement_bound=1 "
             "object_pool_state_bound=1 "
             "level_runtime_load_allowed=1 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             target_object_gameplay_semantics.record,
             target_object_gameplay_semantics.consumer_trace_checksum,
             target_object_gameplay_semantics.selected_dungeon_index,
             target_object_gameplay_semantics.object_table_route_hash,
             target_object_gameplay_semantics.loader_route_pair_hash,
             target_object_gameplay_semantics.object_runtime_state_hash,
             target_object_gameplay_semantics.selected_level_record_count,
             target_object_gameplay_semantics.selected_level_record_hash);
    CHECK(theron_v1_runtime_bind_track02_level_transition_handoff(
        &object_world_handoff, &target_object_gameplay_semantics,
        level_transition_handoff_trace, &level_transition_handoff));
    CHECK(level_transition_handoff.valid);
    CHECK(level_transition_handoff.source_level_index == 0u);
    CHECK(level_transition_handoff.target_level_index == 1u);
    CHECK(level_transition_handoff.level_runtime_load_allowed);
    CHECK(!level_transition_handoff.dungeon_runtime_admission_allowed);
    CHECK(!level_transition_handoff.dungeon_draw_allowed);
    CHECK(!level_transition_handoff.fallback_visuals_allowed);

    memset(&target_level, 0, sizeof(target_level));
    target_level.width = 32;
    target_level.height = 27;
    target_level.start_x = 6;
    target_level.start_y = 7;
    target_level.start_dir = 1;
    target_level.squares[7][6] = THERON_SQUARE_FLOOR;
    target_level.squares[7][7] = THERON_SQUARE_FLOOR;
    target_level.squares[7][8] = THERON_SQUARE_STAIRS_DOWN;
    target_level.squares[6][6] = THERON_SQUARE_WALL;
    target_level.squares[8][6] = THERON_SQUARE_FLOOR;
    world.transition_pending = 1;
    world.transition_type = THERON_TRANSITION_STAIRS;
    world.transition_target_level = 1;
    CHECK(theron_v1_runtime_publish_track02_level_transition(
        &world, &level_transition_handoff, &target_level, &object_table,
        &target_object_gameplay_semantics, &level_transition_runtime));
    CHECK(level_transition_runtime.valid);
    CHECK(level_transition_runtime.world_mutated);
    CHECK(level_transition_runtime.transition_pending_before == 1);
    CHECK(level_transition_runtime.transition_pending_after == 0);
    CHECK(level_transition_runtime.level_loaded);
    CHECK(level_transition_runtime.current_level_after == 1);
    CHECK(level_transition_runtime.party_x == 6);
    CHECK(level_transition_runtime.party_y == 7);
    CHECK(level_transition_runtime.party_dir == 1);
    CHECK(level_transition_runtime.target_object_count == 1);
    CHECK(level_transition_runtime.target_thing_count == 1);
    CHECK(level_transition_runtime.before_world_hash !=
          level_transition_runtime.after_world_hash);
    CHECK(world.current_level == 1);
    CHECK(world.party.leader_x == 6 && world.party.leader_y == 7);
    CHECK(world.objects[0].level == 0);
    CHECK(world.objects[1].level == 1);
    CHECK(world.objects[1].type == THERON_OBJTYPE_KEY);
    CHECK(world.objects[1].quantity == 0x5678);
    CHECK(!level_transition_runtime.dungeon_runtime_admission_allowed);
    CHECK(!level_transition_runtime.dungeon_draw_allowed);
    CHECK(!level_transition_runtime.fallback_visuals_allowed);

    bitmap_palette_source_hash = bitmap_palette_source_hash_for_test(
        &level_transition_runtime, 0x002a06a0u, 0x0029f6b0u, 0x7b0f13c9u,
        0x3a5d7811u, 0x0000000fu, 0x55aa7744u, 4u, 913u);
    snprintf(bitmap_palette_source_trace, sizeof(bitmap_palette_source_trace),
             "theron_track02_bitmap_palette_source "
             "same_capture_as_level_transition=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "selected_dungeon_index=0x%08x "
             "source_level_index=0x%08x "
             "target_level_index=0x%08x "
             "palette_raw_offset=%zu "
             "palette_user_data_offset=%zu "
             "palette_payload_checksum=0x%08x "
             "palette_decoded_checksum=0x%08x "
             "bitmap_route_mask=0x%08x "
             "bitmap_atlas_checksum=0x%08x "
             "bitmap_atlas_route_count=0x%08x "
             "bitmap_atlas_nonzero_pixel_count=0x%08x "
             "bitmap_palette_source_hash=0x%08x "
             "palette_window_source_bound=1 "
             "bitmap_route_source_bound=1 "
             "palette_decode_verified=0 "
             "bitmap_decode_verified=0 "
             "pixel_output_verified=0 "
             "m11_render_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_transition_runtime.record,
             level_transition_runtime.selected_dungeon_index,
             level_transition_runtime.source_level_index,
             level_transition_runtime.target_level_index, (size_t)0x002a06a0u,
             (size_t)0x0029f6b0u, 0x7b0f13c9u, 0x3a5d7811u, 0x0000000fu,
             0x55aa7744u, 4u, 913u, bitmap_palette_source_hash);
    CHECK(theron_v1_runtime_bind_track02_bitmap_palette_source(
        &level_transition_runtime, bitmap_palette_source_trace,
        &bitmap_palette_source));
    CHECK(bitmap_palette_source.valid);
    CHECK(bitmap_palette_source.level_transition_runtime_consumed);
    CHECK(bitmap_palette_source.same_capture_as_level_transition);
    CHECK(bitmap_palette_source.palette_raw_offset == 0x002a06a0u);
    CHECK(bitmap_palette_source.palette_user_data_offset == 0x0029f6b0u);
    CHECK(bitmap_palette_source.palette_payload_checksum == 0x7b0f13c9u);
    CHECK(bitmap_palette_source.palette_decoded_checksum == 0x3a5d7811u);
    CHECK(bitmap_palette_source.bitmap_route_mask == 0x0000000fu);
    CHECK(bitmap_palette_source.bitmap_atlas_checksum == 0x55aa7744u);
    CHECK(bitmap_palette_source.bitmap_atlas_route_count == 4u);
    CHECK(bitmap_palette_source.bitmap_atlas_nonzero_pixel_count == 913u);
    CHECK(bitmap_palette_source.bitmap_palette_source_hash ==
          bitmap_palette_source_hash);
    CHECK(bitmap_palette_source.palette_window_source_bound);
    CHECK(bitmap_palette_source.bitmap_route_source_bound);
    CHECK(!bitmap_palette_source.palette_decode_verified);
    CHECK(!bitmap_palette_source.bitmap_decode_verified);
    CHECK(!bitmap_palette_source.pixel_output_verified);
    CHECK(!bitmap_palette_source.m11_render_allowed);
    CHECK(!bitmap_palette_source.dungeon_draw_allowed);
    CHECK(!bitmap_palette_source.fallback_visuals_allowed);

    memset(&decode_vector, 0, sizeof(decode_vector));
    decode_vector.valid = 1;
    decode_vector.bitmap_palette_source_consumed = 1;
    decode_vector.real_track02_bytes_consumed = 1;
    decode_vector.variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(decode_vector.track02_md5, sizeof(decode_vector.track02_md5),
             "%s", THERON_TRACK02_MD5_US_BIN);
    decode_vector.record = bitmap_palette_source.record;
    decode_vector.selected_dungeon_index =
        bitmap_palette_source.selected_dungeon_index;
    decode_vector.source_level_index = 0u;
    decode_vector.target_level_index = 1u;
    decode_vector.palette_decoded_checksum =
        bitmap_palette_source.palette_decoded_checksum;
    decode_vector.palette_nonblack_entry_count = 9u;
    decode_vector.first_bitmap_route_bit =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
    decode_vector.first_bitmap_route_width = 64u;
    decode_vector.first_bitmap_route_height = 8u;
    decode_vector.first_bitmap_route_tile_count = 8u;
    decode_vector.first_bitmap_route_nonzero_pixel_count = 512u;
    decode_vector.first_bitmap_route_checksum = 0x2468ace0u;
    decode_vector.first_bitmap_raw_offset = 0x00300010u;
    decode_vector.first_bitmap_user_data_offset = 0x002fe020u;
    decode_vector.stage_bitmap_route_bit =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;
    decode_vector.stage_bitmap_route_width = 64u;
    decode_vector.stage_bitmap_route_height = 8u;
    decode_vector.stage_bitmap_route_tile_count = 8u;
    decode_vector.stage_bitmap_route_nonzero_pixel_count = 512u;
    decode_vector.stage_bitmap_route_checksum = 0x33334444u;
    decode_vector.stage_bitmap_raw_offset = 0x00280010u;
    decode_vector.stage_bitmap_user_data_offset = 0x0027f020u;
    decode_vector.first_pixel_row_hash = 0x13572468u;
    decode_vector.palette_decode_verified = 1;
    decode_vector.bitmap_decode_verified = 1;
    decode_vector.pixel_output_verified = 1;

    theron_v1_world_runtime_media_clear(&world);
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_TITLE,
        THERON_TRACK02_MD5_US_BIN, THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
        64u, 8u, 0x00200010u, 0x0020002cu, 0x001ff020u, 8u, 512u,
        0x11112222u, media_pixels, 64u * 8u));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE,
        THERON_TRACK02_MD5_US_BIN, THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
        64u, 8u, 0x00280010u, 0x0028002cu, 0x0027f020u, 8u, 512u,
        0x33334444u, media_pixels, 64u * 8u));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM,
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 64u, 8u,
        decode_vector.first_bitmap_raw_offset, 0x0030002cu,
        decode_vector.first_bitmap_user_data_offset,
        decode_vector.first_bitmap_route_tile_count,
        decode_vector.first_bitmap_route_nonzero_pixel_count,
        decode_vector.first_bitmap_route_checksum, media_pixels, 64u * 8u));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD,
        THERON_TRACK02_MD5_US_BIN,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 64u, 8u,
        0x00380010u, 0x0038002cu, 0x0037f020u, 8u, 512u,
        0x55556666u, media_pixels, 64u * 8u));
    CHECK(!theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE,
        "00000000000000000000000000000000",
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 64u, 8u,
        0x00280010u, 0x0028002cu, 0x0027f020u, 8u, 512u,
        0x33334444u, media_pixels, 64u * 8u));
    world.runtime_media.identity.ready = 1;
    world.runtime_media.identity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    world.runtime_media.identity.bank_stride = 0x0400u;
    world.runtime_media.identity.checksum = 0x1234fedcu;
    CHECK(theron_v1_world_runtime_media_for_level(&world, 0, 0) ==
          &world.runtime_media.soul_room);
    strcpy(world.runtime_media.stage.track02_md5,
           "00000000000000000000000000000000");
    CHECK(!theron_v1_world_runtime_media_select_level_bank(
        &world, THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL,
        THERON_DUNGEON_1_AKUTUBA, 1));
    strcpy(world.runtime_media.stage.track02_md5, THERON_TRACK02_MD5_US_BIN);
    world.runtime_media.identity.track02_variant = THERON_TRACK02_VARIANT_JP_BIN;
    CHECK(!theron_v1_world_runtime_media_select_level_bank(
        &world, THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL,
        THERON_DUNGEON_1_AKUTUBA, 1));
    world.runtime_media.identity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    CHECK(theron_v1_runtime_bind_track02_m11_soul_room_consumption(
        &decode_vector, &world, 320, 200, 16, 24, 1, 1,
        &m11_consumption));
    CHECK(m11_consumption.valid);
    CHECK(m11_consumption.decode_vector_consumed);
    CHECK(m11_consumption.world_runtime_media_consumed);
    CHECK(m11_consumption.soul_room_level0_selected);
    CHECK(m11_consumption.exact_indexed_atlas_consumed);
    CHECK(m11_consumption.huc6260_palette_consumed);
    CHECK(m11_consumption.route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM);
    CHECK(m11_consumption.source_width == 64u);
    CHECK(m11_consumption.source_height == 8u);
    CHECK(m11_consumption.placement_x == 16);
    CHECK(m11_consumption.placement_y == 24);
    CHECK(m11_consumption.clip_x == 16);
    CHECK(m11_consumption.clip_y == 24);
    CHECK(m11_consumption.clip_w == 64);
    CHECK(m11_consumption.clip_h == 8);
    CHECK(m11_consumption.scale_verified);
    CHECK(m11_consumption.clip_verified);
    CHECK(m11_consumption.host_presentation_allowed);
    CHECK(m11_consumption.m11_runtime_consumption_allowed);
    CHECK(m11_consumption.m11_render_allowed);
    CHECK(!m11_consumption.dungeon_draw_allowed);
    CHECK(!m11_consumption.fallback_visuals_allowed);
    CHECK(world.runtime_media.level_bank.ready);
    CHECK(world.runtime_media.level_bank.kind ==
          THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL);
    CHECK(world.runtime_media.level_bank.level_index == 0);
    CHECK(world.runtime_media.level_bank.route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM);

    CHECK(theron_v1_world_runtime_media_for_level(&world, 1, 0) ==
          &world.runtime_media.stage);
    CHECK(theron_v1_runtime_bind_track02_m11_level_consumption(
        &decode_vector, &level_transition_runtime, &world, 320, 200, 32, 40,
        1, 1, &m11_level_consumption));
    CHECK(m11_level_consumption.valid);
    CHECK(m11_level_consumption.decode_vector_consumed);
    CHECK(m11_level_consumption.level_transition_runtime_consumed);
    CHECK(m11_level_consumption.world_runtime_media_consumed);
    CHECK(m11_level_consumption.target_level_selected);
    CHECK(m11_level_consumption.exact_indexed_atlas_consumed);
    CHECK(m11_level_consumption.huc6260_palette_consumed);
    CHECK(m11_level_consumption.route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);
    CHECK(m11_level_consumption.source_level_index == 0u);
    CHECK(m11_level_consumption.target_level_index == 1u);
    CHECK(m11_level_consumption.source_width == 64u);
    CHECK(m11_level_consumption.source_height == 8u);
    CHECK(m11_level_consumption.source_tile_count == 8u);
    CHECK(m11_level_consumption.source_nonzero_pixel_count == 512u);
    CHECK(m11_level_consumption.source_checksum == 0x33334444u);
    CHECK(m11_level_consumption.palette_decoded_checksum ==
          decode_vector.palette_decoded_checksum);
    CHECK(m11_level_consumption.first_raw_offset == 0x00280010u);
    CHECK(m11_level_consumption.first_user_data_offset == 0x0027f020u);
    CHECK(m11_level_consumption.placement_x == 32);
    CHECK(m11_level_consumption.placement_y == 40);
    CHECK(m11_level_consumption.clip_x == 32);
    CHECK(m11_level_consumption.clip_y == 40);
    CHECK(m11_level_consumption.clip_w == 64);
    CHECK(m11_level_consumption.clip_h == 8);
    CHECK(m11_level_consumption.scale_verified);
    CHECK(m11_level_consumption.clip_verified);
    CHECK(m11_level_consumption.host_presentation_allowed);
    CHECK(m11_level_consumption.m11_runtime_consumption_allowed);
    CHECK(m11_level_consumption.m11_render_allowed);
    CHECK(!m11_level_consumption.dungeon_draw_allowed);
    CHECK(!m11_level_consumption.fallback_visuals_allowed);
    CHECK(world.runtime_media.level_bank.ready);
    CHECK(world.runtime_media.level_bank.kind ==
          THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL);
    CHECK(world.runtime_media.level_bank.level_index == 1);
    CHECK(world.runtime_media.level_bank.route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);

    theron_v1_world_init(&blocked_world);
    blocked_transition = level_transition_runtime;
    blocked_transition.target_object_count = 0;
    blocked_transition.target_thing_count = 0;
    memset(&m11_draw_route, 0, sizeof(m11_draw_route));
    CHECK(theron_v1_runtime_bind_track02_level1_draw_blocker(
        &m11_level_consumption, &blocked_transition, &data_gap,
        &blocked_world, &m11_draw_route, &level1_draw_blocker));
    CHECK(level1_draw_blocker.valid);
    CHECK(level1_draw_blocker.m11_level_consumption_consumed);
    CHECK(level1_draw_blocker.level_transition_runtime_consumed);
    CHECK(level1_draw_blocker.original_data_binding_gap_consumed);
    CHECK(level1_draw_blocker.world_runtime_state_inspected);
    CHECK(level1_draw_blocker.real_track02_level1_media_bound);
    CHECK(level1_draw_blocker.nonstartup_geometry_source_blocked);
    CHECK(level1_draw_blocker.object_placement_source_blocked);
    CHECK(level1_draw_blocker.loadertrace_geometry_window_missing);
    CHECK(level1_draw_blocker.loadertrace_object_window_missing);
    CHECK(!level1_draw_blocker.level1_world_geometry_loaded);
    CHECK(!level1_draw_blocker.level1_object_placement_loaded);
    CHECK(level1_draw_blocker.transition_level_loaded);
    CHECK(level1_draw_blocker.transition_target_object_count == 0);
    CHECK(level1_draw_blocker.transition_target_thing_count == 0);
    CHECK(level1_draw_blocker.world_object_count == 0);
    CHECK(level1_draw_blocker.target_level_index == 1u);
    CHECK(level1_draw_blocker.route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);
    CHECK(level1_draw_blocker.media_checksum ==
          m11_level_consumption.source_checksum);
    CHECK(level1_draw_blocker.palette_decoded_checksum ==
          m11_level_consumption.palette_decoded_checksum);
    CHECK(level1_draw_blocker.first_nonstartup_raw_offset ==
          data_gap.first_nonstartup_raw_offset);
    CHECK(level1_draw_blocker.first_nonstartup_user_data_offset ==
          data_gap.first_nonstartup_user_data_offset);
    CHECK(level1_draw_blocker.first_nonstartup_byte_count ==
          data_gap.first_nonstartup_byte_count);
    CHECK(level1_draw_blocker.first_nonstartup_raw_hash ==
          data_gap.first_nonstartup_raw_hash);
    CHECK(level1_draw_blocker.object_table_raw_offset ==
          data_gap.first_container_raw_offset);
    CHECK(level1_draw_blocker.object_table_user_data_offset ==
          data_gap.first_container_user_data_offset);
    CHECK(level1_draw_blocker.object_table_byte_count ==
          data_gap.first_container_user_data_byte_count);
    CHECK(level1_draw_blocker.object_table_raw_hash ==
          data_gap.first_container_user_data_hash);
    CHECK(!level1_draw_blocker.dungeon_draw_route_allowed);
    CHECK(!level1_draw_blocker.dungeon_pixel_blit_allowed);
    CHECK(!level1_draw_blocker.fallback_visuals_allowed);

    CHECK(theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
        &m11_level_consumption, &level_transition_runtime, &world,
        &m11_draw_route));
    CHECK(m11_draw_route.valid);
    CHECK(m11_draw_route.m11_level_consumption_consumed);
    CHECK(m11_draw_route.level_transition_runtime_consumed);
    CHECK(m11_draw_route.world_runtime_geometry_consumed);
    CHECK(m11_draw_route.object_placement_consumed);
    CHECK(m11_draw_route.viewport_composition_route_bound);
    CHECK(m11_draw_route.route_bit ==
          THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);
    CHECK(m11_draw_route.target_level_index == 1u);
    CHECK(m11_draw_route.media_checksum ==
          m11_level_consumption.source_checksum);
    CHECK(m11_draw_route.palette_decoded_checksum ==
          m11_level_consumption.palette_decoded_checksum);
    CHECK(m11_draw_route.level_width == 32);
    CHECK(m11_draw_route.level_height == 27);
    CHECK(m11_draw_route.party_x == 6);
    CHECK(m11_draw_route.party_y == 7);
    CHECK(m11_draw_route.party_dir == 1);
    CHECK(m11_draw_route.current_square == THERON_SQUARE_FLOOR);
    CHECK(m11_draw_route.forward_square == THERON_SQUARE_FLOOR);
    CHECK(m11_draw_route.sampled_cell_count == 5u);
    CHECK(m11_draw_route.sampled_wall_count == 1u);
    CHECK(m11_draw_route.sampled_floor_count == 3u);
    CHECK(m11_draw_route.sampled_special_count == 1u);
    CHECK(m11_draw_route.sampled_object_count == 1u);
    CHECK(m11_draw_route.level_geometry_hash != 0u);
    CHECK(m11_draw_route.object_placement_hash != 0u);
    CHECK(m11_draw_route.viewport_route_hash != 0u);
    CHECK(m11_draw_route.m11_host_presentation_allowed);
    CHECK(m11_draw_route.dungeon_draw_route_allowed);
    CHECK(!m11_draw_route.dungeon_pixel_blit_allowed);
    CHECK(!m11_draw_route.fallback_visuals_allowed);
    CHECK(!theron_v1_runtime_bind_track02_level1_draw_blocker(
        &m11_level_consumption, &level_transition_runtime, &data_gap, &world,
        &m11_draw_route, &level1_draw_blocker));

    world.levels[0][1].squares[7][6] = THERON_SQUARE_WALL;
    CHECK(!theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
        &m11_level_consumption, &level_transition_runtime, &world,
        &m11_draw_route));
    world.levels[0][1].squares[7][6] = THERON_SQUARE_FLOOR;
    m11_level_consumption.source_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
        &m11_level_consumption, &level_transition_runtime, &world,
        &m11_draw_route));
    m11_level_consumption.source_checksum ^= 1u;
    level_transition_runtime.target_object_count = 2;
    CHECK(!theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
        &m11_level_consumption, &level_transition_runtime, &world,
        &m11_draw_route));
    level_transition_runtime.target_object_count = 1;
    world.runtime_media.level_bank.surface_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
        &m11_level_consumption, &level_transition_runtime, &world,
        &m11_draw_route));
    world.runtime_media.level_bank.surface_checksum ^= 1u;

    decode_vector.stage_bitmap_route_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_level_consumption(
        &decode_vector, &level_transition_runtime, &world, 320, 200, 32, 40,
        1, 1, &m11_level_consumption));
    decode_vector.stage_bitmap_route_checksum ^= 1u;
    level_transition_runtime.target_level_index = 0u;
    CHECK(!theron_v1_runtime_bind_track02_m11_level_consumption(
        &decode_vector, &level_transition_runtime, &world, 320, 200, 32, 40,
        1, 1, &m11_level_consumption));
    level_transition_runtime.target_level_index = 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_level_consumption(
        &decode_vector, &level_transition_runtime, &world, 320, 200, 32, 40,
        2, 1, &m11_level_consumption));
    world.runtime_media.stage.checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_level_consumption(
        &decode_vector, &level_transition_runtime, &world, 320, 200, 32, 40,
        1, 1, &m11_level_consumption));
    world.runtime_media.stage.checksum ^= 1u;

    decode_vector.first_bitmap_route_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_soul_room_consumption(
        &decode_vector, &world, 320, 200, 16, 24, 1, 1,
        &m11_consumption));
    decode_vector.first_bitmap_route_checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_soul_room_consumption(
        &decode_vector, &world, 32, 8, 16, 24, 1, 1,
        &m11_consumption));
    CHECK(!theron_v1_runtime_bind_track02_m11_soul_room_consumption(
        &decode_vector, &world, 320, 200, 16, 24, 2, 1,
        &m11_consumption));
    world.runtime_media.soul_room.checksum ^= 1u;
    CHECK(!theron_v1_runtime_bind_track02_m11_soul_room_consumption(
        &decode_vector, &world, 320, 200, 16, 24, 1, 1,
        &m11_consumption));
    world.runtime_media.soul_room.checksum ^= 1u;

    snprintf(bitmap_palette_source_trace, sizeof(bitmap_palette_source_trace),
             "theron_track02_bitmap_palette_source "
             "same_capture_as_level_transition=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "selected_dungeon_index=0x%08x "
             "source_level_index=0x%08x "
             "target_level_index=0x%08x "
             "palette_raw_offset=%zu "
             "palette_user_data_offset=%zu "
             "palette_payload_checksum=0x%08x "
             "palette_decoded_checksum=0x%08x "
             "bitmap_route_mask=0x%08x "
             "bitmap_atlas_checksum=0x%08x "
             "bitmap_atlas_route_count=0x%08x "
             "bitmap_atlas_nonzero_pixel_count=0x%08x "
             "bitmap_palette_source_hash=0x%08x "
             "palette_window_source_bound=1 "
             "bitmap_route_source_bound=1 "
             "palette_decode_verified=0 "
             "bitmap_decode_verified=0 "
             "pixel_output_verified=0 "
             "m11_render_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_transition_runtime.record,
             level_transition_runtime.selected_dungeon_index,
             level_transition_runtime.source_level_index,
             level_transition_runtime.target_level_index, (size_t)0x002a06a0u,
             (size_t)0x0029f6b0u, 0x7b0f13c9u, 0x3a5d7811u, 0x0000000fu,
             0x55aa7744u, 4u, 913u, bitmap_palette_source_hash ^ 1u);
    CHECK(!theron_v1_runtime_bind_track02_bitmap_palette_source(
        &level_transition_runtime, bitmap_palette_source_trace,
        &bitmap_palette_source));
    snprintf(bitmap_palette_source_trace, sizeof(bitmap_palette_source_trace),
             "theron_track02_bitmap_palette_source "
             "same_capture_as_level_transition=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "selected_dungeon_index=0x%08x "
             "source_level_index=0x%08x "
             "target_level_index=0x%08x "
             "palette_raw_offset=%zu "
             "palette_user_data_offset=%zu "
             "palette_payload_checksum=0x%08x "
             "palette_decoded_checksum=0x%08x "
             "bitmap_route_mask=0x%08x "
             "bitmap_atlas_checksum=0x%08x "
             "bitmap_atlas_route_count=0x%08x "
             "bitmap_atlas_nonzero_pixel_count=0x%08x "
             "bitmap_palette_source_hash=0x%08x "
             "palette_window_source_bound=1 "
             "bitmap_route_source_bound=1 "
             "palette_decode_verified=0 "
             "bitmap_decode_verified=0 "
             "pixel_output_verified=1 "
             "m11_render_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_transition_runtime.record,
             level_transition_runtime.selected_dungeon_index,
             level_transition_runtime.source_level_index,
             level_transition_runtime.target_level_index, (size_t)0x002a06a0u,
             (size_t)0x0029f6b0u, 0x7b0f13c9u, 0x3a5d7811u, 0x0000000fu,
             0x55aa7744u, 4u, 913u, bitmap_palette_source_hash);
    CHECK(!theron_v1_runtime_bind_track02_bitmap_palette_source(
        &level_transition_runtime, bitmap_palette_source_trace,
        &bitmap_palette_source));
    snprintf(bitmap_palette_source_trace, sizeof(bitmap_palette_source_trace),
             "theron_track02_bitmap_palette_source "
             "same_capture_as_level_transition=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "selected_dungeon_index=0x%08x "
             "source_level_index=0x%08x "
             "target_level_index=0x%08x "
             "palette_raw_offset=%zu "
             "palette_user_data_offset=%zu "
             "palette_payload_checksum=0x%08x "
             "palette_decoded_checksum=0x%08x "
             "bitmap_route_mask=0x%08x "
             "bitmap_atlas_checksum=0x%08x "
             "bitmap_atlas_route_count=0x%08x "
             "bitmap_atlas_nonzero_pixel_count=0x%08x "
             "bitmap_palette_source_hash=0x%08x "
             "palette_window_source_bound=1 "
             "bitmap_route_source_bound=1 "
             "palette_decode_verified=0 "
             "bitmap_decode_verified=0 "
             "pixel_output_verified=0 "
             "m11_render_allowed=1 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             level_transition_runtime.record,
             level_transition_runtime.selected_dungeon_index,
             level_transition_runtime.source_level_index,
             level_transition_runtime.target_level_index, (size_t)0x002a06a0u,
             (size_t)0x0029f6b0u, 0x7b0f13c9u, 0x3a5d7811u, 0x0000000fu,
             0x55aa7744u, 4u, 913u, bitmap_palette_source_hash);
    CHECK(!theron_v1_runtime_bind_track02_bitmap_palette_source(
        &level_transition_runtime, bitmap_palette_source_trace,
        &bitmap_palette_source));

    snprintf(object_gameplay_semantics_trace,
             sizeof(object_gameplay_semantics_trace),
             "theron_track02_object_gameplay_semantics "
             "same_capture_as_placement_state=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "runtime_kind_low_mask=0x%08x "
             "runtime_kind_quest_item_seen=0 "
             "object_runtime_state_hash=0x%08x "
             "object_kind_semantics_proven=1 "
             "flags_low_bits_state_bound=1 "
             "argument_quantity_bound=0 "
             "object_flags_preserved=1 "
             "all_selected_records_runtime_mappable=1 "
             "world_object_publish_allowed=1 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             object_placement_state.record,
             object_placement_state.consumer_trace_checksum,
             object_placement_state.selected_dungeon_index,
             object_placement_state.selected_level_index,
             object_placement_state.object_table_route_hash,
             object_placement_state.loader_route_pair_hash,
             object_placement_state.object_placement_state_hash,
             object_placement_state.selected_level_record_count,
             object_placement_state.selected_level_record_hash,
             object_placement_state.selected_level_position_hash,
             runtime_kind_low_mask,
             object_runtime_state_hash);
    CHECK(!theron_v1_runtime_bind_track02_object_gameplay_semantics(
        &object_placement_state, &object_table, object_gameplay_semantics_trace,
        &object_gameplay_semantics));
    CHECK(!object_gameplay_semantics.valid);

    object_table.records[0].kind = 0x7fu;
    snprintf(object_gameplay_semantics_trace,
             sizeof(object_gameplay_semantics_trace),
             "theron_track02_object_gameplay_semantics "
             "same_capture_as_placement_state=1 "
             "track02_variant=us_bin "
             "record=0x%08x "
             "consumer_trace_checksum=0x%08x "
             "selected_dungeon_index=0x%08x "
             "selected_level_index=0x%08x "
             "object_table_route_hash=0x%08x "
             "loader_route_pair_hash=0x%08x "
             "object_placement_state_hash=0x%08x "
             "selected_level_record_count=%zu "
             "selected_level_record_hash=0x%08x "
             "selected_level_position_hash=0x%08x "
             "runtime_kind_low_mask=0x%08x "
             "runtime_kind_quest_item_seen=0 "
             "object_runtime_state_hash=0x%08x "
             "object_kind_semantics_proven=1 "
             "flags_low_bits_state_bound=1 "
             "argument_quantity_bound=1 "
             "object_flags_preserved=1 "
             "all_selected_records_runtime_mappable=1 "
             "world_object_publish_allowed=1 "
             "dungeon_runtime_admission_allowed=0 "
             "dungeon_draw_allowed=0 "
             "fallback_visuals_allowed=0",
             object_placement_state.record,
             object_placement_state.consumer_trace_checksum,
             object_placement_state.selected_dungeon_index,
             object_placement_state.selected_level_index,
             object_placement_state.object_table_route_hash,
             object_placement_state.loader_route_pair_hash,
             object_placement_state.object_placement_state_hash,
             object_placement_state.selected_level_record_count,
             object_placement_state.selected_level_record_hash,
             object_placement_state.selected_level_position_hash,
             runtime_kind_low_mask,
             object_runtime_state_hash);
    CHECK(!theron_v1_runtime_bind_track02_object_gameplay_semantics(
        &object_placement_state, &object_table, object_gameplay_semantics_trace,
        &object_gameplay_semantics));
    object_table.records[0].kind = THERON_OBJTYPE_DOOR;
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
