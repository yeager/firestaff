#include "csb_v1_f0886_f0905_source_ownership_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    const CSB_V1_F0886F0905OwnershipPc34 *entry;
    unsigned int number;

    for (number = 886u; number <= 905u; ++number) {
        entry = csb_v1_f0886_f0905_source_ownership_pc34(number);
        assert(entry != 0);
        assert(entry->number == number);
        assert(entry->actual_symbol[0] != '\0');
        assert(entry->source_anchor[0] != '\0');
        assert(!csb_v1_f0886_f0905_has_standalone_synthetic_route_pc34(number));
    }
    assert(csb_v1_f0886_f0905_source_ownership_pc34(885u) == 0);
    assert(csb_v1_f0886_f0905_source_ownership_pc34(906u) == 0);

    entry = csb_v1_f0886_f0905_source_ownership_pc34(902u);
    assert(entry->kind == CSB_V1_F0886_F0905_EXISTING_REAL_OWNER_PC34);
    assert(strstr(entry->owner_or_rationale, "original 320x200") != 0);
    entry = csb_v1_f0886_f0905_source_ownership_pc34(903u);
    assert(strstr(entry->owner_or_rationale, "fail closed") != 0);
    entry = csb_v1_f0886_f0905_source_ownership_pc34(904u);
    assert(strstr(entry->owner_or_rationale, "27 two-word") != 0);
    assert(strstr(csb_v1_f0886_f0905_source_ownership_evidence_pc34(),
                  "no standalone synthetic CSB route") != 0);
    return 0;
}
