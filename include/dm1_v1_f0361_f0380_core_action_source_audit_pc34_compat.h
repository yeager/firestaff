#ifndef FIRESTAFF_DM1_V1_F0361_F0380_CORE_ACTION_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0361_F0380_CORE_ACTION_SOURCE_AUDIT_PC34_COMPAT_H

#include <stdint.h>

typedef enum DM1_V1_F0361F0380AuditKindPc34Compat {
    DM1_V1_F0361_F0380_AUDIT_EXISTING_OWNER = 1,
    DM1_V1_F0361_F0380_AUDIT_FAIL_CLOSED_BOUNDARY = 2
} DM1_V1_F0361F0380AuditKindPc34Compat;

typedef struct DM1_V1_F0361F0380SourceAuditPc34Compat {
    uint16_t functionNumber;
    DM1_V1_F0361F0380AuditKindPc34Compat auditKind;
    int requiresOriginalMaterial;
    int hostFallbackForbidden;
    const char *redmcsbAnchor;
    const char *firestaffOwner;
} DM1_V1_F0361F0380SourceAuditPc34Compat;

/* Audit only. It exposes no command, input, click, queue, or render path. */
const DM1_V1_F0361F0380SourceAuditPc34Compat *
dm1_v1_f0361_f0380_core_action_source_audit_pc34(uint16_t function_number);
const char *dm1_v1_f0361_f0380_core_action_source_evidence_pc34(void);

#endif
