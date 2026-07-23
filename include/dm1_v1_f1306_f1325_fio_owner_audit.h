#ifndef FIRESTAFF_DM1_V1_F1306_F1325_FIO_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1306_F1325_FIO_OWNER_AUDIT_H

#include <stdint.h>

typedef enum DM1V1F1306F1325Admission {
    DM1_V1_F1306_F1325_EXISTING_SOURCE_OWNER = 1,
    DM1_V1_F1306_F1325_FAIL_CLOSED = 2
} DM1V1F1306F1325Admission;

typedef struct DM1V1F1306F1325Audit {
    uint16_t routine;
    DM1V1F1306F1325Admission admission;
} DM1V1F1306F1325Audit;

const DM1V1F1306F1325Audit *
dm1_v1_f1306_f1325_fio_owner_audit(uint16_t routine);

#endif
