#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_bitmap_capture_runtime_admission.h"

#define ROUTES (THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD)

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static void complete_capture(Theron_StartupMediaStateReceipt *capture) {
    memset(capture, 0, sizeof(*capture));
    capture->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(capture->track02_md5, sizeof(capture->track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    capture->startup_media_ready = 1;
    capture->startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    capture->startup_bitmap_sample_count = 48;
    capture->startup_bitmap_route_mask = ROUTES;
    capture->startup_bitmap_nonzero_pixel_count = 1u;
    capture->startup_bitmap_checksum = 1u;
    capture->startup_bitmap_title_route_ready = 1;
    capture->startup_bitmap_stage_route_ready = 1;
    capture->startup_bitmap_soul_room_route_ready = 1;
    capture->startup_bitmap_forcefield_route_ready = 1;
    capture->startup_bitmap_atlas_ready = 1;
    capture->startup_bitmap_atlas_route_count = 4;
    capture->startup_bitmap_atlas_route_mask = ROUTES;
    capture->startup_bitmap_atlas_tile_count = 48u;
    capture->startup_bitmap_atlas_nonzero_pixel_count = 1u;
    capture->startup_bitmap_atlas_checksum = 0x55aa7744u;
    capture->startup_bitmap_wide_route_count = 4;
    capture->startup_bitmap_wide_route_mask = ROUTES;
    capture->startup_bitmap_wide_atlas_tile_count = 48u;
    capture->startup_bitmap_raw_route_mask = ROUTES;
    capture->startup_bitmap_raw_route_count = 4;
    capture->startup_bitmap_raw_atlas_tile_count = 48u;
    capture->startup_bitmap_title_route_ready = 1;
    capture->startup_bitmap_stage_route_ready = 1;
    capture->startup_bitmap_soul_room_route_ready = 1;
    capture->startup_bitmap_forcefield_route_ready = 1;
    capture->startup_bitmap_title_sample_count = 12;
    capture->startup_bitmap_stage_sample_count = 12;
    capture->startup_bitmap_soul_room_sample_count = 12;
    capture->startup_bitmap_forcefield_sample_count = 12;
    capture->startup_bitmap_title_nonzero_pixel_count = 1u;
    capture->startup_bitmap_stage_nonzero_pixel_count = 1u;
    capture->startup_bitmap_soul_room_nonzero_pixel_count = 1u;
    capture->startup_bitmap_forcefield_nonzero_pixel_count = 1u;
    capture->startup_bitmap_title_checksum = 0x11112222u;
    capture->startup_bitmap_stage_checksum = 0x33334444u;
    capture->startup_bitmap_soul_room_checksum = 0x44445555u;
    capture->startup_bitmap_forcefield_checksum = 0x55556666u;
    capture->startup_bitmap_title_atlas_tile_count = 12u;
    capture->startup_bitmap_stage_atlas_tile_count = 12u;
    capture->startup_bitmap_soul_room_atlas_tile_count = 12u;
    capture->startup_bitmap_forcefield_atlas_tile_count = 12u;
    capture->startup_bitmap_title_atlas_width = 96u;
    capture->startup_bitmap_stage_atlas_width = 96u;
    capture->startup_bitmap_soul_room_atlas_width = 96u;
    capture->startup_bitmap_forcefield_atlas_width = 96u;
    capture->startup_bitmap_soul_room_first_raw_offset = 0x00300010u;
    capture->startup_bitmap_soul_room_last_raw_offset = 0x0030002cu;
    capture->startup_bitmap_soul_room_first_user_data_offset = 0x002ff020u;
    capture->runtime_media_identity.ready = 1;
    capture->runtime_media_identity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    capture->runtime_media_identity.bank_stride = 0x0400u;
    capture->runtime_media_identity.checksum = 0x1234fedcu;
}

int main(void) {
    Theron_StartupMediaStateReceipt capture;
    Theron_V1RawLoaderTraceReceipt trace = {0};
    Theron_V1_World world;
    Theron_V1BitmapCaptureRuntimeAdmissionReceipt admission;
    uint8_t pixels[64u * 8u] = {0};

    complete_capture(&capture);
    theron_v1_world_init(&world);
    theron_v1_world_runtime_media_clear(&world);
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_TITLE, capture.track02_md5,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE, 64u, 8u,
        0x00200010u, 0x0020002cu, 0x001ff020u, 8u, 512u,
        capture.startup_bitmap_title_checksum, pixels, sizeof(pixels)));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_STAGE, capture.track02_md5,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE, 64u, 8u,
        0x00280010u, 0x0028002cu, 0x0027f020u, 8u, 512u,
        capture.startup_bitmap_stage_checksum, pixels, sizeof(pixels)));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM, capture.track02_md5,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM, 64u, 8u,
        capture.startup_bitmap_soul_room_first_raw_offset,
        capture.startup_bitmap_soul_room_last_raw_offset,
        capture.startup_bitmap_soul_room_first_user_data_offset, 8u, 512u,
        capture.startup_bitmap_soul_room_checksum, pixels, sizeof(pixels)));
    CHECK(theron_v1_world_runtime_media_set_surface(
        &world, THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD, capture.track02_md5,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD, 64u, 8u,
        0x00380010u, 0x0038002cu, 0x0037f020u, 8u, 512u,
        capture.startup_bitmap_forcefield_checksum, pixels, sizeof(pixels)));
    CHECK(theron_v1_world_runtime_media_set_identity(
        &world, &capture.runtime_media_identity));

    trace.valid = 1;
    trace.variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(trace.track02_md5, sizeof(trace.track02_md5), "%s",
             capture.track02_md5);
    trace.dynamic_cd_read_record = 0x000004e0u;
    trace.dynamic_cd_read_verified = 1;
    trace.dynamic_cd_read_registers_verified = 1;
    trace.dynamic_cd_read_destination_span_verified = 1;
    trace.dynamic_cd_read_media_span_verified = 1;
    trace.stage2_dynamic_payload_verified = 1;
    trace.stage2_dynamic_payload_bytes = THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES;
    trace.soul_room_raw_route_verified = 1;
    trace.soul_room_route_disjoint_from_dynamic_span = 1;
    trace.soul_room_first_raw_offset = capture.startup_bitmap_soul_room_first_raw_offset;
    trace.soul_room_last_raw_offset = capture.startup_bitmap_soul_room_last_raw_offset;
    trace.soul_room_checksum = capture.startup_bitmap_soul_room_checksum;
    trace.bitmap_route_mask = capture.startup_bitmap_raw_route_mask;
    trace.bitmap_atlas_checksum = capture.startup_bitmap_atlas_checksum;

    CHECK(theron_v1_bitmap_capture_admit_soul_room_runtime(
        &capture, &trace, &world, &admission));
    CHECK(admission.valid && admission.source_to_runtime_verified);
    CHECK(admission.route_bit == THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM);
    CHECK(admission.first_raw_offset == capture.startup_bitmap_soul_room_first_raw_offset);
    CHECK(!admission.palette_descriptor_relation_verified);
    CHECK(!admission.pixel_decode_verified && !admission.render_allowed);
    CHECK(!admission.dungeon_draw_allowed && !admission.fallback_visuals_allowed);

    trace.soul_room_checksum ^= 1u;
    CHECK(!theron_v1_bitmap_capture_admit_soul_room_runtime(
        &capture, &trace, &world, &admission));
    trace.soul_room_checksum ^= 1u;
    trace.soul_room_route_disjoint_from_dynamic_span = 0;
    CHECK(!theron_v1_bitmap_capture_admit_soul_room_runtime(
        &capture, &trace, &world, &admission));
    trace.soul_room_route_disjoint_from_dynamic_span = 1;
    strcpy(world.runtime_media.soul_room.track02_md5, THERON_TRACK02_MD5_JP_BIN);
    CHECK(!theron_v1_bitmap_capture_admit_soul_room_runtime(
        &capture, &trace, &world, &admission));

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
