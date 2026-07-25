#include "dm1_v1_f0946_f0965_source_ownership_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    const DM1_V1_F0946F0965OwnershipPc34 *entry;
    (void)entry;
    unsigned int number;
    for (number = 946U; number <= 965U; ++number) {
        entry = dm1_v1_f0946_f0965_source_ownership_pc34(number);
        assert(entry && entry->number == number);
        assert(entry->symbol[0] && entry->source_anchor[0]);
        assert(!dm1_v1_f0946_f0965_has_synthetic_route_pc34(number));
    }
    assert(!dm1_v1_f0946_f0965_source_ownership_pc34(945U));
    assert(!dm1_v1_f0946_f0965_source_ownership_pc34(966U));
    assert(dm1_v1_f0946_f0965_source_ownership_pc34(946)->kind == DM1_V1_F0946_F0965_PLATFORM_BOUNDARY_PC34);
    assert(dm1_v1_f0946_f0965_source_ownership_pc34(952)->kind == DM1_V1_F0946_F0965_EXISTING_PC98_OWNER_PC34);
    assert(strstr(dm1_v1_f0946_f0965_source_ownership_evidence_pc34(), "No generated") != 0);
    return 0;
}
