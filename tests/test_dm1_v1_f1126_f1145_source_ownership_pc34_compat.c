#include "dm1_v1_f1126_f1145_source_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F1126F1145OwnershipPc34 *entry;
    unsigned int number;

    for (number = 1126U; number <= 1145U; ++number) {
        entry = dm1_v1_f1126_f1145_source_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f1126_f1145_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f1126_f1145_source_ownership_pc34(1125U));
    CHECK(!dm1_v1_f1126_f1145_source_ownership_pc34(1146U));
    CHECK(dm1_v1_f1126_f1145_admits_authentic_route_pc34(1128U));
    CHECK(!dm1_v1_f1126_f1145_admits_authentic_route_pc34(1127U));
    CHECK(!dm1_v1_f1126_f1145_admits_authentic_route_pc34(1131U));
    CHECK(!dm1_v1_f1126_f1145_admits_authentic_route_pc34(1141U));
    CHECK(dm1_v1_f1126_f1145_source_ownership_pc34(1145U)->kind ==
          DM1_V1_F1126_F1145_LOCAL_SYMBOL_PC34);
    CHECK(strstr(dm1_v1_f1126_f1145_source_ownership_evidence_pc34(),
                 "No generated") != 0);
    return 0;
}
