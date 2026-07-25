#include "csb_v1_f1186_f1205_anim_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F1186F1205AnimSourceBoundaryReceiptPc34 receipt;
    (void)receipt;
    unsigned int number;

    for (number = 1186u; number <= 1205u; ++number) {
        assert(csb_v1_f1186_f1205_anim_source_boundary_admit_pc34(number, &receipt) == 0);
        assert(receipt.function_number == number);
        assert(receipt.symbol && receipt.redmcsb_anchor);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.runtime_execution_blocked);
        assert(receipt.no_synthetic_ui_graphics_timing);
    }
    assert(csb_v1_f1186_f1205_anim_source_boundary_admit_pc34(1185u, &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(csb_v1_f1186_f1205_anim_source_boundary_admit_pc34(1206u, &receipt) == 0);
    assert(receipt.function_number == 0u);
    assert(strstr(csb_v1_f1186_f1205_anim_source_boundary_evidence_pc34(),
                  "every CSB route fails closed"));
    return 0;
}
