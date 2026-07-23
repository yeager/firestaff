#ifndef FIRESTAFF_DM1_V1_P0501_P0550_PARAMETER_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_P0501_P0550_PARAMETER_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1P0501P0550Audit {
    uint16_t parameter;
    uint16_t ownerRoutine;
} DM1V1P0501P0550Audit;

const DM1V1P0501P0550Audit *
dm1_v1_p0501_p0550_parameter_owner_audit(uint16_t parameter);

#endif
