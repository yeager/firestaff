#ifndef FIRESTAFF_DM2_V1_GDAT_ORIGINAL_PALETTE_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_ORIGINAL_PALETTE_RECEIPT_H
#include "dm2_v1_gdat_draw_temp_palette_surface_receipt.h"
typedef struct { int valid,no_draw; const uint8_t *bytes; uint16_t byte_count; uint32_t byte_hash,palette_surface_hash,identity_hash; } DM2_V1_GdatOriginalPaletteReceipt;
int dm2_v1_gdat_original_palette_receipt_build(const DM2_V1_GdatDrawTempPaletteSurfaceReceipt *,const uint8_t *,uint16_t,uint32_t,DM2_V1_GdatOriginalPaletteReceipt *);
#endif
