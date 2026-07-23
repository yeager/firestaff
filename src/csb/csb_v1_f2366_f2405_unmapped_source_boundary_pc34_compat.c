#include "csb_v1_f2366_f2405_unmapped_source_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f2366_f2405_unmapped_source_boundary_admit_pc34(
    unsigned int number,
    CSB_V1_F2366F2405UnmappedSourceBoundaryReceiptPc34 *out)
{
    CSB_V1_F2366F2405UnmappedSourceBoundaryReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (number < 2366u || number > 2405u) return 0;

    receipt.function_number = number;
    receipt.redmcsb_callable_source =
        "no F2366-F2405 callable symbol in audited ReDMCSB corpus";
    receipt.authentic_pc34_material_required = 1;
    receipt.csb_runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    *out = receipt;
    return 0;
}

const char *csb_v1_f2366_f2405_unmapped_source_boundary_evidence_pc34(void)
{
    return "The ReDMCSB callable inventory contains no F2366-F2405 symbols. "
           "The separately indexed L2366-L2405 entries are DOS, Tandy, VGA, sound, "
           "and joystick configuration locals in DM.C, not CSB PC34 ownership. "
           "Every CSB route fails closed without synthetic UI, graphics, audio, "
           "timing, input, or game behavior.";
}
