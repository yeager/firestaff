#ifndef FIRESTAFF_DM1_V1_L0966_L0985_CHAMPION_OWNER_AUDIT_H
#define FIRESTAFF_DM1_V1_L0966_L0985_CHAMPION_OWNER_AUDIT_H

#include <stdint.h>

/* ReDMCSB CHAMPION.C locals, not independently serializable PC34 records. */
typedef enum DM1V1L0966L0985Owner {
    DM1_V1_L0966_L0985_OWNER_EXISTING_CHAMPION = 1,
    DM1_V1_L0966_L0985_OWNER_FAIL_CLOSED = 2
} DM1V1L0966L0985Owner;

typedef struct DM1V1L0966L0985Audit {
    uint16_t label;
    uint16_t routine;
    DM1V1L0966L0985Owner owner;
} DM1V1L0966L0985Audit;

const DM1V1L0966L0985Audit *
dm1_v1_l0966_l0985_champion_owner_audit(uint16_t label);

#endif
