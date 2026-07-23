#include "dm1_v1_m101_m150_source_audit_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const Dm1V1M101M150SourceAuditPc34 *entry;
    unsigned int number;
    unsigned int verified = 0U;

    for (number = 101U; number <= 150U; ++number) {
        entry = dm1_v1_m101_m150_source_audit_pc34(number);
        CHECK(entry != 0 && entry->number == number);
        CHECK(entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        if (dm1_v1_m101_m150_has_verified_owner_pc34(number)) ++verified;
    }

    CHECK(verified == 2U);
    CHECK(!dm1_v1_m101_m150_source_audit_pc34(100U));
    CHECK(!dm1_v1_m101_m150_source_audit_pc34(151U));
    CHECK(!dm1_v1_m101_m150_has_synthetic_route_pc34());
    CHECK(strstr(dm1_v1_m101_m150_source_audit_evidence_pc34(),
                 "M111-M150 are absent") != 0);
    return 0;
}
