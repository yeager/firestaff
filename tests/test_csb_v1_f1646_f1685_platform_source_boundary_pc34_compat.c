#include "csb_v1_f1646_f1685_platform_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F1646F1685PlatformSourceBoundaryReceiptPc34 receipt;
    unsigned int number;

    for (number = 1646u; number <= 1685u; ++number) {
        assert(csb_v1_f1646_f1685_platform_source_boundary_admit_pc34(number,
                                                                        &receipt) == 0);
        assert(receipt.function_number == number);
        assert(receipt.redmcsb_anchor && receipt.existing_owner_or_boundary);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.csb_runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f1646_f1685_platform_source_boundary_admit_pc34(1645u,
                                                                    &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(csb_v1_f1646_f1685_platform_source_boundary_admit_pc34(1686u,
                                                                    &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(strstr(csb_v1_f1646_f1685_platform_source_boundary_evidence_pc34(),
                  "every CSB route fails closed"));
    return 0;
}
