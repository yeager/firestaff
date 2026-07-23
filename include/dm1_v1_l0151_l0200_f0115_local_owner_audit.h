#ifndef FIRESTAFF_DM1_V1_L0151_L0200_F0115_LOCAL_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_L0151_L0200_F0115_LOCAL_OWNER_AUDIT_H

#include <stddef.h>
#include <stdint.h>

typedef enum DM1V1L0151L0200OwnerKind {
    DM1_V1_L0151L0200_OWNER_F0115_CREATURE_MATERIAL = 0,
    DM1_V1_L0151L0200_OWNER_F0115_PROJECTILE_EFFECT,
    DM1_V1_L0151L0200_OWNER_F0116_SQUARE_ORDER
} DM1V1L0151L0200OwnerKind;

typedef struct DM1V1L0151L0200LocalOwnerAudit {
    uint16_t label;
    uint16_t enclosing_routine;
    const char *redmcsb_anchor;
    const char *firestaff_owner;
    DM1V1L0151L0200OwnerKind owner_kind;
    int standalone_local_forbidden;
    int fallback_or_synthetic_state_forbidden;
} DM1V1L0151L0200LocalOwnerAudit;

const DM1V1L0151L0200LocalOwnerAudit *
dm1_v1_l0151_l0200_local_owner_audit(size_t *out_count);
const DM1V1L0151L0200LocalOwnerAudit *
dm1_v1_l0151_l0200_local_owner_find(uint16_t label, uint16_t enclosing_routine);
const char *dm1_v1_l0151_l0200_local_owner_evidence(void);

#endif /* FIRESTAFF_DM1_V1_L0151_L0200_F0115_LOCAL_OWNER_AUDIT_H */
