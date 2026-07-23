#ifndef FIRESTAFF_CSB_V1_F0600_F0620_CORE_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0600_F0620_CORE_MATERIAL_PC34_COMPAT_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

#define CSB_V1_F0600_F0620_SOURCE_MASK 0x001fffffu

typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    int square_type;
    uint32_t graphics_entry_count;
    uint32_t source_bound_mask;
    int dialog_owner_required;
    int memory_owner_required;
    int viewport_owner_required;
    int action_name_owner_required;
    const char *source_evidence;
} CSB_V1_F0600F0620CoreMaterialReceiptPc34;

/* Read-only ReDMCSB F0600-F0620 material admission. */
int csb_v1_f0600_f0620_core_material_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0600F0620CoreMaterialReceiptPc34 *out_receipt);

#endif
