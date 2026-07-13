/* Independent map-transition consumer regression for the public M11 gate. */
#include "m11_dm2_runtime_frame_receipt_gate.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t map_token;
    uint32_t sequence;
    int tick;
} M11_Dm2MapWatermark;

static int failures;

static void check(int condition, const char *label)
{
    if (condition) fprintf(stderr, "PASS: %s\n", label);
    else { fprintf(stderr, "FAIL: %s\n", label); ++failures; }
}

static DM2_V1_BootRuntimeRenderReceipt make_boot(uint32_t map,
                                                  uint32_t scene,
                                                  uint32_t palette)
{
    DM2_V1_BootRuntimeRenderReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.runtime_m11_frame_receipt_consumed = 1;
    receipt.runtime_m11_frame_map_load_token = map;
    receipt.runtime_m11_frame_scene_control_hash = scene;
    receipt.runtime_m11_frame_palette_hash = palette;
    return receipt;
}

static DM2_V1_ViewportM11FrameReceipt make_frame(uint32_t map,
                                                  uint32_t scene,
                                                  uint32_t palette)
{
    DM2_V1_ViewportM11FrameReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.m11_consume_frame = 1;
    receipt.source_materials_required = 1;
    receipt.map_load_token = map;
    receipt.scene_control_hash = scene;
    receipt.palette_hash = palette;
    return receipt;
}

static int consume_map_frame(const DM2_V1_BootRuntimeRenderReceipt *boot,
                             const DM2_V1_ViewportM11FrameReceipt *frame,
                             uint32_t sequence, int tick,
                             M11_Dm2MapWatermark *watermark)
{
    if (!watermark || !frame || !frame->source_materials_required ||
        sequence == 0u || tick < 0 ||
        !M11_Dm2RuntimeFrameReceipt_ShouldPresent(boot, frame)) {
        return 0;
    }
    if (watermark->map_token == frame->map_load_token &&
        watermark->sequence != 0u &&
        (sequence <= watermark->sequence || tick < watermark->tick)) {
        return 0;
    }
    watermark->map_token = frame->map_load_token;
    watermark->sequence = sequence;
    watermark->tick = tick;
    return 1;
}

int main(void)
{
    DM2_V1_BootRuntimeRenderReceipt old_boot = make_boot(42u, 0x1001u, 0x2001u);
    DM2_V1_ViewportM11FrameReceipt old_frame = make_frame(42u, 0x1001u, 0x2001u);
    DM2_V1_BootRuntimeRenderReceipt new_boot = make_boot(43u, 0x1002u, 0x2002u);
    DM2_V1_ViewportM11FrameReceipt new_frame = make_frame(43u, 0x1002u, 0x2002u);
    M11_Dm2MapWatermark watermark = { 0u, 0u, -1 };

    check(consume_map_frame(&old_boot, &old_frame, 8u, 30, &watermark), "accepts the old map frame");
    check(watermark.map_token == 42u && watermark.sequence == 8u && watermark.tick == 30, "records old map watermark");
    check(consume_map_frame(&new_boot, &new_frame, 1u, 0, &watermark), "accepts a new map with reset sequence and tick");
    check(watermark.map_token == 43u && watermark.sequence == 1u && watermark.tick == 0, "rebinds watermark to new map identity");
    check(!consume_map_frame(&new_boot, &old_frame, 9u, 31, &watermark), "public gate rejects the old map after transition");
    check(watermark.map_token == 43u && watermark.sequence == 1u && watermark.tick == 0, "stale old map cannot alter new watermark");
    check(!consume_map_frame(&new_boot, &new_frame, 1u, 0, &watermark), "rejects replay on the new map");
    check(!consume_map_frame(&new_boot, &new_frame, 2u, -1, &watermark), "rejects negative tick on the new map");
    check(consume_map_frame(&new_boot, &new_frame, 2u, 0, &watermark), "accepts later sequence on transition tick");
    check(consume_map_frame(&new_boot, &new_frame, 3u, 1, &watermark), "accepts later tick on the new map");

    new_frame.source_materials_required = 0;
    check(!consume_map_frame(&new_boot, &new_frame, 4u, 1, &watermark), "rejects non-source-required new map frame");
    check(watermark.sequence == 3u && watermark.tick == 1, "source rejection preserves new map watermark");
    new_frame = make_frame(43u, 0x1002u, 0x2002u); new_frame.scene_control_hash++;
    check(!consume_map_frame(&new_boot, &new_frame, 4u, 2, &watermark), "rejects stale scene after transition");
    new_frame = make_frame(43u, 0x1002u, 0x2002u); new_frame.palette_hash++;
    check(!consume_map_frame(&new_boot, &new_frame, 4u, 2, &watermark), "rejects stale palette after transition");
    new_frame = make_frame(43u, 0x1002u, 0x2002u); new_frame.valid = 0;
    check(!consume_map_frame(&new_boot, &new_frame, 4u, 2, &watermark), "rejects invalid new map frame");
    new_frame = make_frame(43u, 0x1002u, 0x2002u);
    check(!consume_map_frame(NULL, &new_frame, 4u, 2, &watermark) &&
          !consume_map_frame(&new_boot, NULL, 4u, 2, &watermark),
          "rejects missing transition handoff without fallback");
    return failures ? 1 : 0;
}
