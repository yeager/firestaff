#ifndef FIRESTAFF_DM1_V1_F1786_F1825_ANIMATION_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1786_F1825_ANIMATION_OWNER_AUDIT_H

#include <stdint.h>

typedef enum DM1V1F1786F1825Admission {
    DM1_V1_F1786_F1825_REFERENCE_FAIL_CLOSED = 1,
    DM1_V1_F1786_F1825_ABSENT_FAIL_CLOSED = 2
} DM1V1F1786F1825Admission;

typedef struct DM1V1F1786F1825Audit {
    uint16_t routine;
    DM1V1F1786F1825Admission admission;
} DM1V1F1786F1825Audit;

const DM1V1F1786F1825Audit *
dm1_v1_f1786_f1825_animation_owner_audit(uint16_t routine);

#endif
