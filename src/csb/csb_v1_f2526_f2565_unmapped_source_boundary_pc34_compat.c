#include "csb_v1_f2526_f2565_unmapped_source_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f2526_f2565_unmapped_source_boundary_admit_pc34(
    unsigned int inventory_number,
    CSB_V1_F2526F2565UnmappedSourceBoundaryReceiptPc34 *out_receipt)
{
    CSB_V1_F2526F2565UnmappedSourceBoundaryReceiptPc34 receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out_receipt = receipt;
    if (inventory_number < 2526u || inventory_number > 2565u) return 0;

    receipt.inventory_number = inventory_number;
    receipt.redmcsb_source_owner =
        "no F2526-F2565 callable symbol in audited ReDMCSB corpus";
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_platform_or_endgame_behavior = 1;
    *out_receipt = receipt;
    return 0;
}

const char *csb_v1_f2526_f2565_unmapped_source_boundary_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory contains no F2526-F2565 symbols. "
           "The indexed L2526-L2565 entries are automatic locals in NEC816.C, "
           "VIDEODRV.C, DRAWVIEW.C, and ENDGAME.C for hardware, palette, mouse, "
           "or endgame work. They have no standalone CSB PC34 owner. "
           "Every CSB route fails closed without synthetic platform, palette, "
           "viewport, endgame, audio, or timing behavior.";
}
