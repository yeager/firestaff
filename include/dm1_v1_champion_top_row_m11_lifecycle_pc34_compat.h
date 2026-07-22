#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_m11_consumption_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowM11LifecycleStatePc34 {
    unsigned int lastTick;
    unsigned int lastClearGeneration;
    unsigned int pendingClearGeneration;
    unsigned int lastCompositionGeneration;
} Dm1V1ChampionTopRowM11LifecycleStatePc34;

typedef enum Dm1V1ChampionTopRowM11LifecyclePublicationPc34 {
    DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_COMPOSITION_PC34,
    DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_STALE_ZONE_CLEAR_PC34
} Dm1V1ChampionTopRowM11LifecyclePublicationPc34;

typedef struct Dm1V1ChampionTopRowM11LifecycleClearOpPc34 {
    int championIndex;
    int zoneId;
    int x;
    int y;
    int width;
    int height;
} Dm1V1ChampionTopRowM11LifecycleClearOpPc34;

typedef struct Dm1V1ChampionTopRowM11LifecycleReceiptPc34 {
    int valid;
    unsigned int tick;
    Dm1V1ChampionTopRowM11LifecyclePublicationPc34 publication;
    unsigned int generation;
    int clearOperationCount;
    Dm1V1ChampionTopRowM11LifecycleClearOpPc34
        clearOperations[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
    Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 consumption;
} Dm1V1ChampionTopRowM11LifecycleReceiptPc34;

void dm1_v1_champion_top_row_m11_lifecycle_init_pc34(
    Dm1V1ChampionTopRowM11LifecycleStatePc34 *state);

/* Advances active top-row consumption across ticks. A stale composition or
 * stale original proof clears only its status/bar zones and leaves a newer
 * pending clear generation available for the matching composition. */
int dm1_v1_champion_top_row_m11_lifecycle_step_pc34(
    Dm1V1ChampionTopRowM11LifecycleStatePc34 *state,
    const Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *consumption,
    Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_m11_lifecycle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
