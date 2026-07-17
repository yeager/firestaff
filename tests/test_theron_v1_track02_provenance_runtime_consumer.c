#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02_provenance_runtime_consumer.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static int set_surface(Theron_V1_World *world,
                       Theron_RuntimeMediaSurfaceKind kind,
                       unsigned int route_bit,
                       size_t first_raw_offset,
                       size_t first_user_data_offset,
                       uint32_t checksum,
                       const uint8_t *pixels) {
    return theron_v1_world_runtime_media_set_surface(
        world, kind, THERON_TRACK02_MD5_US_BIN, route_bit, 64u, 8u,
        first_raw_offset, first_raw_offset + 0x1cu, first_user_data_offset,
        8u, 512u, checksum, pixels, 64u * 8u);
}

int main(void) {
    Theron_V1_World world;
    Theron_V1BitmapCaptureRuntimeAdmissionReceipt bitmap_capture = {0};
    Theron_V1Track02ProvenanceRuntimeConsumerReceipt consumer;
    Theron_RuntimeMediaIdentity identity = {0};
    uint8_t pixels[64u * 8u] = {0};

    theron_v1_world_init(&world);
    theron_v1_world_runtime_media_clear(&world);
    CHECK(set_surface(&world, THERON_RUNTIME_MEDIA_SURFACE_TITLE,
                      THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
                      0x00200010u, 0x001ff020u, 0x11112222u, pixels));
    CHECK(set_surface(&world, THERON_RUNTIME_MEDIA_SURFACE_STAGE,
                      THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
                      0x00280010u, 0x0027f020u, 0x33334444u, pixels));
    CHECK(set_surface(&world, THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM,
                      THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
                      0x00300010u, 0x002ff020u, 0x44445555u, pixels));
    CHECK(set_surface(&world, THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD,
                      THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD,
                      0x00380010u, 0x0037f020u, 0x55556666u, pixels));
    identity.ready = 1;
    identity.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    identity.bank_stride = 0x0400u;
    identity.checksum = 0x1234fedcu;
    CHECK(theron_v1_world_runtime_media_set_identity(&world, &identity));
    CHECK(!theron_v1_world_runtime_media_set_loader_record(
        &world, "00000000000000000000000000000000", 0x00000b52u, 0x3800u,
        0x00680170u, 2048u, 0x7b0f13c9u, 0x00000020u, 96u,
        0x3a5d7811u, 0x00000080u, 256u, 0x55aa7744u));
    CHECK(!theron_v1_world_runtime_media_set_loader_record(
        &world, THERON_TRACK02_MD5_US_BIN, 0x00000b52u, 0x3800u,
        0x00680171u, 2048u, 0x7b0f13c9u, 0x00000020u, 96u,
        0x3a5d7811u, 0x00000080u, 256u, 0x55aa7744u));
    CHECK(theron_v1_world_runtime_media_set_loader_record(
        &world, THERON_TRACK02_MD5_US_BIN, 0x00000b52u, 0x3800u,
        0x00680170u, 2048u, 0x7b0f13c9u, 0x00000020u, 96u,
        0x3a5d7811u, 0x00000080u, 256u, 0x55aa7744u));

    bitmap_capture.valid = 1;
    bitmap_capture.startup_media_capture_consumed = 1;
    bitmap_capture.raw_loader_trace_consumed = 1;
    bitmap_capture.runtime_surface_consumed = 1;
    bitmap_capture.source_to_runtime_verified = 1;
    bitmap_capture.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(bitmap_capture.track02_md5, sizeof(bitmap_capture.track02_md5),
             "%s", THERON_TRACK02_MD5_US_BIN);
    bitmap_capture.route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
    bitmap_capture.first_raw_offset = 0x00300010u;
    bitmap_capture.last_raw_offset = 0x0030002cu;
    bitmap_capture.first_user_data_offset = 0x002ff020u;
    bitmap_capture.bitmap_checksum = 0x44445555u;

    CHECK(theron_v1_track02_provenance_runtime_consume(
        &bitmap_capture, &world, &consumer));
    CHECK(consumer.valid && consumer.bitmap_capture_runtime_consumed);
    CHECK(consumer.loader_record_runtime_consumed);
    CHECK(consumer.same_track02_source_verified);
    CHECK(consumer.original_level_object_consumer_trace_required);
    CHECK(consumer.loader_record == 0x00000b52u);
    CHECK(!consumer.level_admission_allowed && !consumer.object_admission_allowed);
    CHECK(!consumer.pixel_decode_allowed && !consumer.dungeon_draw_allowed);
    CHECK(!consumer.fallback_visuals_allowed);

    world.runtime_media.loader_record.no_semantic_promotion = 0;
    CHECK(!theron_v1_track02_provenance_runtime_consume(
        &bitmap_capture, &world, &consumer));
    world.runtime_media.loader_record.no_semantic_promotion = 1;
    bitmap_capture.pixel_decode_verified = 1;
    CHECK(!theron_v1_track02_provenance_runtime_consume(
        &bitmap_capture, &world, &consumer));
    bitmap_capture.pixel_decode_verified = 0;
    strcpy(world.runtime_media.loader_record.track02_md5,
           THERON_TRACK02_MD5_JP_BIN);
    CHECK(!theron_v1_track02_provenance_runtime_consume(
        &bitmap_capture, &world, &consumer));

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
