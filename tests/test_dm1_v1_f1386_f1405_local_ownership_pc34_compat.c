#include "dm1_v1_f1386_f1405_local_ownership_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const DM1_V1_F1386F1405LocalOwnershipPc34 *entry;
    unsigned int number;

    for (number = 1386U; number <= 1405U; ++number) {
        entry = dm1_v1_f1386_f1405_local_ownership_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->parent_owner[0] != '\0');
        CHECK(!dm1_v1_f1386_f1405_admits_standalone_route_pc34(number));
        CHECK(!dm1_v1_f1386_f1405_has_synthetic_route_pc34(number));
    }

    CHECK(!dm1_v1_f1386_f1405_local_ownership_pc34(1385U));
    CHECK(!dm1_v1_f1386_f1405_local_ownership_pc34(1406U));
    CHECK(strstr(dm1_v1_f1386_f1405_local_ownership_evidence_pc34(),
                 "All F1386-F1405") != 0);
    return 0;
}
