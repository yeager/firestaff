#ifndef FIRESTAFF_DM1_V1_F1366_F1385_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1366_F1385_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1F1366F1385Audit {
    uint16_t routine;
    int failClosed;
} DM1V1F1366F1385Audit;

const DM1V1F1366F1385Audit *
dm1_v1_f1366_f1385_owner_audit(uint16_t routine);

#endif
