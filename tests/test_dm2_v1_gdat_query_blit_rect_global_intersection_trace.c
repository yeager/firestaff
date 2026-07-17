#include "dm2_v1_gdat_query_blit_rect_global_intersection_trace.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_GdatQueryBlitRectGlobalClipTraceReceipt global_clip = {0};
    DM2_V1_GdatQueryBlitRectMode1TraceReceipt mode1_trace = {0};
    DM2_V1_GdatMaterializationHandoff material_handoff = {0};
    DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t pixels[400];
    int ok;

    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    mode1_trace.valid = 1;
    mode1_trace.no_draw = 1;
    mode1_trace.decoded_x = 5;
    mode1_trace.decoded_y = 25;
    mode1_trace.bitmap_width = 20;
    mode1_trace.bitmap_height = 20;
    mode1_trace.material_handoff_hash = 0x11223344u;
    mode1_trace.surface_generation = viewport.surface_snapshot.generation;
    mode1_trace.identity_hash = 0x55667788u;
    material_handoff.valid = 1;
    material_handoff.no_draw = 1;
    material_handoff.material_bytes = pixels;
    material_handoff.width = 20;
    material_handoff.height = 20;
    material_handoff.stride = 20;
    material_handoff.surface_generation = viewport.surface_snapshot.generation;
    material_handoff.identity_hash = mode1_trace.material_handoff_hash;
    global_clip.valid = 1;
    global_clip.no_draw = 1;
    global_clip.clip_x = 10;
    global_clip.clip_y = 20;
    global_clip.clip_width = 20;
    global_clip.clip_height = 30;
    global_clip.mode1_trace_hash = mode1_trace.identity_hash;
    global_clip.material_handoff_hash = material_handoff.identity_hash;
    global_clip.surface_generation = viewport.surface_snapshot.generation;
    global_clip.identity_hash = 0x99aabbccu;

    ok = dm2_v1_gdat_query_blit_rect_global_intersection_trace_receipt_build(
             &global_clip, &mode1_trace, &material_handoff, &viewport,
             &receipt) &&
         receipt.valid && receipt.no_draw && receipt.source_x == 5 &&
         receipt.source_y == 0 && receipt.destination_x == 10 &&
         receipt.destination_y == 25 && receipt.destination_width == 15 &&
         receipt.destination_height == 20;

    ++global_clip.mode1_trace_hash;
    ok &= !dm2_v1_gdat_query_blit_rect_global_intersection_trace_receipt_build(
        &global_clip, &mode1_trace, &material_handoff, &viewport, &receipt);
    --global_clip.mode1_trace_hash;
    ++material_handoff.identity_hash;
    ok &= !dm2_v1_gdat_query_blit_rect_global_intersection_trace_receipt_build(
        &global_clip, &mode1_trace, &material_handoff, &viewport, &receipt);
    --material_handoff.identity_hash;
    mode1_trace.decoded_x = 40;
    ok &= !dm2_v1_gdat_query_blit_rect_global_intersection_trace_receipt_build(
        &global_clip, &mode1_trace, &material_handoff, &viewport, &receipt);
    mode1_trace.decoded_x = 5;
    ++viewport.surface_snapshot.generation;
    ok &= !dm2_v1_gdat_query_blit_rect_global_intersection_trace_receipt_build(
        &global_clip, &mode1_trace, &material_handoff, &viewport, &receipt);

    printf("%s dm2_v1_gdat_query_blit_rect_global_intersection_trace\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
