#include "csb_v1_startup_raster_present_pc34_compat.h"

#include <string.h>

static uint32_t fnv1a(const uint8_t *pixels, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    for (index = 0U; index < count; ++index) {
        hash ^= pixels[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1U;
}

static uint32_t hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash ? hash : 2166136261u;
}

static uint32_t expected_host_hash(
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt)
{
    uint32_t hash = 2166136261u;

    hash = hash_step(hash, receipt->frame.frame_route_hash);
    hash = hash_step(hash, receipt->raster.route_hash);
    hash = hash_step(hash, receipt->raster.pixel_hash);
    hash = hash_step(hash, (uint32_t)receipt->host_surface);
    hash = hash_step(hash, receipt->frame.hud_binding_hash);
    if (receipt->host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 ||
        receipt->host_surface ==
            CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34) {
        hash = hash_step(hash, (uint32_t)(receipt->special_palette + 1));
        hash = hash_step(hash, (uint32_t)(receipt->title_special_palette + 1));
    }
    return hash;
}

static void deliver_vertical_blank(void *context)
{
    F0693_VerticalBlankCallback_PC34(
        (ReDMCSBF0693WaitVerticalBlankPc34Compat *)context);
}

int main(void)
{
    uint8_t indexed[320 * 200];
    uint8_t packed[320 * 200 / 2];
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host_receipt;
    csb_v1_startup_real_raster_pc34_compat raster = {
        indexed, &host_receipt, 320, 200, 0U, 0x22334455U, 0x66778899U, 1, 1
    };
    ReDMCSBF0693WaitVerticalBlankPc34Compat gate = {
        false, deliver_vertical_blank, NULL
    };
    size_t index;

    memset(indexed, 0, sizeof(indexed));
    memset(packed, 0xff, sizeof(packed));
    memset(&host_receipt, 0, sizeof(host_receipt));
    indexed[0] = 9;
    indexed[1] = 8;
    indexed[63999] = 6;
    raster.source_pixel_hash = fnv1a(indexed, sizeof(indexed));
    host_receipt.frame.frame_route_hash = 0x11223344U;
    host_receipt.frame.hud_binding_hash = 0x88776655U;
    host_receipt.special_palette = -1;
    host_receipt.title_special_palette = -1;
    host_receipt.valid = 1;
    host_receipt.real_asset_matched = 1;
    host_receipt.no_legacy_wrappers = 1;
    host_receipt.no_synthetic_surface = 1;
    host_receipt.host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34;
    host_receipt.raster.pixels = indexed;
    host_receipt.raster.width = 320;
    host_receipt.raster.height = 200;
    host_receipt.raster.valid = 1;
    host_receipt.raster.real_asset_matched = 1;
    host_receipt.raster.pixel_hash = raster.source_pixel_hash;
    host_receipt.raster.route_hash = raster.source_route_hash;
    host_receipt.host_surface_hash = expected_host_hash(&host_receipt);
    raster.source_host_surface_hash = host_receipt.host_surface_hash;
    gate.context = &gate;
    if (!csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate) || packed[0] != 0x98 ||
        packed[sizeof(packed) - 1U] != 0x06 || gate.waiting_for_vertical_blank) {
        return 1;
    }
    raster.host_surface_receipt = NULL;
    memset(packed, 0xaa, sizeof(packed));
    if (csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate)) {
        return 1;
    }
    for (index = 0U; index < sizeof(packed); ++index) {
        if (packed[index] != 0xaa) {
            return 1;
        }
    }
    raster.host_surface_receipt = &host_receipt;
    host_receipt.host_surface_hash ^= 1U;
    if (csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate)) {
        return 1;
    }
    host_receipt.host_surface_hash = expected_host_hash(&host_receipt);
    raster.source_host_surface_hash = host_receipt.host_surface_hash;
    host_receipt.no_synthetic_surface = 0;
    if (csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate)) {
        return 1;
    }
    host_receipt.no_synthetic_surface = 1;
    host_receipt.host_surface_hash ^= 0x10U;
    raster.source_host_surface_hash = host_receipt.host_surface_hash;
    if (csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate)) {
        return 1;
    }
    host_receipt.host_surface_hash = expected_host_hash(&host_receipt);
    raster.source_host_surface_hash = host_receipt.host_surface_hash;
    indexed[100] = 16;
    memset(packed, 0xaa, sizeof(packed));
    if (csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate)) {
        return 1;
    }
    for (index = 0U; index < sizeof(packed); ++index) {
        if (packed[index] != 0xaa) {
            return 1;
        }
    }
    return 0;
}
