#ifndef FIRESTAFF_DM1_V1_MELEE_ACTION_F0402_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MELEE_ACTION_F0402_PC34_COMPAT_H

#include "memory_tick_orchestrator_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int championIndex;
    int actionIndex;
    int championPresent;
    int championDirection;
} DM1_MeleeActionTickInputPc34;

typedef struct {
    int valid;
    unsigned char command;
    unsigned char commandArg1;
    unsigned char commandArg2;
    unsigned char reserved;
    unsigned int reserved2;
    int hasTargetDirection;
    int targetDirection;
} DM1_MeleeActionTickPlanPc34;

int dm1_v1_melee_action_tick_plan_f0402_pc34(
    const DM1_MeleeActionTickInputPc34* in,
    DM1_MeleeActionTickPlanPc34* out);

#ifdef __cplusplus
}
#endif

#endif
