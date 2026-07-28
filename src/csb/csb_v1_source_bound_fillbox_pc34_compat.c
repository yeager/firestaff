#include "csb_v1_source_bound_fillbox_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

static uint32_t hash_pixels(const uint8_t *pixels, size_t pixel_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!pixels || pixel_count == 0u) return 0u;
    for (i = 0u; i < pixel_count; ++i) {
        hash ^= pixels[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int surface_matches(const CSB_V1_StartupRuntimeSurface_PC34 *surface,
                           int asset_id, int width, int height,
                           int transparent_color)
{
    return surface && surface->valid && surface->pixels &&
        surface->source_asset_id == asset_id && surface->width == width &&
        surface->height == height &&
        surface->transparent_color == transparent_color &&
        surface->decode_receipt.valid &&
        surface->decode_receipt.ended_at_record_boundary &&
        surface->decode_receipt.indexed_pixel_fnv1a != 0u;
}

static int receipt_common_valid(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt)
{
    const CSB_V1_StartupRuntimeRaster_PC34 *raster;

    if (!session || !receipt || !session->valid ||
        !session->real_asset_matched || !session->rejects_legacy_wrappers ||
        !session->playback.no_fallback_routes || !receipt->valid ||
        !receipt->real_asset_matched || !receipt->no_legacy_wrappers ||
        !receipt->no_synthetic_surface || receipt->host_surface_hash == 0u ||
        receipt->frame.session_generation != session->generation) {
        return 0;
    }
    raster = &receipt->raster;
    return raster->valid && raster->real_asset_matched && raster->pixels &&
        raster->width == CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34 &&
        raster->height == CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34 &&
        raster->pixel_hash != 0u && raster->route_hash != 0u;
}

static int receipt_is_original_hud(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt)
{
    return receipt->host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34 &&
        receipt->runtime_hud_decision && receipt->uses_c017_inventory &&
        receipt->uses_c040_resurrect && receipt->raster.source_surface_count == 2 &&
        session->playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34 &&
        session->hud_inventory_binding.source ==
            CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 &&
        session->hud_resurrect_binding.source ==
            CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34 &&
        surface_matches(receipt->frame.hud_inventory_surface, 17, 224, 136, -1) &&
        surface_matches(receipt->frame.hud_resurrect_surface, 40, 144, 73, 6);
}

static int receipt_is_original_door(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt)
{
    const int expected_surfaces = 1 +
        (receipt->frame.opening_step < 27 ? 1 : 0) + 1;

    return receipt->host_surface ==
            CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34 &&
        receipt->door_opening_decision &&
        session->playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34 &&
        receipt->frame.opening_step >= 1 && receipt->frame.opening_step <= 31 &&
        receipt->raster.door_composited &&
        receipt->raster.source_surface_count == expected_surfaces &&
        receipt->special_palette == VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE &&
        receipt->title_special_palette == -1 &&
        surface_matches(receipt->frame.left_door_surface, 2, 105, 161, -1) &&
        surface_matches(receipt->frame.right_door_surface, 3, 128, 161, -1);
}

void csb_v1_source_bound_fill_target_init_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target)
{
    if (target) memset(target, 0, sizeof(*target));
}

void csb_v1_source_bound_fill_target_release_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target)
{
    if (!target) return;
    free(target->pixels);
    memset(target, 0, sizeof(*target));
}

int csb_v1_source_bound_fill_target_from_host_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt,
    CSB_V1_SourceBoundFillTarget_PC34 *out_target)
{
    CSB_V1_SourceBoundFillTarget_PC34 target;
    size_t pixel_count;

    if (!out_target || !receipt_common_valid(session, receipt) ||
        !(receipt_is_original_hud(session, receipt) ||
          receipt_is_original_door(session, receipt))) {
        return 0;
    }
    if (receipt->host_surface ==
            CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34) {
        CSB_V1_DoorOpeningPhaseReceipt_PC34 phase;
        if (!csb_v1_door_opening_phase_receipt_from_host_pc34(
                session, receipt, 0, &phase)) {
            return 0;
        }
    }
    pixel_count = (size_t)receipt->raster.width * (size_t)receipt->raster.height;
    if (pixel_count == 0u ||
        hash_pixels(receipt->raster.pixels, pixel_count) !=
            receipt->raster.pixel_hash) {
        return 0;
    }
    memset(&target, 0, sizeof(target));
    target.pixels = (uint8_t *)malloc(pixel_count);
    if (!target.pixels) return 0;
    memcpy(target.pixels, receipt->raster.pixels, pixel_count);
    target.pixel_count = pixel_count;
    target.width = receipt->raster.width;
    target.height = receipt->raster.height;
    target.valid = 1;
    target.real_graphics_dat = 1;
    target.no_fallback_route = 1;
    target.host_surface = receipt->host_surface;
    target.source_pixel_hash = receipt->raster.pixel_hash;
    target.source_route_hash = receipt->raster.route_hash;
    target.source_host_surface_hash = receipt->host_surface_hash;
    target.result_pixel_hash = target.source_pixel_hash;
    csb_v1_source_bound_fill_target_release_pc34(out_target);
    *out_target = target;
    return 1;
}

int csb_v1_door_opening_phase_receipt_from_host_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_receipt,
    int f0128_viewport_bound,
    CSB_V1_DoorOpeningPhaseReceipt_PC34 *out_receipt)
{
    CSB_V1_DoorOpeningPhaseReceipt_PC34 receipt;
    int expected_surface_count;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !receipt_common_valid(session, host_receipt) ||
        !receipt_is_original_door(session, host_receipt) ||
        f0128_viewport_bound < 0 || f0128_viewport_bound > 1 ||
        host_receipt->frame.opening_step < 1 ||
        host_receipt->frame.opening_step > 31 ||
        host_receipt->frame.source_tick == 0u ||
        host_receipt->frame.session_generation != session->generation ||
        host_receipt->frame.frame_route_hash == 0u ||
        host_receipt->host_surface_hash == 0u ||
        host_receipt->raster.route_hash == 0u) {
        return 0;
    }

    /* C004 is always present; C002 is clipped after source step 26 while
     * C003 remains. A verified F0128 viewport contributes one source only
     * after it is already present in the supplied host raster. */
    expected_surface_count = 2 +
        (host_receipt->frame.opening_step < 27 ? 1 : 0) +
        f0128_viewport_bound;
    if (host_receipt->raster.source_surface_count != expected_surface_count ||
        hash_pixels(host_receipt->raster.pixels,
                    (size_t)host_receipt->raster.width *
                    (size_t)host_receipt->raster.height) !=
            host_receipt->raster.pixel_hash) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.opening_step = host_receipt->frame.opening_step;
    receipt.f0128_viewport_bound = f0128_viewport_bound;
    receipt.source_surface_count = expected_surface_count;
    receipt.source_tick = host_receipt->frame.source_tick;
    receipt.session_generation = host_receipt->frame.session_generation;
    receipt.frame_route_hash = host_receipt->frame.frame_route_hash;
    receipt.raster_pixel_hash = host_receipt->raster.pixel_hash;
    receipt.host_surface_hash = host_receipt->host_surface_hash;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_source_bound_f0134_fill_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target,
    uint8_t color)
{
    if (!target || !target->valid || !target->real_graphics_dat ||
        !target->no_fallback_route || !target->pixels ||
        target->pixel_count != (size_t)target->width * (size_t)target->height ||
        color > 15u) {
        return 0;
    }
    memset(target->pixels, color, target->pixel_count);
    target->result_pixel_hash = hash_pixels(target->pixels, target->pixel_count);
    target->operation_count++;
    return target->result_pixel_hash != 0u;
}

int csb_v1_source_bound_f0135_fill_box_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target,
    const int16_t box[4],
    uint16_t color)
{
    const uint8_t fill_color = (uint8_t)(color & 0x000fu);
    const int alternate = (color & 0x8000u) != 0u;
    int left;
    int right;
    int top;
    int bottom;
    int y;

    if (!target || !target->valid || !target->real_graphics_dat ||
        !target->no_fallback_route || !target->pixels || !box ||
        target->width <= 0 || target->height <= 0 ||
        target->pixel_count != (size_t)target->width * (size_t)target->height) {
        return 0;
    }
    left = box[0]; right = box[1]; top = box[2]; bottom = box[3];
    if (left < 0 || top < 0 || left > right || top > bottom ||
        right >= target->width || bottom >= target->height) {
        return 0;
    }
    for (y = top; y <= bottom; ++y) {
        int x;
        for (x = left; x <= right; ++x) {
            if (alternate && (((x - left) + (y - top)) & 1) != 0) continue;
            target->pixels[(size_t)y * (size_t)target->width + (size_t)x] =
                fill_color;
        }
    }
    target->result_pixel_hash = hash_pixels(target->pixels, target->pixel_count);
    target->operation_count++;
    return target->result_pixel_hash != 0u;
}

const char *csb_v1_source_bound_fillbox_source_evidence_pc34(void)
{
    return "ReDMCSB BLITFILL.C F0134_VIDEO_FillBitmap and FILLBOX.C "
           "F0135_VIDEO_FillBox/F0692 operate on caller-owned bitmap data. "
           "The CSB PC34 consumer accepts only a current, decoder-bound "
           "GRAPHICS.DAT C017/C040 HUD raster or C002/C003 opening-door "
           "raster, copies it, and has no fallback source.";
}
