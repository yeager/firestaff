#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_RUNTIME_M11_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_RUNTIME_M11_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_runtime_host_output_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34 {
    unsigned int lastTick;
    unsigned int pendingClearGeneration;
    unsigned int lastCompositionGeneration;
    int m11CompositionActive;
} Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34;

typedef struct Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    Dm1V1ChampionTopRowRuntimeHostOutputActionPc34 action;
    Dm1V1ChampionTopRowM11OriginalMaterialsPc34 originalMaterials;
    int commandCount;
    Dm1V1ChampionTopRowM11HostRenderCommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34;

void dm1_v1_champion_top_row_runtime_m11_bridge_init_pc34(
    Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34 *state);

/* Admits runtime host output at the M11 boundary without invoking M11. Full
 * publish requires complete retained C008/C028/palette/surface proof; clear
 * and revoke output are material-free by construction. */
int dm1_v1_champion_top_row_runtime_m11_bridge_pc34(
    Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34 *state,
    const Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *hostOutput,
    const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *originalMaterials,
    Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_runtime_m11_bridge_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
