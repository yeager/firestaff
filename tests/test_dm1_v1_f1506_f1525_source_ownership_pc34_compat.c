#include "dm1_v1_f1506_f1525_source_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F1506F1525OwnershipPc34 *entry;
    unsigned int number;

    for (number = 1506U; number <= 1525U; ++number) {
        entry = dm1_v1_f1506_f1525_source_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_f1506_f1525_admits_authentic_route_pc34(number));
        CHECK(!dm1_v1_f1506_f1525_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f1506_f1525_source_ownership_pc34(1505U));
    CHECK(!dm1_v1_f1506_f1525_source_ownership_pc34(1526U));
    CHECK(dm1_v1_f1506_f1525_source_ownership_pc34(1512U)->kind ==
          DM1_V1_F1506_F1525_UNASSIGNED_OR_LOCAL_PC34);
    CHECK(strstr(dm1_v1_f1506_f1525_source_ownership_evidence_pc34(),
                 "comments are not") != 0);
    return 0;
}
