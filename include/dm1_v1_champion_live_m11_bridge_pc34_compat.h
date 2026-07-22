#ifndef FIRESTAFF_DM1_V1_CHAMPION_LIVE_M11_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_LIVE_M11_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_champion_runtime_source_m11_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionLiveM11BridgeStatePc34 {
    Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 runtimeSourceState;
} Dm1V1ChampionLiveM11BridgeStatePc34;

typedef struct Dm1V1ChampionLiveM11BridgeReceiptPc34 {
    int valid;
    int clearOnly;
    Dm1V1ChampionRuntimeSourceReceiptPc34 runtimeSource;
    Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 m11;
} Dm1V1ChampionLiveM11BridgeReceiptPc34;

void dm1_v1_champion_live_m11_bridge_init_pc34(
    Dm1V1ChampionLiveM11BridgeStatePc34 *state);

/* Builds the live source receipt from party, C008/C028/C033-C035 and retained
 * statusbar evidence, then crosses the source-only M11 boundary. Repeated or
 * stale live evidence is represented only by the underlying clear receipt. */
int dm1_v1_champion_live_m11_bridge_pc34(
    Dm1V1ChampionLiveM11BridgeStatePc34 *state,
    const struct PartyState_Compat *party,
    int actingChampionOrdinal,
    const Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *liveEvidence,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *topRowAssets,
    Dm1V1ChampionLiveM11BridgeReceiptPc34 *outReceipt);

const char *dm1_v1_champion_live_m11_bridge_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
