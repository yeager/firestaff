#include <stdio.h>
#include <string.h>

#include "theron_v1_bitmap_capture_runtime_admission.h"

static int theron_v1_bitmap_capture_identity_matches(
    const Theron_RuntimeMediaIdentity *left,
    const Theron_RuntimeMediaIdentity *right) {
    return left && right && left->ready && right->ready &&
           left->track02_variant == right->track02_variant &&
           left->bank_anchor_index == right->bank_anchor_index &&
           left->bank_descriptor_offset == right->bank_descriptor_offset &&
           left->bank_first_value == right->bank_first_value &&
           left->bank_last_value == right->bank_last_value &&
           left->bank_stride == right->bank_stride &&
           left->audio_frame_ready == right->audio_frame_ready &&
           left->audio_bank_id == right->audio_bank_id &&
           left->audio_bank_id_offset == right->audio_bank_id_offset &&
           left->audio_bank_prefix_offset == right->audio_bank_prefix_offset &&
           left->checksum == right->checksum;
}

static int theron_v1_bitmap_capture_world_has_one_source(
    const Theron_V1_World *world,
    const char *track02_md5) {
    const Theron_RuntimeMediaSurface *const surfaces[] = {
        &world->runtime_media.title,
        &world->runtime_media.stage,
        &world->runtime_media.soul_room,
        &world->runtime_media.forcefield
    };
    size_t i;

    for (i = 0u; i < sizeof(surfaces) / sizeof(surfaces[0]); ++i) {
        if (!surfaces[i]->ready || !surfaces[i]->raw_source_verified ||
            strcmp(surfaces[i]->track02_md5, track02_md5) != 0) {
            return 0;
        }
    }
    return 1;
}

int theron_v1_bitmap_capture_admit_soul_room_runtime(
    const Theron_StartupMediaStateReceipt *media_capture,
    const Theron_V1RawLoaderTraceReceipt *loader_trace,
    const Theron_V1_World *world,
    Theron_V1BitmapCaptureRuntimeAdmissionReceipt *out) {
    Theron_V1BitmapCaptureRuntimeAdmissionReceipt receipt = {0};
    const Theron_RuntimeMediaSurface *surface;
    Theron_Track02Variant variant;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!media_capture || !loader_trace || !world || !out ||
        !theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            media_capture) ||
        !(variant = theron_v1_track02_variant_for_md5(
              media_capture->track02_md5)) ||
        media_capture->track02_variant != (int)variant ||
        !loader_trace->valid ||
        loader_trace->variant != variant ||
        strcmp(loader_trace->track02_md5, media_capture->track02_md5) != 0 ||
        !loader_trace->dynamic_cd_read_verified ||
        !loader_trace->dynamic_cd_read_registers_verified ||
        !loader_trace->dynamic_cd_read_destination_span_verified ||
        !loader_trace->dynamic_cd_read_media_span_verified ||
        !loader_trace->stage2_dynamic_payload_verified ||
        loader_trace->stage2_dynamic_payload_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        loader_trace->dynamic_cd_read_record == 0u ||
        !loader_trace->soul_room_raw_route_verified ||
        !loader_trace->soul_room_route_disjoint_from_dynamic_span ||
        loader_trace->soul_room_first_raw_offset !=
            media_capture->startup_bitmap_soul_room_first_raw_offset ||
        loader_trace->soul_room_last_raw_offset !=
            media_capture->startup_bitmap_soul_room_last_raw_offset ||
        loader_trace->soul_room_checksum !=
            media_capture->startup_bitmap_soul_room_checksum ||
        loader_trace->bitmap_route_mask !=
            media_capture->startup_bitmap_raw_route_mask ||
        loader_trace->bitmap_atlas_checksum !=
            media_capture->startup_bitmap_atlas_checksum ||
        !world->runtime_media.restored ||
        !theron_v1_bitmap_capture_identity_matches(
            &media_capture->runtime_media_identity,
            &world->runtime_media.identity) ||
        !theron_v1_bitmap_capture_world_has_one_source(
            world, media_capture->track02_md5)) {
        return 0;
    }

    surface = &world->runtime_media.soul_room;
    if (surface->route_bit != THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM ||
        surface->first_raw_offset != loader_trace->soul_room_first_raw_offset ||
        surface->last_raw_offset != loader_trace->soul_room_last_raw_offset ||
        surface->first_user_data_offset !=
            media_capture->startup_bitmap_soul_room_first_user_data_offset ||
        surface->checksum != loader_trace->soul_room_checksum) {
        return 0;
    }

    receipt.valid = 1;
    receipt.startup_media_capture_consumed = 1;
    receipt.raw_loader_trace_consumed = 1;
    receipt.runtime_surface_consumed = 1;
    receipt.source_to_runtime_verified = 1;
    receipt.track02_variant = variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             media_capture->track02_md5);
    receipt.route_bit = surface->route_bit;
    receipt.first_raw_offset = surface->first_raw_offset;
    receipt.last_raw_offset = surface->last_raw_offset;
    receipt.first_user_data_offset = surface->first_user_data_offset;
    receipt.bitmap_checksum = surface->checksum;
    receipt.bitmap_atlas_checksum = loader_trace->bitmap_atlas_checksum;
    receipt.dynamic_cd_read_record = loader_trace->dynamic_cd_read_record;
    receipt.palette_descriptor_relation_verified = 0;
    receipt.pixel_decode_verified = 0;
    receipt.render_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}
