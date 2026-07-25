#include "csb_v1_f1726_f1765_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F1726F1765SourceBoundaryReceiptPc34 receipt;
    (void)receipt;
    unsigned int number;

    for (number = 1726u; number <= 1765u; ++number) {
        assert(csb_v1_f1726_f1765_source_boundary_admit_pc34(number, &receipt) == 0);
        assert(receipt.function_number == number && receipt.redmcsb_anchor);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.csb_runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f1726_f1765_source_boundary_admit_pc34(1725u, &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(csb_v1_f1726_f1765_source_boundary_admit_pc34(1766u, &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(strstr(csb_v1_f1726_f1765_source_boundary_evidence_pc34(),
                  "every CSB route fails closed"));
    return 0;
}
