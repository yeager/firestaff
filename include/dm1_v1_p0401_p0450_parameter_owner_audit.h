#ifndef FIRESTAFF_DM1_V1_P0401_P0450_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0401_P0450_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0401P0450Audit {
    uint16_t parameter;
    uint16_t ownerRoutine;
} DM1V1P0401P0450Audit;

const DM1V1P0401P0450Audit *
dm1_v1_p0401_p0450_parameter_owner_audit(uint16_t parameter);

#endif
