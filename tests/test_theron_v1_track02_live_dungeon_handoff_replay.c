#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_live_dungeon_handoff_replay.h"

static uint32_t palette_identity(const Theron_V1Track02PaletteRouteReceipt *palette)
{
    uint32_t identity = 2166136261u;
    const uint16_t addresses[] = {palette->vce_index_address, palette->vce_low_address, palette->vce_high_address};
    const uint8_t values[] = {palette->vce_index, palette->vce_low, palette->vce_high};
    size_t i;
    for (i = 0u; i < 3u; ++i) {
        identity = (identity ^ (uint8_t)addresses[i]) * 16777619u;
        identity = (identity ^ (uint8_t)(addresses[i] >> 8)) * 16777619u;
        identity = (identity ^ values[i]) * 16777619u;
    }
    return identity;
}

static void fixtures(Theron_V1Track02RawMediaIntakeReceipt *media,
                     Theron_V1RawLoaderTraceReceipt *loader,
                     Theron_V1Track02PaletteRouteReceipt *palette,
                     Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap,
                     Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *destination,
                     Theron_V1Track02LiveDungeonHandoffReplayEvent *events)
{
    memset(media, 0, sizeof(*media)); memset(loader, 0, sizeof(*loader));
    memset(palette, 0, sizeof(*palette)); memset(bitmap, 0, sizeof(*bitmap));
    memset(destination, 0, sizeof(*destination)); memset(events, 0, sizeof(*events) * 6u);
    media->status = THERON_V1_TRACK02_MEDIA_INTAKE_READY; media->cue_consumed = 1;
    media->mode1_2352 = 1; media->raw_trace_preparation_allowed = 1;
    media->variant = THERON_TRACK02_VARIANT_US_BIN; strcpy(media->track02_md5, THERON_TRACK02_MD5_US_BIN);
    media->cue_index01_sector = 150u; media->first_user_data_offset = 16u; media->logical_user_data_window_bytes = 2048u;
    loader->valid = 1; loader->variant = media->variant; strcpy(loader->track02_md5, media->track02_md5);
    loader->dynamic_cd_read_record = 0x4e0u; loader->dynamic_cd_read_raw_offset = 2352u;
    loader->dynamic_cd_read_user_data_offset = 2368u; loader->dynamic_cd_read_destination_span_bytes = 32u;
    loader->dynamic_cd_read_destination_span_checksum = 0x12345678u; loader->dynamic_cd_read_verified = 1;
    loader->dynamic_cd_read_registers_verified = 1; loader->dynamic_cd_read_destination_span_verified = 1;
    loader->dynamic_cd_read_media_span_verified = 1; loader->stage2_dynamic_payload_verified = 1;
    loader->stage2_dynamic_payload_bytes = 2048u; loader->stage2_dynamic_payload_checksum = 0x55aa55aau;
    palette->accepted = 1; palette->real_cd_verified = 1; palette->track02_variant = media->variant;
    strcpy(palette->track02_md5, media->track02_md5); palette->vce_index_address = 0x402u;
    palette->vce_low_address = 0x403u; palette->vce_high_address = 0x404u;
    palette->vce_index = 1u; palette->vce_low = 2u; palette->vce_high = 3u;
    bitmap->valid = 1; bitmap->source_to_runtime_verified = 1; bitmap->track02_variant = media->variant;
    strcpy(bitmap->track02_md5, media->track02_md5); bitmap->dynamic_cd_read_record = loader->dynamic_cd_read_record;
    bitmap->first_raw_offset = 8192u; bitmap->last_raw_offset = 8447u; bitmap->first_user_data_offset = 8208u;
    bitmap->bitmap_checksum = 0x99aabbccu; bitmap->bitmap_atlas_checksum = 0xddeeff11u;
    destination->valid = 1; destination->opaque_route_ready = 1; destination->track02_variant = media->variant;
    strcpy(destination->track02_md5, media->track02_md5); destination->loader_record = 0xb52u;
    destination->dungeon_record_payload_offset = 0x2000u; destination->dungeon_record_byte_count = 128u;
    destination->dungeon_record_window_checksum = 0xcafef00du;
    events[0] = (Theron_V1Track02LiveDungeonHandoffReplayEvent){THERON_V1_TRACK02_LIVE_REPLAY_MEDIA, 1u, 150u, 16u, 16u, 2048u};
    events[1] = (Theron_V1Track02LiveDungeonHandoffReplayEvent){THERON_V1_TRACK02_LIVE_REPLAY_DYNAMIC_CD_READ, 2u, 0x4e0u, 0x12345678u, 2352u, 32u};
    events[2] = (Theron_V1Track02LiveDungeonHandoffReplayEvent){THERON_V1_TRACK02_LIVE_REPLAY_LOADER_CHAIN, 3u, 0x55aa55aau, 0x4e0u, 2368u, 2048u};
    events[3] = (Theron_V1Track02LiveDungeonHandoffReplayEvent){THERON_V1_TRACK02_LIVE_REPLAY_PALETTE_OUTPUT, 4u, palette_identity(palette), 1u, 0u, 0u};
    events[4] = (Theron_V1Track02LiveDungeonHandoffReplayEvent){THERON_V1_TRACK02_LIVE_REPLAY_BITMAP_OUTPUT, 5u, 0x99aabbccu, 0xddeeff11u, 8208u, 256u};
    events[5] = (Theron_V1Track02LiveDungeonHandoffReplayEvent){THERON_V1_TRACK02_LIVE_REPLAY_DESTINATION_RECORD, 6u, 0xb52u, 0xcafef00du, 0x2000u, 128u};
}

int main(void)
{
    Theron_V1Track02RawMediaIntakeReceipt media; Theron_V1RawLoaderTraceReceipt loader;
    Theron_V1Track02PaletteRouteReceipt palette; Theron_V1BitmapCaptureRuntimeAdmissionReceipt bitmap;
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt destination;
    Theron_V1Track02LiveDungeonHandoffReplayEvent events[6];
    Theron_V1Track02LiveDungeonHandoffReplayReceipt replay;
    fixtures(&media, &loader, &palette, &bitmap, &destination, events);
    if (!theron_v1_track02_live_dungeon_handoff_replay_validate(&media, &loader, &palette, &bitmap, &destination, events, 6u, &replay) ||
        !replay.valid || !replay.destination_record_consumed || replay.level_object_semantics_allowed ||
        replay.pixel_decode_allowed || replay.render_allowed || replay.fallback_visuals_allowed) return 1;
    events[3].sequence = 5u;
    if (theron_v1_track02_live_dungeon_handoff_replay_validate(&media, &loader, &palette, &bitmap, &destination, events, 6u, &replay)) return 2;
    fixtures(&media, &loader, &palette, &bitmap, &destination, events);
    events[4].primary_identity++;
    if (theron_v1_track02_live_dungeon_handoff_replay_validate(&media, &loader, &palette, &bitmap, &destination, events, 6u, &replay)) return 3;
    if (theron_v1_track02_live_dungeon_handoff_replay_validate(&media, &loader, &palette, &bitmap, &destination, events, 5u, &replay)) return 4;
    puts("test_theron_v1_track02_live_dungeon_handoff_replay: PASS");
    return 0;
}
