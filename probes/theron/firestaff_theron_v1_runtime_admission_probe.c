#include "theron_v1_runtime_admission.h"

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
    Theron_V1TraceSourceProvenanceReceipt provenance;
    Theron_V1RawLoaderTraceGamePayloadReceipt payload;
    Theron_V1RawLoaderTraceGamePayloadReceipt mutated;
    Theron_V1StartupAllDungeonRouteReceipt route;
    Theron_V1StartupAllDungeonRouteReceipt mutated_route;
    Theron_Track02LevelRouteReceipt level_route;
    Theron_Track02LevelRouteReceipt mutated_level_route;
    Theron_Track02ObjectTableRouteReceipt object_route;
    Theron_Track02ObjectTableRouteReceipt mutated_object_route;
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
