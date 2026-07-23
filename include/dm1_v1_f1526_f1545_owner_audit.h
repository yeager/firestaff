#ifndef FIRESTAFF_DM1_V1_F1526_F1545_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1526_F1545_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1F1526F1545Audit {
    uint16_t routine;
    int failClosed;
} DM1V1F1526F1545Audit;

const DM1V1F1526F1545Audit *
dm1_v1_f1526_f1545_owner_audit(uint16_t routine);

#endif
