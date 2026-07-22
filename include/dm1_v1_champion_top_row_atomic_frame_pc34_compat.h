#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_PC34_COMPAT_H

#include "dm1_v1_champion_status_bar_frame_presentation_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34 32

typedef enum Dm1V1ChampionTopRowAtomicFrameOperationPc34 {
    DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_ATOMIC_BLIT_C008_PC34,
    DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34,
    DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34
} Dm1V1ChampionTopRowAtomicFrameOperationPc34;

typedef struct Dm1V1ChampionTopRowAtomicFrameOpPc34 {
    Dm1V1ChampionTopRowAtomicFrameOperationPc34 operation;
    int championIndex;
    int zoneId;
    int x;
    int y;
    int width;
    int height;
    int graphicIndex;
    const uint8_t *sourcePixels;
    const uint8_t *portraitPixels;
    Dm1V1ChampionStatusBarFrameOpPc34 statusBar;
} Dm1V1ChampionTopRowAtomicFrameOpPc34;

typedef struct Dm1V1ChampionTopRowAtomicFrameReceiptPc34 {
    int valid;
    int originalMaterialsPublished;
    int clearOnly;
    int operationCount;
    Dm1V1ChampionTopRowAtomicFrameOpPc34
        operations[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionTopRowAtomicFrameReceiptPc34;

/* Publishes top-row portrait/status sources and the statusbar frame as one
 * receipt. C008, C028, and the statusbar surface/palette must all be ready;
 * otherwise all selected champion regions are explicitly cleared together. */
int dm1_v1_champion_top_row_atomic_frame_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *portraitPolicy,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets,
    const Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *statusBars,
    Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_atomic_frame_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
