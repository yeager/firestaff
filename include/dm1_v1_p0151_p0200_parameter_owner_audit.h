#ifndef FIRESTAFF_DM1_V1_P0151_P0200_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0151_P0200_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0151P0200Audit {
    uint16_t parameter;
    uint16_t enclosingRoutine;
    int standaloneForbidden;
} DM1V1P0151P0200Audit;

const DM1V1P0151P0200Audit *
dm1_v1_p0151_p0200_parameter_owner_audit(
    uint16_t parameter,
    uint16_t enclosingRoutine);

#endif
