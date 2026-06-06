#include "csb_v1_viewport_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("ok %s=%d\n", label, got);
    return 1;
}

static int expect_u32(const char *label, uint32_t got, uint32_t want)
{
    if (got != want) {
        printf("FAIL %s got=0x%08x want=0x%08x\n", label,
               (unsigned int)got, (unsigned int)want);
        return 0;
    }
    printf("ok %s=0x%08x\n", label, (unsigned int)got);
    return 1;
}

static int test_layer_plan(void)
{
    int ok = 1;
    CSB_V1_ViewportCustomBackgroundLayerPlan layers[3];
    CSB_V1_ViewportCustomBackgroundLayerPlan one_layer[1];
    size_t count;

    memset(layers, 0, sizeof(layers));
    memset(one_layer, 0, sizeof(one_layer));

    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 and F0098 lines
     * 2962-3002 draw only the baseline floor/ceiling. CSBWin Viewport.cpp
     * lines 6593-6612 overlays CustomBackgrounds in large, middle, near
     * order, and gates the near layer with roomNum < 5. */
    count = csb_v1_viewport_custom_background_layer_plan_pc34(
        4, layers, sizeof(layers) / sizeof(layers[0]));
    ok &= expect_int("room4.count", (int)count, 3);
    ok &= expect_int("room4.large.layer", (int)layers[0].layer,
                     CSB_V1_CUSTOM_BACKGROUND_LAYER_LARGE);
    ok &= expect_int("room4.large.bitmap_index",
                     layers[0].bitmap_skin_def_index, 0);
    ok &= expect_int("room4.large.mask_index",
                     layers[0].mask_skin_def_index, 4);
    ok &= expect_int("room4.large.bitmap_min",
                     layers[0].bitmap_min_bytes, 7840);
    ok &= expect_int("room4.large.mask_min",
                     layers[0].mask_min_bytes, 64);
    ok &= expect_int("room4.middle.layer", (int)layers[1].layer,
                     CSB_V1_CUSTOM_BACKGROUND_LAYER_MIDDLE);
    ok &= expect_int("room4.middle.bitmap_index",
                     layers[1].bitmap_skin_def_index, 2);
    ok &= expect_int("room4.middle.mask_index",
                     layers[1].mask_skin_def_index, 6);
    ok &= expect_int("room4.middle.bitmap_min",
                     layers[1].bitmap_min_bytes, 3248);
    ok &= expect_int("room4.middle.mask_min",
                     layers[1].mask_min_bytes, 64);
    ok &= expect_int("room4.near.layer", (int)layers[2].layer,
                     CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR);
    ok &= expect_int("room4.near.bitmap_index",
                     layers[2].bitmap_skin_def_index, 1);
    ok &= expect_int("room4.near.mask_index",
                     layers[2].mask_skin_def_index, 5);
    ok &= expect_int("room4.near.bitmap_min",
                     layers[2].bitmap_min_bytes, 4144);
    ok &= expect_int("room4.near.mask_min",
                     layers[2].mask_min_bytes, 20);

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 has no CSB near-layer
     * rule. CSBWin Viewport.cpp lines 6608-6612 applies pSkinDef[1]/[5]
     * only when roomNum < 5, so room 5 keeps only large and middle. */
    count = csb_v1_viewport_custom_background_layer_plan_pc34(
        5, layers, sizeof(layers) / sizeof(layers[0]));
    ok &= expect_int("room5.count", (int)count, 2);
    ok &= expect_int("room5.large.layer", (int)layers[0].layer,
                     CSB_V1_CUSTOM_BACKGROUND_LAYER_LARGE);
    ok &= expect_int("room5.middle.layer", (int)layers[1].layer,
                     CSB_V1_CUSTOM_BACKGROUND_LAYER_MIDDLE);

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 enumerates the viewport
     * pass, while CSBWin Viewport.cpp lines 6919-7140 has sixteen
     * CustomBackgrounds room slots. Invalid synthetic room ids are rejected. */
    count = csb_v1_viewport_custom_background_layer_plan_pc34(
        16, layers, sizeof(layers) / sizeof(layers[0]));
    ok &= expect_int("room16.count", (int)count, 0);

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 baseline remains independent
     * of caller storage. CSBWin Viewport.cpp lines 6593-6612 still defines
     * the full overlay count even when the caller only asks to copy one row. */
    count = csb_v1_viewport_custom_background_layer_plan_pc34(
        4, one_layer, sizeof(one_layer) / sizeof(one_layer[0]));
    ok &= expect_int("room4.truncated_count", (int)count, 3);
    ok &= expect_int("room4.truncated_first", (int)one_layer[0].layer,
                     CSB_V1_CUSTOM_BACKGROUND_LAYER_LARGE);

    return ok;
}

static int test_aligned_mask_composite(void)
{
    int ok = 1;
    uint32_t bitmap_a[9] = {
        32u,
        0u, 0u, 0u, 0u,
        0u, 0u, 0x12345678u, 0x87654321u
    };
    uint32_t bitmap_b[9] = {
        32u,
        0u, 0u, 0u, 0u,
        0u, 0u, 0x0f0e0d0cu, 0x01020304u
    };
    uint32_t viewport[56];
    const uint16_t mask_low[1] = { 0x00ffu };
    const uint16_t mask_high[1] = { 0xff00u };
    CSB_V1_ViewportCustomBackgroundMask mask;
    int copied;

    for (size_t i = 0; i < sizeof(viewport) / sizeof(viewport[0]); ++i) {
        viewport[i] = 0xccccccccu;
    }
    viewport[30] = 0xaaaaaaaau;
    viewport[31] = 0xbbbbbbbbu;

    mask.src_x = 16;
    mask.src_y = 1;
    mask.dst_x = 16;
    mask.dst_y = 1;
    mask.width = 16;
    mask.height = 1;
    mask.mask_words = mask_low;
    mask.mask_word_count = 1;

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 draws base pixels first.
     * CSBWin Viewport.cpp lines 6444-6470 then expands each 16-bit
     * ApplyBackground mask word across two 32-bit viewport words. */
    copied = csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
        &mask, bitmap_a, sizeof(bitmap_a) / sizeof(bitmap_a[0]),
        viewport, sizeof(viewport) / sizeof(viewport[0]), 224);
    ok &= expect_int("mask.low.copied_words", copied, 2);
    ok &= expect_u32("mask.low.word0", viewport[30], 0xaa34aa78u);
    ok &= expect_u32("mask.low.word1", viewport[31], 0xbb65bb21u);
    ok &= expect_u32("mask.low.neighbor", viewport[29], 0xccccccccu);

    mask.mask_words = mask_high;
    mask.mask_word_count = 1;

    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 keeps the backdrop as the
     * target for later CSB-only overlays. CSBWin Viewport.cpp lines
     * 6593-6603 applies large then middle, so a second mask composes over
     * the first instead of clearing untouched destination bits. */
    copied = csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
        &mask, bitmap_b, sizeof(bitmap_b) / sizeof(bitmap_b[0]),
        viewport, sizeof(viewport) / sizeof(viewport[0]), 224);
    ok &= expect_int("mask.high.copied_words", copied, 2);
    ok &= expect_u32("mask.high.word0", viewport[30], 0x0f340d78u);
    ok &= expect_u32("mask.high.word1", viewport[31], 0x01650321u);

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 has no unaligned background
     * overlay. CSBWin Viewport.cpp lines 6454-6459 dispatches unaligned
     * masks to ApplyBackground3, which is intentionally outside this slice. */
    mask.src_x = 8;
    copied = csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
        &mask, bitmap_b, sizeof(bitmap_b) / sizeof(bitmap_b[0]),
        viewport, sizeof(viewport) / sizeof(viewport[0]), 224);
    ok &= expect_int("mask.unaligned.deferred", copied, -2);

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8542 bounds viewport drawing to
     * the 224-pixel view area. CSBWin Viewport.cpp lines 6451-6466 rejects
     * null data, non-16-pixel widths, and overlays that do not fit. */
    mask.src_x = 16;
    mask.dst_x = 224;
    copied = csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
        &mask, bitmap_b, sizeof(bitmap_b) / sizeof(bitmap_b[0]),
        viewport, sizeof(viewport) / sizeof(viewport[0]), 224);
    ok &= expect_int("mask.bounds.reject", copied, 0);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_custom_background_mask_pc34_compat\n");
    printf("sourceEvidence=%s\n", csb_v1_viewport_source_evidence());

    ok &= test_layer_plan();
    ok &= test_aligned_mask_composite();

    return ok ? 0 : 1;
}
