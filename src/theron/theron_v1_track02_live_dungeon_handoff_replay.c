#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_live_dungeon_handoff_replay.h"

static uint32_t palette_identity(const Theron_V1Track02PaletteRouteReceipt *palette)
{
    uint32_t identity = 2166136261u;
    const uint16_t addresses[] = {
        palette->vce_index_address, palette->vce_low_address,
        palette->vce_high_address
    };
    const uint8_t values[] = {
        palette->vce_index, palette->vce_low, palette->vce_high
    };
    size_t i;

    for (i = 0u; i < sizeof(addresses) / sizeof(addresses[0]); ++i) {
        identity = (identity ^ (uint8_t)addresses[i]) * 16777619u;
        identity = (identity ^ (uint8_t)(addresses[i] >> 8)) * 16777619u;
        identity = (identity ^ values[i]) * 16777619u;
    }
    return identity;
}

int theron_v1_track02_live_dungeon_handoff_replay_validate(
    const Theron_V1Track02RawMediaIntakeReceipt *media,
    const Theron_V1RawLoaderTraceReceipt *loader,
    const Theron_V1Track02PaletteRouteReceipt *palette,
    const Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *destination,
    const Theron_V1Track02LiveDungeonHandoffReplayEvent *events,
    size_t event_count,
    Theron_V1Track02LiveDungeonHandoffReplayReceipt *out)
{
    Theron_V1Track02LiveDungeonHandoffReplayReceipt receipt = {0};
    size_t i;

    if (!out) return 0;
    *out = receipt;
    if (!media || !loader || !palette || !bitmap || !destination || !events ||
        event_count != THERON_V1_TRACK02_LIVE_REPLAY_EVENT_COUNT ||
        media->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
        !media->cue_consumed || !media->mode1_2352 || !media->raw_trace_preparation_allowed ||
        media->variant == THERON_TRACK02_VARIANT_UNKNOWN || !loader->valid ||
        !loader->dynamic_cd_read_verified || !loader->dynamic_cd_read_registers_verified ||
        !loader->dynamic_cd_read_destination_span_verified ||
        !loader->dynamic_cd_read_media_span_verified || !loader->stage2_dynamic_payload_verified ||
        !palette->accepted || !palette->real_cd_verified || palette->render_allowed ||
        !bitmap->valid || !bitmap->source_to_runtime_verified ||
        bitmap->pixel_decode_verified || bitmap->render_allowed || bitmap->dungeon_draw_allowed ||
        bitmap->fallback_visuals_allowed || bitmap->first_raw_offset > bitmap->last_raw_offset ||
        !destination->valid ||
        !destination->opaque_route_ready || destination->level_field_decoder_allowed ||
        destination->object_field_decoder_allowed || destination->bitmap_palette_admission_allowed ||
        destination->pixel_decode_allowed || destination->dungeon_draw_allowed ||
        destination->fallback_visuals_allowed ||
        media->variant != loader->variant || media->variant != palette->track02_variant ||
        media->variant != bitmap->track02_variant || media->variant != destination->track02_variant ||
        strcmp(media->track02_md5, loader->track02_md5) ||
        strcmp(media->track02_md5, palette->track02_md5) ||
        strcmp(media->track02_md5, bitmap->track02_md5) ||
        strcmp(media->track02_md5, destination->track02_md5) ||
        loader->dynamic_cd_read_record != bitmap->dynamic_cd_read_record) return 0;

    for (i = 0u; i < event_count; ++i) {
        if (events[i].sequence != i + 1u ||
            events[i].kind !=
                (Theron_V1Track02LiveDungeonHandoffReplayEventKind)(i + 1u)) {
            return 0;
        }
    }
    if (events[0].primary_identity != media->cue_index01_sector ||
        events[0].secondary_identity != (uint32_t)media->first_user_data_offset ||
        events[0].payload_offset != media->first_user_data_offset ||
        events[0].payload_bytes != media->logical_user_data_window_bytes ||
        events[1].primary_identity != loader->dynamic_cd_read_record ||
        events[1].secondary_identity != loader->dynamic_cd_read_destination_span_checksum ||
        events[1].payload_offset != loader->dynamic_cd_read_raw_offset ||
        events[1].payload_bytes != loader->dynamic_cd_read_destination_span_bytes ||
        events[2].primary_identity != loader->stage2_dynamic_payload_checksum ||
        events[2].secondary_identity != loader->dynamic_cd_read_record ||
        events[2].payload_offset != loader->dynamic_cd_read_user_data_offset ||
        events[2].payload_bytes != loader->stage2_dynamic_payload_bytes ||
        events[3].primary_identity != palette_identity(palette) ||
        events[3].secondary_identity != palette->vce_index ||
        events[3].payload_offset != 0u || events[3].payload_bytes != 0u ||
        events[4].primary_identity != bitmap->bitmap_checksum ||
        events[4].secondary_identity != bitmap->bitmap_atlas_checksum ||
        events[4].payload_offset != bitmap->first_user_data_offset ||
        events[4].payload_bytes != bitmap->last_raw_offset - bitmap->first_raw_offset + 1u ||
        events[5].primary_identity != destination->loader_record ||
        events[5].secondary_identity != destination->dungeon_record_window_checksum ||
        events[5].payload_offset != destination->dungeon_record_payload_offset ||
        events[5].payload_bytes != destination->dungeon_record_byte_count) return 0;

    receipt.valid = 1;
    receipt.raw_media_consumed = 1;
    receipt.dynamic_cd_read_consumed = 1;
    receipt.loader_chain_consumed = 1;
    receipt.palette_output_consumed = 1;
    receipt.bitmap_output_consumed = 1;
    receipt.destination_record_consumed = 1;
    receipt.track02_variant = media->variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", media->track02_md5);
    receipt.dynamic_cd_read_record = loader->dynamic_cd_read_record;
    receipt.destination_loader_record = destination->loader_record;
    receipt.destination_payload_offset = destination->dungeon_record_payload_offset;
    receipt.destination_payload_bytes = destination->dungeon_record_byte_count;
    receipt.destination_window_checksum = destination->dungeon_record_window_checksum;
    *out = receipt;
    return 1;
}
