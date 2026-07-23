#include "csb_v1_f2446_f2485_unmapped_source_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f2446_f2485_unmapped_source_boundary_admit_pc34(
    unsigned int symbol_number,
    CSB_V1_F2446F2485UnmappedSourceBoundaryReceiptPc34 *out_receipt)
{
    CSB_V1_F2446F2485UnmappedSourceBoundaryReceiptPc34 receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out_receipt = receipt;
    if (symbol_number < 2446u || symbol_number > 2485u) return 0;

    receipt.symbol_number = symbol_number;
    receipt.redmcsb_source_owner =
        "no F2446-F2485 callable symbol in audited ReDMCSB corpus";
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_viewport_behavior = 1;
    *out_receipt = receipt;
    return 0;
}

const char *csb_v1_f2446_f2485_unmapped_source_boundary_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory contains no F2446-F2485 symbols. "
           "The indexed L2446-L2485 entries are DUNVIEW.C automatic locals "
           "owned by F0105/F0107-F0115/F0675-F0677/F0791, with existing "
           "M11 or DM1 owner traces and no standalone CSB PC34 owner. "
           "Every CSB route fails closed without synthetic viewport, graphics, "
           "palette, material, or timing behavior.";
}
