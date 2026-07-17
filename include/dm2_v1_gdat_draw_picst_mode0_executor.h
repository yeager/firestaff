#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_MODE0_EXECUTOR_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_MODE0_EXECUTOR_H

#include "dm2_v1_gdat_draw_picst_masked_consume_trace.h"

/* Narrow native 8-bit execution of the fully authenticated BLITMODE0 path. */
int dm2_v1_gdat_draw_picst_mode0_execute(
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *masked_trace,
    const DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt *index_trace,
    const DM2_V1_GdatDrawPicstPaletteWriteTraceReceipt *write_trace,
    const DM2_V1_GdatDrawPicstMaskedConsumeTraceReceipt *consume_trace,
    DM2_V1_ViewportState *owner);

#endif
