#ifndef FIRESTAFF_DM1_V1_L0001_L0050_LOCAL_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_L0001_L0050_LOCAL_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1L0001L0050Audit {
    uint16_t label;
    uint16_t enclosingRoutine;
    int standaloneForbidden;
} DM1V1L0001L0050Audit;

const DM1V1L0001L0050Audit *
dm1_v1_l0001_l0050_local_owner_audit(uint16_t label, uint16_t enclosingRoutine);

#endif
