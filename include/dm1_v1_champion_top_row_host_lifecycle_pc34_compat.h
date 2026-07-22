#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_HOST_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_HOST_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_host_consumption_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowHostLifecycleStatePc34 {
    unsigned int lastTick;
    unsigned int lastClearGeneration;
    unsigned int lastCompositionGeneration;
    unsigned int pendingClearGeneration;
} Dm1V1ChampionTopRowHostLifecycleStatePc34;

typedef struct Dm1V1ChampionTopRowHostLifecycleReceiptPc34 {
    int valid;
    unsigned int tick;
    Dm1V1ChampionTopRowAtomicLifecyclePublicationPc34 publication;
    unsigned int generation;
    Dm1V1ChampionTopRowHostConsumptionReceiptPc34 hostConsumption;
} Dm1V1ChampionTopRowHostLifecycleReceiptPc34;

void dm1_v1_champion_top_row_host_lifecycle_init_pc34(
    Dm1V1ChampionTopRowHostLifecycleStatePc34 *state);

/* Advances the host-consumption state by one strictly newer tick. A host
 * composition is accepted only for the current pending clear generation;
 * a prior composition becomes stale immediately when another clear arrives. */
int dm1_v1_champion_top_row_host_lifecycle_step_pc34(
    Dm1V1ChampionTopRowHostLifecycleStatePc34 *state,
    unsigned int tick,
    const Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *hostConsumption,
    Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_host_lifecycle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
