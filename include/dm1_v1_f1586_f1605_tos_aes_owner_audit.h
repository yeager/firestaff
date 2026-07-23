#ifndef FIRESTAFF_DM1_V1_F1586_F1605_TOS_AES_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1586_F1605_TOS_AES_OWNER_AUDIT_H

#include <stdint.h>

typedef enum DM1V1F1586F1605Admission {
    DM1_V1_F1586_F1605_PLATFORM_FAIL_CLOSED = 1,
    DM1_V1_F1586_F1605_ABSENT_FAIL_CLOSED = 2
} DM1V1F1586F1605Admission;

typedef struct DM1V1F1586F1605Audit {
    uint16_t routine;
    DM1V1F1586F1605Admission admission;
} DM1V1F1586F1605Audit;

const DM1V1F1586F1605Audit *
dm1_v1_f1586_f1605_tos_aes_owner_audit(uint16_t routine);

#endif
