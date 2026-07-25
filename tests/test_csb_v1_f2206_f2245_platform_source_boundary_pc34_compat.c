#include "csb_v1_f2206_f2245_platform_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F2206F2245PlatformSourceBoundaryReceiptPc34 receipt;
    (void)receipt;
    unsigned int number;

    for (number = 2206u; number <= 2245u; ++number) {
        assert(csb_v1_f2206_f2245_platform_source_boundary_admit_pc34(number,
                                                                        &receipt) == 0);
        assert(receipt.function_number == number);
        assert(receipt.redmcsb_anchor && receipt.existing_owner_or_boundary);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.csb_runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f2206_f2245_platform_source_boundary_admit_pc34(2205u,
                                                                    &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(csb_v1_f2206_f2245_platform_source_boundary_admit_pc34(2246u,
                                                                    &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(strstr(csb_v1_f2206_f2245_platform_source_boundary_evidence_pc34(),
                  "every CSB route fails closed"));
    return 0;
}
