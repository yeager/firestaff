#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_capture_target_plan.h"

static void fixture(Theron_V1Track02RawMediaIntakeReceipt *media,
                    Theron_V1RawLoaderTraceReceipt *loader,
                    Theron_V1Track02PaletteRouteReceipt *palette,
                    Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap,
                    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *destination)
{
    memset(media, 0, sizeof(*media)); memset(loader, 0, sizeof(*loader));
    memset(palette, 0, sizeof(*palette)); memset(bitmap, 0, sizeof(*bitmap)); memset(destination, 0, sizeof(*destination));
    media->status = THERON_V1_TRACK02_MEDIA_INTAKE_READY; media->cue_consumed = 1; media->mode1_2352 = 1; media->raw_trace_preparation_allowed = 1;
    media->variant = THERON_TRACK02_VARIANT_US_BIN; strcpy(media->track02_md5, THERON_TRACK02_MD5_US_BIN);
    loader->valid = 1; loader->variant = media->variant; strcpy(loader->track02_md5, media->track02_md5);
    loader->dynamic_cd_read_record = 0x4e0u; loader->dynamic_cd_read_raw_offset = 2352u; loader->dynamic_cd_read_destination_span_bytes = 32u;
    loader->dynamic_cd_read_destination_span_checksum = 0x12345678u; loader->dynamic_cd_read_verified = 1; loader->dynamic_cd_read_registers_verified = 1;
    loader->dynamic_cd_read_destination_span_verified = 1; loader->dynamic_cd_read_media_span_verified = 1; loader->stage2_dynamic_payload_verified = 1;
    palette->accepted = 1; palette->real_cd_verified = 1; palette->track02_variant = media->variant; strcpy(palette->track02_md5, media->track02_md5);
    palette->vce_index_address = 0x402u; palette->vce_low_address = 0x403u; palette->vce_high_address = 0x404u;
    palette->vce_index = 1u; palette->vce_low = 2u; palette->vce_high = 3u;
    bitmap->valid = 1; bitmap->source_to_runtime_verified = 1; bitmap->track02_variant = media->variant; strcpy(bitmap->track02_md5, media->track02_md5);
    bitmap->dynamic_cd_read_record = loader->dynamic_cd_read_record; bitmap->first_raw_offset = 8192u; bitmap->last_raw_offset = 8447u;
    bitmap->bitmap_checksum = 0xa1b2c3d4u;
    destination->valid = 1; destination->opaque_route_ready = 1; destination->track02_variant = media->variant; strcpy(destination->track02_md5, media->track02_md5);
    destination->loader_record = 0xb52u; destination->dungeon_record_payload_offset = 0x2000u;
    destination->dungeon_record_byte_count = 128u; destination->dungeon_record_window_checksum = 0xcafef00du;
}

int main(void)
{
    Theron_V1Track02RawMediaIntakeReceipt media; Theron_V1RawLoaderTraceReceipt loader;
    Theron_V1Track02PaletteRouteReceipt palette; Theron_V1BitmapCaptureRuntimeAdmissionReceipt bitmap;
    Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt destination; Theron_V1Track02CaptureTargetPlan plan;
    fixture(&media, &loader, &palette, &bitmap, &destination);
    if (!theron_v1_track02_capture_target_plan_build(&media, &loader, &palette, &bitmap, &destination, &plan) ||
        !plan.valid || plan.targets[0].route != THERON_V1_TRACK02_CAPTURE_TARGET_START ||
        plan.targets[1].bitmap_identity != bitmap.bitmap_checksum ||
        plan.targets[2].destination_record != destination.loader_record ||
        plan.targets[2].destination_bytes != destination.dungeon_record_byte_count ||
        plan.render_allowed || plan.pixel_decode_allowed || plan.level_object_semantics_allowed || plan.fallback_visuals_allowed) return 1;
    bitmap.track02_md5[0] = '0';
    if (theron_v1_track02_capture_target_plan_build(&media, &loader, &palette, &bitmap, &destination, &plan)) return 2;
    fixture(&media, &loader, &palette, &bitmap, &destination);
    palette.render_allowed = 1;
    if (theron_v1_track02_capture_target_plan_build(&media, &loader, &palette, &bitmap, &destination, &plan)) return 3;
    fixture(&media, &loader, &palette, &bitmap, &destination);
    destination.dungeon_record_byte_count = 0u;
    if (theron_v1_track02_capture_target_plan_build(&media, &loader, &palette, &bitmap, &destination, &plan)) return 4;
    puts("test_theron_v1_track02_capture_target_plan: PASS");
    return 0;
}
