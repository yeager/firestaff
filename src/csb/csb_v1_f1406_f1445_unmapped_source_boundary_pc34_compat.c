#include "csb_v1_f1406_f1445_unmapped_source_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f1406_f1445_unmapped_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F1406F1445UnmappedSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F1406F1445UnmappedSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 1406u || number > 1445u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_callable_source =
        "no F1406-F1445 callable symbol in audited ReDMCSB corpus";
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f1406_f1445_unmapped_source_boundary_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory contains no F1406-F1445 symbols. "
           "The separately indexed L1406-L1445 names are function-local state "
           "for entrance, endgame, startup, and floppy sources, not portable "
           "CSB runtime ownership. Every CSB route fails closed; this receipt "
           "does not render, synthesize UI, or create timing.";
}
