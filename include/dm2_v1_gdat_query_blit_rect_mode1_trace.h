#ifndef FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_MODE1_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_MODE1_TRACE_H

#include "dm2_v1_gdat_query_blit_rect_signed_trace.h"

/* c_xrect.cpp:162-211 and :426-436, restricted to crdecode mode 1. */
typedef struct {
    int valid;
    int no_draw;
    uint16_t decoded_x;
    uint16_t decoded_y;
    uint16_t bitmap_width;
    uint16_t bitmap_height;
    uint32_t signed_trace_hash;
    uint32_t material_handoff_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatQueryBlitRectMode1TraceReceipt;

int dm2_v1_gdat_query_blit_rect_mode1_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectSignedTraceReceipt *signed_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatQueryBlitRectMode1TraceReceipt *out);

#endif
