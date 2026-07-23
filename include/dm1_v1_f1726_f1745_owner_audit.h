#ifndef FIRESTAFF_DM1_V1_F1726_F1745_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1726_F1745_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1F1726F1745Audit {
    uint16_t routine;
    int failClosed;
} DM1V1F1726F1745Audit;

const DM1V1F1726F1745Audit *
dm1_v1_f1726_f1745_owner_audit(uint16_t routine);

#endif
