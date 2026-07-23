#include "dm1_v1_f2026_f2065_source_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F2026F2065OwnershipPc34 *entry;
    unsigned int number;

    for (number = 2026U; number <= 2065U; ++number) {
        entry = dm1_v1_f2026_f2065_source_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f2026_f2065_admits_authentic_route_pc34(number));
        CHECK(!dm1_v1_f2026_f2065_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f2026_f2065_source_ownership_pc34(2025U));
    CHECK(!dm1_v1_f2026_f2065_source_ownership_pc34(2066U));
    CHECK(dm1_v1_f2026_f2065_source_ownership_pc34(2026U)->kind ==
          DM1_V1_F2026_F2065_PLATFORM_HINT_BOUNDARY_PC34);
    CHECK(dm1_v1_f2026_f2065_source_ownership_pc34(2047U)->kind ==
          DM1_V1_F2026_F2065_EXISTING_CALLER_INPUT_OWNER_PC34);
    CHECK(strstr(dm1_v1_f2026_f2065_source_ownership_evidence_pc34(),
                 "existing caller-owned input owner") != 0);
    return 0;
}
