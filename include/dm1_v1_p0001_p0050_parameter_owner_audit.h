#ifndef FIRESTAFF_DM1_V1_P0001_P0050_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0001_P0050_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0001P0050Audit {
    uint16_t parameter;
    uint16_t enclosingRoutine;
    int standaloneForbidden;
} DM1V1P0001P0050Audit;

const DM1V1P0001P0050Audit *
dm1_v1_p0001_p0050_parameter_owner_audit(
    uint16_t parameter,
    uint16_t enclosingRoutine);

#endif
