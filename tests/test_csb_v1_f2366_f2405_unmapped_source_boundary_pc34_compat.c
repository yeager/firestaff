#include "csb_v1_f2366_f2405_unmapped_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F2366F2405UnmappedSourceBoundaryReceiptPc34 receipt;
    (void)receipt;
    unsigned int number;

    for (number = 2366u; number <= 2405u; ++number) {
        assert(csb_v1_f2366_f2405_unmapped_source_boundary_admit_pc34(number,
                                                                        &receipt) == 0);
        assert(receipt.function_number == number && receipt.redmcsb_callable_source);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.csb_runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f2366_f2405_unmapped_source_boundary_admit_pc34(2365u,
                                                                    &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(csb_v1_f2366_f2405_unmapped_source_boundary_admit_pc34(2406u,
                                                                    &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(strstr(csb_v1_f2366_f2405_unmapped_source_boundary_evidence_pc34(),
                  "Every CSB route fails closed"));
    return 0;
}
