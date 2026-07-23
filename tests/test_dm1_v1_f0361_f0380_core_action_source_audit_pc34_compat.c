#include "dm1_v1_f0361_f0380_core_action_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int value, const char *label) { if (value) return 1; fprintf(stderr, "FAIL: %s\n", label); return 0; }

int main(void)
{
    const DM1_V1_F0361F0380SourceAuditPc34Compat *entry;
    uint16_t number;
    int ok = 1;
    for (number = 361u; number <= 380u; ++number) {
        entry = dm1_v1_f0361_f0380_core_action_source_audit_pc34(number);
        ok &= check(entry && entry->functionNumber == number, "every source symbol is catalogued");
        if (entry) ok &= check(entry->requiresOriginalMaterial && entry->hostFallbackForbidden && entry->redmcsbAnchor && entry->firestaffOwner, "all paths are source-bound and fail closed");
    }
    ok &= check(dm1_v1_f0361_f0380_core_action_source_audit_pc34(360u) == NULL && dm1_v1_f0361_f0380_core_action_source_audit_pc34(381u) == NULL, "range is disjoint");
    entry = dm1_v1_f0361_f0380_core_action_source_audit_pc34(369u);
    ok &= check(entry && entry->auditKind == DM1_V1_F0361_F0380_AUDIT_EXISTING_OWNER, "spell zone keeps established owner");
    entry = dm1_v1_f0361_f0380_core_action_source_audit_pc34(380u);
    ok &= check(entry && entry->auditKind == DM1_V1_F0361_F0380_AUDIT_FAIL_CLOSED_BOUNDARY, "queue has no invented owner");
    ok &= check(strstr(dm1_v1_f0361_f0380_core_action_source_evidence_pc34(), "F0380") != NULL, "source evidence is pinned");
    if (!ok) return 1;
    puts("PASS dm1_v1_f0361_f0380_core_action_source_audit_pc34_compat");
    return 0;
}
