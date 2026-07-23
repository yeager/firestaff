#ifndef FIRESTAFF_DM1_V1_CHAMPION_REDRAW_PRIORITY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_REDRAW_PRIORITY_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_presentation_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_REDRAW_PRIORITY_MAX_OPS_PC34 64

typedef struct Dm1V1ChampionRedrawSurfacePc34 {
    int graphicIndex;
    int loaded;
    const uint8_t *pixels;
    int width;
    int height;
} Dm1V1ChampionRedrawSurfacePc34;

typedef struct Dm1V1ChampionRedrawMaterialsPc34 {
    Dm1V1ChampionRedrawSurfacePc34 poisonLabel;
    Dm1V1ChampionRedrawSurfacePc34 damageSmall;
    Dm1V1ChampionRedrawSurfacePc34 damageBig;
} Dm1V1ChampionRedrawMaterialsPc34;

typedef struct Dm1V1ChampionRedrawStatePc34 {
    int partyChampionCount;
    int inventoryChampionIndex;
    int present[4];
    int currentHealth[4];
    int poisonDose[4];
    int pendingDamage[4];
    /* F0320's pending-damage value is carried with the source material
     * receipt. A timer alone is not sufficient to author the 1/2/3-digit
     * F0053 damage string. */
    int pendingDamageAmount[4];
} Dm1V1ChampionRedrawStatePc34;

typedef enum Dm1V1ChampionRedrawPriorityKindPc34 {
    DM1_V1_CHAMPION_REDRAW_STATUS_PC34 = 1,
    DM1_V1_CHAMPION_REDRAW_POISON_PC34,
    DM1_V1_CHAMPION_REDRAW_DAMAGE_PC34
} Dm1V1ChampionRedrawPriorityKindPc34;

typedef struct Dm1V1ChampionRedrawPriorityOpPc34 {
    Dm1V1ChampionRedrawPriorityKindPc34 kind;
    int championSlot;
    int priority;
    int presentationOperationIndex;
    int graphicIndex;
    const uint8_t *sourcePixels;
    int pendingDamageAmount;
    int x;
    int y;
    int width;
    int height;
} Dm1V1ChampionRedrawPriorityOpPc34;

typedef struct Dm1V1ChampionRedrawPriorityReceiptPc34 {
    int valid;
    int operationCount;
    Dm1V1ChampionRedrawPriorityOpPc34
        operations[DM1_V1_CHAMPION_REDRAW_PRIORITY_MAX_OPS_PC34];
} Dm1V1ChampionRedrawPriorityReceiptPc34;

/* F0293 iterates champion state in party order and F0292 emits the status
 * lane before any selected poison/damage overlay. Dead status is terminal for
 * the live-only poison/damage lanes. */
int dm1_v1_champion_redraw_priority_from_top_row_pc34(
    const Dm1V1ChampionTopRowPresentationReceiptPc34 *topRow,
    const Dm1V1ChampionRedrawStatePc34 *state,
    const Dm1V1ChampionRedrawMaterialsPc34 *materials,
    Dm1V1ChampionRedrawPriorityReceiptPc34 *outReceipt);

const char *dm1_v1_champion_redraw_priority_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
