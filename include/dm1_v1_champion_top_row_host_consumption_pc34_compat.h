#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_HOST_CONSUMPTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_HOST_CONSUMPTION_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_lifecycle_render_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowHostConsumptionStatePc34 {
    unsigned int lastClearGeneration;
    unsigned int lastCompositionGeneration;
    unsigned int pendingClearGeneration;
} Dm1V1ChampionTopRowHostConsumptionStatePc34;

typedef struct Dm1V1ChampionTopRowHostConsumptionOpPc34 {
    Dm1V1ChampionTopRowLifecycleRenderCommandKindPc34 kind;
    Dm1V1ChampionTopRowAtomicFrameOpPc34 source;
} Dm1V1ChampionTopRowHostConsumptionOpPc34;

typedef struct Dm1V1ChampionTopRowHostConsumptionReceiptPc34 {
    int valid;
    Dm1V1ChampionTopRowAtomicLifecyclePublicationPc34 publication;
    unsigned int generation;
    int operationCount;
    Dm1V1ChampionTopRowHostConsumptionOpPc34
        operations[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowHostConsumptionReceiptPc34;

void dm1_v1_champion_top_row_host_consumption_init_pc34(
    Dm1V1ChampionTopRowHostConsumptionStatePc34 *state);

/* Authorizes one bridge receipt for host consumption. A composition is fresh
 * only when its generation equals the immediately preceding consumed clear;
 * stale original surfaces/palettes cannot cross this boundary. */
int dm1_v1_champion_top_row_host_consumption_pc34(
    Dm1V1ChampionTopRowHostConsumptionStatePc34 *state,
    const Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *renderReceipt,
    Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_host_consumption_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
