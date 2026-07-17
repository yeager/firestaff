#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_PALETTE_WRITE_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_PALETTE_WRITE_TRACE_H

#include "dm2_v1_gdat_draw_picst_palette_index_trace.h"

/* c_gfx_pal.h:23-44; c_gfx_pixel.h:54-98; c_gfx_blit.cpp:675-682. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t palette_entry_bytes;
    uint16_t palette_entry_count;
    uint8_t alpha_index;
    uint16_t row_count;
    uint16_t writes_per_row;
    uint32_t destination_first_row;
    uint32_t destination_last_row;
    uint32_t destination_end_exclusive;
    uint32_t destination_surface_generation;
    uint32_t palette_index_trace_hash;
    uint32_t masked_palette_trace_hash;
    uint32_t identity_hash;
} DM2_V1_GdatDrawPicstPaletteWriteTraceReceipt;

int dm2_v1_gdat_draw_picst_palette_write_trace_receipt_build(
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *masked_trace,
    const DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt *index_trace,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatDrawPicstPaletteWriteTraceReceipt *out);

#endif
