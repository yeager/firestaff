#include "dm1_v1_m401_m450_source_audit_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const Dm1V1M401M450SourceAuditPc34 *entry;
    unsigned int number;

    for (number = 401U; number <= 450U; ++number) {
        entry = dm1_v1_m401_m450_source_audit_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_m401_m450_has_verified_owner_pc34(number));
    }

    CHECK(!dm1_v1_m401_m450_source_audit_pc34(400U));
    CHECK(!dm1_v1_m401_m450_source_audit_pc34(451U));
    CHECK(!dm1_v1_m401_m450_has_synthetic_route_pc34());
    CHECK(strstr(dm1_v1_m401_m450_source_audit_evidence_pc34(),
                 "no M401-M450 label") != 0);
    return 0;
}
