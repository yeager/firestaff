#ifndef FIRESTAFF_M11_DM2_RUNTIME_FRAME_RECEIPT_GATE_H
#define FIRESTAFF_M11_DM2_RUNTIME_FRAME_RECEIPT_GATE_H

#include "dm2_v1_runtime.h"

/* M11 live dungeon/HUD gate for source-required DM2 frames. It accepts only
 * a boot handoff and runtime receipt from the same GDAT map generation. */
int M11_Dm2RuntimeFrameReceipt_ShouldPresent(
    const DM2_V1_BootRuntimeRenderReceipt *boot_receipt,
    const DM2_V1_ViewportM11FrameReceipt *runtime_receipt);

#endif /* FIRESTAFF_M11_DM2_RUNTIME_FRAME_RECEIPT_GATE_H */
