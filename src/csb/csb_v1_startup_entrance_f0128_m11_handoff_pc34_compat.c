#include "csb_v1_startup_entrance_f0128_m11_handoff_pc34_compat.h"
#include <string.h>

enum { CSB_V1_F0128_VIEWPORT_BYTES_PC34 = 224 * 136 };

static uint32_t csb_v1_f0128_fnv1a_pc34(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    while (count-- != 0u) hash = (hash ^ *bytes++) * 16777619u;
    return hash;
}

int csb_v1_startup_entrance_f0128_produce_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    CSB_V1_ViewportFirstFrameMaterializationReceipt *out_material,
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 *out_raster,
    uint8_t *out_pixels, size_t out_pixel_count)
{
    const CSB_V1_StartupRuntimeSurface_PC34 *c004;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    int row;

    if (out_material) memset(out_material, 0, sizeof(*out_material));
    if (out_raster) memset(out_raster, 0, sizeof(*out_raster));
    if (!session || !plan || !out_material || !out_raster || !out_pixels ||
        out_pixel_count != CSB_V1_F0128_VIEWPORT_BYTES_PC34 ||
        source_tick == 0u || session->generation == 0u ||
        (plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
         plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34 &&
         plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34)) {
        return 0;
    }
    /* This is the producer boundary: advance the verified session to the
     * exact M11 source tick before borrowing C004's decoded interior. */
    if (!csb_v1_boot_startup_runtime_asset_session_frame_pc34(
            session, plan, source_tick, &frame) || !frame.valid) {
        return 0;
    }
    c004 = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    if (!session->valid || !session->real_asset_matched ||
        !session->rejects_legacy_wrappers || !c004->valid || !c004->pixels ||
        c004->source_kind !=
            CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34 ||
        c004->source_asset_id != 4 || c004->width != 320 || c004->height != 200 ||
        frame.entrance_surface != c004 || !c004->decode_receipt.valid ||
        !c004->decode_receipt.ended_at_record_boundary ||
        c004->decode_receipt.indexed_pixel_fnv1a == 0u) return 0;

    for (row = 0; row < 136; ++row) {
        memcpy(out_pixels + (size_t)row * 224u,
               c004->pixels + (size_t)(33 + row) * 320u, 224u);
    }
    out_material->valid = out_material->consumed_by_m11_render = 1;
    out_material->real_graphics_session = out_material->no_synthetic_pixels = 1;
    out_material->no_fallback_visuals = 1;
    out_material->combined_material_hash = c004->decode_receipt.indexed_pixel_fnv1a;
    out_raster->valid = out_raster->consumed_by_raster = 1;
    out_raster->command_count = 1;
    out_raster->combined_material_hash = out_material->combined_material_hash;
    out_raster->raster_hash = csb_v1_f0128_fnv1a_pc34(out_pixels, out_pixel_count);
    return out_raster->raster_hash != 0u;
}
