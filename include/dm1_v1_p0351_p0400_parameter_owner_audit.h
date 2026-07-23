#ifndef FIRESTAFF_DM1_V1_P0351_P0400_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0351_P0400_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0351P0400Audit {
    uint16_t parameter;
    uint16_t ownerRoutine;
} DM1V1P0351P0400Audit;

const DM1V1P0351P0400Audit *
dm1_v1_p0351_p0400_parameter_owner_audit(uint16_t parameter);

#endif
