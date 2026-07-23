#include "redmcsb_l0051_l0100_source_audit_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const RedmcsbL0051L0100SourceAuditPc34 *entry;
    unsigned int index;
    unsigned int l0071 = 0U;
    unsigned int l0097 = 0U;

    CHECK(redmcsb_l0051_l0100_source_audit_count_pc34() == 52U);
    for (index = 0U; index < redmcsb_l0051_l0100_source_audit_count_pc34(); ++index) {
        entry = redmcsb_l0051_l0100_source_audit_pc34(index);
        CHECK(entry != 0 && entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        CHECK(!redmcsb_l0051_l0100_has_verified_owner_pc34(index));
        if (entry->number == 71U) ++l0071;
        if (entry->number == 97U) ++l0097;
    }

    CHECK(l0071 == 2U && l0097 == 2U);
    CHECK(!redmcsb_l0051_l0100_source_audit_pc34(52U));
    CHECK(!redmcsb_l0051_l0100_has_synthetic_route_pc34());
    CHECK(strstr(redmcsb_l0051_l0100_source_audit_evidence_pc34(),
                 "trace-only hits do not establish") != 0);
    return 0;
}
