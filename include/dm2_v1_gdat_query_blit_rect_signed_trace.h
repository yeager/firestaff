#ifndef FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_SIGNED_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_SIGNED_TRACE_H

#include "dm2_v1_gdat_draw_picst_rect_trace.h"

/*
 * Bounded c_xrect.cpp:228-276 signed-root path.  The receipt retains only the
 * source-proven x0/y0 offset before crdecode; it never claims a final rect.
 */
typedef struct {
    int valid;
    int no_draw;
    uint16_t rectangle_node;
    uint8_t mode1;
    uint16_t offset_x;
    uint16_t offset_y;
    uint32_t source_rect_hash;
    uint32_t rectangle_node_hash;
    uint32_t identity_hash;
} DM2_V1_GdatQueryBlitRectSignedTraceReceipt;

int dm2_v1_gdat_query_blit_rect_signed_trace_receipt_build(
    const DM2_V1_GdatDrawPicstRectTrace *source_rect,
    int16_t query1,
    int16_t query2,
    uint8_t mode1,
    uint16_t mode2,
    int16_t datax,
    int16_t datay,
    int16_t input_x,
    int16_t input_y,
    uint32_t rectangle_node_hash,
    int bitmap_present,
    DM2_V1_GdatQueryBlitRectSignedTraceReceipt *out);

#endif
