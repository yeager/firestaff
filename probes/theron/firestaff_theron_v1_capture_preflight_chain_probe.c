#include "theron_v1_raw_loader_trace.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    static const char capture[] =
        "source=mednafen-pce-instrumented\n"
        "boot_pc=e98a physical_pc=0000e98a instruction=LDA $22A4 cd_1800=90\n"
        "post_e98a_controller_transfer_source_pc=e98e source_physical_pc=0000e98e instruction=JSR $EA27 next_pc=ea27 next_physical_pc=0000ea27\n"
        "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 variant=us_bin record=0004e0\n"
        "dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=00 f5_at_irq2_entry=00 status_1802=00 status_1803=00 f2_before_merge=00 f2_at_branch=00\n"
        "dynamic_huc6260_palette_store pc=4a00 physical_pc=00004a00 opcode=8d address=0402 accumulator=01\n";
    Theron_V1RawLoaderTraceReceipt trace;
    Theron_V1RawLoaderTraceReceipt bound;
    Theron_StartupMediaStateReceipt media;

    memset(&media, 0, sizeof(media));
    media.startup_media_ready = 1;
    media.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media.track02_md5, sizeof(media.track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    media.startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media.startup_bitmap_route_mask = 15u;
    media.startup_bitmap_atlas_route_mask = 15u;
    media.startup_bitmap_raw_route_mask = 15u;
    media.startup_bitmap_sample_count = 48u;
    media.startup_bitmap_nonzero_pixel_count = 1u;
    media.startup_bitmap_checksum = 1u;
    media.startup_bitmap_atlas_ready = 1;
    media.startup_bitmap_atlas_route_count = 4u;
    media.startup_bitmap_atlas_tile_count = 48u;
    media.startup_bitmap_atlas_nonzero_pixel_count = 1u;
    media.startup_bitmap_atlas_checksum = 1u;
    media.startup_bitmap_wide_route_mask = 15u;
    media.startup_bitmap_wide_route_count = 4u;
    media.startup_bitmap_wide_atlas_tile_count = 48u;
    media.startup_bitmap_title_route_ready = 1;
    media.startup_bitmap_stage_route_ready = 1;
    media.startup_bitmap_soul_room_route_ready = 1;
    media.startup_bitmap_forcefield_route_ready = 1;
    media.startup_bitmap_title_sample_count = 12u;
    media.startup_bitmap_stage_sample_count = 12u;
    media.startup_bitmap_soul_room_sample_count = 12u;
    media.startup_bitmap_forcefield_sample_count = 12u;
    media.startup_bitmap_title_nonzero_pixel_count = 1u;
    media.startup_bitmap_stage_nonzero_pixel_count = 1u;
    media.startup_bitmap_soul_room_nonzero_pixel_count = 1u;
    media.startup_bitmap_forcefield_nonzero_pixel_count = 1u;
    media.startup_bitmap_title_checksum = 1u;
    media.startup_bitmap_stage_checksum = 1u;
    media.startup_bitmap_soul_room_checksum = 1u;
    media.startup_bitmap_forcefield_checksum = 1u;
    media.startup_bitmap_title_atlas_tile_count = 12u;
    media.startup_bitmap_stage_atlas_tile_count = 12u;
    media.startup_bitmap_soul_room_atlas_tile_count = 12u;
    media.startup_bitmap_forcefield_atlas_tile_count = 12u;
    media.startup_bitmap_title_atlas_width = 96u;
    media.startup_bitmap_stage_atlas_width = 96u;
    media.startup_bitmap_soul_room_atlas_width = 96u;
    media.startup_bitmap_forcefield_atlas_width = 96u;

    return theron_v1_raw_loader_trace_ingest_mednafen_capture(
               capture, THERON_TRACK02_MD5_US_BIN, &trace) &&
           theron_v1_raw_loader_trace_final_bind(&trace, &media, &bound) &&
           bound.bitmap_route_mask == 15u && bound.bitmap_atlas_checksum == 1u &&
           !bound.palette_descriptor_relation_verified ? 0 : 1;
}
