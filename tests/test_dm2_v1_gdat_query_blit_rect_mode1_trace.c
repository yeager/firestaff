#include "dm2_v1_gdat_query_blit_rect_mode1_trace.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_GdatQueryBlitRectSignedTraceReceipt signed_trace = {0};
    DM2_V1_GdatMaterializationHandoff material_handoff = {0};
    DM2_V1_GdatQueryBlitRectMode1TraceReceipt receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t pixels[16];
    int ok;

    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    signed_trace.valid = 1;
    signed_trace.no_draw = 1;
    signed_trace.mode1 = 1;
    signed_trace.offset_x = 15;
    signed_trace.offset_y = 29;
    signed_trace.identity_hash = 0x11223344u;
    material_handoff.valid = 1;
    material_handoff.no_draw = 1;
    material_handoff.material_bytes = pixels;
    material_handoff.width = 4;
    material_handoff.height = 4;
    material_handoff.stride = 4;
    material_handoff.surface_generation = viewport.surface_snapshot.generation;
    material_handoff.identity_hash = 0x55667788u;

    ok = dm2_v1_gdat_query_blit_rect_mode1_trace_receipt_build(
             &signed_trace, &material_handoff, &viewport, &receipt) &&
         receipt.valid && receipt.no_draw && receipt.decoded_x == 15 &&
         receipt.decoded_y == 29 && receipt.bitmap_width == 4 &&
         receipt.bitmap_height == 4;

    signed_trace.mode1 = 2;
    ok &= !dm2_v1_gdat_query_blit_rect_mode1_trace_receipt_build(
        &signed_trace, &material_handoff, &viewport, &receipt);
    signed_trace.mode1 = 1;
    material_handoff.material_bytes = NULL;
    ok &= !dm2_v1_gdat_query_blit_rect_mode1_trace_receipt_build(
        &signed_trace, &material_handoff, &viewport, &receipt);
    material_handoff.material_bytes = pixels;
    ++viewport.surface_snapshot.generation;
    ok &= !dm2_v1_gdat_query_blit_rect_mode1_trace_receipt_build(
        &signed_trace, &material_handoff, &viewport, &receipt);

    printf("%s dm2_v1_gdat_query_blit_rect_mode1_trace\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
