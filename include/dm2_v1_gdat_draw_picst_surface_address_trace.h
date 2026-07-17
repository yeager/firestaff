#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_SURFACE_ADDRESS_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_SURFACE_ADDRESS_TRACE_H

#include "dm2_v1_gdat_query_blit_rect_global_intersection_trace.h"

/* c_image.cpp:293-335 and c_gfx_blit.cpp:604-656, no pixel write. */
typedef struct {
    int valid;
    int no_draw;
    const uint8_t *source_bytes;
    uint8_t *destination_bytes;
    uint16_t source_stride;
    uint16_t destination_stride;
    uint32_t source_row_offset;
    uint32_t destination_row_offset;
    uint16_t width;
    uint16_t height;
    uint32_t intersection_hash;
    uint32_t material_handoff_hash;
    uint32_t draw_picst_image_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatDrawPicstSurfaceAddressTraceReceipt;

int dm2_v1_gdat_draw_picst_surface_address_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt *intersection,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    int destination_is_gfxsys_screen,
    uint8_t source_resolution,
    int palette_absent,
    int16_t alpha_mask,
    uint32_t draw_picst_image_hash,
    DM2_V1_GdatDrawPicstSurfaceAddressTraceReceipt *out);

#endif
