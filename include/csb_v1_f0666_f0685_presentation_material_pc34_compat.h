#ifndef FIRESTAFF_CSB_V1_F0666_F0685_PRESENTATION_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0666_F0685_PRESENTATION_MATERIAL_PC34_COMPAT_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

#define CSB_V1_F0666_F0685_SOURCE_MASK 0x000fffffu
typedef struct {
    int valid, map_index, map_x, map_y, square_type;
    uint32_t graphics_entry_count, source_bound_mask;
    int endgame_owner_required, text_input_owner_required;
    int viewport_bitmap_owner_required, pixel_copy_owner_required;
    const char *source_evidence;
} CSB_V1_F0666F0685PresentationMaterialReceiptPc34;

int csb_v1_f0666_f0685_presentation_material_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0666F0685PresentationMaterialReceiptPc34 *out_receipt);

#endif
