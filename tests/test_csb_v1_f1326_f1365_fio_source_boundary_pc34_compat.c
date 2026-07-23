#include "csb_v1_f1326_f1365_fio_source_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    const CSB_V1_F1326F1365FioSourceBoundaryReceiptPc34 *entries;
    size_t count;
    size_t index;

    entries = csb_v1_f1326_f1365_fio_source_boundary_pc34(&count);
    assert(entries && count == 40u);
    for (index = 0u; index < count; ++index) {
        assert(entries[index].function_number == 1326u + index);
        assert(entries[index].redmcsb_anchor && entries[index].existing_owner_or_boundary);
        assert(entries[index].authentic_pc34_material_required);
        assert(entries[index].csb_runtime_execution_blocked);
        assert(entries[index].no_synthetic_ui_graphics_timing);
        assert(csb_v1_f1326_f1365_fio_source_boundary_find_pc34(1326u + index) ==
               &entries[index]);
    }
    assert(!csb_v1_f1326_f1365_fio_source_boundary_find_pc34(1325u));
    assert(!csb_v1_f1326_f1365_fio_source_boundary_find_pc34(1366u));
    assert(strstr(csb_v1_f1326_f1365_fio_source_boundary_evidence_pc34(),
                  "does not perform file I/O"));
    return 0;
}
