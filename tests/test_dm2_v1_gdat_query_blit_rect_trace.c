#include "dm2_v1_gdat_query_blit_rect_trace.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_GdatDrawPicstRectTrace source_rect = {0};
    DM2_V1_GdatQueryBlitRectTraceReceipt receipt;
    int ok;

    source_rect.valid = 1;
    source_rect.no_draw = 1;
    source_rect.identity_hash = 0x11223344u;

    ok = dm2_v1_gdat_query_blit_rect_trace_receipt_build(
             &source_rect, 0x120, -1, 1, 0, 12, 24, 0x55667788u, 1,
             &receipt) &&
         receipt.valid && receipt.no_draw && receipt.rectangle_node == 0x120 &&
         receipt.mode1 == 1 && receipt.initial_x == 12 && receipt.initial_y == 24 &&
         receipt.source_rect_hash == source_rect.identity_hash;

    ok &= !dm2_v1_gdat_query_blit_rect_trace_receipt_build(
        &source_rect, -2, -1, 1, 0, 12, 24, 0x55667788u, 1, &receipt);
    ok &= !dm2_v1_gdat_query_blit_rect_trace_receipt_build(
        &source_rect, 0x120, 3, 1, 0, 12, 24, 0x55667788u, 1, &receipt);
    ok &= !dm2_v1_gdat_query_blit_rect_trace_receipt_build(
        &source_rect, 0x120, -1, 9, 0, 12, 24, 0x55667788u, 1, &receipt);
    ok &= !dm2_v1_gdat_query_blit_rect_trace_receipt_build(
        &source_rect, 0x120, -1, 1, 5, 12, 24, 0x55667788u, 1, &receipt);
    ok &= !dm2_v1_gdat_query_blit_rect_trace_receipt_build(
        &source_rect, 0x120, -1, 1, 0, 12, 24, 0, 1, &receipt);

    printf("%s dm2_v1_gdat_query_blit_rect_trace\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
