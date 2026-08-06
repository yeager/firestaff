#include "theron_v1_runtime_admission.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint32_t theron_v1_runtime_track02_host_route_identity_checksum(
    const char *identity) {
    uint32_t checksum = 2166136261u;

    if (!identity || identity[0] == '\0') return 0u;
    while (*identity) {
        checksum ^= (uint8_t)*identity++;
        checksum *= 16777619u;
    }
    return checksum;
}

int theron_v1_runtime_bind_track02_capture_campaign_admission(
    const Theron_StartupMediaStateReceipt *startup_media,
    const Theron_V1Track02CaptureCampaignReceipt *campaign,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *dungeon_window,
    Theron_V1RuntimeTrack02CaptureCampaignAdmissionReceipt *out) {
    Theron_V1RuntimeTrack02CaptureCampaignAdmissionReceipt receipt = {0};

    if (!out) return 0;
    *out = receipt;
    if (!startup_media || !campaign || !dungeon_window || !startup_media->startup_media_ready ||
        campaign->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN || !campaign->valid ||
        !campaign->independent_bundles_verified || !campaign->shared_track02_provenance_verified ||
        !campaign->shared_loader_provenance_verified || campaign->pixel_decode_allowed ||
        campaign->level_object_semantics_allowed || campaign->render_allowed || campaign->fallback_visuals_allowed ||
        dungeon_window->valid == 0 || !dungeon_window->opaque_route_ready ||
        dungeon_window->level_field_decoder_allowed || dungeon_window->object_field_decoder_allowed ||
        dungeon_window->bitmap_palette_admission_allowed || dungeon_window->pixel_decode_allowed ||
        dungeon_window->dungeon_draw_allowed || dungeon_window->fallback_visuals_allowed ||
        startup_media->track02_variant != (int)campaign->track02_variant ||
        strcmp(startup_media->track02_md5, campaign->track02_md5) ||
        dungeon_window->track02_variant != campaign->track02_variant ||
        strcmp(dungeon_window->track02_md5, campaign->track02_md5) ||
        dungeon_window->dungeon_record_window_checksum !=
            campaign->route_destination_identity[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF]) return 0;
    receipt.valid = 1;
    receipt.startup_capture_ready = 1;
    receipt.soul_room_capture_ready = 1;
    receipt.dungeon_capture_ready = 1;
    receipt.campaign_consumed = 1;
    receipt.startup_media_consumed = 1;
    receipt.dungeon_window_consumed = 1;
    receipt.track02_variant = campaign->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", campaign->track02_md5);
    *out = receipt;
    return 1;
}

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

void theron_v1_runtime_track02_original_data_binding_gap_init(
    Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
}

static void theron_v1_runtime_track02_original_consumer_binding_init(
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
        out->fail_closed_until_consumer_proven = 1;
    }
}

void theron_v1_runtime_track02_render_asset_proof_init(
    Theron_V1RuntimeTrack02RenderAssetProof *out) {
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

void theron_v1_runtime_track02_host_dungeon_consumer_proof_init(
    Theron_V1RuntimeTrack02HostDungeonConsumerProof *out) {
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

static int theron_v1_runtime_bitmap_atlas_complete(
    const Theron_Track02StartupBitmapAtlas *atlas) {
    const unsigned int required =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    return atlas &&
           atlas->variant == THERON_TRACK02_VARIANT_US_BIN &&
           (atlas->route_mask & required) == required &&
           atlas->route_count == THERON_TRACK02_STARTUP_BITMAP_ATLAS_ROUTE_MAX &&
           atlas->total_tile_count > 0u &&
           atlas->total_nonzero_pixel_count > 0u &&
           atlas->checksum != 0u;
}

static uint32_t theron_v1_runtime_mix_hash(uint32_t hash, uint32_t value) {
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t theron_v1_runtime_track02_loader_route_pair_hash(
    uint32_t record,
    uint32_t consumer_trace_checksum,
    uint32_t selected_dungeon_index,
    uint32_t level_route_hash,
    uint32_t object_table_route_hash,
    size_t nonstartup_level_byte_count,
    uint32_t nonstartup_level_raw_hash,
    size_t object_table_user_data_byte_count,
    uint32_t object_table_user_data_hash,
    uint32_t dungeon_record_consumer_pc,
    uint32_t object_table_consumer_pc) {
    uint32_t hash = 2166136261u;

    hash = theron_v1_runtime_mix_hash(hash, record);
    hash = theron_v1_runtime_mix_hash(hash, consumer_trace_checksum);
    hash = theron_v1_runtime_mix_hash(hash, selected_dungeon_index);
    hash = theron_v1_runtime_mix_hash(hash, level_route_hash);
    hash = theron_v1_runtime_mix_hash(hash, object_table_route_hash);
    hash = theron_v1_runtime_mix_hash(
        hash, (uint32_t)nonstartup_level_byte_count);
    hash = theron_v1_runtime_mix_hash(hash, nonstartup_level_raw_hash);
    hash = theron_v1_runtime_mix_hash(
        hash, (uint32_t)object_table_user_data_byte_count);
    hash = theron_v1_runtime_mix_hash(hash, object_table_user_data_hash);
    hash = theron_v1_runtime_mix_hash(hash, dungeon_record_consumer_pc);
    hash = theron_v1_runtime_mix_hash(hash, object_table_consumer_pc);
    return hash ? hash : 2166136261u;
}

static uint32_t theron_v1_runtime_track02_object_placement_state_hash(
    const Theron_Track02ObjectTable *objects,
    uint32_t selected_level_index) {
    uint32_t hash = 2166136261u;
    size_t i;

    if (!objects || !objects->shape_ok || objects->record_count == 0u ||
        selected_level_index >= THERON_TRACK02_DUNGEON_COUNT) {
        return 0u;
    }
    hash = theron_v1_runtime_mix_hash(hash, objects->checksum);
    hash = theron_v1_runtime_mix_hash(hash, (uint32_t)objects->record_count);
    hash = theron_v1_runtime_mix_hash(hash, selected_level_index);
    for (i = 0u; i < objects->record_count; ++i) {
        const Theron_Track02ObjectTableRecord *record = &objects->records[i];
        if (record->level_index != selected_level_index) {
            continue;
        }
        hash = theron_v1_runtime_mix_hash(hash, (uint32_t)i);
        hash = theron_v1_runtime_mix_hash(hash, record->object_id);
        hash = theron_v1_runtime_mix_hash(hash, record->kind);
        hash = theron_v1_runtime_mix_hash(hash, record->x);
        hash = theron_v1_runtime_mix_hash(hash, record->y);
        hash = theron_v1_runtime_mix_hash(hash, record->level_index);
        hash = theron_v1_runtime_mix_hash(hash, record->flags);
        hash = theron_v1_runtime_mix_hash(hash, record->argument);
    }
    return hash ? hash : 2166136261u;
}

static int theron_v1_runtime_track02_object_kind_supported(uint8_t kind) {
    return (kind >= THERON_OBJTYPE_CHEST &&
            kind <= THERON_OBJTYPE_TRIGGER) ||
           kind == THERON_OBJTYPE_QUEST_ITEM;
}

static uint32_t theron_v1_runtime_track02_object_runtime_state_hash(
    const Theron_Track02ObjectTable *objects,
    uint32_t selected_level_index,
    unsigned int *out_low_kind_mask,
    int *out_quest_item_seen) {
    uint32_t hash = 2166136261u;
    unsigned int low_kind_mask = 0u;
    int quest_item_seen = 0;
    size_t i;

    if (out_low_kind_mask) {
        *out_low_kind_mask = 0u;
    }
    if (out_quest_item_seen) {
        *out_quest_item_seen = 0;
    }
    if (!objects || !objects->shape_ok ||
        selected_level_index >= THERON_TRACK02_DUNGEON_COUNT) {
        return 0u;
    }
    for (i = 0u; i < objects->record_count; ++i) {
        const Theron_Track02ObjectTableRecord *record = &objects->records[i];
        uint8_t runtime_state;
        uint32_t runtime_flags;
        uint32_t runtime_quantity;

        if (record->level_index != selected_level_index) {
            continue;
        }
        if (!theron_v1_runtime_track02_object_kind_supported(record->kind)) {
            return 0u;
        }
        if (record->kind == THERON_OBJTYPE_QUEST_ITEM) {
            quest_item_seen = 1;
        } else {
            low_kind_mask |= 1u << record->kind;
        }
        runtime_state = (uint8_t)(record->flags & 0x03u);
        runtime_flags = record->flags;
        runtime_quantity = record->argument ? record->argument : 1u;
        hash = theron_v1_runtime_mix_hash(hash, (uint32_t)i);
        hash = theron_v1_runtime_mix_hash(hash, record->object_id);
        hash = theron_v1_runtime_mix_hash(hash, record->kind);
        hash = theron_v1_runtime_mix_hash(hash, runtime_state);
        hash = theron_v1_runtime_mix_hash(hash, record->x);
        hash = theron_v1_runtime_mix_hash(hash, record->y);
        hash = theron_v1_runtime_mix_hash(hash, record->level_index);
        hash = theron_v1_runtime_mix_hash(hash, runtime_flags);
        hash = theron_v1_runtime_mix_hash(hash, runtime_quantity);
    }
    if (out_low_kind_mask) {
        *out_low_kind_mask = low_kind_mask;
    }
    if (out_quest_item_seen) {
        *out_quest_item_seen = quest_item_seen;
    }
    return hash ? hash : 2166136261u;
}

static uint32_t theron_v1_runtime_trace_text_checksum(const char *text) {
    uint32_t hash = 2166136261u;
    const unsigned char *p = (const unsigned char *)text;

    while (p && *p) {
        hash ^= (uint32_t)(*p++);
        hash *= 16777619u;
    }
    return hash ? hash : 2166136261u;
}

static int theron_v1_runtime_raw_user_data_coordinate(
    size_t raw_offset,
    size_t user_data_offset,
    size_t byte_count,
    size_t *out_raw_sector,
    size_t *out_raw_sector_user_data_offset) {
    size_t raw_sector;
    size_t raw_sector_offset;
    size_t raw_user_offset;
    size_t expected_user_data_offset;

    if (out_raw_sector) *out_raw_sector = 0u;
    if (out_raw_sector_user_data_offset) {
        *out_raw_sector_user_data_offset = 0u;
    }
    if (raw_offset == 0u || user_data_offset == 0u || byte_count == 0u) {
        return 0;
    }
    raw_sector = raw_offset / THERON_TRACK02_RAW_SECTOR_BYTES;
    raw_sector_offset = raw_offset % THERON_TRACK02_RAW_SECTOR_BYTES;
    if (raw_sector_offset < THERON_TRACK02_RAW_USER_DATA_OFFSET ||
        raw_sector_offset >= THERON_TRACK02_RAW_USER_DATA_OFFSET +
            THERON_TRACK02_RAW_USER_DATA_BYTES) {
        return 0;
    }
    raw_user_offset = raw_sector_offset - THERON_TRACK02_RAW_USER_DATA_OFFSET;
    if (byte_count > THERON_TRACK02_RAW_USER_DATA_BYTES - raw_user_offset) {
        return 0;
    }
    expected_user_data_offset =
        raw_sector * THERON_TRACK02_RAW_USER_DATA_BYTES + raw_user_offset;
    if (user_data_offset != expected_user_data_offset) {
        return 0;
    }
    if (out_raw_sector) *out_raw_sector = raw_sector;
    if (out_raw_sector_user_data_offset) {
        *out_raw_sector_user_data_offset = raw_sector_offset;
    }
    return 1;
}

Theron_Track02SignalStatus theron_v1_track02_build_nonstartup_container_index(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02NonstartupContainerIndex *out_index) {
    Theron_Track02NonstartupSectorReceipt receipt;
    Theron_Track02SignalStatus status;
    size_t anchor;
    size_t window_index;
    uint32_t index_hash = 2166136261u;

    if (!out_index) {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }
    memset(out_index, 0, sizeof(*out_index));
    if (!track02_data || track02_size == 0u || !md5_hex ||
        md5_hex[0] == '\0') {
        return THERON_TRACK02_SIGNAL_BAD_INPUT;
    }

    status = theron_v1_track02_capture_nonstartup_sector_receipt(
        track02_data, track02_size, md5_hex, &receipt);
    if (status != THERON_TRACK02_SIGNAL_OK || !receipt.valid ||
        !receipt.verified_track02 || !receipt.opaque_only ||
        !receipt.promotion_blocked || receipt.receipt_hash == 0u) {
        return status == THERON_TRACK02_SIGNAL_OK ?
            THERON_TRACK02_SIGNAL_NOT_FOUND : status;
    }

    out_index->variant = receipt.variant;
    out_index->verified_track02 = 1;
    out_index->opaque_only = 1;
    out_index->promotion_blocked = 1;
    index_hash = theron_v1_runtime_mix_hash(index_hash, receipt.receipt_hash);

    for (anchor = 0u; anchor < receipt.anchor_count; ++anchor) {
        for (window_index = 0u;
             window_index < receipt.window_count[anchor];
             ++window_index) {
            const Theron_Track02NonstartupSectorWindowReceipt *window =
                &receipt.windows[anchor][window_index];
            Theron_Track02NonstartupContainer *container;

            if (out_index->container_count >=
                THERON_TRACK02_MAX_NONSTARTUP_CONTAINERS) {
                return THERON_TRACK02_SIGNAL_BAD_INPUT;
            }
            if (!window->opaque || !window->promotion_blocked ||
                !window->user_data_span_contiguous ||
                window->raw_span_contains_non_user_data ||
                window->raw_span_hash == 0u ||
                window->byte_count == 0u ||
                window->user_data_end_offset <= window->user_data_offset ||
                window->user_data_end_offset - window->user_data_offset !=
                    window->byte_count ||
                window->raw_offset > track02_size ||
                window->byte_count > track02_size - window->raw_offset) {
                continue;
            }

            container = &out_index->containers[out_index->container_count++];
            container->descriptor_entry_index = window->descriptor_entry_index;
            container->raw_offset = window->raw_offset;
            container->user_data_offset = window->user_data_offset;
            container->user_data_byte_count = window->byte_count;
            container->user_data_hash = window->raw_span_hash;
            container->user_data_segment_count = 1u;
            container->user_data_segments[0].user_data_offset =
                window->user_data_offset;
            container->user_data_segments[0].byte_count = window->byte_count;
            container->user_data_segments[0].hash = window->raw_span_hash;
            container->opaque = 1;
            container->promotion_blocked = 1;

            index_hash = theron_v1_runtime_mix_hash(
                index_hash, (uint32_t)container->descriptor_entry_index);
            index_hash = theron_v1_runtime_mix_hash(
                index_hash, (uint32_t)container->raw_offset);
            index_hash = theron_v1_runtime_mix_hash(
                index_hash, (uint32_t)container->user_data_offset);
            index_hash = theron_v1_runtime_mix_hash(
                index_hash, (uint32_t)container->user_data_byte_count);
            index_hash = theron_v1_runtime_mix_hash(
                index_hash, container->user_data_hash);
        }
    }

    if (out_index->container_count == 0u) {
        memset(out_index, 0, sizeof(*out_index));
        return THERON_TRACK02_SIGNAL_NOT_FOUND;
    }
    out_index->valid = 1;
    out_index->index_hash = index_hash ? index_hash : 2166136261u;
    return THERON_TRACK02_SIGNAL_OK;
}

static int theron_v1_runtime_trace_has(const char *trace, const char *token) {
    return trace && token && strstr(trace, token) != NULL;
}

static int theron_v1_runtime_trace_has_u32(
    const char *trace,
    const char *name,
    uint32_t value) {
    char token[80];

    snprintf(token, sizeof(token), "%s=%u", name, value);
    if (theron_v1_runtime_trace_has(trace, token)) {
        return 1;
    }
    snprintf(token, sizeof(token), "%s=0x%08x", name, value);
    return theron_v1_runtime_trace_has(trace, token);
}

static int theron_v1_runtime_trace_has_size(
    const char *trace,
    const char *name,
    size_t value) {
    char token[96];

    snprintf(token, sizeof(token), "%s=%zu", name, value);
    if (theron_v1_runtime_trace_has(trace, token)) {
        return 1;
    }
    snprintf(token, sizeof(token), "%s=0x%zx", name, value);
    return theron_v1_runtime_trace_has(trace, token);
}

static int theron_v1_runtime_trace_read_unsigned(
    const char *trace,
    const char *name,
    unsigned long long max_value,
    unsigned long long *out_value) {
    char token[80];
    const char *match;

    if (out_value) {
        *out_value = 0u;
    }
    if (!trace || !name || !out_value) {
        return 0;
    }
    snprintf(token, sizeof(token), "%s=", name);
    match = strstr(trace, token);
    if (!match) {
        return 0;
    }
    match += strlen(token);
    {
        char *end = NULL;
        unsigned long long value = strtoull(match, &end, 0);
        if (end == match || value > max_value) {
            return 0;
        }
        *out_value = value;
    }
    return 1;
}

static int theron_v1_runtime_trace_read_u32(
    const char *trace,
    const char *name,
    uint32_t *out_value) {
    unsigned long long value;

    if (!out_value ||
        !theron_v1_runtime_trace_read_unsigned(
            trace, name, 0xffffffffull, &value)) {
        return 0;
    }
    *out_value = (uint32_t)value;
    return 1;
}

static int theron_v1_runtime_trace_read_size(
    const char *trace,
    const char *name,
    size_t *out_value) {
    unsigned long long value;

    if (!out_value ||
        !theron_v1_runtime_trace_read_unsigned(
            trace, name, (unsigned long long)((size_t)-1), &value)) {
        return 0;
    }
    *out_value = (size_t)value;
    return 1;
}

int theron_v1_runtime_track02_render_asset_proof_from_decoded_routes(
    const Theron_V1RuntimeTrack02ConsumerSemanticReceipt *consumer,
    const Theron_Track02LevelRouteReceipt *level_route,
    const Theron_Track02ObjectTableRouteReceipt *object_route,
    const Theron_Track02StartupBitmapAtlas *bitmap_atlas,
    const Theron_Track02PaletteWindowEvidence *palette_window,
    Theron_V1RuntimeTrack02RenderAssetProof *out) {

    uint32_t level_hash = 2166136261u;
    uint32_t object_hash = 2166136261u;
    uint32_t palette_hash = 2166136261u;
    size_t level_index;

    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_render_asset_proof_init(out);
    if (!consumer || !level_route || !object_route ||
        !bitmap_atlas || !palette_window ||
        !consumer->valid ||
        !consumer->capture_consumer_gap_consumed ||
        !consumer->original_consumer_trace_bound ||
        !consumer->runtime_capture_required ||
        consumer->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(consumer->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        consumer->record == 0u ||
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
        !level_route->valid ||
        !level_route->verified_track02 ||
        level_route->variant != consumer->variant ||
        level_route->signal_status != THERON_TRACK02_SIGNAL_OK ||
        level_route->route_hash != consumer->level_route_hash ||
        !level_route->descriptor_route_ready ||
        !level_route->startup_level_route_ready ||
        !level_route->startup_user_data_offset_valid ||
        !level_route->nonstartup_level_decode_ready ||
        level_route->fallback_visuals_allowed ||
        !object_route->valid ||
        !object_route->verified_track02 ||
        object_route->variant != consumer->variant ||
        object_route->signal_status != THERON_TRACK02_SIGNAL_OK ||
        object_route->route_hash != consumer->object_table_route_hash ||
        !object_route->descriptor_route_ready ||
        !object_route->object_table_decode_ready ||
        object_route->fallback_visuals_allowed ||
        !theron_v1_runtime_bitmap_atlas_complete(bitmap_atlas) ||
        !theron_v1_track02_palette_window_evidence_can_promote(
            palette_window) ||
        palette_window->variant != consumer->variant ||
        palette_window->payload_checksum == 0u ||
        palette_window->palette.checksum == 0u ||
        palette_window->palette.nonblack_entry_count == 0u) {
        return 0;
    }

    level_hash = theron_v1_runtime_mix_hash(
        level_hash, level_route->route_hash);
    level_hash = theron_v1_runtime_mix_hash(
        level_hash, (uint32_t)level_route->startup_level_grid_raw_offset);
    level_hash = theron_v1_runtime_mix_hash(
        level_hash, (uint32_t)level_route->startup_level_grid_user_data_offset);
    level_hash = theron_v1_runtime_mix_hash(
        level_hash, level_route->nonstartup_level_candidate_anchor_mask);
    for (level_index = 0u;
         level_index < THERON_TRACK02_MAX_BANK_ANCHORS;
         ++level_index) {
        level_hash = theron_v1_runtime_mix_hash(
            level_hash,
            level_route->nonstartup_level_candidate_hash[level_index]);
    }

    object_hash = theron_v1_runtime_mix_hash(
        object_hash, object_route->route_hash);
    object_hash = theron_v1_runtime_mix_hash(
        object_hash, object_route->object_table_level_consensus_mask);
    for (level_index = 0u;
         level_index < THERON_TRACK02_DUNGEON_COUNT;
         ++level_index) {
        object_hash = theron_v1_runtime_mix_hash(
            object_hash,
            object_route->object_table_level_consensus_record_hashes
                [level_index]);
        object_hash = theron_v1_runtime_mix_hash(
            object_hash,
            object_route->object_table_level_consensus_position_hashes
                [level_index]);
    }

    palette_hash = theron_v1_runtime_mix_hash(
        palette_hash, palette_window->payload_checksum);
    palette_hash = theron_v1_runtime_mix_hash(
        palette_hash, palette_window->palette.checksum);
    palette_hash = theron_v1_runtime_mix_hash(
        palette_hash,
        (uint32_t)palette_window->palette.nonblack_entry_count);

    if (level_hash == 0u || object_hash == 0u || palette_hash == 0u) {
        return 0;
    }

    out->valid = 1;
    out->same_capture_as_consumer_semantics = 1;
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
    out->decoded_level_hash = level_hash;
    out->decoded_object_table_hash = object_hash;
    out->decoded_bitmap_hash = bitmap_atlas->checksum;
    out->decoded_palette_hash = palette_hash;
    out->level_consumer_proven = 1;
    out->object_table_consumer_proven = 1;
    out->bitmap_consumer_proven = 1;
    out->palette_consumer_proven = 1;
    out->decoded_bitmap_pixels_proven = 1;
    out->decoded_palette_words_proven = 1;
    out->synthetic_level_promoted = 0;
    out->synthetic_object_table_promoted = 0;
    out->synthetic_bitmap_promoted = 0;
    out->synthetic_palette_promoted = 0;
    out->fallback_visuals_observed = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_track02_render_asset_proof_from_track02_capture(
    const Theron_V1RuntimeTrack02ConsumerSemanticReceipt *consumer,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    size_t palette_raw_offset,
    int palette_semantic_binding_verified,
    Theron_V1RuntimeTrack02RenderAssetProof *out) {

    Theron_Track02LevelRouteReceipt level_route;
    Theron_Track02ObjectTableRouteReceipt object_route;
    Theron_Track02StartupBitmapCatalog bitmap_catalog;
    Theron_Track02StartupBitmapAtlas bitmap_atlas;
    Theron_Track02PaletteWindowEvidence palette_window;

    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_render_asset_proof_init(out);
    /* A caller-supplied boolean is not source evidence. Earlier revisions
     * copied `palette_semantic_binding_verified` into the palette receipt and
     * could promote a palette-shaped window without an authenticated HuC6280
     * consumer trace. Keep this compatibility entry point fail-closed until
     * the real loader/consumer binding is passed as structured evidence. */
    if (palette_semantic_binding_verified) {
        return 0;
    }
    if (!consumer || !track02_data || track02_size == 0u ||
        !track02_md5 || track02_md5[0] == '\0' ||
        !consumer->valid ||
        strcmp(consumer->track02_md5, track02_md5) != 0) {
        return 0;
    }

    if (!theron_v1_track02_capture_level_route_receipt(
            track02_data, track02_size, track02_md5, &level_route) ||
        !theron_v1_track02_capture_object_table_route_receipt(
            track02_data, track02_size, track02_md5, &object_route)) {
        return 0;
    }

    memset(&bitmap_catalog, 0, sizeof(bitmap_catalog));
    memset(&bitmap_atlas, 0, sizeof(bitmap_atlas));
    if (theron_v1_track02_catalog_startup_bitmap_samples(
            track02_data, track02_size, track02_md5, &bitmap_catalog) !=
            THERON_TRACK02_SIGNAL_OK ||
        theron_v1_track02_build_startup_bitmap_atlas_wide(
            &bitmap_catalog, &bitmap_atlas) != THERON_TRACK02_SIGNAL_OK) {
        return 0;
    }

    memset(&palette_window, 0, sizeof(palette_window));
    if (theron_v1_track02_inspect_4bpp_palette_window(
            track02_data,
            track02_size,
            track02_md5,
            palette_raw_offset,
            &palette_window) != THERON_TRACK02_SIGNAL_OK) {
        return 0;
    }
    return theron_v1_runtime_track02_render_asset_proof_from_decoded_routes(
        consumer,
        &level_route,
        &object_route,
        &bitmap_atlas,
        &palette_window,
        out);
}

int theron_v1_runtime_track02_capture_original_data_binding_gap(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    size_t palette_raw_offset,
    Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *out) {

    Theron_Track02LevelRouteReceipt level_route;
    Theron_Track02ObjectTableRouteReceipt object_route;
    Theron_Track02NonstartupSectorReceipt sector_receipt;
    Theron_Track02NonstartupContainerIndex container_index;
    Theron_Track02PaletteWindowEvidence palette_window;
    size_t anchor;

    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_original_data_binding_gap_init(out);
    if (!track02_data || track02_size == 0u ||
        !track02_md5 || track02_md5[0] == '\0') {
        return 0;
    }
    (void)theron_v1_track02_capture_level_route_receipt(
        track02_data, track02_size, track02_md5, &level_route);
    if (level_route.signal_status != THERON_TRACK02_SIGNAL_OK ||
        !theron_v1_track02_capture_object_table_route_receipt(
            track02_data, track02_size, track02_md5, &object_route) ||
        theron_v1_track02_capture_nonstartup_sector_receipt(
            track02_data, track02_size, track02_md5, &sector_receipt) !=
            THERON_TRACK02_SIGNAL_OK ||
        theron_v1_track02_build_nonstartup_container_index(
            track02_data, track02_size, track02_md5, &container_index) !=
            THERON_TRACK02_SIGNAL_OK ||
        theron_v1_track02_inspect_4bpp_palette_window(
            track02_data, track02_size, track02_md5, palette_raw_offset,
            &palette_window) != THERON_TRACK02_SIGNAL_OK) {
        theron_v1_runtime_track02_original_data_binding_gap_init(out);
        return 0;
    }

    out->valid = 1;
    out->verified_track02_capture_consumed = 1;
    out->fail_closed = 1;
    out->variant = level_route.variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s", track02_md5);
    out->level_route_hash = level_route.route_hash;
    out->object_table_route_hash = object_route.route_hash;
    out->nonstartup_sector_receipt_hash = sector_receipt.receipt_hash;
    out->nonstartup_container_index_hash = container_index.index_hash;
    out->palette_raw_offset = palette_window.raw_offset;
    out->palette_user_data_offset = palette_window.user_data_offset;
    out->palette_payload_checksum = palette_window.payload_checksum;
    out->palette_decoded_checksum = palette_window.palette.checksum;
    out->palette_format_valid = palette_window.format_valid;
    out->palette_semantic_binding_verified =
        palette_window.semantic_binding_verified;
    out->palette_promotion_allowed = palette_window.promotion_allowed;
    out->nonstartup_anchor_count = sector_receipt.anchor_count;
    for (anchor = 0u; anchor < sector_receipt.anchor_count; ++anchor) {
        out->nonstartup_window_count += sector_receipt.window_count[anchor];
        if (out->first_nonstartup_byte_count == 0u &&
            sector_receipt.window_count[anchor] > 0u) {
            const Theron_Track02NonstartupSectorWindowReceipt *window =
                &sector_receipt.windows[anchor][0];
            out->first_nonstartup_entry_index =
                window->descriptor_entry_index;
            out->first_nonstartup_raw_offset = window->raw_offset;
            out->first_nonstartup_user_data_offset =
                window->user_data_offset;
            out->first_nonstartup_byte_count = window->byte_count;
            out->first_nonstartup_raw_hash = window->raw_span_hash;
        }
    }
    out->indexed_container_count = container_index.container_count;
    if (container_index.container_count > 0u) {
        const Theron_Track02NonstartupContainer *container =
            &container_index.containers[0];
        out->first_container_entry_index =
            container->descriptor_entry_index;
        out->first_container_raw_offset = container->raw_offset;
        out->first_container_user_data_byte_count =
            container->user_data_byte_count;
        out->first_container_user_data_hash = container->user_data_hash;
        if (container->user_data_segment_count > 0u) {
            out->first_container_user_data_offset =
                container->user_data_segments[0].user_data_offset;
        }
    }
    out->nonstartup_level_decode_ready =
        level_route.nonstartup_level_decode_ready;
    out->object_table_decode_ready = object_route.object_table_decode_ready;
    out->render_asset_admission_allowed = 0;
    out->fallback_visuals_allowed = 0;
    return out->valid;
}

int theron_v1_runtime_bind_track02_original_consumer_trace(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1Track02Post3800ConsumerSemanticReceipt *consumer,
    size_t palette_raw_offset,
    size_t nonstartup_level_raw_offset,
    size_t object_table_raw_offset,
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_original_consumer_binding_init(out);
    if (!gap || !consumer ||
        !gap->valid ||
        !gap->verified_track02_capture_consumed ||
        !gap->fail_closed ||
        gap->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(gap->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        gap->level_route_hash == 0u ||
        gap->object_table_route_hash == 0u ||
        gap->palette_raw_offset != palette_raw_offset ||
        gap->palette_payload_checksum == 0u ||
        gap->palette_decoded_checksum == 0u ||
        !gap->palette_format_valid ||
        gap->palette_semantic_binding_verified ||
        gap->palette_promotion_allowed ||
        gap->first_nonstartup_raw_offset != nonstartup_level_raw_offset ||
        gap->first_nonstartup_user_data_offset == 0u ||
        gap->first_nonstartup_raw_hash == 0u ||
        gap->first_container_raw_offset != object_table_raw_offset ||
        gap->first_container_user_data_offset == 0u ||
        gap->first_container_user_data_hash == 0u ||
        gap->nonstartup_level_decode_ready ||
        gap->object_table_decode_ready ||
        gap->render_asset_admission_allowed ||
        gap->fallback_visuals_allowed ||
        !consumer->valid ||
        !consumer->no_fallback ||
        !consumer->original_consumer_trace_bound ||
        consumer->track02_variant != gap->variant ||
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
    out->original_data_gap_consumed = 1;
    out->original_consumer_trace_consumed = 1;
    out->same_original_capture_as_gap = 1;
    out->fail_closed_until_consumer_proven = 0;
    out->variant = gap->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             gap->track02_md5);
    out->record = consumer->record;
    out->consumer_trace_checksum = consumer->consumer_trace_checksum;
    out->payload_checksum = consumer->payload_checksum;
    out->level_envelope_checksum = consumer->level_envelope_checksum;
    out->post_envelope_checksum = consumer->post_envelope_checksum;
    out->loader_record_user_data_offset =
        consumer->loader_record_user_data_offset;
    out->loader_destination = consumer->loader_destination;
    out->loader_payload_bytes = consumer->loader_payload_bytes;
    out->palette_raw_offset = gap->palette_raw_offset;
    out->palette_user_data_offset = gap->palette_user_data_offset;
    out->palette_payload_checksum = gap->palette_payload_checksum;
    out->palette_decoded_checksum = gap->palette_decoded_checksum;
    out->nonstartup_level_raw_offset = gap->first_nonstartup_raw_offset;
    out->nonstartup_level_user_data_offset =
        gap->first_nonstartup_user_data_offset;
    out->nonstartup_level_raw_hash = gap->first_nonstartup_raw_hash;
    out->object_table_raw_offset = gap->first_container_raw_offset;
    out->object_table_user_data_offset =
        gap->first_container_user_data_offset;
    out->object_table_user_data_hash = gap->first_container_user_data_hash;
    out->level_route_hash = gap->level_route_hash;
    out->object_table_route_hash = gap->object_table_route_hash;
    out->palette_consumer_bound = 1;
    out->nonstartup_level_consumer_bound = 1;
    out->object_table_consumer_bound = 1;
    out->bitmap_consumer_bound = 1;
    out->runtime_consumer_binding_ready = 1;
    out->render_asset_admission_allowed = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bind_track02_original_object_dungeon_consumer_trace(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1Track02ObjectDungeonConsumerGrammarReceipt *grammar,
    Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_original_consumer_binding_init(out);
    if (!gap || !grammar ||
        !gap->valid ||
        !gap->verified_track02_capture_consumed ||
        !gap->fail_closed ||
        gap->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(gap->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        gap->level_route_hash == 0u ||
        gap->object_table_route_hash == 0u ||
        gap->first_nonstartup_raw_offset == 0u ||
        gap->first_nonstartup_user_data_offset == 0u ||
        gap->first_nonstartup_raw_hash == 0u ||
        gap->first_container_raw_offset == 0u ||
        gap->first_container_user_data_offset == 0u ||
        gap->first_container_user_data_hash == 0u ||
        gap->nonstartup_level_decode_ready ||
        gap->object_table_decode_ready ||
        gap->render_asset_admission_allowed ||
        gap->fallback_visuals_allowed ||
        !grammar->valid ||
        !grammar->no_fallback ||
        !grammar->original_consumer_trace_bound ||
        !grammar->same_capture_as_loader_payload ||
        grammar->track02_variant != gap->variant ||
        grammar->record == 0u ||
        grammar->loader_record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        grammar->loader_destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        grammar->loader_payload_bytes !=
            THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        grammar->payload_checksum == 0u ||
        grammar->level_envelope_checksum == 0u ||
        grammar->post_envelope_checksum == 0u ||
        grammar->consumer_trace_checksum == 0u ||
        grammar->dungeon_record_consumer_pc == 0u ||
        grammar->object_table_consumer_pc == 0u ||
        grammar->dungeon_record_payload_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        grammar->dungeon_record_byte_count !=
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        grammar->dungeon_record_window_checksum !=
            grammar->level_envelope_checksum ||
        grammar->object_table_payload_offset !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        grammar->object_table_byte_count !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        grammar->object_table_window_checksum !=
            grammar->post_envelope_checksum ||
        !grammar->dungeon_record_grammar_proven ||
        !grammar->object_table_grammar_proven ||
        !grammar->dungeon_record_fields_blocked ||
        !grammar->object_table_fields_blocked ||
        grammar->bitmap_route_bound ||
        grammar->palette_binding_verified ||
        grammar->rgba_output_allowed ||
        grammar->runtime_handoff_allowed ||
        grammar->fallback_visuals_allowed) {
        return 0;
    }

    out->valid = 1;
    out->original_data_gap_consumed = 1;
    out->original_consumer_trace_consumed = 1;
    out->same_original_capture_as_gap = 1;
    out->fail_closed_until_consumer_proven = 0;
    out->variant = gap->variant;
    snprintf(out->track02_md5, sizeof(out->track02_md5), "%s",
             gap->track02_md5);
    out->record = grammar->record;
    out->consumer_trace_checksum = grammar->consumer_trace_checksum;
    out->payload_checksum = grammar->payload_checksum;
    out->level_envelope_checksum = grammar->level_envelope_checksum;
    out->post_envelope_checksum = grammar->post_envelope_checksum;
    out->loader_record_user_data_offset =
        grammar->loader_record_user_data_offset;
    out->loader_destination = grammar->loader_destination;
    out->loader_payload_bytes = grammar->loader_payload_bytes;
    out->dungeon_record_consumer_pc = grammar->dungeon_record_consumer_pc;
    out->object_table_consumer_pc = grammar->object_table_consumer_pc;
    out->dungeon_record_payload_offset =
        grammar->dungeon_record_payload_offset;
    out->dungeon_record_byte_count = grammar->dungeon_record_byte_count;
    out->dungeon_record_window_checksum =
        grammar->dungeon_record_window_checksum;
    out->object_table_payload_offset = grammar->object_table_payload_offset;
    out->object_table_byte_count = grammar->object_table_byte_count;
    out->object_table_window_checksum =
        grammar->object_table_window_checksum;
    out->nonstartup_level_raw_offset = gap->first_nonstartup_raw_offset;
    out->nonstartup_level_user_data_offset =
        gap->first_nonstartup_user_data_offset;
    out->nonstartup_level_raw_hash = gap->first_nonstartup_raw_hash;
    out->object_table_raw_offset = gap->first_container_raw_offset;
    out->object_table_user_data_offset =
        gap->first_container_user_data_offset;
    out->object_table_user_data_hash = gap->first_container_user_data_hash;
    out->level_route_hash = gap->level_route_hash;
    out->object_table_route_hash = gap->object_table_route_hash;
    out->palette_consumer_bound = 0;
    out->nonstartup_level_consumer_bound = 1;
    out->object_table_consumer_bound = 1;
    out->bitmap_consumer_bound = 0;
    out->runtime_consumer_binding_ready = 1;
    out->render_asset_admission_allowed = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_bind_track02_raw_nonstartup_dungeon_handoff(
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1RuntimeTrack02OriginalConsumerBindingReceipt *binding,
    Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt *out) {
    Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt receipt = {0};
    size_t level_raw_sector;
    size_t level_raw_sector_user_data_offset;
    size_t object_raw_sector;
    size_t object_raw_sector_user_data_offset;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!gap || !binding || !out ||
        !gap->valid ||
        !gap->verified_track02_capture_consumed ||
        !gap->fail_closed ||
        gap->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(gap->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        gap->level_route_hash == 0u ||
        gap->object_table_route_hash == 0u ||
        gap->first_nonstartup_raw_offset == 0u ||
        gap->first_nonstartup_user_data_offset == 0u ||
        gap->first_nonstartup_byte_count == 0u ||
        gap->first_nonstartup_raw_hash == 0u ||
        gap->first_container_raw_offset == 0u ||
        gap->first_container_user_data_offset == 0u ||
        gap->first_container_user_data_byte_count == 0u ||
        gap->first_container_user_data_hash == 0u ||
        gap->nonstartup_level_decode_ready ||
        gap->object_table_decode_ready ||
        gap->render_asset_admission_allowed ||
        gap->fallback_visuals_allowed ||
        !binding->valid ||
        !binding->original_data_gap_consumed ||
        !binding->original_consumer_trace_consumed ||
        !binding->same_original_capture_as_gap ||
        binding->fail_closed_until_consumer_proven ||
        binding->variant != gap->variant ||
        strcmp(binding->track02_md5, gap->track02_md5) != 0 ||
        binding->record == 0u ||
        binding->consumer_trace_checksum == 0u ||
        binding->payload_checksum == 0u ||
        binding->level_envelope_checksum == 0u ||
        binding->post_envelope_checksum == 0u ||
        binding->loader_record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        binding->loader_destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        binding->loader_payload_bytes !=
            THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        binding->dungeon_record_consumer_pc == 0u ||
        binding->object_table_consumer_pc == 0u ||
        binding->dungeon_record_payload_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        binding->dungeon_record_byte_count !=
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        binding->dungeon_record_window_checksum !=
            binding->level_envelope_checksum ||
        binding->object_table_payload_offset !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        binding->object_table_byte_count !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        binding->object_table_window_checksum !=
            binding->post_envelope_checksum ||
        binding->nonstartup_level_raw_offset !=
            gap->first_nonstartup_raw_offset ||
        binding->nonstartup_level_user_data_offset !=
            gap->first_nonstartup_user_data_offset ||
        binding->nonstartup_level_raw_hash !=
            gap->first_nonstartup_raw_hash ||
        binding->object_table_raw_offset != gap->first_container_raw_offset ||
        binding->object_table_user_data_offset !=
            gap->first_container_user_data_offset ||
        binding->object_table_user_data_hash !=
            gap->first_container_user_data_hash ||
        binding->level_route_hash != gap->level_route_hash ||
        binding->object_table_route_hash != gap->object_table_route_hash ||
        !binding->nonstartup_level_consumer_bound ||
        !binding->object_table_consumer_bound ||
        !binding->runtime_consumer_binding_ready ||
        binding->render_asset_admission_allowed ||
        binding->fallback_visuals_allowed ||
        !theron_v1_runtime_raw_user_data_coordinate(
            gap->first_nonstartup_raw_offset,
            gap->first_nonstartup_user_data_offset,
            gap->first_nonstartup_byte_count,
            &level_raw_sector,
            &level_raw_sector_user_data_offset) ||
        !theron_v1_runtime_raw_user_data_coordinate(
            gap->first_container_raw_offset,
            gap->first_container_user_data_offset,
            gap->first_container_user_data_byte_count,
            &object_raw_sector,
            &object_raw_sector_user_data_offset)) {
        return 0;
    }

    receipt.valid = 1;
    receipt.original_data_gap_consumed = 1;
    receipt.original_consumer_binding_consumed = 1;
    receipt.same_original_capture_as_gap = 1;
    receipt.variant = gap->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             gap->track02_md5);
    receipt.record = binding->record;
    receipt.consumer_trace_checksum = binding->consumer_trace_checksum;
    receipt.payload_checksum = binding->payload_checksum;
    receipt.level_envelope_checksum = binding->level_envelope_checksum;
    receipt.post_envelope_checksum = binding->post_envelope_checksum;
    receipt.loader_record_user_data_offset =
        binding->loader_record_user_data_offset;
    receipt.loader_destination = binding->loader_destination;
    receipt.loader_payload_bytes = binding->loader_payload_bytes;
    receipt.dungeon_record_consumer_pc = binding->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = binding->object_table_consumer_pc;
    receipt.dungeon_record_payload_offset =
        binding->dungeon_record_payload_offset;
    receipt.dungeon_record_byte_count = binding->dungeon_record_byte_count;
    receipt.dungeon_record_window_checksum =
        binding->dungeon_record_window_checksum;
    receipt.object_table_payload_offset = binding->object_table_payload_offset;
    receipt.object_table_byte_count = binding->object_table_byte_count;
    receipt.object_table_window_checksum =
        binding->object_table_window_checksum;
    receipt.nonstartup_level_raw_offset = gap->first_nonstartup_raw_offset;
    receipt.nonstartup_level_raw_sector = level_raw_sector;
    receipt.nonstartup_level_raw_sector_user_data_offset =
        level_raw_sector_user_data_offset;
    receipt.nonstartup_level_user_data_offset =
        gap->first_nonstartup_user_data_offset;
    receipt.nonstartup_level_byte_count = gap->first_nonstartup_byte_count;
    receipt.nonstartup_level_raw_hash = gap->first_nonstartup_raw_hash;
    receipt.object_table_raw_offset = gap->first_container_raw_offset;
    receipt.object_table_raw_sector = object_raw_sector;
    receipt.object_table_raw_sector_user_data_offset =
        object_raw_sector_user_data_offset;
    receipt.object_table_user_data_offset =
        gap->first_container_user_data_offset;
    receipt.object_table_user_data_byte_count =
        gap->first_container_user_data_byte_count;
    receipt.object_table_user_data_hash = gap->first_container_user_data_hash;
    receipt.level_route_hash = gap->level_route_hash;
    receipt.object_table_route_hash = gap->object_table_route_hash;
    receipt.raw_sector_user_data_bound = 1;
    receipt.nonstartup_dungeon_path_ready = 1;
    receipt.exact_level_fields_blocked = 1;
    receipt.exact_object_fields_blocked = 1;
    receipt.bitmap_route_bound = 0;
    receipt.palette_binding_verified = 0;
    receipt.rgba_output_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_object_level_admission(
    const Theron_V1RuntimeTrack02RawNonstartupDungeonHandoffReceipt *handoff,
    const Theron_V1Track02ObjectDungeonConsumerGrammarReceipt *grammar,
    Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt *out) {
    Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!handoff || !grammar || !out ||
        !handoff->valid ||
        !handoff->original_data_gap_consumed ||
        !handoff->original_consumer_binding_consumed ||
        !handoff->same_original_capture_as_gap ||
        handoff->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(handoff->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        handoff->record == 0u ||
        handoff->consumer_trace_checksum == 0u ||
        handoff->payload_checksum == 0u ||
        handoff->level_envelope_checksum == 0u ||
        handoff->post_envelope_checksum == 0u ||
        handoff->loader_record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        handoff->loader_destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        handoff->loader_payload_bytes !=
            THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        handoff->dungeon_record_consumer_pc == 0u ||
        handoff->object_table_consumer_pc == 0u ||
        handoff->dungeon_record_payload_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        handoff->dungeon_record_byte_count !=
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        handoff->dungeon_record_window_checksum !=
            handoff->level_envelope_checksum ||
        handoff->object_table_payload_offset !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        handoff->object_table_byte_count !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        handoff->object_table_window_checksum !=
            handoff->post_envelope_checksum ||
        handoff->nonstartup_level_raw_offset == 0u ||
        handoff->nonstartup_level_raw_hash == 0u ||
        handoff->nonstartup_level_byte_count == 0u ||
        handoff->object_table_raw_offset == 0u ||
        handoff->object_table_user_data_hash == 0u ||
        handoff->object_table_user_data_byte_count == 0u ||
        handoff->level_route_hash == 0u ||
        handoff->object_table_route_hash == 0u ||
        !handoff->raw_sector_user_data_bound ||
        !handoff->nonstartup_dungeon_path_ready ||
        !handoff->exact_level_fields_blocked ||
        !handoff->exact_object_fields_blocked ||
        handoff->bitmap_route_bound ||
        handoff->palette_binding_verified ||
        handoff->rgba_output_allowed ||
        handoff->dungeon_draw_allowed ||
        handoff->fallback_visuals_allowed ||
        !grammar->valid ||
        !grammar->no_fallback ||
        !grammar->original_consumer_trace_bound ||
        !grammar->same_capture_as_loader_payload ||
        grammar->track02_variant != handoff->variant ||
        grammar->record != handoff->record ||
        grammar->payload_checksum != handoff->payload_checksum ||
        grammar->level_envelope_checksum !=
            handoff->level_envelope_checksum ||
        grammar->post_envelope_checksum != handoff->post_envelope_checksum ||
        grammar->consumer_trace_checksum != handoff->consumer_trace_checksum ||
        grammar->loader_record_user_data_offset !=
            handoff->loader_record_user_data_offset ||
        grammar->loader_destination != handoff->loader_destination ||
        grammar->loader_payload_bytes != handoff->loader_payload_bytes ||
        grammar->dungeon_record_consumer_pc !=
            handoff->dungeon_record_consumer_pc ||
        grammar->object_table_consumer_pc !=
            handoff->object_table_consumer_pc ||
        grammar->dungeon_record_payload_offset !=
            handoff->dungeon_record_payload_offset ||
        grammar->dungeon_record_byte_count !=
            handoff->dungeon_record_byte_count ||
        grammar->dungeon_record_window_checksum !=
            handoff->dungeon_record_window_checksum ||
        grammar->object_table_payload_offset !=
            handoff->object_table_payload_offset ||
        grammar->object_table_byte_count != handoff->object_table_byte_count ||
        grammar->object_table_window_checksum !=
            handoff->object_table_window_checksum ||
        grammar->loader_record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        grammar->loader_destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        grammar->loader_payload_bytes !=
            THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        grammar->dungeon_record_consumer_pc == 0u ||
        grammar->object_table_consumer_pc == 0u ||
        grammar->dungeon_record_payload_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        grammar->dungeon_record_byte_count !=
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        grammar->dungeon_record_window_checksum !=
            handoff->level_envelope_checksum ||
        grammar->object_table_payload_offset !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        grammar->object_table_byte_count !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        grammar->object_table_window_checksum !=
            handoff->post_envelope_checksum ||
        !grammar->dungeon_record_grammar_proven ||
        !grammar->object_table_grammar_proven ||
        !grammar->dungeon_record_fields_blocked ||
        !grammar->object_table_fields_blocked ||
        grammar->bitmap_route_bound ||
        grammar->palette_binding_verified ||
        grammar->rgba_output_allowed ||
        grammar->runtime_handoff_allowed ||
        grammar->fallback_visuals_allowed) {
        return 0;
    }

    receipt.valid = 1;
    receipt.raw_nonstartup_dungeon_handoff_consumed = 1;
    receipt.object_dungeon_grammar_consumed = 1;
    receipt.variant = handoff->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             handoff->track02_md5);
    receipt.record = handoff->record;
    receipt.consumer_trace_checksum = handoff->consumer_trace_checksum;
    receipt.payload_checksum = handoff->payload_checksum;
    receipt.level_envelope_checksum = handoff->level_envelope_checksum;
    receipt.post_envelope_checksum = handoff->post_envelope_checksum;
    receipt.loader_record_user_data_offset =
        handoff->loader_record_user_data_offset;
    receipt.loader_destination = handoff->loader_destination;
    receipt.loader_payload_bytes = handoff->loader_payload_bytes;
    receipt.dungeon_record_payload_offset =
        handoff->dungeon_record_payload_offset;
    receipt.dungeon_record_byte_count = handoff->dungeon_record_byte_count;
    receipt.dungeon_record_window_checksum =
        handoff->dungeon_record_window_checksum;
    receipt.object_table_payload_offset = handoff->object_table_payload_offset;
    receipt.object_table_byte_count = handoff->object_table_byte_count;
    receipt.object_table_window_checksum =
        handoff->object_table_window_checksum;
    receipt.nonstartup_level_raw_offset =
        handoff->nonstartup_level_raw_offset;
    receipt.nonstartup_level_raw_sector =
        handoff->nonstartup_level_raw_sector;
    receipt.nonstartup_level_raw_sector_user_data_offset =
        handoff->nonstartup_level_raw_sector_user_data_offset;
    receipt.nonstartup_level_user_data_offset =
        handoff->nonstartup_level_user_data_offset;
    receipt.nonstartup_level_byte_count =
        handoff->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash = handoff->nonstartup_level_raw_hash;
    receipt.object_table_raw_offset = handoff->object_table_raw_offset;
    receipt.object_table_raw_sector = handoff->object_table_raw_sector;
    receipt.object_table_raw_sector_user_data_offset =
        handoff->object_table_raw_sector_user_data_offset;
    receipt.object_table_user_data_offset =
        handoff->object_table_user_data_offset;
    receipt.object_table_user_data_byte_count =
        handoff->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash =
        handoff->object_table_user_data_hash;
    receipt.level_route_hash = handoff->level_route_hash;
    receipt.object_table_route_hash = handoff->object_table_route_hash;
    receipt.dungeon_record_consumer_pc = handoff->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = handoff->object_table_consumer_pc;
    receipt.raw_sector_user_data_bound = 1;
    receipt.dungeon_record_grammar_proven = 1;
    receipt.object_table_grammar_proven = 1;
    receipt.nonstartup_level_admission_allowed = 1;
    receipt.object_table_admission_allowed = 1;
    receipt.exact_level_fields_blocked = 1;
    receipt.exact_object_fields_blocked = 1;
    receipt.bitmap_route_bound = 0;
    receipt.palette_binding_verified = 0;
    receipt.rgba_output_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_nonstartup_level_record_evidence(
    const Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt *admission,
    const char *capture_trace,
    Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt *out) {
    Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!admission || !capture_trace || !out ||
        !admission->valid ||
        !admission->raw_nonstartup_dungeon_handoff_consumed ||
        !admission->object_dungeon_grammar_consumed ||
        admission->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(admission->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        admission->record == 0u ||
        admission->consumer_trace_checksum == 0u ||
        admission->level_route_hash == 0u ||
        admission->nonstartup_level_raw_offset == 0u ||
        admission->nonstartup_level_user_data_offset == 0u ||
        admission->nonstartup_level_byte_count == 0u ||
        admission->nonstartup_level_raw_hash == 0u ||
        admission->dungeon_record_consumer_pc == 0u ||
        admission->dungeon_record_payload_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        admission->dungeon_record_byte_count !=
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        admission->dungeon_record_window_checksum !=
            admission->level_envelope_checksum ||
        !admission->raw_sector_user_data_bound ||
        !admission->dungeon_record_grammar_proven ||
        !admission->nonstartup_level_admission_allowed ||
        !admission->exact_level_fields_blocked ||
        admission->bitmap_route_bound ||
        admission->palette_binding_verified ||
        admission->rgba_output_allowed ||
        admission->dungeon_draw_allowed ||
        admission->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_nonstartup_level_record_trace") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_object_level_admission=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", admission->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            admission->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", admission->level_route_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_raw_offset",
            admission->nonstartup_level_raw_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_user_data_offset",
            admission->nonstartup_level_user_data_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_byte_count",
            admission->nonstartup_level_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "nonstartup_level_raw_hash",
            admission->nonstartup_level_raw_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "dungeon_record_consumer_pc",
            admission->dungeon_record_consumer_pc) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "dungeon_record_payload_offset",
            admission->dungeon_record_payload_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "dungeon_record_byte_count",
            admission->dungeon_record_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "dungeon_record_window_checksum",
            admission->dungeon_record_window_checksum) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "source_nonstartup_level_bytes_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "nonstartup_level_record_route_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "exact_level_fields_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_layout_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "bitmap_route_bound=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "palette_binding_verified=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "rgba_output_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.object_level_admission_consumed = 1;
    receipt.same_capture_as_object_level_admission = 1;
    receipt.variant = admission->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             admission->track02_md5);
    receipt.record = admission->record;
    receipt.consumer_trace_checksum = admission->consumer_trace_checksum;
    receipt.level_route_hash = admission->level_route_hash;
    receipt.nonstartup_level_raw_offset =
        admission->nonstartup_level_raw_offset;
    receipt.nonstartup_level_raw_sector =
        admission->nonstartup_level_raw_sector;
    receipt.nonstartup_level_raw_sector_user_data_offset =
        admission->nonstartup_level_raw_sector_user_data_offset;
    receipt.nonstartup_level_user_data_offset =
        admission->nonstartup_level_user_data_offset;
    receipt.nonstartup_level_byte_count =
        admission->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash =
        admission->nonstartup_level_raw_hash;
    receipt.dungeon_record_consumer_pc =
        admission->dungeon_record_consumer_pc;
    receipt.dungeon_record_payload_offset =
        admission->dungeon_record_payload_offset;
    receipt.dungeon_record_byte_count =
        admission->dungeon_record_byte_count;
    receipt.dungeon_record_window_checksum =
        admission->dungeon_record_window_checksum;
    receipt.source_nonstartup_level_bytes_bound = 1;
    receipt.nonstartup_level_record_route_observed = 1;
    receipt.exact_level_fields_blocked = 1;
    receipt.object_table_layout_blocked = 1;
    receipt.bitmap_route_bound = 0;
    receipt.palette_binding_verified = 0;
    receipt.rgba_output_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_object_table_route_evidence(
    const Theron_V1RuntimeTrack02ObjectLevelAdmissionReceipt *admission,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt *out) {
    Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!admission || !capture_trace || !out ||
        !admission->valid ||
        !admission->raw_nonstartup_dungeon_handoff_consumed ||
        !admission->object_dungeon_grammar_consumed ||
        admission->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(admission->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        admission->record == 0u ||
        admission->consumer_trace_checksum == 0u ||
        admission->object_table_route_hash == 0u ||
        admission->object_table_raw_offset == 0u ||
        admission->object_table_user_data_offset == 0u ||
        admission->object_table_user_data_byte_count == 0u ||
        admission->object_table_user_data_hash == 0u ||
        admission->object_table_consumer_pc == 0u ||
        admission->object_table_payload_offset !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        admission->object_table_byte_count !=
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        admission->object_table_window_checksum !=
            admission->post_envelope_checksum ||
        !admission->raw_sector_user_data_bound ||
        !admission->object_table_grammar_proven ||
        !admission->object_table_admission_allowed ||
        !admission->exact_object_fields_blocked ||
        admission->bitmap_route_bound ||
        admission->palette_binding_verified ||
        admission->rgba_output_allowed ||
        admission->dungeon_draw_allowed ||
        admission->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_object_table_route_trace") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_object_level_admission=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", admission->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            admission->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            admission->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_raw_offset",
            admission->object_table_raw_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_user_data_offset",
            admission->object_table_user_data_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_user_data_byte_count",
            admission->object_table_user_data_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_user_data_hash",
            admission->object_table_user_data_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_consumer_pc",
            admission->object_table_consumer_pc) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_payload_offset",
            admission->object_table_payload_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_byte_count",
            admission->object_table_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_window_checksum",
            admission->object_table_window_checksum) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "source_object_table_bytes_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_route_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_layout_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "exact_object_fields_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "bitmap_route_bound=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "palette_binding_verified=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "rgba_output_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.object_level_admission_consumed = 1;
    receipt.same_capture_as_object_level_admission = 1;
    receipt.variant = admission->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             admission->track02_md5);
    receipt.record = admission->record;
    receipt.consumer_trace_checksum = admission->consumer_trace_checksum;
    receipt.object_table_route_hash = admission->object_table_route_hash;
    receipt.object_table_raw_offset = admission->object_table_raw_offset;
    receipt.object_table_raw_sector = admission->object_table_raw_sector;
    receipt.object_table_raw_sector_user_data_offset =
        admission->object_table_raw_sector_user_data_offset;
    receipt.object_table_user_data_offset =
        admission->object_table_user_data_offset;
    receipt.object_table_user_data_byte_count =
        admission->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash =
        admission->object_table_user_data_hash;
    receipt.object_table_consumer_pc = admission->object_table_consumer_pc;
    receipt.object_table_payload_offset =
        admission->object_table_payload_offset;
    receipt.object_table_byte_count = admission->object_table_byte_count;
    receipt.object_table_window_checksum =
        admission->object_table_window_checksum;
    receipt.source_object_table_bytes_bound = 1;
    receipt.object_table_route_observed = 1;
    receipt.object_table_layout_blocked = 1;
    receipt.exact_object_fields_blocked = 1;
    receipt.bitmap_route_bound = 0;
    receipt.palette_binding_verified = 0;
    receipt.rgba_output_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_level_object_handoff_evidence(
    const Theron_V1RuntimeTrack02NonstartupLevelRecordEvidenceReceipt *level,
    const Theron_V1RuntimeTrack02ObjectTableRouteEvidenceReceipt *object,
    Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt *out) {
    Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!level || !object || !out ||
        !level->valid ||
        !level->object_level_admission_consumed ||
        !level->same_capture_as_object_level_admission ||
        level->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(level->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        level->record == 0u ||
        level->consumer_trace_checksum == 0u ||
        level->level_route_hash == 0u ||
        level->nonstartup_level_raw_offset == 0u ||
        level->nonstartup_level_user_data_offset == 0u ||
        level->nonstartup_level_byte_count == 0u ||
        level->nonstartup_level_raw_hash == 0u ||
        level->dungeon_record_consumer_pc == 0u ||
        level->dungeon_record_window_checksum == 0u ||
        !level->source_nonstartup_level_bytes_bound ||
        !level->nonstartup_level_record_route_observed ||
        !level->exact_level_fields_blocked ||
        !level->object_table_layout_blocked ||
        level->bitmap_route_bound ||
        level->palette_binding_verified ||
        level->rgba_output_allowed ||
        level->dungeon_draw_allowed ||
        level->fallback_visuals_allowed ||
        !object->valid ||
        !object->object_level_admission_consumed ||
        !object->same_capture_as_object_level_admission ||
        object->variant != level->variant ||
        strcmp(object->track02_md5, level->track02_md5) != 0 ||
        object->record != level->record ||
        object->consumer_trace_checksum != level->consumer_trace_checksum ||
        object->object_table_route_hash == 0u ||
        object->object_table_raw_offset == 0u ||
        object->object_table_user_data_offset == 0u ||
        object->object_table_user_data_byte_count == 0u ||
        object->object_table_user_data_hash == 0u ||
        object->object_table_consumer_pc == 0u ||
        object->object_table_window_checksum == 0u ||
        !object->source_object_table_bytes_bound ||
        !object->object_table_route_observed ||
        !object->object_table_layout_blocked ||
        !object->exact_object_fields_blocked ||
        object->bitmap_route_bound ||
        object->palette_binding_verified ||
        object->rgba_output_allowed ||
        object->dungeon_draw_allowed ||
        object->fallback_visuals_allowed) {
        return 0;
    }

    receipt.valid = 1;
    receipt.nonstartup_level_record_evidence_consumed = 1;
    receipt.object_table_route_evidence_consumed = 1;
    receipt.same_capture_as_object_level_admission = 1;
    receipt.variant = level->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             level->track02_md5);
    receipt.record = level->record;
    receipt.consumer_trace_checksum = level->consumer_trace_checksum;
    receipt.level_route_hash = level->level_route_hash;
    receipt.object_table_route_hash = object->object_table_route_hash;
    receipt.nonstartup_level_raw_offset = level->nonstartup_level_raw_offset;
    receipt.nonstartup_level_user_data_offset =
        level->nonstartup_level_user_data_offset;
    receipt.nonstartup_level_byte_count = level->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash = level->nonstartup_level_raw_hash;
    receipt.object_table_raw_offset = object->object_table_raw_offset;
    receipt.object_table_user_data_offset =
        object->object_table_user_data_offset;
    receipt.object_table_user_data_byte_count =
        object->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash = object->object_table_user_data_hash;
    receipt.dungeon_record_consumer_pc = level->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = object->object_table_consumer_pc;
    receipt.dungeon_record_window_checksum =
        level->dungeon_record_window_checksum;
    receipt.object_table_window_checksum =
        object->object_table_window_checksum;
    receipt.source_nonstartup_level_bytes_bound = 1;
    receipt.source_object_table_bytes_bound = 1;
    receipt.level_object_pair_route_observed = 1;
    receipt.exact_level_fields_blocked = 1;
    receipt.exact_object_fields_blocked = 1;
    receipt.object_table_layout_blocked = 1;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_level_object_field_boundary(
    const Theron_V1RuntimeTrack02LevelObjectHandoffEvidenceReceipt *handoff,
    const char *capture_trace,
    Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt *out) {
    Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!handoff || !capture_trace || !out ||
        !handoff->valid ||
        !handoff->nonstartup_level_record_evidence_consumed ||
        !handoff->object_table_route_evidence_consumed ||
        !handoff->same_capture_as_object_level_admission ||
        handoff->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(handoff->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        handoff->record == 0u ||
        handoff->consumer_trace_checksum == 0u ||
        handoff->level_route_hash == 0u ||
        handoff->object_table_route_hash == 0u ||
        handoff->nonstartup_level_byte_count == 0u ||
        handoff->nonstartup_level_raw_hash == 0u ||
        handoff->object_table_user_data_byte_count == 0u ||
        handoff->object_table_user_data_hash == 0u ||
        handoff->dungeon_record_consumer_pc == 0u ||
        handoff->object_table_consumer_pc == 0u ||
        handoff->dungeon_record_window_checksum == 0u ||
        handoff->object_table_window_checksum == 0u ||
        !handoff->source_nonstartup_level_bytes_bound ||
        !handoff->source_object_table_bytes_bound ||
        !handoff->level_object_pair_route_observed ||
        !handoff->exact_level_fields_blocked ||
        !handoff->exact_object_fields_blocked ||
        !handoff->object_table_layout_blocked ||
        handoff->dungeon_draw_allowed ||
        handoff->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_level_object_field_boundary") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_level_object_handoff=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", handoff->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            handoff->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", handoff->level_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            handoff->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_byte_count",
            handoff->nonstartup_level_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "nonstartup_level_raw_hash",
            handoff->nonstartup_level_raw_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_user_data_byte_count",
            handoff->object_table_user_data_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_user_data_hash",
            handoff->object_table_user_data_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "dungeon_record_consumer_pc",
            handoff->dungeon_record_consumer_pc) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_consumer_pc",
            handoff->object_table_consumer_pc) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "dungeon_record_window_checksum",
            handoff->dungeon_record_window_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_window_checksum",
            handoff->object_table_window_checksum) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "source_nonstartup_level_bytes_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "source_object_table_bytes_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "exact_level_fields_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "exact_object_fields_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_layout_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_route_handoff_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.level_object_handoff_evidence_consumed = 1;
    receipt.same_capture_as_level_object_handoff = 1;
    receipt.variant = handoff->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             handoff->track02_md5);
    receipt.record = handoff->record;
    receipt.consumer_trace_checksum = handoff->consumer_trace_checksum;
    receipt.level_route_hash = handoff->level_route_hash;
    receipt.object_table_route_hash = handoff->object_table_route_hash;
    receipt.nonstartup_level_byte_count =
        handoff->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash = handoff->nonstartup_level_raw_hash;
    receipt.object_table_user_data_byte_count =
        handoff->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash =
        handoff->object_table_user_data_hash;
    receipt.dungeon_record_consumer_pc = handoff->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = handoff->object_table_consumer_pc;
    receipt.dungeon_record_window_checksum =
        handoff->dungeon_record_window_checksum;
    receipt.object_table_window_checksum =
        handoff->object_table_window_checksum;
    receipt.source_nonstartup_level_bytes_bound = 1;
    receipt.source_object_table_bytes_bound = 1;
    receipt.field_decoder_required = 1;
    receipt.exact_level_fields_blocked = 1;
    receipt.exact_object_fields_blocked = 1;
    receipt.object_table_layout_blocked = 1;
    receipt.dungeon_route_handoff_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_reviewed_field_decoder_boundary(
    const Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt *boundary,
    const char *reviewed_decoder_identity,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt *out) {
    Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!boundary || !reviewed_decoder_identity || !capture_trace || !out ||
        reviewed_decoder_identity[0] == '\0' ||
        strstr(reviewed_decoder_identity, "placeholder") ||
        strstr(reviewed_decoder_identity, "synthetic") ||
        strstr(reviewed_decoder_identity, "fallback") ||
        !boundary->valid ||
        !boundary->level_object_handoff_evidence_consumed ||
        !boundary->same_capture_as_level_object_handoff ||
        boundary->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(boundary->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        boundary->record == 0u ||
        boundary->consumer_trace_checksum == 0u ||
        boundary->level_route_hash == 0u ||
        boundary->object_table_route_hash == 0u ||
        !boundary->source_nonstartup_level_bytes_bound ||
        !boundary->source_object_table_bytes_bound ||
        !boundary->field_decoder_required ||
        !boundary->exact_level_fields_blocked ||
        !boundary->exact_object_fields_blocked ||
        !boundary->object_table_layout_blocked ||
        boundary->dungeon_route_handoff_allowed ||
        boundary->dungeon_draw_allowed ||
        boundary->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_reviewed_field_decoder_boundary") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_field_boundary=1") ||
        !theron_v1_runtime_trace_has(capture_trace, reviewed_decoder_identity) ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", boundary->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            boundary->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", boundary->level_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            boundary->object_table_route_hash) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "reviewed_decoder_source_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_execution_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "exact_level_fields_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "exact_object_fields_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_layout_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_route_handoff_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.level_object_field_boundary_consumed = 1;
    receipt.same_capture_as_field_boundary = 1;
    receipt.variant = boundary->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             boundary->track02_md5);
    receipt.record = boundary->record;
    receipt.consumer_trace_checksum = boundary->consumer_trace_checksum;
    receipt.level_route_hash = boundary->level_route_hash;
    receipt.object_table_route_hash = boundary->object_table_route_hash;
    snprintf(receipt.reviewed_decoder_identity,
             sizeof(receipt.reviewed_decoder_identity), "%s",
             reviewed_decoder_identity);
    receipt.reviewed_decoder_source_bound = 1;
    receipt.field_decoder_required = 1;
    receipt.field_decoder_execution_allowed = 0;
    receipt.exact_level_fields_blocked = 1;
    receipt.exact_object_fields_blocked = 1;
    receipt.object_table_layout_blocked = 1;
    receipt.dungeon_route_handoff_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_dungeon_route_admission_boundary(
    const Theron_V1RuntimeTrack02ReviewedFieldDecoderBoundaryReceipt *boundary,
    const char *capture_trace,
    Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt *out) {
    Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!boundary || !capture_trace || !out ||
        !boundary->valid ||
        !boundary->level_object_field_boundary_consumed ||
        !boundary->same_capture_as_field_boundary ||
        boundary->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(boundary->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        boundary->record == 0u ||
        boundary->consumer_trace_checksum == 0u ||
        boundary->level_route_hash == 0u ||
        boundary->object_table_route_hash == 0u ||
        boundary->reviewed_decoder_identity[0] == '\0' ||
        !boundary->reviewed_decoder_source_bound ||
        !boundary->field_decoder_required ||
        boundary->field_decoder_execution_allowed ||
        !boundary->exact_level_fields_blocked ||
        !boundary->exact_object_fields_blocked ||
        !boundary->object_table_layout_blocked ||
        boundary->dungeon_route_handoff_allowed ||
        boundary->dungeon_draw_allowed ||
        boundary->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_dungeon_route_admission_boundary") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_reviewed_decoder_boundary=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has(
            capture_trace, boundary->reviewed_decoder_identity) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", boundary->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            boundary->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", boundary->level_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            boundary->object_table_route_hash) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "reviewed_decoder_source_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_execution_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "real_track02_level_object_boundary_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_route_review_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_route_handoff_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_runtime_admission_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.reviewed_field_decoder_boundary_consumed = 1;
    receipt.same_capture_as_reviewed_decoder_boundary = 1;
    receipt.variant = boundary->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             boundary->track02_md5);
    receipt.record = boundary->record;
    receipt.consumer_trace_checksum = boundary->consumer_trace_checksum;
    receipt.level_route_hash = boundary->level_route_hash;
    receipt.object_table_route_hash = boundary->object_table_route_hash;
    snprintf(receipt.reviewed_decoder_identity,
             sizeof(receipt.reviewed_decoder_identity), "%s",
             boundary->reviewed_decoder_identity);
    receipt.reviewed_decoder_source_bound = 1;
    receipt.field_decoder_required = 1;
    receipt.field_decoder_execution_allowed = 0;
    receipt.real_track02_level_object_boundary_bound = 1;
    receipt.dungeon_route_review_required = 1;
    receipt.dungeon_route_handoff_allowed = 0;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_level_object_facts_handoff(
    const Theron_V1RuntimeTrack02DungeonRouteAdmissionBoundaryReceipt *route,
    const Theron_V1RuntimeTrack02LevelObjectFieldBoundaryReceipt *boundary,
    Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt *out) {
    Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!route || !boundary || !out ||
        !route->valid ||
        !route->reviewed_field_decoder_boundary_consumed ||
        !route->same_capture_as_reviewed_decoder_boundary ||
        route->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(route->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        route->record == 0u ||
        route->consumer_trace_checksum == 0u ||
        route->level_route_hash == 0u ||
        route->object_table_route_hash == 0u ||
        route->reviewed_decoder_identity[0] == '\0' ||
        !route->reviewed_decoder_source_bound ||
        !route->field_decoder_required ||
        route->field_decoder_execution_allowed ||
        !route->real_track02_level_object_boundary_bound ||
        !route->dungeon_route_review_required ||
        route->dungeon_route_handoff_allowed ||
        route->dungeon_runtime_admission_allowed ||
        route->dungeon_draw_allowed ||
        route->fallback_visuals_allowed ||
        !boundary->valid ||
        !boundary->level_object_handoff_evidence_consumed ||
        !boundary->same_capture_as_level_object_handoff ||
        boundary->variant != route->variant ||
        strcmp(boundary->track02_md5, route->track02_md5) != 0 ||
        boundary->record != route->record ||
        boundary->consumer_trace_checksum != route->consumer_trace_checksum ||
        boundary->level_route_hash != route->level_route_hash ||
        boundary->object_table_route_hash != route->object_table_route_hash ||
        boundary->nonstartup_level_byte_count == 0u ||
        boundary->nonstartup_level_raw_hash == 0u ||
        boundary->object_table_user_data_byte_count == 0u ||
        boundary->object_table_user_data_hash == 0u ||
        boundary->dungeon_record_consumer_pc == 0u ||
        boundary->object_table_consumer_pc == 0u ||
        !boundary->source_nonstartup_level_bytes_bound ||
        !boundary->source_object_table_bytes_bound ||
        !boundary->field_decoder_required ||
        !boundary->exact_level_fields_blocked ||
        !boundary->exact_object_fields_blocked ||
        !boundary->object_table_layout_blocked ||
        boundary->dungeon_route_handoff_allowed ||
        boundary->dungeon_draw_allowed ||
        boundary->fallback_visuals_allowed) {
        return 0;
    }

    receipt.valid = 1;
    receipt.dungeon_route_boundary_consumed = 1;
    receipt.level_object_field_boundary_consumed = 1;
    receipt.same_capture_as_dungeon_route_boundary = 1;
    receipt.variant = route->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             route->track02_md5);
    receipt.record = route->record;
    receipt.consumer_trace_checksum = route->consumer_trace_checksum;
    receipt.level_route_hash = route->level_route_hash;
    receipt.object_table_route_hash = route->object_table_route_hash;
    receipt.nonstartup_level_byte_count =
        boundary->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash = boundary->nonstartup_level_raw_hash;
    receipt.object_table_user_data_byte_count =
        boundary->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash =
        boundary->object_table_user_data_hash;
    receipt.dungeon_record_consumer_pc = boundary->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = boundary->object_table_consumer_pc;
    snprintf(receipt.reviewed_decoder_identity,
             sizeof(receipt.reviewed_decoder_identity), "%s",
             route->reviewed_decoder_identity);
    receipt.real_track02_level_object_boundary_bound = 1;
    receipt.field_decoder_required = 1;
    receipt.field_decoder_execution_allowed = 0;
    receipt.dungeon_route_review_required = 1;
    receipt.dungeon_route_handoff_allowed = 0;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_dungeon_selection_level_record_boundary(
    const Theron_V1RuntimeTrack02LevelObjectFactsHandoffReceipt *handoff,
    const char *capture_trace,
    Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt *out) {
    Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt receipt =
        {0};
    uint32_t selected_dungeon_index;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!handoff || !capture_trace || !out ||
        !handoff->valid ||
        !handoff->dungeon_route_boundary_consumed ||
        !handoff->level_object_field_boundary_consumed ||
        !handoff->same_capture_as_dungeon_route_boundary ||
        handoff->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(handoff->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        handoff->record == 0u ||
        handoff->consumer_trace_checksum == 0u ||
        handoff->level_route_hash == 0u ||
        handoff->object_table_route_hash == 0u ||
        handoff->nonstartup_level_byte_count == 0u ||
        handoff->nonstartup_level_raw_hash == 0u ||
        handoff->object_table_user_data_byte_count == 0u ||
        handoff->object_table_user_data_hash == 0u ||
        handoff->dungeon_record_consumer_pc == 0u ||
        handoff->object_table_consumer_pc == 0u ||
        !handoff->real_track02_level_object_boundary_bound ||
        !handoff->field_decoder_required ||
        handoff->field_decoder_execution_allowed ||
        !handoff->dungeon_route_review_required ||
        handoff->dungeon_route_handoff_allowed ||
        handoff->dungeon_runtime_admission_allowed ||
        handoff->dungeon_draw_allowed ||
        handoff->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace,
            "theron_track02_dungeon_selection_level_record_boundary") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_facts_handoff=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", handoff->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            handoff->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", handoff->level_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            handoff->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_byte_count",
            handoff->nonstartup_level_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "nonstartup_level_raw_hash",
            handoff->nonstartup_level_raw_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_user_data_byte_count",
            handoff->object_table_user_data_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_user_data_hash",
            handoff->object_table_user_data_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "dungeon_record_consumer_pc",
            handoff->dungeon_record_consumer_pc) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_consumer_pc",
            handoff->object_table_consumer_pc) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "selected_dungeon_index",
            &selected_dungeon_index) ||
        selected_dungeon_index == 0u ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_selection_route_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "level_record_route_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "level_record_review_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_layout_blocked=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_execution_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_runtime_admission_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.level_object_facts_handoff_consumed = 1;
    receipt.same_capture_as_facts_handoff = 1;
    receipt.variant = handoff->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             handoff->track02_md5);
    receipt.record = handoff->record;
    receipt.consumer_trace_checksum = handoff->consumer_trace_checksum;
    receipt.level_route_hash = handoff->level_route_hash;
    receipt.object_table_route_hash = handoff->object_table_route_hash;
    receipt.selected_dungeon_index = selected_dungeon_index;
    receipt.nonstartup_level_byte_count = handoff->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash = handoff->nonstartup_level_raw_hash;
    receipt.object_table_user_data_byte_count =
        handoff->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash =
        handoff->object_table_user_data_hash;
    receipt.dungeon_record_consumer_pc = handoff->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = handoff->object_table_consumer_pc;
    receipt.dungeon_selection_route_observed = 1;
    receipt.level_record_route_bound = 1;
    receipt.level_record_review_required = 1;
    receipt.object_table_layout_blocked = 1;
    receipt.field_decoder_execution_allowed = 0;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_dungeon_object_level_table_binding(
    const Theron_V1RuntimeTrack02DungeonSelectionLevelRecordBoundaryReceipt
        *boundary,
    const char *capture_trace,
    Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt *out) {
    Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt receipt = {0};

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!boundary || !capture_trace || !out ||
        !boundary->valid ||
        !boundary->level_object_facts_handoff_consumed ||
        !boundary->same_capture_as_facts_handoff ||
        boundary->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(boundary->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        boundary->record == 0u ||
        boundary->consumer_trace_checksum == 0u ||
        boundary->level_route_hash == 0u ||
        boundary->object_table_route_hash == 0u ||
        boundary->selected_dungeon_index == 0u ||
        boundary->nonstartup_level_byte_count == 0u ||
        boundary->nonstartup_level_raw_hash == 0u ||
        boundary->object_table_user_data_byte_count == 0u ||
        boundary->object_table_user_data_hash == 0u ||
        boundary->dungeon_record_consumer_pc == 0u ||
        boundary->object_table_consumer_pc == 0u ||
        !boundary->dungeon_selection_route_observed ||
        !boundary->level_record_route_bound ||
        !boundary->level_record_review_required ||
        !boundary->object_table_layout_blocked ||
        boundary->field_decoder_execution_allowed ||
        boundary->dungeon_runtime_admission_allowed ||
        boundary->dungeon_draw_allowed ||
        boundary->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace,
            "theron_track02_dungeon_object_level_table_binding") ||
        !theron_v1_runtime_trace_has(
            capture_trace,
            "same_capture_as_dungeon_selection_boundary=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", boundary->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            boundary->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", boundary->level_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            boundary->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_dungeon_index",
            boundary->selected_dungeon_index) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_byte_count",
            boundary->nonstartup_level_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "nonstartup_level_raw_hash",
            boundary->nonstartup_level_raw_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_user_data_byte_count",
            boundary->object_table_user_data_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_user_data_hash",
            boundary->object_table_user_data_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "dungeon_record_consumer_pc",
            boundary->dungeon_record_consumer_pc) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_consumer_pc",
            boundary->object_table_consumer_pc) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "level_record_table_route_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_route_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "level_object_table_pair_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "level_record_review_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_layout_review_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_execution_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_route_handoff_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_runtime_admission_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.dungeon_selection_level_record_boundary_consumed = 1;
    receipt.same_capture_as_dungeon_selection_boundary = 1;
    receipt.variant = boundary->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             boundary->track02_md5);
    receipt.record = boundary->record;
    receipt.consumer_trace_checksum = boundary->consumer_trace_checksum;
    receipt.level_route_hash = boundary->level_route_hash;
    receipt.object_table_route_hash = boundary->object_table_route_hash;
    receipt.selected_dungeon_index = boundary->selected_dungeon_index;
    receipt.nonstartup_level_byte_count =
        boundary->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash = boundary->nonstartup_level_raw_hash;
    receipt.object_table_user_data_byte_count =
        boundary->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash =
        boundary->object_table_user_data_hash;
    receipt.dungeon_record_consumer_pc = boundary->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = boundary->object_table_consumer_pc;
    receipt.level_record_table_route_bound = 1;
    receipt.object_table_route_bound = 1;
    receipt.level_object_table_pair_bound = 1;
    receipt.level_record_review_required = 1;
    receipt.object_table_layout_review_required = 1;
    receipt.field_decoder_execution_allowed = 0;
    receipt.dungeon_route_handoff_allowed = 0;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_level_object_loader_route(
    const Theron_V1RuntimeTrack02DungeonObjectLevelTableBindingReceipt
        *binding,
    const char *capture_trace,
    Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt *out) {
    Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt receipt = {0};
    uint32_t loader_route_pair_hash;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!binding || !capture_trace || !out ||
        !binding->valid ||
        !binding->dungeon_selection_level_record_boundary_consumed ||
        !binding->same_capture_as_dungeon_selection_boundary ||
        binding->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(binding->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        binding->record == 0u ||
        binding->consumer_trace_checksum == 0u ||
        binding->selected_dungeon_index == 0u ||
        binding->level_route_hash == 0u ||
        binding->object_table_route_hash == 0u ||
        binding->nonstartup_level_byte_count == 0u ||
        binding->nonstartup_level_raw_hash == 0u ||
        binding->object_table_user_data_byte_count == 0u ||
        binding->object_table_user_data_hash == 0u ||
        binding->dungeon_record_consumer_pc == 0u ||
        binding->object_table_consumer_pc == 0u ||
        !binding->level_record_table_route_bound ||
        !binding->object_table_route_bound ||
        !binding->level_object_table_pair_bound ||
        !binding->level_record_review_required ||
        !binding->object_table_layout_review_required ||
        binding->field_decoder_execution_allowed ||
        binding->dungeon_route_handoff_allowed ||
        binding->dungeon_runtime_admission_allowed ||
        binding->dungeon_draw_allowed ||
        binding->fallback_visuals_allowed) {
        return 0;
    }

    loader_route_pair_hash =
        theron_v1_runtime_track02_loader_route_pair_hash(
            binding->record,
            binding->consumer_trace_checksum,
            binding->selected_dungeon_index,
            binding->level_route_hash,
            binding->object_table_route_hash,
            binding->nonstartup_level_byte_count,
            binding->nonstartup_level_raw_hash,
            binding->object_table_user_data_byte_count,
            binding->object_table_user_data_hash,
            binding->dungeon_record_consumer_pc,
            binding->object_table_consumer_pc);

    if (!theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_level_object_loader_route") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_table_binding=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", binding->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            binding->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_dungeon_index",
            binding->selected_dungeon_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", binding->level_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            binding->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_route_pair_hash",
            loader_route_pair_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_byte_count",
            binding->nonstartup_level_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "nonstartup_level_raw_hash",
            binding->nonstartup_level_raw_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_user_data_byte_count",
            binding->object_table_user_data_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_user_data_hash",
            binding->object_table_user_data_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "dungeon_record_consumer_pc",
            binding->dungeon_record_consumer_pc) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_consumer_pc",
            binding->object_table_consumer_pc) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "loader_route_record_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "loader_route_source_windows_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "level_object_table_pair_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "loader_route_review_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "field_decoder_execution_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_route_handoff_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_runtime_admission_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.dungeon_object_level_table_binding_consumed = 1;
    receipt.same_capture_as_table_binding = 1;
    receipt.variant = binding->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             binding->track02_md5);
    receipt.record = binding->record;
    receipt.consumer_trace_checksum = binding->consumer_trace_checksum;
    receipt.selected_dungeon_index = binding->selected_dungeon_index;
    receipt.level_route_hash = binding->level_route_hash;
    receipt.object_table_route_hash = binding->object_table_route_hash;
    receipt.loader_route_pair_hash = loader_route_pair_hash;
    receipt.nonstartup_level_byte_count =
        binding->nonstartup_level_byte_count;
    receipt.nonstartup_level_raw_hash = binding->nonstartup_level_raw_hash;
    receipt.object_table_user_data_byte_count =
        binding->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash =
        binding->object_table_user_data_hash;
    receipt.dungeon_record_consumer_pc = binding->dungeon_record_consumer_pc;
    receipt.object_table_consumer_pc = binding->object_table_consumer_pc;
    receipt.loader_route_record_bound = 1;
    receipt.loader_route_source_windows_bound = 1;
    receipt.level_object_table_pair_bound = 1;
    receipt.loader_route_review_required = 1;
    receipt.field_decoder_execution_allowed = 0;
    receipt.dungeon_route_handoff_allowed = 0;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_object_placement_state(
    const Theron_V1RuntimeTrack02LevelObjectLoaderRouteReceipt *loader,
    const Theron_Track02ObjectTable *objects,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ObjectPlacementStateReceipt *out) {
    Theron_V1RuntimeTrack02ObjectPlacementStateReceipt receipt = {0};
    const Theron_Track02ObjectTableRecord *first = NULL;
    uint32_t selected_level_index = 0u;
    uint32_t placement_hash;
    size_t i;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!loader || !objects || !capture_trace || !out ||
        !loader->valid ||
        !loader->dungeon_object_level_table_binding_consumed ||
        !loader->same_capture_as_table_binding ||
        loader->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(loader->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        loader->record == 0u ||
        loader->consumer_trace_checksum == 0u ||
        loader->selected_dungeon_index == 0u ||
        loader->selected_dungeon_index > THERON_TRACK02_DUNGEON_COUNT ||
        loader->level_route_hash == 0u ||
        loader->object_table_route_hash == 0u ||
        loader->loader_route_pair_hash == 0u ||
        loader->object_table_user_data_byte_count == 0u ||
        loader->object_table_user_data_hash == 0u ||
        !loader->loader_route_record_bound ||
        !loader->loader_route_source_windows_bound ||
        !loader->level_object_table_pair_bound ||
        !loader->loader_route_review_required ||
        loader->field_decoder_execution_allowed ||
        loader->dungeon_route_handoff_allowed ||
        loader->dungeon_runtime_admission_allowed ||
        loader->dungeon_draw_allowed ||
        loader->fallback_visuals_allowed ||
        !objects->shape_ok ||
        objects->record_count == 0u ||
        objects->record_count > THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS ||
        objects->checksum == 0u ||
        objects->byte_count == 0u ||
        objects->byte_count > loader->object_table_user_data_byte_count) {
        return 0;
    }

    if (!theron_v1_runtime_trace_read_u32(
            capture_trace, "selected_level_index", &selected_level_index)) {
        return 0;
    }
    if (selected_level_index >= THERON_TRACK02_DUNGEON_COUNT ||
        objects->level_record_counts[selected_level_index] == 0u ||
        (objects->level_mask & (1u << selected_level_index)) == 0u ||
        objects->level_record_hashes[selected_level_index] == 0u ||
        objects->level_position_hashes[selected_level_index] == 0u) {
        return 0;
    }
    placement_hash =
        theron_v1_runtime_track02_object_placement_state_hash(
            objects, selected_level_index);
    if (placement_hash == 0u) {
        return 0;
    }
    for (i = 0u; i < objects->record_count; ++i) {
        if (objects->records[i].level_index == selected_level_index) {
            first = &objects->records[i];
            break;
        }
    }
    if (!first) {
        return 0;
    }

    if (!theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_object_placement_state") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_loader_route=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", loader->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            loader->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_dungeon_index",
            loader->selected_dungeon_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_level_index", selected_level_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_route_hash", loader->level_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            loader->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_route_pair_hash",
            loader->loader_route_pair_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_user_data_byte_count",
            loader->object_table_user_data_byte_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_user_data_hash",
            loader->object_table_user_data_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_record_count", objects->record_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_checksum", objects->checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_level_mask", objects->level_mask) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "selected_level_record_count",
            objects->level_record_counts[selected_level_index]) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_level_record_hash",
            objects->level_record_hashes[selected_level_index]) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_level_position_hash",
            objects->level_position_hashes[selected_level_index]) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_placement_state_hash", placement_hash) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_placement_bytes_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_state_low_bits_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_kind_semantics_review_required=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "world_object_publish_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_runtime_admission_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.level_object_loader_route_consumed = 1;
    receipt.object_table_shape_consumed = 1;
    receipt.same_capture_as_loader_route = 1;
    receipt.variant = loader->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             loader->track02_md5);
    receipt.record = loader->record;
    receipt.consumer_trace_checksum = loader->consumer_trace_checksum;
    receipt.selected_dungeon_index = loader->selected_dungeon_index;
    receipt.selected_level_index = selected_level_index;
    receipt.level_route_hash = loader->level_route_hash;
    receipt.object_table_route_hash = loader->object_table_route_hash;
    receipt.loader_route_pair_hash = loader->loader_route_pair_hash;
    receipt.object_table_user_data_byte_count =
        loader->object_table_user_data_byte_count;
    receipt.object_table_user_data_hash = loader->object_table_user_data_hash;
    receipt.object_record_count = objects->record_count;
    receipt.object_table_checksum = objects->checksum;
    receipt.object_level_mask = objects->level_mask;
    receipt.selected_level_record_count =
        objects->level_record_counts[selected_level_index];
    receipt.selected_level_record_hash =
        objects->level_record_hashes[selected_level_index];
    receipt.selected_level_position_hash =
        objects->level_position_hashes[selected_level_index];
    receipt.object_placement_state_hash = placement_hash;
    receipt.first_object_id = first->object_id;
    receipt.first_object_kind = first->kind;
    receipt.first_object_x = first->x;
    receipt.first_object_y = first->y;
    receipt.first_object_level_index = first->level_index;
    receipt.first_object_state_low_bits = (uint8_t)(first->flags & 0x03u);
    receipt.first_object_flags = first->flags;
    receipt.first_object_argument = first->argument;
    receipt.object_placement_bytes_bound = 1;
    receipt.object_state_low_bits_bound = 1;
    receipt.object_kind_semantics_review_required = 1;
    receipt.world_object_publish_allowed = 0;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_object_gameplay_semantics(
    const Theron_V1RuntimeTrack02ObjectPlacementStateReceipt *placement,
    const Theron_Track02ObjectTable *objects,
    const char *capture_trace,
    Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt *out) {
    Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt receipt = {0};
    const Theron_Track02ObjectTableRecord *first = NULL;
    uint32_t runtime_state_hash;
    unsigned int low_kind_mask = 0u;
    int quest_item_seen = 0;
    size_t i;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!placement || !objects || !capture_trace || !out ||
        !placement->valid ||
        !placement->level_object_loader_route_consumed ||
        !placement->object_table_shape_consumed ||
        !placement->same_capture_as_loader_route ||
        placement->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(placement->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        placement->record == 0u ||
        placement->consumer_trace_checksum == 0u ||
        placement->selected_dungeon_index == 0u ||
        placement->selected_dungeon_index > THERON_TRACK02_DUNGEON_COUNT ||
        placement->selected_level_index >= THERON_TRACK02_DUNGEON_COUNT ||
        placement->object_table_route_hash == 0u ||
        placement->loader_route_pair_hash == 0u ||
        placement->object_placement_state_hash == 0u ||
        placement->selected_level_record_count == 0u ||
        placement->selected_level_record_hash == 0u ||
        placement->selected_level_position_hash == 0u ||
        !placement->object_placement_bytes_bound ||
        !placement->object_state_low_bits_bound ||
        !placement->object_kind_semantics_review_required ||
        placement->world_object_publish_allowed ||
        placement->dungeon_runtime_admission_allowed ||
        placement->dungeon_draw_allowed ||
        placement->fallback_visuals_allowed ||
        !objects->shape_ok ||
        objects->record_count == 0u ||
        objects->record_count != placement->object_record_count ||
        objects->checksum != placement->object_table_checksum ||
        objects->level_mask != placement->object_level_mask ||
        objects->level_record_counts[placement->selected_level_index] !=
            placement->selected_level_record_count ||
        objects->level_record_hashes[placement->selected_level_index] !=
            placement->selected_level_record_hash ||
        objects->level_position_hashes[placement->selected_level_index] !=
            placement->selected_level_position_hash) {
        return 0;
    }

    runtime_state_hash =
        theron_v1_runtime_track02_object_runtime_state_hash(
            objects, placement->selected_level_index, &low_kind_mask,
            &quest_item_seen);
    if (runtime_state_hash == 0u) {
        return 0;
    }
    for (i = 0u; i < objects->record_count; ++i) {
        if (objects->records[i].level_index == placement->selected_level_index) {
            first = &objects->records[i];
            break;
        }
    }
    if (!first ||
        !theron_v1_runtime_track02_object_kind_supported(first->kind)) {
        return 0;
    }

    if (!theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_object_gameplay_semantics") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_placement_state=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", placement->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            placement->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_dungeon_index",
            placement->selected_dungeon_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_level_index",
            placement->selected_level_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_table_route_hash",
            placement->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_route_pair_hash",
            placement->loader_route_pair_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_placement_state_hash",
            placement->object_placement_state_hash) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "selected_level_record_count",
            placement->selected_level_record_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_level_record_hash",
            placement->selected_level_record_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_level_position_hash",
            placement->selected_level_position_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "runtime_kind_low_mask", low_kind_mask) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "runtime_kind_quest_item_seen",
            (uint32_t)quest_item_seen) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "object_runtime_state_hash",
            runtime_state_hash) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_kind_semantics_proven=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "flags_low_bits_state_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "argument_quantity_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_flags_preserved=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "all_selected_records_runtime_mappable=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "world_object_publish_allowed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_runtime_admission_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.object_placement_state_consumed = 1;
    receipt.object_table_shape_consumed = 1;
    receipt.same_capture_as_placement_state = 1;
    receipt.variant = placement->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             placement->track02_md5);
    receipt.record = placement->record;
    receipt.consumer_trace_checksum = placement->consumer_trace_checksum;
    receipt.selected_dungeon_index = placement->selected_dungeon_index;
    receipt.selected_level_index = placement->selected_level_index;
    receipt.object_table_route_hash = placement->object_table_route_hash;
    receipt.loader_route_pair_hash = placement->loader_route_pair_hash;
    receipt.object_placement_state_hash =
        placement->object_placement_state_hash;
    receipt.selected_level_record_count =
        placement->selected_level_record_count;
    receipt.selected_level_record_hash = placement->selected_level_record_hash;
    receipt.selected_level_position_hash =
        placement->selected_level_position_hash;
    receipt.runtime_kind_low_mask = low_kind_mask;
    receipt.runtime_kind_quest_item_seen = quest_item_seen;
    receipt.object_runtime_state_hash = runtime_state_hash;
    receipt.first_runtime_type = first->kind;
    receipt.first_runtime_state = (uint8_t)(first->flags & 0x03u);
    receipt.first_runtime_flags = first->flags;
    receipt.first_runtime_quantity = first->argument ? first->argument : 1;
    receipt.object_kind_semantics_proven = 1;
    receipt.flags_low_bits_state_bound = 1;
    receipt.argument_quantity_bound = 1;
    receipt.object_flags_preserved = 1;
    receipt.all_selected_records_runtime_mappable = 1;
    receipt.world_object_publish_allowed = 1;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_publish_track02_object_gameplay_state(
    Theron_V1_World *world,
    Theron_DungeonID dungeon_id,
    const Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt *semantics,
    const Theron_Track02ObjectTable *objects,
    Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt *out) {
    Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt receipt = {0};
    int dungeon_slot;
    int selected_level;
    int write_index = 0;
    int removed = 0;
    int placed = 0;
    int original_count;
    size_t i;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!world || !semantics || !objects || !out ||
        !semantics->valid ||
        !semantics->object_placement_state_consumed ||
        !semantics->object_table_shape_consumed ||
        !semantics->same_capture_as_placement_state ||
        semantics->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(semantics->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        semantics->selected_dungeon_index == 0u ||
        semantics->selected_dungeon_index > THERON_DUNGEON_COUNT ||
        semantics->selected_dungeon_index != (uint32_t)dungeon_id ||
        semantics->selected_level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
        semantics->selected_level_record_count == 0u ||
        !semantics->object_kind_semantics_proven ||
        !semantics->flags_low_bits_state_bound ||
        !semantics->argument_quantity_bound ||
        !semantics->object_flags_preserved ||
        !semantics->all_selected_records_runtime_mappable ||
        !semantics->world_object_publish_allowed ||
        semantics->dungeon_runtime_admission_allowed ||
        semantics->dungeon_draw_allowed ||
        semantics->fallback_visuals_allowed ||
        !objects->shape_ok ||
        objects->record_count == 0u ||
        objects->level_record_counts[semantics->selected_level_index] !=
            semantics->selected_level_record_count ||
        objects->level_record_hashes[semantics->selected_level_index] !=
            semantics->selected_level_record_hash ||
        objects->level_position_hashes[semantics->selected_level_index] !=
            semantics->selected_level_position_hash ||
        world->object_count < 0 ||
        world->object_count > THERON_MAX_OBJECTS) {
        return 0;
    }

    dungeon_slot = (int)dungeon_id - 1;
    selected_level = (int)semantics->selected_level_index;
    if (!world->level_loaded[dungeon_slot][selected_level] ||
        world->levels[dungeon_slot][selected_level].width <= 0 ||
        world->levels[dungeon_slot][selected_level].height <= 0) {
        return 0;
    }

    receipt.before_world_hash = theron_v1_world_hash(world);
    receipt.before_object_count = world->object_count;
    original_count = world->object_count;
    for (int read_index = 0; read_index < original_count; ++read_index) {
        Theron_V1_Object object = world->objects[read_index];
        if (object.dungeon_id == (int)dungeon_id &&
            object.level == selected_level) {
            ++removed;
            continue;
        }
        world->objects[write_index++] = object;
    }
    world->object_count = write_index;

    for (i = 0u; i < objects->record_count; ++i) {
        const Theron_Track02ObjectTableRecord *record = &objects->records[i];
        Theron_V1_Object object;

        if (record->level_index != semantics->selected_level_index) {
            continue;
        }
        if (!theron_v1_runtime_track02_object_kind_supported(record->kind) ||
            record->x >=
                (uint8_t)world->levels[dungeon_slot][selected_level].width ||
            record->y >=
                (uint8_t)world->levels[dungeon_slot][selected_level].height) {
            return 0;
        }
        memset(&object, 0, sizeof(object));
        object.type = record->kind;
        object.state = record->flags & 0x03u;
        object.x = record->x;
        object.y = record->y;
        object.level = selected_level;
        object.dungeon_id = dungeon_id;
        object.quantity = record->argument ? record->argument : 1;
        object.flags = record->flags;
        if (theron_v1_object_place(world, &object) != 0) {
            return 0;
        }
        ++placed;
    }
    if (placed != (int)semantics->selected_level_record_count) {
        return 0;
    }

    world->current_dungeon = dungeon_id;
    world->current_level = selected_level;
    world->levels[dungeon_slot][selected_level].thing_count = placed;
    theron_v1_world_runtime_media_invalidate_cache(world);

    receipt.valid = 1;
    receipt.object_gameplay_semantics_consumed = 1;
    receipt.object_table_shape_consumed = 1;
    receipt.world_mutated = 1;
    receipt.variant = semantics->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             semantics->track02_md5);
    receipt.record = semantics->record;
    receipt.selected_dungeon_index = semantics->selected_dungeon_index;
    receipt.selected_level_index = semantics->selected_level_index;
    receipt.selected_level_record_count =
        semantics->selected_level_record_count;
    receipt.removed_selected_level_object_count = removed;
    receipt.placed_object_count = placed;
    receipt.after_object_count = world->object_count;
    receipt.level_loaded_required = 1;
    receipt.level_loaded = 1;
    receipt.current_level_after = world->current_level;
    receipt.thing_count_after =
        world->levels[dungeon_slot][selected_level].thing_count;
    receipt.after_world_hash = theron_v1_world_hash(world);
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_level_transition_handoff(
    const Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt *source,
    const Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt *target,
    const char *capture_trace,
    Theron_V1RuntimeTrack02LevelTransitionHandoffReceipt *out) {
    Theron_V1RuntimeTrack02LevelTransitionHandoffReceipt receipt = {0};
    uint32_t target_level_byte_count = 0u;
    uint32_t target_level_raw_hash = 0u;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!source || !target || !capture_trace || !out ||
        !source->valid ||
        !source->object_gameplay_semantics_consumed ||
        !source->object_table_shape_consumed ||
        !source->world_mutated ||
        source->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(source->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        source->record == 0u ||
        source->selected_dungeon_index == 0u ||
        source->selected_dungeon_index > THERON_DUNGEON_COUNT ||
        source->selected_level_record_count == 0u ||
        !source->level_loaded ||
        source->dungeon_runtime_admission_allowed ||
        source->dungeon_draw_allowed ||
        source->fallback_visuals_allowed ||
        !target->valid ||
        !target->object_placement_state_consumed ||
        !target->object_table_shape_consumed ||
        !target->same_capture_as_placement_state ||
        target->variant != source->variant ||
        strcmp(target->track02_md5, source->track02_md5) != 0 ||
        target->record != source->record ||
        target->consumer_trace_checksum == 0u ||
        target->selected_dungeon_index != source->selected_dungeon_index ||
        target->selected_level_index == source->selected_level_index ||
        target->selected_level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
        target->object_table_route_hash == 0u ||
        target->loader_route_pair_hash == 0u ||
        target->object_runtime_state_hash == 0u ||
        target->selected_level_record_count == 0u ||
        target->selected_level_record_hash == 0u ||
        !target->object_kind_semantics_proven ||
        !target->world_object_publish_allowed ||
        target->dungeon_runtime_admission_allowed ||
        target->dungeon_draw_allowed ||
        target->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_level_transition_handoff") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_target_loader_route=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", target->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "consumer_trace_checksum",
            target->consumer_trace_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_dungeon_index",
            target->selected_dungeon_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "source_level_index",
            source->selected_level_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "target_level_index",
            target->selected_level_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "target_object_table_route_hash",
            target->object_table_route_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "target_loader_route_pair_hash",
            target->loader_route_pair_hash) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "target_object_runtime_state_hash",
            target->object_runtime_state_hash) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "target_level_byte_count",
            &target_level_byte_count) ||
        target_level_byte_count == 0u ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "target_level_raw_hash",
            &target_level_raw_hash) ||
        target_level_raw_hash == 0u ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "target_object_record_count",
            target->selected_level_record_count) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "target_object_level_record_hash",
            target->selected_level_record_hash) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "loader_level_selector_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "transition_source_level_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "transition_target_level_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "party_placement_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_pool_state_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "level_runtime_load_allowed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_runtime_admission_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    receipt.valid = 1;
    receipt.source_object_world_handoff_consumed = 1;
    receipt.target_object_gameplay_semantics_consumed = 1;
    receipt.same_capture_as_target_loader_route = 1;
    receipt.variant = target->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             target->track02_md5);
    receipt.record = target->record;
    receipt.consumer_trace_checksum = target->consumer_trace_checksum;
    receipt.selected_dungeon_index = target->selected_dungeon_index;
    receipt.source_level_index = source->selected_level_index;
    receipt.target_level_index = target->selected_level_index;
    receipt.target_object_table_route_hash = target->object_table_route_hash;
    receipt.target_loader_route_pair_hash = target->loader_route_pair_hash;
    receipt.target_object_runtime_state_hash =
        target->object_runtime_state_hash;
    receipt.target_level_byte_count = target_level_byte_count;
    receipt.target_level_raw_hash = target_level_raw_hash;
    receipt.target_object_record_count = target->selected_level_record_count;
    receipt.target_object_level_record_hash =
        target->selected_level_record_hash;
    receipt.loader_level_selector_bound = 1;
    receipt.transition_source_level_bound = 1;
    receipt.transition_target_level_bound = 1;
    receipt.party_placement_bound = 1;
    receipt.object_pool_state_bound = 1;
    receipt.level_runtime_load_allowed = 1;
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_publish_track02_level_transition(
    Theron_V1_World *world,
    const Theron_V1RuntimeTrack02LevelTransitionHandoffReceipt *handoff,
    const Theron_V1_Level *target_level,
    const Theron_Track02ObjectTable *target_objects,
    const Theron_V1RuntimeTrack02ObjectGameplaySemanticsReceipt
        *target_semantics,
    Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *out) {
    Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt receipt = {0};
    Theron_V1RuntimeTrack02ObjectWorldHandoffReceipt object_handoff;
    int dungeon_slot;
    int target_level_index;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!world || !handoff || !target_level || !target_objects ||
        !target_semantics || !out ||
        !handoff->valid ||
        !handoff->source_object_world_handoff_consumed ||
        !handoff->target_object_gameplay_semantics_consumed ||
        !handoff->same_capture_as_target_loader_route ||
        handoff->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(handoff->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        handoff->record == 0u ||
        handoff->selected_dungeon_index == 0u ||
        handoff->selected_dungeon_index > THERON_DUNGEON_COUNT ||
        handoff->source_level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
        handoff->target_level_index >= THERON_MAX_LEVELS_PER_DUNGEON ||
        handoff->source_level_index == handoff->target_level_index ||
        !handoff->loader_level_selector_bound ||
        !handoff->transition_source_level_bound ||
        !handoff->transition_target_level_bound ||
        !handoff->party_placement_bound ||
        !handoff->object_pool_state_bound ||
        !handoff->level_runtime_load_allowed ||
        handoff->dungeon_runtime_admission_allowed ||
        handoff->dungeon_draw_allowed ||
        handoff->fallback_visuals_allowed ||
        !target_semantics->valid ||
        target_semantics->selected_dungeon_index !=
            handoff->selected_dungeon_index ||
        target_semantics->selected_level_index != handoff->target_level_index ||
        target_semantics->selected_level_record_count !=
            handoff->target_object_record_count ||
        target_semantics->selected_level_record_hash !=
            handoff->target_object_level_record_hash ||
        !target_semantics->world_object_publish_allowed ||
        target_semantics->dungeon_runtime_admission_allowed ||
        target_semantics->dungeon_draw_allowed ||
        target_semantics->fallback_visuals_allowed ||
        !world->transition_pending ||
        world->transition_type != THERON_TRANSITION_STAIRS ||
        world->current_dungeon != (int)handoff->selected_dungeon_index ||
        world->current_level != (int)handoff->source_level_index ||
        world->transition_target_level != (int)handoff->target_level_index ||
        target_level->width <= 0 ||
        target_level->height <= 0 ||
        target_level->start_x < 0 ||
        target_level->start_y < 0 ||
        target_level->start_x >= target_level->width ||
        target_level->start_y >= target_level->height) {
        return 0;
    }

    dungeon_slot = (int)handoff->selected_dungeon_index - 1;
    target_level_index = (int)handoff->target_level_index;
    receipt.before_world_hash = theron_v1_world_hash(world);
    receipt.transition_pending_before = world->transition_pending;

    world->levels[dungeon_slot][target_level_index] = *target_level;
    world->level_loaded[dungeon_slot][target_level_index] = 1;
    if (!theron_v1_runtime_publish_track02_object_gameplay_state(
            world,
            (Theron_DungeonID)handoff->selected_dungeon_index,
            target_semantics,
            target_objects,
            &object_handoff) ||
        !object_handoff.valid ||
        !object_handoff.world_mutated ||
        object_handoff.dungeon_runtime_admission_allowed ||
        object_handoff.dungeon_draw_allowed ||
        object_handoff.fallback_visuals_allowed) {
        return 0;
    }

    theron_v1_party_place(world, target_level->start_x, target_level->start_y,
                          target_level->start_dir);
    world->transition_pending = 0;
    world->transition_type = 0;
    world->transition_target_level = 0;
    theron_v1_world_runtime_media_invalidate_cache(world);

    receipt.valid = 1;
    receipt.level_transition_handoff_consumed = 1;
    receipt.target_object_world_handoff_consumed = 1;
    receipt.world_mutated = 1;
    receipt.variant = handoff->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             handoff->track02_md5);
    receipt.record = handoff->record;
    receipt.selected_dungeon_index = handoff->selected_dungeon_index;
    receipt.source_level_index = handoff->source_level_index;
    receipt.target_level_index = handoff->target_level_index;
    receipt.transition_pending_after = world->transition_pending;
    receipt.level_loaded = world->level_loaded[dungeon_slot][target_level_index];
    receipt.current_level_after = world->current_level;
    receipt.party_x = world->party.leader_x;
    receipt.party_y = world->party.leader_y;
    receipt.party_dir = world->party.leader_dir;
    receipt.target_object_count = object_handoff.placed_object_count;
    receipt.target_thing_count =
        world->levels[dungeon_slot][target_level_index].thing_count;
    receipt.after_world_hash = theron_v1_world_hash(world);
    receipt.dungeon_runtime_admission_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

static uint32_t theron_v1_runtime_track02_bitmap_palette_source_hash(
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

    hash = theron_v1_runtime_mix_hash(hash, runtime->record);
    hash = theron_v1_runtime_mix_hash(hash, runtime->selected_dungeon_index);
    hash = theron_v1_runtime_mix_hash(hash, runtime->source_level_index);
    hash = theron_v1_runtime_mix_hash(hash, runtime->target_level_index);
    hash = theron_v1_runtime_mix_hash(hash, (uint32_t)palette_raw_offset);
    hash = theron_v1_runtime_mix_hash(hash, (uint32_t)palette_user_data_offset);
    hash = theron_v1_runtime_mix_hash(hash, palette_payload_checksum);
    hash = theron_v1_runtime_mix_hash(hash, palette_decoded_checksum);
    hash = theron_v1_runtime_mix_hash(hash, bitmap_route_mask);
    hash = theron_v1_runtime_mix_hash(hash, bitmap_atlas_checksum);
    hash = theron_v1_runtime_mix_hash(hash, bitmap_atlas_route_count);
    hash = theron_v1_runtime_mix_hash(hash, bitmap_atlas_nonzero_pixel_count);
    return hash ? hash : 2166136261u;
}

int theron_v1_runtime_bind_track02_bitmap_palette_source(
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *runtime,
    const char *capture_trace,
    Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt *out) {
    Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt receipt = {0};
    size_t palette_raw_offset;
    size_t palette_user_data_offset;
    uint32_t palette_payload_checksum;
    uint32_t palette_decoded_checksum;
    uint32_t bitmap_route_mask;
    uint32_t bitmap_atlas_checksum;
    uint32_t bitmap_atlas_route_count;
    uint32_t bitmap_atlas_nonzero_pixel_count;
    uint32_t bitmap_palette_source_hash;
    uint32_t expected_hash;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!runtime || !capture_trace || !out ||
        !runtime->valid ||
        !runtime->level_transition_handoff_consumed ||
        !runtime->target_object_world_handoff_consumed ||
        !runtime->world_mutated ||
        runtime->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(runtime->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        runtime->record == 0u ||
        runtime->selected_dungeon_index == 0u ||
        runtime->source_level_index == runtime->target_level_index ||
        runtime->level_loaded != 1 ||
        runtime->transition_pending_before != 1 ||
        runtime->transition_pending_after != 0 ||
        runtime->dungeon_runtime_admission_allowed ||
        runtime->dungeon_draw_allowed ||
        runtime->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_bitmap_palette_source") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_level_transition=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "record", runtime->record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "selected_dungeon_index",
            runtime->selected_dungeon_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "source_level_index",
            runtime->source_level_index) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "target_level_index",
            runtime->target_level_index) ||
        !theron_v1_runtime_trace_read_size(
            capture_trace, "palette_raw_offset", &palette_raw_offset) ||
        !theron_v1_runtime_trace_read_size(
            capture_trace, "palette_user_data_offset",
            &palette_user_data_offset) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "palette_payload_checksum",
            &palette_payload_checksum) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "palette_decoded_checksum",
            &palette_decoded_checksum) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "bitmap_route_mask", &bitmap_route_mask) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "bitmap_atlas_checksum", &bitmap_atlas_checksum) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "bitmap_atlas_route_count",
            &bitmap_atlas_route_count) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "bitmap_atlas_nonzero_pixel_count",
            &bitmap_atlas_nonzero_pixel_count) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "bitmap_palette_source_hash",
            &bitmap_palette_source_hash) ||
        palette_raw_offset == 0u ||
        palette_user_data_offset == 0u ||
        palette_payload_checksum == 0u ||
        palette_decoded_checksum == 0u ||
        bitmap_route_mask == 0u ||
        bitmap_atlas_checksum == 0u ||
        bitmap_atlas_route_count == 0u ||
        bitmap_atlas_nonzero_pixel_count == 0u ||
        !theron_v1_runtime_trace_has(
            capture_trace, "palette_window_source_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "bitmap_route_source_bound=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "palette_decode_verified=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "bitmap_decode_verified=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "pixel_output_verified=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "m11_render_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_draw_allowed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    expected_hash =
        theron_v1_runtime_track02_bitmap_palette_source_hash(
            runtime, palette_raw_offset, palette_user_data_offset,
            palette_payload_checksum, palette_decoded_checksum,
            bitmap_route_mask, bitmap_atlas_checksum, bitmap_atlas_route_count,
            bitmap_atlas_nonzero_pixel_count);
    if (bitmap_palette_source_hash != expected_hash) {
        return 0;
    }

    receipt.valid = 1;
    receipt.level_transition_runtime_consumed = 1;
    receipt.same_capture_as_level_transition = 1;
    receipt.variant = runtime->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             runtime->track02_md5);
    receipt.record = runtime->record;
    receipt.selected_dungeon_index = runtime->selected_dungeon_index;
    receipt.source_level_index = runtime->source_level_index;
    receipt.target_level_index = runtime->target_level_index;
    receipt.palette_raw_offset = palette_raw_offset;
    receipt.palette_user_data_offset = palette_user_data_offset;
    receipt.palette_payload_checksum = palette_payload_checksum;
    receipt.palette_decoded_checksum = palette_decoded_checksum;
    receipt.bitmap_route_mask = bitmap_route_mask;
    receipt.bitmap_atlas_checksum = bitmap_atlas_checksum;
    receipt.bitmap_atlas_route_count = bitmap_atlas_route_count;
    receipt.bitmap_atlas_nonzero_pixel_count =
        bitmap_atlas_nonzero_pixel_count;
    receipt.bitmap_palette_source_hash = bitmap_palette_source_hash;
    receipt.palette_window_source_bound = 1;
    receipt.bitmap_route_source_bound = 1;
    receipt.palette_decode_verified = 0;
    receipt.bitmap_decode_verified = 0;
    receipt.pixel_output_verified = 0;
    receipt.m11_render_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_decode_track02_bitmap_palette_vector(
    const Theron_V1RuntimeTrack02BitmapPaletteSourceReceipt *source,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *track02_md5,
    Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt *out) {
    Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt receipt = {0};
    Theron_Track02PaletteWindowEvidence palette_window;
    Theron_Track02StartupBitmapCatalog bitmap_catalog;
    Theron_Track02StartupBitmapAtlas bitmap_atlas;
    const Theron_Track02StartupBitmapAtlasRoute *first_route;
    const Theron_Track02StartupBitmapAtlasRoute *stage_route = NULL;
    uint32_t row_hash = 2166136261u;
    size_t i;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!source || !track02_data || track02_size == 0u ||
        !track02_md5 || !out ||
        !source->valid ||
        !source->level_transition_runtime_consumed ||
        !source->same_capture_as_level_transition ||
        source->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(source->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        strcmp(track02_md5, source->track02_md5) != 0 ||
        source->record == 0u ||
        source->selected_dungeon_index == 0u ||
        source->source_level_index == source->target_level_index ||
        source->palette_raw_offset == 0u ||
        source->palette_user_data_offset == 0u ||
        source->palette_payload_checksum == 0u ||
        source->palette_decoded_checksum == 0u ||
        source->bitmap_route_mask == 0u ||
        source->bitmap_atlas_checksum == 0u ||
        source->bitmap_atlas_route_count == 0u ||
        source->bitmap_atlas_nonzero_pixel_count == 0u ||
        !source->palette_window_source_bound ||
        !source->bitmap_route_source_bound ||
        source->palette_decode_verified ||
        source->bitmap_decode_verified ||
        source->pixel_output_verified ||
        source->m11_render_allowed ||
        source->dungeon_draw_allowed ||
        source->fallback_visuals_allowed) {
        return 0;
    }

    memset(&palette_window, 0, sizeof(palette_window));
    if (theron_v1_track02_inspect_4bpp_palette_window(
            track02_data,
            track02_size,
            track02_md5,
            source->palette_raw_offset,
            &palette_window) != THERON_TRACK02_SIGNAL_OK ||
        palette_window.variant != source->variant ||
        palette_window.raw_offset != source->palette_raw_offset ||
        palette_window.user_data_offset != source->palette_user_data_offset ||
        palette_window.payload_checksum != source->palette_payload_checksum ||
        palette_window.palette.checksum != source->palette_decoded_checksum ||
        !palette_window.format_valid ||
        !palette_window.palette.valid ||
        palette_window.palette.nonblack_entry_count == 0u ||
        palette_window.semantic_binding_verified ||
        palette_window.promotion_allowed) {
        return 0;
    }

    memset(&bitmap_catalog, 0, sizeof(bitmap_catalog));
    memset(&bitmap_atlas, 0, sizeof(bitmap_atlas));
    if (theron_v1_track02_catalog_startup_bitmap_samples(
            track02_data, track02_size, track02_md5, &bitmap_catalog) !=
            THERON_TRACK02_SIGNAL_OK ||
        theron_v1_track02_build_startup_bitmap_atlas_wide(
            &bitmap_catalog, &bitmap_atlas) != THERON_TRACK02_SIGNAL_OK ||
        bitmap_atlas.variant != source->variant ||
        bitmap_atlas.route_mask != source->bitmap_route_mask ||
        bitmap_atlas.checksum != source->bitmap_atlas_checksum ||
        bitmap_atlas.route_count != source->bitmap_atlas_route_count ||
        bitmap_atlas.total_nonzero_pixel_count !=
            source->bitmap_atlas_nonzero_pixel_count ||
        bitmap_atlas.route_count == 0u ||
        bitmap_atlas.total_tile_count == 0u) {
        return 0;
    }

    first_route = &bitmap_atlas.routes[0];
    for (i = 0u; i < bitmap_atlas.route_count; ++i) {
        if (bitmap_atlas.routes[i].route_bit ==
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) {
            stage_route = &bitmap_atlas.routes[i];
            break;
        }
    }
    if (first_route->route_bit == 0u ||
        first_route->width == 0u ||
        first_route->height == 0u ||
        first_route->tile_count == 0u ||
        first_route->nonzero_pixel_count == 0u ||
        !stage_route ||
        stage_route->width == 0u ||
        stage_route->height == 0u ||
        stage_route->tile_count == 0u ||
        stage_route->nonzero_pixel_count == 0u ||
        stage_route->checksum == 0u ||
        stage_route->first_raw_offset == 0u ||
        stage_route->first_user_data_offset == 0u) {
        return 0;
    }

    for (i = 0u; i < sizeof(receipt.first_pixel_indices); ++i) {
        receipt.first_pixel_indices[i] = first_route->pixels[i];
        row_hash = theron_v1_runtime_mix_hash(row_hash, first_route->pixels[i]);
    }
    if (row_hash == 0u) {
        return 0;
    }

    receipt.valid = 1;
    receipt.bitmap_palette_source_consumed = 1;
    receipt.real_track02_bytes_consumed = 1;
    receipt.variant = source->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             source->track02_md5);
    receipt.record = source->record;
    receipt.selected_dungeon_index = source->selected_dungeon_index;
    receipt.source_level_index = source->source_level_index;
    receipt.target_level_index = source->target_level_index;
    receipt.palette_raw_offset = palette_window.raw_offset;
    receipt.palette_user_data_offset = palette_window.user_data_offset;
    receipt.palette_payload_checksum = palette_window.payload_checksum;
    receipt.palette_decoded_checksum = palette_window.palette.checksum;
    receipt.palette_nonblack_entry_count =
        palette_window.palette.nonblack_entry_count;
    receipt.first_palette_word = palette_window.palette.entries[0].raw_word;
    receipt.first_palette_red = palette_window.palette.entries[0].red;
    receipt.first_palette_green = palette_window.palette.entries[0].green;
    receipt.first_palette_blue = palette_window.palette.entries[0].blue;
    receipt.bitmap_route_mask = bitmap_atlas.route_mask;
    receipt.bitmap_atlas_checksum = bitmap_atlas.checksum;
    receipt.bitmap_atlas_route_count = bitmap_atlas.route_count;
    receipt.bitmap_atlas_tile_count = bitmap_atlas.total_tile_count;
    receipt.bitmap_atlas_nonzero_pixel_count =
        bitmap_atlas.total_nonzero_pixel_count;
    receipt.first_bitmap_route_bit = first_route->route_bit;
    receipt.first_bitmap_route_width = first_route->width;
    receipt.first_bitmap_route_height = first_route->height;
    receipt.first_bitmap_route_tile_count = first_route->tile_count;
    receipt.first_bitmap_route_nonzero_pixel_count =
        first_route->nonzero_pixel_count;
    receipt.first_bitmap_route_checksum = first_route->checksum;
    receipt.first_bitmap_raw_offset = first_route->first_raw_offset;
    receipt.first_bitmap_user_data_offset = first_route->first_user_data_offset;
    receipt.stage_bitmap_route_bit = stage_route->route_bit;
    receipt.stage_bitmap_route_width = stage_route->width;
    receipt.stage_bitmap_route_height = stage_route->height;
    receipt.stage_bitmap_route_tile_count = stage_route->tile_count;
    receipt.stage_bitmap_route_nonzero_pixel_count =
        stage_route->nonzero_pixel_count;
    receipt.stage_bitmap_route_checksum = stage_route->checksum;
    receipt.stage_bitmap_raw_offset = stage_route->first_raw_offset;
    receipt.stage_bitmap_user_data_offset = stage_route->first_user_data_offset;
    receipt.first_pixel_row_hash = row_hash;
    receipt.palette_decode_verified = 1;
    receipt.bitmap_decode_verified = 1;
    receipt.pixel_output_verified = 1;
    receipt.m11_runtime_consumption_allowed = 0;
    receipt.m11_render_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_m11_soul_room_consumption(
    const Theron_V1RuntimeTrack02BitmapPaletteDecodeVectorReceipt *decode,
    Theron_V1_World *world,
    int host_surface_width,
    int host_surface_height,
    int placement_x,
    int placement_y,
    int scale_x,
    int scale_y,
    Theron_V1RuntimeTrack02M11SoulRoomConsumptionReceipt *out) {
    Theron_V1RuntimeTrack02M11SoulRoomConsumptionReceipt receipt = {0};
    const Theron_RuntimeMediaSurface *surface;
    uint32_t placement_hash = 2166136261u;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!decode || !world || !out ||
        !decode->valid ||
        !decode->bitmap_palette_source_consumed ||
        !decode->real_track02_bytes_consumed ||
        decode->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(decode->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        decode->selected_dungeon_index == 0u ||
        decode->source_level_index != 0u ||
        decode->first_bitmap_route_bit !=
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM ||
        decode->first_bitmap_route_width == 0u ||
        decode->first_bitmap_route_height == 0u ||
        decode->first_bitmap_route_tile_count == 0u ||
        decode->first_bitmap_route_nonzero_pixel_count == 0u ||
        decode->first_bitmap_route_checksum == 0u ||
        decode->first_bitmap_raw_offset == 0u ||
        decode->first_bitmap_user_data_offset == 0u ||
        decode->palette_decoded_checksum == 0u ||
        decode->palette_nonblack_entry_count == 0u ||
        !decode->palette_decode_verified ||
        !decode->bitmap_decode_verified ||
        !decode->pixel_output_verified ||
        decode->m11_runtime_consumption_allowed ||
        decode->m11_render_allowed ||
        decode->dungeon_draw_allowed ||
        decode->fallback_visuals_allowed ||
        !world->runtime_media.restored ||
        host_surface_width <= 0 ||
        host_surface_height <= 0 ||
        placement_x < 0 ||
        placement_y < 0 ||
        scale_x != 1 ||
        scale_y != 1) {
        return 0;
    }

    surface = &world->runtime_media.soul_room;
    if (!surface->ready ||
        !surface->raw_source_verified ||
        strcmp(surface->track02_md5, decode->track02_md5) != 0 ||
        surface->route_bit != decode->first_bitmap_route_bit ||
        surface->width != decode->first_bitmap_route_width ||
        surface->height != decode->first_bitmap_route_height ||
        surface->tile_count != decode->first_bitmap_route_tile_count ||
        surface->nonzero_pixel_count !=
            decode->first_bitmap_route_nonzero_pixel_count ||
        surface->checksum != decode->first_bitmap_route_checksum ||
        surface->first_raw_offset != decode->first_bitmap_raw_offset ||
        surface->first_user_data_offset !=
            decode->first_bitmap_user_data_offset ||
        placement_x + (int)surface->width > host_surface_width ||
        placement_y + (int)surface->height > host_surface_height ||
        !theron_v1_world_runtime_media_select_level_bank(
            world,
            THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL,
            (Theron_DungeonID)decode->selected_dungeon_index,
            0)) {
        return 0;
    }
    if (!world->runtime_media.level_bank.ready ||
        !world->runtime_media.level_bank.real_media_gate ||
        world->runtime_media.level_bank.kind !=
            THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL ||
        world->runtime_media.level_bank.level_index != 0 ||
        world->runtime_media.level_bank.route_bit != surface->route_bit ||
        world->runtime_media.level_bank.surface_checksum !=
            surface->checksum ||
        world->runtime_media.level_bank.first_raw_offset !=
            surface->first_raw_offset ||
        world->runtime_media.level_bank.first_user_data_offset !=
            surface->first_user_data_offset) {
        return 0;
    }

    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)host_surface_width);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)host_surface_height);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)placement_x);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)placement_y);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)surface->width);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)surface->height);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, surface->checksum);
    if (placement_hash == 0u) {
        return 0;
    }

    receipt.valid = 1;
    receipt.decode_vector_consumed = 1;
    receipt.world_runtime_media_consumed = 1;
    receipt.soul_room_level0_selected = 1;
    receipt.exact_indexed_atlas_consumed = 1;
    receipt.huc6260_palette_consumed = 1;
    receipt.variant = decode->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             decode->track02_md5);
    receipt.record = decode->record;
    receipt.selected_dungeon_index = decode->selected_dungeon_index;
    receipt.level_index = 0u;
    receipt.route_bit = surface->route_bit;
    receipt.source_width = surface->width;
    receipt.source_height = surface->height;
    receipt.source_tile_count = surface->tile_count;
    receipt.source_nonzero_pixel_count = surface->nonzero_pixel_count;
    receipt.source_checksum = surface->checksum;
    receipt.palette_decoded_checksum = decode->palette_decoded_checksum;
    receipt.palette_nonblack_entry_count =
        decode->palette_nonblack_entry_count;
    receipt.first_raw_offset = surface->first_raw_offset;
    receipt.last_raw_offset = surface->last_raw_offset;
    receipt.first_user_data_offset = surface->first_user_data_offset;
    receipt.host_surface_width = host_surface_width;
    receipt.host_surface_height = host_surface_height;
    receipt.placement_x = placement_x;
    receipt.placement_y = placement_y;
    receipt.scale_x = scale_x;
    receipt.scale_y = scale_y;
    receipt.clip_x = placement_x;
    receipt.clip_y = placement_y;
    receipt.clip_w = surface->width;
    receipt.clip_h = surface->height;
    receipt.placement_hash = placement_hash;
    receipt.clip_verified = 1;
    receipt.scale_verified = 1;
    receipt.host_presentation_allowed = 1;
    receipt.m11_runtime_consumption_allowed = 1;
    receipt.m11_render_allowed = 1;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

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
    Theron_V1RuntimeTrack02M11LevelConsumptionReceipt *out) {
    Theron_V1RuntimeTrack02M11LevelConsumptionReceipt receipt = {0};
    const Theron_RuntimeMediaSurface *surface;
    uint32_t expected_route_bit;
    size_t expected_width;
    size_t expected_height;
    size_t expected_tile_count;
    size_t expected_nonzero_pixel_count;
    uint32_t expected_checksum;
    size_t expected_raw_offset;
    size_t expected_user_data_offset;
    uint32_t placement_hash = 2166136261u;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!decode || !transition || !world || !out ||
        !decode->valid ||
        !decode->bitmap_palette_source_consumed ||
        !decode->real_track02_bytes_consumed ||
        decode->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(decode->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        !decode->palette_decode_verified ||
        !decode->bitmap_decode_verified ||
        !decode->pixel_output_verified ||
        decode->m11_runtime_consumption_allowed ||
        decode->m11_render_allowed ||
        decode->dungeon_draw_allowed ||
        decode->fallback_visuals_allowed ||
        !transition->valid ||
        !transition->level_transition_handoff_consumed ||
        !transition->target_object_world_handoff_consumed ||
        !transition->world_mutated ||
        transition->variant != decode->variant ||
        strcmp(transition->track02_md5, decode->track02_md5) != 0 ||
        transition->record != decode->record ||
        transition->selected_dungeon_index !=
            decode->selected_dungeon_index ||
        transition->source_level_index != decode->source_level_index ||
        transition->target_level_index != decode->target_level_index ||
        transition->target_level_index == transition->source_level_index ||
        transition->target_level_index == 0u ||
        !transition->level_loaded ||
        transition->dungeon_runtime_admission_allowed ||
        transition->dungeon_draw_allowed ||
        transition->fallback_visuals_allowed ||
        !world->runtime_media.restored ||
        host_surface_width <= 0 ||
        host_surface_height <= 0 ||
        placement_x < 0 ||
        placement_y < 0 ||
        scale_x != 1 ||
        scale_y != 1) {
        return 0;
    }

    expected_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;
    expected_width = decode->stage_bitmap_route_width;
    expected_height = decode->stage_bitmap_route_height;
    expected_tile_count = decode->stage_bitmap_route_tile_count;
    expected_nonzero_pixel_count =
        decode->stage_bitmap_route_nonzero_pixel_count;
    expected_checksum = decode->stage_bitmap_route_checksum;
    expected_raw_offset = decode->stage_bitmap_raw_offset;
    expected_user_data_offset = decode->stage_bitmap_user_data_offset;
    if (decode->stage_bitmap_route_bit != expected_route_bit ||
        expected_width == 0u ||
        expected_height == 0u ||
        expected_tile_count == 0u ||
        expected_nonzero_pixel_count == 0u ||
        expected_checksum == 0u ||
        expected_raw_offset == 0u ||
        expected_user_data_offset == 0u) {
        return 0;
    }

    surface = theron_v1_world_runtime_media_for_level(
        world, (int)transition->target_level_index, 0);
    if (!surface ||
        !surface->ready ||
        !surface->raw_source_verified ||
        strcmp(surface->track02_md5, decode->track02_md5) != 0 ||
        surface->route_bit != expected_route_bit ||
        surface->width != expected_width ||
        surface->height != expected_height ||
        surface->tile_count != expected_tile_count ||
        surface->nonzero_pixel_count != expected_nonzero_pixel_count ||
        surface->checksum != expected_checksum ||
        surface->first_raw_offset != expected_raw_offset ||
        surface->first_user_data_offset != expected_user_data_offset ||
        placement_x + (int)surface->width > host_surface_width ||
        placement_y + (int)surface->height > host_surface_height ||
        !theron_v1_world_runtime_media_select_level_bank(
            world,
            THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL,
            (Theron_DungeonID)transition->selected_dungeon_index,
            (int)transition->target_level_index)) {
        return 0;
    }
    if (!world->runtime_media.level_bank.ready ||
        !world->runtime_media.level_bank.real_media_gate ||
        world->runtime_media.level_bank.kind !=
            THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL ||
        world->runtime_media.level_bank.level_index !=
            (int)transition->target_level_index ||
        world->runtime_media.level_bank.route_bit != surface->route_bit ||
        world->runtime_media.level_bank.surface_checksum !=
            surface->checksum ||
        world->runtime_media.level_bank.first_raw_offset !=
            surface->first_raw_offset ||
        world->runtime_media.level_bank.first_user_data_offset !=
            surface->first_user_data_offset) {
        return 0;
    }

    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)transition->target_level_index);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)host_surface_width);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)host_surface_height);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)placement_x);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)placement_y);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)surface->width);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, (uint32_t)surface->height);
    placement_hash = theron_v1_runtime_mix_hash(
        placement_hash, surface->checksum);
    if (placement_hash == 0u) {
        return 0;
    }

    receipt.valid = 1;
    receipt.decode_vector_consumed = 1;
    receipt.level_transition_runtime_consumed = 1;
    receipt.world_runtime_media_consumed = 1;
    receipt.target_level_selected = 1;
    receipt.exact_indexed_atlas_consumed = 1;
    receipt.huc6260_palette_consumed = 1;
    receipt.variant = decode->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             decode->track02_md5);
    receipt.record = decode->record;
    receipt.selected_dungeon_index = decode->selected_dungeon_index;
    receipt.source_level_index = decode->source_level_index;
    receipt.target_level_index = decode->target_level_index;
    receipt.route_bit = surface->route_bit;
    receipt.source_width = surface->width;
    receipt.source_height = surface->height;
    receipt.source_tile_count = surface->tile_count;
    receipt.source_nonzero_pixel_count = surface->nonzero_pixel_count;
    receipt.source_checksum = surface->checksum;
    receipt.palette_decoded_checksum = decode->palette_decoded_checksum;
    receipt.palette_nonblack_entry_count =
        decode->palette_nonblack_entry_count;
    receipt.first_raw_offset = surface->first_raw_offset;
    receipt.last_raw_offset = surface->last_raw_offset;
    receipt.first_user_data_offset = surface->first_user_data_offset;
    receipt.host_surface_width = host_surface_width;
    receipt.host_surface_height = host_surface_height;
    receipt.placement_x = placement_x;
    receipt.placement_y = placement_y;
    receipt.scale_x = scale_x;
    receipt.scale_y = scale_y;
    receipt.clip_x = placement_x;
    receipt.clip_y = placement_y;
    receipt.clip_w = surface->width;
    receipt.clip_h = surface->height;
    receipt.placement_hash = placement_hash;
    receipt.clip_verified = 1;
    receipt.scale_verified = 1;
    receipt.host_presentation_allowed = 1;
    receipt.m11_runtime_consumption_allowed = 1;
    receipt.m11_render_allowed = 1;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_m11_dungeon_draw_route(
    const Theron_V1RuntimeTrack02M11LevelConsumptionReceipt *level,
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *transition,
    const Theron_V1_World *world,
    Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt *out) {
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt receipt = {0};
    const Theron_V1_Level *loaded;
    uint32_t geometry_hash = 2166136261u;
    uint32_t object_hash = 2166136261u;
    uint32_t route_hash = 2166136261u;
    int dungeon_slot;
    int target_level;
    int sample_coords[5][2];
    size_t object_count = 0u;
    size_t i;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!level || !transition || !world || !out ||
        !level->valid ||
        !level->decode_vector_consumed ||
        !level->level_transition_runtime_consumed ||
        !level->world_runtime_media_consumed ||
        !level->target_level_selected ||
        !level->exact_indexed_atlas_consumed ||
        !level->huc6260_palette_consumed ||
        level->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(level->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        level->record == 0u ||
        level->selected_dungeon_index == 0u ||
        level->target_level_index == 0u ||
        level->route_bit != THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE ||
        level->source_width == 0u ||
        level->source_height == 0u ||
        level->source_checksum == 0u ||
        level->palette_decoded_checksum == 0u ||
        !level->host_presentation_allowed ||
        !level->m11_runtime_consumption_allowed ||
        !level->m11_render_allowed ||
        level->dungeon_draw_allowed ||
        level->fallback_visuals_allowed ||
        !transition->valid ||
        !transition->level_transition_handoff_consumed ||
        !transition->target_object_world_handoff_consumed ||
        !transition->world_mutated ||
        transition->variant != level->variant ||
        strcmp(transition->track02_md5, level->track02_md5) != 0 ||
        transition->record != level->record ||
        transition->selected_dungeon_index != level->selected_dungeon_index ||
        transition->target_level_index != level->target_level_index ||
        transition->target_level_index == transition->source_level_index ||
        !transition->level_loaded ||
        transition->target_object_count == 0 ||
        transition->target_thing_count != transition->target_object_count ||
        transition->dungeon_runtime_admission_allowed ||
        transition->dungeon_draw_allowed ||
        transition->fallback_visuals_allowed ||
        !world->runtime_media.restored ||
        !world->runtime_media.level_bank.ready ||
        !world->runtime_media.level_bank.real_media_gate ||
        world->runtime_media.level_bank.level_index !=
            (int)level->target_level_index ||
        world->runtime_media.level_bank.route_bit != level->route_bit ||
        world->runtime_media.level_bank.surface_checksum !=
            level->source_checksum ||
        world->current_dungeon != (int)level->selected_dungeon_index ||
        world->current_level != (int)level->target_level_index ||
        world->party.leader_x != transition->party_x ||
        world->party.leader_y != transition->party_y ||
        world->party.leader_dir != transition->party_dir ||
        world->party.leader_dir < 0 ||
        world->party.leader_dir > 3) {
        return 0;
    }

    dungeon_slot = (int)level->selected_dungeon_index - 1;
    target_level = (int)level->target_level_index;
    if (dungeon_slot < 0 || dungeon_slot >= (int)THERON_DUNGEON_COUNT ||
        target_level < 0 || target_level >= THERON_MAX_LEVELS_PER_DUNGEON ||
        !world->level_loaded[dungeon_slot][target_level]) {
        return 0;
    }
    loaded = &world->levels[dungeon_slot][target_level];
    if (loaded->width <= 0 || loaded->height <= 0 ||
        loaded->width > THERON_MAX_MAP_SIZE ||
        loaded->height > THERON_MAX_MAP_SIZE ||
        world->party.leader_x < 0 ||
        world->party.leader_y < 0 ||
        world->party.leader_x >= loaded->width ||
        world->party.leader_y >= loaded->height ||
        loaded->squares[world->party.leader_y][world->party.leader_x] ==
            THERON_SQUARE_WALL ||
        loaded->squares[world->party.leader_y][world->party.leader_x] ==
            THERON_SQUARE_SECRET) {
        return 0;
    }

    sample_coords[0][0] = world->party.leader_x;
    sample_coords[0][1] = world->party.leader_y;
    sample_coords[1][0] = world->party.leader_x + dx[world->party.leader_dir];
    sample_coords[1][1] = world->party.leader_y + dy[world->party.leader_dir];
    sample_coords[2][0] = world->party.leader_x +
        dx[(world->party.leader_dir + 3) & 3];
    sample_coords[2][1] = world->party.leader_y +
        dy[(world->party.leader_dir + 3) & 3];
    sample_coords[3][0] = world->party.leader_x +
        dx[(world->party.leader_dir + 1) & 3];
    sample_coords[3][1] = world->party.leader_y +
        dy[(world->party.leader_dir + 1) & 3];
    sample_coords[4][0] = world->party.leader_x +
        2 * dx[world->party.leader_dir];
    sample_coords[4][1] = world->party.leader_y +
        2 * dy[world->party.leader_dir];

    for (i = 0u; i < 5u; ++i) {
        int x = sample_coords[i][0];
        int y = sample_coords[i][1];
        int square = THERON_SQUARE_WALL;
        if (x >= 0 && y >= 0 && x < loaded->width && y < loaded->height) {
            square = loaded->squares[y][x];
        }
        geometry_hash = theron_v1_runtime_mix_hash(
            geometry_hash, (uint32_t)i);
        geometry_hash = theron_v1_runtime_mix_hash(
            geometry_hash, (uint32_t)(x & 0xffff));
        geometry_hash = theron_v1_runtime_mix_hash(
            geometry_hash, (uint32_t)(y & 0xffff));
        geometry_hash = theron_v1_runtime_mix_hash(
            geometry_hash, (uint32_t)(square & 0xff));
        ++receipt.sampled_cell_count;
        if (square == THERON_SQUARE_WALL ||
            square == THERON_SQUARE_SECRET) {
            ++receipt.sampled_wall_count;
        } else if (square == THERON_SQUARE_FLOOR) {
            ++receipt.sampled_floor_count;
        } else {
            ++receipt.sampled_special_count;
        }
    }
    if (geometry_hash == 0u || receipt.sampled_floor_count == 0u) {
        return 0;
    }

    for (int oi = 0; oi < world->object_count; ++oi) {
        const Theron_V1_Object *object = &world->objects[oi];
        if (object->dungeon_id != (int)level->selected_dungeon_index ||
            object->level != target_level) {
            continue;
        }
        if (object->x < 0 || object->y < 0 ||
            object->x >= loaded->width || object->y >= loaded->height) {
            return 0;
        }
        object_hash = theron_v1_runtime_mix_hash(
            object_hash, (uint32_t)object->type);
        object_hash = theron_v1_runtime_mix_hash(
            object_hash, (uint32_t)object->x);
        object_hash = theron_v1_runtime_mix_hash(
            object_hash, (uint32_t)object->y);
        object_hash = theron_v1_runtime_mix_hash(
            object_hash, (uint32_t)object->state);
        object_hash = theron_v1_runtime_mix_hash(
            object_hash, object->flags);
        ++object_count;
    }
    if (object_count != (size_t)transition->target_object_count ||
        object_hash == 0u) {
        return 0;
    }

    route_hash = theron_v1_runtime_mix_hash(
        route_hash, level->source_checksum);
    route_hash = theron_v1_runtime_mix_hash(
        route_hash, level->palette_decoded_checksum);
    route_hash = theron_v1_runtime_mix_hash(route_hash, geometry_hash);
    route_hash = theron_v1_runtime_mix_hash(route_hash, object_hash);
    route_hash = theron_v1_runtime_mix_hash(
        route_hash, (uint32_t)level->clip_x);
    route_hash = theron_v1_runtime_mix_hash(
        route_hash, (uint32_t)level->clip_y);
    route_hash = theron_v1_runtime_mix_hash(
        route_hash, (uint32_t)level->clip_w);
    route_hash = theron_v1_runtime_mix_hash(
        route_hash, (uint32_t)level->clip_h);
    if (route_hash == 0u) {
        return 0;
    }

    receipt.valid = 1;
    receipt.m11_level_consumption_consumed = 1;
    receipt.level_transition_runtime_consumed = 1;
    receipt.world_runtime_geometry_consumed = 1;
    receipt.object_placement_consumed = 1;
    receipt.viewport_composition_route_bound = 1;
    receipt.variant = level->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             level->track02_md5);
    receipt.record = level->record;
    receipt.selected_dungeon_index = level->selected_dungeon_index;
    receipt.target_level_index = level->target_level_index;
    receipt.route_bit = level->route_bit;
    receipt.media_width = level->source_width;
    receipt.media_height = level->source_height;
    receipt.media_checksum = level->source_checksum;
    receipt.palette_decoded_checksum = level->palette_decoded_checksum;
    receipt.level_width = loaded->width;
    receipt.level_height = loaded->height;
    receipt.party_x = world->party.leader_x;
    receipt.party_y = world->party.leader_y;
    receipt.party_dir = world->party.leader_dir;
    receipt.current_square =
        loaded->squares[world->party.leader_y][world->party.leader_x];
    if (sample_coords[1][0] >= 0 && sample_coords[1][1] >= 0 &&
        sample_coords[1][0] < loaded->width &&
        sample_coords[1][1] < loaded->height) {
        receipt.forward_square =
            loaded->squares[sample_coords[1][1]][sample_coords[1][0]];
    } else {
        receipt.forward_square = THERON_SQUARE_WALL;
    }
    receipt.sampled_object_count = object_count;
    receipt.level_geometry_hash = geometry_hash;
    receipt.object_placement_hash = object_hash;
    receipt.viewport_route_hash = route_hash;
    receipt.host_surface_width = level->host_surface_width;
    receipt.host_surface_height = level->host_surface_height;
    receipt.clip_x = level->clip_x;
    receipt.clip_y = level->clip_y;
    receipt.clip_w = level->clip_w;
    receipt.clip_h = level->clip_h;
    receipt.m11_host_presentation_allowed = 1;
    receipt.dungeon_draw_route_allowed = 1;
    receipt.dungeon_pixel_blit_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_bind_track02_level1_draw_blocker(
    const Theron_V1RuntimeTrack02M11LevelConsumptionReceipt *level,
    const Theron_V1RuntimeTrack02LevelTransitionRuntimeReceipt *transition,
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    const Theron_V1_World *world,
    const Theron_V1RuntimeTrack02M11DungeonDrawRouteReceipt *draw_route,
    Theron_V1RuntimeTrack02Level1DrawBlockerReceipt *out) {
    Theron_V1RuntimeTrack02Level1DrawBlockerReceipt receipt = {0};
    int dungeon_slot;
    int level_slot;
    int object_count = 0;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!level || !transition || !gap || !world || !draw_route || !out ||
        !level->valid ||
        !level->decode_vector_consumed ||
        !level->level_transition_runtime_consumed ||
        !level->world_runtime_media_consumed ||
        !level->target_level_selected ||
        !level->exact_indexed_atlas_consumed ||
        !level->huc6260_palette_consumed ||
        level->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(level->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        level->record == 0u ||
        level->selected_dungeon_index == 0u ||
        level->target_level_index != 1u ||
        level->route_bit != THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE ||
        level->source_checksum == 0u ||
        level->palette_decoded_checksum == 0u ||
        !level->host_presentation_allowed ||
        !level->m11_runtime_consumption_allowed ||
        !level->m11_render_allowed ||
        level->dungeon_draw_allowed ||
        level->fallback_visuals_allowed ||
        !transition->valid ||
        !transition->level_transition_handoff_consumed ||
        !transition->target_object_world_handoff_consumed ||
        transition->variant != level->variant ||
        strcmp(transition->track02_md5, level->track02_md5) != 0 ||
        transition->record != level->record ||
        transition->selected_dungeon_index != level->selected_dungeon_index ||
        transition->target_level_index != level->target_level_index ||
        transition->target_level_index == transition->source_level_index ||
        transition->dungeon_runtime_admission_allowed ||
        transition->dungeon_draw_allowed ||
        transition->fallback_visuals_allowed ||
        !gap->valid ||
        !gap->verified_track02_capture_consumed ||
        !gap->fail_closed ||
        gap->variant != level->variant ||
        strcmp(gap->track02_md5, level->track02_md5) != 0 ||
        gap->level_route_hash == 0u ||
        gap->object_table_route_hash == 0u ||
        gap->nonstartup_window_count == 0u ||
        gap->first_nonstartup_raw_offset == 0u ||
        gap->first_nonstartup_user_data_offset == 0u ||
        gap->first_nonstartup_byte_count == 0u ||
        gap->first_nonstartup_raw_hash == 0u ||
        gap->first_container_raw_offset == 0u ||
        gap->first_container_user_data_offset == 0u ||
        gap->first_container_user_data_byte_count == 0u ||
        gap->first_container_user_data_hash == 0u ||
        gap->nonstartup_level_decode_ready ||
        gap->object_table_decode_ready ||
        gap->render_asset_admission_allowed ||
        gap->fallback_visuals_allowed ||
        draw_route->valid ||
        draw_route->dungeon_draw_route_allowed ||
        draw_route->dungeon_pixel_blit_allowed ||
        draw_route->fallback_visuals_allowed) {
        return 0;
    }

    dungeon_slot = (int)level->selected_dungeon_index - 1;
    level_slot = (int)level->target_level_index;
    if (dungeon_slot < 0 || dungeon_slot >= (int)THERON_DUNGEON_COUNT ||
        level_slot < 0 || level_slot >= THERON_MAX_LEVELS_PER_DUNGEON) {
        return 0;
    }
    for (int i = 0; i < world->object_count; ++i) {
        const Theron_V1_Object *object = &world->objects[i];
        if (object->dungeon_id == (int)level->selected_dungeon_index &&
            object->level == level_slot) {
            ++object_count;
        }
    }

    receipt.level1_world_geometry_loaded =
        world->level_loaded[dungeon_slot][level_slot] ? 1 : 0;
    receipt.level1_object_placement_loaded =
        (transition->target_object_count > 0 &&
         transition->target_thing_count == transition->target_object_count &&
         object_count == transition->target_object_count) ? 1 : 0;
    if (receipt.level1_world_geometry_loaded &&
        receipt.level1_object_placement_loaded) {
        return 0;
    }

    receipt.valid = 1;
    receipt.m11_level_consumption_consumed = 1;
    receipt.level_transition_runtime_consumed = 1;
    receipt.original_data_binding_gap_consumed = 1;
    receipt.world_runtime_state_inspected = 1;
    receipt.variant = level->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             level->track02_md5);
    receipt.record = level->record;
    receipt.selected_dungeon_index = level->selected_dungeon_index;
    receipt.target_level_index = level->target_level_index;
    receipt.route_bit = level->route_bit;
    receipt.media_checksum = level->source_checksum;
    receipt.palette_decoded_checksum = level->palette_decoded_checksum;
    receipt.nonstartup_window_count = gap->nonstartup_window_count;
    receipt.first_nonstartup_raw_offset = gap->first_nonstartup_raw_offset;
    receipt.first_nonstartup_user_data_offset =
        gap->first_nonstartup_user_data_offset;
    receipt.first_nonstartup_byte_count = gap->first_nonstartup_byte_count;
    receipt.first_nonstartup_raw_hash = gap->first_nonstartup_raw_hash;
    receipt.object_table_raw_offset = gap->first_container_raw_offset;
    receipt.object_table_user_data_offset =
        gap->first_container_user_data_offset;
    receipt.object_table_byte_count =
        gap->first_container_user_data_byte_count;
    receipt.object_table_raw_hash = gap->first_container_user_data_hash;
    receipt.transition_level_loaded = transition->level_loaded;
    receipt.transition_target_object_count = transition->target_object_count;
    receipt.transition_target_thing_count = transition->target_thing_count;
    receipt.world_object_count = object_count;
    receipt.real_track02_level1_media_bound = 1;
    receipt.nonstartup_geometry_source_blocked = 1;
    receipt.object_placement_source_blocked = 1;
    receipt.loadertrace_geometry_window_missing = 1;
    receipt.loadertrace_object_window_missing = 1;
    receipt.dungeon_draw_route_allowed = 0;
    receipt.dungeon_pixel_blit_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}

int theron_v1_runtime_track02_original_consumer_trace_facts_from_capture(
    const char *capture_trace,
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    uint32_t record,
    uint32_t payload_checksum,
    uint32_t level_envelope_checksum,
    uint32_t post_envelope_checksum,
    Theron_V1Track02Post3800ConsumerTraceFacts *out) {
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (!capture_trace || !gap ||
        !gap->valid ||
        !gap->verified_track02_capture_consumed ||
        !gap->fail_closed ||
        gap->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(gap->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        gap->palette_raw_offset == 0u ||
        gap->first_nonstartup_raw_offset == 0u ||
        gap->first_container_raw_offset == 0u ||
        payload_checksum == 0u ||
        level_envelope_checksum == 0u ||
        post_envelope_checksum == 0u ||
        gap->render_asset_admission_allowed ||
        gap->fallback_visuals_allowed ||
        theron_v1_runtime_trace_has(capture_trace, "synthetic") ||
        theron_v1_runtime_trace_has(capture_trace, "fallback") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_original_consumer_trace") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "authenticated_original_trace=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "post_3800_execution_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_loader_payload=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(capture_trace, "record", record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_record_user_data_offset",
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_destination",
            THERON_V1_INITIAL_ENVELOPE_DESTINATION) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_payload_bytes",
            THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "payload_checksum", payload_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_envelope_checksum",
            level_envelope_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "post_envelope_checksum", post_envelope_checksum) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "palette_raw_offset", gap->palette_raw_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_raw_offset",
            gap->first_nonstartup_raw_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_raw_offset",
            gap->first_container_raw_offset) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "palette_consumer_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_record_consumer_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_consumer_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "bitmap_consumer_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_dungeon_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_object_table_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_bitmap_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_palette_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_observed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0")) {
        return 0;
    }

    out->authenticated_original_trace = 1;
    out->post_3800_execution_observed = 1;
    out->same_capture_as_loader_payload = 1;
    out->track02_variant = gap->variant;
    out->record = record;
    out->loader_record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    out->loader_destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    out->loader_payload_bytes = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    out->payload_checksum = payload_checksum;
    out->level_envelope_checksum = level_envelope_checksum;
    out->post_envelope_checksum = post_envelope_checksum;
    out->consumer_trace_checksum =
        theron_v1_runtime_trace_text_checksum(capture_trace);
    out->dungeon_record_consumer_observed = 1;
    out->object_table_consumer_observed = 1;
    out->bitmap_consumer_observed = 1;
    out->palette_consumer_observed = 1;
    out->synthetic_dungeon_promoted = 0;
    out->synthetic_object_table_promoted = 0;
    out->synthetic_bitmap_promoted = 0;
    out->synthetic_palette_promoted = 0;
    out->fallback_visuals_observed = 0;
    out->fallback_visuals_allowed = 0;
    return 1;
}

int theron_v1_runtime_track02_object_dungeon_trace_facts_from_capture(
    const char *capture_trace,
    const Theron_V1RuntimeTrack02OriginalDataBindingGapReceipt *gap,
    uint32_t record,
    uint32_t payload_checksum,
    uint32_t level_envelope_checksum,
    uint32_t post_envelope_checksum,
    Theron_V1Track02Post3800ConsumerTraceFacts *out) {
    uint32_t dungeon_pc;
    uint32_t object_pc;
    size_t dungeon_offset;
    size_t dungeon_bytes;
    size_t object_offset;
    size_t object_bytes;
    uint32_t dungeon_checksum;
    uint32_t object_checksum;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (!capture_trace || !gap ||
        !gap->valid ||
        !gap->verified_track02_capture_consumed ||
        !gap->fail_closed ||
        gap->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(gap->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        gap->first_nonstartup_raw_offset == 0u ||
        gap->first_container_raw_offset == 0u ||
        payload_checksum == 0u ||
        level_envelope_checksum == 0u ||
        post_envelope_checksum == 0u ||
        gap->nonstartup_level_decode_ready ||
        gap->object_table_decode_ready ||
        gap->render_asset_admission_allowed ||
        gap->fallback_visuals_allowed ||
        !theron_v1_runtime_trace_has(
            capture_trace, "theron_track02_object_dungeon_consumer_trace") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "authenticated_original_trace=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "post_3800_execution_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "same_capture_as_loader_payload=1") ||
        !theron_v1_runtime_trace_has(capture_trace, "track02_variant=us_bin") ||
        !theron_v1_runtime_trace_has_u32(capture_trace, "record", record) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_record_user_data_offset",
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_destination",
            THERON_V1_INITIAL_ENVELOPE_DESTINATION) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "loader_payload_bytes",
            THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "payload_checksum", payload_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "level_envelope_checksum",
            level_envelope_checksum) ||
        !theron_v1_runtime_trace_has_u32(
            capture_trace, "post_envelope_checksum", post_envelope_checksum) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "nonstartup_level_raw_offset",
            gap->first_nonstartup_raw_offset) ||
        !theron_v1_runtime_trace_has_size(
            capture_trace, "object_table_raw_offset",
            gap->first_container_raw_offset) ||
        !theron_v1_runtime_trace_has(
            capture_trace, "dungeon_record_consumer_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "object_table_consumer_observed=1") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "bitmap_consumer_observed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "palette_consumer_observed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_dungeon_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_object_table_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_bitmap_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "synthetic_palette_promoted=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_observed=0") ||
        !theron_v1_runtime_trace_has(
            capture_trace, "fallback_visuals_allowed=0") ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "dungeon_record_consumer_pc", &dungeon_pc) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "object_table_consumer_pc", &object_pc) ||
        !theron_v1_runtime_trace_read_size(
            capture_trace, "dungeon_record_payload_offset",
            &dungeon_offset) ||
        !theron_v1_runtime_trace_read_size(
            capture_trace, "dungeon_record_byte_count", &dungeon_bytes) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "dungeon_record_window_checksum",
            &dungeon_checksum) ||
        !theron_v1_runtime_trace_read_size(
            capture_trace, "object_table_payload_offset", &object_offset) ||
        !theron_v1_runtime_trace_read_size(
            capture_trace, "object_table_byte_count", &object_bytes) ||
        !theron_v1_runtime_trace_read_u32(
            capture_trace, "object_table_window_checksum",
            &object_checksum) ||
        dungeon_pc == 0u ||
        object_pc == 0u ||
        dungeon_offset != THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        dungeon_bytes != THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        dungeon_checksum != level_envelope_checksum ||
        object_offset != THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET ||
        object_bytes != THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES ||
        object_checksum != post_envelope_checksum) {
        return 0;
    }

    out->authenticated_original_trace = 1;
    out->post_3800_execution_observed = 1;
    out->same_capture_as_loader_payload = 1;
    out->track02_variant = gap->variant;
    out->record = record;
    out->loader_record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    out->loader_destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    out->loader_payload_bytes = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    out->payload_checksum = payload_checksum;
    out->level_envelope_checksum = level_envelope_checksum;
    out->post_envelope_checksum = post_envelope_checksum;
    out->consumer_trace_checksum =
        theron_v1_runtime_trace_text_checksum(capture_trace);
    out->dungeon_record_consumer_pc = dungeon_pc;
    out->object_table_consumer_pc = object_pc;
    out->dungeon_record_payload_offset = dungeon_offset;
    out->dungeon_record_byte_count = dungeon_bytes;
    out->dungeon_record_window_checksum = dungeon_checksum;
    out->object_table_payload_offset = object_offset;
    out->object_table_byte_count = object_bytes;
    out->object_table_window_checksum = object_checksum;
    out->dungeon_record_consumer_observed = 1;
    out->object_table_consumer_observed = 1;
    out->bitmap_consumer_observed = 0;
    out->palette_consumer_observed = 0;
    out->synthetic_dungeon_promoted = 0;
    out->synthetic_object_table_promoted = 0;
    out->synthetic_bitmap_promoted = 0;
    out->synthetic_palette_promoted = 0;
    out->fallback_visuals_observed = 0;
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

int theron_v1_runtime_track02_host_dungeon_consumer_proof_from_handoff(
    const Theron_V1RuntimeTrack02DungeonHandoffReceipt *handoff,
    const char *original_host_route_identity,
    int level_grid_runtime_consumer_bound,
    int object_table_runtime_consumer_bound,
    int bitmap_palette_runtime_consumer_bound,
    int host_surface_upload_proven,
    int host_capture_frame_proven,
    Theron_V1RuntimeTrack02HostDungeonConsumerProof *out) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_track02_host_dungeon_consumer_proof_init(out);
    if (!handoff ||
        !original_host_route_identity ||
        strcmp(original_host_route_identity,
               THERON_V1_TRACK02_ORIGINAL_HOST_DUNGEON_ROUTE) != 0 ||
        !handoff->valid ||
        !handoff->render_asset_admission_consumed ||
        !handoff->dungeon_handoff_proof_consumed ||
        !handoff->runtime_capture_required ||
        handoff->variant != THERON_TRACK02_VARIANT_US_BIN ||
        strcmp(handoff->track02_md5, THERON_TRACK02_MD5_US_BIN) != 0 ||
        handoff->record == 0u ||
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
        !level_grid_runtime_consumer_bound ||
        !object_table_runtime_consumer_bound ||
        !bitmap_palette_runtime_consumer_bound ||
        !host_surface_upload_proven ||
        !host_capture_frame_proven) {
        return 0;
    }

    out->valid = 1;
    out->same_capture_as_dungeon_handoff = 1;
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
    out->original_host_route_identity_checksum =
        theron_v1_runtime_track02_host_route_identity_checksum(
            original_host_route_identity);
    if (out->original_host_route_identity_checksum == 0u) {
        theron_v1_runtime_track02_host_dungeon_consumer_proof_init(out);
        return 0;
    }
    out->original_host_route_bound = 1;
    out->level_grid_runtime_consumer_bound = 1;
    out->object_table_runtime_consumer_bound = 1;
    out->bitmap_palette_runtime_consumer_bound = 1;
    out->host_surface_upload_proven = 1;
    out->host_capture_frame_proven = 1;
    out->synthetic_host_frame_promoted = 0;
    out->synthetic_level_grid_promoted = 0;
    out->synthetic_object_table_promoted = 0;
    out->synthetic_bitmap_palette_promoted = 0;
    out->fallback_visuals_observed = 0;
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
        proof->original_host_route_identity_checksum !=
            theron_v1_runtime_track02_host_route_identity_checksum(
                THERON_V1_TRACK02_ORIGINAL_HOST_DUNGEON_ROUTE) ||
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
    out->original_host_route_identity_checksum =
        proof->original_host_route_identity_checksum;
    out->real_track02_dungeon_consumer_ready = 1;
    out->host_surface_upload_allowed = 1;
    out->host_capture_frame_required = 1;
    out->dungeon_draw_allowed = 1;
    out->fallback_visuals_allowed = 0;
    return 1;
}
