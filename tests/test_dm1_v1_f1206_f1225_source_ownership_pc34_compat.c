#include "dm1_v1_f1206_f1225_source_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F1206F1225OwnershipPc34 *entry;
    unsigned int number;

    for (number = 1206U; number <= 1225U; ++number) {
        entry = dm1_v1_f1206_f1225_source_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f1206_f1225_admits_authentic_route_pc34(number));
        CHECK(!dm1_v1_f1206_f1225_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f1206_f1225_source_ownership_pc34(1205U));
    CHECK(!dm1_v1_f1206_f1225_source_ownership_pc34(1226U));
    CHECK(dm1_v1_f1206_f1225_source_ownership_pc34(1206U)->kind ==
          DM1_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34);
    CHECK(dm1_v1_f1206_f1225_source_ownership_pc34(1210U)->kind ==
          DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34);
    CHECK(strstr(dm1_v1_f1206_f1225_source_ownership_evidence_pc34(),
                 "No generated") != 0);
    return 0;
}
