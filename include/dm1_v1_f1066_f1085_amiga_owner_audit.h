#ifndef FIRESTAFF_DM1_V1_F1066_F1085_AMIGA_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1066_F1085_AMIGA_OWNER_AUDIT_H

#include <stdint.h>

typedef enum DM1V1F1066F1085Admission {
    DM1_V1_F1066_F1085_EXISTING_SOURCE_OWNER = 1,
    DM1_V1_F1066_F1085_PLATFORM_FAIL_CLOSED = 2
} DM1V1F1066F1085Admission;

typedef struct DM1V1F1066F1085Audit {
    uint16_t routine;
    DM1V1F1066F1085Admission admission;
} DM1V1F1066F1085Audit;

const DM1V1F1066F1085Audit *
dm1_v1_f1066_f1085_amiga_owner_audit(uint16_t routine);

#endif
