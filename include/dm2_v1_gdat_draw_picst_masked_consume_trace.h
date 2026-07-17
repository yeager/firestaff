#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_MASKED_CONSUME_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_MASKED_CONSUME_TRACE_H

#include "dm2_v1_gdat_draw_picst_palette_write_trace.h"

/* c_gfx_blit.cpp:675-682: source++, conditional write, destination++. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t alpha_index;
    uint8_t source_step;
    uint8_t destination_step;
    uint8_t conditional_write_before_destination_step;
    uint16_t row_count;
    uint16_t pixels_per_row;
    uint32_t source_first_row;
    uint32_t source_last_row;
    uint32_t destination_first_row;
    uint32_t destination_last_row;
    uint32_t palette_write_trace_hash;
    uint32_t identity_hash;
} DM2_V1_GdatDrawPicstMaskedConsumeTraceReceipt;

int dm2_v1_gdat_draw_picst_masked_consume_trace_receipt_build(
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *masked_trace,
    const DM2_V1_GdatDrawPicstPaletteWriteTraceReceipt *write_trace,
    DM2_V1_GdatDrawPicstMaskedConsumeTraceReceipt *out);

#endif
