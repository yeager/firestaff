#include "dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34_compat.h"

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
    const DM1_V1_F0161F0180SourceAuditPc34Compat *entry;
    const char *evidence;
    uint16_t function_number;
    int ok = 1;

    for (function_number = 161u; function_number <= 180u; ++function_number) {
        entry = dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34(function_number);
        ok &= check(entry != NULL && entry->functionNumber == function_number,
                    "every audited function has an owner");
        if (!entry) continue;
        ok &= check(entry->requiresOriginalMaterial && entry->hostFallbackForbidden,
                    "every owner is source-bound and fail-closed");
        ok &= check(entry->redmcsbAnchor != NULL && entry->firestaffOwner != NULL,
                    "every owner has source and Firestaff evidence");
    }
    ok &= check(dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34(160u) == NULL &&
                    dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34(181u) == NULL,
                "the audit does not claim neighboring symbols");
    entry = dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34(161u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0161_F0180_OWNER_SQUARE_THING,
                "F0161 retains the square-first-thing owner");
    entry = dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34(175u);
    ok &= check(entry && entry->ownerKind == DM1_V1_F0161_F0180_OWNER_GROUP_TARGETING,
                "F0175 starts the GROUP source-owned range");
    evidence = dm1_v1_f0161_f0180_dungeon_group_source_evidence_pc34();
    ok &= check(strstr(evidence, "F0161:1730-1750") != NULL &&
                    strstr(evidence, "F0180:311-340") != NULL,
                "ReDMCSB source span is pinned");
    if (!ok) return 1;
    puts("PASS dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34_compat");
    return 0;
}
