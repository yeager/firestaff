#include "dm1_v1_m051_m100_source_audit_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const Dm1V1M051M100SourceAuditPc34 *entry;
    unsigned int number;

    for (number = 51U; number <= 100U; ++number) {
        entry = dm1_v1_m051_m100_source_audit_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!dm1_v1_m051_m100_has_verified_owner_pc34(number));
    }

    CHECK(!dm1_v1_m051_m100_source_audit_pc34(50U));
    CHECK(!dm1_v1_m051_m100_source_audit_pc34(101U));
    CHECK(!dm1_v1_m051_m100_has_synthetic_route_pc34());
    CHECK(strstr(dm1_v1_m051_m100_source_audit_evidence_pc34(),
                 "M093-M099 are absent") != 0);
    return 0;
}
