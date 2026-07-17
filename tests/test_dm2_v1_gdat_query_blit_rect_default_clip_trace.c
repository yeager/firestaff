#include "dm2_v1_gdat_query_blit_rect_default_clip_trace.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_GdatQueryBlitRectMode1TraceReceipt mode1_trace = {0};
    DM2_V1_GdatMaterializationHandoff material_handoff = {0};
    DM2_V1_GdatQueryBlitRectDefaultClipTraceReceipt receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t pixels[16];
    int ok;

    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    mode1_trace.valid = 1;
    mode1_trace.no_draw = 1;
    mode1_trace.decoded_x = 15;
    mode1_trace.decoded_y = 29;
    mode1_trace.bitmap_width = 4;
    mode1_trace.bitmap_height = 4;
    mode1_trace.material_handoff_hash = 0x11223344u;
    mode1_trace.surface_generation = viewport.surface_snapshot.generation;
    mode1_trace.identity_hash = 0x55667788u;
    material_handoff.valid = 1;
    material_handoff.no_draw = 1;
    material_handoff.material_bytes = pixels;
    material_handoff.width = 4;
    material_handoff.height = 4;
    material_handoff.stride = 4;
    material_handoff.surface_generation = viewport.surface_snapshot.generation;
    material_handoff.identity_hash = mode1_trace.material_handoff_hash;

    ok = dm2_v1_gdat_query_blit_rect_default_clip_trace_receipt_build(
             &mode1_trace, &material_handoff, &viewport, 0, 0, &receipt) &&
         receipt.valid && receipt.no_draw && receipt.rect_x == 15 &&
         receipt.rect_y == 29 && receipt.rect_width == 4 && receipt.rect_height == 4;

    ok &= !dm2_v1_gdat_query_blit_rect_default_clip_trace_receipt_build(
        &mode1_trace, &material_handoff, &viewport, 1, 0, &receipt);
    ok &= !dm2_v1_gdat_query_blit_rect_default_clip_trace_receipt_build(
        &mode1_trace, &material_handoff, &viewport, 0, 3, &receipt);
    mode1_trace.decoded_x = 9999;
    ok &= !dm2_v1_gdat_query_blit_rect_default_clip_trace_receipt_build(
        &mode1_trace, &material_handoff, &viewport, 0, 0, &receipt);
    mode1_trace.decoded_x = 15;
    ++viewport.surface_snapshot.generation;
    ok &= !dm2_v1_gdat_query_blit_rect_default_clip_trace_receipt_build(
        &mode1_trace, &material_handoff, &viewport, 0, 0, &receipt);

    printf("%s dm2_v1_gdat_query_blit_rect_default_clip_trace\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
