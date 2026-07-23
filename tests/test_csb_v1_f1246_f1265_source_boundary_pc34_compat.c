#include "csb_v1_f1246_f1265_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    const CSB_V1_F1246F1265SourceBoundaryReceiptPc34 *entries;
    size_t count;
    size_t index;

    entries = csb_v1_f1246_f1265_source_boundary_pc34(&count);
    assert(entries && count == 20u);
    for (index = 0u; index < count; ++index) {
        assert(entries[index].symbol_number == 1246u + index);
        assert(entries[index].redmcsb_anchor && entries[index].source_class);
        assert(entries[index].authentic_pc34_material_required);
        assert(entries[index].runtime_execution_blocked);
        assert(entries[index].no_synthetic_ui_graphics_timing);
        assert(csb_v1_f1246_f1265_source_boundary_find_pc34(1246u + index) ==
               &entries[index]);
    }
    assert(!csb_v1_f1246_f1265_source_boundary_find_pc34(1245u));
    assert(!csb_v1_f1246_f1265_source_boundary_find_pc34(1266u));
    assert(strstr(csb_v1_f1246_f1265_source_boundary_evidence_pc34(),
                  "every CSB route fails closed"));
    return 0;
}
