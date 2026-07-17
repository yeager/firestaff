#ifndef FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_RECT_TRACE_H
#define FIRESTAFF_DM2_V1_GDAT_DRAW_PICST_RECT_TRACE_H
#include "dm2_v1_gdat_draw_picst_trace_receipt.h"
typedef struct { int valid,no_draw; uint16_t source_x,source_y,source_width,source_height,destination_x,destination_y,destination_width,destination_height; uint32_t trace_hash,identity_hash; } DM2_V1_GdatDrawPicstRectTrace;
int dm2_v1_gdat_draw_picst_rect_trace_build(const DM2_V1_GdatDrawPicstTraceReceipt *,int16_t,int16_t,int16_t,int16_t,int16_t,int16_t,uint16_t,uint16_t,DM2_V1_GdatDrawPicstRectTrace *);
#endif
