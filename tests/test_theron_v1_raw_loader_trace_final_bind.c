#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <string.h>

#define ROUTES (THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD)

static void complete_trace(Theron_V1RawLoaderTraceReceipt *trace)
{
    memset(trace, 0, sizeof(*trace));
    trace->valid = 1;
    strcpy(trace->track02_md5, THERON_TRACK02_MD5_US_BIN);
    trace->variant = THERON_TRACK02_VARIANT_US_BIN;
    trace->dynamic_cd_read_raw_offset = 2352u;
    trace->dynamic_cd_read_destination_span_bytes = 32u;
    trace->dynamic_cd_read_destination_span_checksum = 0x12345678u;
    trace->dynamic_cd_read_verified = 1;
    trace->dynamic_cd_read_registers_verified = 1;
    trace->dynamic_cd_read_destination_span_verified = 1;
    trace->dynamic_cd_read_media_span_verified = 1;
    trace->stage2_dynamic_payload_verified = 1;
    trace->stage2_dynamic_payload_bytes =
        THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES;
    trace->stage2_dynamic_payload_checksum = 0x87654321u;
    trace->palette_store_observed_after_dynamic_read = 1;
}

static void complete_media(Theron_StartupMediaStateReceipt *media)
{
    memset(media, 0, sizeof(*media));
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(media->track02_md5, THERON_TRACK02_MD5_US_BIN);
    media->track02_size = 2352u * 8u;
    media->startup_media_ready = 1;
    media->startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media->startup_bitmap_sample_count = 48;
    media->startup_bitmap_route_mask = ROUTES;
    media->startup_bitmap_nonzero_pixel_count = 384u;
    media->startup_bitmap_checksum = 0x11111111u;
    media->startup_bitmap_title_route_ready = 1;
    media->startup_bitmap_stage_route_ready = 1;
    media->startup_bitmap_soul_room_route_ready = 1;
    media->startup_bitmap_forcefield_route_ready = 1;
    media->startup_bitmap_atlas_ready = 1;
    media->startup_bitmap_atlas_route_count = 4;
    media->startup_bitmap_atlas_route_mask = ROUTES;
    media->startup_bitmap_atlas_tile_count = 48u;
    media->startup_bitmap_atlas_nonzero_pixel_count = 384u;
    media->startup_bitmap_atlas_checksum = 0x22222222u;
    media->startup_bitmap_wide_route_mask = ROUTES;
    media->startup_bitmap_wide_route_count = 4;
    media->startup_bitmap_wide_atlas_tile_count = 48u;
    media->startup_bitmap_raw_route_mask = ROUTES;
    media->startup_bitmap_raw_route_count = 4;
    media->startup_bitmap_raw_atlas_tile_count = 48u;
    media->startup_bitmap_title_sample_count = 12;
    media->startup_bitmap_stage_sample_count = 12;
    media->startup_bitmap_soul_room_sample_count = 12;
    media->startup_bitmap_forcefield_sample_count = 12;
    media->startup_bitmap_title_nonzero_pixel_count = 96u;
    media->startup_bitmap_stage_nonzero_pixel_count = 96u;
    media->startup_bitmap_soul_room_nonzero_pixel_count = 96u;
    media->startup_bitmap_forcefield_nonzero_pixel_count = 96u;
    media->startup_bitmap_title_checksum = 0x33333333u;
    media->startup_bitmap_stage_checksum = 0x44444444u;
    media->startup_bitmap_soul_room_checksum = 0x55555555u;
    media->startup_bitmap_forcefield_checksum = 0x66666666u;
    media->startup_bitmap_title_atlas_tile_count = 12u;
    media->startup_bitmap_stage_atlas_tile_count = 12u;
    media->startup_bitmap_soul_room_atlas_tile_count = 12u;
    media->startup_bitmap_forcefield_atlas_tile_count = 12u;
    media->startup_bitmap_title_atlas_width = 96u;
    media->startup_bitmap_stage_atlas_width = 96u;
    media->startup_bitmap_soul_room_atlas_width = 96u;
    media->startup_bitmap_forcefield_atlas_width = 96u;
    media->startup_bitmap_soul_room_first_raw_offset = 2352u * 3u;
    media->startup_bitmap_soul_room_last_raw_offset = 2352u * 3u + 255u;
}

static int final_bind_accepts_complete_bitmap_route(void)
{
    Theron_V1RawLoaderTraceReceipt trace;
    Theron_StartupMediaStateReceipt media;
    Theron_V1RawLoaderTraceReceipt bound;

    complete_trace(&trace);
    complete_media(&media);
    if (!theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound)) {
        return 0;
    }
    return bound.soul_room_raw_route_verified &&
           bound.soul_room_route_disjoint_from_dynamic_span &&
           bound.soul_room_first_raw_offset ==
               media.startup_bitmap_soul_room_first_raw_offset &&
           bound.soul_room_last_raw_offset ==
               media.startup_bitmap_soul_room_last_raw_offset &&
           bound.soul_room_checksum ==
               media.startup_bitmap_soul_room_checksum &&
           bound.bitmap_route_mask == media.startup_bitmap_raw_route_mask &&
           bound.bitmap_atlas_checksum ==
               media.startup_bitmap_atlas_checksum &&
           !bound.palette_descriptor_relation_verified;
}

static int final_bind_rejects_empty_bitmap_evidence(void)
{
    Theron_V1RawLoaderTraceReceipt trace;
    Theron_StartupMediaStateReceipt media;
    Theron_V1RawLoaderTraceReceipt bound;

    complete_trace(&trace);
    complete_media(&media);
    media.startup_bitmap_raw_atlas_tile_count = 0u;
    if (theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound)) {
        return 0;
    }

    complete_media(&media);
    media.startup_bitmap_atlas_nonzero_pixel_count = 0u;
    if (theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound)) {
        return 0;
    }

    complete_media(&media);
    media.startup_bitmap_soul_room_nonzero_pixel_count = 0u;
    if (theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound)) {
        return 0;
    }

    complete_media(&media);
    media.startup_bitmap_soul_room_atlas_tile_count = 0u;
    if (theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound)) {
        return 0;
    }

    return 1;
}

int main(void)
{
    if (!final_bind_accepts_complete_bitmap_route()) {
        return 1;
    }
    if (!final_bind_rejects_empty_bitmap_evidence()) {
        return 2;
    }
    puts("test_theron_v1_raw_loader_trace_final_bind: PASS");
    return 0;
}
