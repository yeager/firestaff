#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_ROW_TRAVERSAL_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_ROW_TRAVERSAL_TRACE_H

#include "dm2_v1_gdat_draw_picst_surface_address_trace.h"

/* c_gfx_blit.cpp:604-656, BLITMODE0/default only.  No blit is performed. */
typedef struct {
    int valid;
    int no_draw;
    uint16_t row_count;
    uint16_t bytes_per_row;
    uint32_t source_first_row;
    uint32_t source_last_row;
    uint32_t source_end_exclusive;
    uint32_t destination_first_row;
    uint32_t destination_last_row;
    uint32_t destination_end_exclusive;
    uint32_t addressed_pixel_count;
    uint32_t surface_address_hash;
    uint32_t material_handoff_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatDrawPicstRowTraversalTraceReceipt;

int dm2_v1_gdat_draw_picst_row_traversal_trace_receipt_build(
    const DM2_V1_GdatDrawPicstSurfaceAddressTraceReceipt *surface_address,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    uint8_t blit_mode,
    DM2_V1_GdatDrawPicstRowTraversalTraceReceipt *out);

#endif
