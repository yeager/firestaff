#ifndef FIRESTAFF_DM1_V1_EARLY_VIEWPORT_FAMILY_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_EARLY_VIEWPORT_FAMILY_AUDIT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_EarlyViewportOwnerPc34Compat {
    DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE = 1,
    DM1_V1_EARLY_VIEWPORT_OWNER_CEILING_PIT = 2,
    DM1_V1_EARLY_VIEWPORT_OWNER_FIELD_OR_THING = 3,
    DM1_V1_EARLY_VIEWPORT_OWNER_SQUARE_SCHEDULER = 4
} DM1_V1_EarlyViewportOwnerPc34Compat;

typedef struct DM1_V1_EarlyViewportAuditPc34Compat {
    uint16_t functionNumber;
    DM1_V1_EarlyViewportOwnerPc34Compat owner;
    int requiresSourceReceipt;
    int hostFallbackForbidden;
    const char *sourceAnchor;
    const char *firestaffOwner;
} DM1_V1_EarlyViewportAuditPc34Compat;

/* ReDMCSB DUNVIEW.C F0100-F0120 source-owner catalog. This is audit-only:
 * callers must use the listed existing DM1 owner and no new renderer, UI, or
 * synthetic runtime path is exposed here. */
const DM1_V1_EarlyViewportAuditPc34Compat *
dm1_v1_early_viewport_family_audit_pc34(uint16_t function_number);

const char *dm1_v1_early_viewport_family_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
