#include "dm1_v1_f0886_f0905_source_ownership_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    const DM1_V1_F0886F0905OwnershipPc34 *entry;
    (void)entry;
    unsigned int number;

    for (number = 886U; number <= 905U; ++number) {
        entry = dm1_v1_f0886_f0905_source_ownership_pc34(number);
        assert(entry != 0);
        assert(entry->number == number);
        assert(entry->actual_symbol[0] != '\0');
        assert(entry->source_anchor[0] != '\0');
        assert(!dm1_v1_f0886_f0905_has_standalone_synthetic_route_pc34(number));
    }
    assert(dm1_v1_f0886_f0905_source_ownership_pc34(885U) == 0);
    assert(dm1_v1_f0886_f0905_source_ownership_pc34(906U) == 0);
    entry = dm1_v1_f0886_f0905_source_ownership_pc34(902U);
    assert(entry->kind == DM1_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34);
    assert(strstr(entry->owner_or_rationale, "Authenticated") != 0);
    entry = dm1_v1_f0886_f0905_source_ownership_pc34(903U);
    assert(strstr(entry->owner_or_rationale, "fail closed") != 0);
    assert(strstr(dm1_v1_f0886_f0905_source_ownership_evidence_pc34(),
                  "original palette commands") != 0);
    return 0;
}
