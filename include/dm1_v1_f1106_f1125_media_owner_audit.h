#ifndef FIRESTAFF_DM1_V1_F1106_F1125_MEDIA_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_F1106_F1125_MEDIA_OWNER_AUDIT_H

#include <stdint.h>

typedef enum DM1V1F1106F1125Admission {
    DM1_V1_F1106_F1125_EXISTING_SOURCE_OWNER = 1,
    DM1_V1_F1106_F1125_FAIL_CLOSED = 2
} DM1V1F1106F1125Admission;

typedef struct DM1V1F1106F1125Audit {
    uint16_t routine;
    DM1V1F1106F1125Admission admission;
} DM1V1F1106F1125Audit;

const DM1V1F1106F1125Audit *
dm1_v1_f1106_f1125_media_owner_audit(uint16_t routine);

#endif
