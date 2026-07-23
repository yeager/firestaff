#include "csb_v1_f0128_entrance_runtime_consumer_pc34_compat.h"

#include <string.h>

enum {
    CSB_V1_F0128_ENTRANCE_VIEWPORT_WIDTH_PC34 = 224,
    CSB_V1_F0128_ENTRANCE_VIEWPORT_HEIGHT_PC34 = 136
};

int csb_v1_f0128_entrance_runtime_consume_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_F0128EntranceRuntimeBinding_PC34 *binding,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *out_receipt)
{
    const CSB_V1_ViewportFirstFrameMaterializationReceipt *material;
    const CSB_V1_ViewportFirstFrameRasterReceiptPc34 *raster;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 receipt;
    int expected_surface_count;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!session || !plan || !binding || !out_receipt || !binding->valid ||
        !binding->material_receipt || !binding->raster_receipt ||
        !binding->viewport_pixels || binding->session_generation == 0u ||
        binding->source_tick != session->source_tick ||
        binding->session_generation != session->generation ||
        binding->viewport_pixel_count !=
            (size_t)CSB_V1_F0128_ENTRANCE_VIEWPORT_WIDTH_PC34 *
                CSB_V1_F0128_ENTRANCE_VIEWPORT_HEIGHT_PC34 ||
        (plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
         plan->surface != CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34)) {
        return 0;
    }

    material = binding->material_receipt;
    raster = binding->raster_receipt;
    if (!material->valid || !material->consumed_by_m11_render ||
        !material->real_graphics_session || !material->no_synthetic_pixels ||
        !material->no_fallback_visuals || material->combined_material_hash == 0u ||
        !raster->valid || !raster->consumed_by_raster || raster->rejected ||
        raster->command_count <= 0 ||
        raster->combined_material_hash != material->combined_material_hash ||
        raster->raster_hash == 0u ||
        !csb_v1_boot_startup_runtime_entrance_f0128_receipt_from_session_pc34(
            session, plan, binding->source_tick, raster,
            binding->viewport_pixels, binding->viewport_pixel_count,
            &receipt)) {
        return 0;
    }

    expected_surface_count = 2; /* decoded C004 plus the F0128 viewport */
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34) {
        expected_surface_count += 2; /* real C002 and C003 */
    } else {
        if (plan->opening_left_w > 0) ++expected_surface_count;
        if (plan->opening_right_w > 0) ++expected_surface_count;
    }
    if (!receipt.valid || !receipt.real_asset_matched ||
        !receipt.no_legacy_wrappers || !receipt.no_synthetic_surface ||
        !receipt.raster.valid || receipt.raster.source_surface_count !=
            expected_surface_count || receipt.frame.source_tick !=
            binding->source_tick || receipt.frame.session_generation !=
            binding->session_generation || receipt.host_surface_hash == 0u) {
        csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&receipt);
        return 0;
    }

    *out_receipt = receipt;
    return 1;
}
