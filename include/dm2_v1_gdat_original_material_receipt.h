#ifndef FIRESTAFF_DM2_V1_GDAT_ORIGINAL_MATERIAL_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_ORIGINAL_MATERIAL_RECEIPT_H
#include "dm2_v1_gdat_palette_m11_consumer.h"
typedef struct { int valid,no_draw; const uint8_t *bytes; uint16_t width,height,stride; uint32_t byte_count,byte_hash,palette_consumer_hash,identity_hash; } DM2_V1_GdatOriginalMaterialReceipt;
int dm2_v1_gdat_original_material_receipt_build(const DM2_V1_GdatPaletteM11ConsumerReceipt *,const uint8_t *,uint16_t,uint16_t,uint16_t,uint32_t,uint32_t,DM2_V1_GdatOriginalMaterialReceipt *);
#endif
