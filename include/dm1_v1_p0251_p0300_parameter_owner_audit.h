#ifndef FIRESTAFF_DM1_V1_P0251_P0300_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0251_P0300_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0251P0300Audit {
    uint16_t parameter;
    uint16_t ownerRoutine;
} DM1V1P0251P0300Audit;

const DM1V1P0251P0300Audit *
dm1_v1_p0251_p0300_parameter_owner_audit(uint16_t parameter);

#endif
