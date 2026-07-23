#ifndef FIRESTAFF_DM1_V1_P0201_P0250_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0201_P0250_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef enum DM1V1P0201P0250Admission {
    DM1_V1_P0201_P0250_EXISTING_PC34_OWNER = 1,
    DM1_V1_P0201_P0250_FAIL_CLOSED = 2
} DM1V1P0201P0250Admission;

typedef struct DM1V1P0201P0250Audit {
    uint16_t parameter;
    uint16_t ownerRoutine;
    DM1V1P0201P0250Admission admission;
} DM1V1P0201P0250Audit;

const DM1V1P0201P0250Audit *
dm1_v1_p0201_p0250_parameter_owner_audit(uint16_t parameter);

#endif
