/* skproject routes dungeon GRAPHICSSET fields and dtPalette16 under one
 * active map context. This M11 boundary rejects a stale handoff rather than
 * presenting an old source-required DM2 frame. */
#include "m11_dm2_runtime_frame_receipt_gate.h"

int M11_Dm2RuntimeFrameReceipt_ShouldPresent(
    const DM2_V1_BootRuntimeRenderReceipt *boot_receipt,
    const DM2_V1_ViewportM11FrameReceipt *runtime_receipt)
{
    if (!boot_receipt || !boot_receipt->runtime_m11_frame_receipt_consumed ||
        boot_receipt->runtime_m11_frame_map_load_token == 0u ||
        boot_receipt->runtime_m11_frame_scene_control_hash == 0u ||
        boot_receipt->runtime_m11_frame_palette_hash == 0u ||
        !runtime_receipt) {
        return 0;
    }
    return runtime_receipt->valid && runtime_receipt->m11_consume_frame &&
        runtime_receipt->map_load_token ==
            boot_receipt->runtime_m11_frame_map_load_token &&
        runtime_receipt->scene_control_hash ==
            boot_receipt->runtime_m11_frame_scene_control_hash &&
        runtime_receipt->palette_hash ==
            boot_receipt->runtime_m11_frame_palette_hash;
}
