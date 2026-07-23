#ifndef FIRESTAFF_DM1_V1_F1246_F1265_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1246_F1265_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1F1246F1265Audit {
    uint16_t routine;
    int failClosed;
} DM1V1F1246F1265Audit;

const DM1V1F1246F1265Audit *
dm1_v1_f1246_f1265_owner_audit(uint16_t routine);

#endif
