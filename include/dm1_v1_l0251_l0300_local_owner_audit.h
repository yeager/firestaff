#ifndef FIRESTAFF_DM1_V1_L0251_L0300_LOCAL_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_L0251_L0300_LOCAL_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1L0251L0300LocalOwnerAudit {
    uint16_t label;
    uint16_t enclosing_routine;
    const char *redmcsb_anchor;
    const char *firestaff_owner;
    int standalone_local_forbidden;
    int fallback_or_synthetic_state_forbidden;
} DM1V1L0251L0300LocalOwnerAudit;

const DM1V1L0251L0300LocalOwnerAudit *
dm1_v1_l0251_l0300_local_owner_find(uint16_t label);
const char *dm1_v1_l0251_l0300_local_owner_evidence(void);

#endif /* FIRESTAFF_DM1_V1_L0251_L0300_LOCAL_OWNER_AUDIT_H */
