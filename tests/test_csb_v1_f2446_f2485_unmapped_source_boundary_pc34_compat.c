#include "csb_v1_f2446_f2485_unmapped_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F2446F2485UnmappedSourceBoundaryReceiptPc34 receipt;
    unsigned int symbol_number;

    for (symbol_number = 2446u; symbol_number <= 2485u; ++symbol_number) {
        assert(csb_v1_f2446_f2485_unmapped_source_boundary_admit_pc34(
                   symbol_number, &receipt) == 0);
        assert(receipt.symbol_number == symbol_number);
        assert(receipt.redmcsb_source_owner != 0);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.csb_runtime_execution_blocked);
        assert(receipt.no_synthetic_viewport_behavior);
    }
    assert(csb_v1_f2446_f2485_unmapped_source_boundary_admit_pc34(2445u,
                                                                    &receipt) == 0);
    assert(receipt.symbol_number == 0u);
    assert(csb_v1_f2446_f2485_unmapped_source_boundary_admit_pc34(2486u,
                                                                    &receipt) == 0);
    assert(receipt.symbol_number == 0u);
    assert(strstr(csb_v1_f2446_f2485_unmapped_source_boundary_evidence_pc34(),
                  "Every CSB route fails closed"));
    return 0;
}
