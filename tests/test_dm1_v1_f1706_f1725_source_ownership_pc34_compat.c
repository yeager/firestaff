#include "dm1_v1_f1706_f1725_source_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F1706F1725OwnershipPc34 *entry;
    (void)entry;
    unsigned int number;

    for (number = 1706U; number <= 1725U; ++number) {
        entry = dm1_v1_f1706_f1725_source_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f1706_f1725_admits_authentic_route_pc34(number));
        CHECK(!dm1_v1_f1706_f1725_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f1706_f1725_source_ownership_pc34(1705U));
    CHECK(!dm1_v1_f1706_f1725_source_ownership_pc34(1726U));
    CHECK(dm1_v1_f1706_f1725_source_ownership_pc34(1717U)->kind ==
          DM1_V1_F1706_F1725_PLATFORM_BOUNDARY_PC34);
    CHECK(strstr(dm1_v1_f1706_f1725_source_ownership_evidence_pc34(),
                 "remain separate") != 0);
    return 0;
}
