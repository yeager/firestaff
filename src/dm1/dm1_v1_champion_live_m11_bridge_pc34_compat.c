#include "dm1_v1_champion_live_m11_bridge_pc34_compat.h"

#include <string.h>

void dm1_v1_champion_live_m11_bridge_init_pc34(
    Dm1V1ChampionLiveM11BridgeStatePc34 *state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    dm1_v1_champion_runtime_source_m11_bridge_init_pc34(&state->runtimeSourceState);
}

const char *dm1_v1_champion_live_m11_bridge_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287/F0291 live champion top-row path: "
           "C008/C028, Champion portrait data, statusbar source material, and "
           "C033/C034/C035 hand slots are admitted together before a later M11 "
           "consumer. No fallback or generated artwork is part of this route.";
}

int dm1_v1_champion_live_m11_bridge_pc34(
    Dm1V1ChampionLiveM11BridgeStatePc34 *state,
    const struct PartyState_Compat *party,
    int actingChampionOrdinal,
    const Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *liveEvidence,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *topRowAssets,
    Dm1V1ChampionLiveM11BridgeReceiptPc34 *outReceipt)
{
    Dm1V1ChampionLiveM11BridgeReceiptPc34 pending;
    if (!state || !outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    if (!dm1_v1_champion_runtime_source_receipt_pc34(
            party, actingChampionOrdinal, liveEvidence, topRowAssets,
            &pending.runtimeSource)) return 0;
    if (!dm1_v1_champion_runtime_source_m11_bridge_pc34(
            &state->runtimeSourceState, &pending.runtimeSource, &pending.m11)) return 0;
    pending.clearOnly = pending.m11.clearOnly;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
