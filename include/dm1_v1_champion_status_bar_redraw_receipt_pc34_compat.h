#ifndef FIRESTAFF_DM1_V1_CHAMPION_STATUS_BAR_REDRAW_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_STATUS_BAR_REDRAW_RECEIPT_PC34_COMPAT_H

#include "dm1_v1_champion_portrait_status_redraw_policy_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_STATUS_BAR_REDRAW_MAX_OPS_PC34 24

typedef enum Dm1V1ChampionStatusBarRedrawOperationPc34 {
    DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34
} Dm1V1ChampionStatusBarRedrawOperationPc34;

typedef struct Dm1V1ChampionStatusBarRedrawOpPc34 {
    Dm1V1ChampionStatusBarRedrawOperationPc34 operation;
    Dm1V1ChampionPortraitStatusRedrawRoutePc34 route;
    int championIndex;
    int statIndex;
    int zoneId;
    int x;
    int y;
    int width;
    int height;
    int color;
    int current;
    int maximum;
} Dm1V1ChampionStatusBarRedrawOpPc34;

typedef struct Dm1V1ChampionStatusBarRedrawReceiptPc34 {
    int valid;
    int dataGateAccepted;
    int clearCount;
    int repaintCount;
    int operationCount;
    Dm1V1ChampionStatusBarRedrawOpPc34
        operations[DM1_V1_CHAMPION_STATUS_BAR_REDRAW_MAX_OPS_PC34];
} Dm1V1ChampionStatusBarRedrawReceiptPc34;

/* Turns portrait/status ownership into exact PC34 status-bar operations.
 * A source-owned lane repaints only validated original current/maximum data.
 * A clear lane, dead champion, or absent original material clears its full
 * bars. Any inconsistent party/policy/stat state rejects the entire receipt. */
int dm1_v1_champion_status_bar_redraw_receipt_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *portraitPolicy,
    Dm1V1ChampionStatusBarRedrawReceiptPc34 *outReceipt);

const char *dm1_v1_champion_status_bar_redraw_receipt_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
