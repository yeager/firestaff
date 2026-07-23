#include "dm1_v1_m451_m500_source_audit_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const Dm1V1M451M500SourceAuditPc34 *entry;
    unsigned int number;

    for (number = 451U; number <= 500U; ++number) {
        entry = dm1_v1_m451_m500_source_audit_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_m451_m500_has_verified_owner_pc34(number));
    }

    CHECK(!dm1_v1_m451_m500_source_audit_pc34(450U));
    CHECK(!dm1_v1_m451_m500_source_audit_pc34(501U));
    CHECK(dm1_v1_m451_m500_source_audit_pc34(500U)->disposition ==
          DM1_V1_M451_M500_UNVERIFIED_NO_ROUTE_PC34);
    CHECK(!dm1_v1_m451_m500_has_synthetic_route_pc34());
    CHECK(strstr(dm1_v1_m451_m500_source_audit_evidence_pc34(),
                 "M451-M499 are absent") != 0);
    return 0;
}
