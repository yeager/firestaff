#include "dm1_v1_c001_c004_ers_special_source_audit_pc34_compat.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    const Dm1V1RedmcsbSymbolAuditPc34 *entry;
    unsigned int i;
    unsigned int verified = 0U;

    CHECK(dm1_v1_c001_c004_ers_special_source_audit_count_pc34() == 49U);
    for (i = 0U; i < dm1_v1_c001_c004_ers_special_source_audit_count_pc34(); ++i) {
        entry = dm1_v1_c001_c004_ers_special_source_audit_pc34(i);
        CHECK(entry != 0 && entry->symbol[0] != '\0' && entry->source_anchor[0] != '\0');
        if (entry->disposition == DM1_V1_REDMCSB_SYMBOL_VERIFIED_EXISTING_OWNER_PC34) ++verified;
    }
    CHECK(verified == 4U);
    CHECK(!dm1_v1_c001_c004_ers_special_source_audit_pc34(49U));
    CHECK(!dm1_v1_c001_c004_ers_special_has_synthetic_route_pc34());
    CHECK(strstr(dm1_v1_c001_c004_ers_special_source_audit_evidence_pc34(),
                 "Only C001-C004 movement command owners are verified") != 0);
    return 0;
}
