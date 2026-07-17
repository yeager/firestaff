#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_PALETTE_INDEX_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_PALETTE_INDEX_TRACE_H

#include "dm2_v1_gdat_draw_picst_mode0_palette_mask_trace.h"

/* c_gfx_blit.cpp:39-42,675-682: mask compare precedes PAL256 lookup. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t alpha_index;
    uint16_t palette_index_domain;
    uint16_t row_count;
    uint16_t pixels_per_row;
    uint32_t source_first_row;
    uint32_t source_last_row;
    uint32_t palette_hash;
    uint32_t masked_palette_trace_hash;
    uint32_t identity_hash;
} DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt;

int dm2_v1_gdat_draw_picst_palette_index_trace_receipt_build(
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *masked_trace,
    DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt *out);

#endif
