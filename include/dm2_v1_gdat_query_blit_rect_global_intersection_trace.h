#ifndef FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_GLOBAL_INTERSECTION_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_GLOBAL_INTERSECTION_TRACE_H

#include "dm2_v1_gdat_query_blit_rect_global_clip_trace.h"

/* c_xrect.cpp:446-470, bounded to the authenticated global-clip mode-1 path. */
typedef struct {
    int valid;
    int no_draw;
    uint16_t source_x;
    uint16_t source_y;
    uint16_t destination_x;
    uint16_t destination_y;
    uint16_t destination_width;
    uint16_t destination_height;
    uint32_t global_clip_hash;
    uint32_t mode1_trace_hash;
    uint32_t material_handoff_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt;

int dm2_v1_gdat_query_blit_rect_global_intersection_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectGlobalClipTraceReceipt *global_clip,
    const DM2_V1_GdatQueryBlitRectMode1TraceReceipt *mode1_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt *out);

#endif
