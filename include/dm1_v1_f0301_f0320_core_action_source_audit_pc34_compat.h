#ifndef FIRESTAFF_DM1_V1_F0301_F0320_CORE_ACTION_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0301_F0320_CORE_ACTION_SOURCE_AUDIT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0301F0320OwnerKindPc34Compat {
    DM1_V1_F0301_F0320_OWNER_SLOT_SKILL = 1,
    DM1_V1_F0301_F0320_OWNER_STATS_COMBAT = 2,
    DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE = 3
} DM1_V1_F0301F0320OwnerKindPc34Compat;

typedef struct DM1_V1_F0301F0320SourceAuditPc34Compat {
    uint16_t functionNumber;
    DM1_V1_F0301F0320OwnerKindPc34Compat ownerKind;
    int requiresOriginalMaterial;
    int hostFallbackForbidden;
    const char *redmcsbAnchor;
    const char *firestaffOwner;
} DM1_V1_F0301F0320SourceAuditPc34Compat;

/* Audit only. Existing DM1 owners remain the sole F0301-F0320 behavior.
 * This catalog creates no slot mutation, XP/stat result, panel draw, scent,
 * death state, or host substitute. */
const DM1_V1_F0301F0320SourceAuditPc34Compat *
dm1_v1_f0301_f0320_core_action_source_audit_pc34(uint16_t function_number);

const char *dm1_v1_f0301_f0320_core_action_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
