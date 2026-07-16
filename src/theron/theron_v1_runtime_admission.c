#include "theron_v1_runtime_admission.h"

#include <stdio.h>
#include <string.h>

void theron_v1_runtime_admission_init(
    Theron_V1RuntimeAdmissionReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_session_handoff_init(
    Theron_V1RuntimeSessionHandoffReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_bounded_track02_route_init(
    Theron_V1RuntimeBoundedTrack02RouteReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_startup_level_anchor_init(
    Theron_V1RuntimeStartupLevelAnchorReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_nonstartup_level_route_evidence_init(
    Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_object_table_route_evidence_init(
    Theron_V1RuntimeObjectTableRouteEvidenceReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_track02_capture_consumer_gap_init(
    Theron_V1RuntimeTrack02CaptureConsumerGapReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_track02_consumer_semantic_init(
    Theron_V1RuntimeTrack02ConsumerSemanticReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_track02_render_asset_admission_init(
    Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_track02_dungeon_handoff_init(
    Theron_V1RuntimeTrack02DungeonHandoffReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

void theron_v1_runtime_track02_host_dungeon_consumer_init(
    Theron_V1RuntimeTrack02HostDungeonConsumerReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

int theron_v1_runtime_admission_attach(
    Theron_V1RuntimeAdmissionReceipt *out,
    const char *trace_identity,
    int placeholder_or_synthetic) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_admission_init(out);
    if (!trace_identity || trace_identity[0] == '\0' ||
        placeholder_or_synthetic ||
        strstr(trace_identity, "placeholder") ||
        strstr(trace_identity, "synthetic")) {
        return 0;
    }
    out->attached = 1;
    out->admitted = 0;
    return 1;
}

int theron_v1_runtime_admission_attach_game_owned_fifo_payload(
    Theron_V1RuntimeAdmissionReceipt *out,
    const Theron_V1RawLoaderTraceGamePayloadReceipt *payload) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_admission_init(out);
    if (!payload || !payload->valid ||
        payload->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(payload->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        payload->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        !payload->cdb_read6_verified ||
        !payload->fifo_to_game_ram_verified ||
        !payload->game_ram_consumer_verified ||
        payload->payload_semantics_proven) {
        return 0;
    }

    out->attached = 1;
    out->admitted = 1;
    out->game_owned_fifo_payload_attached = 1;
    out->game_owned_fifo_payload_admitted = 1;
    out->game_owned_fifo_payload_variant = payload->variant;
    snprintf(out->game_owned_fifo_payload_track02_md5,
             sizeof(out->game_owned_fifo_payload_track02_md5), "%s",
             payload->track02_md5);
    out->game_owned_fifo_payload_record = payload->raw_track02_record;
    out->game_owned_fifo_payload_source_offset = payload->source_offset;
    out->game_owned_fifo_payload_source_byte = payload->source_byte;
    out->cdb_read6_verified = payload->cdb_read6_verified;
    out->fifo_to_game_ram_verified = payload->fifo_to_game_ram_verified;
    out->game_ram_consumer_verified = payload->game_ram_consumer_verified;
    out->payload_semantics_proven = 0;
    out->visual_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_trace_identity_valid(
    const char *identity,
    const Theron_V1CaptureConfig *config) {
    char expected[96];
    int written;

    if (!identity || !config || !config->valid || !config->track02_hash ||
        !config->system_card_hash) {
        return 0;
    }
    written = snprintf(expected, sizeof(expected), "v3:%s:%s",
                       config->track02_hash, config->system_card_hash);
    if (written <= 0 || (size_t)written >= sizeof(expected)) {
        return 0;
    }
    return strcmp(identity, expected) == 0;
}

int theron_v1_trace_source_provenance(
    const char *source_id,
    const char *config_identity,
    Theron_V1TraceSourceProvenanceReceipt *out) {
    if (out) {
        out->valid = 0;
        out->runtime_admitted = 0;
    }
    if (!out || !source_id || source_id[0] == '\0' ||
        !config_identity || config_identity[0] == '\0' ||
        strcmp(source_id, config_identity) == 0 ||
        strstr(source_id, "placeholder") ||
        strstr(source_id, "synthetic")) {
        return 0;
    }
    out->valid = 1;
    out->runtime_admitted = 0;
    return 1;
}

int theron_v1_runtime_session_handoff_from_admission(
    const Theron_V1RuntimeAdmissionReceipt *admission,
    Theron_V1RuntimeSessionHandoffReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_session_handoff_init(out);
    if (!admission || !admission->admitted ||
        !admission->game_owned_fifo_payload_admitted ||
        admission->game_owned_fifo_payload_variant !=
            THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(admission->game_owned_fifo_payload_track02_md5,
               THERON_TRACK02_MD5_US_BIN) != 0 ||
        admission->game_owned_fifo_payload_source_offset >=
            THERON_TRACK02_RAW_SECTOR_BYTES ||
        !admission->cdb_read6_verified ||
        !admission->fifo_to_game_ram_verified ||
        !admission->game_ram_consumer_verified ||
        admission->payload_semantics_proven ||
        admission->visual_semantics_proven ||
        admission->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->startup_session_handoff_ready = 1;
    out->runtime_capture_required = 1;
    out->game_owned_fifo_payload_admitted = 1;
    out->variant = admission->game_owned_fifo_payload_variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             admission->game_owned_fifo_payload_track02_md5);
    out->record = admission->game_owned_fifo_payload_record;
    out->source_offset = admission->game_owned_fifo_payload_source_offset;
    out->source_byte = admission->game_owned_fifo_payload_source_byte;
    out->cdb_read6_verified = admission->cdb_read6_verified;
    out->fifo_to_game_ram_verified = admission->fifo_to_game_ram_verified;
    out->game_ram_consumer_verified = admission->game_ram_consumer_verified;
    out->payload_semantics_proven = 0;
    out->visual_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;
    out->object_table_admission_allowed = 0;
    out->level_admission_allowed = 0;
    return 1;
}

int theron_v1_runtime_session_handoff_bind_bounded_track02_route(
    const Theron_V1RuntimeSessionHandoffReceipt *session,
    const Theron_V1StartupAllDungeonRouteReceipt *route,
    Theron_V1RuntimeBoundedTrack02RouteReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_bounded_track02_route_init(out);
    if (!session || !route ||
        !session->valid ||
        !session->startup_session_handoff_ready ||
        !session->runtime_capture_required ||
        !session->game_owned_fifo_payload_admitted ||
        session->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(session->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        session->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        !session->cdb_read6_verified ||
        !session->fifo_to_game_ram_verified ||
        !session->game_ram_consumer_verified ||
        session->payload_semantics_proven ||
        session->visual_semantics_proven ||
        session->fallback_visuals_allowed ||
        session->object_table_admission_allowed ||
        session->level_admission_allowed ||
        !route->valid ||
        route->real_data_capture_ready ||
        route->capture_count <= 0 ||
        route->dungeon_mask == 0u ||
        route->exact_level_semantics_ready ||
        route->exact_object_semantics_ready ||
        !route->object_table_no_fallback_ready ||
        route->object_table_blocked_anchor_mask == 0u ||
        route->object_table_blocked_anchor_count <= 0 ||
        !route->nonstartup_level_no_fallback_ready ||
        route->nonstartup_level_blocked_anchor_mask == 0u ||
        route->nonstartup_level_blocked_anchor_count <= 0 ||
        route->startup_level_anchor_status[0] != THERON_TRACK02_LEVEL_HANDOFF_OK ||
        !route->startup_level_anchor_user_data_valid[0] ||
        route->startup_level_anchor_width[0] == 0u ||
        route->startup_level_anchor_height[0] == 0u ||
        route->object_table_route_hash == 0u ||
        route->level_route_hash == 0u ||
        route->route_hash == 0u) {
        return 0;
    }

    out->valid = 1;
    out->session_handoff_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = session->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             session->track02_md5);
    out->record = session->record;
    out->source_offset = session->source_offset;
    out->source_byte = session->source_byte;
    out->all_dungeon_capture_mask = route->dungeon_mask;
    out->all_dungeon_capture_count = route->capture_count;
    out->no_fallback_semantic_role_mask =
        route->no_fallback_semantic_role_mask;
    out->object_table_no_fallback_ready =
        route->object_table_no_fallback_ready;
    out->object_table_blocked_anchor_mask =
        route->object_table_blocked_anchor_mask;
    out->object_table_blocked_anchor_count =
        route->object_table_blocked_anchor_count;
    out->nonstartup_level_no_fallback_ready =
        route->nonstartup_level_no_fallback_ready;
    out->nonstartup_level_blocked_anchor_mask =
        route->nonstartup_level_blocked_anchor_mask;
    out->nonstartup_level_blocked_anchor_count =
        route->nonstartup_level_blocked_anchor_count;
    out->startup_level_blocked_anchor_mask =
        route->startup_level_blocked_anchor_mask;
    out->startup_level_blocked_anchor_count =
        route->startup_level_blocked_anchor_count;
    out->startup_level_anchor_status = route->startup_level_anchor_status[0];
    out->startup_level_anchor_raw_offset =
        route->startup_level_anchor_raw_offsets[0];
    out->startup_level_anchor_user_data_offset =
        route->startup_level_anchor_user_data_offsets[0];
    out->startup_level_anchor_user_data_valid =
        route->startup_level_anchor_user_data_valid[0];
    out->startup_level_anchor_width = route->startup_level_anchor_width[0];
    out->startup_level_anchor_height = route->startup_level_anchor_height[0];
    out->startup_level_anchor_seed = route->startup_level_anchor_seed[0];
    out->startup_level_anchor_level_index =
        route->startup_level_anchor_level_index[0];
    out->object_table_route_hash = route->object_table_route_hash;
    out->level_route_hash = route->level_route_hash;
    out->all_dungeon_route_hash = route->route_hash;
    out->exact_level_semantics_ready = 0;
    out->exact_object_semantics_ready = 0;
    out->object_table_admission_allowed = 0;
    out->level_admission_allowed = 0;
    out->payload_semantics_proven = 0;
    out->visual_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bounded_track02_route_bind_startup_level_anchor(
    const Theron_V1RuntimeBoundedTrack02RouteReceipt *route,
    Theron_V1RuntimeStartupLevelAnchorReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_startup_level_anchor_init(out);
    if (!route ||
        !route->valid ||
        !route->session_handoff_consumed ||
        !route->runtime_capture_required ||
        route->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(route->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        route->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        route->all_dungeon_capture_count <= 0 ||
        route->all_dungeon_capture_mask == 0u ||
        !route->object_table_no_fallback_ready ||
        route->object_table_blocked_anchor_mask == 0u ||
        route->object_table_blocked_anchor_count <= 0 ||
        !route->nonstartup_level_no_fallback_ready ||
        route->nonstartup_level_blocked_anchor_mask == 0u ||
        route->nonstartup_level_blocked_anchor_count <= 0 ||
        route->startup_level_anchor_status !=
            THERON_TRACK02_LEVEL_HANDOFF_OK ||
        !route->startup_level_anchor_user_data_valid ||
        route->startup_level_anchor_width == 0u ||
        route->startup_level_anchor_height == 0u ||
        route->startup_level_anchor_raw_offset == 0u ||
        route->startup_level_anchor_user_data_offset == 0u ||
        route->level_route_hash == 0u ||
        route->object_table_route_hash == 0u ||
        route->all_dungeon_route_hash == 0u ||
        route->exact_level_semantics_ready ||
        route->exact_object_semantics_ready ||
        route->object_table_admission_allowed ||
        route->level_admission_allowed ||
        route->payload_semantics_proven ||
        route->visual_semantics_proven ||
        route->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->bounded_route_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = route->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             route->track02_md5);
    out->record = route->record;
    out->source_offset = route->source_offset;
    out->source_byte = route->source_byte;
    out->startup_level_raw_offset = route->startup_level_anchor_raw_offset;
    out->startup_level_user_data_offset =
        route->startup_level_anchor_user_data_offset;
    out->startup_level_user_data_valid =
        route->startup_level_anchor_user_data_valid;
    out->startup_level_width = route->startup_level_anchor_width;
    out->startup_level_height = route->startup_level_anchor_height;
    out->startup_level_seed = route->startup_level_anchor_seed;
    out->startup_level_index = route->startup_level_anchor_level_index;
    out->level_route_hash = route->level_route_hash;
    out->object_table_route_hash = route->object_table_route_hash;
    out->all_dungeon_route_hash = route->all_dungeon_route_hash;
    out->startup_level_anchor_admitted = 1;
    out->object_table_admission_allowed = 0;
    out->nonstartup_level_admission_allowed = 0;
    out->exact_level_semantics_ready = 0;
    out->exact_object_semantics_ready = 0;
    out->payload_semantics_proven = 0;
    out->visual_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_startup_level_anchor_bind_nonstartup_level_route_evidence(
    const Theron_V1RuntimeStartupLevelAnchorReceipt *startup_anchor,
    const Theron_Track02LevelRouteReceipt *level_route,
    Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_nonstartup_level_route_evidence_init(out);
    if (!startup_anchor || !level_route ||
        !startup_anchor->valid ||
        !startup_anchor->bounded_route_consumed ||
        !startup_anchor->runtime_capture_required ||
        startup_anchor->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(startup_anchor->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        startup_anchor->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        !startup_anchor->startup_level_anchor_admitted ||
        !startup_anchor->startup_level_user_data_valid ||
        startup_anchor->startup_level_raw_offset == 0u ||
        startup_anchor->startup_level_user_data_offset == 0u ||
        startup_anchor->startup_level_width == 0u ||
        startup_anchor->startup_level_height == 0u ||
        startup_anchor->level_route_hash == 0u ||
        startup_anchor->object_table_route_hash == 0u ||
        startup_anchor->all_dungeon_route_hash == 0u ||
        startup_anchor->object_table_admission_allowed ||
        startup_anchor->nonstartup_level_admission_allowed ||
        startup_anchor->exact_level_semantics_ready ||
        startup_anchor->exact_object_semantics_ready ||
        startup_anchor->payload_semantics_proven ||
        startup_anchor->visual_semantics_proven ||
        startup_anchor->fallback_visuals_allowed ||
        !level_route->valid ||
        !level_route->verified_track02 ||
        level_route->variant != startup_anchor->variant ||
        level_route->signal_status != THERON_TRACK02_SIGNAL_OK ||
        !level_route->descriptor_route_ready ||
        level_route->descriptor_anchor_count == 0u ||
        level_route->descriptor_anchor_mask == 0u ||
        !level_route->startup_level_route_ready ||
        !level_route->startup_user_data_offset_valid ||
        level_route->startup_raw_offset !=
            startup_anchor->startup_level_raw_offset ||
        level_route->startup_user_data_offset !=
            startup_anchor->startup_level_user_data_offset ||
        level_route->startup_header_width !=
            startup_anchor->startup_level_width ||
        level_route->startup_header_height !=
            startup_anchor->startup_level_height ||
        level_route->startup_header_seed !=
            startup_anchor->startup_level_seed ||
        level_route->startup_header_level_index !=
            startup_anchor->startup_level_index ||
        level_route->nonstartup_level_candidate_count == 0u ||
        level_route->nonstartup_level_candidate_anchor_mask == 0u ||
        level_route->nonstartup_level_blocked_anchor_count == 0u ||
        level_route->nonstartup_level_blocked_anchor_mask == 0u ||
        level_route->nonstartup_level_candidate_anchor_mask !=
            level_route->nonstartup_level_blocked_anchor_mask ||
        !level_route->blocked_for_missing_nonstartup_level_evidence ||
        level_route->nonstartup_level_decode_ready ||
        level_route->fallback_visuals_allowed ||
        level_route->route_hash != startup_anchor->level_route_hash ||
        level_route->nonstartup_level_candidate_raw_offsets[0] == 0u ||
        !level_route->nonstartup_level_candidate_user_data_valid[0] ||
        level_route->nonstartup_level_candidate_user_data_offsets[0] == 0u ||
        level_route->nonstartup_level_candidate_byte_counts[0] == 0u ||
        level_route->nonstartup_level_candidate_hash[0] == 0u) {
        return 0;
    }

    out->valid = 1;
    out->startup_level_anchor_consumed = 1;
    out->level_route_receipt_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = startup_anchor->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             startup_anchor->track02_md5);
    out->record = startup_anchor->record;
    out->source_offset = startup_anchor->source_offset;
    out->source_byte = startup_anchor->source_byte;
    out->descriptor_anchor_mask = level_route->descriptor_anchor_mask;
    out->descriptor_anchor_count = (int)level_route->descriptor_anchor_count;
    out->nonstartup_level_candidate_anchor_mask =
        level_route->nonstartup_level_candidate_anchor_mask;
    out->nonstartup_level_candidate_count =
        (int)level_route->nonstartup_level_candidate_count;
    out->nonstartup_level_blocked_anchor_mask =
        level_route->nonstartup_level_blocked_anchor_mask;
    out->nonstartup_level_blocked_anchor_count =
        (int)level_route->nonstartup_level_blocked_anchor_count;
    out->first_candidate_raw_offset =
        level_route->nonstartup_level_candidate_raw_offsets[0];
    out->first_candidate_user_data_offset =
        level_route->nonstartup_level_candidate_user_data_offsets[0];
    out->first_candidate_user_data_valid =
        level_route->nonstartup_level_candidate_user_data_valid[0];
    out->first_candidate_byte_count =
        (uint32_t)level_route->nonstartup_level_candidate_byte_counts[0];
    out->first_candidate_hash =
        level_route->nonstartup_level_candidate_hash[0];
    out->first_candidate_header_width =
        level_route->nonstartup_level_candidate_header_width[0];
    out->first_candidate_header_height =
        level_route->nonstartup_level_candidate_header_height[0];
    out->first_candidate_header_seed =
        level_route->nonstartup_level_candidate_header_seed[0];
    out->first_candidate_header_level_index =
        level_route->nonstartup_level_candidate_header_level_index[0];
    out->level_route_hash = startup_anchor->level_route_hash;
    out->object_table_route_hash = startup_anchor->object_table_route_hash;
    out->all_dungeon_route_hash = startup_anchor->all_dungeon_route_hash;
    out->nonstartup_level_decode_ready = 0;
    out->nonstartup_level_admission_allowed = 0;
    out->exact_level_semantics_ready = 0;
    out->exact_object_semantics_ready = 0;
    out->payload_semantics_proven = 0;
    out->visual_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_startup_level_anchor_bind_object_table_route_evidence(
    const Theron_V1RuntimeStartupLevelAnchorReceipt *startup_anchor,
    const Theron_Track02ObjectTableRouteReceipt *object_route,
    Theron_V1RuntimeObjectTableRouteEvidenceReceipt *out) {

    size_t anchor;
    size_t first_anchor = THERON_TRACK02_MAX_BANK_ANCHORS;

    if (!out) {
        return 0;
    }
    theron_v1_runtime_object_table_route_evidence_init(out);
    if (!startup_anchor || !object_route ||
        !startup_anchor->valid ||
        !startup_anchor->bounded_route_consumed ||
        !startup_anchor->runtime_capture_required ||
        startup_anchor->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(startup_anchor->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        startup_anchor->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        !startup_anchor->startup_level_anchor_admitted ||
        !startup_anchor->startup_level_user_data_valid ||
        startup_anchor->startup_level_raw_offset == 0u ||
        startup_anchor->startup_level_user_data_offset == 0u ||
        startup_anchor->startup_level_width == 0u ||
        startup_anchor->startup_level_height == 0u ||
        startup_anchor->level_route_hash == 0u ||
        startup_anchor->object_table_route_hash == 0u ||
        startup_anchor->all_dungeon_route_hash == 0u ||
        startup_anchor->object_table_admission_allowed ||
        startup_anchor->nonstartup_level_admission_allowed ||
        startup_anchor->exact_level_semantics_ready ||
        startup_anchor->exact_object_semantics_ready ||
        startup_anchor->payload_semantics_proven ||
        startup_anchor->visual_semantics_proven ||
        startup_anchor->fallback_visuals_allowed ||
        !object_route->valid ||
        !object_route->verified_track02 ||
        object_route->variant != startup_anchor->variant ||
        object_route->signal_status != THERON_TRACK02_SIGNAL_OK ||
        !object_route->descriptor_route_ready ||
        object_route->descriptor_anchor_count == 0u ||
        object_route->descriptor_anchor_mask == 0u ||
        object_route->object_table_candidate_count == 0u ||
        object_route->object_table_candidate_anchor_mask == 0u ||
        object_route->object_table_blocked_anchor_count == 0u ||
        object_route->object_table_blocked_anchor_mask == 0u ||
        object_route->object_table_candidate_anchor_mask !=
            object_route->object_table_blocked_anchor_mask ||
        !object_route->blocked_for_missing_real_object_evidence ||
        object_route->object_table_decode_ready ||
        object_route->fallback_visuals_allowed ||
        object_route->route_hash != startup_anchor->object_table_route_hash) {
        return 0;
    }

    for (anchor = 0u; anchor < THERON_TRACK02_MAX_BANK_ANCHORS; ++anchor) {
        unsigned int anchor_bit = 1u << (unsigned int)anchor;
        if ((object_route->object_table_candidate_anchor_mask &
             anchor_bit) != 0u) {
            first_anchor = anchor;
            break;
        }
    }
    if (first_anchor >= THERON_TRACK02_MAX_BANK_ANCHORS ||
        object_route->object_table_candidate_raw_offsets[first_anchor] == 0u ||
        !object_route
             ->object_table_candidate_user_data_valid[first_anchor] ||
        object_route
             ->object_table_candidate_user_data_offsets[first_anchor] == 0u ||
        object_route->object_table_candidate_byte_counts[first_anchor] == 0u ||
        object_route->object_table_candidate_hash[first_anchor] == 0u ||
        !object_route
             ->object_table_candidate_after_descriptor[first_anchor]) {
        return 0;
    }

    out->valid = 1;
    out->startup_level_anchor_consumed = 1;
    out->object_table_route_receipt_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = startup_anchor->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             startup_anchor->track02_md5);
    out->record = startup_anchor->record;
    out->source_offset = startup_anchor->source_offset;
    out->source_byte = startup_anchor->source_byte;
    out->descriptor_anchor_mask = object_route->descriptor_anchor_mask;
    out->descriptor_anchor_count =
        (int)object_route->descriptor_anchor_count;
    out->object_table_candidate_anchor_mask =
        object_route->object_table_candidate_anchor_mask;
    out->object_table_candidate_count =
        (int)object_route->object_table_candidate_count;
    out->object_table_blocked_anchor_mask =
        object_route->object_table_blocked_anchor_mask;
    out->object_table_blocked_anchor_count =
        (int)object_route->object_table_blocked_anchor_count;
    out->first_candidate_entry_index =
        object_route->object_table_candidate_entry_index[first_anchor];
    out->first_candidate_raw_offset =
        object_route->object_table_candidate_raw_offsets[first_anchor];
    out->first_candidate_user_data_offset =
        object_route->object_table_candidate_user_data_offsets[first_anchor];
    out->first_candidate_user_data_valid =
        object_route->object_table_candidate_user_data_valid[first_anchor];
    out->first_candidate_byte_count =
        (uint32_t)object_route
            ->object_table_candidate_byte_counts[first_anchor];
    out->first_candidate_nonzero_byte_count =
        (uint32_t)object_route
            ->object_table_candidate_nonzero_byte_counts[first_anchor];
    out->first_candidate_hash =
        object_route->object_table_candidate_hash[first_anchor];
    out->first_candidate_descriptor_delta =
        (uint32_t)object_route
            ->object_table_candidate_descriptor_delta[first_anchor];
    out->first_candidate_after_descriptor =
        object_route->object_table_candidate_after_descriptor[first_anchor];
    out->first_candidate_binding_status =
        object_route->object_table_anchor_binding_status[first_anchor];
    out->first_candidate_reject_reason =
        (int)object_route->object_table_reject_reason[first_anchor];
    out->first_candidate_declared_record_count =
        (uint32_t)object_route
            ->object_table_declared_record_count[first_anchor];
    out->first_candidate_record_count =
        (uint32_t)object_route->object_table_record_count[first_anchor];
    out->first_candidate_required_byte_count =
        (uint32_t)object_route
            ->object_table_required_byte_count[first_anchor];
    out->object_table_route_hash = startup_anchor->object_table_route_hash;
    out->level_route_hash = startup_anchor->level_route_hash;
    out->all_dungeon_route_hash = startup_anchor->all_dungeon_route_hash;
    out->object_table_decode_ready = 0;
    out->object_table_admission_allowed = 0;
    out->exact_object_semantics_ready = 0;
    out->payload_semantics_proven = 0;
    out->visual_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bind_track02_capture_consumer_gap(
    const Theron_V1RuntimeStartupLevelAnchorReceipt *startup_anchor,
    const Theron_V1RuntimeNonstartupLevelRouteEvidenceReceipt *level_evidence,
    const Theron_V1RuntimeObjectTableRouteEvidenceReceipt *object_evidence,
    Theron_V1RuntimeTrack02CaptureConsumerGapReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_capture_consumer_gap_init(out);
    if (!startup_anchor || !level_evidence || !object_evidence ||
        !startup_anchor->valid ||
        !startup_anchor->bounded_route_consumed ||
        !startup_anchor->runtime_capture_required ||
        startup_anchor->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(startup_anchor->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        startup_anchor->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        !startup_anchor->startup_level_anchor_admitted ||
        startup_anchor->startup_level_raw_offset == 0u ||
        startup_anchor->startup_level_user_data_offset == 0u ||
        !startup_anchor->startup_level_user_data_valid ||
        startup_anchor->level_route_hash == 0u ||
        startup_anchor->object_table_route_hash == 0u ||
        startup_anchor->all_dungeon_route_hash == 0u ||
        startup_anchor->object_table_admission_allowed ||
        startup_anchor->nonstartup_level_admission_allowed ||
        startup_anchor->exact_level_semantics_ready ||
        startup_anchor->exact_object_semantics_ready ||
        startup_anchor->payload_semantics_proven ||
        startup_anchor->visual_semantics_proven ||
        startup_anchor->fallback_visuals_allowed ||
        !level_evidence->valid ||
        !level_evidence->startup_level_anchor_consumed ||
        !level_evidence->level_route_receipt_consumed ||
        !level_evidence->runtime_capture_required ||
        level_evidence->variant != startup_anchor->variant ||
        strcmp(level_evidence->track02_md5, startup_anchor->track02_md5) != 0 ||
        level_evidence->record != startup_anchor->record ||
        level_evidence->source_offset != startup_anchor->source_offset ||
        level_evidence->source_byte != startup_anchor->source_byte ||
        level_evidence->level_route_hash != startup_anchor->level_route_hash ||
        level_evidence->object_table_route_hash !=
            startup_anchor->object_table_route_hash ||
        level_evidence->all_dungeon_route_hash !=
            startup_anchor->all_dungeon_route_hash ||
        level_evidence->nonstartup_level_candidate_anchor_mask == 0u ||
        level_evidence->nonstartup_level_candidate_count <= 0 ||
        level_evidence->first_candidate_hash == 0u ||
        level_evidence->nonstartup_level_decode_ready ||
        level_evidence->nonstartup_level_admission_allowed ||
        level_evidence->exact_level_semantics_ready ||
        level_evidence->exact_object_semantics_ready ||
        level_evidence->payload_semantics_proven ||
        level_evidence->visual_semantics_proven ||
        level_evidence->fallback_visuals_allowed ||
        !object_evidence->valid ||
        !object_evidence->startup_level_anchor_consumed ||
        !object_evidence->object_table_route_receipt_consumed ||
        !object_evidence->runtime_capture_required ||
        object_evidence->variant != startup_anchor->variant ||
        strcmp(object_evidence->track02_md5, startup_anchor->track02_md5) != 0 ||
        object_evidence->record != startup_anchor->record ||
        object_evidence->source_offset != startup_anchor->source_offset ||
        object_evidence->source_byte != startup_anchor->source_byte ||
        object_evidence->level_route_hash != startup_anchor->level_route_hash ||
        object_evidence->object_table_route_hash !=
            startup_anchor->object_table_route_hash ||
        object_evidence->all_dungeon_route_hash !=
            startup_anchor->all_dungeon_route_hash ||
        object_evidence->object_table_candidate_anchor_mask == 0u ||
        object_evidence->object_table_candidate_count <= 0 ||
        object_evidence->first_candidate_hash == 0u ||
        object_evidence->object_table_decode_ready ||
        object_evidence->object_table_admission_allowed ||
        object_evidence->exact_object_semantics_ready ||
        object_evidence->payload_semantics_proven ||
        object_evidence->visual_semantics_proven ||
        object_evidence->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->startup_level_anchor_consumed = 1;
    out->nonstartup_level_evidence_consumed = 1;
    out->object_table_evidence_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = startup_anchor->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             startup_anchor->track02_md5);
    out->record = startup_anchor->record;
    out->source_offset = startup_anchor->source_offset;
    out->source_byte = startup_anchor->source_byte;
    out->level_route_hash = startup_anchor->level_route_hash;
    out->object_table_route_hash = startup_anchor->object_table_route_hash;
    out->all_dungeon_route_hash = startup_anchor->all_dungeon_route_hash;
    out->nonstartup_level_candidate_anchor_mask =
        level_evidence->nonstartup_level_candidate_anchor_mask;
    out->nonstartup_level_candidate_count =
        level_evidence->nonstartup_level_candidate_count;
    out->first_nonstartup_level_candidate_hash =
        level_evidence->first_candidate_hash;
    out->object_table_candidate_anchor_mask =
        object_evidence->object_table_candidate_anchor_mask;
    out->object_table_candidate_count =
        object_evidence->object_table_candidate_count;
    out->first_object_table_candidate_hash =
        object_evidence->first_candidate_hash;
    out->capture_consumer_route_ready = 0;
    out->object_table_decode_ready = 0;
    out->nonstartup_level_decode_ready = 0;
    out->object_table_admission_allowed = 0;
    out->nonstartup_level_admission_allowed = 0;
    out->exact_level_semantics_ready = 0;
    out->exact_object_semantics_ready = 0;
    out->payload_semantics_proven = 0;
    out->visual_semantics_proven = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bind_track02_consumer_semantics(
    const Theron_V1RuntimeTrack02CaptureConsumerGapReceipt *gap,
    const Theron_V1Track02Post3800ConsumerSemanticReceipt *consumer,
    Theron_V1RuntimeTrack02ConsumerSemanticReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_consumer_semantic_init(out);
    if (!gap || !consumer ||
        !gap->valid ||
        !gap->startup_level_anchor_consumed ||
        !gap->nonstartup_level_evidence_consumed ||
        !gap->object_table_evidence_consumed ||
        !gap->runtime_capture_required ||
        gap->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(gap->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        gap->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        gap->level_route_hash == 0u ||
        gap->object_table_route_hash == 0u ||
        gap->all_dungeon_route_hash == 0u ||
        gap->capture_consumer_route_ready ||
        gap->object_table_decode_ready ||
        gap->nonstartup_level_decode_ready ||
        gap->object_table_admission_allowed ||
        gap->nonstartup_level_admission_allowed ||
        gap->exact_level_semantics_ready ||
        gap->exact_object_semantics_ready ||
        gap->payload_semantics_proven ||
        gap->visual_semantics_proven ||
        gap->fallback_visuals_allowed ||
        !consumer->valid ||
        !consumer->no_fallback ||
        !consumer->original_consumer_trace_bound ||
        consumer->track02_variant != gap->variant ||
        consumer->record != gap->record ||
        consumer->payload_checksum == 0u ||
        consumer->level_envelope_checksum == 0u ||
        consumer->post_envelope_checksum == 0u ||
        consumer->consumer_trace_checksum == 0u ||
        !consumer->dungeon_record_semantics_proven ||
        !consumer->object_table_semantics_proven ||
        !consumer->bitmap_route_bound ||
        !consumer->palette_binding_verified ||
        !consumer->rgba_output_allowed ||
        consumer->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->capture_consumer_gap_consumed = 1;
    out->original_consumer_trace_bound = 1;
    out->runtime_capture_required = 1;
    out->variant = gap->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             gap->track02_md5);
    out->record = gap->record;
    out->source_offset = gap->source_offset;
    out->source_byte = gap->source_byte;
    out->level_route_hash = gap->level_route_hash;
    out->object_table_route_hash = gap->object_table_route_hash;
    out->all_dungeon_route_hash = gap->all_dungeon_route_hash;
    out->payload_checksum = consumer->payload_checksum;
    out->level_envelope_checksum = consumer->level_envelope_checksum;
    out->post_envelope_checksum = consumer->post_envelope_checksum;
    out->consumer_trace_checksum = consumer->consumer_trace_checksum;
    out->capture_consumer_route_ready = 1;
    out->exact_level_semantics_ready = 1;
    out->exact_object_semantics_ready = 1;
    out->payload_semantics_proven = 1;
    out->visual_semantics_proven = 1;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bind_track02_render_asset_admission(
    const Theron_V1RuntimeTrack02ConsumerSemanticReceipt *consumer,
    const Theron_V1RuntimeTrack02RenderAssetProof *proof,
    Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_render_asset_admission_init(out);
    if (!consumer || !proof ||
        !consumer->valid ||
        !consumer->capture_consumer_gap_consumed ||
        !consumer->original_consumer_trace_bound ||
        !consumer->runtime_capture_required ||
        consumer->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(consumer->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        consumer->source_offset >= THERON_TRACK02_RAW_SECTOR_BYTES ||
        consumer->level_route_hash == 0u ||
        consumer->object_table_route_hash == 0u ||
        consumer->all_dungeon_route_hash == 0u ||
        consumer->payload_checksum == 0u ||
        consumer->level_envelope_checksum == 0u ||
        consumer->post_envelope_checksum == 0u ||
        consumer->consumer_trace_checksum == 0u ||
        !consumer->capture_consumer_route_ready ||
        !consumer->exact_level_semantics_ready ||
        !consumer->exact_object_semantics_ready ||
        !consumer->payload_semantics_proven ||
        !consumer->visual_semantics_proven ||
        consumer->fallback_visuals_allowed ||
        !proof->valid ||
        !proof->same_capture_as_consumer_semantics ||
        proof->variant != consumer->variant ||
        strcmp(proof->track02_md5, consumer->track02_md5) != 0 ||
        proof->record != consumer->record ||
        proof->level_route_hash != consumer->level_route_hash ||
        proof->object_table_route_hash != consumer->object_table_route_hash ||
        proof->all_dungeon_route_hash != consumer->all_dungeon_route_hash ||
        proof->payload_checksum != consumer->payload_checksum ||
        proof->level_envelope_checksum != consumer->level_envelope_checksum ||
        proof->post_envelope_checksum != consumer->post_envelope_checksum ||
        proof->consumer_trace_checksum != consumer->consumer_trace_checksum ||
        proof->decoded_level_hash == 0u ||
        proof->decoded_object_table_hash == 0u ||
        proof->decoded_bitmap_hash == 0u ||
        proof->decoded_palette_hash == 0u ||
        !proof->level_consumer_proven ||
        !proof->object_table_consumer_proven ||
        !proof->bitmap_consumer_proven ||
        !proof->palette_consumer_proven ||
        !proof->decoded_bitmap_pixels_proven ||
        !proof->decoded_palette_words_proven ||
        proof->synthetic_level_promoted ||
        proof->synthetic_object_table_promoted ||
        proof->synthetic_bitmap_promoted ||
        proof->synthetic_palette_promoted ||
        proof->fallback_visuals_observed ||
        proof->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->consumer_semantics_consumed = 1;
    out->real_render_asset_proof_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = consumer->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             consumer->track02_md5);
    out->record = consumer->record;
    out->level_route_hash = consumer->level_route_hash;
    out->object_table_route_hash = consumer->object_table_route_hash;
    out->all_dungeon_route_hash = consumer->all_dungeon_route_hash;
    out->payload_checksum = consumer->payload_checksum;
    out->level_envelope_checksum = consumer->level_envelope_checksum;
    out->post_envelope_checksum = consumer->post_envelope_checksum;
    out->consumer_trace_checksum = consumer->consumer_trace_checksum;
    out->decoded_level_hash = proof->decoded_level_hash;
    out->decoded_object_table_hash = proof->decoded_object_table_hash;
    out->decoded_bitmap_hash = proof->decoded_bitmap_hash;
    out->decoded_palette_hash = proof->decoded_palette_hash;
    out->object_table_admission_allowed = 1;
    out->level_admission_allowed = 1;
    out->bitmap_admission_allowed = 1;
    out->palette_admission_allowed = 1;
    out->real_render_assets_admitted = 1;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bind_track02_dungeon_handoff(
    const Theron_V1RuntimeTrack02RenderAssetAdmissionReceipt *admission,
    const Theron_V1RuntimeTrack02DungeonHandoffProof *proof,
    Theron_V1RuntimeTrack02DungeonHandoffReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_dungeon_handoff_init(out);
    if (!admission || !proof ||
        !admission->valid ||
        !admission->consumer_semantics_consumed ||
        !admission->real_render_asset_proof_consumed ||
        !admission->runtime_capture_required ||
        admission->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(admission->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        admission->level_route_hash == 0u ||
        admission->object_table_route_hash == 0u ||
        admission->all_dungeon_route_hash == 0u ||
        admission->payload_checksum == 0u ||
        admission->level_envelope_checksum == 0u ||
        admission->post_envelope_checksum == 0u ||
        admission->consumer_trace_checksum == 0u ||
        admission->decoded_level_hash == 0u ||
        admission->decoded_object_table_hash == 0u ||
        admission->decoded_bitmap_hash == 0u ||
        admission->decoded_palette_hash == 0u ||
        !admission->object_table_admission_allowed ||
        !admission->level_admission_allowed ||
        !admission->bitmap_admission_allowed ||
        !admission->palette_admission_allowed ||
        !admission->real_render_assets_admitted ||
        admission->fallback_visuals_allowed ||
        !proof->valid ||
        !proof->same_capture_as_render_admission ||
        proof->variant != admission->variant ||
        strcmp(proof->track02_md5, admission->track02_md5) != 0 ||
        proof->record != admission->record ||
        proof->level_route_hash != admission->level_route_hash ||
        proof->object_table_route_hash != admission->object_table_route_hash ||
        proof->all_dungeon_route_hash != admission->all_dungeon_route_hash ||
        proof->payload_checksum != admission->payload_checksum ||
        proof->level_envelope_checksum !=
            admission->level_envelope_checksum ||
        proof->post_envelope_checksum != admission->post_envelope_checksum ||
        proof->consumer_trace_checksum != admission->consumer_trace_checksum ||
        proof->decoded_level_hash != admission->decoded_level_hash ||
        proof->decoded_object_table_hash !=
            admission->decoded_object_table_hash ||
        proof->decoded_bitmap_hash != admission->decoded_bitmap_hash ||
        proof->decoded_palette_hash != admission->decoded_palette_hash ||
        !proof->dungeon_runtime_consumer_bound ||
        !proof->object_table_layout_proven ||
        !proof->bitmap_palette_decode_proven ||
        !proof->source_level_bytes_bound ||
        !proof->source_object_table_bytes_bound ||
        !proof->source_bitmap_bytes_bound ||
        !proof->source_palette_bytes_bound ||
        proof->synthetic_dungeon_state_promoted ||
        proof->synthetic_object_layout_promoted ||
        proof->synthetic_bitmap_decode_promoted ||
        proof->synthetic_palette_decode_promoted ||
        proof->fallback_visuals_observed ||
        proof->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->render_asset_admission_consumed = 1;
    out->dungeon_handoff_proof_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = admission->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             admission->track02_md5);
    out->record = admission->record;
    out->level_route_hash = admission->level_route_hash;
    out->object_table_route_hash = admission->object_table_route_hash;
    out->all_dungeon_route_hash = admission->all_dungeon_route_hash;
    out->payload_checksum = admission->payload_checksum;
    out->level_envelope_checksum = admission->level_envelope_checksum;
    out->post_envelope_checksum = admission->post_envelope_checksum;
    out->consumer_trace_checksum = admission->consumer_trace_checksum;
    out->decoded_level_hash = admission->decoded_level_hash;
    out->decoded_object_table_hash = admission->decoded_object_table_hash;
    out->decoded_bitmap_hash = admission->decoded_bitmap_hash;
    out->decoded_palette_hash = admission->decoded_palette_hash;
    out->real_data_handoff_to_dungeon = 1;
    out->dungeon_state_admission_allowed = 1;
    out->object_table_layout_admission_allowed = 1;
    out->bitmap_palette_decode_admission_allowed = 1;
    out->dungeon_draw_allowed = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bind_track02_host_dungeon_consumer(
    const Theron_V1RuntimeTrack02DungeonHandoffReceipt *handoff,
    const Theron_V1RuntimeTrack02HostDungeonConsumerProof *proof,
    Theron_V1RuntimeTrack02HostDungeonConsumerReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_host_dungeon_consumer_init(out);
    if (!handoff || !proof ||
        !handoff->valid ||
        !handoff->render_asset_admission_consumed ||
        !handoff->dungeon_handoff_proof_consumed ||
        !handoff->runtime_capture_required ||
        handoff->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(handoff->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        handoff->level_route_hash == 0u ||
        handoff->object_table_route_hash == 0u ||
        handoff->all_dungeon_route_hash == 0u ||
        handoff->payload_checksum == 0u ||
        handoff->level_envelope_checksum == 0u ||
        handoff->post_envelope_checksum == 0u ||
        handoff->consumer_trace_checksum == 0u ||
        handoff->decoded_level_hash == 0u ||
        handoff->decoded_object_table_hash == 0u ||
        handoff->decoded_bitmap_hash == 0u ||
        handoff->decoded_palette_hash == 0u ||
        !handoff->real_data_handoff_to_dungeon ||
        !handoff->dungeon_state_admission_allowed ||
        !handoff->object_table_layout_admission_allowed ||
        !handoff->bitmap_palette_decode_admission_allowed ||
        handoff->dungeon_draw_allowed ||
        handoff->fallback_visuals_allowed ||
        !proof->valid ||
        !proof->same_capture_as_dungeon_handoff ||
        proof->variant != handoff->variant ||
        strcmp(proof->track02_md5, handoff->track02_md5) != 0 ||
        proof->record != handoff->record ||
        proof->level_route_hash != handoff->level_route_hash ||
        proof->object_table_route_hash != handoff->object_table_route_hash ||
        proof->all_dungeon_route_hash != handoff->all_dungeon_route_hash ||
        proof->payload_checksum != handoff->payload_checksum ||
        proof->level_envelope_checksum != handoff->level_envelope_checksum ||
        proof->post_envelope_checksum != handoff->post_envelope_checksum ||
        proof->consumer_trace_checksum != handoff->consumer_trace_checksum ||
        proof->decoded_level_hash != handoff->decoded_level_hash ||
        proof->decoded_object_table_hash != handoff->decoded_object_table_hash ||
        proof->decoded_bitmap_hash != handoff->decoded_bitmap_hash ||
        proof->decoded_palette_hash != handoff->decoded_palette_hash ||
        !proof->original_host_route_bound ||
        !proof->level_grid_runtime_consumer_bound ||
        !proof->object_table_runtime_consumer_bound ||
        !proof->bitmap_palette_runtime_consumer_bound ||
        !proof->host_surface_upload_proven ||
        !proof->host_capture_frame_proven ||
        proof->synthetic_host_frame_promoted ||
        proof->synthetic_level_grid_promoted ||
        proof->synthetic_object_table_promoted ||
        proof->synthetic_bitmap_palette_promoted ||
        proof->fallback_visuals_observed ||
        proof->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->dungeon_handoff_consumed = 1;
    out->host_consumer_proof_consumed = 1;
    out->runtime_capture_required = 1;
    out->variant = handoff->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             handoff->track02_md5);
    out->record = handoff->record;
    out->level_route_hash = handoff->level_route_hash;
    out->object_table_route_hash = handoff->object_table_route_hash;
    out->all_dungeon_route_hash = handoff->all_dungeon_route_hash;
    out->payload_checksum = handoff->payload_checksum;
    out->level_envelope_checksum = handoff->level_envelope_checksum;
    out->post_envelope_checksum = handoff->post_envelope_checksum;
    out->consumer_trace_checksum = handoff->consumer_trace_checksum;
    out->decoded_level_hash = handoff->decoded_level_hash;
    out->decoded_object_table_hash = handoff->decoded_object_table_hash;
    out->decoded_bitmap_hash = handoff->decoded_bitmap_hash;
    out->decoded_palette_hash = handoff->decoded_palette_hash;
    out->real_track02_dungeon_consumer_ready = 1;
    out->host_surface_upload_allowed = 1;
    out->host_capture_frame_required = 1;
    out->dungeon_draw_allowed = 1;
    out->fallback_visuals_allowed = 0;
    return 1;
}
