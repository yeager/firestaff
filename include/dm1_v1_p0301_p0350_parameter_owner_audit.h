#ifndef FIRESTAFF_DM1_V1_P0301_P0350_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0301_P0350_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0301P0350Audit {
    uint16_t parameter;
    uint16_t ownerRoutine;
} DM1V1P0301P0350Audit;

const DM1V1P0301P0350Audit *
dm1_v1_p0301_p0350_parameter_owner_audit(uint16_t parameter);

#endif
