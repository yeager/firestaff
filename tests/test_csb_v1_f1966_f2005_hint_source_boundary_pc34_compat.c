#include "csb_v1_f1966_f2005_hint_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F1966F2005HintSourceBoundaryReceiptPc34 receipt;
    unsigned int number;

    for (number = 1966u; number <= 2005u; ++number) {
        assert(csb_v1_f1966_f2005_hint_source_boundary_admit_pc34(number,
                                                                    &receipt) == 0);
        assert(receipt.function_number == number);
        assert(receipt.redmcsb_anchor && receipt.existing_owner_or_boundary);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.csb_runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f1966_f2005_hint_source_boundary_admit_pc34(1965u,
                                                                &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(csb_v1_f1966_f2005_hint_source_boundary_admit_pc34(2006u,
                                                                &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(strstr(csb_v1_f1966_f2005_hint_source_boundary_evidence_pc34(),
                  "Every CSB route fails closed"));
    return 0;
}
