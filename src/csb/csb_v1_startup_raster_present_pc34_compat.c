#include "csb_v1_startup_raster_present_pc34_compat.h"
#include "redmcsb_f0692_fill_box_pc34_compat.h"

enum {
    CSB_V1_PC34_PRESENT_WIDTH = 320,
    CSB_V1_PC34_PRESENT_HEIGHT = 200,
    CSB_V1_PC34_PRESENT_PACKED_BYTES = 320 * 200 / 2
};

static uint32_t csb_v1_startup_present_fnv1a_pc34(
    const uint8_t *pixels,
    size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (pixels == NULL || count == 0U) return 0U;
    for (index = 0U; index < count; ++index) {
        hash ^= pixels[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1U;
}

static int csb_v1_startup_present_host_receipt_matches_pc34(
    const csb_v1_startup_real_raster_pc34_compat *raster)
{
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt;
    const size_t pixel_count =
        (size_t)CSB_V1_PC34_PRESENT_WIDTH * CSB_V1_PC34_PRESENT_HEIGHT;

    if (raster == NULL || raster->host_surface_receipt == NULL ||
        raster->source_pixel_hash == 0U || raster->source_route_hash == 0U ||
        raster->source_host_surface_hash == 0U) {
        return 0;
    }
    receipt = raster->host_surface_receipt;
    if (!receipt->valid || !receipt->real_asset_matched ||
        !receipt->no_legacy_wrappers || !receipt->no_synthetic_surface ||
        receipt->host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_NONE_PC34 ||
        receipt->host_surface_hash != raster->source_host_surface_hash ||
        !receipt->raster.valid || !receipt->raster.real_asset_matched ||
        receipt->raster.pixels != raster->indexed_pixels ||
        receipt->raster.width != CSB_V1_PC34_PRESENT_WIDTH ||
        receipt->raster.height != CSB_V1_PC34_PRESENT_HEIGHT ||
        receipt->raster.pixel_hash != raster->source_pixel_hash ||
        receipt->raster.route_hash != raster->source_route_hash ||
        csb_v1_startup_present_fnv1a_pc34(raster->indexed_pixels,
                                          pixel_count) !=
            raster->source_pixel_hash) {
        return 0;
    }
    return 1;
}

int csb_v1_startup_present_real_raster_pc34_compat(
    const csb_v1_startup_real_raster_pc34_compat *raster,
    uint8_t *packed_page,
    size_t packed_page_byte_count,
    ReDMCSBF0693WaitVerticalBlankPc34Compat *vblank_gate)
{
    static const int16_t whole_screen[] = {0, 0, 320, 200};
    size_t pixel;

    if (raster == NULL || packed_page == NULL || vblank_gate == NULL ||
        vblank_gate->deliver_vertical_blank == NULL || !raster->valid ||
        !raster->real_asset_matched || raster->indexed_pixels == NULL ||
        raster->width != CSB_V1_PC34_PRESENT_WIDTH ||
        raster->height != CSB_V1_PC34_PRESENT_HEIGHT ||
        !csb_v1_startup_present_host_receipt_matches_pc34(raster) ||
        packed_page_byte_count < CSB_V1_PC34_PRESENT_PACKED_BYTES) {
        return 0;
    }
    for (pixel = 0U; pixel < (size_t)CSB_V1_PC34_PRESENT_WIDTH *
                                  CSB_V1_PC34_PRESENT_HEIGHT; ++pixel) {
        if (raster->indexed_pixels[pixel] > 15U) {
            return 0;
        }
    }
    if (!redmcsb_f0692_fill_box_pc34_compat(
            packed_page, packed_page_byte_count, whole_screen, 0,
            CSB_V1_PC34_PRESENT_WIDTH)) {
        return 0;
    }
    for (pixel = 0U; pixel < (size_t)CSB_V1_PC34_PRESENT_WIDTH *
                                  CSB_V1_PC34_PRESENT_HEIGHT; ++pixel) {
        uint8_t color = raster->indexed_pixels[pixel];
        uint8_t *destination = &packed_page[pixel / 2U];
        if ((pixel & 1U) == 0U) {
            *destination = (uint8_t)((*destination & 0x0fU) | (color << 4));
        } else {
            *destination = (uint8_t)((*destination & 0xf0U) | color);
        }
    }
    return F0693_WaitVerticalBlank_PC34(vblank_gate) ? 1 : 0;
}
