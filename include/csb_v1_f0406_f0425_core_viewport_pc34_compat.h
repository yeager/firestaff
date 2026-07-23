#ifndef FIRESTAFF_CSB_V1_F0406_F0425_CORE_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0406_F0425_CORE_VIEWPORT_PC34_COMPAT_H

#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

#define CSB_V1_F0406_F0425_SOURCE_MASK 0x000fffffu

typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    int square_type;
    uint32_t source_bound_mask;
    int menu_action_owner_required;
    int save_owner_required;
    int dialog_owner_required;
    const char *source_evidence;
} CSB_V1_F0406F0425CoreViewportReceiptPc34;

/* Read-only ReDMCSB MENUS.C/SAVEUTIL.C/DIALOG.C F0406-F0425 admission. */
int csb_v1_f0406_f0425_core_viewport_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0406F0425CoreViewportReceiptPc34 *out_receipt);

#endif
