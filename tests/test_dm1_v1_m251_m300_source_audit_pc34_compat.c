#include "dm1_v1_m251_m300_source_audit_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const Dm1V1M251M300SourceAuditPc34 *entry;
    unsigned int number;

    for (number = 251U; number <= 300U; ++number) {
        entry = dm1_v1_m251_m300_source_audit_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_m251_m300_has_verified_owner_pc34(number));
    }

    CHECK(!dm1_v1_m251_m300_source_audit_pc34(250U));
    CHECK(!dm1_v1_m251_m300_source_audit_pc34(301U));
    CHECK(!dm1_v1_m251_m300_has_synthetic_route_pc34());
    CHECK(strstr(dm1_v1_m251_m300_source_audit_evidence_pc34(),
                 "no M251-M300 label") != 0);
    return 0;
}
