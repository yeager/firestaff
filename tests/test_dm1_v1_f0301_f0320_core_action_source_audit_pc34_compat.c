#include "dm1_v1_f0301_f0320_core_action_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    const DM1_V1_F0301F0320SourceAuditPc34Compat *entry;
    const char *evidence;
    uint16_t function_number;
    int ok = 1;

    for (function_number = 301u; function_number <= 320u; ++function_number) {
        entry = dm1_v1_f0301_f0320_core_action_source_audit_pc34(function_number);
        ok &= check(entry != NULL && entry->functionNumber == function_number,
                    "every audited function has a source owner");
        if (!entry) continue;
        ok &= check(entry->requiresOriginalMaterial && entry->hostFallbackForbidden,
                    "every owner requires original material and fails closed");
        ok &= check(entry->redmcsbAnchor != NULL && entry->firestaffOwner != NULL,
                    "every entry has ReDMCSB and Firestaff evidence");
    }
    ok &= check(dm1_v1_f0301_f0320_core_action_source_audit_pc34(300u) == NULL &&
                    dm1_v1_f0301_f0320_core_action_source_audit_pc34(321u) == NULL,
                "the catalog is disjoint from neighboring symbols");
    entry = dm1_v1_f0301_f0320_core_action_source_audit_pc34(301u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0301_F0320_OWNER_SLOT_SKILL,
                "F0301 retains the slot/skill owner");
    entry = dm1_v1_f0301_f0320_core_action_source_audit_pc34(306u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0301_F0320_OWNER_STATS_COMBAT,
                "F0306 starts the stats/combat owner range");
    entry = dm1_v1_f0301_f0320_core_action_source_audit_pc34(314u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE,
                "F0314 starts the champion lifecycle owner range");
    evidence = dm1_v1_f0301_f0320_core_action_source_evidence_pc34();
    ok &= check(strstr(evidence, "F0301:587-660") != NULL &&
                    strstr(evidence, "F0320:1689-1800") != NULL,
                "ReDMCSB source span is pinned");
    if (!ok) return 1;
    puts("PASS dm1_v1_f0301_f0320_core_action_source_audit_pc34_compat");
    return 0;
}
