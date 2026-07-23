#ifndef FIRESTAFF_DM1_V1_F1426_F1445_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1426_F1445_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1F1426F1445Audit {
    uint16_t routine;
    int failClosed;
} DM1V1F1426F1445Audit;

const DM1V1F1426F1445Audit *
dm1_v1_f1426_f1445_owner_audit(uint16_t routine);

#endif
