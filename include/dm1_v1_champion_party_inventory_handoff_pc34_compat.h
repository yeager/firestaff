#ifndef FIRESTAFF_DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_PC34_COMPAT_H

#include "dm1_v1_champion_redraw_priority_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_MAX_OPS_PC34 96

typedef struct Dm1V1ChampionPartyInventorySwitchPc34 {
    int partyChampionCount;
    int inventoryChampionBefore;
    int inventoryChampionAfter;
} Dm1V1ChampionPartyInventorySwitchPc34;

typedef enum Dm1V1ChampionPartyInventoryHandoffKindPc34 {
    DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_TOP_ROW_PC34 = 1,
    DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_REDRAW_PC34
} Dm1V1ChampionPartyInventoryHandoffKindPc34;

typedef struct Dm1V1ChampionPartyInventoryHandoffOpPc34 {
    Dm1V1ChampionPartyInventoryHandoffKindPc34 kind;
    int sourceOperationIndex;
    int championSlot;
    int zoneId;
    int graphicIndex;
    const uint8_t *sourcePixels;
    int pendingDamageAmount;
} Dm1V1ChampionPartyInventoryHandoffOpPc34;

typedef struct Dm1V1ChampionPartyInventoryHandoffReceiptPc34 {
    int valid;
    Dm1V1ChampionPartyInventorySwitchPc34 transition;
    Dm1V1ChampionTopRowAssetsReceiptPc34 topRowAssets;
    Dm1V1ChampionRedrawMaterialsPc34 redrawMaterials;
    int operationCount;
    Dm1V1ChampionPartyInventoryHandoffOpPc34
        operations[DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_MAX_OPS_PC34];
} Dm1V1ChampionPartyInventoryHandoffReceiptPc34;

/* ReDMCSB F0355/F0293 handoff: retain party status material before and after
 * an inventory-owner switch, then consume the ordered F0292/F0320 priority
 * receipt. This is renderer-neutral and has no replacement artwork path. */
int dm1_v1_champion_party_inventory_handoff_pc34(
    const Dm1V1ChampionTopRowPresentationReceiptPc34 *topRow,
    const Dm1V1ChampionRedrawPriorityReceiptPc34 *redraw,
    const Dm1V1ChampionPartyInventorySwitchPc34 *transition,
    const Dm1V1ChampionRedrawMaterialsPc34 *redrawMaterials,
    Dm1V1ChampionPartyInventoryHandoffReceiptPc34 *outReceipt);

const char *dm1_v1_champion_party_inventory_handoff_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
