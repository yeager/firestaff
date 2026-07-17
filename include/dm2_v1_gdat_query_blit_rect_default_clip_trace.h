#ifndef FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_DEFAULT_CLIP_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_DEFAULT_CLIP_TRACE_H

#include "dm2_v1_gdat_query_blit_rect_mode1_trace.h"

/* c_xrect.cpp:239 and :438-470, before any surface-specific destination. */
typedef struct {
    int valid;
    int no_draw;
    int16_t rect_x;
    int16_t rect_y;
    uint16_t rect_width;
    uint16_t rect_height;
    uint32_t mode1_trace_hash;
    uint32_t material_handoff_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatQueryBlitRectDefaultClipTraceReceipt;

int dm2_v1_gdat_query_blit_rect_default_clip_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectMode1TraceReceipt *mode1_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    int global_clip_override,
    uint16_t terminal_query_node,
    DM2_V1_GdatQueryBlitRectDefaultClipTraceReceipt *out);

#endif
