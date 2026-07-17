#include "dm2_v1_gdat_draw_picst_surface_address_trace.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt intersection = {0};
    DM2_V1_GdatMaterializationHandoff material_handoff = {0};
    DM2_V1_GdatDrawPicstSurfaceAddressTraceReceipt receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t pixels[400];
    int ok;

    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    intersection.valid = 1;
    intersection.no_draw = 1;
    intersection.source_x = 5;
    intersection.source_y = 0;
    intersection.destination_x = 10;
    intersection.destination_y = 25;
    intersection.destination_width = 15;
    intersection.destination_height = 20;
    intersection.material_handoff_hash = 0x11223344u;
    intersection.surface_generation = viewport.surface_snapshot.generation;
    intersection.identity_hash = 0x55667788u;
    material_handoff.valid = 1;
    material_handoff.no_draw = 1;
    material_handoff.material_bytes = pixels;
    material_handoff.width = 20;
    material_handoff.height = 20;
    material_handoff.stride = 20;
    material_handoff.surface_generation = viewport.surface_snapshot.generation;
    material_handoff.identity_hash = intersection.material_handoff_hash;

    ok = dm2_v1_gdat_draw_picst_surface_address_trace_receipt_build(
             &intersection, &material_handoff, &viewport, 1, 8, 1, -1,
             0x99aabbccu, &receipt) &&
         receipt.valid && receipt.no_draw && receipt.source_bytes == pixels &&
         receipt.destination_bytes == framebuffer && receipt.source_stride == 20 &&
         receipt.destination_stride == DM2_VP_WIDTH && receipt.source_row_offset == 5 &&
         receipt.destination_row_offset == 25u * DM2_VP_WIDTH + 10u;

    ok &= !dm2_v1_gdat_draw_picst_surface_address_trace_receipt_build(
        &intersection, &material_handoff, &viewport, 1, 4, 1, -1,
        0x99aabbccu, &receipt);
    ok &= !dm2_v1_gdat_draw_picst_surface_address_trace_receipt_build(
        &intersection, &material_handoff, &viewport, 1, 8, 0, -1,
        0x99aabbccu, &receipt);
    material_handoff.stride = 21;
    ok &= !dm2_v1_gdat_draw_picst_surface_address_trace_receipt_build(
        &intersection, &material_handoff, &viewport, 1, 8, 1, -1,
        0x99aabbccu, &receipt);
    material_handoff.stride = 20;
    ++viewport.surface_snapshot.generation;
    ok &= !dm2_v1_gdat_draw_picst_surface_address_trace_receipt_build(
        &intersection, &material_handoff, &viewport, 1, 8, 1, -1,
        0x99aabbccu, &receipt);

    printf("%s dm2_v1_gdat_draw_picst_surface_address_trace\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
