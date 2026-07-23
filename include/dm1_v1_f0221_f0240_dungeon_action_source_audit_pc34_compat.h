#ifndef FIRESTAFF_DM1_V1_F0221_F0240_DUNGEON_ACTION_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0221_F0240_DUNGEON_ACTION_SOURCE_AUDIT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0221F0240OwnerKindPc34Compat {
    DM1_V1_F0221_F0240_OWNER_ENDGAME = 1,
    DM1_V1_F0221_F0240_OWNER_GROUP_AI = 2,
    DM1_V1_F0221_F0240_OWNER_COMBAT = 3,
    DM1_V1_F0221_F0240_OWNER_TIMELINE = 4
} DM1_V1_F0221F0240OwnerKindPc34Compat;

typedef struct DM1_V1_F0221F0240SourceAuditPc34Compat {
    uint16_t functionNumber;
    DM1_V1_F0221F0240OwnerKindPc34Compat ownerKind;
    int requiresOriginalMaterial;
    int hostFallbackForbidden;
    const char *redmcsbAnchor;
    const char *firestaffOwner;
} DM1_V1_F0221F0240SourceAuditPc34Compat;

/* Audit only. This catalog identifies existing source-bound DM1 owners for
 * ReDMCSB F0221-F0240. It creates no endgame state, combat result, timeline
 * event, pixel output, or synthetic host substitute. */
const DM1_V1_F0221F0240SourceAuditPc34Compat *
dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(uint16_t function_number);

const char *dm1_v1_f0221_f0240_dungeon_action_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
