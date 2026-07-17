#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_provenance_runtime_consumer.h"

int theron_v1_track02_provenance_runtime_consume(
    const Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap_capture,
    const Theron_V1_World *world,
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt *out) {
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt receipt = {0};
    const Theron_RuntimeTrack02LoaderRecord *loader_record;
    const Theron_RuntimeMediaSurface *soul_room;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!bitmap_capture || !world || !out || !bitmap_capture->valid ||
        !bitmap_capture->startup_media_capture_consumed ||
        !bitmap_capture->raw_loader_trace_consumed ||
        !bitmap_capture->runtime_surface_consumed ||
        !bitmap_capture->source_to_runtime_verified ||
        bitmap_capture->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        theron_v1_track02_variant_for_md5(bitmap_capture->track02_md5) !=
            bitmap_capture->track02_variant ||
        bitmap_capture->route_bit !=
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM ||
        bitmap_capture->first_raw_offset == 0u ||
        bitmap_capture->last_raw_offset < bitmap_capture->first_raw_offset ||
        bitmap_capture->first_user_data_offset == 0u ||
        bitmap_capture->bitmap_checksum == 0u ||
        bitmap_capture->palette_descriptor_relation_verified ||
        bitmap_capture->pixel_decode_verified ||
        bitmap_capture->render_allowed ||
        bitmap_capture->dungeon_draw_allowed ||
        bitmap_capture->fallback_visuals_allowed ||
        !world->runtime_media.restored ||
        !world->runtime_media.identity.ready ||
        world->runtime_media.identity.track02_variant !=
            (int)bitmap_capture->track02_variant) {
        return 0;
    }

    soul_room = &world->runtime_media.soul_room;
    loader_record = &world->runtime_media.loader_record;
    if (!soul_room->ready || !soul_room->raw_source_verified ||
        strcmp(soul_room->track02_md5, bitmap_capture->track02_md5) != 0 ||
        soul_room->route_bit != bitmap_capture->route_bit ||
        soul_room->first_raw_offset != bitmap_capture->first_raw_offset ||
        soul_room->last_raw_offset != bitmap_capture->last_raw_offset ||
        soul_room->first_user_data_offset !=
            bitmap_capture->first_user_data_offset ||
        soul_room->checksum != bitmap_capture->bitmap_checksum ||
        !loader_record->ready || !loader_record->raw_source_verified ||
        !loader_record->no_semantic_promotion ||
        strcmp(loader_record->track02_md5, bitmap_capture->track02_md5) != 0 ||
        loader_record->record == 0u || loader_record->destination == 0u ||
        loader_record->raw_user_data_offset == 0u ||
        loader_record->payload_bytes == 0u ||
        loader_record->payload_checksum == 0u ||
        !loader_record->level_envelope_bound ||
        loader_record->level_envelope_offset == 0u ||
        loader_record->level_envelope_bytes == 0u ||
        loader_record->level_envelope_checksum == 0u ||
        loader_record->post_envelope_offset == 0u ||
        loader_record->post_envelope_bytes == 0u ||
        loader_record->post_envelope_checksum == 0u) {
        return 0;
    }

    receipt.valid = 1;
    receipt.bitmap_capture_runtime_consumed = 1;
    receipt.loader_record_runtime_consumed = 1;
    receipt.same_track02_source_verified = 1;
    receipt.original_level_object_consumer_trace_required = 1;
    receipt.track02_variant = bitmap_capture->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             bitmap_capture->track02_md5);
    receipt.loader_record = loader_record->record;
    receipt.loader_destination = loader_record->destination;
    receipt.loader_raw_user_data_offset = loader_record->raw_user_data_offset;
    receipt.loader_payload_bytes = loader_record->payload_bytes;
    receipt.loader_payload_checksum = loader_record->payload_checksum;
    receipt.level_envelope_offset = loader_record->level_envelope_offset;
    receipt.level_envelope_bytes = loader_record->level_envelope_bytes;
    receipt.level_envelope_checksum = loader_record->level_envelope_checksum;
    receipt.post_envelope_offset = loader_record->post_envelope_offset;
    receipt.post_envelope_bytes = loader_record->post_envelope_bytes;
    receipt.post_envelope_checksum = loader_record->post_envelope_checksum;
    receipt.bitmap_route_bit = bitmap_capture->route_bit;
    receipt.bitmap_checksum = bitmap_capture->bitmap_checksum;
    receipt.level_admission_allowed = 0;
    receipt.object_admission_allowed = 0;
    receipt.pixel_decode_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}
