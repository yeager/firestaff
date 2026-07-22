#ifndef FIRESTAFF_DM1_V1_CHAMPION_LIVE_RUNTIME_COMPOSITION_EVIDENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_LIVE_RUNTIME_COMPOSITION_EVIDENCE_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_runtime_m11_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Dm1V1ChampionLiveRuntimeCompositionKindPc34 {
    DM1_V1_CHAMPION_LIVE_RUNTIME_C008_PC34 = 1,
    DM1_V1_CHAMPION_LIVE_RUNTIME_C028_PC34,
    DM1_V1_CHAMPION_LIVE_RUNTIME_STATUS_BAR_PC34
} Dm1V1ChampionLiveRuntimeCompositionKindPc34;

typedef struct Dm1V1ChampionLiveRuntimeCompositionEvidencePc34 {
    Dm1V1ChampionLiveRuntimeCompositionKindPc34 kind;
    int championIndex;
    int zoneId;
    const uint8_t *originalPixels;
    const uint8_t *portraitPixels;
    const uint8_t *originalPalette;
    uint8_t *originalSurface;
} Dm1V1ChampionLiveRuntimeCompositionEvidencePc34;

typedef struct Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 {
    int valid;
    unsigned int tick;
    unsigned int generation;
    int evidenceCount;
    Dm1V1ChampionTopRowM11OriginalMaterialsPc34 originalMaterials;
    Dm1V1ChampionLiveRuntimeCompositionEvidencePc34
        evidence[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionLiveRuntimeCompositionReceiptPc34;

/* Proves that a live, publishable runtime top-row is composed exclusively of
 * original C008/C028, party portrait bitmap, and statusbar palette/surface
 * material. Pointer identity is required: no fallback or generated artwork
 * can enter this receipt. */
int dm1_v1_champion_live_runtime_composition_evidence_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *topRowAssets,
    const Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *statusBars,
    const Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 *runtimeBridge,
    Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *outReceipt);

const char *dm1_v1_champion_live_runtime_composition_evidence_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
