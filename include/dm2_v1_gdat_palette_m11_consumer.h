#ifndef FIRESTAFF_DM2_V1_GDAT_PALETTE_M11_CONSUMER_H
#define FIRESTAFF_DM2_V1_GDAT_PALETTE_M11_CONSUMER_H
#include "dm2_v1_gdat_original_palette_receipt.h"
typedef struct { int valid,no_draw; const uint8_t *palette_bytes; uint16_t palette_byte_count; uint32_t palette_hash,source_identity,surface_generation,identity_hash; } DM2_V1_GdatPaletteM11ConsumerReceipt;
int dm2_v1_gdat_palette_m11_consumer_receipt_build(const DM2_V1_GdatOriginalPaletteReceipt *,const DM2_V1_ViewportState *,DM2_V1_GdatPaletteM11ConsumerReceipt *);
#endif
