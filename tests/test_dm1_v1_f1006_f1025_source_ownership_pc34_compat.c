#include "dm1_v1_f1006_f1025_source_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F1006F1025OwnershipPc34 *entry;
    unsigned int number;

    for (number = 1006U; number <= 1025U; ++number) {
        entry = dm1_v1_f1006_f1025_source_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f1006_f1025_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f1006_f1025_source_ownership_pc34(1005U));
    CHECK(!dm1_v1_f1006_f1025_source_ownership_pc34(1026U));
    CHECK(dm1_v1_f1006_f1025_admits_authentic_route_pc34(1007U));
    CHECK(dm1_v1_f1006_f1025_admits_authentic_route_pc34(1008U));
    CHECK(dm1_v1_f1006_f1025_admits_authentic_route_pc34(1012U));
    CHECK(!dm1_v1_f1006_f1025_admits_authentic_route_pc34(1013U));
    CHECK(!dm1_v1_f1006_f1025_admits_authentic_route_pc34(1016U));
    CHECK(!dm1_v1_f1006_f1025_admits_authentic_route_pc34(1025U));
    CHECK(dm1_v1_f1006_f1025_source_ownership_pc34(1006U)->kind ==
          DM1_V1_F1006_F1025_PLATFORM_BOUNDARY_PC34);
    CHECK(strstr(dm1_v1_f1006_f1025_source_ownership_evidence_pc34(),
                 "No generated") != 0);
    return 0;
}
