#ifndef FIRESTAFF_DM1_V1_P0451_P0500_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0451_P0500_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0451P0500Audit {
    uint16_t parameter;
    uint16_t ownerRoutine;
} DM1V1P0451P0500Audit;

const DM1V1P0451P0500Audit *
dm1_v1_p0451_p0500_parameter_owner_audit(uint16_t parameter);

#endif
