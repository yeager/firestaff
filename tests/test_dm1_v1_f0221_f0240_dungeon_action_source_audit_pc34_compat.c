#include "dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34_compat.h"

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
    const DM1_V1_F0221F0240SourceAuditPc34Compat *entry;
    const char *evidence;
    uint16_t function_number;
    int ok = 1;

    for (function_number = 221u; function_number <= 240u; ++function_number) {
        entry = dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(function_number);
        ok &= check(entry != NULL && entry->functionNumber == function_number,
                    "every audited function has a source owner");
        if (!entry) continue;
        ok &= check(entry->requiresOriginalMaterial && entry->hostFallbackForbidden,
                    "every owner requires original material and fails closed");
        ok &= check(entry->redmcsbAnchor != NULL && entry->firestaffOwner != NULL,
                    "every entry has ReDMCSB and Firestaff evidence");
    }
    ok &= check(dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(220u) == NULL &&
                    dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(241u) == NULL,
                "the catalog is disjoint from neighboring symbols");
    entry = dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(221u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0221_F0240_OWNER_ENDGAME,
                "F0221 remains in the C15/endgame owner");
    entry = dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(230u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0221_F0240_OWNER_COMBAT,
                "F0230 retains the combat owner");
    entry = dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(233u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0221_F0240_OWNER_TIMELINE,
                "F0233 starts the timeline owner range");
    evidence = dm1_v1_f0221_f0240_dungeon_action_source_evidence_pc34();
    ok &= check(strstr(evidence, "F0221:881-903") != NULL &&
                    strstr(evidence, "F0240:682-689") != NULL,
                "ReDMCSB source span is pinned");
    if (!ok) return 1;
    puts("PASS dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34_compat");
    return 0;
}
