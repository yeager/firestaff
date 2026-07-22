#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_F0296_TRANSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_F0296_TRANSITION_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_presentation_pc34_compat.h"
#include "firestaff/dm1/v1/champion_panel/hand_slot_refresh_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_TOP_ROW_F0296_MAX_OPS_PC34 32

typedef enum Dm1V1ChampionTopRowF0296TransitionKindPc34 {
    DM1_V1_CHAMPION_TOP_ROW_F0296_INVALID_PC34 = 0,
    DM1_V1_CHAMPION_TOP_ROW_F0296_CHANGED_HAND_PC34,
    DM1_V1_CHAMPION_TOP_ROW_F0296_CANDIDATE_EARLY_RETURN_PC34,
    DM1_V1_CHAMPION_TOP_ROW_F0296_INVENTORY_F0292_REPAINT_PC34,
    DM1_V1_CHAMPION_TOP_ROW_F0296_DEAD_STATUS_REPAINT_PC34
} Dm1V1ChampionTopRowF0296TransitionKindPc34;

typedef struct Dm1V1ChampionTopRowF0296TransitionOpPc34 {
    int presentationOperationIndex;
    int championSlot;
    int zoneId;
    int graphicIndex;
    const uint8_t *sourcePixels;
} Dm1V1ChampionTopRowF0296TransitionOpPc34;

typedef struct Dm1V1ChampionTopRowF0296TransitionReceiptPc34 {
    int valid;
    Dm1V1ChampionTopRowF0296TransitionKindPc34 kind;
    int candidateSuppressed;
    int inventoryChampionSlot;
    int deadChampionSlot;
    int operationCount;
    Dm1V1ChampionTopRowF0296TransitionOpPc34
        operations[DM1_V1_CHAMPION_TOP_ROW_F0296_MAX_OPS_PC34];
} Dm1V1ChampionTopRowF0296TransitionReceiptPc34;

/* Consumes a source-bound top-row presentation receipt after F0296. Candidate
 * early-return emits no draws; inventory owner uses the F0292 repaint slice;
 * a rejected dead-member refresh retains only the C008 dead-status slice. */
int dm1_v1_champion_top_row_f0296_transition_from_refresh_pc34(
    const Dm1V1ChampionTopRowPresentationReceiptPc34 *presentation,
    const Dm1V1ChampionPanelHandSlotRefreshStatePc34 *refreshState,
    const Dm1V1ChampionPanelHandSlotRefreshResultPc34 *refreshResult,
    Dm1V1ChampionTopRowF0296TransitionReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_f0296_transition_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
