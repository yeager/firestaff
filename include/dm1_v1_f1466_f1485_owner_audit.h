#ifndef FIRESTAFF_DM1_V1_F1466_F1485_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1466_F1485_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1F1466F1485Audit {
    uint16_t routine;
    int failClosed;
} DM1V1F1466F1485Audit;

const DM1V1F1466F1485Audit *
dm1_v1_f1466_f1485_owner_audit(uint16_t routine);

#endif
