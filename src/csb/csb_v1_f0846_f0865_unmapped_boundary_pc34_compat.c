#include "csb_v1_f0846_f0865_unmapped_boundary_pc34_compat.h"

#include <string.h>

int csb_v1_f0846_f0865_unmapped_admit_pc34(
    uint16_t function_id,
    CSB_V1_F0846F0865UnmappedBoundaryReceiptPc34 *out)
{
    CSB_V1_F0846F0865UnmappedBoundaryReceiptPc34 receipt;

    if (!out) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (function_id < 846u || function_id > 865u) {
        return 0;
    }

    /* The ReDMCSB callable inventory jumps from F0819 to F0902. In
     * particular, Firestaff's M10 F0860 generator label is not a ReDMCSB
     * F0860 symbol and cannot authorize this CSB range. */
    receipt.function_id = function_id;
    receipt.source_callable_absent = 1;
    receipt.authentic_package_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.no_synthetic_ui_graphics_timing = 1;
    receipt.source_evidence =
        "ReDMCSB callable inventory: no F0846-F0865 rows; "
        "F0819 is followed by F0902";
    *out = receipt;
    return 0;
}
