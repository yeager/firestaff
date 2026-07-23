#ifndef FIRESTAFF_DM1_V1_F0161_F0180_DUNGEON_GROUP_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0161_F0180_DUNGEON_GROUP_SOURCE_AUDIT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0161F0180OwnerKindPc34Compat {
    DM1_V1_F0161_F0180_OWNER_SQUARE_THING = 1,
    DM1_V1_F0161_F0180_OWNER_THING_LIFECYCLE = 2,
    DM1_V1_F0161_F0180_OWNER_WALL_TEXT_ORNAMENT = 3,
    DM1_V1_F0161_F0180_OWNER_MAP_CONTEXT = 4,
    DM1_V1_F0161_F0180_OWNER_GROUP_TARGETING = 5,
    DM1_V1_F0161_F0180_OWNER_GROUP_LIVE_STATE = 6
} DM1_V1_F0161F0180OwnerKindPc34Compat;

typedef struct DM1_V1_F0161F0180SourceAuditPc34Compat {
    uint16_t functionNumber;
    DM1_V1_F0161F0180OwnerKindPc34Compat ownerKind;
    int requiresOriginalMaterial;
    int hostFallbackForbidden;
    const char *redmcsbAnchor;
    const char *firestaffOwner;
} DM1_V1_F0161F0180SourceAuditPc34Compat;

/* Audit only.  The catalog identifies existing source-bound owners for
 * ReDMCSB DUNGEON.C F0161-F0174 and GROUP.C F0175-F0180.  It creates no
 * dungeon state, events, pixels, or host substitute. */
const DM1_V1_F0161F0180SourceAuditPc34Compat *
dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34(uint16_t function_number);

const char *dm1_v1_f0161_f0180_dungeon_group_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
