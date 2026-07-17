#ifndef FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_TRACE_H

#include "dm2_v1_gdat_draw_picst_rect_trace.h"

/*
 * Bounded c_xrect.cpp:217-280 trace.  This receipt covers only the root,
 * unsigned, unchained rectangle node.  It deliberately has no final clip or
 * destination rectangle: those are established later by QUERY_BLIT_RECT.
 */
typedef struct {
    int valid;
    int no_draw;
    uint16_t rectangle_node;
    uint8_t mode1;
    uint16_t initial_x;
    uint16_t initial_y;
    uint32_t source_rect_hash;
    uint32_t rectangle_node_hash;
    uint32_t identity_hash;
} DM2_V1_GdatQueryBlitRectTraceReceipt;

int dm2_v1_gdat_query_blit_rect_trace_receipt_build(
    const DM2_V1_GdatDrawPicstRectTrace *source_rect,
    int16_t query1,
    int16_t query2,
    uint8_t mode1,
    uint16_t mode2,
    int16_t datax,
    int16_t datay,
    uint32_t rectangle_node_hash,
    int bitmap_present,
    DM2_V1_GdatQueryBlitRectTraceReceipt *out);

#endif
