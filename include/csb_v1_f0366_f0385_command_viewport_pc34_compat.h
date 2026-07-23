#ifndef FIRESTAFF_CSB_V1_F0366_F0385_COMMAND_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0366_F0385_COMMAND_VIEWPORT_PC34_COMPAT_H

#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

#define CSB_V1_F0366_F0385_SOURCE_MASK 0x000fffffu
typedef struct {
    int valid, map_index, map_x, map_y, square_type;
    uint32_t source_bound_mask;
    int command_owner_required, viewport_owner_required, menu_owner_required;
    const char *source_evidence;
} CSB_V1_F0366F0385CommandViewportReceiptPc34;

/* Read-only ReDMCSB COMMAND.C/MENUS.C F0366-F0385 admission. */
int csb_v1_f0366_f0385_command_viewport_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0366F0385CommandViewportReceiptPc34 *out_receipt);
#endif
