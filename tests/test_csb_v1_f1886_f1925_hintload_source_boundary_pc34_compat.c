#include "csb_v1_f1886_f1925_hintload_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    const CSB_V1_F1886F1925HintloadSourceReceiptPc34 *entries;
    size_t count;
    size_t index;
    size_t owner_count = 0u;

    entries = csb_v1_f1886_f1925_hintload_source_receipt_pc34(&count);
    assert(entries && count == 40u);
    for (index = 0u; index < count; ++index) {
        assert(entries[index].function_number == 1886u + index);
        assert(entries[index].redmcsb_anchor && entries[index].existing_owner_or_boundary);
        assert(entries[index].authentic_pc34_material_required);
        assert(entries[index].fail_closed_without_pc34);
        assert(entries[index].no_synthetic_ui_graphics_timing);
        if (entries[index].existing_authenticated_csb_owner) ++owner_count;
        assert(csb_v1_f1886_f1925_hintload_source_find_pc34(1886u + index) ==
               &entries[index]);
    }
    assert(owner_count == 5u);
    assert(!csb_v1_f1886_f1925_hintload_source_find_pc34(1885u));
    assert(!csb_v1_f1886_f1925_hintload_source_find_pc34(1926u));
    assert(strstr(csb_v1_f1886_f1925_hintload_source_evidence_pc34(),
                  "All other routes fail closed"));
    return 0;
}
