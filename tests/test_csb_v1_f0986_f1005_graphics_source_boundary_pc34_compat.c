#include "csb_v1_f0986_f1005_graphics_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F0986F1005SourceBoundaryReceiptPc34 receipt;
    unsigned int function_number;

    for (function_number = 986u; function_number <= 1005u; ++function_number) {
        assert(csb_v1_f0986_f1005_source_boundary_admit_pc34(function_number,
                                                               &receipt) == 0);
        assert(receipt.function_number == function_number);
        assert(receipt.symbol != 0 && receipt.source_anchor != 0);
        assert(receipt.owner_or_rationale != 0);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f0986_f1005_source_boundary_admit_pc34(985u, &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(csb_v1_f0986_f1005_source_boundary_admit_pc34(1006u, &receipt) == 0);
    assert(receipt.function_number == 0u);

    assert(csb_v1_f0986_f1005_source_boundary_admit_pc34(1001u, &receipt) == 0);
    assert(receipt.source_kind == CSB_V1_F0986_F1005_NON_PC34_MEDIA_PC34);
    assert(csb_v1_f0986_f1005_source_boundary_admit_pc34(1002u, &receipt) == 0);
    assert(receipt.source_kind == CSB_V1_F0986_F1005_EXISTING_OWNER_WITHOUT_CSB_ADMISSION_PC34);
    assert(csb_v1_f0986_f1005_source_boundary_admit_pc34(1003u, &receipt) == 0);
    assert(receipt.source_kind == CSB_V1_F0986_F1005_UNNUMBERED_SOURCE_HELPER_PC34);
    assert(strstr(csb_v1_f0986_f1005_source_boundary_evidence_pc34(),
                  "all runtime routes fail closed") != 0);
    return 0;
}
