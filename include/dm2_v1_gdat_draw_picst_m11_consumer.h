#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_M11_CONSUMER_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_M11_CONSUMER_H

#include "dm2_v1_gdat_draw_picst_mode0_executor.h"

/* DM2-owned M11 boundary for the sole admitted native DRAW_PICST branch. */
int dm2_v1_gdat_draw_picst_m11_consume(
    const DM2_V1_GdatMaterializationHandoff *material_handoff,
    const DM2_V1_GdatDrawPicstMode0PaletteMaskTraceReceipt *masked_trace,
    const DM2_V1_GdatDrawPicstPaletteIndexTraceReceipt *index_trace,
    const DM2_V1_GdatDrawPicstPaletteWriteTraceReceipt *write_trace,
    const DM2_V1_GdatDrawPicstMaskedConsumeTraceReceipt *consume_trace,
    DM2_V1_ViewportState *owner);

#endif
