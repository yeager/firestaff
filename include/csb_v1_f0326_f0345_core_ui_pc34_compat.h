#ifndef FIRESTAFF_CSB_V1_F0326_F0345_CORE_UI_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0326_F0345_CORE_UI_PC34_COMPAT_H

#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

#define CSB_V1_F0326_F0345_SOURCE_MASK 0x000fffffu

typedef struct {
    int valid;
    uint16_t thing;
    int thing_type;
    int thing_index;
    uint32_t thing_record_fnv1a;
    uint32_t source_bound_mask;
    int projectile_runtime_owner_required;
    int time_effect_runtime_owner_required;
    int inventory_runtime_owner_required;
    int panel_material_bound;
    const char *source_evidence;
} CSB_V1_F0326F0345CoreUiReceiptPc34;

/* ReDMCSB CHAMPION.C F0326-F0331 and INVNTORY.C F0332-F0345. This is an
 * authenticated PC34 admission receipt only; all projectile, clock, chest,
 * palette, text/font, and panel mutation/rendering remains owner-bound. */
int csb_v1_f0326_f0345_core_ui_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    uint16_t thing, CSB_V1_F0326F0345CoreUiReceiptPc34 *out_receipt);

#endif
