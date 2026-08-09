#include "theron_v1_runtime_admission.h"
#include "theron_v1_startup_media.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int build_payload_receipt(
    Theron_V1RawLoaderTraceGamePayloadReceipt *out)
{
    enum { source_lba = 4165u, raw_record = source_lba - 3009u };
    static const char capture[] =
        "source=mednafen-pce-instrumented-cd\n"
        "main_ram_loader_e009_dispatch sequence=7 logical_pc=3840 physical_pc=1f1840 a=20 x=00 y=00\n"
        "pce_cd_register_write cpu_pc=e90d physical=1801 data=81\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=08\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=00\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=10\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=45\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=01\n"
        "pce_cd_register_write cpu_pc=e981 physical=1801 data=00\n"
        "scsi_read_command generation=4 opcode=08 cdb=080010450100 start_lba=4165 sector_count=1\n"
        "pce_cd_fifo_origin_main_ram_receipt generation=4 source_lba=4165 source_offset=17 fifo_sequence=42 reader_pc=e98a logical_destination=2300 physical_destination=1f2300 writer_pc=3844 writer_physical_pc=1f1844 value=5a\n"
        "pce_cd_fifo_origin_main_ram_consumer sequence=1 generation=4 source_lba=4165 source_offset=17 fifo_sequence=42 logical_address=2300 physical_address=1f2300 value=5a reader_pc=3900 reader_physical_pc=1f1900\n";
    size_t raw_size = (raw_record + 1u) * THERON_TRACK02_RAW_SECTOR_BYTES;
    uint8_t *raw = (uint8_t *)calloc(raw_size, 1u);
    int ok;

    if (!raw) {
        return 0;
    }
    raw[(size_t)raw_record * THERON_TRACK02_RAW_SECTOR_BYTES + 17u] = 0x5au;
    ok = theron_v1_raw_loader_trace_bind_game_owned_fifo_payload(
        capture, raw, raw_size, THERON_TRACK02_MD5_US_BIN, out);
    free(raw);
    return ok;
}

static void build_bounded_route_receipt(
    Theron_V1StartupAllDungeonRouteReceipt *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->real_data_capture_ready = 0;
    out->capture_count = 1;
    out->dungeon_mask = 0x01u;
    out->semantic_level_count = 1;
    out->exact_level_semantics_ready = 0;
    out->exact_object_semantics_ready = 0;
    out->no_fallback_semantic_role_mask =
        (1u << THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE) |
        (1u << THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE) |
        (1u << THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE);
    out->object_table_no_fallback_ready = 1;
    out->object_table_blocked_anchor_mask = 0x07u;
    out->object_table_blocked_anchor_count = 3;
    out->nonstartup_level_no_fallback_ready = 1;
    out->nonstartup_level_blocked_anchor_mask = 0x07u;
    out->nonstartup_level_blocked_anchor_count = 3;
    out->startup_level_blocked_anchor_mask = 0x06u;
    out->startup_level_blocked_anchor_count = 2;
    out->object_table_anchor_binding_status[0] =
        THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
    out->startup_level_anchor_status[0] = THERON_TRACK02_LEVEL_HANDOFF_OK;
    out->startup_level_anchor_status[1] = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    out->startup_level_anchor_status[2] = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    out->startup_level_anchor_raw_offsets[0] = 0x7015b4u;
    out->startup_level_anchor_user_data_offsets[0] = 0x622650u;
    out->startup_level_anchor_user_data_valid[0] = 1;
    out->startup_level_anchor_width[0] = 32u;
    out->startup_level_anchor_height[0] = 27u;
    out->startup_level_anchor_seed[0] = 0x0108e938u;
    out->startup_level_anchor_level_index[0] = 0x0026u;
    out->object_table_route_hash = 0x3a210077u;
    out->level_route_hash = 0x4b5100a1u;
    out->object_route_hash = 0x22c0ffeeu;
    out->route_hash = 0x6d3a91c0u;
}

static void build_nonstartup_level_route_receipt(
    Theron_Track02LevelRouteReceipt *out,
    const Theron_V1StartupAllDungeonRouteReceipt *route)
{
    theron_v1_track02_level_route_receipt_init(out);
    out->valid = 1;
    out->verified_track02 = 1;
    out->signal_status = THERON_TRACK02_SIGNAL_OK;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    out->descriptor_route_ready = 1;
    out->descriptor_anchor_count = 3u;
    out->descriptor_anchor_mask = 0x07u;
    out->startup_level_route_ready = 1;
    out->startup_level_route_count = 1u;
    out->startup_level_route_mask = 0x01u;
    out->startup_level_blocked_anchor_count = 2u;
    out->startup_level_blocked_anchor_mask = 0x06u;
    out->startup_level_anchor_status[0] = THERON_TRACK02_LEVEL_HANDOFF_OK;
    out->startup_level_anchor_status[1] = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    out->startup_level_anchor_status[2] = THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL;
    out->startup_level_anchor_raw_offsets[0] =
        (size_t)route->startup_level_anchor_raw_offsets[0];
    out->startup_level_anchor_user_data_offsets[0] =
        (size_t)route->startup_level_anchor_user_data_offsets[0];
    out->startup_level_anchor_user_data_valid[0] = 1;
    out->startup_level_anchor_width[0] =
        route->startup_level_anchor_width[0];
    out->startup_level_anchor_height[0] =
        route->startup_level_anchor_height[0];
    out->startup_level_anchor_seed[0] =
        route->startup_level_anchor_seed[0];
    out->startup_level_anchor_level_index[0] =
        route->startup_level_anchor_level_index[0];
    out->startup_raw_offset =
        (size_t)route->startup_level_anchor_raw_offsets[0];
    out->startup_user_data_offset =
        (size_t)route->startup_level_anchor_user_data_offsets[0];
    out->startup_user_data_offset_valid = 1;
    out->startup_header_width = route->startup_level_anchor_width[0];
    out->startup_header_height = route->startup_level_anchor_height[0];
    out->startup_header_seed = route->startup_level_anchor_seed[0];
    out->startup_header_level_index =
        route->startup_level_anchor_level_index[0];
    out->semantic_role_mask =
        (1u << THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE) |
        (1u << THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE) |
        (1u << THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE);
    out->startup_level_grid_record_ready = 1;
    out->level_grid_role_mapped = 1;
    out->nonstartup_level_candidate_count = 3u;
    out->nonstartup_level_candidate_anchor_mask = 0x07u;
    out->nonstartup_level_candidate_sample_count[0] = 1u;
    out->nonstartup_level_candidate_sample_entry_index[0][0] = 6u;
    out->nonstartup_level_candidate_sample_raw_offsets[0][0] = 0x701e06u;
    out->nonstartup_level_candidate_sample_user_data_offsets[0][0] =
        0x622ea2u;
    out->nonstartup_level_candidate_sample_user_data_valid[0][0] = 1;
    out->nonstartup_level_candidate_sample_byte_counts[0][0] = 0x0400u;
    out->nonstartup_level_candidate_sample_descriptor_delta[0][0] = 0x29cu;
    out->nonstartup_level_candidate_sample_hash[0][0] = 0x5a77c001u;
    out->nonstartup_level_candidate_entry_index[0] = 6u;
    out->nonstartup_level_candidate_raw_offsets[0] = 0x701e06u;
    out->nonstartup_level_candidate_user_data_offsets[0] = 0x622ea2u;
    out->nonstartup_level_candidate_user_data_valid[0] = 1;
    out->nonstartup_level_candidate_byte_counts[0] = 0x0400u;
    out->nonstartup_level_candidate_nonzero_byte_counts[0] = 16u;
    out->nonstartup_level_candidate_header_width[0] = 0x4142u;
    out->nonstartup_level_candidate_header_height[0] = 0x4344u;
    out->nonstartup_level_candidate_header_seed[0] = 0x45464748u;
    out->nonstartup_level_candidate_header_level_index[0] = 0x494au;
    out->nonstartup_level_candidate_hash[0] = 0x5a77c001u;
    out->nonstartup_level_candidate_descriptor_delta[0] = 0x29cu;
    out->nonstartup_level_candidate_after_descriptor[0] = 1;
    out->nonstartup_level_candidate_entry_role[0] =
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA;
    out->nonstartup_level_candidate_window_kind[0] =
        THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
    out->nonstartup_level_blocked_anchor_count = 3u;
    out->nonstartup_level_blocked_anchor_mask = 0x07u;
    out->nonstartup_level_decode_ready = 0;
    out->blocked_for_missing_nonstartup_level_evidence = 1;
    out->fallback_visuals_allowed = 0;
    out->route_hash = route->level_route_hash;
}

static void build_object_table_route_receipt(
    Theron_Track02ObjectTableRouteReceipt *out,
    const Theron_V1StartupAllDungeonRouteReceipt *route)
{
    theron_v1_track02_object_table_route_receipt_init(out);
    out->valid = 1;
    out->verified_track02 = 1;
    out->signal_status = THERON_TRACK02_SIGNAL_OK;
    out->variant = THERON_TRACK02_VARIANT_US_BIN;
    out->descriptor_route_ready = 1;
    out->descriptor_anchor_count = 3u;
    out->descriptor_anchor_mask = 0x07u;
    out->descriptor_entries_bound = 27u;
    out->semantic_role_mask =
        (1u << THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE) |
        (1u << THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE) |
        (1u << THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE);
    out->descriptor_table_semantic_count = 3u;
    out->descriptor_table_semantic_anchor_count = 3u;
    out->descriptor_table_semantic_anchor_mask = 0x07u;
    out->object_table_role_mapped = 0;
    out->object_table_candidate_count = 3u;
    out->object_table_candidate_anchor_mask = 0x07u;
    out->object_table_candidate_entry_index[0] = 6u;
    out->object_table_candidate_raw_offsets[0] = 0x701e06u;
    out->object_table_candidate_user_data_offsets[0] = 0x622ea2u;
    out->object_table_candidate_user_data_valid[0] = 1;
    out->object_table_candidate_byte_counts[0] = 0x0400u;
    out->object_table_candidate_nonzero_byte_counts[0] = 16u;
    out->object_table_candidate_header_width[0] = 0x4142u;
    out->object_table_candidate_header_height[0] = 0x4344u;
    out->object_table_candidate_header_seed[0] = 0x45464748u;
    out->object_table_candidate_header_level_index[0] = 0x494au;
    out->object_table_candidate_header_matches_startup_shape[0] = 0;
    out->object_table_candidate_hash[0] = 0x5a77c001u;
    out->object_table_candidate_descriptor_delta[0] = 0x29cu;
    out->object_table_candidate_after_descriptor[0] = 1;
    out->object_table_candidate_entry_role[0] =
        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA;
    out->object_table_candidate_window_kind[0] =
        THERON_TRACK02_DESCRIPTOR_WINDOW_DATA;
    out->object_table_blocked_anchor_count = 3u;
    out->object_table_blocked_anchor_mask = 0x07u;
    out->object_table_anchor_binding_status[0] =
        THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND;
    out->object_table_anchor_hash[0] = 0x06000000u;
    out->object_table_declared_record_count[0] = 0u;
    out->object_table_record_count[0] = 0u;
    out->object_table_required_byte_count[0] = 0u;
    out->object_table_reject_reason[0] =
        THERON_TRACK02_OBJECT_TABLE_REJECT_NONE;
    out->object_table_decode_ready = 0;
    out->blocked_for_missing_real_object_evidence = 1;
    out->fallback_visuals_allowed = 0;
    out->route_hash = route->object_table_route_hash;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    long length;
    uint8_t *data;

    if (out_data) {
        *out_data = NULL;
    }
    if (out_size) {
        *out_size = 0u;
    }
    if (!path || !path[0] || !out_data || !out_size) {
        return 0;
    }
    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    length = ftell(file);
    if (length <= 0) {
        fclose(file);
        return 0;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data) {
        fclose(file);
        return 0;
    }
    if (fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)length;
    return 1;
}

static uint32_t mix_hash32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t bitmap_palette_source_hash_for_probe(
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *runtime,
    size_t palette_raw_offset,
    size_t palette_user_data_offset,
    uint32_t palette_payload_checksum,
    uint32_t palette_decoded_checksum,
    uint32_t bitmap_route_mask,
    uint32_t bitmap_atlas_checksum,
    uint32_t bitmap_atlas_route_count,
    uint32_t bitmap_atlas_nonzero_pixel_count)
{
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

static int parse_env_u32(const char *name, uint32_t *out)
{
    const char *text = getenv(name);
    char *end = NULL;
    unsigned long value;

    if (out) {
        *out = 0u;
    }
    if (!name || !out || !text || !text[0]) {
        return 0;
    }
    value = strtoul(text, &end, 0);
    if (!end || *end != '\0' || value > 0xfffffffful) {
        return 0;
    }
    *out = (uint32_t)value;
    return 1;
}

static int resolve_optional_real_us_track02_path(char *out_path, size_t out_cap)
{
    const char *raw_path = getenv("FIRESTAFF_THERON_TRACK02_US_BIN");
    const char *legacy_raw_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *cue_path = getenv("FIRESTAFF_THERON_TRACK02_US_CUE");
    const char *legacy_cue_path = getenv("FIRESTAFF_THERON_TRACK02_CUE");

    if (!out_path || out_cap == 0u) {
        return 0;
    }
    out_path[0] = '\0';
    if (raw_path && raw_path[0]) {
        snprintf(out_path, out_cap, "%s", raw_path);
        return 1;
    }
    if (legacy_raw_path && legacy_raw_path[0]) {
        snprintf(out_path, out_cap, "%s", legacy_raw_path);
        return 1;
    }
    if (cue_path && cue_path[0]) {
        return theron_v1_track02_resolve_media_path(cue_path, out_path) ==
            THERON_TRACK02_SIGNAL_OK;
    }
    if (legacy_cue_path && legacy_cue_path[0]) {
        return theron_v1_track02_resolve_media_path(legacy_cue_path, out_path) ==
            THERON_TRACK02_SIGNAL_OK;
    }
    return 0;
}

static int probe_optional_original_consumer_trace_corpus(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap)
{
    const char *path = getenv("FIRESTAFF_THERON_ORIGINAL_CONSUMER_TRACE");
    uint8_t *trace_data = NULL;
    size_t trace_size = 0u;
    char *trace_text = NULL;
    uint32_t record;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    Theron_V1Track02Post3800ConsumerTraceFacts facts;
    int ok;

    if (!path || !path[0]) {
        return 1;
    }
    if (!gap || !gap->valid ||
        !parse_env_u32("FIRESTAFF_THERON_ORIGINAL_CONSUMER_RECORD", &record) ||
        !parse_env_u32(
            "FIRESTAFF_THERON_ORIGINAL_CONSUMER_PAYLOAD_CHECKSUM",
            &payload_checksum) ||
        !parse_env_u32(
            "FIRESTAFF_THERON_ORIGINAL_CONSUMER_LEVEL_ENVELOPE_CHECKSUM",
            &level_envelope_checksum) ||
        !parse_env_u32(
            "FIRESTAFF_THERON_ORIGINAL_CONSUMER_POST_ENVELOPE_CHECKSUM",
            &post_envelope_checksum) ||
        !read_file(path, &trace_data, &trace_size)) {
        return 0;
    }
    trace_text = (char *)malloc(trace_size + 1u);
    if (!trace_text) {
        free(trace_data);
        return 0;
    }
    memcpy(trace_text, trace_data, trace_size);
    trace_text[trace_size] = '\0';
    ok = theron_v1_runtime_track02_original_consumer_trace_facts_from_capture(
        trace_text,
        gap,
        record,
        payload_checksum,
        level_envelope_checksum,
        post_envelope_checksum,
        &facts);
    if (ok) {
        ok = facts.authenticated_original_trace &&
             facts.post_3800_execution_observed &&
             facts.same_capture_as_loader_payload &&
             facts.track02_variant == THERON_TRACK02_VARIANT_US_BIN &&
             facts.record == record &&
             facts.payload_checksum == payload_checksum &&
             facts.level_envelope_checksum == level_envelope_checksum &&
             facts.post_envelope_checksum == post_envelope_checksum &&
             facts.consumer_trace_checksum != 0u &&
             facts.dungeon_record_consumer_observed &&
             facts.object_table_consumer_observed &&
             facts.bitmap_consumer_observed &&
             facts.palette_consumer_observed &&
             !facts.synthetic_dungeon_promoted &&
             !facts.synthetic_object_table_promoted &&
             !facts.synthetic_bitmap_promoted &&
             !facts.synthetic_palette_promoted &&
             !facts.fallback_visuals_observed &&
             !facts.fallback_visuals_allowed;
    }
    free(trace_text);
    free(trace_data);
    return ok;
}

static int probe_optional_object_dungeon_handoff_corpus(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const uint8_t *track02,
    size_t track02_size)
{
    const char *path = getenv("FIRESTAFF_THERON_OBJECT_DUNGEON_TRACE");
    uint8_t *trace_data = NULL;
    size_t trace_size = 0u;
    char *trace_text = NULL;
    uint32_t record;
    uint32_t payload_checksum;
    uint32_t level_envelope_checksum;
    uint32_t post_envelope_checksum;
    Theron_V1Track02Post3800ConsumerTraceFacts facts;
    Theron_V1Track02LoaderSemanticGateReceipt loader_gate;
    Theron_V1Track02ObjectDungeonConsumerGrammarReceipt grammar;
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt binding;
    Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt raw_handoff;
    Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt object_level;
    Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt level_evidence;
    Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt object_evidence;
    Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt handoff_evidence;
    Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt field_boundary;
    Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt decoder_boundary;
    Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt route_boundary;
    Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt facts_handoff;
    Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt selection;
    Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt table_binding;
    Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt loader_route;
    Theron_V1RuntimeTrack02ObjectPlacementStateReceipt placement_state;
    Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt gameplay_semantics;
    Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt world_handoff;
    Theron_Track02ObjectTable objects;
    Theron_V1_World world;
    int ok = 1;

    if (!path || !path[0]) {
        return 1;
    }
    if (!gap || !gap->valid ||
        !parse_env_u32("FIRESTAFF_THERON_OBJECT_DUNGEON_RECORD", &record) ||
        !parse_env_u32(
            "FIRESTAFF_THERON_OBJECT_DUNGEON_PAYLOAD_CHECKSUM",
            &payload_checksum) ||
        !parse_env_u32(
            "FIRESTAFF_THERON_OBJECT_DUNGEON_LEVEL_ENVELOPE_CHECKSUM",
            &level_envelope_checksum) ||
        !parse_env_u32(
            "FIRESTAFF_THERON_OBJECT_DUNGEON_POST_ENVELOPE_CHECKSUM",
            &post_envelope_checksum) ||
        !read_file(path, &trace_data, &trace_size)) {
        return 0;
    }
    trace_text = (char *)malloc(trace_size + 1u);
    if (!trace_text) {
        free(trace_data);
        return 0;
    }
    memcpy(trace_text, trace_data, trace_size);
    trace_text[trace_size] = '\0';
    if (!theron_v1_runtime_track02_object_dungeon_trace_facts_from_capture(
            trace_text,
            gap,
            record,
            payload_checksum,
            level_envelope_checksum,
            post_envelope_checksum,
            &facts) ||
        !facts.authenticated_original_trace ||
        !facts.post_3800_execution_observed ||
        !facts.same_capture_as_loader_payload ||
        facts.track02_variant != THERON_TRACK02_VARIANT_US_BIN ||
        facts.record != record ||
        facts.payload_checksum != payload_checksum ||
        facts.level_envelope_checksum != level_envelope_checksum ||
        facts.post_envelope_checksum != post_envelope_checksum ||
        facts.consumer_trace_checksum == 0u ||
        !facts.dungeon_record_consumer_observed ||
        !facts.object_table_consumer_observed ||
        facts.bitmap_consumer_observed ||
        facts.palette_consumer_observed ||
        facts.synthetic_dungeon_promoted ||
        facts.synthetic_object_table_promoted ||
        facts.synthetic_bitmap_promoted ||
        facts.synthetic_palette_promoted ||
        facts.fallback_visuals_observed ||
        facts.fallback_visuals_allowed) {
        ok = 0;
    }
    memset(&loader_gate, 0, sizeof(loader_gate));
    loader_gate.valid = 1;
    loader_gate.no_fallback = 1;
    loader_gate.real_payload_available = 1;
    loader_gate.level_envelope_available = 1;
    loader_gate.post_envelope_available = 1;
    loader_gate.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    loader_gate.record = record;
    loader_gate.payload_checksum = payload_checksum;
    loader_gate.level_envelope_checksum = level_envelope_checksum;
    loader_gate.post_envelope_checksum = post_envelope_checksum;
    if (ok && !theron_v1_track02_loader_intake_object_dungeon_consumer_grammar_gate(
            &loader_gate, &facts, &grammar)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_original_object_dungeon_consumer_trace(
            gap, &grammar, &binding) ||
        !binding.valid ||
        !binding.nonstartup_level_consumer_bound ||
        !binding.object_table_consumer_bound ||
        binding.bitmap_consumer_bound ||
        binding.palette_consumer_bound ||
        binding.render_asset_admission_allowed ||
        binding.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
            gap, &binding, &raw_handoff) ||
        !raw_handoff.valid ||
        !raw_handoff.raw_sector_user_data_bound ||
        !raw_handoff.nonstartup_dungeon_path_ready ||
        raw_handoff.bitmap_route_bound ||
        raw_handoff.palette_binding_verified ||
        raw_handoff.rgba_output_allowed ||
        raw_handoff.dungeon_draw_allowed ||
        raw_handoff.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_object_level_admission(
            &raw_handoff, &grammar, &object_level) ||
        !object_level.valid ||
        !object_level.raw_sector_user_data_bound ||
        !object_level.dungeon_record_grammar_proven ||
        !object_level.object_table_grammar_proven ||
        !object_level.nonstartup_level_admission_allowed ||
        !object_level.object_table_admission_allowed ||
        !object_level.exact_level_fields_blocked ||
        !object_level.exact_object_fields_blocked ||
        object_level.bitmap_route_bound ||
        object_level.palette_binding_verified ||
        object_level.rgba_output_allowed ||
        object_level.dungeon_draw_allowed ||
        object_level.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_nonstartup_level_record_evidence(
            &object_level, trace_text, &level_evidence) ||
        !level_evidence.valid ||
        !level_evidence.source_nonstartup_level_bytes_bound ||
        !level_evidence.nonstartup_level_record_route_observed ||
        !level_evidence.exact_level_fields_blocked ||
        level_evidence.dungeon_draw_allowed ||
        level_evidence.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_object_table_route_evidence(
            &object_level, trace_text, &object_evidence) ||
        !object_evidence.valid ||
        !object_evidence.source_object_table_bytes_bound ||
        !object_evidence.object_table_route_observed ||
        !object_evidence.object_table_layout_blocked ||
        !object_evidence.exact_object_fields_blocked ||
        object_evidence.dungeon_draw_allowed ||
        object_evidence.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_level_object_handoff_evidence(
            &level_evidence, &object_evidence, &handoff_evidence) ||
        !handoff_evidence.valid ||
        !handoff_evidence.level_object_pair_route_observed ||
        handoff_evidence.dungeon_draw_allowed ||
        handoff_evidence.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_level_object_field_boundary(
            &handoff_evidence, trace_text, &field_boundary) ||
        !field_boundary.valid ||
        !field_boundary.field_decoder_required ||
        !field_boundary.exact_level_fields_blocked ||
        !field_boundary.exact_object_fields_blocked ||
        field_boundary.dungeon_route_handoff_allowed ||
        field_boundary.dungeon_draw_allowed ||
        field_boundary.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_reviewed_field_decoder_boundary(
            &field_boundary, "theron_track02_level_object_fields_v1",
            trace_text, &decoder_boundary) ||
        !decoder_boundary.valid ||
        !decoder_boundary.reviewed_decoder_source_bound ||
        decoder_boundary.field_decoder_execution_allowed ||
        decoder_boundary.dungeon_route_handoff_allowed ||
        decoder_boundary.dungeon_draw_allowed ||
        decoder_boundary.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_dungeon_route_admission_boundary(
            &decoder_boundary, trace_text, &route_boundary) ||
        !route_boundary.valid ||
        !route_boundary.real_track02_level_object_boundary_bound ||
        !route_boundary.dungeon_route_review_required ||
        route_boundary.field_decoder_execution_allowed ||
        route_boundary.dungeon_route_handoff_allowed ||
        route_boundary.dungeon_runtime_admission_allowed ||
        route_boundary.dungeon_draw_allowed ||
        route_boundary.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_level_object_facts_handoff(
            &route_boundary, &field_boundary, &facts_handoff) ||
        !facts_handoff.valid ||
        !facts_handoff.dungeon_route_review_required ||
        facts_handoff.field_decoder_execution_allowed ||
        facts_handoff.dungeon_route_handoff_allowed ||
        facts_handoff.dungeon_runtime_admission_allowed ||
        facts_handoff.dungeon_draw_allowed ||
        facts_handoff.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok &&
        (!theron_v1_runtime_bind_track02_dungeon_selection_level_record_boundary(
            &facts_handoff, trace_text, &selection) ||
        !selection.valid ||
        selection.selected_dungeon_index == 0u ||
        !selection.level_record_route_bound ||
        !selection.object_table_layout_blocked ||
        selection.field_decoder_execution_allowed ||
        selection.dungeon_runtime_admission_allowed ||
        selection.dungeon_draw_allowed ||
        selection.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok &&
        (!theron_v1_runtime_bind_track02_dungeon_object_level_table_binding(
            &selection, trace_text, &table_binding) ||
        !table_binding.valid ||
        !table_binding.object_table_route_bound ||
        !table_binding.object_table_layout_review_required ||
        table_binding.field_decoder_execution_allowed ||
        table_binding.dungeon_route_handoff_allowed ||
        table_binding.dungeon_runtime_admission_allowed ||
        table_binding.dungeon_draw_allowed ||
        table_binding.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_level_object_loader_route(
            &table_binding, trace_text, &loader_route) ||
        !loader_route.valid ||
        !loader_route.loader_route_source_windows_bound ||
        !loader_route.loader_route_review_required ||
        loader_route.field_decoder_execution_allowed ||
        loader_route.dungeon_route_handoff_allowed ||
        loader_route.dungeon_runtime_admission_allowed ||
        loader_route.dungeon_draw_allowed ||
        loader_route.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!track02 ||
        gap->first_container_raw_offset >= track02_size ||
        gap->first_container_user_data_byte_count >
            track02_size - gap->first_container_raw_offset ||
        theron_v1_track02_read_object_table(
            track02 + gap->first_container_raw_offset,
            gap->first_container_user_data_byte_count,
            &objects) != THERON_TRACK02_SEMANTIC_BINDING_OK ||
        !theron_v1_runtime_bind_track02_object_placement_state(
            &loader_route, &objects, trace_text, &placement_state) ||
        !placement_state.valid ||
        !placement_state.object_placement_bytes_bound ||
        !placement_state.object_state_low_bits_bound ||
        !placement_state.object_kind_semantics_review_required ||
        placement_state.world_object_publish_allowed ||
        placement_state.dungeon_runtime_admission_allowed ||
        placement_state.dungeon_draw_allowed ||
        placement_state.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok && (!theron_v1_runtime_bind_track02_object_gameplay_semantics(
            &placement_state, &objects, trace_text, &gameplay_semantics) ||
        !gameplay_semantics.valid ||
        !gameplay_semantics.object_kind_semantics_proven ||
        !gameplay_semantics.flags_low_bits_state_bound ||
        !gameplay_semantics.argument_quantity_bound ||
        !gameplay_semantics.object_flags_preserved ||
        !gameplay_semantics.all_selected_records_runtime_mappable ||
        !gameplay_semantics.world_object_publish_allowed ||
        gameplay_semantics.dungeon_runtime_admission_allowed ||
        gameplay_semantics.dungeon_draw_allowed ||
        gameplay_semantics.fallback_visuals_allowed)) {
        ok = 0;
    }
    if (ok) {
        theron_v1_world_init(&world);
        if (gameplay_semantics.selected_dungeon_index == 0u ||
            gameplay_semantics.selected_dungeon_index > THERON_DUNGEON_COUNT ||
            gameplay_semantics.selected_level_index >=
                THERON_MAX_LEVELS_PER_DUNGEON) {
            ok = 0;
        } else {
            int dungeon_slot =
                (int)gameplay_semantics.selected_dungeon_index - 1;
            int selected_level =
                (int)gameplay_semantics.selected_level_index;
            world.current_dungeon =
                (int)gameplay_semantics.selected_dungeon_index;
            world.current_level = selected_level;
            world.level_loaded[dungeon_slot][selected_level] = 1;
            world.levels[dungeon_slot][selected_level].width = 32;
            world.levels[dungeon_slot][selected_level].height = 27;
            if (!theron_v1_runtime_publish_track02_object_gameplay_state(
                    &world,
                    (Theron_DungeonID)gameplay_semantics.selected_dungeon_index,
                    &gameplay_semantics, &objects, &world_handoff) ||
                !world_handoff.valid ||
                !world_handoff.world_mutated ||
                world_handoff.placed_object_count <= 0 ||
                world_handoff.dungeon_runtime_admission_allowed ||
                world_handoff.dungeon_draw_allowed ||
                world_handoff.fallback_visuals_allowed) {
                ok = 0;
            }
        }
    }
    if (ok) {
        grammar.bitmap_route_bound = 1;
        if (theron_v1_runtime_bind_track02_object_level_admission(
                &raw_handoff, &grammar, &object_level) ||
            object_level.valid) {
            ok = 0;
        }
    }
    free(trace_text);
    free(trace_data);
    return ok;
}

static int probe_real_track02_capture_producer(
    const Theron_V1RuntimeTrack02ConsumerSemanticReceipt *base_consumer)
{
    char path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    uint8_t *data = NULL;
    size_t size = 0u;
    Theron_Track02LevelRouteReceipt level_route;
    Theron_Track02ObjectTableRouteReceipt object_route;
    Theron_V1RuntimeTrack02ConsumerSemanticReceipt consumer;
    Theron_V1RuntimeTrack02RenderAssetProof proof;
    Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt gap;
    Theron_V1Track02Post3800ConsumerSemanticReceipt original_consumer;
    Theron_V1Track02Post3800ConsumerTraceFacts trace_facts;
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt binding;
    Theron_Track02StartupBitmapCatalog bitmap_catalog;
    Theron_Track02StartupBitmapAtlas bitmap_atlas;
    Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt transition_runtime;
    Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt bitmap_source;
    Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt decode_vector;
    Theron_V1RuntimeTrack02M11SoulRoomConsumptionReceipt m11_consumption;
    Theron_V1RuntimeTrack02M11LevelConsumptionReceipt m11_level_consumption;
    Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt m11_draw_route;
    Theron_V1RuntimeTrack02Level1DrawBlockerReceipt level1_draw_blocker;
    Theron_StartupMediaStateReceipt startup_media;
    Theron_V1_World world;
    char bitmap_source_trace[2048];
    uint32_t bitmap_source_hash;
    int ok = 1;
    int gap_ok;

    if (!resolve_optional_real_us_track02_path(path, sizeof(path))) {
        return 1;
    }
    if (!read_file(path, &data, &size) ||
        size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u) {
        free(data);
        return 0;
    }
    int level_ok = theron_v1_track02_capture_level_route_receipt(
        data, size, THERON_TRACK02_MD5_US_BIN, &level_route);
    int object_ok = theron_v1_track02_capture_object_table_route_receipt(
        data, size, THERON_TRACK02_MD5_US_BIN, &object_route);
    if (level_route.signal_status != THERON_TRACK02_SIGNAL_OK ||
        level_route.nonstartup_level_decode_ready ||
        object_ok != THERON_TRACK02_SIGNAL_OK) {
        (void)level_ok;
        free(data);
        return 0;
    }
    consumer = *base_consumer;
    consumer.level_route_hash = level_route.route_hash;
    consumer.object_table_route_hash = object_route.route_hash;
    theron_v1_runtime_track02_render_asset_proof_init(&proof);
    if (theron_v1_runtime_track02_render_asset_proof_from_track02_capture(
            &consumer,
            data,
            size,
            THERON_TRACK02_MD5_US_BIN,
            0x2a06a0u,
            0,
            &proof) ||
        proof.valid) {
        ok = 0;
    }
    if (theron_v1_runtime_track02_render_asset_proof_from_track02_capture(
            &consumer,
            data,
            size,
            THERON_TRACK02_MD5_US_BIN,
            0x2a06a0u,
            1,
            &proof) ||
        proof.valid) {
        ok = 0;
    }
    if (level_route.nonstartup_level_decode_ready ||
        object_route.object_table_decode_ready ||
        !level_route.blocked_for_missing_nonstartup_level_evidence ||
        !object_route.blocked_for_missing_real_object_evidence) {
        ok = 0;
    }
    gap_ok = theron_v1_runtime_track02_capture_original_data_binding_gap(
            data,
            size,
            THERON_TRACK02_MD5_US_BIN,
            0x2a06a0u,
            &gap);
    if (!gap_ok) {
        if (gap.valid ||
            gap.verified_track02_capture_consumed ||
            gap.render_asset_admission_allowed ||
            gap.fallback_visuals_allowed) {
            ok = 0;
        }
        free(data);
        return ok;
    }
    if (!gap.valid ||
        !gap.verified_track02_capture_consumed ||
        !gap.fail_closed ||
        gap.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(gap.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        gap.level_route_hash != level_route.route_hash ||
        gap.object_table_route_hash != object_route.route_hash ||
        gap.palette_raw_offset != 0x2a06a0u ||
        gap.palette_user_data_offset == 0u ||
        gap.palette_payload_checksum == 0u ||
        gap.palette_decoded_checksum == 0u ||
        !gap.palette_format_valid ||
        gap.palette_semantic_binding_verified ||
        gap.palette_promotion_allowed ||
        gap.nonstartup_anchor_count != 3u ||
        gap.nonstartup_window_count == 0u ||
        gap.first_nonstartup_raw_offset == 0u ||
        gap.first_nonstartup_user_data_offset == 0u ||
        gap.first_nonstartup_byte_count == 0u ||
        gap.first_nonstartup_raw_hash == 0u ||
        gap.indexed_container_count == 0u ||
        gap.first_container_raw_offset == 0u ||
        gap.first_container_user_data_offset == 0u ||
        gap.first_container_user_data_byte_count == 0u ||
        gap.first_container_user_data_hash == 0u ||
        gap.nonstartup_level_decode_ready ||
        gap.object_table_decode_ready ||
        gap.render_asset_admission_allowed ||
        gap.fallback_visuals_allowed) {
        ok = 0;
    }
    memset(&bitmap_catalog, 0, sizeof(bitmap_catalog));
    memset(&bitmap_atlas, 0, sizeof(bitmap_atlas));
    if (theron_v1_track02_catalog_startup_bitmap_samples(
            data, size, THERON_TRACK02_MD5_US_BIN, &bitmap_catalog) !=
            THERON_TRACK02_SIGNAL_OK ||
        theron_v1_track02_build_startup_bitmap_atlas_wide(
            &bitmap_catalog, &bitmap_atlas) != THERON_TRACK02_SIGNAL_OK ||
        bitmap_atlas.route_mask == 0u ||
        bitmap_atlas.checksum == 0u ||
        bitmap_atlas.route_count == 0u ||
        bitmap_atlas.total_nonzero_pixel_count == 0u) {
        ok = 0;
    } else {
        memset(&transition_runtime, 0, sizeof(transition_runtime));
        transition_runtime.valid = 1;
        transition_runtime.level_transition_handoff_consumed = 1;
        transition_runtime.target_object_world_handoff_consumed = 1;
        transition_runtime.world_mutated = 1;
        transition_runtime.variant = THERON_TRACK02_VARIANT_US_BIN;
        snprintf(transition_runtime.track02_md5,
                 sizeof(transition_runtime.track02_md5), "%s",
                 THERON_TRACK02_MD5_US_BIN);
        transition_runtime.record = 1156u;
        transition_runtime.selected_dungeon_index = 1u;
        transition_runtime.source_level_index = 0u;
        transition_runtime.target_level_index = 1u;
        transition_runtime.transition_pending_before = 1;
        transition_runtime.transition_pending_after = 0;
        transition_runtime.level_loaded = 1;
        bitmap_source_hash = bitmap_palette_source_hash_for_probe(
            &transition_runtime, gap.palette_raw_offset,
            gap.palette_user_data_offset, gap.palette_payload_checksum,
            gap.palette_decoded_checksum, bitmap_atlas.route_mask,
            bitmap_atlas.checksum, (uint32_t)bitmap_atlas.route_count,
            (uint32_t)bitmap_atlas.total_nonzero_pixel_count);
        snprintf(bitmap_source_trace, sizeof(bitmap_source_trace),
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
                 transition_runtime.record,
                 transition_runtime.selected_dungeon_index,
                 transition_runtime.source_level_index,
                 transition_runtime.target_level_index,
                 gap.palette_raw_offset,
                 gap.palette_user_data_offset,
                 gap.palette_payload_checksum,
                 gap.palette_decoded_checksum,
                 bitmap_atlas.route_mask,
                 bitmap_atlas.checksum,
                 (uint32_t)bitmap_atlas.route_count,
                 (uint32_t)bitmap_atlas.total_nonzero_pixel_count,
                 bitmap_source_hash);
        if (!theron_v1_runtime_bind_track02_bitmap_palette_source(
                &transition_runtime, bitmap_source_trace, &bitmap_source) ||
            !bitmap_source.valid ||
            !theron_v1_runtime_decode_track02_bitmap_palette_vector(
                &bitmap_source, data, size, THERON_TRACK02_MD5_US_BIN,
                &decode_vector) ||
            !decode_vector.valid ||
            !decode_vector.palette_decode_verified ||
            !decode_vector.bitmap_decode_verified ||
            !decode_vector.pixel_output_verified ||
            decode_vector.m11_runtime_consumption_allowed ||
            decode_vector.m11_render_allowed ||
            decode_vector.dungeon_draw_allowed ||
            decode_vector.fallback_visuals_allowed ||
            decode_vector.palette_payload_checksum !=
                gap.palette_payload_checksum ||
            decode_vector.palette_decoded_checksum !=
                gap.palette_decoded_checksum ||
            decode_vector.bitmap_atlas_checksum != bitmap_atlas.checksum ||
            decode_vector.bitmap_atlas_route_count !=
                bitmap_atlas.route_count ||
            decode_vector.bitmap_atlas_nonzero_pixel_count !=
                bitmap_atlas.total_nonzero_pixel_count ||
            decode_vector.first_pixel_row_hash == 0u) {
            ok = 0;
        }
        memset(&startup_media, 0, sizeof(startup_media));
        theron_v1_startup_media_capture_track02_state_receipt(
            data, size, THERON_TRACK02_MD5_US_BIN, &startup_media);
        theron_v1_world_init(&world);
        if (!theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
                &startup_media) ||
            !theron_v1_startup_media_bind_runtime_receipt(
                &world, &startup_media) ||
            !theron_v1_runtime_bind_track02_m11_soul_room_consumption(
                &decode_vector, &world, 320, 200, 0, 0, 1, 1,
                &m11_consumption) ||
            !m11_consumption.valid ||
            !m11_consumption.soul_room_level0_selected ||
            !m11_consumption.exact_indexed_atlas_consumed ||
            !m11_consumption.huc6260_palette_consumed ||
            !m11_consumption.host_presentation_allowed ||
            !m11_consumption.m11_runtime_consumption_allowed ||
            !m11_consumption.m11_render_allowed ||
            m11_consumption.dungeon_draw_allowed ||
            m11_consumption.fallback_visuals_allowed ||
            m11_consumption.route_bit !=
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM ||
            m11_consumption.source_checksum !=
                decode_vector.first_bitmap_route_checksum ||
            m11_consumption.palette_decoded_checksum !=
                decode_vector.palette_decoded_checksum ||
            world.runtime_media.level_bank.level_index != 0 ||
            world.runtime_media.level_bank.route_bit !=
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) {
            ok = 0;
        }
        if (!theron_v1_runtime_bind_track02_m11_level_consumption(
                &decode_vector, &transition_runtime, &world, 320, 200, 0, 8,
                1, 1, &m11_level_consumption) ||
            !m11_level_consumption.valid ||
            !m11_level_consumption.level_transition_runtime_consumed ||
            !m11_level_consumption.target_level_selected ||
            !m11_level_consumption.exact_indexed_atlas_consumed ||
            !m11_level_consumption.huc6260_palette_consumed ||
            !m11_level_consumption.host_presentation_allowed ||
            !m11_level_consumption.m11_runtime_consumption_allowed ||
            !m11_level_consumption.m11_render_allowed ||
            m11_level_consumption.dungeon_draw_allowed ||
            m11_level_consumption.fallback_visuals_allowed ||
            m11_level_consumption.route_bit !=
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE ||
            m11_level_consumption.target_level_index !=
                transition_runtime.target_level_index ||
            m11_level_consumption.source_checksum !=
                decode_vector.stage_bitmap_route_checksum ||
            m11_level_consumption.palette_decoded_checksum !=
                decode_vector.palette_decoded_checksum ||
            world.runtime_media.level_bank.level_index != 1 ||
            world.runtime_media.level_bank.route_bit !=
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) {
            ok = 0;
        }
        if (theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
                &m11_level_consumption, &transition_runtime, &world,
                &m11_draw_route) ||
            m11_draw_route.valid ||
            m11_draw_route.dungeon_draw_route_allowed ||
            m11_draw_route.fallback_visuals_allowed) {
            ok = 0;
        }
        if (!theron_v1_runtime_bind_track02_level1_draw_blocker(
                &m11_level_consumption, &transition_runtime, &gap, &world,
                &m11_draw_route, &level1_draw_blocker) ||
            !level1_draw_blocker.valid ||
            !level1_draw_blocker.m11_level_consumption_consumed ||
            !level1_draw_blocker.level_transition_runtime_consumed ||
            !level1_draw_blocker.original_data_binding_gap_consumed ||
            !level1_draw_blocker.world_runtime_state_inspected ||
            !level1_draw_blocker.real_track02_level1_media_bound ||
            !level1_draw_blocker.nonstartup_geometry_source_blocked ||
            !level1_draw_blocker.object_placement_source_blocked ||
            !level1_draw_blocker.loadertrace_geometry_window_missing ||
            !level1_draw_blocker.loadertrace_object_window_missing ||
            level1_draw_blocker.level1_world_geometry_loaded ||
            level1_draw_blocker.level1_object_placement_loaded ||
            !level1_draw_blocker.transition_level_loaded ||
            level1_draw_blocker.transition_target_object_count != 0 ||
            level1_draw_blocker.transition_target_thing_count != 0 ||
            level1_draw_blocker.world_object_count != 0 ||
            level1_draw_blocker.variant != THERON_TRACK02_VARIANT_US_BIN ||
            strcmp(level1_draw_blocker.track02_md5,
                   THERON_TRACK02_MD5_US_BIN) != 0 ||
            level1_draw_blocker.record != transition_runtime.record ||
            level1_draw_blocker.selected_dungeon_index !=
                transition_runtime.selected_dungeon_index ||
            level1_draw_blocker.target_level_index != 1u ||
            level1_draw_blocker.route_bit !=
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE ||
            level1_draw_blocker.media_checksum !=
                m11_level_consumption.source_checksum ||
            level1_draw_blocker.palette_decoded_checksum !=
                m11_level_consumption.palette_decoded_checksum ||
            level1_draw_blocker.nonstartup_window_count !=
                gap.nonstartup_window_count ||
            level1_draw_blocker.first_nonstartup_raw_offset !=
                gap.first_nonstartup_raw_offset ||
            level1_draw_blocker.first_nonstartup_user_data_offset !=
                gap.first_nonstartup_user_data_offset ||
            level1_draw_blocker.first_nonstartup_byte_count !=
                gap.first_nonstartup_byte_count ||
            level1_draw_blocker.first_nonstartup_raw_hash !=
                gap.first_nonstartup_raw_hash ||
            level1_draw_blocker.object_table_raw_offset !=
                gap.first_container_raw_offset ||
            level1_draw_blocker.object_table_user_data_offset !=
                gap.first_container_user_data_offset ||
            level1_draw_blocker.object_table_byte_count !=
                gap.first_container_user_data_byte_count ||
            level1_draw_blocker.object_table_raw_hash !=
                gap.first_container_user_data_hash ||
            level1_draw_blocker.dungeon_draw_route_allowed ||
            level1_draw_blocker.dungeon_pixel_blit_allowed ||
            level1_draw_blocker.fallback_visuals_allowed) {
            ok = 0;
        }
        m11_draw_route.valid = 1;
        m11_draw_route.dungeon_draw_route_allowed = 1;
        if (theron_v1_runtime_bind_track02_level1_draw_blocker(
                &m11_level_consumption, &transition_runtime, &gap, &world,
                &m11_draw_route, &level1_draw_blocker) ||
            level1_draw_blocker.valid) {
            ok = 0;
        }
        m11_draw_route.valid = 0;
        m11_draw_route.dungeon_draw_route_allowed = 0;
        decode_vector.stage_bitmap_route_checksum ^= 1u;
        if (theron_v1_runtime_bind_track02_m11_level_consumption(
                &decode_vector, &transition_runtime, &world, 320, 200, 0, 8,
                1, 1, &m11_level_consumption) ||
            m11_level_consumption.valid) {
            ok = 0;
        }
        decode_vector.stage_bitmap_route_checksum ^= 1u;
        transition_runtime.target_level_index = 0u;
        if (theron_v1_runtime_bind_track02_m11_level_consumption(
                &decode_vector, &transition_runtime, &world, 320, 200, 0, 8,
                1, 1, &m11_level_consumption) ||
            m11_level_consumption.valid) {
            ok = 0;
        }
        transition_runtime.target_level_index = 1u;
        world.runtime_media.soul_room.checksum ^= 1u;
        if (theron_v1_runtime_bind_track02_m11_soul_room_consumption(
                &decode_vector, &world, 320, 200, 0, 0, 1, 1,
                &m11_consumption) ||
            m11_consumption.valid) {
            ok = 0;
        }
        bitmap_source.palette_payload_checksum ^= 1u;
        if (theron_v1_runtime_decode_track02_bitmap_palette_vector(
                &bitmap_source, data, size, THERON_TRACK02_MD5_US_BIN,
                &decode_vector) ||
            decode_vector.valid) {
            ok = 0;
        }
        bitmap_source.palette_payload_checksum ^= 1u;
    }
    if (theron_v1_runtime_track02_original_consumer_trace_facts_from_capture(
            NULL,
            &gap,
            1156u,
            0x101u,
            0x202u,
            0x303u,
            &trace_facts) ||
        trace_facts.authenticated_original_trace ||
        trace_facts.consumer_trace_checksum != 0u) {
        ok = 0;
    }
    if (theron_v1_runtime_track02_original_consumer_trace_facts_from_capture(
            "theron_track02_original_consumer_trace\n"
            "authenticated_original_trace=1\n"
            "post_3800_execution_observed=1\n"
            "same_capture_as_loader_payload=1\n"
            "track02_variant=us_bin\n"
            "record=1156\n"
            "payload_checksum=0x00000101\n"
            "level_envelope_checksum=0x00000202\n"
            "post_envelope_checksum=0x00000303\n"
            "palette_consumer_observed=1\n"
            "dungeon_record_consumer_observed=1\n"
            "object_table_consumer_observed=1\n"
            "bitmap_consumer_observed=1\n"
            "synthetic_dungeon_promoted=0\n"
            "synthetic_object_table_promoted=0\n"
            "synthetic_bitmap_promoted=0\n"
            "synthetic_palette_promoted=0\n"
            "fallback_visuals_observed=0\n"
            "fallback_visuals_allowed=0\n",
            &gap,
            1156u,
            0x101u,
            0x202u,
            0x303u,
            &trace_facts) ||
        trace_facts.authenticated_original_trace ||
        trace_facts.consumer_trace_checksum != 0u) {
        ok = 0;
    }
    {
        char forged_summary[4096];
        int forged_summary_length = snprintf(
            forged_summary, sizeof(forged_summary),
            "source=mednafen-pce-instrumented\n"
            "main_ram_loader_e009_dispatch sequence=7\n"
            "theron_track02_original_consumer_trace\n"
            "authenticated_original_trace=1\n"
            "post_3800_execution_observed=1\n"
            "same_capture_as_loader_payload=1\n"
            "track02_variant=us_bin\n"
            "record=1156\n"
            "loader_record_user_data_offset=276\n"
            "loader_destination=14336\n"
            "loader_payload_bytes=2048\n"
            "payload_checksum=257\n"
            "level_envelope_checksum=514\n"
            "post_envelope_checksum=771\n"
            "palette_raw_offset=%zu\n"
            "nonstartup_level_raw_offset=%zu\n"
            "object_table_raw_offset=%zu\n"
            "palette_consumer_observed=1\n"
            "dungeon_record_consumer_observed=1\n"
            "object_table_consumer_observed=1\n"
            "bitmap_consumer_observed=1\n"
            "synthetic_dungeon_promoted=0\n"
            "synthetic_object_table_promoted=0\n"
            "synthetic_bitmap_promoted=0\n"
            "synthetic_palette_promoted=0\n"
            "fallback_visuals_observed=0\n"
            "fallback_visuals_allowed=0\n",
            gap.palette_raw_offset,
            gap.first_nonstartup_raw_offset,
            gap.first_container_raw_offset);
        if (forged_summary_length < 0 ||
            (size_t)forged_summary_length >= sizeof(forged_summary) ||
            theron_v1_runtime_track02_original_consumer_trace_facts_from_capture(
                forged_summary, &gap, 1156u, 0x101u, 0x202u, 0x303u,
                &trace_facts) ||
            trace_facts.authenticated_original_trace ||
            trace_facts.consumer_trace_checksum != 0u) {
            ok = 0;
        }
    }
    if (!probe_optional_original_consumer_trace_corpus(&gap)) {
        ok = 0;
    }
    if (!probe_optional_object_dungeon_handoff_corpus(&gap, data, size)) {
        ok = 0;
    }
    memset(&original_consumer, 0, sizeof(original_consumer));
    if (theron_v1_runtime_bind_track02_original_consumer_trace(
            &gap,
            &original_consumer,
            0x2a06a0u,
            gap.first_nonstartup_raw_offset,
            gap.first_container_raw_offset,
            &binding) ||
        binding.valid ||
        !binding.fail_closed_until_consumer_proven) {
        ok = 0;
    }
    original_consumer.valid = 1;
    original_consumer.no_fallback = 1;
    original_consumer.original_consumer_trace_bound = 1;
    original_consumer.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    original_consumer.record = 1156u;
    original_consumer.payload_checksum = 1u;
    original_consumer.level_envelope_checksum = 2u;
    original_consumer.post_envelope_checksum = 3u;
    original_consumer.consumer_trace_checksum = 4u;
    original_consumer.dungeon_record_semantics_proven = 1;
    original_consumer.object_table_semantics_proven = 1;
    original_consumer.bitmap_route_bound = 1;
    original_consumer.palette_binding_verified = 0;
    original_consumer.rgba_output_allowed = 1;
    if (theron_v1_runtime_bind_track02_original_consumer_trace(
            &gap,
            &original_consumer,
            0x2a06a0u,
            gap.first_nonstartup_raw_offset,
            gap.first_container_raw_offset,
            &binding) ||
        binding.valid ||
        !binding.fail_closed_until_consumer_proven) {
        ok = 0;
    }
    free(data);
    return ok;
}

int main(void)
{
    Theron_V1RuntimeAdmissionReceipt runtime;
    Theron_V1RuntimeSessionHandoffReceipt session;
    Theron_V1RuntimeBoundedTrack02RouteReceipt bounded_route;
    Theron_V1RuntimeStartupLevelAnchorReceipt startup_anchor;
    Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt nonstartup_evidence;
    Theron_V1RuntimeObjectTableRouteEvidenceReceipt object_evidence;
    Theron_V1RuntimeTrack02CaptureConsumerGapReceipt consumer_gap;
    Theron_V1Track02Post3800ConsumerSemanticReceipt post3800_consumer;
    Theron_V1RuntimeTrack02ConsumerSemanticReceipt consumer_semantics;
    Theron_V1RuntimeTrack02RenderAssetProof render_proof;
    Theron_V1RuntimeTrack02RenderAssetProof mutated_render_proof;
    Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt render_admission;
    Theron_V1RuntimeTrack02DungeonHandoffProof dungeon_handoff_proof;
    Theron_V1RuntimeTrack02DungeonHandoffProof mutated_dungeon_handoff_proof;
    Theron_V1RuntimeTrack02DungeonHandoffReceipt dungeon_handoff;
    Theron_V1RuntimeTrack02HostDungeonConsumerProof host_consumer_proof;
    Theron_V1RuntimeTrack02HostDungeonConsumerProof mutated_host_consumer_proof;
    Theron_V1RuntimeTrack02HostDungeonConsumerReceipt host_consumer;
    Theron_V1TraceSourceProvenanceReceipt provenance;
    Theron_V1RawLoaderTraceGamePayloadReceipt payload;
    Theron_V1RawLoaderTraceGamePayloadReceipt mutated;
    Theron_V1StartupAllDungeonRouteReceipt route;
    Theron_V1StartupAllDungeonRouteReceipt mutated_route;
    Theron_Track02LevelRouteReceipt level_route;
    Theron_Track02LevelRouteReceipt mutated_level_route;
    Theron_Track02LevelRouteReceipt promoted_level_route;
    Theron_Track02ObjectTableRouteReceipt object_route;
    Theron_Track02ObjectTableRouteReceipt mutated_object_route;
    Theron_Track02ObjectTableRouteReceipt promoted_object_route;
    Theron_Track02StartupBitmapAtlas promoted_bitmap_atlas;
    Theron_Track02PaletteWindowEvidence promoted_palette_window;
    Theron_V1CaptureConfig config = {
        1, "raw", "card", "raw_track_required_ready", 1, 1
    };

    theron_v1_runtime_admission_init(&runtime);
    if (runtime.admitted || runtime.game_owned_fifo_payload_admitted ||
        runtime.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_session_handoff_init(&session);
    if (session.valid || session.startup_session_handoff_ready ||
        session.runtime_capture_required ||
        session.fallback_visuals_allowed) {
        return 1;
    }
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    if (theron_v1_runtime_admission_attach(&runtime, "synthetic", 0)) {
        return 1;
    }
    if (!theron_v1_runtime_trace_identity_valid("v3:raw:card", &config)) {
        return 1;
    }
    if (!theron_v1_trace_source_provenance(
            "capture-1", "v3:raw:card", &provenance) ||
        !provenance.valid || provenance.runtime_admitted) {
        return 1;
    }
    if (theron_v1_trace_source_provenance(
            "v3:raw:card", "v3:raw:card", &provenance)) {
        return 1;
    }
    if (!theron_v1_runtime_admission_attach(
            &runtime, "real-v3-trace", 0) || runtime.admitted) {
        return 1;
    }

    if (!build_payload_receipt(&payload) ||
        !theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &payload)) {
        return 1;
    }
    if (!runtime.attached || !runtime.admitted ||
        !runtime.game_owned_fifo_payload_attached ||
        !runtime.game_owned_fifo_payload_admitted ||
        runtime.game_owned_fifo_payload_variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(runtime.game_owned_fifo_payload_track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        runtime.game_owned_fifo_payload_record != payload.raw_track02_record ||
        runtime.game_owned_fifo_payload_source_offset != payload.source_offset ||
        runtime.game_owned_fifo_payload_source_byte != payload.source_byte ||
        !runtime.cdb_read6_verified ||
        !runtime.fifo_to_game_ram_verified ||
        !runtime.game_ram_consumer_verified ||
        runtime.payload_semantics_proven ||
        runtime.visual_semantics_proven ||
        runtime.fallback_visuals_allowed) {
        return 1;
    }
    if (!theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session)) {
        return 1;
    }
    build_bounded_route_receipt(&route);
    if (!theron_v1_runtime_session_handoff_bind_bounded_track02_route(
            &session, &route, &bounded_route)) {
        return 1;
    }
    if (!bounded_route.valid ||
        !bounded_route.session_handoff_consumed ||
        !bounded_route.runtime_capture_required ||
        bounded_route.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(bounded_route.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        bounded_route.record != payload.raw_track02_record ||
        bounded_route.source_offset != payload.source_offset ||
        bounded_route.source_byte != payload.source_byte ||
        bounded_route.all_dungeon_capture_mask != route.dungeon_mask ||
        bounded_route.all_dungeon_capture_count != route.capture_count ||
        !bounded_route.object_table_no_fallback_ready ||
        bounded_route.object_table_blocked_anchor_mask != 0x07u ||
        !bounded_route.nonstartup_level_no_fallback_ready ||
        bounded_route.nonstartup_level_blocked_anchor_mask != 0x07u ||
        bounded_route.startup_level_blocked_anchor_mask != 0x06u ||
        bounded_route.startup_level_anchor_status !=
            THERON_TRACK02_LEVEL_HANDOFF_OK ||
        bounded_route.startup_level_anchor_raw_offset != 0x7015b4u ||
        !bounded_route.startup_level_anchor_user_data_valid ||
        bounded_route.startup_level_anchor_width != 32u ||
        bounded_route.startup_level_anchor_height != 27u ||
        bounded_route.startup_level_anchor_level_index != 0x0026u ||
        bounded_route.object_table_route_hash != route.object_table_route_hash ||
        bounded_route.level_route_hash != route.level_route_hash ||
        bounded_route.all_dungeon_route_hash != route.route_hash ||
        bounded_route.exact_level_semantics_ready ||
        bounded_route.exact_object_semantics_ready ||
        bounded_route.object_table_admission_allowed ||
        bounded_route.level_admission_allowed ||
        bounded_route.payload_semantics_proven ||
        bounded_route.visual_semantics_proven ||
        bounded_route.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_startup_level_anchor_init(&startup_anchor);
    if (startup_anchor.valid ||
        startup_anchor.startup_level_anchor_admitted ||
        startup_anchor.fallback_visuals_allowed) {
        return 1;
    }
    if (!theron_v1_runtime_bounded_track02_route_bind_startup_level_anchor(
            &bounded_route, &startup_anchor)) {
        return 1;
    }
    if (!startup_anchor.valid ||
        !startup_anchor.bounded_route_consumed ||
        !startup_anchor.runtime_capture_required ||
        startup_anchor.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(startup_anchor.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        startup_anchor.record != payload.raw_track02_record ||
        startup_anchor.source_offset != payload.source_offset ||
        startup_anchor.source_byte != payload.source_byte ||
        startup_anchor.startup_level_raw_offset != 0x7015b4u ||
        startup_anchor.startup_level_user_data_offset != 0x622650u ||
        !startup_anchor.startup_level_user_data_valid ||
        startup_anchor.startup_level_width != 32u ||
        startup_anchor.startup_level_height != 27u ||
        startup_anchor.startup_level_seed != 0x0108e938u ||
        startup_anchor.startup_level_index != 0x0026u ||
        startup_anchor.level_route_hash != route.level_route_hash ||
        startup_anchor.object_table_route_hash !=
            route.object_table_route_hash ||
        startup_anchor.all_dungeon_route_hash != route.route_hash ||
        !startup_anchor.startup_level_anchor_admitted ||
        startup_anchor.object_table_admission_allowed ||
        startup_anchor.nonstartup_level_admission_allowed ||
        startup_anchor.exact_level_semantics_ready ||
        startup_anchor.exact_object_semantics_ready ||
        startup_anchor.payload_semantics_proven ||
        startup_anchor.visual_semantics_proven ||
        startup_anchor.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_nonstartup_level_route_evidence_init(
        &nonstartup_evidence);
    if (nonstartup_evidence.valid ||
        nonstartup_evidence.nonstartup_level_admission_allowed ||
        nonstartup_evidence.fallback_visuals_allowed) {
        return 1;
    }
    build_nonstartup_level_route_receipt(&level_route, &route);
    if (!theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence(
            &startup_anchor, &level_route, &nonstartup_evidence)) {
        return 1;
    }
    if (!nonstartup_evidence.valid ||
        !nonstartup_evidence.startup_level_anchor_consumed ||
        !nonstartup_evidence.level_route_receipt_consumed ||
        !nonstartup_evidence.runtime_capture_required ||
        nonstartup_evidence.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(nonstartup_evidence.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        nonstartup_evidence.record != payload.raw_track02_record ||
        nonstartup_evidence.source_offset != payload.source_offset ||
        nonstartup_evidence.source_byte != payload.source_byte ||
        nonstartup_evidence.descriptor_anchor_mask != 0x07u ||
        nonstartup_evidence.descriptor_anchor_count != 3 ||
        nonstartup_evidence.nonstartup_level_candidate_anchor_mask != 0x07u ||
        nonstartup_evidence.nonstartup_level_candidate_count != 3 ||
        nonstartup_evidence.nonstartup_level_blocked_anchor_mask != 0x07u ||
        nonstartup_evidence.nonstartup_level_blocked_anchor_count != 3 ||
        nonstartup_evidence.first_candidate_raw_offset != 0x701e06u ||
        nonstartup_evidence.first_candidate_user_data_offset != 0x622ea2u ||
        !nonstartup_evidence.first_candidate_user_data_valid ||
        nonstartup_evidence.first_candidate_byte_count != 0x0400u ||
        nonstartup_evidence.first_candidate_hash != 0x5a77c001u ||
        nonstartup_evidence.first_candidate_header_width != 0x4142u ||
        nonstartup_evidence.first_candidate_header_height != 0x4344u ||
        nonstartup_evidence.first_candidate_header_seed != 0x45464748u ||
        nonstartup_evidence.first_candidate_header_level_index != 0x494au ||
        nonstartup_evidence.level_route_hash != route.level_route_hash ||
        nonstartup_evidence.object_table_route_hash !=
            route.object_table_route_hash ||
        nonstartup_evidence.all_dungeon_route_hash != route.route_hash ||
        nonstartup_evidence.nonstartup_level_decode_ready ||
        nonstartup_evidence.nonstartup_level_admission_allowed ||
        nonstartup_evidence.exact_level_semantics_ready ||
        nonstartup_evidence.exact_object_semantics_ready ||
        nonstartup_evidence.payload_semantics_proven ||
        nonstartup_evidence.visual_semantics_proven ||
        nonstartup_evidence.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_object_table_route_evidence_init(&object_evidence);
    if (object_evidence.valid ||
        object_evidence.object_table_admission_allowed ||
        object_evidence.fallback_visuals_allowed) {
        return 1;
    }
    build_object_table_route_receipt(&object_route, &route);
    if (!theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence(
            &startup_anchor, &object_route, &object_evidence)) {
        return 1;
    }
    if (!object_evidence.valid ||
        !object_evidence.startup_level_anchor_consumed ||
        !object_evidence.object_table_route_receipt_consumed ||
        !object_evidence.runtime_capture_required ||
        object_evidence.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(object_evidence.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        object_evidence.record != payload.raw_track02_record ||
        object_evidence.source_offset != payload.source_offset ||
        object_evidence.source_byte != payload.source_byte ||
        object_evidence.descriptor_anchor_mask != 0x07u ||
        object_evidence.descriptor_anchor_count != 3 ||
        object_evidence.object_table_candidate_anchor_mask != 0x07u ||
        object_evidence.object_table_candidate_count != 3 ||
        object_evidence.object_table_blocked_anchor_mask != 0x07u ||
        object_evidence.object_table_blocked_anchor_count != 3 ||
        object_evidence.first_candidate_entry_index != 6u ||
        object_evidence.first_candidate_raw_offset != 0x701e06u ||
        object_evidence.first_candidate_user_data_offset != 0x622ea2u ||
        !object_evidence.first_candidate_user_data_valid ||
        object_evidence.first_candidate_byte_count != 0x0400u ||
        object_evidence.first_candidate_nonzero_byte_count != 16u ||
        object_evidence.first_candidate_hash != 0x5a77c001u ||
        object_evidence.first_candidate_descriptor_delta != 0x29cu ||
        !object_evidence.first_candidate_after_descriptor ||
        object_evidence.first_candidate_binding_status !=
            THERON_TRACK02_SEMANTIC_BINDING_NOT_BOUND ||
        object_evidence.first_candidate_reject_reason !=
            THERON_TRACK02_OBJECT_TABLE_REJECT_NONE ||
        object_evidence.object_table_route_hash !=
            route.object_table_route_hash ||
        object_evidence.level_route_hash != route.level_route_hash ||
        object_evidence.all_dungeon_route_hash != route.route_hash ||
        object_evidence.object_table_decode_ready ||
        object_evidence.object_table_admission_allowed ||
        object_evidence.exact_object_semantics_ready ||
        object_evidence.payload_semantics_proven ||
        object_evidence.visual_semantics_proven ||
        object_evidence.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_track02_capture_consumer_gap_init(&consumer_gap);
    if (consumer_gap.valid ||
        consumer_gap.capture_consumer_route_ready ||
        consumer_gap.fallback_visuals_allowed) {
        return 1;
    }
    if (!theron_v1_runtime_bind_track02_capture_consumer_gap(
            &startup_anchor, &nonstartup_evidence, &object_evidence,
            &consumer_gap)) {
        return 1;
    }
    if (!consumer_gap.valid ||
        !consumer_gap.startup_level_anchor_consumed ||
        !consumer_gap.nonstartup_level_evidence_consumed ||
        !consumer_gap.object_table_evidence_consumed ||
        !consumer_gap.runtime_capture_required ||
        consumer_gap.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(consumer_gap.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        consumer_gap.record != payload.raw_track02_record ||
        consumer_gap.source_offset != payload.source_offset ||
        consumer_gap.source_byte != payload.source_byte ||
        consumer_gap.level_route_hash != route.level_route_hash ||
        consumer_gap.object_table_route_hash != route.object_table_route_hash ||
        consumer_gap.all_dungeon_route_hash != route.route_hash ||
        consumer_gap.nonstartup_level_candidate_anchor_mask != 0x07u ||
        consumer_gap.nonstartup_level_candidate_count != 3 ||
        consumer_gap.first_nonstartup_level_candidate_hash != 0x5a77c001u ||
        consumer_gap.object_table_candidate_anchor_mask != 0x07u ||
        consumer_gap.object_table_candidate_count != 3 ||
        consumer_gap.first_object_table_candidate_hash != 0x5a77c001u ||
        consumer_gap.capture_consumer_route_ready ||
        consumer_gap.object_table_decode_ready ||
        consumer_gap.nonstartup_level_decode_ready ||
        consumer_gap.object_table_admission_allowed ||
        consumer_gap.nonstartup_level_admission_allowed ||
        consumer_gap.exact_level_semantics_ready ||
        consumer_gap.exact_object_semantics_ready ||
        consumer_gap.payload_semantics_proven ||
        consumer_gap.visual_semantics_proven ||
        consumer_gap.fallback_visuals_allowed) {
        return 1;
    }
    object_evidence.object_table_route_hash ^= 0x04u;
    if (theron_v1_runtime_bind_track02_capture_consumer_gap(
            &startup_anchor, &nonstartup_evidence, &object_evidence,
            &consumer_gap) || consumer_gap.valid) {
        return 1;
    }
    object_evidence.object_table_route_hash ^= 0x04u;
    nonstartup_evidence.nonstartup_level_admission_allowed = 1;
    if (theron_v1_runtime_bind_track02_capture_consumer_gap(
            &startup_anchor, &nonstartup_evidence, &object_evidence,
            &consumer_gap) || consumer_gap.valid) {
        return 1;
    }
    nonstartup_evidence.nonstartup_level_admission_allowed = 0;
    object_evidence.object_table_admission_allowed = 1;
    if (theron_v1_runtime_bind_track02_capture_consumer_gap(
            &startup_anchor, &nonstartup_evidence, &object_evidence,
            &consumer_gap) || consumer_gap.valid) {
        return 1;
    }
    object_evidence.object_table_admission_allowed = 0;
    startup_anchor.exact_level_semantics_ready = 1;
    if (theron_v1_runtime_bind_track02_capture_consumer_gap(
            &startup_anchor, &nonstartup_evidence, &object_evidence,
            &consumer_gap) || consumer_gap.valid) {
        return 1;
    }
    startup_anchor.exact_level_semantics_ready = 0;
    if (!theron_v1_runtime_bind_track02_capture_consumer_gap(
            &startup_anchor, &nonstartup_evidence, &object_evidence,
            &consumer_gap)) {
        return 1;
    }
    theron_v1_runtime_track02_consumer_semantic_init(&consumer_semantics);
    if (consumer_semantics.valid ||
        consumer_semantics.capture_consumer_route_ready ||
        consumer_semantics.fallback_visuals_allowed) {
        return 1;
    }
    memset(&post3800_consumer, 0, sizeof(post3800_consumer));
    post3800_consumer.valid = 1;
    post3800_consumer.no_fallback = 1;
    post3800_consumer.original_consumer_trace_bound = 1;
    post3800_consumer.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    post3800_consumer.record = payload.raw_track02_record;
    post3800_consumer.payload_checksum = 0x6010cafeu;
    post3800_consumer.level_envelope_checksum = 0x5e1e0001u;
    post3800_consumer.post_envelope_checksum = 0x0b1ec7e0u;
    post3800_consumer.consumer_trace_checksum = 0x3800e009u;
    post3800_consumer.dungeon_record_semantics_proven = 1;
    post3800_consumer.object_table_semantics_proven = 1;
    post3800_consumer.bitmap_route_bound = 1;
    post3800_consumer.palette_binding_verified = 1;
    post3800_consumer.rgba_output_allowed = 1;
    if (!theron_v1_runtime_bind_track02_consumer_semantics(
            &consumer_gap, &post3800_consumer, &consumer_semantics)) {
        return 1;
    }
    if (!consumer_semantics.valid ||
        !consumer_semantics.capture_consumer_gap_consumed ||
        !consumer_semantics.original_consumer_trace_bound ||
        !consumer_semantics.runtime_capture_required ||
        consumer_semantics.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(consumer_semantics.track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        consumer_semantics.record != payload.raw_track02_record ||
        consumer_semantics.source_offset != payload.source_offset ||
        consumer_semantics.source_byte != payload.source_byte ||
        consumer_semantics.level_route_hash != route.level_route_hash ||
        consumer_semantics.object_table_route_hash !=
            route.object_table_route_hash ||
        consumer_semantics.all_dungeon_route_hash != route.route_hash ||
        consumer_semantics.payload_checksum !=
            post3800_consumer.payload_checksum ||
        consumer_semantics.level_envelope_checksum !=
            post3800_consumer.level_envelope_checksum ||
        consumer_semantics.post_envelope_checksum !=
            post3800_consumer.post_envelope_checksum ||
        consumer_semantics.consumer_trace_checksum !=
            post3800_consumer.consumer_trace_checksum ||
        !consumer_semantics.capture_consumer_route_ready ||
        !consumer_semantics.exact_level_semantics_ready ||
        !consumer_semantics.exact_object_semantics_ready ||
        !consumer_semantics.payload_semantics_proven ||
        !consumer_semantics.visual_semantics_proven ||
        consumer_semantics.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_track02_render_asset_admission_init(
        &render_admission);
    if (render_admission.valid ||
        render_admission.real_render_assets_admitted ||
        render_admission.fallback_visuals_allowed) {
        return 1;
    }
    if (!probe_real_track02_capture_producer(&consumer_semantics)) {
        return 1;
    }
    theron_v1_runtime_track02_render_asset_proof_init(&render_proof);
    if (render_proof.valid ||
        render_proof.level_consumer_proven ||
        render_proof.fallback_visuals_allowed) {
        return 1;
    }
    promoted_level_route = level_route;
    promoted_level_route.nonstartup_level_decode_ready = 1;
    promoted_level_route.blocked_for_missing_nonstartup_level_evidence = 0;
    promoted_level_route.nonstartup_level_candidate_hash[0] = 0x5a77c001u;
    promoted_object_route = object_route;
    promoted_object_route.object_table_decode_ready = 1;
    promoted_object_route.blocked_for_missing_real_object_evidence = 0;
    promoted_object_route.object_table_level_consensus_mask = 0x01u;
    promoted_object_route.object_table_level_consensus_record_hashes[0] =
        0x0b7ec700u;
    promoted_object_route.object_table_level_consensus_position_hashes[0] =
        0x00c0ffeeu;
    memset(&promoted_bitmap_atlas, 0, sizeof(promoted_bitmap_atlas));
    promoted_bitmap_atlas.variant = THERON_TRACK02_VARIANT_US_BIN;
    promoted_bitmap_atlas.route_count =
        THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX;
    promoted_bitmap_atlas.route_mask =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    promoted_bitmap_atlas.total_tile_count = 8u;
    promoted_bitmap_atlas.total_nonzero_pixel_count = 128u;
    promoted_bitmap_atlas.checksum = 0xb17b00a1u;
    memset(&promoted_palette_window, 0, sizeof(promoted_palette_window));
    promoted_palette_window.variant = THERON_TRACK02_VARIANT_US_BIN;
    promoted_palette_window.payload_checksum = 0x0fa1e77eu;
    promoted_palette_window.format_valid = 1;
    promoted_palette_window.semantic_binding_verified = 1;
    promoted_palette_window.promotion_allowed = 1;
    promoted_palette_window.palette.valid = 1;
    promoted_palette_window.palette.nonblack_entry_count = 15u;
    promoted_palette_window.palette.checksum = 0x7100fa1eu;
    if (!theron_v1_runtime_track02_render_asset_proof_from_decoded_routes(
            &consumer_semantics,
            &promoted_level_route,
            &promoted_object_route,
            &promoted_bitmap_atlas,
            &promoted_palette_window,
            &render_proof)) {
        return 1;
    }
    if (!render_proof.valid ||
        !render_proof.same_capture_as_consumer_semantics ||
        render_proof.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(render_proof.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        render_proof.record != payload.raw_track02_record ||
        render_proof.level_route_hash != route.level_route_hash ||
        render_proof.object_table_route_hash !=
            route.object_table_route_hash ||
        render_proof.all_dungeon_route_hash != route.route_hash ||
        render_proof.payload_checksum !=
            post3800_consumer.payload_checksum ||
        render_proof.level_envelope_checksum !=
            post3800_consumer.level_envelope_checksum ||
        render_proof.post_envelope_checksum !=
            post3800_consumer.post_envelope_checksum ||
        render_proof.consumer_trace_checksum !=
            post3800_consumer.consumer_trace_checksum ||
        render_proof.decoded_level_hash == 0u ||
        render_proof.decoded_object_table_hash == 0u ||
        render_proof.decoded_bitmap_hash != promoted_bitmap_atlas.checksum ||
        render_proof.decoded_palette_hash == 0u ||
        !render_proof.level_consumer_proven ||
        !render_proof.object_table_consumer_proven ||
        !render_proof.bitmap_consumer_proven ||
        !render_proof.palette_consumer_proven ||
        !render_proof.decoded_bitmap_pixels_proven ||
        !render_proof.decoded_palette_words_proven ||
        render_proof.synthetic_level_promoted ||
        render_proof.synthetic_object_table_promoted ||
        render_proof.synthetic_bitmap_promoted ||
        render_proof.synthetic_palette_promoted ||
        render_proof.fallback_visuals_observed ||
        render_proof.fallback_visuals_allowed) {
        return 1;
    }
    if (theron_v1_runtime_track02_render_asset_proof_from_decoded_routes(
            &consumer_semantics,
            &level_route,
            &promoted_object_route,
            &promoted_bitmap_atlas,
            &promoted_palette_window,
            &mutated_render_proof) ||
        mutated_render_proof.valid) {
        return 1;
    }
    promoted_palette_window.promotion_allowed = 0;
    if (theron_v1_runtime_track02_render_asset_proof_from_decoded_routes(
            &consumer_semantics,
            &promoted_level_route,
            &promoted_object_route,
            &promoted_bitmap_atlas,
            &promoted_palette_window,
            &mutated_render_proof) ||
        mutated_render_proof.valid) {
        return 1;
    }
    promoted_palette_window.promotion_allowed = 1;
    promoted_bitmap_atlas.route_mask &=
        ~THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    if (theron_v1_runtime_track02_render_asset_proof_from_decoded_routes(
            &consumer_semantics,
            &promoted_level_route,
            &promoted_object_route,
            &promoted_bitmap_atlas,
            &promoted_palette_window,
            &mutated_render_proof) ||
        mutated_render_proof.valid) {
        return 1;
    }
    promoted_bitmap_atlas.route_mask |=
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    if (!theron_v1_runtime_bind_track02_render_asset_admission(
            &consumer_semantics, &render_proof, &render_admission)) {
        return 1;
    }
    if (!render_admission.valid ||
        !render_admission.consumer_semantics_consumed ||
        !render_admission.real_render_asset_proof_consumed ||
        !render_admission.runtime_capture_required ||
        render_admission.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(render_admission.track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        render_admission.record != payload.raw_track02_record ||
        render_admission.level_route_hash != route.level_route_hash ||
        render_admission.object_table_route_hash !=
            route.object_table_route_hash ||
        render_admission.all_dungeon_route_hash != route.route_hash ||
        render_admission.payload_checksum !=
            post3800_consumer.payload_checksum ||
        render_admission.level_envelope_checksum !=
            post3800_consumer.level_envelope_checksum ||
        render_admission.post_envelope_checksum !=
            post3800_consumer.post_envelope_checksum ||
        render_admission.consumer_trace_checksum !=
            post3800_consumer.consumer_trace_checksum ||
        render_admission.decoded_level_hash !=
            render_proof.decoded_level_hash ||
        render_admission.decoded_object_table_hash !=
            render_proof.decoded_object_table_hash ||
        render_admission.decoded_bitmap_hash !=
            render_proof.decoded_bitmap_hash ||
        render_admission.decoded_palette_hash !=
            render_proof.decoded_palette_hash ||
        !render_admission.object_table_admission_allowed ||
        !render_admission.level_admission_allowed ||
        !render_admission.bitmap_admission_allowed ||
        !render_admission.palette_admission_allowed ||
        !render_admission.real_render_assets_admitted ||
        render_admission.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_track02_dungeon_handoff_init(&dungeon_handoff);
    if (dungeon_handoff.valid ||
        dungeon_handoff.real_data_handoff_to_dungeon ||
        dungeon_handoff.dungeon_draw_allowed ||
        dungeon_handoff.fallback_visuals_allowed) {
        return 1;
    }
    memset(&dungeon_handoff_proof, 0, sizeof(dungeon_handoff_proof));
    dungeon_handoff_proof.valid = 1;
    dungeon_handoff_proof.same_capture_as_render_admission = 1;
    dungeon_handoff_proof.variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(dungeon_handoff_proof.track02_md5, THERON_TRACK02_MD5_US_BIN);
    dungeon_handoff_proof.record = payload.raw_track02_record;
    dungeon_handoff_proof.level_route_hash = route.level_route_hash;
    dungeon_handoff_proof.object_table_route_hash =
        route.object_table_route_hash;
    dungeon_handoff_proof.all_dungeon_route_hash = route.route_hash;
    dungeon_handoff_proof.payload_checksum =
        post3800_consumer.payload_checksum;
    dungeon_handoff_proof.level_envelope_checksum =
        post3800_consumer.level_envelope_checksum;
    dungeon_handoff_proof.post_envelope_checksum =
        post3800_consumer.post_envelope_checksum;
    dungeon_handoff_proof.consumer_trace_checksum =
        post3800_consumer.consumer_trace_checksum;
    dungeon_handoff_proof.decoded_level_hash =
        render_proof.decoded_level_hash;
    dungeon_handoff_proof.decoded_object_table_hash =
        render_proof.decoded_object_table_hash;
    dungeon_handoff_proof.decoded_bitmap_hash =
        render_proof.decoded_bitmap_hash;
    dungeon_handoff_proof.decoded_palette_hash =
        render_proof.decoded_palette_hash;
    dungeon_handoff_proof.dungeon_runtime_consumer_bound = 1;
    dungeon_handoff_proof.object_table_layout_proven = 1;
    dungeon_handoff_proof.bitmap_palette_decode_proven = 1;
    dungeon_handoff_proof.source_level_bytes_bound = 1;
    dungeon_handoff_proof.source_object_table_bytes_bound = 1;
    dungeon_handoff_proof.source_bitmap_bytes_bound = 1;
    dungeon_handoff_proof.source_palette_bytes_bound = 1;
    if (!theron_v1_runtime_bind_track02_dungeon_handoff(
            &render_admission, &dungeon_handoff_proof, &dungeon_handoff)) {
        return 1;
    }
    if (!dungeon_handoff.valid ||
        !dungeon_handoff.render_asset_admission_consumed ||
        !dungeon_handoff.dungeon_handoff_proof_consumed ||
        !dungeon_handoff.runtime_capture_required ||
        dungeon_handoff.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(dungeon_handoff.track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        dungeon_handoff.record != payload.raw_track02_record ||
        dungeon_handoff.level_route_hash != route.level_route_hash ||
        dungeon_handoff.object_table_route_hash !=
            route.object_table_route_hash ||
        dungeon_handoff.all_dungeon_route_hash != route.route_hash ||
        dungeon_handoff.payload_checksum !=
            post3800_consumer.payload_checksum ||
        dungeon_handoff.level_envelope_checksum !=
            post3800_consumer.level_envelope_checksum ||
        dungeon_handoff.post_envelope_checksum !=
            post3800_consumer.post_envelope_checksum ||
        dungeon_handoff.consumer_trace_checksum !=
            post3800_consumer.consumer_trace_checksum ||
        dungeon_handoff.decoded_level_hash !=
            render_proof.decoded_level_hash ||
        dungeon_handoff.decoded_object_table_hash !=
            render_proof.decoded_object_table_hash ||
        dungeon_handoff.decoded_bitmap_hash !=
            render_proof.decoded_bitmap_hash ||
        dungeon_handoff.decoded_palette_hash !=
            render_proof.decoded_palette_hash ||
        !dungeon_handoff.real_data_handoff_to_dungeon ||
        !dungeon_handoff.dungeon_state_admission_allowed ||
        !dungeon_handoff.object_table_layout_admission_allowed ||
        !dungeon_handoff.bitmap_palette_decode_admission_allowed ||
        dungeon_handoff.dungeon_draw_allowed ||
        dungeon_handoff.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_track02_host_dungeon_consumer_init(&host_consumer);
    if (host_consumer.valid ||
        host_consumer.real_track02_dungeon_consumer_ready ||
        host_consumer.dungeon_draw_allowed ||
        host_consumer.fallback_visuals_allowed) {
        return 1;
    }
    theron_v1_runtime_track02_host_dungeon_consumer_proof_init(
        &host_consumer_proof);
    if (host_consumer_proof.valid ||
        host_consumer_proof.original_host_route_bound ||
        host_consumer_proof.fallback_visuals_allowed) {
        return 1;
    }
    if (!theron_v1_runtime_track02_host_dungeon_consumer_proof_from_handoff(
            &dungeon_handoff,
            THERON_V1_TRACK02_ORIGINAL_HOST_DUNGEON_ROUTE,
            1, 1, 1, 1, 1,
            &host_consumer_proof)) {
        return 1;
    }
    if (!host_consumer_proof.valid ||
        !host_consumer_proof.same_capture_as_dungeon_handoff ||
        host_consumer_proof.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(host_consumer_proof.track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        host_consumer_proof.record != payload.raw_track02_record ||
        host_consumer_proof.level_route_hash != route.level_route_hash ||
        host_consumer_proof.object_table_route_hash !=
            route.object_table_route_hash ||
        host_consumer_proof.all_dungeon_route_hash != route.route_hash ||
        host_consumer_proof.decoded_level_hash !=
            render_proof.decoded_level_hash ||
        host_consumer_proof.decoded_object_table_hash !=
            render_proof.decoded_object_table_hash ||
        host_consumer_proof.decoded_bitmap_hash !=
            render_proof.decoded_bitmap_hash ||
        host_consumer_proof.decoded_palette_hash !=
            render_proof.decoded_palette_hash ||
        host_consumer_proof.original_host_route_identity_checksum == 0u ||
        !host_consumer_proof.original_host_route_bound ||
        !host_consumer_proof.level_grid_runtime_consumer_bound ||
        !host_consumer_proof.object_table_runtime_consumer_bound ||
        !host_consumer_proof.bitmap_palette_runtime_consumer_bound ||
        !host_consumer_proof.host_surface_upload_proven ||
        !host_consumer_proof.host_capture_frame_proven ||
        host_consumer_proof.synthetic_host_frame_promoted ||
        host_consumer_proof.synthetic_level_grid_promoted ||
        host_consumer_proof.synthetic_object_table_promoted ||
        host_consumer_proof.synthetic_bitmap_palette_promoted ||
        host_consumer_proof.fallback_visuals_observed ||
        host_consumer_proof.fallback_visuals_allowed) {
        return 1;
    }
    if (!theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &host_consumer_proof, &host_consumer)) {
        return 1;
    }
    if (!host_consumer.valid ||
        !host_consumer.dungeon_handoff_consumed ||
        !host_consumer.host_consumer_proof_consumed ||
        !host_consumer.runtime_capture_required ||
        host_consumer.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(host_consumer.track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        host_consumer.record != payload.raw_track02_record ||
        host_consumer.level_route_hash != route.level_route_hash ||
        host_consumer.object_table_route_hash !=
            route.object_table_route_hash ||
        host_consumer.all_dungeon_route_hash != route.route_hash ||
        host_consumer.payload_checksum !=
            post3800_consumer.payload_checksum ||
        host_consumer.level_envelope_checksum !=
            post3800_consumer.level_envelope_checksum ||
        host_consumer.post_envelope_checksum !=
            post3800_consumer.post_envelope_checksum ||
        host_consumer.consumer_trace_checksum !=
            post3800_consumer.consumer_trace_checksum ||
        host_consumer.decoded_level_hash !=
            render_proof.decoded_level_hash ||
        host_consumer.decoded_object_table_hash !=
            render_proof.decoded_object_table_hash ||
        host_consumer.decoded_bitmap_hash !=
            render_proof.decoded_bitmap_hash ||
        host_consumer.decoded_palette_hash !=
            render_proof.decoded_palette_hash ||
        host_consumer.original_host_route_identity_checksum !=
            host_consumer_proof.original_host_route_identity_checksum ||
        !host_consumer.real_track02_dungeon_consumer_ready ||
        !host_consumer.host_surface_upload_allowed ||
        !host_consumer.host_capture_frame_required ||
        !host_consumer.dungeon_draw_allowed ||
        host_consumer.fallback_visuals_allowed) {
        return 1;
    }
    if (theron_v1_runtime_track02_host_dungeon_consumer_proof_from_handoff(
            &dungeon_handoff,
            "synthetic-host-route",
            1, 1, 1, 1, 1,
            &mutated_host_consumer_proof) ||
        mutated_host_consumer_proof.valid) {
        return 1;
    }
    if (theron_v1_runtime_track02_host_dungeon_consumer_proof_from_handoff(
            &dungeon_handoff,
            "theron-v1-fallback-route",
            1, 1, 1, 1, 1,
            &mutated_host_consumer_proof) ||
        mutated_host_consumer_proof.valid) {
        return 1;
    }
    if (theron_v1_runtime_track02_host_dungeon_consumer_proof_from_handoff(
            &dungeon_handoff,
            THERON_V1_TRACK02_ORIGINAL_HOST_DUNGEON_ROUTE,
            1, 1, 1, 1, 0,
            &mutated_host_consumer_proof) ||
        mutated_host_consumer_proof.valid) {
        return 1;
    }
    mutated_host_consumer_proof = host_consumer_proof;
    mutated_host_consumer_proof.original_host_route_identity_checksum ^= 1u;
    if (theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &mutated_host_consumer_proof, &host_consumer) ||
        host_consumer.valid) {
        return 1;
    }
    mutated_host_consumer_proof = host_consumer_proof;
    mutated_host_consumer_proof.host_capture_frame_proven = 0;
    if (theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &mutated_host_consumer_proof, &host_consumer) ||
        host_consumer.valid) {
        return 1;
    }
    mutated_host_consumer_proof = host_consumer_proof;
    mutated_host_consumer_proof.synthetic_host_frame_promoted = 1;
    if (theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &mutated_host_consumer_proof, &host_consumer) ||
        host_consumer.valid) {
        return 1;
    }
    mutated_host_consumer_proof = host_consumer_proof;
    mutated_host_consumer_proof.decoded_level_hash ^= 0x20u;
    if (theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &mutated_host_consumer_proof, &host_consumer) ||
        host_consumer.valid) {
        return 1;
    }
    mutated_host_consumer_proof = host_consumer_proof;
    mutated_host_consumer_proof.fallback_visuals_observed = 1;
    if (theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &mutated_host_consumer_proof, &host_consumer) ||
        host_consumer.valid) {
        return 1;
    }
    dungeon_handoff.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &host_consumer_proof, &host_consumer) ||
        host_consumer.valid) {
        return 1;
    }
    dungeon_handoff.fallback_visuals_allowed = 0;
    dungeon_handoff.dungeon_draw_allowed = 1;
    if (theron_v1_runtime_bind_track02_host_dungeon_consumer(
            &dungeon_handoff, &host_consumer_proof, &host_consumer) ||
        host_consumer.valid) {
        return 1;
    }
    dungeon_handoff.dungeon_draw_allowed = 0;
    mutated_dungeon_handoff_proof = dungeon_handoff_proof;
    mutated_dungeon_handoff_proof.object_table_layout_proven = 0;
    if (theron_v1_runtime_bind_track02_dungeon_handoff(
            &render_admission, &mutated_dungeon_handoff_proof,
            &dungeon_handoff) || dungeon_handoff.valid) {
        return 1;
    }
    mutated_dungeon_handoff_proof = dungeon_handoff_proof;
    mutated_dungeon_handoff_proof.synthetic_dungeon_state_promoted = 1;
    if (theron_v1_runtime_bind_track02_dungeon_handoff(
            &render_admission, &mutated_dungeon_handoff_proof,
            &dungeon_handoff) || dungeon_handoff.valid) {
        return 1;
    }
    mutated_dungeon_handoff_proof = dungeon_handoff_proof;
    mutated_dungeon_handoff_proof.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_bind_track02_dungeon_handoff(
            &render_admission, &mutated_dungeon_handoff_proof,
            &dungeon_handoff) || dungeon_handoff.valid) {
        return 1;
    }
    mutated_dungeon_handoff_proof = dungeon_handoff_proof;
    mutated_dungeon_handoff_proof.decoded_palette_hash ^= 0x40u;
    if (theron_v1_runtime_bind_track02_dungeon_handoff(
            &render_admission, &mutated_dungeon_handoff_proof,
            &dungeon_handoff) || dungeon_handoff.valid) {
        return 1;
    }
    render_admission.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_bind_track02_dungeon_handoff(
            &render_admission, &dungeon_handoff_proof, &dungeon_handoff) ||
        dungeon_handoff.valid) {
        return 1;
    }
    render_admission.fallback_visuals_allowed = 0;
    mutated_render_proof = render_proof;
    mutated_render_proof.decoded_bitmap_hash = 0u;
    if (theron_v1_runtime_bind_track02_render_asset_admission(
            &consumer_semantics, &mutated_render_proof, &render_admission) ||
        render_admission.valid) {
        return 1;
    }
    mutated_render_proof = render_proof;
    mutated_render_proof.synthetic_object_table_promoted = 1;
    if (theron_v1_runtime_bind_track02_render_asset_admission(
            &consumer_semantics, &mutated_render_proof, &render_admission) ||
        render_admission.valid) {
        return 1;
    }
    mutated_render_proof = render_proof;
    mutated_render_proof.fallback_visuals_observed = 1;
    if (theron_v1_runtime_bind_track02_render_asset_admission(
            &consumer_semantics, &mutated_render_proof, &render_admission) ||
        render_admission.valid) {
        return 1;
    }
    mutated_render_proof = render_proof;
    mutated_render_proof.level_route_hash ^= 0x100u;
    if (theron_v1_runtime_bind_track02_render_asset_admission(
            &consumer_semantics, &mutated_render_proof, &render_admission) ||
        render_admission.valid) {
        return 1;
    }
    consumer_semantics.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_bind_track02_render_asset_admission(
            &consumer_semantics, &render_proof, &render_admission) ||
        render_admission.valid) {
        return 1;
    }
    consumer_semantics.fallback_visuals_allowed = 0;
    post3800_consumer.record ^= 1u;
    if (theron_v1_runtime_bind_track02_consumer_semantics(
            &consumer_gap, &post3800_consumer, &consumer_semantics) ||
        consumer_semantics.valid) {
        return 1;
    }
    post3800_consumer.record ^= 1u;
    post3800_consumer.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_bind_track02_consumer_semantics(
            &consumer_gap, &post3800_consumer, &consumer_semantics) ||
        consumer_semantics.valid) {
        return 1;
    }
    post3800_consumer.fallback_visuals_allowed = 0;
    consumer_gap.capture_consumer_route_ready = 1;
    if (theron_v1_runtime_bind_track02_consumer_semantics(
            &consumer_gap, &post3800_consumer, &consumer_semantics) ||
        consumer_semantics.valid) {
        return 1;
    }
    consumer_gap.capture_consumer_route_ready = 0;
    post3800_consumer.track02_variant = THERON_TRACK02_VARIANT_JP_BIN;
    if (theron_v1_runtime_bind_track02_consumer_semantics(
            &consumer_gap, &post3800_consumer, &consumer_semantics) ||
        consumer_semantics.valid) {
        return 1;
    }
    post3800_consumer.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    mutated_object_route = object_route;
    mutated_object_route.route_hash ^= 0x80u;
    if (theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence(
            &startup_anchor, &mutated_object_route, &object_evidence) ||
        object_evidence.valid) {
        return 1;
    }
    mutated_object_route = object_route;
    mutated_object_route.object_table_decode_ready = 1;
    if (theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence(
            &startup_anchor, &mutated_object_route, &object_evidence) ||
        object_evidence.valid) {
        return 1;
    }
    mutated_object_route = object_route;
    mutated_object_route.blocked_for_missing_real_object_evidence = 0;
    if (theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence(
            &startup_anchor, &mutated_object_route, &object_evidence) ||
        object_evidence.valid) {
        return 1;
    }
    mutated_object_route = object_route;
    mutated_object_route.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence(
            &startup_anchor, &mutated_object_route, &object_evidence) ||
        object_evidence.valid) {
        return 1;
    }
    mutated_level_route = level_route;
    mutated_level_route.route_hash ^= 0x10u;
    if (theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence(
            &startup_anchor, &mutated_level_route, &nonstartup_evidence) ||
        nonstartup_evidence.valid) {
        return 1;
    }
    mutated_level_route = level_route;
    mutated_level_route.nonstartup_level_decode_ready = 1;
    if (theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence(
            &startup_anchor, &mutated_level_route, &nonstartup_evidence) ||
        nonstartup_evidence.valid) {
        return 1;
    }
    mutated_level_route = level_route;
    mutated_level_route.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence(
            &startup_anchor, &mutated_level_route, &nonstartup_evidence) ||
        nonstartup_evidence.valid) {
        return 1;
    }
    startup_anchor.nonstartup_level_admission_allowed = 1;
    if (theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence(
            &startup_anchor, &level_route, &nonstartup_evidence) ||
        nonstartup_evidence.valid) {
        return 1;
    }
    startup_anchor.nonstartup_level_admission_allowed = 0;
    bounded_route.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_bounded_track02_route_bind_startup_level_anchor(
            &bounded_route, &startup_anchor) ||
        startup_anchor.valid) {
        return 1;
    }
    bounded_route.fallback_visuals_allowed = 0;
    bounded_route.startup_level_anchor_width = 0;
    if (theron_v1_runtime_bounded_track02_route_bind_startup_level_anchor(
            &bounded_route, &startup_anchor) ||
        startup_anchor.valid) {
        return 1;
    }
    bounded_route.startup_level_anchor_width = 32u;
    bounded_route.object_table_admission_allowed = 1;
    if (theron_v1_runtime_bounded_track02_route_bind_startup_level_anchor(
            &bounded_route, &startup_anchor) ||
        startup_anchor.valid) {
        return 1;
    }
    bounded_route.object_table_admission_allowed = 0;
    if (!session.valid ||
        !session.startup_session_handoff_ready ||
        !session.runtime_capture_required ||
        !session.game_owned_fifo_payload_admitted ||
        session.variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(session.track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        session.record != payload.raw_track02_record ||
        session.source_offset != payload.source_offset ||
        session.source_byte != payload.source_byte ||
        !session.cdb_read6_verified ||
        !session.fifo_to_game_ram_verified ||
        !session.game_ram_consumer_verified ||
        session.payload_semantics_proven ||
        session.visual_semantics_proven ||
        session.fallback_visuals_allowed ||
        session.object_table_admission_allowed ||
        session.level_admission_allowed) {
        return 1;
    }

    mutated_route = route;
    mutated_route.exact_object_semantics_ready = 1;
    if (theron_v1_runtime_session_handoff_bind_bounded_track02_route(
            &session, &mutated_route, &bounded_route) ||
        bounded_route.valid) {
        return 1;
    }
    mutated_route = route;
    mutated_route.object_table_no_fallback_ready = 0;
    if (theron_v1_runtime_session_handoff_bind_bounded_track02_route(
            &session, &mutated_route, &bounded_route) ||
        bounded_route.valid) {
        return 1;
    }
    mutated_route = route;
    mutated_route.startup_level_anchor_user_data_valid[0] = 0;
    if (theron_v1_runtime_session_handoff_bind_bounded_track02_route(
            &session, &mutated_route, &bounded_route) ||
        bounded_route.valid) {
        return 1;
    }
    session.object_table_admission_allowed = 1;
    if (theron_v1_runtime_session_handoff_bind_bounded_track02_route(
            &session, &route, &bounded_route) ||
        bounded_route.valid) {
        return 1;
    }
    session.object_table_admission_allowed = 0;

    mutated = payload;
    mutated.payload_semantics_proven = 1;
    if (theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &mutated) || runtime.admitted) {
        return 1;
    }
    mutated = payload;
    mutated.game_ram_consumer_verified = 0;
    if (theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &mutated) || runtime.admitted) {
        return 1;
    }
    mutated = payload;
    strcpy(mutated.track02_md5, THERON_TRACK02_MD5_JP_BIN);
    if (theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &mutated) || runtime.admitted) {
        return 1;
    }
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    if (!theron_v1_runtime_admission_attach_game_owned_fifo_payload(
            &runtime, &payload)) {
        return 1;
    }
    runtime.payload_semantics_proven = 1;
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    runtime.payload_semantics_proven = 0;
    runtime.visual_semantics_proven = 1;
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    runtime.visual_semantics_proven = 0;
    runtime.fallback_visuals_allowed = 1;
    if (theron_v1_runtime_session_handoff_from_admission(
            &runtime, &session) || session.valid) {
        return 1;
    }
    return 0;
}
