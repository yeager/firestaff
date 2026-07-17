#include "dm2_v1_gdat_draw_picst_row_traversal_trace.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_GdatDrawPicstSurfaceAddressTraceReceipt surface_address = {0};
    DM2_V1_GdatMaterializationHandoff material_handoff = {0};
    DM2_V1_GdatDrawPicstRowTraversalTraceReceipt receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t pixels[400];
    int ok;

    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    material_handoff.valid = 1;
    material_handoff.no_draw = 1;
    material_handoff.material_bytes = pixels;
    material_handoff.width = 20;
    material_handoff.height = 20;
    material_handoff.stride = 20;
    material_handoff.material_byte_count = sizeof(pixels);
    material_handoff.surface_generation = viewport.surface_snapshot.generation;
    material_handoff.identity_hash = 0x11223344u;
    surface_address.valid = 1;
    surface_address.no_draw = 1;
    surface_address.source_bytes = pixels;
    surface_address.destination_bytes = framebuffer;
    surface_address.source_stride = 20;
    surface_address.destination_stride = DM2_VP_WIDTH;
    surface_address.source_row_offset = 5;
    surface_address.destination_row_offset = 25u * DM2_VP_WIDTH + 10u;
    surface_address.width = 15;
    surface_address.height = 20;
    surface_address.material_handoff_hash = material_handoff.identity_hash;
    surface_address.surface_generation = viewport.surface_snapshot.generation;
    surface_address.identity_hash = 0x55667788u;

    ok = dm2_v1_gdat_draw_picst_row_traversal_trace_receipt_build(
             &surface_address, &material_handoff, &viewport, 0, &receipt) &&
         receipt.valid && receipt.no_draw && receipt.row_count == 20 &&
         receipt.bytes_per_row == 15 && receipt.source_first_row == 5 &&
         receipt.source_last_row == 385 && receipt.source_end_exclusive == 400 &&
         receipt.destination_last_row == 14090 &&
         receipt.destination_end_exclusive == 14105 &&
         receipt.addressed_pixel_count == 300;

    ok &= !dm2_v1_gdat_draw_picst_row_traversal_trace_receipt_build(
        &surface_address, &material_handoff, &viewport, 1, &receipt);
    material_handoff.material_byte_count = 399;
    ok &= !dm2_v1_gdat_draw_picst_row_traversal_trace_receipt_build(
        &surface_address, &material_handoff, &viewport, 0, &receipt);
    material_handoff.material_byte_count = sizeof(pixels);
    surface_address.destination_bytes = pixels;
    ok &= !dm2_v1_gdat_draw_picst_row_traversal_trace_receipt_build(
        &surface_address, &material_handoff, &viewport, 0, &receipt);
    surface_address.destination_bytes = framebuffer;
    ++viewport.surface_snapshot.generation;
    ok &= !dm2_v1_gdat_draw_picst_row_traversal_trace_receipt_build(
        &surface_address, &material_handoff, &viewport, 0, &receipt);

    printf("%s dm2_v1_gdat_draw_picst_row_traversal_trace\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
