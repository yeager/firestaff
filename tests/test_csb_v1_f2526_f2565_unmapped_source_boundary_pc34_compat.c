#include "csb_v1_f2526_f2565_unmapped_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F2526F2565UnmappedSourceBoundaryReceiptPc34 receipt;
    unsigned int inventory_number;

    for (inventory_number = 2526u; inventory_number <= 2565u;
         ++inventory_number) {
        assert(csb_v1_f2526_f2565_unmapped_source_boundary_admit_pc34(
                   inventory_number, &receipt) == 0);
        assert(receipt.inventory_number == inventory_number);
        assert(receipt.redmcsb_source_owner != 0);
        assert(receipt.authentic_pc34_material_required);
        assert(receipt.csb_runtime_execution_blocked);
        assert(receipt.no_synthetic_platform_or_endgame_behavior);
    }
    assert(csb_v1_f2526_f2565_unmapped_source_boundary_admit_pc34(2525u,
                                                                    &receipt) == 0);
    assert(receipt.inventory_number == 0u);
    assert(csb_v1_f2526_f2565_unmapped_source_boundary_admit_pc34(2566u,
                                                                    &receipt) == 0);
    assert(receipt.inventory_number == 0u);
    assert(strstr(csb_v1_f2526_f2565_unmapped_source_boundary_evidence_pc34(),
                  "Every CSB route fails closed"));
    return 0;
}
