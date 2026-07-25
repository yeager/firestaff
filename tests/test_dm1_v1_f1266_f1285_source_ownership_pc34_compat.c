#include "dm1_v1_f1266_f1285_source_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F1266F1285OwnershipPc34 *entry;
    (void)entry;
    unsigned int number;

    for (number = 1266U; number <= 1285U; ++number) {
        entry = dm1_v1_f1266_f1285_source_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f1266_f1285_admits_authentic_route_pc34(number));
        CHECK(!dm1_v1_f1266_f1285_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f1266_f1285_source_ownership_pc34(1265U));
    CHECK(!dm1_v1_f1266_f1285_source_ownership_pc34(1286U));
    CHECK(dm1_v1_f1266_f1285_source_ownership_pc34(1266U)->kind ==
          DM1_V1_F1266_F1285_LOCAL_SYMBOL_PC34);
    CHECK(dm1_v1_f1266_f1285_source_ownership_pc34(1272U)->kind ==
          DM1_V1_F1266_F1285_PLATFORM_BOUNDARY_PC34);
    CHECK(strstr(dm1_v1_f1266_f1285_source_ownership_evidence_pc34(),
                 "No generated") != 0);
    return 0;
}
