/* Independent consumer policy test for the public DM2 M11 frame gate. */
#include "m11_dm2_runtime_frame_receipt_gate.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    uint32_t render_sequence;
    int runtime_tick;
} M11_Dm2FrameWatermark;

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

static int consume_source_frame(
    const DM2_V1_BootRuntimeRenderReceipt *boot,
    const DM2_V1_ViewportM11FrameReceipt *frame,
    uint32_t render_sequence, int runtime_tick,
    M11_Dm2FrameWatermark *watermark)
{
    int same_identity;

    if (!watermark || !frame || !frame->source_materials_required ||
        render_sequence == 0u || runtime_tick < 0 ||
        !M11_Dm2RuntimeFrameReceipt_ShouldPresent(boot, frame)) {
        return 0;
    }
    same_identity = watermark->map_load_token == frame->map_load_token &&
        watermark->scene_control_hash == frame->scene_control_hash &&
        watermark->palette_hash == frame->palette_hash;
    if (watermark->render_sequence != 0u && same_identity &&
        (render_sequence <= watermark->render_sequence ||
         runtime_tick < watermark->runtime_tick)) {
        return 0;
    }
    watermark->map_load_token = frame->map_load_token;
    watermark->scene_control_hash = frame->scene_control_hash;
    watermark->palette_hash = frame->palette_hash;
    watermark->render_sequence = render_sequence;
    watermark->runtime_tick = runtime_tick;
    return 1;
}

int main(void)
{
    DM2_V1_BootRuntimeRenderReceipt boot =
        make_boot(42u, 0x53434e45u, 0x50414c31u);
    DM2_V1_ViewportM11FrameReceipt frame =
        make_frame(42u, 0x53434e45u, 0x50414c31u);
    M11_Dm2FrameWatermark watermark = { 0u, 0u, 0u, 0u, -1 };

    check(consume_source_frame(&boot, &frame, 1u, 10, &watermark), "accepts first source-required identity");
    check(watermark.map_load_token == 42u && watermark.render_sequence == 1u && watermark.runtime_tick == 10, "records identity watermark");
    check(consume_source_frame(&boot, &frame, 2u, 10, &watermark), "accepts later sequence on same tick");
    check(consume_source_frame(&boot, &frame, 3u, 11, &watermark), "accepts later sequence on later tick");
    check(!consume_source_frame(&boot, &frame, 3u, 11, &watermark), "rejects replay within identity");
    check(!consume_source_frame(&boot, &frame, 2u, 11, &watermark), "rejects sequence rollback within identity");
    check(!consume_source_frame(&boot, &frame, 4u, 10, &watermark), "rejects tick rollback within identity");
    check(watermark.render_sequence == 3u && watermark.runtime_tick == 11, "rejected rollback preserves watermark");
    check(consume_source_frame(&boot, &frame, 4u, 11, &watermark), "accepts frame after rejected rollback");

    frame.source_materials_required = 0;
    check(!consume_source_frame(&boot, &frame, 5u, 11, &watermark), "rejects non-source-required frame");
    check(watermark.render_sequence == 4u && watermark.runtime_tick == 11, "source rejection preserves watermark");
    frame = make_frame(42u, 0x53434e45u, 0x50414c31u);

    frame.map_load_token++;
    check(!consume_source_frame(&boot, &frame, 5u, 12, &watermark), "public gate rejects stale map identity");
    frame = make_frame(42u, 0x53434e45u, 0x50414c31u); frame.scene_control_hash++;
    check(!consume_source_frame(&boot, &frame, 5u, 12, &watermark), "public gate rejects stale scene identity");
    frame = make_frame(42u, 0x53434e45u, 0x50414c31u); frame.palette_hash++;
    check(!consume_source_frame(&boot, &frame, 5u, 12, &watermark), "public gate rejects stale palette identity");

    boot = make_boot(43u, 0x53434e46u, 0x50414c32u);
    frame = make_frame(43u, 0x53434e46u, 0x50414c32u);
    check(consume_source_frame(&boot, &frame, 1u, 0, &watermark), "accepts fresh map identity with reset sequence and tick");
    check(watermark.map_load_token == 43u && watermark.scene_control_hash == 0x53434e46u && watermark.palette_hash == 0x50414c32u, "rebinds watermark to fresh identity");
    check(!consume_source_frame(&boot, &frame, 0u, 0, &watermark), "rejects unsequenced frame");
    check(!consume_source_frame(&boot, &frame, 2u, -1, &watermark), "rejects negative tick");
    frame.valid = 0;
    check(!consume_source_frame(&boot, &frame, 2u, 1, &watermark), "rejects invalid frame");
    frame = make_frame(43u, 0x53434e46u, 0x50414c32u);
    check(!consume_source_frame(NULL, &frame, 2u, 1, &watermark) &&
          !consume_source_frame(&boot, NULL, 2u, 1, &watermark),
          "rejects absent handoff inputs without fallback");
    return failures ? 1 : 0;
}
