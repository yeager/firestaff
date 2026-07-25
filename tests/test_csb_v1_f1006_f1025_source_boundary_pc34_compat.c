#include "csb_v1_f1006_f1025_source_boundary_pc34_compat.h"
#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F1006F1025SourceBoundaryReceiptPc34 receipt;
    (void)receipt;
    unsigned int number;
    for (number = 1006u; number <= 1025u; ++number) {
        assert(csb_v1_f1006_f1025_source_boundary_admit_pc34(number, &receipt) == 0);
        assert(receipt.function_number == number && receipt.symbol && receipt.source_anchor);
        assert(receipt.authentic_pc34_material_required && receipt.runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f1006_f1025_source_boundary_admit_pc34(1005u, &receipt) == 0 && receipt.function_number == 0u);
    assert(csb_v1_f1006_f1025_source_boundary_admit_pc34(1026u, &receipt) == 0 && receipt.function_number == 0u);
    assert(csb_v1_f1006_f1025_source_boundary_admit_pc34(1007u, &receipt) == 0);
    assert(receipt.source_kind == CSB_V1_F1006_F1025_EXISTING_OWNER_NO_CSB_ADMISSION_PC34);
    assert(csb_v1_f1006_f1025_source_boundary_admit_pc34(1012u, &receipt) == 0);
    assert(receipt.source_kind == CSB_V1_F1006_F1025_EXISTING_OWNER_NO_CSB_ADMISSION_PC34);
    assert(strstr(csb_v1_f1006_f1025_source_boundary_evidence_pc34(), "all routes fail closed") != 0);
    return 0;
}
