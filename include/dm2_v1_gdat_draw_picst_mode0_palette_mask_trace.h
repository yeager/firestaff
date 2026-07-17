#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_MODE0_PALETTE_MASK_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_MODE0_PALETTE_MASK_TRACE_H

#include "dm2_v1_gdat_query_blit_rect_global_intersection_trace.h"

/* c_image.h:45-70 and c_gfx_blit.cpp:655-760, BLITMODE0 masked/xlat only. */
typedef struct {
    int valid;
    int no_draw;
    const uint8_t *source_bytes;
    const uint8_t *palette_bytes;
    uint8_t *destination_bytes;
    uint8_t alpha_index;
    uint16_t row_count;
    uint16_t bytes_per_row;
    uint32_t source_first_row;
    uint32_t source_last_row;
    uint32_t source_end_exclusive;
    uint32_t destination_first_row;
    uint32_t destination_last_row;
    uint32_t destination_end_exclusive;
    uint32_t palette_hash;
    uint32_t intersection_hash;
    uint32_t material_handoff_hash;
    uint32_t surface_generation;
    uint32_t identity_hash;
} DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt;

int dm2_v1_gdat_draw_picst_mode0_palette_mask_trace_receipt_build(
    const DM2_V1_GdatQueryBlitRectGlobalIntersectionTraceReceipt *intersection,
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_ViewportState *owner,
    uint8_t blit_mode,
    int16_t alpha_mask,
    uint16_t image_palette_color_count,
    uint32_t palette_hash,
    DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *out);

#endif
