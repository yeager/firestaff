#ifndef FIRESTAFF_DM2_V1_GDAT_MATERIALIZATION_HANDOFF_H
#define FIRESTAFF_DM2_V1_GDAT_MATERIALIZATION_HANDOFF_H
#include "dm2_v1_gdat_material_palette_pair_receipt.h"
typedef struct { int valid,no_draw; const uint8_t *material_bytes,*palette_bytes; uint16_t width,height,stride,palette_byte_count; uint32_t material_byte_count,pair_hash,surface_generation,identity_hash; } DM2_V1_GdatMaterializationHandoff;
int dm2_v1_gdat_materialization_handoff_build(const DM2_V1_GdatMaterialPalettePairReceipt *,const DM2_V1_ViewportState *,DM2_V1_GdatMaterializationHandoff *);
#endif
