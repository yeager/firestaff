#ifndef FIRESTAFF_DM1_V1_F1146_F1165_IO_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1146_F1165_IO_OWNER_AUDIT_H

#include <stdint.h>

typedef struct DM1V1F1146F1165Audit {
    uint16_t routine;
    int failClosed;
} DM1V1F1146F1165Audit;

const DM1V1F1146F1165Audit *
dm1_v1_f1146_f1165_io_owner_audit(uint16_t routine);

#endif
