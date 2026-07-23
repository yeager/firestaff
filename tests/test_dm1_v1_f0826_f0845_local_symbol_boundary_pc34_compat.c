#include "dm1_v1_f0826_f0845_local_symbol_boundary_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    unsigned int number;
    const DM1_V1_F0826F0845LocalSymbolBoundaryPc34 *entry;

    for (number = 826U; number <= 845U; ++number) {
        entry = dm1_v1_f0826_f0845_local_symbol_boundary_pc34(number);
        assert(entry != 0);
        assert(entry->number == number);
        assert(entry->label[0] == 'L');
        assert(entry->source_file[0] != '\0');
        assert(strncmp(entry->parent_callable, "F028", 4) == 0);
        assert(!dm1_v1_f0826_f0845_has_standalone_pc34_route(number));
    }
    assert(dm1_v1_f0826_f0845_local_symbol_boundary_pc34(825U) == 0);
    assert(dm1_v1_f0826_f0845_local_symbol_boundary_pc34(846U) == 0);
    assert(strstr(dm1_v1_f0826_f0845_local_symbol_source_evidence_pc34(),
                  "not F callables") != 0);
    return 0;
}
