#ifndef FIRESTAFF_DM1_V1_CHAMPION_INVENTORY_SLOT_REDRAW_POLICY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_INVENTORY_SLOT_REDRAW_POLICY_PC34_COMPAT_H

#include "dm1_v1_champion_leader_ownership_handoff_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_INVENTORY_SLOT_REDRAW_THING_END_OF_LIST_PC34 0xFFFEu

typedef enum Dm1V1ChampionInventorySlotRedrawPolicyPc34 {
    DM1_V1_CHAMPION_INVENTORY_SLOT_SKIP_PC34 = 0,
    DM1_V1_CHAMPION_INVENTORY_SLOT_OWNED_PC34,
    DM1_V1_CHAMPION_INVENTORY_SLOT_CLEAR_PC34
} Dm1V1ChampionInventorySlotRedrawPolicyPc34;

typedef struct Dm1V1ChampionInventorySlotRedrawEntryPc34 {
    Dm1V1ChampionInventorySlotRedrawPolicyPc34 policy;
    int championIndex;
    int slotIndex;
    uint16_t thing;
} Dm1V1ChampionInventorySlotRedrawEntryPc34;

typedef struct Dm1V1ChampionInventorySlotRedrawReceiptPc34 {
    int valid;
    int inventoryChampionIndex;
    int ownedCount;
    int skipCount;
    int clearCount;
    Dm1V1ChampionInventorySlotRedrawEntryPc34 slots[CHAMPION_SLOT_COUNT];
} Dm1V1ChampionInventorySlotRedrawReceiptPc34;

/* Converts the already validated leader-ownership handoff into an explicit
 * F0296 inventory policy. An active inventory owner owns every non-empty
 * original slot and clears every THING_NONE slot; all other states skip.
 * No object, texture, or slot state is manufactured here. */
int dm1_v1_champion_inventory_slot_redraw_policy_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership,
    Dm1V1ChampionInventorySlotRedrawReceiptPc34 *outReceipt);

const char *dm1_v1_champion_inventory_slot_redraw_policy_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
