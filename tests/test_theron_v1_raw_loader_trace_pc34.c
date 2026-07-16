#include "theron_v1_boot.h"
#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <string.h>

#define ROUTES (THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM | \
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD)

static void complete_media(Theron_StartupMediaStateReceipt *media)
{
    memset(media, 0, sizeof(*media));
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    strcpy(media->track02_md5, "0123456789abcdef0123456789abcdef");
    media->track02_size = 2352u * 8u;
    media->startup_media_ready = 1;
    media->startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media->startup_bitmap_sample_count = 48;
    media->startup_bitmap_route_mask = ROUTES;
    media->startup_bitmap_nonzero_pixel_count = 1u;
    media->startup_bitmap_checksum = 1u;
    media->startup_bitmap_title_route_ready = 1;
    media->startup_bitmap_stage_route_ready = 1;
    media->startup_bitmap_soul_room_route_ready = 1;
    media->startup_bitmap_forcefield_route_ready = 1;
    media->startup_bitmap_atlas_ready = 1;
    media->startup_bitmap_atlas_route_count = 4;
    media->startup_bitmap_atlas_route_mask = ROUTES;
    media->startup_bitmap_atlas_tile_count = 48u;
    media->startup_bitmap_atlas_nonzero_pixel_count = 1u;
    media->startup_bitmap_atlas_checksum = 1u;
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
    media->startup_bitmap_title_nonzero_pixel_count = 1u;
    media->startup_bitmap_stage_nonzero_pixel_count = 1u;
    media->startup_bitmap_soul_room_nonzero_pixel_count = 1u;
    media->startup_bitmap_forcefield_nonzero_pixel_count = 1u;
    media->startup_bitmap_title_checksum = 1u;
    media->startup_bitmap_stage_checksum = 1u;
    media->startup_bitmap_soul_room_checksum = 1u;
    media->startup_bitmap_forcefield_checksum = 1u;
    media->startup_bitmap_title_atlas_tile_count = 12u;
    media->startup_bitmap_stage_atlas_tile_count = 12u;
    media->startup_bitmap_soul_room_atlas_tile_count = 12u;
    media->startup_bitmap_forcefield_atlas_tile_count = 12u;
    media->startup_bitmap_title_atlas_width = 96u;
    media->startup_bitmap_stage_atlas_width = 96u;
    media->startup_bitmap_soul_room_atlas_width = 96u;
    media->startup_bitmap_forcefield_atlas_width = 96u;
    media->startup_bitmap_soul_room_first_raw_offset = 2352u;
    media->startup_bitmap_soul_room_last_raw_offset = 2352u + 255u;
}

int main(void)
{
    Theron_V1RawLoaderTraceRow rows[] = {
        {1u, 0x1800u, 1u, 2352u + 128u, 0x3800u, 0u},
        {2u, 0x0400u, 1u, 2352u + 128u, 0u, 0u}
    };
    Theron_StartupMediaStateReceipt media;
    Theron_V1RawLoaderTraceReceipt trace;
    Theron_V1RawLoaderTraceReceipt bound;
    Theron_V1_BootStartupRawMediaGraphicsReceipt gate;

    complete_media(&media);
    if (!theron_v1_raw_loader_trace_ingest(rows, 2u, media.track02_size,
                                            &trace) ||
        !trace.cd_read_source_offset_verified ||
        !trace.cd_read_sector_verified || trace.cd_read_sector != 1u ||
        trace.cd_read_source_offset != rows[0].source_offset) return 1;
    strcpy(trace.track02_md5, media.track02_md5);
    if (!theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound) ||
        !bound.soul_room_raw_span_verified ||
        !theron_v1_boot_startup_raw_media_graphics_receipt_from_loader_trace(
            &media, &trace, &gate) || !gate.valid) return 2;
    rows[0].source_offset = media.startup_bitmap_soul_room_last_raw_offset + 1u;
    rows[1].source_offset = rows[0].source_offset;
    if (!theron_v1_raw_loader_trace_ingest(rows, 2u, media.track02_size,
                                            &trace)) return 3;
    strcpy(trace.track02_md5, media.track02_md5);
    if (!theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound) ||
        bound.soul_room_raw_span_verified ||
        theron_v1_boot_startup_raw_media_graphics_receipt_from_loader_trace(
            &media, &trace, &gate) || gate.valid) return 4;
    rows[0].source_offset = 16u;
    rows[1].source_offset = 16u;
    if (theron_v1_raw_loader_trace_ingest(rows, 2u, media.track02_size,
                                            &trace)) return 5;
    puts("theron raw loader trace soul-span gate: PASS");
    return 0;
}
