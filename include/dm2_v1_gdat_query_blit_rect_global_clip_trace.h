#ifndef FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_GLOBAL_CLIP_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_QUERY_BLIT_RECT_GLOBAL_CLIP_TRACE_H

#include "dm2_v1_gdat_query_blit_rect_mode1_trace.h"

/* c_gui_vp.cpp:570-573 -> c_xrect.cpp:438-439.  No intersection is done. */
typedef struct {
    int valid;
    int no_draw;
    uint16_t clip_x;
    uint16_t clip_y;
    uint16_t clip_width;
    uint16_t clip_height;
    uint32_t trim_call_hash;
    uint32_t mode1_trace_hash;
    uint32_t material_handoff_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatQueryBlitRectGlobalClipTraceReceipt;

int dm2_v1_gdat_query_blit_rect_global_clip_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectMode1TraceReceipt *mode1_trace,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    int global_clip_enabled,
    int16_t trim_x,
    int16_t trim_y,
    int16_t right_trim,
    int16_t bottom_trim,
    uint32_t trim_call_hash,
    DM2_V1_GdatQueryBlitRectGlobalClipTraceReceipt *out);

#endif
