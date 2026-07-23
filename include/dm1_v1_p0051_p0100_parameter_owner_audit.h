#ifndef FIRESTAFF_DM1_V1_P0051_P0100_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0051_P0100_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0051P0100Audit {
    uint16_t parameter;
    uint16_t enclosingRoutine;
    int standaloneForbidden;
} DM1V1P0051P0100Audit;

const DM1V1P0051P0100Audit *
dm1_v1_p0051_p0100_parameter_owner_audit(
    uint16_t parameter,
    uint16_t enclosingRoutine);

#endif
