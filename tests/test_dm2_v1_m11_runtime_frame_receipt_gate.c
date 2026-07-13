/* Data-free M11 acceptance gate for the source-required DM2 viewport frame.
 * skproject binds GRAPHICSSET and dtPalette16 for one active map generation;
 * M11 must not present a receipt from another map, scene, or palette. */
#include "m11_dm2_runtime_frame_receipt_gate.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (condition) {
        fprintf(stderr, "PASS: %s\n", label);
    } else {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static DM2_V1_BootRuntimeRenderReceipt make_boot_receipt(void)
{
    DM2_V1_BootRuntimeRenderReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.runtime_m11_frame_receipt_consumed = 1;
    receipt.runtime_m11_frame_map_load_token = 42u;
    receipt.runtime_m11_frame_scene_control_hash = 0x53434e45u;
    receipt.runtime_m11_frame_palette_hash = 0x50414c31u;
    return receipt;
}

static DM2_V1_ViewportM11FrameReceipt make_runtime_receipt(void)
{
    DM2_V1_ViewportM11FrameReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.m11_consume_frame = 1;
    receipt.source_materials_required = 1;
    receipt.map_load_token = 42u;
    receipt.scene_control_hash = 0x53434e45u;
    receipt.palette_hash = 0x50414c31u;
    return receipt;
}

int main(void)
{
    DM2_V1_BootRuntimeRenderReceipt boot = make_boot_receipt();
    DM2_V1_ViewportM11FrameReceipt runtime = make_runtime_receipt();

    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 1,
          "M11 presents the current verified atomic DM2 frame");

    runtime.map_load_token++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale DM2 map token");
    runtime = make_runtime_receipt();

    runtime.scene_control_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale DM2 scene hash");
    runtime = make_runtime_receipt();

    runtime.palette_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale DM2 palette hash");
    runtime = make_runtime_receipt();

    runtime.valid = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an invalid atomic DM2 frame");
    runtime = make_runtime_receipt();

    boot.runtime_m11_frame_receipt_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a missing boot handoff receipt");
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(NULL, &runtime) == 0 &&
              M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, NULL) == 0,
          "M11 rejects absent receipt inputs without fallback");

    return failures ? 1 : 0;
}
